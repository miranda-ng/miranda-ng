#include "Mra.h"



INT_PTR CALLBACK	DlgProcOptsAccount		(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	DlgProcOptsConnections	(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);



int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp={0};

	odp.cbSize=sizeof(odp);
	odp.hInstance=masMraSettings.hInstance;
	odp.pszTitle=PROTOCOL_NAMEA;
	odp.pszGroup="Network";
	odp.flags=ODPF_BOLDGROUPS;

	odp.pszTab="Account";
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_ACCOUNT);
	odp.pfnDlgProc=DlgProcOptsAccount;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.pszTab="Connections";
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_CONNECTIONS);
	odp.pfnDlgProc=DlgProcOptsConnections;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.pszTab="Anti spam";
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_ANTISPAM);
	odp.pfnDlgProc=MraAntiSpamDlgProcOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.pszTab="Files";
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_FILES);
	odp.pfnDlgProc=MraFilesQueueDlgProcOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.pszTab="Avatars";
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_AVATRS);
	odp.pfnDlgProc=MraAvatarsQueueDlgProcOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	MraPopupOptInit(wParam,lParam);
return(0);
}



INT_PTR CALLBACK DlgProcOptsAccount(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			WCHAR szBuff[MAX_PATH];
			//SIZE_T dwBuffSize;

			TranslateDialogDefault(hWndDlg);

			if (DB_Mra_GetStaticStringW(NULL,"e-mail",szBuff,SIZEOF(szBuff),NULL))
			{
				SET_DLG_ITEM_TEXTW(hWndDlg,IDC_LOGIN,szBuff);
			}
			
			SET_DLG_ITEM_TEXTW(hWndDlg,IDC_PASSWORD,(LPWSTR)L"");
			//if (GetPassDB((LPSTR)szBuff,SIZEOF(szBuff),&dwBuffSize))
			//{//bit of a security hole here, since it's easy to extract a password from an edit box
			//	SET_DLG_ITEM_TEXTA(hWndDlg,IDC_PASSWORD,(LPSTR)szBuff);
			//	SecureZeroMemory(szBuff,sizeof(szBuff));
			//}
		}
		return(TRUE);
	case WM_COMMAND:
		switch (LOWORD(wParam)){
		case IDC_NEW_ACCOUNT_LINK:
			CallService(MS_UTILS_OPENURL,TRUE,(LPARAM)MRA_REGISTER_URL);
			return(TRUE);
		case IDC_LOOKUPLINK:
			CallService(MS_UTILS_OPENURL,TRUE,(LPARAM)MRA_FORGOT_PASSWORD_URL);
			return(TRUE);
		}
		if ((LOWORD(wParam)==IDC_LOGIN || LOWORD(wParam)==IDC_PASSWORD) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) ) return 0;
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				WCHAR szBuff[MAX_EMAIL_LEN];

				GET_DLG_ITEM_TEXT(hWndDlg,IDC_LOGIN,szBuff,SIZEOF(szBuff));
				DB_Mra_SetStringW(NULL,"e-mail",szBuff);

				if (GET_DLG_ITEM_TEXTA(hWndDlg,IDC_PASSWORD,(LPSTR)szBuff,SIZEOF(szBuff))) 
				{
					SetPassDB((LPSTR)szBuff,lstrlenA((LPSTR)szBuff));
					SecureZeroMemory(szBuff,sizeof(szBuff));
				}
			}
			return(TRUE);
		}
		break;
	}
return(FALSE);
}


INT_PTR CALLBACK DlgProcOptsConnections(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			WCHAR szBuff[MAX_PATH];

			TranslateDialogDefault(hWndDlg);

			if (DB_Mra_GetStaticStringW(NULL,"Server",szBuff,SIZEOF(szBuff),NULL))
			{
				SET_DLG_ITEM_TEXT(hWndDlg,IDC_SERVER,szBuff);
			}else{
				SET_DLG_ITEM_TEXTA(hWndDlg,IDC_SERVER,MRA_DEFAULT_SERVER);
			}
			SetDlgItemInt(hWndDlg,IDC_SERVERPORT,DB_Mra_GetWord(NULL,"ServerPort",MRA_DEFAULT_SERVER_PORT),FALSE);
			// if set SSL proxy, setting will ignored
			
			//CHECK_DLG_BUTTON(hWndDlg, IDC_KEEPALIVE,DB_Mra_GetByte(NULL,"keepalive",0));
			CHECK_DLG_BUTTON(hWndDlg,IDC_AUTO_ADD_CONTACTS_TO_SERVER,DB_Mra_GetByte(NULL,"AutoAddContactsToServer",MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER));
			CHECK_DLG_BUTTON(hWndDlg,IDC_AUTO_AUTH_REQ_ON_LOGON,DB_Mra_GetByte(NULL,"AutoAuthRequestOnLogon",MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON));
			CHECK_DLG_BUTTON(hWndDlg,IDC_AUTO_AUTH_GRAND_IN_CLIST,DB_Mra_GetByte(NULL,"AutoAuthGrandUsersInCList",MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST));
			CHECK_DLG_BUTTON(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS,DB_Mra_GetByte(NULL,"AutoAuthGrandNewUsers",MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS));
			CHECK_DLG_BUTTON(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK,DB_Mra_GetByte(NULL,"AutoAuthGrandNewUsersDisableSPAMCheck",MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK));
			EnableWindow(GetDlgItem(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS));

			CHECK_DLG_BUTTON(hWndDlg,IDC_SLOWSEND,DB_Mra_GetByte(NULL,"SlowSend",MRA_DEFAULT_SLOW_SEND));
			CHECK_DLG_BUTTON(hWndDlg,IDC_INCREMENTAL_NEW_MAIL_NOTIFY,DB_Mra_GetByte(NULL,"IncrementalNewMailNotify",MRA_DEFAULT_INC_NEW_MAIL_NOTIFY));
			CHECK_DLG_BUTTON(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY,DB_Mra_GetByte(NULL,"TrayIconNewMailNotify",MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY));
			CHECK_DLG_BUTTON(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX,DB_Mra_GetByte(NULL,"TrayIconNewMailClkToInbox",MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX));
			EnableWindow(GetDlgItem(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX),DB_Mra_GetByte(NULL,"TrayIconNewMailNotify",MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY));
			
			CHECK_DLG_BUTTON(hWndDlg,IDC_HIDE_MENU_ITEMS_FOR_NON_MRA,DB_Mra_GetByte(NULL,"HideMenuItemsForNonMRAContacts",MRA_DEFAULT_HIDE_MENU_ITEMS_FOR_NON_MRA));
		
			CHECK_DLG_BUTTON(hWndDlg,IDC_RTF_RECEIVE_ENABLE,DB_Mra_GetByte(NULL,"RTFReceiveEnable",MRA_DEFAULT_RTF_RECEIVE_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg,IDC_RTF_RECEIVE_ENABLE),(BOOL)(masMraSettings.lpfnUncompress!=NULL));
			
			CHECK_DLG_BUTTON(hWndDlg,IDC_RTF_SEND_ENABLE,DB_Mra_GetByte(NULL,"RTFSendEnable",MRA_DEFAULT_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg,IDC_RTF_SEND_ENABLE),(BOOL)(masMraSettings.lpfnCompress2!=NULL));
			EnableWindow(GetDlgItem(hWndDlg,IDC_RTF_SEND_SMART),(DB_Mra_GetByte(NULL,"RTFSendEnable",MRA_DEFAULT_RTF_SEND_ENABLE) && masMraSettings.lpfnCompress2));
			EnableWindow(GetDlgItem(hWndDlg,IDC_BUTTON_FONT),(DB_Mra_GetByte(NULL,"RTFSendEnable",MRA_DEFAULT_RTF_SEND_ENABLE) && masMraSettings.lpfnCompress2));
			EnableWindow(GetDlgItem(hWndDlg,IDC_RTF_BGCOLOUR),(DB_Mra_GetByte(NULL,"RTFSendEnable",MRA_DEFAULT_RTF_SEND_ENABLE) && masMraSettings.lpfnCompress2));
			SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_RTF_BGCOLOUR,CPM_SETCOLOUR,0,DB_Mra_GetDword(NULL,"RTFBackgroundColour",MRA_DEFAULT_RTF_BACKGROUND_COLOUR));
		}
		return(TRUE);
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDC_BUTTON_DEFAULT:
			SET_DLG_ITEM_TEXTA(hWndDlg,IDC_SERVER,MRA_DEFAULT_SERVER);
			SetDlgItemInt(hWndDlg,IDC_SERVERPORT,MRA_DEFAULT_SERVER_PORT,FALSE);
			break;
		case IDC_AUTO_AUTH_GRAND_NEW_USERS:
			EnableWindow(GetDlgItem(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS));
			break;
		case IDC_TRAYICON_NEW_MAIL_NOTIFY:
			EnableWindow(GetDlgItem(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY));
			break;
		case IDC_RTF_SEND_ENABLE:
			EnableWindow(GetDlgItem(hWndDlg,IDC_RTF_SEND_SMART),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg,IDC_BUTTON_FONT),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_RTF_SEND_ENABLE));
			EnableWindow(GetDlgItem(hWndDlg,IDC_RTF_BGCOLOUR),IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_RTF_SEND_ENABLE));
			break;
		case IDC_BUTTON_FONT:
			{
				LOGFONT lf={0};
				CHOOSEFONT cf={0};

				cf.lStructSize=sizeof(cf);
				cf.lpLogFont=&lf;
				cf.rgbColors=DB_Mra_GetDword(NULL,"RTFFontColour",MRA_DEFAULT_RTF_FONT_COLOUR);
				cf.Flags=(CF_SCREENFONTS|CF_EFFECTS|CF_FORCEFONTEXIST|CF_INITTOLOGFONTSTRUCT);
				if (DB_Mra_GetContactSettingBlob(NULL,"RTFFont",&lf,sizeof(LOGFONT),NULL)==FALSE)
				{
					HDC hDC=GetDC(NULL);// kegl
					lf.lfCharSet=MRA_DEFAULT_RTF_FONT_CHARSET;
					lf.lfHeight=-MulDiv(MRA_DEFAULT_RTF_FONT_SIZE,GetDeviceCaps(hDC,LOGPIXELSY),72);
					lstrcpynW(lf.lfFaceName,MRA_DEFAULT_RTF_FONT_NAME,LF_FACESIZE);
					ReleaseDC(NULL,hDC);
				}

				if (ChooseFont(&cf))
				{
					/*HDC hDC=GetDC(NULL);
					DWORD dwFontSize;
					dwFontSize=-MulDiv(lf.lfHeight,72,GetDeviceCaps(hDC,LOGPIXELSY));
					dwFontSize+=((dwFontSize+4)/8);//MulDiv(dwFontSize,GetDeviceCaps(hDC,PHYSICALWIDTH),72);
					ReleaseDC(NULL,hDC);*/

					DB_Mra_WriteContactSettingBlob(NULL,"RTFFont",&lf,sizeof(LOGFONT));
					DB_Mra_SetDword(NULL,"RTFFontColour",cf.rgbColors);
				}
			}
			break;
		}

		if ((LOWORD(wParam)==IDC_SERVER || LOWORD(wParam)==IDC_SERVERPORT) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()) ) return(FALSE);
		SendMessage(GetParent(hWndDlg),PSM_CHANGED,0,0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code){
		case PSN_APPLY:
			{
				WCHAR szBuff[MAX_PATH];

				GET_DLG_ITEM_TEXT(hWndDlg,IDC_SERVER,szBuff,SIZEOF(szBuff));
				DB_Mra_SetStringW(NULL,"Server",szBuff);
				DB_Mra_SetWord(NULL,"ServerPort",(WORD)GetDlgItemInt(hWndDlg,IDC_SERVERPORT,NULL,FALSE));
				DB_Mra_SetByte(NULL,"AutoAddContactsToServer",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_ADD_CONTACTS_TO_SERVER));
				DB_Mra_SetByte(NULL,"AutoAuthRequestOnLogon",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_AUTH_REQ_ON_LOGON));
				DB_Mra_SetByte(NULL,"AutoAuthGrandUsersInCList",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_AUTH_GRAND_IN_CLIST));
				DB_Mra_SetByte(NULL,"AutoAuthGrandNewUsers",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS));
				DB_Mra_SetByte(NULL,"AutoAuthGrandNewUsersDisableSPAMCheck",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK));

				DB_Mra_SetByte(NULL,"SlowSend",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_SLOWSEND));
				DB_Mra_SetByte(NULL,"IncrementalNewMailNotify",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_INCREMENTAL_NEW_MAIL_NOTIFY));
				DB_Mra_SetByte(NULL,"TrayIconNewMailNotify",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY));
				DB_Mra_SetByte(NULL,"TrayIconNewMailClkToInbox",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_TRAYICON_NEW_MAIL_NOTIFY_CLICK_TO_INBOX));

				DB_Mra_SetByte(NULL,"HideMenuItemsForNonMRAContacts",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_HIDE_MENU_ITEMS_FOR_NON_MRA));

				DB_Mra_SetByte(NULL,"RTFReceiveEnable",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_RTF_RECEIVE_ENABLE));
				DB_Mra_SetByte(NULL,"RTFSendEnable",IS_DLG_BUTTON_CHECKED(hWndDlg,IDC_RTF_SEND_ENABLE));
				DBWriteContactSettingDword(NULL,PROTOCOL_NAMEA,"RTFBackgroundColour",SEND_DLG_ITEM_MESSAGE(hWndDlg,IDC_RTF_BGCOLOUR,CPM_GETCOLOUR,0,0));
			}
			return(TRUE);
		}
		break;
	}
return(FALSE);
}

