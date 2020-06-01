// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright � 2018-20 Miranda NG team
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
	CMStringW wszIconId(ev["bigIconId"].as_mstring());
	if (wszIconId.IsEmpty())
		wszIconId = ev["iconId"].as_mstring();
	
	if (!wszIconId.IsEmpty()) {
		CMStringW oldIconID(getMStringW(hContact, "IconId"));
		if (wszIconId == oldIconID) {
			wchar_t wszFullName[MAX_PATH];
			GetAvatarFileName(hContact, wszFullName, _countof(wszFullName));
			if (_waccess(wszFullName, 0) == 0)
				return;
		}

		setWString(hContact, "IconId", wszIconId);

		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/expressions/get", &CIcqProto::OnReceiveAvatar);
		pReq << CHAR_PARAM("f", "native") << WCHAR_PARAM("t", GetUserId(hContact)) << CHAR_PARAM("type", "bigBuddyIcon");
		pReq->hContact = hContact;
		Push(pReq);
	}
	else delSetting(hContact, "IconId");
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
			<< CHAR_PARAM("f", "json") << CHAR_PARAM("tokenType", "longTerm") << WCHAR_PARAM("s", m_szOwnId) << CHAR_PARAM("pwd", m_szPassword);
		#ifndef _DEBUG
			pReq->flags |= NLHRF_NODUMPSEND;
		#endif
		Push(pReq);
	}
	else StartSession();
}

void CIcqProto::CheckStatus()
{
	time_t now = time(0);
	int diff1 = m_iTimeDiff1, diff2 = m_iTimeDiff2;

	for (auto &it : m_arCache) {
		// this contact is really offline and is on the first timer
		// if the first timer is expired, we clear it and look for the second status
		if (diff1 && it->m_timer1 && now - it->m_timer1 > diff1) {
			it->m_timer1 = 0;

			// if the second timer is set up, activate it
			if (m_iTimeDiff2) {
				setDword(it->m_hContact, "Status", m_iStatus2);
				it->m_timer2 = now;
			}
			// if the second timer is not set, simply mark a contact as offline
			else setDword(it->m_hContact, "Status", ID_STATUS_OFFLINE);
			continue;
		}

		// if the second timer is expired, set status to offline
		if (diff2 && it->m_timer2 && now - it->m_timer2 > m_iTimeDiff2) {
			it->m_timer2 = 0;
			setDword(it->m_hContact, "Status", ID_STATUS_OFFLINE);
		}
	}
}

void CIcqProto::ConnectionFailed(int iReason, int iErrorCode)
{
	debugLogA("ConnectionFailed -> reason %d", iReason);

	if (m_bErrorPopups) {
		POPUPDATAW Popup = {};
		Popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR), true);
		wcscpy_s(Popup.lpwzContactName, m_tszUserName);
		switch (iReason) {
		case LOGINERR_BADUSERID:
			mir_snwprintf(Popup.lpwzText, TranslateT("You have not entered an ICQ number or password.\nConfigure this in Options -> Network -> ICQ and try again."));
			break;
		case LOGINERR_WRONGPASSWORD:
			mir_snwprintf(Popup.lpwzText, TranslateT("Connection failed.\nYour ICQ number or password was rejected (%d)."), iErrorCode);
			break;
		case LOGINERR_NONETWORK:
		case LOGINERR_NOSERVER:
			mir_snwprintf(Popup.lpwzText, TranslateT("Connection failed.\nThe server is temporarily unavailable (%d)."), iErrorCode);
			break;
		default:
			mir_snwprintf(Popup.lpwzText, TranslateT("Connection failed.\nUnknown error during sign on: %d"), iErrorCode);
			break;
		}
		PUAddPopupW(&Popup);
	}

	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, iReason);
	ShutdownSession();
}

void CIcqProto::MoveContactToGroup(MCONTACT hContact, const wchar_t *pwszGroup, const wchar_t *pwszNewGroup)
{
	// otherwise we'll get a server error
	if (!mir_wstrlen(pwszGroup))
		return;

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/buddylist/moveBuddy") << AIMSID(this) << WCHAR_PARAM("buddy", GetUserId(hContact)) 
		<< GROUP_PARAM("group", pwszGroup);
	if (mir_wstrlen(pwszNewGroup))
		pReq << GROUP_PARAM("newGroup", pwszNewGroup);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnLoggedIn()
{
	debugLogA("CIcqProto::OnLoggedIn");
	m_bOnline = true;
	m_impl.m_heartBeat.Start(1000);

	for (auto &it : m_arCache)
		it->m_timer1 = it->m_timer2 = 0;

	SetServerStatus(m_iDesiredStatus);
	RetrieveUserInfo(0);
	GetPermitDeny();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnLoggedOut()
{
	debugLogA("CIcqProto::OnLoggedOut");
	m_bOnline = false;
	m_impl.m_heartBeat.Stop();

	for (auto &it : m_arCache)
		it->m_timer1 = it->m_timer2 = 0;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::MarkAsRead(MCONTACT hContact)
{
	if (!m_bOnline)
		return;

	m_impl.m_markRead.Start(200);

	IcqCacheItem *pCache = FindContactByUIN(GetUserId(hContact));
	if (pCache) {
		mir_cslock lck(m_csMarkReadQueue);
		if (m_arMarkReadQueue.indexOf(pCache) == -1)
			m_arMarkReadQueue.insert(pCache);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CIcqProto::ParseBuddyInfo(const JSONNode &buddy, MCONTACT hContact)
{
	// user chat?
	CMStringW wszId(buddy["aimId"].as_mstring());
	if (IsChat(wszId)) {
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

	if (hContact == -1) {
		hContact = CreateContact(wszId, false);
		FindContactByUIN(wszId)->m_bInList = true;
	}

	CMStringA szVer;
	bool bVersionDetected = false, bSecureIM = false;

	for (auto &it : buddy["capabilities"]) {
		CMStringW wszCap(it.as_mstring());
		if (wszCap.GetLength() != 32)
			continue;

		BYTE cap[16];
		hex2binW(wszCap, cap, sizeof(cap));
		if (!memcmp(cap, "MiNG", 4)) { // Miranda
			int v[4];
			if (4 == swscanf(wszCap.c_str() + 16, L"%04x%04x%04x%04x", &v[0], &v[1], &v[2], &v[3])) {
				szVer.Format("Miranda NG %d.%d.%d.%d (ICQ %d.%d.%d.%d)", v[0], v[1], v[2], v[3], cap[4], cap[5], cap[6], cap[7]);
				setString(hContact, "MirVer", szVer);
				bVersionDetected = true;
			}
		}
		else if (wszCap == _A2W(NG_CAP_SECUREIM)) {
			bSecureIM = bVersionDetected = true;
		}
		else if (!memcmp(cap, "Mod by Mikanoshi", 16)) {
			szVer = "R&Q build by Mikanoshi";
			bVersionDetected = true;
		}
		else if (!memcmp(cap, "Mandarin IM", 11)) {
			szVer = "Mandarin IM";
			bVersionDetected = true;
		}
	}

	if (bVersionDetected) {
		if (bSecureIM)
			szVer.Append(" + SecureIM");
		setString(hContact, "MirVer", szVer);
	}
	else delSetting(hContact, "MirVer");

	CMStringW str(buddy["state"].as_mstring());
	setDword(hContact, "Status", StatusFromString(str));

	const JSONNode &var = buddy["friendly"];
	if (var)
		setWString(hContact, "Nick", var.as_mstring());

	if (buddy["deleted"].as_bool()) {
		setByte(hContact, "IcqDeleted", 1);
		Contact_PutOnList(hContact);
	}

	Json2string(hContact, buddy, "emailId", "Email");
	Json2string(hContact, buddy, "cellNumber", "Cellular");
	Json2string(hContact, buddy, "phoneNumber", "Phone");
	Json2string(hContact, buddy, "workNumber", "CompanyPhone");

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

		ptrW wszNick(getWStringA(hContact, "Nick"));
		if (!wszNick) {
			CMStringW srvNick = profile["friendlyName"].as_mstring();
			if (!srvNick.IsEmpty())
				setWString(hContact, "Nick", srvNick);
		}

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

void CIcqProto::ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &it, bool bCreateRead, bool bLocalTime)
{
	CMStringA szMsgId(it["msgId"].as_mstring());
	__int64 msgId = _atoi64(szMsgId);
	if (msgId > lastMsgId)
		lastMsgId = msgId;

	CMStringW wszText;
	const JSONNode &sticker = it["sticker"];
	if (sticker) {
		CMStringW wszUrl, wszSticker(sticker["id"].as_mstring());
		int iCollectionId, iStickerId;
		if (2 == swscanf(wszSticker, L"ext:%d:sticker:%d", &iCollectionId, &iStickerId))
			wszUrl.Format(L"https://c.icq.com/store/stickers/%d/%d/medium", iCollectionId, iStickerId);
		else
			wszUrl = TranslateT("Unknown sticker");
		wszText.Format(L"%s\n%s", TranslateT("User sent a sticker:"), wszUrl.c_str());
	}
	else {
		wszText = it["text"].as_mstring();
		wszText.TrimRight();

		// user added you
		if (it["class"].as_mstring() == L"event" && it["eventTypeId"].as_mstring() == L"27:33000") {
			if (bLocalTime) {
				CMStringA id = getMStringA(hContact, DB_KEY_ID);
				int pos = id.Find('@');
				CMStringA nick = (pos == -1) ? id : id.Left(pos);
				DB::AUTH_BLOB blob(hContact, nick, nullptr, nullptr, id, nullptr);

				PROTORECVEVENT pre = {};
				pre.timestamp = (DWORD)time(0);
				pre.lParam = blob.size();
				pre.szMessage = blob;
				ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
			}
			return;
		}
	}

	int iMsgTime = (bLocalTime) ? time(0) : it["time"].as_int();

	if (isChatRoom(hContact)) {
		CMStringA reqId(it["reqId"].as_mstring());
		CheckOwnMessage(reqId, szMsgId, true);

		CMStringW wszSender(it["chat"]["sender"].as_mstring());
		CMStringW wszChatId(getMStringW(hContact, "ChatRoomID"));

		GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
		gce.pszID.w = wszChatId;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.pszUID.w = wszSender;
		gce.pszText.w = wszText;
		gce.time = iMsgTime;
		gce.bIsMe = wszSender == m_szOwnId;
		Chat_Event(&gce);
	}
	else {
		// skip own messages, just set the server msgid
		CMStringA reqId(it["reqId"].as_mstring());
		if (CheckOwnMessage(reqId, szMsgId, true))
			return;

		// ignore duplicates
		MEVENT hDbEvent = db_event_getById(m_szModuleName, szMsgId);
		if (hDbEvent != 0) {
			debugLogA("Message %s already exists", szMsgId.c_str());
			return;
		}

		bool bIsOutgoing = it["outgoing"].as_bool();
		if (!bCreateRead && !bIsOutgoing && wszText.Left(26) == L"https://files.icq.net/get/") {
			CMStringW wszUrl(wszText.Mid(26));
			int idx = wszUrl.Find(' ');
			if (idx != -1)
				wszUrl.Truncate(idx);

			CMStringA szUrl(FORMAT, ICQ_FILE_SERVER "/info/%S/", wszUrl.c_str());
			auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, szUrl, &CIcqProto::OnFileInfo);
			pReq->hContact = hContact;
			pReq << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("previews", "600");
			Push(pReq);

			db_event_setId(m_szModuleName, 1, szMsgId);

			MarkAsRead(hContact);
			return;
		}

		debugLogA("Adding message %d:%s (CR=%d)", hContact, szMsgId.c_str(), bCreateRead);

		ptrA szUtf(mir_utf8encodeW(wszText));

		PROTORECVEVENT pre = {};
		if (bIsOutgoing) pre.flags |= PREF_SENT;
		if (bCreateRead) pre.flags |= PREF_CREATEREAD;
		pre.szMsgId = szMsgId;
		pre.timestamp = iMsgTime;
		pre.szMessage = szUtf;
		ProtoChainRecvMsg(hContact, &pre);
	}
}

bool CIcqProto::RefreshRobustToken()
{
	if (!m_szRToken.IsEmpty())
		return true;

	auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER "/genToken", &CIcqProto::OnGenToken);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif

	int ts = TS();
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", ICQ_APP_ID) << CHAR_PARAM("nonce", CMStringA(FORMAT, "%d-%d", ts, rand() % 10)) << INT_PARAM("ts", ts);
	CalcHash(pReq);

	CMStringA szAgent(FORMAT, "%S Mail.ru Windows ICQ (version 10.0.1999)", (wchar_t*)m_szOwnId);
	pReq->AddHeader("User-Agent", szAgent);
	if (!ExecuteRequest(pReq))
		return false;

	// now add this token
	bool bRet = false;
	pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER "/addClient", &CIcqProto::OnAddClient);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("f", "json") << CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("ts", ts)
		<< CHAR_PARAM("client", "icq") << CHAR_PARAM("reqId", pReq->m_reqId) << CHAR_PARAM("authToken", m_szRToken);
	pReq->pUserInfo = &bRet;
	if (!ExecuteRequest(pReq))
		return false;

	return bRet;
}

void CIcqProto::RetrieveUserInfo(MCONTACT hContact)
{
	auto *pReq = UserInfoRequest(hContact);

	if (hContact == INVALID_CONTACT_ID) {
		int i = 0;
		for (auto &it : m_arCache) {
			if (i == 0)
				pReq = UserInfoRequest(hContact);

			pReq << WCHAR_PARAM("t", GetUserId(it->m_hContact));
			if (i == 100) {
				i = 0;
				Push(pReq);

				pReq = UserInfoRequest(hContact);
			}
			else i++;
		}
	}
	else pReq << WCHAR_PARAM("t", GetUserId(hContact));

	Push(pReq);
}

AsyncHttpRequest* CIcqProto::UserInfoRequest(MCONTACT hContact)
{
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/presence/get", &CIcqProto::OnGetUserInfo);
	pReq->hContact = hContact;
	pReq << AIMSID(this) << INT_PARAM("mdir", 1) << INT_PARAM("capabilities", 1);
	return pReq;
}

void CIcqProto::RetrieveUserHistory(MCONTACT hContact, __int64 startMsgId, bool bCreateRead)
{
	if (startMsgId == 0)
		startMsgId = -1;

	auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER, &CIcqProto::OnGetUserHistory);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq->hContact = hContact;
	pReq->pUserInfo = (bCreateRead) ? pReq : 0;

	__int64 patchVer = getId(hContact, DB_KEY_PATCHVER);
	if (patchVer == 0)
		patchVer = 1;

	JSONNode request, params; params.set_name("params");
	params << WCHAR_PARAM("sn", GetUserId(hContact)) << INT64_PARAM("fromMsgId", startMsgId);
	params << INT_PARAM("count", 1000) << CHAR_PARAM("aimSid", m_aimsid) << SINT64_PARAM("patchVersion", patchVer) << CHAR_PARAM("language", "ru-ru");
	request << CHAR_PARAM("method", "getHistory") << CHAR_PARAM("reqId", pReq->m_reqId) << params;
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

	Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/presence/setState")
		<< AIMSID(this) << CHAR_PARAM("view", szStatus) << INT_PARAM("invisible", invisible));

	if (iStatus == ID_STATUS_OFFLINE && !getByte(DB_KEY_PHONEREG)) {
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, ICQ_API_SERVER "/aim/endSession", &CIcqProto::OnSessionEnd);
		pReq << AIMSID(this) << INT_PARAM("invalidateToken", 1);
		Push(pReq);
	}

	int iOldStatus = m_iStatus; m_iStatus = iStatus;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
}

void CIcqProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("CIcqProto::ShutdownSession");

	// shutdown all resources
	DropQueue();

	if (m_hWorkerThread)
		SetEvent(m_evRequestsQueue);

	OnLoggedOut();

	for (auto &it : m_ConnPool) {
		if (it.s) {
			Netlib_Shutdown(it.s);
			it.s = nullptr;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

#define EVENTS "myInfo,presence,buddylist,typing,dataIM,userAddedToBuddyList,mchat,hist,hiddenChat,diff,permitDeny,imState,notification,apps"
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

	int ts = TS();
	CMStringA nonce(FORMAT, "%d-2", ts);
	CMStringA caps(WIM_CAP_UNIQ_REQ_ID "," WIM_CAP_EMOJI "," WIM_CAP_MAIL_NOTIFICATIONS "," WIM_CAP_INTRO_DLG_STATE);
	if (g_bSecureIM) {
		caps.AppendChar(',');
		caps.Append(NG_CAP_SECUREIM);
	}

	MFileVersion v;
	Miranda_GetFileVersion(&v);
	caps.AppendFormat(",%02x%02x%02x%02x%02x%02x%02x%02x%04x%04x%04x%04x", 'M', 'i', 'N', 'G',
		__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM, v[0], v[1], v[2], v[3]);

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/aim/startSession", &CIcqProto::OnStartSession);
	pReq << CHAR_PARAM("a", m_szAToken) << INT_PARAM("activeTimeout", 180) << CHAR_PARAM("assertCaps", caps)
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
	if (root.error() != 200)
		return;

	CMStringW wszId = getMStringW(pReq->hContact, DB_KEY_ID);
	for (auto &it : root.data()["results"]) {
		if (it["buddy"].as_mstring() != wszId)
			continue;

		int iResultCode = it["resultCode"].as_int();
		if (iResultCode != 0) {
			debugLogA("Contact %d failed to add: error %d", pReq->hContact, iResultCode);

			POPUPDATAW Popup = {};
			Popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR));
			wcsncpy_s(Popup.lpwzText, TranslateT("Buddy addition failed"), _TRUNCATE);
			wcsncpy_s(Popup.lpwzContactName, Clist_GetContactDisplayName(pReq->hContact), _TRUNCATE);
			Popup.iSeconds = 20;
			PUAddPopupW(&Popup);

			// Contact_RemoveFromList(pReq->hContact);
		}

		RetrieveUserInfo(pReq->hContact);
		Contact_PutOnList(pReq->hContact);
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
	mir_urlDecode(m_szAToken.GetBuffer());
	setString(DB_KEY_ATOKEN, m_szAToken);

	CMStringA szSessionSecret = data["sessionSecret"].as_mstring();
	CMStringA szPassTemp = m_szPassword;

	unsigned int len;
	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), szPassTemp, szPassTemp.GetLength(), (BYTE*)szSessionSecret.c_str(), szSessionSecret.GetLength(), hashOut, &len);
	m_szSessionKey = ptrA(mir_base64_encode(hashOut, sizeof(hashOut)));
	setString(DB_KEY_SESSIONKEY, m_szSessionKey);

	CMStringW szUin = data["loginId"].as_mstring();
	if (szUin)
		m_szOwnId = szUin;

	int srvTS = data["hostTime"].as_int();
	m_iTimeShift = (srvTS) ? time(0) - srvTS : 0;

	StartSession();
}

void CIcqProto::OnFileContinue(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pOld)
{
	IcqFileTransfer *pTransfer = (IcqFileTransfer*)pOld->pUserInfo;

	switch (pReply->resultCode) {
	case 200: // final ok
	case 206: // partial ok
		break;

	default:
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

			JSONNode bundle, contents; contents.set_name("captionedContent");
			contents << WCHAR_PARAM("caption", pTransfer->m_wszDescr) << WCHAR_PARAM("url", wszUrl);
			bundle << CHAR_PARAM("mediaType", "text") << CHAR_PARAM("text", "") << contents;
			CMStringW wszParts(FORMAT, L"[%s]", ptrW(json_write(&bundle)).get());

			if (!pTransfer->m_wszDescr.IsEmpty())
				wszUrl += L" " + pTransfer->m_wszDescr;

			int id = InterlockedIncrement(&m_msgId);
			auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/im/sendIM", &CIcqProto::OnSendMessage);

			auto *pOwn = new IcqOwnMessage(pTransfer->pfts.hContact, id, pReq->m_reqId);
			pReq->pUserInfo = pOwn;
			{
				mir_cslock lck(m_csOwnIds);
				m_arOwnIds.insert(pOwn);
			}

			pReq << AIMSID(this) << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", ICQ_APP_ID) << CHAR_PARAM("mentions", "") << WCHAR_PARAM("message", wszUrl)
				<< CHAR_PARAM("offlineIM", "true") << WCHAR_PARAM("parts", wszParts) << WCHAR_PARAM("t", GetUserId(pTransfer->pfts.hContact)) << INT_PARAM("ts", TS());
			Push(pReq);

		}
		else ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, pTransfer);
		delete pTransfer;
		return;
	}

	// else send the next portion
	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, pTransfer->m_szHost, &CIcqProto::OnFileContinue);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("ts", TS());
	CalcHash(pReq);
	pReq->m_szUrl.AppendChar('?');
	pReq->m_szUrl += pReq->m_szParam; pReq->m_szParam.Empty();
	pReq->pUserInfo = pTransfer;
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
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("k", ICQ_APP_ID) << INT_PARAM("ts", TS());
	CalcHash(pReq);
	pReq->m_szUrl.AppendChar('?');
	pReq->m_szUrl += pReq->m_szParam; pReq->m_szParam.Empty();
	pReq->pUserInfo = pTransfer;
	pTransfer->FillHeaders(pReq);
	Push(pReq);

	pTransfer->pfts.currentFileTime = time(0);
	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, pTransfer, (LPARAM)&pTransfer->pfts);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnFileInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 200)
		return;

	auto &data = root.result()["info"];
	std::string szUrl(data["dlink"].as_string());
	if (szUrl.empty())
		return;

	mir_urlDecode(&*szUrl.begin());

	CMStringW wszDescr(data["file_name"].as_mstring());

	auto *ft = new IcqFileTransfer(pReq->hContact, szUrl.c_str());
	ft->pfts.totalBytes = ft->pfts.currentFileSize = data["file_size"].as_int();
	ft->pfts.szCurrentFile.w = ft->m_wszFileName.GetBuffer();

	PROTORECVFILE pre = { 0 };
	pre.dwFlags = PRFF_UNICODE;
	pre.fileCount = 1;
	pre.timestamp = time(0);
	pre.files.w = &ft->m_wszShortName;
	pre.descr.w = wszDescr;
	pre.lParam = (LPARAM)ft;
	ProtoChainRecvFile(pReq->hContact, &pre);
}

void CIcqProto::OnFileRecv(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	auto *ft = (IcqFileTransfer*)pReq->pUserInfo;

	if (pReply->resultCode != 200) {
LBL_Error:
		FileCancel(pReq->hContact, ft);
		return;
	}

	ft->hWaitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (ProtoBroadcastAck(ft->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, ft, (LPARAM)&ft->pfts))
		WaitForSingleObject(ft->hWaitEvent, INFINITE);
	CloseHandle(ft->hWaitEvent);

	debugLogW(L"Saving to [%s]", ft->pfts.szCurrentFile.w);
	int fileId = _wopen(ft->pfts.szCurrentFile.w, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY, _S_IREAD | _S_IWRITE);
	if (fileId == -1) {
		debugLogW(L"Cannot open [%s] for writing", ft->pfts.szCurrentFile.w);
		goto LBL_Error;
	}

	int result = _write(fileId, pReply->pData, pReply->dataLength);
	_close(fileId);
	if (result != pReply->dataLength) {
		debugLogW(L"Error writing data into [%s]", ft->pfts.szCurrentFile.w);
		goto LBL_Error;
	}

	ft->pfts.totalProgress += pReply->dataLength;
	ft->pfts.currentFileProgress += pReply->dataLength;
	ProtoBroadcastAck(ft->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->pfts);

	ProtoBroadcastAck(ft->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	delete ft;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGenToken(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest*)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	auto &results = root.results();
	m_szRToken = results["authToken"].as_mstring();
}

void CIcqProto::OnGetUserHistory(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	__int64 lastMsgId = getId(pReq->hContact, DB_KEY_LASTMSGID);

	auto &results = root.results();
	for (auto &it : results["messages"])
		ParseMessage(pReq->hContact, lastMsgId, it, pReq->pUserInfo != nullptr, false);

	setId(pReq->hContact, DB_KEY_LASTMSGID, lastMsgId);
}

void CIcqProto::OnGetUserInfo(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200) {
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, nullptr);
		return;
	}

	auto &data = root.data();
	for (auto &it : data["users"])
		ParseBuddyInfo(it, pReq->hContact);

	ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, nullptr);
}

void CIcqProto::OnStartSession(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *)
{
	JsonReply root(pReply);
	switch (root.error()) {
	case 200:
		break;

	case 451:
		// session forcibly closed from site
		delSetting(DB_KEY_ATOKEN);
		delSetting(DB_KEY_SESSIONKEY);
		CheckPassword();
		return;

	case 401:
		if (root.detail() == 1002) { // session expired
			delSetting(DB_KEY_ATOKEN);
			delSetting(DB_KEY_SESSIONKEY);
			CheckPassword();
		}
		else ConnectionFailed(LOGINERR_WRONGPASSWORD, root.error());
		return;

	case 400:
		if (root.detail() == 1015 && m_iTimeShift == 0) { // wrong timestamp
			JSONNode &data = root.data();
			int srvTS = data["ts"].as_int();
			m_iTimeShift = (srvTS) ? time(0) - srvTS : 0;
			StartSession();
			return;
		}
		__fallthrough;

	default:
		ConnectionFailed(LOGINERR_WRONGPROTOCOL, root.error());
		return;
	}

	JSONNode &data = root.data();
	m_fetchBaseURL = data["fetchBaseURL"].as_mstring();
	m_aimsid = data["aimsid"].as_mstring();

	int srvTS = data["ts"].as_int();
	m_iTimeShift = (srvTS) ? time(0) - srvTS : 0;

	OnLoggedIn();

	for (auto &it : data["events"])
		ProcessEvent(it);

	ForkThread(&CIcqProto::PollThread);
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

	const char *szContentType = Netlib_GetHeader(pReply, "Content-Type");
	if (szContentType == nullptr)
		szContentType = "image/jpeg";

	ai.format = ProtoGetAvatarFormatByMimeType(szContentType);
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
	else ReportSelfAvatarChanged();
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

	if (g_bMessageState)
		CallService(MS_MESSAGESTATE_UPDATE, ownMsg->m_hContact, MRD_TYPE_DELIVERED);

	const JSONNode &data = root.data();
	CMStringA reqId(root.requestId());
	CMStringA msgId(data["histMsgId"].as_mstring());
	CheckOwnMessage(reqId, msgId, false);
	CheckLastId(ownMsg->m_hContact, data);
}

void CIcqProto::OnSessionEnd(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *)
{
	JsonReply root(pReply);
	if (root.error() == 200) {
		m_szAToken.Empty();
		delSetting(DB_KEY_ATOKEN);
		
		m_szSessionKey.Empty();
		delSetting(DB_KEY_SESSIONKEY);

		ShutdownSession();
	}
}
