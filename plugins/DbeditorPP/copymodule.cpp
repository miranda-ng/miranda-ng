#include "headers.h"

void copyModule(char* module, HANDLE hContactFrom, HANDLE hContactTo)
{
	ModuleSettingLL msll;
	struct ModSetLinkLinkItem *setting;

	EnumSettings(hContactFrom,module, &msll);

	setting = msll.first;
	while(setting)
	{
		DBVARIANT dbv;
		if (!GetSetting(hContactFrom, module, setting->name, &dbv))
		{
			switch (dbv.type)
			{
				case DBVT_BYTE:
					DBWriteContactSettingByte(hContactTo, module, setting->name, dbv.bVal);
				break;
				case DBVT_WORD:
					DBWriteContactSettingWord(hContactTo, module, setting->name, dbv.wVal);
				break;
				case DBVT_DWORD:
					DBWriteContactSettingDword(hContactTo, module, setting->name, dbv.dVal);
				break;
				case DBVT_ASCIIZ:
					DBWriteContactSettingString(hContactTo, module, setting->name, dbv.pszVal);
				break;
				case DBVT_UTF8:
					DBWriteContactSettingStringUtf(hContactTo, module, setting->name, dbv.pszVal);
				break;
				case DBVT_BLOB:
					DBWriteContactSettingBlob(hContactTo, module, setting->name, dbv.pbVal, dbv.cpbVal);
				break;
			}
		}
		DBFreeVariant(&dbv);
		setting = (struct ModSetLinkLinkItem *)setting->next;
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
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

		while (hContact)
		{
			if (GetValue(hContact,"Protocol","p",szProto,SIZEOF(szProto)))
				loaded = IsProtocolLoaded(szProto);
			else
				loaded = 0;

			// filter
			if ((loaded && Mode == MODE_UNLOADED) || (!loaded && Mode == MODE_LOADED))
			{
				hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
				continue;
			}

			// contacts name
			if (UOS)
			{
				DBVARIANT dbv ={0};
				WCHAR nick[256];
				WCHAR protoW[256]; // unicode proto

				if (szProto[0])
					a2u(szProto, protoW, SIZEOF(protoW));
				else
					protoW[0] = 0;

				if (!szProto[0] || !loaded)
				{
					if (protoW)
					{
						if (Order)
							mir_snwprintf(nick, SIZEOF(nick), L"(%s) %s %s", protoW, GetContactName(hContact, szProto, 1), L"(UNLOADED)");
						else
							mir_snwprintf(nick, SIZEOF(nick), L"%s (%s) %s", GetContactName(hContact, szProto, 1), protoW, L"(UNLOADED)");
					}
					else
						wcscpy(nick, nick_unknownW);
				}
				else
				{
					if (Order)
						mir_snwprintf(nick, SIZEOF(nick), L"(%s) %s", protoW, GetContactName(hContact, szProto, 1));
					else
						mir_snwprintf(nick, SIZEOF(nick), L"%s (%s)", GetContactName(hContact, szProto, 1), protoW);
				}

				index = SendMessageW(GetDlgItem(hwnd, IDC_CONTACTS), CB_ADDSTRING, 0, (LPARAM)nick);
				SendMessageW(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETITEMDATA, index, (LPARAM)hContact);
			}
			else
			{
				char nick[256];

				if (!szProto[0] || !loaded)
				{
					if (szProto[0])
					{
						if (Order)
							mir_snprintf(nick, SIZEOF(nick), "(%s) %s %s", szProto, (char*)GetContactName(hContact, szProto, 0), "(UNLOADED)");
						else
							mir_snprintf(nick, SIZEOF(nick), "%s (%s) %s", (char*)GetContactName(hContact, szProto, 0), szProto, "(UNLOADED)");
					}
					else
						strcpy(nick, nick_unknown);
				}
				else
				{
					if (Order)
						mir_snprintf(nick, SIZEOF(nick), "(%s) %s", szProto, (char*)GetContactName(hContact, szProto, 0));
					else
						mir_snprintf(nick, SIZEOF(nick), "%s (%s)", (char*)GetContactName(hContact, szProto, 0), szProto);
				}

				index = SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_ADDSTRING, 0, (LPARAM)nick);
				SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETITEMDATA, index, (LPARAM)hContact);
			}

			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)(HANDLE)hContact, 0);
		}

		index = (int)SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_INSERTSTRING, 0, (LPARAM)(char*)Translate("Settings"));
		SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETITEMDATA, index, (LPARAM)0);
		SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_SETCURSEL, index, 0);

		SetWindowLongPtr(hwnd,GWLP_USERDATA,lParam);
		TranslateDialogDefault(hwnd);
	}
	else
	if (msg == WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
			case CHK_COPY2ALL:
				EnableWindow(GetDlgItem(hwnd, IDC_CONTACTS),!IsDlgButtonChecked(hwnd,CHK_COPY2ALL));
			break;
			case IDOK:
			{
				HANDLE hContact;

				if (!IsDlgButtonChecked(hwnd,CHK_COPY2ALL))
				{
					hContact = (HANDLE)SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETITEMDATA, SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETCURSEL, 0, 0), 0);
					copyModule(mac->module, mac->hContact, hContact);
				}
				else
				{
					SetCursor(LoadCursor(NULL,IDC_WAIT));
					hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

					while (hContact)
					{
						copyModule(mac->module, mac->hContact, hContact);
						hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)(HANDLE)hContact, 0);
					}

					SetCursor(LoadCursor(NULL,IDC_ARROW));
				}
				mir_free(mac);
				refreshTree(1);
				DestroyWindow(hwnd);
			}
			break;
			case IDCANCEL:
				{
					mir_free(mac);
					DestroyWindow(hwnd);
				}
			break;
		}
	}
	return 0;
}

void copyModuleMenuItem(char* module, HANDLE hContact)
{
	HWND hwnd;
	ModuleAndContact *mac = (ModuleAndContact *)mir_calloc(sizeof(ModuleAndContact));
	mac->hContact = hContact;
	strncpy(mac->module, module, 255);

	if (UOS)
		hwnd = CreateDialogParamW(hInst, MAKEINTRESOURCEW(IDD_COPY_MOD), 0, copyModDlgProc, (LPARAM)mac);
	else
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_COPY_MOD), 0, copyModDlgProc, (LPARAM)mac);
}