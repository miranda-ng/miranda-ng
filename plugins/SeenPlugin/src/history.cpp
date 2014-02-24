/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-06  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "seen.h"

static HANDLE hWindowList;

char* BuildSetting(int historyLast) {
	static char setting[15];
	static char sztemp[15];
	*setting = '\0';
	strcat(setting, "History_");
	strcat(setting, _itoa(historyLast, sztemp, 10));
	return setting;
}

void HistoryWrite(MCONTACT hContact)
{
	int historyMax = db_get_w(NULL, S_MOD, "HistoryMax", 10);
	if (historyMax < 0)
		historyMax=0;
	else if (historyMax > 99)
		historyMax = 99;
	if (historyMax == 0)
		return;

	int historyFirst = db_get_w(hContact, S_MOD, "HistoryFirst", 0);
	if (historyFirst >=  historyMax)
		historyFirst = 0;

	int historyLast = db_get_w(hContact, S_MOD, "HistoryLast", 0);
	if (historyLast >= historyMax)
		historyLast = historyMax-1;

	TCHAR *ptszString;
	DBVARIANT dbv;
	if ( !db_get_ts(NULL, S_MOD, "HistoryStamp", &dbv)) {
		ptszString = ParseString(dbv.ptszVal, hContact, 0);
		db_free(&dbv);
	}
	else ptszString = ParseString(DEFAULT_HISTORYSTAMP, hContact, 0);
	db_set_ts(hContact, S_MOD, BuildSetting(historyLast), ptszString);

	historyLast = (historyLast+1) % historyMax;
	db_set_w(hContact, S_MOD, "HistoryLast", historyLast);
	if (historyLast == historyFirst)
		db_set_w(hContact, S_MOD, "HistoryFirst", (historyFirst+1) % historyMax);
}

void LoadHistoryList(MCONTACT hContact, HWND hwnd, int nList)
{
	SendDlgItemMessage(hwnd, nList, LB_RESETCONTENT, 0, 0);
	int historyMax = db_get_w(NULL,S_MOD,"HistoryMax",10);
	if (historyMax < 0)
		historyMax = 0;
	else if (historyMax > 99)
		historyMax = 99;
	if (historyMax == 0)
		return;
	
	int historyFirst = db_get_w(hContact,S_MOD,"HistoryFirst",0);
	if (historyFirst >= historyMax)
		historyFirst = 0;
	
	int historyLast = db_get_w(hContact,S_MOD,"HistoryLast",0);
	if (historyLast >= historyMax)
		historyLast = historyMax-1;
	
	for (int i = historyLast; i != historyFirst; ) {
		i = (i-1+historyMax) % historyMax;
		
		DBVARIANT dbv;
		if ( !db_get_ts(hContact, S_MOD, BuildSetting(i), &dbv)) {
			SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, (LPARAM)dbv.ptszVal);
			db_free(&dbv);
		}
		else SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, (LPARAM)_T(""));
	}
}


HDWP MyResizeWindow (HDWP hDwp, HWND hwndDlg, HWND hwndControl,
				 int nHorizontalOffset, int nVerticalOffset, 
				 int nWidthOffset, int nHeightOffset)
{
	POINT pt;
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	
	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	// resize control
	return DeferWindowPos(hDwp, hwndControl, NULL,
			pt.x + nHorizontalOffset, 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left + nWidthOffset, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);


}

HDWP MyHorizCenterWindow (HDWP hDwp, HWND hwndDlg, HWND hwndControl,
				 int nClientWidth, int nVerticalOffset, 
				 int nHeightOffset)
{
	POINT pt;
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	
	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	// resize control
	return DeferWindowPos(hDwp, hwndControl, NULL,
			(int) ((nClientWidth - (rcinit.right - rcinit.left))/2), 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);

}

void MyResizeGetOffset (HWND hwndDlg, HWND hwndControl, int nWidth, int nHeight, int* nDx, int* nDy)
{
	RECT rcinit;

	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);
	
	// calculate offsets
	*nDx = nWidth - (rcinit.right - rcinit.left);
	*nDy = nHeight - (rcinit.bottom - rcinit.top);
}

INT_PTR CALLBACK HistoryDlgProc(HWND hwndDlg, UINT Message, WPARAM wparam, LPARAM lparam)
{
	MCONTACT hContact;
	TCHAR sztemp[1024];

	switch(Message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hContact = (MCONTACT)lparam;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam);
		mir_sntprintf(sztemp, SIZEOF(sztemp), _T("%s: %s"),
			CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR),
			TranslateT("last seen history"));
		SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)sztemp);
		SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
		SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

		if ( db_get_b(hContact,S_MOD,"OnlineAlert",0))
			SendDlgItemMessage(hwndDlg, IDC_STATUSCHANGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		
		SendDlgItemMessage(hwndDlg, IDC_DETAILS,  BM_SETIMAGE, IMAGE_ICON, (WPARAM)LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS));
		SendDlgItemMessage(hwndDlg, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (WPARAM)LoadSkinnedIcon(SKINICON_OTHER_DOWNARROW));
		SendDlgItemMessage(hwndDlg, IDC_SENDMSG,  BM_SETIMAGE, IMAGE_ICON, (WPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));

		//set-up tooltips
		SendDlgItemMessage(hwndDlg, IDC_DETAILS,  BUTTONADDTOOLTIP, (WPARAM)TranslateT("View User's Details"), BATF_TCHAR);
		SendDlgItemMessage(hwndDlg, IDC_USERMENU, BUTTONADDTOOLTIP, (WPARAM)TranslateT("User Menu"), BATF_TCHAR);
		SendDlgItemMessage(hwndDlg, IDC_SENDMSG,  BUTTONADDTOOLTIP, (WPARAM)TranslateT("Send Instant Message"), BATF_TCHAR);

		Utils_RestoreWindowPositionNoMove(hwndDlg,NULL,S_MOD,"History_");
		ShowWindow(hwndDlg, SW_SHOW);
		return TRUE;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM,wparam,lparam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM,wparam,lparam);

	case WM_COMMAND:
		hContact=(MCONTACT)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND,MAKEWPARAM(LOWORD(wparam),MPCF_CONTACTMENU), hContact))
			break;

		switch(LOWORD(wparam)) {
		case IDCANCEL:
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		case IDOK:
			if (SendDlgItemMessage(hwndDlg, IDC_STATUSCHANGE, BM_GETCHECK, 0, 0) == BST_CHECKED)
				db_set_b(hContact,S_MOD,"OnlineAlert",1);
			else
				db_set_b(hContact,S_MOD,"OnlineAlert",0);
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		case IDC_USERMENU:
			{	
				RECT rc;
				HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,hContact,0);
				GetWindowRect(GetDlgItem(hwndDlg,IDC_USERMENU),&rc);
				TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwndDlg,NULL);
				DestroyMenu(hMenu);
			}
			break;
		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG,hContact,0);
			break;
		case IDC_SENDMSG:
			CallService(MS_MSG_SENDMESSAGE,hContact,0);
			break;
		case IDC_TEST:
			debug( ParseString( LPGENT("Date: %d.%m.%y(%Y) \n Date desc: %W - %w - %E - %e \n Time: %H:%M:%S (%h-%p) \n user: %n - %u \n Status: %s \n IP: %i - %r"), hContact, 0));
			break;
		}
		break;

	case WM_SIZE:
		{
			int dx, dy;
			HDWP hDwp = BeginDeferWindowPos(6);
			MyResizeGetOffset(hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORYLIST), LOWORD(lparam)-15, HIWORD(lparam)-99, &dx, &dy);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_USERMENU), dx, 0, 0, 0);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_DETAILS), dx, 0, 0, 0);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_SENDMSG), dx, 0, 0, 0);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORYLIST), 0, 0, dx, dy);
			hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_STATUSCHANGE), 0, dy, dx, 0);
			hDwp = MyHorizCenterWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDOK), LOWORD(lparam), dy, 0);
			EndDeferWindowPos(hDwp);
		}				
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO mmi;
			CopyMemory (&mmi, (LPMINMAXINFO) lparam, sizeof (MINMAXINFO));

			/* The minimum width in points*/
			mmi.ptMinTrackSize.x = 200;
			/* The minimum height in points*/
			mmi.ptMinTrackSize.y = 190;

			CopyMemory ((LPMINMAXINFO) lparam, &mmi, sizeof (MINMAXINFO));
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		WindowList_Remove(hWindowList,hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg,NULL,S_MOD,"History_");
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}

void ShowHistory(MCONTACT hContact, BYTE isAlert)
{
	HWND hHistoryDlg = WindowList_Find(hWindowList,hContact);
	if (hHistoryDlg == NULL) {
		hHistoryDlg = CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_HISTORY),NULL,HistoryDlgProc,hContact);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		WindowList_Add(hWindowList,hHistoryDlg,hContact);
	} 
	else {
		SetForegroundWindow(hHistoryDlg);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		SetFocus(hHistoryDlg);
	}
	
	if (isAlert) 
		SkinPlaySound("LastSeenTrackedStatusChange");
}

void InitHistoryDialog(void)
{
	hWindowList = WindowList_Create();
}

void UninitHistoryDialog(void)
{
	WindowList_Destroy(hWindowList);
}