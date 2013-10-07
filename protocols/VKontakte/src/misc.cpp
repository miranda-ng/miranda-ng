/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

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
	if (szRawPass != NULL) {
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(szRawPass), szRawPass);
		return mir_utf8decodeT(szRawPass);
	}
	return NULL;
}

void CVkProto::SetAllContactStatuses(int iStatus)
{
	for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		if (getWord(hContact, "Status", 0) != iStatus)
			setWord(hContact, "Status", iStatus);
	}
}

HANDLE CVkProto::FindUser(LPCSTR pUserid, bool bCreate)
{
	for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		ptrA dbUserid(getStringA(hContact, "ID"));
		if (dbUserid == NULL)
			continue;

		if ( !strcmp(dbUserid, pUserid))
			return hContact;
	}

	if (!bCreate)
		return NULL;

	HANDLE hNewContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hNewContact, (LPARAM)m_szModuleName);
	setString(hNewContact, "ID", pUserid);
	db_set_ts(hNewContact, "CList", "Group", m_defaultGroup);
	return hNewContact;
}

LPCSTR findHeader(NETLIBHTTPREQUEST *pReq, LPCSTR szField)
{
	for (int i=0; i < pReq->headersCount; i++)
		if ( !_stricmp(pReq->headers[i].szName, szField))
			return pReq->headers[i].szValue;

	return NULL;
}

bool CVkProto::CheckJsonResult(JSONNODE *pNode)
{
	if (pNode == NULL)
		return false;

	JSONNODE *pError = json_get(pNode, "error"), *pErrorCode = json_get(pError, "error_code");
	if (pError == NULL || pErrorCode == NULL)
		return true;

	int iErrorCode = json_as_int(pErrorCode);
	if (iErrorCode == ERROR_ACCESS_DENIED)
		ConnectionFailed(LOGINERR_WRONGPASSWORD);
	return iErrorCode == 0;
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

CMStringA CVkProto::AutoFillForm(char *pBody, CMStringA &szAction)
{
	char *pFormBeg = strstr(pBody, "<form ");
	if (pFormBeg == NULL) return "";

	char *pFormEnd = strstr(pFormBeg, "</form>");
	if (pFormEnd == NULL) return "";

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

			if ( !result.IsEmpty())
				result.AppendChar('&');
			result += name + "=";
			result += ptrA( mir_urlEncode(value));
		}
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringA loginCookie;

CVkProto::AsyncHttpRequest::AsyncHttpRequest()
{
	cbSize = sizeof(NETLIBHTTPREQUEST);

	AddHeader("Connection", "keep-alive");
}

CVkProto::AsyncHttpRequest::~AsyncHttpRequest()
{
	for (int i=0; i < headersCount; i++) {
		mir_free(headers[i].szName);
		mir_free(headers[i].szValue);
	}
	mir_free(headers);
	mir_free(szUrl);
	mir_free(pData);
}

void CVkProto::AsyncHttpRequest::AddHeader(LPCSTR szName, LPCSTR szValue)
{
	headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount+1));
	headers[headersCount].szName = mir_strdup(szName);
	headers[headersCount].szValue = mir_strdup(szValue);
	headersCount++;
}

void CVkProto::AsyncHttpRequest::Redirect(NETLIBHTTPREQUEST *nhr)
{
	CMStringA szCookie;

	for (int i=0; i < nhr->headersCount; i++) {
		LPCSTR szValue = nhr->headers[i].szValue;
		if (!_stricmp(nhr->headers[i].szName, "Location"))
			replaceStr(szUrl, szValue);
		else if (!_stricmp(nhr->headers[i].szName, "Set-cookie")) {
			if ( strstr(szValue, "login.vk.com")) {
				if (!szCookie.IsEmpty())
					szCookie.Append("; ");

				LPCSTR p = strchr(szValue, ';');
				if (p == NULL)
					szCookie += szValue;
				else
					szCookie.Append(szValue, p-szValue);
			}
		}
	}

	if (!szCookie.IsEmpty())
		loginCookie = szCookie;
}
