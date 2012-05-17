#include "headers.h"


void renameModule(char* oldName, char* newName, HANDLE hContact)
{
	DBVARIANT dbv;
	ModuleSettingLL settinglist;
	struct ModSetLinkLinkItem *setting;

	if (!EnumSettings(hContact,oldName,&settinglist)) { msg(Translate("Error Loading Setting List"),modFullname); return;}

	setting = settinglist.first;
	while (setting)
	{
		if (!GetSetting(hContact,oldName,setting->name,&dbv))
		{
			switch (dbv.type)
			{
				case DBVT_BYTE:
					DBWriteContactSettingByte(hContact, newName, setting->name, dbv.bVal);
				break;
				case DBVT_WORD:
					DBWriteContactSettingWord(hContact, newName, setting->name, dbv.wVal);
				break;
				case DBVT_DWORD:
					DBWriteContactSettingDword(hContact, newName, setting->name, dbv.dVal);
				break;
				case DBVT_ASCIIZ:
					DBWriteContactSettingString(hContact, newName, setting->name, dbv.pszVal);
				break;
				case DBVT_UTF8:
					DBWriteContactSettingStringUtf(hContact, newName, setting->name, dbv.pszVal);
				break;
				case DBVT_BLOB:
					DBWriteContactSettingBlob(hContact, newName, setting->name, dbv.pbVal, dbv.cpbVal);
				break;

			}
			DBDeleteContactSetting(hContact, oldName, setting->name);
		}
		DBFreeVariant(&dbv);
		setting = (struct ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
}

INT_PTR CALLBACK AddModDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lParam);
		TranslateDialogDefault(hwnd);
	}
	if (msg == WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
			case IDOK:
			{
				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_MODNAME)))
				{
					char modulename[256];
					GetDlgItemText(hwnd, IDC_MODNAME, modulename, 256);
					if (IsDlgButtonChecked(hwnd,CHK_ADD2ALL))
					{
						HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
						// null contact
						DBWriteContactSettingByte(NULL, modulename, "(Default)", 0);
						while (hContact)
						{
							DBWriteContactSettingByte(hContact, modulename, "(Default)", 0);
							hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)(HANDLE)hContact, 0);
						}
					}
					else
					{
						DBWriteContactSettingByte((HANDLE)GetWindowLongPtr(hwnd,GWLP_USERDATA), modulename, "(Default)", 0);
					}
					refreshTree(1);
				}
			}
			// fall through
			case IDCANCEL:
				DestroyWindow(hwnd);
			break;
		}
	}
	return 0;
}

int CloneContact(HANDLE hContact)
{
	HANDLE newContact = (HANDLE)CallService(MS_DB_CONTACT_ADD,0,0);

	ModuleSettingLL modlist;
	struct ModSetLinkLinkItem *mod;
	if (!newContact) return 0;
	// enum all the modules
	if (!EnumModules(&modlist)) { msg(Translate("Error Loading Module List"),modFullname); return 0;}

	mod = modlist.first;
	while (mod)
	{
		copyModule(mod->name,hContact,newContact);
		mod = (struct ModSetLinkLinkItem *)mod->next;
	}
	FreeModuleSettingLL(&modlist);
	return 1;
}