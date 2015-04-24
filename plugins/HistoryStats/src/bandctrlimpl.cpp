#include "stdafx.h"
#include "bandctrlimpl.h"

#include "main.h"
#include "resource.h"

/*
 * BandCtrlImpl
 */

const TCHAR* BandCtrlImpl::m_ClassName = _T("HistoryStatsBand");
const int BandCtrlImpl::m_PollId = 100;
const int BandCtrlImpl::m_PollDelay = 50;

LRESULT CALLBACK BandCtrlImpl::staticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BandCtrlImpl *pCtrl = reinterpret_cast<BandCtrlImpl*>(GetWindowLongPtr(hWnd, 0));

	switch (msg) {
	case WM_NCCREATE:
		pCtrl = new BandCtrlImpl(hWnd, reinterpret_cast<int>(reinterpret_cast<CREATESTRUCT*>(lParam)->hMenu));
		SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(pCtrl));
		return TRUE;

	case WM_DESTROY:
		delete pCtrl;
		SetWindowLongPtr(hWnd, 0, 0);
		return 0;

	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;

	case WM_SETFOCUS:
		pCtrl->onWMSetFocus();
		return 0;

	case WM_KILLFOCUS:
		if (pCtrl->m_nCurFocused != -1) {
			pCtrl->m_nCurFocused = -1;
			InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
		}
		return 0;

	case WM_ENABLE:
		InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
		return 0;

	case WM_GETFONT:
		return reinterpret_cast<LRESULT>(pCtrl->m_hFont);

	case WM_SETFONT:
		pCtrl->m_hFont = reinterpret_cast<HFONT>(wParam);
		return 0;

	case WM_WINDOWPOSCHANGED:
		pCtrl->recalcButtonRects();
		InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
		return 0;

	case WM_KEYDOWN:
		pCtrl->onWMKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		pCtrl->onWMKeyUp(wParam);
		return 0;

	case WM_MOUSEMOVE:
		pCtrl->onWMMouseMove(MAKEPOINTS(lParam));
		return 0;

	case WM_MOUSELEAVE:
		pCtrl->onWMMouseLeave();
		return 0;

	case WM_TIMER:
		if (wParam == m_PollId) {
			RECT rect;
			POINT pt;

			GetWindowRect(pCtrl->m_hWnd, &rect);
			GetCursorPos(&pt);

			if (!PtInRect(&rect, pt)) {
				PostMessage(pCtrl->m_hWnd, WM_MOUSELEAVE, 0, 0);
				KillTimer(pCtrl->m_hWnd, m_PollId);
			}
		}
		return 0;

	case WM_LBUTTONDOWN:
		pCtrl->onWMLButtonDown(MAKEPOINTS(lParam));
		return 0;

	case WM_LBUTTONUP:
		pCtrl->onWMLButtonUp(MAKEPOINTS(lParam));
		return 0;

	case WM_PAINT:
		pCtrl->onWMPaint();
		return 0;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_THEMECHANGED:
		pCtrl->reloadTheme();
		return 0;

	case BCM_ADDBUTTON:
		return pCtrl->onBCMAddButton(reinterpret_cast<BCBUTTON*>(lParam));

	case BCM_ISBUTTONCHECKED:
		assert(wParam >= 1 && wParam <= pCtrl->m_Items.size());
		return BOOL_(pCtrl->m_Items[wParam - 1].bChecked);

	case BCM_CHECKBUTTON:
		pCtrl->onBCMCheckButton(wParam - 1, bool_(lParam));
		return 0;

	case BCM_GETBUTTONDATA:
		assert(wParam >= 1 && wParam <= pCtrl->m_Items.size());
		return pCtrl->m_Items[wParam - 1].dwData;

	case BCM_SETBUTTONDATA:
		assert(wParam >= 1 && wParam <= pCtrl->m_Items.size());
		pCtrl->m_Items[wParam - 1].dwData = static_cast<INT_PTR>(lParam);
		return 0;

	case BCM_ISBUTTONVISIBLE:
		assert(wParam >= 1 && wParam <= pCtrl->m_Items.size());
		return BOOL_(pCtrl->m_Items[wParam - 1].bVisible);

	case BCM_SHOWBUTTON:
		pCtrl->onBCMShowButton(wParam - 1, bool_(lParam));
		return 0;

	case BCM_SETLAYOUT:
		assert(static_cast<int>(wParam) >= 0);
		pCtrl->m_nLayout = wParam;
		pCtrl->recalcButtonRects();
		InvalidateRect(pCtrl->m_hWnd, NULL, TRUE);
		return 0;

	case BCM_GETBUTTONRECT:
		pCtrl->onBCMGetButtonRect(wParam - 1, reinterpret_cast<RECT*>(lParam));
		return 0;

	case BCM_ISBUTTONENABLED:
		assert(wParam >= 1 && wParam <= pCtrl->m_Items.size());
		return BOOL_(pCtrl->m_Items[wParam - 1].bEnabled);

	case BCM_ENABLEBUTTON:
		pCtrl->onBCMEnableButton(wParam - 1, bool_(lParam));
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool BandCtrlImpl::registerClass()
{
	const WNDCLASSEX wcx = {
		sizeof(wcx),			// cbSize
		0,						// style
		staticWndProc,			// lpfnWndProc
		0,						// cbClsExtra
		sizeof(BandCtrlImpl*),	// cbWndExtra
		g_hInst,				// hInstance
		NULL,					// hIcon
		NULL,					// hCursor
		NULL,					// hbrBackground
		NULL,					// lpszMenuName
		m_ClassName,			// lpszClassName
		NULL					// hIconSm
	};

	if (!RegisterClassEx(&wcx))
		return false;

	return true;
}

void BandCtrlImpl::unregisterClass()
{
	UnregisterClass(m_ClassName, g_hInst);
}

BandCtrlImpl::BandCtrlImpl(HWND hWnd, int nOwnId) :
	m_hWnd(hWnd), m_nOwnId(nOwnId), m_hFont(NULL),
	m_hTheme(NULL), m_hImageList(NULL), m_hImageListD(NULL), m_hTooltip(NULL),
	m_nCurHot(-1), m_nCurFocused(-1), m_nCurPressed(-1), m_bCurPressedDD(false),
	m_nLayout(0), m_nDDWidth(12), m_hDDIcon(NULL)
{
	m_IconSize.cx = m_IconSize.cy;
	m_hDDIcon = reinterpret_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(IDI_DROPDOWN), IMAGE_ICON, OS::smIconCX(), OS::smIconCY(), 0));

	reloadTheme();
}

BandCtrlImpl::~BandCtrlImpl()
{
	if (m_hTooltip) {
		DestroyWindow(m_hTooltip);
		m_hTooltip = NULL;
	}

	if (m_hImageList) {
		ImageList_Destroy(m_hImageList);
		m_hImageList = NULL;
	}

	if (m_hImageListD) {
		ImageList_Destroy(m_hImageListD);
		m_hImageListD = NULL;
	}

	if (m_hTheme) {
		CloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

	if (m_hDDIcon) {
		DestroyIcon(m_hDDIcon);
		m_hDDIcon;
	}
}

void BandCtrlImpl::onWMPaint()
{
	// start painting
	PAINTSTRUCT ps;
	HDC hRealDC = BeginPaint(m_hWnd, &ps);
	if (hRealDC == NULL)
		return;

	// get rect for painting
	RECT rOut;
	GetClientRect(m_hWnd, &rOut);

	// setup memory DC for bufferd drawing
	HDC hDC = CreateCompatibleDC(hRealDC);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(hRealDC, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
	HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject(hDC, hMemBitmap));
	SetWindowOrgEx(hDC, ps.rcPaint.left, ps.rcPaint.top, NULL);

	// fill background
	bool bBandEnabled = bool_(IsWindowEnabled(m_hWnd));

	SetBkColor(hDC, GetSysColor(bBandEnabled ? COLOR_WINDOW : COLOR_BTNFACE));
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rOut, NULL, 0, NULL);

	// draw top and bottom line
	if (bBandEnabled) {
		RECT rLine = { rOut.left, rOut.top, rOut.right, rOut.top + 1 };

		SetBkColor(hDC, GetSysColor(COLOR_3DSHADOW));
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rLine, NULL, 0, NULL);

		rLine.top = (rLine.bottom = rOut.bottom) - 1;
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rLine, NULL, 0, NULL);
	}

	// draw items
	HGDIOBJ hOldFont = SelectObject(hDC, m_hFont);
	SIZE textSize;

	GetTextExtentPoint32(hDC, _T("X"), 1, &textSize);
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, GetSysColor(bBandEnabled ? COLOR_BTNTEXT : COLOR_GRAYTEXT));

	vector_each_(i, m_Items)
	{
		if (m_Items[i].bVisible) {
			drawButton(hDC, i, textSize.cy, bBandEnabled);
		}
	}

	SelectObject(hDC, hOldFont);

	// write back memory DC
	BitBlt(hRealDC, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, hDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	SelectObject(hDC, hOldBitmap);
	DeleteObject(hOldBitmap);
	DeleteDC(hDC);

	// end painting
	EndPaint(m_hWnd, &ps);
}

void BandCtrlImpl::drawButton(HDC hDC, int nItem, int textHeight, bool bBandEnabled)
{
	const ItemData &item = m_Items[nItem];

	bool bFocused = (nItem == m_nCurFocused);
	bool bHot = (nItem == m_nCurHot);
	bool bPressed = (nItem == m_nCurPressed);
	bool bEnabled = bBandEnabled && item.bEnabled;

	// MEMO: beautified keyboard focus, remove following two lines to get back ugly one
	bHot = bHot || bFocused;
	bFocused = false;

	RECT rItem = item.rItem;

	if (item.bDropDown) {
		RECT rDropDown = rItem;

		rDropDown.left = rDropDown.right - m_nDDWidth;
		rItem.right -= m_nDDWidth;

		if (m_hTheme) {
			int state = TS_DISABLED;

			if (bEnabled)
				state = bPressed ? (m_bCurPressedDD ? TS_PRESSED : TS_HOT) : (item.bChecked ? (bHot ? TS_HOTCHECKED : TS_CHECKED) : (bHot ? TS_HOT : TS_NORMAL));

			DrawThemeBackground(m_hTheme, hDC, TP_SPLITBUTTONDROPDOWN, state, &rDropDown, NULL);
		}
		else {
			--rDropDown.left;

			UINT state = 0;

			if (bEnabled)
				state = bPressed ? (m_bCurPressedDD ? DFCS_FLAT | DFCS_PUSHED : DFCS_FLAT) : (bHot ? DFCS_FLAT : (item.bChecked ? DFCS_FLAT | DFCS_CHECKED : 0));

			if (state != 0)
				DrawFrameControl(hDC, &rDropDown, DFC_BUTTON, DFCS_BUTTONPUSH | state);

			int x = rDropDown.left + (rDropDown.right - rDropDown.left - OS::smIconCX()) / 2;
			int y = rDropDown.top + (rDropDown.bottom - rDropDown.top - OS::smIconCY()) / 2;

			DrawState(hDC, NULL, NULL, reinterpret_cast<LPARAM>(m_hDDIcon), 0, x, y, m_IconSize.cx, m_IconSize.cy, DST_ICON | (bEnabled ? 0 : DSS_DISABLED));
		}
	}

	if (m_hTheme) {
		int state = TS_DISABLED;
		int part = item.bDropDown ? TP_SPLITBUTTON : TP_BUTTON;

		if (bEnabled)
			state = bPressed ? (!m_bCurPressedDD ? TS_PRESSED : TS_HOT) : (item.bChecked ? (bHot ? TS_HOTCHECKED : TS_CHECKED) : (bHot ? TS_HOT : TS_NORMAL));

		DrawThemeBackground(m_hTheme, hDC, part, state, &rItem, NULL);
	}
	else {
		UINT state = 0;

		if (bEnabled)
			state = bPressed ? (!m_bCurPressedDD ? DFCS_FLAT | DFCS_PUSHED : DFCS_FLAT) : (bHot ? DFCS_FLAT : (item.bChecked ? DFCS_FLAT | DFCS_CHECKED : 0));

		if (state != 0) {
			DrawFrameControl(hDC, &rItem, DFC_BUTTON, DFCS_BUTTONPUSH | state);
		}
	}

	InflateRect(&rItem, -3, -3);

	if (!item.text.empty()) {
		RECT rText = rItem;
		rText.top += (rItem.bottom - rItem.top + m_IconSize.cy - textHeight) / 2;
		rItem.bottom -= textHeight;
		DrawText(hDC, item.text.c_str(), -1, &rText, DT_TOP | DT_CENTER | DT_END_ELLIPSIS | DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}

	if (item.nIcon != -1) {
		int x = rItem.left + (rItem.right - rItem.left - m_IconSize.cx) / 2;
		int y = rItem.top + (rItem.bottom - rItem.top - m_IconSize.cy) / 2;

		if (bPressed && !m_bCurPressedDD) {
			++x;
			++y;
		}

		if (bEnabled)
			ImageList_Draw(m_hImageList, item.nIcon, hDC, x, y, ILD_NORMAL);
		else if (item.nIconD != -1)
			ImageList_Draw(m_hImageListD, item.nIconD, hDC, x, y, ILD_NORMAL);
		else {
			HICON hIcon = ImageList_GetIcon(m_hImageList, item.nIcon, 0);
			DrawState(hDC, NULL, NULL, reinterpret_cast<LPARAM>(hIcon), 0, x, y, m_IconSize.cx, m_IconSize.cy, DST_ICON | DSS_DISABLED);
			DestroyIcon(hIcon);
		}
	}

	if (bFocused) {
		rItem = item.rItem;

		InflateRect(&rItem, -2, -2);
		DrawFocusRect(hDC, &rItem);
	}
}

void BandCtrlImpl::reloadTheme()
{
	if (m_hTheme) {
		CloseThemeData(m_hTheme);
		m_hTheme = NULL;
	}

	m_nDDWidth = 12;

	m_hTheme = OpenThemeData(0, L"TOOLBAR");
	recalcButtonRects();
}

HICON BandCtrlImpl::convertToGray(HICON hIcon)
{
	// quick and dirty conversion to grayscale
	// preserves transparency
	// works only for 32bit icons

	HICON hIconDisabled = NULL;
	ICONINFO ii;

	if (!GetIconInfo(hIcon, &ii))
		return NULL;

	BITMAP bmp;
	if (GetObject(ii.hbmColor, sizeof(bmp), &bmp) && bmp.bmBitsPixel == 32) {
		int nSize = bmp.bmHeight * bmp.bmWidthBytes;
		BYTE* pBits = new BYTE[nSize];

		if (GetBitmapBits(ii.hbmColor, nSize, pBits)) {
			for (int y = 0; y < bmp.bmHeight; ++y) {
				BYTE* pLine = pBits + y * bmp.bmWidthBytes;

				for (int x = 0; x < bmp.bmWidth; ++x) {
					DWORD color = reinterpret_cast<DWORD*>(pLine)[x];
					BYTE gray = (77 * GetBValue(color) + 150 * GetGValue(color) + 28 * GetRValue(color)) / 255;

					color = (color & 0xFF000000) | RGB(gray, gray, gray);

					reinterpret_cast<DWORD*>(pLine)[x] = color;
				}
			}

			SetBitmapBits(ii.hbmColor, nSize, pBits);

			hIconDisabled = CreateIconIndirect(&ii);
		}

		delete[] pBits;
	}

	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);

	return hIconDisabled;
}

int BandCtrlImpl::onBCMAddButton(BCBUTTON *pButton)
{
	assert(pButton);

	m_Items.push_back(ItemData());

	ItemData &id = m_Items.back();
	id.bRight = bool_(pButton->dwFlags & BCF_RIGHT);
	id.bChecked = bool_(pButton->dwFlags & BCF_CHECKED);
	id.bVisible = !(pButton->dwFlags & BCF_HIDDEN);
	id.bDropDown = bool_(pButton->dwFlags & BCF_DROPDOWN);
	id.text = (pButton->dwFlags & BCF_TEXT) ? TranslateTS(pButton->m_szText) : _T("");
	id.tooltip = (pButton->dwFlags & BCF_TOOLTIP) ? TranslateTS(pButton->m_szTooltip) : _T("");
	id.uTTId = -1;
	id.dwData = (pButton->dwFlags & BCF_DATA) ? pButton->dwData : 0;
	id.bEnabled = !(pButton->dwFlags & BCF_DISABLED);
	id.nIcon = -1;
	id.nIconD = -1;

	if (pButton->dwFlags & BCF_ICON) {
		// create an image list, if needed
		if (!m_hImageList) {
			// guess image size from first inserted icon
			ICONINFO ii;

			if (GetIconInfo(pButton->hIcon, &ii)) {
				BITMAP bmp;

				if (GetObject(ii.hbmColor, sizeof(bmp), &bmp)) {
					m_IconSize.cx = bmp.bmWidth;
					m_IconSize.cy = bmp.bmHeight;
				}

				DeleteObject(ii.hbmColor);
				DeleteObject(ii.hbmMask);
			}

			m_hImageList = ImageList_Create(m_IconSize.cx, m_IconSize.cy, OS::imageListColor() | ILC_MASK, 5, 5);
		}

		// insert icon into image list
		id.nIcon = ImageList_AddIcon(m_hImageList, pButton->hIcon);

		// insert disabled icon into image list
		HICON hIconDisabled = convertToGray(pButton->hIcon);

		if (hIconDisabled) {
			if (!m_hImageListD) {
				m_hImageListD = ImageList_Create(m_IconSize.cx, m_IconSize.cy, OS::imageListColor() | ILC_MASK, 5, 5);
			}

			id.nIconD = ImageList_AddIcon(m_hImageListD, hIconDisabled);

			DestroyIcon(hIconDisabled);
		}
	}

	// atomatically adds tooltip, if needed
	recalcButtonRects();

	if (id.bVisible) {
		InvalidateRect(m_hWnd, &id.rItem, TRUE);
	}

	return m_Items.size();
}

void BandCtrlImpl::onBCMCheckButton(int nItem, bool bCheck)
{
	assert(nItem >= 0 && nItem < m_Items.size());

	ItemData &id = m_Items[nItem];
	if (bCheck != id.bChecked) {
		id.bChecked = bCheck;
		InvalidateRect(m_hWnd, &id.rItem, TRUE);
	}
}

void BandCtrlImpl::onBCMShowButton(int nItem, bool bShow)
{
	assert(nItem >= 0 && nItem < m_Items.size());

	ItemData &id = m_Items[nItem];
	if (bShow != id.bVisible) {
		id.bVisible = bShow;
		recalcButtonRects();
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

void BandCtrlImpl::onBCMGetButtonRect(int nItem, RECT* pRect)
{
	assert(nItem >= 0 && nItem < m_Items.size());
	assert(pRect);

	*pRect = m_Items[nItem].rItem;
}

void BandCtrlImpl::onBCMEnableButton(int nItem, bool bEnable)
{
	assert(nItem >= 0 && nItem < m_Items.size());

	ItemData &id = m_Items[nItem];

	if (bEnable != id.bEnabled) {
		id.bEnabled = bEnable;
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

void BandCtrlImpl::recalcButtonRects()
{
	RECT rOut;

	GetClientRect(m_hWnd, &rOut);
	InflateRect(&rOut, -2, -3);

	int itemHeight = rOut.bottom - rOut.top;
	int itemWidth = itemHeight;

	if (m_nLayout > 0) {
		itemWidth = (rOut.right - rOut.left) / m_nLayout;
	}

	RECT rItemL = { rOut.left, rOut.top, rOut.left + itemWidth, rOut.top + itemHeight };
	RECT rItemR = { rOut.right - itemWidth, rOut.top, rOut.right, rOut.top + itemHeight };

	vector_each_(i, m_Items)
	{
		if (m_Items[i].bVisible) {
			// visible: give it a rect and advance
			int nDDWidth = (m_Items[i].bDropDown && m_nLayout == 0) ? m_nDDWidth : 0;

			if (m_Items[i].bRight) {
				m_Items[i].rItem = rItemR;
				m_Items[i].rItem.left -= nDDWidth;
				OffsetRect(&rItemR, -(itemWidth + nDDWidth), 0);
			}
			else {
				m_Items[i].rItem = rItemL;
				m_Items[i].rItem.right += nDDWidth;
				OffsetRect(&rItemL, itemWidth + nDDWidth, 0);
			}
		}

		if (m_Items[i].uTTId != -1 && m_Items[i].bVisible) {
			// update tooltip rect, if we have a tooltip and are still visible
			TOOLINFO ti = {
				sizeof(TOOLINFO),									// cbSize
				TTF_SUBCLASS,										// uFlags
				m_hWnd,												// hwnd
				m_Items[i].uTTId,									// uId
				m_Items[i].rItem,									// rect
				NULL,												// hInstance
				const_cast<TCHAR*>(m_Items[i].tooltip.c_str()),	// lpszText
			};

			SendMessage(m_hTooltip, TTM_SETTOOLINFO, 0, reinterpret_cast<LPARAM>(&ti));
		}
		else if (m_Items[i].uTTId != -1 && !m_Items[i].bVisible) {
			// remove tooltip, if we have a tooltip but are no longer visible
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO);
			ti.hwnd = m_hWnd;
			ti.uId = m_Items[i].uTTId;
			SendMessage(m_hTooltip, TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(&ti));

			m_Items[i].uTTId = -1;
		}
		else if (m_Items[i].uTTId == -1 && m_Items[i].bVisible && !m_Items[i].tooltip.empty()) {
			// add a tooltip, if we don't have a tooltip but are now visible
			if (!m_hTooltip)
				m_hTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL, g_hInst, NULL);

			TOOLINFO ti = { sizeof(TOOLINFO), TTF_SUBCLASS, m_hWnd, i + 1, m_Items[i].rItem, NULL,
				const_cast<TCHAR*>(m_Items[i].tooltip.c_str()),	// lpszText
			};

			if (SendMessage(m_hTooltip, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti)))
				m_Items[i].uTTId = ti.uId;
		}
	}
}

int BandCtrlImpl::getNextButton(int nItem)
{
	if (nItem < 0 || nItem >= m_Items.size())
		nItem = -1;

	int nNext = nItem;
	int nLastLeft = -1;
	bool bLeft = !(nItem != -1 && m_Items[nItem].bRight);

	vector_each_(i, m_Items)
	{
		if (m_Items[i].bVisible && !m_Items[i].bRight)
			nLastLeft = i;
	}

	vector_each_(i, m_Items)
	{
		if (!m_Items[i].bVisible)
			continue;

		if (nItem == nLastLeft) {
			if (m_Items[i].bRight)
				nNext = i;
		}
		else if (!bLeft) {
			if (m_Items[i].bRight && i < nItem) {
				nNext = i;
				break;
			}
		}
		else {
			if (!m_Items[i].bRight && i > nNext) {
				nNext = i;
				break;
			}
		}
	}

	return nNext;
}

int BandCtrlImpl::getPrevButton(int nItem)
{
	if (nItem < 0 || nItem >= m_Items.size())
		nItem = -1;

	int nPrev = nItem;
	int nFirstRight = -1;
	bool bRight = (nItem != -1 && m_Items[nItem].bRight);

	vector_each_(i, m_Items)
	{
		if (m_Items[i].bVisible && m_Items[i].bRight)
			nFirstRight = i;
	}

	vector_each_(i, m_Items)
	{
		if (!m_Items[i].bVisible)
			continue;

		if (!bRight) {
			if (!m_Items[i].bRight && i < nItem)
				nPrev = i;
		}
		else if (nItem == nFirstRight) {
			if (!m_Items[i].bRight)
				nPrev = i;
		}
		else {
			if (m_Items[i].bRight && i > nPrev) {
				nPrev = i;
				break;
			}
		}
	}

	return nPrev;
}

void BandCtrlImpl::fireEvent(UINT code, int nItem)
{
	NMBANDCTRL nmbc;
	nmbc.hdr.code = code;
	nmbc.hdr.hwndFrom = m_hWnd;
	nmbc.hdr.idFrom = m_nOwnId;
	nmbc.hButton = reinterpret_cast<HANDLE>(nItem + 1);
	nmbc.dwData = m_Items[nItem].dwData;
	SendMessage(GetParent(m_hWnd), WM_NOTIFY, nmbc.hdr.idFrom, reinterpret_cast<LPARAM>(&nmbc));
}

void BandCtrlImpl::onWMSetFocus()
{
	m_nCurFocused = -1;
}

void BandCtrlImpl::onWMKeyDown(int nVirtKey)
{
	if (GetKeyState(VK_CONTROL) & ~1 || GetKeyState(VK_SHIFT) & ~1)
		return;

	if (nVirtKey == VK_RIGHT) {
		int nNext = getNextButton(m_nCurFocused);

		if (nNext != -1 && nNext != m_nCurFocused) {
			m_nCurFocused = nNext;
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
	}
	else if (nVirtKey == VK_LEFT) {
		int nPrev = getPrevButton(m_nCurFocused);

		if (nPrev != -1 && nPrev != m_nCurFocused) {
			m_nCurFocused = nPrev;
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
	}
	else if (nVirtKey == VK_SPACE) {
		if (m_nCurFocused != -1 && m_nCurFocused < m_Items.size() && m_Items[m_nCurFocused].bEnabled) {
			m_nCurPressed = m_nCurFocused;
			m_bCurPressedDD = false;
			InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);
		}
	}
	else if (nVirtKey == VK_DOWN) {
		if (m_nCurFocused != -1 && m_nCurFocused < m_Items.size() && m_Items[m_nCurFocused].bDropDown && m_Items[m_nCurFocused].bEnabled) {
			m_nCurPressed = m_nCurFocused;
			m_bCurPressedDD = true;
			InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);

			fireEvent(BCN_DROP_DOWN, m_nCurPressed);

			InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);
			m_nCurPressed = -1;
			m_bCurPressedDD = false;
		}
	}
}

void BandCtrlImpl::onWMKeyUp(int nVirtKey)
{
	if (GetKeyState(VK_CONTROL) & ~1 || GetKeyState(VK_SHIFT) & ~1)
		return;

	if (nVirtKey == VK_SPACE && m_nCurPressed != -1 && m_nCurPressed < m_Items.size()) {
		if (m_nCurFocused == m_nCurPressed)
			fireEvent(BCN_CLICKED, m_nCurPressed);

		InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);
		m_nCurPressed = -1;
		m_bCurPressedDD = false;
	}
}

void BandCtrlImpl::onWMMouseLeave()
{
	int nOldHot = m_nCurHot;

	m_nCurHot = -1;

	if (nOldHot != -1 && nOldHot < m_Items.size())
		InvalidateRect(m_hWnd, &m_Items[nOldHot].rItem, TRUE);
}

void BandCtrlImpl::onWMMouseMove(POINTS pts)
{
	POINT pt = { pts.x, pts.y };

	if (m_nCurHot != -1 && m_nCurHot < m_Items.size()) {
		if (!PtInRect(&m_Items[m_nCurHot].rItem, pt)) {
			InvalidateRect(m_hWnd, &m_Items[m_nCurHot].rItem, TRUE);
			m_nCurHot = -1;
		}
	}

	if (m_nCurHot == -1) {
		vector_each_(i, m_Items)
		{
			if (PtInRect(&m_Items[i].rItem, pt) && m_Items[i].bVisible) {
				m_nCurHot = i;
				InvalidateRect(m_hWnd, &m_Items[i].rItem, TRUE);
				break;
			}
		}
	}

	if (m_nCurHot != -1) {
		SetTimer(m_hWnd, m_PollId, m_PollDelay, NULL);
	}
}

void BandCtrlImpl::onWMLButtonDown(POINTS pts)
{
	POINT pt = { pts.x, pts.y };

	if (m_nCurHot != -1 && m_nCurHot < m_Items.size() && m_Items[m_nCurHot].bEnabled) {
		if (PtInRect(&m_Items[m_nCurHot].rItem, pt)) {
			m_nCurPressed = m_nCurHot;
			m_bCurPressedDD = false;
			InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);
			SetCapture(m_hWnd);

			if (m_Items[m_nCurHot].bDropDown) {
				RECT rDropDown = m_Items[m_nCurHot].rItem;

				rDropDown.left = rDropDown.right - m_nDDWidth;

				if (PtInRect(&rDropDown, pt)) {
					ReleaseCapture();
					m_bCurPressedDD = true;

					fireEvent(BCN_DROP_DOWN, m_nCurPressed);

					InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);
					m_nCurPressed = -1;
					m_bCurPressedDD = false;
				}
			}
		}
	}
}

void BandCtrlImpl::onWMLButtonUp(POINTS pts)
{
	POINT pt = { pts.x, pts.y };

	if (m_nCurPressed != -1 && m_nCurPressed < m_Items.size()) {
		ReleaseCapture();

		if (PtInRect(&m_Items[m_nCurPressed].rItem, pt))
			fireEvent(BCN_CLICKED, m_nCurPressed);

		InvalidateRect(m_hWnd, &m_Items[m_nCurPressed].rItem, TRUE);
		m_nCurPressed = -1;
		m_bCurPressedDD = false;
	}
}
