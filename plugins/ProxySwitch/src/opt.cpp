/*
proxySwitch

The plugin watches IP address changes, reports them via popups and adjusts
the proxy settings of Miranda and Internet Explorer accordingly.
*/

#include "stdafx.h"

int help_shown;

static INT_PTR CALLBACK HelpProc(HWND hdlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		return 1;

	case WM_COMMAND:
		if (wParam == IDOK || wParam == IDCANCEL)
			EndDialog(hdlg, 0);
		break;

	case WM_CLOSE:
		EndDialog(hdlg, 0);
		break;
	}

	return 0;
}

INT_PTR CALLBACK OptionsProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam) 
{
	switch (msg) {

	case WM_INITDIALOG:
		opt_startup = TRUE;
		LoadSettings();
		SetDlgItemText(hdlg, IDC_EDIT_USEPROXY, opt_useProxy);
		SetDlgItemText(hdlg, IDC_EDIT_NOPROXY, opt_noProxy);
		CheckDlgButton(hdlg, IDC_CHECK_MIRANDA, opt_miranda ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_IE, opt_ie ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_FIREFOX, opt_firefox ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_ALWAY_RECONNECT, opt_alwayReconnect ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hdlg, IDC_EDIT_HIDEINTF, opt_hideIntf);
		SendDlgItemMessage(hdlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, opt_bgColor);
		SendDlgItemMessage(hdlg, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, opt_txtColor);
		CheckDlgButton(hdlg, IDC_CHECK_POPUPS, opt_popups ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_DEFAULTCOLORS, opt_defaultColors ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_SHOWPROXYSTATUS, opt_showProxyState ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_CHECK_FIREFOX), Firefox_Installed());
		EnableWindow(GetDlgItem(hdlg, IDC_CHECK_POPUPS), opt_popupPluginInstalled);
		EnableWindow(GetDlgItem(hdlg, IDC_BGCOLOR), opt_popupPluginInstalled && opt_popups && !opt_defaultColors);
		EnableWindow(GetDlgItem(hdlg, IDC_TEXTCOLOR), opt_popupPluginInstalled && opt_popups && !opt_defaultColors);
		EnableWindow(GetDlgItem(hdlg, IDC_CHECK_DEFAULTCOLORS), opt_popupPluginInstalled && opt_popups);
		EnableWindow(GetDlgItem(hdlg, IDC_CHECK_SHOWPROXYSTATUS), opt_popupPluginInstalled && opt_popups);
		ShowWindow(GetDlgItem(hdlg, IDC_RESTARTREQUIRED), opt_not_restarted ? SW_SHOW : SW_HIDE);
		TranslateDialogDefault(hdlg);
		opt_startup = FALSE;
		return 1;

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->code) {
		case PSN_APPLY:
			ShowWindow(GetDlgItem(hdlg, IDC_RESTARTREQUIRED), opt_not_restarted ? SW_SHOW : SW_HIDE);
			GetDlgItemText(hdlg, IDC_EDIT_NOPROXY, opt_noProxy, MAX_IPLIST_LENGTH);
			GetDlgItemText(hdlg, IDC_EDIT_USEPROXY, opt_useProxy, MAX_IPLIST_LENGTH);
			GetDlgItemText(hdlg, IDC_EDIT_HIDEINTF, opt_hideIntf, MAX_IPLIST_LENGTH);
			opt_miranda = IsDlgButtonChecked(hdlg, IDC_CHECK_MIRANDA);
			opt_ie = IsDlgButtonChecked(hdlg, IDC_CHECK_IE);
			opt_firefox = IsDlgButtonChecked(hdlg, IDC_CHECK_FIREFOX);
			opt_alwayReconnect = IsDlgButtonChecked(hdlg, IDC_CHECK_ALWAY_RECONNECT);
			opt_popups = IsDlgButtonChecked(hdlg, IDC_CHECK_POPUPS);
			opt_defaultColors = IsDlgButtonChecked(hdlg, IDC_CHECK_DEFAULTCOLORS);
			opt_showProxyState = IsDlgButtonChecked(hdlg, IDC_CHECK_SHOWPROXYSTATUS);
			opt_bgColor = SendDlgItemMessage(hdlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
			opt_txtColor = SendDlgItemMessage(hdlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0);
			SaveSettings();
			UpdatePopupMenu(opt_popups);
			return 1;
		}
		break;

	case WM_COMMAND:
		if (opt_startup)
			return 0;

		if (HIWORD(wparam) == BN_CLICKED && GetFocus() == (HWND)lparam && LOWORD(wparam) != IDC_BTN_HELP)
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wparam)) {
		case IDC_BTN_HELP:
			DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HELP), 0, HelpProc);
			break;

		case IDC_EDIT_USEPROXY:
		case IDC_EDIT_NOPROXY:
		case IDC_EDIT_HIDEINTF:
			if (HIWORD(wparam) == EN_CHANGE && (HWND)lparam == GetFocus()) 
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BGCOLOR:
		case IDC_TEXTCOLOR:
			if (HIWORD(wparam) == CPN_COLOURCHANGED)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_CHECK_DEFAULTCOLORS:
		case IDC_CHECK_POPUPS:
			if (!opt_popupPluginInstalled)
				break;
			EnableWindow(GetDlgItem(hdlg, IDC_BGCOLOR), IsDlgButtonChecked(hdlg, IDC_CHECK_POPUPS) && !IsDlgButtonChecked(hdlg, IDC_CHECK_DEFAULTCOLORS));
			EnableWindow(GetDlgItem(hdlg, IDC_TEXTCOLOR), IsDlgButtonChecked(hdlg, IDC_CHECK_POPUPS) && !IsDlgButtonChecked(hdlg, IDC_CHECK_DEFAULTCOLORS));
			EnableWindow(GetDlgItem(hdlg, IDC_CHECK_DEFAULTCOLORS), IsDlgButtonChecked(hdlg, IDC_CHECK_POPUPS));
			EnableWindow(GetDlgItem(hdlg, IDC_CHECK_SHOWPROXYSTATUS), IsDlgButtonChecked(hdlg, IDC_CHECK_POPUPS));
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hdlg, 0);
		break;
	}
	return 0;
}

int OptInit(WPARAM wParam, LPARAM) 
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 95600;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = OptionsProc;
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = LPGENW("ProxySwitch");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wParam, &odp);

	return 0;
}

void LoadSettings(void) 
{
	ptrW wszStr(db_get_wsa(NULL, MODULENAME, "UseProxyIPNets"));
	if (!wszStr)
		opt_useProxy[0] = 0;
	else
		wcsncpy_s(opt_useProxy, wszStr, _TRUNCATE);

	wszStr = db_get_wsa(NULL, MODULENAME, "NoProxyIPNets");
	if (!wszStr)
		opt_noProxy[0] = 0;
	else
		wcsncpy_s(opt_noProxy, wszStr, _TRUNCATE);

	wszStr = db_get_wsa(NULL, MODULENAME, "HideInterfaces");
	if (!wszStr)
		opt_hideIntf[0] = 0;
	else
		wcsncpy_s(opt_hideIntf, wszStr, _TRUNCATE);
	
	opt_miranda = db_get_b(NULL, MODULENAME, "ManageMirandaProxy", TRUE);
	opt_ie = db_get_b(NULL, MODULENAME, "ManageIEProxy", FALSE);
	opt_firefox = db_get_b(NULL, MODULENAME, "ManageFirefoxProxy", FALSE) && Firefox_Installed();
	opt_alwayReconnect = db_get_b(NULL, MODULENAME, "AlwaysReconnect", FALSE);
	opt_popups = db_get_b(NULL, MODULENAME, "PopupEnabled", TRUE);
	opt_defaultColors = db_get_b(NULL, MODULENAME, "PopupDefaultColors", TRUE);
	opt_showProxyState = db_get_b(NULL, MODULENAME, "ShowProxyStatus", TRUE);
	opt_bgColor = db_get_dw(NULL, MODULENAME, "PopupBgColor", GetSysColor(COLOR_BTNFACE));
	opt_txtColor = db_get_dw(NULL, MODULENAME, "PopupTxtColor", GetSysColor(COLOR_WINDOWTEXT));
}

void SaveSettings(void)
{
	db_set_ws(NULL, MODULENAME, "UseProxyIPNets", opt_useProxy);
	db_set_ws(NULL, MODULENAME, "NoProxyIPNets", opt_noProxy);
	db_set_ws(NULL, MODULENAME, "HideInterfaces", opt_hideIntf);
	db_set_b(NULL, MODULENAME, "ManageMirandaProxy", (BYTE)opt_miranda);
	db_set_b(NULL, MODULENAME, "ManageIEProxy", (BYTE)opt_ie);
	db_set_b(NULL, MODULENAME, "ManageFirefoxProxy", (BYTE)opt_firefox);
	db_set_b(NULL, MODULENAME, "AlwaysReconnect", (BYTE)opt_alwayReconnect);
	db_set_b(NULL, MODULENAME, "PopupEnabled", (BYTE)opt_popups);
	db_set_b(NULL, MODULENAME, "PopupDefaultColors", (BYTE)opt_defaultColors);
	db_set_b(NULL, MODULENAME, "ShowProxyStatus", (BYTE)opt_showProxyState);
	db_set_dw(NULL, MODULENAME, "PopupBgColor", (DWORD)opt_bgColor);
	db_set_dw(NULL, MODULENAME, "PopupTxtColor", (DWORD)opt_txtColor);
}
