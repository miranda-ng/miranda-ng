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

	// Get account name from Nick
	CMStringW title = getMStringW(m_hContact, "Nick");
	if (UpdateInterval > 0)
		db_set_ws(m_hContact, "CList", "MyHandle", CMStringW(FORMAT, L"%s [%s]", title.c_str(), TranslateT("updating...")));

	if (!FetchGlucose()) {
		setWord(m_hContact, "Status", ID_STATUS_OFFLINE);
		db_set_ws(m_hContact, "CList", "StatusMsg", TranslateT("LibreView update failed"));
		// Reset contact name to remove [updating...] and show error indicator
		db_set_ws(m_hContact, "CList", "MyHandle", CMStringW(FORMAT, L"%s [%s]", title.c_str(), TranslateT("error")));
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
