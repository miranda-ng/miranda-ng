/*
    AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
    KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "commonstatus.h" 
#include "resource.h"

#define TIMER_ID	1

// variables
static HWND win;
static int timeOut;

struct TConfirmSetting : public PROTOCOLSETTINGEX
{
	TConfirmSetting( const PROTOCOLSETTINGEX& x )
	{
		memcpy( this, &x, sizeof(PROTOCOLSETTINGEX));
		if ( szMsg )
			szMsg = _strdup( szMsg );
	}

	~TConfirmSetting()
	{
		if ( szMsg )
			free( szMsg );
	}
};

static int CompareSettings( const TConfirmSetting* p1, const TConfirmSetting* p2 )
{	return lstrcmpA( p1->szName, p2->szName );
}

static OBJLIST<TConfirmSetting> confirmSettings( 10, CompareSettings );

static INT_PTR CALLBACK StatusMessageDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static PROTOCOLSETTINGEX* protoSetting = NULL;
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		protoSetting = (PROTOCOLSETTINGEX *)lParam;
		if (protoSetting->szMsg == NULL) {
			TCHAR* smsg = GetDefaultStatusMessage(protoSetting, GetActualStatus(protoSetting));
			if	(smsg != NULL) {
				SetDlgItemText(hwndDlg, IDC_STSMSG, smsg);
				mir_free(smsg);
			}
		}
		else SetDlgItemTextA(hwndDlg, IDC_STSMSG, protoSetting->szMsg);
		
		{
			TCHAR desc[ 512 ];
			mir_sntprintf(desc, SIZEOF(desc), 
				TranslateT("Set %s message for %s."),
				CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, GetActualStatus(protoSetting), GSMDF_TCHAR ),
				protoSetting->tszAccName );
			SetDlgItemText(hwndDlg, IDC_DESCRIPTION, desc);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OK:
			{
				int len = SendMessage(GetDlgItem(hwndDlg, IDC_STSMSG), WM_GETTEXTLENGTH, 0, 0);
				if (len > 0) {
					protoSetting->szMsg = ( char* )realloc(protoSetting->szMsg, len+1);
					if (protoSetting->szMsg != NULL)
						GetDlgItemTextA(hwndDlg, IDC_STSMSG, protoSetting->szMsg, len+1);
				}
				SendMessage(GetParent(hwndDlg), UM_STSMSGDLGCLOSED, (WPARAM)TRUE, 0);
				EndDialog(hwndDlg, IDC_OK);
			}
			break;

		case IDC_CANCEL:
			SendMessage(GetParent(hwndDlg), UM_STSMSGDLGCLOSED, (WPARAM)0, 0);
			EndDialog(hwndDlg, IDC_CANCEL);
			break;
		}
		break;
	}

	return FALSE;
}

static int SetStatusList(HWND hwndDlg)
{
	if (confirmSettings.getCount() == 0)
		return -1;

	HWND hList = GetDlgItem(hwndDlg, IDC_STARTUPLIST);
	TCHAR buf[100];

	// create items
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.cchTextMax = 256;

	for( int i=0; i < confirmSettings.getCount(); i++ ) {
		lvItem.pszText = confirmSettings[i].tszAccName;
		if (ListView_GetItemCount(hList) < confirmSettings.getCount()) 
			ListView_InsertItem(hList,&lvItem);

		int actualStatus;
		switch( confirmSettings[i].status ) {
			case ID_STATUS_LAST:    actualStatus = confirmSettings[i].lastStatus;   break;
			case ID_STATUS_CURRENT: actualStatus = CallProtoService(confirmSettings[i].szName,PS_GETSTATUS, 0, 0); break;
			default:                actualStatus = confirmSettings[i].status;
		}
		TCHAR* status = ( TCHAR* )CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, actualStatus, GSMDF_TCHAR );
		switch( confirmSettings[i].status ) {
		case ID_STATUS_LAST:
			mir_sntprintf(buf, SIZEOF(buf), _T("%s (%s)"), TranslateT("<last>"), status);
			ListView_SetItemText(hList, lvItem.iItem, 1, buf);
			break;
		case ID_STATUS_CURRENT:
			mir_sntprintf( buf, SIZEOF(buf), _T("%s (%s)"), TranslateT("<current>"), status);
			ListView_SetItemText(hList, lvItem.iItem, 1, buf);
			break;
		default:
			ListView_SetItemText(hList, lvItem.iItem, 1, status);
		}

		// status message
		if ( !((!((CallProtoService(confirmSettings[i].szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND)&~PF1_INDIVMODEMSG)) || (!(CallProtoService(confirmSettings[i].szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus)))) ) {
			TCHAR *msg = GetDefaultStatusMessage( &confirmSettings[i], actualStatus );
			if ( msg != NULL ) {
				TCHAR* fMsg = variables_parsedup( msg, confirmSettings[i].tszAccName, NULL );
				ListView_SetItemText(hList, lvItem.iItem, 2, fMsg);
				free(fMsg);
				mir_free(msg);
			}
			else ListView_SetItemText(hList, lvItem.iItem, 2, TranslateT("<n/a>"));
		}
		else ListView_SetItemText(hList, lvItem.iItem, 2, TranslateT("<n/a>"));

		ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hList, 2, LVSCW_AUTOSIZE);
		lvItem.lParam = (LPARAM)&confirmSettings[i];
		ListView_SetItem(hList,&lvItem);
		lvItem.iItem++;
	}

	// grey out status box
	EnableWindow(GetDlgItem(hwndDlg,IDC_STATUS), (ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST),-1,LVNI_SELECTED)>=0));
	return 0;
}

static BOOL CALLBACK ConfirmDlgProc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HWND hList=GetDlgItem(hwndDlg,IDC_STARTUPLIST);
			SendMessage(hList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_FULLROWSELECT);

			// create columns
			LVCOLUMN lvCol = { 0 };
			lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
			lvCol.pszText = TranslateT("Protocol");
			ListView_InsertColumn(hList, 0, &lvCol );

			lvCol.cx = 100;
			lvCol.pszText = TranslateT("Status");
			ListView_InsertColumn(hList, 1, &lvCol );

			lvCol.pszText = TranslateT("Message");
			ListView_InsertColumn(hList, 2, &lvCol );
		}

		// create items
		SetStatusList(hwndDlg);

		EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), FALSE);
		// fill profile combo box
		if (!ServiceExists(MS_SS_GETPROFILE))
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), FALSE);
		else {
			int defaultProfile;
			int profileCount = (int)CallService(MS_SS_GETPROFILECOUNT, (WPARAM)&defaultProfile, 0);
			for ( int i=0; i < profileCount; i++ ) {
				char profileName[128];
				CallService( MS_SS_GETPROFILENAME, i, (LPARAM)profileName );
				int item = SendDlgItemMessageA( hwndDlg, IDC_PROFILE, CB_ADDSTRING, 0, (LPARAM)profileName );
				SendDlgItemMessage( hwndDlg, IDC_PROFILE, CB_SETITEMDATA, item, ( LPARAM )i );
			}
			if ( profileCount == 0 )
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROFILE), FALSE);
		}
		// start timer
		if (timeOut > 0) {
			TCHAR text[32];
			_sntprintf(text, SIZEOF(text), TranslateT("Closing in %d"), timeOut);
			SetDlgItemText(hwndDlg, IDC_CLOSE, text);
			SetTimer(hwndDlg, TIMER_ID, 1000, NULL);
		}
		break;

	case WM_TIMER:
		{
			TCHAR text[32];
			_sntprintf(text, SIZEOF(text), TranslateT("Closing in %d"), timeOut-1);
			SetDlgItemText(hwndDlg, IDC_CLOSE, text);
			if (timeOut <= 0) {
				KillTimer(hwndDlg, TIMER_ID);
				SendMessage(hwndDlg, UM_CLOSECONFIRMDLG, 0, 0);
			}
			else timeOut--;
	   }
		break;

	case WM_COMMAND:
		// stop timer
		KillTimer(hwndDlg, TIMER_ID);
		SetDlgItemText(hwndDlg, IDC_CLOSE, TranslateT("Close"));
		if ( (HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == NM_CLICK) ) { // something clicked
			switch (LOWORD(wParam)) {
			case IDC_PROFILE:
				{
					int i, profile = (int)SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_PROFILE, CB_GETCURSEL, 0, 0), 0);
					for ( i=0; i < confirmSettings.getCount(); i++ ) {
						if (confirmSettings[i].szMsg != NULL) {
							free(confirmSettings[i].szMsg);
							confirmSettings[i].szMsg = NULL;
					}	}

					CallService(MS_SS_GETPROFILE, (WPARAM)profile, (LPARAM)&confirmSettings);
					for ( i=0; i < confirmSettings.getCount(); i++ )
						if (confirmSettings[i].szMsg != NULL) // we free this later, copy to our memory space
							confirmSettings[i].szMsg = _strdup(confirmSettings[i].szMsg);

					SetStatusList(hwndDlg);
				}
				break;

			case IDC_STATUS:
				{
					LVITEM lvItem = { 0 };
					lvItem.mask = LVIF_TEXT | LVIF_PARAM;
					lvItem.iSubItem = 0;
					lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST),-1,LVNI_SELECTED);
					if (lvItem.iItem == -1)
						break;
					ListView_GetItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST), &lvItem);
					PROTOCOLSETTINGEX* proto = (PROTOCOLSETTINGEX*)lvItem.lParam;
					int actualStatus = proto->status = (int)SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCURSEL, 0, 0), 0);

					// LAST STATUS
					if (proto->status == ID_STATUS_LAST) {
						TCHAR last[80];
						mir_sntprintf(last, SIZEOF(last), _T("%s (%s)"), 
							TranslateT("<last>"), 
							CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->lastStatus, GSMDF_TCHAR ));
						ListView_SetItemText(GetDlgItem(hwndDlg,IDC_STARTUPLIST), lvItem.iItem, 1, last);
						actualStatus = proto->lastStatus;
					}

					// CURRENT STATUS
					else if (proto->status == ID_STATUS_CURRENT) {
						int currentStatus = CallProtoService(proto->szName,PS_GETSTATUS, 0, 0);
						TCHAR current[80];
						mir_sntprintf(current, SIZEOF(current), _T("%s (%s)"), 
							TranslateT("<current>"),
							CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, currentStatus, GSMDF_TCHAR ));
						ListView_SetItemText(GetDlgItem(hwndDlg,IDC_STARTUPLIST), lvItem.iItem, 1, current);
						actualStatus = currentStatus;
					}
					else ListView_SetItemText(GetDlgItem(hwndDlg,IDC_STARTUPLIST), lvItem.iItem, 1, 
						( TCHAR* )CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, proto->status, GSMDF_TCHAR ));

					if  ( (!((CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND)&~PF1_INDIVMODEMSG)) || (!(CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus))) )
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), FALSE);
					else if ( (proto->status == ID_STATUS_LAST) || (proto->status == ID_STATUS_CURRENT) )
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), TRUE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), (CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus))?TRUE:FALSE);
					SetStatusList(hwndDlg);
				}
				break;

			case IDC_SETSTSMSG:
				{
					LVITEM lvItem = { 0 };
					lvItem.mask=LVIF_TEXT|LVIF_PARAM;
					lvItem.iSubItem=0;
					lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST),-1,LVNI_SELECTED);
					if (lvItem.iItem == -1)
						break;

					ListView_GetItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST), &lvItem);
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETSTSMSGDIALOG), hwndDlg, StatusMessageDlgProc, lvItem.lParam);
				}
				break;

			case IDC_CLOSE:
				SendMessage(hwndDlg, UM_CLOSECONFIRMDLG, 0, 0);
				break;

			case IDC_CANCEL:
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;
	
	case WM_NOTIFY:
		switch(((NMHDR*)lParam)->idFrom) {
		case IDC_STARTUPLIST:
			switch(((NMHDR*)lParam)->code) {
			// fill combobox
			case NM_CLICK:
				{
					LVITEM lvItem;
					PROTOCOLSETTINGEX* proto;
					TCHAR buf[100];
					int item, flags, i, currentStatus, actualStatus;
					
					KillTimer(hwndDlg, TIMER_ID);
					SetDlgItemText(hwndDlg, IDC_CLOSE, TranslateT("Close"));

					memset(&lvItem,0,sizeof(lvItem));
					lvItem.mask=LVIF_TEXT|LVIF_PARAM;
					lvItem.iSubItem=0;
					lvItem.iItem = ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST),-1,LVNI_SELECTED);
				
					if (ListView_GetItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST), &lvItem) == FALSE) {
						SetStatusList(hwndDlg);
						break;
					}
					proto = (PROTOCOLSETTINGEX*)lvItem.lParam;
									
					flags = CallProtoService(proto->szName, PS_GETCAPS,PFLAGNUM_2,0)&~CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);
					// clear box and add new status, loop status and check if compatible with proto
					SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
					actualStatus = proto->status;

					// last
					mir_sntprintf(buf, SIZEOF(buf), _T("%s (%s)"), 
						TranslateT("<last>"), 
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)proto->lastStatus, GSMDF_TCHAR));
					item = SendDlgItemMessage( hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)buf );
					SendDlgItemMessage( hwndDlg, IDC_STATUS, CB_SETITEMDATA, item, (LPARAM)ID_STATUS_LAST );
					if (proto->status == ID_STATUS_LAST) {
						SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_SETCURSEL,(WPARAM)item,0);
						actualStatus = proto->lastStatus;
					}

					// current
					currentStatus = CallProtoService(proto->szName,PS_GETSTATUS, 0, 0);
					mir_sntprintf( buf, SIZEOF(buf), _T("%s (%s)"),
						TranslateT("<current>"),
						CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)currentStatus, GSMDF_TCHAR));
					item = SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_ADDSTRING,0,(LPARAM)buf);
					SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_SETITEMDATA,item,(LPARAM)ID_STATUS_CURRENT);
					if (proto->status == ID_STATUS_CURRENT) {
						SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_SETCURSEL,(WPARAM)item,0);
						actualStatus = currentStatus;
					}

					for( i = 0; i < SIZEOF(statusModeList); i++ ) {
						if ( ((flags&statusModePf2List[i]) || (statusModePf2List[i] == PF2_OFFLINE)) && (!((!(flags)&Proto_Status2Flag(statusModePf2List[i]))) || ((CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0)&Proto_Status2Flag(statusModePf2List[i])))) ) {
							TCHAR* statusMode = ( TCHAR* )CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, statusModeList[i], GSMDF_TCHAR);
							item = SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_ADDSTRING,0,(LPARAM)statusMode);
							SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_SETITEMDATA,item,(LPARAM)statusModeList[i]);
							if (statusModeList[i] == proto->status)
								SendDlgItemMessage(hwndDlg,IDC_STATUS,CB_SETCURSEL,(WPARAM)item,0);
					}	}

					// enable status box
					EnableWindow(GetDlgItem(hwndDlg,IDC_STATUS), (ListView_GetNextItem(GetDlgItem(hwndDlg,IDC_STARTUPLIST),-1,LVNI_SELECTED)>=0));
					if ( (!((CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0)&PF1_MODEMSGSEND)&~PF1_INDIVMODEMSG)) || (!(CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus))) )
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), FALSE);
					else if ( proto->status == ID_STATUS_LAST || proto->status == ID_STATUS_CURRENT )
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), TRUE);
					else
						EnableWindow(GetDlgItem(hwndDlg, IDC_SETSTSMSG), (CallProtoService(proto->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_3, 0)&Proto_Status2Flag(actualStatus))?TRUE:FALSE);
				}
				break;
			}
			break;
		}
		break;
	
	case UM_STSMSGDLGCLOSED:
		SetStatusList(hwndDlg);
		break;

	case UM_CLOSECONFIRMDLG:
		CallService(MS_CS_SETSTATUSEX, (WPARAM)&confirmSettings, 0);
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		confirmSettings.destroy();
		break;
	}

	return 0;
}

INT_PTR ShowConfirmDialogEx(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == 0 )
		return -1;

	confirmSettings.destroy();

	OBJLIST<PROTOCOLSETTINGEX>& param = *( OBJLIST<PROTOCOLSETTINGEX>* )wParam;
	for ( int i=0; i < param.getCount(); i++ )
		confirmSettings.insert( new TConfirmSetting( param[i] ));

	timeOut = lParam;
	if (timeOut < 0)
		timeOut = DEF_CLOSE_TIME;

	if ( GetWindow(win, 0) == NULL ) {
		win = CreateDialogParam( hInst,(LPCTSTR)MAKEINTRESOURCE(IDD_CONFIRMDIALOG),(HWND)NULL,(DLGPROC)ConfirmDlgProc,(LPARAM)NULL);
		EnableWindow(win,TRUE);
	}
	
	return (INT_PTR)win;
}
