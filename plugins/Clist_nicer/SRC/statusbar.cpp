/*
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
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

#include <commonheaders.h>
#include "../CLUIFrames/cluiframes.h"

WNDPROC OldStatusBarProc = 0;
    
static POINT ptMouse = {0};
static RECT rcMouse = {0};
static int timer_set = 0, tooltip_active = 0;
extern HANDLE hStatusBarShowToolTipEvent, hStatusBarHideToolTipEvent;
extern StatusItems_t *StatusItems;
extern HBRUSH g_CLUISkinnedBkColor;

extern HANDLE   (WINAPI *MyOpenThemeData)(HWND, LPCWSTR);
extern HRESULT  (WINAPI *MyCloseThemeData)(HANDLE);
extern HRESULT  (WINAPI *MyDrawThemeBackground)(HANDLE, HDC, int, int, const RECT *, const RECT *);

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
		if(cfg::dat.bSkinnedStatusBar)
			return 1;
		return CallWindowProc(OldStatusBarProc, hwnd, msg, wParam, lParam);

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		KillTimer(hwnd, TIMERID_HOVER);				
		if(!NotifyEventHooks(hStatusBarHideToolTipEvent, 0, 0))
			CallService("mToolTip/HideTip", 0, 0);
		tooltip_active = FALSE;		
		break;

	case WM_PAINT:
		if(cfg::shutDown)
			return 0;

		if(cfg::dat.bSkinnedStatusBar) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hbmMem, hbmOld;
			RECT rcClient, rcWindow;
			StatusItems_t *item = NULL;
			DRAWITEMSTRUCT dis = {0};
			int nParts = 0;
			int i;
			HFONT hOldFont = 0;
			POINT pt;
			BYTE windowStyle = cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_DEFAULT);
			LONG b_offset = cfg::dat.bClipBorder + (windowStyle == SETTING_WINDOWSTYLE_NOBORDER ? 2 : (windowStyle == SETTING_WINDOWSTYLE_THINBORDER ? 1 : 0));

			GetClientRect(hwnd, &rcClient);
			GetWindowRect(hwnd, &rcWindow);
			pt.x = rcWindow.left;
			pt.y = rcWindow.top;
			ScreenToClient(pcli->hwndContactList, &pt);

			hbmMem = CreateCompatibleBitmap(hdc, rcClient.right, rcClient.bottom);
			hbmOld = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hbmMem));
			SetBkMode(hdcMem, TRANSPARENT);
			hOldFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, GetStockObject(DEFAULT_GUI_FONT)));
			BitBlt(hdcMem, 0, 0, rcClient.right, rcClient.bottom, cfg::dat.hdcBg, pt.x, pt.y, SRCCOPY);
			item = &StatusItems[ID_EXTBKSTATUSBAR - ID_STATUS_OFFLINE];
			if(!item->IGNORED) {
				RECT rc = rcClient;
				rc.left += item->MARGIN_LEFT;
				rc.right -= item->MARGIN_RIGHT;
				rc.top += item->MARGIN_TOP;
				rc.bottom -= item->MARGIN_BOTTOM;
				DrawAlpha(hdcMem, &rc, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT,
					item->CORNER, item->BORDERSTYLE, item->imageItem);
				SetTextColor(hdcMem, item->TEXTCOLOR);
			}else{
				SetTextColor(hdcMem, GetSysColor(COLOR_BTNTEXT));
			}
			dis.hwndItem = hwnd;
			dis.hDC = hdcMem;
			dis.CtlType = 0;
			nParts = SendMessage(hwnd, SB_GETPARTS, 0, 0);
			for(i = 0; i < nParts; i++) {
				SendMessage(hwnd, SB_GETRECT, i, (LPARAM)&dis.rcItem);
				OffsetRect(&dis.rcItem, 0, -b_offset);
				dis.itemData = SendMessage(hwnd, SB_GETTEXTA, i, 0);
				SendMessage(pcli->hwndContactList, WM_DRAWITEM, 0, (LPARAM)&dis);
			}
			BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, hdcMem, 0, 0, SRCCOPY);
			if(hOldFont)
				SelectObject(hdcMem, hOldFont);
			SelectObject(hdcMem, hbmOld);
			DeleteObject(hbmMem);
			DeleteDC(hdcMem);
			EndPaint(hwnd, &ps);
			return 0;
		}
		break;

	case WM_DESTROY:
		break;

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
								char szTipText[256], *szStatus = NULL;
								WORD wStatus;

								ti.cbSize = sizeof(ti);
								ti.isTreeFocused = GetFocus() == pcli->hwndContactList ? 1 : 0;
								wStatus = (WORD)CallProtoService(PD->RealName, PS_GETSTATUS, 0, 0);
								isLocked = cfg::getByte(PD->RealName, "LockMainStatus", 0);
								szStatus = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)wStatus, 0);
								mir_snprintf(szTipText, 256, "<b>%s</b>: %s%s", PD->RealName, szStatus, isLocked ? "  (LOCKED)" : "");
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

	return CallWindowProc(OldStatusBarProc, hwnd, msg, wParam, lParam);
}
