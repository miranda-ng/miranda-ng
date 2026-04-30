#include "stdafx.h"

Account* GetAccountByContact(MCONTACT hContact)
{
	if (hContact == 0)
		return nullptr;

	for (auto &it : g_accs)
		if (it->hContact == hContact)
			return it;
	return nullptr;
}

void CheckAccount(Account *pAcc)
{
	mir_cslock lck(pAcc->csLock);
	if (pAcc->bChecking)
		return;

	pAcc->bChecking = true;
	pAcc->tsLastUpdate = time(0);

	ptrW wszNick(pAcc->ppro->getWStringA(pAcc->hContact, "Nick"));
	if (!mir_wstrlen(wszNick))
		wszNick = mir_wstrdup(TranslateT("LibreView"));
	db_set_ws(pAcc->hContact, "CList", "MyHandle", CMStringW(FORMAT, L"%s [%s]", wszNick.get(), TranslateT("updating...")));

	if (!pAcc->FetchGlucose()) {
		pAcc->ppro->setWord(pAcc->hContact, "Status", ID_STATUS_OFFLINE);
		db_set_ws(pAcc->hContact, "CList", "StatusMsg", TranslateT("LibreView update failed"));
	}

	pAcc->bChecking = false;
}

void __cdecl Check_ThreadFunc(void *param)
{
	if (param) {
		CheckAccount((Account*)param);
		return;
	}

	for (auto &it : g_accs)
		if (Proto_GetBaseAccountName(it->hContact))
			CheckAccount(it);
}
