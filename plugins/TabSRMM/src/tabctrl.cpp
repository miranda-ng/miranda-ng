/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

#include "stdafx.h"

static WNDPROC OldTabControlClassProc;

#define FIXED_TAB_SIZE 100

/////////////////////////////////////////////////////////////////////////////////////////
// window data for the tab control window class

struct TabControlData
{
	HWND   hwnd;
	uint32_t  dwStyle;
	uint32_t  cx, cy;
	HANDLE hTheme, hThemeButton, hbp;
	int    xpad;
	int    iBeginIndex;
	int    iHoveredTabIndex;
	int    iHoveredCloseIcon;
	HWND   hwndDrag;

	bool   bVisualStyles;
	bool   bDragging;
	bool   bRefreshWithoutClip;
	bool   bSavePos;
	bool   bTipActive;
	bool   bAeroTabs;
	bool   bCloseButton;

	HIMAGELIST himlDrag;
	TContainerData *pContainer;
	CMsgDialog *dragDat;
	CMsgDialog *helperDat;  // points to the client data of the active tab
	CImageItem *helperItem, *helperGlowItem;				// aero ui, holding the skin image for the tabs
};

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
	TCHITTESTINFO tci;
	tci.pt = (*pt);
	tci.flags = 0;
	ScreenToClient(hwnd, &tci.pt);
	
	int iTab = TabCtrl_HitTest(hwnd, &tci);
	if (iTab == -1 || (tci.flags & TCHT_NOWHERE))
		return -1;

	RECT rcTab;
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

struct colOptions
{
	UINT defclr;
	char *szKey;
	char *szSkinnedKey;
}
static tabcolors[] =
{
	{ COLOR_BTNTEXT, "tab_txt_normal", "S_tab_txt_normal" },
	{ COLOR_BTNTEXT, "tab_txt_active", "S_tab_txt_active" },
	{ COLOR_HOTLIGHT, "tab_txt_hottrack", "S_tab_txt_hottrack" },
	{ COLOR_HOTLIGHT, "tab_txt_unread", "S_tab_txt_unread" },
	{ COLOR_3DFACE, "tab_bg_normal", "tab_bg_normal" },
	{ COLOR_3DFACE, "tab_bg_active", "tab_bg_active" },
	{ COLOR_3DFACE, "tab_bg_hottrack", "tab_bg_hottrack" },
	{ COLOR_3DFACE, "tab_bg_unread", "tab_bg_unread" },
	{ 0, nullptr, nullptr }
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

void TSAPI FillTabBackground(const HDC hdc, int iStateId, const CMsgDialog *dat, RECT* rc)
{
	unsigned clrIndex;

	if (dat && dat->m_bCanFlashTab)
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

static void DrawItem(TabControlData *tabdat, HDC dc, RECT *rcItem, int nHint, int nItem, CMsgDialog *dat)
{
	if (dat == nullptr)
		return;

	InflateRect(rcItem, -2, -2);

	unsigned clrIndex = (dat->m_bCanFlashTab) ? 3 : (nHint & HINT_ACTIVE_ITEM ? 1 : (nHint & HINT_HOTTRACK ? 2 : 0));
	COLORREF clr = PluginConfig.tabConfig.colors[clrIndex];

	int oldMode = SetBkMode(dc, TRANSPARENT);

	if (!(tabdat->dwStyle & TCS_BOTTOM))
		OffsetRect(rcItem, 0, 1);

	int iSize = 16;
	HICON hIcon;
	if (dat->m_bErrorState)
		hIcon = PluginConfig.g_iconErr;
	else if (dat->m_bCanFlashTab)
		hIcon = dat->m_iFlashIcon;
	else {
		if (dat->isChat() && dat->m_iFlashIcon) {
			hIcon = dat->m_iFlashIcon;

			int sizeY;
			Utils::getIconSize(hIcon, iSize, sizeY);
		}
		else if (dat->m_hTabIcon == dat->m_hTabStatusIcon && dat->m_hXStatusIcon)
			hIcon = dat->m_hXStatusIcon;
		else
			hIcon = dat->m_hTabIcon;
	}

	if (!dat->m_bCanFlashTab || (dat->m_bCanFlashTab == TRUE && dat->m_bTabFlash) || !dat->m_pContainer->cfg.flagsEx.m_bTabFlashIcon) {
		uint32_t ix = rcItem->left + tabdat->xpad - 1;
		uint32_t iy = (rcItem->bottom + rcItem->top - iSize) / 2;
		if (dat->m_bIsIdle && PluginConfig.m_bIdleDetect)
			CSkin::DrawDimmedIcon(dc, ix, iy, iSize, iSize, hIcon, 180);
		else
			DrawIconEx(dc, ix, iy, hIcon, iSize, iSize, 0, nullptr, DI_NORMAL | DI_COMPAT);
	}

	rcItem->left += (iSize + 2 + tabdat->xpad);

	if (tabdat->bCloseButton) {
		if (tabdat->iHoveredCloseIcon != nItem)
			CSkin::m_default_bf.SourceConstantAlpha = 150;

		GdiAlphaBlend(dc, rcItem->right - 16 - tabdat->xpad, (rcItem->bottom + rcItem->top - 16) / 2, 16, 16, CSkin::m_tabCloseHDC,
			0, 0, 16, 16, CSkin::m_default_bf);

		rcItem->right -= (18 + tabdat->xpad);
		CSkin::m_default_bf.SourceConstantAlpha = 255;
	}

	if (!dat->m_bCanFlashTab || (dat->m_bCanFlashTab == TRUE && dat->m_bTabFlash) || !dat->m_pContainer->cfg.flagsEx.m_bTabFlashLabel) {
		uint32_t dwTextFlags = DT_SINGLELINE | DT_VCENTER;
		HFONT oldFont = (HFONT)SelectObject(dc, (HFONT)SendMessage(tabdat->hwnd, WM_GETFONT, 0, 0));
		if (tabdat->dwStyle & TCS_BUTTONS || !(tabdat->dwStyle & TCS_MULTILINE)) {
			rcItem->right -= tabdat->xpad;
			dwTextFlags |= DT_WORD_ELLIPSIS;
		}
		CSkin::RenderText(dc, tabdat->dwStyle & TCS_BUTTONS ? tabdat->hThemeButton : tabdat->hTheme, dat->m_wszTitle, rcItem, dwTextFlags, CSkin::m_glowSize, clr);
		SelectObject(dc, oldFont);
	}
	if (oldMode)
		SetBkMode(dc, oldMode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// draws the item rect (the "tab") in *classic* style (no visual themes

static RECT rcTabPage = { 0 };

static void DrawItemRect(TabControlData *tabdat, HDC dc, RECT *rcItem, int nHint, const CMsgDialog *dat)
{
	POINT pt;
	uint32_t dwStyle = tabdat->dwStyle;

	rcItem->bottom -= 1;
	if (rcItem->left < 0)
		return;

	// draw "button style" tabs... raised edge for hottracked, sunken edge for active (pushed)
	// otherwise, they get a normal border
	if (dwStyle & TCS_BUTTONS) {
		bool bClassicDraw = (tabdat->bVisualStyles == false);

		// draw frame controls for button or bottom tabs
		if (dwStyle & TCS_BOTTOM)
			rcItem->top++;

		rcItem->right += 6;
		if (tabdat->bAeroTabs) {
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
					CSkin::SkinDrawBG(tabdat->hwnd, tabdat->pContainer->m_hwnd, tabdat->pContainer, rcItem, dc);
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

static HRESULT DrawThemesPartWithAero(const TabControlData *tabdat, HDC hDC, int iPartId, int iStateId, LPRECT prcBox, CMsgDialog *dat)
{
	HRESULT hResult = 0;
	bool	bAero = M.isAero();

	if (tabdat->bAeroTabs) {
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
	else if (tabdat->hTheme != nullptr) {
		hResult = DrawThemeBackground(tabdat->hTheme, hDC, iPartId, iStateId, prcBox, nullptr);
	}

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// draws a theme part (identifier in uiPartNameID) using the given clipping rectangle

static HRESULT DrawThemesPart(const TabControlData *tabdat, HDC hDC, int iPartId, int iStateId, LPRECT prcBox)
{
	HRESULT hResult = 0;

	if (tabdat->hTheme != nullptr)
		hResult = DrawThemeBackground(tabdat->hTheme, hDC, iPartId, iStateId, prcBox, nullptr);

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// draw a themed tab item. either a tab or the body pane
// handles image mirroring for tabs at the bottom

static void DrawThemesXpTabItem(HDC pDC, RECT *rcItem, UINT uiFlag, TabControlData *tabdat, CMsgDialog *dat)
{
	BOOL bBody = (uiFlag & 1) ? TRUE : FALSE;
	BOOL bSel = (uiFlag & 2) ? TRUE : FALSE;
	BOOL bHot = (uiFlag & 4) ? TRUE : FALSE;
	BOOL bBottom = (uiFlag & 8) ? TRUE : FALSE;	// mirror
	LPBYTE pcImg = nullptr;
	int nStart = 0, nLenSub = 0;

	SIZE szBmp;
	szBmp.cx = rcItem->right - rcItem->left;
	szBmp.cy = rcItem->bottom - rcItem->top;

	// for top row tabs, it's easy. Just draw to the provided dc (it's a mem dc already)
	if (!bBottom) {
		if (bBody) {
			if (IsWinVerVistaPlus()) {
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
	else if (tabdat->bAeroTabs && !bBody) {
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

		uint8_t *pcImg1 = (uint8_t*)mir_alloc(nSzBuffPS);
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
		pcImg = (uint8_t *)mir_alloc(nSzBuffPS);
		if (pcImg) {									// get bits:
			GetDIBits(hdcTemp, hbmTemp, nStart, 50 - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
			bihOut->biHeight = -50;
			SetDIBits(hdcTemp, hbmTemp, nStart, 50 - nLenSub, pcImg, &biOut, DIB_RGB_COLORS);
			mir_free(pcImg);
		}
		CImageItem tempItem(10, 10, 10, 10, hdcTemp, nullptr, IMAGE_FLAG_DIVIDED | IMAGE_FILLSOLID,
			GetSysColorBrush(COLOR_3DFACE), 255, 30, 80, 50, 100);

		if (IsWinVerVistaPlus()) // hide right tab sheet shadow (only draw the actual border line)
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
	pcImg = (uint8_t *)mir_alloc(nSzBuffPS);

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
	if (tabdat == nullptr || tabdat->pContainer == nullptr)
		return;

	if (GetUpdateRect(hwnd, nullptr, TRUE) == 0)
		return;

	RECT  rectTemp, rctPage, rctActive, rcItem, rctClip, rctOrig;
	RECT  rectUpDn = { 0, 0, 0, 0 };
	int   nCount = TabCtrl_GetItemCount(hwnd), i;
	uint32_t dwStyle = tabdat->dwStyle;
	bool  isAero = M.isAero();
	bool  bClassicDraw = !isAero && (tabdat->bVisualStyles == false || CSkin::m_skinEnabled);

	POINT pt;
	GetCursorPos(&pt);
	int hotItem = GetTabItemFromMouse(hwnd, &pt);
	if (tabdat->iHoveredTabIndex != hotItem)
		InvalidateRect(hwnd, nullptr, FALSE);
	tabdat->iHoveredTabIndex = hotItem;

	tabdat->bAeroTabs = CSkin::m_fAeroSkinsValid && (isAero || PluginConfig.m_fillColor);
	tabdat->bCloseButton = tabdat->pContainer->cfg.flagsEx.m_bTabCloseButton;
	tabdat->helperDat = nullptr;

	if (tabdat->bAeroTabs) {
		CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(tabdat->pContainer->m_hwndActive, GWLP_USERDATA);
		if (dat)
			tabdat->helperDat = dat;
		else
			tabdat->bAeroTabs = false;

		tabdat->helperItem = (dwStyle & TCS_BOTTOM) ? CSkin::m_tabBottom : CSkin::m_tabTop;
		tabdat->helperGlowItem = (dwStyle & TCS_BOTTOM) ? CSkin::m_tabGlowBottom : CSkin::m_tabGlowTop;
	}
	else tabdat->bAeroTabs = false;

	PAINTSTRUCT ps;
	HDC hdcreal = BeginPaint(hwnd, &ps);

	// switchbar is active, don't paint a single pixel, the tab control won't be visible at all
	// same when we have only ONE tab and do not want it to be visible because of the container
	// option "Show tab bar only when needed".
	if ((tabdat->pContainer->cfg.flags.m_bSideBar) || (nCount == 1 && tabdat->pContainer->cfg.flags.m_bHideTabs)) {
		if (nCount == 0)
			FillRect(hdcreal, &ps.rcPaint, GetSysColorBrush(COLOR_3DFACE)); // avoid flickering/ugly black background during container creation
		EndPaint(hwnd, &ps);
		return;
	}

	GetClientRect(hwnd, &rctPage);
	rctOrig = rctPage;
	int iActive = TabCtrl_GetCurSel(hwnd);
	TabCtrl_GetItemRect(hwnd, iActive, &rctActive);
	int cx = rctPage.right - rctPage.left;
	int cy = rctPage.bottom - rctPage.top;

	// draw everything to a memory dc to avoid flickering
	HDC hdc;
	HANDLE hpb;
	HBITMAP bmpMem, bmpOld;
	if (CMimAPI::m_haveBufferedPaint) {
		hpb = tabdat->hbp = CSkin::InitiateBufferedPaint(hdcreal, rctPage, hdc);
		bmpMem = bmpOld = nullptr;
	}
	else {
		hpb = nullptr;
		hdc = CreateCompatibleDC(hdcreal);
		bmpMem = tabdat->bAeroTabs ? CSkin::CreateAeroCompatibleBitmap(rctPage, hdcreal) : CreateCompatibleBitmap(hdcreal, cx, cy);
		bmpOld = (HBITMAP)SelectObject(hdc, bmpMem);
	}

	if (nCount == 1 && tabdat->pContainer->cfg.flags.m_bHideTabs)
		rctClip = rctPage;

	if (CSkin::m_skinEnabled)
		CSkin::SkinDrawBG(hwnd, tabdat->pContainer->m_hwnd, tabdat->pContainer, &rctPage, hdc);
	else
		CSkin::FillBack(hdc, &rctPage);

	UINT uiBottom;
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

	if (nCount > 1 || !(tabdat->pContainer->cfg.flags.m_bHideTabs)) {
		rctClip = rctPage;
		InflateRect(&rctClip, -tabdat->pContainer->m_tBorder, -tabdat->pContainer->m_tBorder);
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
		UINT uiFlags = 1;
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
			DrawThemesXpTabItem(hdc, &rcClient, uiFlags, tabdat, nullptr);	// TABP_PANE=9,0,'TAB'
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
			GetWindowRect(tabdat->helperDat->GetHwnd(), &rcLog);
			pt.y = rcLog.bottom;
			pt.x = rcLog.left;
			ScreenToClient(hwnd, &pt);
			rcPage.top = pt.y + ((nCount > 1 || !(tabdat->helperDat->m_pContainer->cfg.flags.m_bHideTabs)) ? tabdat->helperDat->m_pContainer->m_tBorder : 0);
			FillRect(hdc, &rcPage, CSkin::m_BrushBack);
			rcPage.top = 0;
		}
		GetWindowRect(GetDlgItem(tabdat->helperDat->GetHwnd(), IDC_SRMM_LOG), &rcLog);

		pt.y = rcLog.top;
		pt.x = rcLog.left;
		ScreenToClient(hwnd, &pt);
		rcPage.bottom = pt.y;
		FillRect(hdc, &rcPage, CSkin::m_BrushBack);
	}

	// figure out hottracked item (if any)
	if (tabdat->bRefreshWithoutClip)
		goto skip_tabs;

	for (i = 0; i < nCount; i++) {
		if (i == iActive)
			continue;

		CMsgDialog *dat = nullptr;
		if (HWND hDlg = GetTabWindow(hwnd, i))
			dat = (CMsgDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		TabCtrl_GetItemRect(hwnd, i, &rcItem);
		if (!bClassicDraw && uiBottom) {
			rcItem.top -= PluginConfig.tabConfig.m_bottomAdjust;
			rcItem.bottom -= PluginConfig.tabConfig.m_bottomAdjust;
		}
		if (IntersectRect(&rectTemp, &rcItem, &ps.rcPaint) || bClassicDraw) {
			int nHint = 0;
			if (!bClassicDraw && !(dwStyle & TCS_BUTTONS)) {
				DrawThemesXpTabItem(hdc, &rcItem, uiBottom | (i == hotItem ? 4 : 0), tabdat, dat);
				DrawItem(tabdat, hdc, &rcItem, nHint | (i == hotItem ? HINT_HOTTRACK : 0), i, dat);
			}
			else {
				if (tabdat->bAeroTabs && !CSkin::m_skinEnabled && !(dwStyle & TCS_BUTTONS))
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
		CMsgDialog *dat = nullptr;
		int nHint = 0;

		rcItem = rctActive;
		if (HWND hDlg = GetTabWindow(hwnd, iActive))
			dat = (CMsgDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

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
			if (tabdat->bAeroTabs && !CSkin::m_skinEnabled && !(dwStyle & TCS_BUTTONS)) {
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

static bool TabBeginDrag(HWND hwnd, bool bSavePos)
{
	POINT pt;
	pt.x = (short)LOWORD(GetMessagePos());
	pt.y = (short)HIWORD(GetMessagePos());
	if (DragDetect(hwnd, pt) && TabCtrl_GetItemCount(hwnd) > 1) {
		int i = GetTabItemFromMouse(hwnd, &pt);
		if (i != -1) {
			HWND hDlg = GetTabWindow(hwnd, i);
			CMsgDialog *dat = (CMsgDialog *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			if (dat) {
				TabControlData *tabdat = (TabControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				tabdat->bDragging = true;
				tabdat->iBeginIndex = i;
				tabdat->hwndDrag = hDlg;
				tabdat->dragDat = dat;
				tabdat->himlDrag = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 1, 0);
				tabdat->bSavePos = bSavePos;
				ImageList_AddIcon(tabdat->himlDrag, dat->m_hTabIcon);
				ImageList_BeginDrag(tabdat->himlDrag, 0, 8, 8);
				ImageList_DragEnter(hwnd, pt.x, pt.y);
				SetCapture(hwnd);
			}
			return true;
		}
	}
	return false;
}

static LRESULT CALLBACK TabControlSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	TabControlData *tabdat = (TabControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (tabdat) {
		if (tabdat->pContainer == nullptr)
			tabdat->pContainer = (TContainerData*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
		tabdat->dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
	}

	switch (msg) {
	case WM_NCCREATE:
		{
			WNDCLASSEX wcl = { 0 };
			wcl.cbSize = sizeof(wcl);
			GetClassInfoEx(g_plugin.getInst(), L"SysTabControl32", &wcl);
			OldTabControlClassProc = wcl.lpfnWndProc;

			tabdat = (TabControlData*)mir_calloc(sizeof(TabControlData));
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)tabdat);
			tabdat->hwnd = hwnd;
			tabdat->cx = GetSystemMetrics(SM_CXSMICON);
			tabdat->cy = GetSystemMetrics(SM_CYSMICON);
			tabdat->bTipActive = false;
			tabdat->iHoveredTabIndex = -1;
			tabdat->iHoveredCloseIcon = -1;
			SendMessage(hwnd, EM_THEMECHANGED, 0, 0);
		}
		return TRUE;

	case EM_THEMECHANGED:
		tabdat->xpad = M.GetByte("x-pad", 3);
		tabdat->bVisualStyles = false;
		if (IsThemeActive()) {
			tabdat->bVisualStyles = true;
			if (tabdat->hTheme != nullptr) {
				CloseThemeData(tabdat->hTheme);
				CloseThemeData(tabdat->hThemeButton);
			}
			if ((tabdat->hTheme = OpenThemeData(hwnd, L"TAB")) == nullptr || (tabdat->hThemeButton = OpenThemeData(hwnd, L"BUTTON")) == nullptr)
				tabdat->bVisualStyles = false;
		}
		return 0;

	case EM_SEARCHSCROLLER:
		// search the updown control (scroll arrows) to subclass it...
		// the control is dynamically created and may not exist as long as it is
		// not needed. So we have to search it everytime we need to paint. However,
		// it is sufficient to search it once. So this message is called, whenever
		// a new tab is inserted
		HWND hwndChild;
		if ((hwndChild = FindWindowEx(hwnd, nullptr, L"msctls_updown32", nullptr)) != nullptr)
			DestroyWindow(hwndChild);

		return 0;

	case EM_VALIDATEBOTTOM:
		if ((tabdat->dwStyle & TCS_BOTTOM) && tabdat->bVisualStyles && PluginConfig.tabConfig.m_bottomAdjust != 0)
			InvalidateRect(hwnd, nullptr, FALSE);
		break;

	case EM_REFRESHWITHOUTCLIP:
		if (TabCtrl_GetItemCount(hwnd) > 1)
			return 0;

		tabdat->bRefreshWithoutClip = true;
		RedrawWindow(hwnd, nullptr, nullptr, RDW_UPDATENOW | RDW_NOCHILDREN | RDW_INVALIDATE);
		tabdat->bRefreshWithoutClip = false;
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
			if (tabdat->hTheme != nullptr) {
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
		tabdat->pContainer->CloseTabByMouse(&pt);
		return 1;

	case WM_SETCURSOR:
		GetCursorPos(&pt);
		InvalidateRect(hwnd, nullptr, FALSE); /* wine: fix for erase/paint tab on mouse enter/leave tab. */
		SendMessage(GetParent(hwnd), msg, wParam, lParam);
		if (abs(pt.x - ptMouseT.x) < 4 && abs(pt.y - ptMouseT.y) < 4)
			return 1;
		ptMouseT = pt;
		if (tabdat->bTipActive) {
			KillTimer(hwnd, TIMERID_HOVER_T);
			Tipper_Hide();
			tabdat->bTipActive = false;
		}
		KillTimer(hwnd, TIMERID_HOVER_T);
		if (tabdat->pContainer && (!tabdat->pContainer->m_pSideBar->isActive() && (TabCtrl_GetItemCount(hwnd) > 1 || !(tabdat->pContainer->cfg.flags.m_bHideTabs))))
			SetTimer(hwnd, TIMERID_HOVER_T, 750, nullptr);
		break;

	case WM_SIZE:
		if (tabdat->pContainer) {
			int iTabs = TabCtrl_GetItemCount(hwnd);

			if (!(tabdat->dwStyle & TCS_MULTILINE)) {
				if (iTabs > (tabdat->pContainer->cfg.flags.m_bHideTabs ? 1 : 0)) {
					RECT rcClient, rc;
					GetClientRect(hwnd, &rcClient);
					TabCtrl_GetItemRect(hwnd, iTabs - 1, &rc);
					uint32_t newItemSize = (rcClient.right - 6) - (tabdat->dwStyle & TCS_BUTTONS ? (iTabs)* 10 : 0);
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
		if (!tabdat->pContainer->cfg.flagsEx.m_bTabCloseButton) {
			GetCursorPos(&pt);
			tabdat->pContainer->CloseTabByMouse(&pt);
		}
		break;

	case WM_RBUTTONDOWN:
		KillTimer(hwnd, TIMERID_HOVER_T);
		Tipper_Hide();
		tabdat->bTipActive = false;
		break;

	case WM_LBUTTONDOWN:
		KillTimer(hwnd, TIMERID_HOVER_T);
		Tipper_Hide();
		tabdat->bTipActive = false;

		if (GetKeyState(VK_CONTROL) & 0x8000)
			if (TabBeginDrag(hwnd, true))
				return TRUE;
		
		if (GetKeyState(VK_MENU) & 0x8000)
			if (TabBeginDrag(hwnd, false))
				return TRUE;

		if (tabdat->bCloseButton) {
			GetCursorPos(&pt);
			if (TabCtrl_TestForCloseButton(tabdat, hwnd, &pt) != -1)
				return TRUE;
		}
		break;

	case WM_CAPTURECHANGED:
		tabdat->bDragging = false;
		ImageList_DragLeave(hwnd);
		ImageList_EndDrag();
		if (tabdat->himlDrag) {
			ImageList_RemoveAll(tabdat->himlDrag);
			ImageList_Destroy(tabdat->himlDrag);
			tabdat->himlDrag = nullptr;
		}
		break;

	case WM_MOUSEMOVE:
		if (tabdat->bDragging) {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ImageList_DragMove(pt.x, pt.y);
		}

		GetCursorPos(&pt);
		{
			int iOld = tabdat->iHoveredTabIndex;
			tabdat->iHoveredTabIndex = GetTabItemFromMouse(hwnd, &pt);
			if (tabdat->iHoveredTabIndex != iOld)
				InvalidateRect(hwnd, nullptr, FALSE);
			if (tabdat->bCloseButton && tabdat->iHoveredTabIndex != -1) {
				iOld = tabdat->iHoveredCloseIcon;
				tabdat->iHoveredCloseIcon = TabCtrl_TestForCloseButton(tabdat, hwnd, &pt);
				if (tabdat->iHoveredCloseIcon != iOld)
					InvalidateRect(hwnd, nullptr, FALSE);
			}
		}
		break;

	case WM_NCHITTEST: /* wine: fix for erase/paint tab on mouse enter/leave tab. */
		{
			LRESULT res = CallWindowProc(OldTabControlClassProc, hwnd, msg, wParam, lParam);
			if (res != HTCLIENT)
				InvalidateRect(hwnd, nullptr, FALSE);
			return (res);
		}
		break;

	case WM_LBUTTONUP:
		CallWindowProc(OldTabControlClassProc, hwnd, msg, wParam, lParam);
		if (tabdat->bDragging && ReleaseCapture()) {
			int i;
			pt.x = (short)LOWORD(GetMessagePos());
			pt.y = (short)HIWORD(GetMessagePos());
			tabdat->bDragging = false;
			ImageList_DragLeave(hwnd);
			ImageList_EndDrag();

			i = GetTabItemFromMouse(hwnd, &pt);
			if (i != -1 && i != tabdat->iBeginIndex)
				RearrangeTab(tabdat->hwndDrag, tabdat->dragDat, MAKELONG(i, 0xffff), tabdat->bSavePos);
			tabdat->hwndDrag = (HWND)-1;
			tabdat->dragDat = nullptr;
			if (tabdat->himlDrag) {
				ImageList_RemoveAll(tabdat->himlDrag);
				ImageList_Destroy(tabdat->himlDrag);
				tabdat->himlDrag = nullptr;
			}
		}
		if (tabdat->bCloseButton) {
			GetCursorPos(&pt);
			int iItem = TabCtrl_TestForCloseButton(tabdat, hwnd, &pt);
			if (iItem != -1)
				tabdat->pContainer->CloseTabByMouse(&pt);
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
		InvalidateRect(hwnd, nullptr, FALSE); /* wine: fix for erase/paint tab on mouse enter/leave tab. */
		if (wParam == TIMERID_HOVER_T &&  M.GetByte("d_tooltips", 0)) {
			KillTimer(hwnd, TIMERID_HOVER_T);

			GetCursorPos(&pt);
			if (abs(pt.x - ptMouseT.x) < 5 && abs(pt.y - ptMouseT.y) < 5) {
				CLCINFOTIP ti = {};
				ti.cbSize = sizeof(ti);
				ti.ptCursor = pt;

				int nItem = GetTabItemFromMouse(hwnd, &pt);
				if (nItem >= 0 && nItem < TabCtrl_GetItemCount(hwnd)) {
					// get the message window data for the session to which this tab item belongs
					HWND hDlg = GetTabWindow(hwnd, nItem);
					CMsgDialog *dat = nullptr;
					if (IsWindow(hDlg) && hDlg != 0)
						dat = (CMsgDialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					if (dat) {
						tabdat->bTipActive = true;
						ti.isGroup = 0;
						ti.hItem = (HANDLE)dat->m_hContact;
						ti.isTreeFocused = 0;
						Tipper_ShowTip(0, &ti);
					}
				}
			}
		}
		break;

	case WM_MOUSEWHEEL:
		if (lParam == -1) {
			short amount = short(HIWORD(wParam));
			if (amount > 0)
				tabdat->pContainer->SelectTab(DM_SELECT_PREV);
			else if (amount < 0)
				tabdat->pContainer->SelectTab(DM_SELECT_NEXT);
			InvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case WM_USER + 100:
		if (tabdat->bTipActive) {
			tabdat->bTipActive = false;
			Tipper_Hide();
		}
		break;
	}

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

	for (int i = 0; tabcolors[i].szKey != nullptr; i++)
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

	for (auto &it : PluginConfig.tabConfig.m_brushes)
		if (it)
			DeleteObject(it);

	memset(&PluginConfig.tabConfig, 0, sizeof(PluginConfig.tabConfig));
}

/////////////////////////////////////////////////////////////////////////////////////////
// register the new tab control as a window class (TSTabCtrlClass)

int TSAPI RegisterTabCtrlClass(void)
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);

	wc.lpszClassName = L"TSTabCtrlClass";
	wc.lpfnWndProc = TabControlSubclassProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(TabControlData*);
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC;
	RegisterClassEx(&wc);

	wc.lpszClassName = L"TSStatusBarClass";
	wc.lpfnWndProc = &CMsgDialog::StatusBarSubclassProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(void*);
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC;
	RegisterClassEx(&wc);

	wc.lpszClassName = L"TS_SideBarClass";
	wc.lpfnWndProc = CSideBar::wndProcStub;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(void*);
	wc.style = CS_GLOBALCLASS;// | CS_DBLCLKS; // | CS_PARENTDC;
	RegisterClassEx(&wc);

	wc.lpszClassName = L"TSHK";
	wc.lpfnWndProc = HotkeyHandlerDlgProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(void*);
	wc.style = CS_GLOBALCLASS;// | CS_DBLCLKS; // | CS_PARENTDC;
	RegisterClassEx(&wc);
	return 0;
}
