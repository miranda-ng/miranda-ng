/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"

#define SUMMARY     0
#define DETAIL      1
#define DM_FINDNEXT  (WM_USER+10)
#define DM_HREBUILD  (WM_USER+11)

static INT_PTR CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcHistoryFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static HANDLE hWindowList=0;

static INT_PTR UserHistoryCommand(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find( hWindowList,( HANDLE )wParam );
	if ( hwnd ) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		return 0;
	}
	CreateDialogParam(hMirandaInst,MAKEINTRESOURCE(IDD_HISTORY),NULL,DlgProcHistory,wParam);
	return 0;
}

static int HistoryContactDelete(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hWindowList,(HANDLE)wParam);
	if ( hwnd != NULL )
		DestroyWindow(hwnd);
	return 0;
}

int PreShutdownHistoryModule(WPARAM, LPARAM)
{
	if (hWindowList)
		WindowList_BroadcastAsync(hWindowList,WM_DESTROY,0,0);
	return 0;
}

int LoadHistoryModule(void)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = 1000090000;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle( SKINICON_OTHER_HISTORY );
	mi.pszName = LPGEN("View &History");
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY,UserHistoryCommand);
	hWindowList=(HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	HookEvent(ME_DB_CONTACT_DELETED,HistoryContactDelete);
	HookEvent(ME_SYSTEM_PRESHUTDOWN,PreShutdownHistoryModule);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Fills the events list

static void GetMessageDescription( DBEVENTINFO *dbei, TCHAR* buf, int cbBuf )
{
	TCHAR* msg = DbGetEventTextT( dbei, CP_ACP );
    _tcsncpy( buf, msg ? msg : TranslateT("Invalid Message"), cbBuf );
    buf[ cbBuf-1 ] = 0;
	mir_free( msg );
}

static void GetUrlDescription( DBEVENTINFO *dbei, TCHAR* buf, int cbBuf )
{
	int len = dbei->cbBlob;
	if ( len >= cbBuf )
		len = cbBuf-1;

	#if !defined( _UNICODE )
		memcpy( buf, dbei->pBlob, len );
	#else
		MultiByteToWideChar( CP_ACP, 0, ( LPCSTR )dbei->pBlob, len, buf, cbBuf );
	#endif
	buf[ len ] = 0;

	if ( len < cbBuf-3 )
		_tcscat( buf, _T( "\r\n" ));
}

static void GetFileDescription( DBEVENTINFO *dbei, TCHAR* buf, int cbBuf )
{
	int len = dbei->cbBlob - sizeof( DWORD );
	if ( len >= cbBuf )
		len = cbBuf-1;

	#if !defined( _UNICODE )
		memcpy( buf, dbei->pBlob + sizeof( DWORD ), len );
	#else
		MultiByteToWideChar( CP_ACP, 0, ( LPCSTR )dbei->pBlob + sizeof( DWORD ), len, buf, cbBuf );
	#endif
	buf[ len ] = 0;

	if ( len < cbBuf-3 )
		_tcscat( buf, _T( "\r\n" ));
}

static void GetObjectDescription( DBEVENTINFO *dbei, TCHAR* str, int cbStr )
{
	switch( dbei->eventType ) {
	case EVENTTYPE_MESSAGE:
		GetMessageDescription( dbei, str, cbStr );
		break;

	case EVENTTYPE_URL:
		GetUrlDescription( dbei, str, cbStr );
		break;

	case EVENTTYPE_FILE:
		GetFileDescription( dbei, str, cbStr );
		break;

	default:
		{
			DBEVENTTYPEDESCR* et = ( DBEVENTTYPEDESCR* )CallService( MS_DB_EVENT_GETTYPE, ( WPARAM )dbei->szModule, ( LPARAM )dbei->eventType );
			if ( et && ( et->flags & DETF_HISTORY )) {
				GetMessageDescription( dbei, str, cbStr );
			}
			else
				str[ 0 ] = 0;
}	}	}

static void GetObjectSummary( DBEVENTINFO *dbei, TCHAR* str, int cbStr )
{
	TCHAR* pszSrc, *pszTmp = NULL;

	switch( dbei->eventType ) {
	case EVENTTYPE_MESSAGE:
		if ( dbei->flags & DBEF_SENT )   pszSrc = TranslateT( "Outgoing Message" );
		else                             pszSrc = TranslateT( "Incoming Message" );
		break;

	case EVENTTYPE_URL:
		if ( dbei->flags & DBEF_SENT )   pszSrc = TranslateT( "Outgoing URL" );
      else                             pszSrc = TranslateT( "Incoming URL" );
		break;

	case EVENTTYPE_FILE:
		if ( dbei->flags & DBEF_SENT )   pszSrc = TranslateT( "Outgoing File" );
		else                             pszSrc = TranslateT( "Incoming File" );
		break;

	default:
		{
			DBEVENTTYPEDESCR* et = ( DBEVENTTYPEDESCR* )CallService( MS_DB_EVENT_GETTYPE, ( WPARAM )dbei->szModule, ( LPARAM )dbei->eventType );
			if ( et && ( et->flags & DETF_HISTORY )) {
				pszTmp = mir_a2t( et->descr );
				pszSrc = TranslateTS( pszTmp );
				break;
			}
			else {
				str[ 0 ] = 0;
				return;
	}	}	}

	_tcsncpy( str, ( const TCHAR* )pszSrc, cbStr );
	str[ cbStr-1 ] = 0;

	mir_free( pszTmp );
}

typedef struct {
	HANDLE hContact;
	HWND hwnd;
} THistoryThread;

static void FillHistoryThread(void* param)
{
	TCHAR str[200], eventText[256], strdatetime[64];
	HANDLE hDbEvent;
	DBEVENTINFO dbei;
	int newBlobSize,oldBlobSize,i;
	HWND hwndList;
	THistoryThread *hInfo = ( THistoryThread* )param;

	SendDlgItemMessage(hInfo->hwnd,IDC_LIST,LB_RESETCONTENT,0,0);
	i=CallService(MS_DB_EVENT_GETCOUNT,(WPARAM)hInfo->hContact,0);
	SendDlgItemMessage(hInfo->hwnd,IDC_LIST,LB_INITSTORAGE,i,i*40);

	ZeroMemory(&dbei,sizeof(dbei));
	dbei.cbSize=sizeof(dbei);
	oldBlobSize=0;
	hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDLAST,(WPARAM)hInfo->hContact,0);
	hwndList = GetDlgItem(hInfo->hwnd,IDC_LIST);
	while ( hDbEvent != NULL ) {
		if ( !IsWindow( hInfo->hwnd ))
			break;
		newBlobSize=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)hDbEvent,0);
		if(newBlobSize>oldBlobSize) {
			dbei.pBlob=(PBYTE)mir_realloc(dbei.pBlob,newBlobSize);
			oldBlobSize=newBlobSize;
		}
		dbei.cbBlob = oldBlobSize;
		CallService( MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei );
		GetObjectSummary(&dbei,str,SIZEOF(str));
		if(str[0]) {
			tmi.printTimeStamp(NULL, dbei.timestamp, _T("d t"), strdatetime, SIZEOF(strdatetime), 0);
			mir_sntprintf( eventText, SIZEOF(eventText), _T("%s: %s"), strdatetime, str );
			i = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)eventText );
			SendMessage(hwndList, LB_SETITEMDATA, i, (LPARAM)hDbEvent);
		}
		hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDPREV,(WPARAM)hDbEvent,0);
	}
	mir_free(dbei.pBlob);

	SendDlgItemMessage(hInfo->hwnd,IDC_LIST,LB_SETCURSEL,0,0);
	SendMessage(hInfo->hwnd,WM_COMMAND,MAKEWPARAM(IDC_LIST,LBN_SELCHANGE),0);
	EnableWindow(GetDlgItem(hInfo->hwnd, IDC_LIST), TRUE);
	mir_free(hInfo);
}

static int HistoryDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch(urc->wId) {
	case IDC_LIST:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
	case IDC_EDIT:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
	case IDC_FIND:
	case IDC_DELETEHISTORY:
		return RD_ANCHORX_LEFT|RD_ANCHORY_BOTTOM;
	case IDOK:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

static INT_PTR CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;

	hContact=(HANDLE)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)lParam);
		hContact = (HANDLE)lParam;
		WindowList_Add(hWindowList,hwndDlg,hContact);
		Utils_RestoreWindowPosition(hwndDlg,hContact,"History","");
		{
			TCHAR* contactName, str[200];
			contactName = cli.pfnGetContactDisplayName( hContact, 0 );
			mir_sntprintf(str,SIZEOF(str),TranslateT("History for %s"),contactName);
			SetWindowText(hwndDlg,str);
		}
		Window_SetIcon_IcoLib(hwndDlg, SKINICON_OTHER_HISTORY);
		SendMessage(hwndDlg,DM_HREBUILD,0,0);
		return TRUE;

	case DM_HREBUILD:
		{
			THistoryThread* hInfo = (THistoryThread*)mir_alloc(sizeof(THistoryThread));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIST), FALSE);
			hInfo->hContact = hContact;
			hInfo->hwnd = hwndDlg;
			forkthread(FillHistoryThread, 0, hInfo);
		}
		return TRUE;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Utils_SaveWindowPosition(hwndDlg,hContact,"History","");
		WindowList_Remove(hWindowList,hwndDlg);
		return TRUE;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x=300;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y=230;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd={0};
			urd.cbSize=sizeof(urd);
			urd.hwndDlg=hwndDlg;
			urd.hInstance=hMirandaInst;
			urd.lpTemplate=MAKEINTRESOURCEA(IDD_HISTORY);
			urd.lParam=(LPARAM)NULL;
			urd.pfnResizer=HistoryDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);
			return TRUE;
		}
	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		case IDC_FIND:
			ShowWindow(CreateDialogParam(hMirandaInst, MAKEINTRESOURCE(IDD_HISTORY_FIND), hwndDlg, DlgProcHistoryFind, (LPARAM)hwndDlg), SW_SHOW);
			return TRUE;

		case IDC_DELETEHISTORY:
			{
				HANDLE hDbevent;
				int index = SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETCURSEL,0,0);
				if ( index == LB_ERR )
					break;

				if ( MessageBox(hwndDlg,TranslateT("Are you sure you want to delete this history item?"),TranslateT("Delete History"),MB_YESNO|MB_ICONQUESTION)==IDYES) {
					hDbevent = (HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETITEMDATA,index,0);
					CallService(MS_DB_EVENT_DELETE,(WPARAM)hContact,(LPARAM)hDbevent);
					SendMessage(hwndDlg,DM_HREBUILD,0,0);
				}
				return TRUE;
			}
		case IDC_LIST:
			if ( HIWORD(wParam) == LBN_SELCHANGE ) {
				TCHAR str[8192],*contactName;
				HANDLE hDbEvent;
				DBEVENTINFO dbei;
				int sel;
				sel=SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETCURSEL,0,0);
				if(sel==LB_ERR) { EnableWindow(GetDlgItem(hwndDlg,IDC_DELETEHISTORY),FALSE); break; }
				EnableWindow(GetDlgItem(hwndDlg,IDC_DELETEHISTORY),TRUE);
				contactName = cli.pfnGetContactDisplayName( hContact, 0 );
				hDbEvent=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETITEMDATA,sel,0);
				ZeroMemory(&dbei,sizeof(dbei));
				dbei.cbSize=sizeof(dbei);
				dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)hDbEvent,0);
				if ((int)dbei.cbBlob != -1)
				{
					dbei.pBlob=(PBYTE)mir_alloc(dbei.cbBlob);
					if (CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei) == 0)
					{
						GetObjectDescription(&dbei,str,SIZEOF(str));
						if ( str[0] )
							SetDlgItemText(hwndDlg, IDC_EDIT, str);
					}
					mir_free(dbei.pBlob);
				}
			}
			return TRUE;
		}
		break;
	case DM_FINDNEXT:
		{
			TCHAR str[1024];
			HANDLE hDbEvent,hDbEventStart;
			DBEVENTINFO dbei;
			int newBlobSize,oldBlobSize;

			int index = SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETCURSEL,0,0);
			if ( index == LB_ERR )
				break;

			hDbEventStart=(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETITEMDATA,index,0);
			ZeroMemory(&dbei,sizeof(dbei));
			dbei.cbSize=sizeof(dbei);
			dbei.pBlob=NULL;
			oldBlobSize=0;
			for(;;) {
				hDbEvent = (HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,LB_GETITEMDATA,++index,0);
				if(hDbEvent == ( HANDLE )LB_ERR) {
					index = -1;
					continue;
				}
				if(hDbEvent==hDbEventStart) break;
				newBlobSize=CallService(MS_DB_EVENT_GETBLOBSIZE,(WPARAM)hDbEvent,0);
				if(newBlobSize>oldBlobSize) {
					dbei.pBlob=(PBYTE)mir_realloc(dbei.pBlob,newBlobSize);
					oldBlobSize=newBlobSize;
				}
				dbei.cbBlob=oldBlobSize;
				CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
				GetObjectDescription(&dbei,str,SIZEOF(str));
				if(str[0]) {
					CharUpperBuff(str,lstrlen(str));
					if( _tcsstr(str,(const TCHAR*)lParam)!=NULL) {
						SendDlgItemMessage(hwndDlg,IDC_LIST,LB_SETCURSEL,index,0);
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_LIST,LBN_SELCHANGE),0);
						break;
			}	}	}
			
			mir_free(dbei.pBlob);
			break;
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcHistoryFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		return TRUE;

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
			case IDOK://find Next
			{	
				TCHAR str[128];
				HWND hwndParent = ( HWND )GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				GetDlgItemText(hwndDlg, IDC_FINDWHAT, str, SIZEOF(str));
				CharUpperBuff(str,lstrlen(str));
				SendMessage(hwndParent,DM_FINDNEXT,0,(LPARAM)str);
				return TRUE;
			}
			case IDCANCEL:
				DestroyWindow(hwndDlg);
				return TRUE;
		}
		break;
	}
	return FALSE;
}
