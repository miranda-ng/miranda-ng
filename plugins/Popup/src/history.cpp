/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

#include "headers.h"

static CRITICAL_SECTION csPopupHistory;
static LIST<POPUPDATA2> arPopupHistory(SETTING_HISTORYSIZE_DEFAULT);
static int popupHistoryBuffer = 0;

static HWND hwndHistory = NULL;

#define UM_RESIZELIST	(WM_USER+100)
#define UM_SELECTLAST	(WM_USER+101)
#define UM_ADDITEM		(WM_USER+102)

static INT_PTR CALLBACK HistoryDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void FreeHistoryItem(POPUPDATA2 *ppd)
{
	mir_free(ppd->lpzTitle);
	mir_free(ppd->lpzText);
	mir_free(ppd->lpzSkin);
	mir_free(ppd);
}

void PopupHistoryResize()
{
	popupHistoryBuffer = PopupOptions.HistorySize;

	mir_cslock lck(csPopupHistory);
	while (arPopupHistory.getCount() > popupHistoryBuffer) {
		FreeHistoryItem(arPopupHistory[0]);
		arPopupHistory.remove(0);
	}
}

void PopupHistoryLoad()
{
	InitializeCriticalSection(&csPopupHistory);
	popupHistoryBuffer = db_get_w(NULL, MODULNAME, "HistorySize", SETTING_HISTORYSIZE_DEFAULT);
}

void PopupHistoryUnload()
{
	for (int i=0; i < arPopupHistory.getCount(); ++i)
		FreeHistoryItem( arPopupHistory[i] );
	arPopupHistory.destroy();

	DeleteCriticalSection(&csPopupHistory);
}

void PopupHistoryAdd(POPUPDATA2 *ppdNew)
{
	if (!PopupOptions.EnableHistory)
		return;

	POPUPDATA2 *ppd = (POPUPDATA2*)mir_alloc( sizeof(POPUPDATA2));
	*ppd = *ppdNew;
	if (ppd->flags & PU2_UNICODE) {
		ppd->lptzTitle = mir_tstrdup(ppd->lpwzTitle);
		ppd->lptzText = mir_tstrdup(ppd->lptzText);
	}
	else {
		ppd->lpzTitle = mir_strdup(ppd->lpzTitle);
		ppd->lpzText = mir_strdup(ppd->lpzText);
	}
	ppd->lpzSkin = mir_strdup(ppd->lpzSkin);
	ppd->dwTimestamp = time(NULL);
	{
		mir_cslock lck(csPopupHistory);
		if (arPopupHistory.getCount() >= popupHistoryBuffer) {
			FreeHistoryItem(arPopupHistory[0]);
			arPopupHistory.remove(0);
		}
		arPopupHistory.insert(ppd);
	}
	if (hwndHistory)
		PostMessage(hwndHistory, UM_ADDITEM, 0, (LPARAM)ppd);
}

void PopupHistoryShow()
{
	if (!PopupOptions.EnableHistory) {
		MessageBox(NULL, TranslateT("Popup History is disabled"), TranslateT("Popup History message"), MB_OK);
		return;
	}

	if (hwndHistory) {
		ShowWindow(hwndHistory, SW_SHOW);
		SetForegroundWindow(hwndHistory);
		SetFocus(hwndHistory);
		SetActiveWindow(hwndHistory);
	}
	else {
		hwndHistory = CreateDialog(hInst, MAKEINTRESOURCE(IDD_HISTORY), NULL, HistoryDlgProc);
		SetWindowText(hwndHistory, TranslateT("Popup History"));
	}
}

static INT_PTR CALLBACK HistoryDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int oldWidth = 0;
	static int loadItem = -1;

	static enum { LOG_NONE, LOG_DEFAULT, LOG_HPP } logType = LOG_NONE;
	static HWND hwndLog = NULL;

	switch (msg) {
	case WM_INITDIALOG:
		{
			oldWidth = 0;
			HWND hwndList = GetDlgItem(hwnd, IDC_POPUP_LIST);
			for (int i=0; i < arPopupHistory.getCount(); ++i)
				ListBox_SetItemData(hwndList, ListBox_AddString(hwndList, _T("")), 0);
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIcon(ICO_HISTORY,0));
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)IcoLib_GetIcon(ICO_HISTORY,1));

			if (gbHppInstalled && PopupOptions.UseHppHistoryLog) {
				logType = LOG_HPP;
				ShowWindow(GetDlgItem(hwnd, IDC_POPUP_LIST), SW_HIDE);

				IEVIEWWINDOW ieWindow;
				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_CREATE;
				ieWindow.dwFlags = 0;
				ieWindow.dwMode = IEWM_MUCC;
				ieWindow.parent = hwnd;
				ieWindow.x = 0;
				ieWindow.y = 0;
				ieWindow.cx = 100;
				ieWindow.cy = 100;
				CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
				hwndLog = ieWindow.hwnd;
				ShowWindow(hwndLog, SW_SHOW);

				RECT rcLst; GetWindowRect(hwndList, &rcLst);
				POINT pt;
				pt.x = rcLst.left;
				pt.y = rcLst.top;
				ScreenToClient(hwnd, &pt);

				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_SETPOS;
				ieWindow.parent = hwnd;
				ieWindow.hwnd = hwndLog;
				ieWindow.x = pt.x;
				ieWindow.y = pt.y;
				ieWindow.cx = rcLst.right-rcLst.left;
				ieWindow.cy = rcLst.bottom-rcLst.top;
				CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);

				IEVIEWEVENTDATA ieData;

				IEVIEWEVENT ieEvent;
				ieEvent.cbSize = sizeof(ieEvent);
				ieEvent.iType = IEE_LOG_MEM_EVENTS;
				ieEvent.dwFlags = 0;
				ieEvent.hwnd = hwndLog;
				ieEvent.eventData = &ieData;
				ieEvent.count = 1;
				ieEvent.codepage = 0;
				ieEvent.pszProto = NULL;

				for (int i=0; i < arPopupHistory.getCount(); ++i) {
					POPUPDATA2* ppd = arPopupHistory[i];
					ieData.cbSize = sizeof(ieData);
					ieData.iType = IEED_EVENT_SYSTEM;
					ieData.dwFlags =  0;
					ieData.color = ppd->colorText;
					if (ppd->flags & PU2_UNICODE) {
						ieData.dwFlags |= IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK;
						ieData.pszNickW = ppd->lptzTitle;
						ieData.pszTextW = ppd->lptzText;
						ieData.pszText2W = NULL;
					}
					else {
						ieData.dwFlags |= 0;
						ieData.pszNick = ppd->lpzTitle;
						ieData.pszText = ppd->lpzText;
						ieData.pszText2 = NULL;
					}
					ieData.bIsMe = FALSE;
					ieData.time = ppd->dwTimestamp;
					ieData.dwData = 0;
					ieData.next = NULL;
					CallService(MS_HPP_EG_EVENT, 0, (WPARAM)&ieEvent);
				}
			}
			else {
				logType = LOG_DEFAULT;
				hwndLog = hwndList;

				ShowWindow(hwndLog, SW_SHOW);
			}

			Utils_RestoreWindowPosition(hwnd, NULL, MODULNAME, "popupHistory_");

			if (logType == LOG_DEFAULT) {
				SendMessage(hwnd, UM_RESIZELIST, 0, 0);
				ListBox_SetTopIndex(hwndLog, arPopupHistory.getCount()-1);
			}
		}
		return TRUE;

	case WM_MEASUREITEM:
		if (logType == LOG_DEFAULT) {
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;
			if (lpmis->itemID == -1)
				return FALSE;
			lpmis->itemHeight = 50;
		}
		return TRUE;

	case WM_DRAWITEM:
		if (logType == LOG_DEFAULT) {
			LPDRAWITEMSTRUCT lpdis;
			lpdis = (LPDRAWITEMSTRUCT) lParam;
			if (lpdis->itemID == -1)
				return FALSE;

			HWND hwndList = GetDlgItem(hwnd, lpdis->CtlID);
			PopupWnd2 *wndPreview = (PopupWnd2 *)ListBox_GetItemData(hwndList, lpdis->itemID);
			if (!wndPreview) {
				RECT rc; GetWindowRect(hwndLog, &rc);

				if (rc.right - rc.left <= 30)
					return FALSE;

				POPUPOPTIONS customOptions = PopupOptions;
				customOptions.DynamicResize = FALSE;
				customOptions.MinimumWidth = customOptions.MaximumWidth = rc.right-rc.left-30;

				POPUPDATA2 *ppd = arPopupHistory[lpdis->itemID];
				wndPreview = new PopupWnd2(ppd, &customOptions, true);
				wndPreview->buildMText();
				wndPreview->update();

				ListBox_SetItemData(hwndLog, lpdis->itemID, wndPreview);
				ListBox_SetItemHeight(hwndLog, lpdis->itemID, wndPreview->getSize().cy+6);
			}

			if (wndPreview) {
				if (lpdis->itemState & ODS_SELECTED) {
					HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
					FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
					DeleteObject(hbr);
				} 
				else {
					HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
					FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
					DeleteObject(hbr);
				}

				int width = wndPreview->getContent()->getWidth();
				int height = wndPreview->getContent()->getHeight();

				BLENDFUNCTION bf;
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.SourceConstantAlpha = 255;
				bf.AlphaFormat = AC_SRC_ALPHA;
				AlphaBlend(lpdis->hDC, lpdis->rcItem.left+5, lpdis->rcItem.top+3, width, height,
					wndPreview->getContent()->getDC(),
					0, 0, width, height, bf);
			}
		}
		return TRUE;

	case WM_DELETEITEM:
		if (logType != LOG_DEFAULT) {
			DELETEITEMSTRUCT *lpdis = (DELETEITEMSTRUCT *)lParam;
			PopupWnd2 *wnd = (PopupWnd2 *)ListBox_GetItemData(lpdis->hwndItem, lpdis->itemID);
			if (wnd)
				delete wnd;
		}
		return TRUE;

	case WM_SIZE:
		{
			RECT rcLst; GetClientRect(hwnd, &rcLst);
			rcLst.left += 10;
			rcLst.top += 10;
			rcLst.right -= 10;
			rcLst.bottom -= 10;
			if (logType == LOG_HPP) {
				SetWindowPos(hwndLog, NULL,
					rcLst.left, rcLst.top, rcLst.right-rcLst.left, rcLst.bottom-rcLst.top,
					SWP_NOZORDER|SWP_DEFERERASE|SWP_SHOWWINDOW);

				IEVIEWWINDOW ieWindow;
				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_SETPOS;
				ieWindow.parent = hwnd;
				ieWindow.hwnd = hwndLog;
				ieWindow.x = rcLst.left;
				ieWindow.y = rcLst.top;
				ieWindow.cx = rcLst.right-rcLst.left;
				ieWindow.cy = rcLst.bottom-rcLst.top;
				CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
			}
			else if (logType == LOG_DEFAULT) {
				SetWindowPos(hwndLog, NULL,
					rcLst.left, rcLst.top, rcLst.right-rcLst.left, rcLst.bottom-rcLst.top,
					SWP_NOZORDER | SWP_DEFERERASE | SWP_SHOWWINDOW);
				if (rcLst.right-rcLst.left != oldWidth) {
					oldWidth = rcLst.right-rcLst.left;
					PostMessage(hwnd, UM_RESIZELIST, 0, 0);
				}
			}
		}
		return TRUE;

	case UM_RESIZELIST:
		if (logType == LOG_DEFAULT) {
			RECT rc; 
			GetWindowRect(GetDlgItem(hwnd, IDC_POPUP_LIST), &rc);
			if (rc.right-rc.left <= 30)
				return FALSE;

			for (int i=0; i < arPopupHistory.getCount(); ++i) {
				PopupWnd2 *wndPreview = (PopupWnd2 *)ListBox_GetItemData(hwndLog, i);
				if (wndPreview)
					delete wndPreview;

				ListBox_SetItemData(hwndLog, i, 0);
				ListBox_SetItemHeight(hwndLog, i, 50);
			}
			ScrollWindow(hwndLog, 0, 100000, NULL, NULL);
			InvalidateRect(hwndLog, NULL, TRUE);
		}
		return TRUE;

	case UM_ADDITEM:
		if (logType == LOG_HPP) {
			POPUPDATA2 *ppd = (POPUPDATA2 *)lParam;

			IEVIEWEVENTDATA ieData;

			IEVIEWEVENT ieEvent;
			ieEvent.cbSize = sizeof(ieEvent);
			ieEvent.iType = IEE_LOG_MEM_EVENTS;
			ieEvent.dwFlags = 0;
			ieEvent.hwnd = hwndLog;
			ieEvent.eventData = &ieData;
			ieEvent.count = 1;
			ieEvent.codepage = 0;
			ieEvent.pszProto = NULL;

			ieData.cbSize = sizeof(ieData);
			ieData.dwFlags = 0;
			ieData.iType = IEED_EVENT_SYSTEM;
			ieData.color = ppd->colorText;
			if (ppd->flags & PU2_UNICODE) {
				ieData.dwFlags |= IEEDF_UNICODE_TEXT | IEEDF_UNICODE_NICK;
				ieData.pszNickW = ppd->lptzTitle;
				ieData.pszTextW = ppd->lptzText;
				ieData.pszText2W = NULL;
			}
			else {
				ieData.dwFlags |= 0;
				ieData.pszNick = ppd->lpzTitle;
				ieData.pszText = ppd->lpzText;
				ieData.pszText2 = NULL;
			}
			ieData.bIsMe = FALSE;
			ieData.time = ppd->dwTimestamp;
			ieData.dwData = 0;
			ieData.next = NULL;
			CallService(MS_HPP_EG_EVENT, 0, (WPARAM)&ieEvent);
		}
		else if (logType == LOG_DEFAULT) {
			if (arPopupHistory.getCount() <= ListBox_GetCount(hwndLog)) {
				loadItem = 0;
				PostMessage(hwnd, UM_RESIZELIST, 0, 0);
				return TRUE;
			}
			ListBox_SetItemData(hwndLog, ListBox_AddString(hwndLog, _T("")), 0);
		}
		return TRUE;

	case WM_CLOSE:
		Utils_SaveWindowPosition(hwnd, NULL, MODULNAME, "popupHistory_");
		DestroyWindow(hwnd);
		hwndHistory = NULL;
		return TRUE;

	case WM_DESTROY:
		if (logType == LOG_HPP) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.dwFlags = 0;
			ieWindow.dwMode = IEWM_TABSRMM;
			ieWindow.parent = hwnd;
			ieWindow.hwnd = hwndLog;
			CallService(MS_HPP_EG_WINDOW, 0, (LPARAM)&ieWindow);
		}
	}
	return FALSE; // DefWindowProc(hwnd, msg, wParam, lParam);
}
