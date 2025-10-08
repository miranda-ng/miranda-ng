#include "stdafx.h"

void CheckMailInbox(Account *pAcc)
{
	if (pAcc->bIsChecking)
		return;

	if (!pAcc->RefreshToken())
		return;

	pAcc->bIsChecking = true;

	ptrW szNick(g_plugin.getWStringA(pAcc->hContact, "Nick"));
	db_set_ws(pAcc->hContact, "CList", "MyHandle", CMStringW(FORMAT, L"%s [%s]", szNick.get(), TranslateT("Checking...")));

	// go!
	MHttpRequest nlhr(REQUEST_GET);
	nlhr.flags = NLHRF_DUMPASTEXT;
	nlhr.m_szUrl.Format("https://gmail.googleapis.com/gmail/v1/users/me/messages?q=is:unread");
	nlhr.AddHeader("Authorization", "Bearer " + pAcc->szAccessToken);

	NLHR_PTR nlu(Netlib_HttpTransaction(hNetlibUser, &nlhr));
	if (nlu == nullptr) {
		mir_snwprintf(pAcc->results.content, L"%s [%s]", szNick.get(), TranslateT("Wrong name or password!"));
		pAcc->results_num = -1;
	}
	else {
		int num = 0;

		auto *prst = &pAcc->results;
		prst->next = nullptr;

		JSONNode root(JSONNode::parse(nlu->body));
		for (auto &it : root["messages"]) {
			std::string id = it["id"].as_string();

			MHttpRequest request(REQUEST_GET);
			request.flags = NLHRF_DUMPASTEXT;
			request.m_szUrl.Format("https://gmail.googleapis.com/gmail/v1/users/me/messages/%s?format=full", id.c_str());
			request.AddHeader("Authorization", "Bearer " + pAcc->szAccessToken);

			NLHR_PTR msgnlu(Netlib_HttpTransaction(hNetlibUser, &request));
			if (msgnlu == nullptr)
				continue;

			JSONNode msg(JSONNode::parse(msgnlu->body));
			CMStringW wszSubject, wszFrom;
			for (auto &hdr : msg["payload"]["headers"]) {
				if (hdr["name"].as_string() == "Subject")
					wszSubject = hdr["value"].as_mstring();
				else if (hdr["name"].as_string() == "From")
					wszFrom = hdr["value"].as_mstring();
			}

			wszFrom.Truncate(11);
			wszSubject.Truncate(40);
			if (wszSubject.GetLength() == 40)
				wszSubject += L"...";
			wszFrom.AppendFormat(L": %s", wszSubject.c_str());

			prst->next = (resultLink *)malloc(sizeof(resultLink));
			prst = prst->next;
			wcsncpy(prst->content, wszFrom, _countof(prst->content));
			num++;
		}
		prst->next = nullptr;

		pAcc->results_num = num;
		mir_snwprintf(pAcc->results.content, L"%s [%d]", szNick.get(), pAcc->results_num);
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
