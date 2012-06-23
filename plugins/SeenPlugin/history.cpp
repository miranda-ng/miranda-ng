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

File name      : $URL: http://svn.berlios.de/svnroot/repos/mgoodies/trunk/lastseen-mod/history.c $
Revision       : $Rev: 1056 $
Last change on : $Date: 2006-10-30 06:22:07 +0300 (Пн, 30 окт 2006) $
Last change by : $Author: y_b $
*/
#include "seen.h"


extern HINSTANCE hInstance;

static HANDLE hWindowList;

char* BuildSetting(int historyLast) {
	static char setting[15];
	static char sztemp[15];
	*setting = '\0';
	strcat(setting, "History_");
	strcat(setting, itoa(historyLast, sztemp, 10));
	return setting;
}

void HistoryWrite(HANDLE hContact)
{
	short historyFirst, historyLast, historyMax;
	DBVARIANT dbv;

	historyMax = DBGetContactSettingWord(NULL,S_MOD,"HistoryMax",10);
	if (historyMax < 0) historyMax=0; else if (historyMax > 99) historyMax = 99;
	if (historyMax == 0) return;
	historyFirst = DBGetContactSettingWord(hContact,S_MOD,"HistoryFirst",0);
	if (historyFirst >=  historyMax) historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact,S_MOD,"HistoryLast",0);
	if (historyLast >= historyMax) historyLast = historyMax-1;

	DBWriteContactSettingString(hContact,S_MOD,BuildSetting(historyLast),
			ParseString(!DBGetContactSetting(NULL,S_MOD,"HistoryStamp",&dbv)?dbv.pszVal:DEFAULT_HISTORYSTAMP,hContact,0));
	DBFreeVariant(&dbv);

	historyLast = (historyLast+1) % historyMax;
	DBWriteContactSettingWord(hContact,S_MOD,"HistoryLast",historyLast);
	if (historyLast == historyFirst) {
		DBWriteContactSettingWord(hContact,S_MOD,"HistoryFirst",(short) ((historyFirst+1) % historyMax));
	}

}

void LoadHistoryList(HANDLE hContact, HWND hwnd, int nList) {
	short historyFirst, historyLast, historyMax;
	short i;
	DBVARIANT dbv;


	SendDlgItemMessage(hwnd, nList, LB_RESETCONTENT, 0, 0);
	historyMax = DBGetContactSettingWord(NULL,S_MOD,"HistoryMax",10);
	if (historyMax < 0) historyMax = 0; else if (historyMax > 99) historyMax = 99;
	if (historyMax == 0) return;
	historyFirst = DBGetContactSettingWord(hContact,S_MOD,"HistoryFirst",0);
	if (historyFirst >=  historyMax) historyFirst = 0;
	historyLast = DBGetContactSettingWord(hContact,S_MOD,"HistoryLast",0);
	if (historyLast >= historyMax) historyLast = historyMax-1;
	
	i = historyLast;
	while (i != historyFirst) {
		i = (i-1+historyMax) % historyMax;
		SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, 
				(LPARAM)(!DBGetContactSetting(hContact,S_MOD,BuildSetting(i),&dbv)?dbv.pszVal:""));
		DBFreeVariant(&dbv);
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
/*	MoveWindow(hwndControl, 
			pt.x + nHorizontalOffset, 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left + nWidthOffset, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			FALSE);
*/
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
/*	MoveWindow(hwndControl, 
			(int) ((nClientWidth - (rcinit.right - rcinit.left))/2), 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			TRUE);
*/
	return DeferWindowPos(hDwp, hwndControl, NULL,
			(int) ((nClientWidth - (rcinit.right - rcinit.left))/2), 
			pt.y + nVerticalOffset,
			rcinit.right - rcinit.left, 
			rcinit.bottom - rcinit.top + nHeightOffset,
			SWP_NOZORDER);

}
void MyResizeGetOffset (HWND hwndDlg, HWND hwndControl, 
				 int nWidth, int nHeight,
				 int* nDx, int* nDy)
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
	HANDLE hContact;
	char sztemp[1024]="";
	static HIMAGELIST hIml=NULL;
	
	switch(Message)
    {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			hContact = (HANDLE)lparam;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lparam);
			strcpy(sztemp,(char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0));
			strcat(sztemp, ": ");
			strcat(sztemp, Translate("last seen history"));
			SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)sztemp);
			SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));
			SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) LoadSkinnedIcon(SKINICON_OTHER_MIRANDA));

//			LoadHistoryList(hContact, hwndDlg, IDC_HISTORYLIST);

			if (DBGetContactSettingByte(hContact,S_MOD,"OnlineAlert",0))
				SendDlgItemMessage(hwndDlg, IDC_STATUSCHANGE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			{
				hIml=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,3,3);
				ImageList_AddIcon(hIml,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_USERDETAILS)));
				ImageList_AddIcon(hIml,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_DOWNARROW)));
				ImageList_AddIcon(hIml,LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
				SendDlgItemMessage(hwndDlg,IDC_DETAILS,BM_SETIMAGE,IMAGE_ICON,(WPARAM)ImageList_GetIcon(hIml,0,ILD_NORMAL));
				SendDlgItemMessage(hwndDlg,IDC_USERMENU,BM_SETIMAGE,IMAGE_ICON,(WPARAM)ImageList_GetIcon(hIml,1,ILD_NORMAL));
				SendDlgItemMessage(hwndDlg,IDC_SENDMSG,BM_SETIMAGE,IMAGE_ICON,(WPARAM)ImageList_GetIcon(hIml,2,ILD_NORMAL));
			}

			//set-up tooltips
			{
				HWND hwndDlgToolTips;
				TOOLINFO ti;

				hwndDlgToolTips = CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,"",WS_POPUP,0,0,0,0,NULL,NULL,GetModuleHandle(NULL),NULL);
				ZeroMemory(&ti,sizeof(ti));
				ti.cbSize=sizeof(ti);
				ti.uFlags=TTF_IDISHWND|TTF_SUBCLASS;
				ti.uId=(UINT)GetDlgItem(hwndDlg,IDC_USERMENU);
				ti.lpszText=Translate("User Menu");
				SendMessage(hwndDlgToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
				ti.uId=(UINT)GetDlgItem(hwndDlg,IDC_DETAILS);
				ti.lpszText=Translate("View User's Details");
				SendMessage(hwndDlgToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
				ti.uId=(UINT)GetDlgItem(hwndDlg,IDC_SENDMSG);
				ti.lpszText=Translate("Send Instant Message");
				SendMessage(hwndDlgToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
			}


			Utils_RestoreWindowPositionNoMove(hwndDlg,NULL,S_MOD,"History_");
			ShowWindow(hwndDlg, SW_SHOW);
			break;

		case WM_MEASUREITEM:
			return CallService(MS_CLIST_MENUMEASUREITEM,wparam,lparam);
		case WM_DRAWITEM:
			return CallService(MS_CLIST_MENUDRAWITEM,wparam,lparam);
        case WM_COMMAND:
			hContact=(HANDLE)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
			if(CallService(MS_CLIST_MENUPROCESSCOMMAND,MAKEWPARAM(LOWORD(wparam),MPCF_CONTACTMENU),(LPARAM)hContact))
				break;
            switch(LOWORD(wparam))
            {
				case IDCANCEL:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				case IDOK:
					if (SendDlgItemMessage(hwndDlg, IDC_STATUSCHANGE, BM_GETCHECK, 0, 0) == BST_CHECKED)
						DBWriteContactSettingByte(hContact,S_MOD,"OnlineAlert",1);
					else
						DBWriteContactSettingByte(hContact,S_MOD,"OnlineAlert",0);
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
				case IDC_USERMENU:
					{	
						RECT rc;
						HMENU hMenu=(HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)hContact,0);
						GetWindowRect(GetDlgItem(hwndDlg,IDC_USERMENU),&rc);
						TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwndDlg,NULL);
						DestroyMenu(hMenu);
					}
					break;
				case IDC_DETAILS:
					CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)hContact,0);
					break;
				case IDC_SENDMSG:
					CallService(MS_MSG_SENDMESSAGE,(WPARAM)hContact,0);
					break;
                case IDC_TEST:
					debug(ParseString("Date: %d.%m.%y(%Y) \n Date desc: %W - %w - %E - %e \n Time: %H:%M:%S (%h-%p) \n user: %n - %u \n Status: %s \n IP: %i - %r",hContact,0));
					break;
            }
			break;
		case WM_SIZE:
			{
				int dx, dy;
				HDWP hDwp;

				hDwp = BeginDeferWindowPos(6);
				MyResizeGetOffset(hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORYLIST), 
						LOWORD(lparam)-15, HIWORD(lparam)-99, &dx, &dy);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_USERMENU), 
						dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_DETAILS), 
						dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_SENDMSG), 
						dx, 0, 0, 0);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_HISTORYLIST), 
						0, 0, dx, dy);
				hDwp = MyResizeWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDC_STATUSCHANGE), 
						0, dy, dx, 0);
				hDwp = MyHorizCenterWindow(hDwp, hwndDlg, GetDlgItem(hwndDlg, IDOK), 
						LOWORD(lparam), dy, 0);
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
			ImageList_Destroy(hIml);
			break;
        default:
            return FALSE;
    }
	return TRUE;
}

void ShowHistory(HANDLE hContact, BYTE isAlert)
{
	HWND hHistoryDlg;
	
	hHistoryDlg = WindowList_Find(hWindowList,hContact);
	if (hHistoryDlg == NULL)
	{
		hHistoryDlg = CreateDialogParam(hInstance,MAKEINTRESOURCE(IDD_HISTORY),NULL,HistoryDlgProc,(LPARAM)hContact);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		WindowList_Add(hWindowList,hHistoryDlg,hContact);
	} 
	else 
	{
		SetForegroundWindow(hHistoryDlg);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		SetFocus(hHistoryDlg);
	}
	
	if (isAlert) 
	{
		SkinPlaySound("LastSeenTrackedStatusChange");
	}
}


void InitHistoryDialog(void)
{
	hWindowList=(HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
}
