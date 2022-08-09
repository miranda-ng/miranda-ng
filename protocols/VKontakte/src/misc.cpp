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

static const char  *szGiftTypes[] = { "thumb_256", "thumb_96", "thumb_48" };
static const char *szVKUrls[] = { "http://vk.com/", "https://vk.com/", "http://new.vk.com/", "https://new.vk.com/", "http://m.vk.com/", "https://m.vk.com/" };
static const char *szAttachmentMasks[] = { "wall%d_%d",  "video%d_%d",  "photo%d_%d", "audio%d_%d", "doc%d_%d", "market%d_%d", "story%d_%d" };
static const char *szVKLinkParam[] = { "?z=", "?w=", "&z=", "&w=" };

JSONNode nullNode(JSON_NULL);

bool IsEmpty(LPCWSTR str)
{
	return (str == nullptr || str[0] == 0);
}

bool IsEmpty(LPCSTR str)
{
	return (str == nullptr || str[0] == 0);
}

bool wlstrstr(wchar_t *_s1, wchar_t *_s2)
{
	wchar_t s1[1024], s2[1024];

	wcsncpy_s(s1, _s1, _TRUNCATE);
	CharLowerBuff(s1, _countof(s1));
	wcsncpy_s(s2, _s2, _TRUNCATE);
	CharLowerBuff(s2, _countof(s2));

	return wcsstr(s1, s2) != nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Notification icon"), "notification", IDI_NOTIFICATION },
	{ LPGEN("Read message icon"), "read", IDI_READMSG },
	{ LPGEN("Visit profile icon"), "profile", IDI_VISITPROFILE },
	{ LPGEN("Load server history icon"), "history", IDI_HISTORY },
	{ LPGEN("Add to friend list icon"), "addfriend", IDI_FRIENDADD },
	{ LPGEN("Delete from friend list icon"), "delfriend", IDI_FRIENDDEL },
	{ LPGEN("Report abuse icon"), "abuse", IDI_ABUSE },
	{ LPGEN("Ban user icon"), "ban", IDI_BAN },
	{ LPGEN("Broadcast icon"), "broadcast", IDI_BROADCAST },
	{ LPGEN("Status icon"), "status", IDI_STATUS },
	{ LPGEN("Wall message icon"), "wall", IDI_WALL },
	{ LPGEN("Mark messages as read icon"), "markread", IDI_MARKMESSAGESASREAD }
};

void InitIcons()
{
	g_plugin.registerIcon(LPGEN("Protocols") "/" LPGEN("VKontakte"), iconList, "VKontakte");
}

/////////////////////////////////////////////////////////////////////////////////////////

char* ExpUrlEncode(const char *szUrl, bool strict)
{
	const char szHexDigits[] = "0123456789ABCDEF";

	if (szUrl == nullptr)
		return nullptr;

	const uint8_t *s;
	int outputLen;
	for (outputLen = 0, s = (const uint8_t*)szUrl; *s; s++)
		if ((*s & 0x80 && !strict) || // UTF-8 multibyte
			('0' <= *s && *s <= '9') || //0-9
			('A' <= *s && *s <= 'Z') || //ABC...XYZ
			('a' <= *s && *s <= 'z') || //abc...xyz
			*s == '~' || *s == '-' || *s == '_' || *s == '.' || *s == ' ')
			outputLen++;
		else
			outputLen += 3;

	char *szOutput = (char*)mir_alloc(outputLen + 1);
	if (szOutput == nullptr)
		return nullptr;

	char *d = szOutput;
	for (s = (const uint8_t*)szUrl; *s; s++)
		if ((*s & 0x80 && !strict) || // UTF-8 multibyte
			('0' <= *s && *s <= '9') || //0-9
			('A' <= *s && *s <= 'Z') || //ABC...XYZ
			('a' <= *s && *s <= 'z') || //abc...xyz
			*s == '~' || *s == '-' || *s == '_' || *s == '.')
			*d++ = *s;
		else if (*s == ' ')
			*d++ = '+';
		else {
			*d++ = '%';
			*d++ = szHexDigits[*s >> 4];
			*d++ = szHexDigits[*s & 0xF];
		}

		*d = '\0';
		return szOutput;
}


/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ClearAccessToken()
{
	debugLogA("CVkProto::ClearAccessToken");
	setDword("LastAccessTokenTime", (uint32_t)time(0));
	m_szAccessToken = nullptr;
	delSetting("AccessToken");
	ShutdownSession();
}

wchar_t* CVkProto::GetUserStoredPassword()
{
	debugLogA("CVkProto::GetUserStoredPassword");
	ptrA szRawPass(getStringA("Password"));
	return (szRawPass != nullptr) ? mir_utf8decodeW(szRawPass) : nullptr;
}

void CVkProto::SetAllContactStatuses(int iStatus)
{
	debugLogA("CVkProto::SetAllContactStatuses (%d)", iStatus);
	for (auto &hContact : AccContacts()) {
		if (isChatRoom(hContact))
			SetChatStatus(hContact, iStatus);
		else if (getWord(hContact, "Status") != iStatus)
			setWord(hContact, "Status", iStatus);

		if (iStatus == ID_STATUS_OFFLINE) {
			SetMirVer(hContact, -1);
			db_unset(hContact, m_szModuleName, "ListeningTo");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MCONTACT CVkProto::FindUser(LONG dwUserid, bool bCreate)
{
	if (!dwUserid)
		return 0;

	for (auto &hContact : AccContacts()) {
		LONG dbUserid = getDword(hContact, "ID", VK_INVALID_USER);
		if (dbUserid == VK_INVALID_USER)
			continue;

		if (dbUserid == dwUserid)
			return hContact;
	}

	if (!bCreate)
		return 0;

	MCONTACT hNewContact = db_add_contact();
	Proto_AddToContact(hNewContact, m_szModuleName);
	setDword(hNewContact, "ID", dwUserid);
	Clist_SetGroup(hNewContact, m_vkOptions.pwszDefaultGroup);
	if (dwUserid < 0)
		setByte(hNewContact, "IsGroup", 1);
	return hNewContact;
}

MCONTACT CVkProto::FindChat(LONG dwUserid)
{
	if (!dwUserid)
		return 0;

	for (auto &hContact : AccContacts()) {
		LONG dbUserid = getDword(hContact, "vk_chat_id", VK_INVALID_USER);
		if (dbUserid == VK_INVALID_USER)
			continue;

		if (dbUserid == dwUserid)
			return hContact;
	}

	return 0;
}

bool CVkProto::IsGroupUser(MCONTACT hContact)
{
	if (getBool(hContact, "IsGroup", false))
		return true;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	return (userID < 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::CheckMid(LIST<void> &lList, int guid)
{
	for (auto &it : lList)
		if ((INT_PTR)it == guid) {
			lList.removeItem(&it);
			return true;
		}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

JSONNode& CVkProto::CheckJsonResponse(AsyncHttpRequest *pReq, NETLIBHTTPREQUEST *reply, JSONNode &root)
{
	debugLogA("CVkProto::CheckJsonResponse");

	if (!reply || !reply->pData)
		return nullNode;

	root = JSONNode::parse(reply->pData);

	if (!CheckJsonResult(pReq, root))
		return nullNode;

	return root["response"];
}

bool CVkProto::CheckJsonResult(AsyncHttpRequest *pReq, const JSONNode &jnNode)
{
	debugLogA("CVkProto::CheckJsonResult");

	if (!jnNode) {
		if (pReq)
			pReq->m_iErrorCode = VKERR_NO_JSONNODE;
		return false;
	}

	const JSONNode &jnError = jnNode["error"];
	const JSONNode &jnErrorCode = jnError["error_code"];
	const JSONNode &jnRedirectUri = jnError["redirect_uri"];

	if (!jnError || !jnErrorCode)
		return true;

	int iErrorCode = jnErrorCode.as_int();
	debugLogA("CVkProto::CheckJsonResult %d", iErrorCode);
	if (!pReq)
		return (iErrorCode == 0);

	pReq->m_iErrorCode = iErrorCode;
	switch (iErrorCode) {
	case VKERR_AUTHORIZATION_FAILED:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		break;
	case VKERR_ACCESS_DENIED:
		if ((jnError["error_msg"] && jnError["error_msg"].as_mstring() == L"Access denied: can't set typing activity for this peer")
			|| (pReq->m_szUrl.Find("messages.setActivity.json") > -1)
		) {
			debugLogA("CVkProto::CheckJsonResult VKERR_ACCESS_DENIED (can't set typing activity) - ignore");
			break;
		}

		if (time(0) - getDword("LastAccessTokenTime", 0) > 60 * 60 * 24) {
			debugLogA("CVkProto::CheckJsonResult VKERR_ACCESS_DENIED (AccessToken fail?)");
			ClearAccessToken();
			return false;
		}
		debugLogA("CVkProto::CheckJsonResult VKERR_ACCESS_DENIED");
		MsgPopup(TranslateT("Access denied! Data will not be sent or received."), TranslateT("Error"), true);
		break;
	case VKERR_CAPTCHA_NEEDED:
		ApplyCaptcha(pReq, jnError);
		break;
	case VKERR_VALIDATION_REQUIRED:	// Validation Required
		MsgPopup(TranslateT("You have to validate your account before you can use VK in Miranda NG"), TranslateT("Error"), true);
		if (jnRedirectUri) {
			T2Utf szRedirectUri(jnRedirectUri.as_mstring());
			AsyncHttpRequest *pRedirectReq = new AsyncHttpRequest(this, REQUEST_GET, szRedirectUri, false, &CVkProto::OnOAuthAuthorize);
			pRedirectReq->m_bApiReq = false;
			pRedirectReq->bIsMainConn = true;
			Push(pRedirectReq);
		}
		break;
	case VKERR_FLOOD_CONTROL:
		pReq->m_iRetry = 0;
		__fallthrough;

	case VKERR_UNKNOWN:
	case VKERR_TOO_MANY_REQ_PER_SEC:
	case VKERR_INTERNAL_SERVER_ERR:
		if (pReq->m_iRetry > 0) {
			pReq->bNeedsRestart = true;
			Sleep(1000); //Pause for fix err
			debugLogA("CVkProto::CheckJsonResult Retry = %d", pReq->m_iRetry);
			pReq->m_iRetry--;
		}
		else {
			CMStringW msg(FORMAT, TranslateT("Error %d. Data will not be sent or received."), iErrorCode);
			MsgPopup(msg, TranslateT("Error"), true);
			debugLogA("CVkProto::CheckJsonResult SendError");
		}
		break;

	case VKERR_INVALID_PARAMETERS:
		MsgPopup(TranslateT("One of the parameters specified was missing or invalid"), TranslateT("Error"), true);
		break;
	case VKERR_ACC_WALL_POST_DENIED:
		MsgPopup(TranslateT("Access to adding post denied"), TranslateT("Error"), true);
		break;
	case VKERR_CANT_SEND_USER_ON_BLACKLIST:
		MsgPopup(TranslateT("Can't send messages for users from blacklist"), TranslateT("Error"), true);
		break;
	case VKERR_CANT_SEND_USER_WITHOUT_DIALOGS:
		MsgPopup(TranslateT("Can't send messages for users without dialogs"), TranslateT("Error"), true);
		break;
	case VKERR_CANT_SEND_YOU_ON_BLACKLIST:
		MsgPopup(TranslateT("Can't send messages to this user due to their privacy settings"), TranslateT("Error"), true);
		break;
	case VKERR_MESSAGE_IS_TOO_LONG:
		MsgPopup(TranslateT("Message is too long"), TranslateT("Error"), true);
		break;
	case VKERR_COULD_NOT_SAVE_FILE:
	case VKERR_INVALID_ALBUM_ID:
	case VKERR_INVALID_SERVER:
	case VKERR_INVALID_HASH:
	case VKERR_INVALID_AUDIO:
	case VKERR_AUDIO_DEL_COPYRIGHT:
	case VKERR_INVALID_FILENAME:
	case VKERR_INVALID_FILESIZE:
	case VKERR_HIMSELF_AS_FRIEND:
	case VKERR_YOU_ON_BLACKLIST:
	case VKERR_USER_ON_BLACKLIST:
		break;
	// See also CVkProto::SendFileFiled
	}

	return (iErrorCode == 0);
}

void CVkProto::OnReceiveSmth(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	JSONNode jnRoot;
	const JSONNode &jnResponse = CheckJsonResponse(pReq, reply, jnRoot);
	debugLogA("CVkProto::OnReceiveSmth %d", jnResponse.as_int());
}

/////////////////////////////////////////////////////////////////////////////////////////
// Quick & dirty form parser

static CMStringA getAttr(char *szSrc, LPCSTR szAttrName)
{
	char *pEnd = strchr(szSrc, '>');
	if (pEnd == nullptr)
		return "";

	*pEnd = 0;

	char *p1 = strstr(szSrc, szAttrName);
	if (p1 == nullptr) {
		*pEnd = '>';
		return "";
	}

	p1 += mir_strlen(szAttrName);
	if (p1[0] != '=' || p1[1] != '\"') {
		*pEnd = '>';
		return "";
	}

	p1 += 2;
	char *p2 = strchr(p1, '\"');
	*pEnd = '>';
	if (p2 == nullptr)
		return "";

	return CMStringA(p1, (int)(p2 - p1));
}

bool CVkProto::AutoFillForm(char *pBody, CMStringA &szAction, CMStringA& szResult)
{
	debugLogA("CVkProto::AutoFillForm");
	szResult.Empty();

	char *pFormBeg = strstr(pBody, "<form ");
	if (pFormBeg == nullptr)
		return false;

	char *pFormEnd = strstr(pFormBeg, "</form>");
	if (pFormEnd == nullptr)
		return false;

	*pFormEnd = 0;

	szAction = getAttr(pFormBeg, "action");

	CMStringA result;
	char *pFieldBeg = pFormBeg;
	while (true) {
		if ((pFieldBeg = strstr(pFieldBeg + 1, "<input ")) == nullptr)
			break;

		CMStringA type = getAttr(pFieldBeg, "type");
		if (type != "submit") {
			CMStringA name = getAttr(pFieldBeg, "name");
			CMStringA value = getAttr(pFieldBeg, "value");
			if (name == "email")
				value = (char*)T2Utf(ptrW(getWStringA("Login")));
			else if (name == "pass")
				value = (char*)T2Utf(ptrW(GetUserStoredPassword()));
			else if (name == "captcha_key") {
				char *pCaptchaBeg = strstr(pFormBeg, "<img id=\"captcha\"");
				if (!pCaptchaBeg)
					pCaptchaBeg = strstr(pFormBeg, "<img src=\"/captcha.php");

				if (pCaptchaBeg)
					if (!RunCaptchaForm(getAttr(pCaptchaBeg, "src"), value))
						return false;
			}
			else if (name == "code") {
				char szPrefixTel[10], szSufixTel[10];
				CMStringW wszTitle;
				char *pPhonePref = strstr(pFormBeg, "<span class=\"field_prefix\">");
				if (pPhonePref && sscanf(pPhonePref, "<span class=\"field_prefix\">%[^<]", szPrefixTel) == 1) {
					pPhonePref = strstr(pPhonePref + 1, "<span class=\"field_prefix\">&nbsp;");
					if (pPhonePref && sscanf(pPhonePref, "<span class=\"field_prefix\">&nbsp;%[^<]", szSufixTel) == 1) {
						wszTitle.Format(TranslateT("Enter the missing digits between %s and %s of the phone number linked to your account"),
							ptrW(mir_a2u(szPrefixTel)).get(), ptrW(mir_a2u(szSufixTel)).get());
						MessageBoxW(nullptr, wszTitle, TranslateT("Attention!"), MB_ICONWARNING | MB_OK);
					}
				}

				value = RunConfirmationCode(wszTitle);
				if (value.IsEmpty())
					return false;
			}

			if (!result.IsEmpty())
				result.AppendChar('&');
			result += name + "=";
			result += mir_urlEncode(value);
		}
	}

	szResult = result;
	debugLogA("CVkProto::AutoFillForm result = \"%s\"", szResult.c_str());
	return true;
}

CMStringW CVkProto::RunConfirmationCode(LPCWSTR pwszTitle)
{
	ENTER_STRING pForm = {};
	pForm.type = 0;
	CMStringW wszTitle(FORMAT, L"%s: %s", m_tszUserName, IsEmpty(pwszTitle) ? TranslateT("Enter confirmation code") : pwszTitle);
	pForm.caption = wszTitle;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "confirmcode_";
	return (!EnterString(&pForm)) ? CMStringW() : CMStringW(ptrW(pForm.ptszResult));
}

CMStringW CVkProto::RunRenameNick(LPCWSTR pwszOldName)
{
	ENTER_STRING pForm = {};
	pForm.type = 0;
	pForm.caption = TranslateT("Enter new nickname");
	pForm.ptszInitVal = pwszOldName;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "renamenick_";
	return (!EnterString(&pForm)) ? CMStringW() : CMStringW(ptrW(pForm.ptszResult));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::GrabCookies(NETLIBHTTPREQUEST *nhr, CMStringA szDefDomain)
{
	debugLogA("CVkProto::GrabCookies");
	for (int i = 0; i < nhr->headersCount; i++) {
		if (_stricmp(nhr->headers[i].szName, "Set-cookie"))
			continue;

		CMStringA szValue = nhr->headers[i].szValue, szCookieName, szCookieVal, szDomain;
		int iStart = 0;
		while (true) {
			bool bFirstToken = (iStart == 0);
			CMStringA szToken = szValue.Tokenize(";", iStart).Trim();
			if (iStart == -1)
				break;

			if (bFirstToken) {
				int iStart2 = 0;
				szCookieName = szToken.Tokenize("=", iStart2);
				szCookieVal = szToken.Tokenize("=", iStart2);
			}
			else if (!strncmp(szToken, "domain=", 7))
				szDomain = szToken.Mid(7);
		}

		if (szDomain.IsEmpty() && !szDefDomain.IsEmpty())
			szDomain = szDefDomain;

		if (!szCookieName.IsEmpty() && !szDomain.IsEmpty()) {
			bool bFound = false;
			for (auto &it : m_cookies)
				if (it->m_name == szCookieName) {
					bFound = true;
					if (szCookieVal == "DELETED")
						m_cookies.remove(it);
					else
						it->m_value = szCookieVal;
					break;
				}

			if (!bFound && szCookieVal != "DELETED")
				m_cookies.insert(new CVkCookie(szCookieName, szCookieVal, szDomain));
		}
	}
}

void CVkProto::ApplyCookies(AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::ApplyCookies");
	CMStringA szCookie;

	for (auto &it : m_cookies) {
		if (!strstr(pReq->m_szUrl, it->m_domain))
			continue;

		if (!szCookie.IsEmpty())
			szCookie.Append("; ");
		szCookie.Append(it->m_name);
		szCookie.AppendChar('=');
		szCookie.Append(it->m_value);
	}

	if (!szCookie.IsEmpty())
		pReq->AddHeader("Cookie", szCookie);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::IsAuthContactLater(MCONTACT hContact)
{
	if (hContact == 0
		|| hContact == INVALID_CONTACT_ID
		|| isChatRoom(hContact)
		|| IsGroupUser(hContact)
		|| getDword(hContact, "ReqAuthTime") == 0
		|| getBool(hContact, "friend"))
		return false;

	if (time(0) - getDword(hContact, "ReqAuthTime") >= m_vkOptions.iReqAuthTimeLater) {
		db_unset(hContact, m_szModuleName, "ReqAuthTime");
		return false;
	}

	return true;
}

bool CVkProto::AddAuthContactLater(MCONTACT hContact)
{
	if (hContact == 0
		|| hContact == INVALID_CONTACT_ID
		|| isChatRoom(hContact)
		|| IsGroupUser(hContact)
		|| getDword(hContact, "ReqAuthTime") != 0
		|| getBool(hContact, "friend"))
		return false;

	setDword(hContact, "ReqAuthTime", (uint32_t)time(0));
	return true;
}

void __cdecl CVkProto::DBAddAuthRequestThread(void *p)
{
	CVkDBAddAuthRequestThreadParam *param = (CVkDBAddAuthRequestThreadParam *)p;
	if (param->hContact == 0 || param->hContact == INVALID_CONTACT_ID || !IsOnline())
		return;

	for (int i = 0; i < MAX_RETRIES && IsEmpty(ptrW(db_get_wsa(param->hContact, m_szModuleName, "Nick"))); i++) {
		Sleep(1500);

		if (!IsOnline())
			return;
	}

	DBAddAuthRequest(param->hContact, param->bAdded);
	delete param;
}

void CVkProto::DBAddAuthRequest(const MCONTACT hContact, bool added)
{
	debugLogA("CVkProto::DBAddAuthRequest");

	DB::AUTH_BLOB blob(hContact,
		T2Utf(ptrW(db_get_wsa(hContact, m_szModuleName, "Nick"))),
		T2Utf(ptrW(db_get_wsa(hContact, m_szModuleName, "FirstName"))),
		T2Utf(ptrW(db_get_wsa(hContact, m_szModuleName, "LastName"))), nullptr, nullptr);

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (uint32_t)time(0);
	dbei.flags = DBEF_UTF;
	dbei.eventType = added ? EVENTTYPE_ADDED : EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = blob.size();
	dbei.pBlob = blob;
	db_event_add(hContact, &dbei);
	debugLogA("CVkProto::DBAddAuthRequest %s", blob.get_nick() ? blob.get_nick() : "<null>");
}

MCONTACT CVkProto::MContactFromDbEvent(MEVENT hDbEvent)
{
	debugLogA("CVkProto::MContactFromDbEvent");
	if (!hDbEvent || !IsOnline())
		return INVALID_CONTACT_ID;

	uint32_t body[2];
	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(uint32_t) * 2;
	dbei.pBlob = (uint8_t*)&body;

	if (db_event_get(hDbEvent, &dbei))
		return INVALID_CONTACT_ID;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST || mir_strcmp(dbei.szModule, m_szModuleName))
		return INVALID_CONTACT_ID;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	db_unset(hContact, m_szModuleName, "ReqAuthTime");
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::SetMirVer(MCONTACT hContact, int platform)
{
	if (hContact == 0 || hContact == INVALID_CONTACT_ID)
		return;
	if (platform == -1) {
		db_unset(hContact, m_szModuleName, "MirVer");
		return;
	}

	CMStringW MirVer, OldMirVer(ptrW(db_get_wsa(hContact, m_szModuleName, "MirVer")));
	bool bSetFlag = true;

	switch (platform) {
	case VK_APP_ID:
		MirVer = L"Miranda NG VKontakte";
		break;
	case 2386311:
		MirVer = L"QIP 2012 VKontakte";
		break;
	case 1:
		MirVer = L"VKontakte (Mobile)";
		break;
	case 3087106: // iPhone
	case 3140623:
	case 2:
		MirVer = L"VKontakte (iPhone)";
		break;
	case 3682744: // iPad
	case 3:
		MirVer = L"VKontakte (iPad)";
		break;
	case 2685278: // Android - Kate
		MirVer = L"Kate Mobile (Android)";
		break;
	case 2890984: // Android
	case 2274003:
	case 4:
		MirVer = L"VKontakte (Android)";
		break;
	case 3059453: // Windows Phone
	case 2424737:
	case 3502561:
	case 5:
		MirVer = L"VKontakte (WPhone)";
		break;
	case 3584591: // Windows 8.x
	case 6:
		MirVer = L"VKontakte (Windows)";
		break;
	case 7:
		MirVer = L"VKontakte (Website)";
		break;
	case 5027722:
		MirVer = L"VK Messenger";
		break;
	case 4894723:
		MirVer = L"Phoenix Lite";
		break;
	case 4994316:
		MirVer = L"Phoenix Full";
		break;
	default:
		MirVer = L"VKontakte (Other)";
		bSetFlag = OldMirVer.IsEmpty();
	}

	if (OldMirVer == MirVer)
		return;

	if (bSetFlag)
		setWString(hContact, "MirVer", MirVer);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ContactTypingThread(void *p)
{
	debugLogA("CVkProto::ContactTypingThread");
	MCONTACT hContact = (UINT_PTR)p;
	CallService(MS_PROTO_CONTACTISTYPING, hContact, 5);
	Sleep(4500);
	CallService(MS_PROTO_CONTACTISTYPING, hContact);

	if (!g_bMessageState) {
		Sleep(1500);
		SetSrmmReadStatus(hContact);
	}
}

int CVkProto::IsHystoryMessageExist(MCONTACT hContact)
{
	if (!hContact)
		return 0;

	DB::ECPTR pCursor(DB::Events(hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType != VK_USER_DEACTIVATE_ACTION)
			return 1;
	}

	return -1;
}

int CVkProto::OnProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	debugLogA("CVkProto::OnProcessSrmmEvent");
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;


	CMStringA szProto(Proto_GetBaseAccountName(event->hContact));
	if (szProto.IsEmpty() || szProto != m_szModuleName)
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPENING && !g_bMessageState)
		SetSrmmReadStatus(event->hContact);

	if (event->uType == MSG_WINDOW_EVT_OPENING 	&& m_vkOptions.bLoadLastMessageOnMsgWindowsOpen
		&& !isChatRoom(event->hContact) && IsHystoryMessageExist(event->hContact) != 1) {
		m_bNotifyForEndLoadingHistory = false;
		if (!getBool(event->hContact, "ActiveHistoryTask")) {
			setByte(event->hContact, "ActiveHistoryTask", 1);
			GetServerHistory(event->hContact, 0, MAXHISTORYMIDSPERONE, 0, 0, true);
		}
	}

	return 0;
}

void CVkProto::SetSrmmReadStatus(MCONTACT hContact)
{
	time_t time = getDword(hContact, "LastMsgReadTime");
	if (!time)
		return;

	wchar_t ttime[64];
	_locale_t locale = _create_locale(LC_ALL, "");
	_wcsftime_l(ttime, _countof(ttime), L"%X - %x", localtime(&time), locale);
	_free_locale(locale);

	Srmm_SetStatusText(hContact, CMStringW(FORMAT, TranslateT("Message read: %s"), ttime), g_plugin.getIcon(IDI_READMSG));
}

void CVkProto::MarkDialogAsRead(MCONTACT hContact)
{
	debugLogA("CVkProto::MarkDialogAsRead");
	if (!IsOnline())
		return;

	LONG userID = getDword(hContact, "ID", VK_INVALID_USER);
	if (userID == VK_INVALID_USER || userID == VK_FEED_USER)
		return;

	MEVENT hDBEvent = db_event_firstUnread(hContact);
	MCONTACT hMContact = db_mc_tryMeta(hContact);
	while (hDBEvent != 0) {
		DBEVENTINFO dbei = {};
		if (!db_event_get(hDBEvent, &dbei) && !mir_strcmp(m_szModuleName, dbei.szModule)) {
			db_event_markRead(hContact, hDBEvent);
			g_clistApi.pfnRemoveEvent(hMContact, hDBEvent);
			if (hContact != hMContact)
				g_clistApi.pfnRemoveEvent(hContact, hDBEvent);
		}

		hDBEvent = db_event_next(hContact, hDBEvent);
	}
}

char* CVkProto::GetStickerId(const char *Msg, int &stickerid)
{
	stickerid = 0;
	char *retMsg = nullptr;

	int iRes = 0;
	const char *tmpMsg = strstr(Msg, "[sticker");
	if (tmpMsg)
		iRes = sscanf(tmpMsg, "[sticker:%d]", &stickerid) == 1 ? 1 : sscanf(tmpMsg, "[sticker-%d]", &stickerid);

	if (iRes == 1) {
		char HeadMsg[32] = { 0 };
		mir_snprintf(HeadMsg, "[sticker:%d]", stickerid);
		size_t retLen = mir_strlen(HeadMsg);
		if (retLen < mir_strlen(Msg)) {
			CMStringA szMsg(Msg, int(mir_strlen(Msg) - mir_strlen(tmpMsg)));
			szMsg.Append(&tmpMsg[retLen]);
			retMsg = mir_strdup(szMsg.Trim());
		}
	}

	return retMsg;
}

const char* FindVKUrls(const char *Msg)
{
	if (IsEmpty(Msg))
		return nullptr;

	const char *pos = nullptr;
	for (int i = 0; i < _countof(szVKUrls) && !pos; i++) {
		pos = strstr(Msg, szVKUrls[i]);
		if (pos)
			pos += mir_strlen(szVKUrls[i]);
	}

	if (pos >= (Msg + mir_strlen(Msg)))
		pos = nullptr;

	return pos;
}


CMStringA CVkProto::GetAttachmentsFromMessage(const char *Msg)
{
	if (IsEmpty(Msg))
		return CMStringA();

	const char *pos = FindVKUrls(Msg);
	if (!pos)
		return CMStringA();

	const char *nextpos = FindVKUrls(pos);
	const char *pos2 = nullptr;

	for (int i = 0; i < _countof(szVKLinkParam) && !pos2; i++) {
		pos2 = strstr(pos, szVKLinkParam[i]);
		if (pos2 && (!nextpos || pos2 < nextpos))
			pos = pos2 + mir_strlen(szVKLinkParam[i]);
	}

	if (pos >= (Msg + mir_strlen(Msg)))
		return CMStringA();

	int iRes = 0,
		iOwner = 0,
		iId = 0;

	for (auto &it : szAttachmentMasks) {
		iRes = sscanf(pos, it, &iOwner, &iId);
		if (iRes == 2) {
			CMStringA szAttachment(FORMAT, it, iOwner, iId);
			CMStringA szAttachment2;

			if (nextpos)
				szAttachment2 = GetAttachmentsFromMessage(pos + szAttachment.GetLength());

			if (!szAttachment2.IsEmpty())
				szAttachment += "," + szAttachment2;

			return szAttachment;
		}
		else if (iRes == 1)
			break;
	}

	return GetAttachmentsFromMessage(pos);
}

int CVkProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact != 0)
		return 0;

	if (strcmp(cws->szModule, "ListeningTo"))
		return 0;

	CMStringA szListeningTo(m_szModuleName);
	szListeningTo += "Enabled";
	if (!strcmp(cws->szSetting, szListeningTo)) {
		MusicSendMetod iOldMusicSendMetod = (MusicSendMetod)getByte("OldMusicSendMetod", sendBroadcastAndStatus);

		if (cws->value.bVal == 0)
			setByte("OldMusicSendMetod", m_vkOptions.iMusicSendMetod);
		else
			db_unset(0, m_szModuleName, "OldMusicSendMetod");

		m_vkOptions.iMusicSendMetod = cws->value.bVal == 0 ? sendNone : iOldMusicSendMetod;
		setByte("MusicSendMetod", m_vkOptions.iMusicSendMetod);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW CVkProto::SpanVKNotificationType(CMStringW& wszType, VKObjType& vkFeedback, VKObjType& vkParent)
{
	CVKNotification vkNotification[] = {
		// type, parent, feedback, string for translate
		{ L"group", vkInvite, vkNull, TranslateT("has invited you to a group") },
		{ L"page", vkInvite, vkNull, TranslateT("has invited you to subscribe to a page") },
		{ L"event", vkInvite, vkNull, TranslateT("invites you to event") },

		{ L"follow", vkNull, vkUsers, L"" },
		{ L"friend_accepted", vkNull, vkUsers, L"" },
		{ L"mention", vkNull, vkPost, L"" },
		{ L"wall", vkNull, vkPost, L"" },
		{ L"wall_publish", vkNull, vkPost, L"" },

		{ L"comment_post", vkPost, vkComment, TranslateT("commented on your post") },
		{ L"comment_photo", vkPhoto, vkComment, TranslateT("commented on your photo") },
		{ L"comment_video", vkVideo, vkComment, TranslateT("commented on your video") },
		{ L"reply_comment", vkComment, vkComment, TranslateT("replied to your comment") },
		{ L"reply_comment_photo", vkComment, vkComment, TranslateT("replied to your comment to photo") },
		{ L"reply_comment_video", vkComment, vkComment, TranslateT("replied to your comment to video") },
		{ L"reply_topic", vkTopic, vkComment, TranslateT("replied to your topic") },
		{ L"like_post", vkPost, vkUsers, TranslateT("liked your post") },
		{ L"like_comment", vkComment, vkUsers, TranslateT("liked your comment") },
		{ L"like_photo", vkPhoto, vkUsers, TranslateT("liked your photo") },
		{ L"like_video", vkVideo, vkUsers, TranslateT("liked your video") },
		{ L"like_comment_photo", vkComment, vkUsers, TranslateT("liked your comment to photo") },
		{ L"like_comment_video", vkComment, vkUsers, TranslateT("liked your comment to video") },
		{ L"like_comment_topic", vkComment, vkUsers, TranslateT("liked your comment to topic") },
		{ L"copy_post", vkPost, vkCopy, TranslateT("shared your post") },
		{ L"copy_photo", vkPhoto, vkCopy, TranslateT("shared your photo") },
		{ L"copy_video", vkVideo, vkCopy, TranslateT("shared your video") },
		{ L"mention_comments", vkPost, vkComment, L"mentioned you in comment" },
		{ L"mention_comment_photo", vkPhoto, vkComment, L"mentioned you in comment to photo" },
		{ L"mention_comment_video", vkVideo, vkComment, L"mentioned you in comment to video" }
	};

	CMStringW wszRes;
	vkFeedback = vkParent = vkNull;
	for (auto &it : vkNotification)
		if (wszType == it.pwszType) {
			vkFeedback = it.vkFeedback;
			vkParent = it.vkParent;
			wszRes = it.pwszTranslate;
			break;
		}
	return wszRes;
}

CMStringW CVkProto::GetVkPhotoItem(const JSONNode &jnPhoto, BBCSupport iBBC)
{
	CMStringW wszRes;

	if (!jnPhoto)
		return wszRes;

	CVKImageSizeItem vkSizes[6];
	CMStringW wszPriorSize = L"smxyzw", wszPreviewLink;
	int iMaxSize = 0;

	for (auto& it : jnPhoto["sizes"]) {
		int iIndex = wszPriorSize.Find(it["type"].as_mstring());
		if (iIndex < 0)
			continue;

		if (iIndex > iMaxSize)
			iMaxSize = iIndex;

		vkSizes[iIndex].wszUrl = it["url"].as_mstring();
		vkSizes[iIndex].iSizeH = it["height"].as_int();
		vkSizes[iIndex].iSizeW = it["width"].as_int();
	}

	switch (m_vkOptions.iIMGBBCSupport) {
	case imgNo:
		wszPreviewLink = L"";
		break;
	case imgFullSize:
		wszPreviewLink = vkSizes[iMaxSize].wszUrl;
		break;
	case imgPreview130:
		wszPreviewLink = vkSizes[wszPriorSize.Find(L"m")].wszUrl;
		break;
	case imgPreview604:
		wszPreviewLink = vkSizes[wszPriorSize.Find(L"x")].wszUrl.IsEmpty() ? vkSizes[wszPriorSize.Find(L"m")].wszUrl : vkSizes[wszPriorSize.Find(L"x")].wszUrl;
		break;
	}

	wszRes.AppendFormat(L"%s (%dx%d)",
		SetBBCString(TranslateT("Photo"), iBBC, vkbbcUrl, vkSizes[iMaxSize].wszUrl).c_str(),
		vkSizes[iMaxSize].iSizeW,
		vkSizes[iMaxSize].iSizeH
	);
	if (m_vkOptions.iIMGBBCSupport && iBBC != bbcNo)
		wszRes.AppendFormat(L"\n\t%s",
			SetBBCString((!wszPreviewLink.IsEmpty() ? wszPreviewLink : (!vkSizes[iMaxSize].wszUrl.IsEmpty() ? vkSizes[iMaxSize].wszUrl : L"")),
				bbcBasic,
				vkbbcImg).c_str()
		);
	CMStringW wszText(jnPhoto["text"].as_mstring());
	if (!wszText.IsEmpty())
		wszRes += L"\n" + wszText;

	return wszRes;
}

CMStringW CVkProto::SetBBCString(LPCWSTR pwszString, BBCSupport iBBC, VKBBCType bbcType, LPCWSTR wszAddString)
{
	CVKBBCItem bbcItem[] = {
		{ vkbbcB, bbcNo, L"%s" },
		{ vkbbcB, bbcBasic, L"[b]%s[/b]" },
		{ vkbbcB, bbcAdvanced, L"[b]%s[/b]" },
		{ vkbbcI, bbcNo, L"%s" },
		{ vkbbcI, bbcBasic, L"[i]%s[/i]" },
		{ vkbbcI, bbcAdvanced, L"[i]%s[/i]" },
		{ vkbbcS, bbcNo, L"%s" },
		{ vkbbcS, bbcBasic, L"[s]%s[/s]" },
		{ vkbbcS, bbcAdvanced, L"[s]%s[/s]" },
		{ vkbbcU, bbcNo, L"%s" },
		{ vkbbcU, bbcBasic, L"[u]%s[/u]" },
		{ vkbbcU, bbcAdvanced, L"[u]%s[/u]" },
		{ vkbbcCode, bbcNo, L"%s" },
		{ vkbbcCode, bbcBasic, L"%s" },
		{ vkbbcCode, bbcAdvanced, L"[code]%s[/code]" },
		{ vkbbcImg, bbcNo, L"%s" },
		{ vkbbcImg, bbcBasic, L"[img]%s[/img]" },
		{ vkbbcImg, bbcAdvanced, L"[img]%s[/img]" },
		{ vkbbcUrl, bbcNo, L"%s (%s)" },
		{ vkbbcUrl, bbcBasic, L"[i]%s[/i] (%s)" },
		{ vkbbcUrl, bbcAdvanced, L"[url=%s]%s[/url]" },
		{ vkbbcSize, bbcNo, L"%s" },
		{ vkbbcSize, bbcBasic, L"%s" },
		{ vkbbcSize, bbcAdvanced, L"[size=%s]%s[/size]" },
		{ vkbbcColor, bbcNo, L"%s" },
		{ vkbbcColor, bbcBasic, L"%s" },
		{ vkbbcColor, bbcAdvanced, L"[color=%s]%s[/color]" },
	};

	if (IsEmpty(pwszString))
		return CMStringW();

	wchar_t *pwszFormat = nullptr;
	for (auto &it : bbcItem)
		if (it.vkBBCType == bbcType && it.vkBBCSettings == iBBC) {
			pwszFormat = it.pwszTempate;
			break;
		}

	CMStringW res;
	if (pwszFormat == nullptr)
		return CMStringW(pwszString);

	if (bbcType == vkbbcUrl && iBBC != bbcAdvanced)
		res.AppendFormat(pwszFormat, pwszString, wszAddString ? wszAddString : L"");
	else if (iBBC == bbcAdvanced && bbcType >= vkbbcUrl)
		res.AppendFormat(pwszFormat, wszAddString ? wszAddString : L"", pwszString);
	else
		res.AppendFormat(pwszFormat, pwszString);

	return res;
}

CMStringW& CVkProto::ClearFormatNick(CMStringW& wszText)
{
	int iNameEnd = wszText.Find(L"],"), iNameBeg = wszText.Find(L"|");
	if (iNameEnd != -1 && iNameBeg != -1 && iNameBeg < iNameEnd) {
		CMStringW wszName = wszText.Mid(iNameBeg + 1, iNameEnd - iNameBeg - 1);
		CMStringW wszBody = wszText.Mid(iNameEnd + 2);
		if (!wszName.IsEmpty() && !wszBody.IsEmpty())
			wszText = wszName + L"," + wszBody;
	}

	return wszText;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringW CVkProto::GetAttachmentDescr(const JSONNode &jnAttachments, BBCSupport iBBC)
{
	debugLogA("CVkProto::GetAttachmentDescr");
	CMStringW res;
	if (!jnAttachments) {
		debugLogA("CVkProto::GetAttachmentDescr pAttachments == nullptr");
		return res;
	}

	res += SetBBCString(TranslateT("Attachments:"), iBBC, vkbbcB);
	res.AppendChar('\n');

	for (auto& jnAttach : jnAttachments) {
		res.AppendChar('\t');
		CMStringW wszType(jnAttach["type"].as_mstring());
		if (wszType == L"photo") {
			const JSONNode& jnPhoto = jnAttach["photo"];
			if (!jnPhoto)
				continue;

			res += GetVkPhotoItem(jnPhoto, iBBC);
		}
		else if (wszType == L"audio") {
			const JSONNode& jnAudio = jnAttach["audio"];
			if (!jnAudio)
				continue;

			CMStringW wszArtist(jnAudio["artist"].as_mstring());
			CMStringW wszTitle(jnAudio["title"].as_mstring());
			CMStringW wszUrl(jnAudio["url"].as_mstring());
			CMStringW wszAudio(FORMAT, L"%s - %s", wszArtist.c_str(), wszTitle.c_str());

			int iParamPos = wszUrl.Find(L"?");
			if (m_vkOptions.bShortenLinksForAudio && iParamPos != -1)
				wszUrl = wszUrl.Left(iParamPos);

			res.AppendFormat(L"%s: %s",
				SetBBCString(TranslateT("Audio"), iBBC, vkbbcB).c_str(),
				SetBBCString(wszAudio, iBBC, vkbbcUrl, wszUrl).c_str());
		}
		else if (wszType == L"audio_message") {
			const JSONNode& jnAudioMessage = jnAttach["audio_message"];
			if (!jnAudioMessage)
				continue;

			if (m_vkOptions.bFilterAudioMessages)
				return L"== FilterAudioMessages ==";

			CMStringW wszUrl(jnAudioMessage["link_mp3"].as_mstring());
			CMStringW wszTranscriptText(jnAudioMessage["transcript"].as_mstring());

			res.AppendFormat(L"%s", SetBBCString(TranslateT("Audio message"), iBBC, vkbbcUrl, wszUrl).c_str());
			if (!wszTranscriptText.IsEmpty())
				res.AppendFormat(L"\n%s: %s", SetBBCString(TranslateT("Transcription"), iBBC, vkbbcB).c_str(), wszTranscriptText.c_str());
		}
		else if (wszType == L"graffiti") {
			const JSONNode& jnGraffiti = jnAttach["graffiti"];
			if (!jnGraffiti)
				continue;

			CMStringW wszUrl(jnGraffiti["url"].as_mstring());

			res.AppendFormat(L"%s\n\t%s",
				SetBBCString(TranslateT("Graffiti"), iBBC, vkbbcUrl, wszUrl).c_str(),
				SetBBCString(wszUrl, bbcBasic, vkbbcImg).c_str()
			);
		}
		else if (wszType == L"video") {
			const JSONNode& jnVideo = jnAttach["video"];
			if (!jnVideo)
				continue;

			CMStringW wszTitle(jnVideo["title"].as_mstring());
			int iVideoId = jnVideo["id"].as_int();
			int iOwnerId = jnVideo["owner_id"].as_int();
			CMStringW wszUrl(FORMAT, L"https://vk.com/video%d_%d", iOwnerId, iVideoId);

			res.AppendFormat(L"%s: %s",
				SetBBCString(TranslateT("Video"), iBBC, vkbbcB).c_str(),
				SetBBCString(wszTitle.IsEmpty() ? TranslateT("Link") : wszTitle, iBBC, vkbbcUrl, wszUrl).c_str());
		}
		else if (wszType == L"doc") {
			const JSONNode& jnDoc = jnAttach["doc"];
			if (!jnDoc)
				continue;

			if (jnDoc["type"].as_int() == 5 && m_vkOptions.bFilterAudioMessages)
				return L"== FilterAudioMessages ==";

			CMStringW wszTitle(jnDoc["title"].as_mstring());
			CMStringW wszUrl(jnDoc["url"].as_mstring());
			res.AppendFormat(L"%s: %s",
				jnDoc["type"].as_int() == 5 ? SetBBCString(TranslateT("Audio message"), iBBC, vkbbcB).c_str() : SetBBCString(TranslateT("Document"), iBBC, vkbbcB).c_str(),
				SetBBCString(wszTitle.IsEmpty() ? TranslateT("Link") : wszTitle, iBBC, vkbbcUrl, wszUrl).c_str());
		}
		else if (wszType == L"wall") {
			const JSONNode& jnWall = jnAttach["wall"];
			if (!jnWall)
				continue;

			CMStringW wszText(jnWall["text"].as_mstring());
			int iWallId = jnWall["id"].as_int();
			int iFromId = jnWall["from_id"].as_int();
			CMStringW wszUrl(FORMAT, L"https://vk.com/wall%d_%d", iFromId, iWallId);
			res.AppendFormat(L"%s: %s",
				SetBBCString(TranslateT("Wall post"), iBBC, vkbbcUrl, wszUrl).c_str(),
				wszText.IsEmpty() ? L" " : wszText.c_str());

			const JSONNode& jnCopyHystory = jnWall["copy_history"];
			if (jnCopyHystory) {
				for (auto& jnCopyHystoryItem : jnCopyHystory) {
					CMStringW wszCHText(jnCopyHystoryItem["text"].as_mstring());
					int iCHid = jnCopyHystoryItem["id"].as_int();
					int iCHfromID = jnCopyHystoryItem["from_id"].as_int();
					CMStringW wszCHUrl(FORMAT, L"https://vk.com/wall%d_%d", iCHfromID, iCHid);
					wszCHText.Replace(L"\n", L"\n\t\t");
					res.AppendFormat(L"\n\t\t%s: %s",
						SetBBCString(TranslateT("Wall post"), iBBC, vkbbcUrl, wszCHUrl).c_str(),
						wszCHText.IsEmpty() ? L" " : wszCHText.c_str());

					const JSONNode& jnSubAttachments = jnCopyHystoryItem["attachments"];
					if (jnSubAttachments) {
						debugLogA("CVkProto::GetAttachmentDescr SubAttachments");
						CMStringW wszAttachmentDescr = GetAttachmentDescr(jnSubAttachments, iBBC);
						wszAttachmentDescr.Replace(L"\n", L"\n\t\t");
						wszAttachmentDescr.Replace(L"== FilterAudioMessages ==", L"");
						res += L"\n\t\t" + wszAttachmentDescr;
					}
				}
			}

			const JSONNode& jnSubAttachments = jnWall["attachments"];
			if (jnSubAttachments) {
				debugLogA("CVkProto::GetAttachmentDescr SubAttachments");
				CMStringW wszAttachmentDescr = GetAttachmentDescr(jnSubAttachments, iBBC);
				wszAttachmentDescr.Replace(L"\n", L"\n\t");
				wszAttachmentDescr.Replace(L"== FilterAudioMessages ==", L"");
				res += L"\n\t" + wszAttachmentDescr;
			}
		}
		else if (wszType == L"wall_reply") {
			const JSONNode& jnWallReply = jnAttach["wall_reply"];
			if (!jnWallReply)
				continue;

			CMStringW wszText(jnWallReply["text"].as_mstring());
			int iWallReplyId = jnWallReply["id"].as_int();
			int iFromId = jnWallReply["from_id"].as_int();
			int iFromOwnerId = jnWallReply["owner_id"].as_int();
			int iPostOwnerId = jnWallReply["post_id"].as_int();
			int iThreadId = jnWallReply["reply_to_comment"].as_int();

			CMStringW wszUrl(FORMAT, L"https://vk.com/wall%d_%d?reply=%d&thread=%d", iFromOwnerId, iPostOwnerId, iWallReplyId, iThreadId);

			CMStringW wszFromNick, wszFromUrl;
			MCONTACT hFromContact = FindUser(iFromId);
			if (hFromContact || iFromId == m_myUserId)
				wszFromNick = ptrW(db_get_wsa(hFromContact, m_szModuleName, "Nick"));
			else
				wszFromNick = TranslateT("(Unknown contact)");
			wszFromUrl = UserProfileUrl(iFromId);

			res.AppendFormat(L"%s %s %s: %s",
				SetBBCString(TranslateT("Wall reply"), iBBC, vkbbcUrl, wszUrl).c_str(),
				TranslateT("from"),
				SetBBCString(wszFromNick, iBBC, vkbbcUrl, wszFromUrl).c_str(),
				wszText.IsEmpty() ? L" " : wszText.c_str());

			const JSONNode& jnSubAttachments = jnWallReply["attachments"];
			if (jnSubAttachments) {
				debugLogA("CVkProto::GetAttachmentDescr SubAttachments");
				CMStringW wszAttachmentDescr = GetAttachmentDescr(jnSubAttachments, iBBC);
				wszAttachmentDescr.Replace(L"\n", L"\n\t");
				wszAttachmentDescr.Replace(L"== FilterAudioMessages ==", L"");
				res += L"\n\t" + wszAttachmentDescr;
			}
		}
		else if (wszType == L"story") {
			const JSONNode& jnStory = jnAttach["story"];
			if (!jnStory)
				continue;
			int iStoryId = jnStory["id"].as_int();
			int iOwnerID = jnStory["owner_id"].as_int();
			CMStringW wszUrl(FORMAT, L"https://vk.com/story%d_%d", iOwnerID, iStoryId);

			res.AppendFormat(L"%s",
				SetBBCString(TranslateT("Story"), iBBC, vkbbcUrl, wszUrl).c_str());

			CMStringW wszStoryType(jnStory["type"].as_mstring());

			if (wszStoryType == L"photo") {
				const JSONNode& jnPhoto = jnStory["photo"];
				if (!jnPhoto)
					continue;

				res += L"\n\t";
				res += GetVkPhotoItem(jnPhoto, iBBC);
			}
			else if (wszStoryType == L"video") {
				const JSONNode& jnVideo = jnStory["video"];
				if (!jnVideo)
					continue;

				CMStringW wszTitle(jnVideo["title"].as_mstring());
				int iVideoId = jnVideo["id"].as_int();
				int iOwnerId = jnVideo["owner_id"].as_int();
				CMStringW wszVideoUrl(FORMAT, L"https://vk.com/video%d_%d", iOwnerId, iVideoId);

				res.AppendFormat(L"\n\t%s: %s",
					SetBBCString(TranslateT("Video"), iBBC, vkbbcB).c_str(),
					SetBBCString(wszTitle.IsEmpty() ? TranslateT("Link") : wszTitle, iBBC, vkbbcUrl, wszVideoUrl).c_str());

			}

		}
		else if (wszType == L"sticker") {
			const JSONNode& jnSticker = jnAttach["sticker"];
			if (!jnSticker)
				continue;
			res.Empty(); // sticker is not really an attachment, so we don't want all that heading info

			CMStringW wszLink, wszLink128, wszLinkLast, wszUrl;
			const JSONNode& jnImages = jnSticker[m_vkOptions.bStickerBackground ? "images_with_background" : "images"];

			int iStickerId = jnSticker["sticker_id"].as_int();

			for (auto& jnImage : jnImages) {
				if (jnImage["width"].as_int() == (int)m_vkOptions.iStickerSize) {
					wszLink = jnImage["url"].as_mstring();
					break;
				}

				if (jnImage["width"].as_int() == 128) // default size
					wszLink128 = jnImage["url"].as_mstring();

				wszLinkLast = jnImage["url"].as_mstring();
			}

			wszUrl = wszLink.IsEmpty() ? (wszLink128.IsEmpty() ? wszLinkLast : wszLink128) : wszLink;

			if (!m_vkOptions.bStikersAsSmileys)
				res += SetBBCString(wszUrl, iBBC, vkbbcImg);
			else if (m_vkOptions.bUseStikersAsStaticSmileys)
				res.AppendFormat(L"[sticker:%d]", iStickerId);
			else {
				if (ServiceExists(MS_SMILEYADD_LOADCONTACTSMILEYS)) {
					CMStringW wszPath(FORMAT, L"%s\\%S\\Stickers", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
					CreateDirectoryTreeW(wszPath);

					bool bSuccess = false;
					CMStringW wszFileName(FORMAT, L"%s\\[sticker-%d].png", wszPath.c_str(), iStickerId);

					if (GetFileAttributesW(wszFileName) == INVALID_FILE_ATTRIBUTES) {
						T2Utf szUrl(wszUrl);
						NETLIBHTTPREQUEST req = {};
						req.cbSize = sizeof(req);
						req.flags = NLHRF_NODUMP | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
						req.requestType = REQUEST_GET;
						req.szUrl = szUrl;

						NETLIBHTTPREQUEST* pReply = Netlib_HttpTransaction(m_hNetlibUser, &req);
						if (pReply != nullptr && pReply->resultCode == 200 && pReply->pData && pReply->dataLength) {
							bSuccess = true;
							FILE* out = _wfopen(wszFileName, L"wb");
							fwrite(pReply->pData, 1, pReply->dataLength, out);
							fclose(out);
						}
					}
					else bSuccess = true;

					if (bSuccess) {
						res.AppendFormat(L"[sticker-%d]",  iStickerId);

						SMADD_CONT cont = { 1, m_szModuleName, wszFileName };
						CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));
					}
					else res += SetBBCString(TranslateT("Sticker"), iBBC, vkbbcUrl, wszUrl);
				}
			}
		}
		else if (wszType == L"link") {
			const JSONNode& jnLink = jnAttach["link"];
			if (!jnLink)
				continue;

			CMStringW wszUrl(jnLink["url"].as_mstring());
			CMStringW wszTitle(jnLink["title"].as_mstring());
			CMStringW wszCaption(jnLink["caption"].as_mstring());
			CMStringW wszDescription(jnLink["description"].as_mstring());

			res.AppendFormat(L"%s: %s",
				SetBBCString(TranslateT("Link"), iBBC, vkbbcB).c_str(),
				SetBBCString(wszTitle.IsEmpty() ? TranslateT("Link") : wszTitle, iBBC, vkbbcUrl, wszUrl).c_str());

			if (!wszCaption.IsEmpty())
				res.AppendFormat(L"\n\t%s", SetBBCString(wszCaption, iBBC, vkbbcI).c_str());

			if (jnLink["photo"])
				res.AppendFormat(L"\n\t%s", GetVkPhotoItem(jnLink["photo"], iBBC).c_str());

			if (!wszDescription.IsEmpty())
				res.AppendFormat(L"\n\t%s", wszDescription.c_str());
		}
		else if (wszType == L"market") {
			const JSONNode& jnMarket = jnAttach["market"];

			int id = jnMarket["id"].as_int();
			int ownerID = jnMarket["owner_id"].as_int();
			CMStringW wszTitle(jnMarket["title"].as_mstring());
			CMStringW wszDescription(jnMarket["description"].as_mstring());
			CMStringW wszPhoto(jnMarket["thumb_photo"].as_mstring());
			CMStringW wszUrl(FORMAT, L"https://vk.com/%s%d?w=product%d_%d",
				ownerID > 0 ? L"id" : L"club",
				ownerID > 0 ? ownerID : (-1) * ownerID,
				ownerID,
				id);

			res.AppendFormat(L"%s: %s",
				SetBBCString(TranslateT("Product"), iBBC, vkbbcB).c_str(),
				SetBBCString(wszTitle.IsEmpty() ? TranslateT("Link") : wszTitle, iBBC, vkbbcUrl, wszUrl).c_str());

			if (!wszPhoto.IsEmpty())
				res.AppendFormat(L"\n\t%s: %s",
					SetBBCString(TranslateT("Photo"), iBBC, vkbbcB).c_str(),
					SetBBCString(wszPhoto, iBBC, vkbbcImg).c_str());

			if (jnMarket["price"] && jnMarket["price"]["text"])
				res.AppendFormat(L"\n\t%s: %s",
					SetBBCString(TranslateT("Price"), iBBC, vkbbcB).c_str(),
					jnMarket["price"]["text"].as_mstring().c_str());

			if (!wszDescription.IsEmpty())
				res.AppendFormat(L"\n\t%s", wszDescription.c_str());
		}
		else if (wszType == L"gift") {
			const JSONNode& jnGift = jnAttach["gift"];
			if (!jnGift)
				continue;

			CMStringW wszLink;
			for (auto& it : szGiftTypes) {
				const JSONNode& n = jnGift[it];
				if (n) {
					wszLink = n.as_mstring();
					break;
				}
			}
			if (wszLink.IsEmpty())
				continue;
			res += SetBBCString(TranslateT("Gift"), iBBC, vkbbcUrl, wszLink);

			if (m_vkOptions.iIMGBBCSupport && iBBC != bbcNo)
				res.AppendFormat(L"\n\t%s", SetBBCString(wszLink, iBBC, vkbbcImg).c_str());
		}
		else {
			res.AppendFormat(TranslateT("Unsupported or unknown attachment type: %s"), SetBBCString(wszType, iBBC, vkbbcB).c_str());
			const JSONNode& jnUnknown = jnAttach[jnAttach["type"].as_string().c_str()];
			CMStringW wszText(jnUnknown["text"].as_mstring());
			if (!wszText.IsEmpty())
				res.AppendFormat(L"\n%s: %s", TranslateT("Text"), wszText.c_str());
		}

		res.AppendChar('\n');
	}

	return res;
}

CMStringW CVkProto::GetFwdMessage(const JSONNode& jnMsg, const JSONNode& jnFUsers, OBJLIST<CVkUserInfo>& vkUsers, BBCSupport iBBC)
{
	LONG uid = jnMsg["from_id"].as_int();
	CMStringW wszBody(jnMsg["text"].as_mstring());

	CVkUserInfo* vkUser = vkUsers.find((CVkUserInfo*)&uid);
	CMStringW wszNick, wszUrl;

	if (vkUser) {
		wszNick = vkUser->m_wszUserNick;
		wszUrl = vkUser->m_wszLink;
	}
	else {
		MCONTACT hContact = FindUser(uid);
		if (hContact || uid == m_myUserId)
			wszNick = ptrW(db_get_wsa(hContact, m_szModuleName, "Nick"));
		else
			wszNick = TranslateT("(Unknown contact)");
		wszUrl = UserProfileUrl(uid);
	}

	time_t datetime = (time_t)jnMsg["date"].as_int();
	wchar_t ttime[64];
	_locale_t locale = _create_locale(LC_ALL, "");
	_wcsftime_l(ttime, _countof(ttime), L"%x %X", localtime(&datetime), locale);
	_free_locale(locale);

	const JSONNode& jnFwdMessages = jnMsg["fwd_messages"];
	if (jnFwdMessages && !jnFwdMessages.empty()) {
		CMStringW wszFwdMessages = GetFwdMessages(jnFwdMessages, jnFUsers, iBBC == bbcNo ? iBBC : m_vkOptions.BBCForAttachments());
		if (!wszBody.IsEmpty())
			wszFwdMessages = L"\n" + wszFwdMessages;
		wszBody += wszFwdMessages;
	}

	const JSONNode& jnAttachments = jnMsg["attachments"];
	if (jnAttachments && !jnAttachments.empty()) {
		CMStringW wszAttachmentDescr = GetAttachmentDescr(jnAttachments, iBBC == bbcNo ? iBBC : m_vkOptions.BBCForAttachments());
		if (wszAttachmentDescr != L"== FilterAudioMessages ==") {
			if (!wszBody.IsEmpty())
				wszAttachmentDescr = L"\n" + wszAttachmentDescr;

			wszBody += wszAttachmentDescr;
		}
	}

	wszBody.Replace(L"\n", L"\n\t");
	wchar_t tcSplit = m_vkOptions.bSplitFormatFwdMsg ? '\n' : ' ';
	CMStringW wszMes(FORMAT, L"%s %s%c%s %s:\n\n%s\n",
		SetBBCString(TranslateT("Message from"), iBBC, vkbbcB).c_str(),
		SetBBCString(wszNick, iBBC, vkbbcUrl, wszUrl).c_str(),
		tcSplit,
		SetBBCString(TranslateT("at"), iBBC, vkbbcB).c_str(),
		ttime,
		SetBBCString(wszBody, iBBC, vkbbcCode).c_str());

	return wszMes;

}

CMStringW CVkProto::GetFwdMessages(const JSONNode &jnMessages, const JSONNode &jnFUsers, BBCSupport iBBC)
{
	CMStringW res;
	debugLogA("CVkProto::GetFwdMessages");
	if (!jnMessages) {
		debugLogA("CVkProto::GetFwdMessages pMessages == nullptr");
		return res;
	}

	OBJLIST<CVkUserInfo> vkUsers(2, NumericKeySortT);

	for (auto &jnUser : jnFUsers) {
		int iUserId = jnUser["id"].as_int();
		CMStringW wszNick(jnUser["name"].as_mstring());

		if (!wszNick.IsEmpty())
			iUserId *= -1;
		else
			wszNick.AppendFormat(L"%s %s", jnUser["first_name"].as_mstring().c_str(), jnUser["last_name"].as_mstring().c_str());

		CVkUserInfo *vkUser = new CVkUserInfo(jnUser["id"].as_int(), false, wszNick, UserProfileUrl(iUserId), FindUser(iUserId));
		vkUsers.insert(vkUser);
	}

	if (jnMessages.type() == JSON_ARRAY)
		for (auto& jnMsg : jnMessages.as_array()) {
			if (!res.IsEmpty())
				res.AppendChar('\n');
			res += GetFwdMessage(jnMsg, jnFUsers, vkUsers, iBBC);
		}
	else
		res = GetFwdMessage(jnMessages,  jnFUsers, vkUsers, iBBC);

	res.AppendChar('\n');
	vkUsers.destroy();
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::SetInvisible(MCONTACT hContact)
{
	debugLogA("CVkProto::SetInvisible %d", getDword(hContact, "ID", VK_INVALID_USER));
	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
		setWord(hContact, "Status", ID_STATUS_INVISIBLE);
		SetMirVer(hContact, 1);
		db_set_dw(hContact, "BuddyExpectator", "LastStatus", ID_STATUS_INVISIBLE);
		debugLogA("CVkProto::SetInvisible %d set ID_STATUS_INVISIBLE", getDword(hContact, "ID", VK_INVALID_USER));
	}
	time_t now = time(0);
	db_set_dw(hContact, "BuddyExpectator", "LastSeen", (uint32_t)now);
	setDword(hContact, "InvisibleTS", (uint32_t)now);
}

CMStringW CVkProto::RemoveBBC(CMStringW& wszSrc)
{
	static const wchar_t *wszSimpleBBCodes[][2] = {
		{ L"[b]", L"[/b]" },
		{ L"[u]", L"[/u]" },
		{ L"[i]", L"[/i]" },
		{ L"[s]", L"[/s]" },
	};

	static const wchar_t *wszParamBBCodes[][2] = {
		{ L"[url=", L"[/url]" },
		{ L"[img=", L"[/img]" },
		{ L"[size=", L"[/size]" },
		{ L"[color=", L"[/color]" },
	};

	CMStringW wszRes(wszSrc);
	CMStringW wszLow(wszSrc);
	wszLow.MakeLower();

	for (auto &it : wszSimpleBBCodes) {
		CMStringW wszOpenTag(it[0]);
		CMStringW wszCloseTag(it[1]);

		int lenOpen = wszOpenTag.GetLength();
		int lenClose = wszCloseTag.GetLength();

		int posOpen = 0;
		int posClose = 0;

		while (true) {
			if ((posOpen = wszLow.Find(wszOpenTag, posOpen)) < 0)
				break;

			if ((posClose = wszLow.Find(wszCloseTag, posOpen + lenOpen)) < 0)
				break;

			wszLow.Delete(posOpen, lenOpen);
			wszLow.Delete(posClose - lenOpen, lenClose);

			wszRes.Delete(posOpen, lenOpen);
			wszRes.Delete(posClose - lenOpen, lenClose);

		}
	}

	for (auto &it : wszParamBBCodes) {
		CMStringW wszOpenTag(it[0]);
		CMStringW wszCloseTag(it[1]);

		int lenOpen = wszOpenTag.GetLength();
		int lenClose = wszCloseTag.GetLength();

		int posOpen = 0;
		int posOpen2 = 0;
		int posClose = 0;

		while (true) {
			if ((posOpen = wszLow.Find(wszOpenTag, posOpen)) < 0)
				break;

			if ((posOpen2 = wszLow.Find(L"]", posOpen + lenOpen)) < 0)
				break;

			if ((posClose = wszLow.Find(wszCloseTag, posOpen2 + 1)) < 0)
				break;

			wszLow.Delete(posOpen, posOpen2 - posOpen + 1);
			wszLow.Delete(posClose - posOpen2 + posOpen - 1, lenClose);

			wszRes.Delete(posOpen, posOpen2 - posOpen + 1);
			wszRes.Delete(posClose - posOpen2 + posOpen - 1, lenClose);

		}

	}

	return wszRes;
}

void CVkProto::ShowCaptchaInBrowser(HBITMAP hBitmap)
{
	wchar_t wszTempDir[MAX_PATH];
	if (!GetEnvironmentVariable(L"TEMP", wszTempDir, MAX_PATH))
		return;

	CMStringW wszHTMLPath(FORMAT, L"%s\\miranda_captcha.html", wszTempDir);

	FILE *pFile = _wfopen(wszHTMLPath, L"w");
	if (pFile == nullptr)
		return;

	FIBITMAP *dib = FreeImage_CreateDIBFromHBITMAP(hBitmap);
	FIMEMORY *hMem = FreeImage_OpenMemory(nullptr, 0);
	FreeImage_SaveToMemory(FIF_PNG, dib, hMem, 0);

	uint8_t *buf = nullptr;
	uint32_t bufLen;
	FreeImage_AcquireMemory(hMem, &buf, &bufLen);
	ptrA base64(mir_base64_encode(buf, bufLen));
	FreeImage_CloseMemory(hMem);
	FreeImage_Unload(dib);

	CMStringA szHTML(FORMAT, "<html><body><img src=\"data:image/png;base64,%s\" /></body></html>", base64);
	fwrite(szHTML, 1, szHTML.GetLength(), pFile);
	fclose(pFile);

	wszHTMLPath = L"file://" + wszHTMLPath;
	Utils_OpenUrlW(wszHTMLPath);
}

void CVkProto::AddVkDeactivateEvent(MCONTACT hContact, CMStringW&  wszType)
{
	debugLogW(L"CVkProto::AddVkDeactivateEvent hContact=%d, wszType=%s bShowVkDeactivateEvents=<%d,%d,%d>",
		hContact, wszType.c_str(),
		(int)m_vkOptions.bShowVkDeactivateEvents,
		(int)getBool(hContact, "ShowVkDeactivateEvents", true),
		(int)(!Contact::IsHidden(hContact)));

	CVKDeactivateEvent vkDeactivateEvent[] = {
		{ L"", Translate("User restored control over own page") },
		{ L"deleted", Translate("User was deactivated (deleted)") },
		{ L"banned", Translate("User was deactivated (banned)") }
	};

	int iDEIdx = -1;
	for (int i = 0; i < _countof(vkDeactivateEvent); i++)
		if (wszType == vkDeactivateEvent[i].wszType) {
			iDEIdx = i;
			break;
		}

	if (iDEIdx == -1)
		return;

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.timestamp = time(0);
	dbei.eventType = VK_USER_DEACTIVATE_ACTION;
	ptrA pszDescription(mir_utf8encode(vkDeactivateEvent[iDEIdx].szDescription));
	dbei.cbBlob = (uint32_t)mir_strlen(pszDescription) + 1;
	dbei.pBlob = (uint8_t*)mir_strdup(pszDescription);
	dbei.flags = DBEF_UTF | (
		(
			m_vkOptions.bShowVkDeactivateEvents
			&& getBool(hContact, "ShowVkDeactivateEvents", true)
			&& (!Contact::IsHidden(hContact))
		) ? 0 : DBEF_READ);
	db_event_add(hContact, &dbei);
}


MEVENT CVkProto::GetMessageFromDb(UINT iMsgId, UINT &timestamp, CMStringW &msg)
{
	char szMid[40];
	_itoa(iMsgId, szMid, 10);
	return GetMessageFromDb(szMid, timestamp, msg);
}

MEVENT CVkProto::GetMessageFromDb(const char *messageId, UINT &timestamp, CMStringW &msg)
{
	if (messageId == nullptr)
		return 0;

	MEVENT hDbEvent = db_event_getById(m_szModuleName, messageId);
	if (!hDbEvent)
		return 0;

	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	db_event_get(hDbEvent, &dbei);

	msg = ptrW(mir_utf8decodeW((char*)dbei.pBlob));
	timestamp = dbei.timestamp;

	return hDbEvent;
}

int CVkProto::DeleteContact(MCONTACT hContact)
{
	setByte(hContact, "SilentDelete", 1);
	return db_delete_contact(hContact);
}

bool CVkProto::IsMessageExist(UINT iMsgId, VKMesType vkType)
{
	char szMid[40];
	_itoa(iMsgId, szMid, 10);

	MEVENT hDbEvent = db_event_getById(m_szModuleName, szMid);

	if (!hDbEvent)
		return false;

	if (vkType == vkALL)
		return true;

	DBEVENTINFO dbei = {};
	if(db_event_get(hDbEvent, &dbei))
		return false;

	return ((vkType == vkOUT) == (bool)(dbei.flags & DBEF_SENT));
}

CMStringW CVkProto::UserProfileUrl(LONG iUserId)
{
	return CMStringW(FORMAT, L"https://vk.com/%s%d", iUserId > 0 ? L"id" : L"club", iUserId > 0 ? iUserId : -1 * iUserId);
}