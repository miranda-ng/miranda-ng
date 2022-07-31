/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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
char szScore[] = "friends,photos,audio,docs,video,wall,messages,offline,status,notifications,groups";
char szVKUserAgent[] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.54 Safari/537.36 Edg/95.0.1020.30";
char szVKUserAgentCH[] = "\"Microsoft Edge\";v =\"95\", \"Chromium\";v =\"95\", \";Not A Brand\";v = \"99\"";

static char szVKTokenBeg[] = "access_token=";
static char szVKLoginDomain[] = "https://m.vk.com";
static char szVKCookieDomain[] = ".vk.com";
static char szFieldsName[] = "id, first_name, last_name, photo_100, bdate, sex, timezone, "
	"contacts, last_seen, online, status, country, city, relation, interests, activities, "
	"music, movies, tv, books, games, quotes, about,  domain";

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::CheckHealthThreads()
{
	if (!IsOnline()) {
		debugLogA("CVkProto::CheckHealthThreads Offline");
		return false;
	}

	time_t tNow = time(0);

	{
		mir_cslock lck(m_csWorkThreadTimer);
		if ((m_tWorkThreadTimer + 3 * 60) < tNow) {
			debugLogA("CVkProto::CheckHealthThreads Work Thread is freeze => ShutdownSession()");
			ShutdownSession();
			return false;
		}
	}

	{
		mir_cslock lck(m_csPoolThreadTimer);
		if ((m_tPoolThreadTimer + 3 * 60) < tNow) {
			debugLogA("CVkProto::CheckHealthThreads Pool Thread is freeze => ShutdownSession()");
			ShutdownSession();
			return false;
		}
	}

	debugLogA("CVkProto::CheckHealthThreads OK");
	return true;
}

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
	m_bErr404Return = false;

	ProtoBroadcastAck(0, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, iReason);
	debugLogA("CVkProto::ConnectionFailed ShutdownSession");
	ShutdownSession();
}

/////////////////////////////////////////////////////////////////////////////////////////

static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslock lck(csInstances);
	for (auto &it : g_plugin.g_arInstances)
		if (it->IsOnline()) {
			it->debugLogA("Tic timer for %s", it->m_szModuleName);
			if (it->CheckHealthThreads())
				it->OnTimerTic();
		}
}

static void CALLBACK VKSetTimer(void*)
{
	mir_cslock lck(CVkProto::m_csTimer);
	if (CVkProto::m_timer)
		return;
	CVkProto::m_timer = SetTimer(nullptr, 0, 60000, TimerProc);
}

static void CALLBACK VKUnsetTimer(void*)
{
	mir_cslock lck(CVkProto::m_csTimer);
	if (CVkProto::m_timer)
		KillTimer(nullptr, CVkProto::m_timer);
	CVkProto::m_timer = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::OnTimerTic()
{
	RetrieveUsersInfo(true);
	RetrieveUnreadEvents();
	SetServerStatus(m_iDesiredStatus);
}

void CVkProto::OnLoggedIn()
{
	debugLogA("CVkProto::OnLoggedIn");
	m_bOnline = true;
	SetServerStatus(m_iDesiredStatus);

	// initialize online timer
	CallFunctionAsync(VKSetTimer, this);

	db_unset(0, m_szModuleName, "LastNewsReqTime");
	db_unset(0, m_szModuleName, "LastNotificationsReqTime");
}

void CVkProto::ClosePollingConnection(bool bShutdown)
{
	if (!m_pollingConn)
		return;

	debugLogA("CVkProto::ClosePollingConnection %d", bShutdown ? 1 : 0);

	if (bShutdown)
		Netlib_Shutdown(m_pollingConn);
	else
		Netlib_CloseHandle(m_pollingConn);
	m_pollingConn = nullptr;
}

void CVkProto::CloseAPIConnection(bool bShutdown)
{
	if (!m_hAPIConnection)
		return;

	debugLogA("CVkProto::CloseAPIConnection %d", bShutdown ? 1 : 0);

	if (bShutdown)
		Netlib_Shutdown(m_hAPIConnection);
	else
		Netlib_CloseHandle(m_hAPIConnection);
	m_hAPIConnection = nullptr;
}

void CVkProto::OnLoggedOut()
{
	debugLogA("CVkProto::OnLoggedOut");
	m_bOnline = false;

	if (m_hPollingThread) {
		CloseHandle(m_hPollingThread);
		m_hPollingThread = nullptr;
	}

	if (m_hWorkerThread) {
		CloseHandle(m_hWorkerThread);
		m_hWorkerThread = nullptr;
	}

	CloseAPIConnection(true);
	ClosePollingConnection(true);

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	bool bOnline = false;
	{
		mir_cslock lck(csInstances);
		for (auto &it : g_plugin.g_arInstances)
			bOnline = bOnline || it->IsOnline();
	}
	if (!bOnline)
		CallFunctionAsync(VKUnsetTimer, this);
	SetAllContactStatuses(ID_STATUS_OFFLINE);
	m_chats.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::OnOAuthAuthorize(NETLIBHTTPREQUEST *reply, AsyncHttpRequest*)
{
	debugLogA("CVkProto::OnOAuthAuthorize %d", reply->resultCode);
	GrabCookies(reply, szVKCookieDomain);

	if (reply->resultCode == 404 && !m_bErr404Return) {
		m_bErr404Return = true;
		setString("AccessScore", szScore);
		AsyncHttpRequest* pReq = new AsyncHttpRequest(this, REQUEST_GET, "https://oauth.vk.com/authorize", false, &CVkProto::OnOAuthAuthorize);
		pReq
			<< INT_PARAM("client_id", VK_APP_ID)
			<< CHAR_PARAM("scope", szScore)
			<< CHAR_PARAM("redirect_uri", szBlankUrl)
			<< CHAR_PARAM("display", "mobile")
			<< CHAR_PARAM("response_type", "token")
			<< VER_API;
		pReq->m_bApiReq = false;
		pReq->bIsMainConn = true;
		ApplyCookies(pReq);
		pReq->AddHeader("User-agent", szVKUserAgent);
		Push(pReq);
		return;
	}

	if (reply->resultCode == 302) { // manual redirect
		LPCSTR pszLocation = Netlib_GetHeader(reply, "Location");
		if (pszLocation) {
			if (!_strnicmp(pszLocation, szBlankUrl, sizeof(szBlankUrl) - 1)) {
				m_szAccessToken = nullptr;
				LPCSTR p = strstr(pszLocation, szVKTokenBeg);
				if (p) {
					p += sizeof(szVKTokenBeg) - 1;
					for (LPCSTR q = p + 1; *q; q++) {
						if (*q == '&' || *q == '=' || *q == '\"') {
							m_szAccessToken = mir_strndup(p, q - p);
							break;
						}
					}
					if (m_szAccessToken == nullptr)
						m_szAccessToken = mir_strdup(p);
					setString("AccessToken", m_szAccessToken);
					RetrieveMyInfo();
				}
				else
					ConnectionFailed(LOGINERR_NOSERVER);
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
						pRedirectReq->m_szUrl = szVKLoginDomain + pRedirectReq->m_szUrl;
					ApplyCookies(pRedirectReq);
					m_prevUrl = pRedirectReq->m_szUrl;
				}

				pRedirectReq->m_bApiReq = false;
				pRedirectReq->bIsMainConn = true;
				// Headers
				pRedirectReq->AddHeader("User-agent", szVKUserAgent);
				pRedirectReq->AddHeader("dht", "1");
				pRedirectReq->AddHeader("sec-ch-ua", szVKUserAgentCH);
				pRedirectReq->AddHeader("sec-ch-ua-mobile", "?0");
				pRedirectReq->AddHeader("sec-ch-ua-platform", "Windows");
				pRedirectReq->AddHeader("sec-fetch-dest", "document");
				pRedirectReq->AddHeader("sec-fetch-mode", "navigate");
				pRedirectReq->AddHeader("sec-fetch-site", "same-site");
				pRedirectReq->AddHeader("sec-fetch-user", "?1");
				pRedirectReq->AddHeader("upgrade-insecure-requests", "1");
				//Headers

				Push(pRedirectReq);
			}
		}
		else
			ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	if (reply->resultCode != 200 || !reply->pData || (!(strstr(reply->pData, "method=\"post\"") || strstr(reply->pData, "method=\"POST\"")) && !strstr(reply->pData, "meta http-equiv=\"refresh\""))) { // something went wrong
		ConnectionFailed(LOGINERR_NOSERVER);
		return;
	}

	LPCSTR pBlankUrl = strstr(reply->pData, szBlankUrl);
	if (pBlankUrl) {
		debugLogA("CVkProto::OnOAuthAuthorize blank ulr found");
		m_szAccessToken = nullptr;
		LPCSTR p = strstr(pBlankUrl, szVKTokenBeg);
		if (p) {
			p += sizeof(szVKTokenBeg) - 1;
			for (LPCSTR q = p + 1; *q; q++) {
				if (*q == '&' || *q == '=' || *q == '\"') {
					m_szAccessToken = mir_strndup(p, q - p);
					break;
				}
			}
			setString("AccessToken", m_szAccessToken);
			RetrieveMyInfo();
		}
		else {
			debugLogA("CVkProto::OnOAuthAuthorize blank ulr found, access_token not found");
			ConnectionFailed(LOGINERR_NOSERVER);
		}
		return;
	}

	char* pMsgWarning = strstr(reply->pData, "service_msg_warning");
	if (pMsgWarning) {
		char *p1 = strchr(pMsgWarning, '>');
		char *p2 = strchr(pMsgWarning, '<');
		if (p1 && p2 && (p1 + 1 < p2)) {
			CMStringA szMsg(p1 + 1, (int)(p2 - p1 - 1));
			MsgPopup(ptrW(mir_utf8decodeW(szMsg)), TranslateT("Service message"), true);
			debugLogA("CVkProto::OnOAuthAuthorize %s", szMsg.c_str());
		}
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

	AsyncHttpRequest *pReq = new AsyncHttpRequest();
	pReq->requestType = REQUEST_POST;
	pReq->flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	pReq->m_szParam = szBody;
	pReq->m_szUrl = szAction;
	if (!pReq->m_szUrl.IsEmpty() && pReq->m_szUrl[0] == '/')
		pReq->m_szUrl = szVKLoginDomain + pReq->m_szUrl;
	m_prevUrl = pReq->m_szUrl;
	pReq->m_pFunc = &CVkProto::OnOAuthAuthorize;
	pReq->AddHeader("Content-Type", "application/x-www-form-urlencoded");
	pReq->Redirect(reply);
	ApplyCookies(pReq);
	// Headers
	pReq->AddHeader("User-agent", szVKUserAgent);
	pReq->AddHeader("dht", "1");
	pReq->AddHeader("origin", "https://oauth.vk.com");
	pReq->AddHeader("referer", "https://oauth.vk.com/");
	pReq->AddHeader("sec-ch-ua", szVKUserAgentCH);
	pReq->AddHeader("sec-ch-ua-mobile", "?0");
	pReq->AddHeader("sec-ch-ua-platform", "Windows");
	pReq->AddHeader("sec-fetch-dest", "document");
	pReq->AddHeader("sec-fetch-mode", "navigate");
	pReq->AddHeader("sec-fetch-site", "same-site");
	pReq->AddHeader("sec-fetch-user", "?1");
	pReq->AddHeader("upgrade-insecure-requests", "1");
	//Headers
	pReq->m_bApiReq = false;
	pReq->bIsMainConn = true;
	Push(pReq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::TrackVisitor()
{
	debugLogA("CVkProto::TrackVisitor");
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/stats.trackVisitor.json", true, &CVkProto::OnReceiveSmth, AsyncHttpRequest::rpLow));
}

void CVkProto::RetrieveMyInfo()
{
	debugLogA("CVkProto::RetrieveMyInfo");
	m_bErr404Return = false;

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
	TrackVisitor();
	RetrieveUnreadMessages();
	RetrieveFriends(m_vkOptions.bLoadOnlyFriends);
	RetrievePollingInfo();
}

MCONTACT CVkProto::SetContactInfo(const JSONNode &jnItem, bool flag, VKContactType vkContactType)
{
	if (!jnItem) {
		debugLogA("CVkProto::SetContactInfo pItem == nullptr");
		return INVALID_CONTACT_ID;
	}

	LONG userid = jnItem["id"].as_int();
	debugLogA("CVkProto::SetContactInfo %d", userid);
	if (userid == 0 || userid == VK_FEED_USER)
		return 0;

	MCONTACT hContact = FindUser(userid, flag);

	if (userid == m_myUserId) {
		if (hContact != 0)
			if (vkContactType == VKContactType::vkContactSelf)
				hContact = 0;
			else
				SetContactInfo(jnItem, flag, VKContactType::vkContactSelf);
	}
	else if (hContact == 0)
		return 0;

	if (vkContactType == VKContactType::vkContactMUCUser) {
		Contact::Hide(hContact);
		Contact::RemoveFromList(hContact);
		db_set_dw(hContact, "Ignore", "Mask1", 0);
	}

	CMStringW wszNick, wszValue;
	int iValue;

	wszValue = jnItem["first_name"].as_mstring();
	if (!wszValue.IsEmpty()) {
		setWString(hContact, "FirstName", wszValue);
		wszNick.Append(wszValue);
		wszNick.AppendChar(' ');
	}

	wszValue = jnItem["last_name"].as_mstring();
	if (!wszValue.IsEmpty()) {
		setWString(hContact, "LastName", wszValue);
		wszNick.Append(wszValue);
	}

	if (!wszNick.IsEmpty())
		setWString(hContact, "Nick", wszNick);

	wszValue = jnItem["deactivated"].as_mstring();
	CMStringW wszOldDeactivated(ptrW(db_get_wsa(hContact, m_szModuleName, "Deactivated")));

	if (wszValue == L"deleted" && wszOldDeactivated != L"?deleted" && wszOldDeactivated != L"deleted")
		wszValue = L"?deleted";
	else if (wszValue.IsEmpty() && wszOldDeactivated == L"?deleted") {
		db_unset(hContact, m_szModuleName, "Deactivated");
		wszOldDeactivated.Empty();
	}

	if (wszValue != wszOldDeactivated) {
		AddVkDeactivateEvent(hContact, wszValue);
		if (wszValue.IsEmpty())
			db_unset(hContact, m_szModuleName, "Deactivated");
		else
			setWString(hContact, "Deactivated", wszValue);
	}

	if (!wszValue.IsEmpty())
		return hContact;

	int sex = jnItem["sex"].as_int();
	if (sex)
		setByte(hContact, "Gender", sex == 2 ? 'M' : 'F');

	wszValue = jnItem["bdate"].as_mstring();
	if (!wszValue.IsEmpty()) {
		int d, m, y, iReadCount;
		iReadCount = swscanf(wszValue, L"%d.%d.%d", &d, &m, &y);
		if (iReadCount > 1) {
			if (iReadCount == 3)
				setWord(hContact, "BirthYear", y);
			setByte(hContact, "BirthDay", d);
			setByte(hContact, "BirthMonth", m);
		}
	}

	wszValue = jnItem["photo_100"].as_mstring();
	if (!wszValue.IsEmpty()) {
		SetAvatarUrl(hContact, wszValue);
		ReloadAvatarInfo(hContact);
	}

	const JSONNode &jnLastSeen = jnItem["last_seen"];
	if (jnLastSeen) {
		int iLastSeen = jnLastSeen["time"].as_int();
		int iOldLastSeen = db_get_dw(hContact, "BuddyExpectator", "LastSeen");
		if (iLastSeen && iLastSeen > iOldLastSeen) {
			db_set_dw(hContact, "BuddyExpectator", "LastSeen", (uint32_t)iLastSeen);
			db_set_w(hContact, "BuddyExpectator", "LastStatus", ID_STATUS_ONLINE);
		}
	}

	int iNewStatus = (jnItem["online"].as_int() == 0) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE;
	setWord(hContact, "Status", iNewStatus);

	if (iNewStatus == ID_STATUS_ONLINE) {
		db_set_dw(hContact, "BuddyExpectator", "LastSeen", (uint32_t)time(0));
		db_set_dw(hContact, "BuddyExpectator", "LastStatus", ID_STATUS_ONLINE);

		int online_app = _wtoi(jnItem["online_app"].as_mstring());
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

	wszValue = jnItem["mobile_phone"].as_mstring();
	if (!wszValue.IsEmpty())
		setWString(hContact, "Cellular", wszValue);

	wszValue = jnItem["home_phone"].as_mstring();
	if (!wszValue.IsEmpty())
		setWString(hContact, "Phone", wszValue);

	wszValue = jnItem["status"].as_mstring();
	db_set_ws(hContact, hContact ? "CList" : m_szModuleName, "StatusMsg", wszValue);

	CMStringW wszOldListeningTo(ptrW(db_get_wsa(hContact, m_szModuleName, "ListeningTo")));
	const JSONNode &jnAudio = jnItem["status_audio"];
	if (jnAudio) {
		CMStringW wszListeningTo(FORMAT, L"%s - %s", jnAudio["artist"].as_mstring().c_str(), jnAudio["title"].as_mstring().c_str());
		if (wszListeningTo != wszOldListeningTo) {
			setWString(hContact, "ListeningTo", wszListeningTo);
			setWString(hContact, "AudioUrl", jnAudio["url"].as_mstring());

			if (m_vkOptions.bPopupContactsMusic && getBool(hContact, "FloodListingToPopups", true)) {
				CMStringW wszTitle(FORMAT, TranslateT("%s is listening to"), wszNick.c_str());
				MsgPopup(hContact, wszListeningTo, wszTitle);
			}
		}
	}
	else if (wszValue[0] == wchar_t(9835) && wszValue.GetLength() > 2) {
		setWString(hContact, "ListeningTo", &(wszValue.GetBuffer())[2]);
		db_unset(hContact, m_szModuleName, "AudioUrl");
	}
	else {
		db_unset(hContact, m_szModuleName, "ListeningTo");
		db_unset(hContact, m_szModuleName, "AudioUrl");
	}

	const JSONNode &jnCountry = jnItem["country"];
	if (jnCountry) {
		wszValue = jnCountry["title"].as_mstring();
		if (!wszValue.IsEmpty())
			setWString(hContact, "Country", wszValue);
	}

	const JSONNode &jnCity = jnItem["city"];
	if (jnCity) {
		wszValue = jnCity["title"].as_mstring();
		if (!wszValue.IsEmpty())
			setWString(hContact, "City", wszValue);
	}

	// MaritalStatus
	uint8_t cMaritalStatus[] = { 0, 10, 11, 12, 20, 70, 50, 60, 80 };

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

	for (auto &it : vkInteres) {
		wszValue = jnItem[it.szField].as_mstring();
		if (wszValue.IsEmpty())
			continue;

		CMStringA InteresCat(FORMAT, "Interest%dCat", iInteres);
		CMStringA InteresText(FORMAT, "Interest%dText", iInteres);

		setWString(hContact, InteresCat, it.pwszTranslate);
		setWString(hContact, InteresText, wszValue);

		iInteres++;

	}

	for (int i = iInteres; iInteres > 0; i++) {
		CMStringA InteresCat(FORMAT, "Interest%dCat", iInteres);
		ptrW pwszCat(db_get_wsa(hContact, m_szModuleName, InteresCat));
		if (!pwszCat)
			break;
		db_unset(hContact, m_szModuleName, InteresCat);

		CMStringA InteresText(FORMAT, "Interest%dText", iInteres);
		ptrW pwszText(db_get_wsa(hContact, m_szModuleName, InteresText));
		if (!pwszText)
			break;
		db_unset(hContact, m_szModuleName, InteresText);
	}

	wszValue = jnItem["about"].as_mstring();
	if (!wszValue.IsEmpty())
		setWString(hContact, "About", wszValue);

	wszValue = jnItem["domain"].as_mstring();
	if (!wszValue.IsEmpty()) {
		setWString(hContact, "domain", wszValue);
		CMStringW wszUrl("https://vk.com/");
		wszUrl.Append(wszValue);
		setWString(hContact, "Homepage", wszUrl);
	}

	return hContact;
}

void CVkProto::RetrieveUserInfo(LONG userID)
{
	debugLogA("CVkProto::RetrieveUserInfo (%d)", userID);
	if (userID == VK_FEED_USER || !IsOnline())
		return;

	if (userID < 0) {
		RetrieveGroupInfo(userID);
		return;
	}

	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.RetrieveUserInfo", true, &CVkProto::OnReceiveUserInfo)
		<< INT_PARAM("userid", userID)
		<< CHAR_PARAM("fields", szFieldsName)
	);

}

void CVkProto::RetrieveGroupInfo(LONG groupID)
{
	debugLogA("CVkProto::RetrieveGroupInfo (%d)", groupID);
	if (groupID >= VK_INVALID_USER || !IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/groups.getById.json", true, &CVkProto::OnReceiveGroupInfo)
		<< CHAR_PARAM("fields", "description")
		<< INT_PARAM("group_id", -1 * groupID));
}

void CVkProto::RetrieveGroupInfo(CMStringA& groupIDs)
{
	debugLogA("CVkProto::RetrieveGroupInfo (%s)", groupIDs.c_str());
	if (groupIDs.IsEmpty() || !IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/groups.getById.json", true, &CVkProto::OnReceiveGroupInfo)
		<< CHAR_PARAM("fields", "description,status")
		<< CHAR_PARAM("group_ids", groupIDs));
}

void CVkProto::RetrieveUsersInfo(bool bFreeOffline, bool bRepeat)
{
	debugLogA("CVkProto::RetrieveUsersInfo");
	if (!IsOnline())
		return;

	CMStringA userIDs;
	int i = 0;
	for (auto &hContact : AccContacts()) {
		LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
		if (userID == VK_INVALID_USER || userID == VK_FEED_USER || userID < 0)
			continue;

		bool bIsFriend = !getBool(hContact, "Auth", true);
		if (bFreeOffline && !m_vkOptions.bLoadFullCList && bIsFriend)
			continue;

		if (!userIDs.IsEmpty())
			userIDs.AppendChar(',');
		userIDs.AppendFormat("%i", userID);

		if (i == MAX_CONTACTS_PER_REQUEST)
			break;
		i++;
	}

	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.RetrieveUsersInfo", true, &CVkProto::OnReceiveUserInfo)
		<< CHAR_PARAM("userids", userIDs)
		<< CHAR_PARAM("fields", (bFreeOffline ? "online,status" : szFieldsName))
		<< INT_PARAM("norepeat", (int)bRepeat)
		<< INT_PARAM("setonline", (int)m_bNeedSendOnline)
		<< INT_PARAM("func_v", (bFreeOffline && !m_vkOptions.bLoadFullCList) ? 1 : 2)
	);

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

	LIST<void> arContacts(10, PtrKeySortT);

	for (auto &hContact : AccContacts())
		if (!isChatRoom(hContact) && !IsGroupUser(hContact))
			arContacts.insert((HANDLE)hContact);

	for (auto &it : jnUsers) {
		MCONTACT hContact = SetContactInfo(it);
		if (hContact)
			arContacts.remove((HANDLE)hContact);
	}

	if (jnResponse["freeoffline"].as_bool())
		for (auto &it : arContacts) {
			MCONTACT cc = (UINT_PTR)it;
			LONG userID = getDword(cc, "ID", VK_INVALID_USER);
			if (userID == m_myUserId || userID == VK_FEED_USER)
				continue;

			int iContactStatus = getWord(cc, "Status", ID_STATUS_OFFLINE);

			if ((iContactStatus == ID_STATUS_ONLINE)
				|| (iContactStatus == ID_STATUS_INVISIBLE && time(0) - getDword(cc, "InvisibleTS", 0) >= m_vkOptions.iInvisibleInterval * 60LL)) {
				setWord(cc, "Status", ID_STATUS_OFFLINE);
				SetMirVer(cc, -1);
				db_unset(cc, m_szModuleName, "ListeningTo");
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
	for (auto it : jnItems) {
		LONG userid = it.as_int();
		if (userid == 0)
			break;

		MCONTACT hContact = FindUser(userid, true);
		if (!IsAuthContactLater(hContact)) {
			RetrieveUserInfo(userid);
			AddAuthContactLater(hContact);
			CVkDBAddAuthRequestThreadParam *param = new CVkDBAddAuthRequestThreadParam(hContact, false);
			ForkThread(&CVkProto::DBAddAuthRequestThread, (void *)param);
		}
	}
}

void CVkProto::OnReceiveGroupInfo(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUserInfo %d", reply->resultCode);

	if (reply->resultCode != 200 || !IsOnline())
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	for (auto &jnItem : jnResponse) {
		int iGroupId = (-1)*jnItem["id"].as_int();
		bool bIsMember = jnItem["is_member"].as_bool();

		MCONTACT hContact = FindUser(iGroupId, true);
		if (!hContact)
			continue;

		CMStringW wszValue;

		wszValue = jnItem["name"].as_mstring();
		if (!wszValue.IsEmpty())
			setWString(hContact, "Nick", wszValue);

		setWord(hContact, "Status", ID_STATUS_ONLINE);

		setByte(hContact, "Auth", !bIsMember);
		setByte(hContact, "friend", bIsMember);
		setByte(hContact, "IsGroup", 1);

		wszValue = jnItem["screen_name"].as_mstring();
		if (!wszValue.IsEmpty()) {
			setWString(hContact, "domain", wszValue);
			wszValue = L"https://vk.com/" + wszValue;
			setWString(hContact, "Homepage", wszValue);
		}

		wszValue = jnItem["description"].as_mstring();
		if (!wszValue.IsEmpty())
			setWString(hContact, "About", wszValue);

		wszValue = jnItem["photo_100"].as_mstring();
		if (!wszValue.IsEmpty()) {
			SetAvatarUrl(hContact, wszValue);
			ReloadAvatarInfo(hContact);
		}

		wszValue = jnItem["status"].as_mstring();
		db_set_ws(hContact, "CList", "StatusMsg", wszValue);

		CMStringW wszOldListeningTo(ptrW(db_get_wsa(hContact, m_szModuleName, "ListeningTo")));
		const JSONNode &jnAudio = jnItem["status_audio"];
		if (jnAudio) {
			CMStringW wszListeningTo(FORMAT, L"%s - %s", jnAudio["artist"].as_mstring().c_str(), jnAudio["title"].as_mstring().c_str());
			if (wszListeningTo != wszOldListeningTo) {
				setWString(hContact, "ListeningTo", wszListeningTo);
				setWString(hContact, "AudioUrl", jnAudio["url"].as_mstring());
			}
		}
		else if (wszValue[0] == wchar_t(9835) && wszValue.GetLength() > 2) {
			setWString(hContact, "ListeningTo", &(wszValue.GetBuffer())[2]);
			db_unset(hContact, m_szModuleName, "AudioUrl");
		}
		else {
			db_unset(hContact, m_szModuleName, "ListeningTo");
			db_unset(hContact, m_szModuleName, "AudioUrl");
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
		<< CHAR_PARAM("fields", szFieldsName))->pUserInfo = new CVkSendMsgParam(0, bCleanNonFriendContacts ? 1 : 0);
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
	bool bCleanContacts = (param->iMsgID != 0);
	delete param;

	LIST<void> arContacts(10, PtrKeySortT);

	for (auto &hContact : AccContacts()) {
		if (!isChatRoom(hContact) && !IsGroupUser(hContact))
			setByte(hContact, "Auth", 1);
		SetMirVer(hContact, -1);
		if (bCleanContacts && !isChatRoom(hContact))
			arContacts.insert((HANDLE)hContact);
	}

	const JSONNode &jnItems = jnResponse["items"];

	if (jnItems)
		for (auto &it : jnItems) {
			MCONTACT hContact = SetContactInfo(it, true);

			if (hContact == 0 || hContact == INVALID_CONTACT_ID)
				continue;

			arContacts.remove((HANDLE)hContact);
			setByte(hContact, "Auth", 0);
			db_unset(hContact, m_szModuleName, "ReqAuthTime");
		}

	if (bCleanContacts)
		for (auto &it : arContacts) {
			MCONTACT hContact = (UINT_PTR)it;
			LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
			bool bIsFriendGroup = IsGroupUser(hContact) && getBool(hContact, "friend");
			if (userID == m_myUserId || userID == VK_FEED_USER || bIsFriendGroup)
				continue;
			if (!IsAuthContactLater(hContact))
				DeleteContact(hContact);
		}

	arContacts.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR __cdecl CVkProto::SvcAddAsFriend(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcAddAsFriend");
	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (!IsOnline() || userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return 1;
	ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, (LPARAM)TranslateT("Please authorize me to add you to my friend list."));
	return 0;
}

INT_PTR CVkProto::SvcWipeNonFriendContacts(WPARAM, LPARAM)
{
	debugLogA("CVkProto::SvcWipeNonFriendContacts");
	if (IDNO == MessageBoxW(nullptr, TranslateT("Are you sure to wipe local contacts missing in your friend list?"),
		TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 0;

	RetrieveFriends(true);
	return 0;
}

INT_PTR __cdecl CVkProto::SvcDeleteFriend(WPARAM hContact, LPARAM flag)
{
	debugLogA("CVkProto::SvcDeleteFriend");
	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (!IsOnline() || userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return 1;

	if (flag == 0) {
		CMStringW pwszMsg;
		ptrW pwszNick(db_get_wsa(hContact, m_szModuleName, "Nick"));
		pwszMsg.AppendFormat(TranslateT("Are you sure to delete %s from your friend list?"), IsEmpty(pwszNick) ? TranslateT("(Unknown contact)") : pwszNick.get());
		if (IDNO == MessageBoxW(nullptr, pwszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
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
			CMStringW wszNick(db_get_wsm(param->hContact, m_szModuleName, "Nick"));
			if (wszNick.IsEmpty())
				wszNick = TranslateT("(Unknown contact)");
			CMStringW msgformat, msg;

			if (jnResponse["success"].as_bool()) {
				if (jnResponse["friend_deleted"].as_bool())
					msgformat = TranslateT("User %s was deleted from your friend list");
				else if (jnResponse["out_request_deleted"].as_bool())
					msgformat = TranslateT("Your request to the user %s was deleted");
				else if (jnResponse["in_request_deleted"].as_bool())
					msgformat = TranslateT("Friend request from the user %s declined");
				else if (jnResponse["suggestion_deleted"].as_bool())
					msgformat = TranslateT("Friend request suggestion for the user %s deleted");

				msg.AppendFormat(msgformat, wszNick.c_str());
				MsgPopup(param->hContact, msg, wszNick);
				setByte(param->hContact, "Auth", 1);
			}
			else {
				msg = TranslateT("User or request was not deleted");
				MsgPopup(param->hContact, msg, wszNick);
			}
		}
	}

	if (param && (!pReq->bNeedsRestart || m_bTerminated)) {
		delete param;
		pReq->pUserInfo = nullptr;
	}
}

INT_PTR __cdecl CVkProto::SvcBanUser(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcBanUser");
	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (!IsOnline() || userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return 1;

	CMStringA code(FORMAT, "var userID=\"%d\";API.account.banUser({\"user_id\":userID});", userID);
	CMStringW wszVarWarning;

	if (m_vkOptions.bReportAbuse) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bReportAbuse = true");
		code += "API.users.report({\"user_id\":userID,type:\"spam\"});";
		wszVarWarning = TranslateT(" report abuse on him/her");
	}
	if (m_vkOptions.bClearServerHistory) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bClearServerHistory = true");
		code += "API.messages.deleteConversation({\"peer_id\":userID});";
		if (!wszVarWarning.IsEmpty())
			wszVarWarning.AppendChar(L',');
		wszVarWarning += TranslateT(" clear server history with him/her");
	}
	if (m_vkOptions.bRemoveFromFrendlist) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bRemoveFromFrendlist = true");
		code += "API.friends.delete({\"user_id\":userID});";
		if (!wszVarWarning.IsEmpty())
			wszVarWarning.AppendChar(L',');
		wszVarWarning += TranslateT(" remove him/her from your friend list");
	}
	if (m_vkOptions.bRemoveFromCList) {
		debugLogA("CVkProto::SvcBanUser m_vkOptions.bRemoveFromClist = true");
		if (!wszVarWarning.IsEmpty())
			wszVarWarning.AppendChar(L',');
		wszVarWarning += TranslateT(" remove him/her from your contact list");
	}

	if (!wszVarWarning.IsEmpty())
		wszVarWarning += ".\n";
	code += "return 1;";

	ptrW pwszNick(db_get_wsa(hContact, m_szModuleName, "Nick"));
	CMStringW pwszMsg(FORMAT, TranslateT("Are you sure to ban %s? %s%sContinue?"),
		IsEmpty(pwszNick) ? TranslateT("(Unknown contact)") : pwszNick,
		wszVarWarning.IsEmpty() ? L" " : TranslateT("\nIt will also"),
		wszVarWarning.IsEmpty() ? L"\n" : wszVarWarning);

	if (IDNO == MessageBoxW(nullptr, pwszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/execute.json", true, &CVkProto::OnReceiveSmth)
		<< CHAR_PARAM("code", code.c_str()));

	if (m_vkOptions.bRemoveFromCList)
		DeleteContact(hContact);

	return 0;
}

INT_PTR __cdecl CVkProto::SvcReportAbuse(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcReportAbuse");
	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (!IsOnline() || userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return 1;

	CMStringW wszNick(db_get_wsm(hContact, m_szModuleName, "Nick")),
		pwszMsg(FORMAT, TranslateT("Are you sure to report abuse on %s?"), wszNick.IsEmpty() ? TranslateT("(Unknown contact)") : wszNick);
	if (IDNO == MessageBoxW(nullptr, pwszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.report.json", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("user_id", userID)
		<< CHAR_PARAM("type", "spam"));

	return 0;
}

INT_PTR __cdecl CVkProto::SvcOpenBroadcast(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcOpenBroadcast");

	CMStringW wszAudio(db_get_wsm(hContact, m_szModuleName, "AudioUrl"));
	if (!wszAudio.IsEmpty())
		Utils_OpenUrlW(wszAudio);

	return 0;
}

INT_PTR __cdecl CVkProto::SvcVisitProfile(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcVisitProfile");
	if (isChatRoom(hContact)) {
		ptrW wszHomepage(db_get_wsa(hContact, m_szModuleName, "Homepage"));
		if (!IsEmpty(wszHomepage))
			Utils_OpenUrlW(wszHomepage);
		return 0;
	}

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	ptrW wszDomain(db_get_wsa(hContact, m_szModuleName, "domain"));

	CMStringW wszUrl("https://vk.com/");
	if (wszDomain)
		wszUrl.Append(wszDomain);
	else
		wszUrl.AppendFormat(L"id%i", userID);

	Utils_OpenUrlW(wszUrl);
	return 0;
}