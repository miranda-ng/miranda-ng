/*
Copyright (c) 2013-25 Miranda NG team (https://miranda-ng.org)

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

UINT_PTR CVkProto::m_Timer;
mir_cs CVkProto::m_csTimer;

char szBlankUrl[] = "https://oauth.vk.com/blank.html";
char szScore[] = "friends,photos,audio,docs,video,wall,messages,offline,status,notifications,groups";
char szVKTokenBeg[] = "access_token=";
char szVKCookieDomain[] = ".vk.com";



static char szFieldsName[] = "id, first_name, last_name, photo_100, bdate, sex, timezone, "
	"contacts, last_seen, online, status, country, city, relation, interests, activities, "
	"music, movies, tv, books, games, quotes, about,  domain, can_write_private_message";

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
		SetEvent(m_hEvRequestsQueue);
	OnLoggedOut();
}

void CVkProto::ConnectionFailed(int iReason)
{
	if (iReason == LOGINERR_WRONGPASSWORD) 
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
	if (CVkProto::m_Timer)
		return;
	CVkProto::m_Timer = SetTimer(nullptr, 0, 60000, TimerProc);
}

static void CALLBACK VKUnsetTimer(void*)
{
	mir_cslock lck(CVkProto::m_csTimer);
	if (CVkProto::m_Timer)
		KillTimer(nullptr, CVkProto::m_Timer);
	CVkProto::m_Timer = 0;
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
	if (!m_hPollingConn)
		return;

	mir_cslock lck(m_csPoolingConnection);

	debugLogA("CVkProto::ClosePollingConnection %d", bShutdown ? 1 : 0);

	if (bShutdown)
		Netlib_Shutdown(m_hPollingConn);
	else
		Netlib_CloseHandle(m_hPollingConn);
	m_hPollingConn = nullptr;
}

void CVkProto::CloseAPIConnection(bool bShutdown)
{
	if (!m_hAPIConnection)
		return;
	
	mir_cslock lck(m_csAPIConnection);

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

bool CVkProto::LoadToken(LPCSTR pszUrlSring)
{
	LPCSTR p = strstr(pszUrlSring, szVKTokenBeg);
	if (!p) {
		debugLogA("CVkProto::LoadToken error");
		return false;
	}
	
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
	setDword("LastAccessTokenTime", time(0));
	debugLogA("CVkProto::LoadToken OK");
	return true;
}

void CVkProto::LogIn(LPCSTR pszUrl)
{
	debugLogA("CVkProto::LogIn %s", pszUrl ? pszUrl : " ");
	CMStringA szTokenReq(
		FORMAT,
		"https://oauth.vk.com/authorize?client_id=%d&scope=%s&redirect_uri=%s&display=mobile&response_type=token&v=%s", 
		VK_APP_ID,
		mir_urlEncode(szScore).c_str(),
		mir_urlEncode(szBlankUrl).c_str(),
		VER_API
	);

	CVkTokenForm dlg(this, pszUrl ? pszUrl : szTokenReq.c_str());
	if (dlg.DoModal() && LoadToken(dlg.Result))
		RetrieveMyInfo();
	else
		ConnectionFailed(LOGINERR_NOSERVER);
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

void CVkProto::OnReceiveMyInfo(MHttpResponse *reply, AsyncHttpRequest *pReq)
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

	m_iMyUserId = jnUser["id"].as_int();
	
	WriteVKUserID(0, m_iMyUserId);

	OnLoggedIn();
	RetrieveUserInfo(m_iMyUserId);
	TrackVisitor();
	RetrieveUnreadMessages();
	RetrieveFriends(m_vkOptions.bLoadOnlyFriends);
	RetrievePollingInfo();
}

MCONTACT CVkProto::SetContactInfo(const JSONNode &jnItem, bool bFlag, VKContactType vkContactType)
{
	if (!jnItem) {
		debugLogA("CVkProto::SetContactInfo pItem == nullptr");
		return INVALID_CONTACT_ID;
	}

	VKUserID_t iUserId = jnItem["id"].as_int();
	debugLogA("CVkProto::SetContactInfo %d", iUserId);
	if (iUserId == 0 || iUserId == VK_FEED_USER)
		return 0;

	MCONTACT hContact = FindUser(iUserId, bFlag);

	if (iUserId == m_iMyUserId) {
		if (hContact != 0)
			if (vkContactType == VKContactType::vkContactSelf)
				hContact = 0;
			else
				SetContactInfo(jnItem, bFlag, VKContactType::vkContactSelf);
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

	if (!wszValue.IsEmpty()) {
		Contact::Readonly(hContact);
		return hContact;
	}

	int sex = jnItem["sex"].as_int();
	if (sex)
		setByte(hContact, "Gender", sex == 2 ? 'M' : 'F');

	wszValue = jnItem["bdate"].as_mstring();
	if (!wszValue.IsEmpty()) {
		int d, m, y, iReadCount;
		iReadCount = swscanf(wszValue, L"%d.%d.%d", &d, &m, &y);
		if (iReadCount > 1)
			Contact::SetBirthday(hContact, d, m, (iReadCount == 3) ? y : 0);
	}

	wszValue = jnItem["photo_100"].as_mstring();
	if (!wszValue.IsEmpty()) {
		SetAvatarUrl(hContact, wszValue);
		ReloadAvatarInfo(hContact);
	}

	const JSONNode &jnLastSeen = jnItem["last_seen"];
	if (jnLastSeen) {
		time_t tLastSeen = jnLastSeen["time"].as_int();
		int iOldLastSeen = db_get_dw(hContact, "BuddyExpectator", "LastSeen");
		if (tLastSeen && tLastSeen > iOldLastSeen) {
			db_set_dw(hContact, "BuddyExpectator", "LastSeen", (uint32_t)tLastSeen);
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

	if (jnItem["can_write_private_message"])
		Contact::Readonly(hContact, jnItem["can_write_private_message"].as_int() == 0);

	return hContact;
}

void CVkProto::RetrieveUserInfo(VKUserID_t iUserId)
{
	debugLogA("CVkProto::RetrieveUserInfo (%d)", iUserId);
	if (iUserId == VK_FEED_USER || !IsOnline())
		return;

	if (iUserId < 0) {
		RetrieveGroupInfo(iUserId);
		return;
	}

	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.RetrieveUserInfo", true, &CVkProto::OnReceiveUserInfo)
		<< INT_PARAM("userid", iUserId)
		<< CHAR_PARAM("fields", szFieldsName)
	);

}

void CVkProto::RetrieveGroupInfo(VKUserID_t iGroupId)
{
	debugLogA("CVkProto::RetrieveGroupInfo (%d)", iGroupId);
	if (GetVKPeerType(iGroupId) != VKPeerType::vkPeerGroup || !IsOnline())
		return;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/groups.getById.json", true, &CVkProto::OnReceiveGroupInfo)
		<< CHAR_PARAM("fields", "description")
		<< INT_PARAM("group_ids", -1 * iGroupId));
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
		VKUserID_t iUserId = ReadVKUserID(hContact);
		if (iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER || iUserId < 0 || isChatRoom(hContact))
			continue;

		bool bIsFriend = !getBool(hContact, "Auth", true);
		if (bFreeOffline && !m_vkOptions.bLoadFullCList && bIsFriend)
			continue;

		if (!userIDs.IsEmpty())
			userIDs.AppendChar(',');
		userIDs.AppendFormat("%i", iUserId);

		if (i == MAX_CONTACTS_PER_REQUEST)
			break;
		i++;
	}

	Push(new AsyncHttpRequest(this, REQUEST_POST, "/method/execute.RetrieveUsersInfo", true, &CVkProto::OnReceiveUserInfo)
		<< CHAR_PARAM("userids", userIDs)
		<< CHAR_PARAM("fields", (bFreeOffline ? "online,status,can_write_private_message" : szFieldsName))
		<< INT_PARAM("norepeat", (int)bRepeat)
		<< INT_PARAM("setonline", (int)m_bNeedSendOnline)
		<< INT_PARAM("func_v", (bFreeOffline && !m_vkOptions.bLoadFullCList) ? 1 : 2)
	);

}

void CVkProto::OnReceiveUserInfo(MHttpResponse *reply, AsyncHttpRequest *pReq)
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
			VKUserID_t iUserId = ReadVKUserID(cc);
			if (iUserId == m_iMyUserId || iUserId == VK_FEED_USER)
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
		VKUserID_t iUserId = it.as_int();
		if (iUserId == 0)
			break;

		MCONTACT hContact = FindUser(iUserId, true);
		if (!IsAuthContactLater(hContact)) {
			RetrieveUserInfo(iUserId);
			AddAuthContactLater(hContact);
			CVkDBAddAuthRequestThreadParam *param = new CVkDBAddAuthRequestThreadParam(hContact, false);
			ForkThread(&CVkProto::DBAddAuthRequestThread, (void *)param);
		}
	}
}

void CVkProto::OnReceiveGroupInfo(MHttpResponse *reply, AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::OnReceiveUserInfo %d", reply->resultCode);

	if (reply->resultCode != 200 || !IsOnline())
		return;

	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	if (!jnResponse)
		return;

	const JSONNode& jnGroups = jnResponse["groups"];
	if (!jnGroups)
		return;

	for (auto &jnItem : jnGroups) {
		VKUserID_t iGroupId = (-1)*jnItem["id"].as_int();
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

void CVkProto::OnReceiveFriends(MHttpResponse *reply, AsyncHttpRequest *pReq)
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
			VKUserID_t iUserId = ReadVKUserID(hContact);
			bool bIsFriendGroup = IsGroupUser(hContact) && getBool(hContact, "friend");
			if (iUserId == m_iMyUserId || iUserId == VK_FEED_USER || bIsFriendGroup)
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
	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (!IsOnline() || iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER)
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
	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (!IsOnline() || iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER)
		return 1;

	if (flag == 0) {
		CMStringW pwszMsg;
		ptrW pwszNick(db_get_wsa(hContact, m_szModuleName, "Nick"));
		pwszMsg.AppendFormat(TranslateT("Are you sure to delete %s from your friend list?"), IsEmpty(pwszNick) ? TranslateT("(Unknown contact)") : pwszNick.get());
		if (IDNO == MessageBoxW(nullptr, pwszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
			return 1;
	}
	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/friends.delete.json", true, &CVkProto::OnReceiveDeleteFriend)
		<< INT_PARAM("user_id", iUserId))->pUserInfo = new CVkSendMsgParam(hContact);

	return 0;
}

void CVkProto::OnReceiveDeleteFriend(MHttpResponse *reply, AsyncHttpRequest *pReq)
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
			CMStringW wszMsgFormat, wszMsg;

			if (jnResponse["success"].as_bool()) {
				if (jnResponse["friend_deleted"].as_bool())
					wszMsgFormat = TranslateT("User %s was deleted from your friend list");
				else if (jnResponse["out_request_deleted"].as_bool())
					wszMsgFormat = TranslateT("Your request to the user %s was deleted");
				else if (jnResponse["in_request_deleted"].as_bool())
					wszMsgFormat = TranslateT("Friend request from the user %s declined");
				else if (jnResponse["suggestion_deleted"].as_bool())
					wszMsgFormat = TranslateT("Friend request suggestion for the user %s deleted");

				wszMsg.AppendFormat(wszMsgFormat, wszNick.c_str());
				MsgPopup(param->hContact, wszMsg, wszNick);
				setByte(param->hContact, "Auth", 1);
			}
			else {
				wszMsg = TranslateT("User or request was not deleted");
				MsgPopup(param->hContact, wszMsg, wszNick);
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
	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (!IsOnline() || iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER)
		return 1;

	CMStringA code(FORMAT, "var userID=\"%d\";API.account.banUser({\"user_id\":userID});", iUserId);
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
	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (!IsOnline() || iUserId == VK_INVALID_USER || iUserId == VK_FEED_USER)
		return 1;

	CMStringW wszNick(db_get_wsm(hContact, m_szModuleName, "Nick")),
		pwszMsg(FORMAT, TranslateT("Are you sure to report abuse on %s?"), wszNick.IsEmpty() ? TranslateT("(Unknown contact)") : wszNick);
	if (IDNO == MessageBoxW(nullptr, pwszMsg, TranslateT("Attention!"), MB_ICONWARNING | MB_YESNO))
		return 1;

	Push(new AsyncHttpRequest(this, REQUEST_GET, "/method/users.report.json", true, &CVkProto::OnReceiveSmth)
		<< INT_PARAM("user_id", iUserId)
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

	VKUserID_t iUserId = ReadVKUserID(hContact);
	
	ptrW wszDomain(db_get_wsa(hContact, m_szModuleName, "domain"));

	CMStringW wszUrl("https://vk.com/");
	if (wszDomain)
		wszUrl.Append(wszDomain);
	else {
		bool b_isGroupUser = IsGroupUser(hContact);
		wszUrl.AppendFormat(b_isGroupUser ? L"club%i" : L"id%i", b_isGroupUser ? -1* iUserId : iUserId);
	}

	Utils_OpenUrlW(wszUrl);
	return 0;
}

INT_PTR __cdecl CVkProto::SvcGoToSiteIM(WPARAM hContact, LPARAM)
{
	debugLogA("CVkProto::SvcGoToSiteIM");

	VKUserID_t iUserId = ReadVKUserID(hContact);
	if (isChatRoom(hContact))
		iUserId += VK_CHAT_MIN;
	CMStringW wszUrl(FORMAT, L"https://vk.com/im/convo/%d?entrypoint=list_all", iUserId);

	Utils_OpenUrlW(wszUrl);
	return 0;
}