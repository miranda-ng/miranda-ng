/*
 * $Id: options.cpp 12038 2010-06-21 16:46:22Z gena01 $
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#include "yahoo.h"
#include "resource.h"

#include <m_langpack.h>
#include <m_utils.h>
#include <m_options.h>
#include <m_popup.h>

#include "ignore.h"

/*
 * DlgProcYahooOpts - Account Options Dialog
 */
static INT_PTR CALLBACK DlgProcYahooOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA );

	switch ( msg ) {
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);

		ppro = (CYahooProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam );

		if ( !ppro->getString( YAHOO_LOGINID, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_HANDLE, dbv.pszVal);
			db_free(&dbv);
		}

		if ( !ppro->getString("Nick", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_NICK, dbv.pszVal);
			db_free(&dbv);
		}

		if ( !ppro->getString( YAHOO_PASSWORD, &dbv)) {
			//bit of a security hole here, since it's easy to extract a password from an edit box
			SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal );
			db_free(&dbv);
		}

		//SetButtonCheck(hwndDlg, IDC_DISABLE_UTF8, ppro->getByte("DisableUTF8", 0)); 
		SetButtonCheck(hwndDlg, IDC_USE_YAB, ppro->getByte("UseYAB", 1 )); 
		SetButtonCheck(hwndDlg, IDC_SHOW_AVATARS, ppro->getByte("ShowAvatars", 1 )); 
		SetButtonCheck(hwndDlg, IDC_MAIL_AUTOLOGIN, ppro->getByte("MailAutoLogin", 1 )); 
		SetButtonCheck(hwndDlg, IDC_DISABLEYAHOOMAIL, !ppro->getByte("DisableYahoomail", 0));
		SetButtonCheck(hwndDlg, IDC_SHOW_ERRORS, ppro->getByte("ShowErrors", 1 )); 

		return TRUE;

	case WM_COMMAND: 

		switch ( LOWORD( wParam )) {
		case IDC_NEWYAHOOACCOUNTLINK:
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, 
				ppro->getByte("YahooJapan", 0) ?
				(LPARAM)"http://edit.yahoo.co.jp/config/eval_register" :
			(LPARAM)"http://edit.yahoo.com/config/eval_register");
			return TRUE;

			//case IDC_DISABLE_UTF8: 
		case IDC_USE_YAB:	
		case IDC_SHOW_AVATARS:
		case IDC_MAIL_AUTOLOGIN:
		case IDC_SHOW_ERRORS:
		case IDC_DISABLEYAHOOMAIL:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}    

		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus())
			switch( LOWORD( wParam )) {
			case IDC_HANDLE:			
			case IDC_PASSWORD:			
			case IDC_NICK:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}

		break;

	case WM_NOTIFY: 

		if (((LPNMHDR)lParam)->code == (UINT) PSN_APPLY ) {
			bool reconnectRequired = false;

			char str[128];
			GetDlgItemTextA(hwndDlg, IDC_HANDLE, str, SIZEOF(str));
			if (ppro->getString(YAHOO_LOGINID, &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(mir_strcmp(str, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}
			ppro->setString( YAHOO_LOGINID, str );

			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, SIZEOF(str));
			if (ppro->getString(YAHOO_PASSWORD, &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(mir_strcmp(str, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}

			ppro->setString( YAHOO_PASSWORD, str );
			GetDlgItemTextA(hwndDlg, IDC_NICK, str, SIZEOF(str));


			if (str[0] == '\0') {
				/* Check for empty Nick, if so delete the key in the DB */
				ppro->delSetting("Nick");
			} else {
				/* otherwise save the new Nick */
				ppro->setString("Nick", str);
			}

			ppro->setByte("UseYAB", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_USE_YAB)); 
			ppro->setByte("ShowAvatars", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_AVATARS)); 
			ppro->setByte("MailAutoLogin", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MAIL_AUTOLOGIN)); 
			ppro->setByte("DisableYahoomail", (BYTE)!IsDlgButtonChecked(hwndDlg, IDC_DISABLEYAHOOMAIL));
			ppro->setByte("ShowErrors", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOW_ERRORS)); 

			if (reconnectRequired) {
				ppro->delSetting(YAHOO_PWTOKEN);
				if (ppro->m_bLoggedIn )
					MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the Yahoo network before they take effect"), TranslateT("YAHOO Options"), MB_OK );

			}

			return TRUE;
		}

		break;
	}
	return FALSE;
}

/*
 * DlgProcYahooOpts - Connection Options Dialog
 */
static INT_PTR CALLBACK DlgProcYahooOptsConn(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA );
	
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg );

		ppro = ( CYahooProto* )lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam );

		if ( !ppro->getString( YAHOO_LOGINSERVER, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_LOGINSERVER, dbv.pszVal );
			db_free(&dbv);
		}
		else SetDlgItemTextA(hwndDlg, IDC_LOGINSERVER, 
						ppro->getByte("YahooJapan", 0) 
						? YAHOO_DEFAULT_JAPAN_LOGIN_SERVER
						: YAHOO_DEFAULT_LOGIN_SERVER );

		SetDlgItemInt(hwndDlg, IDC_YAHOOPORT, ppro->getWord(YAHOO_LOGINPORT, YAHOO_DEFAULT_PORT), FALSE);
		
		SetButtonCheck(hwndDlg, IDC_YAHOO_JAPAN, ppro->getByte("YahooJapan", 0));
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
 		case IDC_RESETSERVER:
			SetDlgItemTextA(hwndDlg, IDC_LOGINSERVER, YAHOO_DEFAULT_LOGIN_SERVER );
 			SetDlgItemInt(  hwndDlg, IDC_YAHOOPORT,  YAHOO_DEFAULT_PORT, FALSE );
 			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
 			break;
 		
		case IDC_YAHOO_JAPAN:
			SetDlgItemTextA(hwndDlg, IDC_LOGINSERVER, 
				(IsDlgButtonChecked(hwndDlg,IDC_YAHOO_JAPAN)==BST_CHECKED) ?
					YAHOO_DEFAULT_JAPAN_LOGIN_SERVER :
					YAHOO_DEFAULT_LOGIN_SERVER );
				// fall through and enable apply button

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
  		}    

 		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus())
  			switch( LOWORD( wParam )) {
			case IDC_LOGINSERVER:
  			case IDC_YAHOOPORT:			
  				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
  			}

		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY )
		{
			bool reconnectRequired = false;
			char str[128];
			GetDlgItemTextA(hwndDlg, IDC_LOGINSERVER, str, SIZEOF(str));
			
			DBVARIANT dbv;
			if (ppro->getString(YAHOO_LOGINSERVER, &dbv)) {
				reconnectRequired = true;
			}
			else {
				if(mir_strcmp(str, dbv.pszVal))
					reconnectRequired = true;
				db_free(&dbv);
			}

			ppro->setString(YAHOO_LOGINSERVER, str);

			int port = GetDlgItemInt(hwndDlg, IDC_YAHOOPORT, NULL, FALSE );
			if ( ppro->getWord(YAHOO_LOGINPORT, -1) != port)
				reconnectRequired = true;
			
			ppro->setWord(YAHOO_LOGINPORT, port);

			ppro->setByte("YahooJapan", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_YAHOO_JAPAN ));

			if ( reconnectRequired && ppro->m_bLoggedIn )
				MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the Yahoo network before they take effect"), TranslateT("YAHOO Options"), MB_OK );

			return TRUE;
		}
		break;
	}
	return FALSE;
}

/*
 * DlgProcYahooOpts - Connection Options Dialog
 */
static INT_PTR CALLBACK DlgProcYahooOptsIgnore(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	YList *l;
	CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA );

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg );

		ppro = ( CYahooProto* )lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam );

		if ( ppro->getByte("IgnoreUnknown", 0)) {
			CheckDlgButton(hwndDlg, IDC_OPT_IGN_UNKNOWN, 1);

			EnableWindow( GetDlgItem(hwndDlg, IDC_IGN_ADD), 0);
			EnableWindow( GetDlgItem(hwndDlg, IDC_IGN_REMOVE), 0);
			EnableWindow( GetDlgItem(hwndDlg, IDC_YIGN_EDIT), 0);
			EnableWindow( GetDlgItem(hwndDlg, IDC_YIGN_LIST), 0);
		}
		else CheckDlgButton(hwndDlg, IDC_OPT_IGN_LIST, 1);

		/* show our current ignore list */
		LOG(("[DlgProcYahooOptsIgnore] Grabbing current ignore list..."))
		l = (YList *)ppro->GetIgnoreList();
		while (l != NULL) {
			struct yahoo_buddy *b = (struct yahoo_buddy *) l->data;

			LOG(("[DlgProcYahooOptsIgnore] Buddy: %s", b->id ))
			SendDlgItemMessageA(hwndDlg, IDC_YIGN_LIST, LB_ADDSTRING, 0, (LPARAM)b->id);
			l = l->next;
		}
		LOG(("[DlgProcYahooOptsIgnore] End of Ignore List..."))
		
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDC_OPT_IGN_UNKNOWN:
		case IDC_OPT_IGN_LIST:
			if (( HWND )lParam != GetFocus()) return 0;

			EnableWindow( GetDlgItem(hwndDlg, IDC_IGN_ADD), LOWORD( wParam ) == IDC_OPT_IGN_LIST);
			EnableWindow( GetDlgItem(hwndDlg, IDC_IGN_REMOVE), LOWORD( wParam ) == IDC_OPT_IGN_LIST);
			EnableWindow( GetDlgItem(hwndDlg, IDC_YIGN_EDIT), LOWORD( wParam ) == IDC_OPT_IGN_LIST);
			EnableWindow( GetDlgItem(hwndDlg, IDC_YIGN_LIST), LOWORD( wParam ) == IDC_OPT_IGN_LIST);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_IGN_ADD: 
			if (!ppro->m_bLoggedIn)
				MessageBox(hwndDlg, TranslateT("You need to be connected to Yahoo to add to Ignore List."), TranslateT("Yahoo Ignore"), MB_OK| MB_ICONINFORMATION);
			else {
				char id[128];
				int i = GetDlgItemTextA(hwndDlg, IDC_YIGN_EDIT, id, SIZEOF(id));

				if (i < 3) {
					MessageBox(hwndDlg, TranslateT("Please enter a valid buddy name to ignore."), TranslateT("Yahoo Ignore"), MB_OK| MB_ICONINFORMATION);
					break;
				}

				i = SendDlgItemMessageA(hwndDlg, IDC_YIGN_LIST, LB_FINDSTRINGEXACT,(WPARAM) -1, (LPARAM)id);
				if (i != LB_ERR ) {
					MessageBox(hwndDlg, TranslateT("The buddy is already on your ignore list."), TranslateT("Yahoo Ignore"), MB_OK | MB_ICONINFORMATION);
					break;
				}
				ppro->IgnoreBuddy(id, 0);
				SendDlgItemMessageA(hwndDlg, IDC_YIGN_LIST, LB_ADDSTRING, 0, (LPARAM)id);
				SetDlgItemTextA(hwndDlg, IDC_YIGN_EDIT, "");
			}
			break;

		case IDC_IGN_REMOVE:
			{
				int i;
				char id[128];

				if (!ppro->m_bLoggedIn) {
					MessageBox(hwndDlg, TranslateT("You need to be connected to Yahoo to remove from the ignore list."), TranslateT("Yahoo Ignore"), MB_OK| MB_ICONINFORMATION);
					break;
				}

				i = SendDlgItemMessage(hwndDlg,IDC_YIGN_LIST, LB_GETCURSEL, 0, 0);
				if (i == LB_ERR) {
					MessageBox(hwndDlg, TranslateT("Please select a buddy on the ignore list to remove."), TranslateT("Yahoo Ignore"), MB_OK| MB_ICONINFORMATION);
					break;
				}

				SendDlgItemMessageA(hwndDlg, IDC_YIGN_LIST, LB_GETTEXT, i, (LPARAM)id);

				ppro->IgnoreBuddy(id, 1);
				SendDlgItemMessage(hwndDlg,IDC_YIGN_LIST, LB_DELETESTRING, i, 0);
			}	
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			ppro->setByte("IgnoreUnknown", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_OPT_IGN_UNKNOWN ));
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/*
 * YahooOptInit - initialize/register our Options w/ Miranda.
 */

int __cdecl CYahooProto::OnOptionsInit(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };	
	odp.position    = -790000000;
	odp.hInstance   = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_YAHOO);
	odp.ptszTitle   = m_tszUserName;
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTab     = LPGENT("Account");
	odp.pfnDlgProc  = DlgProcYahooOpts;
	odp.dwInitParam = LPARAM(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab     = LPGENT("Connection");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_YAHOO_CONNECTION);
	odp.pfnDlgProc  = DlgProcYahooOptsConn;
	Options_AddPage(wParam, &odp);
	
	odp.ptszTab     = LPGENT("Ignore List");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_YAHOO_IGNORE);
	odp.pfnDlgProc  = DlgProcYahooOptsIgnore;
	Options_AddPage(wParam, &odp);
	return 0;
}
