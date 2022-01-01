/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "../stdafx.h"

struct HyperlinkWndData
{
	HFONT hEnableFont, hDisableFont;
	RECT rcText;
	COLORREF enableColor, disableColor, focusColor;
	uint8_t flags; /* see HLKF_* */
};

/* flags */
#define HLKF_HASENABLECOLOR   0x1 /* dat->enableColor is not system default */
#define HLKF_HASDISABLECOLOR  0x2 /* dat->disableColor is not system default */

/* internal messages */
#define HLK_MEASURETEXT  (WM_USER+1)
#define HLK_INVALIDATE   (WM_USER+2)

static LRESULT CALLBACK HyperlinkWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HyperlinkWndData *dat = (HyperlinkWndData*)GetWindowLongPtr(hwnd, 0);

	HDC hdc;
	RECT rc;
	POINT pt;
	HFONT hFont;
	LOGFONT lf;
	HCURSOR hCursor;
	COLORREF prevColor;

	switch (msg) {
	case WM_NCCREATE:
		dat = (struct HyperlinkWndData*)mir_calloc(sizeof(struct HyperlinkWndData));
		if (dat == nullptr)
			return FALSE; /* fail creation */
		SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat); /* always succeeds */
		/* fall thru */

	case WM_SYSCOLORCHANGE:
		if (!(dat->flags&HLKF_HASENABLECOLOR)) {
			if (GetSysColorBrush(COLOR_HOTLIGHT) == nullptr) dat->enableColor = RGB(0, 0, 255);
			else dat->enableColor = GetSysColor(COLOR_HOTLIGHT);
			dat->focusColor = RGB(GetRValue(dat->enableColor) / 2, GetGValue(dat->enableColor) / 2, GetBValue(dat->enableColor) / 2);
		}
		if (!(dat->flags&HLKF_HASDISABLECOLOR))
			dat->disableColor = GetSysColor(COLOR_GRAYTEXT);
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
		break;

	case WM_MOUSEACTIVATE:
		SetFocus(hwnd);
		return MA_ACTIVATE;

	case WM_GETDLGCODE:
		if (lParam) {
			MSG *pMsg = (MSG *)lParam;
			if (pMsg->message == WM_KEYDOWN) {
				if (pMsg->wParam == VK_TAB)
					return 0;
				if (pMsg->wParam == VK_ESCAPE)
					return 0;
			}
			else if (pMsg->message == WM_CHAR) {
				if (pMsg->wParam == '\t')
					return 0;
				if (pMsg->wParam == 27)
					return 0;
			}
		}
		return DLGC_WANTMESSAGE;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_SPACE:
		case VK_RETURN:
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), STN_CLICKED), (LPARAM)hwnd);
			break;
		}
		return 0;

	case WM_LBUTTONDOWN:
		POINTSTOPOINT(pt, MAKEPOINTS(lParam));
		if (!PtInRect(&dat->rcText, pt)) break;
		SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), STN_CLICKED), (LPARAM)hwnd);
		return 0;

	case WM_SETFONT:
		if ((HFONT)wParam == nullptr) { /* use default system color */
			dat->hEnableFont = dat->hDisableFont = nullptr;
			return 0;
		}
		if (GetObject((HFONT)wParam, sizeof(lf), &lf)) {
			lf.lfUnderline = 1;
			hFont = CreateFontIndirect(&lf);
			if (hFont != nullptr) {
				dat->hEnableFont = hFont;
				dat->hDisableFont = (HFONT)wParam;
				if (LOWORD(lParam)) SendMessage(hwnd, HLK_INVALIDATE, 0, 0);
				SendMessage(hwnd, HLK_MEASURETEXT, 0, 0);
			}
		}
		return 0;
	
	case WM_ERASEBKGND:
		return TRUE;
	
	case WM_ENABLE:
	case HLK_INVALIDATE:
		if (GetWindowRect(hwnd, &rc)) {
			pt.x = rc.left;
			pt.y = rc.top;
			
			HWND hwndParent = GetParent(hwnd);
			if (hwndParent == nullptr)
				hwndParent = hwnd;
			if (!ScreenToClient(hwndParent, &pt))
				break;

			rc.right = pt.x + (rc.right - rc.left);
			rc.bottom = pt.y + (rc.bottom - rc.top);
			rc.left = pt.x;
			rc.top = pt.y;
			InvalidateRect(hwndParent, &rc, TRUE);
		}
		return 0;

	case WM_GETFONT:
		return (LRESULT)dat->hDisableFont;
	
	case WM_CREATE:
	case HLK_MEASURETEXT:
		wchar_t szText[256];
		if (!GetWindowText(hwnd, szText, _countof(szText))) return 0;
		lParam = (LPARAM)szText;
		/* fall thru */

	case WM_SETTEXT:
		hdc = GetDC(hwnd);
		if (hdc == nullptr)  /* text change failed */
			return 0;
		else {
			BOOL fMeasured = FALSE;
			HFONT hPrevFont = nullptr;
			if (dat->hEnableFont != nullptr) hPrevFont = (HFONT)SelectObject(hdc, dat->hEnableFont);
			if (dat->hEnableFont == nullptr || hPrevFont != nullptr) { /* select failed? */
				SIZE textSize;
				if (GetTextExtentPoint32(hdc, (wchar_t*)lParam, (int)mir_wstrlen((wchar_t*)lParam), &textSize)) {
					if (GetClientRect(hwnd, &rc)) {
						dat->rcText.top = 0;
						dat->rcText.bottom = dat->rcText.top + textSize.cy;
						LONG style = GetWindowLongPtr(hwnd, GWL_STYLE);
						if (style & SS_CENTER) dat->rcText.left = (rc.right - textSize.cx) / 2;
						else if (style & SS_RIGHT) dat->rcText.left = rc.right - textSize.cx;
						else dat->rcText.left = 0;
						dat->rcText.right = dat->rcText.left + textSize.cx;
						fMeasured = TRUE;
					}
				}
			}
			if (dat->hEnableFont != nullptr && hPrevFont != nullptr)
				SelectObject(hdc, hPrevFont);
			ReleaseDC(hwnd, hdc);
			if (!fMeasured) /* text change failed */
				return 0;

			SendMessage(hwnd, HLK_INVALIDATE, 0, 0);
		}
		break;

	case WM_SETCURSOR:
		if (!GetCursorPos(&pt)) return FALSE;
		if (!ScreenToClient(hwnd, &pt)) return FALSE;
		if (PtInRect(&dat->rcText, pt)) {
			hCursor = (HCURSOR)GetClassLongPtr(hwnd, GCLP_HCURSOR);
			if (hCursor == nullptr)
				hCursor = LoadCursor(nullptr, IDC_HAND); /* Win2000+ */
		}
		else hCursor = LoadCursor(nullptr, IDC_ARROW);
		SetCursor(hCursor);
		return TRUE;

	case HLK_SETENABLECOLOUR:
		prevColor = dat->enableColor;
		dat->enableColor = (COLORREF)wParam;
		dat->focusColor = RGB(GetRValue(dat->enableColor) / 2, GetGValue(dat->enableColor) / 2, GetBValue(dat->enableColor) / 2);
		dat->flags |= HLKF_HASENABLECOLOR;
		return (LRESULT)prevColor;

	case HLK_SETDISABLECOLOUR:
		prevColor = dat->disableColor;
		dat->disableColor = (COLORREF)wParam;
		dat->flags |= HLKF_HASDISABLECOLOR;
		return (LRESULT)prevColor;

	case WM_NCPAINT:
		return 0;

	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		if (hdc != nullptr) {
			HFONT hPrevFont;
			COLORREF textColor;
			if (IsWindowEnabled(hwnd)) {
				hPrevFont = (HFONT)SelectObject(hdc, dat->hEnableFont);
				textColor = (GetFocus() == hwnd) ? dat->focusColor : dat->enableColor;
			}
			else {
				hPrevFont = (HFONT)SelectObject(hdc, dat->hDisableFont);
				textColor = dat->disableColor;
			}
			if (GetClientRect(hwnd, &rc) && GetWindowText(hwnd, szText, _countof(szText))) {
				BOOL fSmoothing;
				UINT fSmoothingType;
				SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &fSmoothing, 0);
				SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &fSmoothingType, 0);
				if (fSmoothing && fSmoothingType == FE_FONTSMOOTHINGCLEARTYPE)
					DrawThemeParentBackground(hwnd, hdc, &rc);
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, textColor);
				UINT alignFlag = (GetWindowLongPtr(hwnd, GWL_STYLE) & (SS_CENTER | SS_RIGHT | SS_LEFT));
				DrawText(hdc, szText, -1, &rc, alignFlag | DT_NOPREFIX | DT_SINGLELINE | DT_TOP);
			}
			if (hPrevFont != nullptr) SelectObject(hdc, hPrevFont);
			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_NCDESTROY:
		if (dat->hEnableFont != nullptr) DeleteObject(dat->hEnableFont);
		mir_free(dat);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InitHyperlink(void)
{
	g_hCursorNS = LoadCursor(nullptr, IDC_SIZENS);
	g_hCursorWE = LoadCursor(nullptr, IDC_SIZEWE);

	WNDCLASS wcl = { 0 };
	wcl.lpfnWndProc = HyperlinkWndProc;
	wcl.cbWndExtra = sizeof(struct HyperlinkWndData*);
	wcl.hInstance = g_hInst;
	wcl.lpszClassName = WNDCLASS_HYPERLINK;
	wcl.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	RegisterClass(&wcl);  /* automatically unregistered on exit */
}
