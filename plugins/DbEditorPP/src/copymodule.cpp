#include "stdafx.h"

void copyModule(const char *module, MCONTACT hContactFrom, MCONTACT hContactTo)
{
	ModuleSettingLL msll;

	if (IsModuleEmpty(hContactFrom, module) || !EnumSettings(hContactFrom, module, &msll))
		return;

	DBVARIANT dbv;
	for(ModSetLinkLinkItem *setting = msll.first; setting; setting = setting->next) {
		if (!db_get_s(hContactFrom, module, setting->name, &dbv, 0)) {
			db_set(hContactTo, module, setting->name, &dbv);
			db_free(&dbv);
		}
	}
	FreeModuleSettingLL(&msll);
}

INT_PTR CALLBACK copyModDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		ModuleAndContact *mac = (ModuleAndContact *)lParam;
		TCHAR name[NAME_SIZE], msg[MSG_SIZE];

		mir_sntprintf(msg, TranslateT("Copy module \"%s\""), _A2T(mac->module));
		SetWindowText(hwnd, msg);

		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) 
		{
		    if (ApplyProtoFilter(hContact))
		    	continue;

			GetContactName(hContact, NULL, name, SIZEOF(name));
				
			int index = SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_ADDSTRING, 0, (LPARAM)name);
			SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETITEMDATA, index, hContact);
		}

		GetContactName(NULL, NULL, name, SIZEOF(name));
		int index = (int)SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_INSERTSTRING, 0, (LPARAM)name);
		SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETITEMDATA, index, 0);
		SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETCURSEL, index, 0);
		break;
	}
	case WM_COMMAND:
	{
		ModuleAndContact *mac = (ModuleAndContact *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		switch (LOWORD(wParam)) {
		case CHK_COPY2ALL:
			EnableWindow(GetDlgItem(hwnd, IDC_CONTACTS), BST_UNCHECKED == IsDlgButtonChecked(hwnd, CHK_COPY2ALL));
			break;

		case IDOK:

			if (BST_UNCHECKED == IsDlgButtonChecked(hwnd, CHK_COPY2ALL)) {
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_GETITEMDATA, SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_GETCURSEL, 0, 0), 0);
				copyModule(mac->module, mac->hContact, hContact);
			}
			else {
				SetCursor(LoadCursor(NULL, IDC_WAIT));
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
					copyModule(mac->module, mac->hContact, hContact);

				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			refreshTree(1);
		// fall through
		case IDCANCEL:
			mir_free(mac);
			DestroyWindow(hwnd);
			break;
		}
		break;
	}
	} //switch
	return 0;
}

void copyModuleMenuItem(MCONTACT hContact, const char *module)
{
	ModuleAndContact *mac = (ModuleAndContact *)mir_calloc(sizeof(ModuleAndContact));
	mac->hContact = hContact;
	mir_strncpy(mac->module, module, sizeof(mac->module));

	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_COPY_MOD), hwnd2mainWindow, copyModDlgProc, (LPARAM)mac);
}


int CloneContact(MCONTACT hContact)
{
	MCONTACT newContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (!newContact)
		return 0;

	// enum all the modules
	ModuleSettingLL modlist;

	if (!EnumModules(&modlist))
		return 0;

	ModSetLinkLinkItem *mod = modlist.first;
	while (mod) {
		copyModule(mod->name, hContact, newContact);
		mod = (ModSetLinkLinkItem *)mod->next;
	}

	FreeModuleSettingLL(&modlist);
	return 1;
}
