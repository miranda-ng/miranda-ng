/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

HANDLE hNetlibUser = NULL, hNetlibHttp;
BOOL UpdateListFlag = FALSE;

BOOL IsMyContact(HANDLE hContact)
{
	const char *szProto = GetContactProto(hContact);
	return szProto != NULL && strcmp(MODULE, szProto) == 0;
}

VOID NetlibInit()
{
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = TranslateT("NewsAggregator HTTP connection");
	nlu.szSettingsModule = MODULE;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

VOID NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = NULL;
}

static void arrayToHex(BYTE *data, size_t datasz, char *res)
{
	char *resptr = res;
	for (unsigned i = 0; i < datasz ; i++) {
		const BYTE ch = data[i];

		const char ch0 = (char)(ch >> 4);
		*resptr++ = (char)((ch0 <= 9) ? ('0' + ch0) : (('a' - 10) + ch0));

		const char ch1 = (char)(ch & 0xF);
		*resptr++ = (char)((ch1 <= 9) ? ('0' + ch1) : (('a' - 10) + ch1));
	}
	*resptr = '\0';
} 

int GetImageFormat(const TCHAR *ext)
{
	if (!lstrcmp(ext, _T(".jpg")) || !lstrcmp(ext, _T(".jpeg")))
		return PA_FORMAT_JPEG;

	if (!lstrcmp(ext, _T(".png")))
		return PA_FORMAT_PNG;

	if (!lstrcmp(ext, _T(".gif")))
		return PA_FORMAT_GIF;

	if (!lstrcmp(ext, _T(".ico")))
		return PA_FORMAT_ICON;

	if (!lstrcmp(ext, _T(".bmp")))
		return PA_FORMAT_BMP;

	if (!lstrcmp(ext, _T(".swf")))
		return PA_FORMAT_SWF;

	if (!lstrcmp(ext, _T(".xml")))
		return PA_FORMAT_XML;

	return PA_FORMAT_UNKNOWN;
}

void CreateAuthString(char *auth, HANDLE hContact, HWND hwndDlg)
{
	DBVARIANT dbv;
	char *user = NULL, *pass = NULL;
	TCHAR *tlogin = NULL, *tpass = NULL, buf[MAX_PATH] = {0};
	if (hContact && db_get_b(hContact, MODULE, "UseAuth", 0)) {
		if (!db_get_ts(hContact, MODULE, "Login", &dbv)) {
			tlogin = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		}
		if (!db_get_ts(hContact, MODULE, "Password", &dbv)) {
			tpass = mir_tstrdup(dbv.ptszVal);
			db_free(&dbv);
		}
	}
	else if (hwndDlg && IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
		GetDlgItemText(hwndDlg, IDC_LOGIN, buf, SIZEOF(buf));
		tlogin = buf;
		GetDlgItemText(hwndDlg, IDC_PASSWORD, buf, SIZEOF(buf));
		tpass = buf;
	}
	user = mir_t2a(tlogin);
	pass = mir_t2a(tpass);

	char str[MAX_PATH];
	int len = mir_snprintf(str, SIZEOF(str), "%s:%s", user, pass);
	mir_free(user);
	mir_free(pass);
	mir_free(tlogin);
	mir_free(tpass);

	mir_snprintf(auth, 250, "Basic %s", ptrA(mir_base64_encode((PBYTE)str, len)));
}

VOID GetNewsData(TCHAR *tszUrl, char **szData, HANDLE hContact, HWND hwndDlg)
{
	char *szRedirUrl  = NULL;
	NETLIBHTTPREQUEST nlhr = {0};
	NETLIBHTTPHEADER headers[5];

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_HTTP11;
	char *szUrl = mir_t2a(tszUrl);
	nlhr.szUrl = szUrl;
	nlhr.nlc = hNetlibHttp;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	nlhr.headers = headers;
	nlhr.headers[0].szName  = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	nlhr.headers[1].szName  = "Cache-Control";
	nlhr.headers[1].szValue = "no-cache";
	nlhr.headers[2].szName  = "Pragma";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName  = "Connection";
	nlhr.headers[3].szValue = "close";
	nlhr.headers[4].szName  = "Accept";
	nlhr.headers[4].szValue = "*/*";
	if (db_get_b(hContact, MODULE, "UseAuth", 0) || IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
		nlhr.headersCount = 6;
		nlhr.headers[5].szName  = "Authorization";
	
		char auth[256];
		CreateAuthString(auth, hContact, hwndDlg);
		nlhr.headers[5].szValue = auth;
	}
	else nlhr.headersCount = 5;

	// download the page
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&nlhr);
	if (nlhrReply) {
		// if the recieved code is 200 OK
		switch (nlhrReply->resultCode) {
		case 200: 
			if (nlhrReply->dataLength) {
				// allocate memory and save the retrieved data
				*szData = (char *)mir_alloc(nlhrReply->dataLength + 2);
				memcpy(*szData, nlhrReply->pData, nlhrReply->dataLength);
				(*szData)[nlhrReply->dataLength] = 0;
			}
			break;

		case 401:
			//ShowMessage(0, TranslateT("Cannot upload VersionInfo. Incorrect username or password"));
			break;

		case 301:
		case 302:
		case 307:
			// get the url for the new location and save it to szInfo
			// look for the reply header "Location"
			for (int i = 0; i < nlhrReply->headersCount; i++) {
				if (!strcmp(nlhrReply->headers[i].szName, "Location")) {
					size_t rlen = 0;
					if (nlhrReply->headers[i].szValue[0] == '/') {
						const char *szPath;
						const char *szPref = strstr(szUrl, "://");
						szPref = szPref ? szPref + 3 : szUrl;
						szPath = strchr(szPref, '/');
						rlen = szPath != NULL ? szPath - szUrl : strlen(szUrl); 
					}

					szRedirUrl = (char *)mir_realloc(szRedirUrl, rlen + strlen(nlhrReply->headers[i].szValue) * 3 + 1);

					strncpy(szRedirUrl, szUrl, rlen);
					strcpy(szRedirUrl + rlen, nlhrReply->headers[i].szValue); 

					nlhr.szUrl = szRedirUrl;
					break;
				}
			}
			break;

		default:
			//ShowMessage(0, TranslateT("Cannot upload VersionInfo. Unknown error"));
			break;
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	mir_free(szUrl);
}

VOID CreateList(HWND hwndList)
{
	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);	

	LVCOLUMN lvc = {0}; 
	// Initialize the LVCOLUMN structure.
	// The mask specifies that the format, width, text, and
	// subitem members of the structure are valid. 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	lvc.fmt = LVCFMT_LEFT;
	  
	lvc.iSubItem = 0;
	lvc.pszText = TranslateT("Feed");	
	lvc.cx = 160;     // width of column in pixels
	ListView_InsertColumn(hwndList, 0, &lvc);

	lvc.iSubItem = 1;
	lvc.pszText = TranslateT("URL");	
	lvc.cx = 276;     // width of column in pixels
	ListView_InsertColumn(hwndList, 1, &lvc);
}

VOID UpdateList(HWND hwndList)
{
	LVITEM lvI = {0};

	// Some code to create the list-view control.
	// Initialize LVITEM members that are common to all
	// items.
	int i = 0;
	for (HANDLE hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
		UpdateListFlag = TRUE;
		lvI.mask = LVIF_TEXT;
		lvI.iSubItem = 0;
		DBVARIANT dbNick = {0};
		if (!db_get_ts(hContact, MODULE, "Nick", &dbNick)) {
			lvI.pszText = dbNick.ptszVal;
			lvI.iItem = i;
			ListView_InsertItem(hwndList, &lvI);
			lvI.iSubItem = 1;
			DBVARIANT dbURL = {0};
			if (!db_get_ts(hContact, MODULE, "URL", &dbURL)) {
				lvI.pszText = dbURL.ptszVal;
				ListView_SetItem(hwndList, &lvI);
				i += 1;
				ListView_SetCheckState(hwndList, lvI.iItem, db_get_b(hContact, MODULE, "CheckState", 1));
				db_free(&dbURL);
			}
			db_free(&dbNick);
		}
	}
	UpdateListFlag = FALSE;
}

VOID DeleteAllItems(HWND hwndList)
{	
	ListView_DeleteAllItems(hwndList);
}

time_t __stdcall DateToUnixTime(TCHAR *stamp, BOOL FeedType)
{
	struct tm timestamp;
	TCHAR date[9];
	int i, y;
	time_t t;

	if (stamp == NULL)
		return 0;

	TCHAR *p = stamp;

	if (FeedType) {
		// skip '-' chars
		int si = 0, sj = 0;
		while (true) {
			if (p[si] == _T('-'))
				si++;
			else if ( !(p[sj++] = p[si++]))
				break;
		}
	}
	else {
		TCHAR weekday[4], monthstr[4], timezonesign[2];
		INT day, month, year, hour, min, sec, timezoneh, timezonem;
		if (_tcsstr(p, _T(","))) {
			_stscanf( p, _T("%3s, %d %3s %d %d:%d:%d %1s%02d%02d"), &weekday, &day, &monthstr, &year, &hour, &min, &sec, &timezonesign, &timezoneh, &timezonem);
			if (!lstrcmpi(monthstr, _T("Jan")))
				month = 1;
			if (!lstrcmpi(monthstr, _T("Feb")))
				month = 2;
			if (!lstrcmpi(monthstr, _T("Mar")))
				month = 3;
			if (!lstrcmpi(monthstr, _T("Apr")))
				month = 4;
			if (!lstrcmpi(monthstr, _T("May")))
				month = 5;
			if (!lstrcmpi(monthstr, _T("Jun")))
				month = 6;
			if (!lstrcmpi(monthstr, _T("Jul")))
				month = 7;
			if (!lstrcmpi(monthstr, _T("Aug")))
				month = 8;
			if (!lstrcmpi(monthstr, _T("Sep")))
				month = 9;
			if (!lstrcmpi(monthstr, _T("Oct")))
				month = 10;
			if (!lstrcmpi(monthstr, _T("Nov")))
				month = 11;
			if (!lstrcmpi(monthstr, _T("Dec")))
				month = 12;
			if (!lstrcmp(timezonesign, _T("+")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour-timezoneh, min-timezonem, sec);
			else if (!lstrcmp(timezonesign, _T("-")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour+timezoneh, min+timezonem, sec);
			else
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour, min, sec);
		}
		else
		{
			_stscanf( p, _T("%d-%d-%d %d:%d:%d %1s%02d%02d"), &year, &month, &day,  &hour, &min, &sec, &timezonesign, &timezoneh, &timezonem);
			if (!lstrcmp(timezonesign, _T("+")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour-timezoneh, min-timezonem, sec);
			else if (!lstrcmp(timezonesign, _T("-")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour+timezoneh, min+timezonem, sec);
			else
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour, min, sec);
		}
	}
	// Get the date part
	for (i = 0; *p != '\0' && i < 8 && isdigit(*p); p++, i++)
		date[i] = *p;

	// Parse year
	if (i == 6) {
		// 2-digit year ( 1970-2069 )
		y = (date[0] - '0' ) * 10 + (date[1] - '0');
		if (y < 70)
			y += 100;
	}
	else if (i == 8) {
		// 4-digit year
		y = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0';
		y -= 1900;
	}
	else return 0;

	timestamp.tm_year = y;
	// Parse month
	timestamp.tm_mon = (date[i - 4] - '0') * 10 + date[i - 3] - '0' - 1;
	// Parse date
	timestamp.tm_mday = (date[i - 2] - '0') * 10 + date[i - 1] - '0';

	// Skip any date/time delimiter
	for (; *p != '\0' && !isdigit(*p); p++);

	// Parse time
	if (_stscanf(p, _T("%d:%d:%d"), &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
		return 0;

	timestamp.tm_isdst = 0;	// DST is already present in _timezone below
	t = mktime(&timestamp);

	_tzset();
	t -= _timezone;
	return (t >= 0) ? t : 0;
}

TCHAR * _tcsistr(const TCHAR *str, const TCHAR *substr)
{
	if (!str || !substr || (substr[0] == _T('\0')))
		return (TCHAR *)str;

	size_t nLen = _tcslen(substr);
	while (*str)
	{
		if (_tcsnicmp(str, substr, nLen) == 0)
			break;
		str++;
	}

	if (*str == _T('\0'))
		str = NULL;

	return (TCHAR *)str;
}

int StrReplace(TCHAR *lpszOld, TCHAR *lpszNew, TCHAR *&lpszStr)
{
	if (!lpszStr || !lpszOld || !lpszNew)
		return 0;

	size_t nStrLen = _tcslen(lpszStr);
	if (nStrLen == 0)
		return 0;

	size_t nOldLen = _tcslen(lpszOld);
	if (nOldLen == 0)
		return 0;

	size_t nNewLen = _tcslen(lpszNew);

	// loop once to figure out the size of the result string
	int nCount = 0;
	TCHAR *pszStart = (TCHAR *)lpszStr;
	TCHAR *pszEnd = (TCHAR *)lpszStr + nStrLen;
	TCHAR *pszTarget = NULL;
	TCHAR *pszResultStr = NULL;

	while (pszStart < pszEnd) {
		while ((pszTarget = _tcsistr(pszStart, lpszOld)) != NULL) {
			nCount++;
			pszStart = pszTarget + nOldLen;
		}
		pszStart += _tcslen(pszStart);
	}

	// if any changes, make them now
	if (nCount > 0) {
		// allocate buffer for result string
		size_t nResultStrSize = nStrLen + (nNewLen - nOldLen) * nCount + 2;
		pszResultStr = new TCHAR [nResultStrSize];
		ZeroMemory(pszResultStr, nResultStrSize * sizeof(TCHAR));

		pszStart = (TCHAR *)lpszStr;
		pszEnd = (TCHAR *)lpszStr + nStrLen;
		TCHAR *cp = pszResultStr;

		// loop again to actually do the work
		while (pszStart < pszEnd) {
			while ((pszTarget = _tcsistr(pszStart, lpszOld)) != NULL) {
				int nCopyLen = (int)(pszTarget - pszStart);
				_tcsncpy(cp, &lpszStr[pszStart - lpszStr], nCopyLen);

				cp += nCopyLen;

				pszStart = pszTarget + nOldLen;

				_tcscpy(cp, lpszNew);

				cp += nNewLen;
			}
			_tcscpy(cp, pszStart);
			pszStart += _tcslen(pszStart);
		}

		if (pszResultStr)
			lpszStr = mir_tstrdup(pszResultStr);
	}

	int nSize = 0;
	if (pszResultStr) {
		nSize = (int)_tcslen(pszResultStr);
		delete [] pszResultStr;
	}

	return nSize;
}

BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	HANDLE hFile = NULL;
	DWORD dwBytes;

	NETLIBHTTPREQUEST nlhr = {0};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	char *szUrl = mir_t2a(tszURL);
	nlhr.szUrl = szUrl;
	nlhr.headersCount = 4;
	nlhr.headers=(NETLIBHTTPHEADER *)mir_alloc(sizeof(NETLIBHTTPHEADER) * nlhr.headersCount);
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	nlhr.headers[1].szName = "Connection";
	nlhr.headers[1].szValue = "close";
	nlhr.headers[2].szName = "Cache-Control";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName = "Pragma";
	nlhr.headers[3].szValue = "no-cache";

	bool ret = false;
	NETLIBHTTPREQUEST *pReply = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&nlhr);
	if (pReply) {
		if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
			char *date = NULL, *size = NULL;
			for (int i = 0; i < pReply->headersCount; i++) {
				if (!lstrcmpiA(pReply->headers[i].szName, "Last-Modified")) {
					date = pReply->headers[i].szValue;
					continue;
				}
				if (!lstrcmpiA(pReply->headers[i].szName, "Content-Length")) {
					size = pReply->headers[i].szValue;
					continue;
				}
			}
			if (date != NULL && size != NULL) {
				TCHAR *tdate = mir_a2t(date);
				TCHAR *tsize = mir_a2t(size);
				struct _stat buf;

				int fh = _topen(tszLocal, _O_RDONLY);
				if (fh != -1) {
					_fstat(fh, &buf);
					time_t modtime = DateToUnixTime(tdate, 0);
					time_t filemodtime = mktime(localtime(&buf.st_atime));
					if (modtime > filemodtime && buf.st_size != _ttoi(tsize)) {
						hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
						ret = true;
					}
					_close(fh);
				}
				else {
					hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
					ret = true;
				}
				mir_free(tdate);
				mir_free(tsize);
			}
			else {
				hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
				ret = true;
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pReply);
	}

	mir_free(szUrl);
	mir_free(nlhr.headers);

	if (hFile)
		CloseHandle(hFile);

	return ret;
}

typedef HRESULT (MarkupCallback)(IHTMLDocument3 *, BSTR &message);

HRESULT TestMarkupServices(BSTR bstrHtml, MarkupCallback *pCallback, BSTR &message)
{
	IHTMLDocument3 *pHtmlDocRoot = NULL;

	// Create the root document -- a "workspace" for parsing.
	HRESULT hr = CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pHtmlDocRoot));
	if (SUCCEEDED(hr) && pHtmlDocRoot) {
		IPersistStreamInit *pPersistStreamInit = NULL;

		HRESULT hr = pHtmlDocRoot->QueryInterface(IID_PPV_ARGS(&pPersistStreamInit));
		if (SUCCEEDED(hr)) {
			// Initialize the root document to a default state -- ready for parsing.
			hr = pPersistStreamInit->InitNew();

			IMarkupServices *pMarkupServices = NULL;
			hr = pHtmlDocRoot->QueryInterface(IID_PPV_ARGS(&pMarkupServices));
			if (SUCCEEDED(hr)) {
				IMarkupPointer *pMarkupBegin = NULL;
				IMarkupPointer *pMarkupEnd = NULL;

				// These markup pointers indicate the insertion point.
				hr = pMarkupServices->CreateMarkupPointer(&pMarkupBegin);
				if (SUCCEEDED(hr))
					hr = pMarkupServices->CreateMarkupPointer(&pMarkupEnd);

				if (SUCCEEDED(hr) && pMarkupBegin && pMarkupEnd) {
					IMarkupContainer *pMarkupContainer = NULL;

					// Parse the string -- the markup container contains the parsed HTML.
					// Markup pointers are updated to point to begining and end of new container.
					hr = pMarkupServices->ParseString(bstrHtml, 0, &pMarkupContainer, pMarkupBegin, pMarkupEnd);
					if (SUCCEEDED(hr) && pMarkupContainer) {
						IHTMLDocument3 *pHtmlDoc = NULL;

						// Retrieve the document interface to the markup container.
						hr = pMarkupContainer->QueryInterface(IID_PPV_ARGS(&pHtmlDoc));
						if (SUCCEEDED(hr) && pHtmlDoc) {
							// Invoke the user-defined action for this new fragment.
							hr = pCallback(pHtmlDoc, message);

							// Clean up.
							pHtmlDoc->Release();
						}
						pMarkupContainer->Release();
					}
					pMarkupEnd->Release();
				}
				if (pMarkupBegin)
					pMarkupBegin->Release();
				pMarkupServices->Release();
			}
			pPersistStreamInit->Release();
		}
		pHtmlDocRoot->Release();
	}
	return hr;
}

HRESULT TestDocumentText(IHTMLDocument3 *pHtmlDoc, BSTR &message)
{
	IHTMLDocument2 *pDoc = NULL;
	IHTMLElement *pElem = NULL;
	BSTR bstrId = SysAllocString(L"test");

	HRESULT hr = pHtmlDoc->QueryInterface(IID_PPV_ARGS(&pDoc));
	if (SUCCEEDED(hr) && pDoc) {
		hr = pDoc->get_body(&pElem);
		if (SUCCEEDED(hr) && pElem) {
			BSTR bstrText = NULL;
			pElem->get_innerText(&bstrText);
			message = SysAllocString(bstrText);
			SysFreeString(bstrText);
			pElem->Release();
		}

		pDoc->Release();
	}

	SysFreeString(bstrId);
	return hr;
}

VOID ClearText(TCHAR *&message)
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	BSTR bstrHtml = SysAllocString(message), bstrRes = SysAllocString(L"");
	HRESULT hr = TestMarkupServices(bstrHtml, &TestDocumentText, bstrRes);
	if ( SUCCEEDED(hr)) {
		replaceStrT(message, bstrRes);
		SysFreeString(bstrRes);
	}
	SysFreeString(bstrHtml);
	CoUninitialize();
}

TCHAR * CheckFeed(TCHAR *tszURL, HWND hwndDlg)
{
	char *szData = NULL;
	DBVARIANT dbVar = {0};
	GetNewsData(tszURL, &szData, NULL, hwndDlg);
	if (szData) {
		TCHAR *tszData = mir_utf8decodeT(szData);
		if (!tszData)
			tszData = mir_a2t(szData);
		int bytesParsed = 0;
		HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
		mir_free(tszData);
		mir_free(szData);
		if (hXml != NULL) {
			int childcount = 0;
			HXML node;
			if ( !lstrcmpi(xi.getName(hXml), _T("xml")))
				node = xi.getChild(hXml, childcount);
			else
				node = hXml;
			while (node) {
				if (!lstrcmpi(xi.getName(node), _T("rss")) || !lstrcmpi(xi.getName(node), _T("rdf"))) {
					HXML chan = xi.getChild(node, 0);
					for (int j = 0; j < xi.getChildCount(chan); j++) {
						HXML child = xi.getChild(chan, j);
						if (!lstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(NULL, mes, TranslateT("New Aggregator"), MB_OK|MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				else if (!lstrcmpi(xi.getName(node), _T("feed"))) {
					for (int j = 0; j < xi.getChildCount(node); j++) {
						HXML child = xi.getChild(node, j);
						if (!lstrcmpi(xi.getName(child), _T("title"))) {
							TCHAR mes[MAX_PATH];
							mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a valid feed's address."), tszURL);
							MessageBox(NULL, mes, TranslateT("New Aggregator"), MB_OK|MB_ICONINFORMATION);
							TCHAR *tszTitle = (TCHAR *)xi.getText(child);
							return tszTitle;
						}
					}
				}
				childcount +=1;
				node = xi.getChild(hXml, childcount);
			}
		}
		xi.destroyNode(hXml);
	}
	else {
		TCHAR mes[MAX_PATH];
		mir_sntprintf(mes, SIZEOF(mes), TranslateT("%s\nis a not valid feed's address."), tszURL);
		MessageBox(NULL, mes, TranslateT("New Aggregator"), MB_OK|MB_ICONERROR);
	}
	return NULL;
}

VOID CheckCurrentFeed(HANDLE hContact)
{
	char *szData = NULL;
	DBVARIANT dbURL = {0};
	if (db_get_ts(hContact, MODULE, "URL", &dbURL))
		return;

	if (db_get_b(hContact, MODULE, "CheckState", 1) != 0) {
		GetNewsData(dbURL.ptszVal, &szData, hContact, NULL);
		db_free(&dbURL);
		if (szData) {
			TCHAR *tszData = mir_utf8decodeT(szData);
			if (!tszData)
				tszData = mir_a2t(szData);
			int bytesParsed = 0;
			HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
			mir_free(tszData);
			mir_free(szData);
			if(hXml != NULL) {
				int childcount = 0;
				HXML node;
				if ( !lstrcmpi(xi.getName(hXml), _T("xml")))
					node = xi.getChild(hXml, childcount);
				else
					node = hXml;
				while (node) {
					if (!lstrcmpi(xi.getName(node), _T("rss")) || !lstrcmpi(xi.getName(node), _T("rdf"))) {
						if (!lstrcmpi(xi.getName(node), _T("rss"))) {
							for (int i = 0; i < xi.getAttrCount(node); i++) {
								if (!lstrcmpi(xi.getAttrName(node, i), _T("version"))) {
									TCHAR ver[MAX_PATH];
									mir_sntprintf(ver, SIZEOF(ver), _T("RSS %s"), xi.getAttrValue(node, xi.getAttrName(node, i)));
									db_set_ts(hContact, MODULE, "MirVer", ver);
									break;
								}
							}
						}
						else if (!lstrcmpi(xi.getName(node), _T("rdf")))
							db_set_ts(hContact, MODULE, "MirVer", _T("RSS 1.0"));

						HXML chan = xi.getChild(node, 0);
						for (int j = 0; j < xi.getChildCount(chan); j++) {
							HXML child = xi.getChild(chan, j);
							if (!lstrcmpi(xi.getName(child), _T("title")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "FirstName", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("link"))) {
								db_set_ts(hContact, MODULE, "Homepage", xi.getText(child));
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("description")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "About", string);
								db_set_ts(hContact, "CList", "StatusMsg", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("language")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Language1", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("managingEditor")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "e-mail", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("category")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Interest0Text", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("image"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = GetImageFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
							if (!lstrcmpi(xi.getName(child), _T("lastBuildDate")) && xi.getText(child)) {
								TCHAR *lastupdtime = (TCHAR *)xi.getText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 0);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
									xi.destroyNode(hXml);
									return;
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("item"))) {
								TCHAR *title = NULL, *link = NULL, *datetime = NULL, *descr = NULL, *author = NULL, *comments = NULL, *guid = NULL, *category = NULL;
								for (int z = 0; z < xi.getChildCount(child); z++) {
									HXML itemval = xi.getChild(child, z);
									if (!lstrcmpi(xi.getName(itemval), _T("title"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										title = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("link"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										link = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("pubDate"))) {
										datetime = (TCHAR *)xi.getText(itemval);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("dc:date"))) {
										datetime = (TCHAR *)xi.getText(itemval);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("description"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										descr = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("author"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										author = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("comments"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										comments = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("guid"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										guid = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("category"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										category = mir_tstrdup(string);
										mir_free(string);
										continue;
									}
								}
								TCHAR *message;
								DBVARIANT dbMsg = {0};
								if (db_get_ts(hContact, MODULE, "MsgFormat", &dbMsg))
									message = _T(TAGSDEFAULT);
								else
									message = mir_tstrdup(dbMsg.ptszVal);
								db_free(&dbMsg);
								if (!title)
									StrReplace(_T("#<title>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<title>#"), title, message);
									mir_free(title);
								}
								if (!link)
									StrReplace(_T("#<link>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<link>#"), link, message);
									mir_free(link);
								}
								if (!descr)
									StrReplace(_T("#<description>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<description>#"), descr, message);
									mir_free(descr);
								}
								if (!author)
									StrReplace(_T("#<author>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<author>#"), author, message);
									mir_free(author);
								}
								if (!comments)
									StrReplace(_T("#<comments>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<comments>#"), comments, message);
									mir_free(comments);
								}
								if (!guid)
									StrReplace(_T("#<guid>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<guid>#"), guid, message);
									mir_free(guid);
								}
								if (!category)
									StrReplace(_T("#<category>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<category>#"), category, message);
									mir_free(category);
								}

								time_t stamp;
								if (!datetime)
									stamp = time(NULL);
								else
									stamp = DateToUnixTime(datetime, 0);

								HANDLE hDbEvent = db_event_first(hContact);
								BOOL MesExist = FALSE;
								while (hDbEvent) {
									DBEVENTINFO olddbei = { sizeof(olddbei) };
									olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
									olddbei.pBlob = (PBYTE)mir_alloc(olddbei.cbBlob);
									db_event_get(hDbEvent, &olddbei);
									char *pszTemp = mir_utf8encodeT(message);
									if (olddbei.cbBlob == lstrlenA(pszTemp) + 1 && !lstrcmpA((char *)olddbei.pBlob, pszTemp)) {
										MesExist = TRUE;
										break;
									}
									hDbEvent = db_event_next(hDbEvent);
									mir_free(olddbei.pBlob);
									mir_free(pszTemp);
								}

								if (!MesExist) {
									PROTORECVEVENT recv;
									recv.flags = PREF_TCHAR;
									recv.timestamp = stamp;
									recv.tszMessage = message;
									ProtoChainRecvMsg(hContact, &recv);
								}
								mir_free(message);
							}
						}
					}
					else if (!lstrcmpi(xi.getName(node), _T("feed"))) {
						db_set_ts(hContact, MODULE, "MirVer", _T("Atom 3"));
						for (int j = 0; j < xi.getChildCount(node); j++) {
							HXML child = xi.getChild(node, j);
							if (!lstrcmpi(xi.getName(child), _T("title")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "FirstName", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("link"))) {
								for (int x = 0; x < xi.getAttrCount(child); x++) {
									if (!lstrcmpi(xi.getAttrName(child, x), _T("rel")))
										if (!lstrcmpi(xi.getAttrValue(child, xi.getAttrName(child, x)), _T("self")))
											break;

									if (!lstrcmpi(xi.getAttrName(child, x), _T("href")))
										db_set_ts(hContact, MODULE, "Homepage", xi.getAttrValue(child, xi.getAttrName(child, x)));
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("subtitle")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "About", string);
								db_set_ts(hContact, "CList", "StatusMsg", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("language")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Language1", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("author"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML authorval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(authorval), _T("name"))) {
										db_set_ts(hContact, MODULE, "e-mail", xi.getText(authorval));
										break;
									}
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("category")) && xi.getText(child)) {
								TCHAR *string = mir_tstrdup(xi.getText(child));
								ClearText(string);
								db_set_ts(hContact, MODULE, "Interest0Text", string);
								mir_free(string);
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("icon"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = GetImageFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
							if (!lstrcmpi(xi.getName(child), _T("updated")) && xi.getText(child)) {
								TCHAR *lastupdtime = (TCHAR *)xi.getText(child);
								time_t stamp = DateToUnixTime(lastupdtime, 1);
								double deltaupd = difftime(time(NULL), stamp);
								double deltacheck = difftime(time(NULL), db_get_dw(hContact, MODULE, "LastCheck", 0));
								if (deltaupd - deltacheck >= 0) {
									db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
									xi.destroyNode(hXml);
									return;
								}
								continue;
							}
							if (!lstrcmpi(xi.getName(child), _T("entry"))) {
								TCHAR *title = NULL, *link = NULL, *datetime = NULL, *descr = NULL, *author = NULL, *comments = NULL, *guid = NULL, *category = NULL;
								for (int z = 0; z < xi.getChildCount(child); z++) {
									HXML itemval = xi.getChild(child, z);
									if (!lstrcmpi(xi.getName(itemval), _T("title")) && xi.getText(itemval)) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										title = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("link"))) {
										for (int x = 0; x < xi.getAttrCount(itemval); x++) {
											if (!lstrcmpi(xi.getAttrName(itemval, x), _T("href"))) {
												TCHAR *string = mir_tstrdup(xi.getAttrValue(itemval, xi.getAttrName(itemval, x)));
												ClearText(string);
												link = string;
												break;
											}
										}
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("updated"))) {
										datetime = (TCHAR *)xi.getText(itemval);
										continue;
									}
									if ((!lstrcmpi(xi.getName(itemval), _T("summary")) || !lstrcmpi(xi.getName(itemval), _T("content"))) && xi.getText(itemval)) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										descr = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("author"))) {
										for (int x = 0; x < xi.getChildCount(itemval); x++) {
											HXML authorval = xi.getChild(itemval, x);
											if (!lstrcmpi(xi.getName(authorval), _T("name")) && xi.getText(authorval)) {
												TCHAR *string = mir_tstrdup(xi.getText(authorval));
												ClearText(string);
												author = string;
												break;
											}
										}
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("comments")) && xi.getText(itemval)) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										comments = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("id"))) {
										TCHAR *string = mir_tstrdup(xi.getText(itemval));
										ClearText(string);
										guid = string;
										continue;
									}
									if (!lstrcmpi(xi.getName(itemval), _T("category"))) {
										for (int x = 0; x < xi.getAttrCount(itemval); x++) {
											if (!lstrcmpi(xi.getAttrName(itemval, x), _T("term")) && xi.getText(itemval)) {
												TCHAR *string = mir_tstrdup(xi.getAttrValue(itemval, xi.getAttrName(itemval, x)));
												ClearText(string);
												category = string;
												break;
											}
										}
										continue;
									}
								}
								TCHAR *message;
								DBVARIANT dbMsg = {0};
								if (db_get_ts(hContact, MODULE, "MsgFormat", &dbMsg))
									message = _T(TAGSDEFAULT);
								else
									message = mir_tstrdup(dbMsg.ptszVal);
								db_free(&dbMsg);

								if (!title)
									StrReplace(_T("#<title>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<title>#"), title, message);
									mir_free(title);
								}
								if (!link)
									StrReplace(_T("#<link>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<link>#"), link, message);
									mir_free(link);
								}
								if (!descr)
									StrReplace(_T("#<description>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<description>#"), descr, message);
									mir_free(descr);
								}
								if (!author)
									StrReplace(_T("#<author>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<author>#"), author, message);
									mir_free(author);
								}
								if (!comments)
									StrReplace(_T("#<comments>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<comments>#"), comments, message);
									mir_free(comments);
								}
								if (!guid)
									StrReplace(_T("#<guid>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<guid>#"), guid, message);
									mir_free(guid);
								}
								if (!category)
									StrReplace(_T("#<category>#"), TranslateT("empty"), message);
								else {
									StrReplace(_T("#<category>#"), category, message);
									mir_free(category);
								}

								time_t stamp;
								if (!datetime)
									stamp = time(NULL);
								else
									stamp = DateToUnixTime(datetime, 1);

								HANDLE hDbEvent = db_event_first(hContact);
								BOOL MesExist = FALSE;
								while (hDbEvent) {
									DBEVENTINFO olddbei = { sizeof(olddbei) };
									olddbei.cbBlob = db_event_getBlobSize(hDbEvent);
									olddbei.pBlob = (PBYTE)mir_alloc(olddbei.cbBlob);
									db_event_get(hDbEvent, &olddbei);
									char *pszTemp = mir_utf8encodeT(message);
									if (olddbei.cbBlob == lstrlenA(pszTemp) + 1 && !lstrcmpA((char *)olddbei.pBlob, pszTemp))
										MesExist = TRUE;
									hDbEvent = db_event_next(hDbEvent);
									mir_free(olddbei.pBlob);
									mir_free(pszTemp);
								}

								if (!MesExist) {
									PROTORECVEVENT recv;
									recv.flags = PREF_TCHAR;
									recv.timestamp = stamp;
									recv.tszMessage = message;
									ProtoChainRecvMsg(hContact, &recv);
								}
								mir_free(message);
							}
						}
					}
					childcount +=1;
					node = xi.getChild(hXml, childcount);
				}
				xi.destroyNode(hXml);
			}
		}
		db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
	}
}

VOID CheckCurrentFeedAvatar(HANDLE hContact)
{
	char *szData = NULL;
	DBVARIANT dbURL = {0};
	if (db_get_ts(hContact, MODULE, "URL", &dbURL))
		return;
	
	if (db_get_b(hContact, MODULE, "CheckState", 1) != 0) {
		GetNewsData(dbURL.ptszVal, &szData, hContact, NULL);
		db_free(&dbURL);
		if (szData) {
			TCHAR *tszData = mir_utf8decodeT(szData);
			if (!tszData)
				tszData = mir_a2t(szData);
			int bytesParsed = 0;
			HXML hXml = xi.parseString(tszData, &bytesParsed, NULL);
			mir_free(tszData);
			mir_free(szData);
			if(hXml != NULL) {
				int childcount = 0;
				HXML node = xi.getChild(hXml, childcount);
				while (node) {
					if (!lstrcmpi(xi.getName(node), _T("rss")) || !lstrcmpi(xi.getName(node), _T("rdf"))) {
						HXML chan = xi.getChild(node, 0);
						for (int j = 0; j < xi.getChildCount(chan); j++) {
							HXML child = xi.getChild(chan, j);
							if (!lstrcmpi(xi.getName(child), _T("image"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = GetImageFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
						}
					}
					else if (!lstrcmpi(xi.getName(node), _T("feed"))) {
						for (int j = 0; j < xi.getChildCount(node); j++) {
							HXML child = xi.getChild(node, j);
							if (!lstrcmpi(xi.getName(child), _T("icon"))) {
								for (int x = 0; x < xi.getChildCount(child); x++) {
									HXML imageval = xi.getChild(child, x);
									if (!lstrcmpi(xi.getName(imageval), _T("url"))) {
										LPCTSTR url = xi.getText(imageval);
										db_set_ts(hContact, MODULE, "ImageURL", url);

										PROTO_AVATAR_INFORMATIONT pai = {NULL};
										pai.cbSize = sizeof(pai);
										pai.hContact = hContact;
										DBVARIANT dbVar = {0};

										if (!db_get_ts(hContact, MODULE, "Nick", &dbVar)) {
											TCHAR *ext = _tcsrchr((TCHAR *)url, _T('.')) + 1;
											pai.format = GetImageFormat(ext);

											TCHAR *filename = dbVar.ptszVal;
											mir_sntprintf(pai.filename, SIZEOF(pai.filename), _T("%s\\%s.%s"), tszRoot, filename, ext);
											if (DownloadFile(url, pai.filename)) {
												db_set_ts(hContact, MODULE, "ImagePath", pai.filename);
												ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE) &pai, NULL);
											}
											else ProtoBroadcastAck(MODULE, hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE) &pai, NULL);
											db_free(&dbVar);
											break;
										}
									}
								}
							}
						}
					}
					childcount +=1;
					node = xi.getChild(hXml, childcount);
				}
				xi.destroyNode(hXml);
			}
		}
	}
}

int ImportFeedsDialog()
			{
				TCHAR FileName[MAX_PATH];
				TCHAR *tszMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));

				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				TCHAR tmp[MAX_PATH];
				mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.opml, *.xml)%c*.opml;*.xml%c%c"), TranslateT("OPML files"), 0, 0, 0);
				ofn.lpstrFilter = tmp;
				ofn.hwndOwner = 0;
				ofn.lpstrFile = FileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.Flags = OFN_HIDEREADONLY;
				ofn.lpstrInitialDir = tszMirDir;
				*FileName = '\0';
				ofn.lpstrDefExt = _T("");

				if (GetOpenFileName(&ofn)) {
					int bytesParsed = 0;
					HXML hXml = xi.parseFile(FileName, &bytesParsed, NULL);
					if(hXml != NULL) {
						BYTE isTitleUTF = 0, isURLUTF = 0, isSiteURLUTF = 0, isGroupUTF = 0;
						HXML node = xi.getChildByPath(hXml, _T("opml/body/outline"), 0);
						if ( !node)
							node = xi.getChildByPath(hXml, _T("body/outline"), 0);
						if (node) {
							while (node) {
								int outlineAttr = xi.getAttrCount(node);
								int outlineChildsCount = xi.getChildCount(node);
								TCHAR *type = (TCHAR *)xi.getAttrValue(node, _T("type"));
								if ( !type && !outlineChildsCount) {
									HXML tmpnode = node;
									node = xi.getNextNode(node);
									if ( !node) {
										do {
											node = tmpnode;
											node = xi.getParent(node);
											tmpnode = node;
											node = xi.getNextNode(node);
											if (node)
												break;
										} while (lstrcmpi(xi.getName(node), _T("body")));
									}
								}
								else if (!type && outlineChildsCount)
									node = xi.getFirstChild(node);
								else if (type) {
									TCHAR *title = NULL, *url = NULL, *siteurl = NULL, *group = NULL, *utfgroup = NULL;
									for (int i = 0; i < outlineAttr; i++) {
										if (!lstrcmpi(xi.getAttrName(node, i), _T("title"))) {
											title = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
											if ( !title) {
												isTitleUTF = 0;
												title = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
											} else
												isTitleUTF = 1;
											continue;
										}
										if (!lstrcmpi(xi.getAttrName(node, i), _T("xmlUrl"))) {
											url = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
											if ( !url) {
												isURLUTF = 0;
												url = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
											} else
												isURLUTF = 1;
											continue;
										}
										if (!lstrcmpi(xi.getAttrName(node, i), _T("htmlUrl"))) {
											siteurl = mir_utf8decodeT(_T2A(xi.getAttrValue(node, xi.getAttrName(node, i))));
											if ( !siteurl) {
												isSiteURLUTF = 0;
												siteurl = (TCHAR *)xi.getAttrValue(node, xi.getAttrName(node, i));
											} else
												isSiteURLUTF = 1;
											continue;
										}
										if (title && url && siteurl)
											break;
									}

									HXML parent = xi.getParent(node);
									while (lstrcmpi(xi.getName(parent), _T("body"))) {
										for (int i = 0; i < xi.getAttrCount(parent); i++) {
											if (!lstrcmpi(xi.getAttrName(parent, i), _T("title"))) {
												if ( !group)
													group = (TCHAR *)xi.getAttrValue(parent, xi.getAttrName(parent, i));
												else {
													TCHAR tmpgroup[1024];
													mir_sntprintf(tmpgroup, SIZEOF(tmpgroup), _T("%s\\%s"), xi.getAttrValue(parent, xi.getAttrName(parent, i)), group);
													group = tmpgroup;
												}
												break;
											}
										}
										parent = xi.getParent(parent);
									}

									if (group) {
										utfgroup = mir_utf8decodeT(_T2A(group));
										if ( !utfgroup) {
											isGroupUTF = 0;
											utfgroup = group;
										} else
											isGroupUTF = 1;
									}

									HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
									CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)MODULE);
									db_set_ts(hContact, MODULE, "Nick", title);
									db_set_ts(hContact, MODULE, "URL", url);
									db_set_ts(hContact, MODULE, "Homepage", siteurl);
									db_set_b(hContact, MODULE, "CheckState", 1);
									db_set_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME);
									db_set_ts(hContact, MODULE, "MsgFormat", _T(TAGSDEFAULT));
									db_set_w(hContact, MODULE, "Status", CallProtoService(MODULE, PS_GETSTATUS, 0, 0));
									if (utfgroup) {
										db_set_ts(hContact, "CList", "Group", utfgroup);
										int hGroup = 1;
										char *group_name;
										BYTE GroupExist = 0;
										do {
											group_name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
											if (group_name != NULL && !strcmp(group_name, _T2A(utfgroup))) {
												GroupExist = 1;
												break;
											}
											hGroup++;
										}
											while (group_name);

										if(!GroupExist)
											CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)utfgroup);
									}
									if (isTitleUTF)
										mir_free(title);
									if (isURLUTF)
										mir_free(url);
									if (isGroupUTF)
										mir_free(utfgroup);
									if (isSiteURLUTF)
										mir_free(siteurl);

									HXML tmpnode = node;
									node = xi.getNextNode(node);
									if ( !node) {
										do {
											node = tmpnode;
											node = xi.getParent(node);
											tmpnode = node;
											node = xi.getNextNode(node);
											if (node)
												break;
										}
											while (lstrcmpi(xi.getName(tmpnode), _T("body")));
									}
								}
							}
						}
						else MessageBox(NULL, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);
						xi.destroyNode(hXml);
					}
					else MessageBox(NULL, TranslateT("Not valid import file."), TranslateT("Error"), MB_OK | MB_ICONERROR);

					mir_free(tszMirDir);
					return 0;
				}
				mir_free(tszMirDir);
				return 1;
			}
