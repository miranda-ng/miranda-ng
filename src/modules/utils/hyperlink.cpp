/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

struct HyperlinkWndData {
	HFONT hEnableFont, hDisableFont;
	RECT rcText;
	COLORREF enableColor, disableColor, focusColor;
	BYTE flags; /* see HLKF_* */
};

/* flags */
#define HLKF_HASENABLECOLOR   0x1 /* dat->enableColor is not system default */
#define HLKF_HASDISABLECOLOR  0x2 /* dat->disableColor is not system default */

/* internal messages */
#define HLK_MEASURETEXT  (WM_USER+1)
#define HLK_INVALIDATE   (WM_USER+2)

static LRESULT CALLBACK HyperlinkWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct HyperlinkWndData *dat = (struct HyperlinkWndData*)GetWindowLongPtr(hwnd, 0);
	switch(msg) {
		case WM_NCCREATE:
			dat = (struct HyperlinkWndData*)mir_calloc(sizeof(struct HyperlinkWndData));
			if (dat == NULL) return FALSE; /* fail creation */
			SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat); /* always succeeds */
			/* fall thru */
		case WM_SYSCOLORCHANGE:
			if (!(dat->flags&HLKF_HASENABLECOLOR)) {
				if (GetSysColorBrush(COLOR_HOTLIGHT) == NULL) dat->enableColor = RGB(0, 0, 255);
				else dat->enableColor = GetSysColor(COLOR_HOTLIGHT);
				dat->focusColor = RGB(GetRValue(dat->enableColor) / 2, GetGValue(dat->enableColor) / 2, GetBValue(dat->enableColor) / 2);
			}
			if (!(dat->flags&HLKF_HASDISABLECOLOR))
				dat->disableColor = GetSysColor(COLOR_GRAYTEXT);
			break;

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
			break;
		case WM_MOUSEACTIVATE:
			SetFocus(hwnd);
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
			switch (wParam)
			{
			case VK_SPACE:
			case VK_RETURN:
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), STN_CLICKED), (LPARAM)hwnd);
				break;
			}
			return 0;
		}

		case WM_LBUTTONDOWN:
		{	POINT pt;
			POINTSTOPOINT(pt, MAKEPOINTS(lParam));
			if (!PtInRect(&dat->rcText, pt)) break;
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), STN_CLICKED), (LPARAM)hwnd);
			return 0;
		}
		case WM_SETFONT:
		{	LOGFONT lf;
			HFONT hFont;
			if ((HFONT)wParam == NULL) { /* use default system color */
				dat->hEnableFont = dat->hDisableFont = NULL;
				return 0;
			}
			if (GetObject((HFONT)wParam, sizeof(lf), &lf)) {
				lf.lfUnderline = 1;
				hFont = CreateFontIndirect(&lf);
				if (hFont != NULL) {
					dat->hEnableFont = hFont;
					dat->hDisableFont = (HFONT)wParam;
					if (LOWORD(lParam)) SendMessage(hwnd, HLK_INVALIDATE, 0, 0);
					SendMessage(hwnd, HLK_MEASURETEXT, 0, 0);
				}
			}
			return 0;
		}
		case WM_ERASEBKGND:
			return TRUE;
		case WM_ENABLE:
		case HLK_INVALIDATE:
		{	RECT rcWnd;
			POINT pt;
			HWND hwndParent;
			if (!GetWindowRect(hwnd, &rcWnd)) break;
			pt.x = rcWnd.left;
			pt.y = rcWnd.top;
			hwndParent = GetParent(hwnd);
			if (hwndParent == NULL) hwndParent = hwnd;
			if (!ScreenToClient(hwndParent, &pt)) break;
			rcWnd.right = pt.x+(rcWnd.right-rcWnd.left);
			rcWnd.bottom = pt.y+(rcWnd.bottom-rcWnd.top);
			rcWnd.left = pt.x;
			rcWnd.top = pt.y;
			InvalidateRect(hwndParent, &rcWnd, TRUE);
			return 0;
		}
		case WM_GETFONT:
			return (LRESULT)dat->hDisableFont;
		case WM_CREATE:
		case HLK_MEASURETEXT:
		{	TCHAR szText[256];
			if (!GetWindowText(hwnd, szText, SIZEOF(szText))) return 0;
			lParam = (LPARAM)szText;
			/* fall thru */
		case WM_SETTEXT:
		{	HFONT hPrevFont = NULL;
			SIZE textSize;
			RECT rc;
			HDC hdc;
			LONG style;
			BOOL fMeasured = FALSE;
			hdc = GetDC(hwnd);
			if (hdc == NULL) return 0; /* text change failed */
			if (dat->hEnableFont != NULL) hPrevFont = (HFONT)SelectObject(hdc, dat->hEnableFont);
			if (dat->hEnableFont == NULL || hPrevFont != NULL) /* select failed? */
				if (GetTextExtentPoint32(hdc, (TCHAR*)lParam, (int)mir_tstrlen((TCHAR*)lParam), &textSize))
					if (GetClientRect(hwnd, &rc)) {
						dat->rcText.top = 0;
						dat->rcText.bottom = dat->rcText.top+textSize.cy;
						style = GetWindowLongPtr(hwnd, GWL_STYLE);
						if (style&SS_CENTER) dat->rcText.left = (rc.right-textSize.cx)/2;
						else if (style&SS_RIGHT) dat->rcText.left = rc.right-textSize.cx;
						else dat->rcText.left = 0;
						dat->rcText.right = dat->rcText.left+textSize.cx;
						fMeasured = TRUE;
					}
			if (dat->hEnableFont != NULL && hPrevFont != NULL) SelectObject(hdc, hPrevFont);
			ReleaseDC(hwnd, hdc);
			if (!fMeasured) return 0; /* text change failed */
			SendMessage(hwnd, HLK_INVALIDATE, 0, 0);
			break;
		}}
		case WM_SETCURSOR:
		{	POINT pt;
			HCURSOR hCursor;
			if (!GetCursorPos(&pt)) return FALSE;
			if (!ScreenToClient(hwnd, &pt)) return FALSE;
			if (PtInRect(&dat->rcText, pt)) {
				hCursor = (HCURSOR)GetClassLongPtr(hwnd, GCLP_HCURSOR);
				if (hCursor == NULL) hCursor = LoadCursor(NULL, IDC_HAND); /* Win2000+ */
			}
			else hCursor = LoadCursor(NULL, IDC_ARROW);
			SetCursor(hCursor);
			return TRUE;
		}
		case HLK_SETENABLECOLOUR:
		{	COLORREF prevColor = dat->enableColor;
			dat->enableColor = (COLORREF)wParam;
			dat->focusColor = RGB(GetRValue(dat->enableColor) / 2, GetGValue(dat->enableColor) / 2, GetBValue(dat->enableColor) / 2);
			dat->flags|=HLKF_HASENABLECOLOR;
			return (LRESULT)prevColor;
		}
		case HLK_SETDISABLECOLOUR:
		{	COLORREF prevColor = dat->disableColor;
			dat->disableColor = (COLORREF)wParam;
			dat->flags|=HLKF_HASDISABLECOLOR;
			return (LRESULT)prevColor;
		}
		case WM_NCPAINT:
			return 0;
		case WM_PAINT:
		{	HFONT hPrevFont;
			RECT rc;
			TCHAR szText[256];
			UINT alignFlag;
			COLORREF textColor;
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hwnd, &ps);
			if (hdc != NULL) {
				if (IsWindowEnabled(hwnd)) {
					hPrevFont = (HFONT)SelectObject(hdc, dat->hEnableFont);
					textColor = (GetFocus() == hwnd) ? dat->focusColor : dat->enableColor;
				} else {
					hPrevFont = (HFONT)SelectObject(hdc, dat->hDisableFont);
					textColor = dat->disableColor;
				}
				if (GetClientRect(hwnd, &rc) && GetWindowText(hwnd, szText, SIZEOF(szText))) {
					BOOL fSmoothing;
					UINT fSmoothingType;
					SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &fSmoothing, 0);
					SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &fSmoothingType, 0);
					if (fSmoothing && fSmoothingType == FE_FONTSMOOTHINGCLEARTYPE)
						DrawThemeParentBackground(hwnd, hdc, &rc);
					SetBkMode(hdc, TRANSPARENT);
					SetTextColor(hdc, textColor);
					alignFlag = (GetWindowLongPtr(hwnd, GWL_STYLE)&(SS_CENTER|SS_RIGHT|SS_LEFT));
					DrawText(hdc, szText, -1, &rc, alignFlag|DT_NOPREFIX|DT_SINGLELINE|DT_TOP);
				}
				if (hPrevFont != NULL) SelectObject(hdc, hPrevFont);
				EndPaint(hwnd, &ps);
			}
			return 0;
		}
		case WM_NCDESTROY:
			if (dat->hEnableFont != NULL) DeleteObject(dat->hEnableFont);
			mir_free(dat);
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int InitHyperlink(void)
{
	WNDCLASS wcl;

	wcl.lpfnWndProc = HyperlinkWndProc;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = sizeof(struct HyperlinkWndData*);
	wcl.hInstance = hInst;
	wcl.hCursor = NULL;
	wcl.lpszClassName = WNDCLASS_HYPERLINK;
	wcl.hbrBackground = NULL;
	wcl.hIcon = NULL;
	wcl.lpszMenuName = NULL;
	wcl.style = CS_HREDRAW|CS_VREDRAW|CS_GLOBALCLASS|CS_PARENTDC;
	RegisterClass(&wcl);  /* automatically unregistered on exit */
	return 0;
}
