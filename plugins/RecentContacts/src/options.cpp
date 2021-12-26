#include "stdafx.h"

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

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case EN_CHANGE:
		case BN_CLICKED:
		case CBN_EDITCHANGE:
		case CBN_SELCHANGE:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		LPNMHDR phdr = (LPNMHDR)(lParam);
		if (phdr->idFrom == 0 && phdr->code == PSN_APPLY) {
			LastUCOpt.HideOffline = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_HIDEOFFLINE);
			g_plugin.setByte(dbLastUC_HideOfflineContacts, (uint8_t)LastUCOpt.HideOffline);

			LastUCOpt.WindowAutoSize = (BOOL)IsDlgButtonChecked(hwndDlg, IDC_WINDOWAUTOSIZE);
			g_plugin.setByte(dbLastUC_WindowAutosize, (uint8_t)LastUCOpt.WindowAutoSize);

			GetDlgItemTextA(hwndDlg, IDC_SHOWNCONTACTS, str, _countof(str));
			LastUCOpt.MaxShownContacts = atoi(str);
			g_plugin.setByte(dbLastUC_MaxShownContacts, LastUCOpt.MaxShownContacts);

			GetDlgItemTextA(hwndDlg, IDC_DATETIME, str, _countof(str));
			g_plugin.setString(dbLastUC_DateTimeFormat, str);

			LoadDBSettings();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_LASTUC_OPT);
	odp.szTitle.a = LPGEN("Recent Contacts");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
