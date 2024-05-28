// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-24 Miranda NG team
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

#pragma comment(lib, "libcrypto.lib")

void CIcqProto::CheckAvatarChange(MCONTACT hContact, const JSONNode &ev)
{
	CMStringW wszIconId(ev["bigIconId"].as_mstring());
	if (wszIconId.IsEmpty())
		wszIconId = ev["iconId"].as_mstring();
	if (wszIconId.IsEmpty())
		wszIconId = ev["avatarId"].as_mstring();
	
	if (!wszIconId.IsEmpty()) {
		CMStringW oldIconID(getMStringW(hContact, "IconId"));
		if (wszIconId == oldIconID) {
			wchar_t wszFullName[MAX_PATH];
			GetAvatarFileName(hContact, wszFullName, _countof(wszFullName));
			if (_waccess(wszFullName, 0) == 0)
				return;
		}

		setWString(hContact, "IconId", wszIconId);

		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/expressions/get", &CIcqProto::OnReceiveAvatar);
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

	MCONTACT ret = pOwn->m_hContact;

	if (pOwn->pTransfer) {
		if (bRemove) {
			pOwn->pTransfer->m_szMsgId = msgId;
			ProtoBroadcastAck(pOwn->m_hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, pOwn->pTransfer);
			delete pOwn->pTransfer;
		}
	}
	else if (!Contact::IsGroupChat(ret))
		ProtoBroadcastAck(ret, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pOwn->m_msgid, (LPARAM)msgId.c_str());
	else {
		T2Utf szOwnId(m_szOwnId);

		DB::EventInfo dbei;
		dbei.szId = msgId.c_str();
		dbei.timestamp = time(0);
		dbei.pBlob = pOwn->m_szText;
		dbei.flags = DBEF_SENT | DBEF_READ;
		dbei.szUserId = szOwnId;
		ProtoChainRecvMsg(pOwn->m_hContact, dbei);
	}

	if (auto hDbEvent = db_event_getById(m_szModuleName, msgId))
		db_event_delivered(pOwn->m_hContact, hDbEvent);

	if (bRemove) {
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
	if (!m_szAToken.IsEmpty() && !m_szSessionKey.IsEmpty()) {
		StartSession();
		return;
	}

	if (m_isMra) {
		m_bError462 = false;
		SendMrimLogin(nullptr);
	}
	else {
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "https://api.login.icq.net/auth/clientLogin", &CIcqProto::OnCheckPassword);
		pReq << CHAR_PARAM("clientName", "Miranda NG") << CHAR_PARAM("clientVersion", mirVer) << CHAR_PARAM("devId", appId())
			<< CHAR_PARAM("f", "json") << CHAR_PARAM("tokenType", "longTerm") << WCHAR_PARAM("s", m_szOwnId) << CHAR_PARAM("pwd", m_szPassword);
		#ifndef _DEBUG
			pReq->flags |= NLHRF_NODUMPSEND;
		#endif
		Push(pReq);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnFileInfo(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	IcqFileInfo **res = (IcqFileInfo **)pReq->pUserInfo;
	*res = nullptr;

	RobustReply root(pReply);
	if (root.error() != 200)
		return;

	auto &pData = root.result();
	auto &pInfo = pData["info"];
	std::string szUrl(pInfo["dlink"].as_string());
	if (szUrl.empty())
		return;

	OnMarkRead(pReq->hContact, 0);

	mir_urlDecode(&*szUrl.begin());

	CMStringW wszDescr(pInfo["file_name"].as_mstring());
	*res = new IcqFileInfo(szUrl, wszDescr, pInfo["file_size"].as_int());
}

IcqFileInfo *CIcqProto::RetrieveFileInfo(MCONTACT hContact, const CMStringW &wszUrl)
{
	IcqFileInfo *pFileInfo = nullptr;
	CMStringA szUrl(FORMAT, ICQ_FILE_SERVER "/info/%S/", wszUrl.c_str());
	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, szUrl, &CIcqProto::OnFileInfo);
	pReq->hContact = hContact;
	pReq->pUserInfo = &pFileInfo;
	pReq << CHAR_PARAM("aimsid", m_aimsid) << CHAR_PARAM("previews", "192,600,xlarge");
	if (!ExecuteRequest(pReq))
		return nullptr;

	return pFileInfo;
}

bool CIcqProto::CheckFile(MCONTACT hContact, CMStringW &wszText, IcqFileInfo *&pFileInfo)
{
	bool bRet;
	CMStringW wszUrl;
	int idx = wszText.Find(' ');
	if (idx == -1)
		bRet = fileText2url(wszText, &wszUrl);
	else
		bRet = fileText2url(wszText.Mid(0, idx), &wszUrl);
	if (!bRet)
		return false;

	pFileInfo = nullptr;

	// download file info
	pFileInfo = RetrieveFileInfo(hContact, wszUrl);
	if (!pFileInfo)
		return false;

	if (idx != -1) {
		pFileInfo->szOrigUrl = wszText.Mid(0, idx);
		wszText.Delete(0, idx + 1);
	}
	else {
		pFileInfo->szOrigUrl = wszText;
		wszText.Empty();
	}
	return true;
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
				setWord(it->m_hContact, "Status", m_iStatus2);
				it->m_timer2 = now;
			}
			// if the second timer is not set, simply mark a contact as offline
			else setWord(it->m_hContact, "Status", ID_STATUS_OFFLINE);
			continue;
		}

		// if the second timer is expired, set status to offline
		if (diff2 && it->m_timer2 && now - it->m_timer2 > m_iTimeDiff2) {
			it->m_timer2 = 0;
			setWord(it->m_hContact, "Status", ID_STATUS_OFFLINE);
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
			mir_snwprintf(Popup.lpwzText, TranslateT("You have not entered a login or password.\nConfigure this in Options -> Network -> ICQ and try again."));
			break;
		case LOGINERR_WRONGPASSWORD:
			mir_snwprintf(Popup.lpwzText, TranslateT("Connection failed.\nYour login or password was rejected (%d)."), iErrorCode);
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

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/buddylist/moveBuddy") << AIMSID(this) << WCHAR_PARAM("buddy", GetUserId(hContact)) 
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

	if (m_hFavContact != INVALID_CONTACT_ID)
		setWord(m_hFavContact, "Status", ID_STATUS_ONLINE);

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

MCONTACT CIcqProto::ParseBuddyInfo(const JSONNode &buddy, MCONTACT hContact, bool bIsPartial)
{
	// user chat?
	CMStringW wszId(buddy["aimId"].as_mstring());
	if (IsChat(wszId)) {
		if (auto *pUser = FindUser(wszId))
			if (pUser->m_iApparentMode == ID_STATUS_OFFLINE)
				return INVALID_CONTACT_ID;

		CMStringW wszChatName(buddy["friendly"].as_mstring());
		auto *si = GcCreate(wszId, wszChatName);
		return (si) ? si->hContact : INVALID_CONTACT_ID;
	}

	bool bIgnored = !IsValidType(buddy);
	if (hContact == INVALID_CONTACT_ID) {
		if (bIgnored)
			return INVALID_CONTACT_ID;

		debugLogA("creating a user with id=%S", wszId.c_str());
		hContact = CreateContact(wszId, false);
		if (auto *pUser = FindUser(wszId))
			pUser->m_bInList = true;
	}
	else if (bIgnored) {
		db_delete_contact(hContact, CDF_FROM_SERVER);
		return INVALID_CONTACT_ID;
	}

	CMStringA szVer;
	bool bVersionDetected = false, bSecureIM = false;

	for (auto &it : buddy["capabilities"]) {
		CMStringW wszCap(it.as_mstring());
		if (wszCap.GetLength() != 32)
			continue;

		uint8_t cap[16];
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

	const JSONNode &var = buddy["friendly"];
	if (var && hContact != m_hFavContact)
		setWString(hContact, "Nick", var.as_mstring());

	if (buddy["deleted"].as_bool()) {
		setByte(hContact, "IcqDeleted", 1);
		Contact::PutOnList(hContact);
	}

	Json2string(hContact, buddy, "about", "About", bIsPartial);
	Json2string(hContact, buddy, "emailId", "Email", bIsPartial);
	Json2string(hContact, buddy, "cellNumber", "Cellular", bIsPartial);
	Json2string(hContact, buddy, "workNumber", "CompanyPhone", bIsPartial);

	// we shall not remove existing phone number anyhow
	Json2string(hContact, buddy, "phoneNumber", DB_KEY_PHONE, true);

	Json2int(hContact, buddy, "official", "Official", bIsPartial);
	Json2int(hContact, buddy, "idleTime", "IdleTS", bIsPartial);

	int iStatus = StatusFromPresence(buddy, hContact);
	if (iStatus > 0)
		setWord(hContact, "Status", iStatus);

	const JSONNode &profile = buddy["profile"];
	if (profile) {
		Json2string(hContact, profile, "friendlyName", DB_KEY_ICQNICK, bIsPartial);
		Json2string(hContact, profile, "firstName", "FirstName", bIsPartial);
		Json2string(hContact, profile, "lastName", "LastName", bIsPartial);
		Json2string(hContact, profile, "aboutMe", DB_KEY_ABOUT, bIsPartial);

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

		CMStringW str = profile["gender"].as_mstring();
		if (!str.IsEmpty()) {
			if (str == "male")
				setByte(hContact, "Gender", 'M');
			else if (str == "female")
				setByte(hContact, "Gender", 'F');
		}

		for (auto &it : profile["homeAddress"]) {
			Json2string(hContact, it, "city", "City", bIsPartial);
			Json2string(hContact, it, "state", "State", bIsPartial);
			Json2string(hContact, it, "country", "Country", bIsPartial);
		}
	}
	else {
		Json2string(hContact, buddy, "firstName", "FirstName", bIsPartial);
		Json2string(hContact, buddy, "lastName", "LastName", bIsPartial);
	}

	CMStringW str = buddy["statusMsg"].as_mstring();
	if (str.IsEmpty())
		db_unset(hContact, "CList", "StatusMsg");
	else
		db_set_ws(hContact, "CList", "StatusMsg", str);

	CheckAvatarChange(hContact, buddy);
	return hContact;
}

void CIcqProto::ParseMessage(MCONTACT hContact, __int64 &lastMsgId, const JSONNode &it, int flags)
{
	CMStringA szMsgId(it["msgId"].as_mstring()), szSender, szReply;
	__int64 msgId = _atoi64(szMsgId);
	if (msgId > lastMsgId)
		lastMsgId = msgId;

	MEVENT hOldEvent = db_event_getById(m_szModuleName, szMsgId);
	bool bLocalTime = (flags & PM::LocalTime) != 0;
	int iMsgTime = (bLocalTime) ? time(0) : it["time"].as_int();

	if (auto &node = it["chat"]["sender"])
		szSender = node.as_mstring();

	CMStringW wszText = it["text"].as_mstring();
	wszText.TrimRight();

	if (it["hasAnimatedSticker"].as_bool()) {
		CMStringW wszUrl;
		if (fileText2url(wszText, &wszUrl)) {
			// is it already downloaded sticker?
			CMStringW wszLoadedPath(FORMAT, L"%s\\%S\\Stickers\\STK{%s}.png", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName, wszUrl.c_str());
			if (_waccess(wszLoadedPath, 0)) {
				CMStringA szFullUrl("https://cicq.org/lottie_preview/stickerpicker_large/");
				szFullUrl += wszUrl;
				auto *pNew = new AsyncHttpRequest(CONN_NONE, REQUEST_GET, szFullUrl, &CIcqProto::OnGetSticker);
				pNew->flags |= NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
				pNew->pUserInfo = wszUrl.GetBuffer();
				pNew->AddHeader("Sec-Fetch-User", "?1");
				pNew->AddHeader("Sec-Fetch-Site", "cross-site");
				pNew->AddHeader("Sec-Fetch-Mode", "navigate");
				pNew->AddHeader("Accept-Encoding", "gzip");
				ExecuteRequest(pNew);
			}
			wszText.Format(L"STK{%s}", wszUrl.c_str());
		}
	}
	else {
		if (it["class"].as_mstring() == L"event") {
			CMStringW wszType(it["eventTypeId"].as_mstring());

			// user added you
			if (wszType == L"27:33000") {
				if (bLocalTime) {
					CMStringA id = getMStringA(hContact, DB_KEY_ID);
					int pos = id.Find('@');
					CMStringA nick = (pos == -1) ? id : id.Left(pos);
					DB::AUTH_BLOB blob(hContact, nick, nullptr, nullptr, id, nullptr);

					DB::EventInfo dbei;
					dbei.timestamp = (uint32_t)time(0);
					dbei.cbBlob = blob.size();
					dbei.pBlob = blob;
					ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&dbei);
				}
				return;
			}

			if (auto &pChat = it["chat"]) {
				auto *si = Chat_Find(pChat["sender"].as_mstring(), m_szModuleName);
				if (si == nullptr)
					return;

				if (pChat["type"].as_string() == "group") {
					if (pChat["memberEvent"]["type"].as_string() == "kicked") {
						GCEVENT gce = {};
						gce.si = si;
						gce.time = iMsgTime;
						gce.iType = GC_EVENT_KICK;

						for (auto &jt : pChat["memberEvent"]["members"]) {
							auto userId = jt.as_mstring();
							gce.pszUID.w = userId;
							Chat_Event(&gce);
						}
					}
				}
				return;
			}

			// message was deleted
			if (wszType == L"27:51000")
				wszText = TranslateT("Message was deleted");
		}
	}

	bool bIsOutgoing = it["outgoing"].as_bool();
	bool bIsChat = Contact::IsGroupChat(hContact);

	// check for embedded file
	IcqFileInfo *pFileInfo = nullptr;
	const JSONNode *pForward = nullptr, *pQuote = nullptr;

	for (auto &jt : it["parts"]) {
		auto szType = jt["mediaType"].as_string();
		if (szType == "forward")
			pForward = &jt;
		else if (szType == "quote")
			pQuote = &jt;
		else if (szType == "text")
			ParseMessagePart(hContact, jt, pFileInfo);
	}

	if (pForward) {
		int idx = wszText.Find(L"\n\n");
		if (idx != -1)
			wszText.Truncate(idx + 2);

		if (!pFileInfo) {
			ParseMessagePart(hContact, *pForward, pFileInfo);
			if (pFileInfo)
				pFileInfo->wszDescr = wszText;
		}
	}

	if (pQuote) {
		szReply = pQuote->at("msgId").as_mstring();

		if (!pFileInfo) {
			ParseMessagePart(hContact, *pQuote, pFileInfo);
			if (pFileInfo)
				pFileInfo->wszDescr = wszText;
		}
	}

	// message text might be a separate file link as well
	if (pFileInfo == nullptr && fileText2url(wszText)) {
		if (hOldEvent)
			return;
		
		CheckFile(hContact, wszText, pFileInfo);
		if (pFileInfo)
			pFileInfo->wszDescr = wszText;
	}

	// process our own messages
	bool bCreateRead = (flags & PM::CreateRead) != 0;
	CMStringA reqId(it["reqId"].as_mstring());
	if (CheckOwnMessage(reqId, szMsgId, true)) {
		debugLogA("Skipping our own message %s", szMsgId.c_str());
		if (!bIsChat) // prevent duplicates in private chats 
			return;
		bIsOutgoing = bCreateRead = true;
	}
	else if (bIsChat)
		bCreateRead = true;

	// convert a file info into Miranda's file transfer
	if (pFileInfo) {
		auto *p = strrchr(pFileInfo->szUrl, '/');
		auto *pszShortName = (p == nullptr) ? pFileInfo->szUrl.c_str() : p + 1;

		DB::EventInfo dbei(hOldEvent);
		dbei.eventType = EVENTTYPE_FILE;
		dbei.flags = DBEF_TEMPORARY;
		dbei.szId = szMsgId;
		dbei.timestamp = iMsgTime;
		if (bCreateRead)
			dbei.flags |= DBEF_READ;
		if (bIsOutgoing)
			dbei.flags |= DBEF_SENT;
		if (!szReply.IsEmpty())
			dbei.szReplyId = szReply;
		if (isChatRoom(hContact))
			dbei.szUserId = szSender;

		DB::FILE_BLOB blob(pFileInfo, pszShortName, T2Utf(pFileInfo->wszDescr));
		if (hOldEvent) {
			OnReceiveOfflineFile(blob);
			blob.write(dbei);
			db_event_edit(hOldEvent, &dbei, true);
		}
		else {
			CMStringA szUrl(pFileInfo->szUrl);
			MEVENT hEvent = ProtoChainRecvFile(hContact, blob, dbei);

			if (flags & PM::FetchFiles) {
				if (!blob.isCompleted()) {
					wchar_t wszReceiveFolder[MAX_PATH];
					File::GetReceivedFolder(hContact, wszReceiveFolder, _countof(wszReceiveFolder), true);
					CMStringW wszFileName(FORMAT, L"%s%s", wszReceiveFolder, blob.getName());

					MHttpRequest nlhr(REQUEST_GET);
					nlhr.flags = NLHRF_REDIRECT;
					nlhr.m_szUrl = szUrl;
					nlhr.AddHeader("Sec-Fetch-User", "?1");
					nlhr.AddHeader("Sec-Fetch-Site", "cross-site");
					nlhr.AddHeader("Sec-Fetch-Mode", "navigate");
					nlhr.AddHeader("Accept-Encoding", "gzip");

					debugLogW(L"Saving to [%s]", wszFileName.c_str());
					NLHR_PTR reply(Netlib_DownloadFile(m_hNetlibUser, &nlhr, wszFileName.c_str(), 0, 0));
					if (reply && reply->resultCode == 200) {
						struct _stat st;
						_wstat(wszFileName, &st);

						DBVARIANT dbv = { DBVT_DWORD };
						dbv.dVal = st.st_size;
						db_event_setJson(hEvent, "ft", &dbv);
					}
				}
			}
		}
		
		return;
	}

	// suppress notifications for already loaded/processed messages
	__int64 storedLastId = getId(hContact, DB_KEY_LASTMSGID);
	if (msgId <= storedLastId) {
		debugLogA("Parsing old/processed message with id %lld < %lld, setting CR to true", msgId, storedLastId);
		bCreateRead = true;
	}

	debugLogA("Adding message %d:%lld (CR=%d)", hContact, msgId, bCreateRead);

	ptrA szUtf(mir_utf8encodeW(wszText));

	DB::EventInfo dbei(hOldEvent);
	dbei.szModule = m_szModuleName;
	dbei.timestamp = iMsgTime;
	dbei.flags = DBEF_UTF;
	if (bIsOutgoing)
		dbei.flags |= DBEF_SENT;
	if (bCreateRead)
		dbei.flags |= DBEF_READ;
	dbei.cbBlob = (int)mir_strlen(szUtf);
	dbei.szId = szMsgId;
	if (isChatRoom(hContact))
		dbei.szUserId = szSender;
	if (!szReply.IsEmpty())
		dbei.szReplyId = szReply;

	if (dbei) {
		replaceStr(dbei.pBlob, szUtf.detach());
		db_event_edit(hOldEvent, &dbei, true);
	}
	else {
		dbei.pBlob = szUtf;
		ProtoChainRecvMsg(hContact, dbei);
	}
}

void CIcqProto::ParseMessagePart(MCONTACT hContact, const JSONNode &part, IcqFileInfo *&pFileInfo)
{
	if (pFileInfo != nullptr)
		return;

	if (auto &content = part["captionedContent"]) {
		CMStringW wszUrl(content["url"].as_mstring());
		if (wszUrl.IsEmpty())
			return;

		if (!CheckFile(hContact, wszUrl, pFileInfo))
			return;

		CMStringW wszDescr(content["caption"].as_mstring());
		if (!wszDescr.IsEmpty())
			pFileInfo->wszDescr = wszDescr;
	}
	else {
		auto wszText = part["text"].as_mstring();
		CheckFile(hContact, wszText, pFileInfo);
	}
}

bool CIcqProto::RefreshRobustToken(AsyncHttpRequest *pOrigReq)
{
	if (!m_szRToken.IsEmpty())
		return true;

	auto *pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER "/genToken", &CIcqProto::OnGenToken);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif

	int ts = TS();
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", appId()) << CHAR_PARAM("nonce", CMStringA(FORMAT, "%d-%d", ts, rand() % 10)) << INT_PARAM("ts", ts);
	CalcHash(pReq);

	CMStringA szAgent(FORMAT, "%S Mail.ru Windows ICQ (version 10.0.1999)", (wchar_t*)m_szOwnId);
	pReq->AddHeader("User-Agent", szAgent);
	if (!ExecuteRequest(pReq)) {
LBL_Error:
		(this->*(pOrigReq->m_pFunc))(nullptr, pOrigReq);
		return false;
	}
	if (m_szRToken.IsEmpty())
		goto LBL_Error;

	// now add this token
	bool bRet = false;
	pReq = new AsyncHttpRequest(CONN_RAPI, REQUEST_POST, ICQ_ROBUST_SERVER "/addClient", &CIcqProto::OnAddClient);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("f", "json") << CHAR_PARAM("k", appId()) << INT_PARAM("ts", ts)
		<< CHAR_PARAM("client", "icq") << CHAR_PARAM("reqId", pReq->m_reqId) << CHAR_PARAM("authToken", m_szRToken);
	pReq->pUserInfo = &bRet;
	if (!ExecuteRequest(pReq))
		goto LBL_Error;

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGetUserCaps(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200)
		return;

	auto &data = root.data();
	for (auto &it : data["users"]) {
		ParseBuddyInfo(it, pReq->hContact, true);

		if (auto *pUser = (IcqUser *)pReq->pUserInfo)
			pUser->m_bGotCaps = true;
	}
}

void CIcqProto::RetrieveUserCaps(IcqUser *pUser)
{
	auto *pReq = new AsyncHttpRequest(CONN_OLD, REQUEST_GET, "/presence/get", &CIcqProto::OnGetUserCaps);
	pReq->hContact = pUser->m_hContact;
	pReq->pUserInfo = pUser;
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("f", "json") << CHAR_PARAM("k", appId()) << CHAR_PARAM("r", pReq->m_reqId)
		<< WCHAR_PARAM("t", GetUserId(pUser->m_hContact)) << INT_PARAM("mdir", 0) << INT_PARAM("capabilities", 1);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGePresence(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200)
		return;

	auto &data = root.data();
	for (auto &it : data["users"]) {
		ParseBuddyInfo(it, pReq->hContact, true);
		ProcessOnline(it, pReq->hContact);
	}
}

void CIcqProto::RetrievePresence(MCONTACT hContact)
{
	CMStringW wszId(GetUserId(hContact));

	auto *pReq = new AsyncHttpRequest(CONN_OLD, REQUEST_GET, "/presence/get", &CIcqProto::OnGePresence);
	pReq->hContact = hContact;
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("f", "json") << CHAR_PARAM("k", appId()) << CHAR_PARAM("r", pReq->m_reqId)
		<< WCHAR_PARAM("t", wszId) << INT_PARAM("mdir", 1);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGetUserInfo(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000) {
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, nullptr);
		return;
	}

	ParseBuddyInfo(root.results(), pReq->hContact, true);

	ProtoBroadcastAck(pReq->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, nullptr);
}

void CIcqProto::RetrieveUserInfo(MCONTACT hContact)
{
	CMStringW wszId(GetUserId(hContact));

	auto *pReq = new AsyncRapiRequest(this, "getUserInfo", &CIcqProto::OnGetUserInfo);
	pReq->params << WCHAR_PARAM("sn", wszId);
	pReq->hContact = hContact;
	Push(pReq);

	if (hContact)
		if (auto *pUser = FindUser(wszId))
			if (!pUser->m_bGotCaps)
				RetrieveUserCaps(pUser);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGetPatches(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	auto &results = root.results();

	CMStringW wszNewPatch(results["patchVersion"].as_mstring());
	if (!wszNewPatch.IsEmpty())
		setId(pReq->hContact, DB_KEY_PATCHVER, _wtoi64(wszNewPatch));

	std::map<__int64, bool> events;
	for (auto &it : results["patch"]) {
		std::string type = it["type"].as_string();
		__int64 msgId = _wtoi64(it["msgId"].as_mstring());
		if (type == "update" || type == "modify")
			events[msgId] = true;
		else
			events[msgId] = false;
	}

	for (auto &it : events) {
		if (it.second) {
			bool bFound = false;

			for (auto &msg: results["messages"])
				if (_wtoi64(msg["msgId"].as_mstring()) == it.first) {
					bFound = true;
					__int64 lastMsgId;
					ParseMessage(pReq->hContact, lastMsgId, msg, PM::LocalTime);
				}

			if (!bFound)
				RetrieveHistoryChunk(pReq->hContact, it.first, it.first - 1, 1);
		}
		else {
			char msgId[100];
			_i64toa(it.first, msgId, 10);
			if (MEVENT hEvent = db_event_getById(m_szModuleName, msgId))
				db_event_delete(hEvent, CDF_FROM_SERVER);
		}
	}
}

void CIcqProto::ProcessPatchVersion(MCONTACT hContact, __int64 currPatch)
{
	__int64 oldPatch(getId(hContact, DB_KEY_PATCHVER));
	if (!oldPatch)
		oldPatch = 1;

	if (currPatch == oldPatch)
		return;

	auto *pReq = new AsyncRapiRequest(this, "getHistory", &CIcqProto::OnGetPatches);
	#ifndef _DEBUG
	pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq->hContact = hContact;
	pReq->params << WCHAR_PARAM("sn", GetUserId(hContact)) << INT_PARAM("fromMsgId", 0) << INT_PARAM("count", 0) << SINT64_PARAM("patchVersion", oldPatch);
	Push(pReq);
}

void CIcqProto::RetrievePatches(MCONTACT hContact)
{
	__int64 oldPatch(getId(hContact, DB_KEY_PATCHVER));
	if (!oldPatch)
		oldPatch = 1;

	auto *pReq = new AsyncRapiRequest(this, "getHistory", &CIcqProto::OnGetPatches);
	#ifndef _DEBUG
	pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq->hContact = hContact;
	pReq->params << WCHAR_PARAM("sn", GetUserId(hContact)) << INT_PARAM("fromMsgId", -1) << INT_PARAM("count", -1) << SINT64_PARAM("patchVersion", oldPatch);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGetUserHistory(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	__int64 lastMsgId = getId(pReq->hContact, DB_KEY_LASTMSGID);

	wchar_t wszReceiveFolder[MAX_PATH];
	File::GetReceivedFolder(pReq->hContact, wszReceiveFolder, _countof(wszReceiveFolder), true);
	CreateDirectoryTreeW(wszReceiveFolder);

	int count = 0, flags = PM::FetchFiles + (pReq->pUserInfo ? PM::CreateRead : 0);
	auto &results = root.results();
	for (auto &it : results["messages"]) {
		ParseMessage(pReq->hContact, lastMsgId, it, flags);
		count++;
	}

	setId(pReq->hContact, DB_KEY_LASTMSGID, lastMsgId);

	if (count >= 999)
		RetrieveUserHistory(pReq->hContact, lastMsgId, flags);
}

void CIcqProto::RetrieveUserHistory(MCONTACT hContact, __int64 startMsgId, bool bCreateRead)
{
	if (startMsgId == 0)
		startMsgId = -1;

	__int64 patchVer = getId(hContact, DB_KEY_PATCHVER);
	if (patchVer == 0)
		patchVer = 1;

	auto *pReq = new AsyncRapiRequest(this, "getHistory", &CIcqProto::OnGetUserHistory);
	#ifndef _DEBUG
		pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq->hContact = hContact;
	pReq->pUserInfo = (bCreateRead) ? pReq : 0;
	pReq->params << WCHAR_PARAM("sn", GetUserId(hContact)) << INT64_PARAM("fromMsgId", startMsgId) << INT_PARAM("count", 1000) << SINT64_PARAM("patchVersion", patchVer);
	Push(pReq);
}

void CIcqProto::RetrieveHistoryChunk(MCONTACT hContact, __int64 patchVer, __int64 startMsgId, unsigned iCount)
{
	auto *pReq = new AsyncRapiRequest(this, "getHistory", &CIcqProto::OnGetUserHistory);
	#ifndef _DEBUG
	pReq->flags |= NLHRF_NODUMPSEND;
	#endif
	pReq->hContact = hContact;
	pReq->params << WCHAR_PARAM("sn", GetUserId(hContact)) << INT64_PARAM("fromMsgId", startMsgId) << INT_PARAM("count", iCount) << SINT64_PARAM("patchVersion", patchVer);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::SetOwnId(const CMStringW &wszId)
{
	if (wszId.IsEmpty())
		return;

	m_szOwnId = wszId;

	auto *pUser = FindUser(wszId);
	if (!pUser) {
		CreateContact(wszId, false);
		pUser = FindUser(wszId);
	}
	
	setWString(pUser->m_hContact, "Nick", TranslateT("Favorites"));
	ptrW wszMyHandle(db_get_wsa(pUser->m_hContact, "CList", "MyHandle"));
	if (!wszMyHandle)
		db_set_ws(pUser->m_hContact, "CList", "MyHandle", TranslateT("Favorites"));

	if (m_hFavContact == INVALID_CONTACT_ID) {
		m_hFavContact = pUser->m_hContact;
		pUser->m_hContact = 0;
	}
}

void CIcqProto::SetServerStatus(int iStatus)
{
	const char *szStatus = "online";
	m_bInvisible = false;

	switch (iStatus) {
	case ID_STATUS_OFFLINE: szStatus = "offline"; break;
	case ID_STATUS_NA: szStatus = "occupied"; break;
	case ID_STATUS_AWAY:
	case ID_STATUS_DND: szStatus = "away"; break;
	case ID_STATUS_INVISIBLE:
		m_bInvisible = true;
	}

	Push(new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "/presence/setState")
		<< AIMSID(this) << CHAR_PARAM("view", szStatus) << INT_PARAM("invisible", m_bInvisible));

	if (iStatus == ID_STATUS_OFFLINE && !getByte(DB_KEY_PHONEREG)) {
		auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_GET, "/aim/endSession", &CIcqProto::OnSessionEnd);
		pReq << AIMSID(this) << INT_PARAM("invalidateToken", 1);
		ExecuteRequest(pReq);
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

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "/aim/startSession", &CIcqProto::OnStartSession);
	pReq << CHAR_PARAM("a", m_szAToken) << INT_PARAM("activeTimeout", 180) << CHAR_PARAM("assertCaps", caps)
		<< INT_PARAM("buildNumber", __BUILD_NUM) << CHAR_PARAM("deviceId", szDeviceId) << CHAR_PARAM("events", EVENTS)
		<< CHAR_PARAM("f", "json") << CHAR_PARAM("imf", "plain") << CHAR_PARAM("inactiveView", "offline")
		<< CHAR_PARAM("includePresenceFields", FIELDS) << CHAR_PARAM("invisible", "false")
		<< CHAR_PARAM("k", appId()) << INT_PARAM("mobile", 0) << CHAR_PARAM("nonce", nonce) << CHAR_PARAM("r", pReq->m_reqId)
		<< INT_PARAM("rawMsg", 0) << INT_PARAM("sessionTimeout", 7776000) << INT_PARAM("ts", ts) << CHAR_PARAM("view", "online");

	CalcHash(pReq);
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnAddBuddy(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200)
		return;

	CMStringW wszId = getMStringW(pReq->hContact, DB_KEY_ID);
	for (auto &it : root.data()["results"]) {
		if (it["buddy"].as_mstring() != wszId)
			continue;

		switch (int iResultCode = it["resultCode"].as_int()) {
		case 0: // success
		case 3: // already in contact list
			break;

		default:
			debugLogA("Contact %d failed to add: error %d", pReq->hContact, iResultCode);

			POPUPDATAW Popup = {};
			Popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR));
			wcsncpy_s(Popup.lpwzText, TranslateT("Buddy addition failed"), _TRUNCATE);
			wcsncpy_s(Popup.lpwzContactName, Clist_GetContactDisplayName(pReq->hContact), _TRUNCATE);
			Popup.iSeconds = 20;
			PUAddPopupW(&Popup);

			// Contact::RemoveFromList(pReq->hContact);
		}

		RetrieveUserInfo(pReq->hContact);
		Contact::PutOnList(pReq->hContact);
	}
}

void CIcqProto::OnAddClient(MHttpResponse *pReply, AsyncHttpRequest *pReq)
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

void CIcqProto::OnCheckPassword(MHttpResponse *pReply, AsyncHttpRequest*)
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
	uint8_t hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), szPassTemp, szPassTemp.GetLength(), (uint8_t*)szSessionSecret.c_str(), szSessionSecret.GetLength(), hashOut, &len);
	m_szSessionKey = ptrA(mir_base64_encode(hashOut, sizeof(hashOut)));
	setString(DB_KEY_SESSIONKEY, m_szSessionKey);

	SetOwnId(data["loginId"].as_mstring());

	int srvTS = data["hostTime"].as_int();
	m_iTimeShift = (srvTS) ? time(0) - srvTS : 0;

	StartSession();
}

void CIcqProto::OnFileContinue(MHttpResponse *pReply, AsyncHttpRequest *pOld)
{
	IcqFileTransfer *pTransfer = (IcqFileTransfer*)pOld->pUserInfo;
	if (pTransfer->m_bCanceled) {
LBL_Error:
		ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, pTransfer);
		delete pTransfer;
		return;
	}

	switch (pReply->resultCode) {
	case 200: // final ok
	case 206: // partial ok
		break;

	default:
		goto LBL_Error;
	}

	// file transfer succeeded?
	if (pTransfer->pfts.currentFileProgress == pTransfer->pfts.currentFileSize) {
		FileReply root(pReply);
		if (root.error() != 200)
			goto LBL_Error;

		const JSONNode &data = root.data();
		CMStringW wszUrl(data["static_url"].as_mstring());

		JSONNode bundle, contents; contents.set_name("captionedContent");
		contents << WCHAR_PARAM("caption", pTransfer->m_wszDescr) << WCHAR_PARAM("url", wszUrl);
		bundle << CHAR_PARAM("mediaType", "text") << WCHAR_PARAM("text", wszUrl) << contents;
		CMStringW wszParts(FORMAT, L"[%s]", ptrW(json_write(&bundle)).get());

		pTransfer->m_szHost = wszUrl;
		if (!pTransfer->m_wszDescr.IsEmpty())
			wszUrl += L" " + pTransfer->m_wszDescr;

		int id = InterlockedIncrement(&m_msgId);
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "/im/sendIM", &CIcqProto::OnSendMessage);

		auto *pOwn = new IcqOwnMessage(pTransfer->pfts.hContact, id, T2Utf(wszUrl));
		pOwn->setGuid(pReq->m_reqId);
		pOwn->pTransfer = pTransfer;
		pReq->pUserInfo = pOwn;
		{
			mir_cslock lck(m_csOwnIds);
			m_arOwnIds.insert(pOwn);
		}

		pReq << AIMSID(this) << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", appId()) << CHAR_PARAM("mentions", "") << WCHAR_PARAM("message", wszUrl)
			<< CHAR_PARAM("offlineIM", "true") << WCHAR_PARAM("parts", wszParts) << WCHAR_PARAM("t", GetUserId(pTransfer->pfts.hContact)) << INT_PARAM("ts", TS());
		Push(pReq);
		return;
	}

	// else send the next portion
	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, pTransfer->m_szHost, &CIcqProto::OnFileContinue);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("k", appId()) << INT_PARAM("ts", TS());
	CalcHash(pReq);
	pReq->m_szUrl.AppendChar('?');
	pReq->m_szUrl += pReq->m_szParam; pReq->m_szParam.Empty();
	pReq->pUserInfo = pTransfer;
	pTransfer->FillHeaders(pReq);
	Push(pReq);

	pTransfer->pfts.currentFileTime = time(0);
	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, pTransfer, (LPARAM)&pTransfer->pfts);
}

void CIcqProto::OnFileInit(MHttpResponse *pReply, AsyncHttpRequest *pOld)
{
	IcqFileTransfer *pTransfer = (IcqFileTransfer*)pOld->pUserInfo;
	if (pTransfer->m_bCanceled) {
LBL_Error:
		ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, pTransfer);
		delete pTransfer;
		return;
	}

	FileReply root(pReply);
	if (root.error() != 200)
		goto LBL_Error;

	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, pTransfer);
	pTransfer->pfts.currentFileTime = time(0);

	const JSONNode &data = root.data();
	CMStringW wszHost(data["host"].as_mstring());
	CMStringW wszUrl(data["url"].as_mstring());
	pTransfer->m_szHost = L"https://" + wszHost + wszUrl;

	auto *pReq = new AsyncHttpRequest(CONN_NONE, REQUEST_POST, pTransfer->m_szHost, &CIcqProto::OnFileContinue);
	pReq << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("client", "icq") << CHAR_PARAM("k", appId()) << INT_PARAM("ts", TS());
	CalcHash(pReq);
	pReq->m_szUrl.AppendChar('?');
	pReq->m_szUrl += pReq->m_szParam; pReq->m_szParam.Empty();
	pReq->pUserInfo = pTransfer;
	pTransfer->FillHeaders(pReq);
	Push(pReq);

	ProtoBroadcastAck(pTransfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, pTransfer, (LPARAM)&pTransfer->pfts);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Support for stickers

void CIcqProto::OnGetSticker(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	if (pReply->resultCode != 200) {
		debugLogA("Error getting sticker: %d", pReply->resultCode);
		return;
	}

	CMStringW wszPath(FORMAT, L"%s\\%S\\Stickers", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
	CreateDirectoryTreeW(wszPath);

	CMStringW wszFileName(FORMAT, L"%s\\STK{%s}.png", wszPath.c_str(), (wchar_t*)pReq->pUserInfo);
	FILE *out = _wfopen(wszFileName, L"wb");
	fwrite(pReply->body, 1, pReply->body.GetLength(), out);
	fclose(out);

	SmileyAdd_LoadContactSmileys(SMADD_FILE, m_szModuleName, wszFileName);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnGenToken(MHttpResponse *pReply, AsyncHttpRequest*)
{
	RobustReply root(pReply);
	if (root.error() != 20000)
		return;

	auto &results = root.results();
	m_szRToken = results["authToken"].as_mstring();
}

void CIcqProto::OnStartSession(MHttpResponse *pReply, AsyncHttpRequest *)
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
		delSetting(DB_KEY_ATOKEN);
		delSetting(DB_KEY_SESSIONKEY);
		if (root.detail() == 1002) // session expired
			CheckPassword();
		else
			ConnectionFailed(LOGINERR_WRONGPASSWORD, root.error());
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

	auto &myInfo = data["myInfo"];
	ProcessMyInfo(myInfo);
	SetOwnId(myInfo["aimId"].as_mstring());

	int srvTS = data["ts"].as_int();
	m_iTimeShift = (srvTS) ? time(0) - srvTS : 0;

	for (auto &it : data["events"])
		ProcessEvent(it);

	OnLoggedIn();

	ForkThread(&CIcqProto::PollThread);
}

void CIcqProto::OnReceiveAvatar(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = pReq->hContact;

	if (pReply->resultCode != 200 || pReply->body.IsEmpty()) {
LBL_Error:
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);
		return;
	}

	const char *szContentType = pReply->FindHeader("Content-Type");
	if (szContentType == nullptr)
		szContentType = "image/jpeg";

	ai.format = ProtoGetAvatarFormatByMimeType(szContentType);
	setByte(pReq->hContact, "AvatarType", ai.format);
	GetAvatarFileName(pReq->hContact, ai.filename, _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == nullptr)
		goto LBL_Error;

	fwrite(pReply->body, pReply->body.GetLength(), 1, out);
	fclose(out);

	if (pReq->hContact != 0) {
		ProtoBroadcastAck(pReq->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
		debugLogW(L"Broadcast new avatar: %s", ai.filename);
	}
	else ReportSelfAvatarChanged();
}

void CIcqProto::OnSearchResults(MHttpResponse *pReply, AsyncHttpRequest *pReq)
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
	for (auto &it : results["persons"]) {
		CMStringW wszId = it["sn"].as_mstring();
		if (wszId == m_szOwnId)
			continue;

		CMStringW wszNick = it["friendly"].as_mstring();
		CMStringW wszFirst = it["firstName"].as_mstring();
		CMStringW wszLast = it["lastName"].as_mstring();

		psr.id.w = wszId.GetBuffer();
		psr.nick.w = wszNick.GetBuffer();
		psr.firstName.w = wszFirst.GetBuffer();
		psr.lastName.w = wszLast.GetBuffer();
		ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)pReq, LPARAM(&psr));
	}

	ProtoBroadcastAck(0, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::PatchProfileInfo(const char *pszVariable, const wchar_t *pwszValue)
{
	if (!mir_wstrlen(pwszValue))
		return;

	auto *pReq = new AsyncRapiRequest(this, "/profile/update", nullptr, 1);
	pReq->params << WCHAR_PARAM(pszVariable, pwszValue);
	Push(pReq);

	char *buf = NEWSTR_ALLOCA(pszVariable);
	buf[0] = _toupper(buf[0]);
	setWString(buf, pwszValue);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Send message

void CIcqProto::OnSendMessage(MHttpResponse *pReply, AsyncHttpRequest *pReq)
{
	IcqOwnMessage *ownMsg = (IcqOwnMessage *)pReq->pUserInfo;

	JsonReply root(pReply);
	if (root.error() != 200) {
		if (ownMsg) {
			ProtoBroadcastAck(ownMsg->m_hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)ownMsg->m_msgid);

			mir_cslock lck(m_csOwnIds);
			m_arOwnIds.remove(ownMsg);
		}
		return;
	}

	const JSONNode &data = root.data();
	if (auto &jsonMsg = data.at("histMsgId")) {
		CMStringA reqId(root.requestId());
		CMStringA msgId(jsonMsg.as_mstring());
		CheckOwnMessage(reqId, msgId, false);
	}

	if (ownMsg) {
		if (g_bMessageState)
			CallService(MS_MESSAGESTATE_UPDATE, ownMsg->m_hContact, MRD_TYPE_DELIVERED);
		CheckLastId(ownMsg->m_hContact, data);
	}
}

void CIcqProto::SendMessageParts(MCONTACT hContact, const JSONNode &parts, IcqOwnMessage *pOwn)
{
	CMStringA szUserid(GetUserId(hContact));
	if (szUserid.IsEmpty())
		return;

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "/im/sendIM", &CIcqProto::OnSendMessage);
	pReq->pUserInfo = pOwn;
	if (pOwn)
		pOwn->setGuid(pReq->m_reqId);

	pReq << AIMSID(this) << CHAR_PARAM("a", m_szAToken) << CHAR_PARAM("k", appId()) << CHAR_PARAM("mentions", "")
		<< CHAR_PARAM("offlineIM", "true") << CHAR_PARAM("parts", parts.write().c_str()) << CHAR_PARAM("t", szUserid) << INT_PARAM("ts", TS());
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::OnSessionEnd(MHttpResponse *pReply, AsyncHttpRequest *)
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
