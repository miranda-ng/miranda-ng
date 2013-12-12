#include "MagneticWindowsCore.h"

TOptions Options = {
	true,
	cDefaultSnapWidth,
	false
};

INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR str[64];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_CHK_SNAP, Options.DoSnap?BST_CHECKED:BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg, IDC_SLIDER_SNAPWIDTH, TBM_SETRANGE, FALSE, MAKELONG(1,32));
		SendDlgItemMessage(hwndDlg, IDC_SLIDER_SNAPWIDTH, TBM_SETPOS, TRUE, Options.SnapWidth);

		mir_sntprintf(str, SIZEOF(str),TranslateT("%d pix"), Options.SnapWidth);
		SetDlgItemText(hwndDlg, IDC_TXT_SNAPWIDTH, str);

		EnableWindow(GetDlgItem(hwndDlg, IDC_SLIDER_SNAPWIDTH), Options.DoSnap);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_SNAPWIDTH), Options.DoSnap);

		CheckDlgButton(hwndDlg, IDC_CHK_SCRIVERWORKAROUND, Options.ScriverWorkAround?BST_CHECKED:BST_UNCHECKED);
		break;		

	case WM_HSCROLL:
		mir_sntprintf(str, SIZEOF(str), TranslateT("%d pix"), SendDlgItemMessage(hwndDlg, IDC_SLIDER_SNAPWIDTH, TBM_GETPOS, 0, 0));
		SetDlgItemText(hwndDlg, IDC_TXT_SNAPWIDTH, str);
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam)) {
		case IDC_CHK_SNAP:
			if (HIWORD(wParam) == BN_CLICKED) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_SLIDER_SNAPWIDTH), IsDlgButtonChecked(hwndDlg, IDC_CHK_SNAP));
				EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_SNAPWIDTH), IsDlgButtonChecked(hwndDlg, IDC_CHK_SNAP));

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_CHK_SCRIVERWORKAROUND:
			if (HIWORD(wParam) == BN_CLICKED)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:  //Here we have pressed either the OK or the APPLY button.
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:							
				LoadOptions();
				break;

			case PSN_APPLY:
				Options.DoSnap = (IsDlgButtonChecked(hwndDlg, IDC_CHK_SNAP) == TRUE);
				Options.SnapWidth = SendDlgItemMessage(hwndDlg, IDC_SLIDER_SNAPWIDTH, TBM_GETPOS, 0, 0);
				Options.ScriverWorkAround = (IsDlgButtonChecked(hwndDlg, IDC_CHK_SCRIVERWORKAROUND) == TRUE);

				db_set_b(NULL, MODULE_NAME, "DoSnap", Options.DoSnap);
				db_set_b(NULL, MODULE_NAME, "SnapWidth", Options.SnapWidth);
				db_set_b(NULL, MODULE_NAME, "ScriverWorkAround", Options.ScriverWorkAround);
				break;
			} 
		} 
		break;
	}
	return 0;
}

int InitOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE Opt = { sizeof(Opt) };
	Opt.pfnDlgProc = OptionsDlgProc;
	Opt.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAGNETICWINDOWS);
	Opt.hInstance = hInst;
	Opt.pszGroup = LPGEN("Customize");
	Opt.pszTitle = LPGEN("Magnetic Windows");
	Opt.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &Opt);
	return 0;
}

void LoadOptions()
{
	Options.DoSnap = db_get_b(NULL, MODULE_NAME, "DoSnap", 1) != 0;
	Options.SnapWidth = db_get_b(NULL, MODULE_NAME, "SnapWidth", cDefaultSnapWidth);
	Options.ScriverWorkAround = db_get_b(NULL, MODULE_NAME, "ScriverWorkAround", 0) != 0;
}
