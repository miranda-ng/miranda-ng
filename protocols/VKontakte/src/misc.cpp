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

static char* szImageTypes[] = { "photo_2560", "photo_1280", "photo_807", "photo_604", "photo_256", "photo_130", "photo_128", "photo_75", "photo_64" };

static char* szGiftTypes[] = { "thumb_256", "thumb_96", "thumb_48" };

bool IsEmpty(TCHAR *str)
{
	if (str == NULL)
		return true;
	if (str[0] == 0)
		return true;
	return false;
}

bool IsEmpty(char *str)
{
	if (str == NULL)
		return true;
	if (str[0] == 0)
		return true;
	return false;
}

CMString json_as_CMString(JSONNODE* pNode)
{
	ptrT pString(json_as_string(pNode));
	CMString tszString = pString;
	return tszString;
}

LPCSTR findHeader(NETLIBHTTPREQUEST *pReq, LPCSTR szField)
{
	for (int i = 0; i < pReq->headersCount; i++)
		if (!_stricmp(pReq->headers[i].szName, szField))
			return pReq->headers[i].szValue;

	return NULL;
}

bool tlstrstr(TCHAR* _s1, TCHAR* _s2)
{
	TCHAR s1[1024], s2[1024];

	_tcsncpy_s(s1, _s1, _TRUNCATE);
	CharLowerBuff(s1, SIZEOF(s1));
	_tcsncpy_s(s2, _s2, _TRUNCATE);
	CharLowerBuff(s2, SIZEOF(s2));

	return _tcsstr(s1, s2) != NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Captcha form icon"), "key", IDI_KEYS },
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
	{ LPGEN("Wall message icon"), "wall", IDI_WALL }
};

void InitIcons()
{
	Icon_Register(hInst, LPGEN("Protocols")"/"LPGEN("VKontakte"), iconList, SIZEOF(iconList), "VKontakte");
}

HANDLE GetIconHandle(int iCommand)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iCommand)
			return iconList[i].hIcolib;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

char* ExpUrlEncode(const char *szUrl, bool strict)
{
	const char szHexDigits[] = "0123456789ABCDEF";

	if (szUrl == NULL)
		return NULL;

	const BYTE *s;
	int outputLen;
	for (outputLen = 0, s = (const BYTE*)szUrl; *s; s++) 
		if ((*s & 0x80 && !strict) || // UTF-8 multibyte
			('0' <= *s && *s <= '9') || //0-9
			('A' <= *s && *s <= 'Z') || //ABC...XYZ
			('a' <= *s && *s <= 'z') || //abc...xyz
			*s == '~' || *s == '-' || *s == '_' 	|| *s == '.' || *s == ' ') 
			outputLen++;
		else 
			outputLen += 3;

	char *szOutput = (char*)mir_alloc(outputLen + 1);
	if (szOutput == NULL)
		return NULL;

	char *d = szOutput;
	for (s = (const BYTE*)szUrl; *s; s++)
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

ULONG AsyncHttpRequest::m_reqCount = 0;

AsyncHttpRequest::AsyncHttpRequest()
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	m_bApiReq = true;
	AddHeader("Connection", "keep-alive");
	AddHeader("Accept-Encoding", "booo");
	pUserInfo = NULL;
	m_iRetry = MAX_RETRIES;
	bNeedsRestart = false;
	bIsMainConn = false;
	m_pFunc = NULL;
	bExpUrlEncode = false;
	m_reqNum = ::InterlockedIncrement(&m_reqCount);
	m_priority = rpLow;
}

AsyncHttpRequest::AsyncHttpRequest(CVkProto *ppro, int iRequestType, LPCSTR _url, bool bSecure, VK_REQUEST_HANDLER pFunc, RequestPriority rpPriority)
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	m_bApiReq = true;
	bIsMainConn = false;
	bExpUrlEncode = ppro->m_bUseNonStandardUrlEncode;
	AddHeader("Connection", "keep-alive");
	AddHeader("Accept-Encoding", "booo");

	flags = VK_NODUMPHEADERS | NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (bSecure)
		flags |= NLHRF_SSL;

	if (*_url == '/') {	// relative url leads to a site
		m_szUrl = ((bSecure) ? "https://" : "http://") + CMStringA("api.vk.com");
		m_szUrl += _url;
		bIsMainConn = true;
	}
	else m_szUrl = _url;

	if (bSecure)
		this << CHAR_PARAM("access_token", ppro->m_szAccessToken);

	requestType = iRequestType;
	m_pFunc = pFunc;
	pUserInfo = NULL;
	m_iRetry = MAX_RETRIES;
	bNeedsRestart = false;
	m_reqNum = ::InterlockedIncrement(&m_reqCount);
	m_priority = rpPriority;
}

AsyncHttpRequest::~AsyncHttpRequest()
{
	for (int i = 0; i < headersCount; i++) {
		mir_free(headers[i].szName);
		mir_free(headers[i].szValue);
	}
	mir_free(headers);
	mir_free(pData);
}

void AsyncHttpRequest::AddHeader(LPCSTR szName, LPCSTR szValue)
{
	headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
	headers[headersCount].szName = mir_strdup(szName);
	headers[headersCount].szValue = mir_strdup(szValue);
	headersCount++;
}

void AsyncHttpRequest::Redirect(NETLIBHTTPREQUEST *nhr)
{
	for (int i = 0; i < nhr->headersCount; i++) {
		LPCSTR szValue = nhr->headers[i].szValue;
		if (!_stricmp(nhr->headers[i].szName, "Location"))
			m_szUrl = szValue;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

TCHAR* CVkProto::GetUserStoredPassword()
{
	debugLogA("CVkProto::GetUserStoredPassword");
	ptrA szRawPass(getStringA("Password"));
	return (szRawPass != NULL) ? mir_utf8decodeT(szRawPass) : NULL;
}

void CVkProto::SetAllContactStatuses(int iStatus)
{
	debugLogA("CVkProto::SetAllContactStatuses (%d)", iStatus);
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			SetChatStatus(hContact, iStatus);
		else if (getWord(hContact, "Status", 0) != iStatus)
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
		return NULL;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		LONG dbUserid = getDword(hContact, "ID", -1);
		if (dbUserid == -1)
			continue;

		if (dbUserid == dwUserid)
			return hContact;
	}

	if (!bCreate)
		return NULL;

	MCONTACT hNewContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hNewContact, (LPARAM)m_szModuleName);
	setDword(hNewContact, "ID", dwUserid);
	db_set_ts(hNewContact, "CList", "Group", m_defaultGroup);
	return hNewContact;
}

MCONTACT CVkProto::FindChat(LONG dwUserid)
{
	if (!dwUserid)
		return NULL;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		LONG dbUserid = getDword(hContact, "vk_chat_id", -1);
		if (dbUserid == -1)
			continue;

		if (dbUserid == dwUserid)
			return hContact;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CVkProto::CheckMid(LIST<void> &lList, int guid)
{
	for (int i = lList.getCount() - 1; i >= 0; i--)
		if ((int)lList[i] == guid) {
			lList.remove(i);
			return true;
		}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

JSONNODE* CVkProto::CheckJsonResponse(AsyncHttpRequest *pReq, NETLIBHTTPREQUEST *reply, JSONROOT &pRoot)
{
	debugLogA("CVkProto::CheckJsonResponse");
	pRoot.Parse(reply->pData);
	if (pRoot == NULL)
		return NULL;

	if (!CheckJsonResult(pReq, pRoot))
		return NULL;

	return json_get(pRoot, "response");
}

bool CVkProto::CheckJsonResult(AsyncHttpRequest *pReq, JSONNODE *pNode)
{
	debugLogA("CVkProto::CheckJsonResult");
	if (pNode == NULL)
		return false;

	JSONNODE *pError = json_get(pNode, "error"), *pErrorCode = json_get(pError, "error_code");
	if (pError == NULL || pErrorCode == NULL)
		return true;

	int iErrorCode = json_as_int(pErrorCode);
	debugLogA("CVkProto::CheckJsonResult %d", iErrorCode);
	CVkFileUploadParam * fup = (CVkFileUploadParam *)pReq->pUserInfo;
	CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
	switch (iErrorCode) {
	case VKERR_AUTHORIZATION_FAILED:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		break;
	case VKERR_ACCESS_DENIED:
		if (time(NULL) - getDword("LastAccessTokenTime", 0) > 60 * 60 * 24) {
			debugLogA("CVkProto::CheckJsonResult VKERR_ACCESS_DENIED (AccessToken fail?)");
			setDword("LastAccessTokenTime", (DWORD)time(NULL));	
			delSetting("AccessToken");
			ShutdownSession();
			return false;
		}
		debugLogA("CVkProto::CheckJsonResult VKERR_ACCESS_DENIED");	
		MsgPopup(NULL, TranslateT("Access denied! Data will not be sent or received."), TranslateT("Error"), true);
		break;
	case VKERR_CAPTCHA_NEEDED:
		ApplyCaptcha(pReq, pError);
		break;
	case VKERR_COULD_NOT_SAVE_FILE:
	case VKERR_INVALID_ALBUM_ID:
	case VKERR_INVALID_SERVER:
	case VKERR_INVALID_HASH:
	case VKERR_INVALID_AUDIO:
	case VKERR_AUDIO_DEL_COPYRIGHT:
	case VKERR_INVALID_FILENAME:
	case VKERR_INVALID_FILESIZE:
		if (fup)
			fup->iErrorCode = iErrorCode;
		break;
	case VKERR_FLOOD_CONTROL:
		pReq->m_iRetry = 0;
	case VKERR_UNKNOWN:
	case VKERR_TOO_MANY_REQ_PER_SEC:
	case VKERR_INTERNAL_SERVER_ERR:
		if (pReq->m_iRetry > 0) {
			pReq->bNeedsRestart = true;
			Sleep(500); //Pause for fix err 
			debugLogA("CVkProto::CheckJsonResult Retry = %d", pReq->m_iRetry);
			pReq->m_iRetry--;
		}
		else {
			CMString msg;
			msg.AppendFormat(TranslateT("Error %d. Data will not be sent or received."), iErrorCode);
			MsgPopup(NULL, msg.GetBuffer(), TranslateT("Error"), true);
			debugLogA("CVkProto::CheckJsonResult SendError");
		}
		break;
	case VKERR_HIMSELF_AS_FRIEND:
	case VKERR_YOU_ON_BLACKLIST:
	case VKERR_USER_ON_BLACKLIST:
		if (param)
			param->iCount = iErrorCode;
		break;
	case VKERR_INVALID_PARAMETERS:
		MsgPopup(NULL, TranslateT("One of the parameters specified was missing or invalid"), TranslateT("Error"), true);
		break;
	case VKERR_ACC_WALL_POST_DENIED:
		MsgPopup(NULL, TranslateT("Access to adding post denied"), TranslateT("Error"), true);
		break;
	}

	return iErrorCode == 0;
}

void CVkProto::OnReceiveSmth(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	ptrT ptszLog(json_as_string(pResponse));
	debugLog(_T("CVkProto::OnReceiveSmth %s"), ptszLog);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Quick & dirty form parser

static CMStringA getAttr(char *szSrc, LPCSTR szAttrName)
{
	char *pEnd = strchr(szSrc, '>');
	if (pEnd == NULL)
		return "";

	*pEnd = 0;

	char *p1 = strstr(szSrc, szAttrName);
	if (p1 == NULL) {
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
	if (p2 == NULL) 
		return "";
	
	return CMStringA(p1, (int)(p2-p1));
}

bool CVkProto::AutoFillForm(char *pBody, CMStringA &szAction, CMStringA& szResult)
{
	debugLogA("CVkProto::AutoFillForm");
	szResult.Empty();

	char *pFormBeg = strstr(pBody, "<form ");
	if (pFormBeg == NULL) return false;

	char *pFormEnd = strstr(pFormBeg, "</form>");
	if (pFormEnd == NULL) return false;

	*pFormEnd = 0;

	szAction = getAttr(pFormBeg, "action");

	CMStringA result;
	char *pFieldBeg = pFormBeg;
	while (true) {
		if ((pFieldBeg = strstr(pFieldBeg+1, "<input ")) == NULL)
			break;

		CMStringA type = getAttr(pFieldBeg, "type");
		if (type != "submit") {
			CMStringA name = getAttr(pFieldBeg, "name");
			CMStringA value = getAttr(pFieldBeg, "value");
			if (name == "email")
				value = (char*)T2Utf(ptrT(getTStringA("Login")));
			else if (name == "pass")
				value = (char*)T2Utf(ptrT(GetUserStoredPassword()));
			else if (name == "captcha_key") {
				char *pCaptchaBeg = strstr(pFormBeg, "<img id=\"captcha\"");
				if (pCaptchaBeg != NULL)
					if (!RunCaptchaForm(getAttr(pCaptchaBeg, "src"), value))
						return false;
			}
			else if (name == "code") 
				value = RunConfirmationCode();

			if (!result.IsEmpty())
				result.AppendChar('&');
			result += name + "=";
			result += ptrA(mir_urlEncode(value));
		}
	}

	szResult = result;
	debugLogA("CVkProto::AutoFillForm result = \"%s\"", szResult.GetBuffer());
	return true;
}

CMString CVkProto::RunConfirmationCode()
{
	ENTER_STRING pForm = { sizeof(pForm) };
	pForm.type = ESF_PASSWORD;
	pForm.caption = TranslateT("Enter confirmation code");
	pForm.ptszInitVal = NULL;
	pForm.szModuleName = m_szModuleName;
	pForm.szDataPrefix = "confirmcode_";
	return (!EnterString(&pForm)) ? CMString() : CMString(ptrT(pForm.ptszResult));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::GrabCookies(NETLIBHTTPREQUEST *nhr)
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

		if (!szCookieName.IsEmpty() && !szDomain.IsEmpty()) {
			int k;
			for (k=0; k < m_cookies.getCount(); k++) {
				if (m_cookies[k].m_name == szCookieName) {
					m_cookies[k].m_value = szCookieVal;
					break;
				}
			}
			if (k == m_cookies.getCount())
				m_cookies.insert(new Cookie(szCookieName, szCookieVal, szDomain));
		}
	}
}

void CVkProto::ApplyCookies(AsyncHttpRequest *pReq)
{
	debugLogA("CVkProto::ApplyCookies");
	CMStringA szCookie;

	for (int i=0; i < m_cookies.getCount(); i++) {
		if (!strstr(pReq->m_szUrl, m_cookies[i].m_domain))
			continue;

		if (!szCookie.IsEmpty())
			szCookie.Append("; ");
		szCookie.Append(m_cookies[i].m_name);
		szCookie.AppendChar('=');
		szCookie.Append(m_cookies[i].m_value);
	}

	if (!szCookie.IsEmpty())
		pReq->AddHeader("Cookie", szCookie);
}

/////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CVkProto::DBAddAuthRequestThread(void *p)
{
	MCONTACT hContact = (MCONTACT)p;
	if (hContact == NULL || hContact == INVALID_CONTACT_ID || !IsOnline())
		return;

	for (int i = 0; i < MAX_RETRIES && IsEmpty(ptrT(db_get_tsa(hContact, m_szModuleName, "Nick"))); i++) {
		Sleep(1500);
		
		if (!IsOnline())
			return;
	}

	DBAddAuthRequest(hContact);
}

void CVkProto::DBAddAuthRequest(const MCONTACT hContact)
{
	debugLogA("CVkProto::DBAddAuthRequest");
		
	T2Utf szNick(ptrT(db_get_tsa(hContact, m_szModuleName, "Nick")));
	T2Utf szFirstName(ptrT(db_get_tsa(hContact, m_szModuleName, "FirstName")));
	T2Utf szLastName(ptrT(db_get_tsa(hContact, m_szModuleName, "LastName")));
		
	//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
	//blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ)
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = (DWORD)(sizeof(DWORD) * 2 + mir_strlen(szNick) + mir_strlen(szFirstName) + mir_strlen(szLastName) + 5);
	
	PBYTE pCurBlob = dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);

	*((PDWORD)pCurBlob) = 0; 
	pCurBlob += sizeof(DWORD); // uin(DWORD) = 0 (DWORD)
	
	*((PDWORD)pCurBlob) = (DWORD)hContact;
	pCurBlob += sizeof(DWORD); // hContact(DWORD)

	mir_strcpy((char*)pCurBlob, szNick); 
	pCurBlob += mir_strlen(szNick) + 1;

	mir_strcpy((char*)pCurBlob, szFirstName);
	pCurBlob += mir_strlen(szFirstName) + 1;

	mir_strcpy((char*)pCurBlob, szLastName);
	pCurBlob += mir_strlen(szLastName) + 1;
	
	*pCurBlob = '\0';	//email
	pCurBlob++;
	*pCurBlob = '\0';	//reason

	db_event_add(NULL, &dbei);
	debugLogA("CVkProto::DBAddAuthRequest %s", szNick ? szNick : "<null>");
}

MCONTACT CVkProto::MContactFromDbEvent(MEVENT hDbEvent)
{
	debugLogA("CVkProto::MContactFromDbEvent");
	if (!hDbEvent || !IsOnline())
		return INVALID_CONTACT_ID;

	DWORD body[2];
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hDbEvent, &dbei))
		return INVALID_CONTACT_ID;
	if (dbei.eventType != EVENTTYPE_AUTHREQUEST || mir_strcmp(dbei.szModule, m_szModuleName))
		return INVALID_CONTACT_ID;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	db_unset(hContact, m_szModuleName, "ReqAuth");
	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::SetMirVer(MCONTACT hContact, int platform)
{
	if (hContact == NULL || hContact == INVALID_CONTACT_ID)
		return;
	if (platform == -1) {
		db_unset(hContact, m_szModuleName, "MirVer");
		return;
	}

	CMString MirVer, OldMirVer;
	OldMirVer = ptrT(db_get_tsa(hContact, m_szModuleName, "MirVer"));
	bool bSetFlag = true;

	switch (platform) {
	case VK_APP_ID:
		MirVer = _T("Miranda NG VKontakte");
		break;
	case 2386311:
		MirVer = _T("QIP 2012 VKontakte");
		break;
	case 1:
		MirVer = _T("VKontakte (mobile)");
		break;
	case 3087106: // iPhone
	case 3140623:
	case 2:
		MirVer = _T("VKontakte (iphone)");
		break;
	case 3682744: // iPad
	case 3:
		MirVer = _T("VKontakte (ipad)");
		break;
	case 2685278: // Android - Kate
	case 2890984: // Android
	case 2274003:
	case 4:
		MirVer = _T("VKontakte (android)");
		break;
	case 3059453: // Windows Phone
	case 2424737:
	case 3502561:
	case 5:
		MirVer = _T("VKontakte (wphone)");
		break;
	case 3584591: // Windows 8.x
	case 6:
		MirVer = _T("VKontakte (windows)");
		break; 
	case 7:
		MirVer = _T("VKontakte (website)");
		break;
	default:
		MirVer = _T("VKontakte (other)");
		bSetFlag = OldMirVer.IsEmpty();
	}

	if (OldMirVer == MirVer)
		return;

	if (bSetFlag)
		setTString(hContact, "MirVer", MirVer.GetBuffer());
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::ContactTypingThread(void *p)
{
	debugLogA("CVkProto::ContactTypingThread");
	MCONTACT hContact = (MCONTACT)p;
	CallService(MS_PROTO_CONTACTISTYPING, hContact, 5);
	Sleep(5500);
	CallService(MS_PROTO_CONTACTISTYPING, hContact, 0);
	Sleep(1500);
	SetSrmmReadStatus(hContact);
}

int CVkProto::OnProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	debugLogA("CVkProto::OnProcessSrmmEvent");
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPENING)
		SetSrmmReadStatus(event->hContact);

	return 0;
}

void CVkProto::SetSrmmReadStatus(MCONTACT hContact)
{
	time_t time = getDword(hContact, "LastMsgReadTime", 0);
	if (!time)
		return;

	TCHAR ttime[64];
	_locale_t locale = _create_locale(LC_ALL, "");
	_tcsftime_l(ttime, SIZEOF(ttime), _T("%X - %x"), localtime(&time), locale);
	_free_locale(locale);

	StatusTextData st = { 0 };
	st.cbSize = sizeof(st);
	st.hIcon = Skin_GetIconByHandle(GetIconHandle(IDI_READMSG));
	mir_sntprintf(st.tszText, SIZEOF(st.tszText), TranslateT("Message read: %s"), ttime);
	CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hContact, (LPARAM)&st);
}

char* CVkProto::GetStickerId(const char* Msg, int &stickerid)
{
	stickerid = 0;
	char* retMsg = NULL;
	
	int iRes = 0;
	char HeadMsg[32] = { 0 };
	const char * tmpMsg = strstr(Msg, "[sticker:");
	if (tmpMsg)
		iRes = sscanf(tmpMsg, "[sticker:%d]", &stickerid);
	if (iRes == 1) {
		mir_snprintf(HeadMsg, SIZEOF(HeadMsg), "[sticker:%d]", stickerid);
		size_t retLen = mir_strlen(HeadMsg);
		if (retLen < mir_strlen(Msg)) {
			CMStringA szMsg(Msg, int(mir_strlen(Msg) - mir_strlen(tmpMsg)));
			szMsg.Append(&tmpMsg[retLen]);
			retMsg = mir_strdup(szMsg.GetBuffer());
		}	
	}

	return retMsg;
}

int CVkProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact != NULL)
		return 0;

	if (mir_strcmp(cws->szModule, "ListeningTo"))
		return 0;
	
	CMStringA szListeningTo(m_szModuleName);
	szListeningTo += "Enabled";
	if (!mir_strcmp(cws->szSetting, szListeningTo.GetBuffer())) {
		MusicSendMetod iOldMusicSendMetod = (MusicSendMetod)getByte("OldMusicSendMetod", sendBroadcastAndStatus);
		
		if (cws->value.bVal == 0)
			setByte("OldMusicSendMetod", m_iMusicSendMetod);
		else
			db_unset(0, m_szModuleName, "OldMusicSendMetod");
		
		m_iMusicSendMetod = cws->value.bVal == 0 ? sendNone : iOldMusicSendMetod;
		setByte("MusicSendMetod", m_iMusicSendMetod);
	}	

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMString CVkProto::SpanVKNotificationType(CMString& tszType, VKObjType& vkFeedback, VKObjType& vkParent)
{
	CVKNotification vkNotification[] = {
		// type, parent, feedback, string for translate
		{ _T("group"), vkInvite, vkNull, TranslateT("has invited you to a group") },
		{ _T("page"), vkInvite, vkNull, TranslateT("has invited you to subscribe to a page") },
		{ _T("event"), vkInvite, vkNull, TranslateT("invites you to event") },

		{ _T("follow"), vkNull, vkUsers, _T("") },
		{ _T("friend_accepted"), vkNull, vkUsers, _T("") },
		{ _T("mention"), vkNull, vkPost, _T("") },
		{ _T("wall"), vkNull, vkPost, _T("") },
		{ _T("wall_publish"), vkNull, vkPost, _T("") },

		{ _T("comment_post"), vkPost, vkComment, TranslateT("commented on your post") },
		{ _T("comment_photo"), vkPhoto, vkComment, TranslateT("commented on your photo") },
		{ _T("comment_video"), vkVideo, vkComment, TranslateT("commented on your video") },
		{ _T("reply_comment"), vkComment, vkComment, TranslateT("replied to your comment") },
		{ _T("reply_comment_photo"), vkComment, vkComment, TranslateT("replied to your comment to photo") },
		{ _T("reply_comment_video"), vkComment, vkComment, TranslateT("replied to your comment to video") },
		{ _T("reply_topic"), vkTopic, vkComment, TranslateT("replied to your topic") },
		{ _T("like_post"), vkPost, vkUsers, TranslateT("liked your post") },
		{ _T("like_comment"), vkComment, vkUsers, TranslateT("liked your comment") },
		{ _T("like_photo"), vkPhoto, vkUsers, TranslateT("liked your photo") },
		{ _T("like_video"), vkVideo, vkUsers, TranslateT("liked your video") },
		{ _T("like_comment_photo"), vkComment, vkUsers, TranslateT("liked your comment to photo") },
		{ _T("like_comment_video"), vkComment, vkUsers, TranslateT("liked your comment to video" ) },
		{ _T("like_comment_topic"), vkComment, vkUsers, TranslateT("liked your comment to topic") },
		{ _T("copy_post"), vkPost, vkCopy, TranslateT("shared your post") },
		{ _T("copy_photo"), vkPhoto, vkCopy, TranslateT("shared your photo") },
		{ _T("copy_video"), vkVideo, vkCopy, TranslateT("shared your video") },
		{ _T("mention_comments"), vkPost, vkComment, _T("mentioned you in comment") },
		{ _T("mention_comment_photo"), vkPhoto, vkComment, _T("mentioned you in comment to photo") }, 
		{ _T("mention_comment_video"), vkVideo, vkComment, _T("mentioned you in comment to video") }
	};

	CMString tszRes;
	vkFeedback = vkParent = vkNull;
	for (int i = 0; i < SIZEOF(vkNotification); i++)
		if (tszType == vkNotification[i].ptszType) {
			vkFeedback = vkNotification[i].vkFeedback;
			vkParent = vkNotification[i].vkParent;
			tszRes = vkNotification[i].ptszTranslate;
			break;
		}
	return tszRes;
}

CMString CVkProto::GetVkPhotoItem(JSONNODE *pPhoto, BBCSupport iBBC)
{
	CMString tszRes;

	if (pPhoto == NULL)
		return tszRes;

	ptrT ptszLink, ptszPreviewLink;
	for (int i = 0; i < SIZEOF(szImageTypes); i++) {
		JSONNODE *n = json_get(pPhoto, szImageTypes[i]);
		if (n != NULL) {
			ptszLink = json_as_string(n);
			break;
		}
	}

	switch (m_iIMGBBCSupport) {
	case imgNo:
		ptszPreviewLink = NULL;
		break;
	case imgFullSize:
		ptszPreviewLink = ptszLink;
		break;
	case imgPreview130:
	case imgPreview604:
		ptszPreviewLink = json_as_string(json_get(pPhoto, m_iIMGBBCSupport == imgPreview130 ? "photo_130" : "photo_604"));
		break;
	}

	int iWidth = json_as_int(json_get(pPhoto, "width"));
	int iHeight = json_as_int(json_get(pPhoto, "height"));

	tszRes.AppendFormat(_T("%s (%dx%d)"), SetBBCString(TranslateT("Photo"), iBBC, vkbbcUrl, ptszLink).GetBuffer(), iWidth, iHeight);
	if (m_iIMGBBCSupport)
		tszRes.AppendFormat(_T("\n\t[img]%s[/img]"), ptszPreviewLink ? ptszPreviewLink : (ptszLink ? ptszLink : _T("")));
	CMString tszText = json_as_CMString(json_get(pPhoto, "text"));
	if (!tszText.IsEmpty())
		tszRes += _T("\n") + tszText;

	return tszRes;
}

CMString CVkProto::SetBBCString(TCHAR *ptszString, BBCSupport iBBC, VKBBCType bbcType, TCHAR *tszAddString)
{
	CVKBBCItem bbcItem[] = {
		{ vkbbcB, bbcNo, _T("%s") },
		{ vkbbcB, bbcBasic, _T("[b]%s[/b]") },
		{ vkbbcB, bbcAdvanced, _T("[b]%s[/b]") },
		{ vkbbcI, bbcNo, _T("%s") },
		{ vkbbcI, bbcBasic, _T("[i]%s[/i]") },
		{ vkbbcI, bbcAdvanced, _T("[i]%s[/i]") },
		{ vkbbcS, bbcNo, _T("%s") },
		{ vkbbcS, bbcBasic, _T("[s]%s[/s]") },
		{ vkbbcS, bbcAdvanced, _T("[s]%s[/s]") },
		{ vkbbcU, bbcNo, _T("%s") },
		{ vkbbcU, bbcBasic, _T("[u]%s[/u]") },
		{ vkbbcU, bbcAdvanced, _T("[u]%s[/u]") },
		{ vkbbcUrl, bbcNo, _T("%s (%s)") },
		{ vkbbcUrl, bbcBasic, _T("[i]%s[/i] (%s)") },
		{ vkbbcUrl, bbcAdvanced, _T("[url=%s]%s[/url]") },
		{ vkbbcSize, bbcNo, _T("%s") },
		{ vkbbcSize, bbcBasic, _T("%s") },
		{ vkbbcSize, bbcAdvanced, _T("[size=%s]%s[/size]") },
		{ vkbbcColor, bbcNo, _T("%s") },
		{ vkbbcColor, bbcBasic, _T("%s") },
		{ vkbbcColor, bbcAdvanced, _T("[color=%s]%s[/color]") },
	};

	if (ptszString == NULL)
		return CMString();

	if (ptszString[0] == '\0')
		return CMString();

	TCHAR *ptszFormat = NULL;
	for (int i = 0; i < SIZEOF(bbcItem); i++)
		if (bbcItem[i].vkBBCType == bbcType && bbcItem[i].vkBBCSettings == iBBC) {
			ptszFormat = bbcItem[i].ptszTempate;
			break;
		}

	CMString res;
	if (ptszFormat == NULL)
		return CMString(ptszString);

	if (bbcType == vkbbcUrl && iBBC != bbcAdvanced)
		res.AppendFormat(ptszFormat, ptszString, tszAddString ? tszAddString : _T(""));
	else if (iBBC == bbcAdvanced && bbcType >= vkbbcUrl)
		res.AppendFormat(ptszFormat, tszAddString ? tszAddString : _T(""), ptszString);
	else
		res.AppendFormat(ptszFormat, ptszString);

	return res;
}

CMString& CVkProto::ClearFormatNick(CMString& tszText)
{
	int iNameEnd = tszText.Find(_T("],")), iNameBeg = tszText.Find(_T("|"));
	if (iNameEnd != -1 && iNameBeg != -1 && iNameBeg < iNameEnd) {
		CMString tszName = tszText.Mid(iNameBeg + 1, iNameEnd - iNameBeg - 1);
		CMString tszBody = tszText.Mid(iNameEnd + 2);
		if (!tszName.IsEmpty() && !tszBody.IsEmpty())
			tszText = tszName + _T(",") + tszBody;
	}

	return tszText;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMString CVkProto::GetAttachmentDescr(JSONNODE *pAttachments, BBCSupport iBBC)
{
	debugLogA("CVkProto::GetAttachmentDescr");
	CMString res;
	if (pAttachments == NULL) {
		debugLogA("CVkProto::GetAttachmentDescr pAttachments == NULL");
		return res;
	}

	res += SetBBCString(TranslateT("Attachments:"), iBBC, vkbbcB);
	res.AppendChar('\n');
	JSONNODE *pAttach;
	for (int k = 0; (pAttach = json_at(pAttachments, k)) != NULL; k++) {
		res.AppendChar('\t');
		ptrT ptszType(json_as_string(json_get(pAttach, "type")));
		if (!mir_tstrcmp(ptszType, _T("photo"))) {
			JSONNODE *pPhoto = json_get(pAttach, "photo");
			if (pPhoto == NULL)
				continue;

			res += GetVkPhotoItem(pPhoto, iBBC);
		}
		else if (!mir_tstrcmp(ptszType, _T("audio"))) {
			JSONNODE *pAudio = json_get(pAttach, "audio");
			if (pAudio == NULL)
				continue;

			ptrT ptszArtist(json_as_string(json_get(pAudio, "artist")));
			ptrT ptszTitle(json_as_string(json_get(pAudio, "title")));
			ptrT ptszUrl(json_as_string(json_get(pAudio, "url")));
			CMString tszAudio;
			tszAudio.AppendFormat(_T("%s - %s"), ptszArtist, ptszTitle);

			res.AppendFormat(_T("%s: %s"),
				SetBBCString(TranslateT("Audio"), iBBC, vkbbcB).GetBuffer(), 
				SetBBCString(tszAudio.GetBuffer(), iBBC, vkbbcUrl, ptszUrl).GetBuffer());
		}
		else if (!mir_tstrcmp(ptszType, _T("video"))) {
			JSONNODE *pVideo = json_get(pAttach, "video");
			if (pVideo == NULL)
				continue;

			ptrT ptszTitle(json_as_string(json_get(pVideo, "title")));
			int vid = json_as_int(json_get(pVideo, "id"));
			int ownerID = json_as_int(json_get(pVideo, "owner_id"));
			CMString tszUrl;
			tszUrl.AppendFormat(_T("http://vk.com/video%d_%d"), ownerID, vid);
			res.AppendFormat(_T("%s: %s"), 
				SetBBCString(TranslateT("Video"), iBBC, vkbbcB).GetBuffer(), 
				SetBBCString(ptszTitle, iBBC, vkbbcUrl, tszUrl.GetBuffer()).GetBuffer());
		}
		else if (!mir_tstrcmp(ptszType, _T("doc"))) {
			JSONNODE *pDoc = json_get(pAttach, "doc");
			if (pDoc == NULL)
				continue;

			ptrT ptszTitle(json_as_string(json_get(pDoc, "title")));
			ptrT ptszUrl(json_as_string(json_get(pDoc, "url")));
			res.AppendFormat(_T("%s: %s"), 
				SetBBCString(TranslateT("Document"), iBBC, vkbbcB).GetBuffer(), 
				SetBBCString(ptszTitle, iBBC, vkbbcUrl, ptszUrl).GetBuffer());
		}
		else if (!mir_tstrcmp(ptszType, _T("wall"))) {
			JSONNODE *pWall = json_get(pAttach, "wall");
			if (pWall == NULL)
				continue;

			ptrT ptszText(json_as_string(json_get(pWall, "text")));
			int id = json_as_int(json_get(pWall, "id"));
			int fromID = json_as_int(json_get(pWall, "from_id"));
			CMString tszUrl;
			tszUrl.AppendFormat(_T("http://vk.com/wall%d_%d"), fromID, id);
			res.AppendFormat(_T("%s: %s"), 
				SetBBCString(TranslateT("Wall post"), iBBC, vkbbcUrl, tszUrl.GetBuffer()).GetBuffer(), 
				ptszText ? ptszText : _T(" "));

			JSONNODE *pSubAttachments = json_get(pWall, "attachments");
			if (pSubAttachments != NULL) {
				debugLogA("CVkProto::GetAttachmentDescr SubAttachments");
				CMString tszAttachmentDescr = GetAttachmentDescr(pSubAttachments, m_iBBCForAttachments);
				tszAttachmentDescr.Replace(_T("\n"), _T("\n\t"));
				res += _T("\n\t") + tszAttachmentDescr;
			}
		}
		else if (!mir_tstrcmp(ptszType, _T("sticker"))) {
			JSONNODE *pSticker = json_get(pAttach, "sticker");
			if (pSticker == NULL)
				continue;
			res.Empty(); // sticker is not really an attachment, so we don't want all that heading info

			if (m_bStikersAsSmyles) {
				int id = json_as_int(json_get(pSticker, "id"));
				res.AppendFormat(_T("[sticker:%d]"), id);
			}
			else {
				ptrT ptszLink;
				for (int i = 0; i < SIZEOF(szImageTypes); i++) {
					JSONNODE *n = json_get(pSticker, szImageTypes[i]);
					if (n != NULL) {
						ptszLink = json_as_string(n);
						break;
					}
				}
				res.AppendFormat(_T("%s"), ptszLink);

				if (m_iIMGBBCSupport)
					res.AppendFormat(_T("[img]%s[/img]"), ptszLink);
			}
		}
		else if (!mir_tstrcmp(ptszType, _T("link"))) {
			JSONNODE *pLink = json_get(pAttach, "link");
			if (pLink == NULL)
				continue;

			ptrT ptszUrl(json_as_string(json_get(pLink, "url")));
			ptrT ptszTitle(json_as_string(json_get(pLink, "title")));
			ptrT ptszDescription(json_as_string(json_get(pLink, "description")));
			ptrT ptszImage(json_as_string(json_get(pLink, "image_src")));

			res.AppendFormat(_T("%s: %s"), 
				SetBBCString(TranslateT("Link"), iBBC, vkbbcB).GetBuffer(), 
				SetBBCString(ptszTitle, iBBC, vkbbcUrl, ptszUrl).GetBuffer());
			if (!IsEmpty(ptszImage))
				if (m_iIMGBBCSupport)
					res.AppendFormat(_T("\n\t%s: [img]%s[/img]"), TranslateT("Image"), ptszImage);
				else
					res.AppendFormat(_T("\n\t%s: %s"), TranslateT("Image"), ptszImage);

			if (ptszDescription)
				res.AppendFormat(_T("\n\t%s"), ptszDescription);
		}
		else if (!mir_tstrcmp(ptszType, _T("gift"))) {
			JSONNODE *pGift = json_get(pAttach, "gift");
			if (pGift == NULL)
				continue;

			ptrT ptszLink;
			for (int i = 0; i < SIZEOF(szGiftTypes); i++) {
				JSONNODE *n = json_get(pGift, szGiftTypes[i]);
				if (n != NULL) {
					ptszLink = json_as_string(n);
					break;
				}
			}
			if (IsEmpty(ptszLink))
				continue;
			res += SetBBCString(TranslateT("Gift"), iBBC, vkbbcUrl, ptszLink);

			if (m_iIMGBBCSupport)
				res.AppendFormat(_T("\n\t[img]%s[/img]"), ptszLink);

		}
		else 
			res.AppendFormat(TranslateT("Unsupported or unknown attachment type: %s"), SetBBCString(ptszType, iBBC, vkbbcB).GetBuffer());

		res.AppendChar('\n');
	}

	return res;
}

CMString CVkProto::GetFwdMessages(JSONNODE *pMessages, BBCSupport iBBC)
{
	CMString res;
	debugLogA("CVkProto::GetFwdMessages");
	if (pMessages == NULL) {
		debugLogA("CVkProto::GetFwdMessages pMessages == NULL");
		return res;
	}

	JSONNODE *pMsg;
	for (int i = 0; (pMsg = json_at(pMessages, i)) != NULL; i++) {
		int uid = json_as_int(json_get(pMsg, "user_id"));
		MCONTACT hContact = FindUser(uid);
		CMString tszNick;
		if (hContact)
			tszNick = ptrT(db_get_tsa(hContact, m_szModuleName, "Nick"));
		if (tszNick.IsEmpty())
			tszNick = TranslateT("(Unknown contact)");
		
		CMString tszUrl = _T("https://vk.com/id");
		tszUrl.AppendFormat(_T("%d"), uid);

		time_t datetime = (time_t)json_as_int(json_get(pMsg, "date"));
		TCHAR ttime[64];
		_locale_t locale = _create_locale(LC_ALL, "");
		_tcsftime_l(ttime, SIZEOF(ttime), _T("%x %X"), localtime(&datetime), locale);
		_free_locale(locale);

		CMString tszBody = json_as_CMString(json_get(pMsg, "body"));

		JSONNODE *pFwdMessages = json_get(pMsg, "fwd_messages");
		if (pFwdMessages != NULL) {
			CMString tszFwdMessages = GetFwdMessages(pFwdMessages, m_iBBCForAttachments);
			if (!tszBody.IsEmpty())
				tszFwdMessages = _T("\n") + tszFwdMessages;
			tszBody += tszFwdMessages;
		}
		
		JSONNODE *pAttachments = json_get(pMsg, "attachments");
		if (pAttachments != NULL) {
			CMString tszAttachmentDescr = GetAttachmentDescr(pAttachments, m_iBBCForAttachments);
			if (!tszBody.IsEmpty())
				tszAttachmentDescr = _T("\n") + tszAttachmentDescr;
			tszBody +=  tszAttachmentDescr;
		}

		tszBody.Replace(_T("\n"), _T("\n\t"));
		CMString tszMes;
		tszMes.AppendFormat(_T("%s %s %s %s:\n%s"),
			SetBBCString(TranslateT("Message from"), iBBC, vkbbcB).GetBuffer(),
			SetBBCString(tszNick.GetBuffer(), iBBC, vkbbcUrl, tszUrl.GetBuffer()).GetBuffer(),
			SetBBCString(TranslateT("at"), iBBC, vkbbcB).GetBuffer(),
			ttime,
			tszBody.GetBuffer());

		if (!res.IsEmpty())
			res.AppendChar(_T('\n'));
		res += tszMes;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CVkProto::SetInvisible(MCONTACT hContact)
{
	debugLogA("CVkProto::SetInvisible %d", getDword(hContact, "ID", -1));
	if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE) {
		setWord(hContact, "Status", ID_STATUS_INVISIBLE);
		SetMirVer(hContact, 1);
		debugLogA("CVkProto::SetInvisible %d set ID_STATUS_INVISIBLE", getDword(hContact, "ID", -1));
	}
	setDword(hContact, "InvisibleTS", time(NULL));
}