#include "stdafx.h"

int CToxProto::OnAccountRenamed(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;
	if (wParam == PRAC_CHANGED && pa->ppro == this) {
		mir_cslock lock(m_profileLock);

		ptrW newPath(GetToxProfilePath());
		wchar_t oldPath[MAX_PATH];
		mir_snwprintf(oldPath, MAX_PATH, L"%s\\%s.tox", VARSW(L"%miranda_userdata%").get(), m_accountName);
		_wrename(oldPath, newPath);
		m_accountName = mir_wstrdup(m_tszUserName);
	}
	return 0;
}

INT_PTR CToxProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)(CToxOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}
