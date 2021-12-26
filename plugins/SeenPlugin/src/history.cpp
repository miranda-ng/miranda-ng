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

#include "stdafx.h"

static MWindowList hWindowList;

char* BuildSetting(int historyLast)
{
	static char setting[15];
	static char sztemp[15];
	*setting = '\0';
	mir_strcat(setting, "History_");
	mir_strcat(setting, _itoa(historyLast, sztemp, 10));
	return setting;
}

void HistoryWrite(MCONTACT hContact)
{
	int historyMax = g_plugin.getWord("HistoryMax", 10);
	if (historyMax < 0)
		historyMax = 0;
	else if (historyMax > 99)
		historyMax = 99;
	if (historyMax == 0)
		return;

	int historyFirst = g_plugin.getWord(hContact, "HistoryFirst", 0);
	if (historyFirst >= historyMax)
		historyFirst = 0;

	int historyLast = g_plugin.getWord(hContact, "HistoryLast", 0);
	if (historyLast >= historyMax)
		historyLast = historyMax - 1;

	ptrW str(g_plugin.getWStringA("HistoryStamp"));
	g_plugin.setWString(hContact, BuildSetting(historyLast), ParseString(str ? str : DEFAULT_HISTORYSTAMP, hContact));

	historyLast = (historyLast + 1) % historyMax;
	g_plugin.setWord(hContact, "HistoryLast", historyLast);
	if (historyLast == historyFirst)
		g_plugin.setWord(hContact, "HistoryFirst", (historyFirst + 1) % historyMax);
}

void LoadHistoryList(MCONTACT hContact, HWND hwnd, int nList)
{
	SendDlgItemMessage(hwnd, nList, LB_RESETCONTENT, 0, 0);
	int historyMax = g_plugin.getWord("HistoryMax", 10);
	if (historyMax < 0)
		historyMax = 0;
	else if (historyMax > 99)
		historyMax = 99;
	if (historyMax == 0)
		return;

	int historyFirst = g_plugin.getWord(hContact, "HistoryFirst", 0);
	if (historyFirst >= historyMax)
		historyFirst = 0;

	int historyLast = g_plugin.getWord(hContact, "HistoryLast", 0);
	if (historyLast >= historyMax)
		historyLast = historyMax - 1;

	for (int i = historyLast; i != historyFirst;) {
		i = (i - 1 + historyMax) % historyMax;

		DBVARIANT dbv;
		if (!g_plugin.getWString(hContact, BuildSetting(i), &dbv)) {
			SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, (LPARAM)dbv.pwszVal);
			db_free(&dbv);
		}
		else SendDlgItemMessage(hwnd, nList, LB_ADDSTRING, 0, (LPARAM)L"");
	}
}


HDWP MyResizeWindow(HDWP hDwp, HWND hwndDlg, HWND hwndControl,
	int nHorizontalOffset, int nVerticalOffset,
	int nWidthOffset, int nHeightOffset)
{
	POINT pt;
	RECT rcinit;

	if (nullptr == hwndDlg) /* Wine fix. */
		return hDwp;
	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);

	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	// resize control
	return DeferWindowPos(hDwp, hwndControl, nullptr,
		pt.x + nHorizontalOffset,
		pt.y + nVerticalOffset,
		rcinit.right - rcinit.left + nWidthOffset,
		rcinit.bottom - rcinit.top + nHeightOffset,
		SWP_NOZORDER);


}

HDWP MyHorizCenterWindow(HDWP hDwp, HWND hwndDlg, HWND hwndControl,
	int nClientWidth, int nVerticalOffset,
	int nHeightOffset)
{
	POINT pt;
	RECT rcinit;

	if (nullptr == hwndDlg) /* Wine fix. */
		return hDwp;
	// get current bounding rectangle
	GetWindowRect(hwndControl, &rcinit);

	// get current top left point
	pt.x = rcinit.left;
	pt.y = rcinit.top;
	ScreenToClient(hwndDlg, &pt);

	// resize control
	return DeferWindowPos(hDwp, hwndControl, nullptr,
		(int)((nClientWidth - (rcinit.right - rcinit.left)) / 2),
		pt.y + nVerticalOffset,
		rcinit.right - rcinit.left,
		rcinit.bottom - rcinit.top + nHeightOffset,
		SWP_NOZORDER);

}

void MyResizeGetOffset(HWND hwndControl, int nWidth, int nHeight, int* nDx, int* nDy)
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
	wchar_t sztemp[1024];

	switch (Message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		hContact = (MCONTACT)lparam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam);
		mir_snwprintf(sztemp, L"%s: %s", Clist_GetContactDisplayName(hContact), TranslateT("last seen history"));
		SetWindowText(hwndDlg, sztemp);
		Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_OTHER_MIRANDA);

		if (g_plugin.getByte(hContact, "OnlineAlert"))
			CheckDlgButton(hwndDlg, IDC_STATUSCHANGE, BST_CHECKED);

		SendDlgItemMessage(hwndDlg, IDC_DETAILS, BM_SETIMAGE, IMAGE_ICON, (WPARAM)Skin_LoadIcon(SKINICON_OTHER_USERDETAILS));
		SendDlgItemMessage(hwndDlg, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (WPARAM)Skin_LoadIcon(SKINICON_OTHER_DOWNARROW));
		SendDlgItemMessage(hwndDlg, IDC_SENDMSG, BM_SETIMAGE, IMAGE_ICON, (WPARAM)Skin_LoadIcon(SKINICON_EVENT_MESSAGE));

		//set-up tooltips
		SendDlgItemMessage(hwndDlg, IDC_DETAILS, BUTTONADDTOOLTIP, (WPARAM)TranslateT("View User's Details"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_USERMENU, BUTTONADDTOOLTIP, (WPARAM)TranslateT("User Menu"), BATF_UNICODE);
		SendDlgItemMessage(hwndDlg, IDC_SENDMSG, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Send Instant Message"), BATF_UNICODE);

		Utils_RestoreWindowPositionNoMove(hwndDlg, NULL, MODULENAME, "History_");
		ShowWindow(hwndDlg, SW_SHOW);
		return TRUE;

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lparam);

	case WM_DRAWITEM:
		return Menu_DrawItem(lparam);

	case WM_COMMAND:
		hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (Clist_MenuProcessCommand(LOWORD(wparam), MPCF_CONTACTMENU, hContact))
			break;

		switch (LOWORD(wparam)) {
		case IDCANCEL:
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		case IDOK:
			if (IsDlgButtonChecked(hwndDlg, IDC_STATUSCHANGE) == BST_CHECKED)
				g_plugin.setByte(hContact, "OnlineAlert", 1);
			else
				g_plugin.setByte(hContact, "OnlineAlert", 0);
			SendMessage(hwndDlg, WM_CLOSE, 0, 0);
			break;
		case IDC_USERMENU:
			{
				RECT rc;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rc);
				HMENU hMenu = Menu_BuildContactMenu(hContact);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, nullptr);
				DestroyMenu(hMenu);
			}
			break;
		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
			break;
		case IDC_SENDMSG:
			CallService(MS_MSG_SENDMESSAGE, hContact, 0);
			break;
		case IDC_TEST:
			debug(ParseString(LPGENW("Date: %d.%m.%y(%Y) \n Date desc: %W - %w - %E - %e \n Time: %H:%M:%S (%h-%p) \n user: %n - %u \n Status: %s \n IP: %i - %r"), hContact));
			break;
		}
		break;

	case WM_SIZE:
		int dx, dy;
		{
			HDWP hDwp = BeginDeferWindowPos(6);
			MyResizeGetOffset(GetDlgItem(hwndDlg, IDC_HISTORYLIST), LOWORD(lparam) - 15, HIWORD(lparam) - 99, &dx, &dy);
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
		MINMAXINFO mmi;
		{
			memcpy(&mmi, (LPMINMAXINFO)lparam, sizeof(MINMAXINFO));

			/* The minimum width in points*/
			mmi.ptMinTrackSize.x = 200;
			/* The minimum height in points*/
			mmi.ptMinTrackSize.y = 190;

			memcpy((LPMINMAXINFO)lparam, &mmi, sizeof(MINMAXINFO));
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		WindowList_Remove(hWindowList, hwndDlg);
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "History_");
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
		DestroyIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}

void ShowHistory(MCONTACT hContact, uint8_t isAlert)
{
	HWND hHistoryDlg = WindowList_Find(hWindowList, hContact);
	if (hHistoryDlg == nullptr) {
		hHistoryDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HISTORY), nullptr, HistoryDlgProc, hContact);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		WindowList_Add(hWindowList, hHistoryDlg, hContact);
	}
	else {
		SetForegroundWindow(hHistoryDlg);
		LoadHistoryList(hContact, hHistoryDlg, IDC_HISTORYLIST);
		SetFocus(hHistoryDlg);
	}

	if (isAlert)
		Skin_PlaySound("LastSeenTrackedStatusChange");
}

void InitHistoryDialog(void)
{
	hWindowList = WindowList_Create();
}

void UninitHistoryDialog(void)
{
	WindowList_Destroy(hWindowList);
}
