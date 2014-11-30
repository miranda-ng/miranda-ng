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
#include "m_descbutton.h"

extern HINSTANCE hInst;

////////////////////////////////////////////////////////////////////////////////////
// Internals

#define DBC_BORDER_SIZE		7
#define DBC_VSPACING		15
#define DBC_HSPACING		10

static LRESULT CALLBACK MDescButtonWndProc(HWND hwnd, UINT  msg, WPARAM wParam, LPARAM lParam);

// structure is used for storing list of tab info
typedef struct {
	HWND		hwnd;
	BOOL		bSharedIcon;
	HICON		hIcon;
	TCHAR		*lpzTitle;
	TCHAR		*lpzDescription;

	// UI info
	BOOL		bMouseInside;
	RECT		rc;
	int			width, height;
	HFONT		hfntTitle;

	// control colors
	RGBQUAD		rgbBkgTop, rgbBkgBottom;
	RGBQUAD		rgbSelTop, rgbSelBottom;
	RGBQUAD		rgbHotTop, rgbHotBottom;
	COLORREF	clText, clBackground;
	COLORREF	clSelText, clSelBorder;
	COLORREF	clHotText, clHotBorder;

	// fonts
	HFONT		hFont;
} MDescButtonCtrl;

int LoadDescButtonModule()
{
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = MIRANDADESCBUTTONCLASS;
	wc.lpfnWndProc = MDescButtonWndProc;
	wc.hCursor = LoadCursor(NULL, IDC_HAND);
	wc.cbWndExtra = sizeof(MDescButtonCtrl *);
	wc.hbrBackground = 0; //GetStockObject(WHITE_BRUSH);
	wc.style = CS_GLOBALCLASS|CS_SAVEBITS;
	RegisterClassEx(&wc);
	return 0;
}

static void MDescButton_SetupColors(MDescButtonCtrl *dat)
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

	dat->clBackground	 = GetSysColor(COLOR_3DFACE);
	dat->clText			 = GetSysColor(COLOR_WINDOWTEXT);
	dat->clSelText		 = GetSysColor(COLOR_HIGHLIGHTTEXT);
	dat->clSelBorder	 = RGB(dat->rgbSelTop.rgbRed, dat->rgbSelTop.rgbGreen, dat->rgbSelTop.rgbBlue);
	dat->clHotBorder	 = RGB(dat->rgbHotTop.rgbRed, dat->rgbHotTop.rgbGreen, dat->rgbHotTop.rgbBlue);

	if (!dat->hFont) dat->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

static void MDescButton_FillRect(HDC hdc, int x, int y, int width, int height, COLORREF cl)
{
	int oldMode			 = SetBkMode(hdc, OPAQUE);
	COLORREF oldColor	 = SetBkColor(hdc, cl);

	RECT rc; SetRect(&rc, x, y, x+width, y+height);
	ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, &rc, "", 0, 0);

	SetBkMode(hdc, oldMode);
	SetBkColor(hdc, oldColor);
}

static void MDescButton_DrawGradient(HDC hdc, int x, int y, int width, int height, RGBQUAD *rgb0, RGBQUAD *rgb1)
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

static LRESULT MDescButton_OnPaint(HWND hwndDlg, MDescButtonCtrl *dat, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HBITMAP hBmp, hOldBmp;
	RECT temprc;
	HFONT hfntSave;

	HDC hdc = BeginPaint(hwndDlg, &ps);
	HDC tempDC = CreateCompatibleDC(hdc);

	SIZE titleSize = {0};

	hBmp = CreateCompatibleBitmap(hdc, dat->width, dat->height);
	hOldBmp = (HBITMAP)SelectObject(tempDC, hBmp);

	temprc.left = 0;
	temprc.right = dat->width;
	temprc.top = 0;

	//Draw background
	if (dat->bMouseInside || (GetFocus() == hwndDlg)) {
		MDescButton_FillRect(tempDC, 0, 0, dat->width, dat->height, dat->clSelBorder);
		MDescButton_DrawGradient(tempDC, 1, 1, dat->width-2, dat->height-2, &dat->rgbSelTop, &dat->rgbSelBottom);
		SetTextColor(tempDC, dat->clSelText);
	}
	else {
		MDescButton_FillRect(tempDC, 0, 0, dat->width, dat->height, dat->clBackground);
		SetTextColor(tempDC, dat->clText);
	}

	if (dat->hIcon)
		DrawIcon(tempDC, DBC_BORDER_SIZE, DBC_BORDER_SIZE, dat->hIcon);

	hfntSave = (HFONT)SelectObject(tempDC, dat->hFont);
	SetBkMode(tempDC, TRANSPARENT);

	if (dat->lpzTitle) {
		LOGFONT lf;
		RECT textRect;
		HFONT hfntSave;

		GetObject(dat->hFont, sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfHeight *= 1.5;
		hfntSave = (HFONT)SelectObject(tempDC, CreateFontIndirect(&lf));

		textRect.left = DBC_BORDER_SIZE + dat->hIcon ? 32 + DBC_VSPACING : 0;
		textRect.right = dat->width - DBC_BORDER_SIZE;
		textRect.top = DBC_BORDER_SIZE;
		textRect.bottom = dat->height - DBC_BORDER_SIZE;
		DrawText(tempDC, dat->lpzTitle, -1, &textRect, DT_TOP|DT_LEFT|DT_END_ELLIPSIS);
		GetTextExtentPoint32(tempDC, dat->lpzTitle, mir_tstrlen(dat->lpzTitle), &titleSize);

		DeleteObject(SelectObject(tempDC, hfntSave));
	}

	if (dat->lpzDescription) {
		RECT textRect;
		textRect.left = DBC_BORDER_SIZE + dat->hIcon ? 32 + DBC_VSPACING : 0;
		textRect.right = dat->width - DBC_BORDER_SIZE;
		textRect.top = DBC_BORDER_SIZE + titleSize.cy ? titleSize.cy + DBC_HSPACING : 0;
		textRect.bottom = dat->height - DBC_BORDER_SIZE;
		DrawText(tempDC, dat->lpzDescription, -1, &textRect, DT_TOP|DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS);
		GetTextExtentPoint32(tempDC, dat->lpzTitle, mir_tstrlen(dat->lpzTitle), &titleSize);
	}

	SelectObject(tempDC, hfntSave);

	//Copy to output
	BitBlt(hdc, dat->rc.left, dat->rc.top, dat->width, dat->height, tempDC, 0, 0, SRCCOPY);
	SelectObject(tempDC, hOldBmp);
	DeleteObject(hBmp);
	DeleteDC(tempDC);
	EndPaint(hwndDlg, &ps);

	return TRUE;
}

static LRESULT CALLBACK MDescButtonWndProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	MDescButtonCtrl *dat = (MDescButtonCtrl *)GetWindowLongPtr(hwndDlg, 0);
	switch(msg) {
	case WM_NCCREATE:
		dat = (MDescButtonCtrl*)mir_alloc(sizeof(MDescButtonCtrl));
		if (dat == NULL)
			return FALSE;

		memset(dat, 0, sizeof(MDescButtonCtrl));
		SetWindowLongPtr(hwndDlg, 0, (LONG_PTR)dat);
		MDescButton_SetupColors(dat);
		return TRUE;

	case WM_SETFONT:
		dat->hFont = (HFONT)wParam;
		break;

	case WM_SIZE:
		GetClientRect(hwndDlg, &dat->rc);
		dat->width = dat->rc.right-dat->rc.left;
		dat->height = dat->rc.bottom-dat->rc.top;
		return TRUE;

	case WM_THEMECHANGED:
	case WM_STYLECHANGED:
		MDescButton_SetupColors(dat);
		return TRUE;

	case WM_MOUSEMOVE:
		if (!dat->bMouseInside) {
			TRACKMOUSEEVENT tme = {0};
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hwndDlg;
			_TrackMouseEvent(&tme);
			dat->bMouseInside = TRUE;
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		}
		return 0;

	case WM_MOUSELEAVE:
		dat->bMouseInside = FALSE;
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return 0;

	case WM_LBUTTONUP:
		SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKEWPARAM(GetWindowLongPtr(hwndDlg, GWL_ID), 0), 0);
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_NCPAINT:
		InvalidateRect(hwndDlg, NULL, FALSE);
		break;

	case WM_PAINT:
		MDescButton_OnPaint(hwndDlg, dat, msg, wParam, lParam);
		break;

	case DBCM_SETTITLE:
		if (dat->lpzTitle)
			mir_free(dat->lpzTitle);
		if (wParam & MDBCF_UNICODE)
			dat->lpzTitle = mir_u2t((WCHAR *)lParam);
		else
			dat->lpzTitle = mir_a2t((char *)lParam);
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return TRUE;

	case DBCM_SETDESCRIPTION:
		if (dat->lpzDescription)
			mir_free(dat->lpzDescription);
		if (wParam & MDBCF_UNICODE)
			dat->lpzDescription = mir_u2t((WCHAR *)lParam);
		else
			dat->lpzDescription = mir_a2t((char *)lParam);
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return TRUE;

	case DBCM_SETICON:
		if (dat->hIcon && !dat->bSharedIcon)
			DestroyIcon(dat->hIcon);

		if (wParam & MDBCF_SHAREDICON) {
			dat->bSharedIcon = TRUE;
			dat->hIcon = (HICON)lParam;
		}
		else {
			dat->bSharedIcon = FALSE;
			dat->hIcon = CopyIcon((HICON)lParam);
		}
		RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE);
		return TRUE;

	case WM_DESTROY:
		if (dat->lpzTitle)
			mir_free(dat->lpzTitle);
		if (dat->lpzDescription)
			mir_free(dat->lpzDescription);
		if (dat->hIcon && !dat->bSharedIcon)
			DestroyIcon(dat->hIcon);
		mir_free(dat);
		return TRUE;
	}

	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}
