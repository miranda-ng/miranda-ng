#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"
#include "m_api/m_skinbutton.h"
#include "hdr/modern_clcpaint.h"

#include <m_button_int.h>

#ifdef __MINGW32__
#include <ctype.h>
#endif

#define BUTTON_POLLID       100
#define BUTTON_POLLDELAY    50
#define b2str(a) ((a) ? "True" : "False")

struct TBBUTTONDATA : public MButtonCtrl
{
	char  szButtonID[64];  // Unique stringID of button in form Module.Name
	BOOL  fSendOnDown;     // send event on button pushed
	BOOL  fHotMark;        // button is hot marked (e.g. current state)
	BOOL  fFocused;
	int   nFontID;         // internal font ID
	HICON hIconPrivate;    // icon need to be destroyed
	TCHAR szText[128];     // text on the button
	RECT  rcMargins;       // margins of inner content

	HANDLE  hIcolibHandle; // handle of icon in iconlib

	XPTHANDLE	hThemeButton;
	XPTHANDLE	hThemeToolbar;
};

static CRITICAL_SECTION csTips;
static HWND hwndToolTips = NULL;
static BOOL	bThemed = FALSE;

static HANDLE hButtonWindowList = NULL;
static HANDLE hBkgChangedHook = NULL;

static int OnIconLibIconChanged(WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(hButtonWindowList, MBM_REFRESHICOLIBICON,0,0);
	return 0;
}

static void InvalidateParentRect(HWND hwndChild, RECT * lpRect, BOOL fErase)
{
	LONG lExStyle = GetWindowLongPtr(hwndChild,GWL_EXSTYLE);
	if (lExStyle & WS_EX_TRANSPARENT) {
		NMHDR hdr;
		hdr.hwndFrom = hwndChild;
		hdr.idFrom = 0;
		hdr.code = BUTTONNEEDREDRAW;
		SendMessage(GetParent(hwndChild),WM_NOTIFY,(WPARAM)hwndChild,(LPARAM)&hdr);
	}
	else InvalidateRect(hwndChild,lpRect,fErase);
}

static int TBStateConvert2Flat(int state)
{
	switch (state) {
	case PBS_NORMAL:
		return TS_NORMAL;
	case PBS_HOT:
		return TS_HOT;
	case PBS_PRESSED:
		return TS_PRESSED;
	case PBS_DISABLED:
		return TS_DISABLED;
	case PBS_DEFAULTED:
		return TS_NORMAL;
	}
	return TS_NORMAL;
}

static void PaintWorker(TBBUTTONDATA *lpSBData, HDC hdcPaint , POINT * pOffset)
{
	HDC hdcMem;
	HBITMAP hbmMem;	
	RECT rcClient;
	int width;
	int height;
	HBITMAP hbmOld = NULL;
	HFONT hOldFont = NULL;
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	POINT offset = {0};
	if (pOffset) offset = *pOffset;

	if ( !hdcPaint) return;  //early exit

	GetClientRect(lpSBData->hwnd, &rcClient);
	width   = rcClient.right - rcClient.left;
	height  = rcClient.bottom - rcClient.top;

	hdcMem = pOffset?hdcPaint:CreateCompatibleDC(hdcPaint);
	hOldFont = (HFONT)SelectObject(hdcMem, lpSBData->hFont);
	if ( !pOffset) {
		hbmMem = ske_CreateDIB32(width, height);
		hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);		
	}
	else OffsetRect(&rcClient,offset.x,offset.y);

	if ( !g_CluiData.fDisableSkinEngine) {
		char szRequest[128];
		/* painting */
		mir_snprintf(szRequest,SIZEOF(szRequest),"Button,ID = %s,Hovered = %s,Pressed = %s,Focused = %s",
			lpSBData->szButtonID,				// ID		
			b2str(lpSBData->stateId == PBS_HOT),	// Hovered
			b2str(lpSBData->stateId == PBS_PRESSED || lpSBData->bIsPushed == TRUE),	// Pressed
			b2str(lpSBData->fFocused));		// Focused

		SkinDrawGlyph(hdcMem,&rcClient,&rcClient,szRequest);
	}
	else 
	{
		if (xpt_IsThemed(lpSBData->hThemeToolbar))
		{
			RECT *rc = &rcClient;
			int state = IsWindowEnabled(lpSBData->hwnd) ? /*(lpSBData->stateId == PBS_PRESSED || lpSBData->bIsPushed == TRUE) ? PBS_PRESSED :*/ (lpSBData->stateId == PBS_NORMAL && lpSBData->bIsDefault ? PBS_DEFAULTED : lpSBData->stateId) : PBS_DISABLED;
			xpt_DrawTheme(lpSBData->hThemeToolbar,lpSBData->hwnd,hdcMem,TP_BUTTON, TBStateConvert2Flat(state), rc, rc);
		}
		else 
		{
			HBRUSH hbr = NULL;

			if (lpSBData->stateId == PBS_PRESSED || lpSBData->stateId == PBS_HOT)
				hbr = GetSysColorBrush(COLOR_3DLIGHT);
			else {
				RECT btnRect;			
				POINT pt = {0};
				int ret;
				HWND hwndParent = GetParent(lpSBData->hwnd);
				HDC dc = CreateCompatibleDC(NULL);
				HBITMAP memBM, oldBM;
				GetWindowRect(hwndParent,&btnRect);
				memBM = ske_CreateDIB32( btnRect.right-btnRect.left, btnRect.bottom-btnRect.top );
				oldBM = (HBITMAP)SelectObject ( dc, memBM );
				ret = SendMessage(hwndParent,WM_ERASEBKGND,(WPARAM)dc,0);
				GetWindowRect(lpSBData->hwnd,&btnRect);
				ClientToScreen(hwndParent,&pt);
				OffsetRect(&btnRect,-pt.x,-pt.y);
				if (ret)
					BitBlt(hdcMem,0,0,btnRect.right-btnRect.left,btnRect.bottom-btnRect.top,dc,btnRect.left,btnRect.top,SRCCOPY);					
				oldBM = (HBITMAP)SelectObject ( dc, oldBM );
				DeleteObject(memBM);
				DeleteDC(dc);
				if ( !ret)	//WM_ERASEBKG return false need to paint
				{
					HDC pdc = GetDC(hwndParent);
					HBRUSH oldBrush = (HBRUSH)GetCurrentObject( pdc, OBJ_BRUSH );
					hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)pdc, (LPARAM)hwndParent);
					SelectObject(pdc,oldBrush);
					ReleaseDC(hwndParent,pdc);
				}

			}
			if (hbr) {
				FillRect(hdcMem, &rcClient, hbr);
				DeleteObject(hbr);
			}
			if (lpSBData->stateId == PBS_HOT || lpSBData->fFocused) {
				if (lpSBData->bIsPushed)
					DrawEdge(hdcMem,&rcClient, EDGE_ETCHED,BF_RECT|BF_SOFT);
				else DrawEdge(hdcMem,&rcClient, BDR_RAISEDOUTER,BF_RECT|BF_SOFT|BF_FLAT);
			}
			else if (lpSBData->stateId == PBS_PRESSED)
				DrawEdge(hdcMem, &rcClient, BDR_SUNKENOUTER,BF_RECT|BF_SOFT);
		}
	}

	RECT rcTemp	 = rcClient;  //content rect
	BYTE bPressed = (lpSBData->stateId == PBS_PRESSED || lpSBData->bIsPushed == TRUE)?1:0;
	HICON hHasIcon = lpSBData->hIcon?lpSBData->hIcon:lpSBData->hIconPrivate?lpSBData->hIconPrivate:NULL;
	BOOL fHasText  = (lpSBData->szText[0] != '\0');

	/* formatter */
	RECT rcIcon;
	RECT rcText;
	
	if ( !g_CluiData.fDisableSkinEngine) {
		/* correct rect according to rcMargins */

		rcTemp.left	 += lpSBData->rcMargins.left;
		rcTemp.top  += lpSBData->rcMargins.top;
		rcTemp.bottom  -= lpSBData->rcMargins.bottom;
		rcTemp.right  -= lpSBData->rcMargins.right;
	}

	rcIcon = rcTemp;
	rcText = rcTemp;

	/* reposition button items */
	if (hHasIcon && fHasText ) {
		rcIcon.right = rcIcon.left+16; /* CXSM_ICON */
		rcText.left = rcIcon.right+2;
	}
	else if (hHasIcon) {
		rcIcon.left += (rcIcon.right-rcIcon.left)/2-8;
		rcIcon.right = rcIcon.left+16;
	}

	/*	Check sizes*/
	if (hHasIcon && (rcIcon.right>rcTemp.right || rcIcon.bottom>rcTemp.bottom || rcIcon.left < rcTemp.left || rcIcon.top < rcTemp.top))
		hHasIcon = NULL;

	if (fHasText && (rcText.right>rcTemp.right || rcText.bottom>rcTemp.bottom || rcText.left < rcTemp.left || rcText.top < rcTemp.top))
		fHasText = FALSE;			

	if (hHasIcon) {
		/* center icon vertically */
		rcIcon.top += (rcClient.bottom-rcClient.top)/2 - 8; /* CYSM_ICON/2 */
		rcIcon.bottom = rcIcon.top + 16; /* CYSM_ICON */
		/* draw it */
		ske_DrawIconEx(hdcMem, rcIcon.left+bPressed, rcIcon.top+bPressed, hHasIcon,
			16, 16, 0, NULL, DI_NORMAL);
	}

	if (fHasText) {
		BOOL bCentered = TRUE;
		SetBkMode(hdcMem,TRANSPARENT);
		if (lpSBData->nFontID >= 0)
			g_clcPainter.ChangeToFont(hdcMem,NULL,lpSBData->nFontID,NULL);
		
		RECT TextRequiredRect = rcText;
		ske_DrawText(hdcMem, lpSBData->szText, -1, &TextRequiredRect, DT_CENTER | DT_VCENTER | DT_CALCRECT | DT_SINGLELINE);
		if (TextRequiredRect.right-TextRequiredRect.left>rcText.right-rcText.left)
			bCentered = FALSE;

		ske_DrawText(hdcMem, lpSBData->szText, -1, &rcText, (bCentered ? DT_CENTER: 0) | DT_VCENTER | DT_SINGLELINE);
		ske_ResetTextEffect(hdcMem);
	}
	if ( !pOffset)
		BitBlt(hdcPaint,0,0,width,height,hdcMem,0,0,SRCCOPY);

	// better to use try/finally but looks like last one is Microsoft specific
	SelectObject(hdcMem,hOldFont);
	if ( !pOffset) {	
		SelectObject(hdcMem,hbmOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
	}
}

static LRESULT CALLBACK ToolbarButtonProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	TBBUTTONDATA *lpSBData = (TBBUTTONDATA *) GetWindowLongPtr(hwndDlg, 0);
	switch (msg) {
	case WM_DESTROY:
		xpt_FreeThemeForWindow(hwndDlg);
		WindowList_Remove(hButtonWindowList, hwndDlg);
		if (lpSBData) {
			if (lpSBData->hIconPrivate)
				DestroyIcon(lpSBData->hIconPrivate);
			free(lpSBData);  // lpSBData was malloced by native malloc
		}
		break;  // DONT! fall thru

	case WM_SETTEXT:
		lstrcpyn(lpSBData->szText, (TCHAR *)lParam, SIZEOF(lpSBData->szText)-1);
		lpSBData->szText[SIZEOF(lpSBData->szText)-1] = '\0';
		break;

	case WM_SETFONT:			
		// remember the font so we can use it later
		lpSBData->hFont = (HFONT) wParam; // maybe we should redraw?
		lpSBData->nFontID = (int) lParam - 1;
		break;
	
	case BUTTONSETSENDONDOWN:
		lpSBData->fSendOnDown = (BOOL) lParam;
		break;

	case BUTTONSETMARGINS:
		if (lParam)	lpSBData->rcMargins = *(RECT*)lParam;
		else {
			RECT nillRect = {0};
			lpSBData->rcMargins = nillRect;
		}
		return 0;

	case BUTTONSETID:
		lstrcpynA(lpSBData->szButtonID, (char *)lParam, SIZEOF(lpSBData->szButtonID)-1);
		lpSBData->szButtonID[SIZEOF(lpSBData->szButtonID)-1] = '\0';
		return 0;

	case BUTTONDRAWINPARENT:
		if (IsWindowVisible(hwndDlg))
			PaintWorker(lpSBData, (HDC) wParam, (POINT*) lParam);
		return 0;

	case WM_NCPAINT:
	case WM_PAINT:
		if (g_CluiData.fDisableSkinEngine) {
			PAINTSTRUCT ps;
			HDC hdcPaint = BeginPaint(hwndDlg, &ps);
			if (hdcPaint) {
				PaintWorker(lpSBData, hdcPaint, NULL);
				EndPaint(hwndDlg, &ps);
			}
		}
		ValidateRect(hwndDlg,NULL);
		return 1;

	case WM_CAPTURECHANGED:
		if ( (HWND)lParam != lpSBData->hwnd && lpSBData->stateId != PBS_DISABLED) {
			// don't change states if disabled
			lpSBData->stateId = PBS_NORMAL;
			InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			int xPos = ( ( int )( short ) LOWORD( lParam ));
			int yPos = ( ( int )( short ) HIWORD( lParam ));
			POINT ptMouse = { xPos, yPos };

			RECT rcClient;
			GetClientRect( lpSBData->hwnd, &rcClient );

			if ( !PtInRect( &rcClient, ptMouse ))
			{
				lpSBData->fHotMark = FALSE;
				ReleaseCapture();
				break;
			}

			if (lpSBData->stateId != PBS_DISABLED && lpSBData->stateId != PBS_PRESSED) 
			{
				lpSBData->stateId = PBS_PRESSED;
				lpSBData->fHotMark = TRUE;
				InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
				if (lpSBData->fSendOnDown) 
				{
					SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM) hwndDlg);
					lpSBData->stateId = PBS_NORMAL;
					InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
				}
			}
			SetCapture( lpSBData->hwnd );
			break;
		}

	case WM_LBUTTONUP:
		if ( GetCapture() == lpSBData->hwnd ) {
			int xPos = ( ( int )( short ) LOWORD( lParam ));
			int yPos = ( ( int )( short ) HIWORD( lParam ));
			POINT ptMouse = { xPos, yPos };

			RECT rcClient;
			GetClientRect( lpSBData->hwnd, &rcClient );
			
			if ( !PtInRect( &rcClient, ptMouse )) {
				lpSBData->fHotMark = FALSE;
				ReleaseCapture();
				break;
			}

			if (lpSBData->bIsPushBtn) {
				if (lpSBData->bIsPushed)
					lpSBData->bIsPushed = FALSE;
				else
					lpSBData->bIsPushed = TRUE;
			}

			if (lpSBData->stateId != PBS_DISABLED)
			{
				// don't change states if disabled
				if (msg == WM_LBUTTONUP)
					lpSBData->stateId = PBS_HOT;
				else
					lpSBData->stateId = PBS_NORMAL;
				InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
			}
			if ( !lpSBData->fSendOnDown && lpSBData->fHotMark)
				SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM) hwndDlg);
			lpSBData->fHotMark = FALSE;
			break;
		}
	case WM_MOUSEMOVE:
		{
			RECT rc;
			POINT pt;
			BOOL bPressed = (wParam & MK_LBUTTON) != 0;
			if ( bPressed && !lpSBData->fHotMark )
				break;
			GetWindowRect(hwndDlg, &rc);
			GetCursorPos(&pt);
			BOOL inClient = PtInRect(&rc, pt);
			if ( inClient )
			{
				SetCapture( lpSBData->hwnd );
				if ( lpSBData->stateId == PBS_NORMAL ) 
				{
					lpSBData->stateId = PBS_HOT;
					InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
				}
			}

			if ( !inClient && lpSBData->stateId == PBS_PRESSED )
			{
				lpSBData->stateId = PBS_HOT; 
				InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
			}
			else if ( inClient && lpSBData->stateId == PBS_HOT && bPressed )
			{
				if ( lpSBData->fHotMark )
				{
					lpSBData->stateId = PBS_PRESSED;
					InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
				}
			}
			else if ( !inClient && !bPressed)
			{
				lpSBData->fHotMark = FALSE;
				ReleaseCapture();
			}
		}
		break;

	case WM_NCHITTEST:
		{
			LRESULT lr = SendMessage(GetParent(hwndDlg), WM_NCHITTEST, wParam, lParam);
			if (lr == HTLEFT || lr == HTRIGHT || lr == HTBOTTOM || lr == HTTOP || lr == HTTOPLEFT || lr == HTTOPRIGHT
				 ||  lr == HTBOTTOMLEFT || lr == HTBOTTOMRIGHT)
				return HTTRANSPARENT;
		}
		break;

	case MBM_SETICOLIBHANDLE:
		if (lpSBData->hIconPrivate) {
			DestroyIcon(lpSBData->hIconPrivate);
			lpSBData->hIconPrivate = 0;
		}

		lpSBData->hIcolibHandle = (HANDLE)lParam;
		if (lpSBData->hIcolibHandle)
			lpSBData->hIcon = (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0 , (LPARAM) lpSBData->hIcolibHandle);
		else
			lpSBData->hIcon = NULL;
		return 1;

	case MBM_REFRESHICOLIBICON:
		if (lpSBData->hIconPrivate) {
			DestroyIcon(lpSBData->hIconPrivate);
			lpSBData->hIconPrivate = 0;
		}
		if (lpSBData->hIcolibHandle)
			lpSBData->hIcon = (HICON)CallService(MS_SKIN2_GETICONBYHANDLE, 0 , (LPARAM) lpSBData->hIcolibHandle);
		else		
			lpSBData->hIcon = NULL;
		InvalidateRect(hwndDlg,NULL,TRUE);
		pcli->pfnInvalidateRect(GetParent(GetParent(hwndDlg)),NULL,TRUE);
		return 1;

	case MBM_UPDATETRANSPARENTFLAG:
		{
			LONG flag = GetWindowLongPtr(hwndDlg,GWL_EXSTYLE);
			LONG oldFlag = flag;
			if (lParam == 2) 
				lParam = (g_CluiData.fDisableSkinEngine)?0:1;
			flag &= ~WS_EX_TRANSPARENT;
			if (lParam) flag |= WS_EX_TRANSPARENT;
			if (flag != oldFlag) {
				SetWindowLongPtr(hwndDlg,GWL_EXSTYLE,flag);
				RedrawWindow(hwndDlg,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
			}
		}
		return 0;

	case BM_GETIMAGE:
		if (wParam == IMAGE_ICON)
			return (LRESULT)(lpSBData->hIconPrivate ? lpSBData->hIconPrivate : lpSBData->hIcon);
		break;

	case BM_SETIMAGE:
		if ( !lParam)
			break;

		if (wParam == IMAGE_ICON) {
			ICONINFO ii = {0};
			BITMAP bm = {0};

			if (lpSBData->hIconPrivate) {
				DestroyIcon(lpSBData->hIconPrivate);
				lpSBData->hIconPrivate = 0;
			}

			GetIconInfo((HICON) lParam, &ii);
			GetObject(ii.hbmColor, sizeof(bm), &bm);
			if (bm.bmWidth > 16 || bm.bmHeight > 16) {
				HIMAGELIST hImageList;
				hImageList = ImageList_Create(16, 16, IsWinVerXPPlus() ? ILC_COLOR32 | ILC_MASK : ILC_COLOR16 | ILC_MASK, 1, 0);
				ImageList_AddIcon(hImageList, (HICON) lParam);
				lpSBData->hIconPrivate = ImageList_GetIcon(hImageList, 0, ILD_NORMAL);
				ImageList_RemoveAll(hImageList);
				ImageList_Destroy(hImageList);
				lpSBData->hIcon = 0;
			} 
			else {
				lpSBData->hIcon = (HICON) lParam;
				lpSBData->hIconPrivate = NULL;
			}

			DeleteObject(ii.hbmMask);
			DeleteObject(ii.hbmColor);
			InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
		}
		else if (wParam == IMAGE_BITMAP) 
		{
			if (lpSBData->hIconPrivate)
				DestroyIcon(lpSBData->hIconPrivate);
			lpSBData->hIcon = lpSBData->hIconPrivate = NULL;
			InvalidateParentRect(lpSBData->hwnd, NULL, TRUE);
			return 0; // not supported
		}
		break;
	}
	return 0;
}

void CustomizeButton(HWND hwnd)
{
	MButtonCustomize Custom;
	Custom.cbLen = sizeof(TBBUTTONDATA);
	Custom.fnPainter = (pfnPainterFunc)PaintWorker;
	Custom.fnWindowProc = ToolbarButtonProc;
	SendMessage(hwnd, BUTTONSETCUSTOM, 0, (LPARAM)&Custom);

	TBBUTTONDATA* p = (TBBUTTONDATA*)GetWindowLongPtr(hwnd, 0);
	p->nFontID = -1;
	p->hThemeButton = xpt_AddThemeHandle(p->hwnd, L"BUTTON");
	p->hThemeToolbar = xpt_AddThemeHandle(p->hwnd, L"TOOLBAR");
	WindowList_Add(hButtonWindowList, hwnd, NULL);				
}

/////////////////////////////////////////////////////////////////////////////////////////

int Buttons_OnSkinModeSettingsChanged(WPARAM wParam, LPARAM lParam)
{	
	WindowList_BroadcastAsync(hButtonWindowList, MBM_UPDATETRANSPARENTFLAG,0,2);
	return 0;
}

HRESULT ToolbarButtonLoadModule()
{
	hButtonWindowList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	hIconChangedHook = HookEvent(ME_SKIN2_ICONSCHANGED,OnIconLibIconChanged);
	hBkgChangedHook = HookEvent(ME_BACKGROUNDCONFIG_CHANGED,Buttons_OnSkinModeSettingsChanged);
	return S_OK;
}
