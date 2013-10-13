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

HANDLE CVkProto::FindUser(LONG dwUserid, bool bCreate)
{
	for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		LONG dbUserid = getDword(hContact, "ID", -1);
		if (dbUserid == -1)
			continue;

		if (dbUserid == dwUserid)
			return hContact;
	}

	if (!bCreate)
		return NULL;

	HANDLE hNewContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hNewContact, (LPARAM)m_szModuleName);
	setDword(hNewContact, "ID", dwUserid);
	db_set_ts(hNewContact, "CList", "Group", m_defaultGroup);
	return hNewContact;
}

bool CVkProto::CheckMid(int msgid)
{
	for (int i=m_sendIds.getCount()-1; i >= 0; i--)
		if (m_sendIds[i] == (HANDLE)msgid) {
			m_sendIds.remove(i);
			return true;
		}

	return false;
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

static IconItem iconList[] =
{
	{ LPGEN("Captcha form icon"), "key",  IDI_KEYS }
};

void InitIcons()
{
	Icon_Register(hInst, LPGEN("Protocols")"/"LPGEN("VKontakte"), iconList, SIZEOF(iconList), "VKontakte");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Captcha form

struct CAPTCHA_FORM_PARAMS
{
	HBITMAP bmp;
	int w,h;
	char Result[100];
};

static INT_PTR CALLBACK CaptchaFormDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAPTCHA_FORM_PARAMS *params = (CAPTCHA_FORM_PARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG: {
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(iconList[0].hIcolib, TRUE));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(iconList[0].hIcolib));
		params = (CAPTCHA_FORM_PARAMS*)lParam;

		SetDlgItemText(hwndDlg, IDC_INSTRUCTION, TranslateT("Enter the text you see"));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG)params);

		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
		switch(GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_WHITERECT:
		case IDC_INSTRUCTION:
		case IDC_TITLE:
			return (BOOL)GetStockObject(WHITE_BRUSH);
		}
		return NULL;

	case WM_PAINT:
		if (params) {
			PAINTSTRUCT ps;
			HDC hdc, hdcMem;
			RECT rc;

			GetClientRect(hwndDlg, &rc);
			hdc = BeginPaint(hwndDlg, &ps);
			hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ hOld = SelectObject(hdcMem, params->bmp);

			int y = (rc.bottom + rc.top - params->h) / 2;
			int x = (rc.right + rc.left - params->w) / 2;
			BitBlt(hdc, x, y, params->w, params->h, hdcMem, 0,0, SRCCOPY);
			SelectObject(hdcMem, hOld);
			DeleteDC(hdcMem);

			EndPaint(hwndDlg, &ps);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDOK:
			GetDlgItemTextA(hwndDlg, IDC_VALUE, params->Result, SIZEOF(params->Result));
			EndDialog(hwndDlg, 1);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}

CMStringA CVkProto::RunCaptchaForm(LPCSTR szUrl)
{
	debugLogA("CVkProto::RunCaptchaForm: reading picture from %s", szUrl);

	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.szUrl = (LPSTR)szUrl;
	req.flags = NLHRF_HTTP11 | NLHRF_NODUMPHEADERS;
	req.timeout = 60;

	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (reply == NULL)
		return "";

	if (reply->resultCode != 200) {
		debugLogA("CVkProto::RunCaptchaForm: failed with code %d", reply->resultCode);
		return "";
	}

	CAPTCHA_FORM_PARAMS param = { 0 };

	IMGSRVC_MEMIO memio = { 0 };
	memio.iLen = reply->dataLength;
	memio.pBuf = reply->pData;
	memio.fif = FIF_UNKNOWN; /* detect */
	param.bmp = (HBITMAP)CallService(MS_IMG_LOADFROMMEM, (WPARAM)&memio, 0);
	
	BITMAP bmp = {0};
	GetObject(param.bmp, sizeof(bmp), &bmp);
	param.w = bmp.bmWidth;
	param.h = bmp.bmHeight;
	int res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAPTCHAFORM), NULL, CaptchaFormDlgProc, (LPARAM)&param);
	if (res == 0)
		return "";
	
	debugLogA("CVkProto::RunCaptchaForm: user entered text %s", param.Result);
	return param.Result;
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
			else if (name == "captcha_key") {
				char *pCaptchaBeg = strstr(pFormBeg, "<img id=\"captcha\"");
				if (pCaptchaBeg != NULL)
					value = RunCaptchaForm( getAttr(pCaptchaBeg, "src"));
			}

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
