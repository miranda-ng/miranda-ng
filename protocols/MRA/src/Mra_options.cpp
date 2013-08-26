#include "Mra.h"

INT_PTR CALLBACK DlgProcOptsAccount(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);
	WCHAR szBuff[MAX_EMAIL_LEN];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		if ( ppro->mraGetStaticStringW(NULL, "e-mail", szBuff, SIZEOF(szBuff), NULL))
			SET_DLG_ITEM_TEXTW(hWndDlg, IDC_LOGIN, szBuff);

		SET_DLG_ITEM_TEXTW(hWndDlg, IDC_PASSWORD, (LPWSTR)L"");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NEW_ACCOUNT_LINK:
			CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)MRA_REGISTER_URL);
			return TRUE;

		case IDC_LOOKUPLINK:
			CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)MRA_FORGOT_PASSWORD_URL);
			return TRUE;
		}
		if ((LOWORD(wParam) == IDC_LOGIN || LOWORD(wParam) == IDC_PASSWORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			GET_DLG_ITEM_TEXT(hWndDlg, IDC_LOGIN, szBuff, SIZEOF(szBuff));
			ppro->mraSetStringW(NULL, "e-mail", szBuff);

			if (GET_DLG_ITEM_TEXTA(hWndDlg, IDC_PASSWORD, (LPSTR)szBuff, SIZEOF(szBuff))) {
				ppro->SetPassDB((LPSTR)szBuff, lstrlenA((LPSTR)szBuff));
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
	WCHAR szBuff[MAX_EMAIL_LEN];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		if ( ppro->mraGetStaticStringW(NULL, "e-mail", szBuff, SIZEOF(szBuff), NULL))
			SET_DLG_ITEM_TEXTW(hWndDlg, IDC_LOGIN, szBuff);

		SET_DLG_ITEM_TEXTW(hWndDlg, IDC_PASSWORD, (LPWSTR)L"");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_NEW_ACCOUNT_LINK:
			CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)MRA_REGISTER_URL);
			return TRUE;
		}
		if ( HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
		{
			switch(LOWORD(wParam))
			{
			case IDC_LOGIN:
			case IDC_PASSWORD:
				SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			GET_DLG_ITEM_TEXT(hWndDlg, IDC_LOGIN, szBuff, SIZEOF(szBuff));
			ppro->mraSetStringW(NULL, "e-mail", szBuff);

			if (GET_DLG_ITEM_TEXTA(hWndDlg, IDC_PASSWORD, (LPSTR)szBuff, SIZEOF(szBuff))) {
				ppro->SetPassDB((LPSTR)szBuff, lstrlenA((LPSTR)szBuff));
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
	WCHAR szBuff[MAX_PATH];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;

		if (ppro->mraGetStaticStringW(NULL, "Server", szBuff, SIZEOF(szBuff), NULL))
			SET_DLG_ITEM_TEXT(hWndDlg, IDC_SERVER, szBuff);
		else
			SET_DLG_ITEM_TEXTA(hWndDlg, IDC_SERVER, MRA_DEFAULT_SERVER);

		SetDlgItemInt(hWndDlg, IDC_SERVERPORT, ppro->getWord("ServerPort", MRA_DEFAULT_SERVER_PORT), FALSE);
		// if set SSL proxy, setting will ignored

		//CHECK_DLG_BUTTON(hWndDlg, IDC_KEEPALIVE, getByte("keepalive", 0));
		CHECK_DLG_BUTTON(hWndDlg, IDC_AUTO_ADD_CONTACTS_TO_SERVER, ppro->getByte("AutoAddContactsToServer", MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER));
		CHECK_DLG_BUTTON(hWndDlg, IDC_AUTO_AUTH_REQ_ON_LOGON, ppro->getByte("AutoAuthRequestOnLogon", MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON));
		CHECK_DLG_BUTTON(hWndDlg, IDC_AUTO_AUTH_GRAND_IN_CLIST, ppro->getByte("AutoAuthGrandUsersInCList", MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST));
		CHECK_DLG_BUTTON(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS, ppro->getByte("AutoAuthGrandNewUsers", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS));
		CHECK_DLG_BUTTON(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK, ppro->getByte("AutoAuthGrandNewUsersDisableSPAMCheck", MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK));
		EnableWindow(GetDlgItem(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS));

		CHECK_DLG_BUTTON(hWndDlg, IDC_SLOWSEND, ppro->getByte("SlowSend", MRA_DEFAULT_SLOW_SEND));
		CHECK_DLG_BUTTON(hWndDlg, IDC_INCREMENTAL_NEW_MAIL_NOTIFY, ppro->getByte("IncrementalNewMailNotify", MRA_DEFAULT_INC_NEW_MAIL_NOTIFY));
		CHECK_DLG_BUTTON(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY, ppro->getByte("TrayIconNewMailNotify", MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY));
		CHECK_DLG_BUTTON(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX, ppro->getByte("TrayIconNewMailClkToInbox", MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX));
		EnableWindow(GetDlgItem(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX), ppro->getByte("TrayIconNewMailNotify", MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY));

		CHECK_DLG_BUTTON(hWndDlg, IDC_RTF_RECEIVE_ENABLE, ppro->getByte("RTFReceiveEnable", MRA_DEFAULT_RTF_RECEIVE_ENABLE));

		CHECK_DLG_BUTTON(hWndDlg, IDC_RTF_SEND_ENABLE, ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_SEND_SMART), ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		EnableWindow(GetDlgItem(hWndDlg, IDC_BUTTON_FONT), ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_BGCOLOUR), ppro->getByte("RTFSendEnable", MRA_DEFAULT_RTF_SEND_ENABLE));
		SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_RTF_BGCOLOUR, CPM_SETCOLOUR, 0, ppro->getDword("RTFBackgroundColour", MRA_DEFAULT_RTF_BACKGROUND_COLOUR));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_DEFAULT:
			SET_DLG_ITEM_TEXTA(hWndDlg, IDC_SERVER, MRA_DEFAULT_SERVER);
			SetDlgItemInt(hWndDlg, IDC_SERVERPORT, MRA_DEFAULT_SERVER_PORT, FALSE);
			break;
		case IDC_AUTO_AUTH_GRAND_NEW_USERS:
			EnableWindow(GetDlgItem(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS));
			break;
		case IDC_TRAYICON_NEW_MAIL_NOTIFY:
			EnableWindow(GetDlgItem(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY));
			break;
		case IDC_RTF_SEND_ENABLE:
			EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_SEND_SMART), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg, IDC_BUTTON_FONT), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg, IDC_RTF_BGCOLOUR), IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_RTF_SEND_ENABLE));
			break;
		case IDC_BUTTON_FONT:
			{
				LOGFONT lf = {0};
				CHOOSEFONT cf = {0};

				cf.lStructSize = sizeof(cf);
				cf.lpLogFont = &lf;
				cf.rgbColors = ppro->getDword("RTFFontColour", MRA_DEFAULT_RTF_FONT_COLOUR);
				cf.Flags = (CF_SCREENFONTS|CF_EFFECTS|CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT);
				if (ppro->mraGetContactSettingBlob(NULL, "RTFFont", &lf, sizeof(LOGFONT), NULL) == FALSE) {
					HDC hDC = GetDC(NULL);// kegl
					lf.lfCharSet = MRA_DEFAULT_RTF_FONT_CHARSET;
					lf.lfHeight = -MulDiv(MRA_DEFAULT_RTF_FONT_SIZE, GetDeviceCaps(hDC, LOGPIXELSY), 72);
					lstrcpynW(lf.lfFaceName, MRA_DEFAULT_RTF_FONT_NAME, LF_FACESIZE);
					ReleaseDC(NULL, hDC);
				}

				if (ChooseFont(&cf)) {
					ppro->mraWriteContactSettingBlob(NULL, "RTFFont", &lf, sizeof(LOGFONT));
					ppro->setDword("RTFFontColour", cf.rgbColors);
				}
			}
			break;
		}

		if ((LOWORD(wParam) == IDC_SERVER || LOWORD(wParam) == IDC_SERVERPORT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return FALSE;
		SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			GET_DLG_ITEM_TEXT(hWndDlg, IDC_SERVER, szBuff, SIZEOF(szBuff));
			ppro->mraSetStringW(NULL, "Server", szBuff);
			ppro->setWord("ServerPort", (WORD)GetDlgItemInt(hWndDlg, IDC_SERVERPORT, NULL, FALSE));
			ppro->setByte("AutoAddContactsToServer", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_ADD_CONTACTS_TO_SERVER));
			ppro->setByte("AutoAuthRequestOnLogon", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_AUTH_REQ_ON_LOGON));
			ppro->setByte("AutoAuthGrandUsersInCList", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_AUTH_GRAND_IN_CLIST));
			ppro->setByte("AutoAuthGrandNewUsers", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS));
			ppro->setByte("AutoAuthGrandNewUsersDisableSPAMCheck", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK));

			ppro->setByte("SlowSend", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_SLOWSEND));
			ppro->setByte("IncrementalNewMailNotify", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_INCREMENTAL_NEW_MAIL_NOTIFY));
			ppro->setByte("TrayIconNewMailNotify", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY));
			ppro->setByte("TrayIconNewMailClkToInbox", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX));

			ppro->setByte("RTFReceiveEnable", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_RTF_RECEIVE_ENABLE));
			ppro->setByte("RTFSendEnable", IS_DLG_BUTTON_CHECKED(hWndDlg, IDC_RTF_SEND_ENABLE));
			ppro->setDword("RTFBackgroundColour", SEND_DLG_ITEM_MESSAGE(hWndDlg, IDC_RTF_BGCOLOUR, CPM_GETCOLOUR, 0, 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int CMraProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.dwInitParam = (LPARAM)this;
	odp.hInstance = masMraSettings.hInstance;
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

	odp.ptszTab = LPGENT("Anti-spam");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ANTISPAM);
	odp.pfnDlgProc = MraAntiSpamDlgProcOpts;
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
