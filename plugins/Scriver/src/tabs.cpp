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

#include "stdafx.h"

#define SB_CHAR_WIDTH		 40
#define SB_SENDING_WIDTH 	 25
#define SB_UNICODE_WIDTH 	 18

#define TIMEOUT_FLASHWND     900

static void DrawTab(ParentWindowData *dat, HWND hwnd, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////

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
		CScriverWindow *pDlg = (CScriverWindow *)tci.lParam;
		if (pDlg->GetHwnd() == child)
			return i;
	}
	return -1;
}

static CScriverWindow* GetChildFromTab(HWND hwndTabs, int tabId)
{
	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM;
	if (TabCtrl_GetItem(hwndTabs, tabId, &tci))
		return (CScriverWindow *)tci.lParam;

	return nullptr;
}

static CScriverWindow* GetChildFromHWND(ParentWindowData *dat, HWND hwnd)
{
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	for (int i = 0; i < l; i++) {
		TCITEM tci = { 0 };
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(dat->hwndTabs, i, &tci);
		CScriverWindow *pDlg = (CScriverWindow *)tci.lParam;
		if (pDlg->GetHwnd() == hwnd)
			return pDlg;
	}
	return nullptr;
}

static void GetMinimunWindowSize(ParentWindowData *dat, SIZE *size)
{
	MINMAXINFO mmi;
	RECT rc, rcWindow;
	int minW = 216, minH = 80;
	GetWindowRect(dat->hwnd, &rcWindow);
	GetChildWindowRect(dat, &rc);
	for (int i = 0; i < dat->childrenCount; i++) {
		CScriverWindow * pDlg = GetChildFromTab(dat->hwndTabs, i);
		SendMessage(pDlg->GetHwnd(), WM_GETMINMAXINFO, 0, (LPARAM)&mmi);
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
	g_dat.tabIconListUsage[usageIdx].index = ImageList_ReplaceIcon(hList, prevIndex, hIcon);
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
	SetWindowPos(child, HWND_TOP, rcChild.left, rcChild.top, rcChild.right - rcChild.left, rcChild.bottom - rcChild.top, 0);

	int i = GetTabFromHWND(dat, child);
	if (i == -1)
		return;

	CScriverWindow *pDlg = GetChildFromTab(dat->hwndTabs, i);
	if (pDlg == nullptr)
		return;

	dat->hContact = pDlg->m_hContact;
	if (child != dat->hwndActive) {
		HWND prev = dat->hwndActive;
		dat->hwndActive = child;
		SetupStatusBar(dat);
		pDlg->UpdateStatusBar();
		pDlg->UpdateTitle();
		SendMessage(dat->hwndActive, WM_SIZE, 0, 0);
		ShowWindow(dat->hwndActive, SW_SHOWNOACTIVATE);
		SendMessage(dat->hwndActive, DM_SCROLLLOGTOBOTTOM, 0, 0);
		if (prev != nullptr)
			ShowWindow(prev, SW_HIDE);
	}
	else SendMessage(dat->hwnd, WM_SIZE, 0, 0);

	TabCtrl_SetCurSel(dat->hwndTabs, i);
	SendMessage(dat->hwndActive, DM_ACTIVATE, WA_ACTIVE, 0);
}

static void AddChild(ParentWindowData *dat, CScriverWindow *pDlg)
{
	dat->childrenCount++;

	TCITEM tci;
	tci.mask = TCIF_PARAM | TCIF_IMAGE | TCIF_TEXT;
	tci.lParam = (LPARAM)pDlg;
	tci.iImage = -1;
	tci.pszText = L"";
	TabCtrl_InsertItem(dat->hwndTabs, dat->childrenCount - 1, &tci);
	SetWindowPos(pDlg->GetHwnd(), HWND_TOP, dat->childRect.left, dat->childRect.top, dat->childRect.right - dat->childRect.left, dat->childRect.bottom - dat->childRect.top, SWP_HIDEWINDOW);
	SendMessage(dat->hwnd, WM_SIZE, 0, 0);

	EnableThemeDialogTexture(pDlg->GetHwnd(), ETDT_ENABLETAB);
}

static void RemoveChild(ParentWindowData *dat, HWND child)
{
	int tab = GetTabFromHWND(dat, child);
	if (tab >= 0) {
		TCITEM tci;
		tci.mask = TCIF_PARAM | TCIF_IMAGE;
		TabCtrl_GetItem(dat->hwndTabs, tab, &tci);
		TabCtrl_DeleteItem(dat->hwndTabs, tab);
		dat->childrenCount--;
		if (child == dat->hwndActive) {
			if (tab == TabCtrl_GetItemCount(dat->hwndTabs)) tab--;
			if (tab >= 0)
				ActivateChild(dat, GetChildFromTab(dat->hwndTabs, tab)->GetHwnd());
			else
				dat->hwndActive = nullptr;
		}
		ReleaseIcon(tci.iImage);
	}
}

static void CloseOtherChilden(ParentWindowData *dat, HWND child)
{
	ActivateChild(dat, child);
	for (int i = dat->childrenCount - 1; i >= 0; i--) {
		CScriverWindow *pDlg = GetChildFromTab(dat->hwndTabs, i);
		if (pDlg != nullptr && pDlg->GetHwnd() != child)
			pDlg->Close();
	}
	ActivateChild(dat, child);
}

static void ActivateNextChild(ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i + 1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->GetHwnd());
}

static void ActivatePrevChild(ParentWindowData *dat, HWND child)
{
	int i = GetTabFromHWND(dat, child);
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	i = (i + l - 1) % l;
	ActivateChild(dat, GetChildFromTab(dat->hwndTabs, i)->GetHwnd());
}

static void ActivateChildByIndex(ParentWindowData *dat, int index)
{
	int l = TabCtrl_GetItemCount(dat->hwndTabs);
	if (index < l) {
		CScriverWindow *pDlg = GetChildFromTab(dat->hwndTabs, index);
		if (pDlg != nullptr)
			ActivateChild(dat, pDlg->GetHwnd());
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
			CScriverWindow *pDlg = (CScriverWindow *)tci.lParam;
			if (pDlg != nullptr) {
				SendMessage(pDlg->GetHwnd(), WM_CLOSE, 0, 0);
				dat->srcTab = -1;
			}
		}
		return 0;

	case WM_LBUTTONDBLCLK:
		thinfo.pt.x = LOWORD(lParam);
		thinfo.pt.y = HIWORD(lParam);
		tabId = TabCtrl_HitTest(hwnd, &thinfo);
		if (tabId >= 0 && tabId == dat->srcTab) {
			GetChildFromTab(hwnd, tabId)->Close();
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
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
				dat->destTab = TabCtrl_HitTest(hwnd, &thinfo);
				if (thinfo.flags != TCHT_NOWHERE && dat->destTab != dat->srcTab) {
					NMHDR nmh;
					wchar_t  sBuffer[501];
					TCITEM item;
					int curSel;
					curSel = TabCtrl_GetCurSel(hwnd);
					item.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT;
					item.pszText = sBuffer;
					item.cchTextMax = _countof(sBuffer);
					TabCtrl_GetItem(hwnd, dat->srcTab, &item);
					sBuffer[_countof(sBuffer) - 1] = '\0';

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
					dat->destTab = -1;

					TCITEM tci;
					tci.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, dat->srcTab, &tci);
					CScriverWindow *pDlg = (CScriverWindow*)tci.lParam;
					if (pDlg != nullptr)
						pDlg->Reattach(GetParent(hwnd));
				}
				else {
					dat->destTab = -1;
					RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
				}
			}
			else if (dat->srcTab >= 0 && g_dat.flags2 & SMF2_TABCLOSEBUTTON) {
				int atTop = (GetWindowLongPtr(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;

				RECT rect;
				TabCtrl_GetItemRect(hwnd, dat->srcTab, &rect);
				
				POINT pt = { LOWORD(lParam), HIWORD(lParam) };
				IMAGEINFO info;
				ImageList_GetImageInfo(g_dat.hButtonIconList, 0, &info);
				rect.left = rect.right - (info.rcImage.right - info.rcImage.left) - 6;
				if (!atTop)
					rect.top = rect.bottom - (info.rcImage.bottom - info.rcImage.top);

				if (pt.x >= rect.left && pt.x < rect.left + (info.rcImage.right - info.rcImage.left) && pt.y >= rect.top && pt.y < rect.top + (info.rcImage.bottom - info.rcImage.top)) {
					HBITMAP hOldBitmap, hBmp;
					HDC hdc = GetDC(nullptr);
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
					ReleaseDC(nullptr, hdc);
					if (color1 != 0x000000 || color2 != 0xFFFFFF) {
						GetChildFromTab(hwnd, dat->srcTab)->Close();
						dat->srcTab = -1;
					}
				}
				else SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);
			}
			else SendMessage(hwnd, WM_LBUTTONDOWN, dat->clickWParam, dat->clickLParam);

			dat->bDragged = dat->bDragging = FALSE;
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
					SetWindowOrgEx(hMemDC, rect.left, rect.top, nullptr);
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
						RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
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

static int ScriverRestoreWindowPosition(HWND hwnd, MCONTACT hContact, const char *szModule, const char *szNamePrefix, int flags, int showCmd)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);

	char szSettingName[64];
	mir_snprintf(szSettingName, "%sx", szNamePrefix);
	int x = db_get_dw(hContact, szModule, szSettingName, -1);
	mir_snprintf(szSettingName, "%sy", szNamePrefix);
	int y = db_get_dw(hContact, szModule, szSettingName, -1);
	if (x == -1)
		return 1;

	if (flags & RWPF_NOSIZE)
		OffsetRect(&wp.rcNormalPosition, x - wp.rcNormalPosition.left, y - wp.rcNormalPosition.top);
	else {
		wp.rcNormalPosition.left = x;
		wp.rcNormalPosition.top = y;
		mir_snprintf(szSettingName, "%swidth", szNamePrefix);
		wp.rcNormalPosition.right = wp.rcNormalPosition.left + db_get_dw(hContact, szModule, szSettingName, -1);
		mir_snprintf(szSettingName, "%sheight", szNamePrefix);
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + db_get_dw(hContact, szModule, szSettingName, -1);
	}
	wp.flags = 0;
	wp.showCmd = showCmd;

	HMONITOR hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);
	RECT rcDesktop = mi.rcWork;
	if (wp.rcNormalPosition.left > rcDesktop.right || wp.rcNormalPosition.top > rcDesktop.bottom ||
		wp.rcNormalPosition.right < rcDesktop.left || wp.rcNormalPosition.bottom < rcDesktop.top) return 1;
	SetWindowPlacement(hwnd, &wp);
	return 0;
}

struct NewMessageWindowLParam
{
	MCONTACT hContact;
	BOOL isChat;
};

static INT_PTR CALLBACK DlgProcParentWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ParentWindowData *dat = (ParentWindowData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!dat && msg != WM_INITDIALOG)
		return FALSE;

	DWORD ws;

	switch (msg) {
	case WM_INITDIALOG:
		{
			int savePerContact = db_get_b(0, SRMM_MODULE, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *)lParam;
			dat = (ParentWindowData *)mir_alloc(sizeof(ParentWindowData));
			dat->hContact = newData->hContact;
			dat->nFlash = 0;
			dat->nFlashMax = db_get_b(0, SRMM_MODULE, SRMSGSET_FLASHCOUNT, SRMSGDEFSET_FLASHCOUNT);
			dat->childrenCount = 0;
			dat->hwnd = hwndDlg;
			dat->mouseLBDown = 0;
			dat->windowWasCascaded = 0;
			dat->bMinimized = 0;
			dat->bVMaximized = 0;
			dat->iSplitterX = db_get_dw(0, SRMM_MODULE, "splitterx", -1);
			dat->iSplitterY = db_get_dw(0, SRMM_MODULE, "splittery", -1);
			dat->flags2 = g_dat.flags2;
			dat->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwndDlg, nullptr, g_hInst, nullptr);
			dat->isChat = newData->isChat;
			SendMessage(dat->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
			SetupStatusBar(dat);
			dat->hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
			dat->hwndActive = nullptr;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			if (g_dat.hTabIconList != nullptr)
				TabCtrl_SetImageList(dat->hwndTabs, g_dat.hTabIconList);

			dat->next = nullptr;
			if (!newData->isChat) {
				dat->prev = g_dat.lastParent;
				g_dat.lastParent = dat;
			}
			else {
				dat->prev = g_dat.lastChatParent;
				g_dat.lastChatParent = dat;
			}
			if (dat->prev != nullptr)
				dat->prev->next = dat;

			WindowList_Add(g_dat.hParentWindowList, hwndDlg, (UINT_PTR)hwndDlg);
			SubclassTabCtrl(dat->hwndTabs);

			SetContainerWindowStyle(dat);

			MCONTACT hSContact = savePerContact ? dat->hContact : 0;
			dat->bTopmost = db_get_b(hSContact, SRMM_MODULE, SRMSGSET_TOPMOST, SRMSGDEFSET_TOPMOST);
			if (ScriverRestoreWindowPosition(hwndDlg, hSContact, SRMM_MODULE, (newData->isChat && !savePerContact) ? "chat" : "", 0, SW_HIDE))
				SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);

			if (!savePerContact && db_get_b(0, SRMM_MODULE, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE))
				WindowList_Broadcast(g_dat.hParentWindowList, DM_CASCADENEWWINDOW, (WPARAM)hwndDlg, (LPARAM)&dat->windowWasCascaded);

			PostMessage(hwndDlg, WM_SIZE, 0, 0);

			HMENU hMenu = GetSystemMenu(hwndDlg, FALSE);
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
			if (dat->bTopmost) {
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_CHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always on top"));
				SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else InsertMenu(hMenu, 0, MF_BYPOSITION | MF_ENABLED | MF_UNCHECKED | MF_STRING, IDM_TOPMOST, TranslateT("Always on top"));
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

		if (IsIconic(hwndDlg))
			MoveWindow(dat->hwndActive, dat->childRect.left, dat->childRect.top, dat->childRect.right - dat->childRect.left, dat->childRect.bottom - dat->childRect.top, TRUE);
		else {
			RECT rcStatus, rcChild, rcWindow, rc;
			SIZE size;
			dat->bMinimized = 0;
			GetClientRect(hwndDlg, &rc);
			GetWindowRect(hwndDlg, &rcWindow);
			rcStatus.top = rcStatus.bottom = 0;
			if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
				GetWindowRect(dat->hwndStatus, &rcStatus);
				SetupStatusBar(dat);
			}
			MoveWindow(dat->hwndTabs, 0, 2, (rc.right - rc.left), (rc.bottom - rc.top) - (rcStatus.bottom - rcStatus.top) - 2, FALSE);
			RedrawWindow(dat->hwndTabs, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE);
			GetMinimunWindowSize(dat, &size);
			if ((rcWindow.bottom - rcWindow.top) < size.cy || (rcWindow.right - rcWindow.left) < size.cx) {
				if ((rcWindow.bottom - rcWindow.top) < size.cy)
					rcWindow.bottom = rcWindow.top + size.cy;
				if ((rcWindow.right - rcWindow.left) < size.cx)
					rcWindow.right = rcWindow.left + size.cx;
				MoveWindow(hwndDlg, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
			}
			GetChildWindowRect(dat, &rcChild);
			dat->childRect = rcChild;
			MoveWindow(dat->hwndActive, rcChild.left, rcChild.top, rcChild.right - rcChild.left, rcChild.bottom - rcChild.top, TRUE);
			RedrawWindow(GetDlgItem(dat->hwndActive, IDC_SRMM_LOG), nullptr, nullptr, RDW_INVALIDATE);
			if (dat->flags2 & SMF2_SHOWSTATUSBAR) {
				SendMessage(dat->hwndStatus, WM_SIZE, 0, 0);
				RedrawWindow(dat->hwndStatus, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
		}
		return FALSE;

	case WM_SETFOCUS:
		if (dat->hwndActive != nullptr)
			SetFocus(dat->hwndActive);
		return TRUE;

	case WM_CLOSE:
		if (g_dat.flags2 & SMF2_HIDECONTAINERS && dat->childrenCount > 0)
			ShowWindow(hwndDlg, SW_HIDE);
		else
			DestroyWindow(hwndDlg);
		return TRUE;

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dat && dat->hwndActive && dis->hwndItem == dat->hwndStatus) {
				CScriverWindow *pDlg = GetChildFromHWND(dat, dat->hwndActive);
				if (pDlg != nullptr)
					DrawStatusIcons(pDlg->m_hContact, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
			if (dis->hwndItem == dat->hwndTabs) {
				DrawTab(dat, dat->hwndTabs, wParam, lParam);
				return TRUE;
			}
		}
		return Menu_DrawItem(lParam);

	case WM_COMMAND:
		if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, dat->hContact))
			break;

		if (LOWORD(wParam) == IDCANCEL)
			return TRUE;
		break;

	case WM_NOTIFY:
		{
			NMHDR *pNMHDR = (NMHDR*)lParam;
			if (pNMHDR->hwndFrom == dat->hwndTabs) {
				switch (pNMHDR->code) {
				case TCN_SELCHANGE:
					{
						TCITEM tci = { 0 };
						int iSel = TabCtrl_GetCurSel(dat->hwndTabs);
						tci.mask = TCIF_PARAM;
						if (TabCtrl_GetItem(dat->hwndTabs, iSel, &tci)) {
							CScriverWindow * pDlg = (CScriverWindow *)tci.lParam;
							ActivateChild(dat, pDlg->GetHwnd());
							SetFocus(dat->hwndActive);
						}
					}
					break;

				case NM_RCLICK:
					TCHITTESTINFO thinfo;
					GetCursorPos(&thinfo.pt);

					int x = thinfo.pt.x;
					int y = thinfo.pt.y;
					ScreenToClient(dat->hwndTabs, &thinfo.pt);
					int tabId = TabCtrl_HitTest(dat->hwndTabs, &thinfo);
					if (tabId != -1) {
						CScriverWindow *pDlg = GetChildFromTab(dat->hwndTabs, tabId);
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
						HMENU hSubMenu = GetSubMenu(hMenu, 3);
						TranslateMenu(hSubMenu);
						HMENU hUserMenu = (HMENU)SendMessage(pDlg->GetHwnd(), DM_GETCONTEXTMENU, 0, 0);
						if (hUserMenu != nullptr) {
							InsertMenu(hSubMenu, 0, MF_POPUP | MF_BYPOSITION, (UINT_PTR)hUserMenu, TranslateT("User menu"));
							InsertMenu(hSubMenu, 1, MF_SEPARATOR | MF_BYPOSITION, 0, 0);
						}
						BOOL menuResult = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, x, y, 0, hwndDlg, nullptr);
						switch (menuResult) {
						case IDM_CLOSETAB:
							SendMessage(pDlg->GetHwnd(), WM_CLOSE, 0, 0);
							break;
						case IDM_CLOSEOTHERTABS:
							CloseOtherChilden(dat, pDlg->GetHwnd());
							break;
						default:
							Clist_MenuProcessCommand(LOWORD(menuResult), MPCF_CONTACTMENU, pDlg->m_hContact);
						}
						if (hUserMenu != nullptr)
							DestroyMenu(hUserMenu);
						DestroyMenu(hMenu);
					}
				}
				break;
			}
			else if (pNMHDR->hwndFrom == dat->hwndStatus) {
				switch (pNMHDR->code) {
				case NM_CLICK:
					RECT rc;
					NMMOUSE *nm = (NMMOUSE*)lParam;
					SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
					if (nm->pt.x >= rc.left) {
						CScriverWindow *pDlg = GetChildFromHWND(dat, dat->hwndActive);
						if (pDlg != nullptr)
							CheckStatusIconClick(pDlg->m_hContact, dat->hwndStatus, nm->pt, rc, 2, (pNMHDR->code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0));
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
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2 = pt;
			ScreenToClient(dat->hwndStatus, &pt);

			RECT rc;
			SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
			if (pt.x >= rc.left) {
				CScriverWindow *pDlg = GetChildFromHWND(dat, dat->hwndActive);
				if (pDlg != nullptr)
					CheckStatusIconClick(pDlg->m_hContact, dat->hwndStatus, pt, rc, 2, MBCF_RIGHTBUTTON);
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
		if (dat->hwndActive != nullptr) {
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
		SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, ws);
		if (dat->flags2 & SMF2_USETRANSPARENCY)
			SetLayeredWindowAttributes(hwndDlg, RGB(255, 255, 255), (BYTE)(255 - g_dat.activeAlpha), LWA_ALPHA);
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
			RECT *pRect = (RECT *)lParam;
			HMONITOR hMonitor = MonitorFromRect(pRect, MONITOR_DEFAULTTONEAREST);
			SIZE szSize = { pRect->right - pRect->left, pRect->bottom - pRect->top };

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hMonitor, &mi);

			POINT pt;
			GetCursorPos(&pt);

			RECT rcDesktop = mi.rcWork;
			pRect->left = pt.x - dat->mouseLBDownPos.x;
			pRect->top = pt.y - dat->mouseLBDownPos.y;
			pRect->right = pRect->left + szSize.cx;
			pRect->bottom = pRect->top + szSize.cy;
			if (pRect->top < rcDesktop.top + snapPixels && pRect->top > rcDesktop.top - snapPixels) {
				pRect->top = rcDesktop.top;
				pRect->bottom = rcDesktop.top + szSize.cy;
			}
			if (pRect->left < rcDesktop.left + snapPixels && pRect->left > rcDesktop.left - snapPixels) {
				pRect->left = rcDesktop.left;
				pRect->right = rcDesktop.left + szSize.cx;
			}
			if (pRect->right < rcDesktop.right + snapPixels && pRect->right > rcDesktop.right - snapPixels) {
				pRect->right = rcDesktop.right;
				pRect->left = rcDesktop.right - szSize.cx;
			}
			if (pRect->bottom < rcDesktop.bottom + snapPixels && pRect->bottom > rcDesktop.bottom - snapPixels) {
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
			if (dat->bTopmost) {
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
		char szSettingName[64];

		for (int i = dat->childrenCount; i >= 0; i--) {
			TCITEM tci;
			tci.mask = TCIF_PARAM | TCIF_IMAGE;
			if (TabCtrl_GetItem(dat->hwndTabs, i, &tci)) {
				ReleaseIcon(tci.iImage);
				TabCtrl_DeleteItem(dat->hwndTabs, i);
			}
		}

		db_set_dw(0, SRMM_MODULE, "splitterx", dat->iSplitterX);
		db_set_dw(0, SRMM_MODULE, "splittery", dat->iSplitterY);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		WindowList_Remove(g_dat.hParentWindowList, hwndDlg);
		{
			int savePerContact = db_get_b(0, SRMM_MODULE, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
			MCONTACT hContact = (savePerContact) ? dat->hContact : 0;

			WINDOWPLACEMENT wp = { sizeof(wp) };
			GetWindowPlacement(hwndDlg, &wp);

			char *szNamePrefix = (!savePerContact && dat->isChat) ? "chat" : "";
			if (!dat->windowWasCascaded) {
				mir_snprintf(szSettingName, "%sx", szNamePrefix);
				db_set_dw(hContact, SRMM_MODULE, szSettingName, wp.rcNormalPosition.left);
				mir_snprintf(szSettingName, "%sy", szNamePrefix);
				db_set_dw(hContact, SRMM_MODULE, szSettingName, wp.rcNormalPosition.top);
			}
			mir_snprintf(szSettingName, "%swidth", szNamePrefix);
			db_set_dw(hContact, SRMM_MODULE, szSettingName, wp.rcNormalPosition.right - wp.rcNormalPosition.left);
			mir_snprintf(szSettingName, "%sheight", szNamePrefix);
			db_set_dw(hContact, SRMM_MODULE, szSettingName, wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
			db_set_b(hContact, SRMM_MODULE, SRMSGSET_TOPMOST, (BYTE)dat->bTopmost);
			if (g_dat.lastParent == dat)
				g_dat.lastParent = dat->prev;

			if (g_dat.lastChatParent == dat)
				g_dat.lastChatParent = dat->prev;

			if (dat->prev != nullptr)
				dat->prev->next = dat->next;

			if (dat->next != nullptr)
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
			SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
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
		AddChild(dat, (CScriverWindow*)wParam);
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
			CScriverWindow * pDlg = GetChildFromTab(dat->hwndTabs, i);
			SendMessage(pDlg->GetHwnd(), DM_SENDMESSAGE, wParam, lParam);
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
			if (tbd != nullptr && dat->hwndActive == hwnd) {
				if (tbd->iFlags & TBDF_TEXT) {
					wchar_t oldtitle[256];
					GetWindowText(hwndDlg, oldtitle, _countof(oldtitle));
					if (mir_wstrcmp(tbd->pszText, oldtitle))
						SetWindowText(hwndDlg, tbd->pszText);
				}
				if (tbd->iFlags & TBDF_ICON) {
					SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)tbd->hIcon);
					if (tbd->hIconBig != nullptr)
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
			if (sbd != nullptr) {
				if ((sbd->iFlags & SBDF_TEXT) && dat->hwndActive == hwnd)
					SendMessage(dat->hwndStatus, SB_SETTEXT, sbd->iItem, (LPARAM)sbd->pszText);
				if ((sbd->iFlags & SBDF_ICON) && dat->hwndActive == hwnd)
					SendMessage(dat->hwndStatus, SB_SETICON, sbd->iItem, (LPARAM)sbd->hIcon);
				RedrawWindow(dat->hwndStatus, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
		SetupStatusBar(dat);
		RedrawWindow(dat->hwndStatus, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case CM_UPDATETABCONTROL:
		{
			TabControlData *tcd = (TabControlData*)wParam;
			int tabId = GetTabFromHWND(dat, (HWND)lParam);
			if (tabId >= 0 && tcd != nullptr) {
				wchar_t *ptszTemp = nullptr;

				TCITEM tci;
				tci.mask = 0;
				if (tcd->iFlags & TCDF_TEXT) {
					tci.mask |= TCIF_TEXT;
					tci.pszText = tcd->pszText;
					if (g_dat.flags2 & SMF2_LIMITNAMES) {
						wchar_t *ltext = limitText(tcd->pszText, g_dat.limitNamesLength);
						if (ltext != tcd->pszText)
							tci.pszText = ptszTemp = ltext;
					}
				}
				if (tcd->iFlags & TCDF_ICON) {
					int iconIdx = -1;
					if (tcd->hIcon != nullptr) {
						TCITEM tci2;
						tci2.mask = TCIF_IMAGE;
						TabCtrl_GetItem(dat->hwndTabs, tabId, &tci2);
						iconIdx = AddOrReplaceIcon(g_dat.hTabIconList, tci2.iImage, tcd->hIcon);
					}
					tci.mask |= TCIF_IMAGE;
					tci.iImage = iconIdx;
				}
				TabCtrl_SetItem(dat->hwndTabs, tabId, &tci);
				mir_free(ptszTemp);
			}
		}
		break;

	case DM_SWITCHINFOBAR:
		dat->flags2 ^= SMF2_SHOWINFOBAR;

		for (int i = 0; i < dat->childrenCount; i++) {
			CScriverWindow * pDlg = GetChildFromTab(dat->hwndTabs, i);
			SendMessage(pDlg->GetHwnd(), DM_SWITCHINFOBAR, 0, 0);
		}
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_SWITCHSTATUSBAR:
		dat->flags2 ^= SMF2_SHOWSTATUSBAR;
		ShowWindow(dat->hwndStatus, (dat->flags2 & SMF2_SHOWSTATUSBAR) ? SW_SHOW : SW_HIDE);
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_SWITCHTOOLBAR:
		dat->flags2 ^= SMF2_SHOWTOOLBAR;

		for (int i = 0; i < dat->childrenCount; i++) {
			CScriverWindow * pDlg = GetChildFromTab(dat->hwndTabs, i);
			SendMessage(pDlg->GetHwnd(), DM_SWITCHTOOLBAR, 0, 0);
		}

		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_SWITCHTITLEBAR:
		dat->flags2 ^= SMF2_SHOWTITLEBAR;
		ws = GetWindowLongPtr(hwndDlg, GWL_STYLE) & ~(WS_CAPTION);
		if (dat->flags2 & SMF2_SHOWTITLEBAR)
			ws |= WS_CAPTION;

		SetWindowLongPtr(hwndDlg, GWL_STYLE, ws);
		RECT rc;
		GetWindowRect(hwndDlg, &rc);
		SetWindowPos(hwndDlg, 0, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
		RedrawWindow(hwndDlg, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		break;

	case DM_CASCADENEWWINDOW:
		if ((HWND)wParam != hwndDlg) {
			RECT rcThis, rcNew;
			GetWindowRect(hwndDlg, &rcThis);
			GetWindowRect((HWND)wParam, &rcNew);
			if (abs(rcThis.left - rcNew.left) < 3 && abs(rcThis.top - rcNew.top) < 3) {
				int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
				SetWindowPos((HWND)wParam, 0, rcNew.left + offset, rcNew.top + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				*(int *)lParam = 1;
			}
		}
		break;
	}
	return FALSE;
}

static void DrawTab(ParentWindowData *dat, HWND hwnd, WPARAM, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
	int iTabIndex = lpDIS->itemID;
	if (iTabIndex < 0)
		return;

	TabCtrlData *tcdat = (TabCtrlData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HANDLE hTheme = nullptr;
	int tstate = 0;
	wchar_t szLabel[1024];
	TCITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	tci.pszText = szLabel;
	tci.cchTextMax = _countof(szLabel);
	tci.dwStateMask = TCIS_HIGHLIGHTED;
	if (TabCtrl_GetItem(hwnd, iTabIndex, &tci)) {
		IMAGEINFO info;
		RECT rIcon = lpDIS->rcItem;
		RECT rect = lpDIS->rcItem;
		RECT rectTab = lpDIS->rcItem;
		int bSelected = lpDIS->itemState & ODS_SELECTED;
		int atTop = (GetWindowLongPtr(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;
		UINT dwFormat;
		if (!IsAppThemed())
			FillRect(lpDIS->hDC, &rect, GetSysColorBrush(COLOR_BTNFACE));
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
			DrawThemeBackground(hTheme, lpDIS->hDC, TABP_TABITEM, tstate, &rectTab, nullptr);
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

HWND GetParentWindow(MCONTACT hContact, BOOL bChat)
{
	NewMessageWindowLParam newData = { 0 };
	newData.hContact = hContact;
	newData.isChat = bChat;
	if (g_dat.flags2 & SMF2_USETABS) {
		if (!bChat || !(g_dat.flags2 & SMF2_SEPARATECHATSCONTAINERS)) {
			if (g_dat.lastParent != nullptr) {
				int tabsNum = (int)SendMessage(g_dat.lastParent->hwnd, CM_GETCHILDCOUNT, 0, 0);
				if (!(g_dat.flags2 & SMF2_LIMITTABS) || tabsNum < g_dat.limitTabsNum)
					return g_dat.lastParent->hwnd;
			}
		}
		else {
			if (g_dat.lastChatParent != nullptr) {
				int tabsNum = (int)SendMessage(g_dat.lastChatParent->hwnd, CM_GETCHILDCOUNT, 0, 0);
				if (!(g_dat.flags2 & SMF2_LIMITCHATSTABS) || tabsNum < g_dat.limitChatsTabsNum)
					return g_dat.lastChatParent->hwnd;
			}
		}
	}

	if (!(g_dat.flags2 & SMF2_SEPARATECHATSCONTAINERS))
		newData.isChat = FALSE;

	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGWIN), nullptr, DlgProcParentWindow, (LPARAM)&newData);
}
