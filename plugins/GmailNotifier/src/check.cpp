#include "gmail.h"

//		#include <stdio.h>

void CheckMailInbox(Account *curAcc)
{
	HINTERNET hHTTPOpen;			// internet open handle
	HINTERNET hHTTPConnection;		// internet connection hadle
	HINTERNET hHTTPRequest;		    // internet request hadle

	DBVARIANT dbv;
	static char *contentType = "Content-Type: application/x-www-form-urlencoded";
	char requestBuffer[256] = "continue=https%3A%2F%2Fmail.google.com%2Fa%2F";
	char fileBuffer[_MAX_DOWN_BUFFER] = "";
	char *tail;
	char str[64];
	char temp[_MAX_DOWN_BUFFER] = "";
	unsigned long bufferLength;

	if (curAcc->IsChecking)
		return;

	curAcc->IsChecking = TRUE;

	if (!db_get_s(curAcc->hContact, "CList", "MyHandle", &dbv)) {
		lstrcpyA(curAcc->results.content, dbv.pszVal);
		db_free(&dbv);
	}
	else lstrcpyA(curAcc->results.content, curAcc->name);

	tail = strstr(curAcc->results.content, " [");
	if (tail) *tail = '\0';
	lstrcatA(curAcc->results.content, " [");

	lstrcpyA(str, curAcc->results.content);
	lstrcatA(str, Translate("Checking..."));
	lstrcatA(str, "]");

	db_set_s(curAcc->hContact, "CList", "MyHandle", str);
	hHTTPOpen = InternetOpenA("", INTERNET_OPEN_TYPE_PRECONFIG, "", "", 0);
	if (!hHTTPOpen) {
		lstrcatA(curAcc->results.content, Translate("Can't open Internet!"));
		goto error_handle;
	}

	if (curAcc->hosted[0]) {
		hHTTPConnection = InternetConnectA(hHTTPOpen,
			"www.google.com",
			INTERNET_DEFAULT_HTTPS_PORT,
			NULL,
			NULL,
			INTERNET_SERVICE_HTTP,
			0,
			0);

		if (!hHTTPConnection) {
			lstrcatA(curAcc->results.content, Translate("Can't reach server!"));
			goto error_handle;
		}
		lstrcpyA(str, "/a/");
		lstrcatA(str, curAcc->hosted);
		lstrcatA(str, "/LoginAction");
		hHTTPRequest = HttpOpenRequestA(hHTTPConnection, "POST", str, HTTP_VERSIONA, NULL, NULL, INTERNET_FLAG_SECURE, 0);
		lstrcatA(requestBuffer, curAcc->hosted);
		lstrcatA(requestBuffer, "%2Ffeed%2Fatom&service=mail&userName=");
		lstrcatA(requestBuffer, curAcc->name);
		tail = strchr(requestBuffer, '@');
		*tail = '\0';
		lstrcatA(requestBuffer, "&password=");
		lstrcatA(requestBuffer, curAcc->pass);
		if (!HttpSendRequestA(hHTTPRequest, contentType, (int)strlen(contentType) + 1, requestBuffer, (int)strlen(requestBuffer) + 1)) {
			lstrcpyA(curAcc->results.content, Translate("Can't send account data!"));
			goto error_handle;
		}

		InternetCloseHandle(hHTTPConnection);
		InternetCloseHandle(hHTTPRequest);
	}

	hHTTPConnection = InternetConnectA(hHTTPOpen, "mail.google.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hHTTPConnection) {
		lstrcatA(curAcc->results.content, Translate("Can't reach server!"));
		goto error_handle;
	}
	if (curAcc->hosted[0]) {
		lstrcpyA(str, "/a/");
		lstrcatA(str, curAcc->hosted);
		lstrcatA(str, "/feed/atom");
	}
	else lstrcpyA(str, "/mail/feed/atom");

	hHTTPRequest = HttpOpenRequest(hHTTPConnection, _T("GET"), _A2T(str), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);
	InternetSetOption(hHTTPRequest, INTERNET_OPTION_USERNAME, _A2T(curAcc->name), (int)strlen(curAcc->name) + 1);
	InternetSetOption(hHTTPRequest, INTERNET_OPTION_PASSWORD, _A2T(curAcc->pass), (int)strlen(curAcc->pass) + 1);
	if (!HttpSendRequest(hHTTPRequest, NULL, 0, NULL, 0)) {
		lstrcatA(curAcc->results.content, Translate("Can't get RSS feed!"));
		goto error_handle;
	}
	while (InternetReadFile(hHTTPRequest, temp, _MAX_DOWN_BUFFER, &bufferLength) && bufferLength > 0) {
		temp[bufferLength] = '\0';
		lstrcatA(fileBuffer, temp);
	}

	fileBuffer[_MAX_DOWN_BUFFER - 1] = '\0';
	curAcc->results_num = ParsePage(fileBuffer, &curAcc->results);
	if (curAcc->results_num == -1) {
		lstrcatA(curAcc->results.content, Translate("Wrong name or password!"));
		goto error_handle;
	}
	InternetCloseHandle(hHTTPOpen);
	InternetCloseHandle(hHTTPConnection);
	InternetCloseHandle(hHTTPRequest);

	lstrcatA(curAcc->results.content, _itoa(curAcc->results_num, str, 10));
	lstrcatA(curAcc->results.content, "]");

	curAcc->IsChecking = FALSE;
	return;

error_handle:
	curAcc->results_num = -1;
	InternetCloseHandle(hHTTPOpen);
	InternetCloseHandle(hHTTPConnection);
	InternetCloseHandle(hHTTPRequest);

	lstrcatA(curAcc->results.content, "]");

	curAcc->IsChecking = FALSE;
}

void __cdecl Check_ThreadFunc(LPVOID lpParam)
{
	InternetSetCookieA("https://mail.google.com/mail/", "GX", "");
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

int ParsePage(char *page, resultLink *prst)
{
	char *str_head;
	char *str_tail;
	char name[64], title[64];
	int num = 0;
	wchar_t str[64];

	prst->next = NULL;
	if (strstr(page, "Unauthorized"))
		return -1;
	if (!(str_head = strstr(page, "<entry>")))
		return 0;

	while (str_head = strstr(str_head, "<title>")) {
		prst = prst->next = (resultLink *)malloc(sizeof(resultLink));
		str_head += 7;
		str_tail = strstr(str_head, "</title>");
		*str_tail = '\0';
		lstrcpynA(title, str_head, 41);
		if (lstrlenA(title) == 40)
			lstrcatA(title, "...");
		*str_tail = ' ';

		str_head = strstr(str_head, "<name>") + 6;
		str_tail = strstr(str_head, "</name>");
		*str_tail = '\0';
		lstrcpynA(name, str_head, 11);
		lstrcatA(name, ": ");
		*str_tail = ' ';

		lstrcpyA(prst->content, name);
		lstrcatA(prst->content, title);
		MultiByteToWideChar(CP_UTF8, 0, prst->content, -1, str, 64);
		WideCharToMultiByte(CP_ACP, 0, str, -1, prst->content, 64, NULL, NULL);
		num++;
	}
	prst->next = NULL;
	return num;
}
