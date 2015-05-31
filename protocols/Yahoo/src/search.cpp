/*
 * $Id: search.cpp 11632 2010-04-24 12:32:03Z borkra $
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
#include <m_protosvc.h>
#include <m_langpack.h>

#include "resource.h"

////////////////////////////////////////////////////////////////////////////////////////
// SearchBasic - searches the contact by UIN

void __cdecl CYahooProto::search_simplethread(void *snsearch)
{
	TCHAR *id = (TCHAR *) snsearch;

	if (mir_tstrlen(id) < 4) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
		MessageBoxA(NULL, "Please enter a valid ID to search for.", "Search", MB_OK);
		return;
	}

	TCHAR *c = _tcschr(id, '@');
	if (c) *c = 0;

	YAHOO_SEARCH_RESULT psr;
	memset(&psr, 0, sizeof(psr));
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_TCHAR;
	psr.id.t = (TCHAR*)_tcslwr(id);
	psr.protocol = YAHOO_IM_YAHOO;

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);

	//yahoo_search(m_id, YAHOO_SEARCH_YID, m, YAHOO_GENDER_NONE, YAHOO_AGERANGE_NONE, 0, 1);

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

HANDLE __cdecl CYahooProto::SearchBasic( const TCHAR* nick )
{
	LOG(("[YahooBasicSearch] Searching for: %S", nick));
	
	if ( !m_bLoggedIn )
		return 0;

	ForkThread(&CYahooProto::search_simplethread, _tcsdup( nick ));
	return ( HANDLE )1;
}

////////////////////////////////////////////////////////////////////////////////////////
// SearchAdvanced - searches the contact by UIN

void CYahooProto::ext_got_search_result(int found, int start, int total, YList *contacts)
{
	struct yahoo_found_contact *yct=NULL;
	int i=start;
	YList *en=contacts;

	LOG(("got search result: "));
	
	LOG(("Found: %d", found));
	LOG(("Start: %d", start));
	LOG(("Total: %d", total));
		
	YAHOO_SEARCH_RESULT psr;
	memset(&psr, 0, sizeof(psr));
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_TCHAR;
	psr.protocol = YAHOO_IM_YAHOO;
	
	while (en) {
		yct = ( yahoo_found_contact* )en->data;

		if (yct == NULL) {
			LOG(("[%d] Empty record?",i++));
		} else {
			LOG(("[%d] id: '%s', online: %d, age: %d, sex: '%s', location: '%s'", i++, yct->id, yct->online, yct->age, yct->gender, yct->location));
			psr.id.t = mir_utf8decodeT( yct->id );
			
			if (yct->gender[0] != 5)
				psr.firstName.t = mir_utf8decodeT( yct->gender );
			else
				psr.firstName.t = NULL;
			
			TCHAR c[10];
			if (yct->age > 0) {
				_itot(yct->age, c,10);
				psr.lastName.t = ( TCHAR* )c;
			}
			else
				psr.lastName.t = NULL;
			
			if (yct->location[0] != 5)
				psr.email.t = mir_utf8decodeT( yct->location );
			else
				psr.email.t = NULL;
    
			//void yahoo_search(int id, enum yahoo_search_type t, const char *text, enum yahoo_search_gender g, enum yahoo_search_agerange ar, 
			//	int photo, int yahoo_only)

			ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);

			mir_free(psr.id.t);
			mir_free(psr.firstName.t);
			mir_free(psr.email.t);
		}
		en = y_list_next(en);
	}
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

/*
 * This service function creates an advanced search dialog in Find/Add contacts Custom area.
 *
 * Returns: 0 on failure or HWND on success
 */

static INT_PTR CALLBACK YahooSearchAdvancedDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
			
		SendDlgItemMessageA(hwndDlg, IDC_SEARCH_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Yahoo! Messenger");
		SendDlgItemMessageA(hwndDlg, IDC_SEARCH_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Lotus Sametime");
		SendDlgItemMessageA(hwndDlg, IDC_SEARCH_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"LCS");
		SendDlgItemMessageA(hwndDlg, IDC_SEARCH_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Windows Live (MSN)");
		
		// select the first one
		SendDlgItemMessage(hwndDlg, IDC_SEARCH_PROTOCOL, CB_SETCURSEL, 0, 0);
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_SEARCH_ID && HIWORD(wParam)==EN_CHANGE){
			PostMessage(GetParent(hwndDlg),WM_COMMAND, MAKEWPARAM(0,EN_SETFOCUS), (LPARAM)hwndDlg);
			return TRUE;
		}
		if(LOWORD(wParam)==IDC_SEARCH_PROTOCOL && HIWORD(wParam)==CBN_SELCHANGE){
			PostMessage(GetParent(hwndDlg),WM_COMMAND, MAKEWPARAM(0,EN_SETFOCUS), (LPARAM)hwndDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

HWND __cdecl CYahooProto::CreateExtendedSearchUI( HWND parent )
{
	if ( parent && hInstance )
		return CreateDialogParam( hInstance, MAKEINTRESOURCE(IDD_SEARCHUSER), parent, YahooSearchAdvancedDlgProc, (LPARAM)this );

	return 0;
}

void __cdecl CYahooProto::searchadv_thread(void *pHWND)
{
	HWND hwndDlg = (HWND) pHWND;

	TCHAR searchid[128];
	GetDlgItemText(hwndDlg, IDC_SEARCH_ID, searchid, SIZEOF(searchid));

	if (mir_tstrlen(searchid) == 0) {
		ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
		MessageBoxA(NULL, "Please enter a valid ID to search for.", "Search", MB_OK);
		return;
	} 

	YAHOO_SEARCH_RESULT psr;
	memset(&psr, 0, sizeof(psr));
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_TCHAR;
	psr.id.t = _tcslwr(searchid);

	switch (SendDlgItemMessage(hwndDlg, IDC_SEARCH_PROTOCOL, CB_GETCURSEL, 0, 0)) {
		case 0: psr.firstName.t = _T("<Yahoo >");  psr.protocol = YAHOO_IM_YAHOO; break;
		case 1: psr.firstName.t = _T("<Lotus Sametime>"); psr.protocol = YAHOO_IM_SAMETIME; break;
		case 2: psr.firstName.t = _T("<LCS>"); psr.protocol = YAHOO_IM_LCS; break;
		case 3: psr.firstName.t = _T("<Windows Live (MSN)>"); psr.protocol = YAHOO_IM_MSN; break;
	}

	/*
	* Show this in results
	*/
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);

	/*
	* Done searching.
	*/
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

/*
 * This service function does the advanced search
 *
 * Returns: 0 on failure or HWND on success
 */

HWND __cdecl CYahooProto::SearchAdvanced( HWND owner )
{
	LOG(("[YahooAdvancedSearch]"));

	if ( !m_bLoggedIn )
		return 0;

	ForkThread( &CYahooProto::searchadv_thread, owner );
	return ( HWND )1;
}
