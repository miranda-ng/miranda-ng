/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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

TCHAR* CVkProto::GetUserStoredPassword()
{
	ptrA szRawPass( getStringA("Password"));
	return (szRawPass != NULL) ? mir_utf8decodeT(szRawPass) : NULL;
}

void CVkProto::SetAllContactStatuses(int iStatus)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			SetChatStatus(hContact, iStatus);
		else if (getWord(hContact, "Status", 0) != iStatus)
			setWord(hContact, "Status", iStatus);
	}
}

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

bool CVkProto::CheckMid(int guid)
{
	for (int i=m_sendIds.getCount()-1; i >= 0; i--)
		if ((int)m_sendIds[i] == guid) {
			m_sendIds.remove(i);
			return true;
		}

	return false;
}

LPCSTR findHeader(NETLIBHTTPREQUEST *pReq, LPCSTR szField)
{
	for (int i=0; i < pReq->headersCount; i++)
		if (!_stricmp(pReq->headers[i].szName, szField))
			return pReq->headers[i].szValue;

	return NULL;
}

JSONNODE* CVkProto::CheckJsonResponse(AsyncHttpRequest *pReq, NETLIBHTTPREQUEST *reply, JSONROOT &pRoot)
{
	pRoot.Parse(reply->pData);
	if (pRoot == NULL)
		return NULL;

	if (!CheckJsonResult(pReq, reply, pRoot))
		return NULL;

	return json_get(pRoot, "response");
}

bool CVkProto::CheckJsonResult(AsyncHttpRequest *pReq, NETLIBHTTPREQUEST *reply, JSONNODE *pNode)
{
	if (pNode == NULL)
		return false;

	JSONNODE *pError = json_get(pNode, "error"), *pErrorCode = json_get(pError, "error_code");
	if (pError == NULL || pErrorCode == NULL)
		return true;

	int iErrorCode = json_as_int(pErrorCode);
	debugLogA("CVkProto::CheckJsonResult %d", iErrorCode);
	switch (iErrorCode){
	case VKERR_AUTHORIZATION_FAILED:
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
		break;
	case VKERR_CAPTCHA_NEEDED:
		ApplyCaptcha(pReq, pError);
		break;
	case VKERR_UNKNOWN:
	case VKERR_TOO_MANY_REQ_PER_SEC:
	case VKERR_FLOOD_CONTROL:
	case VKERR_INTERNAL_SERVER_ERR:
		if (pReq->m_iRetry > 0){
			pReq->bNeedsRestart = true;
			Sleep(500); //Pause for fix err 
			pReq->m_iRetry--;
		}
		else{
			CMString msg, msgformat = TranslateT("Error %d. Data will not be sent or received.");
			msg.AppendFormat(msgformat, iErrorCode);
			MsgPopup(NULL, msg.GetBuffer(), TranslateT("Error"), true);
		}
		break;
	case VKERR_HIMSELF_AS_FRIEND:
	case VKERR_YOU_ON_BLACKLIST:
	case VKERR_USER_ON_BLACKLIST:
		CVkSendMsgParam *param = (CVkSendMsgParam*)pReq->pUserInfo;
		if (param)
			param->iCount = iErrorCode;
		break;
	}

	return iErrorCode == 0;
}

void CVkProto::OnReceiveSmth(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	JSONROOT pRoot;
	JSONNODE *pResponse = CheckJsonResponse(pReq, reply, pRoot);
	debugLogA("CVkProto::OnReceiveSmth %s", json_as_string(pResponse));
}


/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Captcha form icon"), "key",  IDI_KEYS }
};

void InitIcons()
{
	Icon_Register(hInst, LPGEN("Protocols")"/"LPGEN("VKontakte"), iconList, SIZEOF(iconList), "VKontakte");
}

HANDLE GetIconHandle(int iCommand)
{
	for (int i=0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iCommand)
			return iconList[i].hIcolib;

	return 0;
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
LBL_NotFound:
		*pEnd = '>';
		return "";
	}
	p1 += strlen(szAttrName);
	if (p1[0] != '=' || p1[1] != '\"')
		goto LBL_NotFound;

	p1 += 2;
	char *p2 = strchr(p1, '\"');
	if (p2 == NULL) 
		goto LBL_NotFound;

	*pEnd = '>';
	return CMStringA(p1, (int)(p2-p1));
}

bool CVkProto::AutoFillForm(char *pBody, CMStringA &szAction, CMStringA& szResult)
{
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
				value = ptrA( mir_utf8encodeT( ptrT( getTStringA("Login"))));
			else if (name == "pass")
				value = ptrA( mir_utf8encodeT( ptrT( GetUserStoredPassword())));
			else if (name == "captcha_key") {
				char *pCaptchaBeg = strstr(pFormBeg, "<img id=\"captcha\"");
				if (pCaptchaBeg != NULL)
					if (!RunCaptchaForm( getAttr(pCaptchaBeg, "src"), value))
						return false;
			}

			if (!result.IsEmpty())
				result.AppendChar('&');
			result += name + "=";
			result += ptrA( mir_urlEncode(value));
		}
	}

	szResult = result;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

AsyncHttpRequest::AsyncHttpRequest()
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	m_bApiReq = true;
	AddHeader("Connection", "keep-alive");
	AddHeader("Accept-Encoding", "booo");
	pUserInfo = NULL;
	m_iRetry = MAX_RETRIES;
}

AsyncHttpRequest::AsyncHttpRequest(CVkProto *ppro, int iRequestType, LPCSTR _url, bool bSecure, VK_REQUEST_HANDLER pFunc)
{
	cbSize = sizeof(NETLIBHTTPREQUEST);
	m_bApiReq = true;
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
}

AsyncHttpRequest::~AsyncHttpRequest()
{
	for (int i=0; i < headersCount; i++) {
		mir_free(headers[i].szName);
		mir_free(headers[i].szValue);
	}
	mir_free(headers);
	mir_free(pData);
}

void AsyncHttpRequest::AddHeader(LPCSTR szName, LPCSTR szValue)
{
	headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount+1));
	headers[headersCount].szName = mir_strdup(szName);
	headers[headersCount].szValue = mir_strdup(szValue);
	headersCount++;
}

void AsyncHttpRequest::Redirect(NETLIBHTTPREQUEST *nhr)
{
	for (int i=0; i < nhr->headersCount; i++) {
		LPCSTR szValue = nhr->headers[i].szValue;
		if (!_stricmp(nhr->headers[i].szName, "Location"))
			m_szUrl = szValue;
	}
}

void CVkProto::GrabCookies(NETLIBHTTPREQUEST *nhr)
{
	for (int i=0; i < nhr->headersCount; i++) {
		if ( _stricmp(nhr->headers[i].szName, "Set-cookie"))
			continue;

		CMStringA szValue = nhr->headers[i].szValue, szCookieName, szCookieVal, szDomain;
		int iStart = 0;
		while (true) {
			bool bFirstToken = iStart == 0;
			CMStringA szToken = szValue.Tokenize(";", iStart).Trim();
			if (iStart == -1)
				break;

			if (bFirstToken) {
				int iStart2 = 0;
				szCookieName = szToken.Tokenize("=", iStart2);
				szCookieVal  = szToken.Tokenize("=", iStart2);
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
				m_cookies.insert( new Cookie(szCookieName, szCookieVal, szDomain));
		}
	}
}

void CVkProto::ApplyCookies(AsyncHttpRequest *pReq)
{
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

void CVkProto::DBAddAuthRequest(const MCONTACT hContact)
{
	//char* szJid = mir_utf8encodeT(jid);
	CMString tszNick = db_get_sa(hContact, m_szModuleName, "Nick");
	char* szNick = mir_utf8encodeT(tszNick.GetBuffer());

	//blob is: uin(DWORD), hContact(DWORD), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
	//blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ)
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob = (DWORD)(sizeof(DWORD) * 2 + strlen(szNick) + 5);
	
	PBYTE pCurBlob = dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);

	*((PDWORD)pCurBlob) = 0; 
	pCurBlob += sizeof(DWORD); // uin(DWORD) = 0 (DWORD)
	
	*((PDWORD)pCurBlob) = (DWORD)hContact;  
	pCurBlob += sizeof(DWORD); // hContact(DWORD)

	strcpy((char*)pCurBlob, szNick); 
	pCurBlob += strlen(szNick) + 1;

	*pCurBlob = '\0';	//firstName
	pCurBlob++;
	*pCurBlob = '\0';	//lastName 
	pCurBlob++;
	*pCurBlob = '\0';	//email
	pCurBlob++;
	*pCurBlob = '\0';	//reason

	db_event_add(NULL, &dbei);
	debugLogA("CVkProto::DBAddAuthRequest '%s'", szNick);

	mir_free(szNick);
}

MCONTACT CVkProto::MContactFromDbEvent(HANDLE hDbEvent)
{
	if (!hDbEvent || !IsOnline())
		return (MCONTACT)-1;

	DWORD body[2];
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hDbEvent, &dbei))
		return (MCONTACT)-1;
	if ((dbei.eventType != EVENTTYPE_AUTHREQUEST) || (strcmp(dbei.szModule, m_szModuleName)))
		return (MCONTACT)-1;

	MCONTACT hContact = DbGetAuthEventContact(&dbei);
	db_unset(hContact, m_szModuleName, "ReqAuth");
	return hContact;
}

void CVkProto::SetMirVer(MCONTACT hContact, int platform)
{
	if (hContact == NULL || hContact == -1)
		return;
	if (platform == -1){
		db_unset(hContact, m_szModuleName, "MirVer");
		return;
	}

	CMString MirVer, OldMirVer;
	OldMirVer = db_get_sa(hContact, m_szModuleName, "MirVer");
	bool bSetFlag = true;

	switch (platform){
	case VK_APP_ID:
		MirVer = "Miranda NG VKontakte";
		break;
	case 2386311:
		MirVer = "QIP 2012";
		break;
	case 1:
		MirVer = "VKontakte (mobile)";
		break;
	case 3087106: // iPhone
	case 2:
		MirVer = "VKontakte (iphone)";
		break;
	case 3682744: // iPad
	case 3:
		MirVer = "VKontakte (ipad)";
		break;
	case 2890984: // Android
	case 2274003:
	case 4:
		MirVer = "VKontakte (android)";
		break;
	case 3059453: // Windows Phone
	case 2424737:
	case 3502561:
	case 5:
		MirVer = "VKontakte (wphone)";
		break;
	case 3584591: // Windows 8.x
	case 6:
		MirVer = "VKontakte (windows)";
		break; 
	case 7:
		MirVer = "VKontakte (website)";
		break;
	default:
		MirVer = "VKontakte (other)";
		bSetFlag = OldMirVer.IsEmpty();
	}

	if (OldMirVer == MirVer)
		return;

	if (bSetFlag)
		setTString(hContact, "MirVer", MirVer.GetBuffer());
}

bool tlstrstr(TCHAR* _s1, TCHAR* _s2)
{
	TCHAR s1[1024], s2[1024];
	mir_sntprintf(s1, SIZEOF(s1), _T("%s"), _s1);
	mir_sntprintf(s2, SIZEOF(s2), _T("%s"), _s2);
	CharLowerBuff(s1, SIZEOF(s1));
	CharLowerBuff(s2, SIZEOF(s2));
	return (_tcsstr(s1, s2) != NULL);
}