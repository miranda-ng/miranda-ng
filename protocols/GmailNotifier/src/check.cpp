#include "stdafx.h"

#pragma comment(lib, "Wininet.lib")

static int ParsePage(char *page, resultLink *prst)
{
	char *str_head;
	char *str_tail;
	char name[64], title[64];
	int num = 0;
	wchar_t str[64];

	prst->next = nullptr;
	if (!(str_head = strstr(page, "<entry>")))
		return 0;

	while (str_head = strstr(str_head, "<title>")) {
		prst = prst->next = (resultLink *)malloc(sizeof(resultLink));
		str_head += 7;
		str_tail = strstr(str_head, "</title>");
		*str_tail = '\0';
		mir_strncpy(title, str_head, 41);
		if (mir_strlen(title) == 40)
			mir_strcat(title, "...");
		*str_tail = ' ';

		str_head = strstr(str_head, "<name>") + 6;
		str_tail = strstr(str_head, "</name>");
		*str_tail = '\0';
		mir_strncpy(name, str_head, 11);
		mir_strcat(name, ": ");
		*str_tail = ' ';

		mir_strcpy(prst->content, name);
		mir_strcat(prst->content, title);
		MultiByteToWideChar(CP_UTF8, 0, prst->content, -1, str, 64);
		WideCharToMultiByte(CP_ACP, 0, str, -1, prst->content, 64, nullptr, nullptr);
		num++;
	}
	prst->next = nullptr;
	return num;
}

void CheckMailInbox(Account *curAcc)
{
	if (curAcc->IsChecking)
		return;

	curAcc->IsChecking = true;

	ptrA szNick(db_get_sa(curAcc->hContact, "CList", "MyHandle", curAcc->name));

	char *tail = strstr(szNick, " [");
	if (tail) *tail = 0;

	db_set_s(curAcc->hContact, "CList", "MyHandle", CMStringA(FORMAT, "%s [%s]", szNick.get(), Translate("Checking...")));

	if (curAcc->hosted[0]) {
		CMStringA szUrl(FORMAT, "https://www.google.com/a/%s/LoginAction", curAcc->hosted);
		CMStringA szBody("continue=https%3A%2F%2Fmail.google.com%2Fa%2F");
		szBody.Append(curAcc->hosted);
		szBody.Append("%2Ffeed%2Fatom&service=mail&userName=");
		tail = strchr(curAcc->name, '@');
		if (tail) *tail = 0;
		szBody.Append(curAcc->name);
		if (tail) *tail = '@';
		szBody.Append("&password=");
		szBody.Append(curAcc->pass);

		NETLIBHTTPHEADER headers[1] = {
			{ "Content-Type", "application/x-www-form-urlencoded" }
		};

		NETLIBHTTPREQUEST nlr = {};
		nlr.cbSize = sizeof(nlr);
		nlr.szUrl = szUrl.GetBuffer();
		nlr.requestType = REQUEST_POST;
		nlr.headersCount = _countof(headers);
		nlr.headers = headers;
		nlr.dataLength = szBody.GetLength();
		nlr.pData = szBody.GetBuffer();

		NLHR_PTR nlu(Netlib_HttpTransaction(hNetlibUser, &nlr));
		if (nlu == nullptr || nlu->resultCode != 200) {
			mir_strcpy(curAcc->results.content, Translate("Can't send account data!"));

			curAcc->results_num = -1;
			mir_strcat(curAcc->results.content, "]");
			curAcc->IsChecking = false;
		}
	}

	// go!
	CMStringA loginPass(FORMAT, "%s:%s", curAcc->name, curAcc->pass);
	ptrA loginPassEncoded(mir_base64_encode(loginPass.c_str(), loginPass.GetLength()));

	CMStringA szUrl("https://mail.google.com"), szAuth(FORMAT, "Basic %s", loginPassEncoded.get());
	if (curAcc->hosted[0])
		szUrl.AppendFormat("/a/%s/feed/atom", curAcc->hosted);
	else
		szUrl.Append("/mail/feed/atom");

	NETLIBHTTPHEADER headers[1] = {
		{ "Authorization", szAuth.GetBuffer() }
	};

	NETLIBHTTPREQUEST nlr = {};
	nlr.cbSize = sizeof(nlr);
	nlr.szUrl = szUrl.GetBuffer();
	nlr.requestType = REQUEST_GET;
	nlr.headers = headers;
	nlr.headersCount = _countof(headers);

	NLHR_PTR nlu(Netlib_HttpTransaction(hNetlibUser, &nlr));
	if (nlu == nullptr) {
		mir_snprintf(curAcc->results.content, "%s [%s]", szNick.get(), 
			(nlr.resultCode == 401) ? Translate("Wrong name or password!") : Translate("Can't get RSS feed!"));

		curAcc->results_num = -1;
	}
	else {
		curAcc->results_num = ParsePage(nlu->pData, &curAcc->results);
		mir_snprintf(curAcc->results.content, "%s [%d]", szNick.get(), curAcc->results_num);
	}

	curAcc->IsChecking = false;
}

void __cdecl Check_ThreadFunc(void *lpParam)
{
	if (lpParam) {
		CheckMailInbox((Account *)lpParam);
		NotifyUser((Account *)lpParam);
	}
	else {
		for (auto &it : g_accs) {
			if (Proto_GetBaseAccountName(it->hContact)) {
				CheckMailInbox(it);
				NotifyUser(it);
			}
		}
	}
}
