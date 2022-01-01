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
// - Windows 7 taskbar integration class
// - Proxy window class, needed to support custom aero peek tab
//   thumbnails
// - Thumbnail classes to provide task bar thumbnails for Aero peek
//   preview.

#ifndef __TASKBAR_H
#define __TASKBAR_H

#define PROXYCLASSNAME  L"TabSRMM_DWMProxy"

class CProxyWindow;

class CThumbBase {
public:
	CThumbBase(const CProxyWindow* pWnd);
	virtual ~CThumbBase();

	__inline const HBITMAP getHBM() const { return m_hbmThumb; }
	__inline const bool    isValid() const { return m_isValid; }

	virtual void  setValid(const bool fNewValid) { m_isValid = fNewValid; }
	virtual void  update() = 0;

protected:
	const CMsgDialog *m_dat;
	const CProxyWindow *m_pWnd;

	HBITMAP m_hbmThumb, m_hbmOld;
	LONG    m_width, m_height;
	HDC     m_hdc;
	RECT    m_rc, m_rcTop, m_rcBottom, m_rcIcon;
	uint32_t   m_dtFlags;
	SIZE    m_sz;
	LONG    m_cx, m_cy;
	HFONT   m_hOldFont;

	virtual void renderBase();
	virtual void renderContent() = 0;

private:
	void setupRect();

private:
	bool m_isValid;
};

class CThumbIM : public CThumbBase
{
public:
	CThumbIM(const CProxyWindow* pWnd);
	virtual ~CThumbIM() {};
	void update();

private:
	virtual void renderContent() override;
};

class CThumbMUC : public CThumbBase
{
	SESSION_INFO *si;

public:
	CThumbMUC(const CProxyWindow* pWnd, SESSION_INFO*);
	virtual ~CThumbMUC() {};
	void update();

private:
	virtual void renderContent() override;
};

class CProxyWindow : public MZeroedObject
{
	CMsgDialog *m_dat;

	HWND m_hwndProxy;
	LONG m_width, m_height;
	HICON m_hBigIcon, m_hOverlayIcon;

	LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void sendThumb(LONG width, LONG height);
	void sendPreview();
	CThumbBase *m_thumb;

public:
	CProxyWindow(CMsgDialog *dat);
	~CProxyWindow();

	void updateIcon(const HICON hIcon) const;
	void updateTitle(const wchar_t *tszTitle) const;
	void setBigIcon(const HICON hIcon, bool fInvalidate = true);
	void setOverlayIcon(const HICON hIcon, bool fInvalidate = true);
	void activateTab() const;
	void Invalidate() const;
	void verifyDwmState();

	__inline const CMsgDialog* getDat() const { return m_dat; }
	__inline const LONG getWidth() const { return m_width; }
	__inline const LONG getHeight() const { return m_height; }
	__inline const HWND getHwnd() const { return m_hwndProxy; }
	__inline const HICON getBigIcon() const { return m_hBigIcon; }
	__inline const HICON getOverlayIcon() const { return m_hOverlayIcon; }

	static LRESULT CALLBACK stubWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

class CTaskbarInteract
{
public:
	CTaskbarInteract()
	{
		m_pTaskbarInterface = nullptr;
		m_IconSize = 0;
		m_isEnabled = IsWinVer7Plus() ? true : false;

		if (m_isEnabled) {
			::CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&m_pTaskbarInterface);
			updateMetrics();
			if (nullptr == m_pTaskbarInterface)
				m_isEnabled = false;
		}

		/*
		* register proxy window class
		*/
		WNDCLASSEX wcex = { sizeof(wcex) };
		wcex.lpfnWndProc = CProxyWindow::stubWndProc;
		wcex.hInstance = g_plugin.getInst();
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszClassName = PROXYCLASSNAME;
		::RegisterClassEx(&wcex);
	}

	~CTaskbarInteract()
	{
		if (m_isEnabled && m_pTaskbarInterface) {
			m_pTaskbarInterface->Release();
			m_pTaskbarInterface = nullptr;
			m_isEnabled = false;
		}
		::UnregisterClass(PROXYCLASSNAME, g_plugin.getInst());
	}
	const LONG getIconSize() const { return m_IconSize; }
	const bool haveAlwaysGroupingMode() const { return m_fHaveAlwaysGrouping; }

	bool setOverlayIcon(HWND hwndDlg, LPARAM lParam) const;
	void clearOverlayIcon(HWND hwndDlg) const;
	bool haveLargeIcons();
	LONG updateMetrics();
	void registerTab(const HWND hwndTab, const HWND hwndContainer) const;
	void unRegisterTab(const HWND hwndTab) const;
	void SetTabActive(const HWND hwndTab, const HWND hwndGroup) const;

	//const wchar_t*	getFileNameFromWindow			(const HWND hWnd);
private:
	ITaskbarList3 *m_pTaskbarInterface;

	bool m_isEnabled;
	bool m_fHaveLargeicons;
	bool m_fHaveAlwaysGrouping;
	LONG m_IconSize;
};

extern CTaskbarInteract *Win7Taskbar;

#endif /* __TASKBAR_H */
