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
	if (iErrorCode == ERROR_ACCESS_DENIED)
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
	else if (iErrorCode == 14) // captcha
		ApplyCaptcha(pReq, pError);
	return iErrorCode == 0;
}

void CVkProto::OnReceiveSmth(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	JSONROOT pRoot;
	CheckJsonResponse(pReq, reply, pRoot);
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

	AddHeader("Connection", "keep-alive");
	AddHeader("Accept-Encoding", "booo");
}

AsyncHttpRequest::AsyncHttpRequest(CVkProto *ppro, int iRequestType, LPCSTR _url, bool bSecure, VK_REQUEST_HANDLER pFunc)
{
	cbSize = sizeof(NETLIBHTTPREQUEST);

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
