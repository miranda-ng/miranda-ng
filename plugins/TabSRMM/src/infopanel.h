/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-15 Miranda NG project,
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
// the info area for both im and chat sessions

#ifndef __INFOPANEL_H
#define __INFOPANEL_H

/*
 * configuration data for the info panel (fonts, colors)
 * this is global for all panels
 */

#define IPFONTCOUNT 6						// # of fonts needed for the info panel

/*
 * command ids for the info panel contextual menus
 */

#define CMD_IP_USERDETAILS					20000
#define CMD_IP_USERPREFS					20001
#define CMD_IP_ROOMPREFS					20002
#define CMD_IP_HISTORY						20003
#define CMD_IP_COPY							21000

struct TInfoPanelConfig {
	HFONT       hFonts[IPFONTCOUNT];
	COLORREF    clrs[IPFONTCOUNT];
	UINT		height1, height2;
};

extern TCHAR *xStatusDescr[];

/**
 * a simple tooltip class using a richedit control to display its content. Allows
 * for formatted text and clickable hyperlinks
 *
 * Used by: Info panel to display status messages, topics etc.
 */
class CTip
{
public:
	enum {
		TOP_BORDER = 25,
		LEFT_BORDER = 2,
		RIGHT_BORDER = 2,
		BOTTOM_BORDER = 1,
		LEFT_BAR_WIDTH = 20
	};

	CTip(const HWND hwndParent, const MCONTACT hContact, const TCHAR *pszText = 0, const CInfoPanel *panel = 0);
	~CTip()
	{
		mir_free(m_pszText);
	}
	void						show(const RECT& rc, POINT& pt, const HICON hIcon = 0, const TCHAR *szTitle = 0);
	const HWND					getHwnd() const { return(m_hwnd); }

	static void					registerClass();
private:

	INT_PTR CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProcStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK RichEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND     m_hwnd;            // our window handle
	HWND     m_hRich;           // handle of the rich edit child
	HWND     m_hwndParent;      // parent window (used for position calculations and to send notifications)
	MCONTACT m_hContact;        // contact handle
	char    *m_pszText;         // the richedit text
	SIZE     m_szRich;          // size of the richedit control (height auto-calculated to make it fit the text)
	RECT     m_rcRich;          // adjusted rectangle for the richedit control (client coordinates)
	HICON    m_hIcon;           // optional icon to show in the title line
	LPCTSTR  m_szTitle;         // optional text to show in the title
	int      m_leftWidth;

	const CInfoPanel *m_panel; // the info panel parent (if any)
};

/**
 * the info panel class definition. The panel itself is not a real window class - it
 * is implemented as a drawing canvas on the message window background.
 *
 * Each message session has its own info panel object
 */
class CInfoPanel
{
public:
	enum {
		DEGRADE_THRESHOLD = 37,					// defines the height at which the infopanel will do the transition from 1 to 2 lines
		LEFT_OFFSET_LOGO = 3
	};
	enum {
		HOVER_NICK = 1,
		HOVER_STATUS = 2,
		HOVER_UIN = 4
	};
	enum {
		HTNICK = 1,
		HTUIN = 2,
		HTSTATUS = 3,
		HTNIRVANA = 0
	};

	CInfoPanel(TWindowData *dat)
	{
		if (dat) {
			m_dat = dat;
			m_isChat = dat->bType == SESSIONTYPE_CHAT ? true : false;
		}
		m_defaultHeight = PluginConfig.m_panelHeight;
		m_defaultMUCHeight = PluginConfig.m_MUCpanelHeight;
		m_hwndConfig = 0;
		m_hoverFlags = 0;
		m_tip = 0;
	}

	~CInfoPanel()
	{
		if (m_hwndConfig)
			::DestroyWindow(m_hwndConfig);
		saveHeight(true);
	}

	__forceinline const LONG getHeight() const { return m_height; }
	__forceinline bool isActive() const { return m_active; }
	__forceinline bool isPrivateHeight() const { return m_fPrivateHeight; }
	__forceinline DWORD isHovered() const { return m_active ? m_hoverFlags : 0; }
	__forceinline const TWindowData* getDat() const { return m_dat; }

	void setHeight(LONG newHeight, bool fBroadcast = false);
	void setActive(const int newActive);
	void loadHeight();
	void saveHeight(bool fFlush = false);

	void Configure() const;
	void showHide() const;
	bool getVisibility();
	void renderBG(const HDC hdc, RECT& rc, CSkinItem *item, bool bAero, bool fAutoCalc = true) const;
	void renderContent(const HDC hdcMem);
	void Invalidate(BOOL fErase = FALSE) const;
	void trackMouse(POINT& pt);
	int  hitTest(POINT  pt);
	void handleClick(const POINT& pt);
	void showTip(UINT ctrlId, const LPARAM lParam);
	void hideTip(const HWND hWndNew);
	int  invokeConfigDialog(const POINT& pt);
	void dismissConfig(bool fForced = false);

public:
	static TInfoPanelConfig m_ipConfig;
	static int setPanelHandler(TWindowData *dat, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK avatarParentSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void  mapRealRect(const RECT& rcSrc, RECT& rcDest, const SIZE& sz);
	void  mapRealRectOnTop(const RECT& rcSrc, RECT& rcDest, const SIZE& sz);
	HFONT setUnderlinedFont(const HDC hdc, HFONT hFontOrig);
	void  RenderIPNickname(const HDC hdc, RECT& rc);
	void  RenderIPUIN(const HDC hdc, RECT& rcItem);
	void  RenderIPStatus(const HDC hdc, RECT& rcItem);
	void  Chat_RenderIPNickname(const HDC hdc, RECT& rcItem);
	void  Chat_RenderIPSecondLine(const HDC hdc, RECT& rcItem);
	HMENU constructContextualMenu() const;
	LRESULT cmdHandler(UINT cmd);
	INT_PTR CALLBACK ConfigDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ConfigDlgProcStub(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool  m_isChat;             // is MUC session
	bool  m_active;             // panel active and visible
	bool  m_fPrivateHeight;
	bool  m_fDialogCreated;
	TWindowData *m_dat;         // this one OWNS us...
	LONG  m_height;             // height (determined by position of IDC_PANELSPLITTER)
	LONG  m_defaultHeight,
		m_defaultMUCHeight;   // global values for the info bar height
	HWND  m_hwndConfig;         // window handle of the config dialog window
	HFONT m_configDlgFont,
		m_configDlgBoldFont;
	SIZE  m_szNick;             // rectangle where the nick has been rendered,
	/*
	 * these are used to store rectangles important to mouse tracking.
	 */
	RECT  m_rcNick;
	RECT  m_rcUIN;
	RECT  m_rcStatus;
	DWORD m_hoverFlags;
	CTip *m_tip;
};

#endif /* __INFOPANEL_H */
