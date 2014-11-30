/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

#define SB_CHAR_WIDTH		 40
#define SB_SENDING_WIDTH 	 25
#define SB_UNICODE_WIDTH 	 18

#define TIMEOUT_FLASHWND     900

static  void DrawTab(ParentWindowData *dat, HWND hwnd, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////

extern TCHAR *GetNickname(MCONTACT hContact, const char* szProto);

static const TCHAR *titleTokenNames[] = {_T("%name%"), _T("%status%"), _T("%statusmsg%"), _T("%account%")};

TCHAR* GetWindowTitle(MCONTACT hContact, const char *szProto)
{
	int isTemplate;
	int i, j, len;
	TCHAR* tokens[4] = {0};
	int tokenLen[4] = {0};
	TCHAR *p, *tmplt, *title;
	char *accModule;
	TCHAR *pszNewTitleEnd = mir_tstrdup(TranslateT("Message Session"));
	isTemplate = 0;
	if (hContact && szProto) {
		tokens[0] = GetNickname(hContact, szProto);
		tokenLen[0] = mir_tstrlen(tokens[0]);
		tokens[1] = mir_tstrdup(pcli->pfnGetStatusModeDescription(szProto ? db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) : ID_STATUS_OFFLINE, 0));
		tokenLen[1] = mir_tstrlen(tokens[1]);
		tokens[2] = db_get_tsa(hContact, "CList", "StatusMsg");
		if (tokens[2] != NULL) {
			tokenLen[2] = mir_tstrlen(tokens[2]);
			for (i = j = 0; i < tokenLen[2]; i++) {
				if (tokens[2][i] == '\r')
					continue;
				if (tokens[2][i] == '\n')
					tokens[2][j++] = ' ';
				else
					tokens[2][j++] = tokens[2][i];
			}
			tokens[2][j] = '\0';
			tokenLen[2] = j;
		}

		accModule = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, hContact, 0);
		if (accModule != NULL) {
			PROTOACCOUNT* proto = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)accModule);
			if (proto != NULL) {
				tokens[3] = mir_tstrdup(proto->tszAccountName);
				tokenLen[3] = mir_tstrlen(tokens[3]);
			}
		}
		tmplt = db_get_tsa(NULL, SRMMMOD, SRMSGSET_WINDOWTITLE);
		if (tmplt != NULL)
			isTemplate = 1;
		else {
			if (g_dat.flags & SMF_STATUSICON)
				tmplt = _T("%name% - ");
			else
				tmplt = _T("%name% (%status%) : ");
		}
	}
	else tmplt = _T("");

	for (len = 0, p = tmplt; *p; p++, len++) {
		if (*p == '%') {
			for (i = 0; i < SIZEOF(titleTokenNames); i++) {
				int tnlen = (int)_tcslen(titleTokenNames[i]);
				if (!_tcsncmp(p, titleTokenNames[i], tnlen)) {
					len += tokenLen[i] - 1;
					p += tnlen - 1;
					break;
				}
			}
		}
	}
	if (!isTemplate)
		len += mir_tstrlen(pszNewTitleEnd);

	title = (TCHAR *)mir_alloc(sizeof(TCHAR) * (len + 1));
	for (len = 0, p = tmplt; *p; p++) {
		if (*p == '%') {
			for (i = 0; i < SIZEOF(titleTokenNames); i ++) {
				int tnlen = mir_tstrlen(titleTokenNames[i]);
				if (!_tcsncmp(p, titleTokenNames[i], tnlen)) {
					if (tokens[i] != NULL) {
						memcpy(title+len, tokens[i], sizeof(TCHAR) * tokenLen[i]);
						len += tokenLen[i];
					}
					p += tnlen - 1;
					break;
				}
			}
			if (i < SIZEOF(titleTokenNames)) continue;
		}
		title[len++] = *p;
	}
	if (!isTemplate) {
		memcpy(title+len, pszNewTitleEnd, sizeof(TCHAR) * mir_tstrlen(pszNewTitleEnd));
		len += mir_tstrlen(pszNewTitleEnd);
	}
	title[len] = '\0';
	if (isTemplate)
		mir_free(tmplt);

	for (i = 0; i < SIZEOF(titleTokenNames); i++)
		mir_free(tokens[i]);

	mir_free(pszNewTitleEnd);
	return title;
}

TCHAR* GetTabName(MCONTACT hContact)
{
	if (hContact)
		return GetNickname(hContact, NULL);

	return NULL;
}

static int GetChildCount(ParentWindowData *dat)
{
	return TabCtrl_GetItemCount(dat->hwndTabs);
}

static void GetChildWindowRect(ParentWindowData *dat, RECT *rcChild)
{
	RECT rc, rcStatus, rcTabs;
	GetClientRect(dat->hwnd, &rc);
	GetClientRect(dat->hwndTabs, &rcTabs);
	TabCtrl_AdjustRect(dat->hwndTabs, FALSE, &rcTabs);
	rcStatus.top = rcStatus.bottom = 0;
	if (dat->flags2 & SMF2_SHOWSTATUSBAR)
		GetWindowRect(dat->hwndStatus, &rcStatus);

	rcChild->left = 0;
	rcChild->right = rc.right;
	if (dat->flags2 & SMF2_TABSATBOTTOM) {
		rcChild->top = 2;
		if ((dat->flags2 & SMF2_USETABS && !(dat->flags2 & SMF2_HIDEONETAB)) || (dat->childrenCount > 1))
			rcChild->bottom = rcTabs.bottom + 4;
		else
			rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
	}
	else {
		if ((dat->flags2 & SMF2_USETABS && !(dat->flags2 & SMF2_HIDEONETAB)) || (dat->childrenCount > 1))
			rcChild->top = rcTabs.top;
		else
			rcChild->top = 2;

		rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
	}
}

static int GetTabFromHWND(ParentWindowData *dat, HWND child)
{
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (int i = 0; i < l; i++) {
		TCITEM tci = { 0 };
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		MessageWindowTabData *mwtd = (MessageWindowTabData *)tci.lParam;
		if (mwtd->hwnd == child)
			return i;
	}
	return -1;
}

static MessageWindowTabData * GetChildFromTab(HWND hwndTabs, int tabId)
{
	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM;
	if (TabCtrl_GetItem(hwndTabs, tabId, &tci))
		return (MessageWindowTabData *) tci.lParam;

	return NULL;
}

static MessageWindowTabData * GetChildFromHWND(ParentWindowData *dat, HWND hwnd)
{
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (int i = 0; i < l; i++) {
		TCITEM tci = { 0 };
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		MessageWindowTabData *mwtd = (MessageWindowTabData *)tci.lParam;
		if (mwtd->hwnd == hwnd)
			return mwtd;
	}
	return NULL;
}

static void GetMinimunWindowSize(ParentWindowData *dat, SIZE *size)
{
	MINMAXINFO mmi;
	RECT rc, rcWindow;
	int minW = 216, minH = 80;
	GetWindowRect(dat->hwnd, &rcWindow);
	GetChildWindowRect(dat, &rc);
	for (int i = 0; i<dat->childrenCount; i++) {
		MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
		SendMessage(mwtd->hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
		if (i == 0 || mmi.ptMinTrackSize.x > minW) minW = mmi.ptMinTrackSize.x;
		if (i == 0 || mmi.ptMinTrackSize.y > minH) minH = mmi.ptMinTrackSize.y;
	}
	if (dat->bMinimized) {
		size->cx = minW;
		size->cy = minH;
	}
	else {
		size->cx = minW + (rcWindow.right - rcWindow.left) - (rc.right - rc.left);
		size->cy = minH + (rcWindow.bottom - rcWindow.top) - (rc.bottom - rc.top);
	}
}

static void SetupStatusBar(ParentWindowData *dat)
{
	int statusIconNum = GetStatusIconsCount(dat->hContact);
	int statwidths[4];
	RECT rc;
	GetClientRect(dat->hwnd, &rc);
	statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_UNICODE_WIDTH - 2 * (statusIconNum > 0) - statusIconNum * (GetSystemMetrics(SM_CXSMICON) + 2);
	statwidths[1] = rc.right - rc.left - SB_UNICODE_WIDTH - 2 * (statusIconNum > 0) - statusIconNum * (GetSystemMetrics(SM_CXSMICON) + 2);
	statwidths[2] = rc.right - rc.left - SB_UNICODE_WIDTH;
	statwidths[3] = -1;
	SendMessage(dat->hwndStatus, SB_SETPARTS, 4, (LPARAM)statwidths);
	SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
	SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_NOBORDERS) | 3, 0);
}

static int AddOrReplaceIcon(HIMAGELIST hList, int prevIndex, HICON hIcon)
{
	int usageIdx = -1;
	for (int i = 0; i < g_dat.tabIconListUsageSize; i++) {
		if (!g_dat.tabIconListUsage[i].used && usageIdx == -1)
			usageIdx = i;

		if (g_dat.tabIconListUsage[i].index == prevIndex) {
			usageIdx = i;
			break;
		}
	}
	if (usageIdx == -1) {
		usageIdx = g_dat.tabIconListUsageSize;
		g_dat.tabIconListUsage = (ImageListUsageEntry*)mir_realloc(g_dat.tabIconListUsage, sizeof(ImageListUsageEntry)* (g_dat.tabIconListUsageSize + 1));
		g_dat.tabIconListUsageSize++;
	}
	else prevIndex = g_dat.tabIconListUsage[usageIdx].index;

	g_dat.tabIconListUsage[usageIdx].used = 1;
	g_dat.tabIconListUsage[usageIdx].index = (int)ImageList_ReplaceIcon(hList, prevIndex, hIcon);
	return g_dat.tabIconListUsage[usageIdx].index;
}

static void ReleaseIcon(int index)
{
	for (int i = 0; i < g_dat.tabIconListUsageSize; i++)
		if (g_dat.tabIconListUsage[i].index == index)
			g_dat.tabIconListUsage[i].used = 0;
}

static void ActivateChild(ParentWindowData *dat, HWND child)
{
	RECT rcChild;
	GetChildWindowRect(dat, &rcChild);
	SetWindowPos(child, HWND_TOP, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, SWP_NOSIZE);

	int i = GetTabFromHWND(dat, child);
	if (i == -1)
		return;

	MessageWindowTabData *mwtd;
	if ((mwtd = GetChildFromTab(dat->hwndTabs, i)) == NULL)
		return;

	dat->hContact = mwtd->hContact;
	if (child != dat->hwndActive) {
		HWND prev = dat->hwndActive;
		dat->hwndActive = child;
		SetupStatusBar(dat);
		SendMessage(dat->hwndActive, DM_UPDATESTATUSBAR, 0, 0);
		SendMessage(dat->hwndActive, DM_UPDATETITLEBAR, 0, 0);
		SendMessage(dat->hwnd, WM_SIZE, 0, 0);
		ShowWindow(dat->hwndActive, SW_SHOWNOACTIVATE);
		SendMessage(dat->hwndActive, DM_SCROLLLOGTOBOTTOM, 0, 0);
		if (prev != NULL)
			ShowWindow(prev, SW_HIDE);
	}
	else SendMessage(dat->hwnd, WM_SIZE, 0, 0);

	TabCtrl_SetCurSel(dat->hwndTabs, i);
	SendMessage(dat->hwndActive, DM_ACTIVATE, WA_ACTIVE, 0);
}

static void AddChild(ParentWindowData *dat, HWND hwnd, MCONTACT hContact)
{
	MessageWindowTabData *mwtd = (MessageWindowTabData *)mir_alloc(sizeof(MessageWindowTabData));
	mwtd->hwnd = hwnd;
	mwtd->hContact = hContact;
	mwtd->szProto = GetContactProto(hContact);
	mwtd->parent = dat;

	dat->childrenCount++;

	TCITEM tci;
	tci.mask = TCIF_PARAM | TCIF_IMAGE | TCIF_TEXT;
	tci.lParam = (LPARAM)mwtd;
	tci.iImage = -1;
	tci.pszText = _T("");
	int tabId = TabCtrl_InsertItem(dat->hwndTabs, dat->childrenCount - 1, &tci);
	SetWindowPos(mwtd->hwnd, HWND_TOP, dat->childRect.left, dat->childRect.top, dat->childRect.right - dat->childRect.left, dat->childRect.bottom - dat->childRect.top, SWP_HIDEWINDOW);
	SendMessage(dat->hwnd, WM_SIZE, 0, 0);

	EnableThemeDialogTexture(hwnd, ETDT_ENABLETAB);
}

static void RemoveChild(ParentWindowData *dat, HWND child)
{
	int tab = GetTabFromHWND(dat, child);
	if (tab >= 0) {
		TCITEM tci;
		tci.mask = TCIF_PARAM | TCIF_IMAGE;
		TabCtrl_GetItem(dat->hwndTabs, tab, &tci);
		TabCtrl_DeleteItem(dat->hwndTabs, tab);
		mir_free((MessageWindowTabData *)tci.lParam);
		dat->childrenCount--;
		if (child == dat->hwndActive) {
			if (tab == TabCtrl_GetItemCount(dat->hwndTabs)) tab--;
			if (tab >= 0)
				ActivateChild(dat, GetChildFromTab(dat->hwndTabs, tab)->hwnd);
			else
				dat->hwndActive = NULL;
		}
		ReleaseIcon(tci.iImage);
	}
}

static void CloseOtherChilden(ParentWindowData *dat, HWND child)
{
	ActivateChild(dat, child);
	for (int i = dat->childrenCount - 1; i >= 0; i--) {
		MessageWindowTabData *mwtd = GetChildFromTab(dat->hwndTabs, i);
		if (mwtd != NULL && mwtd->hwnd != child)
			SendMessage(mwtd->hwnd, WM_CLOSE, 0, 0);
	}
	ActivateChild(dat, child);
}

static void ActivateNextChild(ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i+1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
}

static void ActivatePrevChild(ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i+l-1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->hwnd);
}

static void ActivateChildByIndex(ParentWindowData *dat, int index)
{
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	if (index < l) {
		MessageWindowTabData *mwtd = GetChildFromTab(dat->hwndTabs, index);
		if (mwtd != NULL)
			ActivateChild(dat, mwtd->hwnd);
	}
}

static void SetContainerWindowStyle(ParentWindowData *dat)
{
	ShowWindow(dat->hwndStatus, (dat->flags2 & SMF2_SHOWSTATUSBAR) ? SW_SHOW : SW_HIDE);

	DWORD ws = GetWindowLongPtr(dat->hwnd, GWL_STYLE) & ~(WS_CAPTION);
	if (dat->flags2 & SMF2_SHOWTITLEBAR)
		ws |= WS_CAPTION;

	SetWindowLongPtr(dat->hwnd, GWL_STYLE, ws);

	ws = GetWindowLongPtr(dat->hwnd, GWL_EXSTYLE)& ~WS_EX_LAYERED;
	ws |= dat->flags2 & SMF2_USETRANSPARENCY ? WS_EX_LAYERED : 0;
	SetWindowLongPtr(dat->hwnd, GWL_EXSTYLE, ws);
	if (dat->flags2 & SMF2_USETRANSPARENCY)
		SetLayeredWindowAttributes(dat->hwnd, RGB(255, 255, 255), (BYTE)(255 - g_dat.inactiveAlpha), LWA_ALPHA);

	ws = GetWindowLongPtr(dat->hwndTabs, GWL_STYLE) & ~(TCS_BOTTOM | 0x2000);
	if (dat->flags2 & SMF2_TABSATBOTTOM)
		ws |= TCS_BOTTOM;

	ws |= 0x2000;
	if (dat->flags2 & SMF2_TABCLOSEBUTTON)
		TabCtrl_SetPadding(dat->hwndTabs, GetSystemMetrics(SM_CXEDGE) + 12, GetSystemMetrics(SM_CYEDGE) + 1);
	else
		TabCtrl_SetPadding(dat->hwndTabs, GetSystemMetrics(SM_CXEDGE) + 4, GetSystemMetrics(SM_CYEDGE) + 1);

	SetWindowLongPtr(dat->hwndTabs, GWL_STYLE, ws);

	RECT rc;
	GetWindowRect(dat->hwnd, &rc);
	SetWindowPos(dat->hwnd, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TabCtrlData *dat = (TabCtrlData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	TCHITTESTINFO thinfo;
	int tabId;

	switch (msg) {
	case EM_SUBCLASSED:
		dat = (TabCtrlData*)mir_alloc(sizeof(TabCtrlData));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		dat->bDragging = FALSE;
		dat->bDragged = FALSE;
		dat->srcTab = -1;
		dat->destTab = -1;
		return 0;

	case WM_MBUTTONDOWN:
		thinfo.pt.x = LOWORD(lParam);
		thinfo.pt.y = HIWORD(lParam);
		tabId = TabCtrl_HitTest(hwnd, &thinfo);
		if (tabId >= 0) {
			TCITEM tci;
			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwnd, tabId, &tci);
			MessageWindowTabData *mwtd = (MessageWindowTabData *)tci.lParam;
			if (mwtd != NULL) {
				SendMessage(mwtd->hwnd, WM_CLOSE, 0, 0);
				dat->srcTab = -1;
			}
		}
		return 0;

	case WM_LBUTTONDBLCLK:
		thinfo.pt.x = LOWORD(lParam);
		thinfo.pt.y = HIWORD(lParam);
		tabId = TabCtrl_HitTest(hwnd, &thinfo);
		if (tabId >= 0 && tabId == dat->srcTab) {
			SendMessage(GetChildFromTab(hwnd, tabId)->hwnd, WM_CLOSE, 0, 0);
			dat->srcTab = -1;
		}
		dat->destTab = -1;
		break;

	case WM_LBUTTONDOWN:
		if (!dat->bDragging) {
			thinfo.pt.x = LOWORD(lParam);
			thinfo.pt.y = HIWORD(lParam);
			dat->srcTab = TabCtrl_HitTest(hwnd, &thinfo);

			FILETIME ft;
			GetSystemTimeAsFileTime(&ft);
			if (dat->srcTab >= 0) {
				dat->bDragging = TRUE;
				dat->bDragged = FALSE;
				dat->clickLParam = lParam;
				dat->clickWParam = wParam;
				dat->lastClickTime = ft.dwLowDateTime;
				dat->mouseLBDownPos.x = thinfo.pt.x;
				dat->mouseLBDownPos.y = thinfo.pt.y;
				SetCapture(hwnd);
			}
			return 0;
		}
		break;

	case WM_CAPTURECHANGED:
	case WM_LBUTTONUP:
		if (dat->bDragging) {
			thinfo.pt.x = LOWORD(lParam);
			thinfo.pt.y = HIWORD(lParam);
			if (dat->bDragged) {
				ImageList_DragLeave(GetDesktopWindow());
				ImageList_EndDrag();
				ImageList_Destroy(dat->hDragImageList);
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				dat->destTab = TabCtrl_HitTest(hwnd, &thinfo);
				if (thinfo.flags != TCHT_NOWHERE && dat->destTab != dat->srcTab) {
					NMHDR nmh;
					TCHAR  sBuffer[501];
					TCITEM item;
					int curSel;
					curSel = TabCtrl_GetCurSel(hwnd);
					item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
					item.pszText = sBuffer;
					item.cchTextMax = SIZEOF(sBuffer);
					TabCtrl_GetItem(hwnd, dat->srcTab, &item);
					sBuffer[SIZEOF(sBuffer) - 1] = '\0';

					if (curSel == dat->srcTab)
						curSel = dat->destTab;
					else if (curSel > dat->srcTab && curSel <= dat->destTab)
						curSel--;
					else if (curSel < dat->srcTab && curSel >= dat->destTab)
						curSel++;

					TabCtrl_DeleteItem(hwnd, dat->srcTab);
					TabCtrl_InsertItem(hwnd, dat->destTab, &item);
					TabCtrl_SetCurSel(hwnd, curSel);
					dat->destTab = -1;
					nmh.hwndFrom = hwnd;
					nmh.idFrom = GetDlgCtrlID(hwnd);
					nmh.code = TCN_SELCHANGE;
					SendMessage(GetParent(hwnd), WM_NOTIFY, nmh.idFrom, (LPARAM)&nmh);
					UpdateWindow(hwnd);
				}
				else if (thinfo.flags == TCHT_NOWHERE) {
					TCITEM tci;
					POINT pt;
					NewMessageWindowLParam newData = { 0 };
					dat->destTab = -1;
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, dat->srcTab, &tci);
					MessageWindowTabData *mwtd = (MessageWindowTabData *)tci.lParam;
					if (mwtd != NULL) {
						HWND hChild = mwtd->hwnd;
						MCONTACT hContact = mwtd->hContact;
						GetCursorPos(&pt);
						HWND hParent = WindowFromPoint(pt);
						while (GetParent(hParent) != NULL)
							hParent = GetParent(hParent);

						hParent = WindowList_Find(g_dat.hParentWindowList, (MCONTACT)hParent);
						if ((hParent != NULL && hParent != GetParent(hwnd)) || (hParent == NULL && mwtd->parent->childrenCount > 1 && (GetKeyState(VK_CONTROL) & 0x8000))) {
							if (hParent == NULL) {
								RECT rc, rcDesktop;
								newData.hContact = hContact;
								hParent = (HWND)CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGWIN), NULL, DlgProcParentWindow, (LPARAM)&newData);
								GetWindowRect(hParent, &rc);
								rc.right = (rc.right - rc.left);
								rc.bottom = (rc.bottom - rc.top);
								rc.left = pt.x - rc.right / 2;
								rc.top = pt.y - rc.bottom / 2;
								HMONITOR hMonitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
								MONITORINFO mi;
								mi.cbSize = sizeof(mi);
								GetMonitorInfo(hMonitor, &mi);
								rcDesktop = mi.rcWork;
								if (rc.left < rcDesktop.left)
									rc.left = rcDesktop.left;
								if (rc.top < rcDesktop.top)
									rc.top = rcDesktop.top;
								MoveWindow(hParent, rc.left, rc.top, rc.right, rc.bottom, FALSE);
							}
							NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_CLOSING);
							NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_CLOSE);
							SetParent(hChild, hParent);
							SendMessage(GetParent(hwnd), CM_REMOVECHILD, 0, (LPARAM)hChild);
							SendMessage(hChild, DM_SETPARENT, 0, (LPARAM)hParent);
							SendMessage(hParent, CM_ADDCHILD, (WPARAM)hChild, hContact);
							SendMessage(hChild, DM_UPDATETABCONTROL, 0, 0);
							SendMessage(hParent, CM_ACTIVATECHILD, 0, (LPARAM)hChild);
							NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_OPENING);
							NotifyLocalWinEvent(hContact, hChild, MSG_WINDOW_EVT_OPEN);
							ShowWindow(hParent, SW_SHOWNA);
							EnableWindow(hParent, TRUE);
						}
					}
				}
				else {
					dat->destTab = -1;
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				}
			}
			else if (dat->srcTab >= 0 && g_dat.flags2 & SMF2_TABCLOSEBUTTON) {
				IMAGEINFO info;
				POINT pt;
				RECT rect;
				int atTop = (GetWindowLongPtr(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;
				TabCtrl_GetItemRect(hwnd, dat->srcTab, &rect);
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				ImageList_GetImageInfo(g_dat.hButtonIconList, 0, &info);
				rect.left = rect.right - (info.rcImage.right - info.rcImage.left) - 6;
				if (!atTop)
					rect.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top);

				if (pt.x >= rect.left && pt.x < rect.left + (info.rcImage.right - info.rcImage.left) && pt.y >= rect.top && pt.y < rect.top + (info.rcImage.bottom - info.rcImage.top)) {
					HBITMAP hOldBitmap, hBmp;
					HDC hdc = GetDC(NULL);
					HDC hdcMem = CreateCompatibleDC(hdc);
					pt.x -= rect.left;
					pt.y -= rect.top;
					hBmp = CreateCompatibleBitmap(hdc, info.rcImage.right - info.rcImage.left + 1, info.rcImage.bottom - info.rcImage.top + 1);
					hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBmp);
					SetPixel(hdcMem, pt.x, pt.y, 0x000000);
					ImageList_DrawEx(g_dat.hButtonIconList, 0, hdcMem, 0, 0, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
					COLORREF color1 = GetPixel(hdcMem, pt.x, pt.y);
					SetPixel(hdcMem, pt.x, pt.y, 0xFFFFFF);
					ImageList_DrawEx(g_dat.hButtonIconList, 0, hdcMem, 0, 0, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
					COLORREF color2 = GetPixel(hdcMem, pt.x, pt.y);
					SelectObject(hdcMem, hOldBitmap);
					DeleteDC(hdcMem);
					DeleteObject(hBmp);
					ReleaseDC(NULL, hdc);
					if (color1 != 0x000000 || color2 != 0xFFFFFF) {
						SendMessage(GetChildFromTab(hwnd, dat->srcTab)->hwnd, WM_CLOSE, 0, 0);
						dat->srcTab = -1;
					}
				}
				else SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);
			}
			else SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);

			dat->bDragged = FALSE;
			dat->bDragging = FALSE;
			dat->destTab = -1;
			ReleaseCapture();
		}
		break;

	case WM_MOUSEMOVE:
		if (wParam & MK_LBUTTON) {
			if (dat->bDragging) {
				FILETIME ft;
				GetSystemTimeAsFileTime(&ft);
				thinfo.pt.x = LOWORD(lParam);
				thinfo.pt.y = HIWORD(lParam);
				if (!dat->bDragged) {
					if ((abs(thinfo.pt.x - dat->mouseLBDownPos.x) < 3 && abs(thinfo.pt.y - dat->mouseLBDownPos.y) < 3)
						|| (ft.dwLowDateTime - dat->lastClickTime) < 10 * 1000 * 150)
						break;
				}
				if (!dat->bDragged) {
					POINT pt;
					RECT rect;
					RECT rect2;
					HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 254));
					GetCursorPos(&pt);
					TabCtrl_GetItemRect(hwnd, dat->srcTab, &rect);
					rect.right -= rect.left - 1;
					rect.bottom -= rect.top - 1;
					rect2.left = 0; rect2.right = rect.right; rect2.top = 0; rect2.bottom = rect.bottom;
					dat->hDragImageList = ImageList_Create(rect.right, rect.bottom, ILC_COLOR | ILC_MASK, 0, 1);
					HDC hDC = GetDC(hwnd);
					HDC hMemDC = CreateCompatibleDC(hDC);
					HBITMAP hBitmap = CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
					HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
					FillRect(hMemDC, &rect2, hBrush);
					SetWindowOrgEx(hMemDC, rect.left, rect.top, NULL);
					SendMessage(hwnd, WM_PRINTCLIENT, (WPARAM)hMemDC, PRF_CLIENT);
					SelectObject(hMemDC, hOldBitmap);
					ImageList_AddMasked(dat->hDragImageList, hBitmap, RGB(255, 0, 254));
					DeleteObject(hBitmap);
					DeleteObject(hBrush);
					ReleaseDC(hwnd, hDC);
					DeleteDC(hMemDC);
					ImageList_BeginDrag(dat->hDragImageList, 0, dat->mouseLBDownPos.x - rect.left, dat->mouseLBDownPos.y - rect.top);
					ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
					SetCursor(hDragCursor);
					dat->mouseLBDownPos.x = thinfo.pt.x;
					dat->mouseLBDownPos.y = thinfo.pt.y;
				}
				else {
					POINT pt;
					GetCursorPos(&pt);
					thinfo.pt = pt;
					ScreenToClient(hwnd, &thinfo.pt);
					int newDest = TabCtrl_HitTest(hwnd, &thinfo);
					if (thinfo.flags == TCHT_NOWHERE)
						newDest = -1;

					if (newDest != dat->destTab) {
						dat->destTab = newDest;
						ImageList_DragLeave(GetDesktopWindow());
						RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
						ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);
					}
					else ImageList_DragMove(pt.x, pt.y);
				}
				dat->bDragged = TRUE;
				return 0;
			}
		}
		break;

	case EM_UNSUBCLASSED:
		mir_free(dat);
		return 0;
	}
	return mir_callNextSubclass(hwnd, TabCtrlProc, msg, wParam, lParam);
}

__forceinline void SubclassTabCtrl(HWND hwnd)
{
	mir_subclassWindow(hwnd, TabCtrlProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

__forceinline void UnsubclassTabCtrl(HWND hwnd)
{
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcParentWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DWORD ws;
	ParentWindowData *dat;
	dat = (ParentWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!dat && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		{
			HMENU hMenu;
			int savePerContact = db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *) lParam;
			dat = (ParentWindowData *) mir_alloc(sizeof(ParentWindowData));
			dat->hContact = newData->hContact;
			dat->nFlash = 0;
			dat->nFlashMax = db_get_b(NULL, SRMMMOD, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
			dat->childrenCount = 0;
			dat->hwnd = hwndDlg;
			dat->mouseLBDown = 0;
			dat->windowWasCascaded = 0;
			dat->bMinimized = 0;
			dat->bVMaximized = 0;
			dat->flags2 = g_dat.flags2;
			dat->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);
			dat->isChat = newData->isChat;
			SendMessage(dat->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
			//SetupStatusBar(dat);
			dat->hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
			dat->hwndActive = NULL;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dat);
			if (g_dat.hTabIconList != NULL)
				TabCtrl_SetImageList(dat->hwndTabs, g_dat.hTabIconList);

			dat->next = NULL;
			if (!newData->isChat) {
				dat->prev = g_dat.lastParent;
				g_dat.lastParent = dat;
			}
			else {
				dat->prev = g_dat.lastChatParent;
				g_dat.lastChatParent = dat;
			}
			if (dat->prev != NULL)
				dat->prev->next = dat;

			WindowList_Add(g_dat.hParentWindowList, hwndDlg, (MCONTACT)hwndDlg);
			SubclassTabCtrl(dat->hwndTabs);

			SetContainerWindowStyle(dat);

			MCONTACT hSContact = savePerContact ? dat->hContact : NULL;
			dat->bTopmost = db_get_b(hSContact, SRMMMOD, SRMSGSET_TOPMOST, SRMSGDEFSET_TOPMOST);
			if (ScriverRestoreWindowPosition(hwndDlg, hSContact, SRMMMOD, (newData->isChat && !savePerContact) ? "chat" : "", 0, SW_HIDE))
				SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);

			if (!savePerContact && db_get_b(NULL, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE))
				WindowList_Broadcast(g_dat.hParentWindowList, DM_CASCADENEWWINDOW, (WPARAM) hwndDlg, (LPARAM)&dat->windowWasCascaded);

			hMenu = GetSystemMenu( hwndDlg, FALSE );
			InsertMenu( hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
			if (dat->bTopmost) {
				InsertMenu( hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_CHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always On Top"));
				SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else InsertMenu( hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_UNCHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always On Top"));
		}
		return TRUE;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			if (dat->bVMaximized) {
				MONITORINFO mi;
				HMONITOR hMonitor;
				WINDOWPLACEMENT wp;
				RECT rcDesktop;
				wp.length = sizeof(wp);
				GetWindowPlacement(hwndDlg, &wp);
				hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
				mi.cbSize = sizeof(mi);
				GetMonitorInfo(hMonitor, &mi);
				rcDesktop = mi.rcWork;

				mmi->ptMaxSize.x = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
				mmi->ptMaxSize.y = rcDesktop.bottom - rcDesktop.top;
				mmi->ptMaxPosition.x = wp.rcNormalPosition.left;
				if (IsIconic(hwndDlg))
					mmi->ptMaxPosition.y = rcDesktop.top;
				else
					mmi->ptMaxPosition.y = 0;
			}
			SIZE size;
			GetMinimunWindowSize(dat, &size);
			mmi->ptMinTrackSize.x = size.cx;
			mmi->ptMinTrackSize.y = size.cy;
		}
		return FALSE;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			dat->bMinimized = 1;

		if ( IsIconic(hwndDlg))
			MoveWindow(dat->hwndActive, dat->childRect.left, dat->childRect.top, dat->childRect.right-dat->childRect.left, dat->childRect.bottom - dat->childRect.top, TRUE);
		else {
			RECT rc, rcStatus, rcChild, rcWindow;
			SIZE size;
			dat->bMinimized = 0;
			GetClientRect(hwndDlg, &rc);
			GetWindowRect(hwndDlg, &rcWindow);
			rcStatus.top = rcStatus.bottom = 0;
			if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
				GetWindowRect(dat->hwndStatus, &rcStatus);
				SetupStatusBar(dat);
			}
			MoveWindow(dat->hwndTabs, 0, 2, (rc.right - rc.left), (rc.bottom - rc.top) - (rcStatus.bottom - rcStatus.top) - 2,	FALSE);
			RedrawWindow(dat->hwndTabs, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE);
			GetMinimunWindowSize(dat, &size);
			if ((rcWindow.bottom-rcWindow.top) < size.cy || (rcWindow.right-rcWindow.left) < size.cx) {
				if ((rcWindow.bottom-rcWindow.top) < size.cy)
					rcWindow.bottom = rcWindow.top + size.cy;
				if ((rcWindow.right-rcWindow.left) < size.cx)
					rcWindow.right = rcWindow.left + size.cx;
				MoveWindow(hwndDlg, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top,	TRUE);
			}
			GetChildWindowRect(dat, &rcChild);
			memcpy(&dat->childRect, &rcChild, sizeof(RECT));
			MoveWindow(dat->hwndActive, rcChild.left, rcChild.top, rcChild.right-rcChild.left, rcChild.bottom - rcChild.top, TRUE);
			RedrawWindow(GetDlgItem(dat->hwndActive, IDC_LOG), NULL, NULL, RDW_INVALIDATE);
			if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
				SendMessage(dat->hwndStatus, WM_SIZE, 0, 0);
				RedrawWindow(dat->hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
		}
		return FALSE;

	case WM_SETFOCUS:
		if (dat->hwndActive != NULL)
			SetFocus(dat->hwndActive);
		return TRUE;

	case WM_CLOSE:
		if (g_dat.flags2 & SMF2_HIDECONTAINERS && dat->childrenCount > 0)
			ShowWindow(hwndDlg, SW_HIDE);
		else
			DestroyWindow(hwndDlg);
		return TRUE;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
			if (dat && dat->hwndActive && dis->hwndItem == dat->hwndStatus) {
				MessageWindowTabData *mwtd = GetChildFromHWND(dat, dat->hwndActive);
				if (mwtd != NULL)
					DrawStatusIcons(mwtd->hContact, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
			if (dis->hwndItem == dat->hwndTabs) {
				DrawTab(dat, dat->hwndTabs, wParam, lParam);
				return TRUE;
			}
		}
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact))
			break;

		if (LOWORD(wParam) == IDCANCEL)
			return TRUE;
		break;

	case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*) lParam;
			if (pNMHDR->hwndFrom == dat->hwndTabs) {
				switch (pNMHDR->code) {
				case TCN_SELCHANGE:
					{
						TCITEM tci = {0};
						int iSel = TabCtrl_GetCurSel(dat->hwndTabs);
						tci.mask = TCIF_PARAM;
						if (TabCtrl_GetItem(dat->hwndTabs, iSel, &tci)) {
							MessageWindowTabData * mwtd = (MessageWindowTabData *) tci.lParam;
							ActivateChild(dat, mwtd->hwnd);
							SetFocus(dat->hwndActive);
						}
					}
					break;
				case NM_RCLICK:
					{
						TCHITTESTINFO thinfo;
						GetCursorPos(&thinfo.pt);

						int x = thinfo.pt.x;
						int y = thinfo.pt.y;
						ScreenToClient(dat->hwndTabs, &thinfo.pt);
						int tabId = TabCtrl_HitTest(dat->hwndTabs, &thinfo);
						if (tabId != -1) {
							HMENU hMenu, hSubMenu, hUserMenu;
							BOOL menuResult;
							MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, tabId);
							hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
							hSubMenu = GetSubMenu(hMenu, 3);
							TranslateMenu(hSubMenu);
							hUserMenu = (HMENU) SendMessage(mwtd->hwnd, DM_GETCONTEXTMENU, 0, 0);
							if (hUserMenu != NULL) {
								InsertMenu(hSubMenu, 0, MF_POPUP | MF_BYPOSITION, (UINT_PTR)hUserMenu, TranslateT("User Menu"));
								InsertMenu(hSubMenu, 1, MF_SEPARATOR | MF_BYPOSITION, 0, 0);
							}
							menuResult = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, x, y, 0, hwndDlg, NULL);
							switch (menuResult) {
							case IDM_CLOSETAB:
								SendMessage(mwtd->hwnd, WM_CLOSE, 0, 0);
								break;
							case IDM_CLOSEOTHERTABS:
								CloseOtherChilden(dat, mwtd->hwnd);
								break;
							default:
								CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(menuResult), MPCF_CONTACTMENU), (LPARAM)mwtd->hContact);
							}
							if (hUserMenu != NULL)
								DestroyMenu(hUserMenu);
							DestroyMenu(hMenu);
						}
					}
					break;
				}
			}
			else if (pNMHDR->hwndFrom == dat->hwndStatus)  {
				switch (pNMHDR->code) {
				case NM_CLICK:
					NMMOUSE *nm = (NMMOUSE*)lParam;
					RECT rc;
					SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
					if (nm->pt.x >= rc.left) {
						MessageWindowTabData *mwtd = GetChildFromHWND(dat, dat->hwndActive);
						if (mwtd != NULL) {
							CheckStatusIconClick(mwtd->hContact, dat->hwndStatus, nm->pt, rc, 2, (pNMHDR->code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0));
						}
					}
					return TRUE;
				}
			}
		}
		break;

	case WM_DROPFILES:
		SendMessage(dat->hwndActive, WM_DROPFILES, wParam, lParam);
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND) {
			if (dat->nFlash < 2 * dat->nFlashMax) {
				FlashWindow(hwndDlg, TRUE);
				dat->nFlash++;
			}
			else {
				KillTimer(hwndDlg, TIMERID_FLASHWND);
				FlashWindow(hwndDlg, FALSE);
			}
		}
		break;

	case WM_CONTEXTMENU:
		if (dat->hwndStatus && dat->hwndStatus == (HWND)wParam) {
			RECT rc;
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2 = pt;
			ScreenToClient(dat->hwndStatus, &pt);

			SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
			if (pt.x >= rc.left) {
				MessageWindowTabData *mwtd = GetChildFromHWND(dat, dat->hwndActive);
				if (mwtd != NULL)
					CheckStatusIconClick(mwtd->hContact, dat->hwndStatus, pt, rc, 2, MBCF_RIGHTBUTTON);
				break;
			}
			else SendMessage(dat->hwndActive, WM_CONTEXTMENU, (WPARAM)hwndDlg, 0);
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			ws = GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			ws |= dat->flags2 & SMF2_USETRANSPARENCY ? WS_EX_LAYERED : 0;
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, ws);
			if (dat->flags2 & SMF2_USETRANSPARENCY)
				SetLayeredWindowAttributes(hwndDlg, RGB(255, 255, 255), (BYTE)(255 - g_dat.inactiveAlpha), LWA_ALPHA);
			break;
		}
		if (dat->hwndActive != NULL) {
			ActivateChild(dat, dat->hwndActive);
			g_dat.hFocusWnd = dat->hwndActive;
			PostMessage(dat->hwndActive, DM_SETFOCUS, 0, msg);
		}
		if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
			FlashWindow(hwndDlg, FALSE);
			dat->nFlash = 0;
		}
		ws = GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
		ws |= dat->flags2 & SMF2_USETRANSPARENCY ? WS_EX_LAYERED : 0;
		SetWindowLongPtr(hwndDlg , GWL_EXSTYLE , ws);
		if (dat->flags2 & SMF2_USETRANSPARENCY)
			SetLayeredWindowAttributes(hwndDlg, RGB(255,255,255), (BYTE)(255-g_dat.activeAlpha), LWA_ALPHA);
		break;

	case WM_LBUTTONDOWN:
		if (!IsZoomed(hwndDlg)) {
			POINT pt;
			GetCursorPos(&pt);
			return SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		break;

	case WM_MOVING:
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			int snapPixels = 10;
			RECT rcDesktop;
			RECT *pRect = (RECT *)lParam;
			POINT pt;
			MONITORINFO mi;
			HMONITOR hMonitor = MonitorFromRect(pRect, MONITOR_DEFAULTTONEAREST);
			SIZE szSize = {pRect->right-pRect->left,pRect->bottom-pRect->top};
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);
			GetCursorPos(&pt);

			rcDesktop = mi.rcWork;
			pRect->left = pt.x-dat->mouseLBDownPos.x;
			pRect->top = pt.y-dat->mouseLBDownPos.y;
			pRect->right = pRect->left+szSize.cx;
			pRect->bottom = pRect->top+szSize.cy;
			if (pRect->top < rcDesktop.top+snapPixels && pRect->top > rcDesktop.top-snapPixels) {
				pRect->top = rcDesktop.top;
				pRect->bottom = rcDesktop.top + szSize.cy;
			}
			if (pRect->left < rcDesktop.left+snapPixels && pRect->left > rcDesktop.left-snapPixels) {
				pRect->left = rcDesktop.left;
				pRect->right = rcDesktop.left + szSize.cx;
			}
			if (pRect->right < rcDesktop.right+snapPixels && pRect->right > rcDesktop.right-snapPixels) {
				pRect->right = rcDesktop.right;
				pRect->left = rcDesktop.right - szSize.cx;
			}
			if (pRect->bottom < rcDesktop.bottom+snapPixels && pRect->bottom > rcDesktop.bottom-snapPixels) {
				pRect->bottom = rcDesktop.bottom;
				pRect->top = rcDesktop.bottom - szSize.cy;
			}
		}
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MAXIMIZE) {
			if (GetKeyState(VK_CONTROL) & 0x8000)
				dat->bVMaximized = 1;
			else
				dat->bVMaximized = 0;
		}
		else if ((wParam & 0xFFF0) == SC_MOVE) {
			RECT  rc;
			GetWindowRect(hwndDlg, &rc);
			dat->mouseLBDownPos.x = LOWORD(lParam) - rc.left;
			dat->mouseLBDownPos.y = HIWORD(lParam) - rc.top;
		}
		else if (wParam == IDM_TOPMOST) {
			HMENU hMenu = GetSystemMenu(hwndDlg, FALSE);
			if (dat->bTopmost)  {
				CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);
				SetWindowPos(hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				dat->bTopmost = FALSE;
			}
			else {
				CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
				SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				dat->bTopmost = TRUE;
			}
		}
		break;
	case WM_DESTROY:
		{
			char szSettingName[64];
			for (int i = dat->childrenCount; i >= 0; i--) {
				TCITEM tci;
				tci.mask = TCIF_PARAM | TCIF_IMAGE;
				if (TabCtrl_GetItem(dat->hwndTabs, i, &tci)) {
					mir_free((MessageWindowTabData *)tci.lParam);
					ReleaseIcon(tci.iImage);
					TabCtrl_DeleteItem(dat->hwndTabs, i);
				}
			}
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			WindowList_Remove(g_dat.hParentWindowList, hwndDlg);

			int savePerContact = db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			MCONTACT hContact = (savePerContact) ? dat->hContact : NULL;

			WINDOWPLACEMENT wp = { sizeof(wp) };
			GetWindowPlacement(hwndDlg, &wp);

			char *szNamePrefix = (!savePerContact && dat->isChat) ? "chat" : "";
			if (!dat->windowWasCascaded) {
				mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sx", szNamePrefix);
				db_set_dw(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.left);
				mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sy", szNamePrefix);
				db_set_dw(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.top);
			}
			mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", szNamePrefix);
			db_set_dw(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.right - wp.rcNormalPosition.left);
			mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sheight", szNamePrefix);
			db_set_dw(hContact, SRMMMOD, szSettingName, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
			db_set_b(hContact, SRMMMOD, SRMSGSET_TOPMOST, (BYTE)dat->bTopmost);
			if (g_dat.lastParent == dat)
				g_dat.lastParent = dat->prev;

			if (g_dat.lastChatParent == dat)
				g_dat.lastChatParent = dat->prev;

			if (dat->prev != NULL)
				dat->prev->next = dat->next;

			if (dat->next != NULL)
				dat->next->prev = dat->prev;

			UnsubclassTabCtrl(dat->hwndTabs);
			mir_free(dat);
		}
		break;

	case DM_ERRORDECIDED:
		break;

	case CM_STARTFLASHING:
		if ((GetActiveWindow() != hwndDlg || GetForegroundWindow() != hwndDlg)) {// && !(g_dat.flags2 & SMF2_STAYMINIMIZED)) {
			dat->nFlash = 0;
			SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
		}
		break;

	case CM_POPUPWINDOW:
		EnableWindow(hwndDlg, TRUE);
		if (wParam) { /* incoming message */
			if (g_dat.flags & SMF_STAYMINIMIZED) {
				if (!IsWindowVisible(hwndDlg))
					ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);

				if (dat->childrenCount == 1 || ((g_dat.flags2 & SMF2_SWITCHTOACTIVE) && (IsIconic(hwndDlg) || GetForegroundWindow() != hwndDlg)))
					SendMessage(hwndDlg, CM_ACTIVATECHILD, 0, (LPARAM)lParam);
			}
			else {
				ShowWindow(hwndDlg, IsIconic(hwndDlg) ? SW_SHOWNORMAL : SW_SHOWNA);

				if (dat->childrenCount == 1 || ((g_dat.flags2 & SMF2_SWITCHTOACTIVE) && (IsIconic(hwndDlg) || GetForegroundWindow() != hwndDlg)))
					SendMessage(hwndDlg, CM_ACTIVATECHILD, 0, (LPARAM)lParam);

				SetWindowPos(hwndDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			}
		}
		else { /* outgoing message */
			ShowWindow(hwndDlg, IsIconic(hwndDlg) ? SW_SHOWNORMAL : SW_SHOW);
			SetForegroundWindow(hwndDlg);
			SetFocus((HWND)lParam);
		}
		break;

	case CM_REMOVECHILD:
		RemoveChild(dat, (HWND)lParam);
		if (dat->childrenCount != 0)
			SetFocus(dat->hwndActive);
		else
			PostMessage(hwndDlg, WM_CLOSE, 0, 0);
		return TRUE;

	case CM_ADDCHILD:
		AddChild(dat, (HWND)wParam, lParam);
		return TRUE;

	case CM_ACTIVATECHILD:
		ActivateChild(dat, (HWND)lParam);
		return TRUE;

	case CM_ACTIVATEPREV:
		ActivatePrevChild(dat, (HWND)lParam);
		SetFocus(dat->hwndActive);
		return TRUE;

	case CM_ACTIVATENEXT:
		ActivateNextChild(dat, (HWND)lParam);
		SetFocus(dat->hwndActive);
		return TRUE;

	case CM_ACTIVATEBYINDEX:
		ActivateChildByIndex(dat, (int)lParam);
		SetFocus(dat->hwndActive);
		return TRUE;

	case CM_GETCHILDCOUNT:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)GetChildCount(dat));
		return TRUE;

	case CM_GETACTIVECHILD:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)dat->hwndActive);
		return TRUE;

	case CM_GETTOOLBARSTATUS:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)(dat->flags2 & SMF2_SHOWTOOLBAR) != 0);
		return TRUE;

	case DM_SENDMESSAGE:
		for (int i = 0; i < dat->childrenCount; i++) {
			MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
			SendMessage(mwtd->hwnd, DM_SENDMESSAGE, wParam, lParam);
		}
		break;

	case DM_OPTIONSAPPLIED:
		dat->flags2 = g_dat.flags2;
		SetContainerWindowStyle(dat);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case CM_UPDATETITLEBAR:
		{
			HWND hwnd = (HWND)lParam;
			TitleBarData *tbd = (TitleBarData *)wParam;
			if (tbd != NULL && dat->hwndActive == hwnd) {
				if (tbd->iFlags & TBDF_TEXT) {
					TCHAR oldtitle[256];
					GetWindowText(hwndDlg, oldtitle, SIZEOF(oldtitle));
					if (mir_tstrcmp(tbd->pszText, oldtitle))
						SetWindowText(hwndDlg, tbd->pszText);
				}
				if (tbd->iFlags & TBDF_ICON) {
					SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)tbd->hIcon);
					if (tbd->hIconBig != NULL)
						SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)tbd->hIconBig);
					if (pTaskbarInterface)
						pTaskbarInterface->SetOverlayIcon(hwndDlg, tbd->hIconNot, L"");
				}
			}
		}
		break;

	case CM_UPDATESTATUSBAR:
		{
			HWND hwnd = (HWND)lParam;
			StatusBarData *sbd = (StatusBarData *)wParam;
			if (sbd != NULL) {
				if ((sbd->iFlags & SBDF_TEXT) && dat->hwndActive == hwnd)
					SendMessage(dat->hwndStatus, SB_SETTEXT, sbd->iItem, (LPARAM)sbd->pszText);
				if ((sbd->iFlags & SBDF_ICON) && dat->hwndActive == hwnd)
					SendMessage(dat->hwndStatus, SB_SETICON, sbd->iItem, (LPARAM)sbd->hIcon);
				RedrawWindow(dat->hwndStatus, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
		SetupStatusBar(dat);
		RedrawWindow(dat->hwndStatus, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case CM_UPDATETABCONTROL:
		{
			TCHAR *ptszTemp = NULL;
			TabControlData *tcd = (TabControlData *) wParam;
			int tabId = GetTabFromHWND(dat, (HWND) lParam);
			if (tabId >= 0 && tcd != NULL) {
				TCITEM tci;
				tci.mask = 0;
				if (tcd->iFlags & TCDF_TEXT) {
					tci.mask |= TCIF_TEXT;
					tci.pszText = tcd->pszText;
					if (g_dat.flags2 & SMF2_LIMITNAMES) {
						TCHAR * ltext = limitText(tcd->pszText, g_dat.limitNamesLength);
						if (ltext != tcd->pszText) 
							tci.pszText = ptszTemp = ltext;
					}
				}
				if (tcd->iFlags & TCDF_ICON) {
					int iconIdx = -1;
					if (tcd->hIcon != NULL) {
						TCITEM tci2;
						tci2.mask = TCIF_IMAGE;
						TabCtrl_GetItem(dat->hwndTabs, tabId, &tci2);
						iconIdx = AddOrReplaceIcon(g_dat.hTabIconList, tci2.iImage, tcd->hIcon);
					}
					tci.mask |= TCIF_IMAGE;
					tci.iImage = iconIdx;
				}
				TabCtrl_SetItem(dat->hwndTabs, tabId, &tci);
			}
			mir_free(ptszTemp);
		}
		break;

	case DM_SWITCHINFOBAR:
		dat->flags2 ^= SMF2_SHOWINFOBAR;
		{
			for (int i=0; i < dat->childrenCount; i++) {
				MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
				SendMessage(mwtd->hwnd, DM_SWITCHINFOBAR, 0, 0);
			}
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		}
		break;

	case DM_SWITCHSTATUSBAR:
		dat->flags2 ^= SMF2_SHOWSTATUSBAR;
		ShowWindow(dat->hwndStatus, (dat->flags2 & SMF2_SHOWSTATUSBAR) ? SW_SHOW : SW_HIDE);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_SWITCHTOOLBAR:
		dat->flags2 ^= SMF2_SHOWTOOLBAR;
		{
			for (int i=0; i < dat->childrenCount; i++) {
				MessageWindowTabData * mwtd = GetChildFromTab(dat->hwndTabs, i);
				SendMessage(mwtd->hwnd, DM_SWITCHTOOLBAR, 0, 0);
			}
		}
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_SWITCHTITLEBAR:
		{
			RECT rc;
			dat->flags2 ^= SMF2_SHOWTITLEBAR;
			ws = GetWindowLongPtr(hwndDlg, GWL_STYLE) & ~(WS_CAPTION);
			if (dat->flags2 & SMF2_SHOWTITLEBAR)
				ws |= WS_CAPTION;

			SetWindowLongPtr(hwndDlg, GWL_STYLE, ws);
			GetWindowRect(hwndDlg, &rc);
			SetWindowPos(hwndDlg, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER  | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
		break;

	case DM_CASCADENEWWINDOW:
		if ((HWND)wParam != hwndDlg) {
			RECT rcThis, rcNew;
			GetWindowRect(hwndDlg, &rcThis);
			GetWindowRect((HWND) wParam, &rcNew);
			if (abs(rcThis.left - rcNew.left) < 3 && abs(rcThis.top - rcNew.top) < 3) {
				int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
				SetWindowPos((HWND) wParam, 0, rcNew.left + offset, rcNew.top + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				*(int *) lParam = 1;
			}
		}
		break;
	}
	return FALSE;
}

static void DrawTab(ParentWindowData *dat, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT) lParam;
	int iTabIndex = lpDIS->itemID;
	if (iTabIndex < 0)
		return;
	
	TabCtrlData *tcdat = (TabCtrlData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HANDLE hTheme = NULL;
	int tstate;
	TCHAR szLabel[1024];
	TCITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	tci.pszText = szLabel;
	tci.cchTextMax = SIZEOF(szLabel);
	tci.dwStateMask = TCIS_HIGHLIGHTED;
	if (TabCtrl_GetItem(hwnd, iTabIndex, &tci)) {
		IMAGEINFO info;
		RECT rIcon = lpDIS->rcItem;
		RECT rect = lpDIS->rcItem;
		RECT rectTab = lpDIS->rcItem;
		int bSelected = lpDIS->itemState & ODS_SELECTED;
		int atTop = (GetWindowLongPtr(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;
		UINT dwFormat;
		if (!IsAppThemed()) {
			FillRect(lpDIS->hDC, &rect, GetSysColorBrush(COLOR_BTNFACE));
		}
		else {
			if (lpDIS->itemState & ODS_SELECTED)
				tstate = TTIS_SELECTED;
			else if (lpDIS->itemState & ODS_FOCUS)
				tstate = TTIS_FOCUSED;
			else if (lpDIS->itemState & ODS_HOTLIGHT)
				tstate = TTIS_HOT;
			else
				tstate = TTIS_NORMAL;

			if (!bSelected)
				InflateRect(&rectTab, 1, 1);

			hTheme = OpenThemeData(hwnd, L"TAB");
			if (IsThemeBackgroundPartiallyTransparent(hTheme, TABP_TABITEM, tstate))
				DrawThemeParentBackground(hwnd, lpDIS->hDC, &rectTab);
			DrawThemeBackground(hTheme, lpDIS->hDC, TABP_TABITEM, tstate, &rectTab, NULL);
		}
		if (atTop) {
			dwFormat = DT_SINGLELINE | DT_TOP | DT_CENTER | DT_NOPREFIX | DT_NOCLIP;
			rIcon.top = rect.top + GetSystemMetrics(SM_CYEDGE);
			if (tci.iImage >= 0) {
				rIcon.left = rect.left + GetSystemMetrics(SM_CXEDGE) + (bSelected ? 6 : 2);
				ImageList_GetImageInfo(g_dat.hTabIconList, tci.iImage, &info);
				ImageList_DrawEx(g_dat.hTabIconList, tci.iImage, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
				rect.left = rIcon.left + (info.rcImage.right - info.rcImage.left);
			}
			if (dat->flags2 & SMF2_TABCLOSEBUTTON) {
				ImageList_GetImageInfo(g_dat.hButtonIconList, 0, &info);
				rIcon.left = rect.right - GetSystemMetrics(SM_CXEDGE) - (bSelected ? 6 : 2) - (info.rcImage.right - info.rcImage.left);
				ImageList_DrawEx(g_dat.hButtonIconList, 0, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
				rect.right = rIcon.left - 1;
			}
			rect.top += GetSystemMetrics(SM_CYEDGE) + 2;
		}
		else {
			dwFormat = DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOPREFIX | DT_NOCLIP;
			rIcon.left = rect.left + GetSystemMetrics(SM_CXEDGE) + (bSelected ? 6 : 2);
			if (tci.iImage >= 0) {
				ImageList_GetImageInfo(g_dat.hTabIconList, tci.iImage, &info);
				rIcon.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top) - 1;
				ImageList_DrawEx(g_dat.hTabIconList, tci.iImage, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
				rect.left = rIcon.left + (info.rcImage.right - info.rcImage.left);
			}
			if (dat->flags2 & SMF2_TABCLOSEBUTTON) {
				ImageList_GetImageInfo(g_dat.hButtonIconList, 0, &info);
				rIcon.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top) - 2;
				rIcon.left = rect.right - GetSystemMetrics(SM_CXEDGE) - (bSelected ? 6 : 2) - (info.rcImage.right - info.rcImage.left);
				ImageList_DrawEx(g_dat.hButtonIconList, 0, lpDIS->hDC, rIcon.left, rIcon.top, 0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
				rect.right = rIcon.left - 1;
			}
			rect.bottom -= GetSystemMetrics(SM_CYEDGE) + 2;
		}

		if (hTheme)
			DrawThemeText(hTheme, lpDIS->hDC, TABP_TABITEM, tstate, szLabel, -1, dwFormat, 0, &rect);
		else 
			DrawText(lpDIS->hDC, szLabel, -1, &rect, dwFormat);

		if (tcdat->bDragged && iTabIndex == tcdat->destTab && iTabIndex != tcdat->srcTab) {
			RECT hlRect = lpDIS->rcItem;
			if (bSelected) {
				hlRect.bottom -= GetSystemMetrics(SM_CYEDGE);
				hlRect.top += GetSystemMetrics(SM_CYEDGE);
				hlRect.left += GetSystemMetrics(SM_CXEDGE);
				hlRect.right -= GetSystemMetrics(SM_CXEDGE);
			}
			else {
				if (atTop) {
					hlRect.top += GetSystemMetrics(SM_CYEDGE);
					hlRect.bottom += GetSystemMetrics(SM_CYEDGE);
				}
				else {
					hlRect.top -= GetSystemMetrics(SM_CYEDGE);
					hlRect.bottom -= GetSystemMetrics(SM_CYEDGE);
				}
			}
			FrameRect(lpDIS->hDC, &hlRect, GetSysColorBrush(COLOR_HIGHLIGHT));
			hlRect.left++;
			hlRect.top++;
			hlRect.right--;
			hlRect.bottom--;
			FrameRect(lpDIS->hDC, &hlRect, GetSysColorBrush(COLOR_HIGHLIGHT));
		}
		if (hTheme)
			CloseThemeData(hTheme);
	}
}

int ScriverRestoreWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule,const char *szNamePrefix, int flags, int showCmd)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);

	char szSettingName[64];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sx", szNamePrefix);
	int x = db_get_dw(hContact,szModule,szSettingName,-1);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sy", szNamePrefix);
	int y = db_get_dw(hContact,szModule,szSettingName,-1);
	if (x == -1)
		return 1;

	if (flags & RWPF_NOSIZE)
		OffsetRect(&wp.rcNormalPosition,x-wp.rcNormalPosition.left,y-wp.rcNormalPosition.top);
	else {
		wp.rcNormalPosition.left = x;
		wp.rcNormalPosition.top = y;
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", szNamePrefix);
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + db_get_dw(hContact,szModule,szSettingName,-1);
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sheight", szNamePrefix);
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + db_get_dw(hContact,szModule,szSettingName,-1);
	}
	wp.flags = 0;
	wp.showCmd = showCmd;

	HMONITOR hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);
	RECT rcDesktop = mi.rcWork;
	if (wp.rcNormalPosition.left > rcDesktop.right || wp.rcNormalPosition.top > rcDesktop.bottom
		|| wp.rcNormalPosition.right < rcDesktop.left || wp.rcNormalPosition.bottom < rcDesktop.top) return 1;
	SetWindowPlacement(hwnd,&wp);
	return 0;
}

HWND GetParentWindow(MCONTACT hContact, BOOL bChat)
{
	NewMessageWindowLParam newData = { 0 };
	newData.hContact = hContact;
	newData.isChat = bChat;
	if (g_dat.flags2 & SMF2_USETABS) {
		if (!bChat || !(g_dat.flags2 & SMF2_SEPARATECHATSCONTAINERS)) {
			if (g_dat.lastParent != NULL) {
				int tabsNum = (int)SendMessage(g_dat.lastParent->hwnd, CM_GETCHILDCOUNT, 0, 0);
				if (!(g_dat.flags2 & SMF2_LIMITTABS) || tabsNum < g_dat.limitTabsNum)
					return g_dat.lastParent->hwnd;
			}
		}
		else {
			if (g_dat.lastChatParent != NULL) {
				int tabsNum = (int)SendMessage(g_dat.lastChatParent->hwnd, CM_GETCHILDCOUNT, 0, 0);
				if (!(g_dat.flags2 & SMF2_LIMITCHATSTABS) || tabsNum < g_dat.limitChatsTabsNum) {
					return g_dat.lastChatParent->hwnd;
				}
			}
		}
	}
	if (!(g_dat.flags2 & SMF2_SEPARATECHATSCONTAINERS))
		newData.isChat = FALSE;

	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGWIN), NULL, DlgProcParentWindow, (LPARAM)&newData);
}
