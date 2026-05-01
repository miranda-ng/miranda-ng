#include "stdafx.h"

void CLibreViewProto::CheckAccount()
{
	if (m_hContact == 0)
		return;

	mir_cslock lck(csLock);
	if (bChecking)
		return;

	bChecking = true;
	tsLastUpdate = time(0);

	ptrW wszNick(getWStringA(m_hContact, "Nick"));
	if (!mir_wstrlen(wszNick))
		wszNick = mir_wstrdup(TranslateT("LibreView"));
	if (UpdateInterval > 0)
		db_set_ws(m_hContact, "CList", "MyHandle", CMStringW(FORMAT, L"%s [%s]", wszNick.get(), TranslateT("updating...")));

	if (!FetchGlucose()) {
		setWord(m_hContact, "Status", ID_STATUS_OFFLINE);
		db_set_ws(m_hContact, "CList", "StatusMsg", TranslateT("LibreView update failed"));
	}

	bChecking = false;
}

void __cdecl Check_ThreadFunc(void *param)
{
	if (param) {
		((CLibreViewProto *)param)->CheckAccount();
		return;
	}

	for (auto &it : g_plugin.g_arInstances)
		it->CheckAccount();
}
