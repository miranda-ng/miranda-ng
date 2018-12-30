// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

#pragma comment(lib, "libeay32.lib")

void CIcqProto::CheckAvatarChange(MCONTACT hContact, const JSONNode &ev)
{
	CMStringW wszIconId(ev["iconId"].as_mstring());
	CMStringW oldIconID(getMStringW(hContact, "IconId"));
	if (wszIconId != oldIconID) {
		setWString(hContact, "IconId", wszIconId);

		CMStringA szUrl(ev["buddyIcon"].as_mstring());
		auto *p = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, szUrl, &CIcqProto::OnReceiveAvatar);
		p->pUserInfo = (void*)hContact;
		Push(p);
	}
}

void CIcqProto::CheckNickChange(MCONTACT hContact, const JSONNode &ev)
{
	CMStringW wszNick(ev["profile"]["friendlyName"].as_mstring());
	if (wszNick.IsEmpty())
		wszNick = ev["friendly"].as_mstring();
	if (!wszNick.IsEmpty())
		setWString(hContact, "Nick", wszNick);
}

void CIcqProto::CheckPassword()
{
	char mirVer[100];
	Miranda_GetVersionText(mirVer, _countof(mirVer));

	m_szAToken = getMStringA("AToken");
	m_szRToken = getMStringA("RToken");
	m_iRClientId = getDword("RClientID");
	m_szSessionKey = getMStringA("SessionKey");
	if (m_szAToken.IsEmpty() || m_szSessionKey.IsEmpty()) {
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "https://api.login.icq.net/auth/clientLogin", &CIcqProto::OnCheckPassword);
		pReq << CHAR_PARAM("clientName", "Miranda NG") << CHAR_PARAM("clientVersion", mirVer) << CHAR_PARAM("devId", ICQ_APP_ID)
			<< CHAR_PARAM("f", "json") << CHAR_PARAM("tokenType", "longTerm") << INT_PARAM("s", m_dwUin) << WCHAR_PARAM("pwd", m_szPassword);
		pReq->flags |= NLHRF_NODUMPSEND;
		Push(pReq);
	}
	else StartSession();
}

void CIcqProto::ConnectionFailed(int iReason)
{
	debugLogA("ConnectionFailed -> reason %d", iReason);

	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, iReason);
	ShutdownSession();
}

void CIcqProto::OnLoggedIn()
{
	debugLogA("CIcqProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);
}

void CIcqProto::OnLoggedOut()
{
	debugLogA("CIcqProto::OnLoggedOut");
	m_bOnline = false;
	m_bTerminated = true;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

MCONTACT CIcqProto::ParseBuddyInfo(const JSONNode &buddy)
{
	DWORD dwUin = _wtol(buddy["aimId"].as_mstring());

	MCONTACT hContact = CreateContact(dwUin, false);
	FindContactByUIN(dwUin)->m_bInList = true;

	CheckNickChange(hContact, buddy);

	CMStringW str(buddy["state"].as_mstring());
	setDword(hContact, "Status", StatusFromString(str));

	const JSONNode &profile = buddy["profile"];
	if (profile) {
		str = profile["firstName"].as_mstring();
		if (!str.IsEmpty())
			setWString(hContact, "FirstName", str);

		str = profile["lastName"].as_mstring();
		if (!str.IsEmpty())
			setWString(hContact, "LastName", str);
	}

	int lastLogin = buddy["lastseen"].as_int();
	if (lastLogin)
		setDword(hContact, "LastSeen", lastLogin);

	str = buddy["statusMsg"].as_mstring();
	if (str.IsEmpty())
		db_unset(hContact, "CList", "StatusMsg");
	else
		db_set_ws(hContact, "CList", "StatusMsg", str);

	CheckAvatarChange(hContact, buddy);
	return hContact;
}

bool CIcqProto::RefreshRobustToken()
{
	if (!m_szRToken.IsEmpty())
		return true;

	bool bRet = false;
	auto *tmp = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER "/genToken");

	time_t ts = time(0);
	tmp << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", ICQ_APP_ID)
		<< CHAR_PARAM("nonce", CMStringA(FORMAT, "%d-%d", ts, rand() % 10)) << INT_PARAM("ts", ts);
	CalcHash(tmp);
	tmp->flags |= NLHRF_PERSISTENT;
	tmp->nlc = m_ConnPool[CONN_RAPI];
	tmp->dataLength = tmp->m_szParam.GetLength();
	tmp->pData = tmp->m_szParam.Detach();
	tmp->szUrl = tmp->m_szUrl.GetBuffer();

	CMStringA szAgent(FORMAT, "%d Mail.ru Windows ICQ (version 10.0.1999)", DWORD(m_dwUin));
	tmp->AddHeader("User-Agent", szAgent);

	NETLIBHTTPREQUEST *reply = Netlib_HttpTransaction(m_hNetlibUser, tmp);
	m_ConnPool[CONN_RAPI] = nullptr;
	if (reply != nullptr) {
		RobustReply result(reply);
		if (result.error() == 20000) {
			const JSONNode &results = result.results();
			m_szRToken = results["authToken"].as_mstring();
			setString("RToken", m_szRToken);

			// now add this token
			auto *add = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, &CIcqProto::OnAddClient);
			JSONNode request, params; params.set_name("params");
			request << CHAR_PARAM("method", "addClient") << CHAR_PARAM("reqId", add->m_reqId) << CHAR_PARAM("authToken", m_szRToken) << params;
			add->m_szParam = ptrW(json_write(&request));
			add->pUserInfo = &bRet;
			ExecuteRequest(add);
		}

		m_ConnPool[CONN_RAPI] = reply->nlc;
		Netlib_FreeHttpRequest(reply);
	}

	delete tmp;
	return bRet;
}

void CIcqProto::RetrieveUserInfo(MCONTACT hContact)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/presence/get", &CIcqProto::OnGetUserInfo);
	pReq->flags |= NLHRF_NODUMPSEND;
	pReq->pUserInfo = (void*)hContact;
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << INT_PARAM("mdir", 1) << INT_PARAM("t", getDword(hContact, "UIN"));
	Push(pReq);
}

void CIcqProto::SetServerStatus(int iStatus)
{
	const char *szStatus = "online";
	int invisible = 0;

	switch (iStatus) {
	case ID_STATUS_OFFLINE: szStatus = "offline"; break;
	case ID_STATUS_NA: szStatus = "occupied"; break;
	case ID_STATUS_AWAY:
	case ID_STATUS_DND: szStatus = "away"; break;
	case ID_STATUS_INVISIBLE:
		invisible = 1;	
	}

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/presence/setState");
	pReq->flags |= NLHRF_NODUMPSEND;
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("r", pReq->m_reqId) 
		<< CHAR_PARAM("view", szStatus) << INT_PARAM("invisible", invisible);
	Push(pReq);

	int iOldStatus = m_iStatus; m_iStatus = iStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

void CIcqProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("CIcqProto::ShutdownSession");

	// shutdown all resources
	if (m_hWorkerThread)
		SetEvent(m_evRequestsQueue);

	for (auto &it : m_ConnPool)
		if (it)
			Netlib_Shutdown(it);

	OnLoggedOut();
}

/////////////////////////////////////////////////////////////////////////////////////////

#define CAPS "094613504c7f11d18222444553540000,094613514c7f11d18222444553540000,094613534c7f11d18222444553540000,094613544c7f11d18222444553540000,094613594c7f11d18222444553540000,0946135b4c7f11d18222444553540000,0946135a4c7f11d18222444553540000"
#define EVENTS "myInfo,presence,buddylist,typing,dataIM,userAddedToBuddyList,webrtcMsg,mchat,hist,hiddenChat,diff,permitDeny,imState,notification,apps"
#define FIELDS "aimId,buddyIcon,bigBuddyIcon,iconId,bigIconId,largeIconId,displayId,friendly,offlineMsg,state,statusMsg,userType,phoneNumber,cellNumber,smsNumber,workNumber,otherNumber,capabilities,ssl,abPhoneNumber,moodIcon,lastName,abPhones,abContactName,lastseen,mute,livechat,official"

void CIcqProto::StartSession()
{
	ptrA szDeviceId(getStringA("DeviceId"));
	if (szDeviceId == nullptr) {
		UUID deviceId;
		UuidCreate(&deviceId);
		RPC_CSTR szId;
		UuidToStringA(&deviceId, &szId);
		szDeviceId = mir_strdup((char*)szId);
		setString("DeviceId", szDeviceId);
		RpcStringFreeA(&szId);
	}

	int ts = time(0);
	CMStringA nonce(FORMAT, "%d-2", ts);

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/aim/startSession", &CIcqProto::OnStartSession);

	pReq << CHAR_PARAM("a", m_szAToken) << INT_PARAM("activeTimeout", 180) << CHAR_PARAM("assertCaps", CAPS)
		<< INT_PARAM("buildNumber", __BUILD_NUM) << CHAR_PARAM("deviceId", szDeviceId) << CHAR_PARAM("events", EVENTS) 
		<< CHAR_PARAM("f", "json") << CHAR_PARAM("imf", "plain") << CHAR_PARAM("inactiveView", "offline") 
		<< CHAR_PARAM("includePresenceFields", FIELDS) << CHAR_PARAM("invisible", "false")
		<< CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("mobile", 0) << CHAR_PARAM("nonce", nonce) << CHAR_PARAM("r", pReq->m_reqId) 
		<< INT_PARAM("rawMsg", 0) << INT_PARAM("sessionTimeout", 7776000) << INT_PARAM("ts", ts) << CHAR_PARAM("view", "online");

	CalcHash(pReq);

	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnAddBuddy(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (MCONTACT)pReq->pUserInfo;

	JsonReply root(pReply);
	if (root.error() == 200) {
		RetrieveUserInfo(getDword(hContact, "UIN"));
		db_unset(hContact, "CList", "NotOnList");
	}
}

void CIcqProto::OnAddClient(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	bool *pRet = (bool*)pReq->pUserInfo;

	RobustReply reply(pReply);
	if (reply.error() != 20000) {
		*pRet = false;
		return;
	}

	const JSONNode &results = reply.results();
	m_iRClientId = results["clientId"].as_int();
	setDword("RClientID", m_iRClientId);
	*pRet = true;
}

void CIcqProto::OnCheckPassword(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
		break;

	case 330:
	case 440:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONNode &data = root.data();
	m_szAToken = data["token"]["a"].as_mstring();
	m_szAToken = ptrA(mir_urlDecode(m_szAToken));
	setString("AToken", m_szAToken);

	CMStringA szSessionSecret = data["sessionSecret"].as_mstring();
	CMStringA szPassTemp = m_szPassword;

	unsigned int len;
	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), szPassTemp, szPassTemp.GetLength(), (BYTE*)szSessionSecret.c_str(), szSessionSecret.GetLength(), hashOut, &len);
	m_szSessionKey = ptrA(mir_base64_encode(hashOut, sizeof(hashOut)));
	setString("SessionKey", m_szSessionKey);

	CMStringA szUin = data["loginId"].as_mstring();
	if (szUin)
		m_dwUin = atoi(szUin);

	StartSession();
}

void CIcqProto::OnGetUserInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (MCONTACT)pReq->pUserInfo;

	JsonReply root(pReply);
	if (root.error() != 200) {
		ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, nullptr);
		return;
	}

	const JSONNode &data = root.data();
	for (auto &it : data["users"])
		ParseBuddyInfo(it);

	ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, nullptr);
}

void CIcqProto::OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
		break;

	case 401:
		if (root.detail() == 1002) { // session expired
			delSetting("AToken");
			delSetting("SessionKey");
			CheckPassword();
		}
		else ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL);
		return;
	}

	JSONNode &data = root.data();
	m_fetchBaseURL = data["fetchBaseURL"].as_mstring();
	m_aimsid = data["aimsid"].as_mstring();

	OnLoggedIn();

	for (auto &it : data["events"])
		ProcessEvent(it);

	m_hPollThread = ForkThreadEx(&CIcqProto::PollThread, 0, 0);
}

void CIcqProto::OnReceiveAvatar(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	MCONTACT hContact = (MCONTACT)pReq->pUserInfo;

	const wchar_t *pwszExtension;
	PROTO_AVATAR_INFORMATION ai;
	ai.hContact = hContact;
	ai.format = ProtoGetBufferFormat(pReply->pData, &pwszExtension);
	setByte(hContact, "AvatarType", ai.format);
	GetAvatarFileName(hContact, ai.filename, _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out != nullptr) {
		fwrite(pReply->pData, pReply->dataLength, 1, out);
		fclose(out);

		if (hContact != 0) {
			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai), 0);
			debugLogW(L"Broadcast new avatar: %s", ai.filename);
		}
		else CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
	}
	else ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai), 0);
}

void CIcqProto::OnSearchResults(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000) {
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)pReq, 0);
		return;
	}

	const JSONNode &results = root.results();

	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	for (auto &it : results["data"]) {
		const JSONNode &anketa = it["anketa"];

		CMStringW wszId = it["sn"].as_mstring();
		CMStringW wszNick = anketa["nickname"].as_mstring();
		CMStringW wszFirst = anketa["firstName"].as_mstring();
		CMStringW wszLast = anketa["lastName"].as_mstring();

		psr.id.w = wszId.GetBuffer();
		psr.nick.w = wszNick.GetBuffer();
		psr.firstName.w = wszFirst.GetBuffer();
		psr.lastName.w = wszLast.GetBuffer();
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)pReq, LPARAM(&psr));
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)pReq);
}

void CIcqProto::OnSendMessage(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	IcqOwnMessage *ownMsg = (IcqOwnMessage*)pReq->pUserInfo;

	JsonReply root(pReply);
	if (root.error() != 200) {
		ProtoBroadcastAck(ownMsg->m_hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)ownMsg->m_msgid, 0);
		for (auto &it : m_arOwnIds) {
			if (it == ownMsg) {
				m_arOwnIds.remove(m_arOwnIds.indexOf(&it));
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::ProcessBuddyList(const JSONNode &ev)
{
	for (auto &it : ev["groups"]) {
		CMStringW szGroup = it["name"].as_mstring();
		Clist_GroupCreate(0, szGroup);

		for (auto &buddy : it["buddies"]) {
			MCONTACT hContact = ParseBuddyInfo(buddy);
			db_set_ws(hContact, "CList", "Group", szGroup);
		}
	}

	for (auto &it : m_arCache)
		if (!it->m_bInList)
			db_set_b(it->m_hContact, "CList", "NotOnList", 1);
}

void CIcqProto::ProcessEvent(const JSONNode &ev)
{
	const JSONNode &pData = ev["eventData"];
	CMStringW szType = ev["type"].as_mstring();
	if (szType == L"buddylist")
		ProcessBuddyList(pData);
	else if (szType == L"histDlgState")
		ProcessHistData(pData);
	else if (szType == L"myInfo")
		ProcessMyInfo(pData);
	else if (szType == L"presence")
		ProcessPresence(pData);
	else if (szType == L"typing")
		ProcessTyping(pData);
}

void CIcqProto::ProcessHistData(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["sn"].as_mstring());

	MCONTACT hContact = CreateContact(dwUin, true);

	for (auto &it : ev["tail"]["messages"]) {
		CMStringA msgId(it["msgId"].as_mstring());
		CMStringW type(it["mediaType"].as_mstring());
		if (type != "text")
			continue;

		// ignore duplicates
		MEVENT hDbEvent = db_event_getById(m_szModuleName, msgId);
		if (hDbEvent != 0)
			continue;

		// skip own messages, just set the server msgid
		bool bSkipped = false;
		CMStringA reqId(it["reqId"].as_mstring());
		for (auto &ownMsg : m_arOwnIds)
			if (!mir_strcmp(reqId, ownMsg->m_guid)) {
				bSkipped = true;
				if (m_bSlowSend)
					ProtoBroadcastAck(ownMsg->m_hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)ownMsg->m_msgid, (LPARAM)msgId.c_str());
				m_arOwnIds.remove(m_arOwnIds.indexOf(&ownMsg));
				break;
			}

		if (!bSkipped) {
			bool bIsOutgoing = it["outgoing"].as_bool();
			ptrA szUtf(mir_utf8encodeW(it["text"].as_mstring()));

			PROTORECVEVENT pre = {};
			pre.flags = (bIsOutgoing) ? PREF_SENT : 0;
			pre.szMsgId = msgId;
			pre.timestamp = it["time"].as_int();
			pre.szMessage = szUtf;
			ProtoChainRecvMsg(hContact, &pre);
		}
	}
}

void CIcqProto::ProcessMyInfo(const JSONNode &ev)
{
	CheckNickChange(0, ev);
	CheckAvatarChange(0, ev);
}

void CIcqProto::ProcessPresence(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["aimId"].as_mstring());

	IcqCacheItem *pCache = FindContactByUIN(dwUin);
	if (pCache) {
		setDword(pCache->m_hContact, "Status", StatusFromString(ev["state"].as_mstring()));

		CheckAvatarChange(pCache->m_hContact, ev);
	}
}

void CIcqProto::ProcessTyping(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["aimId"].as_mstring());
	CMStringW wszStatus = ev["typingStatus"].as_mstring();

	IcqCacheItem *pCache = FindContactByUIN(dwUin);
	if (pCache) {
		if (wszStatus == "typing")
			CallService(MS_PROTO_CONTACTISTYPING, pCache->m_hContact, 60);
		else 
			CallService(MS_PROTO_CONTACTISTYPING, pCache->m_hContact, PROTOTYPE_CONTACTTYPING_OFF);
	}
}

void CIcqProto::OnFetchEvents(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	if (root.error() != 200) {
		ShutdownSession();
		return;
	}

	JSONNode &data = root.data();
	m_fetchBaseURL = data["fetchBaseURL"].as_mstring();

	for (auto &it : data["events"])
		ProcessEvent(it);
}

void __cdecl CIcqProto::PollThread(void*)
{
	debugLogA("Polling thread started");
	bool bFirst = true;

	while (!m_bTerminated) {
		CMStringA szUrl = m_fetchBaseURL;
		if (bFirst) {
			bFirst = false;
			szUrl.Append("&first=1");
		}
		else szUrl.Append("&timeout=60000");

		auto *pReq = new AsyncHttpRequest(CONN_FETCH, REQUEST_GET, szUrl, &CIcqProto::OnFetchEvents);
		if (!bFirst)
			pReq->timeout = 62000;
		ExecuteRequest(pReq);
	}

	debugLogA("Polling thread ended");
	m_hPollThread = nullptr;
}
