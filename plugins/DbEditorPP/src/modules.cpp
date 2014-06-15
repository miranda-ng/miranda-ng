#include "headers.h"

void renameModule(char* oldName, char* newName, MCONTACT hContact)
{
	DBVARIANT dbv;
	ModuleSettingLL settinglist;
	ModSetLinkLinkItem *setting;

	if (!EnumSettings(hContact, oldName, &settinglist)) { msg(Translate("Error Loading Setting List"), modFullname); return; }

	setting = settinglist.first;
	while (setting) {
		if (!GetSetting(hContact, oldName, setting->name, &dbv)) {
			switch (dbv.type) {
			case DBVT_BYTE:
				db_set_b(hContact, newName, setting->name, dbv.bVal);
				break;
			case DBVT_WORD:
				db_set_w(hContact, newName, setting->name, dbv.wVal);
				break;
			case DBVT_DWORD:
				db_set_dw(hContact, newName, setting->name, dbv.dVal);
				break;
			case DBVT_ASCIIZ:
				db_set_s(hContact, newName, setting->name, dbv.pszVal);
				break;
			case DBVT_UTF8:
				db_set_utf(hContact, newName, setting->name, dbv.pszVal);
				break;
			case DBVT_BLOB:
				db_set_blob(hContact, newName, setting->name, dbv.pbVal, dbv.cpbVal);
				break;

			}
			db_unset(hContact, oldName, setting->name);
		}
		db_free(&dbv);
		setting = (ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
}

INT_PTR CALLBACK AddModDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG) {
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		TranslateDialogDefault(hwnd);
	}

	if (msg == WM_COMMAND) {
		switch (LOWORD(wParam)) {
		case IDOK:
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_MODNAME))) {
				char modulename[256];
				GetDlgItemText(hwnd, IDC_MODNAME, modulename, 256);
				if (IsDlgButtonChecked(hwnd, CHK_ADD2ALL)) {
					// null contact
					db_set_b(NULL, modulename, "(Default)", 0);
					for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
						db_set_b(hContact, modulename, "(Default)", 0);
				}
				else db_set_b((MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA), modulename, "(Default)", 0);

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

int CloneContact(MCONTACT hContact)
{
	MCONTACT newContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (!newContact)
		return 0;

	// enum all the modules
	ModuleSettingLL modlist;
	if (!EnumModules(&modlist)) {
		msg(Translate("Error loading module list"), modFullname);
		return 0;
	}

	ModSetLinkLinkItem *mod = modlist.first;
	while (mod) {
		copyModule(mod->name, hContact, newContact);
		mod = (ModSetLinkLinkItem *)mod->next;
	}
	FreeModuleSettingLL(&modlist);
	return 1;
}
