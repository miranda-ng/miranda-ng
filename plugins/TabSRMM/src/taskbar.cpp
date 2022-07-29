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
// Windows 7 taskbar integration
//
// - overlay icons
// - custom taskbar thumbnails for aero peek in tabbed containers
// - read Windows 7 task bar configuration from the registry.
//
/////////////////////////////////////////////////////////////////////////////////////////
//  how it works:
//
//  Because of the fact, the DWM does not talk to non-toplevel windows
//  we need an invisible "proxy window" for each tab. This window is a very
//  small and hidden toplevel tool window which is used to communicate
//  with the dwm. Each proxy is associated with the client window (the "tab")
//  and registers itself with the message container window via
//  ITaskbarList3::RegisterTab().
//
//  Instead of automatically created snapshots of the window content, we
//  use custom generated thumbnails for the task bar buttons, including
//  nickname, UID, status message and avatar. This makes the thumbnails
//  easily recognizable.
//
//  Thumbnails are generated "on request", only when the desktop window
//  manager needs one.
//
// Each proxy window has a CThumbIM or CThumbMUC object which represents
// the actual thumbnail bitmap.

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// maps MUC event types to icon names for retrieving the "big" icons
// while generating task bar thumbnails.
// used by getMUCBigICon()

CTaskbarInteract* Win7Taskbar = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// sets the overlay icon for a task bar button. Used for typing notifications and incoming
// message indicator.

bool CTaskbarInteract::setOverlayIcon(HWND hwndDlg, LPARAM lParam) const
{
	if (m_pTaskbarInterface && m_isEnabled && m_fHaveLargeicons) {
		m_pTaskbarInterface->SetOverlayIcon(hwndDlg, (HICON)lParam, nullptr);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// checks the task bar status for "large icon mode".
// @return bool: true if large icons are in use, false otherwise

bool CTaskbarInteract::haveLargeIcons()
{
	m_fHaveLargeicons = false;

	if (m_pTaskbarInterface && m_isEnabled) {
		uint32_t val = 0;
		uint32_t valGrouping = 2;
		/*
		 * check whether the taskbar is set to show large icons. This is necessary, because the method SetOverlayIcon()
		 * always returns S_OK, but the icon is simply ignored when using small taskbar icons.
		 * also, figure out the button grouping mode.
		 */
		HKEY hKey;
		if (::RegOpenKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey) == ERROR_SUCCESS) {
			DWORD size = sizeof(val), dwType = REG_DWORD;
			::RegQueryValueEx(hKey, L"TaskbarSmallIcons", nullptr, &dwType, (LPBYTE)&val, &size);
			size = 4;
			dwType = REG_DWORD;
			/*
			 * this is the "grouping mode" setting for the task bar. 0 = always combine, no labels
			 */
			size = sizeof(valGrouping);
			::RegQueryValueEx(hKey, L"TaskbarGlomLevel", nullptr, &dwType, (LPBYTE)&valGrouping, &size);
			::RegCloseKey(hKey);
		}
		m_fHaveLargeicons = (val ? false : true);			// small icons in use, revert to default icon feedback
		m_fHaveAlwaysGrouping = (valGrouping == 0 ? true : false);
	}
	return(m_fHaveLargeicons);
}

/////////////////////////////////////////////////////////////////////////////////////////
// removes the overlay icon for the given container window

void CTaskbarInteract::clearOverlayIcon(HWND hwndDlg) const
{
	if (m_pTaskbarInterface && m_isEnabled)
		m_pTaskbarInterface->SetOverlayIcon(hwndDlg, nullptr, nullptr);
}

LONG CTaskbarInteract::updateMetrics()
{
	m_IconSize = 32;

	return(m_IconSize);
}

/////////////////////////////////////////////////////////////////////////////////////////
// register a new task bar button ("tab") for the button group hwndContainer
// (one of the top level message windows)

void CTaskbarInteract::registerTab(const HWND hwndTab, const HWND hwndContainer) const
{
	if (m_isEnabled) {
		m_pTaskbarInterface->RegisterTab(hwndTab, hwndContainer);
		m_pTaskbarInterface->SetTabOrder(hwndTab, nullptr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// remove a previously registered proxy window. The destructor of the proxy
// window class is using this before destroying the proxy window itself.
// @param hwndTab	proxy window handle

void CTaskbarInteract::unRegisterTab(const HWND hwndTab) const
{
	if (m_isEnabled)
		m_pTaskbarInterface->UnregisterTab(hwndTab);
}

/////////////////////////////////////////////////////////////////////////////////////////
// set a tab as active. The active thumbnail will appear with a slightly
// different background and transparency.

void CTaskbarInteract::SetTabActive(const HWND hwndTab, const HWND hwndGroup) const
{
	if (m_isEnabled)
		m_pTaskbarInterface->SetTabActive(hwndTab, hwndGroup, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This is called from the broadcasted WM_DWMCOMPOSITIONCHANGED event by all messages
// sessions. It checks and, if needed, destroys or creates a proxy object, based on
// the status of the DWM

void CMsgDialog::VerifyProxy()
{
	if (IsWinVer7Plus() && PluginConfig.m_useAeroPeek) {
		if (nullptr == m_pWnd) {
			m_pWnd = new CProxyWindow(this);
			m_pWnd->updateIcon(m_hTabStatusIcon);
			m_pWnd->updateTitle(m_cache->getNick());
		}
		else m_pWnd->verifyDwmState();
	}
	/*
	 * this should not happens, but who knows...
	 */
	else {
		if (m_pWnd) {
			delete m_pWnd;
			m_pWnd = nullptr;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// create the proxy (toplevel) window required to show per tab thumbnails
// and previews for a message session.
// each tab has one invisible proxy window

CProxyWindow::CProxyWindow(CMsgDialog *dat) :
	m_dat(dat)
{
	m_hwndProxy = ::CreateWindowEx(/*WS_EX_TOOLWINDOW | */WS_EX_NOACTIVATE, PROXYCLASSNAME, L"",
		WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION, -32000, -32000, 10, 10, nullptr, nullptr, g_plugin.getInst(), (LPVOID)this);

	Win7Taskbar->registerTab(m_hwndProxy, m_dat->m_pContainer->m_hwnd);
	if (CMimAPI::m_pfnDwmSetWindowAttribute) {
		BOOL	fIconic = TRUE;
		BOOL	fHasIconicBitmap = TRUE;

		CMimAPI::m_pfnDwmSetWindowAttribute(m_hwndProxy, DWMWA_FORCE_ICONIC_REPRESENTATION, &fIconic, sizeof(fIconic));
		CMimAPI::m_pfnDwmSetWindowAttribute(m_hwndProxy, DWMWA_HAS_ICONIC_BITMAP, &fHasIconicBitmap, sizeof(fHasIconicBitmap));
	}
}

CProxyWindow::~CProxyWindow()
{
	Win7Taskbar->unRegisterTab(m_hwndProxy);
	::DestroyWindow(m_hwndProxy);

	delete m_thumb;
}

/////////////////////////////////////////////////////////////////////////////////////////
// verify status of DWM when system broadcasts a WM_DWMCOMPOSITIONCHANGED message
// delete thumbnails, if no longer needed

void CProxyWindow::verifyDwmState()
{
	if (!M.isDwmActive()) {
		if (m_thumb) {
			delete m_thumb;
			m_thumb = nullptr;
		}
	}
	else {
		/*
		 * force thumbnail recreation
		 */
		m_width = 0;
		m_height = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// send a thumbnail to the DWM. If required, refresh it first.
// called by WM_DWMSENDICONICTHUMBNAIL handler.

void CProxyWindow::sendThumb(LONG width, LONG height)
{
	if (nullptr == m_thumb) {
		m_width = width;
		m_height = height;
		m_thumb = m_dat->tabCreateThumb(this);
	}
	else if (width != m_width || height != m_height || !m_thumb->isValid()) {
		m_width = width;
		m_height = height;
		m_thumb->update();
	}
	if (m_thumb)
		CMimAPI::m_pfnDwmSetIconicThumbnail(m_hwndProxy, m_thumb->getHBM(), DWM_SIT_DISPLAYFRAME);
}

/////////////////////////////////////////////////////////////////////////////////////////
// send a live preview image of a given message session to the DWM.
// called by WM_DWMSENDICONICLIVEPREVIEWBITMAP on DWM's request.
//
// The bitmap can be deleted after submitting it, because the DWM
// will cache a copy of it (and re-request it when its own bitmap cache was purged).

void CProxyWindow::sendPreview()
{
	if (m_dat->m_pContainer == nullptr)
		return;

	CMsgDialog *dat_active = reinterpret_cast<CMsgDialog *>(::GetWindowLongPtr(m_dat->m_pContainer->m_hwndActive, GWLP_USERDATA));
	if (!m_thumb || !dat_active)
		return;

	FORMATRANGE fr = {};
	POINT pt = {};
	RECT rcContainer, rcTemp, rcRich, rcLog;
	HDC hdc, dc;
	int twips = (int)(15.0f / PluginConfig.m_DPIscaleY);
	bool fIsChat = m_dat->isChat();
	HWND hwndRich = m_dat->m_pLog->GetHwnd();
	POINT	ptOrigin = {}, ptBottom;

	if (m_dat->m_bNeedCheckSize) {
		RECT rcClient;
		m_dat->m_pContainer->QueryClientArea(rcClient);
		::MoveWindow(m_dat->GetHwnd(), rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), FALSE);
		::SendMessage(m_dat->GetHwnd(), WM_SIZE, 0, 0);
		m_dat->DM_ScrollToBottom(0, 1);
	}
	
	// a minimized container has a null rect as client area, so do not use it
	// use the last known client area size instead.
	if (!::IsIconic(m_dat->m_pContainer->m_hwnd)) {
		::GetWindowRect(m_dat->m_pContainer->m_hwndActive, &rcLog);
		::GetClientRect(m_dat->m_pContainer->m_hwnd, &rcContainer);
		pt.x = rcLog.left;
		pt.y = rcLog.top;
		::ScreenToClient(m_dat->m_pContainer->m_hwnd, &pt);
	}
	else {
		rcLog = m_dat->m_pContainer->m_rcLogSaved;
		rcContainer = m_dat->m_pContainer->m_rcSaved;
		pt = m_dat->m_pContainer->m_ptLogSaved;
	}

	::GetWindowRect(::GetDlgItem(m_dat->m_pContainer->m_hwndActive, IDC_SRMM_LOG), &rcTemp);
	ptBottom.x = rcTemp.left;
	ptBottom.y = rcTemp.bottom;
	::ScreenToClient(m_dat->m_pContainer->m_hwnd, &ptBottom);

	int cx = rcLog.right - rcLog.left;
	int cy = rcLog.bottom - rcLog.top;
	rcRich.left = 0;
	rcRich.top = 0;
	rcRich.right = cx;
	rcRich.bottom = ptBottom.y - pt.y;

	dc = ::GetDC(m_dat->GetHwnd());
	hdc = ::CreateCompatibleDC(dc);
	HBITMAP hbm = CSkin::CreateAeroCompatibleBitmap(rcContainer, hdc);
	HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(hdc, hbm));

	HBRUSH brb = ::CreateSolidBrush(RGB(20, 20, 20));
	::FillRect(hdc, &rcContainer, brb);
	::DeleteObject(brb);
	CImageItem::SetBitmap32Alpha(hbm, 100);

	LRESULT first = ::SendMessage(hwndRich, EM_CHARFROMPOS, 0, LPARAM(&ptOrigin));

	// paint the content of the message log control into a separate bitmap without transparency
	HDC hdcRich = ::CreateCompatibleDC(dc);
	HBITMAP hbmRich = CSkin::CreateAeroCompatibleBitmap(rcRich, hdcRich);
	HBITMAP hbmRichOld = reinterpret_cast<HBITMAP>(::SelectObject(hdcRich, hbmRich));

	COLORREF clr = fIsChat ? db_get_dw(0, FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR) : m_dat->m_pContainer->m_theme.inbg;
	HBRUSH br = ::CreateSolidBrush(clr);
	::FillRect(hdcRich, &rcRich, br);
	::DeleteObject(br);

	if (m_dat->m_iLogMode == WANT_IEVIEW_LOG)
		::SendMessage(hwndRich, WM_PRINT, reinterpret_cast<WPARAM>(hdcRich), PRF_CLIENT | PRF_NONCLIENT);
	else if (m_dat->m_iLogMode == WANT_HPP_LOG) {
		CSkin::RenderText(hdcRich, m_dat->m_hTheme, TranslateT("Previews not available when using History++ plugin for message log display."),
			&rcRich, DT_VCENTER | DT_CENTER | DT_WORDBREAK, 10, m_dat->m_pContainer->m_theme.fontColors[MSGFONTID_MYMSG], false);
	}
	else {
		rcRich.right *= twips;
		rcRich.bottom *= twips;

		fr.hdc = hdcRich;
		fr.hdcTarget = hdcRich;
		fr.rc = rcRich;
		fr.rcPage = rcRich;
		fr.chrg.cpMax = -1;
		fr.chrg.cpMin = first;
		::SendMessage(hwndRich, EM_FORMATRANGE, 1, LPARAM(&fr));
	}

	::SelectObject(hdcRich, hbmRichOld);
	CImageItem::SetBitmap32Alpha(hbmRich, 255);
	::SelectObject(hdcRich, hbmRich);
	::BitBlt(hdc, pt.x, pt.y, cx, cy, hdcRich, 0, 0, SRCCOPY);
	::SelectObject(hdcRich, hbmRichOld);
	::DeleteObject(hbmRich);
	::DeleteDC(hdcRich);

	::SelectObject(hdc, hbmOld);
	::DeleteDC(hdc);
	if (CSkin::m_skinEnabled && CSkin::m_frameSkins) {
		pt.x = CSkin::m_SkinnedFrame_left;
		pt.y = CSkin::m_SkinnedFrame_caption + CSkin::m_SkinnedFrame_bottom;
	}
	else pt.x = pt.y = 0;

	CMimAPI::m_pfnDwmSetIconicLivePreviewBitmap(m_hwndProxy, hbm, &pt, m_dat->m_pContainer->cfg.flags.m_bCreateMinimized ? 0 : DWM_SIT_DISPLAYFRAME);
	::ReleaseDC(m_dat->GetHwnd(), dc);
	::DeleteObject(hbm);
}

/////////////////////////////////////////////////////////////////////////////////////////
// set the large icon for the thumbnail. This is mostly used by group chats
// to indicate last active event in the session.
//
// hIcon may be 0 to remove a custom big icon. In that case, the renderer
// will try to figure out a suitable one, based on session data.

void CProxyWindow::setBigIcon(const HICON hIcon, bool fInvalidate)
{
	m_hBigIcon = hIcon;
	if (fInvalidate)
		Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////
// set a overlay icon for the thumbnail. This is mostly used by group chats
// to indicate last active event in the session.
// 
// hIcon may be 0 to remove a custom overlay icon.

void CProxyWindow::setOverlayIcon(const HICON hIcon, bool fInvalidate)
{
	m_hOverlayIcon = hIcon;
	if (fInvalidate)
		Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////
// update the (small) thumbnail icon in front of the title string

void CProxyWindow::updateIcon(const HICON hIcon) const
{
	if (m_hwndProxy && hIcon)
		::SendMessage(m_hwndProxy, WM_SETICON, ICON_SMALL, LPARAM(hIcon));
}

/////////////////////////////////////////////////////////////////////////////////////////
// set the task bar button ("tab") active. This activates the proxy
// window as a sub-window of the (top level) container window.
// This is called whenever the active message tab or window changes

void CProxyWindow::activateTab() const
{
	Win7Taskbar->SetTabActive(m_hwndProxy, m_dat->m_pContainer->m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// invalidate the thumbnail, it will be recreated at the next request by the DWM
// 
// this is called from several places whenever a relevant information,
// represented in a thumbnail image, has changed.
// also tells the DWM that it must request a new thumb.

void CProxyWindow::Invalidate() const
{
	if (m_thumb) {
		m_thumb->setValid(false);
		/*
		 * tell the DWM to request a new thumbnail for the proxy window m_hwnd
		 * when it needs one.
		 */
		CMimAPI::m_pfnDwmInvalidateIconicBitmaps(m_hwndProxy);
	}
}

/**
 * update the thumb title string (usually, the nickname)
 * @param tszTitle: 	new title string
 */
void CProxyWindow::updateTitle(const wchar_t *tszTitle) const
{
	if (m_hwndProxy && tszTitle)
		::SetWindowText(m_hwndProxy, tszTitle);
}

/**
 * stub window procedure for the custom proxy window class
 * just initialize GWLP_USERDATA and call the object's method
 *
 * static member function
 */
LRESULT CALLBACK CProxyWindow::stubWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CProxyWindow *pWnd = reinterpret_cast<CProxyWindow *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pWnd)
		return pWnd->wndProc(hWnd, msg, wParam, lParam);

	switch (msg) {
	case WM_NCCREATE:
		CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
		pWnd = reinterpret_cast<CProxyWindow *>(cs->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		return pWnd->wndProc(hWnd, msg, wParam, lParam);
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// window procedure for the proxy window

LRESULT CALLBACK CProxyWindow::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CLOSE:
		{
			HWND hwndCont = m_dat->m_pContainer->m_hwnd;

			SendMessage(m_dat->GetHwnd(), WM_CLOSE, 1, 2);

			if (!IsIconic(hwndCont))
				SetForegroundWindow(hwndCont);
		}
		return 0;

	case WM_ACTIVATE:
		// proxy window was activated by clicking on the thumbnail. Send this
		// to the real message window.
		if (WA_ACTIVE == wParam) {
			if (IsWindow(m_dat->GetHwnd()))
				m_dat->ActivateTab();
			
			// no default processing, avoid flickering.
			return 0;
		}
		break;

	case WM_NCDESTROY:
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		break;

	case WM_DWMSENDICONICTHUMBNAIL:
		sendThumb(HIWORD(lParam), LOWORD(lParam));
		return 0;

	case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
		sendPreview();
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// base thumbnail class. Create the background and common parts for a thumbnail

CThumbBase::CThumbBase(const CProxyWindow* _p) :
	m_isValid(false)
{
	m_pWnd = _p;
	m_hbmThumb = nullptr;
	renderBase();
}

/////////////////////////////////////////////////////////////////////////////////////////
// render base for a thumbnail. This creates the background, the large icon
// and the basic status mode text. It also divides the thumbnail rectangle
// into a few content rectangles used later by the content renderer.

void CThumbBase::renderBase()
{
	HICON	hIcon = nullptr;
	HBRUSH	brBack;
	LONG	lIconSize = 32;

	m_width = m_pWnd->getWidth();
	m_height = m_pWnd->getHeight();
	m_dat = m_pWnd->getDat();
	m_dtFlags = 0;
	m_hOldFont = nullptr;

	m_rc.right = m_width;
	m_rc.bottom = m_height;
	m_rc.left = m_rc.top = 0;

	if (m_hbmThumb) {
		::DeleteObject(m_hbmThumb);
		m_hbmThumb = nullptr;
	}

	HDC dc = ::GetDC(m_pWnd->getHwnd());
	m_hdc = ::CreateCompatibleDC(dc);

	m_hbmThumb = CSkin::CreateAeroCompatibleBitmap(m_rc, m_hdc);
	m_hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(m_hdc, m_hbmThumb));
	ReleaseDC(m_pWnd->getHwnd(), dc);

	brBack = ::CreateSolidBrush(m_dat->m_dwUnread ? RGB(80, 60, 60) : RGB(60, 60, 60));
	::FillRect(m_hdc, &m_rc, brBack);
	::DeleteObject(brBack);

	::SelectObject(m_hdc, m_hbmOld);
	CImageItem::SetBitmap32Alpha(m_hbmThumb, m_dat->m_dwUnread ? 110 : 60);
	m_hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(m_hdc, m_hbmThumb));

	SetBkMode(m_hdc, TRANSPARENT);

	m_hOldFont = reinterpret_cast<HFONT>(::SelectObject(m_hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_STATUS]));
	::GetTextExtentPoint32A(m_hdc, "A", 1, &m_sz);

	InflateRect(&m_rc, -3, -3);

	setupRect();
	hIcon = m_pWnd->getBigIcon();

	if (nullptr == hIcon) {
		if (m_dat->m_dwUnread) {
			if (PluginConfig.g_IconMsgEventBig)
				hIcon = PluginConfig.g_IconMsgEventBig;
			else {
				hIcon = PluginConfig.g_IconMsgEvent;
				lIconSize = 16;
			}
		}
		else {
			hIcon = reinterpret_cast<HICON>(Skin_LoadProtoIcon(m_dat->m_cache->getActiveProto(), m_dat->m_cache->getActiveStatus(), true));
			if (nullptr == hIcon || reinterpret_cast<HICON>(CALLSERVICE_NOTFOUND) == hIcon) {
				hIcon = reinterpret_cast<HICON>(Skin_LoadProtoIcon(m_dat->m_cache->getActiveProto(), m_dat->m_cache->getActiveStatus()));
				lIconSize = 16;
			}
		}
	}
	::DrawIconEx(m_hdc, m_rcIcon.right / 2 - lIconSize / 2, m_rcIcon.top, hIcon, lIconSize, lIconSize, 0, nullptr, DI_NORMAL);
	hIcon = m_pWnd->getOverlayIcon();
	if (hIcon)
		::DrawIconEx(m_hdc, m_rcIcon.right - 16, m_rcIcon.top + 16, hIcon, 16, 16, 0, nullptr, DI_NORMAL);

	m_rcIcon.top += (lIconSize + 3);
	CSkin::RenderText(m_hdc, m_dat->m_hTheme, m_dat->m_wszStatus, &m_rcIcon, m_dtFlags | DT_CENTER | DT_WORD_ELLIPSIS, 10, 0, true);
	if (m_dat->m_dwUnread && !m_dat->isChat()) {
		wchar_t	tszTemp[30];

		m_rcIcon.top += m_sz.cy;
		mir_snwprintf(tszTemp, TranslateT("%d unread"), m_dat->m_dwUnread);
		CSkin::RenderText(m_hdc, m_dat->m_hTheme, tszTemp, &m_rcIcon, m_dtFlags | DT_CENTER | DT_WORD_ELLIPSIS, 10, 0, true);
	}
	m_rcIcon = m_rcTop;
	m_rcIcon.top += 2;
	m_rcIcon.left = m_rc.right / 3;
	m_cx = m_rcIcon.right - m_rcIcon.left;
	m_cy = m_rcIcon.bottom - m_rcIcon.top;
}

/////////////////////////////////////////////////////////////////////////////////////////
// divide space into content rectangles for normal thumbnails

void CThumbBase::setupRect()
{
	if (!m_pWnd->getDat()->isChat()) {
		m_rcTop = m_rc;
		m_rcBottom = m_rc;
		m_rcBottom.top = m_rc.bottom - (2 * (m_rcBottom.bottom / 5)) - 2;
		m_rcTop.bottom = m_rcBottom.top - 2;

		m_rcIcon = m_rcTop;
		m_rcIcon.right = m_rc.right / 3;
	}
	else {
		m_rcTop = m_rc;
		m_rcBottom = m_rc;
		m_rcBottom.top = m_rc.bottom - (2 * (m_rcBottom.bottom / 5)) - 2;
		m_rcTop.bottom = m_rcBottom.top - 2;

		m_rcIcon = m_rcTop;
		m_rcIcon.right = m_rc.left + 42;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// destroy the thumbnail object. Just delete the bitmap we cached 

CThumbBase::~CThumbBase()
{
	if (m_hbmThumb) {
		::DeleteObject(m_hbmThumb);
		m_hbmThumb = nullptr;
		m_isValid = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// create a IM session thumbnail. base class will create the bitmap and render the background.

CThumbIM::CThumbIM(const CProxyWindow* _p) : CThumbBase(_p)
{
	renderContent();
	setValid(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update the thumbnail, render everything and set it valid

void CThumbIM::update()
{
	renderBase();
	renderContent();
	setValid(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// render the content for an IM chat session thumbnail m_hdc etc. 
// must already be initialized (done by the constructor) background had been already rendered

void CThumbIM::renderContent()
{
	double dNewWidth = 0.0, dNewHeight = 0.0;

	HBITMAP hbmAvatar = (m_dat->m_ace && m_dat->m_ace->hbmPic) ? m_dat->m_ace->hbmPic : PluginConfig.g_hbmUnknown;
	Utils::scaleAvatarHeightLimited(hbmAvatar, dNewWidth, dNewHeight, m_rcIcon.bottom - m_rcIcon.top);

	HBITMAP hbmResized = ::Image_Resize(hbmAvatar, RESIZEBITMAP_STRETCH, dNewWidth, dNewHeight);

	HDC	dc = CreateCompatibleDC(m_hdc);
	HBITMAP hbmOldAv = reinterpret_cast<HBITMAP>(::SelectObject(dc, hbmResized));

	LONG xOff = m_rcIcon.right - (LONG)dNewWidth - 2;
	LONG yOff = (m_cy - (LONG)dNewHeight) / 2 + m_rcIcon.top;

	HRGN hRgn = ::CreateRectRgn(xOff - 1, yOff - 1, xOff + (LONG)dNewWidth + 2, yOff + (LONG)dNewHeight + 2);
	CSkin::m_default_bf.SourceConstantAlpha = 150;
	GdiAlphaBlend(m_hdc, xOff, yOff, (LONG)dNewWidth, (LONG)dNewHeight, dc, 0, 0, (LONG)dNewWidth, (LONG)dNewHeight, CSkin::m_default_bf);
	CSkin::m_default_bf.SourceConstantAlpha = 255;
	::FrameRgn(m_hdc, hRgn, reinterpret_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)), 1, 1);

	::DeleteObject(hRgn);
	::SelectObject(dc, hbmOldAv);

	if (hbmResized != hbmAvatar)
		::DeleteObject(hbmResized);

 	::DeleteDC(dc);
	m_rcBottom.bottom -= 16;

	// status message and bottom line (either UID or nick name, depending on
	// task bar grouping mode). For chat rooms, it is the topic.
	if ((m_rcBottom.bottom - m_rcBottom.top) < 2 * m_sz.cy)
		m_dtFlags |= DT_SINGLELINE;

	m_rcBottom.bottom -= ((m_rcBottom.bottom - m_rcBottom.top) % m_sz.cy);		// adjust to a multiple of line height

	const wchar_t *tszStatusMsg = m_dat->m_cache->getStatusMsg();
	if (tszStatusMsg == nullptr)
		tszStatusMsg = TranslateT("No status message");

	CSkin::RenderText(m_hdc, m_dat->m_hTheme, tszStatusMsg, &m_rcBottom, DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | m_dtFlags, 10, 0, true);
	m_rcBottom.bottom = m_rc.bottom;
	m_rcBottom.top = m_rcBottom.bottom - m_sz.cy - 2;
	CSkin::RenderText(m_hdc, m_dat->m_hTheme, Win7Taskbar->haveAlwaysGroupingMode() ? m_dat->m_cache->getUIN() : m_dat->m_cache->getNick(),
		&m_rcBottom, m_dtFlags | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS, 10, 0, true);

	// finalize it
	// do NOT delete the bitmap, the dwm will need the handle
	// m_hbm is deleted when a new thumbnail is generated on dwm's request.
	// this is not a leak!
	if (m_hOldFont)
		::SelectObject(m_hdc, m_hOldFont);

	::SelectObject(m_hdc, m_hbmOld);
	::DeleteDC(m_hdc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// create a MUC session thumbnail. base class will create the 
// bitmap and render the background.

CThumbMUC::CThumbMUC(const CProxyWindow* _p, SESSION_INFO *_si)
	: CThumbBase(_p),
	si(_si)
{
	renderContent();
	setValid(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update an invalidated thumbnail

void CThumbMUC::update()
{
	renderBase();
	renderContent();
	setValid(true);
}

/////////////////////////////////////////////////////////////////////////////////////////
// render content area for a MUC thumbnail

void CThumbMUC::renderContent()
{
	if (si == nullptr)
		return;

	const MODULEINFO *mi = si->pMI;
	wchar_t szTemp[250];
	if (m_dat->m_dwUnread) {
		mir_snwprintf(szTemp, TranslateT("%d unread"), m_dat->m_dwUnread);
		CSkin::RenderText(m_hdc, m_dat->m_hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
		m_rcIcon.top += m_sz.cy;
	}
	if (si->iType != GCW_SERVER) {
		wchar_t* _p = nullptr;
		if (si->ptszStatusbarText)
			_p = wcschr(si->ptszStatusbarText, ']');
		if (_p) {
			_p++;
			wchar_t	_t = *_p;
			*_p = 0;
			mir_snwprintf(szTemp, TranslateT("Chat room %s"), si->ptszStatusbarText);
			*_p = _t;
		}
		else
			mir_snwprintf(szTemp, TranslateT("Chat room %s"), L"");
		CSkin::RenderText(m_hdc, m_dat->m_hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
		m_rcIcon.top += m_sz.cy;
		mir_snwprintf(szTemp, TranslateT("%d user(s)"), si->getUserList().getCount());
		CSkin::RenderText(m_hdc, m_dat->m_hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
	}
	else {
		mir_snwprintf(szTemp, TranslateT("Server window"));
		CSkin::RenderText(m_hdc, m_dat->m_hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
		if (mi->tszIdleMsg[0] && mir_wstrlen(mi->tszIdleMsg) > 2) {
			m_rcIcon.top += m_sz.cy;
			CSkin::RenderText(m_hdc, m_dat->m_hTheme, &mi->tszIdleMsg[2], &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
		}
	}

	if ((m_rcBottom.bottom - m_rcBottom.top) < 2 * m_sz.cy)
		m_dtFlags |= DT_SINGLELINE;

	m_rcBottom.bottom -= ((m_rcBottom.bottom - m_rcBottom.top) % m_sz.cy);		// adjust to a multiple of line height

	const wchar_t *szStatusMsg = nullptr;
	if (si->iType != GCW_SERVER) {
		if (nullptr == (szStatusMsg = si->ptszTopic))
			szStatusMsg = TranslateT("no topic set.");
	}
	else if (mi) {
		mir_snwprintf(szTemp, TranslateT("%s on %s%s"), m_dat->m_wszMyNickname, mi->ptszModDispName, L"");
		szStatusMsg = szTemp;
	}

	CSkin::RenderText(m_hdc, m_dat->m_hTheme, szStatusMsg, &m_rcBottom, DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | m_dtFlags, 10, 0, true);

	// finalize it
	// do NOT delete the bitmap, the dwm will need the handle
	// m_hbm is deleted when a new thumbnail is generated on dwm's request.
	// this is not a leak!
	if (m_hOldFont)
		::SelectObject(m_hdc, m_hOldFont);

	::SelectObject(m_hdc, m_hbmOld);
	::DeleteDC(m_hdc);
}
