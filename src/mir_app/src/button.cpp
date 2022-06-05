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

#include "stdafx.h"

#include <m_button_int.h>

#include <initguid.h>
#include <oleacc.h>

struct TTooltips
{
	uint32_t ThreadId;
	HWND  hwnd;
};

static LIST<TTooltips> lToolTips(1, NumericKeySortT);
static mir_cs csTips;
static bool bModuleInitialized = false;

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLID       100
#define BUTTON_POLLDELAY    50

static void DestroyTheme(MButtonCtrl *ctl)
{
	if (ctl->hThemeButton) {
		CloseThemeData(ctl->hThemeButton);
		ctl->hThemeButton = nullptr;
	}
	if (ctl->hThemeToolbar) {
		CloseThemeData(ctl->hThemeToolbar);
		ctl->hThemeToolbar = nullptr;
	}
	ctl->bIsThemed = false;
}

static void LoadTheme(MButtonCtrl *ctl)
{
	DestroyTheme(ctl);
	ctl->hThemeButton = OpenThemeData(ctl->hwnd, L"BUTTON");
	ctl->hThemeToolbar = OpenThemeData(ctl->hwnd, L"TOOLBAR");
	ctl->bIsThemed = true;
}

static int TBStateConvert2Flat(int state)
{
	switch(state) {
		case PBS_NORMAL:    return TS_NORMAL;
		case PBS_HOT:       return TS_HOT;
		case PBS_PRESSED:   return TS_PRESSED;
		case PBS_DISABLED:  return TS_DISABLED;
		case PBS_DEFAULTED: return TS_NORMAL;
	}
	return TS_NORMAL;
}

#ifndef DFCS_HOT
#define DFCS_HOT 0x1000
#endif

///////////////////////////////////////////////////////////////////////////////
// Button painter

static void PaintWorker(MButtonCtrl *ctl, HDC hdcPaint)
{
	if (!hdcPaint)
		return;

	RECT rcClient;
	GetClientRect(ctl->hwnd, &rcClient);

	HDC hdcMem = CreateCompatibleDC(hdcPaint);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdcPaint, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);
	HDC hOld = (HDC)SelectObject(hdcMem, hbmMem);

	// If its a push button, check to see if it should stay pressed
	if (ctl->bIsPushBtn && ctl->bIsPushed)
		ctl->stateId = PBS_PRESSED;

	// Draw the flat button
	if (ctl->bIsFlat) {
		if (ctl->hThemeToolbar && ctl->bIsThemed) {
			int state = IsWindowEnabled(ctl->hwnd)?(ctl->stateId == PBS_NORMAL && ctl->bIsDefault ? PBS_DEFAULTED : ctl->stateId):PBS_DISABLED;
			if (IsThemeBackgroundPartiallyTransparent(ctl->hThemeToolbar, TP_BUTTON, TBStateConvert2Flat(state)))
				DrawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);
			DrawThemeBackground(ctl->hThemeToolbar, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), &rcClient, &rcClient);
		}
		else {
			HBRUSH hbr;

			if (ctl->stateId == PBS_PRESSED || ctl->stateId == PBS_HOT)
				hbr = GetSysColorBrush(COLOR_3DLIGHT);
			else {
				HWND hwndParent = GetParent(ctl->hwnd);
				HDC dc = GetDC(hwndParent);
				HBRUSH oldBrush = (HBRUSH)GetCurrentObject(dc, OBJ_BRUSH);
				hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwndParent);
				SelectObject(dc, oldBrush);
				ReleaseDC(hwndParent, dc);
			}
			if (hbr)
				FillRect(hdcMem, &rcClient, hbr);
			if (ctl->stateId == PBS_HOT || ctl->focus) {
				if (ctl->bIsPushed)
					DrawEdge(hdcMem, &rcClient, EDGE_ETCHED, BF_RECT|BF_SOFT);
				else DrawEdge(hdcMem, &rcClient, BDR_RAISEDOUTER, BF_RECT|BF_SOFT|BF_FLAT);
			}
			else if (ctl->stateId == PBS_PRESSED)
				DrawEdge(hdcMem, &rcClient, BDR_SUNKENOUTER, BF_RECT|BF_SOFT);
		}
	}
	else {
		// Draw background/border
		if (ctl->hThemeButton && ctl->bIsThemed) {
			int state = IsWindowEnabled(ctl->hwnd)?(ctl->stateId == PBS_NORMAL && ctl->bIsDefault ? PBS_DEFAULTED : ctl->stateId) : PBS_DISABLED;

			if (IsThemeBackgroundPartiallyTransparent(ctl->hThemeButton, BP_PUSHBUTTON, state))
				DrawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);

			DrawThemeBackground(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, state, &rcClient, &rcClient);
		}
		else {
			UINT uState = DFCS_BUTTONPUSH | ((ctl->stateId == PBS_HOT) ? DFCS_HOT : 0) | ((ctl->stateId == PBS_PRESSED) ? DFCS_PUSHED : 0);
			if (ctl->bIsDefault && ctl->stateId == PBS_NORMAL)
				uState |= DLGC_DEFPUSHBUTTON;
			DrawFrameControl(hdcMem, &rcClient, DFC_BUTTON, uState);
		}

		// Draw focus rectangle if button has focus
		if (ctl->focus) {
			RECT focusRect = rcClient;
			InflateRect(&focusRect, -3, -3);
			DrawFocusRect(hdcMem, &focusRect);
		}
	}

	// If we have an icon or a bitmap, ignore text and only draw the image on the button
	int textLen = GetWindowTextLength(ctl->hwnd);

	if (ctl->hIcon) {
		int ix = (rcClient.right-rcClient.left)/2 - (g_iIconSX/2);
		int iy = (rcClient.bottom-rcClient.top)/2 - (g_iIconSY/2);
		if (ctl->stateId == PBS_PRESSED) {
			ix++;
			iy++;
		}

		HIMAGELIST hImageList = ImageList_Create(g_iIconSX, g_iIconSY, ILC_MASK | ILC_COLOR32, 1, 0);
		ImageList_AddIcon(hImageList, ctl->hIcon);
		HICON hIconNew = ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
		DrawState(hdcMem, nullptr, nullptr, (LPARAM) hIconNew, 0, ix, iy, g_iIconSX, g_iIconSY, DST_ICON | (IsWindowEnabled(ctl->hwnd) ? DSS_NORMAL : DSS_DISABLED));
		ImageList_RemoveAll(hImageList);
		ImageList_Destroy(hImageList);
		DestroyIcon(hIconNew);
	}
	else if (ctl->hBitmap) {
		BITMAP bminfo;
		int ix, iy;

		GetObject(ctl->hBitmap, sizeof(bminfo), &bminfo);
		ix = (rcClient.right-rcClient.left)/2 - (bminfo.bmWidth/2);
		iy = (rcClient.bottom-rcClient.top)/2 - (bminfo.bmHeight/2);
		if (ctl->stateId == PBS_PRESSED) {
			ix++;
			iy++;
		}
		DrawState(hdcMem, nullptr, nullptr, (LPARAM)ctl->hBitmap, 0, ix, iy, bminfo.bmWidth, bminfo.bmHeight, IsWindowEnabled(ctl->hwnd)?DST_BITMAP:DST_BITMAP|DSS_DISABLED);
	}
	else if (textLen > 0) {
		// Draw the text and optinally the arrow
		SetBkMode(hdcMem, TRANSPARENT);
		HFONT hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);

		SIZE sz;
		wchar_t szText[MAX_PATH];
		GetWindowText(ctl->hwnd, szText, _countof(szText));
		GetTextExtentPoint32(hdcMem, szText, (int)mir_wstrlen(szText), &sz);
		int xOffset = (rcClient.right - rcClient.left - sz.cx)/2;
		int yOffset = (rcClient.bottom - rcClient.top - sz.cy)/2;

		// XP w/themes doesn't used the glossy disabled text.  Is it always using COLOR_GRAYTEXT?  Seems so.
		SetTextColor(hdcMem, IsWindowEnabled(ctl->hwnd) || !ctl->hThemeButton?GetSysColor(COLOR_BTNTEXT):GetSysColor(COLOR_GRAYTEXT));
		//!! move it up, to text extent points?
		if (ctl->cHot) {
			SIZE szHot;
			GetTextExtentPoint32 (hdcMem, L"&", 1, &szHot);
			sz.cx -= szHot.cx;
		}
		if (ctl->arrow)
			DrawState(hdcMem, nullptr, nullptr, (LPARAM)ctl->arrow, 0, rcClient.right-rcClient.left-5-g_iIconSX+(!ctl->hThemeButton && ctl->stateId == PBS_PRESSED?1:0), (rcClient.bottom-rcClient.top)/2-g_iIconSY/2+(!ctl->hThemeButton && ctl->stateId == PBS_PRESSED?1:0), g_iIconSX, g_iIconSY, IsWindowEnabled(ctl->hwnd)?DST_ICON:DST_ICON|DSS_DISABLED);

		SelectObject(hdcMem, ctl->hFont);
		DrawState(hdcMem, nullptr, nullptr, (LPARAM)szText, 0,
			xOffset+(!ctl->hThemeButton && ctl->stateId == PBS_PRESSED?1:0),
			ctl->hThemeButton ? yOffset : yOffset - (ctl->stateId == PBS_PRESSED?0:1),
			sz.cx, sz.cy,
			IsWindowEnabled(ctl->hwnd) || ctl->hThemeButton?DST_PREFIXTEXT|DSS_NORMAL:DST_PREFIXTEXT|DSS_DISABLED);
		SelectObject(hdcMem, hOldFont);
	}
	BitBlt(hdcPaint, 0, 0, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);
	DeleteObject(hbmMem);
	DeleteDC(hdcMem);
}

///////////////////////////////////////////////////////////////////////////////
// Button's window procedure

static LRESULT CALLBACK MButtonWndProc(HWND hwnd, UINT msg,  WPARAM wParam, LPARAM lParam)
{
	MButtonCtrl *bct = (MButtonCtrl *)GetWindowLongPtr(hwnd, 0);

	switch(msg) {
	case WM_NCCREATE:
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | BS_OWNERDRAW);
		bct = (MButtonCtrl*)mir_calloc(sizeof(MButtonCtrl));
		if (bct == nullptr)
			return FALSE;
		bct->hwnd = hwnd;
		bct->stateId = PBS_NORMAL;
		bct->fnPainter = PaintWorker;
		bct->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		LoadTheme(bct);

		SetWindowLongPtr(hwnd, 0, (LONG_PTR)bct);
		if (((CREATESTRUCT *)lParam)->lpszName)
			SetWindowText(hwnd, ((CREATESTRUCT*)lParam)->lpszName);
		return TRUE;

	case WM_DESTROY:
		if (bct) {
			if (bct->hwndToolTips) {
				TOOLINFO ti = {0};
				ti.cbSize = sizeof(ti);
				ti.uFlags = TTF_IDISHWND;
				ti.hwnd = bct->hwnd;
				ti.uId = (UINT_PTR)bct->hwnd;
				if (SendMessage(bct->hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
					SendMessage(bct->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);

				if (SendMessage(bct->hwndToolTips, TTM_GETTOOLCOUNT, 0, (LPARAM)&ti) == 0) {
					TTooltips tt;
					tt.ThreadId = GetCurrentThreadId();

					mir_cslock lck(csTips);
					int idx = lToolTips.getIndex(&tt);
					if (idx != -1) {
						mir_free(lToolTips[idx]);
						lToolTips.remove(idx);
						DestroyWindow(bct->hwndToolTips);
					}
					bct->hwndToolTips = nullptr;
				}
			}
			if (bct->arrow)
				IcoLib_ReleaseIcon(bct->arrow);
			DestroyTheme(bct);
		}
		break;

	case WM_NCDESTROY:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		mir_free(bct);
		break;

	case WM_SETTEXT:
		bct->cHot = 0;
		if (lParam != 0) {
			wchar_t *tmp = (wchar_t*)lParam;
			while (*tmp) {
				if (*tmp == '&' && *(tmp+1)) {
					bct->cHot = _tolower(*(tmp+1));
					break;
				}
				tmp++;
			}
			InvalidateRect(bct->hwnd, nullptr, TRUE);
		}
		break;

	case WM_KEYUP:
		if (bct->stateId != PBS_DISABLED && wParam == VK_SPACE && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
			if (bct->bIsPushBtn) {
				if (bct->bIsPushed) {
					bct->bIsPushed = 0;
					bct->stateId = PBS_NORMAL;
				}
				else {
					bct->bIsPushed = 1;
					bct->stateId = PBS_PRESSED;
				}
				InvalidateRect(bct->hwnd, nullptr, TRUE);
			}
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM)hwnd);
			return 0;
		}
		break;

	case WM_SYSKEYUP:
		if (bct->stateId != PBS_DISABLED && !bct->bSendOnDown && bct->cHot && bct->cHot == tolower((int)wParam)) {
			if (bct->bIsPushBtn) {
				if (bct->bIsPushed) {
					bct->bIsPushed = false;
					bct->stateId = PBS_NORMAL;
				}
				else {
					bct->bIsPushed = true;
					bct->stateId = PBS_PRESSED;
				}
				InvalidateRect(bct->hwnd, nullptr, TRUE);
			}
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM)hwnd);
			return 0;
		}
		break;

	case WM_THEMECHANGED:
		// themed changed, reload theme object
		if (bct->bIsThemed)
			LoadTheme(bct);
		InvalidateRect(bct->hwnd, nullptr, TRUE); // repaint it
		break;

	case WM_SETFONT: // remember the font so we can use it later
		bct->hFont = (HFONT)wParam; // maybe we should redraw?
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdcPaint = BeginPaint(hwnd, &ps);
			if (hdcPaint) {
				bct->fnPainter(bct, hdcPaint);
				EndPaint(hwnd, &ps);
			}
		}
		break;

	case BM_SETIMAGE:
		{
			HGDIOBJ hnd = nullptr;
			if (bct->hIcon) hnd = bct->hIcon;
			else if (bct->hBitmap) hnd = bct->hBitmap;

			if (wParam == IMAGE_ICON) {
				bct->hIcon = (HICON)lParam;
				bct->hBitmap = nullptr;
				InvalidateRect(bct->hwnd, nullptr, TRUE);
			}
			else if (wParam == IMAGE_BITMAP) {
				bct->hBitmap = (HBITMAP)lParam;
				bct->hIcon = nullptr;
				InvalidateRect(bct->hwnd, nullptr, TRUE);
			}
			return (LRESULT)hnd;
		}

	case BM_GETIMAGE:
		if (bct->hIcon) return (LRESULT)bct->hIcon;
		else if (bct->hBitmap) return (LRESULT)bct->hBitmap;
		else return 0;

	case BM_SETCHECK:
		if (!bct->bIsPushBtn) break;
		if (wParam == BST_CHECKED) {
			bct->bIsPushed = 1;
			bct->stateId = PBS_PRESSED;
		}
		else if (wParam == BST_UNCHECKED) {
			bct->bIsPushed = 0;
			bct->stateId = PBS_NORMAL;
		}
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case BM_GETCHECK:
		if (bct->bIsPushBtn)
			return bct->bIsPushed ? BST_CHECKED : BST_UNCHECKED;
		return 0;

	case BUTTONSETARROW: // turn arrow on/off
		if (wParam) {
			if (!bct->arrow)
				bct->arrow = Skin_LoadIcon(SKINICON_OTHER_DOWNARROW);
		}
		else {
			if (bct->arrow) {
				IcoLib_ReleaseIcon(bct->arrow);
				bct->arrow = nullptr;
			}
		}
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case BUTTONSETDEFAULT:
		bct->bIsDefault = (wParam != 0);
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case BUTTONSETASPUSHBTN:
		bct->bIsPushBtn = (wParam != 0);
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case BUTTONSETASFLATBTN:
		bct->bIsFlat = (wParam != 0);
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case BUTTONSETASTHEMEDBTN:
		if ((bct->bIsThemed = (wParam != 0)) != 0)
			bct->bIsSkinned = false;
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case BUTTONADDTOOLTIP:
		if (wParam) {
			if (!bct->hwndToolTips) {
				TTooltips tt;
				tt.ThreadId = GetCurrentThreadId();

				mir_cslock lck(csTips);
				int idx = lToolTips.getIndex(&tt);
				if (idx != -1)
					bct->hwndToolTips = lToolTips[idx]->hwnd;
				else {
					TTooltips *ptt = (TTooltips*)mir_alloc(sizeof(TTooltips));
					ptt->ThreadId = tt.ThreadId;
					ptt->hwnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, L"", TTS_ALWAYSTIP, 0, 0, 0, 0, nullptr, nullptr, g_plugin.getInst(), nullptr);
					lToolTips.insert(ptt);
					bct->hwndToolTips = ptt->hwnd;
				}
			}
			TOOLINFO ti = {};
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = bct->hwnd;
			ti.uId = (UINT_PTR)bct->hwnd;
			if (SendMessage(bct->hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
				SendMessage(bct->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);

			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			ti.uId = (UINT_PTR)bct->hwnd;
			if (lParam & BATF_UNICODE)
				ti.lpszText = mir_wstrdup(TranslateW((wchar_t*)wParam));
			else
				ti.lpszText = Langpack_PcharToTchar((char*)wParam);
			SendMessage(bct->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)&ti);
			SendMessage(bct->hwndToolTips, TTM_SETMAXTIPWIDTH, 0, 300);
			mir_free(ti.lpszText);
		}
		break;

	case BUTTONSETCUSTOMPAINT:
		if (wParam > sizeof(MButtonCtrl)) {
			bct = (MButtonCtrl*)mir_realloc(bct, wParam);
			memset(bct+1, 0, wParam - sizeof(MButtonCtrl));
			SetWindowLongPtr(hwnd, 0, (LONG_PTR)bct);
		}
		if (lParam)
			bct->fnPainter = pfnPainterFunc(lParam);
		break;

	case BUTTONSETSENDONDOWN:
		bct->bSendOnDown = (wParam != 0);
		break;

	case WM_SETFOCUS: // set keybord focus and redraw
		bct->focus = 1;
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case WM_KILLFOCUS: // kill focus and redraw
		bct->focus = 0;
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case WM_WINDOWPOSCHANGED:
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case WM_ENABLE: // windows tells us to enable/disable
		bct->stateId = wParam ? PBS_NORMAL : PBS_DISABLED;
		InvalidateRect(bct->hwnd, nullptr, TRUE);
		break;

	case WM_MOUSELEAVE: // faked by the WM_TIMER
		if (bct->stateId != PBS_DISABLED) { // don't change states if disabled
			bct->stateId = PBS_NORMAL;
			InvalidateRect(bct->hwnd, nullptr, TRUE);
		}
		break;

	case WM_LBUTTONDOWN:
		if (bct->stateId != PBS_DISABLED) { // don't change states if disabled
			bct->stateId = PBS_PRESSED;
			InvalidateRect(bct->hwnd, nullptr, TRUE);
			if (bct->bSendOnDown) {
				SendMessage( GetParent(hwnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM)hwnd);
				bct->stateId = PBS_NORMAL;
				InvalidateRect(bct->hwnd, nullptr, TRUE);
			}
		}
		break;

	case WM_LBUTTONUP:
		{
			int showClick = 0;
			if (bct->bIsPushBtn)
				bct->bIsPushed = !bct->bIsPushed;

			if (bct->stateId != PBS_DISABLED) { // don't change states if disabled
				if (bct->stateId == PBS_PRESSED)
					showClick = 1;
				bct->stateId = PBS_HOT;
				InvalidateRect(bct->hwnd, nullptr, TRUE);
			}
			if (showClick && !bct->bSendOnDown) // Tell your daddy you got clicked.
				SendMessage( GetParent(hwnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwnd), BN_CLICKED), (LPARAM)hwnd);
		}
		break;

	case WM_MOUSEMOVE:
		if (bct->stateId == PBS_NORMAL) {
			bct->stateId = PBS_HOT;
			InvalidateRect(bct->hwnd, nullptr, TRUE);
		}
		// Call timer, used to start cheesy TrackMouseEvent faker
		SetTimer(hwnd, BUTTON_POLLID, BUTTON_POLLDELAY, nullptr);
		break;

	case WM_TIMER: // use a timer to check if they have did a mouseout
		if (wParam == BUTTON_POLLID) {
			RECT rc;
			GetWindowRect(hwnd, &rc);

			POINT pt;
			GetCursorPos(&pt);
			if (!PtInRect(&rc, pt)) { // mouse must be gone, trigger mouse leave
				PostMessage(hwnd, WM_MOUSELEAVE, 0, 0L);
				KillTimer(hwnd, BUTTON_POLLID);
		}	}
		break;

	case WM_ERASEBKGND:
		return 1;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
// Module load

static INT_PTR GetButtonProc(WPARAM, LPARAM)
{
	return (INT_PTR)MButtonWndProc;
}

int LoadButtonModule(void)
{
	if (bModuleInitialized)
		return 0;

	bModuleInitialized = true;

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = MIRANDABUTTONCLASS;
	wc.lpfnWndProc = MButtonWndProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(MButtonCtrl*);
	wc.hbrBackground = nullptr;
	wc.style = CS_GLOBALCLASS;
	RegisterClassEx(&wc);

	CreateServiceFunction("Button/GetWindowProc", GetButtonProc);
	return 0;
}
