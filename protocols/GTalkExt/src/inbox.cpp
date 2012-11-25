//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "stdafx.h"
#include "inbox.h"
#include "notifications.h"
#include "db.h"
#include "options.h"

static const LPTSTR COMMON_GMAIL_HOST1 = _T("gmail.com");
static const LPTSTR COMMON_GMAIL_HOST2 = _T("googlemail.com");

static const LPSTR AUTH_REQUEST_URL = "https://www.google.com/accounts/ClientAuth";
static const LPSTR AUTH_REQUEST_PARAMS = "Email=%s&Passwd=%s&"
	"accountType=HOSTED_OR_GOOGLE&"
	"skipvpage=true&"
	"PersistentCookie=false";

static const LPSTR ISSUE_TOKEN_REQUEST_URL = "https://www.google.com/accounts/IssueAuthToken";
static const LPSTR ISSUE_TOKEN_REQUEST_PARAMS = "SID=%s&LSID=%s&"
	"Session=true&"
	"skipvpage=true&"
	"service=gaia";

static const LPSTR TOKEN_AUTH_URL = "https://www.google.com/accounts/TokenAuth?"\
	"auth=%s&"
	"service=mail&"
	"continue=%s&"
	"source=googletalk";


static const NETLIBHTTPHEADER HEADER_URL_ENCODED = {"Content-Type", "application/x-www-form-urlencoded"};
static const int HTTP_OK = 200;

static const LPSTR SID_KEY_NAME = "SID=";
static const LPSTR LSID_KEY_NAME = "LSID=";

static const LPSTR LOGIN_PASS_SETTING_NAME = "LoginPassword";

static const LPTSTR INBOX_URL_FORMAT = _T("https://mail.google.com/%s%s/#inbox");

static const DWORD SIZE_OF_JABBER_OPTIONS = 243 * sizeof(DWORD);

// 3 lines from netlib.h
#define GetNetlibHandleType(h)  (h?*(int*)h:NLH_INVALID)
#define NLH_INVALID      0
#define NLH_USER         'USER'

char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

char *url_encode(char *str) {
  char *pstr = str, *buf = (char*)malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == ',' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

LPSTR HttpPost(HANDLE hUser, LPSTR reqUrl, LPSTR reqParams)
{
	NETLIBHTTPREQUEST nlhr = {0};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_POST;
	nlhr.flags = NLHRF_GENERATEHOST | NLHRF_SMARTAUTHHEADER | NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP | NLHRF_NODUMPHEADERS;
	nlhr.szUrl = reqUrl;
	nlhr.headers = (NETLIBHTTPHEADER*)&HEADER_URL_ENCODED;
	nlhr.headersCount = 1;
	nlhr.pData = reqParams;
	nlhr.dataLength = lstrlenA(reqParams);

	NETLIBHTTPREQUEST *pResp = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hUser, (LPARAM)&nlhr);
	if (!pResp) return NULL;
	__try {
		if (HTTP_OK == pResp->resultCode)
			return _strdup(pResp->pData);
		else
			return NULL;
	}
	__finally {
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pResp);
	}
}

LPSTR MakeRequest(HANDLE hUser, LPSTR reqUrl, LPSTR reqParamsFormat, LPSTR p1, LPSTR p2)
{
	LPSTR encodedP1 = url_encode(p1);
	__try {
		LPSTR encodedP2 = url_encode(p2);
		__try {
			LPSTR reqParams = (LPSTR)malloc(lstrlenA(reqParamsFormat) + 1 + lstrlenA(encodedP1) + lstrlenA(encodedP2));
			__try {
				sprintf(reqParams, reqParamsFormat, encodedP1, encodedP2);
				return HttpPost(hUser, reqUrl, reqParams);
			}
			__finally {
				free(reqParams);
			}
		}
		__finally {
			free(encodedP2);
		}
	}
	__finally {
		free(encodedP1);
	}
}

LPSTR FindSid(LPSTR resp, LPSTR *LSID)
{
	LPSTR SID = strstr(resp, SID_KEY_NAME);
	*LSID = strstr(resp, LSID_KEY_NAME);
	if (!SID || !*LSID) return NULL;

	if (SID - 1 == *LSID) SID = strstr(SID + 1, SID_KEY_NAME);
	if (!SID) return NULL;

	SID += lstrlenA(SID_KEY_NAME);
	LPSTR term = strstr(SID, "\n");
	if (term) term[0] = 0;

	*LSID += lstrlenA(LSID_KEY_NAME);
	term = strstr(*LSID, "\n");
	if (term) term[0] = 0;

	return SID;
}

void DoOpenUrl(LPSTR tokenResp, LPSTR url)
{
	LPSTR encodedUrl = url_encode(url);
	__try {
		LPSTR encodedToken = url_encode(tokenResp);
		__try {
			LPSTR composedUrl = (LPSTR)malloc(lstrlenA(TOKEN_AUTH_URL) + 1 + lstrlenA(encodedToken) + lstrlenA(encodedUrl));
			__try {
				sprintf(composedUrl, TOKEN_AUTH_URL, encodedToken, encodedUrl);
				ShellExecuteA(0, NULL, composedUrl, NULL, NULL, SW_SHOW);
			}
			__finally {
				free(composedUrl);
			}
		}
		__finally {
			free(encodedToken);
		}
	}
	__finally {
		free(encodedUrl);
	}
}

BOOL AuthAndOpen(HANDLE hUser, LPSTR url, LPSTR mailbox, LPSTR pwd)
{
	LPSTR authResp = MakeRequest(hUser, AUTH_REQUEST_URL, AUTH_REQUEST_PARAMS, mailbox, pwd);
	if (!authResp) return FALSE;

	__try {
		LPSTR LSID;
		LPSTR SID = FindSid(authResp, &LSID);
		LPSTR tokenResp = MakeRequest(hUser, ISSUE_TOKEN_REQUEST_URL, ISSUE_TOKEN_REQUEST_PARAMS, SID, LSID);
		if (!tokenResp) return FALSE;

		__try {
			DoOpenUrl(tokenResp, url);
			return TRUE;
		}
		__finally {
			free(tokenResp);
		}
	}
	__finally {
		free(authResp);
	}
}

struct OPEN_URL_HEADER {
	LPSTR url;
	LPSTR mailbox;
	LPSTR pwd;
	LPCSTR acc;
};

HANDLE FindNetUserHandle(LPCSTR acc)
{
	IJabberInterface *ji = getJabberApi(acc);
	if (!ji) return NULL;

	PBYTE m_psProto = *(PBYTE*)((PBYTE)ji + sizeof(*ji));   // see CJabberInterface in jabber_proto.h

	PHANDLE pResult = (PHANDLE)(m_psProto +                                 // see CJabberProto in jabber_proto.h
		sizeof(PVOID) +                                                                         // skip vtable ptr
		sizeof(PVOID) +                                                                         // skip m_ThreadInfo
		SIZE_OF_JABBER_OPTIONS);                                                        // skip m_options

	for (int i=0; i < 100; i++) {
		__try {
			if (GetNetlibHandleType(*pResult) == NLH_USER)
				break;
		}
		__except (EXCEPTION_EXECUTE_HANDLER){
		}
		pResult++;
	}

	assert(GetNetlibHandleType(*pResult) == NLH_USER);
	return *pResult;
}

unsigned __stdcall OpenUrlThread(OPEN_URL_HEADER* data)
{
	__try {
		HANDLE hUser = FindNetUserHandle(data->acc);
		if (!hUser || !AuthAndOpen(hUser, data->url, data->mailbox, data->pwd))
			ShellExecuteA(0, NULL, data->url, NULL, NULL, SW_SHOW);
	}
	__finally {
		free(data);
	}
	return 0;
}

void __forceinline DecryptString(LPSTR str, int len)
{
	for (--len; len >= 0; len--)
	{
		const char c = str[len] ^ 0xc3;
		if (c) str[len] = c;
	}
}

int GetMailboxPwd(LPCSTR acc, LPCTSTR mailbox, LPSTR *pwd, int buffSize)
{
	char buff[256];

	DBCONTACTGETSETTING cgs;
	DBVARIANT dbv;
	cgs.szModule = acc;
	cgs.szSetting = LOGIN_PASS_SETTING_NAME;
	cgs.pValue = &dbv;
	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = &buff[0];
	dbv.cchVal = sizeof(buff);
	if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, 0, (LPARAM)&cgs))
		return 0;

	int result = dbv.cchVal;

	if (pwd) {
		if (buffSize < result + 1) result = buffSize - 1;
		memcpy(*pwd, &buff, result + 1);
		DecryptString(*pwd, result);
	}

	return result;
}

BOOL OpenUrlWithAuth(LPCSTR acc, LPCTSTR mailbox, LPCTSTR url)
{
	int pwdLen = GetMailboxPwd(acc, mailbox, NULL, 0);
	if (!pwdLen++) return FALSE;

	int urlLen = lstrlen(url) + 1;
	int mailboxLen = lstrlen(mailbox) + 1;

	OPEN_URL_HEADER *data = (OPEN_URL_HEADER*)malloc(sizeof(OPEN_URL_HEADER) + urlLen + mailboxLen + pwdLen);
	__try {
		data->url = (LPSTR)data + sizeof(OPEN_URL_HEADER);
		LPSTR ansi = WtoA(url);
		__try {
			memcpy(data->url, ansi, urlLen);
		}
		__finally {
			free(ansi);
		}

		data->mailbox = data->url + urlLen;
		ansi = WtoA(mailbox);
		__try {
			memcpy(data->mailbox, ansi, mailboxLen);
		}
		__finally {
			free(ansi);
		}

		data->pwd = data->mailbox + mailboxLen;
		if (!GetMailboxPwd(acc, mailbox, &data->pwd, pwdLen)) return FALSE;

		data->acc = acc;

		if (HANDLE h = mir_forkthreadex((pThreadFuncEx)OpenUrlThread, data, NULL)) {
			CloseHandle(h);
			data = NULL;
		}
		else return FALSE;
	}
	__finally {
		free(data);
	}

	return TRUE;
}

unsigned __stdcall ShellExecuteThread(PVOID param)
{
	__try {
		ShellExecute(0, NULL, (LPTSTR)param, NULL, NULL, SW_SHOW);
	}
	__finally {
		free(param);
	}
	return 0;
}

void StartShellExecuteThread(LPCTSTR url)
{
	LPTSTR urlCopy = _tcsdup(url);
	__try {
		if (HANDLE h = mir_forkthreadex(ShellExecuteThread, urlCopy, NULL)) {
			CloseHandle(h);
			urlCopy = NULL;
		}
	}
	__finally {
		free(urlCopy);
	}
}

void OpenUrl(LPCSTR acc, LPCTSTR mailbox, LPCTSTR url)
{
	extern DWORD itlsSettings;
	if (!ReadCheckbox(0, IDC_AUTHONMAILBOX, (DWORD)TlsGetValue(itlsSettings)) ||
		!OpenUrlWithAuth(acc, mailbox, url))
			StartShellExecuteThread(url);
}

LPTSTR CraftInboxUrl(LPTSTR jid)
{
	LPTSTR host = _tcsstr(jid, _T("@")) + 1;

	LPTSTR result = (LPTSTR)malloc((lstrlen(INBOX_URL_FORMAT) + 1 + lstrlen(jid)) * sizeof(TCHAR));
	__try {
		if (lstrcmpi(host, COMMON_GMAIL_HOST1) && lstrcmpi(host, COMMON_GMAIL_HOST2))
			wsprintf(result, INBOX_URL_FORMAT, _T("a/"), host);   // hosted
		else
			wsprintf(result, INBOX_URL_FORMAT, NULL, _T("mail")); // common
	}
	__except (
		free(result),
		EXCEPTION_CONTINUE_SEARCH
	) {}

	return result;
}

void OpenContactInbox(HANDLE hContact)
{
	LPSTR acc = GetContactProto(hContact);
	if (!acc) return;

	DBVARIANT dbv;
	if (!DBGetContactSettingTString(0, acc, "jid", &dbv))
		__try {
			LPTSTR url = CraftInboxUrl(dbv.ptszVal);
			__try {
				OpenUrl(acc, dbv.ptszVal, url);
			}
			__finally {
				free(url);
			}
	}
	__finally {
		DBFreeVariant(&dbv);
	}
}
