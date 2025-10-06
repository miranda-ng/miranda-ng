#include "stdafx.h"

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

void CheckMailInbox(Account *pAcc)
{
	if (pAcc->bIsChecking)
		return;

	if (!pAcc->RefreshToken())
		return;

	pAcc->bIsChecking = true;

	ptrA szNick(db_get_sa(pAcc->hContact, "CList", "MyHandle", pAcc->szName));

	char *tail = strstr(szNick, " [");
	if (tail)
		*tail = 0;

	db_set_s(pAcc->hContact, "CList", "MyHandle", CMStringA(FORMAT, "%s [%s]", szNick.get(), Translate("Checking...")));

	// go!
	MHttpRequest nlhr(REQUEST_GET);
	nlhr.m_szUrl.Format("https://gmail.googleapis.com/gmail/v1/users/me/messages?q=is:unread");
	nlhr.AddHeader("Authorization", "Bearer " + pAcc->szAccessToken);

	NLHR_PTR nlu(Netlib_HttpTransaction(hNetlibUser, &nlhr));
	if (nlu == nullptr) {
		mir_snprintf(pAcc->results.content, "%s [%s]", szNick.get(), Translate("Wrong name or password!"));
		pAcc->results_num = -1;
	}
	else {
		pAcc->results_num = ParsePage(nlu->body.GetBuffer(), &pAcc->results);
		mir_snprintf(pAcc->results.content, "%s [%d]", szNick.get(), pAcc->results_num);
	}

	pAcc->bIsChecking = false;
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
