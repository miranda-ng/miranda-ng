/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "commonheaders.h"
#include <initguid.h>
#include <oleacc.h>

// TODO:
// - Support for bitmap buttons (simple call to DrawIconEx())

static LRESULT CALLBACK MButtonWndProc(HWND hwnd, UINT  msg, WPARAM wParam, LPARAM lParam);

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
	HTHEME  hThemeButton;
	HTHEME  hThemeToolbar;
	char	cHot;
	int     flatBtn;
	HWND    hwndToolTips;
	IAccPropServices* pAccPropServices;
} MButtonCtrl;


static CRITICAL_SECTION csTips;
static SortedList lToolTips;
static BOOL bModuleInitialized = FALSE;

typedef struct
{
	DWORD ThreadId;
	HWND  hwnd;
} TTooltips;

int LoadButtonModule(void)
{
	WNDCLASSEX wc = {0};

	if ( bModuleInitialized ) return 0;
	bModuleInitialized = TRUE;

	wc.cbSize         = sizeof(wc);
	wc.lpszClassName  = MIRANDABUTTONCLASS;
	wc.lpfnWndProc    = MButtonWndProc;
	wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wc.cbWndExtra     = sizeof(MButtonCtrl*);
	wc.hbrBackground  = 0;
	wc.style          = CS_GLOBALCLASS;
	RegisterClassEx(&wc);

	InitializeCriticalSection(&csTips);
	lToolTips.increment = 1;
	lToolTips.sortFunc = NumericKeySort;
	return 0;
}

void UnloadButtonModule()
{
	if ( !bModuleInitialized ) return;
    EnterCriticalSection(&csTips);
	List_Destroy(&lToolTips);
    LeaveCriticalSection(&csTips);
	DeleteCriticalSection(&csTips);
}

// Used for our own cheap TrackMouseEvent
#define BUTTON_POLLID       100
#define BUTTON_POLLDELAY    50

static void DestroyTheme(MButtonCtrl *ctl) {
	if (closeThemeData) {
		if (ctl->hThemeButton) {
			closeThemeData(ctl->hThemeButton);
			ctl->hThemeButton = NULL;
		}
		if (ctl->hThemeToolbar) {
			closeThemeData(ctl->hThemeToolbar);
			ctl->hThemeToolbar = NULL;
		}
	}
}

static void LoadTheme(MButtonCtrl *ctl)
{
	if (openThemeData) {
		DestroyTheme(ctl);
		ctl->hThemeButton = openThemeData(ctl->hwnd, L"BUTTON");
		ctl->hThemeToolbar = openThemeData(ctl->hwnd, L"TOOLBAR");
	}
}

static void SetHwndPropInt(MButtonCtrl* bct, DWORD idObject, DWORD idChild, MSAAPROPID idProp, int val) 
{
	if (bct->pAccPropServices == NULL) return;
	VARIANT var;
	var.vt = VT_I4;
	var.lVal = val;
	bct->pAccPropServices->SetHwndProp(bct->hwnd, idObject, idChild, idProp, var);
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

static void PaintWorker(MButtonCtrl *ctl, HDC hdcPaint)
{
	if (hdcPaint) {
		HDC hdcMem;
		HBITMAP hbmMem;
		HDC hOld;
		RECT rcClient;

		GetClientRect(ctl->hwnd, &rcClient);
		hdcMem = CreateCompatibleDC(hdcPaint);
		hbmMem = CreateCompatibleBitmap(hdcPaint, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top);
		hOld = ( HDC )SelectObject(hdcMem, hbmMem);

		// If its a push button, check to see if it should stay pressed
		if (ctl->pushBtn && ctl->pbState) ctl->stateId = PBS_PRESSED;

		// Draw the flat button
		if (ctl->flatBtn) {
			if (ctl->hThemeToolbar) {
				int state = IsWindowEnabled(ctl->hwnd)?(ctl->stateId==PBS_NORMAL&&ctl->defbutton?PBS_DEFAULTED:ctl->stateId):PBS_DISABLED;
				if (isThemeBackgroundPartiallyTransparent(ctl->hThemeToolbar, TP_BUTTON, TBStateConvert2Flat(state))) {
					drawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);
				}
				drawThemeBackground(ctl->hThemeToolbar, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), &rcClient, &rcClient);
			}
			else {
				HBRUSH hbr;

				if (ctl->stateId==PBS_PRESSED||ctl->stateId==PBS_HOT)
					hbr = GetSysColorBrush(COLOR_3DLIGHT);
				else {
					HWND hwndParent = GetParent(ctl->hwnd);
					HDC dc = GetDC(hwndParent);
					HBRUSH oldBrush = (HBRUSH)GetCurrentObject( dc, OBJ_BRUSH );
					hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)dc, (LPARAM)hwndParent);
					SelectObject(dc,oldBrush);
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
				if (isThemeBackgroundPartiallyTransparent(ctl->hThemeButton, BP_PUSHBUTTON, state)) {
					drawThemeParentBackground(ctl->hwnd, hdcMem, &rcClient);
				}
				drawThemeBackground(ctl->hThemeButton, hdcMem, BP_PUSHBUTTON, state, &rcClient, &rcClient);
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
			{
				HIMAGELIST hImageList;
				HICON hIconNew;

				hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON), IsWinVerXPPlus()? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 1, 0);
				ImageList_AddIcon(hImageList, ctl->hIcon);
				hIconNew = ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
				DrawState(hdcMem,NULL,NULL,(LPARAM)hIconNew,0,ix,iy,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),IsWindowEnabled(ctl->hwnd)?DST_ICON|DSS_NORMAL:DST_ICON|DSS_DISABLED);
				ImageList_RemoveAll(hImageList);
				ImageList_Destroy(hImageList);
				DestroyIcon(hIconNew);
			}
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
			TCHAR szText[MAX_PATH];
			SIZE sz;
			RECT rcText;
			HFONT hOldFont;

			CopyRect(&rcText, &rcClient);
			GetWindowText(ctl->hwnd, szText, SIZEOF(szText));
			SetBkMode(hdcMem, TRANSPARENT);
			hOldFont = (HFONT)SelectObject(hdcMem, ctl->hFont);
			// XP w/themes doesn't used the glossy disabled text.  Is it always using COLOR_GRAYTEXT?  Seems so.
			SetTextColor(hdcMem, IsWindowEnabled(ctl->hwnd)||!ctl->hThemeButton?GetSysColor(COLOR_BTNTEXT):GetSysColor(COLOR_GRAYTEXT));
			GetTextExtentPoint32(hdcMem, szText, lstrlen(szText), &sz);
			if (ctl->cHot) {
				SIZE szHot;

				GetTextExtentPoint32 (hdcMem, _T("&"), 1, &szHot);
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
		SetWindowLongPtr(hwndDlg, GWL_STYLE, GetWindowLongPtr(hwndDlg, GWL_STYLE) | BS_OWNERDRAW);
		bct = ( MButtonCtrl* )mir_calloc(sizeof(MButtonCtrl));
		if (bct==NULL) return FALSE;
		bct->hwnd = hwndDlg;
		bct->stateId = PBS_NORMAL;
		bct->hFont = ( HFONT )GetStockObject(DEFAULT_GUI_FONT);
		LoadTheme(bct);
		if (SUCCEEDED(CoCreateInstance(CLSID_AccPropServices, NULL, CLSCTX_SERVER, 
			IID_IAccPropServices, (void**)&bct->pAccPropServices))) 
		{
			// Annotating the Role of this object to be PushButton
			SetHwndPropInt(bct, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_ROLE, ROLE_SYSTEM_PUSHBUTTON);
		} 
		else 
			bct->pAccPropServices = NULL;
		SetWindowLongPtr(hwndDlg, 0, (LONG_PTR)bct);
		if (((CREATESTRUCT *)lParam)->lpszName) SetWindowText(hwndDlg, ((CREATESTRUCT *)lParam)->lpszName);
		return TRUE;

	case WM_DESTROY:
		if (bct) {
			if (bct->pAccPropServices) {
				bct->pAccPropServices->Release();
				bct->pAccPropServices = NULL;
			}
			if (bct->hwndToolTips) {
				TOOLINFO ti = {0};
				ti.cbSize = sizeof(ti);
				ti.uFlags = TTF_IDISHWND;
				ti.hwnd = bct->hwnd;
				ti.uId = (UINT_PTR)bct->hwnd;
				if (SendMessage(bct->hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti)) {
					SendMessage(bct->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);
				}
				if ( SendMessage(bct->hwndToolTips, TTM_GETTOOLCOUNT, 0, (LPARAM)&ti) == 0 ) {
					int idx;
					TTooltips tt;
					tt.ThreadId = GetCurrentThreadId();
			        
                    EnterCriticalSection(&csTips);
					if ( List_GetIndex( &lToolTips, &tt, &idx ) ) {
						mir_free( lToolTips.items[idx] );
						List_Remove( &lToolTips, idx );
						DestroyWindow( bct->hwndToolTips );
					}
			        LeaveCriticalSection(&csTips);
					
                    bct->hwndToolTips = NULL;
				}
			}
			if (bct->arrow) IconLib_ReleaseIcon(bct->arrow, 0);
			DestroyTheme(bct);
		}
		break;	// DONT! fall thru

    case WM_NCDESTROY:
		mir_free(bct);
        break;

	case WM_SETTEXT:
		bct->cHot = 0;
		if ( lParam != 0 ) {
			TCHAR *tmp = ( TCHAR* )lParam;
			while (*tmp) {
				if (*tmp=='&' && *(tmp+1)) {
					bct->cHot = _tolower(*(tmp+1));
					break;
				}
				tmp++;
			}
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		break;

	case WM_KEYUP:
		if (bct->stateId!=PBS_DISABLED && wParam == VK_SPACE) {
			if (bct->pushBtn) {
				if (bct->pbState) {
					bct->pbState = 0;
					bct->stateId = PBS_NORMAL;
				}
				else {
					bct->pbState = 1;
					bct->stateId = PBS_PRESSED;
				}
				InvalidateRect(bct->hwnd, NULL, TRUE);
			}
			SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM)hwndDlg);
			return 0;
		}
		break;

	case WM_SYSKEYUP:
		if (bct->stateId!=PBS_DISABLED && bct->cHot && bct->cHot == tolower((int)wParam)) {
			if (bct->pushBtn) {
				if (bct->pbState) {
					bct->pbState = 0;
					bct->stateId = PBS_NORMAL;
				}
				else {
					bct->pbState = 1;
					bct->stateId = PBS_PRESSED;
				}
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
		break;
	}
	case BM_SETIMAGE:
	{
		HGDIOBJ hnd = NULL;
		if (bct->hIcon) hnd = bct->hIcon;
		else if (bct->hBitmap) hnd = bct->hBitmap;

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
		return (LRESULT)hnd;
	}
	case BM_GETIMAGE:
		if (bct->hIcon) return (LRESULT)bct->hIcon;
		else if (bct->hBitmap) return (LRESULT)bct->hBitmap;
		else return 0;
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
		if (bct->pushBtn) {
			return bct->pbState?BST_CHECKED:BST_UNCHECKED;
		}
		return 0;
	case BUTTONSETARROW: // turn arrow on/off
		if (wParam) {
			if (!bct->arrow) {
				bct->arrow = LoadSkinIcon(SKINICON_OTHER_DOWNARROW);
				SetHwndPropInt(bct, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_ROLE, ROLE_SYSTEM_BUTTONDROPDOWN);
			}
		}
		else {
			if (bct->arrow) {
				IconLib_ReleaseIcon(bct->arrow, 0);
				bct->arrow = NULL;
				SetHwndPropInt(bct, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_ROLE, ROLE_SYSTEM_PUSHBUTTON);
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
		if ( wParam ) {
			TOOLINFO ti = {0};
			if ( !bct->hwndToolTips ) {
				int idx;
				TTooltips tt;
				tt.ThreadId = GetCurrentThreadId();
			    
                EnterCriticalSection(&csTips);
				if ( List_GetIndex( &lToolTips, &tt, &idx )) {
					bct->hwndToolTips = ((TTooltips*)lToolTips.items[idx])->hwnd;
				} else {
					TTooltips *ptt = ( TTooltips* )mir_alloc( sizeof(TTooltips) );
					ptt->ThreadId = tt.ThreadId;
					ptt->hwnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), TTS_ALWAYSTIP, 0, 0, 0, 0, NULL, NULL, hMirandaInst, NULL);
					List_Insert( &lToolTips, ptt, idx );
					bct->hwndToolTips = ptt->hwnd;
				}
    			LeaveCriticalSection(&csTips);
			}
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND;
			ti.hwnd = bct->hwnd;
			ti.uId = (UINT_PTR)bct->hwnd;
			if (SendMessage(bct->hwndToolTips, TTM_GETTOOLINFO, 0, (LPARAM)&ti))
				SendMessage(bct->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)&ti);
			ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
			ti.uId = (UINT_PTR)bct->hwnd;
			#if defined( _UNICODE )
				if ( lParam & BATF_UNICODE )
					ti.lpszText = mir_wstrdup( TranslateW(( WCHAR* )wParam ));
				else
					ti.lpszText = LangPackPcharToTchar(( char* )wParam );
			#else
				ti.lpszText = Translate(( char* )wParam );
			#endif
			if (bct->pAccPropServices) {
				wchar_t *tmpstr = mir_t2u(ti.lpszText);
				bct->pAccPropServices->SetHwndPropStr(bct->hwnd, OBJID_CLIENT, 
					CHILDID_SELF, PROPID_ACC_DESCRIPTION, tmpstr);
				mir_free(tmpstr);
			}
			SendMessage( bct->hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)&ti);
			#if defined( _UNICODE )
				mir_free( ti.lpszText );
			#endif
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
    {
        int showClick = 0;
		if (bct->pushBtn) {
			if (bct->pbState) bct->pbState = 0;
			else bct->pbState = 1;
		}
		if (bct->stateId!=PBS_DISABLED) { // don't change states if disabled
            if (bct->stateId==PBS_PRESSED)
                showClick = 1;
			if (msg==WM_LBUTTONUP) bct->stateId = PBS_HOT;
			else bct->stateId = PBS_NORMAL;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
        if (showClick) // Tell your daddy you got clicked.
            SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM)hwndDlg);
		break;
    }
	case WM_MOUSEMOVE:
		if (bct->stateId == PBS_NORMAL) {
			bct->stateId = PBS_HOT;
			InvalidateRect(bct->hwnd, NULL, TRUE);
		}
		// Call timer, used to start cheesy TrackMouseEvent faker
		SetTimer(hwndDlg,BUTTON_POLLID,BUTTON_POLLDELAY,NULL);
		break;
	case WM_TIMER: // use a timer to check if they have did a mouseout
		if (wParam == BUTTON_POLLID) {
			RECT rc;
			POINT pt;
			GetWindowRect(hwndDlg,&rc);
			GetCursorPos(&pt);
			if(!PtInRect(&rc,pt)) { // mouse must be gone, trigger mouse leave
				PostMessage(hwndDlg,WM_MOUSELEAVE,0,0L);
				KillTimer(hwndDlg,BUTTON_POLLID);
		}	}
		break;

	case WM_ERASEBKGND:
		return 1;
	}
	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}
