#include "headers.h"

#define WNDCLASSNAME _T("W7DwmWndClass")

CDwmWindow::CDwmWindow()
{
	GlobalInitWndClass();

	m_btnInitialized = false;
	m_btnCount = 0;
	for (int i = 0; i < SIZEOF(m_btnInfo); ++i)
	{
		m_btnInfo[i].iId = i;
		m_btnInfo[i].dwMask = THUMBBUTTONMASK(THB_FLAGS);
		m_btnInfo[i].dwFlags = THUMBBUTTONFLAGS(THBF_HIDDEN);
		m_btnData[i] = 0;
	}

	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW|WS_EX_NOACTIVATE, WNDCLASSNAME, NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInst, NULL);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
	SetWindowPos(m_hwnd, 0, -100000, -100000, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_DEFERERASE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW);

	BOOL val = 1;
	DwmSetWindowAttribute(m_hwnd, DWMWA_HAS_ICONIC_BITMAP, &val, 4);
	DwmSetWindowAttribute(m_hwnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &val, 4);
	dwmInvalidateIconicBitmaps(m_hwnd);
}

HBITMAP CDwmWindow::CreateDwmBitmap(int width, int height)
{
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = -height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	return CreateDIBSection(0, &bi, DIB_RGB_COLORS, NULL, 0, 0);
}

void CDwmWindow::MakeBitmapOpaque(HBITMAP hBmp)
{
	BITMAP bmp;
	GetObject(hBmp, sizeof(bmp), &bmp);
	if (bmp.bmBitsPixel != 32) return;
	if (bmp.bmHeight < 0) bmp.bmHeight *= -1;

	int size = bmp.bmWidth * bmp.bmHeight * 4;
	BYTE *data = new BYTE[size];
	GetBitmapBits(hBmp, size, data);
	for (int i = 3; i < size; i += 4)
		data[i] = 255;
	SetBitmapBits(hBmp, size, data);
	delete [] data;

}

void CDwmWindow::DrawGradient(HDC hdc, int x, int y, int width, int height, RGBQUAD *rgb0, RGBQUAD *rgb1)
{
	int oldMode			= SetBkMode(hdc, OPAQUE);
	COLORREF oldColor	= SetBkColor(hdc, 0);

	RECT rc; SetRect(&rc, x, 0, x+width, 0);
	for (int i=y+height; --i >= y; ) {
		COLORREF color = RGB(
			((height-i-1)*rgb0->rgbRed   + i*rgb1->rgbRed)   / height,
			((height-i-1)*rgb0->rgbGreen + i*rgb1->rgbGreen) / height,
			((height-i-1)*rgb0->rgbBlue  + i*rgb1->rgbBlue)  / height);
		rc.top = rc.bottom = i;
		++rc.bottom;
		SetBkColor(hdc, color);
		ExtTextOutA(hdc, 0, 0, ETO_OPAQUE, &rc, "", 0, 0);
	}

	SetBkMode(hdc, oldMode);
	SetBkColor(hdc, oldColor);
}

bool CDwmWindow::AddButton(HICON hIcon, TCHAR *text, INT_PTR data, DWORD flags)
{
	if (m_btnCount == SIZEOF(m_btnInfo)) return false;
	m_btnInfo[m_btnCount].dwMask = THUMBBUTTONMASK(THB_ICON|THB_TOOLTIP|THB_FLAGS);
	m_btnInfo[m_btnCount].hIcon = hIcon;
	lstrcpyn(m_btnInfo[m_btnCount].szTip, text, SIZEOF(m_btnInfo[m_btnCount].szTip));
	m_btnInfo[m_btnCount].dwFlags = THUMBBUTTONFLAGS(flags);
	m_btnData[m_btnCount] = data;
	m_btnCount++;
	return true;
}

void CDwmWindow::UpdateButtons(ITaskbarList3 *p)
{
	if (m_btnInitialized)
		p->ThumbBarUpdateButtons(hwnd(), SIZEOF(m_btnInfo), m_btnInfo);
	else
		p->ThumbBarAddButtons(hwnd(), SIZEOF(m_btnInfo), m_btnInfo);

	m_btnCount = 0; // reset this for next iteration
	m_btnInitialized = true;
}

void CDwmWindow::SetTimer(int id, int timeout)
{
	::SetTimer(m_hwnd, id, timeout, NULL);
}

void CDwmWindow::KillTimer(int id)
{
	::KillTimer(m_hwnd, id);
}

void CDwmWindow::InvalidateThumbnail()
{
	dwmInvalidateIconicBitmaps(m_hwnd);
}

void CDwmWindow::SetPreview(HBITMAP hbmp, int x, int y)
{
	POINT pt = { x, y };
	dwmSetIconicLivePreviewBitmap(m_hwnd, hbmp, &pt, 0);
}

void CDwmWindow::SetThumbnail(HBITMAP hbmp)
{
	dwmSetIconicThumbnail(m_hwnd, hbmp, 0);
}

LRESULT CDwmWindow::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				OnActivate((HWND)wParam);
			}
			break;
		}

		case WM_CLOSE:
		{
			OnClose();
			return FALSE;
		}

		case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
		{
			OnRenderPreview();
			break;
		}

		case WM_DWMSENDICONICTHUMBNAIL:
		{
			int width = HIWORD(lParam);
			int height = LOWORD(lParam);
			OnRenderThumbnail(width, height);
			break;
		}

		case WM_TIMER:
		{
			OnTimer(wParam);
			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD(wParam) == THBN_CLICKED)
				OnToolbar(LOWORD(wParam), m_btnData[LOWORD(wParam)]);
			break;
		}
	}

	return DefWindowProc(m_hwnd, msg, wParam, lParam);
}

void CDwmWindow::GlobalInitWndClass()
{
	static bool bInitialized = false;
	if (bInitialized) return;

	WNDCLASSEX wcl = {0};
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = GlobalWndProc;
	wcl.style = 0;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = g_hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = WNDCLASSNAME;
	wcl.hIconSm = NULL;
	RegisterClassEx(&wcl);
}

LRESULT CALLBACK CDwmWindow::GlobalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDwmWindow *wnd = (CDwmWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!wnd) return DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_DESTROY)
	{
		delete wnd;
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return wnd->WndProc(msg, wParam, lParam);
}