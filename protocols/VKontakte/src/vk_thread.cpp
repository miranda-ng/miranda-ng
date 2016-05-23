/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

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
mir_cs CVkProto::m_csTimer;

char szBlankUrl[] = "https://oauth.vk.com/blank.html";
static char VK_TOKEN_BEG[] = "access_token=";
static char VK_LOGIN_DOMAIN[] = "https://m.vk.com";
static char fieldsName[] = "id, first_name, last_name, photo_100, bdate, sex, timezone, " 
	"contacts, last_seen, online, status, country, city, relation, interests, activities, "
	"music, movies, tv, books, games, quotes, about,  domain";

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ShutdownSession()
{
	debugLogA("CVkProto::ShutdownSession");
	m_bTerminated = true;
	if (m_hWorkerThread) 
		SetEvent(m_evRequestsQueue);
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
	mir_cslock lck(csInstances);
	for (int i = 0; i < vk_Instances.getCount(); i++)
		if (vk_Instances[i]->IsOnline()) {
			vk_Instances[i]->debugLogA("Tic timer for %i - %s", i, vk_Instances[i]->m_szModuleName);
			vk_Instances[i]->OnTimerTic();			
		}
}

static void CALLBACK VKSetTimer(void*)
{
	mir_cslock lck(CVkProto::m_csTimer);
	if (CVkProto::m_timer)
		return;
	CVkProto::m_timer = SetTimer(NULL, 0, 60000, TimerProc);
}

static void CALLBACK VKUnsetTimer(void*)
{
	mir_cslock lck(CVkProto::m_csTimer);
	if (CVkProto::m_timer)
		KillTimer(NULL, CVkProto::m_timer);
	CVkProto::m_timer = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::OnTimerTic()
{
	SetServerStatus(m_iDesiredStatus);
	RetrieveUsersInfo(true);
	RetrieveUnreadEvents();
}

void CVkProto::OnLoggedIn()
{
	debugLogA("CVkProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);

	// initialize online timer
	CallFunctionAsync(VKSetTimer, this);

	db_unset(NULL, m_szModuleName, "LastNewsReqTime");
	db_unset(NULL, m_szModuleName, "LastNotificationsReqTime");
}

void CVkProto::OnLoggedOut()
{
	debugLogA("CVkProto::OnLoggedOut");
	m_bOnline = false;

	if (m_hAPIConnection)
		Netlib_CloseHandle(m_hAPIConnection);

	if (m_pollingConn)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_pollingConn);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	bool bOnline = false;
	{
		mir_cslock lck(csInstances);
		for (int i = 0; i < vk_Instances.getCount(); i++)
			bOnline = bOnline || vk_Instances[i]->IsOnline();
	}
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
				AsyncHttpRequest *pRedirectReq = new AsyncHttpRequest();
				pRedirectReq->requestType = REQUEST_GET;
				pRedirectReq->flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
				pRedirectReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
				pRedirectReq->AddHeader("Referer", m_prevUrl);
				pRedirectReq->Redirect(reply);
				if (!pRedirectReq->m_szUrl.IsEmpty()) {
					if (pRedirectReq->m_szUrl[0] == '/')
						pRedirectReq->m_szUrl = VK_LOGIN_DOMAIN + pRedirectReq->m_szUrl;
					ApplyCookies(pRedirectReq);
					m_prevUrl = pRedirectReq->m_szUrl;
				}
				pRedirectReq->m_bApiReq = false;
				pRedirectReq->bIsMainConn = true;
				Push(pRedirectReq);
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
	if (!pReq->m_szUrl.IsEmpty() && pReq->m_szUrl[0] == '/')
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
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.get.json", true, &CVkProto::OnReceiveMyInfo, AsyncHttpRequest::rpHigh));
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

	const JSONNode &jnLastSeen = jnItem["last_seen"];
	if (jnLastSeen) {
		int iLastSeen = jnLastSeen["time"].as_int();
		int iOldLastSeen = db_get_dw(hContact, "BuddyExpectator", "LastSeen");
		if (iLastSeen && iLastSeen > iOldLastSeen)  {
			db_set_dw(hContact, "BuddyExpectator", "LastSeen", (DWORD)iLastSeen);
			db_set_w(hContact, "BuddyExpectator", "LastStatus", ID_STATUS_ONLINE);
		}
	}

	int iNewStatus = (jnItem["online"].as_int() == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE;
	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != iNewStatus)
		setWord(hContact, "Status", iNewStatus);

	if (iNewStatus == ID_STATUS_ONLINE) {
		db_set_dw(hContact, "BuddyExpectator", "LastSeen", (DWORD)time(NULL));
		db_set_dw(hContact, "BuddyExpectator", "LastStatus", ID_STATUS_ONLINE);

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
	if (tszValue != tszOldStatus)
		db_set_ts(hContact, hContact ? "CList" : m_szModuleName, "StatusMsg", tszValue);

	CMString tszOldListeningTo(ptrT(db_get_tsa(hContact, m_szModuleName, "ListeningTo")));
	const JSONNode &jnAudio = jnItem["status_audio"];
	if (jnAudio) {
		CMString tszListeningTo(FORMAT, _T("%s - %s"), jnAudio["artist"].as_mstring(), jnAudio["title"].as_mstring());
		if (tszListeningTo != tszOldListeningTo) {
			setTString(hContact, "ListeningTo", tszListeningTo);
			setTString(hContact, "AudioUrl", jnAudio["url"].as_mstring());
		}
	}
	else if (tszValue[0] == TCHAR(9835) && tszValue.GetLength() > 2) {
		setTString(hContact, "ListeningTo", &(tszValue.GetBuffer())[2]);
		db_unset(hContact, m_szModuleName, "AudioUrl");
	}
	else {
		db_unset(hContact, m_szModuleName, "ListeningTo");
		db_unset(hContact, m_szModuleName, "AudioUrl");
	}

	const JSONNode &jnCountry = jnItem["country"];
	if (jnCountry) {
		tszValue = jnCountry["title"].as_mstring();
		if (!tszValue.IsEmpty())
			setTString(hContact, "Country", tszValue);
	}

	const JSONNode &jnCity = jnItem["city"];
	if (jnCity) {
		tszValue = jnCity["title"].as_mstring();
		if (!tszValue.IsEmpty())
			setTString(hContact, "City", tszValue);
	}

	// MaritalStatus
	BYTE cMaritalStatus[] = {0, 10, 11, 12, 20, 70, 50, 60};

	if (jnItem["relation"] && jnItem["relation"].as_int() < _countof(cMaritalStatus))
		setByte(hContact, "MaritalStatus", cMaritalStatus[jnItem["relation"].as_int()]);

	//  interests, activities, music, movies, tv, books, games, quotes
	CVKInteres vkInteres[] = { 
		{ "interests", TranslateT("Interests") }, 
		{ "activities", TranslateT("Activities") }, 
		{ "music", TranslateT("Music") }, 
		{ "movies", TranslateT("Movies") }, 
		{ "tv", TranslateT("TV") },
		{ "books", TranslateT("Books") }, 
		{ "games", TranslateT("Games") },
		{ "quotes", TranslateT("Quotes") } 
	};

	int iInteres = 0;

	for (int i = 0; i < _countof(vkInteres); i++) {
		tszValue = jnItem[vkInteres[i].szField].as_mstring();
		if (tszValue.IsEmpty())
			continue;
		
		CMStringA InteresCat(FORMAT, "Interest%dCat", iInteres);
		CMStringA InteresText(FORMAT, "Interest%dText", iInteres);

		setTString(hContact, InteresCat, vkInteres[i].ptszTranslate);
		setTString(hContact, InteresText, tszValue);

		iInteres++;
		
	}

	for (int i = iInteres; iInteres > 0; i++) {
		CMStringA InteresCat(FORMAT, "Interest%dCat", iInteres);
		ptrT ptszCat(db_get_tsa(hContact, m_szModuleName, InteresCat));
		if (!ptszCat)
			break;
		db_unset(hContact, m_szModuleName, InteresCat);

		CMStringA InteresText(FORMAT, "Interest%dText", iInteres);
		ptrT ptszText(db_get_tsa(hContact, m_szModuleName, InteresText));
		if (!ptszText)
			break;
		db_unset(hContact, m_szModuleName, InteresText);
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

	CMString code(FORMAT, _T("var userIDs=\"%i\";var res=API.users.get({\"user_ids\":userIDs,\"fields\":\"%s\",\"name_case\":\"nom\"});return{\"freeoffline\":0,\"norepeat\":1,\"usercount\":res.length,\"users\":res};"),
		userID, CMString(fieldsName));
	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.json", true, &CVkProto::OnReceiveUserInfo)
		<< TCHAR_PARAM("code", code));
}

void CVkProto::RetrieveUsersInfo(bool bFreeOffline, bool bRepeat)
{
	debugLogA("CVkProto::RetrieveUsersInfo");
	if (!IsOnline())
		return;

	CMString userIDs, code;
	int i = 0;
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		LONG userID = getDword(hContact, "ID", -1);
		if (userID == -1 || userID == VK_FEED_USER)
			continue;
		if (!userIDs.IsEmpty())
			userIDs.AppendChar(',');
		userIDs.AppendFormat(_T("%i"), userID);
		
		if (i == MAX_CONTACTS_PER_REQUEST)
			break;
		i++;
	}

	CMString codeformat("var userIDs=\"%s\";var _fields=\"%s\";");

	if (m_bNeedSendOnline)
		codeformat += _T("API.account.setOnline();");

	if (bFreeOffline && !m_vkOptions.bLoadFullCList)
		codeformat += CMString("var US=[];var res=[];var t=10;while(t>0){"
			"US=API.users.get({\"user_ids\":userIDs,\"fields\":_fields,\"name_case\":\"nom\"});"
			"var index=US.length;while(index>0){"
			"index=index-1;if(US[index].online!=0){res.push(US[index]);};};"
			"t=t-1;if(res.length>0)t=0;};"
			"return{\"freeoffline\":1,\"norepeat\":%d,\"usercount\":res.length,\"users\":res,\"requests\":API.friends.getRequests({\"extended\":0,\"need_mutual\":0,\"out\":0})};");
	else
		codeformat += CMString("var res=API.users.get({\"user_ids\":userIDs,\"fields\":_fields,\"name_case\":\"nom\"});"
			"return{\"freeoffline\":0,\"norepeat\":%d,\"usercount\":res.length,\"users\":res,\"requests\":API.friends.getRequests({\"extended\":0,\"need_mutual\":0,\"out\":0})};");
	code.AppendFormat(codeformat, userIDs, CMString(bFreeOffline ? "online,status" : fieldsName), (int)bRepeat);

	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.json", true, &CVkProto::OnReceiveUserInfo)
		<< TCHAR_PARAM("code", code));
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

	if (!jnResponse["norepeat"].as_bool() && jnResponse["usercount"].as_int() == 0) {
		RetrieveUsersInfo(true, true);
		return;
	}

	MCONTACT hContact;
	LIST<void> arContacts(10, PtrKeySortT);

	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		if (!isChatRoom(hContact))
			arContacts.insert((HANDLE)hContact);

	for (auto it = jnUsers.begin(); it != jnUsers.end(); ++it) {
		hContact = SetContactInfo((*it));
		if (hContact)
			arContacts.remove((HANDLE)hContact);
	}

	if (jnResponse["freeoffline"].as_bool())
		for (int i = 0; i < arContacts.getCount(); i++) {
			hContact = (UINT_PTR)arContacts[i];
			LONG userID = getDword(hContact, "ID", -1);
			if (userID == m_myUserId || userID == VK_FEED_USER)
				continue;

			int iContactStatus = getWord(hContact, "Status", ID_STATUS_OFFLINE);

			if ((iContactStatus == ID_STATUS_ONLINE)
				|| (iContactStatus == ID_STATUS_INVISIBLE && time(NULL) - getDword(hContact, "InvisibleTS", 0) >= m_vkOptions.iInvisibleInterval * 60LL)) {
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
		if (!getBool(hContact, "ReqAuth")) {
			RetrieveUserInfo(userid);
			setByte(hContact, "ReqAuth", 1);
			ForkThread(&CVkProto::DBAddAuthRequestThread, (void *)hContact);
		}
	}
}

void CVkProto::RetrieveFriends(bool bCleanNonFriendContacts)
{
	debugLogA("CVkProto::RetrieveFriends");
	if (!IsOnline())
		return;
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.get.json", true, &CVkProto::OnReceiveFriends)
		<< INT_PARAM("count", m_vkOptions.iMaxFriendsCount > 5000 ? 1000 : m_vkOptions.iMaxFriendsCount)
		<< CHAR_PARAM("fields", fieldsName))->pUserInfo = new CVkSendMsgParam(NULL, bCleanNonFriendContacts ? 1 : 0);
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
	
	CVkSendMsgParam *param = (CVkSendMsgParam *)pReq->pUserInfo;
	bool bCleanContacts = getBool("AutoClean") || (param->iMsgID != 0);
	delete param;

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

	if (jnItems)
		for (auto it = jnItems.begin(); it != jnItems.end(); ++it) {
			MCONTACT hContact = SetContactInfo((*it), true);

			if (hContact == NULL || hContact == INVALID_CONTACT_ID)
				continue;

			arContacts.remove((HANDLE)hContact);
			setByte(hContact, "Auth", 0);
		}

	if (bCleanContacts)
		for (int i = 0; i < arContacts.getCount(); i++) {
			MCONTACT hContact = (UINT_PTR)arContacts[i];
			LONG userID = getDword(hContact, "ID", -1);
			if (userID == m_myUserId || userID == VK_FEED_USER)
				continue;
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact);
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
	CallContactService(hContact, PSS_AUTHREQUEST, 0, (LPARAM)TranslateT("Please authorize me to add you to my friend list."));
	return 0;
}

INT_PTR CVkProto::SvcWipeNonFriendContacts(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcWipeNonFriendContacts");
	if (IDNO == MessageBox(NULL, TranslateT("Are you sure to wipe local contacts missing in your friend list?"), 
		TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 0;

	RetrieveFriends(true);
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
		<< INT_PARAM("user_id", userID))->pUserInfo = new CVkSendMsgParam(hContact);

	return 0;
}

void CVkProto::OnReceiveDeleteFriend(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveDeleteFriend %d", reply->resultCode);
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	if (reply->resultCode == 200 && param) {
		JSONNode jnRoot;
		const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
		if (jnResponse) {
			CMString tszNick(ptrT(db_get_tsa(param->hContact, m_szModuleName, "Nick")));
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

	if (param && (!pReq->bNeedsRestart || m_bTerminated)) {
		delete param;
		pReq->pUserInfo = NULL;
	}
}

INT_PTR __cdecl CVkProto::SvcBanUser(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcBanUser");
	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || userID == -1 || userID == VK_FEED_USER)
		return 1;

	CMStringA code(FORMAT, "var userID=\"%d\";API.account.banUser({\"user_id\":userID});", userID);
	CMString tszVarWarning;

	if (m_vkOptions.bReportAbuse) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bReportAbuse = true");
		code += "API.users.report({\"user_id\":userID,type:\"spam\"});";
		tszVarWarning = TranslateT(" report abuse on him/her");
	}
	if (m_vkOptions.bClearServerHistory) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bClearServerHistory = true");
		code += "API.messages.deleteDialog({\"user_id\":userID,count:10000});";
		if (!tszVarWarning.IsEmpty())
			tszVarWarning.AppendChar(L',');
		tszVarWarning += TranslateT(" clear server history with him/her");
	}
	if (m_vkOptions.bRemoveFromFrendlist) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bRemoveFromFrendlist = true");
		code += "API.friends.delete({\"user_id\":userID});";
		if (!tszVarWarning.IsEmpty())
			tszVarWarning.AppendChar(L',');
		tszVarWarning += TranslateT(" remove him/her from your friend list");
	}
	if (m_vkOptions.bRemoveFromCList) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bRemoveFromClist = true");
		if (!tszVarWarning.IsEmpty())
			tszVarWarning.AppendChar(L',');
		tszVarWarning += TranslateT(" remove him/her from your contact list");
	}

	if (!tszVarWarning.IsEmpty())
		tszVarWarning += ".\n";
	code += "return 1;";

	ptrT ptszNick(db_get_tsa(hContact, m_szModuleName, "Nick"));
	CMString ptszMsg(FORMAT, TranslateT("Are you sure to ban %s? %s%sContinue?"),
		IsEmpty(ptszNick) ? TranslateT("(Unknown contact)") : ptszNick, 
		tszVarWarning.IsEmpty() ? _T(" ") : TranslateT("\nIt will also"),
		tszVarWarning.IsEmpty() ? _T("\n") : tszVarWarning);

	if (IDNO == MessageBox(NULL, ptszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;
	
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveSmth)
		<< CHAR_PARAM("code", code));

	if (m_vkOptions.bRemoveFromCList)
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact);

	return 0;
}

INT_PTR __cdecl CVkProto::SvcReportAbuse(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcReportAbuse");
	LONG userID = getDword(hContact, "ID", -1);
	if (!IsOnline() || userID == -1 || userID == VK_FEED_USER)
		return 1;

	CMString tszNick(ptrT(db_get_tsa(hContact, m_szModuleName, "Nick"))),
		ptszMsg(FORMAT, TranslateT("Are you sure to report abuse on %s?"), tszNick.IsEmpty() ? TranslateT("(Unknown contact)") : tszNick);
	if (IDNO == MessageBox(NULL, ptszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.report.json", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("user_id", userID)
		<< CHAR_PARAM("type", "spam"));

	return 0;
}

INT_PTR __cdecl CVkProto::SvcOpenBroadcast(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcOpenBroadcast");

	CMString tszAudio(ptrT(db_get_tsa(hContact, m_szModuleName, "AudioUrl")));
	if (!tszAudio.IsEmpty())
		Utils_OpenUrlT(tszAudio);

	return 0;
}

INT_PTR __cdecl CVkProto::SvcVisitProfile(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcVisitProfile");
	if (isChatRoom(hContact)) {
		ptrT tszHomepage(db_get_tsa(hContact, m_szModuleName, "Homepage"));
		if(!IsEmpty(tszHomepage))
			Utils_OpenUrlT(tszHomepage);
		return 0;
	}

	LONG userID = getDword(hContact, "ID", -1);
	ptrT tszDomain(db_get_tsa(hContact, m_szModuleName, "domain"));

	CMString tszUrl("https://vk.com/");
	if (tszDomain)
		tszUrl.Append(tszDomain);
	else
		tszUrl.AppendFormat(_T("id%i"), userID);
	
	Utils_OpenUrlT(tszUrl);
	return 0;
}