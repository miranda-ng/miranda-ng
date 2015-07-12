/*
* astyle --force-indent=tab=4 --brackets=linux --indent-switches
*		  --pad=oper --one-line=keep-blocks  --unpad=paren
*
* Miranda IM: the free IM client for Microsoft* Windows*
*
* Copyright 2000-2010 Miranda ICQ/IM project,
* all portions of this codebase are copyrighted to the people
* listed in contributors.txt.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* you should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* part of clist_ng plugin for Miranda.
*
* (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
*
* $Id: statusbar.cpp 134 2010-10-01 10:23:10Z silvercircle $
*
* clist_ng status bar subclassing
*/

#include <commonheaders.h>

static POINT 	ptMouse = {0};
static RECT 	rcMouse = {0};
static int 		timer_set = 0, tooltip_active = 0;
extern HANDLE 	hStatusBarShowToolTipEvent, hStatusBarHideToolTipEvent;

#define TIMERID_HOVER 1000

LRESULT CALLBACK NewStatusBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_SETCURSOR:
	{
		POINT pt;

		GetCursorPos(&pt);
		SendMessage(GetParent(hwnd),msg,wParam,lParam);
		if (pt.x == ptMouse.x && pt.y == ptMouse.y)
			return 1;//return(TestCursorOnBorders());

		ptMouse = pt;
		if (tooltip_active){
			KillTimer(hwnd, TIMERID_HOVER);
			if(!NotifyEventHooks(hStatusBarHideToolTipEvent, 0, 0))
				CallService("mToolTip/HideTip", 0, 0);
			tooltip_active = FALSE;
			}
		KillTimer(hwnd, TIMERID_HOVER);
		SetTimer(hwnd, TIMERID_HOVER, 750, 0);
		break;
	}
	case WM_NCHITTEST:
	{
		LRESULT lr = SendMessage(GetParent(hwnd), WM_NCHITTEST, wParam, lParam);
		if(lr == HTLEFT || lr == HTRIGHT || lr == HTBOTTOM || lr == HTTOP || lr == HTTOPLEFT || lr == HTTOPRIGHT
			|| lr == HTBOTTOMLEFT || lr == HTBOTTOMRIGHT)
			return HTTRANSPARENT;
		break;
	}
	case WM_ERASEBKGND:
		return(1);

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		KillTimer(hwnd, TIMERID_HOVER);
		if(!NotifyEventHooks(hStatusBarHideToolTipEvent, 0, 0))
			CallService("mToolTip/HideTip", 0, 0);
		tooltip_active = FALSE;
		break;

	case WM_PAINT: {
		if(cfg::shutDown)
			return 0;

		PAINTSTRUCT 	ps;
		HDC hdc = 		BeginPaint(hwnd, &ps);
		HDC 			hdcMem;
		HANDLE 			hbp = 0;
		RECT 			rcClient;
		TStatusItem*	item = NULL;
		DRAWITEMSTRUCT 	dis = {0};
		int 			nParts = 0;
		int 			i;
		HFONT 			hOldFont = 0;
		HANDLE 			hTheme;
		BYTE 			windowStyle = cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_DEFAULT);
		LONG 			b_offset = (windowStyle == SETTING_WINDOWSTYLE_NOBORDER ? 2 : (windowStyle == SETTING_WINDOWSTYLE_THINBORDER ? 1 : 0));

		GetClientRect(hwnd, &rcClient);
		INIT_PAINT(hdc, rcClient, hdcMem);
		SetBkMode(hdcMem, TRANSPARENT);
		hOldFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, GetStockObject(DEFAULT_GUI_FONT)));
		Gfx::drawBGFromSurface(hwnd, rcClient, hdcMem);
		item = &Skin::statusItems[ID_EXTBKSTATUSBAR];
		if(!item->IGNORED) {
			RECT 		rc = rcClient;
			RGBQUAD*	rgb = 0;
			int			iWidth;

			Api::pfnGetBufferedPaintBits(hbp, &rgb, &iWidth);
			AGGPaintHelper* ph = new AGGPaintHelper(hdcMem);
			ph->aggctx->attach(rgb, iWidth, rc.bottom);
			ph->current_shape = 0;

			rc.left += item->MARGIN_LEFT;
			rc.right -= item->MARGIN_RIGHT;
			if(!Skin::metrics.fHaveFrame) {
				rc.left += Skin::metrics.cFakeLeftBorder;
				rc.right -= Skin::metrics.cFakeRightBorder;
			}
			rc.top += item->MARGIN_TOP;
			rc.bottom -= item->MARGIN_BOTTOM;
			Gfx::renderSkinItem(ph, item, &rc);
			Gfx::setTextColor(item->TEXTCOLOR);
			delete ph;
		}else{
			Gfx::setTextColor(GetSysColor(COLOR_BTNTEXT));
		}
		dis.hwndItem = hwnd;
		dis.hDC = hdcMem;
		dis.CtlType = 0;
		nParts = SendMessage(hwnd, SB_GETPARTS, 0, 0);

		hTheme = Api::pfnOpenThemeData(hwnd, L"BUTTON");
		for(i = 0; i < nParts; i++) {
			SendMessage(hwnd, SB_GETRECT, i, (LPARAM)&dis.rcItem);
			OffsetRect(&dis.rcItem, 0, -b_offset);
			dis.itemData = SendMessage(hwnd, SB_GETTEXTA, i, 0);
			dis.CtlID = (UINT)hTheme;
			SendMessage(pcli->hwndContactList, WM_DRAWITEM, 0, (LPARAM)&dis);
		}
		Api::pfnCloseThemeData(hTheme);
		if(hOldFont)
			SelectObject(hdcMem, hOldFont);
		FINALIZE_PAINT(hbp, &rcClient, 0);
		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_SIZE:
		return(DefWindowProc(hwnd, msg, wParam, lParam));

	case WM_TIMER:
		if(wParam == TIMERID_HOVER) {
			POINT pt;
			KillTimer(hwnd, TIMERID_HOVER);

			GetCursorPos(&pt);
			if (pt.x == ptMouse.x && pt.y == ptMouse.y) {
				int i,nParts;
				RECT rc;

				ScreenToClient(hwnd, &pt);
				nParts = SendMessage(hwnd, SB_GETPARTS, 0, 0);
				for(i = 0; i < nParts; i++) {
					SendMessage(hwnd, SB_GETRECT, i, (LPARAM)&rc);
					if(PtInRect(&rc,pt)) {
						ProtocolData *PD;
						PD = (ProtocolData *)SendMessageA(hwnd, SB_GETTEXTA, i, 0);

						if(PD) {
							if(NotifyEventHooks(hStatusBarShowToolTipEvent, (WPARAM)PD->RealName, 0) > 0) // a plugin handled this event
								tooltip_active = TRUE;
							else if(cfg::getDword("mToolTip", "ShowStatusTip", 0)) {
								CLCINFOTIP ti = {0};
								BYTE isLocked = 0;
								wchar_t szTipText[256], *szStatus = NULL;
								WORD wStatus;

								ti.cbSize = sizeof(ti);
								ti.isTreeFocused = GetFocus() == pcli->hwndContactList ? 1 : 0;
								wStatus = (WORD)CallProtoService(PD->RealName, PS_GETSTATUS, 0, 0);
								isLocked = cfg::getByte(PD->RealName, "LockMainStatus", 0);
								szStatus = pcli->pfnGetStatusModeDescription(wStatus, 0);
								mir_snwprintf(szTipText, 256, L"<b>%s</b>: %s%s", PD->RealName, szStatus, isLocked ? L"  (LOCKED)" : L"");
								CallService("mToolTip/ShowTip", (WPARAM)szTipText, (LPARAM)&ti);
							}
						}
						break;
					}
				}
			}
		}
		break;
	}

	return CallWindowProc(CLUI::OldStatusBarProc, hwnd, msg, wParam, lParam);
}
