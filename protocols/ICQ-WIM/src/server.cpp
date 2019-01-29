// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-19 Miranda NG team
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
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, szUrl, &CIcqProto::OnReceiveAvatar);
		pReq->hContact = hContact;
		Push(pReq);
	}
}

void CIcqProto::CheckLastId(MCONTACT hContact, const JSONNode &ev)
{
	__int64 msgId = _wtoi64(ev["histMsgId"].as_mstring());
	__int64 lastId = getId(hContact, DB_KEY_LASTMSGID);
	if (msgId > lastId)
		setId(hContact, DB_KEY_LASTMSGID, msgId);
}

MCONTACT CIcqProto::CheckOwnMessage(const CMStringA &reqId, const CMStringA &msgId, bool bRemove)
{
	IcqOwnMessage *pOwn = nullptr;
	{
		mir_cslock lck(m_csOwnIds);
		for (auto &it : m_arOwnIds) {
			if (reqId == it->m_guid) {
				pOwn = it;
				break;
			}
		}
	}

	if (pOwn == nullptr)
		return 0;

	ProtoBroadcastAck(pOwn->m_hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pOwn->m_msgid, (LPARAM)msgId.c_str());

	MCONTACT ret = pOwn->m_hContact;
	if (bRemove) {
		// here we filter service messages for SecureIM, OTR etc, i.e. messages that 
		// weren't initialized by SRMM (we identify it by missing server id)
		if (db_event_getById(m_szModuleName, msgId) == 0)
			db_event_setId(m_szModuleName, 1, msgId);

		mir_cslock lck(m_csOwnIds);
		m_arOwnIds.remove(pOwn);
	}
	return ret;
}

void CIcqProto::CheckPassword()
{
	char mirVer[100];
	Miranda_GetVersionText(mirVer, _countof(mirVer));

	m_szAToken = getMStringA(DB_KEY_ATOKEN);
	m_iRClientId = getDword(DB_KEY_RCLIENTID);
	m_szSessionKey = getMStringA(DB_KEY_SESSIONKEY);
	if (m_szAToken.IsEmpty() || m_szSessionKey.IsEmpty()) {
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "https://api.login.icq.net/auth/clientLogin", &CIcqProto::OnCheckPassword);
		pReq << CHAR_PARAM("clientName", "Miranda NG") << CHAR_PARAM("clientVersion", mirVer) << CHAR_PARAM("devId", ICQ_APP_ID)
			<< CHAR_PARAM("f", "json") << CHAR_PARAM("tokenType", "longTerm") << INT_PARAM("s", m_dwUin) << WCHAR_PARAM("pwd", m_szPassword);
		pReq->flags |= NLHRF_NODUMPSEND;
		Push(pReq);
	}
	else StartSession();
}

void CIcqProto::ConnectionFailed(int iReason, int iErrorCode)
{
	debugLogA("ConnectionFailed -> reason %d", iReason);

	if (g_bPopupService) {
		POPUPDATAT Popup = {};
		Popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR), true);
		wcscpy_s(Popup.lptzContactName, m_tszUserName);
		switch (iReason) {
		case LOGINERR_BADUSERID:
			mir_snwprintf(Popup.lptzText, LPGENW("You have not entered an ICQ number.\nConfigure this in Options -> Network -> ICQ and try again."));
			break;
		case LOGINERR_WRONGPASSWORD:
			mir_snwprintf(Popup.lptzText, LPGENW("Connection failed.\nYour ICQ number or password was rejected (%d)."), iErrorCode);
			break;
		case LOGINERR_NONETWORK:
		case LOGINERR_NOSERVER:
			mir_snwprintf(Popup.lptzText, LPGENW("Connection failed.\nThe server is temporarily unavailable (%d)."), iErrorCode);
			break;
		default:
			mir_snwprintf(Popup.lptzText, LPGENW("Connection failed.\nUnknown error during sign on: %d"), iErrorCode);
			break;
		}

		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&Popup, 0);
	}

	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, iReason);
	ShutdownSession();
}

void CIcqProto::MoveContactToGroup(MCONTACT hContact, const wchar_t *pwszGroup, const wchar_t *pwszNewGroup)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/moveBuddy");
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("r", pReq->m_reqId)
			<< CHAR_PARAM("buddy", GetUserId(hContact)) << WCHAR_PARAM("group", pwszGroup) << WCHAR_PARAM("newGroup", pwszNewGroup);
	Push(pReq);
}

void CIcqProto::OnLoggedIn()
{
	debugLogA("CIcqProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);
	RetrieveUserInfo(0);
}

void CIcqProto::OnLoggedOut()
{
	debugLogA("CIcqProto::OnLoggedOut");
	m_bOnline = false;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

MCONTACT CIcqProto::ParseBuddyInfo(const JSONNode &buddy, MCONTACT hContact)
{
	// user chat?
	if (buddy["userType"].as_mstring() == "interop") {
		CMStringW wszChatId(buddy["aimId"].as_mstring());
		CMStringW wszChatName(buddy["friendly"].as_mstring());
		
		SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, wszChatId, wszChatName);
		if (si == nullptr)
			return INVALID_CONTACT_ID;

		Chat_AddGroup(si, TranslateT("admin"));
		Chat_AddGroup(si, TranslateT("member"));
		Chat_Control(m_szModuleName, wszChatId, m_bHideGroupchats ? WINDOW_HIDDEN : SESSION_INITDONE);
		Chat_Control(m_szModuleName, wszChatId, SESSION_ONLINE);
		return si->hContact;
	}

	DWORD dwUin = _wtol(buddy["aimId"].as_mstring());

	if (hContact == -1) {
		hContact = CreateContact(dwUin, false);
		FindContactByUIN(dwUin)->m_bInList = true;
	}

	CMStringW str(buddy["state"].as_mstring());
	setDword(hContact, "Status", StatusFromString(str));

	Json2string(hContact, buddy, "friendly", "Nick");
	Json2string(hContact, buddy, "emailId", "e-mail");
	Json2string(hContact, buddy, "cellNumber", "Cellular");
	Json2string(hContact, buddy, "phoneNumber", "Phone");
	Json2string(hContact, buddy, "workNumber", "CompanyPhone");
	Json2string(hContact, buddy, "emailId", "e-mail");

	// zero here means that a contact is currently online
	int lastSeen = buddy["lastseen"].as_int();
	setDword(hContact, DB_KEY_LASTSEEN, lastSeen ? lastSeen : time(0));

	Json2int(hContact, buddy, "official", "Official");
	Json2int(hContact, buddy, "onlineTime", DB_KEY_ONLINETS);
	Json2int(hContact, buddy, "idleTime", "IdleTS");
	Json2int(hContact, buddy, "memberSince", DB_KEY_MEMBERSINCE);

	const JSONNode &profile = buddy["profile"];
	if (profile) {
		Json2string(hContact, profile, "friendlyName", DB_KEY_ICQNICK);
		Json2string(hContact, profile, "firstName", "FirstName");
		Json2string(hContact, profile, "lastName", "LastName");
		Json2string(hContact, profile, "aboutMe", DB_KEY_ABOUT);

		time_t birthDate = profile["birthDate"].as_int();
		if (birthDate != 0) {
			struct tm *timeinfo = localtime(&birthDate);
			if (timeinfo != nullptr) {
				setWord(hContact, "BirthDay", timeinfo->tm_mday);
				setWord(hContact, "BirthMonth", timeinfo->tm_mon+1);
				setWord(hContact, "BirthYear", timeinfo->tm_year+1900);
			}
		}

		str = profile["gender"].as_mstring();
		if (!str.IsEmpty()) {
			if (str == "male")
				setByte(hContact, "Gender", 'M');
			else if (str == "female")
				setByte(hContact, "Gender", 'F');
		}

		for (auto &it : profile["homeAddress"]) {
			Json2string(hContact, it, "city", "City");
			Json2string(hContact, it, "state", "State");
			Json2string(hContact, it, "country", "Country");
		}
	}

	str = buddy["statusMsg"].as_mstring();
	if (str.IsEmpty())
		db_unset(hContact, "CList", "StatusMsg");
	else
		db_set_ws(hContact, "CList", "StatusMsg", str);

	CheckAvatarChange(hContact, buddy);
	return hContact;
}

void CIcqProto::ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &it)
{
	CMStringA szMsgId(it["msgId"].as_mstring());
	__int64 msgId = _atoi64(szMsgId);
	if (msgId > lastMsgId)
		lastMsgId = msgId;

	CMStringW wszText;
	CMStringW type(it["mediaType"].as_mstring());
	if (type == "text" || type.IsEmpty())
		wszText = it["text"].as_mstring();
	else if (type == "sticker") {
		CMStringW wszUrl, wszSticker(it["sticker"]["id"].as_mstring());
		int iCollectionId, iStickerId;
		if (2 == swscanf(wszSticker, L"ext:%d:sticker:%d", &iCollectionId, &iStickerId))
			wszUrl.Format(L"https://c.icq.com/store/stickers/%d/%d/medium", iCollectionId, iStickerId);
		else
			wszUrl = TranslateT("Unknown sticker");
		wszText.Format(L"%s\n%s", TranslateT("User sent a sticker:"), wszUrl.c_str());
	}
	else return;

	if (isChatRoom(hContact)) {
		CMStringA reqId(it["reqId"].as_mstring());
		CheckOwnMessage(reqId, szMsgId, true);

		CMStringW wszSender(it["chat"]["sender"].as_mstring());
		CMStringW wszChatId(getMStringW(hContact, "ChatRoomID"));

		GCEVENT gce = { m_szModuleName, wszChatId, GC_EVENT_MESSAGE };
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.ptszUID = wszSender;
		gce.ptszText = wszText;
		gce.time = it["time"].as_int();
		gce.bIsMe = _wtoi(wszSender) == (int)m_dwUin;
		Chat_Event(&gce);
	}
	else {
		// skip own messages, just set the server msgid
		CMStringA reqId(it["reqId"].as_mstring());
		if (CheckOwnMessage(reqId, szMsgId, true))
			return;

		// ignore duplicates
		MEVENT hDbEvent = db_event_getById(m_szModuleName, szMsgId);
		if (hDbEvent != 0)
			return;

		bool bIsOutgoing = it["outgoing"].as_bool();
		ptrA szUtf(mir_utf8encodeW(wszText));

		PROTORECVEVENT pre = {};
		pre.flags = (bIsOutgoing) ? PREF_SENT : 0;
		pre.szMsgId = szMsgId;
		pre.timestamp = it["time"].as_int();
		pre.szMessage = szUtf;
		ProtoChainRecvMsg(hContact, &pre);
	}
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
	tmp->nlc = m_ConnPool[CONN_RAPI].s;
	tmp->dataLength = tmp->m_szParam.GetLength();
	tmp->pData = tmp->m_szParam.Detach();
	tmp->szUrl = tmp->m_szUrl.GetBuffer();

	CMStringA szAgent(FORMAT, "%d Mail.ru Windows ICQ (version 10.0.1999)", DWORD(m_dwUin));
	tmp->AddHeader("User-Agent", szAgent);

	NETLIBHTTPREQUEST *reply = Netlib_HttpTransaction(m_hNetlibUser, tmp);
	if (reply != nullptr) {
		m_ConnPool[CONN_RAPI].s = reply->nlc;

		RobustReply result(reply);
		if (result.error() == 20000) {
			const JSONNode &results = result.results();
			m_szRToken = results["authToken"].as_mstring();

			// now add this token
			auto *add = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, &CIcqProto::OnAddClient);
			JSONNode request, params; params.set_name("params");
			request << CHAR_PARAM("method", "addClient") << CHAR_PARAM("reqId", add->m_reqId) << CHAR_PARAM("authToken", m_szRToken) << params;
			add->m_szParam = ptrW(json_write(&request));
			add->pUserInfo = &bRet;
			ExecuteRequest(add);
		}

		Netlib_FreeHttpRequest(reply);
	}
	else m_ConnPool[CONN_RAPI].s = nullptr;

	delete tmp;
	return bRet;
}

void CIcqProto::RetrieveUserInfo(MCONTACT hContact)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/presence/get", &CIcqProto::OnGetUserInfo);
	pReq->flags |= NLHRF_NODUMPSEND;
	pReq->hContact = hContact;
	pReq << CHAR_PARAM("f", "json") << CHAR_PARAM("aimsid", m_aimsid) << INT_PARAM("mdir", 1) << CHAR_PARAM("t", GetUserId(hContact));
	Push(pReq);
}

void CIcqProto::RetrieveUserHistory(MCONTACT hContact, __int64 startMsgId, __int64 endMsgId)
{
	if (startMsgId == 0)
		startMsgId = -1;
	if (endMsgId == 0)
		endMsgId = -1;

	if (startMsgId >= endMsgId)
		return;

	auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, &CIcqProto::OnGetUserHistory);
	pReq->flags |= NLHRF_NODUMPSEND;
	pReq->hContact = hContact;

	JSONNode request, params; params.set_name("params");
	params << CHAR_PARAM("sn", GetUserId(hContact)) << INT64_PARAM("fromMsgId", startMsgId);
	if (endMsgId != -1)
		params << INT64_PARAM("tillMsgId", endMsgId);
	params << INT_PARAM("count", 1000) << CHAR_PARAM("aimSid", m_aimsid) << CHAR_PARAM("patchVersion", "1") << CHAR_PARAM("language", "ru-ru");
	request << CHAR_PARAM("method", "getHistory") << CHAR_PARAM("reqId", pReq->m_reqId) << CHAR_PARAM("authToken", m_szRToken)
		<< INT_PARAM("clientId", m_iRClientId) << params;
	pReq->m_szParam = ptrW(json_write(&request));
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

	OnLoggedOut();

	for (auto &it : m_ConnPool)
		if (it.s)
			Netlib_Shutdown(it.s);
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
	JsonReply root(pReply);
	if (root.error() == 200) {
		RetrieveUserInfo(pReq->hContact);
		db_unset(pReq->hContact, "CList", "NotOnList");
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
	setDword(DB_KEY_RCLIENTID, m_iRClientId);
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
		ConnectionFailed(LOGINERR_WRONGPASSWORD, root.error());
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL, root.error());
		return;
	}

	JSONNode &data = root.data();
	m_szAToken = data["token"]["a"].as_mstring();
	m_szAToken = mir_urlDecode(m_szAToken);
	setString(DB_KEY_ATOKEN, m_szAToken);

	CMStringA szSessionSecret = data["sessionSecret"].as_mstring();
	CMStringA szPassTemp = m_szPassword;

	unsigned int len;
	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), szPassTemp, szPassTemp.GetLength(), (BYTE*)szSessionSecret.c_str(), szSessionSecret.GetLength(), hashOut, &len);
	m_szSessionKey = ptrA(mir_base64_encode(hashOut, sizeof(hashOut)));
	setString(DB_KEY_SESSIONKEY, m_szSessionKey);

	CMStringA szUin = data["loginId"].as_mstring();
	if (szUin)
		m_dwUin = atoi(szUin);

	StartSession();
}

void CIcqProto::OnFileContinue(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pOld)
{
	IcqFileTransfer *pTransfer = (IcqFileTransfer*)pOld->pUserInfo;

	if (pReply->resultCode != 200) {
		ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, pTransfer);
		delete pTransfer;
		return;
	}

	// file transfer succeeded?
	if (pTransfer->pfts.currentFileProgress == pTransfer->pfts.currentFileSize) {
		FileReply root(pReply);
		if (root.error() == 200) {
			ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, pTransfer);

			const JSONNode &data = root.data();
			CMStringW wszUrl(data["static_url"].as_mstring());
			SendMsg(pTransfer->pfts.hContact, 0, _T2A(wszUrl));
		}
		else ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, pTransfer);
		delete pTransfer;
		return;
	}

	// else send the next portion
	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, pTransfer->m_szHost, &CIcqProto::OnFileContinue);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("ts", time(0));
	CalcHash(pReq);
	pTransfer->FillHeaders(pReq);
	Push(pReq);

	pTransfer->pfts.currentFileTime = time(0);
	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, pTransfer, (LPARAM)&pTransfer->pfts);
}

void CIcqProto::OnFileInit(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pOld)
{
	IcqFileTransfer *pTransfer = (IcqFileTransfer*)pOld->pUserInfo;
	
	FileReply root(pReply);
	if (root.error() != 200) {
		ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, pTransfer);
		delete pTransfer;
		return;
	}

	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, pTransfer);

	const JSONNode &data = root.data();
	CMStringW wszHost(data["host"].as_mstring());
	CMStringW wszUrl(data["url"].as_mstring());
	pTransfer->m_szHost = L"https://" + wszHost + wszUrl;

	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, pTransfer->m_szHost, &CIcqProto::OnFileContinue);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("ts", time(0));
	CalcHash(pReq);
	pReq->m_szUrl.AppendChar('?');
	pReq->m_szUrl += pReq->m_szParam; pReq->m_szParam.Empty();
	pReq->pUserInfo = pTransfer;
	pTransfer->FillHeaders(pReq);
	Push(pReq);

	pTransfer->pfts.currentFileTime = time(0);
	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, pTransfer, (LPARAM)&pTransfer->pfts);
}

void CIcqProto::OnGetUserHistory(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	__int64 lastMsgId = getId(pReq->hContact, DB_KEY_LASTMSGID);

	const JSONNode &results = root.results();
	for (auto &it : results["messages"])
		ParseMessage(pReq->hContact, lastMsgId, it);

	setId(pReq->hContact, DB_KEY_LASTMSGID, lastMsgId);
}

void CIcqProto::OnGetUserInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200) {
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, nullptr);
		return;
	}

	const JSONNode &data = root.data();
	for (auto &it : data["users"])
		ParseBuddyInfo(it, pReq->hContact);

	ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, nullptr);
}

void CIcqProto::OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
		break;

	case 401:
		if (root.detail() == 1002) { // session expired
			delSetting(DB_KEY_ATOKEN);
			delSetting(DB_KEY_SESSIONKEY);
			CheckPassword();
		}
		else ConnectionFailed(LOGINERR_WRONGPASSWORD, root.error());
		return;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL, root.error());
		return;
	}

	JSONNode &data = root.data();
	m_fetchBaseURL = data["fetchBaseURL"].as_mstring();
	m_aimsid = data["aimsid"].as_mstring();

	OnLoggedIn();

	for (auto &it : data["events"])
		ProcessEvent(it);

	if (m_hPollThread == nullptr)
		m_hPollThread = ForkThreadEx(&CIcqProto::PollThread, 0, 0);
}

void CIcqProto::OnReceiveAvatar(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = pReq->hContact;

	if (pReply->resultCode != 200 || pReply->pData == nullptr) {
LBL_Error:
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai), 0);
		return;
	}

	const wchar_t *pwszExtension;
	ai.format = ProtoGetBufferFormat(pReply->pData, &pwszExtension);
	setByte(pReq->hContact, "AvatarType", ai.format);
	GetAvatarFileName(pReq->hContact, ai.filename, _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == nullptr)
		goto LBL_Error;
		
	fwrite(pReply->pData, pReply->dataLength, 1, out);
	fclose(out);

	if (pReq->hContact != 0) {
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai), 0);
		debugLogW(L"Broadcast new avatar: %s", ai.filename);
	}
	else CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
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

		mir_cslock lck(m_csOwnIds);
		m_arOwnIds.remove(ownMsg);
	}

	const JSONNode &data = root.data();
	CMStringA reqId(root.requestId());
	CMStringA msgId(data["histMsgId"].as_mstring());
	CheckOwnMessage(reqId, msgId, false);
 	CheckLastId(ownMsg->m_hContact, data);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::ProcessBuddyList(const JSONNode &ev)
{
	bool bEnableMenu = false;

	for (auto &it : ev["groups"]) {
		CMStringW szGroup = it["name"].as_mstring();
		bool bCreated = false;

		for (auto &buddy : it["buddies"]) {
			MCONTACT hContact = ParseBuddyInfo(buddy);
			if (hContact == INVALID_CONTACT_ID)
				continue;

			setWString(hContact, "IcqGroup", szGroup);

			CMStringW mirGroup(db_get_sm(hContact, "CList", "Group"));
			if (mirGroup != szGroup)
				bEnableMenu = true;

			if (mirGroup.IsEmpty()) {
				if (!bCreated) {
					Clist_GroupCreate(0, szGroup);
					bCreated = true;
				}

				db_set_ws(hContact, "CList", "Group", szGroup);
			}
		}
	}

	if (bEnableMenu)
		Menu_ShowItem(m_hUploadGroups, true);

	for (auto &it : m_arCache)
		if (!it->m_bInList)
			db_set_b(it->m_hContact, "CList", "NotOnList", 1);
}

void CIcqProto::ProcessDiff(const JSONNode &ev)
{
	for (auto &block : ev) {
		CMStringW szType = block["type"].as_mstring();
		if (szType != "updated" && szType != "created")
			continue;

		for (auto &it : block["data"]) {
			CMStringW szGroup = it["name"].as_mstring();
			bool bCreated = false;

			for (auto &buddy : it["buddies"]) {
				MCONTACT hContact = ParseBuddyInfo(buddy);
				if (hContact == INVALID_CONTACT_ID)
					continue;

				setWString(hContact, "IcqGroup", szGroup);

				if (db_get_sm(hContact, "CList", "Group").IsEmpty()) {
					if (!bCreated) {
						Clist_GroupCreate(0, szGroup);
						bCreated = true;
					}

					db_set_ws(hContact, "CList", "Group", szGroup);
				}
			}
		}
	}
}

void CIcqProto::ProcessEvent(const JSONNode &ev)
{
	const JSONNode &pData = ev["eventData"];
	CMStringW szType = ev["type"].as_mstring();
	if (szType == L"buddylist")
		ProcessBuddyList(pData);
	else if (szType == L"diff")
		ProcessDiff(pData);
	else if (szType == L"histDlgState")
		ProcessHistData(pData);
	else if (szType == L"imState")
		ProcessImState(pData);
	else if (szType == L"mchat")
		ProcessGroupChat(pData);
	else if (szType == L"myInfo")
		ProcessMyInfo(pData);
	else if (szType == L"presence")
		ProcessPresence(pData);
	else if (szType == L"typing")
		ProcessTyping(pData);
}

void CIcqProto::ProcessHistData(const JSONNode &ev)
{
	MCONTACT hContact;

	CMStringW wszId(ev["sn"].as_mstring());
	if (IsChat(wszId)) {
		SESSION_INFO *si = g_chatApi.SM_FindSession(wszId, m_szModuleName);
		if (si == nullptr)
			return;

		hContact = si->hContact;

		if (si->arUsers.getCount() == 0) {
			__int64 srvInfoVer = _wtoi64(ev["mchatState"]["infoVersion"].as_mstring());
			__int64 srvMembersVer = _wtoi64(ev["mchatState"]["membersVersion"].as_mstring());
			if (srvInfoVer != getId(hContact, "InfoVersion") || srvMembersVer != getId(hContact, "MembersVersion")) {
				auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, &CIcqProto::OnGetChatInfo);
				JSONNode request, params; params.set_name("params");
				params << WCHAR_PARAM("sn", wszId) << INT_PARAM("memberLimit", 100) << CHAR_PARAM("aimSid", m_aimsid);
				request << CHAR_PARAM("method", "getChatInfo") << CHAR_PARAM("reqId", pReq->m_reqId) << CHAR_PARAM("authToken", m_szRToken) << INT_PARAM("clientId", m_iRClientId) << params;
				pReq->m_szParam = ptrW(json_write(&request));
				pReq->pUserInfo = si;
				Push(pReq);
			}
			else LoadChatInfo(si);
		}
	}
	else hContact = CreateContact(_wtol(wszId), true);

	__int64 lastMsgId = getId(hContact, DB_KEY_LASTMSGID);
	if (lastMsgId == 0)
		lastMsgId = _wtoi64(ev["yours"]["lastRead"].as_mstring());

	// or load missing messages if any
	if (ev["unreadCnt"].as_int() > 0)
		RetrieveUserHistory(hContact, lastMsgId, _wtoi64(ev["lastMsgId"].as_mstring()));

	// check remote read
	if (g_bMessageState) {
		__int64 srvRemoteRead = _wtoi64(ev["theirs"]["lastRead"].as_mstring());
		__int64 lastRemoteRead = getId(hContact, DB_KEY_REMOTEREAD);
		if (srvRemoteRead > lastRemoteRead) {
			setId(hContact, DB_KEY_REMOTEREAD, srvRemoteRead);

			MessageReadData data(time(0), MRD_TYPE_READTIME);
			CallService(MS_MESSAGESTATE_UPDATE, hContact, (LPARAM)&data);
		}
	}

	for (auto &it : ev["tail"]["messages"])
		ParseMessage(hContact, lastMsgId, it);
	setId(hContact, DB_KEY_LASTMSGID, lastMsgId);
}

void CIcqProto::ProcessImState(const JSONNode &ev)
{
	for (auto &it : ev["imStates"]) {
		if (it["state"].as_mstring() != L"sent")
			continue;

		CMStringA reqId(it["sendReqId"].as_mstring());
		CMStringA msgId(it["histMsgId"].as_mstring());
		MCONTACT hContact = CheckOwnMessage(reqId, msgId, false);
		if (hContact) {
			CheckLastId(hContact, ev);

			if (g_bMessageState) {
				MessageReadData data(time(0), MRD_TYPE_MESSAGETIME);
				CallService(MS_MESSAGESTATE_UPDATE, hContact, (LPARAM)&data);
			}
		}
	}
}

void CIcqProto::ProcessMyInfo(const JSONNode &ev)
{
	Json2string(0, ev, "friendly", "Nick");
	CheckAvatarChange(0, ev);
}

void CIcqProto::ProcessPresence(const JSONNode &ev)
{
	DWORD dwUin = _wtol(ev["aimId"].as_mstring());

	IcqCacheItem *pCache = FindContactByUIN(dwUin);
	if (pCache) {
		setDword(pCache->m_hContact, "Status", StatusFromString(ev["state"].as_mstring()));

		Json2string(pCache->m_hContact, ev, "friendly", "Nick");
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

	while (m_bOnline) {
		CMStringA szUrl = m_fetchBaseURL;
		if (bFirst) {
			bFirst = false;
			szUrl.Append("&first=1");
		}
		else szUrl.Append("&timeout=25000");

		auto *pReq = new AsyncHttpRequest(CONN_FETCH, REQUEST_GET, szUrl, &CIcqProto::OnFetchEvents);
		if (!bFirst)
			pReq->timeout = 62000;
		ExecuteRequest(pReq);
	}

	debugLogA("Polling thread ended");
	m_hPollThread = nullptr;
}
