/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// a custom tab control, skinable, aero support, single/multi row, button
// tabs support, proper rendering for bottom row tabs and more.

#include "commonheaders.h"

static WNDPROC		OldTabControlClassProc;

#define FIXED_TAB_SIZE 100

// returns the index of the tab under the mouse pointer. Used for
// context menu popup and tooltips
// pt: mouse coordinates, obtained from GetCursorPos()

int TSAPI GetTabItemFromMouse(HWND hwndTab, POINT *pt)
{
	TCHITTESTINFO tch;
	tch.pt = (*pt);
	tch.flags = 0;
	ScreenToClient(hwndTab, &tch.pt);

	return TabCtrl_HitTest(hwndTab, &tch);
}

static int TabCtrl_TestForCloseButton(const TabControlData *tabdat, HWND hwnd, POINT *pt)
{
	int iTab;
	RECT rcTab;
	TCHITTESTINFO tci;

	tci.pt = (*pt);
	tci.flags = 0;
	ScreenToClient(hwnd, &tci.pt);
	iTab = TabCtrl_HitTest(hwnd, &tci);
	if (iTab == -1 || (tci.flags & TCHT_NOWHERE))
		return -1;

	TabCtrl_GetItemRect(hwnd, iTab, &rcTab);
	if (tabdat->dwStyle & TCS_BUTTONS) {
		rcTab.right -= 1;
		rcTab.left = rcTab.right - 18;
	}
	else {
		rcTab.left = rcTab.right - 18;
		rcTab.right -= 5;
	}
	rcTab.bottom -= 4;
	rcTab.top += 4;
	if (PtInRect(&rcTab, tci.pt))
		return (iTab);
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// tabctrl helper function
// Finds leftmost down item.

static UINT FindLeftDownItem(HWND hwnd)
{
	RECT rctLeft = { 100000, 0, 0, 0 }, rctCur;
	int nCount = TabCtrl_GetItemCount(hwnd) - 1;
	UINT nItem = 0;

	for (int i = 0; i < nCount; i++) {
		TabCtrl_GetItemRect(hwnd, i, &rctCur);
		if (rctCur.left > 0 && rctCur.left <= rctLeft.left) {
			if (rctCur.bottom > rctLeft.bottom) {
				rctLeft = rctCur;
				nItem = i;
			}
		}
	}
	return nItem;
}

/////////////////////////////////////////////////////////////////////////////////////////
// tab control color definitions, including the database setting key names

static struct colOptions
{
	UINT defclr;
	char *szKey;
	char *szSkinnedKey;
} tabcolors[] = {
	COLOR_BTNTEXT, "tab_txt_normal", "S_tab_txt_normal",
	COLOR_BTNTEXT, "tab_txt_active", "S_tab_txt_active",
	COLOR_HOTLIGHT, "tab_txt_hottrack", "S_tab_txt_hottrack",
	COLOR_HOTLIGHT, "tab_txt_unread", "S_tab_txt_unread",
	COLOR_3DFACE, "tab_bg_normal", "tab_bg_normal",
	COLOR_3DFACE, "tab_bg_active", "tab_bg_active",
	COLOR_3DFACE, "tab_bg_hottrack", "tab_bg_hottrack",
	COLOR_3DFACE, "tab_bg_unread", "tab_bg_unread",
	0, 0, NULL, NULL
};

/////////////////////////////////////////////////////////////////////////////////////////
// hints for drawing functions

#define HINT_ACTIVATE_RIGHT_SIDE 1
#define HINT_ACTIVE_ITEM 2
#define FLOAT_ITEM_HEIGHT_SHIFT 2
#define ACTIVE_ITEM_HEIGHT_SHIFT 2
#define SHIFT_FROM_CUT_TO_SPIN 4
#define HINT_TRANSPARENT 16
#define HINT_HOTTRACK 32

static void TSAPI DrawCustomTabPage(HDC hdc, RECT& rcClient)
{
	HBRUSH brOld = reinterpret_cast<HBRUSH>(::SelectObject(hdc, CSkin::m_BrushFill));
	HPEN   hPen = ::CreatePen(PS_SOLID, 1, PluginConfig.m_cRichBorders);
	HPEN   hPenOld = reinterpret_cast<HPEN>(::SelectObject(hdc, hPen));
	::Rectangle(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
	::SelectObject(hdc, hPenOld);
	::SelectObject(hdc, brOld);
	::DeleteObject(hPen);
}

void TSAPI FillTabBackground(const HDC hdc, int iStateId, const TWindowData *dat, RECT* rc)
{
	unsigned clrIndex;

	if (dat && dat->mayFlashTab)
		clrIndex = 7;
	else
		clrIndex = (iStateId == PBS_PRESSED ? 5 : (iStateId == PBS_HOT ? 6 : 4));

	if (PluginConfig.tabConfig.colors[clrIndex] != PluginConfig.m_fillColor)
		FillRect(hdc, rc, PluginConfig.tabConfig.m_brushes[clrIndex - 4]);
	else
		CSkin::FillBack(hdc, rc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// draws the item contents (icon and label)
// it obtains the label and icon handle directly from the message window data
// no image list is used and necessary, the message window dialog procedure has to provide a valid
// icon handle in dat->hTabIcon

static void DrawItem(TabControlData *tabdat, HDC dc, RECT *rcItem, int nHint, int nItem, TWindowData *dat)
{
	if (dat == NULL)
		return;

	InflateRect(rcItem, -2, -2);

	unsigned clrIndex = (dat->mayFlashTab) ? 3 : (nHint & HINT_ACTIVE_ITEM ? 1 : (nHint & HINT_HOTTRACK ? 2 : 0));
	COLORREF clr = PluginConfig.tabConfig.colors[clrIndex];

	int oldMode = SetBkMode(dc, TRANSPARENT);

	if (!(tabdat->dwStyle & TCS_BOTTOM))
		OffsetRect(rcItem, 0, 1);

	int iSize = 16;
	HICON hIcon;
	if (dat->dwFlags & MWF_ERRORSTATE)
		hIcon = PluginConfig.g_iconErr;
	else if (dat->mayFlashTab)
		hIcon = dat->iFlashIcon;
	else {
		if (dat->si && dat->iFlashIcon) {
			hIcon = dat->iFlashIcon;

			int sizeY;
			Utils::getIconSize(hIcon, iSize, sizeY);
		}
		else if (dat->hTabIcon == dat->hTabStatusIcon && dat->hXStatusIcon)
			hIcon = dat->hXStatusIcon;
		else
			hIcon = dat->hTabIcon;
	}

	if (dat->mayFlashTab == FALSE || (dat->mayFlashTab == TRUE && dat->bTabFlash != 0) || !(dat->pContainer->dwFlagsEx & TCF_FLASHICON)) {
		DWORD ix = rcItem->left + tabdat->m_xpad - 1;
		DWORD iy = (rcItem->bottom + rcItem->top - iSize) / 2;
		if (dat->dwFlagsEx & MWF_SHOW_ISIDLE && PluginConfig.m_bIdleDetect)
			CSkin::DrawDimmedIcon(dc, ix, iy, iSize, iSize, hIcon, 180);
		else
			DrawIconEx(dc, ix, iy, hIcon, iSize, iSize, 0, NULL, DI_NORMAL | DI_COMPAT);
	}

	rcItem->left += (iSize + 2 + tabdat->m_xpad);

	if (tabdat->fCloseButton) {
		if (tabdat->iHoveredCloseIcon != nItem)
			CSkin::m_default_bf.SourceConstantAlpha = 150;

		GdiAlphaBlend(dc, rcItem->right - 16 - tabdat->m_xpad, (rcItem->bottom + rcItem->top - 16) / 2, 16, 16, CSkin::m_tabCloseHDC,
			0, 0, 16, 16, CSkin::m_default_bf);

		rcItem->right -= (18 + tabdat->m_xpad);
		CSkin::m_default_bf.SourceConstantAlpha = 255;
	}

	if (dat->mayFlashTab == FALSE || (dat->mayFlashTab == TRUE && dat->bTabFlash != 0) || !(dat->pContainer->dwFlagsEx & TCF_FLASHLABEL)) {
		DWORD dwTextFlags = DT_SINGLELINE | DT_VCENTER;
		HFONT oldFont = (HFONT)SelectObject(dc, (HFONT)SendMessage(tabdat->hwnd, WM_GETFONT, 0, 0));
		if (tabdat->dwStyle & TCS_BUTTONS || !(tabdat->dwStyle & TCS_MULTILINE)) {
			rcItem->right -= tabdat->m_xpad;
			dwTextFlags |= DT_WORD_ELLIPSIS;
		}
		CSkin::RenderText(dc, tabdat->dwStyle & TCS_BUTTONS ? tabdat->hThemeButton : tabdat->hTheme, dat->newtitle, rcItem, dwTextFlags, CSkin::m_glowSize, clr);
		SelectObject(dc, oldFont);
	}
	if (oldMode)
		SetBkMode(dc, oldMode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// draws the item rect (the "tab") in *classic* style (no visual themes

static RECT rcTabPage = { 0 };

static void DrawItemRect(TabControlData *tabdat, HDC dc, RECT *rcItem, int nHint, const TWindowData *dat)
{
	POINT pt;
	DWORD dwStyle = tabdat->dwStyle;

	rcItem->bottom -= 1;
	if (rcItem->left < 0)
		return;

	// draw "button style" tabs... raised edge for hottracked, sunken edge for active (pushed)
	// otherwise, they get a normal border
	if (dwStyle & TCS_BUTTONS) {
		BOOL bClassicDraw = (tabdat->m_VisualStyles == FALSE);

		// draw frame controls for button or bottom tabs
		if (dwStyle & TCS_BOTTOM)
			rcItem->top++;

		rcItem->right += 6;
		if (tabdat->fAeroTabs) {
			if (M.isAero()) {
				InflateRect(rcItem, 2, 0);
				FillRect(dc, rcItem, CSkin::m_BrushBack);
			}
			else if (dat) {
				int iStateId = (nHint & HINT_ACTIVE_ITEM ? PBS_PRESSED : 0) | (nHint & HINT_HOTTRACK ? PBS_HOT : 0);

				InflateRect(rcItem, 1, 0);
				FillTabBackground(dc, iStateId, dat, rcItem);
			}
			CSkin::m_switchBarItem->setAlphaFormat(AC_SRC_ALPHA, nHint & HINT_ACTIVE_ITEM ? 255 : 200);
			CSkin::m_switchBarItem->Render(dc, rcItem, true);


			if (nHint & HINT_ACTIVE_ITEM || nHint & HINT_HOTTRACK) {
				RECT rcGlow = *rcItem;

				if (dwStyle & TCS_BOTTOM)
					rcGlow.top++;
				else
					rcGlow.bottom--;

				tabdat->helperGlowItem->setAlphaFormat(AC_SRC_ALPHA, nHint & HINT_ACTIVE_ITEM ? 200 : 150);
				tabdat->helperGlowItem->Render(dc, &rcGlow, true);
			}
		}
		else if (bClassicDraw) {
			if (CSkin::m_skinEnabled) {
				CSkinItem *item = nHint & HINT_ACTIVE_ITEM ? &SkinItems[ID_EXTBKBUTTONSPRESSED] : (nHint & HINT_HOTTRACK ? &SkinItems[ID_EXTBKBUTTONSMOUSEOVER] : &SkinItems[ID_EXTBKBUTTONSNPRESSED]);

				if (!item->IGNORED) {
					CSkin::SkinDrawBG(tabdat->hwnd, tabdat->pContainer->hwnd, tabdat->pContainer, rcItem, dc);
					CSkin::DrawItem(dc, rcItem, item);
				}
				else
					goto b_nonskinned;
			}
			else {
			b_nonskinned:
				if (nHint & HINT_ACTIVE_ITEM)
					DrawEdge(dc, rcItem, EDGE_ETCHED, BF_RECT | BF_SOFT);
				else if (nHint & HINT_HOTTRACK)
					DrawEdge(dc, rcItem, EDGE_BUMP, BF_RECT | BF_MONO | BF_SOFT);
				else
					DrawEdge(dc, rcItem, EDGE_RAISED, BF_RECT | BF_SOFT);
			}
		}
		else {
			if (M.isAero() && !(dwStyle & TCS_BOTTOM))
				FillRect(dc, rcItem, CSkin::m_BrushBack);
			else
				CSkin::FillBack(dc, rcItem);
			DrawThemeBackground(tabdat->hThemeButton, dc, 1, nHint & HINT_ACTIVE_ITEM ? 3 : (nHint & HINT_HOTTRACK ? 2 : 1), rcItem, rcItem);
		}
		return;
	}
	SelectObject(dc, PluginConfig.tabConfig.m_hPenLight);

	if (nHint & HINT_ACTIVE_ITEM) {
		if (dwStyle & TCS_BOTTOM) {
			if (!CSkin::m_skinEnabled)
				CSkin::FillBack(dc, rcItem);
			rcItem->bottom += 2;
		}
		else {
			rcItem->bottom += 2;
			if (!CSkin::m_skinEnabled)
				CSkin::FillBack(dc, rcItem);
			rcItem->bottom--;
			rcItem->top -= 2;
		}
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[dwStyle & TCS_BOTTOM ? ID_EXTBKTABITEMACTIVEBOTTOM : ID_EXTBKTABITEMACTIVE];
			if (!item->IGNORED) {
				rcItem->left += item->MARGIN_LEFT;
				rcItem->right -= item->MARGIN_RIGHT;
				rcItem->top += item->MARGIN_TOP;
				rcItem->bottom -= item->MARGIN_BOTTOM;
				CSkin::DrawItem(dc, rcItem, item);
				return;
			}
		}
	}
	if (CSkin::m_skinEnabled) {
		CSkinItem *item = &SkinItems[dwStyle & TCS_BOTTOM ? (nHint & HINT_HOTTRACK ? ID_EXTBKTABITEMHOTTRACKBOTTOM : ID_EXTBKTABITEMBOTTOM) :
			(nHint & HINT_HOTTRACK ? ID_EXTBKTABITEMHOTTRACK : ID_EXTBKTABITEM)];
		if (!item->IGNORED) {
			if (dwStyle & TCS_BOTTOM) {
				rcItem->top = (rcItem->top > rcTabPage.bottom + 5) ? (rcItem->top-1) : rcItem->top;
				rcItem->top--;
			}
			else
				rcItem->bottom++;

			rcItem->left += item->MARGIN_LEFT;
			rcItem->right -= item->MARGIN_RIGHT;
			CSkin::DrawItem(dc, rcItem, item);
			return;
		}
	}
	if (dwStyle & TCS_BOTTOM) {
		MoveToEx(dc, rcItem->left, rcItem->top - (nHint & HINT_ACTIVE_ITEM ? 1 : 0), &pt);
		LineTo(dc, rcItem->left, rcItem->bottom - 2);
		LineTo(dc, rcItem->left + 2, rcItem->bottom);
		SelectObject(dc, PluginConfig.tabConfig.m_hPenShadow);
		LineTo(dc, rcItem->right - 3, rcItem->bottom);

		LineTo(dc, rcItem->right - 1, rcItem->bottom - 2);
		LineTo(dc, rcItem->right - 1, rcItem->top - 1);
		MoveToEx(dc, rcItem->right - 2, rcItem->top, &pt);
		SelectObject(dc, PluginConfig.tabConfig.m_hPenItemShadow);
		LineTo(dc, rcItem->right - 2, rcItem->bottom - 1);
		MoveToEx(dc, rcItem->right - 3, rcItem->bottom - 1, &pt);
		LineTo(dc, rcItem->left + 2, rcItem->bottom - 1);
	}
	else {
		MoveToEx(dc, rcItem->left, rcItem->bottom, &pt);
		LineTo(dc, rcItem->left, rcItem->top + 2);
		LineTo(dc, rcItem->left + 2, rcItem->top);
		LineTo(dc, rcItem->right - 2, rcItem->top);
		SelectObject(dc, PluginConfig.tabConfig.m_hPenItemShadow);

		MoveToEx(dc, rcItem->right - 2, rcItem->top + 1, &pt);
		LineTo(dc, rcItem->right - 2, rcItem->bottom + 1);
		SelectObject(dc, PluginConfig.tabConfig.m_hPenShadow);
		MoveToEx(dc, rcItem->right - 1, rcItem->top + 2, &pt);
		LineTo(dc, rcItem->right - 1, rcItem->bottom + 1);
	}
}

static int DWordAlign(int n)
{
	int rem = n % 4;
	if (rem)
		n += (4 - rem);
	return n;
}

static HRESULT DrawThemesPartWithAero(const TabControlData *tabdat, HDC hDC, int iPartId, int iStateId, LPRECT prcBox, TWindowData *dat)
{
	HRESULT hResult = 0;
	bool	bAero = M.isAero();

	if (tabdat->fAeroTabs) {
		if (tabdat->dwStyle & TCS_BOTTOM)
			prcBox->top += (bAero ? 2 : iStateId == PBS_PRESSED ? (M.isVSThemed() ? 1 : -1) : 0);
		else if (!bAero)
			prcBox->bottom -= (iStateId == PBS_PRESSED ? (M.isVSThemed() ? 1 : -1) : 0);

		if (bAero)
			FillRect(hDC, prcBox, CSkin::m_BrushBack);
		else if (dat)
			FillTabBackground(hDC, iStateId, dat, prcBox);

		tabdat->helperItem->setAlphaFormat(AC_SRC_ALPHA, iStateId == PBS_PRESSED ? 255 : (bAero ? 240 : 255));
		tabdat->helperItem->Render(hDC, prcBox, true);
		tabdat->helperGlowItem->setAlphaFormat(AC_SRC_ALPHA, iStateId == PBS_PRESSED ? 220 : 180);

		if (iStateId != PBS_NORMAL)
			tabdat->helperGlowItem->Render(hDC, prcBox, true);
	}
	else if (tabdat->hTheme != 0) {
		hResult = DrawThemeBackground(tabdat->hTheme, hDC, iPartId, iStateId, prcBox, NULL);
	}

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// draws a theme part (identifier in uiPartNameID) using the given clipping rectangle

static HRESULT DrawThemesPart(const TabControlData *tabdat, HDC hDC, int iPartId, int iStateId, LPRECT prcBox)
{
	HRESULT hResult = 0;

	if (tabdat->hTheme != 0)
		hResult = DrawThemeBackground(tabdat->hTheme, hDC, iPartId, iStateId, prcBox, NULL);

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// draw a themed tab item. either a tab or the body pane
// handles image mirroring for tabs at the bottom

static void DrawThemesXpTabItem(HDC pDC, RECT *rcItem, UINT uiFlag, TabControlData *tabdat, TWindowData *dat)
{
	BOOL bBody = (uiFlag & 1) ? TRUE : FALSE;
	BOOL bSel = (uiFlag & 2) ? TRUE : FALSE;
	BOOL bHot = (uiFlag & 4) ? TRUE : FALSE;
	BOOL bBottom = (uiFlag & 8) ? TRUE : FALSE;	// mirror
	LPBYTE pcImg = NULL;
	int nStart = 0, nLenSub = 0;

	SIZE szBmp;
	szBmp.cx = rcItem->right - rcItem->left;
	szBmp.cy = rcItem->bottom - rcItem->top;

	// for top row tabs, it's easy. Just draw to the provided dc (it's a mem dc already)
	if (!bBottom) {
		if (bBody) {
			if (PluginConfig.m_bIsVista) {
				rcItem->right += 2; // hide right tab sheet shadow (only draw the actual border line)
				rcItem->bottom += 1;
			}
			DrawThemesPart(tabdat, pDC, 9, 0, rcItem);	// TABP_PANE id = 9
		}
		else {
			int iStateId = bSel ? 3 : (bHot ? 2 : 1); // leftmost item has different part id
			DrawThemesPartWithAero(tabdat, pDC, rcItem->left < 20 ? 2 : 1, iStateId, rcItem, dat);
		}
		return;
	}
	else if (tabdat->fAeroTabs && !bBody) {
		int iStateId = bSel ? 3 : (bHot ? 2 : 1); // leftmost item has different part id
		DrawThemesPartWithAero(tabdat, pDC, rcItem->left < 20 ? 2 : 1, iStateId, rcItem, dat);
		return;
	}

	// remaining code is for bottom tabs only.
	HDC dcMem = CreateCompatibleDC(pDC);
	HBITMAP bmpMem = CreateCompatibleBitmap(pDC, szBmp.cx, szBmp.cy);
	HBITMAP pBmpOld = (HBITMAP)SelectObject(dcMem, bmpMem);

	RECT rcMem;
	rcMem.left = rcMem.top = 0;
	rcMem.right = szBmp.cx;
	rcMem.bottom = szBmp.cy;

	BITMAPINFO biOut;
	memset(&biOut, 0, sizeof(BITMAPINFO));	// Fill local pixel arrays

	BITMAPINFOHEADER *bihOut = &biOut.bmiHeader;
	bihOut->biSize = sizeof(BITMAPINFOHEADER);
	bihOut->biCompression = BI_RGB;
	bihOut->biPlanes = 1;
	bihOut->biBitCount = 24;	// force as RGB: 3 bytes, 24 bits
	bihOut->biWidth = szBmp.cx;
	bihOut->biHeight = szBmp.cy;

	int nBmpWdtPS = DWordAlign(szBmp.cx * 3);
	int nSzBuffPS = ((nBmpWdtPS * szBmp.cy) / 8 + 2) * 8;

	// blit the background to the memory dc, so that transparent tabs will draw properly
	// for bottom tabs, it's more complex, because the background part must not be mirrored
	// the body part does not need that (filling with the background color is much faster
	// and sufficient for the tab "page" part.
	if (!bSel)
		CSkin::FillBack(dcMem, &rcMem);
	else {
		// mirror the background horizontally for bottom selected tabs (they can overwrite others)
		// needed, because after drawing the theme part the images will again be mirrored
		// to "flip" the tab item.
		BitBlt(dcMem, 0, 0, szBmp.cx, szBmp.cy, pDC, rcItem->left, rcItem->top, SRCCOPY);

		BYTE *pcImg1 = (BYTE*)mir_alloc(nSzBuffPS);
		if (pcImg1) {
			GetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg1, &biOut, DIB_RGB_COLORS);
			bihOut->biHeight = -szBmp.cy; 				// to mirror bitmap is eough to use negative height between Get/SetDIBits
			SetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg1, &biOut, DIB_RGB_COLORS);
			mir_free(pcImg1);
		}
	}

	// body may be *large* so rotating the final image can be very slow.
	// workaround: draw the skin item (tab pane) into a small dc, rotate this (small) image and render
	// it to the final DC with the IMG_RenderItem() routine.
	if (bBody) {
		HDC hdcTemp = CreateCompatibleDC(pDC);
		HBITMAP hbmTemp = CreateCompatibleBitmap(pDC, 100, 50);
		HBITMAP hbmTempOld = (HBITMAP)SelectObject(hdcTemp, hbmTemp);
		RECT rcTemp = { 0 };

		rcTemp.right = 100;
		rcTemp.bottom = 50;

		bihOut->biWidth = 100;
		bihOut->biHeight = 50;

		nBmpWdtPS = DWordAlign(100 * 3);
		nSzBuffPS = ((nBmpWdtPS * 50) / 8 + 2) * 8;

		CSkin::FillBack(hdcTemp, &rcTemp);
		DrawThemesPart(tabdat, hdcTemp, 9, 0, &rcTemp);	// TABP_PANE id = 9
		pcImg = (BYTE *)mir_alloc(nSzBuffPS);
		if (pcImg) {									// get bits:
			GetDIBits(hdcTemp, hbmTemp, nStart, 50 - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
			bihOut->biHeight = -50;
			SetDIBits(hdcTemp, hbmTemp, nStart, 50 - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
			mir_free(pcImg);
		}
		CImageItem tempItem(10, 10, 10, 10, hdcTemp, 0, IMAGE_FLAG_DIVIDED | IMAGE_FILLSOLID,
			GetSysColorBrush(COLOR_3DFACE), 255, 30, 80, 50, 100);

		if (PluginConfig.m_bIsVista) // hide right tab sheet shadow (only draw the actual border line)
			rcItem->right += 2;

		tempItem.Render(pDC, rcItem, true);
		tempItem.Clear();
		SelectObject(hdcTemp, hbmTempOld);
		DeleteObject(hbmTemp);
		DeleteDC(hdcTemp);

		SelectObject(dcMem, pBmpOld);
		DeleteObject(bmpMem);
		DeleteDC(dcMem);
		return;
	}
	else {
		int iStateId = bSel ? 3 : (bHot ? 2 : 1);
		DrawThemesPart(tabdat, dcMem, rcItem->left < 20 ? 2 : 1, iStateId, &rcMem);
	}

	bihOut->biHeight = szBmp.cy;
	pcImg = (BYTE *)mir_alloc(nSzBuffPS);

	if (pcImg) {									// get bits:
		GetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
		bihOut->biHeight = -szBmp.cy;
		SetDIBits(pDC, bmpMem, nStart, szBmp.cy - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
		mir_free(pcImg);
	}

	// finally, blit the result to the destination dc
	BitBlt(pDC, rcItem->left, rcItem->top, szBmp.cx, szBmp.cy, dcMem, 0, 0, SRCCOPY);
	SelectObject(dcMem, pBmpOld);
	DeleteObject(bmpMem);
	DeleteDC(dcMem);
}

static POINT ptMouseT = { 0 };

static void PaintWorker(HWND hwnd, TabControlData *tabdat)
{
	if (tabdat == NULL || tabdat->pContainer == NULL)
		return;

	PAINTSTRUCT ps;
	HDC hdc;
	RECT rectTemp, rctPage, rctActive, rcItem, rctClip, rctOrig;
	RECT rectUpDn = { 0, 0, 0, 0 };
	int nCount = TabCtrl_GetItemCount(hwnd), i;
	TCITEM item = { 0 };
	int iActive, hotItem;
	POINT pt;
	DWORD dwStyle = tabdat->dwStyle;
	UINT uiFlags = 1;
	UINT uiBottom = 0;
	bool  isAero = M.isAero();
	HANDLE hpb = 0;
	BOOL bClassicDraw = !isAero && (tabdat->m_VisualStyles == FALSE || CSkin::m_skinEnabled);

	if (GetUpdateRect(hwnd, NULL, TRUE) == 0)
		return;

	GetCursorPos(&pt);
	hotItem = GetTabItemFromMouse(hwnd, &pt);
	if (tabdat->iHoveredTabIndex != hotItem)
		InvalidateRect(hwnd, NULL, FALSE);
	tabdat->iHoveredTabIndex = hotItem;

	item.mask = TCIF_PARAM;

	tabdat->fAeroTabs = (CSkin::m_fAeroSkinsValid && (isAero || PluginConfig.m_fillColor)) ? TRUE : FALSE;
	tabdat->fCloseButton = (tabdat->pContainer->dwFlagsEx & TCF_CLOSEBUTTON ? TRUE : FALSE);
	tabdat->helperDat = 0;

	if (tabdat->fAeroTabs) {
		TWindowData *dat = (TWindowData*)GetWindowLongPtr(tabdat->pContainer->hwndActive, GWLP_USERDATA);
		if (dat)
			tabdat->helperDat = dat;
		else
			tabdat->fAeroTabs = FALSE;

		tabdat->helperItem = (dwStyle & TCS_BOTTOM) ? CSkin::m_tabBottom : CSkin::m_tabTop;
		tabdat->helperGlowItem = (dwStyle & TCS_BOTTOM) ? CSkin::m_tabGlowBottom : CSkin::m_tabGlowTop;
	}
	else tabdat->fAeroTabs = FALSE;

	HDC hdcreal = BeginPaint(hwnd, &ps);

	// switchbar is active, don't paint a single pixel, the tab control won't be visible at all
	// same when we have only ONE tab and do not want it to be visible because of the container
	// option "Show tab bar only when needed".
	if ((tabdat->pContainer->dwFlags & CNT_SIDEBAR) || (nCount == 1 && tabdat->pContainer->dwFlags & CNT_HIDETABS)) {
		if (nCount == 0)
			FillRect(hdcreal, &ps.rcPaint, GetSysColorBrush(COLOR_3DFACE)); // avoid flickering/ugly black background during container creation
		EndPaint(hwnd, &ps);
		return;
	}

	GetClientRect(hwnd, &rctPage);
	rctOrig = rctPage;
	iActive = TabCtrl_GetCurSel(hwnd);
	TabCtrl_GetItemRect(hwnd, iActive, &rctActive);
	int cx = rctPage.right - rctPage.left;
	int cy = rctPage.bottom - rctPage.top;

	// draw everything to a memory dc to avoid flickering
	HBITMAP bmpMem, bmpOld;
	if (CMimAPI::m_haveBufferedPaint) {
		hpb = tabdat->hbp = CSkin::InitiateBufferedPaint(hdcreal, rctPage, hdc);
		bmpMem = bmpOld = 0;
	}
	else {
		hdc = CreateCompatibleDC(hdcreal);
		bmpMem = tabdat->fAeroTabs ? CSkin::CreateAeroCompatibleBitmap(rctPage, hdcreal) : CreateCompatibleBitmap(hdcreal, cx, cy);
		bmpOld = (HBITMAP)SelectObject(hdc, bmpMem);
	}

	if (nCount == 1 && tabdat->pContainer->dwFlags & CNT_HIDETABS)
		rctClip = rctPage;

	if (CSkin::m_skinEnabled)
		CSkin::SkinDrawBG(hwnd, tabdat->pContainer->hwnd, tabdat->pContainer, &rctPage, hdc);
	else
		CSkin::FillBack(hdc, &rctPage);

	if (dwStyle & TCS_BUTTONS) {
		RECT rc1;
		TabCtrl_GetItemRect(hwnd, nCount - 1, &rc1);
		if (dwStyle & TCS_BOTTOM) {
			rctPage.bottom = rc1.top;
			uiBottom = 8;
		}
		else {
			rctPage.top = rc1.bottom + 2;
			uiBottom = 0;
		}
	}
	else {
		if (dwStyle & TCS_BOTTOM) {
			rctPage.bottom = rctActive.top;
			uiBottom = 8;
		}
		else {
			rctPage.top = rctActive.bottom;
			uiBottom = 0;
		}
	}

	if (nCount > 1 || !(tabdat->pContainer->dwFlags & CNT_HIDETABS)) {
		rctClip = rctPage;
		InflateRect(&rctClip, -tabdat->pContainer->tBorder, -tabdat->pContainer->tBorder);
	}
	else memset(&rctClip, 0, sizeof(RECT));

	HPEN hPenOld = (HPEN)SelectObject(hdc, PluginConfig.tabConfig.m_hPenLight);

	// visual style support
	CopyRect(&rcTabPage, &rctPage);
	if (!tabdat->bRefreshWithoutClip)
		ExcludeClipRect(hdc, rctClip.left, rctClip.top, rctClip.right, rctClip.bottom);
	else
		memset(&rctClip, 0, sizeof(RECT));
	if ((!bClassicDraw || PluginConfig.m_fillColor) && IntersectRect(&rectTemp, &rctPage, &ps.rcPaint) && !CSkin::m_skinEnabled) {
		RECT rcClient = rctPage;
		if (dwStyle & TCS_BOTTOM) {
			rcClient.bottom = rctPage.bottom;
			uiFlags |= uiBottom;
		}
		else
			rcClient.top = rctPage.top;
		if (PluginConfig.m_fillColor)
			DrawCustomTabPage(hdc, rcClient);
		else
			DrawThemesXpTabItem(hdc, &rcClient, uiFlags, tabdat, 0);	// TABP_PANE=9,0,'TAB'
		if (tabdat->bRefreshWithoutClip)
			goto skip_tabs;
	}
	else if (IntersectRect(&rectTemp, &rctPage, &ps.rcPaint)) {
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKTABPAGE];
			if (!item->IGNORED) {
				DrawAlpha(hdc, &rctPage, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
				goto page_done;
			}
		}

		if (tabdat->bRefreshWithoutClip)
			goto skip_tabs;

		if (dwStyle & TCS_BUTTONS) {
			rectTemp = rctPage;
			if (dwStyle & TCS_BOTTOM) {
				rectTemp.top--;
				rectTemp.bottom--;
			}
			else {
				rectTemp.bottom--;
				rectTemp.top++;
			}
			if (PluginConfig.m_fillColor)
				DrawCustomTabPage(hdc, rectTemp);
			else {
				MoveToEx(hdc, rectTemp.left, rectTemp.bottom, &pt);
				LineTo(hdc, rectTemp.left, rectTemp.top + 1);
				LineTo(hdc, rectTemp.right - 1, rectTemp.top + 1);
				SelectObject(hdc, PluginConfig.tabConfig.m_hPenShadow);
				LineTo(hdc, rectTemp.right - 1, rectTemp.bottom);
				LineTo(hdc, rectTemp.left, rectTemp.bottom);
			}
		}
		else {
			rectTemp = rctPage;
			if (PluginConfig.m_fillColor)
				DrawCustomTabPage(hdc, rectTemp);
			else {
				MoveToEx(hdc, rectTemp.left, rectTemp.bottom - 1, &pt);
				LineTo(hdc, rectTemp.left, rectTemp.top);

				if (dwStyle & TCS_BOTTOM) {
					LineTo(hdc, rectTemp.right - 1, rectTemp.top);
					SelectObject(hdc, PluginConfig.tabConfig.m_hPenShadow);
					LineTo(hdc, rectTemp.right - 1, rectTemp.bottom - 1);
					LineTo(hdc, rctActive.right, rectTemp.bottom - 1);
					MoveToEx(hdc, rctActive.left - 2, rectTemp.bottom - 1, &pt);
					LineTo(hdc, rectTemp.left - 1, rectTemp.bottom - 1);
					SelectObject(hdc, PluginConfig.tabConfig.m_hPenItemShadow);
					MoveToEx(hdc, rectTemp.right - 2, rectTemp.top + 1, &pt);
					LineTo(hdc, rectTemp.right - 2, rectTemp.bottom - 2);
					LineTo(hdc, rctActive.right, rectTemp.bottom - 2);
					MoveToEx(hdc, rctActive.left - 2, rectTemp.bottom - 2, &pt);
					LineTo(hdc, rectTemp.left, rectTemp.bottom - 2);
				}
				else {
					if (rctActive.left >= 0) {
						LineTo(hdc, rctActive.left, rctActive.bottom);
						if (IsRectEmpty(&rectUpDn))
							MoveToEx(hdc, rctActive.right, rctActive.bottom, &pt);
						else {
							if (rctActive.right >= rectUpDn.left)
								MoveToEx(hdc, rectUpDn.left - SHIFT_FROM_CUT_TO_SPIN + 2, rctActive.bottom + 1, &pt);
							else
								MoveToEx(hdc, rctActive.right, rctActive.bottom, &pt);
						}
						LineTo(hdc, rectTemp.right - 2, rctActive.bottom);
					}
					else {
						RECT rectItemLeftmost;
						UINT nItemLeftmost = FindLeftDownItem(hwnd);
						TabCtrl_GetItemRect(hwnd, nItemLeftmost, &rectItemLeftmost);
						LineTo(hdc, rectTemp.right - 2, rctActive.bottom);
					}
					SelectObject(hdc, PluginConfig.tabConfig.m_hPenItemShadow);
					LineTo(hdc, rectTemp.right - 2, rectTemp.bottom - 2);
					LineTo(hdc, rectTemp.left, rectTemp.bottom - 2);

					SelectObject(hdc, PluginConfig.tabConfig.m_hPenShadow);
					MoveToEx(hdc, rectTemp.right - 1, rctActive.bottom, &pt);
					LineTo(hdc, rectTemp.right - 1, rectTemp.bottom - 1);
					LineTo(hdc, rectTemp.left - 2, rectTemp.bottom - 1);
				}
			}
		}
	}
page_done:
	// if aero is active _and_ the infopanel is visible in the current window, we "flatten" out the top area
	// of the tab page by overpainting it black (thus it will appear transparent)
	if (isAero && tabdat->helperDat) {
		RECT rcLog, rcPage;
		GetClientRect(hwnd, &rcPage);
		if (dwStyle & TCS_BOTTOM) {
			GetWindowRect(tabdat->helperDat->hwnd, &rcLog);
			pt.y = rcLog.bottom;
			pt.x = rcLog.left;
			ScreenToClient(hwnd, &pt);
			rcPage.top = pt.y + ((nCount > 1 || !(tabdat->helperDat->pContainer->dwFlags & CNT_HIDETABS)) ? tabdat->helperDat->pContainer->tBorder : 0);
			FillRect(hdc, &rcPage, CSkin::m_BrushBack);
			rcPage.top = 0;
		}
		GetWindowRect(GetDlgItem(tabdat->helperDat->hwnd, tabdat->helperDat->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG), &rcLog);

		pt.y = rcLog.top;
		pt.x = rcLog.left;
		ScreenToClient(hwnd, &pt);
		rcPage.bottom = pt.y;
		FillRect(hdc, &rcPage, CSkin::m_BrushBack);
	}

	uiFlags = 0;

	// figure out hottracked item (if any)
	if (tabdat->bRefreshWithoutClip)
		goto skip_tabs;

	for (i = 0; i < nCount; i++) {
		TWindowData *dat = 0;

		if (i == iActive)
			continue;
		TabCtrl_GetItem(hwnd, i, &item);
		if (item.lParam)
			dat = (TWindowData*)GetWindowLongPtr((HWND)item.lParam, GWLP_USERDATA);
		TabCtrl_GetItemRect(hwnd, i, &rcItem);
		if (!bClassicDraw && uiBottom) {
			rcItem.top -= PluginConfig.tabConfig.m_bottomAdjust;
			rcItem.bottom -= PluginConfig.tabConfig.m_bottomAdjust;
		}
		if (IntersectRect(&rectTemp, &rcItem, &ps.rcPaint) || bClassicDraw) {
			int nHint = 0;
			if (!bClassicDraw && !(dwStyle & TCS_BUTTONS)) {
				DrawThemesXpTabItem(hdc, &rcItem, uiFlags | uiBottom | (i == hotItem ? 4 : 0), tabdat, dat);
				DrawItem(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0), i, dat);
			}
			else {
				if (tabdat->fAeroTabs && !CSkin::m_skinEnabled && !(dwStyle & TCS_BUTTONS))
					DrawThemesPartWithAero(tabdat, hdc, 0, (i == hotItem ? PBS_HOT : PBS_NORMAL), &rcItem, dat);
				else
					DrawItemRect(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0), dat);
				DrawItem(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0), i, dat);
			}
		}
	}

	// draw the active item
	if (!bClassicDraw && uiBottom) {
		rctActive.top -= PluginConfig.tabConfig.m_bottomAdjust;
		rctActive.bottom -= PluginConfig.tabConfig.m_bottomAdjust;
	}
	if (rctActive.left >= 0) {
		TWindowData *dat = 0;
		int nHint = 0;

		rcItem = rctActive;
		TabCtrl_GetItem(hwnd, iActive, &item);
		if (item.lParam)
			dat = (TWindowData*)GetWindowLongPtr((HWND)item.lParam, GWLP_USERDATA);

		if (!bClassicDraw && !(dwStyle & TCS_BUTTONS)) {
			InflateRect(&rcItem, 2, 2);
			DrawThemesXpTabItem(hdc, &rcItem, 2 | uiBottom, tabdat, dat);
			DrawItem(tabdat, hdc, &rcItem, nHint | HINT_ACTIVE_ITEM, iActive, dat);
		}
		else {
			if (!(dwStyle & TCS_BUTTONS)) {
				if (iActive == 0) {
					rcItem.right += 2;
					rcItem.left--;
				}
				else InflateRect(&rcItem, 2, 0);
			}
			if (tabdat->fAeroTabs && !CSkin::m_skinEnabled && !(dwStyle & TCS_BUTTONS)) {
				if (dwStyle & TCS_BOTTOM)
					rcItem.bottom += 2;
				else
					rcItem.top -= 2;
				DrawThemesPartWithAero(tabdat, hdc, 0, PBS_PRESSED, &rcItem, dat);
			}
			else DrawItemRect(tabdat, hdc, &rcItem, HINT_ACTIVATE_RIGHT_SIDE | HINT_ACTIVE_ITEM | nHint, dat);

			DrawItem(tabdat, hdc, &rcItem, HINT_ACTIVE_ITEM | nHint, iActive, dat);
		}
	}
skip_tabs:
	if (hPenOld)
		SelectObject(hdc, hPenOld);

	// finally, bitblt the contents of the memory dc to the real dc
	if (!tabdat->bRefreshWithoutClip)
		ExcludeClipRect(hdcreal, rctClip.left, rctClip.top, rctClip.right, rctClip.bottom);

	if (hpb)
		CSkin::FinalizeBufferedPaint(hpb, &rctOrig);
	else {
		BitBlt(hdcreal, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);
		SelectObject(hdc, bmpOld);
		DeleteObject(bmpMem);
		DeleteDC(hdc);
	}
	EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK TabControlSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	TabControlData *tabdat = (TabControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (tabdat) {
		if (tabdat->pContainer == NULL)
			tabdat->pContainer = (TContainerData*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
		tabdat->dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	}

	switch (msg) {
	case WM_NCCREATE:
	{
		WNDCLASSEX wcl = { 0 };
		wcl.cbSize = sizeof(wcl);
		GetClassInfoEx(g_hInst, _T("SysTabControl32"), &wcl);
		OldTabControlClassProc = wcl.lpfnWndProc;

		tabdat = (TabControlData*)mir_calloc(sizeof(TabControlData));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)tabdat);
		tabdat->hwnd = hwnd;
		tabdat->cx = GetSystemMetrics(SM_CXSMICON);
		tabdat->cy = GetSystemMetrics(SM_CYSMICON);
		tabdat->fTipActive = FALSE;
		tabdat->iHoveredTabIndex = -1;
		tabdat->iHoveredCloseIcon = -1;
		SendMessage(hwnd, EM_THEMECHANGED, 0, 0);
	}
	return TRUE;

	case EM_THEMECHANGED:
		tabdat->m_xpad = M.GetByte("x-pad", 3);
		tabdat->m_VisualStyles = FALSE;
		if (IsThemeActive()) {
			tabdat->m_VisualStyles = TRUE;
			if (tabdat->hTheme != 0) {
				CloseThemeData(tabdat->hTheme);
				CloseThemeData(tabdat->hThemeButton);
			}
			if ((tabdat->hTheme = OpenThemeData(hwnd, L"TAB")) == 0 || (tabdat->hThemeButton = OpenThemeData(hwnd, L"BUTTON")) == 0)
				tabdat->m_VisualStyles = FALSE;
		}
		return 0;

	case EM_SEARCHSCROLLER:
		// search the updown control (scroll arrows) to subclass it...
		// the control is dynamically created and may not exist as long as it is
		// not needed. So we have to search it everytime we need to paint. However,
		// it is sufficient to search it once. So this message is called, whenever
		// a new tab is inserted
		HWND hwndChild;
		if ((hwndChild = FindWindowEx(hwnd, 0, _T("msctls_updown32"), NULL)) != 0)
			DestroyWindow(hwndChild);

		return 0;

	case EM_VALIDATEBOTTOM:
		if ((tabdat->dwStyle & TCS_BOTTOM) && tabdat->m_VisualStyles != 0 && PluginConfig.tabConfig.m_bottomAdjust != 0)
			InvalidateRect(hwnd, NULL, FALSE);
		break;

	case EM_REFRESHWITHOUTCLIP:
		if (TabCtrl_GetItemCount(hwnd) > 1)
			return 0;

		tabdat->bRefreshWithoutClip = TRUE;
		RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW | RDW_NOCHILDREN | RDW_INVALIDATE);
		tabdat->bRefreshWithoutClip = FALSE;
		return 0;

	case TCM_INSERTITEM:
	case TCM_DELETEITEM:
		tabdat->iHoveredTabIndex = -1;
		tabdat->iHoveredCloseIcon = -1;
		if (!(tabdat->dwStyle & TCS_MULTILINE) || (tabdat->dwStyle & TCS_BUTTONS)) {
			int iTabs = TabCtrl_GetItemCount(hwnd);
			if (iTabs >= 1 && msg == TCM_INSERTITEM) {
				RECT rc;
				TabCtrl_GetItemRect(hwnd, 0, &rc);
				TabCtrl_SetItemSize(hwnd, 10, rc.bottom - rc.top);
			}
			LRESULT result = CallWindowProc(OldTabControlClassProc, hwnd, msg, wParam, lParam);
			SendMessage(hwnd, WM_SIZE, 0, 0);
			return result;
		}
		break;

	case WM_DESTROY:
		if (tabdat) {
			if (tabdat->hTheme != 0) {
				CloseThemeData(tabdat->hTheme);
				CloseThemeData(tabdat->hThemeButton);
			}
		}
		break;

	case WM_NCDESTROY:
		if (tabdat) {
			mir_free(tabdat);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		}
		break;

	case WM_MBUTTONDOWN:
		GetCursorPos(&pt);
		SendMessage(GetParent(hwnd), DM_CLOSETABATMOUSE, 0, (LPARAM)&pt);
		return 1;

	case WM_SETCURSOR:
		GetCursorPos(&pt);
		InvalidateRect(hwnd, NULL, FALSE); /* wine: fix for erase/paint tab on mouse enter/leave tab. */
		SendMessage(GetParent(hwnd), msg, wParam, lParam);
		if (abs(pt.x - ptMouseT.x) < 4 && abs(pt.y - ptMouseT.y) < 4)
			return 1;
		ptMouseT = pt;
		if (tabdat->fTipActive) {
			KillTimer(hwnd, TIMERID_HOVER_T);
			CallService("mToolTip/HideTip", 0, 0);
			tabdat->fTipActive = FALSE;
		}
		KillTimer(hwnd, TIMERID_HOVER_T);
		if (tabdat->pContainer && (!tabdat->pContainer->SideBar->isActive() && (TabCtrl_GetItemCount(hwnd) > 1 || !(tabdat->pContainer->dwFlags & CNT_HIDETABS))))
			SetTimer(hwnd, TIMERID_HOVER_T, 750, 0);
		break;

	case WM_SIZE:
		if (tabdat->pContainer) {
			int iTabs = TabCtrl_GetItemCount(hwnd);

			if (!(tabdat->dwStyle & TCS_MULTILINE)) {
				if (iTabs > (tabdat->pContainer->dwFlags & CNT_HIDETABS ? 1 : 0)) {
					RECT rcClient, rc;
					GetClientRect(hwnd, &rcClient);
					TabCtrl_GetItemRect(hwnd, iTabs - 1, &rc);
					DWORD newItemSize = (rcClient.right - 6) - (tabdat->dwStyle & TCS_BUTTONS ? (iTabs)* 10 : 0);
					newItemSize = newItemSize / iTabs;
					if (newItemSize < PluginConfig.tabConfig.m_fixedwidth)
						TabCtrl_SetItemSize(hwnd, newItemSize, rc.bottom - rc.top);
					else
						TabCtrl_SetItemSize(hwnd, PluginConfig.tabConfig.m_fixedwidth, rc.bottom - rc.top);

					SendMessage(hwnd, EM_SEARCHSCROLLER, 0, 0);
				}
			}
			else if ((tabdat->dwStyle & TCS_BUTTONS) && iTabs > 0) {
				RECT rcClient, rcItem;
				GetClientRect(hwnd, &rcClient);
				TabCtrl_GetItemRect(hwnd, 0, &rcItem);
				int nrTabsPerLine = (rcClient.right) / PluginConfig.tabConfig.m_fixedwidth;
				if (iTabs >= nrTabsPerLine && nrTabsPerLine > 0)
					TabCtrl_SetItemSize(hwnd, ((rcClient.right - 6) / nrTabsPerLine) - (tabdat->dwStyle & TCS_BUTTONS ? 8 : 0), rcItem.bottom - rcItem.top);
				else
					TabCtrl_SetItemSize(hwnd, PluginConfig.tabConfig.m_fixedwidth, rcItem.bottom - rcItem.top);
			}
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (!(tabdat->pContainer->settings->dwFlagsEx & TCF_CLOSEBUTTON)) {
			GetCursorPos(&pt);
			SendMessage(GetParent(hwnd), DM_CLOSETABATMOUSE, 0, (LPARAM)&pt);
		}
		break;

	case WM_RBUTTONDOWN:
		KillTimer(hwnd, TIMERID_HOVER_T);
		CallService("mToolTip/HideTip", 0, 0);
		tabdat->fTipActive = FALSE;
		break;

	case WM_LBUTTONDOWN:
		KillTimer(hwnd, TIMERID_HOVER_T);
		CallService("mToolTip/HideTip", 0, 0);
		tabdat->fTipActive = FALSE;

		if (GetKeyState(VK_CONTROL) & 0x8000) {
			pt.x = (short)LOWORD(GetMessagePos());
			pt.y = (short)HIWORD(GetMessagePos());
			if (DragDetect(hwnd, pt) && TabCtrl_GetItemCount(hwnd) > 1) {
				int i = GetTabItemFromMouse(hwnd, &pt);
				if (i != -1) {
					TCITEM tc;
					TWindowData *dat = NULL;

					tc.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, i, &tc);
					dat = (TWindowData*)GetWindowLongPtr((HWND)tc.lParam, GWLP_USERDATA);
					if (dat) {
						tabdat->bDragging = TRUE;
						tabdat->iBeginIndex = i;
						tabdat->hwndDrag = (HWND)tc.lParam;
						tabdat->dragDat = dat;
						tabdat->fSavePos = TRUE;
						tabdat->himlDrag = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 1, 0);
						ImageList_AddIcon(tabdat->himlDrag, dat->hTabIcon);
						ImageList_BeginDrag(tabdat->himlDrag, 0, 8, 8);
						ImageList_DragEnter(hwnd, pt.x, pt.y);
						SetCapture(hwnd);
					}
					return TRUE;
				}
			}
		}

		if (GetKeyState(VK_MENU) & 0x8000) {
			pt.x = (short)LOWORD(GetMessagePos());
			pt.y = (short)HIWORD(GetMessagePos());
			if (DragDetect(hwnd, pt) && TabCtrl_GetItemCount(hwnd) > 1) {
				int i = GetTabItemFromMouse(hwnd, &pt);
				if (i != -1) {
					TCITEM tc;
					tc.mask = TCIF_PARAM;
					TabCtrl_GetItem(hwnd, i, &tc);

					TWindowData *dat = (TWindowData*)GetWindowLongPtr((HWND)tc.lParam, GWLP_USERDATA);
					if (dat) {
						tabdat->bDragging = TRUE;
						tabdat->iBeginIndex = i;
						tabdat->hwndDrag = (HWND)tc.lParam;
						tabdat->dragDat = dat;
						tabdat->himlDrag = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 1, 0);
						tabdat->fSavePos = FALSE;
						ImageList_AddIcon(tabdat->himlDrag, dat->hTabIcon);
						ImageList_BeginDrag(tabdat->himlDrag, 0, 8, 8);
						ImageList_DragEnter(hwnd, pt.x, pt.y);
						SetCapture(hwnd);
					}
					return TRUE;
				}
			}
		}

		if (tabdat->fCloseButton) {
			GetCursorPos(&pt);
			if (TabCtrl_TestForCloseButton(tabdat, hwnd, &pt) != -1)
				return TRUE;
		}
		break;

	case WM_CAPTURECHANGED:
		tabdat->bDragging = FALSE;
		ImageList_DragLeave(hwnd);
		ImageList_EndDrag();
		if (tabdat->himlDrag) {
			ImageList_RemoveAll(tabdat->himlDrag);
			ImageList_Destroy(tabdat->himlDrag);
			tabdat->himlDrag = 0;
		}
		break;

	case WM_MOUSEMOVE:
		if (tabdat->bDragging) {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ImageList_DragMove(pt.x, pt.y);
		}

		{
			GetCursorPos(&pt);
			int iOld = tabdat->iHoveredTabIndex;
			tabdat->iHoveredTabIndex = GetTabItemFromMouse(hwnd, &pt);
			if (tabdat->iHoveredTabIndex != iOld)
				InvalidateRect(hwnd, NULL, FALSE);
			if (tabdat->fCloseButton && tabdat->iHoveredTabIndex != -1) {
				iOld = tabdat->iHoveredCloseIcon;
				tabdat->iHoveredCloseIcon = TabCtrl_TestForCloseButton(tabdat, hwnd, &pt);
				if (tabdat->iHoveredCloseIcon != iOld)
					InvalidateRect(hwnd, NULL, FALSE);
			}
		}
		break;

	case WM_NCHITTEST: /* wine: fix for erase/paint tab on mouse enter/leave tab. */
		{
			LRESULT res = CallWindowProc(OldTabControlClassProc, hwnd, msg, wParam, lParam);
			if (res != HTCLIENT)
				InvalidateRect(hwnd, NULL, FALSE);
			return (res);
		}
		break;

	case WM_LBUTTONUP:
		CallWindowProc(OldTabControlClassProc, hwnd, msg, wParam, lParam);
		if (tabdat->bDragging && ReleaseCapture()) {
			int i;
			pt.x = (short)LOWORD(GetMessagePos());
			pt.y = (short)HIWORD(GetMessagePos());
			tabdat->bDragging = FALSE;
			ImageList_DragLeave(hwnd);
			ImageList_EndDrag();

			i = GetTabItemFromMouse(hwnd, &pt);
			if (i != -1 && i != tabdat->iBeginIndex)
				RearrangeTab(tabdat->hwndDrag, tabdat->dragDat, MAKELONG(i, 0xffff), tabdat->fSavePos);
			tabdat->hwndDrag = (HWND)-1;
			tabdat->dragDat = NULL;
			if (tabdat->himlDrag) {
				ImageList_RemoveAll(tabdat->himlDrag);
				ImageList_Destroy(tabdat->himlDrag);
				tabdat->himlDrag = 0;
			}
		}
		if (tabdat->fCloseButton) {
			GetCursorPos(&pt);
			int iItem = TabCtrl_TestForCloseButton(tabdat, hwnd, &pt);
			if (iItem != -1)
				SendMessage(GetParent(hwnd), DM_CLOSETABATMOUSE, 0, (LPARAM)&pt);
		}
		break;

	case WM_ERASEBKGND:
		if (tabdat->pContainer && (CSkin::m_skinEnabled || M.isAero()))
			return TRUE;
		return 0;

	case WM_PAINT:
		PaintWorker(hwnd, tabdat);
		return 0;

	case WM_TIMER:
		InvalidateRect(hwnd, NULL, FALSE); /* wine: fix for erase/paint tab on mouse enter/leave tab. */
		if (wParam == TIMERID_HOVER_T &&  M.GetByte("d_tooltips", 0)) {
			KillTimer(hwnd, TIMERID_HOVER_T);

			GetCursorPos(&pt);
			if (abs(pt.x - ptMouseT.x) < 5 && abs(pt.y - ptMouseT.y) < 5) {
				CLCINFOTIP ti = { 0 };
				ti.cbSize = sizeof(ti);
				ti.ptCursor = pt;

				TCITEM item = { 0 };
				item.mask = TCIF_PARAM;
				int nItem = GetTabItemFromMouse(hwnd, &pt);
				if (nItem >= 0 && nItem < TabCtrl_GetItemCount(hwnd)) {
					TabCtrl_GetItem(hwnd, nItem, &item);

					/*
					 * get the message window data for the session to which this tab item belongs
					 */
					TWindowData *dat = 0;
					if (IsWindow((HWND)item.lParam) && item.lParam != 0)
						dat = (TWindowData*)GetWindowLongPtr((HWND)item.lParam, GWLP_USERDATA);
					if (dat) {
						tabdat->fTipActive = TRUE;
						ti.isGroup = 0;
						ti.hItem = (HANDLE)dat->hContact;
						ti.isTreeFocused = 0;
						CallService("mToolTip/ShowTip", 0, (LPARAM)&ti);
					}
				}
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (lParam == -1) {
			short amount = short(HIWORD(wParam));
			if (amount > 0)
				SendMessage(GetParent(hwnd), DM_SELECTTAB, DM_SELECT_PREV, 0);
			else if (amount < 0)
				SendMessage(GetParent(hwnd), DM_SELECTTAB, DM_SELECT_NEXT, 0);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		break;

	case WM_USER + 100:
		if (tabdat->fTipActive) {
			tabdat->fTipActive = FALSE;
			CallService("mToolTip/HideTip", 0, 0);
		}
		break;
	}

#if defined(__LOGDEBUG_)
	_DebugTraceW(L"msg: %hu", msg);
#endif
	return CallWindowProc(OldTabControlClassProc, hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// load the tab control configuration data (colors, fonts, flags...

void TSAPI ReloadTabConfig()
{
	PluginConfig.tabConfig.m_hPenLight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	PluginConfig.tabConfig.m_hPenShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
	PluginConfig.tabConfig.m_hPenItemShadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

	NONCLIENTMETRICS nclim;
	nclim.cbSize = sizeof(nclim);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &nclim, 0);
	PluginConfig.tabConfig.m_hMenuFont = CreateFontIndirect(&nclim.lfMessageFont);

	for (int i = 0; tabcolors[i].szKey != NULL; i++)
		PluginConfig.tabConfig.colors[i] = M.GetDword(CSkin::m_skinEnabled ? tabcolors[i].szSkinnedKey : tabcolors[i].szKey, GetSysColor(tabcolors[i].defclr));

	PluginConfig.tabConfig.m_brushes[0] = CreateSolidBrush(PluginConfig.tabConfig.colors[4]);
	PluginConfig.tabConfig.m_brushes[1] = CreateSolidBrush(PluginConfig.tabConfig.colors[5]);
	PluginConfig.tabConfig.m_brushes[2] = CreateSolidBrush(PluginConfig.tabConfig.colors[6]);
	PluginConfig.tabConfig.m_brushes[3] = CreateSolidBrush(PluginConfig.tabConfig.colors[7]);

	PluginConfig.tabConfig.m_bottomAdjust = (int)M.GetDword("bottomadjust", 0);
	PluginConfig.tabConfig.m_fixedwidth = M.GetDword("fixedwidth", FIXED_TAB_SIZE);

	PluginConfig.tabConfig.m_fixedwidth = (PluginConfig.tabConfig.m_fixedwidth < 60 ? 60 : PluginConfig.tabConfig.m_fixedwidth);
}

void TSAPI FreeTabConfig()
{
	if (PluginConfig.tabConfig.m_hPenItemShadow)
		DeleteObject(PluginConfig.tabConfig.m_hPenItemShadow);

	if (PluginConfig.tabConfig.m_hPenLight)
		DeleteObject(PluginConfig.tabConfig.m_hPenLight);

	if (PluginConfig.tabConfig.m_hPenShadow)
		DeleteObject(PluginConfig.tabConfig.m_hPenShadow);

	if (PluginConfig.tabConfig.m_hMenuFont)
		DeleteObject(PluginConfig.tabConfig.m_hMenuFont);

	for (int i = 0; i < SIZEOF(PluginConfig.tabConfig.m_brushes); i++)
		if (PluginConfig.tabConfig.m_brushes[i])
			DeleteObject(PluginConfig.tabConfig.m_brushes[i]);

	memset(&PluginConfig.tabConfig, 0, sizeof(PluginConfig.tabConfig));
}

/////////////////////////////////////////////////////////////////////////////////////////
// options dialog for setting up tab options

static bool tconfig_init = false;

INT_PTR CALLBACK DlgProcTabConfig(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		tconfig_init = false;
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_USER + 100, 0, 0);
		tconfig_init = true;
		return TRUE;

	case WM_USER + 100:
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPIN, UDM_SETRANGE, 0, MAKELONG(10, 0));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPIN, UDM_SETPOS, 0, (int)M.GetByte(CSkin::m_skinEnabled ? "S_tborder" : "tborder", 2));
		SetDlgItemInt(hwndDlg, IDC_TABBORDER, (int)M.GetByte(CSkin::m_skinEnabled ? "S_tborder" : "tborder", 2), FALSE);

		SendDlgItemMessage(hwndDlg, IDC_BOTTOMTABADJUSTSPIN, UDM_SETRANGE, 0, MAKELONG(3, -3));
		SendDlgItemMessage(hwndDlg, IDC_BOTTOMTABADJUSTSPIN, UDM_SETPOS, 0, PluginConfig.tabConfig.m_bottomAdjust);
		SetDlgItemInt(hwndDlg, IDC_BOTTOMTABADJUST, PluginConfig.tabConfig.m_bottomAdjust, TRUE);

		SendDlgItemMessage(hwndDlg, IDC_TABWIDTHSPIN, UDM_SETRANGE, 0, MAKELONG(400, 50));
		SendDlgItemMessage(hwndDlg, IDC_TABWIDTHSPIN, UDM_SETPOS, 0, PluginConfig.tabConfig.m_fixedwidth);
		SetDlgItemInt(hwndDlg, IDC_TABWIDTH, PluginConfig.tabConfig.m_fixedwidth, TRUE);

		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTER, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTERRIGHT, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTERTOP, UDM_SETRANGE, 0, MAKELONG(40, 0));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTERBOTTOM, UDM_SETRANGE, 0, MAKELONG(40, 0));

		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTER, UDM_SETPOS, 0, (int)M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_left" : "tborder_outer_left", 2));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTERRIGHT, UDM_SETPOS, 0, (int)M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_right" : "tborder_outer_right", 2));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTERTOP, UDM_SETPOS, 0, (int)M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_top" : "tborder_outer_top", 2));
		SendDlgItemMessage(hwndDlg, IDC_TABBORDERSPINOUTERBOTTOM, UDM_SETPOS, 0, (int)M.GetByte(CSkin::m_skinEnabled ? "S_tborder_outer_bottom" : "tborder_outer_bottom", 2));

		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(10, 1));
		SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_SETRANGE, 0, MAKELONG(10, 1));
		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETPOS, 0, (LPARAM)M.GetByte("y-pad", 3));
		SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_SETPOS, 0, (LPARAM)M.GetByte("x-pad", 4));
		SetDlgItemInt(hwndDlg, IDC_TABPADDING, (int)M.GetByte("y-pad", 3), FALSE);
		SetDlgItemInt(hwndDlg, IDC_HTABPADDING, (int)M.GetByte("x-pad", 4), FALSE);
		return 0;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				BOOL translated;

				db_set_b(0, SRMSGMOD_T, "y-pad", (BYTE)(GetDlgItemInt(hwndDlg, IDC_TABPADDING, NULL, FALSE)));
				db_set_b(0, SRMSGMOD_T, "x-pad", (BYTE)(GetDlgItemInt(hwndDlg, IDC_HTABPADDING, NULL, FALSE)));
				db_set_b(0, SRMSGMOD_T, "tborder", (BYTE)GetDlgItemInt(hwndDlg, IDC_TABBORDER, &translated, FALSE));
				db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_left" : "tborder_outer_left", (BYTE)GetDlgItemInt(hwndDlg, IDC_TABBORDEROUTER, &translated, FALSE));
				db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_right" : "tborder_outer_right", (BYTE)GetDlgItemInt(hwndDlg, IDC_TABBORDEROUTERRIGHT, &translated, FALSE));
				db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_top" : "tborder_outer_top", (BYTE)GetDlgItemInt(hwndDlg, IDC_TABBORDEROUTERTOP, &translated, FALSE));
				db_set_b(0, SRMSGMOD_T, CSkin::m_skinEnabled ? "S_tborder_outer_bottom" : "tborder_outer_bottom", (BYTE)GetDlgItemInt(hwndDlg, IDC_TABBORDEROUTERBOTTOM, &translated, FALSE));
				db_set_dw(0, SRMSGMOD_T, "bottomadjust", GetDlgItemInt(hwndDlg, IDC_BOTTOMTABADJUST, &translated, TRUE));

				int fixedWidth = GetDlgItemInt(hwndDlg, IDC_TABWIDTH, &translated, FALSE);
				fixedWidth = (fixedWidth < 60 ? 60 : fixedWidth);
				db_set_dw(0, SRMSGMOD_T, "fixedwidth", fixedWidth);
				FreeTabConfig();
				ReloadTabConfig();

				for (TContainerData *p = pFirstContainer; p; p = p->pNext) {
					HWND hwndTab = GetDlgItem(p->hwnd, IDC_MSGTABS);
					TabCtrl_SetPadding(hwndTab, GetDlgItemInt(hwndDlg, IDC_HTABPADDING, NULL, FALSE), GetDlgItemInt(hwndDlg, IDC_TABPADDING, NULL, FALSE));
					RedrawWindow(hwndTab, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
				}
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TABWIDTH:
		case IDC_TABPADDING:
		case IDC_HTABPADDING:
		case IDC_TABBORDER:
		case IDC_TABBORDEROUTER:
		case IDC_TABBORDEROUTERBOTTOM:
		case IDC_TABBORDEROUTERRIGHT:
		case IDC_TABBORDEROUTERTOP:
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;
			break;
		}
		if (tconfig_init)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_DESTROY:
		tconfig_init = false;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// register the new tab control as a window class (TSTabCtrlClass)

int TSAPI RegisterTabCtrlClass(void)
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);

	wc.lpszClassName = _T("TSTabCtrlClass");
	wc.lpfnWndProc = TabControlSubclassProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(TabControlData*);
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC;
	RegisterClassEx(&wc);

	wc.lpszClassName = _T("TSStatusBarClass");
	wc.lpfnWndProc = StatusBarSubclassProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(void*);
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC;
	RegisterClassEx(&wc);

	wc.lpszClassName = _T("TS_SideBarClass");
	wc.lpfnWndProc = CSideBar::wndProcStub;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(void*);
	wc.style = CS_GLOBALCLASS;// | CS_DBLCLKS; // | CS_PARENTDC;
	RegisterClassEx(&wc);

	wc.lpszClassName = _T("TSHK");
	wc.lpfnWndProc = HotkeyHandlerDlgProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(void*);
	wc.style = CS_GLOBALCLASS;// | CS_DBLCLKS; // | CS_PARENTDC;
	RegisterClassEx(&wc);
	return 0;
}
