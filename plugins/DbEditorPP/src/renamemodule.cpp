#include "stdafx.h"


int renameModule(MCONTACT hContact, const char *oldName, const char *newName)
{
	DBVARIANT dbv;
	ModuleSettingLL settinglist;

	if (IsModuleEmpty(hContact, oldName) || !EnumSettings(hContact, oldName, &settinglist))
		return 0; 

	int cnt = 0;

	for(ModSetLinkLinkItem *setting = settinglist.first; setting; setting = setting->next) {
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

INT_PTR CALLBACK AddModDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwnd);

		TCHAR msg[MSG_SIZE], name[NAME_SIZE];
		GetContactName((MCONTACT)lParam, NULL, name, _countof(name));

		mir_sntprintf(msg, TranslateT("Add module to \"%s\""), name);
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
					db_set_b(NULL, modulename, "(Default)", 0);
					for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
						db_set_b(hContact, modulename, "(Default)", 0);
				}
				else 
					db_set_b((MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA), modulename, "(Default)", 0);

				refreshTree(1);
			}
			// fall through
		case IDCANCEL:
			DestroyWindow(hwnd);
			break;
		}
	}
	return 0;
}


void addModuleDlg(MCONTACT hContact)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADD_MODULE), hwnd2mainWindow, AddModDlgProc, hContact);
}
