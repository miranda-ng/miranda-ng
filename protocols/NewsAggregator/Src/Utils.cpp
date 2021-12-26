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

#include "stdafx.h"

HNETLIBUSER hNetlibUser = nullptr;
HNETLIBCONN hNetlibHttp;
bool UpdateListFlag = FALSE;

bool IsMyContact(MCONTACT hContact)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	return szProto != nullptr && mir_strcmp(MODULENAME, szProto) == 0;
}

void NetlibInit()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = TranslateT("NewsAggregator HTTP connections");
	nlu.szSettingsModule = MODULENAME;
	hNetlibUser = Netlib_RegisterUser(&nlu);
}

void NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = nullptr;
}

void GetNewsData(wchar_t *tszUrl, char **szData, MCONTACT hContact, CFeedEditor *pEditDlg)
{
	Netlib_LogfW(hNetlibUser, L"Getting feed data %s.", tszUrl);
	NETLIBHTTPREQUEST nlhr = { 0 };

	// initialize the netlib request
	nlhr.cbSize = sizeof(nlhr);
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_REDIRECT;
	if (wcsstr(tszUrl, L"https://") != nullptr)
		nlhr.flags |= NLHRF_SSL;
	char *szUrl = mir_u2a(tszUrl);
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
	if (g_plugin.getByte(hContact, "UseAuth", 0) || (pEditDlg && pEditDlg->m_useauth.IsChecked()) /*IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)*/) {
		nlhr.headersCount++;
		nlhr.headers[4].szName = "Authorization";

		CreateAuthString(auth, hContact, pEditDlg);
		nlhr.headers[4].szValue = auth;
	}

	// download the page
	NLHR_PTR nlhrReply(Netlib_HttpTransaction(hNetlibUser, &nlhr));
	if (nlhrReply) {
		// if the recieved code is 200 OK
		if (nlhrReply->resultCode == 200 && nlhrReply->dataLength > 0) {
			Netlib_LogfW(hNetlibUser, L"Code 200: Succeeded getting feed data %s.", tszUrl);
			// allocate memory and save the retrieved data
			*szData = (char *)mir_alloc((size_t)nlhrReply->dataLength + 2);
			memcpy(*szData, nlhrReply->pData, (size_t)nlhrReply->dataLength);
			(*szData)[nlhrReply->dataLength] = 0;
		}
		else if (nlhrReply->resultCode == 401) {
			Netlib_LogfW(hNetlibUser, L"Code 401: feed %s needs auth data.", tszUrl);

			if (CAuthRequest(pEditDlg, hContact).DoModal())
				GetNewsData(tszUrl, szData, hContact, pEditDlg);
		}
		else Netlib_LogfW(hNetlibUser, L"Code %d: Failed getting feed data %s.", nlhrReply->resultCode, tszUrl);
	}
	else Netlib_LogfW(hNetlibUser, L"Failed getting feed data %s, no response.", tszUrl);

	mir_free(szUrl);
}

time_t DateToUnixTime(const char *stamp, bool FeedType)
{
	struct tm timestamp;
	char date[9];
	int i, y;
	time_t t;

	if (stamp == nullptr)
		return 0;

	char *p = NEWSTR_ALLOCA(stamp);

	if (FeedType) {
		// skip '-' chars
		int si = 0, sj = 0;
		while (true) {
			if (p[si] == '-')
				si++;
			else if (!(p[sj++] = p[si++]))
				break;
		}
	}
	else {
		char monthstr[4], timezonesign[2];
		int day, month = 0, year, hour, min, sec, timezoneh, timezonem;
		if (strchr(p, ',')) {
			strtok(p, ",");
			p = strtok(nullptr, ",");
			sscanf(p + 1, "%d %3s %d %d:%d:%d %1s%02d%02d", &day, monthstr, &year, &hour, &min, &sec, timezonesign, &timezoneh, &timezonem);
			if (!mir_strcmpi(monthstr, "Jan"))
				month = 1;
			if (!mir_strcmpi(monthstr, "Feb"))
				month = 2;
			if (!mir_strcmpi(monthstr, "Mar"))
				month = 3;
			if (!mir_strcmpi(monthstr, "Apr"))
				month = 4;
			if (!mir_strcmpi(monthstr, "May"))
				month = 5;
			if (!mir_strcmpi(monthstr, "Jun"))
				month = 6;
			if (!mir_strcmpi(monthstr, "Jul"))
				month = 7;
			if (!mir_strcmpi(monthstr, "Aug"))
				month = 8;
			if (!mir_strcmpi(monthstr, "Sep"))
				month = 9;
			if (!mir_strcmpi(monthstr, "Oct"))
				month = 10;
			if (!mir_strcmpi(monthstr, "Nov"))
				month = 11;
			if (!mir_strcmpi(monthstr, "Dec"))
				month = 12;
			if (year < 2000)
				year += 2000;
			if (!mir_strcmp(timezonesign, "+"))
				mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour - timezoneh, min - timezonem, sec);
			else if (!mir_strcmp(timezonesign, "-"))
				mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour + timezoneh, min + timezonem, sec);
			else
				mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour, min, sec);
		}
		else if (strchr(p, 'T')) {
			sscanf(p, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &min, &sec);
			mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour, min, sec);
		}
		else {
			sscanf(p, "%d-%d-%d %d:%d:%d %1s%02d%02d", &year, &month, &day, &hour, &min, &sec, &timezonesign, &timezoneh, &timezonem);
			if (!mir_strcmp(timezonesign, "+"))
				mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour - timezoneh, min - timezonem, sec);
			else if (!mir_strcmp(timezonesign, "-"))
				mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour + timezoneh, min + timezonem, sec);
			else
				mir_snprintf(p, 4 + 2 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1, "%04d%02d%02dT%02d:%02d:%02d", year, month, day, hour, min, sec);
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
	if (sscanf(p, "%d:%d:%d", &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3)
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
	char *szUrl = mir_u2a(tszURL);
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
	NLHR_PTR pReply(Netlib_HttpTransaction(hNetlibUser, &nlhr));
	if (pReply) {
		if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
			char *date = nullptr, *size = nullptr;
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
			if (date != nullptr && size != nullptr) {
				wchar_t *tsize = mir_a2u(size);
				struct _stat buf;

				int fh = _wopen(tszLocal, _O_RDONLY);
				if (fh != -1) {
					_fstat(fh, &buf);
					time_t modtime = DateToUnixTime(date, 0);
					time_t filemodtime = mktime(localtime(&buf.st_atime));
					if (modtime > filemodtime && buf.st_size != _wtoi(tsize)) {
						DWORD dwBytes;
						HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
						WriteFile(hFile, pReply->pData, (uint32_t)pReply->dataLength, &dwBytes, nullptr);
						ret = true;
						if (hFile)
							CloseHandle(hFile);
					}
					_close(fh);
				}
				else {
					DWORD dwBytes;
					HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					WriteFile(hFile, pReply->pData, (uint32_t)pReply->dataLength, &dwBytes, nullptr);
					ret = true;
					if (hFile)
						CloseHandle(hFile);
				}
				mir_free(tsize);
			}
			else {
				DWORD dwBytes;
				HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				WriteFile(hFile, pReply->pData, (uint32_t)pReply->dataLength, &dwBytes, nullptr);
				ret = true;
				if (hFile)
					CloseHandle(hFile);
			}
		}
	}

	mir_free(szUrl);

	return ret;
}

typedef HRESULT(MarkupCallback)(IHTMLDocument3 *, BSTR &message);

HRESULT TestMarkupServices(BSTR bstrHtml, MarkupCallback *pCallback, BSTR &message)
{
	IHTMLDocument3 *pHtmlDocRoot = nullptr;

	// Create the root document -- a "workspace" for parsing.
	HRESULT hr = CoCreateInstance(CLSID_HTMLDocument, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pHtmlDocRoot));
	if (SUCCEEDED(hr) && pHtmlDocRoot) {
		IPersistStreamInit *pPersistStreamInit = nullptr;

		hr = pHtmlDocRoot->QueryInterface(IID_PPV_ARGS(&pPersistStreamInit));
		if (SUCCEEDED(hr)) {
			// Initialize the root document to a default state -- ready for parsing.
			pPersistStreamInit->InitNew();

			IMarkupServices *pMarkupServices = nullptr;
			hr = pHtmlDocRoot->QueryInterface(IID_PPV_ARGS(&pMarkupServices));
			if (SUCCEEDED(hr)) {
				IMarkupPointer *pMarkupBegin = nullptr;
				IMarkupPointer *pMarkupEnd = nullptr;

				// These markup pointers indicate the insertion point.
				hr = pMarkupServices->CreateMarkupPointer(&pMarkupBegin);
				if (SUCCEEDED(hr))
					hr = pMarkupServices->CreateMarkupPointer(&pMarkupEnd);

				if (SUCCEEDED(hr) && pMarkupBegin && pMarkupEnd) {
					IMarkupContainer *pMarkupContainer = nullptr;

					// Parse the string -- the markup container contains the parsed HTML.
					// Markup pointers are updated to point to begining and end of new container.
					hr = pMarkupServices->ParseString(bstrHtml, 0, &pMarkupContainer, pMarkupBegin, pMarkupEnd);
					if (SUCCEEDED(hr) && pMarkupContainer) {
						IHTMLDocument3 *pHtmlDoc = nullptr;

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
	IHTMLDocument2 *pDoc = nullptr;
	IHTMLElement *pElem = nullptr;
	BSTR bstrId = SysAllocString(L"test");

	HRESULT hr = pHtmlDoc->QueryInterface(IID_PPV_ARGS(&pDoc));
	if (SUCCEEDED(hr) && pDoc) {
		hr = pDoc->get_body(&pElem);
		if (SUCCEEDED(hr) && pElem) {
			BSTR bstrText = nullptr;
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

LPCTSTR ClearText(CMStringW &result, const wchar_t *message)
{
	BSTR bstrHtml = SysAllocString(message), bstrRes = SysAllocString(L"");
	HRESULT hr = TestMarkupServices(bstrHtml, &TestDocumentText, bstrRes);
	if (SUCCEEDED(hr))
		result = bstrRes;
	else
		result = message;
	SysFreeString(bstrHtml);
	SysFreeString(bstrRes);

	result.Replace(L"&#163;", L"£"); //pound
	result.Replace(L"&#178;", L"²"); //sup2
	result.Replace(L"&#228;", L"ä"); //auml
	result.Replace(L"&#233;", L"é"); //latin small letter e with acute
	result.Replace(L"&#235;", L"ë"); //euml
	result.Replace(L"&#246;", L"ö"); //ouml
	result.Replace(L"&#382;", L"ž"); //Latin Small Letter Z With Caron
	result.Replace(L"&#665;", L"ʙ"); //latin letter small capital b
	result.Replace(L"&#774;", L"˘"); //Combining Breve
	result.Replace(L"&#769;", L"´"); //Combining Acute Accent острое ударение
	result.Replace(L"&#959;", L"ό"); // greek small letter omicron with tonos
	result.Replace(L"&#1123;", L"ѣ"); //Cyrillic Small Letter Yat
	result.Replace(L"&#1180;", L"Ҝ"); //cyrillic capital letter ka with vertical stroke
	result.Replace(L"&#8203;", L"");
	result.Replace(L"&#8206;", L""); //lrm
	result.Replace(L"&#8207;", L""); //rlm
	result.Replace(L"&#8209;", L"‑"); //Non-Breaking Hyphen
	result.Replace(L"&#8227;", L"‣"); //Triangular Bullet
	result.Replace(L"&#8722;", L"−"); //minus
	result.Replace(L"&#9786;", L"☺"); //White Smiling Face
	result.Replace(L"&#65279;", L"");

	result.Trim();

	return result;
}

MCONTACT GetContactByNick(const wchar_t *nick)
{
	for (auto &hContact : Contacts(MODULENAME)) {
		ptrW contactNick(g_plugin.getWStringA(hContact, "Nick"));
		if (!mir_wstrcmpi(contactNick, nick))
			return hContact;
	}
	return 0;
}

MCONTACT GetContactByURL(const wchar_t *url)
{
	for (auto &hContact : Contacts(MODULENAME)) {
		ptrW contactURL(g_plugin.getWStringA(hContact, "URL"));
		if (!mir_wstrcmpi(contactURL, url))
			return hContact;
	}
	return 0;
}
