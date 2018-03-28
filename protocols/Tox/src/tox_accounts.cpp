#include "stdafx.h"

int CToxProto::OnAccountLoaded(WPARAM, LPARAM)
{
	Clist_GroupCreate(0, m_defaultGroup);

	HookProtoEvent(ME_OPT_INITIALISE, &CToxProto::OnOptionsInit);
	HookProtoEvent(ME_USERINFO_INITIALISE, &CToxProto::OnUserInfoInit);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &CToxProto::OnPreCreateMessage);

	InitCustomDbEvents();
	return 0;
}

int CToxProto::OnAccountRenamed(WPARAM, LPARAM)
{
	mir_cslock lock(m_profileLock);

	ptrW newPath(GetToxProfilePath());
	wchar_t oldPath[MAX_PATH];
	mir_snwprintf(oldPath, MAX_PATH, L"%s\\%s.tox", VARSW(L"%miranda_userdata%"), m_accountName);
	_wrename(oldPath, newPath);
	m_accountName = mir_wstrdup(m_tszUserName);
	return 0;
}

INT_PTR CToxProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)(CToxOptionsMain::CreateAccountManagerPage(this, (HWND)lParam))->GetHwnd();
}