#include "stdafx.h"

void copyModule(const char *module, MCONTACT hContactFrom, MCONTACT hContactTo)
{
	ModuleSettingLL msll;
	if (IsModuleEmpty(hContactFrom, module) || !EnumSettings(hContactFrom, module, &msll))
		return;

	DBVARIANT dbv;
	for (ModSetLinkLinkItem *setting = msll.first; setting; setting = setting->next) {
		if (!db_get_s(hContactFrom, module, setting->name, &dbv, 0)) {
			db_set(hContactTo, module, setting->name, &dbv);
			db_free(&dbv);
		}
	}
	FreeModuleSettingLL(&msll);
}

INT_PTR CALLBACK copyModDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ModuleAndContact *mac = (ModuleAndContact *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		{
			mac = (ModuleAndContact *)lParam;
			wchar_t name[NAME_SIZE], msg[MSG_SIZE];

			mir_snwprintf(msg, TranslateT("Copy module \"%s\""), _A2T(mac->module).get());
			SetWindowText(hwnd, msg);

			for (auto &hContact : Contacts()) {
				if (ApplyProtoFilter(hContact))
					continue;

				GetContactName(hContact, nullptr, name, _countof(name));

				int index = SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_ADDSTRING, 0, (LPARAM)name);
				SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETITEMDATA, index, hContact);
			}

			GetContactName(NULL, nullptr, name, _countof(name));
			int index = (int)SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_INSERTSTRING, 0, (LPARAM)name);
			SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETITEMDATA, index, 0);
			SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETCURSEL, index, 0);
		}
		break;

	case WM_COMMAND:
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
				SetCursor(LoadCursor(nullptr, IDC_WAIT));
				for (auto &hContact : Contacts())
					copyModule(mac->module, mac->hContact, hContact);

				SetCursor(LoadCursor(nullptr, IDC_ARROW));
			}
			refreshTree(1);
			__fallthrough;

		case IDCANCEL:
			mir_free(mac);
			DestroyWindow(hwnd);
			break;
		}
		break;
	}
	return 0;
}

void copyModuleMenuItem(HWND hwndParent, MCONTACT hContact, const char *module)
{
	ModuleAndContact *mac = (ModuleAndContact *)mir_calloc(sizeof(ModuleAndContact));
	mac->hContact = hContact;
	mir_strncpy(mac->module, module, sizeof(mac->module));

	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_COPY_MOD), hwndParent, copyModDlgProc, (LPARAM)mac);
}
