/*
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * Plugin configuration variables and functions. Implemented as a class
 * though there will always be only a single instance.
 *
 */

#ifndef __GLOBALS_H
#define __GLOBALS_H

struct TSplitterBroadCast {
	TContainerData *pSrcContainer;
	TWindowData  *pSrcDat;
	LONG pos, pos_chat;
	LONG off_chat, off_im;
	LPARAM lParam;
	BYTE bSync;
};

typedef BOOL (WINAPI *pfnSetMenuInfo )( HMENU hmenu, LPCMENUINFO lpcmi );

class CGlobals
{
public:
	CGlobals()
	{
		::ZeroMemory(this, sizeof(CGlobals));
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
   bool        haveAutoSwitch();

   const HMENU getMenuBar();

   HWND        g_hwndHotkeyHandler;
   HICON       g_iconIn, g_iconOut, g_iconErr, g_iconContainer, g_iconStatus;
   HICON       g_iconOverlayDisabled, g_iconOverlayEnabled, g_iconClock;
   HCURSOR     hCurSplitNS, hCurSplitWE, hCurHyperlinkHand;
   HBITMAP     g_hbmUnknown;
   int         g_SmileyAddAvail, g_WantIEView, g_PopupAvail, g_WantHPP;
   int         g_FlashAvatarAvail;
   HIMAGELIST  g_hImageList;
   HICON       g_IconMsgEvent, g_IconTypingEvent, g_IconFileEvent, g_IconSend;
   HICON       g_IconMsgEventBig, g_IconTypingEventBig;
   HICON       g_IconFolder, g_IconChecked, g_IconUnchecked;
   HMENU       g_hMenuContext, g_hMenuContainer, g_hMenuEncoding, g_hMenuTrayUnread;
   HMENU       g_hMenuFavorites, g_hMenuRecent, g_hMenuTrayContext;
   HICON       g_buttonBarIcons[NR_BUTTONBARICONS];
   HICON       g_sideBarIcons[NR_SIDEBARICONS];
   HANDLE      g_buttonBarIconHandles[23];
   
	// dynamic options, need reload when options change
   int         m_SendOnShiftEnter;
   int         m_SendOnEnter;
   int         m_SendOnDblEnter;
   int         m_AutoLocaleSupport;
   int         m_AutoSwitchTabs;
   int         m_CutContactNameOnTabs;
   int         m_CutContactNameTo;
   int         m_StatusOnTabs;
   int         m_LogStatusChanges;
   int         m_UseDividers;
   int         m_DividersUsePopupConfig;
   int         m_MsgTimeout;
   int         m_EscapeCloses;
   int         m_FlashOnClist;
   int         m_AlwaysFullToolbarWidth;
   int         m_LimitStaticAvatarHeight;
   int         m_SendFormat;
   int         m_FormatWholeWordsOnly;
   int         m_RTLDefault;
   int         m_UnreadInTray;
   int         m_TrayFlashes;
   int         m_TrayFlashState;
   BOOL        m_SuperQuiet;
   HANDLE      m_UserMenuItem;
   double      g_DPIscaleX;
   double      g_DPIscaleY;
   BOOL        m_HideOnClose;
   BOOL        g_bSoundOnTyping;
   BOOL        m_AllowTab;
   BYTE        m_AllowOfflineMultisend;
   BOOL        g_bDisableAniAvatars;
   HBITMAP     m_hbmMsgArea;
   BYTE        g_iButtonsBarGap;
   BYTE        m_WinVerMajor;
   BYTE        m_WinVerMinor;
   bool        m_bIsVista, m_bIsWin7;
   HWND        m_hwndClist;
   int         m_TabAppearance;
   myTabCtrl   tabConfig;
   int         m_panelHeight, m_MUCpanelHeight;
   int         m_IdleDetect;
   int         m_smcxicon, m_smcyicon;
   int         m_PasteAndSend;
   TCHAR      *m_szNoStatus;
   COLORREF    crIncoming, crOutgoing, crOldIncoming, crOldOutgoing, crStatus;
   BOOL        bUnicodeBuild;
   HFONT       hFontCaption;
   DWORD       m_LangPackCP;
   BYTE        m_SmileyButtonOverride;
   HICON       m_AnimTrayIcons[4];
   BOOL        m_visualMessageSizeIndicator;
   BOOL        m_autoSplit;
   BOOL        m_FlashOnMTN;
   DWORD       dwThreadID;
   BYTE        bMetaEnabled;
   HANDLE      m_hMessageWindowList, hUserPrefsWindowList;
   HMENU       m_MenuBar;
   COLORREF    m_ipBackgroundGradient;
   COLORREF    m_ipBackgroundGradientHigh;
   COLORREF    m_tbBackgroundHigh, m_tbBackgroundLow, m_fillColor, m_cRichBorders, m_genericTxtColor;
   BYTE        m_dontUseDefaultKbd;
   HANDLE      m_event_MsgWin, m_event_MsgPopup, m_event_WriteEvent;
   HGENMENU    m_hMenuItem;
   BYTE        m_useAeroPeek;

   WINDOWPLACEMENT    m_GlobalContainerWpos;
   NONCLIENTMETRICS   m_ncm;

	TSplitterBroadCast lastSPlitterPos;
	TContainerSettings globalContainerSettings;

	static TCHAR* m_default_container_name;
	
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
	static int     TopToolbarLoaded(WPARAM,LPARAM);
	static int     ModulesLoaded(WPARAM wParam, LPARAM lParam);
	static int     DBSettingChanged(WPARAM wParam, LPARAM lParam);
	static int     DBContactDeleted(WPARAM wParam, LPARAM lParam);
	static int     PreshutdownSendRecv(WPARAM wParam, LPARAM lParam);
	static int     MetaContactEvent(WPARAM wParam, LPARAM lParam);
	static int     OkToExit(WPARAM wParam, LPARAM lParam);
	static void    RestoreUnreadMessageAlerts(void);
};

extern	CGlobals	PluginConfig;
extern	CGlobals	*pConfig;

#define DPISCALEY_S(argY) ((int) ((double)(argY) * PluginConfig.g_DPIscaleY))
#define DPISCALEX_S(argX) ((int) ((double)(argX) * PluginConfig.g_DPIscaleX))

#endif /* __GLOBALS_H */
