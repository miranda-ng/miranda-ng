#include "../commonheaders.h"

extern HINSTANCE g_hInst;
HANDLE hStatusBarShowToolTipEvent,hStatusBarHideToolTipEvent;
boolean canloadstatusbar = FALSE;
HWND helperhwnd = 0;
HANDLE hFrameHelperStatusBar;

void CluiProtocolStatusChanged(int parStatus, const char* szProto);

INT_PTR GetConnectingIconService (WPARAM wParam, LPARAM lParam);

int RecreateStatusBar(HWND);
int UseOwnerDrawStatusBar;

#define TM_STATUSBAR 23435234
#define TM_STATUSBARHIDE 23435235
boolean tooltipshoing;
WNDPROC OldWindowProc = NULL;


POINT lastpnt;
RECT OldRc = {0};
static	HBITMAP hBmpBackground;
static int backgroundBmpUse;
static COLORREF bkColour;
extern BYTE showOpts;
int extraspace;

int OnStatusBarBackgroundChange()
{
	showOpts = db_get_b(NULL,"CLUI","SBarShow",1);		
	bkColour = db_get_dw(NULL,"StatusBar","BkColour",CLCDEFAULT_BKCOLOUR);
	if (hBmpBackground)
		DeleteObject(hBmpBackground), hBmpBackground = NULL;

	if ( db_get_b(NULL,"StatusBar","UseBitmap",CLCDEFAULT_USEBITMAP)) {
		DBVARIANT dbv;
		if ( !db_get_ts(NULL, "StatusBar", "BkBitmap", &dbv)) {
			hBmpBackground = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (LPARAM)dbv.ptszVal);
			db_free(&dbv);
		}
	}

	backgroundBmpUse = db_get_w(NULL,"StatusBar","BkBmpUse",CLCDEFAULT_BKBMPUSE);
	extraspace = db_get_dw(NULL,"StatusBar","BkExtraSpace",0);

	RecreateStatusBar(pcli->hwndContactList);
	if (pcli->hwndStatus) InvalidateRect(pcli->hwndStatus,NULL,TRUE);
	return 0;
}


void DrawDataForStatusBar(LPDRAWITEMSTRUCT dis)
{
	ProtocolData *PD = (ProtocolData *)dis->itemData;
	SIZE textSize;
	boolean NeedDestroy = FALSE;

	if (PD == NULL || dis->hDC == NULL)
		return;

	//clip it
	HRGN hrgn = CreateRectRgn(dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom); 
	SelectClipRgn(dis->hDC, hrgn);

	char *szProto = PD->RealName;
	int status = CallProtoService(szProto,PS_GETSTATUS,0,0);
	SetBkMode(dis->hDC,TRANSPARENT);
	int x = dis->rcItem.left+extraspace;

	if (showOpts & 1) {
		HICON hIcon;
		if ( db_get_b(NULL, "CLUI", "UseConnectingIcon", 1) == 1 && status < ID_STATUS_OFFLINE) {
			hIcon = (HICON)GetConnectingIconService((WPARAM)szProto, 0);
			if (hIcon)
				NeedDestroy = TRUE;
			else
				hIcon = LoadSkinnedProtoIcon(szProto,status);
		}
		else hIcon = LoadSkinnedProtoIcon(szProto,status);

		DrawIconEx(dis->hDC,x,(dis->rcItem.top+dis->rcItem.bottom-GetSystemMetrics(SM_CYSMICON))>>1,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
		if (NeedDestroy) DestroyIcon(hIcon);
		x += GetSystemMetrics(SM_CXSMICON)+2;
	}
	else x += 2;

	if (showOpts & 2) {
		PROTOACCOUNT *pa = ProtoGetAccount(szProto);
		TCHAR szName[64];
		mir_sntprintf(szName, SIZEOF(szName), _T("%s%s"), pa->tszAccountName, showOpts&4 ? _T(" ") : _T(""));
		GetTextExtentPoint32(dis->hDC, szName, (int)_tcslen(szName), &textSize);

		TextOut(dis->hDC,x,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,szName,lstrlen(szName));
		x += textSize.cx;
	}

	if (showOpts & 4) {
		TCHAR *szStatus = pcli->pfnGetStatusModeDescription(status, 0);
		if ( !szStatus)
			szStatus = _T("");
		GetTextExtentPoint32(dis->hDC,szStatus,lstrlen(szStatus),&textSize);
		TextOut(dis->hDC,x,(dis->rcItem.top+dis->rcItem.bottom-textSize.cy)>>1,szStatus,lstrlen(szStatus));
	}
	SelectClipRgn(dis->hDC, NULL);
	DeleteObject(hrgn);
}

void DrawBackGround(HWND hwnd,HDC mhdc)
{
	HDC hdcMem,hdc;
	RECT clRect,*rcPaint;

	int yScroll = 0;
	int y;
	PAINTSTRUCT paintst = {0};
	HBITMAP hBmpOsb,holdbmp;
	DWORD style = GetWindowLongPtr(hwnd,GWL_STYLE);
	int grey = 0;
	HFONT oFont;
	HBRUSH hBrushAlternateGrey = NULL;

	HFONT hFont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
	if (mhdc) {
		hdc = mhdc;
		rcPaint = NULL;
	}
	else {
		hdc = BeginPaint(hwnd,&paintst);
		rcPaint = &(paintst.rcPaint);
	}

	GetClientRect(hwnd,&clRect);
	if (rcPaint == NULL) rcPaint = &clRect;
	if (rcPaint->right-rcPaint->left == 0||rcPaint->top-rcPaint->bottom == 0) rcPaint = &clRect;
	y = -yScroll;
	hdcMem = CreateCompatibleDC(hdc);
	hBmpOsb = CreateBitmap(clRect.right,clRect.bottom,1,GetDeviceCaps(hdc,BITSPIXEL),NULL);
	holdbmp = (HBITMAP)SelectObject(hdcMem,hBmpOsb);
	oFont = (HFONT)SelectObject(hdcMem,hFont);
	SetBkMode(hdcMem,TRANSPARENT);
	{
		HBRUSH hBrush = CreateSolidBrush(bkColour);
		HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem,hBrush);
		FillRect(hdcMem,rcPaint,hBrush);
		SelectObject(hdcMem,hoBrush);
		DeleteObject(hBrush);
		if (hBmpBackground) {
			BITMAP bmp;
			int x,y;
			int maxx,maxy;
			int destw,desth;

			GetObject(hBmpBackground,sizeof(bmp),&bmp);
			HDC hdcBmp = CreateCompatibleDC(hdcMem);
			HDC holdbackbmp = (HDC)SelectObject(hdcBmp,hBmpBackground);
			y = backgroundBmpUse&CLBF_SCROLL?-yScroll:0;
			maxx = backgroundBmpUse&CLBF_TILEH?clRect.right:1;
			maxy = backgroundBmpUse&CLBF_TILEV?maxy = rcPaint->bottom:y+1;
			switch(backgroundBmpUse&CLBM_TYPE) {
			case CLB_STRETCH:
				if (backgroundBmpUse&CLBF_PROPORTIONAL) {
					if (clRect.right*bmp.bmHeight<clRect.bottom*bmp.bmWidth) {
						desth = clRect.bottom;
						destw = desth*bmp.bmWidth/bmp.bmHeight;
					}
					else {
						destw = clRect.right;
						desth = destw*bmp.bmHeight/bmp.bmWidth;
					}
				}
				else {
					destw = clRect.right;
					desth = clRect.bottom;
				}
				break;

			case CLB_STRETCHH:
				if (backgroundBmpUse&CLBF_PROPORTIONAL) {
					destw = clRect.right;
					desth = destw*bmp.bmHeight/bmp.bmWidth;
				}
				else {
					destw = clRect.right;
					desth = bmp.bmHeight;
				}
				break;

			case CLB_STRETCHV:
				if (backgroundBmpUse&CLBF_PROPORTIONAL) {
					desth = clRect.bottom;
					destw = desth*bmp.bmWidth/bmp.bmHeight;
				}
				else {
					destw = bmp.bmWidth;
					desth = clRect.bottom;
				}
				break;

			default:    //clb_topleft
				destw = bmp.bmWidth;
				desth = bmp.bmHeight;
				break;
			}
			desth = clRect.bottom -clRect.top;
			for (;y<maxy;y += desth) {
				if (y<rcPaint->top-desth) continue;
				for (x = 0;x<maxx;x += destw)
					StretchBlt(hdcMem,x,y,destw,desth,hdcBmp,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY);
			}
			SelectObject(hdcBmp,holdbackbmp);
			DeleteDC(hdcBmp);
		}
	}

	//call to draw icons
	{
		DRAWITEMSTRUCT ds = { 0 };
		ds.hwndItem = hwnd;
		ds.hDC = hdcMem;

		int startoffset = db_get_dw(NULL,"StatusBar","FirstIconOffset",0);
		extraspace = db_get_dw(NULL,"StatusBar","BkExtraSpace",0);
		int nParts = SendMessage(hwnd,SB_GETPARTS,0,0);

		RECT rc = { 0 }, clrc;
		GetClientRect(hwnd,&clrc);
		clrc.right -= clrc.left;
		clrc.right -= startoffset;
		int sectwidth = clrc.right/nParts;

		for (int nPanel = 0; nPanel < nParts; nPanel++) {
			ProtocolData *PD = (ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,0);
			if (PD == NULL)
				continue;

			SendMessage(hwnd,SB_GETRECT,(WPARAM)nPanel,(LPARAM)&rc);

			rc.top = 0;
			rc.left = nPanel*sectwidth+startoffset;
			rc.right = rc.left+sectwidth-1;
			ds.rcItem = rc;
			ds.itemData = (ULONG_PTR)PD;
			ds.itemID = nPanel;

			DrawDataForStatusBar(&ds);
	}	}

	BitBlt(hdc,rcPaint->left,rcPaint->top,rcPaint->right-rcPaint->left,rcPaint->bottom-rcPaint->top,hdcMem,rcPaint->left,rcPaint->top,SRCCOPY);

	SelectObject(hdcMem,holdbmp);
	SelectObject(hdcMem,oFont);
	DeleteObject(hBmpOsb);
	DeleteDC(hdcMem);
	paintst.fErase = FALSE;
	if ( !mhdc)
		EndPaint(hwnd,&paintst);	
}

static LRESULT CALLBACK StatusBarOwnerDrawProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (UseOwnerDrawStatusBar) {
		switch(uMsg) {
		case WM_ERASEBKGND:
			return 0;

		case WM_PRINT:
			DrawBackGround(hwnd,(HDC)wParam);
			return 0;

		case WM_PAINT:
			DrawBackGround(hwnd,0);
			return 0;
		}
	}
	return mir_callNextSubclass(hwnd, StatusBarOwnerDrawProc, uMsg, wParam, lParam);
}

LRESULT CALLBACK StatusHelperProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CREATE:
		tooltipshoing = FALSE;
		return FALSE;

	case WM_GETMINMAXINFO:
		if (pcli->hwndStatus) {
			RECT rct;
			GetWindowRect(pcli->hwndStatus,&rct);
			memset((LPMINMAXINFO)lParam, 0, sizeof(MINMAXINFO));
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 5;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = rct.bottom-rct.top;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = 1600;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = rct.bottom-rct.top;
			return 0;
		}
		break;

	case WM_NCHITTEST:
	case WM_SHOWWINDOW:
		if (hFrameHelperStatusBar) {
			int res = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS,hFrameHelperStatusBar),0);
			db_set_b(0, "CLUI", "ShowSBar", (res & F_VISIBLE) != 0);
		}

		if (tooltipshoing){
			NotifyEventHooks(hStatusBarHideToolTipEvent, 0, 0);
			tooltipshoing = FALSE;
		}
		return 0;

	case WM_TIMER:
		if (wParam == TM_STATUSBARHIDE) {
			KillTimer(hwnd,TM_STATUSBARHIDE);

			if (tooltipshoing){
				NotifyEventHooks(hStatusBarHideToolTipEvent,0,0);
				tooltipshoing = FALSE;
			}
		}

		if (wParam == TM_STATUSBAR) {
			KillTimer(hwnd,TM_STATUSBAR);

			POINT pt;
			GetCursorPos(&pt);
			if (pt.x == lastpnt.x&&pt.y == lastpnt.y) {
				RECT rc;
				ScreenToClient(pcli->hwndStatus, &pt);
				int nParts = SendMessage(pcli->hwndStatus,SB_GETPARTS,0,0);
				for (int i = 0;i<nParts;i++) {
					SendMessage(pcli->hwndStatus,SB_GETRECT,i,(LPARAM)&rc);
					if (PtInRect(&rc,pt)) {							
						ProtocolData *PD = (ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,i,0);
						if (PD == NULL)
							return 0;

						NotifyEventHooks(hStatusBarShowToolTipEvent,(WPARAM)PD->RealName,0);
						SetTimer(hwnd,TM_STATUSBARHIDE,db_get_w(NULL,"CLUIFrames","HideToolTipTime",5000),0);
						tooltipshoing = TRUE;
						break;
					}
				}
			}
		}
		return 0;
	
	case WM_SETCURSOR:
		{		
			POINT pt;
			GetCursorPos(&pt);
			if (pt.x == lastpnt.x&&pt.y == lastpnt.y)
				return 0;

			lastpnt = pt;
			if (tooltipshoing){
				KillTimer(hwnd,TM_STATUSBARHIDE);				
				NotifyEventHooks(hStatusBarHideToolTipEvent,0,0);
				tooltipshoing = FALSE;		
			}
			KillTimer(hwnd,TM_STATUSBAR);
			SetTimer(hwnd,TM_STATUSBAR,db_get_w(NULL,"CLC","InfoTipHoverTime",750),0);
		}			
		return 0;

	case WM_NOTIFY:
		if (lParam == 0)
			return 0;

		if (((LPNMHDR)lParam)->hwndFrom == pcli->hwndStatus)
			if (((LPNMHDR)lParam)->code == WM_NCHITTEST)
				LPNMMOUSE lpnmmouse = (LPNMMOUSE) lParam;

	case WM_CONTEXTMENU:
		KillTimer(hwnd,TM_STATUSBARHIDE);

		if (tooltipshoing)
			NotifyEventHooks(hStatusBarHideToolTipEvent,0,0);

		tooltipshoing = FALSE;

	case WM_MEASUREITEM:
	case WM_DRAWITEM:
		//parent do all work for us
		return(SendMessage(pcli->hwndContactList,msg,wParam,lParam));

	case WM_MOVE:
		PostMessage(pcli->hwndStatus,WM_MOVE,wParam,lParam);

	case WM_SIZE:
		{
			RECT rc;
			int b;
			if (pcli->hwndStatus != 0) {
				GetClientRect(hwnd,&rc);

				b = LOWORD(lParam);
				if (b != 0&&(rc.right-rc.left) != (OldRc.right-OldRc.left)) {
					OldRc = rc;
					if (canloadstatusbar) 	
						if ( db_get_b(NULL,"CLUI","UseOwnerDrawStatusBar",0)||db_get_b(NULL,"CLUI","EqualSections",1)) 
							CluiProtocolStatusChanged(0,0);
				}
				if (msg == WM_SIZE) PostMessage(pcli->hwndStatus,WM_SIZE,wParam,lParam);
				if (pcli->hwndStatus != 0) InvalidateRect(pcli->hwndStatus,NULL,TRUE);
				return 0;
			}
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND CreateStatusHelper(HWND parent)
{
	WNDCLASS wndclass = {0};
	TCHAR pluginname[] = _T("Statushelper");

	if (GetClassInfo(g_hInst,pluginname,&wndclass) == 0 ) {
		wndclass.style         = 0;
		wndclass.lpfnWndProc   = StatusHelperProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = g_hInst;
		wndclass.hIcon         = NULL;
		wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = pluginname;
		RegisterClass(&wndclass);
	}

	return(CreateWindow(pluginname,pluginname,
		/*WS_THICKFRAME|*/WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,
		0,0,0,0,parent,NULL,g_hInst,NULL));
}

HANDLE CreateStatusBarFrame()
{
	RECT rc;
	GetWindowRect(helperhwnd,&rc);
	int h = rc.bottom-rc.top;

	CLISTFrame Frame = { 0 };
	Frame.cbSize = sizeof(CLISTFrame);
	Frame.hWnd = helperhwnd;
	Frame.align = alBottom;
	Frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
	Frame.Flags = ( db_get_b(NULL,"CLUI","ShowSBar",1)?F_VISIBLE:0)|F_LOCKED|F_NOBORDER|F_TCHAR;
	Frame.height = (h == 0) ? 20 : h;
	Frame.tname = _T("Status");
	Frame.TBtname = TranslateT("Status bar");
	hFrameHelperStatusBar = (HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME,(WPARAM)&Frame,0);

	return hFrameHelperStatusBar;
}

int RecreateStatusBar(HWND parent)
{
	if (pcli->hwndStatus) {
		FreeProtocolData();
		DestroyWindow(pcli->hwndStatus);
	}
	pcli->hwndStatus = 0;
	if (hFrameHelperStatusBar) CallService(MS_CLIST_FRAMES_REMOVEFRAME,(WPARAM)hFrameHelperStatusBar,0);

	helperhwnd = CreateStatusHelper(parent);
	UseOwnerDrawStatusBar = db_get_b(NULL,"CLUI","UseOwnerDrawStatusBar",0);

	//create the status wnd
	pcli->hwndStatus = CreateStatusWindow(
		( db_get_b(0,"CLUI","SBarUseSizeGrip",TRUE) && (!UseOwnerDrawStatusBar)?SBARS_SIZEGRIP:0)|
		WS_CHILD | ( db_get_b(NULL,"CLUI","ShowSBar",1)?WS_VISIBLE:0), _T(""), helperhwnd, 0);

	OldWindowProc = (WNDPROC)GetWindowLongPtr(pcli->hwndStatus,GWLP_WNDPROC);
	mir_subclassWindow(pcli->hwndStatus, StatusBarOwnerDrawProc);
	CreateStatusBarFrame();

	SetWindowPos(helperhwnd,NULL,1,1,1,1,SWP_NOZORDER);
	CluiProtocolStatusChanged(0,0);
	CallService(MS_CLIST_FRAMES_UPDATEFRAME,-1,0);
	return 0;
}

HWND CreateStatusBarhWnd(HWND parent)
{	
	RecreateStatusBar(parent);
	OnStatusBarBackgroundChange();

	hStatusBarShowToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_SHOW_TOOLTIP);
	hStatusBarHideToolTipEvent = CreateHookableEvent(ME_CLIST_FRAMES_SB_HIDE_TOOLTIP);
	return pcli->hwndStatus;
}
