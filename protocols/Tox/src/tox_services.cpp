#include "common.h"

INT_PTR CALLBACK CToxProto::AccountManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

INT_PTR CToxProto::CreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_ACCMGR),
		(HWND)lParam,
		&CToxProto::AccountManagerProc,
		(LPARAM)this);
}