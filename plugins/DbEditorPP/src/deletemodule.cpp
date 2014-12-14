#include "headers.h"

static int working;
static HWND hwnd2Delete = NULL;

int deleteModule(char *module, MCONTACT hContact, int fromMenu)
{
	if (!module)
		return 0;

	if (!fromMenu && db_get_b(NULL, modname, "WarnOnDelete", 1)) {
		char msg[1024];
		mir_snprintf(msg, SIZEOF(msg), Translate("Are you sure you want to delete module \"%s\"?"), module);
		if (MessageBox(0, msg, Translate("Confirm module deletion"), MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			return 0;
	}

	ModuleSettingLL settinglist;
	if (!EnumSettings(hContact, module, &settinglist))
		return 0;

	for (ModSetLinkLinkItem *setting = settinglist.first; setting; setting = setting->next) {
		db_unset(hContact, module, setting->name);
	}

	FreeModuleSettingLL(&settinglist);
	return 1;
}

void __cdecl PopulateModuleDropListThreadFunc(void *di)
{
	HWND hwnd = (HWND)di;
	ModuleSettingLL msll;
	if (!EnumModules(&msll)) {
		DestroyWindow(hwnd);
		return;
	}
	int moduleEmpty;
	ModSetLinkLinkItem *module = msll.first;
	while (module && working) {
		moduleEmpty = 1;
		// check the null
		if (!IsModuleEmpty(NULL, module->name)) {
			SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_ADDSTRING, 0, (LPARAM)module->name);
			moduleEmpty = 0;
			module = module->next;
			continue;
		}
		for (MCONTACT hContact = db_find_first(); moduleEmpty && hContact; hContact = db_find_next(hContact)) {
			if (!IsModuleEmpty(hContact, module->name)) {
				SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_ADDSTRING, 0, (LPARAM)module->name);
				moduleEmpty = 0;
				break;
			}
		}

		module = module->next;
		SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETCURSEL, 0, 0);
	}
	SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETCURSEL, 0, 0);
	FreeModuleSettingLL(&msll);
	SetWindowText(hwnd, Translate("Delete module from database"));
	EnableWindow(GetDlgItem(hwnd, IDC_CONTACTS), 1);
	EnableWindow(GetDlgItem(hwnd, IDOK), 1);
	EnableWindow(GetDlgItem(hwnd, IDCANCEL), 1);

	if (!working)
		PostMessage(hwnd, WM_COMMAND, (WPARAM)IDCANCEL, 0);
	else
		working = 2;
}

INT_PTR CALLBACK DeleteModuleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowText(hwnd, Translate("Delete module from database... Loading"));
		EnableWindow(GetDlgItem(hwnd, IDC_CONTACTS), 0);
		EnableWindow(GetDlgItem(hwnd, IDOK), 0);
		SetDlgItemText(hwnd, IDC_INFOTEXT, Translate("Delete module from database"));
		SetDlgItemText(hwnd, CHK_COPY2ALL, Translate("Delete module from all contacts (including Setting)"));
		EnableWindow(GetDlgItem(hwnd, CHK_COPY2ALL), 0);
		CheckDlgButton(hwnd, CHK_COPY2ALL, BST_CHECKED);
		TranslateDialogDefault(hwnd);
		working = 1;
		forkthread(PopulateModuleDropListThreadFunc, 0, hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			char text[128];
			GetDlgItemText(hwnd, IDC_CONTACTS, text, SIZEOF(text));
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
				deleteModule(text, hContact, 1);

			// do the null
			deleteModule(text, NULL, 1);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			refreshTree(1);
		}
		// fall through
		case IDCANCEL:
			if (working == 1) {
				working = 0;
				EnableWindow(GetDlgItem(hwnd, IDCANCEL), 0);
			}
			else DestroyWindow(hwnd);
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
		hwnd2Delete = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_COPY_MOD), hwnd2mainWindow, DeleteModuleDlgProc, 0);
	else
		SetForegroundWindow(hwnd2Delete);
}