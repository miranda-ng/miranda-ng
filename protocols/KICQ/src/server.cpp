/*
Copyright (C) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// The connection, as Jasmine IM does it against kicq.ru:
//   connect login server -> server hello (FLAP ch1) -> XOR login (ch1)
//   -> ch4 with BOS address + cookie -> reconnect to BOS -> hello -> cookie (ch1)
//   -> 01/03 families -> 01/17 versions -> 01/18 -> 01/06 rates -> 01/07 -> 01/08 ack,
//      01/0E, 13/02, 02/02, 03/02, 04/04, 09/02, 13/04 roster request
//   -> 13/06 roster -> 02/04 caps, 04/02 icbm, 01/02 client ready, 01/1E status,
//      13/07 activate, 15/02 offline messages
//   -> online: 03/0B,0C presence; 04/07 messages; 04/0B acks; 04/14 typing; 13/19,1B auth ...

#include "stdafx.h"

#define KEEPALIVE_INTERVAL 60
#define INFO_TIMEOUT 8

static int LoginErrorToMiranda(int code)
{
	switch (code) {
	case 0x01: case 0x07: case 0x08:
		return LOGINERR_BADUSERID;
	case 0x04: case 0x05:
		return LOGINERR_WRONGPASSWORD;
	case 0x18: case 0x1D:
		return LOGINERR_TIMEOUT;
	}
	return LOGINERR_WRONGPROTOCOL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// the session thread

void CKicqProto::ServerThread(void *)
{
	m_iLoginError = 0;
	m_cookie.clear();
	m_szUin.Format("%u", getDword("UIN"));
	{
		mir_cslock lck(m_csRoster);
		m_arUsers.destroy();
		m_arGroups.destroy();
		m_offlineSeen.clear();
	}
	{
		mir_cslock lck(m_csPending);
		m_pendingMsgs.clear();
		m_infoRequests.clear();
	}

	T2Utf szServer(m_wszServer);
	int port = m_iPort;
	if (port <= 0)
		port = KICQ_DEFAULT_PORT;

	debugLogA("connecting to %s:%d as %s", szServer.get(), port, m_szUin.c_str());
	if (RunConnection(szServer, port, true) && !m_bTerminated && !m_cookie.empty())
		RunConnection(m_szBosHost, m_iBosPort, false);

	debugLogA("session finished");
	m_bOnline = false;

	int iOldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	setAllContactStatuses(ID_STATUS_OFFLINE);
	if (m_iLoginError)
		ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, m_iLoginError);
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

// one TCP connection (the login server or the BOS); returns true when it ended the way it
// should: with a redirect for the login server, or a normal close of the BOS session
bool CKicqProto::RunConnection(const char *host, int port, bool bLogin)
{
	HNETLIBCONN hConn = Netlib_OpenConnection(m_hNetlibUser, host, port, 20);
	if (hConn == nullptr) {
		debugLogA("cannot connect to %s:%d", host, port);
		m_iLoginError = LOGINERR_NONETWORK;
		return false;
	}
	{
		mir_cslock lck(m_csSend);
		m_hConn = hConn;
		m_seq = (rand() & 0x7FFF) + 1;
	}

	ConnResult res = CONN_CLOSE;
	bytes_t inbound;
	char buf[8192];

	while (!m_bTerminated) {
		NETLIBSELECT nls = {};
		nls.dwTimeout = 1000;
		nls.hReadConns[0] = hConn;
		int ret = Netlib_Select(&nls);
		if (m_bTerminated || ret < 0)
			break;
		if (ret == 0) {
			CheckTimeouts();
			continue;
		}

		int n = Netlib_Recv(hConn, buf, sizeof(buf), MSG_NODUMP);
		if (n <= 0) {
			debugLogA("connection closed by server");
			break;
		}
		inbound.append(buf, n);

		res = CONN_CONTINUE;
		while (inbound.size() >= 6 && res == CONN_CONTINUE) {
			if ((uint8_t)inbound[0] != 0x2A) {
				debugLogA("protocol error: not a FLAP frame");
				res = CONN_CLOSE;
				break;
			}
			int channel = (uint8_t)inbound[1];
			int len = ((uint8_t)inbound[4] << 8) | (uint8_t)inbound[5];
			if ((int)inbound.size() < 6 + len)
				break;

			bytes_t data = inbound.substr(6, len);
			inbound.erase(0, 6 + len);
			Netlib_Dump(hConn, data.data(), data.size(), false, 0);
			res = HandleFlap(channel, data, bLogin);
		}
		if (res != CONN_CONTINUE)
			break;
		CheckTimeouts();
	}

	{
		mir_cslock lck(m_csSend);
		m_hConn = nullptr;
	}
	Netlib_CloseHandle(hConn);
	m_bOnline = false;
	return bLogin ? (res == CONN_REDIRECT) : true;
}

void CKicqProto::ShutdownSession()
{
	m_bTerminated = true;

	mir_cslock lck(m_csSend);
	if (m_hConn) {
		bytes_t bye = OscarPackets::flap(4, m_seq++, bytes_t());
		Netlib_Send(m_hConn, bye.data(), (int)bye.size(), MSG_NODUMP);
		Netlib_Shutdown(m_hConn);
	}
}

void CKicqProto::Send(int channel, const bytes_t &payload)
{
	mir_cslock lck(m_csSend);
	if (m_hConn == nullptr)
		return;

	bytes_t frame = OscarPackets::flap(channel, m_seq, payload);
	m_seq = (m_seq + 1) & 0xFFFF;
	if (channel == 2) // the login packets carry the password
		Netlib_Dump(m_hConn, payload.data(), payload.size(), true, 0);
	Netlib_Send(m_hConn, frame.data(), (int)frame.size(), MSG_NODUMP);
}

void CKicqProto::Send(const OscarSnac &snac)
{
	Send(2, OscarPackets::snacBytes(snac));
}

void CKicqProto::CheckTimeouts()
{
	time_t now = time(0);

	if (m_bOnline && now - m_lastProbe >= KEEPALIVE_INTERVAL) {
		if (!m_bProbeAnswered) {
			debugLogA("server stopped answering");
			m_bTerminated = true;
			return;
		}
		m_bProbeAnswered = false;
		m_lastProbe = now;
		Send(OscarPackets::keepAliveProbe());
	}

	// whatever arrived is delivered: a server that does not know a chunk never sends it
	std::vector<KicqInfoRequest> expired;
	{
		mir_cslock lck(m_csPending);
		for (auto it = m_infoRequests.begin(); it != m_infoRequests.end();) {
			if (now - it->second.started >= INFO_TIMEOUT) {
				expired.push_back(it->second);
				it = m_infoRequests.erase(it);
			}
			else ++it;
		}
	}

	for (auto &req : expired) {
		if (req.hSearch) {
			CMStringW wszUin(FORMAT, L"%u", req.uin);
			PROTOSEARCHRESULT psr = {};
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.id.w = wszUin.GetBuffer();
			psr.nick.w = req.nick.GetBuffer();
			psr.firstName.w = req.firstName.GetBuffer();
			psr.lastName.w = req.lastName.GetBuffer();
			psr.email.w = req.email.GetBuffer();
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, req.hSearch, (LPARAM)&psr);
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, req.hSearch, 0);
		}
		else ProtoBroadcastAck(req.hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// FLAP level

CKicqProto::ConnResult CKicqProto::HandleFlap(int channel, const bytes_t &data, bool bLogin)
{
	switch (channel) {
	case 1:
		if (bLogin) {
			debugLogA("login server hello, sending XOR login");
			ptrA szPassword(getStringA("Password"));
			Send(1, OscarPackets::xorLogin(m_szUin, szPassword ? szPassword.get() : ""));
		}
		else {
			debugLogA("BOS hello, sending cookie");
			Send(1, OscarPackets::bosCookie(m_cookie));
		}
		break;

	case 2:
		HandleSnac(data);
		break;

	case 4:
		return HandleChannel4(data);
	}
	return CONN_CONTINUE;
}

CKicqProto::ConnResult CKicqProto::HandleChannel4(const bytes_t &data)
{
	OscarTlvList tlvs(data);
	auto *server = tlvs.find(5), *cookie = tlvs.find(6);
	if (server && cookie) {
		CMStringA bos(server->data.c_str());
		int idx = bos.Find(':');
		m_szBosHost = (idx == -1) ? bos : bos.Left(idx);
		m_iBosPort = (idx == -1) ? 0 : atoi(bos.Mid(idx + 1));
		if (m_iBosPort <= 0)
			m_iBosPort = m_iPort;
		m_cookie = cookie->data;
		debugLogA("login accepted, BOS at %s:%d", m_szBosHost.c_str(), m_iBosPort);
		Send(4, bytes_t());
		return CONN_REDIRECT;
	}

	if (auto *error = tlvs.find(8)) {
		int code = error->u16();
		debugLogA("login refused, code %d", code);
		m_iLoginError = LoginErrorToMiranda(code);
		return CONN_CLOSE;
	}

	auto *t9 = tlvs.find(9);
	if (t9 && t9->u16() == 1) {
		debugLogA("this UIN signed on from another place");
		m_iLoginError = LOGINERR_OTHERLOCATION;
	}
	else debugLogA("server closed the session");
	return CONN_CLOSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// SNAC dispatch

void CKicqProto::HandleSnac(const bytes_t &data)
{
	if (data.size() < 10)
		return;

	OscarReader r(data);
	int family = r.u16();
	int subtype = r.u16();
	int flags = r.u16();
	uint32_t reqId = r.u32();
	bytes_t body = r.rest();
	m_bProbeAnswered = true;

	// an optional block of extra data precedes the payload when bit 15 of the flags is set
	if (flags & 0x8000) {
		OscarReader e(body);
		e.skip(e.u16());
		body = e.rest();
	}

	switch (family) {
	case 0x01:
		switch (subtype) {
		case 0x03: case 0x15: Send(OscarPackets::clientFamilies()); break;
		case 0x18: Send(OscarPackets::ratesRequest()); break;
		case 0x07: HandleRates(body); break;
		}
		break;

	case 0x03:
		if (subtype == 0x0B)
			HandleUserOnline(body);
		else if (subtype == 0x0C)
			HandleUserOffline(body);
		break;

	case 0x04:
		switch (subtype) {
		case 0x01: HandleMessageError(body, reqId); break;
		case 0x07: HandleIncomingMessage(body); break;
		case 0x0B: case 0x0C: HandleMessageAck(body); break;
		case 0x14: HandleTyping(body); break;
		}
		break;

	case 0x13:
		switch (subtype) {
		case 0x06: HandleRoster(body, flags); break;
		case 0x08: case 0x09: HandleSsiItem(body, false); break;
		case 0x0A: HandleSsiItem(body, true); break;
		case 0x0E: HandleSsiResult(body, reqId); break;
		case 0x19: HandleAuthRequest(body); break;
		case 0x1B: HandleAuthReply(body); break;
		case 0x1C: HandleYouWereAdded(body); break;
		}
		break;

	case 0x15:
		if (subtype == 0x03) {
			if (reqId == REQ_OFFLINE_MSGS)
				HandleOfflineMessage(body);
			else
				HandleUserInfo(body, reqId);
		}
		break;
	}
}

void CKicqProto::HandleRates(const bytes_t &data)
{
	int groups = OscarReader(data).u16();
	debugLogA("rates: %d classes; requesting roster", groups);
	Send(OscarPackets::ackRates(groups));
	Send(OscarPackets::reqSelfInfo());
	Send(OscarPackets::reqSsiRights());
	Send(OscarPackets::reqLocationRights());
	Send(OscarPackets::reqBuddyRights());
	Send(OscarPackets::reqIcbmParams());
	Send(OscarPackets::reqPrivacyRights());
	Send(OscarPackets::requestRoster());
}

/////////////////////////////////////////////////////////////////////////////////////////
// roster

// mirrors one server-side contact into the contact list
void CKicqProto::ApplyRosterItem(uint32_t uin, int groupId, int ssiId, const OscarTlvList &tlvs, bool bFromLogin)
{
	bool bTemporary;
	CMStringW wszGroup;
	{
		mir_cslock lck(m_csRoster);
		auto *pGroup = FindGroup(groupId);
		bTemporary = pGroup && pGroup->bNotInList;
		if (pGroup && !pGroup->bNotInList)
			wszGroup = pGroup->name;
	}

	MCONTACT hContact = FindContact(uin);
	bool bNew = (hContact == 0);
	if (bNew)
		hContact = CreateContact(uin, bTemporary);

	bool bAuthorized = !tlvs.has(0x66);
	{
		mir_cslock lck(m_csRoster);
		auto *pUser = AddUser(uin, hContact);
		pUser->hContact = hContact;
		pUser->groupId = groupId;
		pUser->ssiId = bTemporary ? 0 : ssiId;
		pUser->bAuthorized = bAuthorized;
	}

	if (bAuthorized)
		delSetting(hContact, "Auth");
	else
		setByte(hContact, "Auth", 1);

	if (bTemporary)
		return;

	Contact::PutOnList(hContact);
	if (bNew || !bFromLogin)
		Contact::Hide(hContact, false);

	if (auto *nick = tlvs.find(0x131)) {
		CMStringW wszNick(OscarText::textOrCp1251(nick->data));
		if (!wszNick.IsEmpty()) {
			// SrvNick goes first: OnSettingChanged compares MyHandle with it to skip the echo
			setWString(hContact, "SrvNick", wszNick);
			if (db_get_wsm(hContact, "CList", "MyHandle") != wszNick)
				db_set_ws(hContact, "CList", "MyHandle", wszNick);
			if (getMStringW(hContact, "Nick").IsEmpty())
				setWString(hContact, "Nick", wszNick);
		}
	}

	if (!wszGroup.IsEmpty()) {
		ptrW wszOld(Clist_GetGroup(hContact));
		if (mir_wstrcmp(wszOld, wszGroup)) {
			Clist_GroupCreate(0, wszGroup);
			Clist_SetGroup(hContact, wszGroup);
		}
	}
}

void CKicqProto::HandleRoster(const bytes_t &data, int flags)
{
	struct Item { CMStringA name; int groupId, itemId; bytes_t extra; };
	std::vector<Item> contacts;

	OscarReader r(data);
	r.skip(1); // SSI version
	int count = r.u16();
	for (int i = 0; i < count && !r.atEnd(); i++) {
		bytes_t rawName = r.bytes(r.u16());
		int groupId = r.u16();
		int itemId = r.u16();
		int type = r.u16();
		bytes_t extra = r.bytes(r.u16());

		if (type == 0 && groupId != 0)
			contacts.push_back({ rawName.c_str(), groupId, itemId, extra });
		else if (type == 1 && groupId != 0) {
			OscarTlvList tlvs(extra);
			mir_cslock lck(m_csRoster);
			auto *pGroup = FindGroup(groupId);
			if (pGroup == nullptr)
				m_arGroups.insert(pGroup = new KicqGroup());
			pGroup->id = groupId;
			pGroup->name = OscarText::textOrCp1251(rawName);
			pGroup->bNotInList = tlvs.has(0x6A);
		}
		// types 2/3/14 (visible/invisible/ignore lists), 4 (privacy), 20 (icon) are not used
	}

	// groups may come after their contacts, so contacts are applied once the packet is read
	for (auto &it : contacts) {
		uint32_t uin = strtoul(it.name, nullptr, 10);
		if (uin)
			ApplyRosterItem(uin, it.groupId, it.itemId, OscarTlvList(it.extra), true);
	}

	if ((flags & 0x0001) == 0) {
		debugLogA("roster: %d groups, %d contacts", m_arGroups.getCount(), m_arUsers.getCount());
		FinishLogin();
	}
}

void CKicqProto::FinishLogin()
{
	int iOldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	if (m_iStatus < ID_STATUS_ONLINE)
		m_iStatus = ID_STATUS_ONLINE;

	auto *pVer = &pluginInfoEx.version;
	Send(OscarPackets::setUserInfo(HIBYTE(HIWORD(*pVer)), LOBYTE(HIWORD(*pVer)), HIBYTE(LOWORD(*pVer))));
	Send(OscarPackets::setIcbmParams());
	Send(OscarPackets::clientReady());
	Send(OscarPackets::setDcInfo(MirandaToIcqStatus(m_iStatus), 256, 11));
	CMStringW wszAway(GetOwnAwayText());
	if (!wszAway.IsEmpty())
		Send(OscarPackets::setAwayText(wszAway));
	Send(OscarPackets::rosterActivate());
	Send(OscarPackets::offlineMsgsRequest(atoi(m_szUin), m_seq));

	m_bOnline = true;
	m_bProbeAnswered = true;
	m_lastProbe = time(0);
	debugLogA("online");
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

// 13/08 (add), 13/09 (update), 13/0A (delete) pushed by the server
void CKicqProto::HandleSsiItem(const bytes_t &data, bool bDeleted)
{
	OscarReader r(data);
	bytes_t rawName = r.bytes(r.u16());
	int groupId = r.u16();
	int itemId = r.u16();
	int type = r.u16();
	OscarTlvList tlvs(r.bytes(r.u16()));

	if (type == 1 && groupId != 0) {
		mir_cslock lck(m_csRoster);
		auto *pGroup = FindGroup(groupId);
		if (bDeleted) {
			if (pGroup)
				m_arGroups.remove(pGroup);
			return;
		}
		if (pGroup == nullptr)
			m_arGroups.insert(pGroup = new KicqGroup());
		pGroup->id = groupId;
		pGroup->name = OscarText::textOrCp1251(rawName);
		pGroup->bNotInList = tlvs.has(0x6A);
		return;
	}

	uint32_t uin = strtoul(rawName.c_str(), nullptr, 10);
	if (type != 0 || uin == 0)
		return;

	if (bDeleted) {
		mir_cslock lck(m_csRoster);
		if (auto *pUser = FindUser(uin))
			if (pUser->ssiId == itemId)
				pUser->ssiId = pUser->groupId = 0;
		return;
	}

	ApplyRosterItem(uin, groupId, itemId, tlvs, false);
}

void CKicqProto::HandleSsiResult(const bytes_t &data, uint32_t reqId)
{
	int result = OscarReader(data).u16();
	debugLogA("SSI result %d for request %u", result, reqId);

	switch (reqId) {
	case REQ_ADD_CONTACT:
		if (m_pendAdd.uin == 0)
			break;

		if (result == 0 || result == 0x0E) {
			char szUin[20];
			_itoa(m_pendAdd.uin, szUin, 10);
			{
				mir_cslock lck(m_csRoster);
				auto *pUser = AddUser(m_pendAdd.uin, m_pendAdd.hContact);
				pUser->groupId = m_pendAdd.groupId;
				pUser->ssiId = m_pendAdd.ssiId;
				pUser->bAuthorized = (result == 0);
			}
			setWString(m_pendAdd.hContact, "SrvNick", m_pendAdd.name);

			if (result == 0x0E) {
				// the contact requires authorization: add it flagged, then ask
				setByte(m_pendAdd.hContact, "Auth", 1);
				Send(OscarPackets::ssiEditStart());
				Send(OscarPackets::addContact(szUin, m_pendAdd.name, m_pendAdd.groupId, m_pendAdd.ssiId, true));
				Send(OscarPackets::ssiEditEnd());
				Send(OscarPackets::futureAuthGrant(szUin));
				Send(OscarPackets::authRequest(szUin, TranslateT("Please authorize me")));
			}
			m_pendAdd.uin = 0;
		}
		else {
			debugLogA("adding %u failed, code %d", m_pendAdd.uin, result);
			m_pendAdd.uin = 0;
		}
		break;

	case REQ_DEL_CONTACT:
		m_pendDel.uin = 0;
		break;

	case REQ_RENAME:
		if (result == 0 && m_pendRename.uin)
			if (MCONTACT hContact = FindContact(m_pendRename.uin))
				setWString(hContact, "SrvNick", m_pendRename.name);
		m_pendRename.uin = 0;
		break;

	case REQ_ADD_GROUP:
		if (result == 0 && !m_pendGroup.name.IsEmpty()) {
			{
				mir_cslock lck(m_csRoster);
				auto *pGroup = new KicqGroup();
				pGroup->id = m_pendGroup.groupId;
				pGroup->name = m_pendGroup.name;
				pGroup->bNotInList = false;
				m_arGroups.insert(pGroup);
			}

			if (m_hAddAfterGroup && m_wszAddAfterGroup == m_pendGroup.name)
				SsiAddContact(m_hAddAfterGroup, m_pendGroup.groupId);
		}
		m_hAddAfterGroup = 0;
		m_wszAddAfterGroup.Empty();
		m_pendGroup.name.Empty();
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// presence

void CKicqProto::HandleUserOnline(const bytes_t &data)
{
	OscarReader r(data);
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);
	r.skip(2); // warning level
	r.skip(2); // TLV count
	OscarTlvList tlvs(r);

	MCONTACT hContact;
	bool bFull;
	{
		mir_cslock lck(m_csRoster);
		auto *pUser = FindUser(uin);
		if (pUser == nullptr)
			return;

		hContact = pUser->hContact;
		pUser->bOnline = true;

		// an abbreviated status update carries no capabilities and must not wipe them
		auto *caps = tlvs.find(0x0D), *shortCaps = tlvs.find(0x19);
		bFull = caps || shortCaps;
		if (bFull) {
			pUser->bUtf8 = pUser->bServerRelay = false;
			CMStringA szMirVer;
			if (caps) {
				OscarReader cr(caps->data);
				while (cr.remaining() >= 16) {
					bytes_t raw = cr.bytes(16);
					CMStringA cap(OscarText::hex(raw));
					if (cap == CAP_UTF8)
						pUser->bUtf8 = true;
					else if (cap == CAP_SERVER_RELAY)
						pUser->bServerRelay = true;
					else if (!memcmp(raw.data(), "Jasmine ver\xFF", 12))
						szMirVer.Format("Jasmine IM %d.%d.%d", (uint8_t)raw[12], (uint8_t)raw[13], (uint8_t)raw[14]);
					else if (!memcmp(raw.data(), "Miranda KICQ", 12) && szMirVer.IsEmpty())
						szMirVer = "Miranda NG KICQ";
				}
			}
			if (shortCaps) {
				OscarReader cr(shortCaps->data);
				while (cr.remaining() >= 2) {
					int sc = cr.u16();
					if (sc == 0x134E)
						pUser->bUtf8 = true;
					else if (sc == 0x1349)
						pUser->bServerRelay = true;
				}
			}
			if (!szMirVer.IsEmpty())
				setString(hContact, "MirVer", szMirVer);
		}
	}

	auto *st = tlvs.find(6);
	int iStatus = st ? OscarReader(st->data, 2).u16() : ICQ_STATUS_ONLINE;
	setWord(hContact, "Status", IcqToMirandaStatus(iStatus));

	if (auto *since = tlvs.find(3))
		setDword(hContact, "LogonTS", since->u32());

	CMStringW wszAway;
	if (auto *avail = tlvs.find(0x1D)) {
		OscarTlvList inner(avail->data);
		if (auto *t2 = inner.find(2)) {
			OscarReader ar(t2->data);
			wszAway = OscarText::textOrCp1251(ar.bytes(ar.u16()));
			wszAway.Trim();
		}
	}
	if (wszAway.IsEmpty())
		db_unset(hContact, "CList", "StatusMsg");
	else
		db_set_ws(hContact, "CList", "StatusMsg", wszAway);
}

void CKicqProto::HandleUserOffline(const bytes_t &data)
{
	OscarReader r(data);
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);

	MCONTACT hContact;
	{
		mir_cslock lck(m_csRoster);
		auto *pUser = FindUser(uin);
		if (pUser == nullptr)
			return;
		pUser->bOnline = false;
		hContact = pUser->hContact;
	}

	setWord(hContact, "Status", ID_STATUS_OFFLINE);
	db_unset(hContact, "CList", "StatusMsg");
	CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);
}

/////////////////////////////////////////////////////////////////////////////////////////
// messages

void CKicqProto::DeliverMessage(uint32_t uin, const CMStringW &text, time_t when)
{
	MCONTACT hContact = FindContact(uin);
	if (hContact == 0)
		hContact = CreateContact(uin, true);
	{
		mir_cslock lck(m_csRoster);
		AddUser(uin, hContact);
	}

	CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

	T2Utf szText(text);
	DB::EventInfo dbei;
	dbei.iTimestamp = when;
	dbei.pBlob = szText.get();
	ProtoChainRecvMsg(hContact, dbei);
}

void CKicqProto::HandleIncomingMessage(const bytes_t &data)
{
	OscarReader r(data);
	bytes_t cookie = r.bytes(8);
	int channel = r.u16();
	bytes_t sender = r.ascii8();
	r.skip(2);                           // warning level
	int infoCount = r.u16();
	OscarTlvList info(r, infoCount);     // sender's presence TLVs
	OscarTlvList msgTlvs(r);

	uint32_t uin = strtoul(sender.c_str(), nullptr, 10);
	if (uin == 0)
		return;

	CMStringW text;
	bool bHaveText = false, bOffline = false;
	time_t when = time(0);

	if (channel == 1) {
		auto *t2 = msgTlvs.find(2);
		if (!t2)
			return;

		OscarReader m(t2->data);
		while (m.remaining() >= 4) {
			int fragId = m.u8();
			m.u8();                          // fragment version
			int len = m.u16();
			if (fragId == 1 && len >= 4) {
				int charset = m.u16();
				m.u16();                      // sub-charset
				text = OscarText::cleanup(OscarText::decodeCharset(m.bytes(len - 4), charset));
				bHaveText = true;
			}
			else m.skip(len);
		}

		if (auto *ts = msgTlvs.find(0x16)) {
			when = ts->u32();
			bOffline = true;
		}
	}
	else if (channel == 2) {
		auto *t5 = msgTlvs.find(5);
		if (!t5)
			return;

		OscarReader m(t5->data);
		int msgType = m.u16();
		m.skip(8);                          // cookie again
		CMStringA guid(OscarText::hex(m.bytes(16)));
		if (guid == CAP_FILE || msgType != 0)
			return;                          // file transfer, or cancel/accept of a rendezvous

		OscarTlvList inner(m);
		auto *ext = inner.find(0x2711);
		if (!ext)
			return;

		OscarReader e(ext->data);
		e.skip(e.u16le());                  // protocol/plugin block
		e.skip(e.u16le());                  // second block
		int type = e.u8();
		e.u8();                              // flags
		e.u16le();                           // status
		e.u16le();                           // priority
		int len = e.u16le();
		if (type != 1)
			return;                          // only plain text (0x1A is an Xtraz plugin request)

		bytes_t raw = e.bytes(len > 0 ? len - 1 : 0);
		e.skip(1);

		// fg/bg colours, then an optional encoding GUID
		bool bUtf8 = false;
		if (e.remaining() >= 8) {
			e.u32(); e.u32();
			if (e.remaining() >= 4) {
				bytes_t g = e.bytes(e.u32le());
				if (strstr(g.c_str(), "0946134E") || strstr(g.c_str(), "0946134e"))
					bUtf8 = true;
			}
		}

		if (bUtf8 && OscarText::isUtf8(raw))
			text = OscarText::fromUtf8(raw);
		else
			text = OscarText::decodeGuess(raw);
		text = OscarText::cleanup(text);
		bHaveText = true;
	}
	else return;

	if (!bHaveText)
		return;

	if (!bOffline)
		Send(OscarPackets::messageAck(cookie, sender.c_str()));
	DeliverMessage(uin, text, when);
}

void CKicqProto::HandleOfflineMessage(const bytes_t &data)
{
	OscarReader r(data);
	r.skip(10);
	int type = r.u16();
	if (type == 0x4100) {
		r.skip(2);
		uint32_t uin = r.u32le();
		int year = r.u16le();
		int month = r.u8();
		int day = r.u8();
		r.u8(); r.u8();                     // hour, minute
		r.skip(2);
		bytes_t raw = r.bytes(r.u16le());
		CMStringW text(OscarText::cleanup(OscarText::decodeGuess(raw)));

		CMStringW key(FORMAT, L"%u|%s", uin, text.c_str());
		{
			mir_cslock lck(m_csRoster);
			if (m_offlineSeen.count(key))
				return;
			m_offlineSeen.insert(key);
		}

		// the server's timestamp is in some private timezone (off by hours from UTC): the
		// date is kept, the time of day is not trusted
		time_t when = time(0);
		struct tm tmNow;
		localtime_s(&tmNow, &when);
		if (year > 1990 && (year < tmNow.tm_year + 1900 || (year == tmNow.tm_year + 1900 &&
			(month < tmNow.tm_mon + 1 || (month == tmNow.tm_mon + 1 && day < tmNow.tm_mday))))) {
			struct tm tmMsg = {};
			tmMsg.tm_year = year - 1900;
			tmMsg.tm_mon = month - 1;
			tmMsg.tm_mday = day;
			tmMsg.tm_hour = 23;
			tmMsg.tm_min = 59;
			tmMsg.tm_isdst = -1;
			when = mktime(&tmMsg);
		}
		DeliverMessage(uin, text, when);
	}
	else if (type == 0x4200) {
		Send(OscarPackets::deleteOfflineMsgs(atoi(m_szUin), m_seq));
		Send(OscarPackets::anotherOfflineMsgsRequest());
	}
}

// kicq.ru rewrites message cookies on the way back, so an ack is matched to the oldest
// message still pending for that uin
void CKicqProto::HandleMessageAck(const bytes_t &data)
{
	OscarReader r(data);
	r.skip(8);                              // cookie
	r.u16();                                // channel
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);

	int seq = 0;
	{
		mir_cslock lck(m_csPending);
		for (auto it = m_pendingMsgs.begin(); it != m_pendingMsgs.end(); ++it) {
			if (it->second.uin == uin) {
				seq = it->second.seq;
				m_pendingMsgs.erase(it);
				break;
			}
		}
	}

	if (seq)
		if (MCONTACT hContact = FindContact(uin))
			ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)seq, 0);
}

// errors answer the request id they refer to: our keep-alive probe (error 9, by design),
// an ack we sent for a stranger's message (error 14), or a message of ours
void CKicqProto::HandleMessageError(const bytes_t &data, uint32_t reqId)
{
	int code = OscarReader(data).u16();

	KicqPendingMsg msg;
	{
		mir_cslock lck(m_csPending);
		auto it = m_pendingMsgs.find(reqId);
		if (it == m_pendingMsgs.end())
			return;
		msg = it->second;
		m_pendingMsgs.erase(it);
	}

	debugLogA("message to %u failed, error %d", msg.uin, code);
	if (MCONTACT hContact = FindContact(msg.uin)) {
		CMStringW wszError(FORMAT, TranslateT("The server refused the message (error %d)"), code);
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)msg.seq, (LPARAM)wszError.c_str());
	}
}

void CKicqProto::HandleTyping(const bytes_t &data)
{
	OscarReader r(data);
	r.skip(10);
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);
	int type = r.u16();

	if (MCONTACT hContact = FindContact(uin))
		CallService(MS_PROTO_CONTACTISTYPING, hContact, type == 2 ? PROTOTYPE_CONTACTTYPING_INFINITE : PROTOTYPE_CONTACTTYPING_OFF);
}

/////////////////////////////////////////////////////////////////////////////////////////
// user details: the meta "full user info" reply comes in chunks; 0xFA is the last one

static CMStringW MetaString(OscarReader &r)
{
	bytes_t raw = r.bytes(r.u16le());
	CMStringW s(OscarText::decodeGuess(raw));
	s.Trim();
	return s;
}

void CKicqProto::HandleUserInfo(const bytes_t &data, uint32_t reqId)
{
	KicqInfoRequest req;
	{
		mir_cslock lck(m_csPending);
		auto it = m_infoRequests.find(reqId);
		if (it == m_infoRequests.end())
			return;
		req = it->second;
	}

	OscarReader r(data);
	r.skip(14);                             // TLV head, chunk length, our UIN, 0x07DA, sequence
	int chunk = r.u16le();
	int result = r.u8();

	MCONTACT hContact = req.hContact;
	auto setStr = [this, hContact](const char *szSetting, const CMStringW &val)
	{
		if (hContact == 0)
			return;
		if (val.IsEmpty())
			delSetting(hContact, szSetting);
		else
			setWString(hContact, szSetting, val);
	};

	bool bDone = (result != 0x0A);          // the UIN is unknown (or the request was refused)
	if (!bDone) {
		switch (chunk) {
		case 0xC8:                           // basic
			req.nick = MetaString(r);
			req.firstName = MetaString(r);
			req.lastName = MetaString(r);
			req.email = MetaString(r);
			setStr("Nick", req.nick);
			setStr("FirstName", req.firstName);
			setStr("LastName", req.lastName);
			setStr("e-mail", req.email);
			setStr("City", MetaString(r));
			setStr("State", MetaString(r));
			setStr("Phone", MetaString(r));
			MetaString(r);                    // fax
			MetaString(r);                    // street
			setStr("Cellular", MetaString(r));
			break;

		case 0xDC:                           // more
			if (hContact) {
				int age = r.u16le();
				int gender = r.u8();
				CMStringW homepage(MetaString(r));
				int year = r.u16le();
				int month = r.u8();
				int day = r.u8();
				if (age) setWord(hContact, "Age", age); else delSetting(hContact, "Age");
				if (gender == 1) setByte(hContact, "Gender", 'F');
				else if (gender == 2) setByte(hContact, "Gender", 'M');
				else delSetting(hContact, "Gender");
				setStr("Homepage", homepage);
				if (year && month && day) {
					setWord(hContact, "BirthYear", year);
					setByte(hContact, "BirthMonth", month);
					setByte(hContact, "BirthDay", day);
				}
			}
			break;

		case 0xD2:                           // work
			for (int i = 0; i < 6; i++)
				MetaString(r);                 // city, state, phone, fax, address, zip
			r.u16le();                        // country
			setStr("Company", MetaString(r));
			setStr("CompanyDepartment", MetaString(r));
			setStr("CompanyPosition", MetaString(r));
			break;

		case 0xE6:                           // about
			setStr("About", MetaString(r));
			break;

		case 0xFA:                           // affiliations
			bDone = true;
			break;
		}
	}

	{
		mir_cslock lck(m_csPending);
		if (bDone)
			m_infoRequests.erase(reqId);
		else
			m_infoRequests[reqId] = req;
	}

	if (bDone) {
		if (req.hSearch) {
			CMStringW wszUin(FORMAT, L"%u", req.uin);
			PROTOSEARCHRESULT psr = {};
			psr.cbSize = sizeof(psr);
			psr.flags = PSR_UNICODE;
			psr.id.w = wszUin.GetBuffer();
			psr.nick.w = req.nick.GetBuffer();
			psr.firstName.w = req.firstName.GetBuffer();
			psr.lastName.w = req.lastName.GetBuffer();
			psr.email.w = req.email.GetBuffer();
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, req.hSearch, (LPARAM)&psr);
			ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, req.hSearch, 0);
		}
		else ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// authorization

void CKicqProto::HandleAuthRequest(const bytes_t &data)
{
	OscarReader r(data);
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);
	CMStringW reason(OscarText::textOrCp1251(r.bytes(r.u16())));
	if (uin == 0)
		return;

	MCONTACT hContact = FindContact(uin);
	if (hContact == 0)
		hContact = CreateContact(uin, true);

	char szUin[20];
	_itoa(uin, szUin, 10);
	DB::AUTH_BLOB blob(hContact, T2Utf(getMStringW(hContact, "Nick")), nullptr, nullptr, szUin, T2Utf(reason));
	blob.set_uin(uin);

	DB::EventInfo dbei;
	dbei.iTimestamp = time(0);
	dbei.cbBlob = blob.size();
	dbei.pBlob = blob;
	ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&dbei);
}

void CKicqProto::HandleAuthReply(const bytes_t &data)
{
	OscarReader r(data);
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);
	bool bGranted = r.u8() == 1;
	debugLogA("authorization %s by %u", bGranted ? "granted" : "denied", uin);

	MCONTACT hContact = FindContact(uin);
	if (hContact == 0 || !bGranted)
		return;

	delSetting(hContact, "Auth");

	mir_cslock lck(m_csRoster);
	if (auto *pUser = FindUser(uin))
		pUser->bAuthorized = true;
}

void CKicqProto::HandleYouWereAdded(const bytes_t &data)
{
	OscarReader r(data);
	uint32_t uin = strtoul(r.ascii8().c_str(), nullptr, 10);
	if (uin == 0)
		return;

	MCONTACT hContact = FindContact(uin);
	if (hContact == 0)
		hContact = CreateContact(uin, true);

	char szUin[20];
	_itoa(uin, szUin, 10);
	DB::AUTH_BLOB blob(hContact, T2Utf(getMStringW(hContact, "Nick")), nullptr, nullptr, szUin, nullptr);
	blob.set_uin(uin);

	DB::EventInfo dbei;
	dbei.szModule = m_szModuleName;
	dbei.iTimestamp = time(0);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_ADDED;
	dbei.cbBlob = blob.size();
	dbei.pBlob = blob;
	db_event_add(0, &dbei);
}
