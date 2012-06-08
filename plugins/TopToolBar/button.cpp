/*
Miranda IM
Copyright (C) 2002 Robert Rainwater

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
#include "common.h"

// TODO:
// - Support for bitmap buttons (simple call to DrawIconEx())

typedef struct {
	HWND    hwnd;
	int     stateId; // button state
	int     focus;   // has focus (1 or 0)
	HFONT   hFont;   // font
	HICON   arrow;   // uses down arrow
	int     defbutton; // default button
	HICON   hIcon;
	HBITMAP hBitmap;
	int     pushBtn;
	int     pbState;
	HANDLE  hThemeButton;
	HANDLE  hThemeToolbar;
	char	cHot;
	int     flatBtn;
} MButtonCtrl;


// External theme methods and properties
static HMODULE  themeAPIHandle = NULL; // handle to uxtheme.dll
static HANDLE   (WINAPI *MyOpenThemeData)(HWND,LPCWSTR);
static HRESULT  (WINAPI *MyCloseThemeData)(HANDLE);
static BOOL     (WINAPI *MyIsThemeBackgroundPartiallyTransparent)(HANDLE,int,int);
static HRESULT  (WINAPI *MyDrawThemeParentBackground)(HWND,HDC,RECT *);
static HRESULT  (WINAPI *MyDrawThemeBackground)(HANDLE,HDC,int,int,const RECT *,const RECT *);
static HRESULT  (WINAPI *MyDrawThemeText)(HANDLE,HDC,int,int,LPCWSTR,int,DWORD,DWORD,const RECT *);

static CRITICAL_SECTION csTips;
static HWND hwndToolTips = NULL;

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLID       100
#define BUTTON_POLLDELAY    50

#define MGPROC(x) GetProcAddress(themeAPIHandle,x)
static int ThemeSupport()
{
	if (IsWinVerXPPlus()) {
		if (!themeAPIHandle) {
			themeAPIHandle = GetModuleHandle("uxtheme");
			if (themeAPIHandle) {
				MyOpenThemeData = (HANDLE (WINAPI *)(HWND,LPCWSTR))MGPROC("OpenThemeData");
				MyCloseThemeData = (HRESULT (WINAPI *)(HANDLE))MGPROC("CloseThemeData");
				MyIsThemeBackgroundPartiallyTransparent = (BOOL (WINAPI *)(HANDLE,int,int))MGPROC("IsThemeBackgroundPartiallyTransparent");
				MyDrawThemeParentBackground = (HRESULT (WINAPI *)(HWND,HDC,RECT *))MGPROC("DrawThemeParentBackground");
				MyDrawThemeBackground = (HRESULT (WINAPI *)(HANDLE,HDC,int,int,const RECT *,const RECT *))MGPROC("DrawThemeBackground");
				MyDrawThemeText = (HRESULT (WINAPI *)(HANDLE,HDC,int,int,LPCWSTR,int,DWORD,DWORD,const RECT *))MGPROC("DrawThemeText");
			}
		}
		// Make sure all of these methods are valid (i would hope either all or none work)
		if (MyOpenThemeData && MyCloseThemeData && MyIsThemeBackgroundPartiallyTransparent &&
			 MyDrawThemeParentBackground && MyDrawThemeBackground && MyDrawThemeText)
			return 1;
	}
	return 0;
}

static void DestroyTheme(MButtonCtrl *ctl)
{
	if (ThemeSupport()) {
		if (ctl->hThemeButton) {
			MyCloseThemeData(ctl->hThemeButton);
			ctl->hThemeButton = NULL;
		}
		if (ctl->hThemeToolbar) {
			MyCloseThemeData(ctl->hThemeToolbar);
			ctl->hThemeToolbar = NULL;
		}
	}
}

static void LoadTheme(MButtonCtrl *ctl)
{
	if (ThemeSupport()) {
		DestroyTheme(ctl);
		ctl->hThemeButton = MyOpenThemeData(ctl->hwnd,L"BUTTON");
		ctl->hThemeToolbar = MyOpenThemeData(ctl->hwnd,L"TOOLBAR");
	}
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

static void PaintWorker(MButtonCtrl *ctl, HDC hdcPaint)
{
	if (hdcPaint) {
		RECT rcClient;
		GetClientRect(ctl->hwnd, &rcClient);

		HDC hdcMem = (HDC)CreateCompatibleDC(hdcPaint);
		HBITMAP hbmMem = (HBITMAP)CreateCompatibleBitmap(hdcPaint, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);
		HDC hOld = (HDC)SelectObject(hdcMem, hbmMem);

		// If its a push button, check to see if it should stay pressed
		if (ctl->pushBtn && ctl->pbState) ctl->stateId = PBS_PRESSED;

		// Draw the flat button
		if (ctl->flatBtn) {
			if (ctl->hThemeToolbar) {
				int state = IsWindowEnabled(ctl->hwnd)?(ctl->stateId==PBS_NORMAL&&ctl->defbutton?PBS_DEFAULTED:ctl->stateId):PBS_DISABLED;
                if (MyIsThemeBackgroundPartiallyTransparent(ctl->hThemeToolbar, TP_BUTTON, TBStateConvert2Flat(state))) {
					MyDrawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);
				}
				MyDrawThemeBackground(ctl->hThemeToolbar, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), &rcClient, &rcClient);
			}
			else {
				HBRUSH hbr;
				
				if (ctl->stateId==PBS_PRESSED||ctl->stateId==PBS_HOT)
					hbr = GetSysColorBrush(COLOR_3DLIGHT);
				else {
					HDC dc;
					HWND hwndParent;

					hwndParent = GetParent(ctl->hwnd);
					dc=GetDC(hwndParent);
					hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwndParent);
					ReleaseDC(hwndParent,dc);
				}
				if (hbr) {
					FillRect(hdcMem, &rcClient, hbr);
					DeleteObject(hbr);
				}
				if (ctl->stateId==PBS_HOT||ctl->focus) {
					if (ctl->pbState)
						DrawEdge(hdcMem,&rcClient, EDGE_ETCHED,BF_RECT|BF_SOFT);
					else DrawEdge(hdcMem,&rcClient, BDR_RAISEDOUTER,BF_RECT|BF_SOFT|BF_FLAT);
				}
				else if (ctl->stateId==PBS_PRESSED)
					DrawEdge(hdcMem, &rcClient, BDR_SUNKENOUTER,BF_RECT|BF_SOFT);
			}
		}
		else {
			// Draw background/border
			if (ctl->hThemeButton) {
				int state = IsWindowEnabled(ctl->hwnd)?(ctl->stateId==PBS_NORMAL&&ctl->defbutton?PBS_DEFAULTED:ctl->stateId):PBS_DISABLED;
				if (MyIsThemeBackgroundPartiallyTransparent(ctl->hThemeButton, BP_PUSHBUTTON, state)) {
					MyDrawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);
				}
				MyDrawThemeBackground(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, state, &rcClient, &rcClient);
			}
			else {
				UINT uState = DFCS_BUTTONPUSH|((ctl->stateId==PBS_HOT)?DFCS_HOT:0)|((ctl->stateId == PBS_PRESSED)?DFCS_PUSHED:0);
				if (ctl->defbutton&&ctl->stateId==PBS_NORMAL) uState |= DLGC_DEFPUSHBUTTON;
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
		if (ctl->hIcon) {
			int ix = (rcClient.right-rcClient.left)/2 - (GetSystemMetrics(SM_CXSMICON)/2);
			int iy = (rcClient.bottom-rcClient.top)/2 - (GetSystemMetrics(SM_CYSMICON)/2);
			if (ctl->stateId == PBS_PRESSED) {
				ix++;
				iy++;
			}

			HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON), IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 1, 0);
			ImageList_AddIcon(hImageList, ctl->hIcon);
			HICON hIconNew = ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
			DrawState(hdcMem,NULL,NULL,(LPARAM)hIconNew,0,ix,iy,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWindowEnabled(ctl->hwnd)?DST_ICON|DSS_NORMAL:DST_ICON|DSS_DISABLED);
			ImageList_RemoveAll(hImageList);
			ImageList_Destroy(hImageList);
			DestroyIcon(hIconNew);
		}
		else if (ctl->hBitmap) {
			BITMAP bminfo;
			int ix,iy;

			GetObject(ctl->hBitmap, sizeof(bminfo), &bminfo);
			ix = (rcClient.right-rcClient.left)/2 - (bminfo.bmWidth/2);
			iy = (rcClient.bottom-rcClient.top)/2 - (bminfo.bmHeight/2);
			if (ctl->stateId == PBS_PRESSED) {
				ix++;
				iy++;
			}
			DrawState(hdcMem,NULL,NULL,(LPARAM)ctl->hBitmap,0,ix,iy,bminfo.bmWidth,bminfo.bmHeight,IsWindowEnabled(ctl->hwnd)?DST_BITMAP:DST_BITMAP|DSS_DISABLED);
		}
		else if (GetWindowTextLength(ctl->hwnd)) {
			// Draw the text and optinally the arrow
			char szText[MAX_PATH];
			SIZE sz;
			RECT rcText;
			CopyRect(&rcText, &rcClient);

			GetWindowText(ctl->hwnd, szText, sizeof(szText));
			SetBkMode(hdcMem, TRANSPARENT);
			HFONT hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);
			// XP w/themes doesn't used the glossy disabled text.  Is it always using COLOR_GRAYTEXT?  Seems so.
			SetTextColor(hdcMem, IsWindowEnabled(ctl->hwnd)||!ctl->hThemeButton?GetSysColor(COLOR_BTNTEXT):GetSysColor(COLOR_GRAYTEXT));
			GetTextExtentPoint32(hdcMem, szText, lstrlen(szText), &sz);
			if (ctl->cHot) {
				SIZE szHot;
				
				GetTextExtentPoint32(hdcMem, "&", 1, &szHot);
				sz.cx -= szHot.cx;
			}
			if (ctl->arrow) {
				DrawState(hdcMem,NULL,NULL,(LPARAM)ctl->arrow,0,rcClient.right-rcClient.left-5-GetSystemMetrics(SM_CXSMICON)+(!ctl->hThemeButton&&ctl->stateId==PBS_PRESSED?1:0),(rcClient.bottom-rcClient.top)/2-GetSystemMetrics(SM_CYSMICON)/2+(!ctl->hThemeButton&&ctl->stateId==PBS_PRESSED?1:0),GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWindowEnabled(ctl->hwnd)?DST_ICON:DST_ICON|DSS_DISABLED);
			}
			SelectObject(hdcMem, ctl->hFont);
			DrawState(hdcMem,NULL,NULL,(LPARAM)szText,0,(rcText.right-rcText.left-sz.cx)/2+(!ctl->hThemeButton&&ctl->stateId==PBS_PRESSED?1:0),ctl->hThemeButton?(rcText.bottom-rcText.top-sz.cy)/2:(rcText.bottom-rcText.top-sz.cy)/2-(ctl->stateId==PBS_PRESSED?0:1),sz.cx,sz.cy,IsWindowEnabled(ctl->hwnd)||ctl->hThemeButton?DST_PREFIXTEXT|DSS_NORMAL:DST_PREFIXTEXT|DSS_DISABLED);
			SelectObject(hdcMem, hOldFont);
		}
		BitBlt(hdcPaint, 0, 0, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
	}
}

static LRESULT CALLBACK MButtonWndProc(HWND hwndDlg, UINT msg,  WPARAM wParam, LPARAM lParam)
{
	MButtonCtrl* bct =  (MButtonCtrl *)GetWindowLongPtr(hwndDlg, 0);
	switch(msg) {
		case WM_NCCREATE:
		SetWindowLongPtr(hwndDlg, GWL_STYLE, GetWindowLongPtr(hwndDlg, GWL_STYLE)|BS_OWNERDRAW);
		bct = (MButtonCtrl*)malloc(sizeof(MButtonCtrl));
		if (bct == NULL)
			return FALSE;

		bct->hwnd = hwndDlg;
		bct->stateId = PBS_NORMAL;
		bct->focus = 0;
		bct->hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		bct->arrow = NULL;
		bct->defbutton = 0;
		bct->hIcon = NULL;
		bct->hBitmap = NULL;
		bct->pushBtn = 0;
		bct->pbState = 0;
		bct->hThemeButton = NULL;
		bct->hThemeToolbar = NULL;
		bct->cHot = 0;
		bct->flatBtn = 0;
		LoadTheme(bct);
		SetWindowLongPtr(hwndDlg, 0, (LONG)bct);
		if (((CREATESTRUCT *)lParam)->lpszName) SetWindowText(hwndDlg, ((CREATESTRUCT *)lParam)->lpszName);
		return TRUE;

	case WM_SETTEXT:
		bct->cHot = 0;
		if ((char*)lParam) {
			char *tmp = (char*)lParam;
			while (*tmp) {
				if (*tmp=='&' && *(tmp+1)) {
					bct->cHot = tolower(*(tmp+1));
					break;
				}
				tmp++;
			}
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;

	case WM_SYSKEYUP:
		if (bct->stateId!=PBS_DISABLED && bct->cHot && bct->cHot == tolower((int)wParam)) {
			if (bct->pushBtn) {
				if (bct->pbState) bct->pbState = 0;
				else bct->pbState = 1;
				InvalidateRect(bct->hwnd, NULL, TRUE);
			}
			SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM)hwndDlg);
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
			
			hdcPaint = BeginPaint(hwndDlg, &ps);
			if (hdcPaint) {
				PaintWorker(bct, hdcPaint);
				EndPaint(hwndDlg, &ps);
			}
		}
		break;

	case BM_SETIMAGE:
		if (wParam == IMAGE_ICON) {
			bct->hIcon = (HICON)lParam;
			bct->hBitmap = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		else if (wParam == IMAGE_BITMAP) {
			bct->hBitmap = (HBITMAP)lParam;
			bct->hIcon = NULL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;

	case BM_SETCHECK:
		if (!bct->pushBtn) break;
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
		if (bct->pushBtn)
			return bct->pbState?BST_CHECKED:BST_UNCHECKED;

		return 0;

	case BUTTONSETARROW: // turn arrow on/off
		if (wParam) {
//				if (!bct->arrow)
//					bct->arrow = (HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_DOWNARROW),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
		}
		else {
			if (bct->arrow) {
				DestroyIcon(bct->arrow);
				bct->arrow = NULL;
			}
		}
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case BUTTONSETDEFAULT:
		bct->defbutton = wParam?1:0;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case BUTTONSETASPUSHBTN:
		bct->pushBtn = 1;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case BUTTONSETASFLATBTN:
		bct->flatBtn = 1;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case BUTTONADDTOOLTIP:
		if (wParam) {
			EnterCriticalSection(&csTips);
			if (!hwndToolTips)
				hwndToolTips = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, "", WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

			TOOLINFO ti = { 0 };
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = bct->hwnd;
			ti.uId = (UINT)bct->hwnd;
			if (SendMessage(hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
				SendMessage(hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);

			ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
			ti.uId = (UINT)bct->hwnd;
			ti.lpszText=(char*)wParam;
			SendMessage(hwndToolTips,TTM_ADDTOOL,0,(LPARAM)&ti);
			LeaveCriticalSection(&csTips);
		}
		break;

	case WM_SETFOCUS: // set keybord focus and redraw
		bct->focus = 1;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_KILLFOCUS: // kill focus and redraw
		bct->focus = 0;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_WINDOWPOSCHANGED:
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_ENABLE: // windows tells us to enable/disable
		bct->stateId = wParam?PBS_NORMAL:PBS_DISABLED;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_MOUSELEAVE: // faked by the WM_TIMER
		if (bct->stateId!=PBS_DISABLED) { // don't change states if disabled
			bct->stateId = PBS_NORMAL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;

	case WM_LBUTTONDOWN:
		if (bct->stateId!=PBS_DISABLED) { // don't change states if disabled
			bct->stateId = PBS_PRESSED;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;

	case WM_LBUTTONUP:
		if (bct->pushBtn) {
			if (bct->pbState) bct->pbState = 0;
			else bct->pbState = 1;
		}
		if (bct->stateId!=PBS_DISABLED) { // don't change states if disabled
			if (msg==WM_LBUTTONUP) bct->stateId = PBS_HOT;
			else bct->stateId = PBS_NORMAL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		// Tell your daddy you got clicked.
		SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM)hwndDlg);
		break;

	case WM_MOUSEMOVE:
		if (bct->stateId == PBS_NORMAL) {
			bct->stateId = PBS_HOT;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		// Call timer, used to start cheesy TrackMouseEvent faker
		SetTimer(hwndDlg,BUTTON_POLLID,BUTTON_POLLDELAY,NULL);
		break;

	case WM_TIMER: // use a timer to check if they have did a mouseout
		if (wParam==BUTTON_POLLID) {
			RECT rc;
			POINT pt;
			GetWindowRect(hwndDlg,&rc);
			GetCursorPos(&pt);
			if(!PtInRect(&rc,pt)) { // mouse must be gone, trigger mouse leave
				PostMessage(hwndDlg,WM_MOUSELEAVE,0,0L);
				KillTimer(hwndDlg,BUTTON_POLLID);
			}
		}
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_DESTROY:
		if (bct) {
			EnterCriticalSection(&csTips);
			if (hwndToolTips) {
				TOOLINFO ti = { 0 };
				ti.cbSize = sizeof(ti);
				ti.uFlags = TTF_IDISHWND;
				ti.hwnd = bct->hwnd;
				ti.uId = (UINT)bct->hwnd;
				if (SendMessage(hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
					SendMessage(hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);

				if (SendMessage(hwndToolTips, TTM_GETTOOLCOUNT, 0, (LPARAM)&ti)==0) {
					DestroyWindow(hwndToolTips);
					hwndToolTips = NULL;
				}
			}
			LeaveCriticalSection(&csTips);
			DestroyTheme(bct);
			free(bct);
		}
		SetWindowLongPtr(hwndDlg,0,(LONG)NULL);
		break;	// DONT! fall thru
	}
	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}

int UnloadButtonModule(WPARAM wParam, LPARAM lParam)
{
	DeleteCriticalSection(&csTips);
	return 0;
}

int LoadButtonModule(void)
{
	WNDCLASSEX wc;
	
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize         = sizeof(wc);
	wc.lpszClassName  = MYMIRANDABUTTONCLASS;
	wc.lpfnWndProc    = MButtonWndProc;
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra     = sizeof(MButtonCtrl*);
	wc.hbrBackground  = 0;
	wc.style          = CS_GLOBALCLASS;
	RegisterClassEx(&wc);
	InitializeCriticalSection(&csTips);
	HookEvent(ME_SYSTEM_SHUTDOWN, UnloadButtonModule);
	return 0;
}
