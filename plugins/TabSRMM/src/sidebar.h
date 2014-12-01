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
// the contact switch bar on the left (or right) side

#ifndef __SIDEBAR_H
#define  __SIDEBAR_H

struct TSideBarNotify
{
	NMHDR 				nmHdr;
	const TWindowData*	dat;
};
/* layout description structure */

struct TSideBarLayout
{
	TCHAR szName[50];  // everything wants a name...
	LONG  width;       // width of the switchbar element (a single button)
	LONG  height;      // height of a single switchbar element
	DWORD dwFlags;     // flags, obviously :)

	/*
	 * the following 4 items define pointers to the actual renderer functions for that sidebar layout
	 * a default is always provided, however, it has been designed to be easily extendible without
	 * rewriting lots of code just in order to change how the switchbar items look like.
	 */
	void		(__fastcall *pfnContentRenderer)(const HDC hdc, const RECT *rc, const CSideBarButton *item);
	void		(__fastcall *pfnBackgroundRenderer)(const HDC hdc, const RECT *rc, const CSideBarButton *item);
	const SIZE&	(__fastcall *pfnMeasureItem)(CSideBarButton *item);
	void		(__fastcall *pfnLayout)(const CSideBar *sideBar, RECT *rc);
	UINT		uId;						// numeric id by which the layout is identified. basically, the index into the array.
};

class CSideBar;

class CSideBarButton
{
public:
	CSideBarButton(const UINT id, CSideBar *sideBar);
	CSideBarButton(const TWindowData *dat, CSideBar *sideBar);
	~CSideBarButton();

   LONG                  getHeight() const { return(m_sz.cy); }
   const SIZE&           getSize() const { return(m_sz); }
   void                  setSize(const SIZE& newSize) { m_sz = newSize; }
   const bool            isTopAligned() const { return(m_isTopAligned); }
   const HWND            getHwnd() const { return(m_hwnd); }
   const UINT            getID() const { return(m_id); }
   const MCONTACT        getContactHandle() const { return(m_dat->hContact); }
   const TWindowData*    getDat() const { return(m_dat); }
   const TSideBarLayout* getLayout() const { return(m_sideBarLayout); }

   void                  RenderThis(const HDC hdc) const;
   void                  renderIconAndNick(const HDC hdc, const RECT *rcItem) const;
   int                   testCloseButton() const;
   void                  Show(const int showCmd) const;
   void                  activateSession() const;
   const SIZE&           measureItem();
   void                  setLayout(const TSideBarLayout *newLayout);
   void                  invokeContextMenu();

public:
   CSideBar*             m_sideBar;
   const TSButtonCtrl*   m_buttonControl;  // private data struct of the Win32 button object
private:
   void                  _create();
private:
   const TSideBarLayout* m_sideBarLayout;
   HWND                  m_hwnd;           // window handle for the TSButton object
   const TWindowData*    m_dat;            // session data
   UINT                  m_id;             // control id
   bool                  m_isTopAligned;
   SIZE                  m_sz;
};

class CSideBar
{
public:
	enum {
		NR_LAYOUTS = 4
	};

	enum {
		/* layout ids. index into m_layouts[] */

		SIDEBARLAYOUT_VERTICAL = 0,
		SIDEBARLAYOUT_NORMAL = 1,
		SIDEBARLAYOUT_COMPLEX = 2,
		SIDEBARLAYOUT_LARGE = 3,

		/* flags */

		SIDEBARORIENTATION_LEFT = 8,
		SIDEBARORIENTATION_RIGHT = 16,

		SIDEBARLAYOUT_DYNHEIGHT = 32,
		SIDEBARLAYOUT_VERTICALORIENTATION = 64,
		SIDEBARLAYOUT_NOCLOSEBUTTONS = 128
	};

	enum {
		SIDEBAR_GAP = 2									// gap between sidebar container window and content tab sheet border
	};

	CSideBar(TContainerData *pContainer);
	~CSideBar();

   void                  Init(const bool fForce = false);
   void                  addSession(const TWindowData *dat, int position);
   HRESULT               removeSession(const TWindowData *dat);
   void                  updateSession(const TWindowData *dat);

   void                  processScrollerButtons(UINT cmd);
   void                  Layout(const RECT *rc = 0, bool fOnlyCalc = false);
   void                  setVisible(bool fNewVisibility);
   void                  showAll(int showCmd);

   const LONG            getWidth() const { return(m_isVisible ? m_width + SIDEBAR_GAP : 0); }
   const DWORD           getFlags() const { return(m_dwFlags); }
   const TContainerData* getContainer() const { return(m_pContainer); }
   const bool            isActive() const { return(m_isActive); }
   const bool            isVisible() const { return(m_isVisible); }
   const CSideBarButton* getActiveItem() const { return(m_activeItem); }
   const CSideBarButton* getScrollUp() const { return(m_up); }
   const CSideBarButton* getScrollDown() const { return(m_down); }
   bool                  isSkinnedContainer() const { return(CSkin::m_skinEnabled ? true : false); }
   const UINT            getLayoutId() const { return(m_uLayout); }
   void                  invalidateButton(const TWindowData *dat);

   const CSideBarButton* setActiveItem(const CSideBarButton *newItem)
   {
      CSideBarButton *oldItem = m_activeItem;
      m_activeItem = const_cast<CSideBarButton *>(newItem);
      if (oldItem)
         ::InvalidateRect(oldItem->getHwnd(), NULL, FALSE);
      ::InvalidateRect(m_activeItem->getHwnd(), NULL, FALSE);
      scrollIntoView(m_activeItem);
      return(oldItem);
   }
   /**
    * this item has its close button currently hovered
    * @param item: the CSideBarButton* which is hovered
    */
   void                  setHoveredClose               (CSideBarButton* item)
   {
      m_hoveredClose = item;
   }
   HWND                  getScrollWnd() const { return(m_hwndScrollWnd); }
   const CSideBarButton* getHoveredClose() const { return(m_hoveredClose); }
   const CSideBarButton* setActiveItem (const TWindowData *dat);

   static LRESULT CALLBACK wndProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static const TSideBarLayout* getLayouts                  (int& uLayoutCount)
   {
      uLayoutCount = NR_LAYOUTS;
      return(m_layouts);
   }
   void                  scrollIntoView               (const CSideBarButton *item = 0);
   void                  resizeScrollWnd               (LONG x, LONG y, LONG width, LONG height) const;

private:
   void                  createScroller();
   void                  destroyScroller();
   void                  populateAll();
   void                  removeAll();
   void                  Invalidate();
   CSideBarButton*       findSession(const TWindowData *dat);
   CSideBarButton*       findSession(const MCONTACT hContact);

   LRESULT CALLBACK      wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
   HWND                  m_hwndScrollWnd;
   OBJLIST<CSideBarButton> m_buttonlist;                      // our list of buttons
   TContainerData*       m_pContainer;                      // our master and commander...
   LONG                  m_width;                           // required width of the bar (m_elementWidth + padding)
   DWORD                 m_dwFlags;
   CSideBarButton*       m_up, *m_down;                     // the scroller buttons (up down)
   CSideBarButton*       m_activeItem;                      // active button item (for highlighting)
   const CSideBarButton* m_hoveredClose;                    // item which must display an active close button
   LONG                  m_topHeight, m_bottomHeight;
   LONG                  m_firstVisibleOffset, m_totalItemHeight;
   int                   m_iTopButtons, m_iBottomButtons;
   LONG                  m_elementHeight, m_elementWidth;   // width / height for a single element.
                                                            // can be dynamic (see measeureItem() in CSideBarButtonItem
   bool                  m_isActive;                        // the sidebar is active (false, if it does _nothing at all_
   bool                  m_isVisible;                       // visible aswell (not collapsed)
   TSideBarLayout*       m_currentLayout;                   // the layout in use. will be passed to new button items
   UINT                  m_uLayout;                         // layout id number, currently in use

private:
   /*
    * layouts. m_layouts[] is static and contains layout descriptions
    * renderer functions are static aswell
    */
   static TSideBarLayout  m_layouts[NR_LAYOUTS];
   static void __fastcall m_DefaultBackgroundRenderer(const HDC hdc, const RECT *rc, const CSideBarButton *item);
   static void __fastcall m_DefaultContentRenderer(const HDC hdc, const RECT *rc, const CSideBarButton *item);
   static void __fastcall m_AdvancedContentRenderer(const HDC hdc, const RECT *rc, const CSideBarButton *item);

   static const SIZE& __fastcall   m_measureAdvancedVertical(CSideBarButton *item);
};

inline void	CSideBarButton::setLayout(const TSideBarLayout *newLayout)
{
	m_sideBarLayout = newLayout;
}

#endif
