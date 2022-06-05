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
// Plugin configuration variables and functions. Implemented as a class
// though there will always be only a single instance.

#ifndef __GLOBALS_H
#define __GLOBALS_H

struct TSplitterBroadCast {
	TContainerData *pSrcContainer;
	CMsgDialog *pSrcDat;
	LONG pos, pos_chat;
	LONG off_chat, off_im;
	LPARAM lParam;
};

typedef BOOL(WINAPI *pfnSetMenuInfo)(HMENU hmenu, LPCMENUINFO lpcmi);

class CGlobals
{
public:
	CGlobals()
	{
		memset(this, 0, sizeof(CGlobals));
		m_TypingSoundAdded = false;
	}

	~CGlobals()
	{
		if (m_MenuBar)
			::DestroyMenu(m_MenuBar);
	}
	void        reloadAdv();
	void        reloadSystemStartup();
	void        reloadSystemModulesChanged();
	void        reloadSettings(bool fReloadSkins = true);

	void        hookSystemEvents();

	const HMENU getMenuBar();

	HWND        g_hwndHotkeyHandler;
	HICON       g_iconIn, g_iconOut, g_iconErr, g_iconContainer, g_iconStatus;
	HICON       g_iconOverlayDisabled, g_iconOverlayEnabled, g_iconClock;
	HCURSOR     hCurSplitNS, hCurSplitWE, hCurSplitSW, hCurSplitWSE;
	HBITMAP     g_hbmUnknown;
	bool        g_SmileyAddAvail;
	HIMAGELIST  g_hImageList;
	HICON       g_IconMsgEvent, g_IconTypingEvent, g_IconFileEvent, g_IconSend;
	HICON       g_IconMsgEventBig, g_IconTypingEventBig;
	HMENU       g_hMenuContext, g_hMenuContainer;
	HICON       g_buttonBarIcons[NR_BUTTONBARICONS];
	HANDLE      g_buttonBarIconHandles[23];

	// dynamic options, need reload when options change
	int         m_iTabNameLimit;
	bool        m_bAllowTab;
	bool        m_bAutoSwitchTabs;
	bool        m_bCutContactNameOnTabs;
	bool        m_bHideOnClose;
	bool        m_bStatusOnTabs;
	bool        m_bLogStatusChanges;
	bool        m_bUseSameSplitSize;
	bool        m_bUseDividers;
	bool        m_bDividersUsePopupConfig;
	bool        m_bAllowOfflineMultisend;
	bool        m_bFlashOnClist;
	bool        m_bAlwaysFullToolbarWidth;
	bool        m_bIdleDetect;

	int         m_MsgTimeout;
	int         m_EscapeCloses;
	int         m_LimitStaticAvatarHeight;
	int         m_SendFormat;
	int         m_UnreadInTray;
	int         m_TrayFlashes;
	int         m_TrayFlashState;
	HANDLE      m_UserMenuItem;
	double      m_DPIscaleX;
	double      m_DPIscaleY;
	HBITMAP     m_hbmMsgArea;
	HWND        m_hwndClist;
	myTabCtrl   tabConfig;
	int         m_panelHeight, m_MUCpanelHeight;
	int         m_smcxicon, m_smcyicon;
	int         m_PasteAndSend;
	COLORREF    crIncoming, crOutgoing, crOldIncoming, crOldOutgoing, crStatus;
	BOOL        bUnicodeBuild;
	HFONT       hFontCaption;
	uint32_t    m_LangPackCP;
	uint8_t     m_SmileyButtonOverride;
	HICON       m_AnimTrayIcons[4];
	BOOL        m_visualMessageSizeIndicator;
	BOOL        m_autoSplit;
	BOOL        m_FlashOnMTN;
	uint32_t    dwThreadID;
	MWindowList m_hMessageWindowList, hUserPrefsWindowList;
	HMENU       m_MenuBar;
	COLORREF    m_ipBackgroundGradient;
	COLORREF    m_ipBackgroundGradientHigh;
	COLORREF    m_tbBackgroundHigh, m_tbBackgroundLow, m_fillColor, m_cRichBorders, m_genericTxtColor;
	HGENMENU    m_hMenuItem;
	uint8_t     m_useAeroPeek;

	WINDOWPLACEMENT    m_GlobalContainerWpos;
	NONCLIENTMETRICS   m_ncm;

	TSplitterBroadCast lastSPlitterPos;
	TContainerSettings globalContainerSettings;

	static wchar_t* m_default_container_name;

	static void logStatusChange(WPARAM wParam, const CContactCache *c);

private:
	bool m_TypingSoundAdded;

	static EXCEPTION_RECORD m_exRecord;
	static CONTEXT m_exCtx;
	static LRESULT m_exLastResult;
	static char    m_exSzFile[MAX_PATH];
	static wchar_t m_exReason[256];
	static int     m_exLine;
	static bool    m_exAllowContinue;
private:
	static int     ModulesLoaded(WPARAM wParam, LPARAM lParam);
	static int     DBSettingChanged(WPARAM wParam, LPARAM lParam);
	static int     DBContactDeleted(WPARAM wParam, LPARAM lParam);
	static int     PreshutdownSendRecv(WPARAM wParam, LPARAM lParam);
	static int     MetaContactEvent(WPARAM wParam, LPARAM lParam);
	static int     OkToExit(WPARAM wParam, LPARAM lParam);
	static void    RestoreUnreadMessageAlerts(void);
};

extern	CGlobals	PluginConfig;

#define DPISCALEY_S(argY) ((int) ((double)(argY) * PluginConfig.m_DPIscaleY))
#define DPISCALEX_S(argX) ((int) ((double)(argX) * PluginConfig.m_DPIscaleX))

#endif /* __GLOBALS_H */
