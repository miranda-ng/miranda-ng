/*
 * $Id: proto.cpp 9802 2009-05-18 03:03:48Z gena01 $
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

#include <m_options.h>
#include <m_skin.h>
#include <m_userinfo.h>

#include "resource.h"

#include <commctrl.h>

static INT_PTR CALLBACK YahooUserInfoDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg ) {
	case WM_INITDIALOG:
		// lParam is hContact
		TranslateDialogDefault( hwndDlg );

		//SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS));

		
		break;

	case WM_NOTIFY:
		if (( ( LPNMHDR )lParam )->idFrom == 0 ) {
			switch (( ( LPNMHDR )lParam )->code ) {
			case PSN_PARAMCHANGED:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (( PSHNOTIFY* )lParam )->lParam );
				break;
			case PSN_INFOCHANGED:
				{
					CYahooProto* ppro = (CYahooProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

					if (!ppro)
						break;

					char* szProto;
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;

					if (hContact == NULL) {
						szProto = ppro->m_szModuleName;
					} else {
						szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
					}

					if (!szProto)
						break;

					if (hContact) {
						DBVARIANT dbv;
						char z[128];
						
						if (ppro->GetString( hContact,  YAHOO_LOGINID, &dbv ) == 0) {
							strcpy(z, dbv.pszVal);
							DBFreeVariant( &dbv );
						} else {
							strcpy(z, "???");
						}
						
						SetDlgItemTextA(hwndDlg, IDC_SEARCH_ID, z);
						
						if (ppro->GetString( hContact,  "Transport", &dbv ) == 0) {
							strcpy(z, dbv.pszVal);
							DBFreeVariant( &dbv );
						} else {
							strcpy(z, "Yahoo");
						}
						
						SetDlgItemTextA(hwndDlg, IDC_SEARCH_PROTOCOL, z);
						
						if (ppro->GetString( hContact,  "MirVer", &dbv ) == 0) {
							strcpy(z, dbv.pszVal);
							DBFreeVariant( &dbv );
						} else {
							strcpy(z, "???");
						}
						
						SetDlgItemTextA(hwndDlg, IDC_NFO_CLIENT, z);
						
					} else {
					}
				}
				break;
			}	
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnInfoInit - initializes user info option dialogs

INT_PTR __cdecl CYahooProto::OnUserInfoInit( WPARAM wParam, LPARAM lParam )
{
	//if ( !JCallService( MS_PROTO_ISPROTOCOLLOADED, 0, ( LPARAM )m_szModuleName ))
	//	return 0;

	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof( odp );
	odp.hInstance = hInstance;
	odp.dwInitParam = ( LPARAM )this;

	HANDLE hContact = ( HANDLE )lParam;
	if ( hContact )
	{
		char* szProto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
		if ( szProto != NULL && !strcmp( szProto, m_szModuleName ))
		{
			odp.pfnDlgProc = YahooUserInfoDlgProc;
			odp.position = -1900000000;
			odp.pszTemplate = MAKEINTRESOURCEA( IDD_USER_INFO );
			odp.pszTitle = m_szModuleName;
			CallService( MS_USERINFO_ADDPAGE, wParam, ( LPARAM )&odp );

		}
	} 

	return 0;
}

