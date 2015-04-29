#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"
#include "m_skinbutton.h"
#include "hdr/modern_clcpaint.h"

#include <m_button_int.h>
#include <m_toptoolbar.h>

#ifdef __MINGW32__
#include <ctype.h>
#endif

#define BUTTON_POLLID       100
#define BUTTON_POLLDELAY    50
#define b2str(a) ((a) ? "True" : "False")

void CustomizeToolbar(HWND);

struct TBBUTTONDATA : public MButtonCtrl
{
	char  *szButtonID;      // button id
	bool   bHotMark;        // button is hot marked (e.g. current state)
	bool   bFocused;
	int    nFontID;         // internal font ID
	TCHAR  szText[128];     // text on the button
	RECT   rcMargins;       // margins of inner content

	HANDLE hIcolibHandle;   // handle of icon in iconlib

	XPTHANDLE hThemeButton, hThemeToolbar;
};

static CRITICAL_SECTION csTips;
static HWND hwndToolTips = NULL;
static BOOL	bThemed = FALSE;

static HANDLE hButtonWindowList = NULL;

static int OnIconLibIconChanged(WPARAM, LPARAM)
{
	WindowList_BroadcastAsync(hButtonWindowList, MBM_REFRESHICOLIBICON, 0, 0);
	return 0;
}

static void InvalidateParentRect(HWND hwndChild, RECT *lpRect, BOOL fErase)
{
	LONG lExStyle = GetWindowLongPtr(hwndChild, GWL_EXSTYLE);
	if (lExStyle & WS_EX_TRANSPARENT) {
		NMHDR hdr;
		hdr.hwndFrom = hwndChild;
		hdr.idFrom = 0;
		hdr.code = BUTTONNEEDREDRAW;
		SendMessage(GetParent(hwndChild), WM_NOTIFY, (WPARAM)hwndChild, (LPARAM)&hdr);
	}
	else InvalidateRect(hwndChild, lpRect, fErase);
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

static void PaintWorker(TBBUTTONDATA *bct, HDC hdcPaint, POINT *pOffset)
{
	if (!hdcPaint)
		return;  //early exit

	POINT offset;
	if (pOffset)
		offset = *pOffset;
	else
		offset.x = offset.y = 0;

	RECT rcClient;
	GetClientRect(bct->hwnd, &rcClient);
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;

	HBITMAP hbmMem = NULL;
	HBITMAP hbmOld = NULL;
	HDC hdcMem = pOffset ? hdcPaint : CreateCompatibleDC(hdcPaint);
	HFONT hOldFont = (HFONT)SelectObject(hdcMem, bct->hFont);
	if (!pOffset) {
		hbmMem = ske_CreateDIB32(width, height);
		hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
	}
	else OffsetRect(&rcClient, offset.x, offset.y);

	if (!g_CluiData.fDisableSkinEngine) {
		char szRequest[128];
		/* painting */
		mir_snprintf(szRequest, SIZEOF(szRequest), "Button,ID=%s,Hovered=%s,Pressed=%s,Focused=%s",
			bct->szButtonID,				// ID		
			b2str(bct->stateId == PBS_HOT),	// Hovered
			b2str(bct->stateId == PBS_PRESSED || bct->bIsPushed == TRUE),	// Pressed
			b2str(bct->bFocused));		// Focused

		SkinDrawGlyph(hdcMem, &rcClient, &rcClient, szRequest);
	}
	else if (xpt_IsThemed(bct->hThemeToolbar)) {
		RECT *rc = &rcClient;
		int state = IsWindowEnabled(bct->hwnd) ? /*(bct->stateId == PBS_PRESSED || bct->bIsPushed == TRUE) ? PBS_PRESSED :*/ (bct->stateId == PBS_NORMAL && bct->bIsDefault ? PBS_DEFAULTED : bct->stateId) : PBS_DISABLED;
		xpt_DrawTheme(bct->hThemeToolbar, bct->hwnd, hdcMem, TP_BUTTON, TBStateConvert2Flat(state), rc, rc);
	}
	else {
		HBRUSH hbr = NULL;

		if (bct->stateId == PBS_PRESSED || bct->stateId == PBS_HOT)
			hbr = GetSysColorBrush(COLOR_3DLIGHT);
		else {
			RECT btnRect;
			POINT pt = { 0 };
			int ret;
			HWND hwndParent = GetParent(bct->hwnd);
			HDC dc = CreateCompatibleDC(NULL);
			HBITMAP memBM, oldBM;
			GetWindowRect(hwndParent, &btnRect);
			memBM = ske_CreateDIB32(btnRect.right - btnRect.left, btnRect.bottom - btnRect.top);
			oldBM = (HBITMAP)SelectObject(dc, memBM);
			ret = SendMessage(hwndParent, WM_ERASEBKGND, (WPARAM)dc, 0);
			GetWindowRect(bct->hwnd, &btnRect);
			ClientToScreen(hwndParent, &pt);
			OffsetRect(&btnRect, -pt.x, -pt.y);
			if (ret)
				BitBlt(hdcMem, 0, 0, btnRect.right - btnRect.left, btnRect.bottom - btnRect.top, dc, btnRect.left, btnRect.top, SRCCOPY);
			oldBM = (HBITMAP)SelectObject(dc, oldBM);
			DeleteObject(memBM);
			DeleteDC(dc);
			if (!ret) { //WM_ERASEBKG return false need to paint
				HDC pdc = GetDC(hwndParent);
				HBRUSH oldBrush = (HBRUSH)GetCurrentObject(pdc, OBJ_BRUSH);
				hbr = (HBRUSH)SendMessage(hwndParent, WM_CTLCOLORDLG, (WPARAM)pdc, (LPARAM)hwndParent);
				SelectObject(pdc, oldBrush);
				ReleaseDC(hwndParent, pdc);
			}
		}
		if (hbr) {
			FillRect(hdcMem, &rcClient, hbr);
			DeleteObject(hbr);
		}
		if (bct->stateId == PBS_HOT || bct->bFocused) {
			if (bct->bIsPushed)
				DrawEdge(hdcMem, &rcClient, EDGE_ETCHED, BF_RECT | BF_SOFT);
			else
				DrawEdge(hdcMem, &rcClient, BDR_RAISEDOUTER, BF_RECT | BF_SOFT | BF_FLAT);
		}
		else if (bct->stateId == PBS_PRESSED)
			DrawEdge(hdcMem, &rcClient, BDR_SUNKENOUTER, BF_RECT | BF_SOFT);
	}

	RECT  rcTemp = rcClient;  //content rect
	bool  bPressed = (bct->stateId == PBS_PRESSED || bct->bIsPushed == TRUE);
	bool  bHasText = (bct->szText[0] != '\0');

	/* formatter */
	if (!g_CluiData.fDisableSkinEngine) {
		/* correct rect according to rcMargins */

		rcTemp.left += bct->rcMargins.left;
		rcTemp.top += bct->rcMargins.top;
		rcTemp.bottom -= bct->rcMargins.bottom;
		rcTemp.right -= bct->rcMargins.right;
	}

	/* reposition button items */
	RECT rcIcon = rcTemp, rcText = rcTemp;
	if (bct->hIcon) {
		if (bHasText) {
			rcIcon.right = rcIcon.left + 16; /* CXSM_ICON */
			rcText.left = rcIcon.right + 2;
		}
		else {
			rcIcon.left += (rcIcon.right - rcIcon.left) / 2 - 8;
			rcIcon.right = rcIcon.left + 16;
		}
	}

	/*	Check sizes*/
	if (bct->hIcon && (rcIcon.right > rcTemp.right || rcIcon.bottom > rcTemp.bottom || rcIcon.left < rcTemp.left || rcIcon.top < rcTemp.top))
		bct->hIcon = NULL;

	if (bHasText && (rcText.right>rcTemp.right || rcText.bottom>rcTemp.bottom || rcText.left < rcTemp.left || rcText.top < rcTemp.top))
		bHasText = FALSE;

	if (bct->hIcon) {
		/* center icon vertically */
		rcIcon.top += (rcClient.bottom - rcClient.top) / 2 - 8; /* CYSM_ICON/2 */
		rcIcon.bottom = rcIcon.top + 16; /* CYSM_ICON */
		/* draw it */
		ske_DrawIconEx(hdcMem, rcIcon.left + bPressed, rcIcon.top + bPressed, bct->hIcon, 16, 16, 0, NULL, DI_NORMAL);
	}

	if (bHasText) {
		BOOL bCentered = TRUE;
		SetBkMode(hdcMem, TRANSPARENT);
		if (bct->nFontID >= 0)
			g_clcPainter.ChangeToFont(hdcMem, NULL, bct->nFontID, NULL);

		RECT TextRequiredRect = rcText;
		ske_DrawText(hdcMem, bct->szText, -1, &TextRequiredRect, DT_CENTER | DT_VCENTER | DT_CALCRECT | DT_SINGLELINE);
		if (TextRequiredRect.right - TextRequiredRect.left > rcText.right - rcText.left)
			bCentered = FALSE;

		ske_DrawText(hdcMem, bct->szText, -1, &rcText, (bCentered ? DT_CENTER : 0) | DT_VCENTER | DT_SINGLELINE);
		ske_ResetTextEffect(hdcMem);
	}
	if (!pOffset)
		BitBlt(hdcPaint, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

	// better to use try/finally but looks like last one is Microsoft specific
	SelectObject(hdcMem, hOldFont);
	if (!pOffset) {
		SelectObject(hdcMem, hbmOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
	}
}

static LRESULT CALLBACK ToolbarButtonProc(HWND hwndDlg, UINT  msg, WPARAM wParam, LPARAM lParam)
{
	TBBUTTONDATA *bct = (TBBUTTONDATA*)GetWindowLongPtr(hwndDlg, 0);

	switch (msg) {
	case WM_DESTROY:
		xpt_FreeThemeForWindow(hwndDlg);
		WindowList_Remove(hButtonWindowList, hwndDlg);
		break;

	case WM_SETTEXT:
		_tcsncpy_s(bct->szText, (TCHAR*)lParam, _TRUNCATE);
		break;

	case WM_SETFONT:
		// remember the font so we can use it later
		bct->hFont = (HFONT)wParam; // maybe we should redraw?
		bct->nFontID = (int)lParam - 1;
		break;

	case BUTTONSETMARGINS:
		if (!lParam) {
			RECT nillRect = { 0 };
			bct->rcMargins = nillRect;
		}
		else bct->rcMargins = *(RECT*)lParam;
		break;

	case BUTTONSETID:
		bct->szButtonID = (char *)lParam;
		break;

	case BUTTONDRAWINPARENT:
		if (IsWindowVisible(hwndDlg)) {
			PaintWorker(bct, (HDC)wParam, (POINT*)lParam);
			return 0;
		}
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		if (g_CluiData.fDisableSkinEngine) {
			PAINTSTRUCT ps;
			HDC hdcPaint = BeginPaint(hwndDlg, &ps);
			if (hdcPaint) {
				PaintWorker(bct, hdcPaint, NULL);
				EndPaint(hwndDlg, &ps);
			}
		}
		ValidateRect(hwndDlg, NULL);
		return 0;

	case WM_CAPTURECHANGED:
		if ((HWND)lParam != bct->hwnd && bct->stateId != PBS_DISABLED) {
			// don't change states if disabled
			bct->stateId = PBS_NORMAL;
			InvalidateParentRect(bct->hwnd, NULL, TRUE);
		}
		break;

	case WM_MOUSELEAVE:
	case BUTTONSETASPUSHBTN:
		return 0;

	case WM_ENABLE: // windows tells us to enable/disable
		bct->stateId = wParam ? PBS_NORMAL : PBS_DISABLED;
		InvalidateParentRect(bct->hwnd, NULL, TRUE);
		return 0;

	case WM_LBUTTONDOWN:
	{
		POINT ptMouse = UNPACK_POINT(lParam);

		RECT rcClient;
		GetClientRect(bct->hwnd, &rcClient);
		if (!PtInRect(&rcClient, ptMouse)) {
			bct->bHotMark = false;
			ReleaseCapture();
		}
		else {
			if (bct->stateId != PBS_DISABLED && bct->stateId != PBS_PRESSED) {
				bct->stateId = PBS_PRESSED;
				bct->bHotMark = true;
				InvalidateParentRect(bct->hwnd, NULL, TRUE);
				if (bct->bSendOnDown) {
					SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM)hwndDlg);
					bct->stateId = PBS_NORMAL;
					InvalidateParentRect(bct->hwnd, NULL, TRUE);
				}
			}
			SetCapture(bct->hwnd);
		}
	}
	return 0;

	case WM_LBUTTONUP:
		if (GetCapture() == bct->hwnd) {
			POINT ptMouse = UNPACK_POINT(lParam);

			RECT rcClient;
			GetClientRect(bct->hwnd, &rcClient);

			if (!PtInRect(&rcClient, ptMouse)) {
				bct->bHotMark = false;
				ReleaseCapture();
				return 0;
			}

			if (bct->bIsPushBtn)
				bct->bIsPushed = !bct->bIsPushed;

			if (bct->stateId != PBS_DISABLED) {
				// don't change states if disabled
				bct->stateId = PBS_HOT;
				InvalidateParentRect(bct->hwnd, NULL, TRUE);
			}
			if (!bct->bSendOnDown) {
				bct->bHotMark = false;
				SendMessage(GetParent(hwndDlg), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndDlg), BN_CLICKED), (LPARAM)hwndDlg);
			}
		}
		else {
			bct->bHotMark = false;
			InvalidateParentRect(bct->hwnd, NULL, TRUE);
		}
		return 0;

	case WM_MOUSEMOVE:
	{
		BOOL bPressed = (wParam & MK_LBUTTON) != 0;
		if (bPressed && !bct->bHotMark)
			break;

		RECT rc;
		POINT pt;
		GetWindowRect(hwndDlg, &rc);
		GetCursorPos(&pt);
		BOOL inClient = PtInRect(&rc, pt);
		if (inClient) {
			SetCapture(bct->hwnd);
			if (bct->stateId == PBS_NORMAL) {
				bct->stateId = PBS_HOT;
				InvalidateParentRect(bct->hwnd, NULL, TRUE);
			}
		}

		if (!inClient && bct->stateId == PBS_PRESSED) {
			bct->stateId = PBS_HOT;
			InvalidateParentRect(bct->hwnd, NULL, TRUE);
		}
		else if (inClient && bct->stateId == PBS_HOT && bPressed) {
			if (bct->bHotMark) {
				bct->stateId = PBS_PRESSED;
				InvalidateParentRect(bct->hwnd, NULL, TRUE);
			}
		}
		else if (!inClient && !bPressed) {
			bct->bHotMark = false;
			ReleaseCapture();
		}
	}
	return 0;

	case WM_NCHITTEST:
	{
		LRESULT lr = SendMessage(GetParent(hwndDlg), WM_NCHITTEST, wParam, lParam);
		if (lr == HTLEFT || lr == HTRIGHT || lr == HTBOTTOM || lr == HTTOP || lr == HTTOPLEFT || lr == HTTOPRIGHT || lr == HTBOTTOMLEFT || lr == HTBOTTOMRIGHT)
			return HTTRANSPARENT;
	}
	break;

	case BM_SETCHECK:
		if (!bct->bIsPushBtn) break;
		if (wParam == BST_CHECKED)
			bct->bIsPushed = 1;
		else if (wParam == BST_UNCHECKED)
			bct->bIsPushed = 0;
		InvalidateRect(bct->hwnd, NULL, TRUE);
		return 0;

	case WM_SETFOCUS: // set keyboard focus and redraw
		bct->bFocused = true;
		InvalidateParentRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_KILLFOCUS: // kill focus and redraw
		bct->bFocused = false;
		InvalidateParentRect(bct->hwnd, NULL, TRUE);
		break;

	case WM_ERASEBKGND:
		return 1;

	case MBM_SETICOLIBHANDLE:
		bct->hIcolibHandle = (HANDLE)lParam;
		bct->hIcon = (bct->hIcolibHandle) ? Skin_GetIconByHandle(bct->hIcolibHandle) : NULL;
		return 1;

	case MBM_REFRESHICOLIBICON:
		if (bct->hIcolibHandle)
			bct->hIcon = Skin_GetIconByHandle(bct->hIcolibHandle);
		else
			bct->hIcon = NULL;
		InvalidateRect(hwndDlg, NULL, TRUE);
		pcli->pfnInvalidateRect(GetParent(GetParent(hwndDlg)), NULL, TRUE);
		return 1;

	case MBM_UPDATETRANSPARENTFLAG:
		LONG_PTR flag = GetWindowLongPtr(hwndDlg, GWL_EXSTYLE);
		LONG_PTR oldFlag = flag;
		if (lParam == 2)
			lParam = (g_CluiData.fDisableSkinEngine) ? 0 : 1;
		flag &= ~WS_EX_TRANSPARENT;
		if (lParam) flag |= WS_EX_TRANSPARENT;
		if (flag != oldFlag) {
			SetWindowLongPtr(hwndDlg, GWL_EXSTYLE, flag);
			RedrawWindow(hwndDlg, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		return 1;
	}

	LRESULT res = mir_callNextSubclass(hwndDlg, ToolbarButtonProc, msg, wParam, lParam);
	if (msg == BM_SETIMAGE)
		InvalidateParentRect(hwndDlg, NULL, TRUE);
	return res;
}

void MakeButtonSkinned(HWND hWnd)
{
	SendMessage(hWnd, BUTTONSETCUSTOMPAINT, sizeof(TBBUTTONDATA), (LPARAM)PaintWorker);
	mir_subclassWindow(hWnd, ToolbarButtonProc);

	TBBUTTONDATA* p = (TBBUTTONDATA*)GetWindowLongPtr(hWnd, 0);
	p->nFontID = -1;
	p->hThemeButton = xpt_AddThemeHandle(p->hwnd, L"BUTTON");
	p->hThemeToolbar = xpt_AddThemeHandle(p->hwnd, L"TOOLBAR");
	WindowList_Add(hButtonWindowList, hWnd, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CustomizeButton(HANDLE ttbid, HWND hWnd, LPARAM)
{
	if (ttbid == TTB_WINDOW_HANDLE) {
		CustomizeToolbar(hWnd);
		return;
	}

	MakeButtonSkinned(hWnd);

	TBBUTTONDATA *p = (TBBUTTONDATA*)GetWindowLongPtr(hWnd, 0);
	p->szButtonID = "Toolbar.MissingID";
	SendMessage(hWnd, MBM_UPDATETRANSPARENTFLAG, 0, 2);
}

/////////////////////////////////////////////////////////////////////////////////////////

int Buttons_OnSkinModeSettingsChanged(WPARAM, LPARAM)
{
	WindowList_BroadcastAsync(hButtonWindowList, MBM_UPDATETRANSPARENTFLAG, 0, 2);
	return 0;
}

HRESULT ToolbarButtonLoadModule()
{
	hButtonWindowList = WindowList_Create();
	HookEvent(ME_SKIN2_ICONSCHANGED, OnIconLibIconChanged);
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, Buttons_OnSkinModeSettingsChanged);
	return S_OK;
}

void ToolbarButtonUnloadModule()
{
	WindowList_Destroy(hButtonWindowList);
}