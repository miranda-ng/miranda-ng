#include "headers.h"

static int working;
static HWND hwnd2Delete = NULL;

int deleteModule(char* module, HANDLE hContact, int fromMenu)
{
	char msg[1024];
	ModuleSettingLL settinglist;
	struct ModSetLinkLinkItem *setting;

	if (!module) return 0;

	if (!fromMenu)
	{
		mir_snprintf(msg, SIZEOF(msg), Translate("Are you sure you want to delete module \"%s\"?"), module);
		if (DBGetContactSettingByte(NULL,modname, "WarnOnDelete",1))
		{
			if (MessageBox(0,msg, Translate("Confirm Module Deletion"), MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
				return 0;
		}
	}

	if (!EnumSettings(hContact,module,&settinglist)) return 0;

	setting = settinglist.first;
	while (setting)
	{
		DBDeleteContactSetting(hContact, module, setting->name);
		setting = (struct ModSetLinkLinkItem *)setting->next;
	}
	FreeModuleSettingLL(&settinglist);
	return 1;
}

void __cdecl PopulateModuleDropListThreadFunc(LPVOID di)
{
	HWND hwnd = (HWND)di;
	ModuleSettingLL msll;
	struct ModSetLinkLinkItem *module;
	HANDLE hContact;
	int moduleEmpty;
	if (!EnumModules(&msll)) DestroyWindow(hwnd);
	module = msll.first;
	while (module && working)
	{
		moduleEmpty = 1;
		// check the null
		if (!IsModuleEmpty(NULL,module->name))
		{
			SendDlgItemMessage(hwnd,IDC_CONTACTS,CB_ADDSTRING,0,(LPARAM)module->name);
			moduleEmpty = 0;
			module = (struct ModSetLinkLinkItem *)module->next;
			continue;
		}
		for (hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);moduleEmpty && hContact;hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
		{
			if (!IsModuleEmpty(hContact,module->name))
			{
				SendDlgItemMessage(hwnd,IDC_CONTACTS,CB_ADDSTRING,0,(LPARAM)module->name);
				moduleEmpty = 0;
				break;
			}
		}

		module = (struct ModSetLinkLinkItem *)module->next;
		SendDlgItemMessage(hwnd,IDC_CONTACTS,CB_SETCURSEL,0,0);
	}
	SendDlgItemMessage(hwnd,IDC_CONTACTS,CB_SETCURSEL,0,0);
	FreeModuleSettingLL(&msll);
	SetWindowText(hwnd,Translate("Delete module from Database"));
	EnableWindow(GetDlgItem(hwnd,IDC_CONTACTS),1);
	EnableWindow(GetDlgItem(hwnd,IDOK),1);
	EnableWindow(GetDlgItem(hwnd,IDCANCEL),1);

	if (!working)
		PostMessage(hwnd, WM_COMMAND, (WPARAM)IDCANCEL, 0);
	else
		working = 2;
}

INT_PTR CALLBACK DeleteModuleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetWindowText(hwnd,Translate("Delete module from Database... Loading"));
			EnableWindow(GetDlgItem(hwnd,IDC_CONTACTS),0);
			EnableWindow(GetDlgItem(hwnd,IDOK),0);
			SetDlgItemText(hwnd,IDC_INFOTEXT,"Delete module from Database");
			SetDlgItemText(hwnd,CHK_COPY2ALL,"Delete module from all contacts (Includes Setting)");
			EnableWindow(GetDlgItem(hwnd,CHK_COPY2ALL),0);
			CheckDlgButton(hwnd,CHK_COPY2ALL,1);
			TranslateDialogDefault(hwnd);
			working = 1;
			forkthread(PopulateModuleDropListThreadFunc,0,hwnd);
		}
		return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					char text[128];
					HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
					GetDlgItemText(hwnd,IDC_CONTACTS,text,128);
					SetCursor(LoadCursor(NULL,IDC_WAIT));
					while (hContact)
					{
						deleteModule(text,hContact,1);
						hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
					}
					// do the null
					deleteModule(text,NULL,1);
					SetCursor(LoadCursor(NULL,IDC_ARROW));
					refreshTree(1);
				}
					// fall through
				case IDCANCEL:
				{
					if (working == 1)
					{
						working = 0;
						EnableWindow(GetDlgItem(hwnd,IDCANCEL),0);
					}
					else
						DestroyWindow(hwnd);
				}
				break;
			}
		break;
		case WM_DESTROY:
			hwnd2Delete = NULL;
		break;
	}
	return 0;
}



void deleteModuleGui()
{
    if (!hwnd2Delete)
		hwnd2Delete = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_COPY_MOD), hwnd2mainWindow, DeleteModuleDlgProc, (LPARAM)0);
	else
		SetForegroundWindow(hwnd2Delete);
}