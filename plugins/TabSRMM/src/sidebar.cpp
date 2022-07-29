/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// the contact switch bar on the left (or right) side

#include "stdafx.h"

TSideBarLayout CSideBar::m_layouts[CSideBar::NR_LAYOUTS] =
{
	{
		LPGENW("Like tabs, vertical text orientation"),
		26, 30,
		SIDEBARLAYOUT_DYNHEIGHT | SIDEBARLAYOUT_VERTICALORIENTATION,
		CSideBar::m_DefaultContentRenderer,
		CSideBar::m_DefaultBackgroundRenderer,
		nullptr,
		nullptr,
		SIDEBARLAYOUT_VERTICAL
	},
	{
		LPGENW("Compact layout, horizontal buttons"),
		100, 24,
		0,
		CSideBar::m_DefaultContentRenderer,
		CSideBar::m_DefaultBackgroundRenderer,
		nullptr,
		nullptr,
		SIDEBARLAYOUT_NORMAL
	},
	{
		LPGENW("Advanced layout with avatars"),
		140, 40,
		SIDEBARLAYOUT_NOCLOSEBUTTONS,
		CSideBar::m_AdvancedContentRenderer,
		CSideBar::m_DefaultBackgroundRenderer,
		nullptr,
		nullptr,
		SIDEBARLAYOUT_NORMAL
	},
	{
		LPGENW("Advanced with avatars, vertical orientation"),
		40, 40,
		SIDEBARLAYOUT_DYNHEIGHT | SIDEBARLAYOUT_VERTICALORIENTATION | SIDEBARLAYOUT_NOCLOSEBUTTONS,
		CSideBar::m_AdvancedContentRenderer,
		CSideBar::m_DefaultBackgroundRenderer,
		CSideBar::m_measureAdvancedVertical,
		nullptr,
		SIDEBARLAYOUT_VERTICAL
	}
};

CSideBarButton::CSideBarButton(CMsgDialog *dat, CSideBar *sideBar)
{
	m_dat = dat;
	m_id = UINT(dat->m_hContact);  // set the control id
	m_sideBar = sideBar;
	_create();
}

CSideBarButton::CSideBarButton(const UINT id, CSideBar *sideBar)
{
	m_dat = nullptr;
	m_id = id;
	m_sideBar = sideBar;
	_create();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Internal method to create the button item and configure the associated button control

void CSideBarButton::_create()
{
	m_hwnd = nullptr;
	m_isTopAligned = true;
	m_sz.cx = m_sz.cy = 0;

	m_hwnd = ::CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, 40, 40, m_sideBar->getScrollWnd(), reinterpret_cast<HMENU>(m_id), g_plugin.getInst(), nullptr);
	if (m_hwnd) {
		CustomizeButton(m_hwnd);
		::SendMessage(m_hwnd, BUTTONSETASSIDEBARBUTTON, (WPARAM)this, 0);
		::SendMessage(m_hwnd, BUTTONSETASFLATBTN, FALSE, 0);
		::SendMessage(m_hwnd, BUTTONSETASTHEMEDBTN, TRUE, 0);
		::SendMessage(m_hwnd, BUTTONSETCONTAINER, (LPARAM)m_sideBar->getContainer(), 0);
		m_buttonControl = (TSButtonCtrl *)::GetWindowLongPtr(m_hwnd, 0);
		if (m_id == IDC_SIDEBARUP || m_id == IDC_SIDEBARDOWN)
			::SetParent(m_hwnd, m_sideBar->getContainer()->m_hwnd);
	}
	else delete this;
}

CSideBarButton::~CSideBarButton()
{
	if (m_hwnd) {
		::SendMessage(m_hwnd, BUTTONSETASSIDEBARBUTTON, 0, 0);		// make sure, the button will no longer call us back
		::DestroyWindow(m_hwnd);
	}
}

void CSideBarButton::Show(const int showCmd) const
{
	::ShowWindow(m_hwnd, showCmd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Measure the metrics for the current item. The side bar layouting will call this
// whenever a layout with a dynamic height is active). For fixed dimension layouts,
// m_elementWidth and m_elementHeight will be used.
//
// @return SIZE&: reference to the item's size member. The caller may use cx and cy values
// to determine further layouting actions.

const SIZE& CSideBarButton::measureItem()
{
	if (m_sideBarLayout->pfnMeasureItem)
		m_sideBarLayout->pfnMeasureItem(this);        // use the current layout's function, if available, else use default
	else {
		HDC dc = ::GetDC(m_hwnd);
		HFONT oldFont = reinterpret_cast<HFONT>(::SelectObject(dc, ::GetStockObject(DEFAULT_GUI_FONT)));

		wchar_t tszLabel[255];
		wcsncpy_s(tszLabel, m_dat->m_wszTitle, _TRUNCATE);

		SIZE sz;
		::GetTextExtentPoint32(dc, tszLabel, (int)mir_wstrlen(tszLabel), &sz);
		sz.cx += 28;
		if (m_dat->m_pContainer->cfg.flagsEx.m_bTabCloseButton)
			sz.cx += 20;

		if (m_sideBarLayout->dwFlags & CSideBar::SIDEBARLAYOUT_VERTICALORIENTATION)
			m_sz.cy = sz.cx;
		else
			m_sz.cx = sz.cx;

		::SelectObject(dc, oldFont);
		::ReleaseDC(m_hwnd, dc);
	}
	return m_sz;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Render the button item. Callback from the button window procedure
//
// @param ctl    TSButtonCtrl *: pointer to the private button data structure
// @param hdc    HDC: device context for painting

void CSideBarButton::RenderThis(const HDC hdc) const
{
	RECT rc;
	::GetClientRect(m_hwnd, &rc);

	bool bVertical = (m_sideBarLayout->dwFlags & CSideBar::SIDEBARLAYOUT_VERTICALORIENTATION) ? true : false;
	if (m_id == IDC_SIDEBARUP || m_id == IDC_SIDEBARDOWN)
		bVertical = false;

	int cx, cy;
	if (bVertical) {
		cx = rc.bottom;
		cy = rc.right;
	}
	else {
		cx = rc.right;
		cy = rc.bottom;
	}

	HDC hdcMem = ::CreateCompatibleDC(hdc);

	HBITMAP hbmMem;
	if (bVertical) {
		RECT	rcFlipped = { 0, 0, cx, cy };
		hbmMem = CSkin::CreateAeroCompatibleBitmap(rcFlipped, hdcMem);
		rc = rcFlipped;
	}
	else hbmMem = CSkin::CreateAeroCompatibleBitmap(rc, hdcMem);

	HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(hdcMem, hbmMem));
	HFONT hFontOld = reinterpret_cast<HFONT>(::SelectObject(hdcMem, ::GetStockObject(DEFAULT_GUI_FONT)));

	m_sideBarLayout->pfnBackgroundRenderer(hdcMem, &rc, this);
	m_sideBarLayout->pfnContentRenderer(hdcMem, &rc, this);

	::SelectObject(hdcMem, hFontOld);

	// for vertical tabs, we did draw to a rotated rectangle, so we now must rotate the
	// final bitmap back to it's original orientation
	if (bVertical) {
		::SelectObject(hdcMem, hbmOld);

		FIBITMAP *fib = FreeImage_CreateDIBFromHBITMAP(hbmMem);
		FIBITMAP *fib_new = FreeImage_Rotate(fib, 90.0f, nullptr);
		FreeImage_Unload(fib);
		::DeleteObject(hbmMem);
		hbmMem = FreeImage_CreateHBITMAPFromDIB(fib_new);
		FreeImage_Unload(fib_new);

		hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(hdcMem, hbmMem));
		::BitBlt(hdc, 0, 0, cy, cx, hdcMem, 0, 0, SRCCOPY);
		::SelectObject(hdcMem, hbmOld);
		::DeleteObject(hbmMem);
		::DeleteDC(hdcMem);
	}
	else {
		::BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
		::SelectObject(hdcMem, hbmOld);
		::DeleteObject(hbmMem);
		::DeleteDC(hdcMem);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// render basic button content like nickname and icon. Used for the basic layouts
// only. Pretty much the same code as used for the tabs.
//
// @param hdc	 : target device context
// @param rcItem : rectangle to render into

void CSideBarButton::renderIconAndNick(const HDC hdc, const RECT *rcItem) const
{
	const TContainerData *pContainer = m_sideBar->getContainer();
	if (!m_dat || !pContainer)
		return;

	int   iSize = 16;
	HICON	hIcon = m_dat->m_cache->getIcon(iSize);
	RECT	rc = *rcItem;

	if (!m_dat->m_bCanFlashTab || (m_dat->m_bCanFlashTab == TRUE && m_dat->m_bTabFlash) || !pContainer->cfg.flagsEx.m_bTabFlashIcon) {
		uint32_t ix = rc.left + 4;
		uint32_t iy = (rc.bottom + rc.top - iSize) / 2;
		if (m_dat->m_bIsIdle && PluginConfig.m_bIdleDetect)
			CSkin::DrawDimmedIcon(hdc, ix, iy, iSize, iSize, hIcon, 180);
		else
			::DrawIconEx(hdc, ix, iy, hIcon, iSize, iSize, 0, nullptr, DI_NORMAL | DI_COMPAT);
	}

	rc.left += (iSize + 7);

	// draw the close button if enabled
	if (m_sideBar->getContainer()->cfg.flagsEx.m_bTabCloseButton) {
		if (m_sideBar->getHoveredClose() != this)
			CSkin::m_default_bf.SourceConstantAlpha = 150;

		GdiAlphaBlend(hdc, rc.right - 20, (rc.bottom + rc.top - 16) / 2, 16, 16, CSkin::m_tabCloseHDC, 0, 0, 16, 16, CSkin::m_default_bf);

		rc.right -= 19;
		CSkin::m_default_bf.SourceConstantAlpha = 255;
	}

	::SetBkMode(hdc, TRANSPARENT);

	if (!m_dat->m_bCanFlashTab || (m_dat->m_bCanFlashTab == TRUE && m_dat->m_bTabFlash) || !pContainer->cfg.flagsEx.m_bTabFlashLabel) {
		bool fIsActive = (m_sideBar->getActiveItem() == this);
		
		COLORREF clr;
		int stateId = m_buttonControl->stateId;
		if (fIsActive || stateId == PBS_PRESSED)
			clr = PluginConfig.tabConfig.colors[1];
		else if (stateId == PBS_HOT)
			clr = PluginConfig.tabConfig.colors[3];
		else
			clr = PluginConfig.tabConfig.colors[0];

		uint32_t	dwTextFlags = DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS;
		CSkin::RenderText(hdc, m_buttonControl->hThemeButton, m_dat->m_wszTitle, &rc, dwTextFlags, CSkin::m_glowSize, clr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// test if we have the mouse pointer over our close button.
// @return: 1 if the pointer is inside the button's rect, -1 otherwise

int CSideBarButton::testCloseButton() const
{
	if (m_id == IDC_SIDEBARUP || m_id == IDC_SIDEBARDOWN)							// scroller buttons don't have a close button
		return -1;

	if (m_sideBar->getContainer()->cfg.flagsEx.m_bTabCloseButton && !(getLayout()->dwFlags & CSideBar::SIDEBARLAYOUT_NOCLOSEBUTTONS)) {
		POINT pt;
		::GetCursorPos(&pt);
		::ScreenToClient(m_hwnd, &pt);
		RECT rc;

		::GetClientRect(m_hwnd, &rc);
		if (getLayout()->dwFlags & CSideBar::SIDEBARLAYOUT_VERTICALORIENTATION) {
			rc.bottom = rc.top + 18; rc.top += 2;
			rc.left += 2; rc.right -= 2;
			if (::PtInRect(&rc, pt))
				return 1;
		}
		else {
			rc.bottom -= 4; rc.top += 4;
			rc.right -= 3; rc.left = rc.right - 16;
			if (::PtInRect(&rc, pt))
				return 1;
		}
	}
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// call back from the button window procedure. Activate my session

void CSideBarButton::activateSession() const
{
	if (m_dat)
		m_dat->ActivateTab();
}

/////////////////////////////////////////////////////////////////////////////////////////
// show the context menu (same as on tabs

void CSideBarButton::invokeContextMenu()
{
	const TContainerData *pContainer = m_sideBar->getContainer();

	if (pContainer) {
		TSideBarNotify tsn = {};
		tsn.nmHdr.code = NM_RCLICK;
		tsn.nmHdr.idFrom = 5000;
		tsn.nmHdr.hwndFrom = ::GetDlgItem(pContainer->m_hwnd, 5000);
		tsn.dat = m_dat;
		::SendMessage(pContainer->m_hwnd, WM_NOTIFY, 0, LPARAM(&tsn));
	}
}

CSideBar::CSideBar(TContainerData *pContainer) :
	m_buttonlist(1, PtrKeySortT)
{
	m_pContainer = pContainer;
	m_up = m_down = nullptr;
	m_hwndScrollWnd = nullptr;
	m_activeItem = nullptr;
	m_isVisible = true;

	Init();
}

CSideBar::~CSideBar()
{
	destroyScroller();

	if (m_hwndScrollWnd)
		::DestroyWindow(m_hwndScrollWnd);
}

void CSideBar::Init()
{
	m_iTopButtons = m_iBottomButtons = 0;
	m_topHeight = m_bottomHeight = 0;
	m_firstVisibleOffset = 0;
	m_totalItemHeight = 0;

	m_uLayout = (m_pContainer->cfg.flagsEx.dw & 0xff000000) >> 24;
	m_uLayout = ((m_uLayout < NR_LAYOUTS) ? m_uLayout : 0);

	m_currentLayout = &m_layouts[m_uLayout];

	m_dwFlags = m_currentLayout->dwFlags;

	m_dwFlags = (m_pContainer->cfg.flagsEx.m_bTabSBarLeft ? m_dwFlags | SIDEBARORIENTATION_LEFT : m_dwFlags & ~SIDEBARORIENTATION_LEFT);
	m_dwFlags = (m_pContainer->cfg.flagsEx.m_bTabSBarRight ? m_dwFlags | SIDEBARORIENTATION_RIGHT : m_dwFlags & ~SIDEBARORIENTATION_RIGHT);

	if (m_pContainer->cfg.flags.m_bSideBar) {
		if (m_hwndScrollWnd == nullptr)
			m_hwndScrollWnd = ::CreateWindowEx(0, L"TS_SideBarClass", L"", WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD,
			0, 0, m_width, 40, m_pContainer->m_hwnd, reinterpret_cast<HMENU>(5000), g_plugin.getInst(), this);

		m_isActive = m_isVisible = true;
		createScroller();
		m_elementHeight = m_currentLayout->height;
		m_elementWidth = m_currentLayout->width;
		m_width = m_elementWidth + 4;
		populateAll();
		if (m_activeItem)
			setActiveItem(m_activeItem, true);
	}
	else {
		destroyScroller();
		m_width = 0;
		m_isActive = m_isVisible = false;
		m_activeItem = nullptr;

		removeAll();
		if (m_hwndScrollWnd)
			::DestroyWindow(m_hwndScrollWnd);
		m_hwndScrollWnd = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// sets visibility status for the sidebar. An invisible sidebar (collapsed
// by the button in the message dialog) will remain active, it will, however
// not do any drawing or other things that are only visually important.
//
// @param fNewVisible : set the new visibility status

void CSideBar::setVisible(bool fNewVisible)
{
	m_isVisible = fNewVisible;

	// only needed on hiding. Layout() will do it when showing it
	if (!m_isVisible)
		showAll(SW_HIDE);
	else {
		m_up->Show(SW_SHOW);
		m_down->Show(SW_SHOW);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Create both scrollbar buttons which can be used to scroll the switchbar
// up and down.

void CSideBar::createScroller()
{
	if (m_up == nullptr)
		m_up = new CSideBarButton(IDC_SIDEBARUP, this);
	if (m_down == nullptr)
		m_down = new CSideBarButton(IDC_SIDEBARDOWN, this);

	m_up->setLayout(m_currentLayout);
	m_down->setLayout(m_currentLayout);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Destroy the scroller buttons.

void CSideBar::destroyScroller()
{
	if (m_up) {
		delete m_up;
		m_up = nullptr;
	}
	if (m_down) {
		delete m_down;
		m_down = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// remove all buttons from the current list
// Does not remove the sessions. This is basically only used when switching
// from a sidebar to a tabbed interface

void CSideBar::removeAll()
{
	m_buttonlist.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// popuplate the side bar with all sessions inside the current window. Information
// is gathered from the tab control, which remains active (but invisible) when the
// switch bar is in use.
//
// This is needed when the user switches from tabs to a switchbar layout while a
// window is open.

void CSideBar::populateAll()
{
	int iItems = (int)TabCtrl_GetItemCount(m_pContainer->m_hwndTabs);

	m_iTopButtons = 0;

	for (int i = 0; i < iItems; i++) {
		HWND hDlg = GetTabWindow(m_pContainer->m_hwndTabs, i);
		if (hDlg == 0 || !IsWindow(hDlg))
			continue;

		CMsgDialog *dat = (CMsgDialog *)::GetWindowLongPtr(hDlg, GWLP_USERDATA);
		if (dat == nullptr)
			continue;

		CSideBarButton *b_item = findSession(dat);
		if (b_item == nullptr)
			addSession(dat, i);
		else {
			b_item->setLayout(m_currentLayout);
			if (m_dwFlags & SIDEBARLAYOUT_VERTICALORIENTATION) {
				b_item->measureItem();
				m_topHeight += b_item->getHeight() + 1;
			}
			else m_topHeight += m_elementHeight + 1;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Add a new session to the switchbar.
//
// @param dat    _MessageWindowData *: session data for a client session. Must be fully initialized
//    (that is, it can only be used after WM_INITIALOG completed).
// position: -1 = append, otherwise insert it at the given position

void CSideBar::addSession(CMsgDialog *dat, int position)
{
	if (!m_isActive)
		return;

	CSideBarButton *item = new CSideBarButton(dat, this);

	item->setLayout(m_currentLayout);

	if (m_dwFlags & SIDEBARLAYOUT_DYNHEIGHT) {
		SIZE sz = item->measureItem();
		m_topHeight += (sz.cy + 1);
	}
	else
		m_topHeight += (m_elementHeight + 1);

	m_iTopButtons++;
	if (position == -1 || position >= m_buttonlist.getCount())
		m_buttonlist.insert(item);
	else
		m_buttonlist.insert(item, position);

	SendDlgItemMessage(dat->GetHwnd(), IDC_TOGGLESIDEBAR, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)(m_dwFlags & SIDEBARORIENTATION_LEFT ? PluginConfig.g_buttonBarIcons[ICON_DEFAULT_LEFT] : PluginConfig.g_buttonBarIcons[ICON_DEFAULT_RIGHT]));

	moveButtons();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Remove a new session from the switchbar.
//
// @param dat    _MessageWindowData *: session data for a client session.

HRESULT CSideBar::removeSession(CMsgDialog *dat)
{
	if (dat) {
		CSideBarButton *item = findSession(dat);

		if (item != nullptr) {
			m_iTopButtons--;
			if (m_dwFlags & SIDEBARLAYOUT_DYNHEIGHT) {
				SIZE sz = item->getSize();
				m_topHeight -= (sz.cy + 1);
			}
			else m_topHeight -= (m_elementHeight + 1);

			m_buttonlist.remove(item);
			moveButtons();
			return(S_OK);
		}
	}
	return(S_FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// make sure the given item is visible in a scrolled switch bar
//
// @param item   CSideBarButtonItem*: the item which must be visible in the switch bar

void CSideBar::scrollIntoView(const CSideBarButton *item)
{
	LONG spaceUsed = 0, itemHeight = 0;
	bool fNeedLayout = false, bFound = false;

	if (!m_isActive)
		return;

	if (item == nullptr)
		item = m_activeItem;

	for (auto &it : m_buttonlist) {
		itemHeight = it->getHeight();
		spaceUsed += itemHeight;
		if (it == item) {
			bFound = true;
			break;
		}
	}

	RECT rc;
	GetClientRect(m_hwndScrollWnd, &rc);
	if (m_topHeight <= rc.bottom)
		m_firstVisibleOffset = 0;

	else if (!bFound || (item == &m_buttonlist[0] && m_firstVisibleOffset == 0))
		; // do nothing for the first item and .end() should not really happen

	else if (spaceUsed <= rc.bottom && spaceUsed - (itemHeight + 1) >= m_firstVisibleOffset)
		; // item fully visible, do nothing

	else { // button partially or not at all visible at the top
		if (spaceUsed < m_firstVisibleOffset || spaceUsed - (itemHeight + 1) < m_firstVisibleOffset) {
			m_firstVisibleOffset = spaceUsed - (itemHeight + 1);
			fNeedLayout = true;
		}
		else {
			if (spaceUsed > rc.bottom) { // partially or not at all visible at the bottom
				fNeedLayout = true;
				m_firstVisibleOffset = spaceUsed - rc.bottom;
			}
		}
	}

	moveButtons();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invalidate the button associated with the given session.
//
// @param dat    _MessageWindowData*: Session data

void CSideBar::updateSession(CMsgDialog *dat)
{
	if (!m_isVisible || !m_isActive)
		return;

	CSideBarButton *item = findSession(dat);
	if (item == nullptr)
		return;

	if (m_dwFlags & SIDEBARLAYOUT_DYNHEIGHT) {
		LONG oldHeight = item->getHeight();
		m_topHeight -= (oldHeight + 1);
		SIZE sz = item->measureItem();
		m_topHeight += (sz.cy + 1);
		if (sz.cy != oldHeight) {
			moveButtons();
			::InvalidateRect(item->getHwnd(), nullptr, TRUE);
		}
		else ::InvalidateRect(item->getHwnd(), nullptr, FALSE);
	}
	else ::InvalidateRect(item->getHwnd(), nullptr, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sets the active session item
// called from the global update handler in msgdialog/group room window
// on every tab activation to ensure consistency
//
// @param dat    _MessageWindowData*: session data
//
// @return The previously active item (that can be zero)

void CSideBar::setActiveItem(const CMsgDialog *dat, bool bScroll)
{
	if (auto *item = findSession(dat))
		setActiveItem(item, bScroll);
}

void CSideBar::setActiveItem(CSideBarButton *newItem, bool bScroll)
{
	CSideBarButton *oldItem = m_activeItem;
	m_activeItem = newItem;
	if (oldItem)
		::InvalidateRect(oldItem->getHwnd(), nullptr, FALSE);
	::InvalidateRect(m_activeItem->getHwnd(), nullptr, FALSE);
	if (bScroll)
		scrollIntoView(m_activeItem);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Layout the buttons within the available space... ensure that buttons are
// set to invisible if there is not enough space. Also, update the state of
// the scroller buttons
//
// @param rc        RECT*:the window rectangle
//
// @param fOnlyCalc bool: if false (default), window positions will be updated, otherwise,
// the method will only calculate the layout parameters. A final call to
// Layout() with the parameter set to false is required to perform the
// position update.

void CSideBar::moveButtons()
{
	if (!m_isVisible)
		return;

	RECT rcWnd;
	::GetClientRect(m_hwndScrollWnd, &rcWnd);

	if (m_currentLayout->pfnLayout) {
		m_currentLayout->pfnLayout(this, &rcWnd);
		return;
	}

	HDWP hdwp = ::BeginDeferWindowPos(1);

	BOOL 	topEnabled = FALSE, bottomEnabled = FALSE;
	uint32_t dwFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW;

	m_firstVisibleOffset = max(0, m_firstVisibleOffset);
	m_totalItemHeight = 0;

	int height = m_elementHeight, spaceUsed = 0, iSpaceAvail = rcWnd.bottom;

	for (auto &p : m_buttonlist) {
		HWND hwnd = p->getHwnd();

		if (m_dwFlags & SIDEBARLAYOUT_DYNHEIGHT)
			height = p->getHeight();

		if (spaceUsed > iSpaceAvail || m_totalItemHeight + height < m_firstVisibleOffset) {
			::ShowWindow(hwnd, SW_HIDE);
			m_totalItemHeight += (height + 1);
			continue;
		}

		if (p->isTopAligned()) {
			if (m_totalItemHeight <= m_firstVisibleOffset) {				// partially visible
				if (nullptr != hwnd) /* Wine fix. */
					hdwp = ::DeferWindowPos(hdwp, hwnd, nullptr, 2, -(m_firstVisibleOffset - m_totalItemHeight), m_elementWidth, height, dwFlags);
				spaceUsed += ((height + 1) - (m_firstVisibleOffset - m_totalItemHeight));
				m_totalItemHeight += (height + 1);
			}
			else {
				if (nullptr != hwnd) /* Wine fix. */
					hdwp = ::DeferWindowPos(hdwp, hwnd, nullptr, 2, spaceUsed, m_elementWidth, height, dwFlags);
				spaceUsed += (height + 1);
				m_totalItemHeight += (height + 1);
			}
		}
	}
	topEnabled = m_firstVisibleOffset > 0;
	bottomEnabled = (m_totalItemHeight - m_firstVisibleOffset > rcWnd.bottom);
	::EndDeferWindowPos(hdwp);

	RECT rcContainer;
	::GetClientRect(m_pContainer->m_hwnd, &rcContainer);

	LONG dx = m_dwFlags & SIDEBARORIENTATION_LEFT ? m_pContainer->m_tBorder_outer_left : rcContainer.right - m_pContainer->m_tBorder_outer_right - (m_elementWidth + 4);
	::SetWindowPos(m_up->getHwnd(), nullptr, dx, m_pContainer->m_tBorder_outer_top + m_pContainer->m_pMenuBar->getHeight(), m_elementWidth + 4, 14, dwFlags);
	::SetWindowPos(m_down->getHwnd(), nullptr, dx, (rcContainer.bottom - 14 - m_pContainer->m_statusBarHeight - 1), m_elementWidth + 4, 14, dwFlags);

	::EnableWindow(m_up->getHwnd(), topEnabled);
	::EnableWindow(m_down->getHwnd(), bottomEnabled);
	::InvalidateRect(m_up->getHwnd(), nullptr, FALSE);
	::InvalidateRect(m_down->getHwnd(), nullptr, FALSE);
}

void CSideBar::showAll(int showCmd)
{
	::ShowWindow(m_up->getHwnd(), showCmd);
	::ShowWindow(m_down->getHwnd(), showCmd);

	for (auto &it : m_buttonlist)
		::ShowWindow(it->getHwnd(), showCmd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Helper function: find a button item associated to the given
// session data
//
// @param dat    _MessageWindowData*: session information
//
// @return CSideBarButtonItem*: pointer to the found item. Zero, if none was found
 
CSideBarButton* CSideBar::findSession(const CMsgDialog *dat)
{
	if (dat == nullptr)
		return nullptr;

	for (auto &it : m_buttonlist)
		if (it->getDat() == dat)
			return it;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Helper function: find a button item associated to the given
// contact handle
//
// @param hContact  HANDLE: contact's handle to look for
//
// @return CSideBarButtonItem*: pointer to the found item. Zero, if none was found

CSideBarButton* CSideBar::findSession(const MCONTACT hContact)
{
	if (hContact == 0)
		return nullptr;

	for (auto &it : m_buttonlist)
		if (it->getContactHandle() == hContact)
			return it;

	return nullptr;
}

void CSideBar::processScrollerButtons(UINT commandID)
{
	if (!m_isActive || m_down == nullptr)
		return;

	if (commandID == IDC_SIDEBARDOWN && ::IsWindowEnabled(m_down->getHwnd()))
		m_firstVisibleOffset += 10;
	else if (commandID == IDC_SIDEBARUP && ::IsWindowEnabled(m_up->getHwnd()))
		m_firstVisibleOffset = max(0, m_firstVisibleOffset - 10);

	moveButtons();
}

void CSideBar::resizeScrollWnd(LONG x, LONG y, LONG, LONG height) const
{
	if (!m_isVisible || !m_isActive) {
		::ShowWindow(m_hwndScrollWnd, SW_HIDE);
		return;
	}
	::SetWindowPos(m_hwndScrollWnd, nullptr, x, y + 15, m_width, height - 30,
		SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSENDCHANGING | SWP_DEFERERASE | SWP_ASYNCWINDOWPOS);
}

void CSideBar::invalidateButton(CMsgDialog *dat)
{
	if (m_isActive && m_isVisible) {
		CSideBarButton *item = findSession(dat);
		if (item != nullptr)
			RedrawWindow(item->m_buttonControl->hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// the window procedure for the sidebar container window (the window which
// acts as a parent for the actual buttons). itself, this window is a child
// of the container window.

LRESULT CALLBACK CSideBar::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_SIZE:
		return TRUE;

	case WM_ERASEBKGND:
		HDC hdc = (HDC)wParam;
		RECT rc;
		::GetClientRect(hwnd, &rc);

		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKSIDEBARBG];

			if (item->IGNORED)
				CSkin::SkinDrawBG(hwnd, m_pContainer->m_hwnd, m_pContainer, &rc, hdc);
			else
				CSkin::DrawItem(hdc, &rc, item);
		}
		else if (M.isAero() || M.isVSThemed()) {
			HDC hdcMem;
			HANDLE hbp = nullptr;
			HBITMAP hbm, hbmOld;

			if (CMimAPI::m_haveBufferedPaint) {
				hbp = CSkin::InitiateBufferedPaint(hdc, rc, hdcMem);
				hbm = hbmOld = nullptr;
			}
			else {
				hdcMem = ::CreateCompatibleDC(hdc);
				hbm = CSkin::CreateAeroCompatibleBitmap(rc, hdcMem);
				hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(hdcMem, hbm));
			}

			if (M.isAero())
				::FillRect(hdcMem, &rc, CSkin::m_BrushBack);
			else
				CSkin::FillBack(hdcMem, &rc);

			if (hbp)
				CSkin::FinalizeBufferedPaint(hbp, &rc);
			else {
				::BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
				::SelectObject(hdcMem, hbmOld);
				::DeleteObject(hbm);
				::DeleteDC(hdcMem);
			}
		}
		else ::FillRect(hdc, &rc, ::GetSysColorBrush(COLOR_3DFACE));
		return 1;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK CSideBar::wndProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSideBar *sideBar = (CSideBar *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (sideBar)
		return(sideBar->wndProc(hwnd, msg, wParam, lParam));

	switch (msg) {
	case WM_NCCREATE:
		CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
		return TRUE;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// paints the background for a switchbar item. It can paint aero, visual styles, skins or
// classic buttons (depending on os and current plugin settings).
//
// @param hdc     HDC: target device context
// @param rc      RECT*: target rectangle
// @param stateId the state identifier (normal, pressed, hot, disabled etc.)

void __fastcall CSideBar::m_DefaultBackgroundRenderer(const HDC hdc, const RECT *rc, const CSideBarButton *item)
{
	UINT  id = item->getID();
	int   stateId = item->m_buttonControl->stateId;
	bool  fIsActiveItem = (item->m_sideBar->getActiveItem() == item);

	if (CSkin::m_skinEnabled) {
		TContainerData *pContainer = const_cast<TContainerData *>(item->m_sideBar->getContainer());
		int ctrlId = stateId == PBS_PRESSED || fIsActiveItem ? ID_EXTBKBUTTONSPRESSED : (stateId == PBS_HOT ? ID_EXTBKBUTTONSMOUSEOVER : ID_EXTBKBUTTONSNPRESSED);
		CSkinItem *skinItem = &SkinItems[ctrlId];
		HWND hwnd = item->m_buttonControl->hwnd;

		CSkin::SkinDrawBG(hwnd, pContainer->m_hwnd, pContainer, const_cast<RECT *>(rc), hdc);
		CSkin::DrawItem(hdc, rc, skinItem);
	}
	else if (M.isAero() || PluginConfig.m_fillColor) {
		if (id == IDC_SIDEBARUP || id == IDC_SIDEBARDOWN) {
			if (M.isAero())
				::FillRect(hdc, const_cast<RECT *>(rc), CSkin::m_BrushBack);
			else
				CSkin::FillBack(hdc, const_cast<RECT *>(rc));

			if (stateId == PBS_HOT || stateId == PBS_PRESSED)
				DrawAlpha(hdc, const_cast<RECT *>(rc), 0xf0f0f0, 70, 0x000000, 0, 9, 31, 4, nullptr);
			else
				DrawAlpha(hdc, const_cast<RECT *>(rc), 0xf0f0f0, 30, 0x707070, 0, 9, 31, 4, nullptr);
		}
		else {
			if (PluginConfig.m_fillColor)
				FillTabBackground(hdc, stateId, item->getDat(), const_cast<RECT *>(rc));

			CSkin::m_switchBarItem->setAlphaFormat(AC_SRC_ALPHA, (stateId == PBS_HOT && !fIsActiveItem) ? 250 : (fIsActiveItem || stateId == PBS_PRESSED ? 250 : 230));
			CSkin::m_switchBarItem->Render(hdc, rc, true);
			if (stateId == PBS_HOT || stateId == PBS_PRESSED || fIsActiveItem) {
				RECT rcGlow = *rc;
				rcGlow.top += 1;
				rcGlow.bottom -= 2;

				CSkin::m_tabGlowTop->setAlphaFormat(AC_SRC_ALPHA, (stateId == PBS_PRESSED || fIsActiveItem) ? 180 : 100);
				CSkin::m_tabGlowTop->Render(hdc, &rcGlow, true);
			}
		}
	}
	else if (M.isVSThemed()) {
		RECT *rcDraw = const_cast<RECT *>(rc);
		if (id == IDC_SIDEBARUP || id == IDC_SIDEBARDOWN) {
			::FillRect(hdc, rc, stateId == PBS_HOT ? ::GetSysColorBrush(COLOR_HOTLIGHT) : ::GetSysColorBrush(COLOR_3DFACE));
			::InflateRect(rcDraw, -2, 0);
			::DrawEdge(hdc, rcDraw, EDGE_ETCHED, BF_SOFT | BF_RECT | BF_FLAT);
		}
		else {
			CSkin::FillBack(hdc, rcDraw);

			if (IsThemeBackgroundPartiallyTransparent(item->m_buttonControl->hThemeToolbar, TP_BUTTON, stateId))
				DrawThemeParentBackground(item->getHwnd(), hdc, rcDraw);

			if (M.isAero() || IsWinVerVistaPlus()) {
				stateId = (fIsActiveItem ? PBS_PRESSED : PBS_HOT);
				DrawThemeBackground(item->m_buttonControl->hThemeToolbar, hdc, 8, RBStateConvert2Flat(stateId), rcDraw, rcDraw);
			}
			else {
				stateId = (fIsActiveItem ? PBS_PRESSED : PBS_HOT);
				DrawThemeBackground(item->m_buttonControl->hThemeToolbar, hdc, TP_BUTTON, TBStateConvert2Flat(stateId), rcDraw, rcDraw);
			}
		}
	}
	else {
		RECT *rcDraw = const_cast<RECT *>(rc);
		if (!(id == IDC_SIDEBARUP || id == IDC_SIDEBARDOWN)) {
			HBRUSH br = (stateId == PBS_HOT && !fIsActiveItem) ? ::GetSysColorBrush(COLOR_BTNSHADOW) : (fIsActiveItem || stateId == PBS_PRESSED ? ::GetSysColorBrush(COLOR_HOTLIGHT) : ::GetSysColorBrush(COLOR_3DFACE));
			::FillRect(hdc, rc, br);
			::DrawEdge(hdc, rcDraw, (stateId == PBS_HOT && !fIsActiveItem) ? EDGE_ETCHED : (fIsActiveItem || stateId == PBS_PRESSED) ? EDGE_BUMP : EDGE_ETCHED, BF_RECT | BF_SOFT | BF_FLAT);
		}
		else {
			::FillRect(hdc, rc, stateId == PBS_HOT ? ::GetSysColorBrush(COLOR_HOTLIGHT) : ::GetSysColorBrush(COLOR_3DFACE));
			::InflateRect(rcDraw, -2, 0);
			::DrawEdge(hdc, rcDraw, EDGE_ETCHED, BF_SOFT | BF_RECT | BF_FLAT);
		}
	}
}

void __fastcall CSideBar::m_DefaultContentRenderer(const HDC hdc, const RECT *rcBox, const CSideBarButton *item)
{
	UINT id = item->getID();
	int stateID = item->m_buttonControl->stateId;

	if (id == IDC_SIDEBARUP || id == IDC_SIDEBARDOWN) {
		::DrawIconEx(hdc, (rcBox->left + rcBox->right) / 2 - 8, (rcBox->top + rcBox->bottom) / 2 - 8, id == IDC_SIDEBARUP ? PluginConfig.g_buttonBarIcons[26] : PluginConfig.g_buttonBarIcons[16],
			16, 16, 0, nullptr, DI_NORMAL);
		if (!M.isAero() && stateID == PBS_HOT)
			::DrawEdge(hdc, const_cast<RECT *>(rcBox), BDR_INNER, BF_RECT | BF_SOFT | BF_FLAT);
	}
	else if (item->getDat())
		item->renderIconAndNick(hdc, rcBox);
}

/////////////////////////////////////////////////////////////////////////////////////////
// content renderer for the advanced side bar button layout. includes
// avatars

void __fastcall CSideBar::m_AdvancedContentRenderer(const HDC hdc, const RECT *rcBox, const CSideBarButton *item)
{
	const CMsgDialog *dat = item->getDat();
	UINT id = item->getID();

	LONG	cx = rcBox->right - rcBox->left;
	LONG	cy = rcBox->bottom - rcBox->top;
	SIZE	szFirstLine, szSecondLine;

	if (id == IDC_SIDEBARUP || id == IDC_SIDEBARDOWN)
		m_DefaultContentRenderer(hdc, rcBox, item);
	else if (dat) {
		RECT rc = *rcBox;

		if (dat->m_ace && dat->m_ace->hbmPic) {		// we have an avatar
			double dNewHeight, dNewWidth;
			LONG maxHeight = cy - 8;

			Utils::scaleAvatarHeightLimited(dat->m_ace->hbmPic, dNewWidth, dNewHeight, maxHeight);

			HBITMAP hbmResized = ::Image_Resize(dat->m_ace->hbmPic, RESIZEBITMAP_STRETCH, dNewWidth, dNewHeight);
			HDC dc = CreateCompatibleDC(hdc);
			HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(dc, hbmResized));

			LONG xOff = (cx - maxHeight) + (maxHeight - (LONG)dNewWidth) / 2 - 4;
			LONG yOff = (cy - (LONG)dNewHeight) / 2;

			GdiAlphaBlend(hdc, xOff, yOff, (LONG)dNewWidth, (LONG)dNewHeight, dc, 0, 0, (LONG)dNewWidth, (LONG)dNewHeight, CSkin::m_default_bf);
			::SelectObject(dc, hbmOld);
			if (hbmResized != dat->m_ace->hbmPic)
				::DeleteObject(hbmResized);
			::DeleteDC(dc);
			rc.right -= (maxHeight + 6);
		}

		// calculate metrics based on font configuration. Determine if we have enough
		// space for both lines
		rc.left += 3;
		HFONT	hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_NICK]));
		::GetTextExtentPoint32A(hdc, "A", 1, &szFirstLine);
		::SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_STATUS]);
		::GetTextExtentPoint32A(hdc, "A", 1, &szSecondLine);
		szSecondLine.cy = max(szSecondLine.cy, 18);

		LONG	required = szFirstLine.cy + szSecondLine.cy;
		bool	fSecondLine = (required < cy ? true : false);

		uint32_t	dtFlags = DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | (!fSecondLine ? DT_VCENTER : 0);

		::SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_NICK]);
		rc.top++;
		::SetBkMode(hdc, TRANSPARENT);
		CSkin::RenderText(hdc, dat->m_hThemeIP, dat->m_cache->getNick(), &rc,
			dtFlags, CSkin::m_glowSize, CInfoPanel::m_ipConfig.clrs[IPFONTID_NICK]);

		if (fSecondLine) {
			int		iSize;
			HICON	hIcon = dat->m_cache->getIcon(iSize);

			// TODO support larger icons at a later time. This side bar button
			// could use 32x32 icons as well.
			rc.top = rc.bottom - szSecondLine.cy - 2;
			::DrawIconEx(hdc, rc.left, rc.top + (rc.bottom - rc.top) / 2 - 8, hIcon, 16, 16, 0, nullptr, DI_NORMAL);
			rc.left += 18;
			::SelectObject(hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_STATUS]);
			CSkin::RenderText(hdc, dat->m_hThemeIP, dat->m_wszStatus, &rc,
				dtFlags | DT_VCENTER, CSkin::m_glowSize, CInfoPanel::m_ipConfig.clrs[IPFONTID_STATUS]);
		}
		::SelectObject(hdc, hOldFont);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// measure callback for the advanced sidebar button layout (vertical mode
// with variable height buttons)

const SIZE &__fastcall CSideBar::m_measureAdvancedVertical(CSideBarButton *item)
{
	SIZE sz = {};

	const CMsgDialog *dat = item->getDat();
	if (dat) {
		SIZE szFirstLine, szSecondLine;

		if (dat->m_ace && dat->m_ace->hbmPic)
			sz.cy = item->getLayout()->width;

		HDC dc = ::GetDC(dat->GetHwnd());

		HFONT	hOldFont = reinterpret_cast<HFONT>(::SelectObject(dc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_NICK]));
		::GetTextExtentPoint32(dc, dat->m_cache->getNick(), (int)mir_wstrlen(dat->m_cache->getNick()), &szFirstLine);
		::SelectObject(dc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_STATUS]);
		::GetTextExtentPoint32(dc, dat->m_wszStatus, (int)mir_wstrlen(dat->m_wszStatus), &szSecondLine);
		::SelectObject(dc, hOldFont);
		ReleaseDC(dat->GetHwnd(), dc);

		szSecondLine.cx += 18;				// icon space

		sz.cy += max(szFirstLine.cx + 4, szSecondLine.cx + 4);
		sz.cy += 2;
	}
	item->setSize(sz);
	return(item->getSize());
}
