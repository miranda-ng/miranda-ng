// Tooltip.cpp: implementation of the CTooltip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

/*static*/ const TCHAR *CTooltip::s_szTooltipClass = _T("MimTooltipNotify");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTooltip::CTooltip(CTooltipNotify *pTooltipNotify)
{
	m_pTooltipNotify = pTooltipNotify;
	m_bLDblClick = DEF_SETTING_LDBLCLICK;
	m_hFont = 0;
	m_hWnd = 0;
	m_szText = 0;

	m_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, s_szTooltipClass, 0, 
							WS_POPUP|WS_BORDER, 100, 100, 50, 50, 0, 0, 
							g_hInstDLL, NULL);

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}


CTooltip::~CTooltip()
{
	if (m_hWnd) DestroyWindow(m_hWnd);
	if (m_hFont) DeleteObject(m_hFont);

	if (m_szText) free(m_szText);
}

/*static*/ void CTooltip::Initialize()
{
	WNDCLASSEX wcexWndClass = { 0 };
	wcexWndClass.cbSize = sizeof(WNDCLASSEX); 
	wcexWndClass.style = CS_SAVEBITS;
	wcexWndClass.lpfnWndProc = (WNDPROC)CTooltip::WindowProcWrapper;
	wcexWndClass.hInstance = g_hInstDLL;
	wcexWndClass.lpszClassName	= s_szTooltipClass;
	RegisterClassEx(&wcexWndClass);
}

/*static*/ void CTooltip::Deinitialize()
{
	UnregisterClass(s_szTooltipClass, g_hInstDLL);
}

LRESULT CALLBACK CTooltip::WindowProcWrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CTooltip* pThis = reinterpret_cast<CTooltip *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pThis->WindowProc(hWnd, message, wParam, lParam);
}



LRESULT CALLBACK CTooltip::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_CREATE:
			break;
		
		case WM_COMMAND:
			break;
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);

			RECT rect;
			GetClientRect(hWnd, &rect);
			
			rect.top += 1;
			rect.left += 1;
			rect.right -= 1;
			rect.bottom -= 1;

			SetBkMode(hDC, TRANSPARENT);
			SetTextColor(hDC, m_dwTextColor);
			SelectObject(hDC, m_hFont);
			DrawText(hDC, m_szText, lstrlen(m_szText), &rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

			EndPaint(hWnd, &ps);

			break;
		}

		case WM_ERASEBKGND:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			HBRUSH hBgBrush = CreateSolidBrush(m_dwBgColor);
			FillRect((HDC)wParam, &rect, hBgBrush);
			DeleteObject(hBgBrush);

			return TRUE;
		}

		case WM_NCHITTEST:
		{
			UINT uHitTest = DefWindowProc(hWnd, message, wParam, lParam);
			if(uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
		}

		case WM_NCLBUTTONDBLCLK:
		{
			m_pTooltipNotify->OnTooltipDblClicked(this);
			break;
		}

		case WM_SYSCOMMAND:
		{
			if (!m_pTooltipNotify->OnTooltipBeginMove(this))
				break;

			// doesn't return until the moving is complete
			DefWindowProc(hWnd, message, wParam, lParam);
			// DefWindowProc returned
			m_pTooltipNotify->OnTooltipEndMove(this);

			break;
		}

		case WM_DESTROY:
			break;
			
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}


void CTooltip::Validate()
{
	m_hFont = CreateFontIndirect(&m_lfFont);
	SIZE Size;
	HDC hDC = GetDC(m_hWnd);
	SelectObject(hDC, m_hFont);
	GetTextExtentPoint32(hDC, m_szText, lstrlen(m_szText), &Size);
	SetWindowPos(m_hWnd, 0, 0, 0, Size.cx+6, Size.cy+4, 
		SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);
	ReleaseDC(m_hWnd, hDC);
}


void CTooltip::Show()
{
	ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
}


void CTooltip::Hide()
{
	ShowWindow(m_hWnd, SW_HIDE);
}



void CTooltip::set_Translucency(BYTE bAlpha)
{
	if (SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED) != 0)
		SetLayeredWindowAttributes(m_hWnd, RGB(0,0,0), bAlpha, LWA_ALPHA);
}

void CTooltip::set_TransparentInput(BOOL bOnOff)
{
	if (bOnOff)
		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	else
		SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);		
}


void CTooltip::get_Rect(RECT *Rect) const
{
	GetWindowRect(m_hWnd, Rect);
}

void CTooltip::set_Position(INT x, INT y)
{
	SetWindowPos(m_hWnd, 0, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void CTooltip::set_Text(const TCHAR* szText)
{
	if (m_szText) free(m_szText);
	m_szText = _tcsdup(szText);
}

