
#include "stdafx.h"

COLORREF bkColour;
HBITMAP hBmpBackground;
int backgroundBmpUse;

static wchar_t pluginname[] = L"TopToolBar";

/////////////////////////////////////////////////////////////////////////////////////////
// Toolbar window procedure

static void PaintToolbar(HWND hwnd)
{
	InvalidateRect(hwnd, nullptr, FALSE);

	PAINTSTRUCT paintst;
	HDC hdc = BeginPaint(hwnd, &paintst);
	RECT *rcPaint = &paintst.rcPaint;

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	int yScroll = 0;

	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBmpOsb = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), nullptr);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmpOsb);
	SetBkMode(hdcMem, TRANSPARENT);

	HBRUSH hBrush = CreateSolidBrush(bkColour);
	HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
	FillRect(hdcMem, rcPaint, hBrush);
	SelectObject(hdcMem, hoBrush);
	DeleteObject(hBrush);
	if (hBmpBackground) {
		BITMAP bmp;
		GetObject(hBmpBackground, sizeof(bmp), &bmp);

		HDC hdcBmp = CreateCompatibleDC(hdcMem);
		SelectObject(hdcBmp, hBmpBackground);
		int y = backgroundBmpUse & CLBF_SCROLL ? -yScroll : 0;
		int maxx = backgroundBmpUse & CLBF_TILEH ? clRect.right : 1;
		int maxy = backgroundBmpUse & CLBF_TILEV ? rcPaint->bottom : y+1;

		int destw, desth;
		switch (backgroundBmpUse & CLBM_TYPE) {
		case CLB_STRETCH:
			if (backgroundBmpUse & CLBF_PROPORTIONAL) {
				if (clRect.right * bmp.bmHeight < clRect.bottom * bmp.bmWidth) {
					desth = clRect.bottom;
					destw = desth * bmp.bmWidth / bmp.bmHeight;
				}
				else {
					destw = clRect.right;
					desth = destw * bmp.bmHeight / bmp.bmWidth;
				}
			}
			else {
				destw = clRect.right;
				desth = clRect.bottom;
			}
			break;
		case CLB_STRETCHH:
			if (backgroundBmpUse & CLBF_PROPORTIONAL) {
				destw = clRect.right;
				desth = destw * bmp.bmHeight / bmp.bmWidth;
				if (backgroundBmpUse & CLBF_TILEVTOROWHEIGHT)
					desth = g_ctrl->nButtonHeight + 2;
			}
			else {
				destw = clRect.right;
				desth = bmp.bmHeight;
			}
			break;
		case CLB_STRETCHV:
			if (backgroundBmpUse & CLBF_PROPORTIONAL) {
				desth = clRect.bottom;
				destw = desth * bmp.bmWidth / bmp.bmHeight;
			}
			else {
				destw = bmp.bmWidth;
				desth = clRect.bottom;
			}
			break;
		default:    //clb_topleft
			destw = bmp.bmWidth;
			desth = bmp.bmHeight;
			if (backgroundBmpUse & CLBF_TILEVTOROWHEIGHT)
				desth = g_ctrl->nButtonHeight + 2;
			break;
		}

		for (; y < maxy; y += desth) {
			if (y < rcPaint->top - desth)
				continue;

			for (int x = 0; x < maxx; x += destw)
				StretchBlt(hdcMem, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		}
		DeleteDC(hdcBmp);
	}
	BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);
	SelectObject(hdcMem, hOldBmp);
	DeleteDC(hdcMem);
	DeleteObject(hBmpOsb);

	paintst.fErase = FALSE;
	EndPaint(hwnd, &paintst);
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK TopToolBarProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool supressRepos = false;

	switch (msg) {
	case WM_CREATE:
		g_ctrl->hWnd = hwnd;
		PostMessage(hwnd, TTB_UPDATEFRAMEVISIBILITY, 0, 0);
		return FALSE;

	case WM_DESTROY:
		g_ctrl->hWnd = nullptr;
		break;

	case WM_MOVE:
		return 0;

	case WM_WINDOWPOSCHANGING:
	case WM_SIZE:
		if (g_ctrl->nLastHeight != HIWORD(lParam)) {
			g_plugin.setDword("LastHeight", g_ctrl->nLastHeight = HIWORD(lParam));
			ArrangeButtons();
		}
		if (supressRepos) {
			supressRepos = false;
			break;
		}
		__fallthrough;

	case TTB_REPOSBUTTONS:
		if (g_ctrl->hWnd == hwnd) {
			int iHeight = ArrangeButtons();
			if (g_ctrl->bAutoSize) {
				RECT rcClient;
				GetClientRect(g_ctrl->hWnd, &rcClient);
				rcClient.bottom -= rcClient.top;
				if (rcClient.bottom != iHeight && iHeight && rcClient.bottom) {
					supressRepos = true;
					PostMessage(hwnd, TTB_UPDATEFRAMEVISIBILITY, 0, 0);
				}
			}
			return 0;
		}
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		PaintToolbar(hwnd);
		return 0;

	case WM_LBUTTONDOWN:
		if (Clist::bClientAreaDrag) {
			POINT pt;
			GetCursorPos(&pt);
			return SendMessage(GetParent(hwnd), WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		return 0;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
		case BN_DOUBLECLICKED:
			{
				int id = GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
				if (id != 0) {
					mir_cslock lck(csButtonsHook);
					TopButtonInt *b = idtopos(id);
					if (b == nullptr || b->isSep())
						return 0;

					if (b->dwFlags & TTBBF_ASPUSHBUTTON)
						b->bPushed = !b->bPushed;

					if (b->bPushed) { //Dn -> Up
						if (!(b->dwFlags & TTBBF_ISLBUTTON)) // must be always true
							if (b->pszService != nullptr)
								CallService(b->pszService, b->wParamUp, b->lParamUp);
					}
					else { //Up -> Dn
						if (b->pszService != nullptr)
							CallService(b->pszService, b->wParamDown, b->lParamDown);
					}

					b->SetBitmap();
				}
			}
			break;
		}
		break;

	case TTB_UPDATEFRAMEVISIBILITY:
		{
			bool bResize = false;

			if (g_ctrl->bAutoSize) {
				int Height = ArrangeButtons();
				INT_PTR frameopt = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, g_ctrl->hFrame), 0);
				if (Height != 0 && Height != frameopt) {
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_HEIGHT, g_ctrl->hFrame), Height);
					bResize = TRUE;
				}
			}

			if (g_ctrl->bOrderChanged)
				bResize = TRUE, g_ctrl->bOrderChanged = FALSE;

			if (bResize)
				CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)g_ctrl->hFrame, FU_FMPOS);
		}
		break;

	case TTB_SETCUSTOMDATASIZE:
		g_ctrl = (TTBCtrl*)mir_realloc(g_ctrl, lParam);
		if (lParam > sizeof(TTBCtrl))
			memset(g_ctrl+1, 0, lParam - sizeof(TTBCtrl));

		SetWindowLongPtr(hwnd, 0, (LONG_PTR)g_ctrl);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK OnEventFire()
{
	HWND parent = g_clistApi.hwndContactList;
	if (parent == nullptr) // no clist, no buttons
		return;

	WNDCLASS wndclass = { 0 };
	wndclass.lpfnWndProc = TopToolBarProc;
	wndclass.cbWndExtra = sizeof(void *);
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszClassName = pluginname;
	RegisterClass(&wndclass);

	g_ctrl->pButtonList = (SortedList *)&g_arButtons;
	g_ctrl->hWnd = CreateWindow(pluginname, L"Toolbar",
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, 0, g_ctrl->nLastHeight, parent, nullptr, g_plugin.getInst(), nullptr);
	SetWindowLongPtr(g_ctrl->hWnd, 0, (LONG_PTR)g_ctrl);

	LoadBackgroundOptions();
	LoadAllSeparators();
	LoadAllLButs();

	// if we're working in skinned clist, receive the standard buttons & customizations
	if (g_CustomProc && g_ctrl->hWnd)
		g_CustomProc(TTB_WINDOW_HANDLE, g_ctrl->hWnd, g_CustomProcParam);
	else
		InitInternalButtons();

	// if there's no customized frames, create our own
	if (g_ctrl->hFrame == 0) {
		CLISTFrame Frame = { sizeof(Frame) };
		Frame.szName.a = "Toolbar";
		Frame.hWnd = g_ctrl->hWnd;
		Frame.align = alTop;
		Frame.Flags = F_VISIBLE | F_NOBORDER | F_LOCKED;
		Frame.height = g_ctrl->nLastHeight;
		Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
		g_ctrl->hFrame = g_plugin.addFrame(&Frame);
	}

	// receive buttons
	NotifyEventHooks(hTTBModuleLoaded, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadBackgroundOptions()
{
	bkColour = g_plugin.getDword("BkColour", TTBDEFAULT_BKCOLOUR);
	if (hBmpBackground) {
		DeleteObject(hBmpBackground);
		hBmpBackground = nullptr;
	}

	if (g_plugin.getByte("UseBitmap", TTBDEFAULT_USEBITMAP)) {
		ptrW tszBitmapName(g_plugin.getWStringA("BkBitmap"));
		if (tszBitmapName != NULL)
			hBmpBackground = Bitmap_Load(tszBitmapName);
	}
	backgroundBmpUse = g_plugin.getWord("BkBmpUse", TTBDEFAULT_BKBMPUSE);

	RECT rc;
	GetClientRect(g_ctrl->hWnd, &rc);
	InvalidateRect(g_ctrl->hWnd, &rc, TRUE);
	UpdateWindow(g_ctrl->hWnd);
	return 0;
}
