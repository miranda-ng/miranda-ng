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

#define TIMERID_FLASHWND 1

#define SB_CHAR_WIDTH		 40
#define SB_SENDING_WIDTH 	 25
#define SB_UNICODE_WIDTH 	 18

#define TIMEOUT_FLASHWND     900

static void DrawTab(ParentWindowData *dat, HWND hwnd, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////

int ParentWindowData::GetChildCount()
{
	return TabCtrl_GetItemCount(m_hwndTabs);
}

void ParentWindowData::GetChildWindowRect(RECT *rcChild)
{
	RECT rc, rcStatus, rcTabs;
	GetClientRect(m_hwnd, &rc);
	GetClientRect(m_hwndTabs, &rcTabs);
	TabCtrl_AdjustRect(m_hwndTabs, FALSE, &rcTabs);
	rcStatus.top = rcStatus.bottom = 0;
	if (flags2.bShowStatusBar)
		GetWindowRect(m_hwndStatus, &rcStatus);

	rcChild->left = 0;
	rcChild->right = rc.right;
	if (flags2.bTabsAtBottom) {
		rcChild->top = 2;
		if ((flags2.bUseTabs && !(flags2.bHideOneTab)) || (m_iChildrenCount > 1))
			rcChild->bottom = rcTabs.bottom + 4;
		else
			rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
	}
	else {
		if ((flags2.bUseTabs && !(flags2.bHideOneTab)) || (m_iChildrenCount > 1))
			rcChild->top = rcTabs.top;
		else
			rcChild->top = 2;

		rcChild->bottom = rc.bottom - rc.top - (rcStatus.bottom - rcStatus.top);
	}
}

int ParentWindowData::GetTabFromHWND(HWND child)
{
	int l = TabCtrl_GetItemCount(m_hwndTabs);
	for (int i = 0; i < l; i++) {
		TCITEM tci = { 0 };
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_hwndTabs, i, &tci);
		CMsgDialog *pDlg = (CMsgDialog *)tci.lParam;
		if (pDlg->GetHwnd() == child)
			return i;
	}
	return -1;
}

static CMsgDialog* GetChildFromTab(HWND m_hwndTabs, int tabId)
{
	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM;
	if (TabCtrl_GetItem(m_hwndTabs, tabId, &tci))
		return (CMsgDialog *)tci.lParam;

	return nullptr;
}

CMsgDialog* ParentWindowData::GetChildFromHWND(HWND hwnd)
{
	int l = TabCtrl_GetItemCount(m_hwndTabs);
	for (int i = 0; i < l; i++) {
		TCITEM tci = { 0 };
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_hwndTabs, i, &tci);
		CMsgDialog *pDlg = (CMsgDialog *)tci.lParam;
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
	GetWindowRect(dat->m_hwnd, &rcWindow);
	dat->GetChildWindowRect(&rc);
	for (int i = 0; i < dat->m_iChildrenCount; i++) {
		CMsgDialog * pDlg = GetChildFromTab(dat->m_hwndTabs, i);
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
	int statusIconNum = GetStatusIconsCount(dat->m_hContact);
	int statwidths[4];
	RECT rc;
	GetClientRect(dat->m_hwnd, &rc);
	statwidths[0] = rc.right - rc.left - SB_CHAR_WIDTH - SB_UNICODE_WIDTH - 2 * (statusIconNum > 0) - statusIconNum * (GetSystemMetrics(SM_CXSMICON) + 2);
	statwidths[1] = rc.right - rc.left - SB_UNICODE_WIDTH - 2 * (statusIconNum > 0) - statusIconNum * (GetSystemMetrics(SM_CXSMICON) + 2);
	statwidths[2] = rc.right - rc.left - SB_UNICODE_WIDTH;
	statwidths[3] = -1;
	SendMessage(dat->m_hwndStatus, SB_SETPARTS, 4, (LPARAM)statwidths);
	SendMessage(dat->m_hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
	SendMessage(dat->m_hwndStatus, SB_SETTEXT, (WPARAM)(SBT_NOBORDERS) | 3, 0);
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

void ParentWindowData::ActivateChild(CMsgDialog *pDlg)
{
	if (pDlg == nullptr)
		return;

	RECT rcChild;
	GetChildWindowRect(&rcChild);
	SetWindowPos(pDlg->GetHwnd(), HWND_TOP, rcChild.left, rcChild.top, rcChild.right - rcChild.left, rcChild.bottom - rcChild.top, 0);

	int i = GetTabFromHWND(pDlg->GetHwnd());
	if (i == -1)
		return;

	m_hContact = pDlg->m_hContact;
	if (pDlg->GetHwnd() != m_hwndActive) {
		HWND hwndPrev = m_hwndActive;
		m_hwndActive = pDlg->GetHwnd();
		SetupStatusBar(this);
		pDlg->UpdateStatusBar();
		pDlg->UpdateTitle();
		SendMessage(m_hwndActive, WM_SIZE, 0, 0);
		ShowWindow(m_hwndActive, SW_SHOWNOACTIVATE);
		pDlg->m_pLog->ScrollToBottom();
		if (hwndPrev != nullptr)
			ShowWindow(hwndPrev, SW_HIDE);
	}
	else SendMessage(m_hwnd, WM_SIZE, 0, 0);

	TabCtrl_SetCurSel(m_hwndTabs, i);
	SendMessage(m_hwndActive, DM_ACTIVATE, WA_ACTIVE, 0);
}

void ParentWindowData::ActivateChildByIndex(int index)
{
	int l = TabCtrl_GetItemCount(m_hwndTabs);
	if (index < l) {
		CMsgDialog *pDlg = GetChildFromTab(m_hwndTabs, index);
		if (pDlg != nullptr)
			ActivateChild(pDlg);
	}
	SetFocus(m_hwndActive);
}

void ParentWindowData::ActivateNextChild(HWND child)
{
	int i = GetTabFromHWND(child);
	int l = TabCtrl_GetItemCount(m_hwndTabs);
	i = (i + 1) % l;
	ActivateChild(GetChildFromTab(m_hwndTabs, i));
	SetFocus(m_hwndActive);
}

void ParentWindowData::ActivatePrevChild(HWND child)
{
	int i = GetTabFromHWND(child);
	int l = TabCtrl_GetItemCount(m_hwndTabs);
	i = (i + l - 1) % l;
	ActivateChild(GetChildFromTab(m_hwndTabs, i));
	SetFocus(m_hwndActive);
}

void ParentWindowData::AddChild(CMsgDialog *pDlg)
{
	TCITEM tci;
	tci.mask = TCIF_PARAM | TCIF_IMAGE | TCIF_TEXT;
	tci.lParam = (LPARAM)pDlg;
	tci.iImage = -1;
	tci.pszText = L"";
	TabCtrl_InsertItem(m_hwndTabs, m_iChildrenCount, &tci);

	m_iChildrenCount++;

	SetWindowPos(pDlg->GetHwnd(), HWND_TOP, childRect.left, childRect.top, childRect.right - childRect.left, childRect.bottom - childRect.top, SWP_HIDEWINDOW);
	SendMessage(m_hwnd, WM_SIZE, 0, 0);

	EnableThemeDialogTexture(pDlg->GetHwnd(), ETDT_ENABLETAB);
}

void ParentWindowData::CloseOtherChilden(CMsgDialog *pChildDlg)
{
	ActivateChild(pChildDlg);
	
	for (int i = m_iChildrenCount - 1; i >= 0; i--) {
		CMsgDialog *pDlg = GetChildFromTab(m_hwndTabs, i);
		if (pDlg != nullptr && pDlg != pChildDlg)
			pDlg->Close();
	}
	
	ActivateChild(pChildDlg);
}

void ParentWindowData::MessageSend(const MessageSendQueueItem &msg)
{
	for (int i = 0; i < m_iChildrenCount; i++) {
		CMsgDialog *pDlg = GetChildFromTab(m_hwndTabs, i);
		if (pDlg != nullptr)
			pDlg->MessageSend(msg);
	}
}

void ParentWindowData::PopupWindow(CMsgDialog *pDlg, bool bIncoming)
{
	EnableWindow(m_hwnd, TRUE);
	if (bIncoming) { /* incoming message */
		if (g_dat.flags.bStayMinimized) {
			if (!IsWindowVisible(m_hwnd))
				ShowWindow(m_hwnd, SW_SHOWMINNOACTIVE);

			if (m_iChildrenCount == 1 || (g_dat.flags2.bSwitchToActive && (IsIconic(m_hwnd) || GetForegroundWindow() != m_hwnd)))
				ActivateChild(pDlg);
		}
		else {
			ShowWindow(m_hwnd, IsIconic(m_hwnd) ? SW_SHOWNORMAL : SW_SHOWNA);

			if (m_iChildrenCount == 1 || (g_dat.flags2.bSwitchToActive && (IsIconic(m_hwnd) || GetForegroundWindow() != m_hwnd)))
				ActivateChild(pDlg);

			SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
	}
	else { /* outgoing message */
		ShowWindow(m_hwnd, IsIconic(m_hwnd) ? SW_SHOWNORMAL : SW_SHOW);
		SetForegroundWindow(m_hwnd);
		SetFocus(pDlg->GetHwnd());
	}
}

void ParentWindowData::RemoveChild(HWND child)
{
	int tab = GetTabFromHWND(child);
	if (tab >= 0) {
		TCITEM tci;
		tci.mask = TCIF_PARAM | TCIF_IMAGE;
		TabCtrl_GetItem(m_hwndTabs, tab, &tci);
		TabCtrl_DeleteItem(m_hwndTabs, tab);
		m_iChildrenCount--;
		if (child == m_hwndActive) {
			if (tab == TabCtrl_GetItemCount(m_hwndTabs)) tab--;
			if (tab >= 0)
				ActivateChild(GetChildFromTab(m_hwndTabs, tab));
			else
				m_hwndActive = nullptr;
		}
		ReleaseIcon(tci.iImage);
	}

	if (m_iChildrenCount != 0)
		SetFocus(m_hwndActive);
	else
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void ParentWindowData::SetContainerWindowStyle()
{
	ShowWindow(m_hwndStatus, (flags2.bShowStatusBar) ? SW_SHOW : SW_HIDE);

	uint32_t ws = GetWindowLongPtr(m_hwnd, GWL_STYLE) & ~(WS_CAPTION);
	if (flags2.bShowTitleBar)
		ws |= WS_CAPTION;

	SetWindowLongPtr(m_hwnd, GWL_STYLE, ws);

	ws = GetWindowLongPtr(m_hwnd, GWL_EXSTYLE)& ~WS_EX_LAYERED;
	ws |= flags2.bUseTransparency ? WS_EX_LAYERED : 0;
	SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, ws);
	if (flags2.bUseTransparency)
		SetLayeredWindowAttributes(m_hwnd, RGB(255, 255, 255), (uint8_t)(255 - g_dat.inactiveAlpha), LWA_ALPHA);

	ws = GetWindowLongPtr(m_hwndTabs, GWL_STYLE) & ~(TCS_BOTTOM | 0x2000);
	if (flags2.bTabsAtBottom)
		ws |= TCS_BOTTOM;

	ws |= 0x2000;
	if (flags2.bTabCloseButton)
		TabCtrl_SetPadding(m_hwndTabs, GetSystemMetrics(SM_CXEDGE) + 12, GetSystemMetrics(SM_CYEDGE) + 1);
	else
		TabCtrl_SetPadding(m_hwndTabs, GetSystemMetrics(SM_CXEDGE) + 4, GetSystemMetrics(SM_CYEDGE) + 1);

	SetWindowLongPtr(m_hwndTabs, GWL_STYLE, ws);

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	SetWindowPos(m_hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
}

void ParentWindowData::StartFlashing()
{
	if ((GetActiveWindow() != m_hwnd || GetForegroundWindow() != m_hwnd)) {
		nFlash = 0;
		SetTimer(m_hwnd, TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
	}
}

void ParentWindowData::ToggleInfoBar()
{
	flags2.bShowInfoBar = !flags2.bShowInfoBar;

	for (int i = 0; i < m_iChildrenCount; i++) {
		CMsgDialog *pDlg = GetChildFromTab(m_hwndTabs, i);
		if (pDlg)
			pDlg->SetDialogToType();
	}
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void ParentWindowData::ToggleStatusBar()
{
	flags2.bShowStatusBar = !flags2.bShowStatusBar;
	ShowWindow(m_hwndStatus, (flags2.bShowStatusBar) ? SW_SHOW : SW_HIDE);
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void ParentWindowData::ToggleTitleBar()
{
	flags2.bShowTitleBar = !flags2.bShowTitleBar;

	uint32_t ws = GetWindowLongPtr(m_hwnd, GWL_STYLE) & ~(WS_CAPTION);
	if (flags2.bShowTitleBar)
		ws |= WS_CAPTION;
	SetWindowLongPtr(m_hwnd, GWL_STYLE, ws);

	RECT rc;
	GetWindowRect(m_hwnd, &rc);
	SetWindowPos(m_hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
	RedrawWindow(m_hwnd, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void ParentWindowData::ToggleToolBar()
{
	flags2.bShowToolBar = !flags2.bShowToolBar;

	for (int i = 0; i < m_iChildrenCount; i++) {
		CMsgDialog *pDlg = GetChildFromTab(m_hwndTabs, i);
		if (pDlg)
			pDlg->SetDialogToType();
	}

	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void ParentWindowData::UpdateStatusBar(const StatusBarData &sbd, HWND hwnd)
{
	if (m_hwndActive == hwnd) {
		if (sbd.iFlags & SBDF_TEXT)
			SendMessage(m_hwndStatus, SB_SETTEXT, sbd.iItem, (LPARAM)sbd.pszText);
		if (sbd.iFlags & SBDF_ICON)
			SendMessage(m_hwndStatus, SB_SETICON, sbd.iItem, (LPARAM)sbd.hIcon);
	}
	RedrawWindow(m_hwndStatus, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}

void ParentWindowData::UpdateTabControl(const TabControlData &tcd, HWND hwnd)
{
	int tabId = GetTabFromHWND(hwnd);
	if (tabId < 0)
		return;

	wchar_t *ptszTemp = nullptr;

	TCITEM tci;
	tci.mask = 0;
	if (tcd.iFlags & TCDF_TEXT) {
		tci.mask |= TCIF_TEXT;
		tci.pszText = tcd.pszText;
		if (g_dat.flags2.bLimitNames) {
			wchar_t *ltext = limitText(tcd.pszText, g_dat.limitNamesLength);
			if (ltext != tcd.pszText)
				tci.pszText = ptszTemp = ltext;
		}
	}
	if (tcd.iFlags & TCDF_ICON) {
		int iconIdx = -1;
		if (tcd.hIcon != nullptr) {
			TCITEM tci2;
			tci2.mask = TCIF_IMAGE;
			TabCtrl_GetItem(m_hwndTabs, tabId, &tci2);
			iconIdx = AddOrReplaceIcon(g_dat.hTabIconList, tci2.iImage, tcd.hIcon);
		}
		tci.mask |= TCIF_IMAGE;
		tci.iImage = iconIdx;
	}
	TabCtrl_SetItem(m_hwndTabs, tabId, &tci);
	mir_free(ptszTemp);
}

void ParentWindowData::UpdateTitleBar(const TitleBarData &tbd, HWND hwnd)
{
	if (m_hwndActive != hwnd)
		return;

	if (tbd.iFlags & TBDF_TEXT) {
		wchar_t oldtitle[256];
		GetWindowText(m_hwnd, oldtitle, _countof(oldtitle));
		if (mir_wstrcmp(tbd.pszText, oldtitle))
			SetWindowText(m_hwnd, tbd.pszText);
	}
	if (tbd.iFlags & TBDF_ICON) {
		SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)tbd.hIcon);
		if (tbd.hIconBig != nullptr)
			SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)tbd.hIconBig);
		if (pTaskbarInterface)
			pTaskbarInterface->SetOverlayIcon(m_hwnd, tbd.hIconNot, L"");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
			CMsgDialog *pDlg = (CMsgDialog *)tci.lParam;
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
					CMsgDialog *pDlg = (CMsgDialog*)tci.lParam;
					if (pDlg != nullptr)
						pDlg->Reattach(GetParent(hwnd));
				}
				else {
					dat->destTab = -1;
					RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
				}
			}
			else if (dat->srcTab >= 0 && g_dat.flags2.bTabCloseButton) {
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

	switch (msg) {
	case WM_INITDIALOG:
		{
			int savePerContact = g_plugin.bSavePerContact && !g_dat.flags2.bUseTabs;
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *)lParam;
			dat = (ParentWindowData *)mir_alloc(sizeof(ParentWindowData));
			dat->m_hContact = newData->hContact;
			dat->nFlash = 0;
			dat->nFlashMax = g_plugin.iFlashCount;
			dat->m_iChildrenCount = 0;
			dat->m_hwnd = hwndDlg;
			dat->mouseLBDown = 0;
			dat->windowWasCascaded = 0;
			dat->bMinimized = 0;
			dat->bVMaximized = 0;
			dat->iSplitterX = g_plugin.getDword("splitterx", -1);
			dat->iSplitterY = g_plugin.getDword("splittery", -1);
			dat->flags2 = g_dat.flags2;
			dat->m_hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, nullptr, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hwndDlg, nullptr, g_plugin.getInst(), nullptr);
			dat->isChat = newData->isChat;
			SendMessage(dat->m_hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
			SetupStatusBar(dat);
			dat->m_hwndTabs = GetDlgItem(hwndDlg, IDC_TABS);
			dat->m_hwndActive = nullptr;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			if (g_dat.hTabIconList != nullptr)
				TabCtrl_SetImageList(dat->m_hwndTabs, g_dat.hTabIconList);

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
			SubclassTabCtrl(dat->m_hwndTabs);

			dat->SetContainerWindowStyle();

			MCONTACT hSContact = savePerContact ? dat->m_hContact : 0;
			dat->bTopmost = g_plugin.bTopmost;
			if (Utils_RestoreWindowPosition(hwndDlg, hSContact, SRMM_MODULE, (newData->isChat && !savePerContact) ? "chat" : "", RWPF_HIDDEN))
				SetWindowPos(hwndDlg, nullptr, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);

			if (!savePerContact && g_plugin.bCascade)
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
			MoveWindow(dat->m_hwndActive, dat->childRect.left, dat->childRect.top, dat->childRect.right - dat->childRect.left, dat->childRect.bottom - dat->childRect.top, TRUE);
		else {
			RECT rcStatus, rcChild, rcWindow, rc;
			SIZE size;
			dat->bMinimized = 0;
			GetClientRect(hwndDlg, &rc);
			GetWindowRect(hwndDlg, &rcWindow);
			rcStatus.top = rcStatus.bottom = 0;
			if (dat->flags2.bShowStatusBar) {
				GetWindowRect(dat->m_hwndStatus, &rcStatus);
				SetupStatusBar(dat);
			}
			MoveWindow(dat->m_hwndTabs, 0, 2, (rc.right - rc.left), (rc.bottom - rc.top) - (rcStatus.bottom - rcStatus.top) - 2, FALSE);
			RedrawWindow(dat->m_hwndTabs, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ERASE);
			GetMinimunWindowSize(dat, &size);
			if ((rcWindow.bottom - rcWindow.top) < size.cy || (rcWindow.right - rcWindow.left) < size.cx) {
				if ((rcWindow.bottom - rcWindow.top) < size.cy)
					rcWindow.bottom = rcWindow.top + size.cy;
				if ((rcWindow.right - rcWindow.left) < size.cx)
					rcWindow.right = rcWindow.left + size.cx;
				MoveWindow(hwndDlg, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
			}
			dat->GetChildWindowRect(&rcChild);
			dat->childRect = rcChild;
			MoveWindow(dat->m_hwndActive, rcChild.left, rcChild.top, rcChild.right - rcChild.left, rcChild.bottom - rcChild.top, TRUE);
			RedrawWindow(GetDlgItem(dat->m_hwndActive, IDC_SRMM_LOG), nullptr, nullptr, RDW_INVALIDATE);
			if (dat->flags2.bShowStatusBar) {
				SendMessage(dat->m_hwndStatus, WM_SIZE, 0, 0);
				RedrawWindow(dat->m_hwndStatus, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
		}
		return FALSE;

	case WM_SETFOCUS:
		if (dat->m_hwndActive != nullptr)
			SetFocus(dat->m_hwndActive);
		return TRUE;

	case WM_CLOSE:
		if (g_dat.flags2.bHideContainer && dat->m_iChildrenCount > 0)
			ShowWindow(hwndDlg, SW_HIDE);
		else
			DestroyWindow(hwndDlg);
		return TRUE;

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dat && dat->m_hwndActive && dis->hwndItem == dat->m_hwndStatus) {
				CMsgDialog *pDlg = dat->GetChildFromHWND(dat->m_hwndActive);
				if (pDlg != nullptr)
					DrawStatusIcons(pDlg->m_hContact, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
			if (dis->hwndItem == dat->m_hwndTabs) {
				DrawTab(dat, dat->m_hwndTabs, wParam, lParam);
				return TRUE;
			}
		}
		return Menu_DrawItem(lParam);

	case WM_COMMAND:
		if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, dat->m_hContact))
			break;

		if (LOWORD(wParam) == IDCANCEL)
			return TRUE;
		break;

	case WM_NOTIFY:
		{
			NMHDR *pNMHDR = (NMHDR*)lParam;
			if (pNMHDR->hwndFrom == dat->m_hwndTabs) {
				switch (pNMHDR->code) {
				case TCN_SELCHANGE:
					{
						TCITEM tci = { 0 };
						int iSel = TabCtrl_GetCurSel(dat->m_hwndTabs);
						tci.mask = TCIF_PARAM;
						if (TabCtrl_GetItem(dat->m_hwndTabs, iSel, &tci)) {
							CMsgDialog *pDlg = (CMsgDialog *)tci.lParam;
							dat->ActivateChild(pDlg);
							SetFocus(dat->m_hwndActive);
						}
					}
					break;

				case NM_RCLICK:
					TCHITTESTINFO thinfo;
					GetCursorPos(&thinfo.pt);

					int x = thinfo.pt.x;
					int y = thinfo.pt.y;
					ScreenToClient(dat->m_hwndTabs, &thinfo.pt);
					int tabId = TabCtrl_HitTest(dat->m_hwndTabs, &thinfo);
					if (tabId != -1) {
						CMsgDialog *pDlg = GetChildFromTab(dat->m_hwndTabs, tabId);
						HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
						HMENU hSubMenu = GetSubMenu(hMenu, 1);
						TranslateMenu(hSubMenu);
						HMENU hUserMenu = (HMENU)SendMessage(pDlg->GetHwnd(), DM_GETCONTEXTMENU, 0, 0);
						if (hUserMenu != nullptr) {
							InsertMenu(hSubMenu, 0, MF_POPUP | MF_BYPOSITION, (UINT_PTR)hUserMenu, TranslateT("User menu"));
							InsertMenu(hSubMenu, 1, MF_SEPARATOR | MF_BYPOSITION, 0, nullptr);
						}
						BOOL menuResult = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, x, y, 0, hwndDlg, nullptr);
						switch (menuResult) {
						case IDM_CLOSETAB:
							SendMessage(pDlg->GetHwnd(), WM_CLOSE, 0, 0);
							break;
						case IDM_CLOSEOTHERTABS:
							dat->CloseOtherChilden(pDlg);
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
			else if (pNMHDR->hwndFrom == dat->m_hwndStatus) {
				switch (pNMHDR->code) {
				case NM_CLICK:
					RECT rc;
					NMMOUSE *nm = (NMMOUSE*)lParam;
					SendMessage(dat->m_hwndStatus, SB_GETRECT, SendMessage(dat->m_hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
					if (nm->pt.x >= rc.left) {
						CMsgDialog *pDlg = dat->GetChildFromHWND(dat->m_hwndActive);
						if (pDlg != nullptr)
							CheckStatusIconClick(pDlg->m_hContact, dat->m_hwndStatus, nm->pt, rc, 2, 0);
					}
					return TRUE;
				}
			}
		}
		break;

	case WM_DROPFILES:
		SendMessage(dat->m_hwndActive, WM_DROPFILES, wParam, lParam);
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
		if (dat->m_hwndStatus && dat->m_hwndStatus == (HWND)wParam) {
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2 = pt;
			ScreenToClient(dat->m_hwndStatus, &pt);

			RECT rc;
			SendMessage(dat->m_hwndStatus, SB_GETRECT, SendMessage(dat->m_hwndStatus, SB_GETPARTS, 0, 0) - 2, (LPARAM)&rc);
			if (pt.x >= rc.left) {
				CMsgDialog *pDlg = dat->GetChildFromHWND(dat->m_hwndActive);
				if (pDlg != nullptr)
					CheckStatusIconClick(pDlg->m_hContact, dat->m_hwndStatus, pt, rc, 2, MBCF_RIGHTBUTTON);
				break;
			}
			else SendMessage(dat->m_hwndActive, WM_CONTEXTMENU, (WPARAM)hwndDlg, 0);
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			uint32_t ws = GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			ws |= dat->flags2.bUseTransparency ? WS_EX_LAYERED : 0;
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, ws);
			if (dat->flags2.bUseTransparency)
				SetLayeredWindowAttributes(hwndDlg, RGB(255, 255, 255), (uint8_t)(255 - g_dat.inactiveAlpha), LWA_ALPHA);
			break;
		}
		if (dat->m_hwndActive != nullptr) {
			dat->ActivateChild(dat->GetChildFromHWND(dat->m_hwndActive));
			g_dat.hFocusWnd = dat->m_hwndActive;
			PostMessage(dat->m_hwndActive, DM_SETFOCUS, 0, msg);
		}
		if (KillTimer(hwndDlg, TIMERID_FLASHWND)) {
			FlashWindow(hwndDlg, FALSE);
			dat->nFlash = 0;
		}

		{
			uint32_t ws = GetWindowLongPtr(hwndDlg, GWL_EXSTYLE) & ~WS_EX_LAYERED;
			ws |= dat->flags2.bUseTransparency ? WS_EX_LAYERED : 0;
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, ws);
		}
		if (dat->flags2.bUseTransparency)
			SetLayeredWindowAttributes(hwndDlg, RGB(255, 255, 255), (uint8_t)(255 - g_dat.activeAlpha), LWA_ALPHA);
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
				dat->bTopmost = false;
			}
			else {
				CheckMenuItem(hMenu, IDM_TOPMOST, MF_BYCOMMAND | MF_CHECKED);
				SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				dat->bTopmost = true;
			}
		}
		break;
	
	case WM_DESTROY:
		for (int i = dat->m_iChildrenCount; i >= 0; i--) {
			TCITEM tci;
			tci.mask = TCIF_PARAM | TCIF_IMAGE;
			if (TabCtrl_GetItem(dat->m_hwndTabs, i, &tci)) {
				ReleaseIcon(tci.iImage);
				TabCtrl_DeleteItem(dat->m_hwndTabs, i);
			}
		}

		g_plugin.setDword("splitterx", dat->iSplitterX);
		g_plugin.setDword("splittery", dat->iSplitterY);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		WindowList_Remove(g_dat.hParentWindowList, hwndDlg);
		{
			int savePerContact = g_plugin.bSavePerContact && !dat->flags2.bUseTabs;
			MCONTACT hContact = (savePerContact) ? dat->m_hContact : 0;
			Utils_SaveWindowPosition(hwndDlg, hContact, SRMM_MODULE, (!savePerContact && dat->isChat) ? "chat" : "");
			g_plugin.bTopmost = dat->bTopmost;
		}

		if (g_dat.lastParent == dat)
			g_dat.lastParent = dat->prev;

		if (g_dat.lastChatParent == dat)
			g_dat.lastChatParent = dat->prev;

		if (dat->prev != nullptr)
			dat->prev->next = dat->next;

		if (dat->next != nullptr)
			dat->next->prev = dat->prev;

		UnsubclassTabCtrl(dat->m_hwndTabs);
		mir_free(dat);
		break;

	case DM_OPTIONSAPPLIED:
		dat->flags2 = g_dat.flags2;
		dat->SetContainerWindowStyle();
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(dat->m_hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW) | 2, 0);
		SetupStatusBar(dat);
		RedrawWindow(dat->m_hwndStatus, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	case DM_CASCADENEWWINDOW:
		if ((HWND)wParam != hwndDlg) {
			RECT rcThis, rcNew;
			GetWindowRect(hwndDlg, &rcThis);
			GetWindowRect((HWND)wParam, &rcNew);
			if (abs(rcThis.left - rcNew.left) < 3 && abs(rcThis.top - rcNew.top) < 3) {
				int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
				SetWindowPos((HWND)wParam, nullptr, rcNew.left + offset, rcNew.top + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
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
	int tstate = 0;
	wchar_t szLabel[1024];
	TCITEM tci;
	tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
	tci.pszText = szLabel;
	tci.cchTextMax = _countof(szLabel);
	tci.dwStateMask = TCIS_HIGHLIGHTED;
	if (!TabCtrl_GetItem(hwnd, iTabIndex, &tci))
		return;

	IMAGEINFO info;
	RECT rIcon = lpDIS->rcItem;
	RECT rect = lpDIS->rcItem;
	RECT rectTab = lpDIS->rcItem;
	int bSelected = lpDIS->itemState & ODS_SELECTED;
	int atTop = (GetWindowLongPtr(hwnd, GWL_STYLE) & TCS_BOTTOM) == 0;
	UINT dwFormat;
	HANDLE hTheme = nullptr;
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
		if (dat->flags2.bTabCloseButton) {
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
		if (dat->flags2.bTabCloseButton) {
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

HWND GetParentWindow(MCONTACT hContact, bool bChat)
{
	NewMessageWindowLParam newData = { 0 };
	newData.hContact = hContact;
	newData.isChat = bChat;
	if (g_dat.flags2.bUseTabs) {
		if (!bChat || !g_dat.flags2.bSeparateChats) {
			if (g_dat.lastParent != nullptr) {
				int tabsNum = g_dat.lastParent->GetChildCount();
				if (!g_dat.flags2.bLimitTabs || tabsNum < g_dat.limitTabsNum)
					return g_dat.lastParent->m_hwnd;
			}
		}
		else {
			if (g_dat.lastChatParent != nullptr) {
				int tabsNum = g_dat.lastChatParent->GetChildCount();
				if (!g_dat.flags2.bLimitChatTabs || tabsNum < g_dat.limitChatsTabsNum)
					return g_dat.lastChatParent->m_hwnd;
			}
		}
	}

	if (!g_dat.flags2.bSeparateChats)
		newData.isChat = false;

	return CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MSGWIN), nullptr, DlgProcParentWindow, (LPARAM)&newData);
}
