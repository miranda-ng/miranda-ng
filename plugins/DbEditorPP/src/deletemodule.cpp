#include "stdafx.h"

volatile BOOL working;
static HWND hwnd2Delete = nullptr;

int deleteModule(MCONTACT hContact, const char *module, int confirm)
{
	if (!module || IsModuleEmpty(hContact, module))
		return 0;

	if (confirm && g_plugin.bWarnOnDelete) {
		wchar_t text[MSG_SIZE];
		mir_snwprintf(text, TranslateT("Are you sure you want to delete module \"%s\"?"), _A2T(module).get());
		if (dlg(text, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
			return 0;
	}

	ModuleSettingLL settinglist;
	if (!EnumSettings(hContact, module, &settinglist))
		return 0;

	for (ModSetLinkLinkItem *setting = settinglist.first; setting; setting = setting->next)
		db_unset(hContact, module, setting->name);

	FreeModuleSettingLL(&settinglist);
	return 1;
}

void __cdecl PopulateModuleDropListThreadFunc(void *param)
{
	HWND hwnd = (HWND)param;
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
			SendDlgItemMessageA(hwnd, IDC_CONTACTS, CB_ADDSTRING, 0, (LPARAM)module->name);
			moduleEmpty = 0;
			module = module->next;
			continue;
		}

		for (auto &hContact : Contacts()) {
			if (!IsModuleEmpty(hContact, module->name)) {
				SendDlgItemMessageA(hwnd, IDC_CONTACTS, CB_ADDSTRING, 0, (LPARAM)module->name);
				moduleEmpty = 0;
				break;
			}
		}

		module = module->next;
		SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETCURSEL, 0, 0);
	}
	SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_SETCURSEL, 0, 0);
	FreeModuleSettingLL(&msll);
	SetWindowText(hwnd, TranslateT("Delete module from database"));
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
		SetWindowText(hwnd, LPGENW("Delete module from database... Loading"));
		EnableWindow(GetDlgItem(hwnd, IDC_CONTACTS), 0);
		EnableWindow(GetDlgItem(hwnd, IDOK), 0);
		SetDlgItemText(hwnd, IDC_INFOTEXT, LPGENW("Delete module from database"));
		SetDlgItemText(hwnd, CHK_COPY2ALL, LPGENW("Delete from all contacts (including Settings)"));
		EnableWindow(GetDlgItem(hwnd, CHK_COPY2ALL), 0);
		CheckDlgButton(hwnd, CHK_COPY2ALL, BST_CHECKED);
		TranslateDialogDefault(hwnd);
		working = 1;
		mir_forkthread(PopulateModuleDropListThreadFunc, hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			char module[FLD_SIZE];
			GetDlgItemTextA(hwnd, IDC_CONTACTS, module, _countof(module));
			SetCursor(LoadCursor(nullptr, IDC_WAIT));
			for (auto &hContact : Contacts())
				deleteModule(hContact, module, 0);

			// do the null
			deleteModule(NULL, module, 0);
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			refreshTree(1);
			__fallthrough;

		case IDCANCEL:
			if (working == 1) {
				working = 0;
				EnableWindow(GetDlgItem(hwnd, IDCANCEL), 0);
			}
			else DestroyWindow(hwnd);
		}
		break;

	case WM_DESTROY:
		hwnd2Delete = nullptr;
		break;
	}
	return 0;
}

void deleteModuleDlg()
{
	if (!hwnd2Delete)
		hwnd2Delete = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_COPY_MOD), hwnd2mainWindow, DeleteModuleDlgProc);
	else
		SetForegroundWindow(hwnd2Delete);
}
