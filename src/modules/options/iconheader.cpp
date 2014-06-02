/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
Copyright (c) 2007 Artem Shpynov
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

#include "..\..\core\commonheaders.h"
#include "m_iconheader.h"

extern HINSTANCE hInst;

static BOOL IsAeroMode()
{
	BOOL result;
	return dwmIsCompositionEnabled && (dwmIsCompositionEnabled(&result) == S_OK) && result;
}

static BOOL IsVSMode()
{
	return IsWinVerVistaPlus() && IsThemeActive();
}

////////////////////////////////////////////////////////////////////////////////////
// Internals

#define ITC_BORDER_SIZE		3

static LRESULT CALLBACK MIcoTabWndProc(HWND hwnd, UINT  msg, WPARAM wParam, LPARAM lParam);

// structure is used for storing list of tab info
struct MIcoTabCtrl : public MZeroedObject
{
	MIcoTabCtrl(): pList(1) {}

	HWND		hwnd;
	int			nSelectedIdx, nHotIdx;
	LIST<MIcoTab> pList;

	// UI info
	BOOL		bMouseInside;
	RECT		rc;
	int			width, height;
	int			itemWidth, itemHeight;

	//background bitmap
	HBITMAP		hBkgBmp;
	HBITMAP		hBkgOldBmp;
	HDC			hBkgDC;
	SIZE		BkgSize;

	// control colors
	RGBQUAD		rgbBkgTop, rgbBkgBottom;
	RGBQUAD		rgbSelTop, rgbSelBottom;
	RGBQUAD		rgbHotTop, rgbHotBottom;
	COLORREF	clText;
	COLORREF	clSelText, clSelBorder;
	COLORREF	clHotText, clHotBorder;

	// fonts
	HFONT		hFont;
};

typedef void (*ItemDestuctor)(void*);

static void MITListDestructor(void * adr)
{
	MIcoTab * mit = (MIcoTab *)adr;
	mir_free(mit->tcsName);
	if (mit->hIcon && !(mit->flag&MITCF_SHAREDICON))
		DestroyIcon(mit->hIcon);
	mir_free(adr);
}

void li_ListDestruct(LIST<MIcoTab> &pList, ItemDestuctor pItemDestructor)
{
	for (int i=0; i<pList.getCount(); i++)	pItemDestructor(pList[i]);
	pList.destroy();
}

int LoadIcoTabsModule()
{
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = MIRANDAICOTABCLASS;
	wc.lpfnWndProc = MIcoTabWndProc;
//	wc.hCursor = LoadCursor(NULL, IDC_HAND);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra = sizeof(MIcoTabCtrl*);
	wc.hbrBackground = 0; //GetStockObject(WHITE_BRUSH);
	wc.style = CS_GLOBALCLASS/*|CS_SAVEBITS*/;
	RegisterClassEx(&wc);
	return 0;
}

static void MIcoTab_SetupColors(MIcoTabCtrl *dat)
{
	COLORREF cl;

	cl = GetSysColor(COLOR_WINDOW);
	dat->rgbBkgBottom.rgbRed	 = (dat->rgbBkgTop.rgbRed	 = GetRValue(cl)) * .95;
	dat->rgbBkgBottom.rgbGreen	 = (dat->rgbBkgTop.rgbGreen	 = GetGValue(cl)) * .95;
	dat->rgbBkgBottom.rgbBlue	 = (dat->rgbBkgTop.rgbBlue	 = GetBValue(cl)) * .95;

	cl = GetSysColor(COLOR_HIGHLIGHT);
	dat->rgbSelTop.rgbRed	 = (dat->rgbSelBottom.rgbRed		 = GetRValue(cl)) * .75;
	dat->rgbSelTop.rgbGreen	 = (dat->rgbSelBottom.rgbGreen	 = GetGValue(cl)) * .75;
	dat->rgbSelTop.rgbBlue	 = (dat->rgbSelBottom.rgbBlue	 = GetBValue(cl)) * .75;

	dat->rgbHotTop.rgbRed	 = (dat->rgbSelTop.rgbRed	+ 255) / 2;
	dat->rgbHotTop.rgbGreen	 = (dat->rgbSelTop.rgbGreen	+ 255) / 2;
	dat->rgbHotTop.rgbBlue	 = (dat->rgbSelTop.rgbBlue	+ 255) / 2;

	dat->rgbHotBottom.rgbRed	 = (dat->rgbSelBottom.rgbRed		+ 255) / 2;
	dat->rgbHotBottom.rgbGreen	 = (dat->rgbSelBottom.rgbGreen	+ 255) / 2;
	dat->rgbHotBottom.rgbBlue	 = (dat->rgbSelBottom.rgbBlue	+ 255) / 2;

	dat->clText			 = GetSysColor(COLOR_WINDOWTEXT);
	dat->clSelText		 = GetSysColor(COLOR_HIGHLIGHTTEXT);
	dat->clSelBorder	 = RGB(dat->rgbSelTop.rgbRed, dat->rgbSelTop.rgbGreen, dat->rgbSelTop.rgbBlue);
	dat->clHotBorder	 = RGB(dat->rgbHotTop.rgbRed, dat->rgbHotTop.rgbGreen, dat->rgbHotTop.rgbBlue);

	if (!dat->hFont) dat->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

static void MIcoTab_FillRect(HDC hdc, int x, int y, int width, int height, COLORREF cl)
{
	int oldMode			 = SetBkMode(hdc, OPAQUE);
	COLORREF oldColor	 = SetBkColor(hdc, cl);

	RECT rc; SetRect(&rc, x, y, x+width, y+height);
	ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, &rc, "", 0, 0);

	SetBkMode(hdc, oldMode);
	SetBkColor(hdc, oldColor);
}

static void MIcoTab_DrawGradient(HDC hdc, int x, int y, int width, int height, RGBQUAD *rgb0, RGBQUAD *rgb1)
{
	int oldMode			 = SetBkMode(hdc, OPAQUE);
	COLORREF oldColor	 = SetBkColor(hdc, 0);

	RECT rc; SetRect(&rc, x, 0, x+width, 0);
	for (int i = y+height; --i >= y;) {
		COLORREF color = RGB(
			((height-i-1)*rgb0->rgbRed   + i*rgb1->rgbRed)   / height,
			((height-i-1)*rgb0->rgbGreen + i*rgb1->rgbGreen) / height,
			((height-i-1)*rgb0->rgbBlue  + i*rgb1->rgbBlue)  / height);
		rc.top = rc.bottom = i;
		++rc.bottom;
		SetBkColor(hdc, color);
		ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, &rc, "", 0, 0);
	}

	SetBkMode(hdc, oldMode);
	SetBkColor(hdc, oldColor);
}

static void MIcoTab_DrawItem(HWND hwnd, HDC hdc, MIcoTabCtrl *dat, MIcoTab *tab, int i)
{
	int iTopSpace = IsAeroMode() ? 0 : ITC_BORDER_SIZE;
	int itemX	 = ITC_BORDER_SIZE + dat->itemWidth * i;
	int iconTop	 = iTopSpace + 5;
	int textTop	 = iconTop + 32 + 3;

	HFONT hFntSave = NULL;

	if (dat->nSelectedIdx == i) {
		LOGFONT lf;
		GetObject(GetCurrentObject(hdc, OBJ_FONT), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		hFntSave = (HFONT)SelectObject(hdc, CreateFontIndirect(&lf));

		if (IsVSMode()) {
			RECT rc;
			rc.left = itemX;
			rc.top = iTopSpace;
			rc.right = itemX + dat->itemWidth;
			rc.bottom = iTopSpace + dat->itemHeight;
			HANDLE hTheme = OpenThemeData(hwnd, L"ListView");
			if (dat->nHotIdx == i || GetFocus() == hwnd)
				DrawThemeBackground(hTheme, hdc, LVP_LISTITEM, LISS_HOTSELECTED, &rc, NULL);
			else
				DrawThemeBackground(hTheme, hdc, LVP_LISTITEM, LISS_SELECTED, &rc, NULL);

			CloseThemeData(hTheme);
		}
		else {
			MIcoTab_FillRect(hdc, itemX, ITC_BORDER_SIZE, dat->itemWidth, dat->itemHeight, dat->clSelBorder);
			MIcoTab_DrawGradient(hdc, itemX+1, ITC_BORDER_SIZE+1, dat->itemWidth-2, dat->itemHeight-2, &dat->rgbSelTop, &dat->rgbSelBottom);
		}
		SetTextColor(hdc, dat->clSelText);
	}
	else if (dat->nHotIdx == i) {
		if (IsVSMode()) {
			RECT rc;
			rc.left = itemX;
			rc.top = iTopSpace;
			rc.right = itemX + dat->itemWidth;
			rc.bottom = iTopSpace + dat->itemHeight;
			SetWindowTheme(hwnd, L"explorer", NULL);
			HANDLE hTheme = OpenThemeData(hwnd, L"ListView");
			DrawThemeBackground(hTheme, hdc, LVP_LISTITEM, LISS_HOT, &rc, NULL);
			CloseThemeData(hTheme);
		}
		else {
			MIcoTab_FillRect(hdc, itemX, ITC_BORDER_SIZE, dat->itemWidth, dat->itemHeight, dat->clHotBorder);
			MIcoTab_DrawGradient(hdc, itemX+1, ITC_BORDER_SIZE+1, dat->itemWidth-2, dat->itemHeight-2, &dat->rgbHotTop, &dat->rgbHotBottom);
		}
		SetTextColor(hdc, dat->clHotText);
	}
	else SetTextColor(hdc, dat->clText);

	RECT textRect;
	textRect.left = itemX;
	textRect.right = itemX+dat->itemWidth;
	textRect.top = textTop;
	textRect.bottom = iconTop+dat->itemHeight;
	DrawIcon(hdc, itemX+dat->itemWidth/2-16, iconTop, tab->hIcon);

	if (IsVSMode()) {
		DTTOPTS dto = {0};
		dto.dwSize = sizeof(dto);
		dto.dwFlags = DTT_COMPOSITED|DTT_GLOWSIZE;
		dto.iGlowSize = 10;
		HANDLE hTheme = OpenThemeData(hwnd, L"Window");
		wchar_t *tcsNameW = mir_t2u(tab->tcsName);
		drawThemeTextEx(hTheme, hdc, WP_CAPTION, CS_ACTIVE, tcsNameW, -1, DT_VCENTER|DT_CENTER|DT_END_ELLIPSIS, &textRect, &dto);
		mir_free(tcsNameW);
		CloseThemeData(hTheme);
	}
	else DrawText(hdc, tab->tcsName, -1, &textRect, DT_VCENTER|DT_CENTER|DT_END_ELLIPSIS);

	if (hFntSave)
		DeleteObject(SelectObject(hdc, hFntSave));
}

static LRESULT MIcoTab_OnPaint(HWND hwndDlg, MIcoTabCtrl *mit, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hwndDlg, &ps);
	HDC tempDC = CreateCompatibleDC(hdc);

	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = mit->width;
	bmi.bmiHeader.biHeight = -mit->height; // we need this for DrawThemeTextEx
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	HBITMAP hBmp = CreateDIBSection(tempDC, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);

	HBITMAP hOldBmp = (HBITMAP)SelectObject(tempDC, hBmp);

	if (IsAeroMode()) {
		RECT temprc;
		temprc.left = 0;
		temprc.right = mit->width;
		temprc.top = 0;
		temprc.bottom = mit->width;
		FillRect(tempDC, &temprc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
	else {
		if (mit->hBkgBmp)
			StretchBlt(tempDC, 0, 0, mit->width, mit->height, mit->hBkgDC, 0, 0, mit->BkgSize.cx, mit->BkgSize.cy, SRCCOPY);
		else {
			if (IsVSMode())
				MIcoTab_FillRect(tempDC, 0, 0, mit->width, mit->height, GetSysColor(COLOR_WINDOW));
			else
				MIcoTab_DrawGradient(tempDC, 0, 0, mit->width, mit->height, &mit->rgbBkgTop, &mit->rgbBkgBottom);

			MIcoTab_FillRect(tempDC, 0, mit->height-2, mit->width, 1, GetSysColor(COLOR_BTNSHADOW));
			MIcoTab_FillRect(tempDC, 0, mit->height-1, mit->width, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
		}
	}

	//Draw Items
	HFONT hFont = mit->hFont;
	HFONT hOldFont = (HFONT)SelectObject(tempDC, hFont);
	SetBkMode(tempDC, TRANSPARENT);

	for (int i=0; i<mit->pList.getCount(); i++) {
		MIcoTab *tab = (MIcoTab *)mit->pList[i];
		MIcoTab_DrawItem(hwndDlg, tempDC, mit, tab, i);
	}

	//Copy to output
	BitBlt(hdc, mit->rc.left, mit->rc.top, mit->width, mit->height, tempDC, 0, 0, SRCCOPY);
	SelectObject(tempDC, hOldBmp);
	DeleteObject(hBmp);
	SelectObject(tempDC,hOldFont);
	DeleteDC(tempDC);

	EndPaint(hwndDlg, &ps);

	return TRUE;
}

static LRESULT CALLBACK MIcoTabWndProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	MIcoTabCtrl* itc = (MIcoTabCtrl *)GetWindowLongPtr(hwndDlg, 0);
	switch(msg) {
	case WM_NCCREATE:
		itc = new MIcoTabCtrl; //(MIcoTabCtrl*)mir_alloc(sizeof(MIcoTabCtrl));
		if (itc == NULL) return FALSE;
		itc->nSelectedIdx = -1;
		itc->nHotIdx = -1;
		itc->bMouseInside = FALSE;
		SetWindowLongPtr(hwndDlg, 0, (LONG_PTR)itc);
		MIcoTab_SetupColors(itc);

		if (IsAeroMode()) {
			RECT rc; GetWindowRect(hwndDlg, &rc);
			MARGINS margins = {0, 0, rc.bottom-rc.top, 0};
			dwmExtendFrameIntoClientArea(GetParent(hwndDlg), &margins);
		}

		return TRUE;

	case WM_SETFONT:
		itc->hFont = (HFONT)wParam;
		break;

	case WM_SIZE:
		GetClientRect(hwndDlg, &itc->rc);
		itc->width = itc->rc.right-itc->rc.left;
		itc->height = itc->rc.bottom-itc->rc.top;

		if (itc->pList.getCount()) {
			itc->itemWidth = (itc->width-2*ITC_BORDER_SIZE)/itc->pList.getCount();
			itc->itemHeight = itc->height-2*ITC_BORDER_SIZE-2;
		}
		else itc->itemWidth = itc->itemHeight = 0;
		return TRUE;

	case WM_THEMECHANGED:
	case WM_STYLECHANGED:
		MIcoTab_SetupColors(itc);
		return TRUE;

	case WM_MOUSEMOVE:
		if (!itc->bMouseInside) {
			TRACKMOUSEEVENT tme = {0};
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwndDlg;
			_TrackMouseEvent(&tme);
			itc->bMouseInside = TRUE;
		}

		itc->nHotIdx = (LOWORD(lParam) - ITC_BORDER_SIZE) / itc->itemWidth;
		if (itc->nHotIdx >= itc->pList.getCount())
			itc->nHotIdx = -1;
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return 0;

	case WM_MOUSELEAVE:
		itc->bMouseInside = FALSE;
		itc->nHotIdx = -1;
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return 0;

	case WM_LBUTTONUP:
		if ((itc->nHotIdx >= 0) && (itc->nHotIdx != itc->nSelectedIdx))
		{
			itc->nSelectedIdx = itc->nHotIdx;
			SetWindowText(hwndDlg, itc->pList[itc->nSelectedIdx]->tcsName);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
			SendMessage(GetParent(hwndDlg), WM_COMMAND,
				MAKEWPARAM(GetWindowLongPtr(hwndDlg, GWL_ID), ITCN_SELCHANGED),
				itc->nSelectedIdx);
		}
		return 0;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		break;

	case WM_MOUSEACTIVATE:
		SetFocus(hwndDlg);
		return MA_ACTIVATE;

	case WM_GETDLGCODE:
	{
		if (lParam)
		{
			MSG *msg = (MSG *) lParam;
			if (msg->message == WM_KEYDOWN)
			{
				if (msg->wParam == VK_TAB)
					return 0;
				if (msg->wParam == VK_ESCAPE)
					return 0;
			} else
			if (msg->message == WM_CHAR)
			{
				if (msg->wParam == '\t')
					return 0;
				if (msg->wParam == 27)
					return 0;
			}
		}
		return DLGC_WANTMESSAGE;
	}

	case WM_KEYDOWN:
	{
		int newIdx = itc->nSelectedIdx;
		switch (wParam)
		{
		case VK_NEXT:
		case VK_RIGHT:
			newIdx++;
			break;
		case VK_PRIOR:
		case VK_LEFT:
			newIdx--;
			break;
		}
		if ((newIdx >= 0) && (newIdx < itc->pList.getCount()) && (newIdx != itc->nSelectedIdx))
		{
			itc->nSelectedIdx = newIdx;
			SetWindowText(hwndDlg, itc->pList[itc->nSelectedIdx]->tcsName);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
			SendMessage(GetParent(hwndDlg), WM_COMMAND,
				MAKEWPARAM(GetWindowLongPtr(hwndDlg, GWL_ID), ITCN_SELCHANGEDKBD),
				itc->nSelectedIdx);
		}
		return 0;
	}

	case WM_ERASEBKGND:
		return 1;

	case WM_NCPAINT:
		InvalidateRect(hwndDlg, NULL, FALSE);
		break;

	case WM_PAINT:
		MIcoTab_OnPaint(hwndDlg, itc, msg, wParam, lParam);
		break;

	case ITCM_SETBACKGROUND:
		itc->hBkgBmp = (HBITMAP)lParam;
		if (!itc->hBkgDC)
			itc->hBkgDC = CreateCompatibleDC(NULL);
		itc->hBkgOldBmp = (HBITMAP)SelectObject(itc->hBkgDC, itc->hBkgBmp);
		{
			BITMAPINFO bmp;
			GetObject(itc->hBkgBmp, sizeof(bmp), &bmp);
			itc->BkgSize.cx = bmp.bmiHeader.biWidth;
			itc->BkgSize.cy = bmp.bmiHeader.biHeight;
		}
		return TRUE;

	case ITCM_ADDITEM:
	{
		MIcoTab* pMit = (MIcoTab *)wParam;
		if (!pMit)
			return FALSE;

		MIcoTab* pListMit = (MIcoTab *)mir_calloc(sizeof(MIcoTab));
		pListMit->flag = pMit->flag;
		pListMit->data = pMit->data;
		if (pMit->flag & MITCF_UNICODE)
			pListMit->tcsName = mir_u2t(pMit->lpwzName);
		else
			pListMit->tcsName = mir_a2t(pMit->lpzName);
		if (pMit->hIcon) {
			if (pListMit->flag&MITCF_SHAREDICON)
				pListMit->hIcon = pMit->hIcon;
			else
				pListMit->hIcon = CopyIcon(pMit->hIcon);
		}
		itc->pList.insert(pListMit);

		itc->itemWidth = (itc->width-2*ITC_BORDER_SIZE)/itc->pList.getCount();
		itc->itemHeight = itc->height-2*ITC_BORDER_SIZE-2;

		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return TRUE;
	}

	case ITCM_SETSEL:
		if ((int)wParam >= 0 && (int)wParam < itc->pList.getCount()) {
			itc->nSelectedIdx = wParam;
			SetWindowText(hwndDlg, itc->pList[itc->nSelectedIdx]->tcsName);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
			SendMessage(GetParent(hwndDlg), WM_COMMAND,
				MAKEWPARAM(GetWindowLongPtr(hwndDlg, GWL_ID), ITCN_SELCHANGED),
				itc->nSelectedIdx);
		}
		return TRUE;

	case ITCM_GETSEL:
		return itc->nSelectedIdx;

	case ITCM_GETITEMDATA:
		if ((int)wParam >= 0 && (int)wParam < itc->pList.getCount())
			return ((MIcoTab *)itc->pList[wParam])->data;
		return 0;

	case WM_DESTROY:
		if (itc->hBkgDC) {
			SelectObject(itc->hBkgDC, itc->hBkgOldBmp);
			DeleteDC(itc->hBkgDC);
		}
		li_ListDestruct(itc->pList, MITListDestructor);
		delete itc;
		return TRUE;
	}
	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}
