#include "stdafx.h"

extern HINSTANCE hInst;
void LoadDBSettings();

static INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char str[32];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_HIDEOFFLINE, (LastUCOpt.HideOffline ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_WINDOWAUTOSIZE, (LastUCOpt.WindowAutoSize ? BST_CHECKED : BST_UNCHECKED));

		mir_snprintf(str, "%d", LastUCOpt.MaxShownContacts);
		SetDlgItemTextA(hwndDlg, IDC_SHOWNCONTACTS, str);

		strncpy_s(str, LastUCOpt.DateTimeFormat.c_str(), _TRUNCATE);
		SetDlgItemTextA(hwndDlg, IDC_DATETIME, str);

		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
		return TRUE;

	case WM_COMMAND:
		switch(HIWORD(wParam)) {
		case EN_CHANGE:
		case BN_CLICKED:
		case CBN_EDITCHANGE:
		case CBN_SELCHANGE:
			SendMessage(GetParent(hwndDlg),PSM_CHANGED,0,0);
		}
		break;

	case WM_NOTIFY:
		LPNMHDR phdr = (LPNMHDR)(lParam);
		if (phdr->idFrom == 0 && phdr->code == PSN_APPLY) {
			LastUCOpt.HideOffline = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_HIDEOFFLINE);
			db_set_b(NULL, MODULENAME, dbLastUC_HideOfflineContacts, (BYTE)LastUCOpt.HideOffline);

			LastUCOpt.WindowAutoSize = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_WINDOWAUTOSIZE);
			db_set_b(NULL, MODULENAME, dbLastUC_WindowAutosize, (BYTE)LastUCOpt.WindowAutoSize);

			GetDlgItemTextA(hwndDlg, IDC_SHOWNCONTACTS, str, _countof(str));
			LastUCOpt.MaxShownContacts= atoi(str);
			db_set_b(0,MODULENAME, dbLastUC_MaxShownContacts, LastUCOpt.MaxShownContacts);

			GetDlgItemTextA(hwndDlg, IDC_DATETIME, str, _countof(str));
			db_set_s(0,MODULENAME, dbLastUC_DateTimeFormat, str );

			LoadDBSettings();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.szGroup.a = LPGEN("Contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_LASTUC_OPT);
	odp.szTitle.a = LPGEN("Recent Contacts");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
