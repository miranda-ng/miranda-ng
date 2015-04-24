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
bool UpdateListFlag = FALSE;

bool IsMyContact(MCONTACT hContact)
{
	const char *szProto = GetContactProto(hContact);
	return szProto != NULL && strcmp(MODULE, szProto) == 0;
}

void NetlibInit()
{
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = TranslateT("NewsAggregator HTTP connection");
	nlu.szSettingsModule = MODULE;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

void NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = NULL;
}

void GetNewsData(TCHAR *tszUrl, char **szData, MCONTACT hContact, HWND hwndDlg)
{
	Netlib_LogfT(hNetlibUser, _T("Getting feed data %s."), tszUrl);
	NETLIBHTTPREQUEST nlhr = { 0 };

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (_tcsstr(tszUrl, _T("https://")) != NULL)
		nlhr.flags |= NLHRF_SSL;
	char *szUrl = mir_t2a(tszUrl);
	nlhr.szUrl = szUrl;
	nlhr.nlc = hNetlibHttp;

	// change the header so the plugin is pretended to be IE 6 + WinXP
	NETLIBHTTPHEADER headers[5];
	nlhr.headersCount = 4;
	nlhr.headers = headers;
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = NETLIB_USER_AGENT;
	nlhr.headers[1].szName = "Cache-Control";
	nlhr.headers[1].szValue = "no-cache";
	nlhr.headers[2].szName = "Pragma";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName = "Connection";
	nlhr.headers[3].szValue = "close";
	char auth[256];
	if (db_get_b(hContact, MODULE, "UseAuth", 0) || IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
		nlhr.headersCount++;
		nlhr.headers[4].szName = "Authorization";

		CreateAuthString(auth, hContact, hwndDlg);
		nlhr.headers[4].szValue = auth;
	}

	// download the page
	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&nlhr);
	if (nlhrReply) {
		// if the recieved code is 200 OK
		if (nlhrReply->resultCode == 200 && nlhrReply->dataLength > 0) {
			Netlib_LogfT(hNetlibUser, _T("Code 200: Succeeded getting feed data %s."), tszUrl);
			// allocate memory and save the retrieved data
			*szData = (char *)mir_alloc((size_t)(nlhrReply->dataLength + 2));
			memcpy(*szData, nlhrReply->pData, (size_t)nlhrReply->dataLength);
			(*szData)[nlhrReply->dataLength] = 0;
		}
		else if (nlhrReply->resultCode == 401) {
			Netlib_LogfT(hNetlibUser, _T("Code 401: feed %s needs auth data."), tszUrl);
			ItemInfo SelItem = { 0 };
			SelItem.hwndList = hwndDlg;
			SelItem.hContact = hContact;
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AUTHENTICATION), hwndDlg, AuthenticationProc, (LPARAM)&SelItem) == IDOK)
				GetNewsData(tszUrl, szData, hContact, hwndDlg);
		}
		else
			Netlib_LogfT(hNetlibUser, _T("Code %d: Failed getting feed data %s."), nlhrReply->resultCode, tszUrl);
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	}
	else
		Netlib_LogfT(hNetlibUser, _T("Failed getting feed data %s, no response."), tszUrl);

	mir_free(szUrl);
}

void CreateList(HWND hwndList)
{
	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	LVCOLUMN lvc = { 0 };
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

void UpdateList(HWND hwndList)
{
	LVITEM lvI = { 0 };

	// Some code to create the list-view control.
	// Initialize LVITEM members that are common to all
	// items.
	int i = 0;
	for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
		UpdateListFlag = TRUE;
		lvI.mask = LVIF_TEXT;
		lvI.iSubItem = 0;
		TCHAR *ptszNick = db_get_tsa(hContact, MODULE, "Nick");
		if (ptszNick) {
			lvI.pszText = ptszNick;
			lvI.iItem = i;
			ListView_InsertItem(hwndList, &lvI);
			lvI.iSubItem = 1;

			TCHAR *ptszURL = db_get_tsa(hContact, MODULE, "URL");
			if (ptszURL) {
				lvI.pszText = ptszURL;
				ListView_SetItem(hwndList, &lvI);
				i++;
				ListView_SetCheckState(hwndList, lvI.iItem, db_get_b(hContact, MODULE, "CheckState", 1));
				mir_free(ptszURL);
			}
			mir_free(ptszNick);
		}
	}
	UpdateListFlag = FALSE;
}

void DeleteAllItems(HWND hwndList)
{
	ListView_DeleteAllItems(hwndList);
}

time_t __stdcall DateToUnixTime(const TCHAR *stamp, bool FeedType)
{
	struct tm timestamp;
	TCHAR date[9];
	int i, y;
	time_t t;

	if (stamp == NULL)
		return 0;

	TCHAR *p = NEWTSTR_ALLOCA(stamp);

	if (FeedType) {
		// skip '-' chars
		int si = 0, sj = 0;
		while (true) {
			if (p[si] == _T('-'))
				si++;
			else if (!(p[sj++] = p[si++]))
				break;
		}
	}
	else {
		TCHAR *weekday, monthstr[4], timezonesign[2];
		int day, month = 0, year, hour, min, sec, timezoneh, timezonem;
		if (_tcsstr(p, _T(","))) {
			weekday = _tcstok(p, _T(","));
			p = _tcstok(NULL, _T(","));
			_stscanf(p + 1, _T("%d %3s %d %d:%d:%d %1s%02d%02d"), &day, &monthstr, &year, &hour, &min, &sec, &timezonesign, &timezoneh, &timezonem);
			if (!mir_tstrcmpi(monthstr, _T("Jan")))
				month = 1;
			if (!mir_tstrcmpi(monthstr, _T("Feb")))
				month = 2;
			if (!mir_tstrcmpi(monthstr, _T("Mar")))
				month = 3;
			if (!mir_tstrcmpi(monthstr, _T("Apr")))
				month = 4;
			if (!mir_tstrcmpi(monthstr, _T("May")))
				month = 5;
			if (!mir_tstrcmpi(monthstr, _T("Jun")))
				month = 6;
			if (!mir_tstrcmpi(monthstr, _T("Jul")))
				month = 7;
			if (!mir_tstrcmpi(monthstr, _T("Aug")))
				month = 8;
			if (!mir_tstrcmpi(monthstr, _T("Sep")))
				month = 9;
			if (!mir_tstrcmpi(monthstr, _T("Oct")))
				month = 10;
			if (!mir_tstrcmpi(monthstr, _T("Nov")))
				month = 11;
			if (!mir_tstrcmpi(monthstr, _T("Dec")))
				month = 12;
			if (year < 2000)
				year += 2000;
			if (!mir_tstrcmp(timezonesign, _T("+")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour - timezoneh, min - timezonem, sec);
			else if (!mir_tstrcmp(timezonesign, _T("-")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour + timezoneh, min + timezonem, sec);
			else
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour, min, sec);
		}
		else if (_tcsstr(p, _T("T"))) {
			_stscanf(p, _T("%d-%d-%dT%d:%d:%d"), &year, &month, &day, &hour, &min, &sec);
			mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour, min, sec);
		}
		else
		{
			_stscanf(p, _T("%d-%d-%d %d:%d:%d %1s%02d%02d"), &year, &month, &day, &hour, &min, &sec, &timezonesign, &timezoneh, &timezonem);
			if (!mir_tstrcmp(timezonesign, _T("+")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour - timezoneh, min - timezonem, sec);
			else if (!mir_tstrcmp(timezonesign, _T("-")))
				mir_sntprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, _T("%04d%02d%02dT%02d:%02d:%02d"), year, month, day, hour + timezoneh, min + timezonem, sec);
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
		y = (date[0] - '0') * 10 + (date[1] - '0');
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
	t -= (time_t)_timezone;
	return (t >= 0) ? t : 0;
}

bool DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	NETLIBHTTPREQUEST nlhr = { 0 };
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	char *szUrl = mir_t2a(tszURL);
	nlhr.szUrl = szUrl;
	NETLIBHTTPHEADER headers[4];
	nlhr.headersCount = 4;
	nlhr.headers = headers;
	nlhr.headers[0].szName = "User-Agent";
	nlhr.headers[0].szValue = NETLIB_USER_AGENT;
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
				if (!mir_strcmpi(pReply->headers[i].szName, "Last-Modified")) {
					date = pReply->headers[i].szValue;
					continue;
				}
				else if (!mir_strcmpi(pReply->headers[i].szName, "Content-Length")) {
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
						DWORD dwBytes;
						HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
						ret = true;
						if (hFile)
							CloseHandle(hFile);
					}
					_close(fh);
				}
				else {
					DWORD dwBytes;
					HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
					ret = true;
					if (hFile)
						CloseHandle(hFile);
				}
				mir_free(tdate);
				mir_free(tsize);
			}
			else {
				DWORD dwBytes;
				HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
				ret = true;
				if (hFile)
					CloseHandle(hFile);
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pReply);
	}

	mir_free(szUrl);

	return ret;
}

typedef HRESULT(MarkupCallback)(IHTMLDocument3 *, BSTR &message);

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
			pPersistStreamInit->InitNew();

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

LPCTSTR ClearText(CMString &result, const TCHAR *message)
{
	BSTR bstrHtml = SysAllocString(message), bstrRes = SysAllocString(_T(""));
	HRESULT hr = TestMarkupServices(bstrHtml, &TestDocumentText, bstrRes);
	if (SUCCEEDED(hr))
		result = bstrRes;
	else
		result = message;
	SysFreeString(bstrHtml);
	SysFreeString(bstrRes);

	return result;
}

MCONTACT GetContactByNick(const TCHAR *nick)
{
	MCONTACT hContact = NULL;

	for (hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
		ptrW contactNick(::db_get_wsa(hContact, MODULE, "Nick"));
		if (!mir_tstrcmpi(contactNick, nick))
			break;
	}
	return hContact;
}

MCONTACT GetContactByURL(const TCHAR *url)
{
	MCONTACT hContact = NULL;

	for (hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
		ptrW contactURL(::db_get_wsa(hContact, MODULE, "URL"));
		if (!mir_tstrcmpi(contactURL, url))
			break;
	}
	return hContact;
}
