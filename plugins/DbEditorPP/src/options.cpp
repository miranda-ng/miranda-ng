#include "stdafx.h"

static INT_PTR CALLBACK DlgProcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInitDone = true;
	switch (msg) {
	case WM_INITDIALOG:
		bInitDone = false;
		CheckDlgButton(hwnd, IDC_EXPANDSETTINGS, db_get_b(NULL, MODULENAME, "ExpandSettingsOnOpen", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_RESTORESETTINGS, db_get_b(NULL, MODULENAME, "RestoreOnOpen", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_WARNONDEL, db_get_b(NULL, MODULENAME, "WarnOnDelete", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_POPUPS, g_bUsePopups ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwnd, IDC_POPUPTIMEOUT, db_get_w(NULL, MODULENAME, "PopupDelay", 4), 0);
		SendDlgItemMessage(hwnd, IDC_COLOUR, CPM_SETCOLOUR, 0, (LPARAM)db_get_dw(NULL, MODULENAME, "PopupColour", RGB(255, 0, 0)));
		TranslateDialogDefault(hwnd);
		bInitDone = true;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RESTORESETTINGS:
		case IDC_EXPANDSETTINGS:
		case IDC_POPUPS:
		case IDC_WARNONDEL:
		case IDC_COLOUR:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_POPUPTIMEOUT:
			if (bInitDone && (HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, MODULENAME, "ExpandSettingsOnOpen", (BYTE)IsDlgButtonChecked(hwnd, IDC_EXPANDSETTINGS));
				db_set_b(NULL, MODULENAME, "RestoreOnOpen", (BYTE)IsDlgButtonChecked(hwnd, IDC_RESTORESETTINGS));
				db_set_b(NULL, MODULENAME, "WarnOnDelete", (BYTE)IsDlgButtonChecked(hwnd, IDC_WARNONDEL));
				g_bUsePopups = IsDlgButtonChecked(hwnd, IDC_POPUPS) != 0;
				db_set_b(NULL, MODULENAME, "UsePopUps", (BYTE)g_bUsePopups);
				db_set_w(NULL, MODULENAME, "PopupDelay", (WORD)GetDlgItemInt(hwnd, IDC_POPUPTIMEOUT, nullptr, 0));
				db_set_dw(NULL, MODULENAME, "PopupColour", (DWORD)SendDlgItemMessage(hwnd, IDC_COLOUR, CPM_GETCOLOUR, 0, 0));
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

INT OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szGroup.a = LPGEN("Database");
	odp.szTitle.a = modFullname;
	odp.pfnDlgProc = DlgProcOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
