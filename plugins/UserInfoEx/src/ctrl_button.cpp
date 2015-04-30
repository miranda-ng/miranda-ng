/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLID			 100
#define BUTTON_POLLDELAY		50

typedef struct TMBCtrl{
	HWND		hwnd;
	HANDLE		hThemeButton;
	HANDLE		hThemeToolbar;

	HICON		hIcon;
	HICON		arrow;			// uses down arrow
	HBITMAP		hBitmap;
	HFONT		hFont;			// font

	DWORD		dwStyle;	
	BYTE		bFocus;	
	
	int			stateId;		// button state
	int			defbutton;		// default button
	int			pbState;
	TCHAR		cHot;
} BTNCTRL, *LPBTNCTRL;

// External theme methods and properties
static mir_cs csTips;
static HWND hwndToolTips = NULL;

/**
 * name:	DestroyTheme
 * desc:	destroys theme data for buttons
 * param:	ctl - BTNCTRL structure with the information about the theme to close
 * return:	nothing
 **/
static void __fastcall DestroyTheme(BTNCTRL *ctl) {
	if (ctl->hThemeButton) {
		CloseThemeData(ctl->hThemeButton);
		ctl->hThemeButton = NULL;
	}
	if (ctl->hThemeToolbar) {
		CloseThemeData(ctl->hThemeToolbar);
		ctl->hThemeToolbar = NULL;
	}
}

/**
 * name:	LoadTheme
 * desc:	load theme data for buttons if supported by os
 * param:	ctl - BTNCTRL structure with the information about the theme to load
 * return:	nothing
 **/
static void __fastcall LoadTheme(BTNCTRL *ctl) {
	DestroyTheme(ctl);
	ctl->hThemeButton = OpenThemeData(ctl->hwnd,L"BUTTON");
	ctl->hThemeToolbar = OpenThemeData(ctl->hwnd,L"TOOLBAR");
}

/**
 * name:	TBStateConvert2Flat
 * desc:	convert button stateIDs
 * param:	state - state id for the normal theme button
 * return:	stateID for the flat theme button
 **/
static int __fastcall TBStateConvert2Flat(int state) {
	switch (state) {
		case PBS_NORMAL:		return TS_NORMAL;
		case PBS_HOT:			 return TS_HOT;
		case PBS_PRESSED:	 return TS_PRESSED;
		case PBS_DISABLED:	return TS_DISABLED;
		case PBS_DEFAULTED: return TS_NORMAL;
	}
	return TS_NORMAL;
}

/**
 * name:	PaintIcon
 * desc:	Draws the Icon of the button
 * param:	ctl			- BTNCTRL structure for the button
 *			hdcMem		- device context to draw to
 *			ccText		- character count of the text of the button
 *			rcClient	- rectangle of the whole button
 *			rcText		- rectangle of the text to draw later on
 * return:	nothing
 **/
static void __fastcall PaintIcon(BTNCTRL *ctl, HDC hdcMem, LPWORD ccText, LPRECT rcClient, LPRECT rcText)
{
	RECT rcImage;

	// draw icon on the left of the button
	if (ctl->hIcon) {
		rcImage.right = GetSystemMetrics(SM_CXSMICON);
		rcImage.bottom = GetSystemMetrics(SM_CYSMICON);
		rcImage.left = (rcClient->right - rcClient->left) / 2 - ((rcImage.right + rcText->right + (*ccText > 0 ? 4 : 0) + (ctl->arrow ? rcImage.right : 0)) / 2);
		rcImage.top = (rcClient->bottom - rcClient->top - rcImage.bottom) / 2;
		rcImage.right += rcImage.left;
		rcImage.bottom += rcImage.top;
		
		OffsetRect(rcText, rcImage.right + 4, 0);
		if (ctl->stateId == PBS_PRESSED)	OffsetRect(&rcImage, 1, 1);

		DrawState(hdcMem, NULL, NULL, (LPARAM)ctl->hIcon, 0, 
			rcImage.left, rcImage.top, 
			rcImage.right - rcImage.left, rcImage.bottom - rcImage.top,
			IsWindowEnabled(ctl->hwnd) ? DST_ICON | DSS_NORMAL : DST_ICON | DSS_DISABLED);
	}

	// draw arrow on the right of the button
	if (ctl->arrow) {
		rcImage.right = GetSystemMetrics(SM_CXSMICON);
		rcImage.left = (*ccText > 0 || ctl->hIcon) 
					 ? rcClient->right - GetSystemMetrics(SM_CXSMICON) 
					 : (rcClient->right - rcClient->left - rcImage.right) / 2;
		rcImage.right += rcImage.left;
		rcImage.bottom = GetSystemMetrics(SM_CYSMICON);
		rcImage.top = (rcClient->bottom - rcClient->top - rcImage.bottom) / 2;
		if (ctl->stateId == PBS_PRESSED)	OffsetRect(&rcImage, 1, 1);

		DrawState(hdcMem, NULL, NULL, (LPARAM)ctl->arrow, 0, 
			rcImage.left, rcImage.top, 
			rcImage.right - rcImage.left, rcImage.bottom - rcImage.top,
			IsWindowEnabled(ctl->hwnd) ? DST_ICON | DSS_NORMAL : DST_ICON | DSS_DISABLED);
	}
}

/**
 * name:	PaintThemeButton
 * desc:	Draws the themed button
 * param:	ctl			- BTNCTRL structure for the button
 *			hdcMem		- device context to draw to
 *			rcClient	- rectangle of the whole button
 * return:	nothing
 **/
static void __fastcall PaintThemeButton(BTNCTRL *ctl, HDC hdcMem, LPRECT rcClient)
{
	RECT rcText = { 0, 0, 0, 0 };
	WCHAR wszText[MAX_PATH] = { 0 };
	WORD ccText;

	// Draw the flat button
	if ((ctl->dwStyle & MBS_FLAT) && ctl->hThemeToolbar) {
		int state = IsWindowEnabled(ctl->hwnd)
				? (ctl->stateId == PBS_NORMAL && ctl->defbutton 
					? PBS_DEFAULTED
					: ctl->stateId)
				: PBS_DISABLED;
		if (IsThemeBackgroundPartiallyTransparent(ctl->hThemeToolbar, TP_BUTTON, TBStateConvert2Flat(state))) {
			if (SUCCEEDED(DrawThemeParentBackground(ctl->hwnd, hdcMem, rcClient)))
				DrawThemeParentBackground(GetParent(ctl->hwnd), hdcMem, rcClient);
		}
		DrawThemeBackground(ctl->hThemeToolbar, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), rcClient, rcClient);
	}
	else {
		// draw themed button background
		if (ctl->hThemeButton) {
			int state = IsWindowEnabled(ctl->hwnd)
				? (ctl->stateId == PBS_NORMAL && ctl->defbutton 
					? PBS_DEFAULTED
					: ctl->stateId)
				: PBS_DISABLED;
			if (IsThemeBackgroundPartiallyTransparent(ctl->hThemeButton, BP_PUSHBUTTON, state)) {
				if (SUCCEEDED(DrawThemeParentBackground(ctl->hwnd, hdcMem, rcClient)))
					DrawThemeParentBackground(GetParent(ctl->hwnd), hdcMem, rcClient);
			}
			DrawThemeBackground(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, state, rcClient, rcClient);
		}
	}
	
	// calculate text rect
	{
		RECT	sizeText;
		HFONT	hOldFont;

		ccText = GetWindowTextW(ctl->hwnd, wszText, SIZEOF(wszText));

		if (ccText > 0) {
			hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);
			
			GetThemeTextExtent(
				ctl->hThemeButton,
				hdcMem,
				BP_PUSHBUTTON,
				IsWindowEnabled(ctl->hwnd) ? ctl->stateId : PBS_DISABLED,
				wszText,
				ccText,
				DST_PREFIXTEXT,
				NULL,
				&sizeText);
			
			if (ctl->cHot) {
				RECT rcHot;
				
				GetThemeTextExtent(ctl->hThemeButton,
					hdcMem,
					BP_PUSHBUTTON,
					IsWindowEnabled(ctl->hwnd) ? ctl->stateId : PBS_DISABLED,
					L"&",
					1,
					DST_PREFIXTEXT,
					NULL,
					&rcHot);
				
				sizeText.right -= (rcHot.right - rcHot.left);
			}
			SelectObject(hdcMem, hOldFont);

			rcText.left = (ctl->hIcon) ? 0 : (rcClient->right - rcClient->left - (sizeText.right - sizeText.left)) / 2;
			rcText.top = (rcClient->bottom - rcClient->top - (sizeText.bottom - sizeText.top)) / 2;
			rcText.right = rcText.left + (sizeText.right - sizeText.left);
			rcText.bottom = rcText.top + (sizeText.bottom - sizeText.top);
			if (ctl->stateId == PBS_PRESSED) {
				OffsetRect(&rcText, 1, 1);
			}
		}
	}
	PaintIcon(ctl, hdcMem, &ccText, rcClient, &rcText);
	// draw text
	if (ccText > 0 && ctl->hThemeButton) { 
		HFONT hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);
		DrawThemeText(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, IsWindowEnabled(ctl->hwnd) ? ctl->stateId : PBS_DISABLED, wszText, ccText, DST_PREFIXTEXT, 0, &rcText);
		SelectObject(hdcMem, hOldFont);
	}
}

/**
 * name:	PaintThemeButton
 * desc:	Draws the none themed button
 * param:	ctl			- BTNCTRL structure for the button
 *			hdcMem		- device context to draw to
 *			rcClient	- rectangle of the whole button
 * return:	nothing
 **/
static void __fastcall PaintButton(BTNCTRL *ctl, HDC hdcMem, LPRECT rcClient)
{
	RECT rcText = { 0, 0, 0, 0 };
	TCHAR szText[MAX_PATH] = { 0 };
	WORD ccText;

	// Draw the flat button
	if (ctl->dwStyle & MBS_FLAT) {
		HBRUSH hbr = NULL;
		
		if (ctl->stateId == PBS_PRESSED || ctl->stateId == PBS_HOT)
			hbr = GetSysColorBrush(COLOR_3DLIGHT);
		else {
			HDC dc;
			HWND hwndParent;

			hwndParent = GetParent(ctl->hwnd);
			if (dc = GetDC(hwndParent)) {
				hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwndParent);
				ReleaseDC(hwndParent, dc);
			}
		}
		if (hbr) {
			FillRect(hdcMem, rcClient, hbr);
			DeleteObject(hbr);
		}
		if (ctl->stateId == PBS_HOT || ctl->bFocus) {
			if (ctl->pbState) DrawEdge(hdcMem, rcClient, EDGE_ETCHED, BF_RECT|BF_SOFT);
			else DrawEdge(hdcMem, rcClient, BDR_RAISEDOUTER, BF_RECT|BF_SOFT|BF_FLAT);
		}
		else
		if (ctl->stateId == PBS_PRESSED)
			DrawEdge(hdcMem, rcClient, BDR_SUNKENOUTER, BF_RECT|BF_SOFT);
	}
	else {
		UINT uState = DFCS_BUTTONPUSH|((ctl->stateId == PBS_HOT) ? DFCS_HOT : 0)|((ctl->stateId == PBS_PRESSED) ? DFCS_PUSHED : 0);
		if (ctl->defbutton&&ctl->stateId==PBS_NORMAL) uState |= DLGC_DEFPUSHBUTTON;
		DrawFrameControl(hdcMem, rcClient, DFC_BUTTON, uState);
		// Draw focus rectangle if button has focus
		if (ctl->bFocus) {
			RECT focusRect = *rcClient;
			InflateRect(&focusRect, -3, -3);
			DrawFocusRect(hdcMem, &focusRect);
		}
	}
	// calculate text rect
	{
		SIZE	sizeText;
		HFONT	hOldFont;

		ccText = GetWindowText(ctl->hwnd, szText, SIZEOF(szText));

		if (ccText > 0) {
			hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);
			GetTextExtentPoint32(hdcMem, szText, ccText, &sizeText);
			if (ctl->cHot) {
				SIZE sizeHot;
				
				GetTextExtentPoint32A(hdcMem, "&", 1, &sizeHot);
				sizeText.cx -= sizeHot.cx;
			}
			SelectObject(hdcMem, hOldFont);

			rcText.left = (ctl->hIcon) ? 0 : (rcClient->right - rcClient->left - sizeText.cx) / 2;
			rcText.top = (rcClient->bottom - rcClient->top - sizeText.cy) / 2;
			rcText.right = rcText.left + sizeText.cx;
			rcText.bottom = rcText.top + sizeText.cy;
			if (ctl->stateId == PBS_PRESSED)
				OffsetRect(&rcText, 1, 1);
		}
	}
	PaintIcon(ctl, hdcMem, &ccText, rcClient, &rcText);

	// draw text
	if (ccText > 0) { 
		HFONT hOldFont;

		hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);

		SetBkMode(hdcMem, TRANSPARENT);
		SetTextColor(hdcMem, 
			IsWindowEnabled(ctl->hwnd) || !ctl->hThemeButton 
			? ctl->stateId == PBS_HOT
					? GetSysColor(COLOR_HOTLIGHT)
					: GetSysColor(COLOR_BTNTEXT) 
				: GetSysColor(COLOR_GRAYTEXT));

		DrawState(hdcMem, NULL, NULL, (LPARAM)szText, 0, 
			rcText.left, rcText.top, rcText.right - rcText.left, rcText.bottom - rcText.top,
			IsWindowEnabled(ctl->hwnd) || ctl->hThemeButton ? DST_PREFIXTEXT | DSS_NORMAL : DST_PREFIXTEXT | DSS_DISABLED);
		SelectObject(hdcMem, hOldFont);
	}
}

/**
 * name:	Button_WndProc
 * desc:	window procedure for the button class
 * param:	hwndBtn		- window handle to the button
 *			uMsg		- message to handle
 *			wParam		- message specific parameter
 *			lParam		- message specific parameter
 * return:	message specific
 **/
static LRESULT CALLBACK Button_WndProc(HWND hwndBtn, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LPBTNCTRL bct = (LPBTNCTRL)GetWindowLongPtr(hwndBtn, 0);
	
	switch (uMsg) {
	case WM_NCCREATE:
		{
			LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;

			cs->style |= BS_OWNERDRAW;
			if (!(bct = (LPBTNCTRL)mir_alloc(sizeof(BTNCTRL))))
				return FALSE;
			memset(bct, 0, sizeof(BTNCTRL));
			bct->hwnd = hwndBtn;
			bct->stateId = PBS_NORMAL;
			bct->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			bct->dwStyle = cs->style;
			if (cs->style & MBS_DOWNARROW)
				bct->arrow = Skin_GetIcon(ICO_BTN_DOWNARROW);
			LoadTheme(bct);
			SetWindowLongPtr(hwndBtn, 0, (LONG_PTR)bct);
			if (cs->lpszName) SetWindowText(hwndBtn, cs->lpszName);
			return TRUE;
		}
	case WM_DESTROY:
		if (bct) {
			mir_cslock lck(csTips);
			if (hwndToolTips) {
				TOOLINFO ti;

				memset(&ti, 0, sizeof(ti));
				ti.cbSize = sizeof(ti);
				ti.uFlags = TTF_IDISHWND;
				ti.hwnd = bct->hwnd;
				ti.uId = (UINT_PTR)bct->hwnd;
				if (SendMessage(hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti)) {
					SendMessage(hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);
				}
				if (SendMessage(hwndToolTips, TTM_GETTOOLCOUNT, 0, (LPARAM)&ti) == 0) {
					DestroyWindow(hwndToolTips);
					hwndToolTips = NULL;
				}
			}
			DestroyTheme(bct);
			mir_free(bct);
		}
		SetWindowLongPtr(hwndBtn, 0, 0);
		break;
	case WM_SETTEXT:
		bct->cHot = 0;
		if ((LPTSTR)lParam) {
			LPTSTR tmp = (LPTSTR)lParam;

			while (*tmp) {
				if (*tmp == '&' && *(tmp + 1)) {
					bct->cHot = _totlower(*(tmp + 1));
					break;
				}
				tmp++;
			}
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;
	case WM_SYSKEYUP:
		if (bct->stateId != PBS_DISABLED && bct->cHot && bct->cHot == _totlower((TCHAR)wParam)) {
			if (bct->dwStyle & MBS_PUSHBUTTON) {
				if (bct->pbState) bct->pbState = 0;
				else bct->pbState = 1;
				InvalidateRect(bct->hwnd, NULL, TRUE);
			}
			else
				SetFocus(hwndBtn);
			SendMessage(GetParent(hwndBtn), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndBtn), BN_CLICKED), (LPARAM)hwndBtn);
			return 0;
		}
		break;
	case WM_THEMECHANGED: 
		// themed changed, reload theme object
		LoadTheme(bct);
		InvalidateRect(bct->hwnd, NULL, TRUE); // repaint it
		break;
	case WM_SETFONT: // remember the font so we can use it later
		bct->hFont = (HFONT)wParam; // maybe we should redraw?
		break;
	case WM_NCPAINT:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdcPaint;
			HDC hdcMem;
			HBITMAP hbmMem;
			HDC hOld;
			RECT rcClient;
			
			if (hdcPaint = BeginPaint(hwndBtn, &ps)) {
				GetClientRect(bct->hwnd, &rcClient);
				hdcMem = CreateCompatibleDC(hdcPaint);
				hbmMem = CreateCompatibleBitmap(hdcPaint, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
				hOld = (HDC)SelectObject(hdcMem, hbmMem);

				// If its a push button, check to see if it should stay pressed
				if ((bct->dwStyle & MBS_PUSHBUTTON) && bct->pbState) bct->stateId = PBS_PRESSED;

				if ((bct->dwStyle & MBS_FLAT) && bct->hThemeToolbar || bct->hThemeButton)
					PaintThemeButton(bct, hdcMem, &rcClient);
				else
					PaintButton(bct, hdcMem, &rcClient);

				BitBlt(hdcPaint, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hOld);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);				
				EndPaint(hwndBtn, &ps);
			}
		}
		return 0;
	case BM_SETIMAGE:
		if (wParam == IMAGE_ICON) {
			bct->hIcon = (HICON)lParam;
			bct->hBitmap = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		else if (wParam == IMAGE_BITMAP) {
			bct->hIcon = NULL;
			bct->hBitmap = (HBITMAP)lParam;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		else if (wParam == NULL && lParam == NULL) {
			bct->hIcon = NULL;
			bct->hBitmap = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;
	case BM_SETCHECK:
		if (!(bct->dwStyle & MBS_PUSHBUTTON)) break;
		if (wParam == BST_CHECKED) {
			bct->pbState = 1;
			bct->stateId = PBS_PRESSED;
		}
		else if (wParam == BST_UNCHECKED) {
			bct->pbState = 0;
			bct->stateId = PBS_NORMAL;
		}
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;
	case BM_GETCHECK:
		if (bct->dwStyle & MBS_PUSHBUTTON) return bct->pbState ? BST_CHECKED : BST_UNCHECKED;
		return 0;
	case BUTTONSETDEFAULT:
		bct->defbutton = (wParam != 0);
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;
	case BUTTONADDTOOLTIP:
		if (wParam) {			
			mir_cslock lck(csTips);
			if (!hwndToolTips)
				hwndToolTips = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

			if (lParam == MBBF_UNICODE) {
				TOOLINFOW ti;

				memset(&ti, 0, sizeof(TOOLINFOW));
				ti.cbSize = sizeof(TOOLINFOW);
				ti.uFlags = TTF_IDISHWND;
				ti.hwnd = bct->hwnd;
				ti.uId = (UINT_PTR)bct->hwnd;
				if (SendMessage(hwndToolTips, TTM_GETTOOLINFOW, 0, (LPARAM)&ti)) {
					SendMessage(hwndToolTips, TTM_DELTOOLW, 0, (LPARAM)&ti);
				}
				ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
				ti.uId = (UINT_PTR)bct->hwnd;
				ti.lpszText=(LPWSTR)wParam;
				SendMessage(hwndToolTips, TTM_ADDTOOLW, 0, (LPARAM)&ti);
			}
			else {
				TOOLINFOA ti;

				memset(&ti, 0, sizeof(TOOLINFOA));
				ti.cbSize = sizeof(TOOLINFOA);
				ti.uFlags = TTF_IDISHWND;
				ti.hwnd = bct->hwnd;
				ti.uId = (UINT_PTR)bct->hwnd;
				if (SendMessage(hwndToolTips, TTM_GETTOOLINFOA, 0, (LPARAM)&ti)) {
					SendMessage(hwndToolTips, TTM_DELTOOLA, 0, (LPARAM)&ti);
				}
				ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
				ti.uId = (UINT_PTR)bct->hwnd;
				ti.lpszText=(LPSTR)wParam;
				SendMessage(hwndToolTips, TTM_ADDTOOLA, 0, (LPARAM)&ti);
			}
		}
		break;
	case BUTTONTRANSLATE:
		{
			TCHAR szButton[MAX_PATH];
			GetWindowText(bct->hwnd, szButton, SIZEOF(szButton));
			SetWindowText(bct->hwnd, TranslateTS(szButton));
		}
		break;
	case WM_SETFOCUS: // set keybord bFocus and redraw
		bct->bFocus = 1;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;
	case WM_KILLFOCUS: // kill bFocus and redraw
		bct->bFocus = 0;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;
	case WM_WINDOWPOSCHANGED:
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;
	case WM_ENABLE: // windows tells us to enable/disable
		bct->stateId = wParam ? PBS_NORMAL : PBS_DISABLED;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;
	case WM_MOUSELEAVE: // faked by the WM_TIMER
		if (bct->stateId != PBS_DISABLED) { // don't change states if disabled
			bct->stateId = PBS_NORMAL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;
	case WM_LBUTTONDOWN:
		if (bct->stateId != PBS_DISABLED) { // don't change states if disabled
			bct->stateId = PBS_PRESSED;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;
	case WM_LBUTTONUP:
		if (bct->stateId != PBS_DISABLED) { // don't change states if disabled
			BYTE bPressed = bct->stateId == PBS_PRESSED;

			if (bct->dwStyle & MBS_PUSHBUTTON) {
				if (bct->pbState) bct->pbState = 0;
				else bct->pbState = 1;
			}
			bct->stateId = PBS_HOT;

			// Tell your daddy you got clicked, if mouse is still over the button.
			if ((bct->dwStyle & MBS_PUSHBUTTON) || bPressed)
				SendMessage(GetParent(hwndBtn), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndBtn), BN_CLICKED), (LPARAM)hwndBtn);
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;
	case WM_MOUSEMOVE:
		if (bct->stateId == PBS_NORMAL) {
			bct->stateId = PBS_HOT;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		// Call timer, used to start cheesy TrackMouseEvent faker
		SetTimer(hwndBtn, BUTTON_POLLID, BUTTON_POLLDELAY, NULL);
		break;
	case WM_TIMER: // use a timer to check if they have did a mouseout
		if (wParam == BUTTON_POLLID) {
			RECT rc;
			POINT pt;

			GetWindowRect(hwndBtn, &rc);
			GetCursorPos(&pt);
			if (!PtInRect(&rc, pt)) { // mouse must be gone, trigger mouse leave
				PostMessage(hwndBtn, WM_MOUSELEAVE, 0, 0L);
				KillTimer(hwndBtn, BUTTON_POLLID);
			}
		}
		break;
	case WM_ERASEBKGND:
		return 1;
	}
	return DefWindowProc(hwndBtn, uMsg, wParam, lParam);
}

void CtrlButtonUnloadModule() 
{
	UnregisterClass(UINFOBUTTONCLASS, ghInst);
}

void CtrlButtonLoadModule()
{
	WNDCLASSEX wc;
	
	memset(&wc, 0, sizeof(wc));
	wc.cbSize				 = sizeof(wc);
	wc.lpszClassName	= UINFOBUTTONCLASS;
	wc.lpfnWndProc		= Button_WndProc;
	wc.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra		 = sizeof(LPBTNCTRL);
	wc.style					= CS_GLOBALCLASS;
	RegisterClassEx(&wc);
}

