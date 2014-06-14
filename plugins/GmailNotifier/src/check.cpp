#include "gmail.h"

//		#include <stdio.h>

void CheckMailInbox(Account *curAcc)
{
	HINTERNET hHTTPOpen;			// internet open handle
	HINTERNET hHTTPConnection;		// internet connection hadle
	HINTERNET hHTTPRequest;		    // internet request hadle

	DBVARIANT dbv;
	static TCHAR *contentType = _T("Content-Type: application/x-www-form-urlencoded");
	TCHAR requestBuffer[256] = _T("continue=https%3A%2F%2Fmail.google.com%2Fa%2F");
	TCHAR fileBuffer[_MAX_DOWN_BUFFER] = _T("");
	TCHAR *tail;
	TCHAR str[64];
	TCHAR temp[_MAX_DOWN_BUFFER] = _T("");
	unsigned long bufferLength;

	if (curAcc->IsChecking)
		return;

	curAcc->IsChecking = TRUE;

	if (!db_get_ts(curAcc->hContact, "CList", "MyHandle", &dbv)) {
		lstrcpy(curAcc->results.content, dbv.ptszVal);
		db_free(&dbv);
	}
	else lstrcpy(curAcc->results.content, curAcc->name);

	tail = _tcsstr(curAcc->results.content, _T(" ["));
	if (tail) *tail = '\0';
	lstrcat(curAcc->results.content, _T(" ["));

	lstrcpy(str, curAcc->results.content);
	lstrcat(str, TranslateT("Checking..."));
	lstrcat(str, _T("]"));

	db_set_ts(curAcc->hContact, "CList", "MyHandle", str);
	hHTTPOpen = InternetOpen(_T(""), INTERNET_OPEN_TYPE_PRECONFIG, _T(""), _T(""), 0);
	if (!hHTTPOpen) {
		lstrcat(curAcc->results.content, TranslateT("Can't open Internet!"));
		goto error_handle;
	}

	if (curAcc->hosted[0]) {
		hHTTPConnection = InternetConnect(hHTTPOpen,
			_T("www.google.com"),
			INTERNET_DEFAULT_HTTPS_PORT,
			NULL,
			NULL,
			INTERNET_SERVICE_HTTP,
			0,
			0);

		if (!hHTTPConnection) {
			lstrcat(curAcc->results.content, TranslateT("Can't reach server!"));
			goto error_handle;
		}
		lstrcpy(str, _T("/a/"));
		lstrcat(str, curAcc->hosted);
		lstrcat(str, _T("/LoginAction"));
		hHTTPRequest = HttpOpenRequest(hHTTPConnection, _T("POST"), str, HTTP_VERSION, NULL, NULL, INTERNET_FLAG_SECURE, 0);
		lstrcat(requestBuffer, curAcc->hosted);
		lstrcat(requestBuffer, _T("%2Ffeed%2Fatom&service=mail&userName="));
		lstrcat(requestBuffer, curAcc->name);
		tail = _tcschr(requestBuffer, '@');
		*tail = '\0';
		lstrcat(requestBuffer, _T("&password="));
		lstrcat(requestBuffer, curAcc->pass);
		if (!HttpSendRequest(hHTTPRequest, contentType, (int)_tcslen(contentType) + 1, requestBuffer, (int)_tcslen(requestBuffer) + 1)) {
			lstrcpy(curAcc->results.content, TranslateT("Can't send account data!"));
			goto error_handle;
		}

		InternetCloseHandle(hHTTPConnection);
		InternetCloseHandle(hHTTPRequest);
	}

	hHTTPConnection = InternetConnect(hHTTPOpen, _T("mail.google.com"), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hHTTPConnection) {
		lstrcat(curAcc->results.content, TranslateT("Can't reach server!"));
		goto error_handle;
	}
	if (curAcc->hosted[0]) {
		lstrcpy(str, _T("/a/"));
		lstrcat(str, curAcc->hosted);
		lstrcat(str, _T("/feed/atom"));
	}
	else lstrcpy(str, _T("/mail/feed/atom"));

	hHTTPRequest = HttpOpenRequest(hHTTPConnection, _T("GET"), str, NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);
	InternetSetOption(hHTTPRequest, INTERNET_OPTION_USERNAME, curAcc->name, (int)_tcslen(curAcc->name) + 1);
	InternetSetOption(hHTTPRequest, INTERNET_OPTION_PASSWORD, curAcc->pass, (int)_tcslen(curAcc->pass) + 1);
	if (!HttpSendRequest(hHTTPRequest, NULL, 0, NULL, 0)) {
		lstrcat(curAcc->results.content, TranslateT("Can't get RSS feed!"));
		goto error_handle;
	}
	while (InternetReadFile(hHTTPRequest, temp, _MAX_DOWN_BUFFER, &bufferLength) && bufferLength > 0) {
		temp[bufferLength] = '\0';
		lstrcat(fileBuffer, temp);
	}

	fileBuffer[_MAX_DOWN_BUFFER - 1] = '\0';
	curAcc->results_num = ParsePage(fileBuffer, &curAcc->results);
	if (curAcc->results_num == -1) {
		lstrcat(curAcc->results.content, TranslateT("Wrong name or password!"));
		goto error_handle;
	}
	InternetCloseHandle(hHTTPOpen);
	InternetCloseHandle(hHTTPConnection);
	InternetCloseHandle(hHTTPRequest);

	lstrcat(curAcc->results.content, _itot(curAcc->results_num, str, 10));
	lstrcat(curAcc->results.content, _T("]"));

	curAcc->IsChecking = FALSE;
	return;

error_handle:
	curAcc->results_num = -1;
	InternetCloseHandle(hHTTPOpen);
	InternetCloseHandle(hHTTPConnection);
	InternetCloseHandle(hHTTPRequest);

	lstrcat(curAcc->results.content, _T("]"));

	curAcc->IsChecking = FALSE;
}

void __cdecl Check_ThreadFunc(void *lpParam)
{
	InternetSetCookie(_T("https://mail.google.com/mail/"), _T("GX"), _T(""));
	if (lpParam) {
		CheckMailInbox((Account *)lpParam);
		NotifyUser((Account *)lpParam);
	}
	else {
		for (int i = 0; i < acc_num && GetContactProto(acc[i].hContact); i++) {
			CheckMailInbox(&acc[i]);
			NotifyUser(&acc[i]);
		}
	}
}

int ParsePage(TCHAR *page, resultLink *prst)
{
	TCHAR *str_head, *str_tail;
	TCHAR name[64], title[64];
	int num = 0;

	prst->next = NULL;
	if (_tcsstr(page, _T("Unauthorized")))
		return -1;
	if (!(str_head = _tcsstr(page, _T("<entry>"))))
		return 0;

	while (str_head = _tcsstr(str_head, _T("<title>"))) {
		prst = prst->next = (resultLink *)malloc(sizeof(resultLink));
		str_head += 7;
		str_tail = _tcsstr(str_head, _T("</title>"));
		*str_tail = '\0';
		lstrcpyn(title, str_head, 41);
		if (lstrlen(title) == 40)
			lstrcat(title, _T("..."));
		*str_tail = ' ';

		str_head = _tcsstr(str_head, _T("<name>")) + 6;
		str_tail = _tcsstr(str_head, _T("</name>"));
		*str_tail = '\0';
		lstrcpyn(name, str_head, 11);
		lstrcat(name, _T(": "));
		*str_tail = ' ';

		lstrcpy(prst->content, name);
		lstrcat(prst->content, title);
		num++;
	}
	prst->next = NULL;
	return num;
}
