/*
Copyright (c) 2013-15 Miranda NG project (http://miranda-ng.org)

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

UINT_PTR CVkProto::m_timer;
char szBlankUrl[] = "https://oauth.vk.com/blank.html";
static char VK_TOKEN_BEG[] = "access_token=";
static char VK_LOGIN_DOMAIN[] = "https://m.vk.com";
static char fieldsName[] = "id, first_name, last_name, photo_100, bdate, sex, timezone, contacts, online, status, about, domain";

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ShutdownSession()
{
	debugLogA("CVkProto::ShutdownSession");
	if (m_hWorkerThread) {
		m_bTerminated = true;
		SetEvent(m_evRequestsQueue);
	}

	OnLoggedOut();
}

void CVkProto::ConnectionFailed(int iReason)
{
	delSetting("AccessToken");

	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, iReason);
	debugLogA("CVkProto::ConnectionFailed ShutdownSession");
	ShutdownSession();
}

/////////////////////////////////////////////////////////////////////////////////////////

static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	for (int i = 0; i < vk_Instances.getCount(); i++)
		if (vk_Instances[i]->IsOnline()) {
			vk_Instances[i]->SetServerStatus(vk_Instances[i]->m_iDesiredStatus);
			vk_Instances[i]->RetrieveUsersInfo(true);
			vk_Instances[i]->RetrieveUnreadEvents();			
		}
}

static void CALLBACK VKSetTimer(void*)
{
	CVkProto::m_timer = SetTimer(NULL, 0, 60000, TimerProc);
}

static void CALLBACK VKUnsetTimer(void*)
{
	if (CVkProto::m_timer)
		KillTimer(NULL, CVkProto::m_timer);
	CVkProto::m_timer = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::OnLoggedIn()
{
	debugLogA("CVkProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);

	// initialize online timer
	if (!m_timer)
		CallFunctionAsync(VKSetTimer, this);
}

void CVkProto::OnLoggedOut()
{
	debugLogA("CVkProto::OnLoggedOut");
	m_bOnline = false;

	if (m_pollingConn)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_pollingConn, 0);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	bool bOnline = false;
	for (int i = 0; i < vk_Instances.getCount(); i++)
		bOnline = bOnline || vk_Instances[i]->IsOnline();
	if (!bOnline)
		CallFunctionAsync(VKUnsetTimer, this);
	SetAllContactStatuses(ID_STATUS_OFFLINE);
	m_chats.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::OnOAuthAuthorize(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnOAuthAuthorize %d", reply->resultCode);
	GrabCookies(reply);

	if (reply->resultCode == 302) { // manual redirect
		LPCSTR pszLocation = findHeader(reply, "Location");
		if (pszLocation) {
			if (!_strnicmp(pszLocation, szBlankUrl, sizeof(szBlankUrl)-1)) {
				m_szAccessToken = NULL;
				LPCSTR p = strstr(pszLocation, VK_TOKEN_BEG);
				if (p) {
					p += sizeof(VK_TOKEN_BEG)-1;
					for (LPCSTR q = p+1; *q; q++) {
						if (*q == '&' || *q == '=' || *q == '\"') {
							m_szAccessToken = mir_strndup(p, q-p);
							break;
						}
					}
					if (m_szAccessToken == NULL)
						m_szAccessToken = mir_strdup(p);
					setString("AccessToken", m_szAccessToken);
					RetrieveMyInfo();
				}
				else {
					delSetting("AccessToken");
					ConnectionFailed(LOGINERR_NOSERVER);
				}
			}
			else {
				AsyncHttpRequest *pReq = new AsyncHttpRequest();
				pReq->requestType = REQUEST_GET;
				pReq->flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
				pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
				pReq->AddHeader("Referer", m_prevUrl);
				pReq->Redirect(reply);
				if (!pReq->m_szUrl.IsEmpty()) {
					if (pReq->m_szUrl[0] == '/')
						pReq->m_szUrl = VK_LOGIN_DOMAIN + pReq->m_szUrl;
					ApplyCookies(pReq);
					m_prevUrl = pReq->m_szUrl;
				}
				pReq->m_bApiReq = false;
				pReq->bIsMainConn = true;
				Push(pReq);
			}
		}
		else 
			ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	if (reply->resultCode != 200 || !strstr(reply->pData, "form method=\"post\"")) { // something went wrong
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	if (strstr(reply->pData, "service_msg_warning")) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}
		
	CMStringA szAction, szBody;
	bool bSuccess = AutoFillForm(reply->pData, szAction, szBody);
	if (!bSuccess || szAction.IsEmpty() || szBody.IsEmpty()) {
		if (m_prevError) {
			ConnectionFailed(LOGINERR_NOSERVER);
			return;
		}
		m_prevError = true;
	}

	pReq = new AsyncHttpRequest();
	pReq->requestType = REQUEST_POST;
	pReq->flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	pReq->m_szParam = szBody;
	pReq->m_szUrl = szAction;
	if (!pReq->m_szUrl.IsEmpty()) 
		if (pReq->m_szUrl[0] == '/')
			pReq->m_szUrl = VK_LOGIN_DOMAIN + pReq->m_szUrl;
	m_prevUrl = pReq->m_szUrl;
	pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->Redirect(reply);
	ApplyCookies(pReq);
	pReq->m_bApiReq = false;
	pReq->bIsMainConn = true;
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::RetrieveMyInfo()
{
	debugLogA("CVkProto::RetrieveMyInfo");
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnReceiveMyInfo,AsyncHttpRequest::rpHigh)
		<< VER_API);
}

void CVkProto::OnReceiveMyInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveMyInfo %d", reply->resultCode);
	if (reply->resultCode != 200) {
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		return;
	}

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;
	
	const JSONNode &jnUser = *(jnResponse.begin());


	m_myUserId = jnUser["id"].as_int();
	setDword("ID", m_myUserId);
		
	OnLoggedIn();
	RetrieveUserInfo(m_myUserId);
	RetrieveUnreadMessages();
	RetrieveFriends();
	RetrievePollingInfo();	
}

MCONTACT CVkProto::SetContactInfo(const JSONNode &jnItem, bool flag, bool self)
{
	if (!jnItem) {
		debugLogA("CVkProto::SetContactInfo pItem == NULL");
		return INVALID_CONTACT_ID;
	}

	LONG userid = jnItem["id"].as_int();
	debugLogA("CVkProto::SetContactInfo %d", userid);
	if (userid == 0 || userid == VK_FEED_USER)
		return NULL;

	MCONTACT hContact = FindUser(userid, flag);
	
	if (userid == m_myUserId) {
		if (hContact != NULL)
			if (self)
				hContact = NULL;
			else
				SetContactInfo(jnItem, flag, true);
	}
	else if (hContact == NULL)
		return NULL;
	
	CMString tszNick, tszValue;
	int iValue;

	tszValue = jnItem["first_name"].as_mstring();
	if (!tszValue.IsEmpty()) {
		setTString(hContact, "FirstName", tszValue);
		tszNick.Append(tszValue);
		tszNick.AppendChar(' ');
	}

	tszValue = jnItem["last_name"].as_mstring();
	if (!tszValue.IsEmpty()) {
		setTString(hContact, "LastName", tszValue);
		tszNick.Append(tszValue);
	}

	if (!tszNick.IsEmpty())
		setTString(hContact, "Nick", tszNick);

	int sex = jnItem["sex"].as_int();
	if (sex)
		setByte(hContact, "Gender", sex == 2 ? 'M' : 'F');

	tszValue = jnItem["bdate"].as_mstring();
	if (!tszValue.IsEmpty()) {
		int d, m, y, iReadCount;
		iReadCount = _stscanf(tszValue, _T("%d.%d.%d"), &d, &m, &y);
		if (iReadCount > 1) {
			if (iReadCount == 3)
				setWord(hContact, "BirthYear", y);
			setByte(hContact, "BirthDay", d);
			setByte(hContact, "BirthMonth", m);
		}
	}

	tszValue = jnItem["photo_100"].as_mstring();
	if (!tszValue.IsEmpty()) {
		SetAvatarUrl(hContact, tszValue);
		ReloadAvatarInfo(hContact);
	}

	int iNewStatus = (jnItem["online"] == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE;
	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != iNewStatus)
		setWord(hContact, "Status", iNewStatus);

	if (iNewStatus == ID_STATUS_ONLINE) {
		int online_app = _ttoi(jnItem["online_app"].as_mstring());
		int online_mobile = jnItem["online_mobile"].as_int();
		
		if (online_app == 0 && online_mobile == 0)
			SetMirVer(hContact, 7); // vk.com
		else if (online_app != 0)
			SetMirVer(hContact, online_app); // App
		else 
			SetMirVer(hContact, 1); // m.vk.com
	}
	else
		SetMirVer(hContact, -1); // unset MinVer
		
	if ((iValue = jnItem["timezone"].as_int()) != 0)
		setByte(hContact, "Timezone", iValue * -2);

	tszValue = jnItem["mobile_phone"].as_mstring();
	if (!tszValue.IsEmpty())
		setTString(hContact, "Cellular", tszValue);
	
	tszValue = jnItem["home_phone"].as_mstring();
	if (!tszValue.IsEmpty())
		setTString(hContact, "Phone", tszValue);

	tszValue = jnItem["status"].as_mstring();
	CMString tszOldStatus(ptrT(db_get_tsa(hContact, hContact ? "CList" : m_szModuleName, "StatusMsg")));
	if (tszValue != tszOldStatus) {
		db_set_ts(hContact, hContact ? "CList" : m_szModuleName, "StatusMsg", tszValue);
		db_unset(hContact, m_szModuleName, "AudioUrl");
		const JSONNode &jnAudio = jnItem["status_audio"];
		if (!jnAudio.isnull()) {
			db_set_ts(hContact, m_szModuleName, "ListeningTo", tszValue);
			tszValue = jnAudio["url"].as_mstring();
			db_set_ts(hContact, m_szModuleName, "AudioUrl", tszValue);
		}
		else if (tszValue[0] == TCHAR(9835) && tszValue.GetLength() > 2)
			db_set_ts(hContact, m_szModuleName, "ListeningTo", &(tszValue.GetBuffer())[2]);
		else
			db_unset(hContact, m_szModuleName, "ListeningTo");
	}

	tszValue = jnItem["about"].as_mstring();

	if (!tszValue.IsEmpty())
		setTString(hContact, "About", tszValue);

	tszValue = jnItem["domain"].as_mstring();
	if (!tszValue.IsEmpty()) {
		setTString(hContact, "domain", tszValue);
		CMString tszUrl("https://vk.com/");
		tszUrl.Append(tszValue);
		setTString(hContact, "Homepage", tszUrl);
	}

	return hContact;
}

void CVkProto::RetrieveUserInfo(LONG userID)
{
	debugLogA("CVkProto::RetrieveUserInfo (%d)", userID);
	if (userID == VK_FEED_USER || !IsOnline())
		return;
	
	CMString userIDs, code;
	userIDs.AppendFormat(_T("%i"), userID);
			
	code.AppendFormat(_T("var userIDs=\"%s\";return{\"freeoffline\":0,\"users\":API.users.get({\"user_ids\":userIDs,\"fields\":\"%s\",\"name_case\":\"nom\"})};"), 
		userIDs, CMString(fieldsName));
	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.json", true, &CVkProto::OnReceiveUserInfo)
		<< TCHAR_PARAM("code", code)
		<< VER_API);
}

void CVkProto::RetrieveUsersInfo(bool flag)
{
	debugLogA("CVkProto::RetrieveUsersInfo");
	if (!IsOnline())
		return;

	CMString userIDs, code;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		LONG userID = getDword(hContact, "ID", -1);
		if (userID == -1 || userID == VK_FEED_USER)
			continue;
		if (!userIDs.IsEmpty())
			userIDs.AppendChar(',');
		userIDs.AppendFormat(_T("%i"), userID);
	}

	CMString codeformat("var userIDs=\"%s\";");

	if (m_bNeedSendOnline)
		codeformat += _T("API.account.setOnline();");
	
	if (flag)
		codeformat += CMString("var US=API.users.get({\"user_ids\":userIDs,\"fields\":\"%s\",\"name_case\":\"nom\"});"
			"var res=[];var index=US.length;while(index >0){index=index-1;if(US[index].online==1){res.unshift(US[index]);};};"
			"return{\"freeoffline\":1,\"users\":res,\"requests\":API.friends.getRequests({\"extended\":0,\"need_mutual\":0,\"out\":0})};");
	else
		codeformat += CMString("var res=API.users.get({\"user_ids\":userIDs,\"fields\":\"%s\",\"name_case\":\"nom\"});"
			"return{\"freeoffline\":0,\"users\":res};");
	code.AppendFormat(codeformat, userIDs, CMString(flag ? "online,status" : fieldsName));

	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.json", true, &CVkProto::OnReceiveUserInfo)
		<< TCHAR_PARAM("code", code)
		<< VER_API);
}

void CVkProto::OnReceiveUserInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUserInfo %d", reply->resultCode);

	if (reply->resultCode != 200 || !IsOnline())
		return;
	
	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	const JSONNode &jnUsers = jnResponse["users"];
	if (!jnUsers)
		return;

	LIST<void> arContacts(10, PtrKeySortT);
	MCONTACT hContact;

	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (!isChatRoom(hContact))
			arContacts.insert((HANDLE)hContact);

	for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it){
		hContact = SetContactInfo((*it));
		if (hContact)
			arContacts.remove((HANDLE)hContact);
	}
	
	if (jnResponse["freeoffline"].as_bool())
		for (int i = 0; i < arContacts.getCount(); i++) {
			hContact = (MCONTACT)arContacts[i];
			LONG userID = getDword(hContact, "ID", -1);
			if (userID == m_myUserId || userID == VK_FEED_USER)
				continue;

			int iContactStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);

			if ((iContactStatus == ID_STATUS_ONLINE)
				|| (iContactStatus == ID_STATUS_INVISIBLE && time(NULL) - getDword(hContact, "InvisibleTS", 0) >= m_iInvisibleInterval * 60)) {
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
				SetMirVer(hContact, -1);
				db_unset(hContact, m_szModuleName, "ListeningTo");
			}
		}

	arContacts.destroy();
	AddFeedSpecialUser();

	const JSONNode &jnRequests = jnResponse["requests"];
	if (!jnRequests)
		return;
	
	int iCount = jnRequests["count"].as_int();
	const JSONNode &jnItems = jnRequests["items"];
	if (!iCount || !jnItems)
		return;

	debugLogA("CVkProto::OnReceiveUserInfo AuthRequests");
	for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
		LONG userid = (*it).as_int();
		if (userid == 0)
			break;
		hContact = FindUser(userid, true);
		if (!getBool(hContact, "ReqAuth", false)) {
			RetrieveUserInfo(userid);
			setByte(hContact, "ReqAuth", 1);	
			ForkThread(&CVkProto::DBAddAuthRequestThread, (void *)hContact);		
		}
	}
}

void CVkProto::RetrieveFriends()
{
	debugLogA("CVkProto::RetrieveFriends");
	if (!IsOnline())
		return;
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.get.json", true, &CVkProto::OnReceiveFriends)
		<< INT_PARAM("count", 1000) 
		<< CHAR_PARAM("fields", fieldsName)
		<<VER_API);
}

void CVkProto::OnReceiveFriends(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveFriends %d", reply->resultCode);
	if (reply->resultCode != 200 || !IsOnline())
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	bool bCleanContacts = getBool("AutoClean", false);
	LIST<void> arContacts(10, PtrKeySortT);
		
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!isChatRoom(hContact))
			setByte(hContact, "Auth", 1);
		db_unset(hContact, m_szModuleName, "ReqAuth");
		SetMirVer(hContact, -1);
		if (bCleanContacts && !isChatRoom(hContact))
			arContacts.insert((HANDLE)hContact);
	}


	const JSONNode &jnItems = jnResponse["items"];

	if (!jnItems.isnull())
		for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
			MCONTACT hContact = SetContactInfo((*it), true);

			if (hContact == NULL || hContact == INVALID_CONTACT_ID)
				continue;

			arContacts.remove((HANDLE)hContact);
			setByte(hContact, "Auth", 0);
		}

	if (bCleanContacts)
		for (int i = 0; i < arContacts.getCount(); i++) {
			MCONTACT hContact = (MCONTACT)arContacts[i];
			LONG userID = getDword(hContact, "ID", -1);
			if (userID == m_myUserId || userID == VK_FEED_USER)
				continue;
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		}
	
	arContacts.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR __cdecl CVkProto::SvcAddAsFriend(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcAddAsFriend");
	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || userID == -1 || userID == VK_FEED_USER)
		return 1;
	CallContactService(hContact, PSS_AUTHREQUESTW, 0, (LPARAM)TranslateT("Please authorize me to add you to my friend list."));
	return 0;
}

INT_PTR __cdecl CVkProto::SvcDeleteFriend(WPARAM hContact, LPARAM flag)
{
	debugLogA("CVkProto::SvcDeleteFriend");
	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || userID == -1 || userID == VK_FEED_USER)
		return 1;
	
	 
	ptrT ptszNick(db_get_tsa(hContact, m_szModuleName, "Nick"));
	CMString ptszMsg;
	if (flag == 0) {
		ptszMsg.AppendFormat(TranslateT("Are you sure to delete %s from your friend list?"), IsEmpty(ptszNick) ? TranslateT("(Unknown contact)") : ptszNick);
		if (IDNO == MessageBox(NULL, ptszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
			return 1;
	}
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.delete.json", true, &CVkProto::OnReceiveDeleteFriend)
		<< INT_PARAM("user_id", userID)
		<< VER_API)->pUserInfo = new CVkSendMsgParam(hContact);

	return 0;
}

void CVkProto::OnReceiveDeleteFriend(NETLIBHTTPREQUEST* reply, AsyncHttpRequest* pReq)
{
	debugLogA("CVkProto::OnReceiveDeleteFriend %d", reply->resultCode);
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	if (reply->resultCode == 200) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (!jnResponse.isnull()) {
			CMString tszNick = ptrT(db_get_tsa(param->hContact, m_szModuleName, "Nick"));
			if (tszNick.IsEmpty())
				tszNick = TranslateT("(Unknown contact)");
			CMString msgformat, msg;

			if (jnResponse["success"].as_bool()) {
				if (jnResponse["friend_deleted"].as_bool())
					msgformat = TranslateT("User %s was deleted from your friend list");
				else if (jnResponse["out_request_deleted"].as_bool())
					msgformat = TranslateT("Your request to the user %s was deleted");
				else if (jnResponse["in_request_deleted"].as_bool())
					msgformat = TranslateT("Friend request from the user %s declined");
				else if (jnResponse["suggestion_deleted"].as_bool())
					msgformat = TranslateT("Friend request suggestion for the user %s deleted");
				
				msg.AppendFormat(msgformat, tszNick);
				MsgPopup(param->hContact, msg, tszNick);
				setByte(param->hContact, "Auth", 1);			
			}
			else {
				msg = TranslateT("User or request was not deleted");
				MsgPopup(param->hContact, msg, tszNick);
			}	
		}
	}
	delete param;
}

INT_PTR __cdecl CVkProto::SvcBanUser(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcBanUser");
	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || userID == -1 || userID == VK_FEED_USER)
		return 1;

	CMStringA code;
	code.AppendFormat("var userID=\"%d\";API.account.banUser({\"user_id\":userID});", userID);
	CMString tszVarWarning;

	if (m_bReportAbuse) {
		debugLogA("CVkProto::SvcBanUser m_bReportAbuse = true");
		code += "API.users.report({\"user_id\":userID,type:\"spam\"});";
		tszVarWarning = TranslateT(" report abuse on him/her");
	}
	if (m_bClearServerHistory) {
		debugLogA("CVkProto::SvcBanUser m_bClearServerHistory = true");
		code += "API.messages.deleteDialog({\"user_id\":userID,count:10000});";
		if (!tszVarWarning.IsEmpty())
			tszVarWarning.AppendChar(L',');
		tszVarWarning += TranslateT(" clear server history with him/her");
	}
	if (m_bRemoveFromFrendlist) {
		debugLogA("CVkProto::SvcBanUser m_bRemoveFromFrendlist = true");
		code += "API.friends.delete({\"user_id\":userID});";
		if (!tszVarWarning.IsEmpty())
			tszVarWarning.AppendChar(L',');
		tszVarWarning += TranslateT(" remove him/her from your friend list");
	}
	if (m_bRemoveFromClist) {
		debugLogA("CVkProto::SvcBanUser m_bRemoveFromClist = true");
		if (!tszVarWarning.IsEmpty())
			tszVarWarning.AppendChar(L',');
		tszVarWarning += TranslateT(" remove him/her from your contact list");
	}

	if (!tszVarWarning.IsEmpty())
		tszVarWarning += ".\n";
	code += "return 1;";

	ptrT ptszNick(db_get_tsa(hContact, m_szModuleName, "Nick"));
	CMString ptszMsg;

	ptszMsg.AppendFormat(TranslateT("Are you sure to ban %s? %s%sContinue?"), 
		IsEmpty(ptszNick) ? TranslateT("(Unknown contact)") : ptszNick, 
		tszVarWarning.IsEmpty() ? _T(" ") : TranslateT("\nIt will also"),
		tszVarWarning.IsEmpty() ? _T("\n") : tszVarWarning);

	if (IDNO == MessageBox(NULL, ptszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;
	
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveSmth)
		<< CHAR_PARAM("code", code)
		<< VER_API);

	if (m_bRemoveFromClist)
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);

	return 0;
}

INT_PTR __cdecl CVkProto::SvcReportAbuse(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcReportAbuse");
	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || userID == -1 || userID == VK_FEED_USER)
		return 1;

	CMString formatstr = TranslateT("Are you sure to report abuse on %s?"),
		tszNick = ptrT(db_get_tsa(hContact, m_szModuleName, "Nick")),
		ptszMsg;
	ptszMsg.AppendFormat(formatstr, tszNick.IsEmpty() ? TranslateT("(Unknown contact)") : tszNick);
	if (IDNO == MessageBox(NULL, ptszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.report.json", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("user_id", userID)
		<< CHAR_PARAM("type", "spam")
		<< VER_API);

	return 0;
}

INT_PTR __cdecl CVkProto::SvcOpenBroadcast(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcOpenBroadcast");
	CMString tszAudio(ptrT(db_get_tsa(hContact, m_szModuleName, "AudioUrl")));
	
	if (!tszAudio.IsEmpty())
		CallService(MS_UTILS_OPENURL, (WPARAM)OUF_TCHAR, (LPARAM)tszAudio.GetBuffer());
	
	return 0;
}

INT_PTR __cdecl CVkProto::SvcVisitProfile(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcVisitProfile");
	LONG userID = getDword(hContact, "ID", -1);
	ptrT tszDomain(db_get_tsa(hContact, m_szModuleName, "domain"));
	CMString tszUrl("https://vk.com/");

	if (tszDomain)
		tszUrl.Append(tszDomain);
	else
		tszUrl.AppendFormat(_T("id%i"), userID);
	
	CallService(MS_UTILS_OPENURL, (WPARAM)OUF_TCHAR, (LPARAM)tszUrl.GetBuffer());
	
	return 0;
}