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
// menu bar and status bar classes for the container window.

#ifndef __CONTROLS_H
#define __CONTROLS_H

extern LONG_PTR CALLBACK StatusBarSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class CMenuBar
{
public:
	enum {
		NR_BUTTONS = 8
	};

	CMenuBar(HWND hwndParent, const TContainerData *pContainer);
	~CMenuBar();

	const RECT&  getClientRect();
	void         Resize(WORD wWidth) const
	{
		::SetWindowPos(m_hwndToolbar, 0, 4, 0, wWidth, m_size_y, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW);
	}

	LONG         getHeight() const;
	void         Show(int showCmd) const
	{
		::ShowWindow(m_hwndToolbar, showCmd);
	}
	LONG_PTR     Handle(const NMTOOLBAR *nmtb);
	void         Cancel();
	LONG_PTR     processMsg(const UINT msg, const WPARAM wParam, const LPARAM lParam);
	bool         isContactMenu() const { return(m_isContactMenu); }
	bool         isMainMenu() const { return(m_isMainMenu); }
	void         configureMenu(void) const;
	void         resetLP(void);
	void         setActive(HMENU hMenu)
	{
		m_activeSubMenu = hMenu;
	}
	void			setAero(bool fState) { m_isAero = fState; }
	const bool		getAero(void) const { return(m_isAero); }

	const LRESULT	processAccelerator(TCHAR a, UINT& ctlId) const
	{
		UINT		_ctlId;

		const LRESULT result = ::SendMessage(m_hwndToolbar, TB_MAPACCELERATOR, (WPARAM)a, (LPARAM)&_ctlId);
		ctlId = _ctlId;

		return(result);
	}
	void autoShow(const int showcmd = 1);

	const int	idToIndex(const int id) const
	{
		for (int i=0; i < NR_BUTTONS; i++) {
			if (m_TbButtons[i].idCommand == id )
				return(i);
		}
		return -1;
	}
public:
	static   HHOOK   m_hHook;
   static   HBITMAP m_MimIcon;

private:
   HWND     m_hwndToolbar;
   RECT     m_rcClient;
   TContainerData *m_pContainer;
   HMENU    m_activeMenu, m_activeSubMenu;
   int      m_activeID;
   bool     m_fTracking;
   bool     m_isContactMenu;
   bool     m_isMainMenu;
   bool     m_isAero;
   bool     m_mustAutoHide;
   LONG     m_size_y;

   /*
    * for custom drawing
    */
   RECT     m_rcItem;
   HDC      m_hdcDraw;
   HBITMAP  m_hbmDraw, m_hbmOld;
   HANDLE   m_hTheme;
   HFONT    m_hOldFont;

   static   TBBUTTON m_TbButtons[8];
   static   bool m_buttonsInit;
   static   CMenuBar *m_Owner;
   static   int m_MimIconRefCount;
private:
	LONG_PTR	 customDrawWorker(NMCUSTOMDRAW *nm);
	void		 updateState(const HMENU hMenu) const;
	void		 invoke(const int id);
	void		 cancel();
	void 		 obtainHook();
	void		 releaseHook();
	void      checkButtons();

	static 		LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);		// subclassing for the toolbar control
	static 		LRESULT CALLBACK MessageHook(int nCode, WPARAM wParam, LPARAM lParam);				// message hook (only active when modal menus are active)
};

#endif /* __CONTROLS_H */
