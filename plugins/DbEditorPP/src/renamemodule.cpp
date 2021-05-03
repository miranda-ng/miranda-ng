#include "stdafx.h"

int renameModule(MCONTACT hContact, const char *oldName, const char *newName)
{
	ModuleSettingLL settinglist;
	if (IsModuleEmpty(hContact, oldName) || !EnumSettings(hContact, oldName, &settinglist))
		return 0;

	int cnt = 0;

	for (ModSetLinkLinkItem *setting = settinglist.first; setting; setting = setting->next) {
		DBVARIANT dbv;
		if (!db_get_s(hContact, oldName, setting->name, &dbv, 0)) {
			db_set(hContact, newName, setting->name, &dbv);
			db_unset(hContact, oldName, setting->name);
			db_free(&dbv);
			cnt++;
		}
	}
	FreeModuleSettingLL(&settinglist);
	return cnt;
}

static INT_PTR CALLBACK AddModDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwnd);

		wchar_t msg[MSG_SIZE], name[NAME_SIZE];
		GetContactName((MCONTACT)lParam, nullptr, name, _countof(name));

		mir_snwprintf(msg, TranslateT("Add module to \"%s\""), name);
		SetWindowText(hwnd, msg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_MODNAME))) {
				char modulename[FLD_SIZE];
				GetDlgItemTextA(hwnd, IDC_MODNAME, modulename, _countof(modulename));
				if (IsDlgButtonChecked(hwnd, CHK_ADD2ALL)) {
					// null contact
					db_set_b(0, modulename, "(Default)", 0);
					for (auto &hContact : Contacts())
						db_set_b(hContact, modulename, "(Default)", 0);
				}
				else db_set_b((MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA), modulename, "(Default)", 0);

				refreshTree(1);
			}
			__fallthrough;

		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
	}
	return 0;
}

void CMainDlg::addModuleDlg(MCONTACT hContact)
{
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ADD_MODULE), m_hwnd, AddModDlgProc, hContact);
}
