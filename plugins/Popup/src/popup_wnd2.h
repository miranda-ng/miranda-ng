/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __popup_wnd2_h__
#define __popup_wnd2_h__

#define UM_CALLMETHOD (WM_USER+2048)

class PopupAvatar;

#define POPUP_OBJECT_SIGNARURE	0x0B1A11BE

class PopupWnd2 : public MZeroedObject
{
public:
	typedef		LRESULT (PopupWnd2::*MethodPtr)(LPARAM lParam);
	enum		TextType {TT_NONE, TT_UNICODE, TT_MTEXT};

	struct		ActionInfo
	{
		POPUPACTION actionA;
		RECT rc;
		bool hover;

		ActionInfo(): hover(false) {}
	};

	DWORD		m_signature;

private:
	// style
	COLORREF m_clBack, m_clText, m_clTitle, m_clClock;
	int m_iTimeout;

	// content
	TextType    m_textType;
	TCHAR      *m_lptzTitle, *m_lptzText;
	HANDLE      m_mtTitle, m_mtText;
	bool        m_bTextEmpty, m_bIcoLib;
	HFONT       m_hfnTitle, m_hfnText;
	HICON       m_hIcon;
	HBITMAP     m_hbmAvatar;
	TCHAR       m_time[2+1+2+1];
	ActionInfo* m_actions;
	int         m_actionCount;
	HANDLE      m_hNotification;

	// other data
	Formula::Args m_args;
	MCONTACT    m_hContact, m_hContactPassed;
	WNDPROC     m_PluginWindowProc;
	void       *m_PluginData;
	
	// the window
	LPTSTR      m_lpzSkin;
	bool        m_customPopup;
	HWND        m_hwnd, m_hwndToolTip;
	bool        m_bPositioned;
	POINT       m_pos;
	SIZE        m_sz;
	MyBitmap   *m_bmpBase, *m_bmp, *m_bmpAnimate;
	PopupAvatar*m_avatar;
	int         m_avatarFrameDelay;
	bool        m_bReshapeWindow;
	HANDLE      m_hhkAvatarChanged;
	bool        m_bIsPinned;

	// show & hide
	bool        m_bFade;
	BYTE        m_btAlpha0,		m_btAlpha1;
	bool        m_bSlide;
	POINT       m_ptPosition0,	m_ptPosition1;
	bool        m_bDestroy;
	bool        m_bIsHovered;

	// prevent unwanted clicks
	POINT       m_ptPrevCursor;

	// system
	POPUPOPTIONS *m_options;
	DWORD       m_lockCount;

	PopupSkin::RenderInfo	m_renderInfo;

	void       fixDefaults();
	void       fixAvatar();
	int        fixActions(POPUPACTION *theActions, int count);
	int        fixActions(POPUPACTION *theActions, int count, int additional);

public:
	PopupWnd2(POPUPDATA2 *ppd, POPUPOPTIONS *theCustomOptions=NULL, bool renderOnly=false);
	~PopupWnd2();

	void	startThread();

	void	create();
	void	updateLayered(BYTE opacity);
	SIZE	measure();
	void	update();
	void	animate();

	void	show();
	void	hide();
	void	idle();

	DWORD	lock()				{ return ++m_lockCount; }
	DWORD	unlock()			{ return m_lockCount = m_lockCount ? m_lockCount-1 : 0; }
	bool	isLocked()			{ return m_lockCount != 0; }

	void  setIcon(HICON);

	void	updateData(POPUPDATAW_V2 *ppd);
	void	updateData(POPUPDATA2 *ppd);
	void	buildMText();
	void	updateText(TCHAR *text);
	void	updateTitle(TCHAR *title);

	void	updateTimer();

	MyBitmap *getContent()      { return m_bmp; }

	COLORREF getTextColor()     { return m_clText; }
	COLORREF getTitleColor()    { return m_clTitle; }
	COLORREF getClockColor()    { return m_clClock; }
	COLORREF getBackColor()     { return m_clBack; }

	bool     isTextEmpty()      { return m_bTextEmpty; }
	bool     isIcolib()         { return m_bIcoLib; }
	TextType getTextType()      { return m_textType; }
	TCHAR   *getText()          { return m_lptzText; }
	HANDLE   getTextM()         { return m_mtText; }
	TCHAR   *getTitle()         { return m_lptzTitle; }
	HANDLE   getTitleM()        { return m_mtTitle; }

	int      getActionCount()   { return m_actionCount; }
	ActionInfo *getActions()    { return m_actions; }

	TCHAR   *getTime()          { return m_time; }
	HICON    getIcon()          { return m_hIcon; }
	MCONTACT getContact()       { return m_hContact; }
	MCONTACT getContactPassed() { return m_hContactPassed; }
	int      getTimeout()       { return m_iTimeout; }
	HWND     getHwnd()          { return m_hwnd; }
	void    *getData()          { return m_PluginData; }

	PopupAvatar *getAvatar()    { return m_avatar; }

	Formula::Args *getArgs()    { return &m_args; }

	PopupSkin::RenderInfo *getRenderInfo() { return &m_renderInfo; }

	SIZE   getSize()            { return m_sz; }
	void   setSize(SIZE sz)
	{
		this->m_sz = sz;
		if (m_hwnd)
		{
			SetWindowPos(m_hwnd, 0, 0, 0, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
			if (!m_customPopup)
				PopupThreadUpdateWindow(this);
		}
	}
	bool	isPositioned()        { return m_bPositioned; }
	POINT	getPosition()         { return m_pos; }
	POINT	getRealPosition()     { return POINT(); }
	void	setPosition(POINT pt)
	{
		m_bPositioned = true;
		m_pos = pt;
		if (m_bSlide)
			m_ptPosition1 = pt;
		else
			SetWindowPos(m_hwnd, 0, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING);
	}

	// Thread-related methods
	void	callMethodAsync(MethodPtr method, LPARAM lParam)
	{
		// we can't pass MethodPtr via WPARAM because it's size can differ! well, it's still 4 bytes on msvc but
		// on gcc we will get 8 bytes...
		MethodPtr *method_copy = new MethodPtr; *method_copy = method;
		PostMessage(m_hwnd, UM_CALLMETHOD, (WPARAM)method_copy, (LPARAM)lParam);
	}
	void	callMethodSync(MethodPtr method, LPARAM lParam)
	{
		// we can't pass MethodPtr via WPARAM because it's size can differ! well, it's still 4 bytes on msvc but
		// on gcc we will get 8 bytes...
		MethodPtr *method_copy = new MethodPtr; *method_copy = method;
		SendMessage(m_hwnd, UM_CALLMETHOD, (WPARAM)method_copy, (LPARAM)lParam);
	}

	LRESULT m_updateData_POPUPDATAW_V2(LPARAM arg)	{ updateData((POPUPDATAW_V2 *)arg); update(); return 0; }
	LRESULT m_updateData_POPUPDATA2(LPARAM arg)		{ updateData((POPUPDATA2 *)arg); update(); return 0; }
	LRESULT m_updateText(LPARAM arg)				{ updateText((TCHAR *)arg); update(); return 0; }
	LRESULT m_updateTitle(LPARAM arg)				{ updateTitle((TCHAR *)arg); update(); return 0; }
	LRESULT m_show(LPARAM arg)						{ show(); return 0; }
	LRESULT m_hide(LPARAM arg)						{ hide(); return 0; }

	// window related stuff
	LRESULT CALLBACK WindowProc(UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

	// window creation flag
	volatile bool m_bWindowCreated;
};

bool LoadPopupWnd2();
void UnloadPopupWnd2();

static inline bool IsValidPopupObject(PopupWnd2 *wnd)
{
	bool res = false;
	__try
	{
		if (wnd->m_signature == POPUP_OBJECT_SIGNARURE)
			res = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		res = false;
	}
	return res;
}

#endif // __popup_wnd2_h__
