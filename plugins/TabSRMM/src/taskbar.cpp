/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
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

#include "commonheaders.h"

/**
 * maps MUC event types to icon names for retrieving the "big" icons
 * while generating task bar thumbnails.
 * used by getMUCBigICon()
 */

CTaskbarInteract* Win7Taskbar = 0;

/**
 * set the overlay icon for a task bar button. Used for typing notifications and incoming
 * message indicator.
 *
 * @param 	hwndDlg HWND: 	container window handle
 * @param 	lParam  LPARAM:	icon handle
 * @return	true if icon has been set and taskbar will accept it, false otherwise
 */
bool CTaskbarInteract::setOverlayIcon(HWND hwndDlg, LPARAM lParam) const
{
	if (m_pTaskbarInterface && m_isEnabled && m_fHaveLargeicons) {
		m_pTaskbarInterface->SetOverlayIcon(hwndDlg,(HICON)lParam, NULL);
		return true;
	}
	return false;
}

/**
 * check the task bar status for "large icon mode".
 * @return bool: true if large icons are in use, false otherwise
 */
bool CTaskbarInteract::haveLargeIcons()
{
	m_fHaveLargeicons = false;

	if (m_pTaskbarInterface && m_isEnabled) {
		HKEY 	hKey;
		DWORD 	val = 1;
		DWORD	valGrouping = 2;
		DWORD	size = 4;
		DWORD	dwType = REG_DWORD;
		/*
		 * check whether the taskbar is set to show large icons. This is necessary, because the method SetOverlayIcon()
		 * always returns S_OK, but the icon is simply ignored when using small taskbar icons.
		 * also, figure out the button grouping mode.
		 */
		if (::RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), &hKey) == ERROR_SUCCESS) {
			::RegQueryValueEx(hKey, _T("TaskbarSmallIcons"), 0, &dwType, (LPBYTE)&val, &size);
			size = 4;
			dwType = REG_DWORD;
			/*
			 * this is the "grouping mode" setting for the task bar. 0 = always combine, no labels
			 */
			::RegQueryValueEx(hKey, _T("TaskbarGlomLevel"), 0, &dwType, (LPBYTE)&valGrouping, &size);
			::RegCloseKey(hKey);
		}
		m_fHaveLargeicons = (val ? false : true);			// small icons in use, revert to default icon feedback
		m_fHaveAlwaysGrouping = (valGrouping == 0 ? true : false);
	}
	return(m_fHaveLargeicons);
}

/**
 * removes the overlay icon for the given container window
 * @param hwndDlg HWND: window handle
 */
void CTaskbarInteract::clearOverlayIcon(HWND hwndDlg) const
{
	if (m_pTaskbarInterface && m_isEnabled)
		m_pTaskbarInterface->SetOverlayIcon(hwndDlg, NULL, NULL);
}

LONG CTaskbarInteract::updateMetrics()
{
	m_IconSize = 32;

	return(m_IconSize);
}

/**
 * register a new task bar button ("tab") for the button group hwndContainer
 * (one of the top level message windows)
 * @param hwndTab			proxy window handle
 * @param hwndContainer		a message container window
 */
void CTaskbarInteract::registerTab(const HWND hwndTab, const HWND hwndContainer) const
{
	if (m_isEnabled) {
		m_pTaskbarInterface->RegisterTab(hwndTab, hwndContainer);
		m_pTaskbarInterface->SetTabOrder(hwndTab, 0);
	}
}

/**
 * remove a previously registered proxy window. The destructor of the proxy
 * window class is using this before destroying the proxy window itself.
 * @param hwndTab	proxy window handle
 */
void CTaskbarInteract::unRegisterTab(const HWND hwndTab) const
{
	if (m_isEnabled)
		m_pTaskbarInterface->UnregisterTab(hwndTab);
}

/**
 * set a tab as active. The active thumbnail will appear with a slightly
 * different background and transparency.
 *
 * @param hwndTab			window handle of the PROXY window to activate
 * 							(don't use the real window handle of the message
 * 							tab, because it is not a toplevel window).
 * @param hwndGroup			window group to which it belongs (usually, the
 * 							container window handle).
 */
void CTaskbarInteract::SetTabActive(const HWND hwndTab, const HWND hwndGroup) const
{
	if (m_isEnabled)
		m_pTaskbarInterface->SetTabActive(hwndTab, hwndGroup, 0);
}

/**
 * create a proxy window object for the given session. Do NOT call this more than once
 * per session and not outside of WM_INITDIALOG after most things are initialized.
 * @param dat		session window data
 *
 * static member function. Ignored when OS is not Windows 7 or global option for
 * Windows 7 task bar support is diabled.
 */
void CProxyWindow::add(TWindowData *dat)
{
	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek) // && (!CSkin::m_skinEnabled || M.GetByte("forceAeroPeek", 0)))
		dat->pWnd = new CProxyWindow(dat);
	else
		dat->pWnd = 0;
}

/**
 * This is called from the broadcasted WM_DWMCOMPOSITIONCHANGED event by all messages
 * sessions. It checks and, if needed, destroys or creates a proxy object, based on
 * the status of the DWM
 * @param dat		session window data
 *
 * static member function
 */
void CProxyWindow::verify(TWindowData *dat)
{
	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek) {
		if (0 == dat->pWnd) {
			dat->pWnd = new CProxyWindow(dat);
			if (dat->pWnd) {
				dat->pWnd->updateIcon(dat->hTabStatusIcon);
				dat->pWnd->updateTitle(dat->cache->getNick());
			}
		}
		else
			dat->pWnd->verifyDwmState();
	}
	/*
	 * this should not happens, but who knows...
	 */
	else {
		if (dat->pWnd) {
			delete dat->pWnd;
			dat->pWnd = 0;
		}
	}
}

/**
 * create the proxy (toplevel) window required to show per tab thumbnails
 * and previews for a message session.
 * each tab has one invisible proxy window
 */
CProxyWindow::CProxyWindow(const TWindowData *dat)
{
	m_dat = dat;
	m_hBigIcon = 0;
	m_thumb = 0;

	m_hwndProxy = ::CreateWindowEx(/*WS_EX_TOOLWINDOW | */WS_EX_NOACTIVATE, PROXYCLASSNAME, _T(""),
		WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION, -32000, -32000, 10, 10, NULL, NULL, g_hInst, (LPVOID)this);

#if defined(__LOGDEBUG_)
	_DebugTraceW(_T("create proxy object for: %s"), m_dat->cache->getNick());
#endif
	Win7Taskbar->registerTab(m_hwndProxy, m_dat->pContainer->hwnd);
	if (CMimAPI::m_pfnDwmSetWindowAttribute) {
		BOOL	fIconic = TRUE;
		BOOL	fHasIconicBitmap = TRUE;

		CMimAPI::m_pfnDwmSetWindowAttribute(m_hwndProxy, DWMWA_FORCE_ICONIC_REPRESENTATION, &fIconic,  sizeof(fIconic));
		CMimAPI::m_pfnDwmSetWindowAttribute(m_hwndProxy, DWMWA_HAS_ICONIC_BITMAP, &fHasIconicBitmap, sizeof(fHasIconicBitmap));
	}
}

CProxyWindow::~CProxyWindow()
{
	Win7Taskbar->unRegisterTab(m_hwndProxy);
	::DestroyWindow(m_hwndProxy);

#if defined(__LOGDEBUG_)
	_DebugTraceW(_T("destroy proxy object for: %s"), m_dat->cache->getNick());
#endif
	if (m_thumb) {
		delete m_thumb;
		m_thumb = 0;
	}
}

/**
 * verify status of DWM when system broadcasts a WM_DWMCOMPOSITIONCHANGED message
 * delete thumbnails, if no longer needed
 */
void CProxyWindow::verifyDwmState()
{
	if (!M.isDwmActive()) {
		if (m_thumb) {
			delete m_thumb;
			m_thumb = 0;
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

/**
 * send a thumbnail to the DWM. If required, refresh it first.
 * called by WM_DWMSENDICONICTHUMBNAIL handler.
 *
 * @param width		thumbnail width as requested by DWM via lParam
 * @param height	thumbnail height as requested by DWM via lParam
 */
void CProxyWindow::sendThumb(LONG width, LONG height)
{
	if (0 == m_thumb) {
		m_width = width;
		m_height = height;
		if (m_dat->bType == SESSIONTYPE_IM)
			m_thumb = new CThumbIM(this);
		else
			m_thumb = new CThumbMUC(this);
	}
	else if (width != m_width || height != m_height || !m_thumb->isValid()) {
		m_width = width;
		m_height = height;
		m_thumb->update();
	}
	if (m_thumb)
		CMimAPI::m_pfnDwmSetIconicThumbnail(m_hwndProxy, m_thumb->getHBM(), DWM_SIT_DISPLAYFRAME);
}

/**
 * send a live preview image of a given message session to the DWM.
 * called by WM_DWMSENDICONICLIVEPREVIEWBITMAP on DWM's request.
 *
 * The bitmap can be deleted after submitting it, because the DWM
 * will cache a copy of it (and re-request it when its own bitmap cache
 * was purged).
 */
void CProxyWindow::sendPreview()
{
	if (m_dat->pContainer == NULL)
		return;

	TWindowData *dat_active = reinterpret_cast<TWindowData *>(::GetWindowLongPtr(m_dat->pContainer->hwndActive, GWLP_USERDATA));
	if (!m_thumb || !dat_active)
		return;

	FORMATRANGE fr = {0};
	POINT pt = {0};
	RECT rcContainer, rcTemp, rcRich, rcLog;
	HDC hdc, dc;
	int twips = (int)(15.0f / PluginConfig.g_DPIscaleY);
	bool fIsChat = m_dat->bType != SESSIONTYPE_IM;
	HWND 	hwndRich = ::GetDlgItem(m_dat->hwnd, fIsChat ? IDC_CHAT_LOG : IDC_LOG);
	LONG 	cx, cy;
	POINT	ptOrigin = {0}, ptBottom;

	if (m_dat->dwFlags & MWF_NEEDCHECKSIZE) {
		RECT	rcClient;

		::SendMessage(m_dat->pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
		::MoveWindow(m_dat->hwnd, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), FALSE);
		::SendMessage(m_dat->hwnd, WM_SIZE, 0, 0);
		::SendMessage(m_dat->hwnd, DM_FORCESCROLL, 0, 0);
	}
	/*
		* a minimized container has a null rect as client area, so do not use it
		* use the last known client area size instead.
		*/

	if (!::IsIconic(m_dat->pContainer->hwnd)) {
		::GetWindowRect(m_dat->pContainer->hwndActive, &rcLog);
		::GetClientRect(m_dat->pContainer->hwnd, &rcContainer);
		pt.x = rcLog.left;
		pt.y = rcLog.top;
		::ScreenToClient(m_dat->pContainer->hwnd, &pt);
	}
	else {
		rcLog = m_dat->pContainer->rcLogSaved;
		rcContainer = m_dat->pContainer->rcSaved;
		pt = m_dat->pContainer->ptLogSaved;
	}

	::GetWindowRect(::GetDlgItem(m_dat->pContainer->hwndActive, dat_active->bType == SESSIONTYPE_IM ? IDC_LOG : IDC_CHAT_LOG), &rcTemp);
	ptBottom.x = rcTemp.left;
	ptBottom.y = rcTemp.bottom;
	::ScreenToClient(m_dat->pContainer->hwnd, &ptBottom);

	cx = rcLog.right - rcLog.left;
	cy = rcLog.bottom - rcLog.top;
	rcRich.left = 0;
	rcRich.top = 0;
	rcRich.right = cx;
	rcRich.bottom = ptBottom.y - pt.y;

	dc = ::GetDC(m_dat->hwnd);
	hdc = ::CreateCompatibleDC(dc);
	HBITMAP hbm = CSkin::CreateAeroCompatibleBitmap(rcContainer, hdc);
	HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(hdc, hbm));

	HBRUSH brb = ::CreateSolidBrush(RGB(20, 20, 20));
	::FillRect(hdc, &rcContainer, brb);
	::DeleteObject(brb);
	CImageItem::SetBitmap32Alpha(hbm, 100);

	LRESULT first = ::SendMessage(hwndRich, EM_CHARFROMPOS, 0, LPARAM(&ptOrigin));

	/*
		* paint the content of the message log control into a separate bitmap without
		* transparency
		*/
	HDC hdcRich = ::CreateCompatibleDC(dc);
	HBITMAP hbmRich = CSkin::CreateAeroCompatibleBitmap(rcRich, hdcRich);
	HBITMAP hbmRichOld = reinterpret_cast<HBITMAP>(::SelectObject(hdcRich, hbmRich));

	COLORREF clr = fIsChat ? M.GetDword(FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR) : m_dat->pContainer->theme.inbg;
	HBRUSH br = ::CreateSolidBrush(clr);
	::FillRect(hdcRich, &rcRich, br);
	::DeleteObject(br);

	if (m_dat->hwndIEView)
		::SendMessage(m_dat->hwndIEView, WM_PRINT, reinterpret_cast<WPARAM>(hdcRich), PRF_CLIENT | PRF_NONCLIENT);
	else if (m_dat->hwndHPP) {
		CSkin::RenderText(hdcRich, m_dat->hTheme, TranslateT("Previews not availble when using History++ plugin for message log display."),
							&rcRich, DT_VCENTER | DT_CENTER | DT_WORDBREAK, 10, m_dat->pContainer->theme.fontColors[MSGFONTID_MYMSG], false);
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

	CMimAPI::m_pfnDwmSetIconicLivePreviewBitmap(m_hwndProxy, hbm, &pt, m_dat->pContainer->dwFlags & CNT_CREATE_MINIMIZED ? 0 : DWM_SIT_DISPLAYFRAME);
	::ReleaseDC(m_dat->hwnd, dc);
	::DeleteObject(hbm);
}

/**
 * set the large icon for the thumbnail. This is mostly used by group chats
 * to indicate last active event in the session.
 *
 * hIcon may be 0 to remove a custom big icon. In that case, the renderer
 * will try to figure out a suitable one, based on session data.
 *
 * @param hIcon			icon handle (should be a 32x32 icon)
 * @param fInvalidate	invalidate the thumbnail (default value = true)
 */
void CProxyWindow::setBigIcon(const HICON hIcon, bool fInvalidate)
{
	m_hBigIcon = hIcon;
	if (fInvalidate)
		Invalidate();
}

/**
 * set a overlay icon for the thumbnail. This is mostly used by group chats
 * to indicate last active event in the session.
 *
 * hIcon may be 0 to remove a custom overlay icon.
 *
 * @param hIcon			icon handle (should be a 16x16 icon)
 * @param fInvalidate	invalidate the thumbnail (default value = true)
 */
void CProxyWindow::setOverlayIcon(const HICON hIcon, bool fInvalidate)
{
	m_hOverlayIcon = hIcon;
	if (fInvalidate)
		Invalidate();
}

/**
 * update the (small) thumbnail icon in front of the title string
 * @param hIcon		new icon handle
 */
void CProxyWindow::updateIcon(const HICON hIcon) const
{
	if (m_hwndProxy && hIcon)
		::SendMessage(m_hwndProxy, WM_SETICON, ICON_SMALL, LPARAM(hIcon));
}

/**
 * set the task bar button ("tab") active. This activates the proxy
 * window as a sub-window of the (top level) container window.
 * This is called whenever the active message tab or window changes
 */
void CProxyWindow::activateTab() const
{
	Win7Taskbar->SetTabActive(m_hwndProxy, m_dat->pContainer->hwnd);
}
/**
 * invalidate the thumbnail, it will be recreated at the next request
 * by the DWM
 *
 * this is called from several places whenever a relevant information,
 * represented in a thumbnail image, has changed.
 *
 * also tell the DWM that it must request a new thumb.
 */
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
void CProxyWindow::updateTitle(const TCHAR *tszTitle) const
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
	CProxyWindow* pWnd = reinterpret_cast<CProxyWindow *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (pWnd)
		return(pWnd->wndProc(hWnd, msg, wParam, lParam));

	switch(msg) {
	case WM_NCCREATE:
		CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
		CProxyWindow *pWnd = reinterpret_cast<CProxyWindow *>(cs->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		return pWnd->wndProc(hWnd, msg, wParam, lParam);
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

/**
 * window procedure for the proxy window
 */
LRESULT CALLBACK CProxyWindow::wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

#if defined(__LOGDEBUG_)
	case WM_NCCREATE:
		_DebugTraceW(_T("create proxy WINDOW for: %s"), m_dat->cache->getNick());
		break;
#endif
	case WM_CLOSE:
		{
			TContainerData* pC = m_dat->pContainer;

			if (m_dat->hwnd != pC->hwndActive)
				SendMessage(m_dat->hwnd, WM_CLOSE, 1, 3);
			else
				SendMessage(m_dat->hwnd, WM_CLOSE, 1, 2);
			if (!IsIconic(pC->hwnd))
				SetForegroundWindow(pC->hwnd);
		}
		return 0;

		/*
		* proxy window was activated by clicking on the thumbnail. Send this
		* to the real message window.
		*/
	case WM_ACTIVATE:
		if (WA_ACTIVE == wParam) {
			if (IsWindow(m_dat->hwnd))
				::PostMessage(m_dat->hwnd, DM_ACTIVATEME, 0, 0);
			return 0;			// no default processing, avoid flickering.
		}
		break;

	case WM_NCDESTROY:
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
#if defined(__LOGDEBUG_)
		_DebugTraceW(_T("destroy proxy WINDOW for: %s"), m_dat->cache->getNick());
#endif
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

/**
 * base thumbnail class. Create the background and common parts for a
 * thumbnail
 *
 * @param _p			 owner proxy window object
 * @return
 */
CThumbBase::CThumbBase(const CProxyWindow* _p)
{
	m_pWnd = _p;
	m_hbmThumb = 0;
	renderBase();
}

/**
 * render base for a thumbnail. This creates the background, the large icon
 * and the basic status mode text. It also divides the thumbnail rectangle
 * into a few content rectangles used later by the content renderer.
 */
void CThumbBase::renderBase()
{
	HICON	hIcon = 0;
	HBRUSH	brBack;
	LONG	lIconSize = 32;

	m_width = m_pWnd->getWidth();
	m_height = m_pWnd->getHeight();
	m_dat = m_pWnd->getDat();
	m_dtFlags = 0;
	m_hOldFont = 0;

#if defined(__LOGDEBUG_)
	_DebugTraceW(_T("refresh base (background) with %d, %d"), m_width, m_height);
#endif

	m_rc.right = m_width;
	m_rc.bottom = m_height;
	m_rc.left = m_rc.top = 0;

	if (m_hbmThumb) {
		::DeleteObject(m_hbmThumb);
		m_hbmThumb = 0;
	}

	HDC dc = ::GetDC(m_pWnd->getHwnd());
	m_hdc = ::CreateCompatibleDC(dc);

	m_hbmThumb = CSkin::CreateAeroCompatibleBitmap(m_rc, m_hdc);
	m_hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(m_hdc, m_hbmThumb));
	ReleaseDC(m_pWnd->getHwnd(), dc);

	brBack = ::CreateSolidBrush(m_dat->dwUnread ? RGB(80, 60, 60) : RGB(60, 60, 60));
	::FillRect(m_hdc, &m_rc, brBack);
	::DeleteObject(brBack);

	::SelectObject(m_hdc, m_hbmOld);
	CImageItem::SetBitmap32Alpha(m_hbmThumb, m_dat->dwUnread ? 110 : 60);
	m_hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(m_hdc, m_hbmThumb));

	SetBkMode(m_hdc, TRANSPARENT);

	m_hOldFont = reinterpret_cast<HFONT>(::SelectObject(m_hdc, CInfoPanel::m_ipConfig.hFonts[IPFONTID_STATUS]));
	::GetTextExtentPoint32A(m_hdc, "A", 1, &m_sz);

	InflateRect(&m_rc, -3, -3);

	setupRect();
	hIcon = m_pWnd->getBigIcon();

	if (0 == hIcon) {
		if (m_dat->dwUnread) {
			if (PluginConfig.g_IconMsgEventBig)
				hIcon = PluginConfig.g_IconMsgEventBig;
			else {
				hIcon = PluginConfig.g_IconMsgEvent;
				lIconSize = 16;
			}
		}
		else {
			hIcon = reinterpret_cast<HICON>(LoadSkinnedProtoIconBig(m_dat->cache->getActiveProto(), m_dat->cache->getActiveStatus()));
			if (0 == hIcon || reinterpret_cast<HICON>(CALLSERVICE_NOTFOUND) == hIcon) {
				hIcon = reinterpret_cast<HICON>(LoadSkinnedProtoIcon(m_dat->cache->getActiveProto(), m_dat->cache->getActiveStatus()));
				lIconSize = 16;
			}
		}
	}
	::DrawIconEx(m_hdc, m_rcIcon.right / 2 - lIconSize / 2, m_rcIcon.top, hIcon, lIconSize, lIconSize, 0, 0, DI_NORMAL);
	hIcon = m_pWnd->getOverlayIcon();
	if (hIcon)
		::DrawIconEx(m_hdc, m_rcIcon.right - 16, m_rcIcon.top + 16, hIcon, 16, 16, 0, 0, DI_NORMAL);

	m_rcIcon.top += (lIconSize + 3);
	CSkin::RenderText(m_hdc, m_dat->hTheme, m_dat->szStatus, &m_rcIcon, m_dtFlags | DT_CENTER | DT_WORD_ELLIPSIS, 10, 0, true);
	if (m_dat->dwUnread && SESSIONTYPE_IM == m_dat->bType) {
		wchar_t	tszTemp[30];

		m_rcIcon.top += m_sz.cy;
		mir_sntprintf(tszTemp, 30, TranslateT("%d Unread"), m_dat->dwUnread);
		CSkin::RenderText(m_hdc, m_dat->hTheme, tszTemp, &m_rcIcon, m_dtFlags | DT_CENTER | DT_WORD_ELLIPSIS, 10, 0, true);
	}
	m_rcIcon= m_rcTop;
	m_rcIcon.top += 2;
	m_rcIcon.left = m_rc.right / 3;
	m_cx = m_rcIcon.right - m_rcIcon.left;
	m_cy = m_rcIcon.bottom - m_rcIcon.top;
}

/**
 * divide space into content rectangles for normal thumbnails
 */
void CThumbBase::setupRect()
{
	if (SESSIONTYPE_IM == m_pWnd->getDat()->bType) {
		m_rcTop = m_rc;
		m_rcBottom = m_rc;
		m_rcBottom.top = m_rc.bottom - ( 2 * (m_rcBottom.bottom / 5)) - 2;
		m_rcTop.bottom = m_rcBottom.top - 2;

		m_rcIcon = m_rcTop;
		m_rcIcon.right = m_rc.right / 3;
	}
	else {
		m_rcTop = m_rc;
		m_rcBottom = m_rc;
		m_rcBottom.top = m_rc.bottom - ( 2 * (m_rcBottom.bottom / 5)) - 2;
		m_rcTop.bottom = m_rcBottom.top - 2;

		m_rcIcon = m_rcTop;
		m_rcIcon.right = m_rc.left + 42;
	}
}

/**
 * destroy the thumbnail object. Just delete the bitmap we cached
 * @return
 */
CThumbBase::~CThumbBase()
{
	if (m_hbmThumb) {
		::DeleteObject(m_hbmThumb);
		m_hbmThumb = 0;
		m_isValid = false;
	}
#if defined(__LOGDEBUG_)
	_DebugTraceW(_T("destroy CThumbBase"));
#endif
}

/**
 * create a IM session thumbnail. base class will create the
 * bitmap and render the background.
 *
 * @param _p	our owner (CProxyWindow object)
 */
CThumbIM::CThumbIM(const CProxyWindow* _p) : CThumbBase(_p)
{
	renderContent();
	setValid(true);
}

/**
 * update the thumbnail, render everything and set it valid
 */
void CThumbIM::update()
{
	renderBase();
	renderContent();
	setValid(true);
}

/**
 * render the content for an IM chat session thumbnail
 * m_hdc etc. must already be initialized (done by the constructor)
 * background had been already rendered
 */
void CThumbIM::renderContent()
{
	HBITMAP			hbmAvatar, hbmOldAv;
	double			dNewWidth = 0.0, dNewHeight = 0.0;
	bool			fFree = false;
	HRGN			hRgn = 0;
	HDC				dc;
	const wchar_t*	tszStatusMsg = 0;

	hbmAvatar = (m_dat->ace && m_dat->ace->hbmPic) ? m_dat->ace->hbmPic : PluginConfig.g_hbmUnknown;
	Utils::scaleAvatarHeightLimited(hbmAvatar, dNewWidth, dNewHeight, m_rcIcon.bottom - m_rcIcon.top);

	HBITMAP hbmResized = CSkin::ResizeBitmap(hbmAvatar, dNewWidth, dNewHeight, fFree);

	dc = CreateCompatibleDC(m_hdc);
	hbmOldAv = reinterpret_cast<HBITMAP>(::SelectObject(dc, hbmResized));

	LONG xOff = m_rcIcon.right - (LONG)dNewWidth - 2;
	LONG yOff = (m_cy - (LONG)dNewHeight) / 2 + m_rcIcon.top;

	hRgn = ::CreateRectRgn(xOff - 1, yOff - 1, xOff + (LONG)dNewWidth + 2, yOff + (LONG)dNewHeight + 2);
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

	/*
	 * status message and bottom line (either UID or nick name, depending on
	 * task bar grouping mode). For chat rooms, it is the topic.
	 */
	if ((m_rcBottom.bottom - m_rcBottom.top) < 2 * m_sz.cy)
		m_dtFlags |= DT_SINGLELINE;

	m_rcBottom.bottom -= ((m_rcBottom.bottom - m_rcBottom.top) % m_sz.cy);		// adjust to a multiple of line height

	if (0 == (tszStatusMsg = m_dat->cache->getStatusMsg()))
		tszStatusMsg = TranslateT("No status message");

	CSkin::RenderText(m_hdc, m_dat->hTheme, tszStatusMsg, &m_rcBottom, DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | m_dtFlags, 10, 0, true);
	m_rcBottom.bottom = m_rc.bottom;
	m_rcBottom.top = m_rcBottom.bottom - m_sz.cy - 2;
	CSkin::RenderText(m_hdc, m_dat->hTheme, Win7Taskbar->haveAlwaysGroupingMode() ? m_dat->cache->getUIN() : m_dat->cache->getNick(),
					  &m_rcBottom, m_dtFlags | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS, 10, 0, true);

	/*
	 * finalize it
	 * do NOT delete the bitmap, the dwm will need the handle
	 * m_hbm is deleted when a new thumbnail is generated on dwm's request.
	 * this is not a leak!
	 */
	if (m_hOldFont)
		::SelectObject(m_hdc, m_hOldFont);

	::SelectObject(m_hdc, m_hbmOld);
	::DeleteDC(m_hdc);
}

/**
 * create a MUC session thumbnail. base class will create the
 * bitmap and render the background.
 *
 * @param _p	our owner (CProxyWindow object)
 * @return
 */
CThumbMUC::CThumbMUC(const CProxyWindow* _p) : CThumbBase(_p)
{
	renderContent();
	setValid(true);
}

/**
 * update an invalidated thumbnail
 */
void CThumbMUC::update()
{
	renderBase();
	renderContent();
	setValid(true);
}

/**
 * render content area for a MUC thumbnail
 */
void CThumbMUC::renderContent()
{
	if (m_dat->si) {
		const MODULEINFO*	mi = pci->MM_FindModule(m_dat->si->pszModule);
		wchar_t				szTemp[250];
		const wchar_t*		szStatusMsg = 0;

		if (mi) {
			if (m_dat->dwUnread) {
				mir_sntprintf(szTemp, 30, TranslateT("%d Unread"), m_dat->dwUnread);
				CSkin::RenderText(m_hdc, m_dat->hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
				m_rcIcon.top += m_sz.cy;
			}
			if (m_dat->si->iType != GCW_SERVER) {
				wchar_t* _p = NULL;
				if ( m_dat->si->ptszStatusbarText )
					_p = wcschr(m_dat->si->ptszStatusbarText, ']');
				if ( _p ) {
					_p++;
					wchar_t	_t = *_p;
					*_p = 0;
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Chat room %s"), m_dat->si->ptszStatusbarText);
					*_p = _t;
				}
				else
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Chat room %s"), L"");
				CSkin::RenderText(m_hdc, m_dat->hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
				m_rcIcon.top += m_sz.cy;
				mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("%d User(s)"), m_dat->si->nUsersInNicklist);
				CSkin::RenderText(m_hdc, m_dat->hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
			}
			else {
				mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("Server window"));
				CSkin::RenderText(m_hdc, m_dat->hTheme, szTemp, &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
				if (mi->tszIdleMsg[0] && _tcslen(mi->tszIdleMsg) > 2) {
					m_rcIcon.top += m_sz.cy;
					CSkin::RenderText(m_hdc, m_dat->hTheme, &mi->tszIdleMsg[2], &m_rcIcon, m_dtFlags | DT_SINGLELINE | DT_RIGHT, 10, 0, true);
				}
			}
		}

		if ((m_rcBottom.bottom - m_rcBottom.top) < 2 * m_sz.cy)
			m_dtFlags |= DT_SINGLELINE;

		m_rcBottom.bottom -= ((m_rcBottom.bottom - m_rcBottom.top) % m_sz.cy);		// adjust to a multiple of line height

		if (m_dat->si->iType != GCW_SERVER) {
			if (0 == (szStatusMsg = m_dat->si->ptszTopic))
				szStatusMsg = TranslateT("no topic set.");
		}
		else if (mi) {
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("%s on %s%s"), m_dat->szMyNickname, mi->ptszModDispName, L"");
			szStatusMsg = szTemp;
		}

		CSkin::RenderText(m_hdc, m_dat->hTheme, szStatusMsg, &m_rcBottom, DT_WORD_ELLIPSIS | DT_END_ELLIPSIS | m_dtFlags, 10, 0, true);
	}
	/*
	 * finalize it
	 * do NOT delete the bitmap, the dwm will need the handle
	 * m_hbm is deleted when a new thumbnail is generated on dwm's request.
	 * this is not a leak!
	 */
	if (m_hOldFont)
		::SelectObject(m_hdc, m_hOldFont);

	::SelectObject(m_hdc, m_hbmOld);
	::DeleteDC(m_hdc);
}
