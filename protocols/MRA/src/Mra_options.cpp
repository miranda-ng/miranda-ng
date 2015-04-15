#include "Mra.h"

INT_PTR CALLBACK DlgProcOptsAccount(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);
	CMStringW szBuff;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		if (ppro->mraGetStringW(NULL, "e-mail", szBuff))
			SetDlgItemText(hWndDlg, IDC_LOGIN, szBuff.c_str());

		SetDlgItemText(hWndDlg, IDC_PASSWORD, _T(""));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NEW_ACCOUNT_LINK:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)MRA_REGISTER_URL);
			return TRUE;

		case IDC_LOOKUPLINK:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)MRA_FORGOT_PASSWORD_URL);
			return TRUE;
		}
		if ((LOWORD(wParam) == IDC_LOGIN || LOWORD(wParam) == IDC_PASSWORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			TCHAR szBuff[MAX_EMAIL_LEN];
			GetDlgItemText(hWndDlg, IDC_LOGIN, szBuff, SIZEOF(szBuff));
			ppro->setTString(NULL, "e-mail", szBuff);

			if (GetDlgItemText(hWndDlg, IDC_PASSWORD, szBuff, SIZEOF(szBuff))) {
				ppro->setTString("Password", szBuff);
				SecureZeroMemory(szBuff, sizeof(szBuff));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcAccount(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);
	CMStringW szBuff;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		if (ppro->mraGetStringW(NULL, "e-mail", szBuff))
			SetDlgItemText(hWndDlg, IDC_LOGIN, szBuff.c_str());

		SetDlgItemText(hWndDlg, IDC_PASSWORD, _T(""));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NEW_ACCOUNT_LINK:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)MRA_REGISTER_URL);
			return TRUE;
		}
		if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
			switch (LOWORD(wParam)) {
			case IDC_LOGIN:
			case IDC_PASSWORD:
				SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			TCHAR szBuff[MAX_EMAIL_LEN];
			GetDlgItemText(hWndDlg, IDC_LOGIN, szBuff, SIZEOF(szBuff));
			ppro->mraSetStringW(NULL, "e-mail", szBuff);

			if (GetDlgItemText(hWndDlg, IDC_PASSWORD, szBuff, SIZEOF(szBuff))) {
				ppro->setTString("Password", szBuff);
				SecureZeroMemory(szBuff, sizeof(szBuff));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcOptsConnections(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);
	CMStringW szBuff;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		if (ppro->mraGetStringW(NULL, "Server", szBuff))
			SetDlgItemText(hWndDlg, IDC_SERVER, szBuff.c_str());
		else
			SetDlgItemTextA(hWndDlg, IDC_SERVER, MRA_DEFAULT_SERVER);

		SetDlgItemInt(hWndDlg, IDC_SERVERPORT, ppro->getWord("ServerPort", MRA_DEFAULT_SERVER_PORT), FALSE);
		// if set SSL proxy, setting will ignored

		//CheckDlgButton(hWndDlg, IDC_KEEPALIVE, getByte("keepalive", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_AUTO_ADD_CONTACTS_TO_SERVER, ppro->getByte("AutoAddContactsToServer", MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER)) ? BST_CHECKED : BST_UNCHECKED;
		CheckDlgButton(hWndDlg, IDC_AUTO_AUTH_REQ_ON_LOGON, ppro->getByte("AutoAuthRequestOnLogon", MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_AUTO_AUTH_GRAND_IN_CLIST, ppro->getByte("AutoAuthGrandUsersInCList", MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS, ppro->getByte("AutoAuthGrandNewUsers", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_AUTO_AUTH_ON_WEB_SVCS, ppro->getByte("AutoAuthOnWebServices", MRA_DEFAULT_AUTO_AUTH_ON_WEB_SVCS) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hWndDlg, IDC_SLOWSEND, ppro->getByte("SlowSend", MRA_DEFAULT_SLOW_SEND) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_INCREMENTAL_NEW_MAIL_NOTIFY, ppro->getByte("IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY, ppro->getByte("TrayIconNewMailNotify", MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX, ppro->getByte("TrayIconNewMailClkToInbox", MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX), ppro->getByte("TrayIconNewMailNotify", MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY));

		CheckDlgButton(hWndDlg, IDC_RTF_RECEIVE_ENABLE, ppro->getByte("RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hWndDlg, IDC_RTF_SEND_ENABLE, ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_SEND_SMART), ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		EnableWindow(GetDlgItem(hWndDlg, IDC_BUTTON_FONT), ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_BGCOLOUR), ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		SendDlgItemMessage(hWndDlg, IDC_RTF_BGCOLOUR, CPM_SETCOLOUR, 0, ppro->getDword("RTFBackgroundColour", MRA_DEFAULT_RTF_BACKGROUND_COLOUR));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_DEFAULT:
			SetDlgItemTextA(hWndDlg, IDC_SERVER, MRA_DEFAULT_SERVER);
			SetDlgItemInt(hWndDlg, IDC_SERVERPORT, MRA_DEFAULT_SERVER_PORT, FALSE);
			break;
		case IDC_TRAYICON_NEW_MAIL_NOTIFY:
			EnableWindow(GetDlgItem(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX), IsDlgButtonChecked(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY));
			break;
		case IDC_RTF_SEND_ENABLE:
			EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_SEND_SMART), IsDlgButtonChecked(hWndDlg, IDC_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg, IDC_BUTTON_FONT), IsDlgButtonChecked(hWndDlg, IDC_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_BGCOLOUR), IsDlgButtonChecked(hWndDlg, IDC_RTF_SEND_ENABLE));
			break;
		case IDC_BUTTON_FONT:
			LOGFONT lf = { 0 };
			CHOOSEFONT cf = { 0 };

			cf.lStructSize = sizeof(cf);
			cf.lpLogFont = &lf;
			cf.rgbColors = ppro->getDword("RTFFontColour", MRA_DEFAULT_RTF_FONT_COLOUR);
			cf.Flags = (CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT);
			if (ppro->mraGetContactSettingBlob(NULL, "RTFFont", &lf, sizeof(LOGFONT), NULL) == FALSE) {
				HDC hDC = GetDC(NULL);// kegl
				lf.lfCharSet = MRA_DEFAULT_RTF_FONT_CHARSET;
				lf.lfHeight = -MulDiv(MRA_DEFAULT_RTF_FONT_SIZE, GetDeviceCaps(hDC, LOGPIXELSY), 72);
				mir_tstrncpy(lf.lfFaceName, MRA_DEFAULT_RTF_FONT_NAME, LF_FACESIZE);
				ReleaseDC(NULL, hDC);
			}

			if (ChooseFont(&cf)) {
				ppro->mraWriteContactSettingBlob(NULL, "RTFFont", &lf, sizeof(LOGFONT));
				ppro->setDword("RTFFontColour", cf.rgbColors);
			}
			break;
		}

		if ((LOWORD(wParam) == IDC_SERVER || LOWORD(wParam) == IDC_SERVERPORT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return FALSE;
		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			TCHAR szBuff[MAX_PATH];
			GetDlgItemText(hWndDlg, IDC_SERVER, szBuff, SIZEOF(szBuff));
			ppro->mraSetStringW(NULL, "Server", szBuff);
			ppro->setWord("ServerPort", (WORD)GetDlgItemInt(hWndDlg, IDC_SERVERPORT, NULL, FALSE));
			ppro->setByte("AutoAddContactsToServer", IsDlgButtonChecked(hWndDlg, IDC_AUTO_ADD_CONTACTS_TO_SERVER));
			ppro->setByte("AutoAuthRequestOnLogon", IsDlgButtonChecked(hWndDlg, IDC_AUTO_AUTH_REQ_ON_LOGON));
			ppro->setByte("AutoAuthGrandUsersInCList", IsDlgButtonChecked(hWndDlg, IDC_AUTO_AUTH_GRAND_IN_CLIST));
			ppro->setByte("AutoAuthGrandNewUsers", IsDlgButtonChecked(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS));
			ppro->setByte("AutoAuthOnWebServices", IsDlgButtonChecked(hWndDlg, IDC_AUTO_AUTH_ON_WEB_SVCS));

			ppro->setByte("SlowSend", IsDlgButtonChecked(hWndDlg, IDC_SLOWSEND));
			ppro->setByte("IncrementalNewMailNotify", IsDlgButtonChecked(hWndDlg, IDC_INCREMENTAL_NEW_MAIL_NOTIFY));
			ppro->setByte("TrayIconNewMailNotify", IsDlgButtonChecked(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY));
			ppro->setByte("TrayIconNewMailClkToInbox", IsDlgButtonChecked(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX));

			ppro->setByte("RTFReceiveEnable", IsDlgButtonChecked(hWndDlg, IDC_RTF_RECEIVE_ENABLE));
			ppro->setByte("RTFSendEnable", IsDlgButtonChecked(hWndDlg, IDC_RTF_SEND_ENABLE));
			ppro->setDword("RTFBackgroundColour", SendDlgItemMessage(hWndDlg, IDC_RTF_BGCOLOUR, CPM_GETCOLOUR, 0, 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int CMraProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.dwInitParam = (LPARAM)this;
	odp.hInstance = g_hInstance;
	odp.ptszTitle = m_tszUserName;
	odp.ptszGroup = LPGENT("Network");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ACCOUNT);
	odp.pfnDlgProc = DlgProcOptsAccount;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Connections");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONNECTIONS);
	odp.pfnDlgProc = DlgProcOptsConnections;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Files");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FILES);
	odp.pfnDlgProc = MraFilesQueueDlgProcOpts;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Avatars");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AVATRS);
	odp.pfnDlgProc = MraAvatarsQueueDlgProcOpts;
	Options_AddPage(wParam, &odp);

	OnPopupOptInit(wParam, lParam);
	return 0;
}
