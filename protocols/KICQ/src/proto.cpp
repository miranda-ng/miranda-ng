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

#include "stdafx.h"

static int CompareUsers(const KicqUser *p1, const KicqUser *p2)
{
	return (p1->uin < p2->uin) ? -1 : (p1->uin > p2->uin) ? 1 : 0;
}

CKicqProto::CKicqProto(const char *szModuleName, const wchar_t *wszUserName) :
	PROTO<CKicqProto>(szModuleName, wszUserName),
	m_arUsers(50, CompareUsers),
	m_arGroups(10),
	m_wszServer(this, "Server", _A2W(KICQ_DEFAULT_SERVER)),
	m_iPort(this, "Port", KICQ_DEFAULT_PORT),
	m_wszDefaultGroup(this, "DefaultGroup", L"General")
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	HookProtoEvent(ME_OPT_INITIALISE, &CKicqProto::OnOptionsInit);
	HookProtoEvent(ME_DB_CONTACT_SETTINGCHANGED, &CKicqProto::OnSettingChanged);
	HookProtoEvent(ME_CLIST_GROUPCHANGE, &CKicqProto::OnGroupChange);
}

CKicqProto::~CKicqProto()
{}

void CKicqProto::OnModulesLoaded()
{
	// statuses are not kept between sessions
	setAllContactStatuses(ID_STATUS_OFFLINE);
}

void CKicqProto::OnShutdown()
{
	m_bTerminated = true;
	ShutdownSession();
}

/////////////////////////////////////////////////////////////////////////////////////////
// contacts

MCONTACT CKicqProto::FindContact(uint32_t uin)
{
	for (auto &hContact : AccContacts())
		if (getDword(hContact, "UIN") == uin)
			return hContact;
	return 0;
}

MCONTACT CKicqProto::CreateContact(uint32_t uin, bool bTemporary)
{
	MCONTACT hContact = FindContact(uin);
	if (hContact)
		return hContact;

	hContact = db_add_contact(m_szModuleName, bTemporary ? DBAC_NOTINLIST : 0);
	setDword(hContact, "UIN", uin);
	setWord(hContact, "Status", ID_STATUS_OFFLINE);
	return hContact;
}

KicqUser* CKicqProto::FindUser(uint32_t uin)
{
	KicqUser tmp(uin, 0);
	return m_arUsers.find(&tmp);
}

KicqUser* CKicqProto::AddUser(uint32_t uin, MCONTACT hContact)
{
	if (auto *pUser = FindUser(uin))
		return pUser;

	auto *pUser = new KicqUser(uin, hContact);
	m_arUsers.insert(pUser);
	return pUser;
}

KicqGroup* CKicqProto::FindGroup(int id)
{
	for (auto &it : m_arGroups)
		if (it->id == id)
			return it;
	return nullptr;
}

KicqGroup* CKicqProto::FindGroup(const wchar_t *name)
{
	for (auto &it : m_arGroups)
		if (!it->bNotInList && !mir_wstrcmpi(it->name, name))
			return it;
	return nullptr;
}

int CKicqProto::NewSsiId()
{
	// Jasmine: a random 16-bit id above 4096 that the roster does not use yet
	for (int tries = 0; tries < 100; tries++) {
		int id = (rand() & 0x6FFF) + 4096;
		bool bUsed = FindGroup(id) != nullptr;
		for (auto &it : m_arUsers)
			if (it->ssiId == id)
				bUsed = true;
		if (!bUsed)
			return id;
	}
	return (rand() & 0x6FFF) + 4096;
}

// the server group a contact should go to: the one named like its clist group, else the
// default group of the account (0 when that group does not exist on the server yet)
int CKicqProto::ResolveGroupFor(MCONTACT hContact)
{
	ptrW wszGroup(Clist_GetGroup(hContact));
	if (mir_wstrlen(wszGroup) == 0)
		wszGroup = mir_wstrdup(m_wszDefaultGroup);

	mir_cslock lck(m_csRoster);
	if (auto *pGroup = FindGroup(wszGroup))
		return pGroup->id;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// statuses

int CKicqProto::MirandaToIcqStatus(int iStatus)
{
	switch (iStatus) {
	case ID_STATUS_AWAY:      return ICQ_STATUS_AWAY;
	case ID_STATUS_NA:        return ICQ_STATUS_NA;
	case ID_STATUS_DND:       return ICQ_STATUS_DND;
	case ID_STATUS_OCCUPIED:  return ICQ_STATUS_OCCUPIED;
	case ID_STATUS_FREECHAT:  return ICQ_STATUS_FFC;
	case ID_STATUS_INVISIBLE: return ICQ_STATUS_INVISIBLE;
	}
	return ICQ_STATUS_ONLINE;
}

int CKicqProto::IcqToMirandaStatus(int iIcqStatus)
{
	iIcqStatus &= 0xFFFF;
	if (iIcqStatus & ICQ_STATUS_INVISIBLE)
		return ID_STATUS_INVISIBLE;
	if (iIcqStatus & ICQ_STATUS_DND)
		return ID_STATUS_DND;
	if (iIcqStatus & ICQ_STATUS_OCCUPIED)
		return ID_STATUS_OCCUPIED;
	if (iIcqStatus & ICQ_STATUS_NA)
		return ID_STATUS_NA;
	if (iIcqStatus & ICQ_STATUS_AWAY)
		return ID_STATUS_AWAY;
	if (iIcqStatus & ICQ_STATUS_FFC)
		return ID_STATUS_FREECHAT;
	return ID_STATUS_ONLINE;
}

CMStringW CKicqProto::GetOwnAwayText()
{
	char szSetting[40];
	mir_snprintf(szSetting, "AwayMsg%d", m_iStatus);
	return getMStringW(szSetting);
}

void CKicqProto::SendStatus()
{
	Send(OscarPackets::setStatus(MirandaToIcqStatus(m_iStatus), 256));
	Send(OscarPackets::setAwayText(GetOwnAwayText()));
}

int CKicqProto::SetStatus(int iNewStatus)
{
	debugLogA("SetStatus %d (current %d)", iNewStatus, m_iStatus);

	if (iNewStatus != ID_STATUS_OFFLINE && !(GetCaps(PFLAGNUM_2) & Proto_Status2Flag(iNewStatus)))
		iNewStatus = ID_STATUS_ONLINE;

	int iOldStatus = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		ShutdownSession();
		if (!m_bOnline && !IsStatusConnecting(m_iStatus) && m_iStatus != ID_STATUS_OFFLINE) {
			m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		}
		return 0;
	}

	if (m_bOnline) {
		m_iStatus = iNewStatus;
		SendStatus();
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		return 0;
	}

	if (IsStatusConnecting(m_iStatus))
		return 0;

	if (getDword("UIN") == 0 || getMStringA("Password").IsEmpty()) {
		ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_BADUSERID);
		return 0;
	}

	m_bTerminated = false;
	m_iStatus = ID_STATUS_CONNECTING;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	ForkThread(&CKicqProto::ServerThread);
	return 0;
}

int CKicqProto::SetAwayMsg(int iStatus, const wchar_t *msg)
{
	char szSetting[40];
	mir_snprintf(szSetting, "AwayMsg%d", iStatus);
	setWString(szSetting, msg ? msg : L"");

	if (m_bOnline && iStatus == m_iStatus)
		Send(OscarPackets::setAwayText(msg ? msg : L""));
	return 0;
}

void __cdecl CKicqProto::GetAwayMsgThread(void *param)
{
	// the server sends the text with the presence, so it is already known
	MCONTACT hContact = (UINT_PTR)param;
	CMStringW wszMsg(db_get_wsm(hContact, "CList", "StatusMsg"));
	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)wszMsg.c_str());
}

HANDLE CKicqProto::GetAwayMsg(MCONTACT hContact)
{
	ForkThread(&CKicqProto::GetAwayMsgThread, (void *)hContact);
	return (HANDLE)1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// capabilities

INT_PTR CKicqProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_SERVERCLIST | PF1_AUTHREQ | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSGRECV | PF1_MODEMSGSEND | PF1_NUMERICUSERID;

	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE;

	case PFLAGNUM_3:
		return PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;

	case PFLAGNUM_4:
		return PF4_SUPPORTTYPING | PF4_IMSENDOFFLINE;

	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("UIN");

	case PFLAG_MAXLENOFMESSAGE:
		return 2048;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// messaging

int CKicqProto::SendMsg(MCONTACT hContact, MEVENT, const char *pszSrc)
{
	uint32_t uin = getDword(hContact, "UIN");
	int seq = InterlockedIncrement((long *)&m_msgSeq);
	if (!m_bOnline || uin == 0) {
		ProtoBroadcastAsync(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, (LPARAM)TranslateT("You are offline"));
		return seq;
	}

	CMStringW text(Utf2T(pszSrc).get());
	text.Replace(L"\n", L"\r\n");

	char szUin[20];
	_itoa(uin, szUin, 10);

	bytes_t cookie(8, 0);
	uint64_t id = GetTickCount64() ^ (uint64_t(rand()) << 32);
	for (int i = 0; i < 8; i++)
		cookie[i] = char(id >> (8 * (7 - i)));

	bool bRelay = false, bUtf8 = false, bOnline = false;
	{
		mir_cslock lck(m_csRoster);
		if (auto *pUser = FindUser(uin)) {
			bOnline = pUser->bOnline;
			bUtf8 = pUser->bUtf8;
			bRelay = pUser->ssiId != 0 && pUser->bAuthorized && pUser->bOnline && pUser->bServerRelay;
		}
	}

	uint32_t reqId;
	{
		mir_cslock lck(m_csPending);
		reqId = m_nextMsgReq++;
		if (m_nextMsgReq > 0x7FFFFFF0)
			m_nextMsgReq = 0x1000;
		m_pendingMsgs[reqId] = { uin, seq };
	}

	if (bRelay)
		Send(OscarPackets::messageChannel2(szUin, text, bUtf8, cookie, reqId));
	else if (!bOnline) {
		// kicq.ru's offline store keeps only bytes 0x01-0x7F and stops at the first 0x00:
		// ASCII text survives as charset 0, anything else only as UCS-2 where only characters
		// >= U+0100 get through. Spaces become EN SPACE (U+2002) so that whole Cyrillic
		// sentences survive; digits and Latin letters inside them cannot.
		bool bAscii = true;
		for (int i = 0; i < text.GetLength() && bAscii; i++)
			if (text[i] >= 0x80)
				bAscii = false;

		if (bAscii)
			Send(OscarPackets::messageChannel1(szUin, OscarText::toCp1251(text), 0, cookie, reqId));
		else {
			CMStringW tmp(text);
			tmp.Replace(' ', 0x2002);
			Send(OscarPackets::messageChannel1(szUin, OscarText::toUcs2(tmp), 2, cookie, reqId));
		}
	}
	else Send(OscarPackets::messageChannel1(szUin, OscarText::toUcs2(text), 2, cookie, reqId));

	return seq;
}

int CKicqProto::UserIsTyping(MCONTACT hContact, int type)
{
	uint32_t uin = getDword(hContact, "UIN");
	if (!m_bOnline || uin == 0 || m_iStatus == ID_STATUS_INVISIBLE)
		return 0;

	char szUin[20];
	_itoa(uin, szUin, 10);
	Send(OscarPackets::typingNotify(szUin, type == PROTOTYPE_SELFTYPING_ON ? 2 : 0));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// roster edits

void CKicqProto::SsiAddContact(MCONTACT hContact, int groupId)
{
	uint32_t uin = getDword(hContact, "UIN");
	char szUin[20];
	_itoa(uin, szUin, 10);

	CMStringW wszNick(db_get_wsm(hContact, "CList", "MyHandle"));
	if (wszNick.IsEmpty())
		wszNick = getMStringW(hContact, "Nick");
	if (wszNick.IsEmpty())
		wszNick = _A2T(szUin).get();

	m_pendAdd.uin = uin;
	m_pendAdd.name = wszNick;
	m_pendAdd.groupId = groupId;
	m_pendAdd.hContact = hContact;
	{
		mir_cslock lck(m_csRoster);
		m_pendAdd.ssiId = NewSsiId();
	}

	Send(OscarPackets::ssiEditStart());
	Send(OscarPackets::addContact(szUin, wszNick, groupId, m_pendAdd.ssiId, false));
	Send(OscarPackets::ssiEditEnd());
}

MCONTACT CKicqProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	uint32_t uin = (psr->flags & PSR_UNICODE) ? _wtoi(psr->id.w) : atoi(psr->id.a);
	if (uin == 0)
		return 0;

	bool bTemporary = (flags & PALF_TEMPORARY) != 0;
	MCONTACT hContact = CreateContact(uin, bTemporary);
	if (psr->nick.w && (psr->flags & PSR_UNICODE))
		setWString(hContact, "Nick", psr->nick.w);

	if (!bTemporary) {
		Contact::PutOnList(hContact);
		Contact::Hide(hContact, false);

		if (m_bOnline) {
			int groupId = ResolveGroupFor(hContact);
			if (groupId)
				SsiAddContact(hContact, groupId);
			else {
				// the group does not exist on the server yet: create it first
				ptrW wszGroup(Clist_GetGroup(hContact));
				m_wszAddAfterGroup = mir_wstrlen(wszGroup) ? wszGroup.get() : (wchar_t *)m_wszDefaultGroup;
				m_hAddAfterGroup = hContact;
				m_pendGroup.name = m_wszAddAfterGroup;
				{
					mir_cslock lck(m_csRoster);
					m_pendGroup.groupId = NewSsiId();
				}
				Send(OscarPackets::ssiEditStart());
				Send(OscarPackets::addGroup(m_pendGroup.name, m_pendGroup.groupId));
				Send(OscarPackets::ssiEditEnd());
			}
		}
	}
	return hContact;
}

MCONTACT CKicqProto::AddToListByEvent(int flags, int, MEVENT hDbEvent)
{
	DB::EventInfo dbei(hDbEvent);
	if (!dbei || mir_strcmp(dbei.szModule, m_szModuleName))
		return 0;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST && dbei.eventType != EVENTTYPE_ADDED)
		return 0;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	if (hContact == 0)
		return 0;

	CMStringW wszUin(FORMAT, L"%u", getDword(hContact, "UIN"));
	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.id.w = wszUin.GetBuffer();
	return AddToList(flags, &psr);
}

bool CKicqProto::OnContactDeleted(MCONTACT hContact, uint32_t flags)
{
	if (!(flags & CDF_DEL_CONTACT) || !m_bOnline)
		return true;

	uint32_t uin = getDword(hContact, "UIN");
	int groupId = 0, ssiId = 0;
	{
		mir_cslock lck(m_csRoster);
		if (auto *pUser = FindUser(uin)) {
			groupId = pUser->groupId;
			ssiId = pUser->ssiId;
			m_arUsers.remove(pUser);
		}
	}

	if (ssiId != 0) {
		char szUin[20];
		_itoa(uin, szUin, 10);
		m_pendDel.uin = uin;
		Send(OscarPackets::ssiEditStart());
		Send(OscarPackets::deleteContact(szUin, groupId, ssiId));
		Send(OscarPackets::ssiEditEnd());
	}
	return true;
}

// renames a contact on the server when its clist name changes
int CKicqProto::OnSettingChanged(WPARAM hContact, LPARAM lParam)
{
	auto *cws = (DBCONTACTWRITESETTING *)lParam;
	if (hContact == 0 || !m_bOnline || mir_strcmp(cws->szModule, "CList") || mir_strcmp(cws->szSetting, "MyHandle"))
		return 0;
	if (mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	uint32_t uin = getDword(hContact, "UIN");
	CMStringW wszNick(db_get_wsm(hContact, "CList", "MyHandle"));
	if (wszNick.IsEmpty())
		wszNick.Format(L"%u", uin);

	int groupId = 0, ssiId = 0;
	bool bAuthorized = true;
	{
		mir_cslock lck(m_csRoster);
		auto *pUser = FindUser(uin);
		if (pUser == nullptr || pUser->ssiId == 0)
			return 0;
		groupId = pUser->groupId;
		ssiId = pUser->ssiId;
		bAuthorized = pUser->bAuthorized;
	}

	if (getMStringW(hContact, "SrvNick") == wszNick)
		return 0; // the change came from the server

	char szUin[20];
	_itoa(uin, szUin, 10);
	m_pendRename.uin = uin;
	m_pendRename.name = wszNick;
	Send(OscarPackets::ssiEditStart());
	Send(OscarPackets::renameContact(szUin, wszNick, groupId, ssiId, !bAuthorized));
	Send(OscarPackets::ssiEditEnd());
	return 0;
}

// moves a contact between server groups when it is dragged to another clist group
int CKicqProto::OnGroupChange(WPARAM hContact, LPARAM lParam)
{
	auto *pParam = (CLISTGROUPCHANGE *)lParam;
	if (hContact == 0 || !m_bOnline || pParam->pszNewName == nullptr)
		return 0;
	if (mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	uint32_t uin = getDword(hContact, "UIN");
	int oldGroup = 0, ssiId = 0, newGroup = 0;
	{
		mir_cslock lck(m_csRoster);
		auto *pUser = FindUser(uin);
		if (pUser == nullptr || pUser->ssiId == 0)
			return 0;

		auto *pGroup = FindGroup(pParam->pszNewName);
		if (pGroup == nullptr || pGroup->id == pUser->groupId)
			return 0;

		oldGroup = pUser->groupId;
		ssiId = pUser->ssiId;
		newGroup = pGroup->id;
		pUser->ssiId = 0;
	}

	// OSCAR has no "move": the item is deleted from the old group and added to the new one
	char szUin[20];
	_itoa(uin, szUin, 10);
	m_pendDel.uin = 0;
	Send(OscarPackets::ssiEditStart());
	Send(OscarPackets::deleteContact(szUin, oldGroup, ssiId));
	Send(OscarPackets::ssiEditEnd());
	SsiAddContact(hContact, newGroup);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// authorization

int CKicqProto::AuthRecv(MCONTACT, DB::EventInfo &dbei)
{
	return Proto_AuthRecv(m_szModuleName, dbei);
}

int CKicqProto::AuthRequest(MCONTACT hContact, const wchar_t *szMessage)
{
	uint32_t uin = getDword(hContact, "UIN");
	if (!m_bOnline || uin == 0)
		return 1;

	char szUin[20];
	_itoa(uin, szUin, 10);
	Send(OscarPackets::futureAuthGrant(szUin));
	Send(OscarPackets::authRequest(szUin, mir_wstrlen(szMessage) ? szMessage : TranslateT("Please authorize me")));
	return 0;
}

static MCONTACT AuthEventContact(const char *szModule, MEVENT hDbEvent)
{
	DB::EventInfo dbei(hDbEvent);
	if (!dbei || mir_strcmp(dbei.szModule, szModule) || dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return 0;
	return DbGetAuthEventContact(&dbei);
}

int CKicqProto::Authorize(MEVENT hDbEvent)
{
	MCONTACT hContact = AuthEventContact(m_szModuleName, hDbEvent);
	uint32_t uin = hContact ? getDword(hContact, "UIN") : 0;
	if (!m_bOnline || uin == 0)
		return 1;

	char szUin[20];
	_itoa(uin, szUin, 10);
	Send(OscarPackets::authReply(szUin, true));
	return 0;
}

int CKicqProto::AuthDeny(MEVENT hDbEvent, const wchar_t *)
{
	MCONTACT hContact = AuthEventContact(m_szModuleName, hDbEvent);
	uint32_t uin = hContact ? getDword(hContact, "UIN") : 0;
	if (!m_bOnline || uin == 0)
		return 1;

	char szUin[20];
	_itoa(uin, szUin, 10);
	Send(OscarPackets::authReply(szUin, false));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// user details & search

uint32_t CKicqProto::RequestUserInfo(uint32_t uin, MCONTACT hContact, HANDLE hSearch)
{
	uint32_t reqId;
	{
		mir_cslock lck(m_csPending);
		reqId = m_nextInfoReq++;
		auto &req = m_infoRequests[reqId];
		req.hContact = hContact;
		req.hSearch = hSearch;
		req.uin = uin;
		req.started = time(0);
	}
	Send(OscarPackets::userInfoRequest(atoi(m_szUin), uin, reqId & 0xFFFF, reqId));
	return reqId;
}

int CKicqProto::GetInfo(MCONTACT hContact, int)
{
	uint32_t uin = hContact ? getDword(hContact, "UIN") : getDword("UIN");
	if (!m_bOnline || uin == 0)
		return 1;

	RequestUserInfo(uin, hContact, nullptr);
	return 0;
}

HANDLE CKicqProto::SearchBasic(const wchar_t *id)
{
	uint32_t uin = _wtoi(id);
	if (!m_bOnline || uin == 0)
		return nullptr;

	static long searchId = 0;
	HANDLE hSearch = (HANDLE)(INT_PTR)InterlockedIncrement(&searchId);
	RequestUserInfo(uin, 0, hSearch);
	return hSearch;
}
