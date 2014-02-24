#include "headers.h"

void copyModule(char* module, MCONTACT hContactFrom, MCONTACT hContactTo)
{
	ModuleSettingLL msll;

	EnumSettings(hContactFrom, module, &msll);

	ModSetLinkLinkItem *setting = msll.first;
	while (setting) {
		DBVARIANT dbv;
		if (!GetSetting(hContactFrom, module, setting->name, &dbv)) {
			switch (dbv.type) {
			case DBVT_BYTE:
				db_set_b(hContactTo, module, setting->name, dbv.bVal);
				break;
			case DBVT_WORD:
				db_set_w(hContactTo, module, setting->name, dbv.wVal);
				break;
			case DBVT_DWORD:
				db_set_dw(hContactTo, module, setting->name, dbv.dVal);
				break;
			case DBVT_ASCIIZ:
				db_set_s(hContactTo, module, setting->name, dbv.pszVal);
				break;
			case DBVT_UTF8:
				db_set_utf(hContactTo, module, setting->name, dbv.pszVal);
				break;
			case DBVT_BLOB:
				db_set_blob(hContactTo, module, setting->name, dbv.pbVal, dbv.cpbVal);
				break;
			}
		}
		db_free(&dbv);
		setting = setting->next;
	}
	FreeModuleSettingLL(&msll);
}

INT_PTR CALLBACK copyModDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ModuleAndContact *mac = (ModuleAndContact *)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	if (msg == WM_INITDIALOG)
	{
		int index, loaded;
		char szProto[256];
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			if (GetValue(hContact,"Protocol","p",szProto,SIZEOF(szProto)))
				loaded = IsProtocolLoaded(szProto);
			else
				loaded = 0;

			// filter
			if ((loaded && Mode == MODE_UNLOADED) || (!loaded && Mode == MODE_LOADED))
				continue;

			// contacts name
			DBVARIANT dbv ={0};
			WCHAR nick[256];
			WCHAR protoW[256]; // unicode proto

			if (szProto[0])
				a2u(szProto, protoW, SIZEOF(protoW));
			else
				protoW[0] = 0;

			if (!szProto[0] || !loaded) {
				if (protoW) {
					if (Order)
						mir_snwprintf(nick, SIZEOF(nick), L"(%s) %s %s", protoW, GetContactName(hContact, szProto, 1), L"(UNLOADED)");
					else
						mir_snwprintf(nick, SIZEOF(nick), L"%s (%s) %s", GetContactName(hContact, szProto, 1), protoW, L"(UNLOADED)");
				}
				else wcscpy(nick, nick_unknownW);
			}
			else {
				if (Order)
					mir_snwprintf(nick, SIZEOF(nick), L"(%s) %s", protoW, GetContactName(hContact, szProto, 1));
				else
					mir_snwprintf(nick, SIZEOF(nick), L"%s (%s)", GetContactName(hContact, szProto, 1), protoW);
			}

			index = SendMessageW(GetDlgItem(hwnd, IDC_CONTACTS), CB_ADDSTRING, 0, (LPARAM)nick);
			SendMessageW(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETITEMDATA, index, hContact);
		}

		index = (int)SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_INSERTSTRING, 0, (LPARAM)(char*)Translate("Settings"));
		SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETITEMDATA, index, 0);
		SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETCURSEL, index, 0);

		SetWindowLongPtr(hwnd,GWLP_USERDATA,lParam);
		TranslateDialogDefault(hwnd);
	}
	else if (msg == WM_COMMAND)
	{
		switch(LOWORD(wParam)) {
		case CHK_COPY2ALL:
			EnableWindow(GetDlgItem(hwnd, IDC_CONTACTS),!IsDlgButtonChecked(hwnd,CHK_COPY2ALL));
			break;

		case IDOK:
			if (!IsDlgButtonChecked(hwnd,CHK_COPY2ALL)) {
				MCONTACT hContact = (MCONTACT)SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETITEMDATA, SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETCURSEL, 0, 0), 0);
				copyModule(mac->module, mac->hContact, hContact);
			}
			else {
				SetCursor(LoadCursor(NULL,IDC_WAIT));
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
					copyModule(mac->module, mac->hContact, hContact);

				SetCursor(LoadCursor(NULL,IDC_ARROW));
			}
			mir_free(mac);
			refreshTree(1);
			DestroyWindow(hwnd);
			break;

		case IDCANCEL:
			mir_free(mac);
			DestroyWindow(hwnd);
			break;
		}
	}
	return 0;
}

void copyModuleMenuItem(char* module, MCONTACT hContact)
{
	ModuleAndContact *mac = (ModuleAndContact *)mir_calloc(sizeof(ModuleAndContact));
	mac->hContact = hContact;
	strncpy(mac->module, module, 255);

	CreateDialogParamW(hInst, MAKEINTRESOURCEW(IDD_COPY_MOD), 0, copyModDlgProc, (LPARAM)mac);
}