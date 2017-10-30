/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

#ifndef _MSGS_H
#define _MSGS_H

/*
 * required for MingW32 compatibility
 */

#define MSGERROR_CANCEL	0
#define MSGERROR_RETRY	    1
#define MSGERROR_SENDLATER  2

#define CONTAINER_NAMELEN 25
#define TITLE_FORMATLEN 30

#define MWF_SAVEBTN_SAV 2

#define MWF_DEFERREDSCROLL 4
#define MWF_NEEDHISTORYSAVE 8
#define MWF_WASBACKGROUNDCREATE 16
//#define MWF_MOUSEDOWN 32
#define MWF_ERRORSTATE 128
#define MWF_DEFERREDREMAKELOG 256

#define MWF_LOG_NORMALTEMPLATES 512
#define MWF_LOG_SHOWTIME 1024
#define MWF_LOG_SHOWSECONDS 2048
#define MWF_LOG_SHOWDATES 4096

#define MWF_LOG_INDENT 16384
#define MWF_LOG_RTL 32768

// ieview still mistakenly uses these...
#define MWF_LOG_NEWLINE   8192
#define MWF_LOG_UNDERLINE 65536
#define MWF_LOG_SWAPNICK  131072

#define MWF_LOG_BBCODE		 1
#define MWF_LOG_LOCALTIME	 64

#define MWF_LOG_SHOWICONS 262144
#define MWF_LOG_SYMBOLS 0x200000
#define MWF_INITMODE  0x400000
#define MWF_NEEDCHECKSIZE 0x800000
#define MWF_DIVIDERSET 0x1000000
#define MWF_LOG_TEXTFORMAT 0x2000000
#define MWF_LOG_GRID 0x4000000
// #define MWF_LOG_INDIVIDUALBKG 0x8000000 * FREE *
#define MWF_LOG_INOUTICONS 0x10000000
#define MWF_SMBUTTONSELECTED 0x20000000
#define MWF_DIVIDERWANTED 0x40000000
#define MWF_LOG_GROUPMODE 0x80000000

#define MWF_SHOW_FLASHCLIST 64
#define MWF_SHOW_SPLITTEROVERRIDE 128
#define MWF_SHOW_SCROLLINGDISABLED 256
#define MWF_SHOW_ISIDLE 4096
#define MWF_SHOW_AWAYMSGTIMER 8192
#define MWF_EX_DELAYEDSPLITTER 32768
#define MWF_EX_AVATARCHANGED 65536
#define MWF_EX_WARNCLOSE     0x20000

#define SMODE_DEFAULT 0
#define SMODE_MULTIPLE 1
#define SMODE_CONTAINER 2
#define SMODE_SENDLATER 8
#define SMODE_NOACK 16

#define SENDFORMAT_BBCODE 1
#define SENDFORMAT_NONE 0

#define AVATARMODE_DYNAMIC 0

#define MSGDLGFONTCOUNT 22
#define CHATFONTCOUNT 19

#define TMPL_MSGIN 0
#define TMPL_MSGOUT 1
#define TMPL_GRPSTARTIN 2
#define TMPL_GRPSTARTOUT 3
#define TMPL_GRPINNERIN 4
#define TMPL_GRPINNEROUT 5
#define TMPL_STATUSCHG 6
#define TMPL_ERRMSG 7

#define TEMPLATE_LENGTH 150
#define CUSTOM_COLORS 5

struct TTemplateSet {
	BOOL valid;             // all templates populated (may still contain crap.. so it's only half-assed safety :)
	wchar_t szTemplates[TMPL_ERRMSG + 1][TEMPLATE_LENGTH];      // the template strings
	char szSetName[20];     // everything in this world needs a name. so does this poor template set.
};

struct TitleBtn {
	BOOL isHot;
	BOOL isPressed;
};

#define BTN_MIN 0
#define BTN_MAX 1
#define BTN_CLOSE 2

#define NR_LOGICONS 7
#define NR_BUTTONBARICONS 37//MaD: 29
#define NR_SIDEBARICONS 2

class CTaskbarInteract;
class CMenuBar;
class CSideBar;
class CProxyWindow;
class CThumbBase;

struct CContactCache;

#define STICK_ICON_MSG 10

struct TLogTheme
{
	COLORREF  inbg, outbg, bg, oldinbg, oldoutbg, statbg, inputbg;
	COLORREF  hgrid;
	COLORREF  custom_colors[5];
	DWORD     dwFlags;
	DWORD     left_indent, right_indent;
	LOGFONTA *logFonts;
	COLORREF *fontColors;
	char     *rtfFonts;
	bool      isPrivate;
};

struct TContainerSettings
{
	DWORD   dwFlags;
	DWORD   dwFlagsEx;
	DWORD   dwTransparency;
	DWORD   panelheight;
	int     iSplitterX, iSplitterY;
	wchar_t szTitleFormat[TITLE_FORMATLEN + 2];
	WORD    avatarMode;
	WORD    ownAvatarMode;
	WORD    autoCloseSeconds;
	bool    fPrivate;
};

struct ButtonItem;

struct TContainerData
{
	TContainerData *pNext;

	wchar_t  m_wszName[CONTAINER_NAMELEN + 4];		// container name
	HWND     m_hwndActive;		// active message window
	HWND     m_hwnd;				// the container handle
	int      iTabIndex;			// next tab id
	int	   iChilds;
	int      iContainerIndex;
	bool	   fHidden;
	HWND     m_hwndTip;			// tab - tooltips...
	BOOL     bDontSmartClose;      // if set, do not search and select the next possible tab after closing one.
	DWORD    dwFlags;
	DWORD    dwFlagsEx;
	LONG     uChildMinHeight;
	int      tBorder;
	int	   tBorder_outer_left, tBorder_outer_right, tBorder_outer_top, tBorder_outer_bottom;
	MCONTACT hContactFrom;
	BOOL     isCloned;
	HWND     hwndStatus;
	int      statusBarHeight;
	DWORD    dwLastActivity;
	int      hIcon;                	// current window icon stick indicator
	HICON	   hIconTaskbarOverlay;	// contains a "sticky" taskbar overlay (e.g. new message icon)
	DWORD    dwFlashingStarted;
	HWND     hWndOptions;
	BOOL     bSizingLoop;
	wchar_t  szRelThemeFile[MAX_PATH], szAbsThemeFile[MAX_PATH];
	TTemplateSet *ltr_templates, *rtl_templates;
	HDC      cachedDC;
	HBITMAP  cachedHBM, oldHBM;
	SIZE     oldDCSize;
	RECT     rcClose, rcMin, rcMax;
	TitleBtn buttons[3];
	TitleBtn oldbuttons[3];
	int      ncActive;
	HWND     hwndSaved;
	ButtonItem *buttonItems;
	RECT     rcSaved, rcLogSaved;
	POINT	   ptLogSaved;
	DWORD    exFlags;
	BOOL	   fPrivateThemeChanged;
	MARGINS  mOld;
	HDC		cachedToolbarDC;
	HBITMAP  hbmToolbarBG, oldhbmToolbarBG;
	SIZE	   szOldToolbarSize;
	SIZE     oldSize, preSIZE;
	WORD	   avatarMode, ownAvatarMode;
	BYTE	   bTBRenderingMode;
	TLogTheme theme;
	TContainerSettings* settings;
	CTaskbarInteract*	TaskBar;
	CMenuBar *MenuBar;
	CSideBar *SideBar;

	void UpdateTabs();
	void UpdateTitle(MCONTACT, class CTabBaseDlg* = nullptr);

	void ClearMargins()
	{	memset(&mOld, 0xfe, sizeof(mOld));
	}
};

struct SESSION_INFO;

class CTabBaseDlg : public CSrmmBaseDialog
{
	typedef CSrmmBaseDialog CSuper;
	friend class CInfoPanel;

protected:
	virtual void LoadSettings() override;
	virtual void SetStatusText(const wchar_t*, HICON) override;

	void    DM_AddDivider();
	void    DM_DismissTip(const POINT& pt);
	void    DM_ErrorDetected(int type, int flag);
	bool    DM_GenericHotkeysCheck(MSG *message);
	int     DM_SplitterGlobalEvent(WPARAM wParam, LPARAM lParam);
	void    DM_UpdateLastMessage() const;
			  
	void    DetermineMinHeight();
	void    FindFirstEvent();
	int     FindRTLLocale();
	void    GetSendFormat();
	bool    IsAutoSplitEnabled() const;
	void    ReplaceIcons(LONG startAt, int fAppend, BOOL isSent);
	void    ResizeIeView();
	void    ShowPopupMenu(const CCtrlBase&, POINT pt);
	void    VerifyProxy();
	LRESULT WMCopyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);

	WORD     m_wStatus, m_wOldStatus;
	size_t   m_iSendBufferSize;
	int      m_iSendLength;				// message length in utf-8 octets
	HICON    m_hSmileyIcon;
	HWND     m_hwndContactPic, m_hwndPanelPic, m_hwndPanelPicParent;
	UINT     m_bbLSideWidth, m_bbRSideWidth;
	BYTE     kstate[256];

	RECT     m_rcNick, m_rcUIN, m_rcStatus, m_rcPic;
	int      m_originalSplitterY;
	SIZE     m_minEditBoxSize;
	int      m_nTypeMode;
	DWORD    m_nLastTyping;
	DWORD    m_lastMessage;
	DWORD    m_dwTickLastEvent;
	HBITMAP  m_hOwnPic;
	SIZE     m_pic;

	CMStringW m_szStatusText;
	HICON     m_szStatusIcon;
	bool      m_bStatusSet;

	bool     m_bShowInfoAvatar, m_bShowUIElements;
	bool     m_bUseOffset;
	bool     m_bkeyProcessed;
	bool     m_fLimitedUpdate;
	bool     m_bClrAdded;
	bool     m_bInsertMode;

	int      m_iRealAvatarHeight;
	int      m_iButtonBarReallyNeeds;
	DWORD    m_dwLastActivity;
	MEVENT   m_hFlashingEvent;
	int      m_SendFormat;
	MEVENT  *m_hQueuedEvents;
	int      m_iNextQueuedEvent;
#define EVENT_QUEUE_SIZE 10
	int      m_iEventQueueSize;
	LCID     m_lcid;
	wchar_t  m_lcID[10];
	int      m_iPanelAvatarX, m_iPanelAvatarY;
	HWND     m_hwndTip;
	TOOLINFO ti;
	DWORD    m_panelStatusCX;
	int      m_textLen;         // current text len
	LONG     m_ipFieldHeight;
	WPARAM   m_wParam;          // used for "delayed" actions like moved splitters in minimized windows
	LPARAM   m_lParam;
	int      m_iHaveRTLLang;

	DWORD    m_iSplitterSaved;
	POINT    m_ptTipActivation;
	char    *m_enteredText; // Used for history in chats.

public:
	char    *m_szProto;
	int      m_iTabID;
	BYTE     m_bShowTyping;
	bool     m_bIsHistory, m_bNotOnList;
	bool     m_bActualHistory;
	bool     m_bIsAutosizingInput;
	bool     m_bCanFlashTab, m_bTabFlash;
	bool     m_bEditNotesActive;
	bool     m_bShowAvatar;
	int      m_sendMode;
	HKL      m_hkl;                                    // keyboard layout identifier
	DWORD    m_isAutoRTL;
	DWORD    m_idle;
	DWORD    m_dwFlags, m_dwFlagsEx;
	DWORD    m_dwUnread;
	HANDLE   m_hTheme, m_hThemeIP, m_hThemeToolbar;
	HWND     m_hwndIEView, m_hwndIWebBrowserControl, m_hwndHPP;
	HICON    m_hXStatusIcon, m_hTabStatusIcon, m_hTabIcon, m_iFlashIcon, m_hTaskbarIcon, m_hClientIcon;
	MEVENT   m_hDbEventFirst, m_hDbEventLast;
	HANDLE   m_hTimeZone;
	MEVENT  *m_hHistoryEvents;
	time_t   m_lastEventTime;
	int      m_iLastEventType;
	int      m_nTypeSecs;
	int      m_iOpenJobs;
	int      m_iInputAreaHeight;
	int      m_maxHistory, m_curHistory;
	int      m_iCurrentQueueError;
	int      m_iSplitterY, m_dynaSplitter;
	int      m_savedSplitterY, m_savedDynaSplit;
	char    *m_sendBuffer;
	int      m_nMax;            // max message size

	wchar_t  m_wszMyNickname[130];
	wchar_t  m_wszStatus[50];
	wchar_t  m_wszTitle[130];        // tab title...
	wchar_t  m_myUin[80];
	wchar_t  m_wszStatusBar[100];
	char     m_szMicroLf[128];

	CInfoPanel m_pPanel;
	CContactCache *m_cache;
	TContainerData *m_pContainer;		// parent container description structure
	AVATARCACHEENTRY *m_ace, *m_ownAce;
	CProxyWindow  *m_pWnd;	// proxy window object (win7+, for taskbar support).
									// ALWAYS check this pointer before using it, it is not guaranteed to exist.

public:
	CTabBaseDlg(int iDialogId, SESSION_INFO* = nullptr);
	virtual ~CTabBaseDlg();

	virtual void OnInitDialog() override;
	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	virtual CThumbBase* tabCreateThumb(CProxyWindow*) const = 0;
	virtual void tabClearLog() = 0;
	void tabUpdateStatusBar() const;

	static LONG_PTR CALLBACK StatusBarSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	__forceinline CCtrlRichEdit& GetEntry() { return m_message; }

	HWND  DM_CreateClist();
	void  DM_EventAdded(WPARAM wParam, LPARAM lParam);
	void  DM_InitRichEdit();
	void  DM_InitTip();
	void  DM_LoadLocale();
	void  DM_NotifyTyping(int mode);
	void  DM_RecalcPictureSize();
	void  DM_SaveLocale(WPARAM wParam, LPARAM lParam);
	void  DM_SaveLogAsRTF() const;
	void  DM_ScrollToBottom(WPARAM wParam, LPARAM lParam);
	void  DM_Typing(bool fForceOff);
	
	LRESULT DM_MsgWindowCmdHandler(UINT cmd, WPARAM wParam, LPARAM lParam);
	LRESULT DM_MouseWheelHandler(WPARAM wParam, LPARAM lParam);

	void  DM_HandleAutoSizeRequest(REQRESIZE* rr);

	void  DM_FreeTheme();
	void  DM_ThemeChanged();
		   
	void  BB_InitDlgButtons();
	void  BB_RefreshTheme();
	BOOL  BB_SetButtonsPos();
	void  BB_RedrawButtons();
	void  DM_SetDBButtonStates();
		   
	void  CB_DestroyAllButtons();
	void  CB_DestroyButton(DWORD dwButtonCID, DWORD dwFlags);
	void  CB_ChangeButton(CustomButtonData *cbd);

	void  AdjustBottomAvatarDisplay();
	void  CalcDynamicAvatarSize(BITMAP *bminfo);
	void  CheckStatusIconClick(POINT pt, const RECT &rc, int gap, int code);
	BOOL  DoRtfToTags(CMStringW &pszText) const;
	void  DrawStatusIcons(HDC hDC, const RECT &rc, int gap);
	void  EnableSendButton(bool bMode) const;
	void  EnableSending(bool bMode) const;
	void  FlashOnClist(MEVENT hEvent, DBEVENTINFO *dbei);
	void  FlashTab(bool bInvertMode);
	void  FormatRaw(CMStringW&, int flags, bool isSent);
	bool  FormatTitleBar(const wchar_t *szFormat, CMStringW &dest);
	bool  GetAvatarVisibility();
	void  GetClientIcon();
	LONG  GetDefaultMinimumInputHeight() const;
	void  GetLocaleID(const wchar_t *szKLName);
	HICON GetMyContactIcon(LPCSTR szSetting);
	void  GetMYUIN();
	void  GetMyNick();
	HICON GetXStatusIcon() const;
	void  HandlePasteAndSend();
	HICON IconFromAvatar() const;
	void  KbdState(bool &isShift, bool &isControl, bool &isAlt);
	int   LoadLocalFlags();
	void  LoadSplitter();
	int   MustPlaySound() const;
	void  NotifyDeliveryFailure() const;
	void  PlayIncomingSound() const;
	void 	SendHBitmapAsFile(HBITMAP hbmp) const;
	void  SaveSplitter();
	void  SetDialogToType();
	void  SetMessageLog();
	void  ShowPicture(bool showNewPic);
	void  StreamInEvents(MEVENT hDbEventFirst, int count, int fAppend, DBEVENTINFO *dbei_s);
	void  UpdateReadChars() const;
	void  UpdateSaveAndSendButton();

	int   MsgWindowDrawHandler(WPARAM wParam, LPARAM lParam);
	int   MsgWindowUpdateMenu(HMENU submenu, int menuID);
	int   MsgWindowMenuHandler(int selection, int menuId);

	void  RenderToolbarBG(HDC hdc, const RECT &rcWindow) const;
	void  UpdateToolbarBG();
};

class CSrmmWindow : public CTabBaseDlg
{
	typedef CTabBaseDlg CSuper;

	virtual CThumbBase* tabCreateThumb(CProxyWindow *pProxy) const override;
	virtual void tabClearLog() override;

	CCtrlButton m_btnOk, m_btnAdd, m_btnQuote, m_btnCancelAdd;

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void LoadContactAvatar();
	void LoadOwnAvatar();
	void MsgWindowUpdateState(UINT msg);
	void ReplayQueue();

public:
	int m_iMultiSplit;
	int msgTop, rcLogBottom;
	wchar_t *wszInitialText;
	bool m_bActivate, m_bWantPopup, m_bIsMeta;

public:
	CSrmmWindow();

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual int Resizer(UTILRESIZECONTROL *urc) override;
	
	virtual void UpdateTitle() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void onClick_Ok(CCtrlButton*);
	void onClick_Add(CCtrlButton*);
	void onClick_Color(CCtrlButton*);
	void onClick_Quote(CCtrlButton*);
	void onClick_CancelAdd(CCtrlButton*);

	void onChange_Message(CCtrlEdit*);

	int OnFilter(MSGFILTER*);

	void DM_OptionsApplied(WPARAM wParam, LPARAM lParam);
};

class CChatRoomDlg : public CTabBaseDlg
{
	typedef CTabBaseDlg CSuper;

	HWND m_hwndFilter;
	CCtrlButton m_btnOk;

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam) override;

	static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual CThumbBase* tabCreateThumb(CProxyWindow *pProxy) const override;
	virtual void tabClearLog() override;

	bool TabAutoComplete();

	int m_iSearchItem;
	BOOL m_iSavedSpaces;
	wchar_t m_wszSearch[255];
	wchar_t *m_wszSearchQuery, *m_wszSearchResult;
	SESSION_INFO *m_pLastSession;

public:
	CChatRoomDlg(SESSION_INFO*);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual int Resizer(UTILRESIZECONTROL *urc) override;
	
	virtual void AddLog() override;
	virtual void CloseTab() override;
	virtual void RedrawLog() override;
	virtual void ScrollToBottom() override;
	virtual void ShowFilterMenu() override;
	virtual void StreamInEvents(LOGINFO* lin, bool bRedraw) override;
	virtual void UpdateNickList() override;
	virtual void UpdateOptions() override;
	virtual void UpdateStatusBar() override;
	virtual void UpdateTitle() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void onClick_OK(CCtrlButton*);
	void onClick_Filter(CCtrlButton*);
	void onClick_ShowNickList(CCtrlButton*);

	void onChange_Message(CCtrlEdit*);

	void UpdateWindowState(UINT msg);
};

class CTemplateEditDlg : public CTabBaseDlg
{
	typedef CTabBaseDlg CSuper;

	BOOL rtl;
	BOOL changed;           // template in edit field is changed
	BOOL selchanging;
	int  inEdit;            // template currently in editor
	BOOL updateInfo[TMPL_ERRMSG + 1];  // item states...

	TTemplateSet *tSet;

	CCtrlEdit edtText;
	CCtrlButton btnResetAll, btnHelp, btnSave, btnForget, btnRevert, btnPreview;
	CCtrlListBox listTemplates;

	virtual CThumbBase* tabCreateThumb(CProxyWindow*) const override { return nullptr; }
	virtual void tabClearLog() override {}
	virtual void UpdateTitle() override {};

public:
	CTemplateEditDlg(BOOL rtl, HWND hwndParent);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void onChange_Text(CCtrlEdit*);

	void onClick_Forget(CCtrlButton*);
	void onClick_Help(CCtrlButton*);
	void onClick_Preview(CCtrlButton*);
	void onClick_Reset(CCtrlButton*);
	void onClick_Revert(CCtrlButton*);
	void onClick_Save(CCtrlButton*);

	void onDblClick_List(CCtrlListBox*);
	void onSelChange_List(CCtrlListBox*);
};

#define MESSAGE_WINDOW_DATA_SIZE offsetof(_MessageWindowData, hdbEventFirst);

struct RECENTINFO
{
	DWORD dwFirst, dwMostRecent;        // timestamps
	int   iFirstIndex, iMostRecent;     // tab indices
	HWND  hwndFirst, hwndMostRecent;    // client window handles
};

/*
 * configuration data for custom tab ctrl
 */

struct myTabCtrl
{
	HPEN    m_hPenShadow, m_hPenItemShadow, m_hPenLight;
	HFONT   m_hMenuFont;
	COLORREF colors[10];
	HBRUSH  m_brushes[4];
	DWORD   m_fixedwidth;
	int     m_bottomAdjust;
};

struct TIconDesc
{
	char   *szName;
	char   *szDesc;
	HICON  *phIcon;       // where the handle is saved...
	INT_PTR uId;           // icon ID
	BOOL    bForceSmall;   // true: force 16x16
};

struct TIconDescW
{
	wchar_t  *szName;
	wchar_t  *szDesc;
	HICON  *phIcon;       // where the handle is saved...
	INT_PTR uId;           // icon ID
	BOOL    bForceSmall;   // true: force 16x16
};

// menu IDS

#define MENU_LOGMENU 1
#define MENU_PICMENU 2
#define MENU_TABCONTEXT 3
#define MENU_PANELPICMENU 4

#define TABSRMM_SMILEYADD_BKGCOLORMODE 0x10000000
#define ADDEDEVENTSQUEUESIZE 100

/*
 * tab config flags
 */

#define TCF_FLAT 1
//#define TCF_STYLED 2
#define TCF_CLOSEBUTTON 4
#define TCF_FLASHICON 8
#define TCF_FLASHLABEL 16
#define TCF_SINGLEROWTABCONTROL 32
//#define TCF_LABELUSEWINCOLORS 64
//#define TCF_BKGUSEWINCOLORS 128
#define TCF_SBARLEFT 256
#define TCF_SBARRIGHT 512

#define TCF_DEFAULT (TCF_FLASHICON)

#define MIN_PANELHEIGHT 20

// flags for the container dwFlags
#define CNT_MOUSEDOWN                   1
#define CNT_NOTITLE                     2
#define CNT_HIDETABS                    4
#define CNT_SIDEBAR                     8
#define CNT_NOFLASH                  0x10
#define CNT_STICKY                   0x20
#define CNT_DONTREPORT               0x40
#define CNT_FLASHALWAYS              0x80
#define CNT_TRANSPARENCY            0x100
#define CNT_AVATARSONTASKBAR        0x200
#define CNT_DONTREPORTFOCUSED       0x400
#define CNT_GLOBALSIZE              0x800
#define CNT_INFOPANEL              0x1000
#define CNT_NOSOUND                0x2000
#define CNT_AUTOSPLITTER           0x4000
#define CNT_DEFERREDCONFIGURE      0x8000
#define CNT_CREATE_MINIMIZED      0x10000
#define CNT_NEED_UPDATETITLE      0x20000
#define CNT_DEFERREDSIZEREQUEST   0x40000
#define CNT_DONTREPORTUNFOCUSED   0x80000
#define CNT_ALWAYSREPORTINACTIVE 0x100000
#define CNT_NEWCONTAINERFLAGS    0x200000
#define CNT_DEFERREDTABSELECT    0x400000
#define CNT_CREATE_CLONED        0x800000
#define CNT_NOSTATUSBAR         0x1000000
#define CNT_NOMENUBAR           0x2000000
#define CNT_TABSBOTTOM          0x4000000
#define CNT_BOTTOMTOOLBAR      0x10000000
#define CNT_HIDETOOLBAR        0x20000000
#define CNT_UINSTATUSBAR       0x40000000
#define CNT_VERTICALMAX        0x80000000

#define CNT_EX_SOUNDS_MINIMIZED      1024
#define CNT_EX_SOUNDS_UNFOCUSED      2048
#define CNT_EX_SOUNDS_INACTIVETABS   4096
#define CNT_EX_SOUNDS_FOCUSED	       8192

#define CNT_FLAGS_DEFAULT (CNT_DONTREPORT | CNT_DONTREPORTUNFOCUSED | CNT_ALWAYSREPORTINACTIVE | CNT_HIDETABS | CNT_NEWCONTAINERFLAGS | CNT_NOMENUBAR | CNT_INFOPANEL)
#define CNT_TRANS_DEFAULT 0x00ff00ff

#define CNT_FLAGSEX_DEFAULT (TCF_FLASHICON | CNT_EX_SOUNDS_MINIMIZED | CNT_EX_SOUNDS_UNFOCUSED | CNT_EX_SOUNDS_INACTIVETABS | CNT_EX_SOUNDS_FOCUSED)

#define CNT_CREATEFLAG_CLONED 1
#define CNT_CREATEFLAG_MINIMIZED 2

#define CNT_EX_CLOSEWARN 1

#define MWF_LOG_ALL (MWF_LOG_NORMALTEMPLATES | MWF_LOG_SHOWTIME | MWF_LOG_SHOWSECONDS | \
        MWF_LOG_SHOWDATES | MWF_LOG_INDENT | MWF_LOG_TEXTFORMAT | MWF_LOG_SYMBOLS | MWF_LOG_INOUTICONS | \
        MWF_LOG_SHOWICONS | MWF_LOG_GRID | MWF_LOG_GROUPMODE | \
 		MWF_LOG_RTL | MWF_LOG_BBCODE | MWF_LOG_LOCALTIME/*MAD:*/ | \
		MWF_LOG_NEWLINE|MWF_LOG_UNDERLINE|MWF_LOG_SWAPNICK /*_MAD*/)

#define MWF_LOG_DEFAULT (MWF_LOG_GROUPMODE | MWF_LOG_SHOWTIME | MWF_LOG_NORMALTEMPLATES | MWF_LOG_SHOWDATES | MWF_LOG_SYMBOLS | MWF_LOG_GRID | MWF_LOG_INOUTICONS)

/*
 * custom dialog window messages
 */

#define TM_USER                  (WM_USER+300)

#define EM_SEARCHSCROLLER        (TM_USER+0x103)
#define EM_VALIDATEBOTTOM        (TM_USER+0x104)
#define EM_THEMECHANGED          (TM_USER+0x105)
#define EM_REFRESHWITHOUTCLIP    (TM_USER+0x106)

#define HM_EVENTSENT             (TM_USER+10)
#define DM_REMAKELOG             (TM_USER+11)
#define HM_DBEVENTADDED          (TM_USER+12)
#define DM_SETINFOPANEL          (TM_USER+13)
#define DM_OPTIONSAPPLIED        (TM_USER+14)
#define DM_SPLITTERMOVED         (TM_USER+15)
#define DM_SPLITSENDACK          (TM_USER+19)
#define DM_TYPING                (TM_USER+20)
#define DM_UPDATEWINICON         (TM_USER+21)
#define DM_UPDATELASTMESSAGE     (TM_USER+22)

#define DM_SELECTTAB             (TM_USER+23)
#define DM_CLOSETABATMOUSE       (TM_USER+24)
#define DM_STATUSICONCHANGE      (TM_USER+25)
#define DM_SETLOCALE             (TM_USER+26)
#define DM_QUERYLASTUNREAD       (TM_USER+28)
#define DM_QUERYPENDING          (TM_USER+29)
#define DM_UPDATEPICLAYOUT       (TM_USER+30)
#define DM_QUERYCONTAINER        (TM_USER+31)
#define DM_MUCFLASHWORKER        (TM_USER+32)
#define DM_APPENDMCEVENT         (TM_USER+34)
#define DM_CHECKINFOTIP		      (TM_USER+35)
#define DM_SAVESIZE              (TM_USER+36)
#define DM_CHECKSIZE             (TM_USER+37)
#define DM_FORCEREDRAW           (TM_USER+38)
#define DM_CONTAINERSELECTED     (TM_USER+39)
#define DM_CONFIGURECONTAINER    (TM_USER+40)
#define DM_QUERYHCONTACT         (TM_USER+41)
#define DM_DEFERREDREMAKELOG     (TM_USER+42)
#define DM_RESTOREWINDOWPOS      (TM_USER+43)
#define DM_QUERYCLIENTAREA       (TM_USER+45)
#define DM_QUERYRECENT           (TM_USER+47)
#define DM_ACTIVATEME            (TM_USER+46)
#define DM_STATUSMASKSET         (TM_USER+51)
#define DM_UPDATESTATUSMSG       (TM_USER+53)
#define DM_OWNNICKCHANGED        (TM_USER+55)
#define DM_CONFIGURETOOLBAR      (TM_USER+56)
#define DM_ACTIVATETOOLTIP       (TM_USER+58)
#define DM_UINTOCLIPBOARD        (TM_USER+59)
#define DM_FORCEDREMAKELOG       (TM_USER+62)
#define DM_STATUSBARCHANGED      (TM_USER+64)
#define DM_SETICON               (TM_USER+68)
#define DM_CHECKQUEUEFORCLOSE    (TM_USER+70)
#define DM_CHECKAUTOHIDE         (TM_USER+71)
#define DM_SETPARENTDIALOG       (TM_USER+72)
#define DM_HANDLECLISTEVENT      (TM_USER+73)
#define DM_TRAYICONNOTIFY        (TM_USER+74)
#define DM_REMOVECLISTEVENT      (TM_USER+75)
#define DM_DOCREATETAB           (TM_USER+77)
#define DM_SMILEYOPTIONSCHANGED  (TM_USER+85)
#define DM_MYAVATARCHANGED	      (TM_USER+86)
#define DM_IEVIEWOPTIONSCHANGED  (TM_USER+88)
#define DM_SPLITTERGLOBALEVENT   (TM_USER+89)
#define DM_CLIENTCHANGED         (TM_USER+91)
#define DM_SENDMESSAGECOMMANDW   (TM_USER+93)
#define DM_LOGSTATUSCHANGE	      (TM_USER+98)
#define DM_SC_BUILDLIST          (TM_USER+100)
#define DM_SC_INITDIALOG         (TM_USER+101)
#define DM_SC_CONFIG             (TM_USER+104)
#define DM_SCROLLIEVIEW          (TM_USER+102)
#define DM_UPDATEUIN             (TM_USER+103)

#define MINSPLITTERX         60
#define MINSPLITTERY         42
#define MINLOGHEIGHT         30
#define ERRORPANEL_HEIGHT    51

// wParam values for DM_SELECTTAB

#define DM_SELECT_NEXT		   1
#define DM_SELECT_PREV		   2
									   
#define DM_SELECT_BY_HWND	   3 // lParam specifies hwnd
#define DM_SELECT_BY_INDEX    4 // lParam specifies tab index
									   
#define DM_QUERY_NEXT         1
#define DM_QUERY_MOSTRECENT   2

// implement a callback for the rich edit. Without it, no bitmaps
// can be added to the richedit control.
// this class has to implement the GetNewStorage() method

#define MSGFONTID_MYMSG            0
#define MSGFONTID_MYMISC           1
#define MSGFONTID_YOURMSG          2
#define MSGFONTID_YOURMISC         3
#define MSGFONTID_MYNAME           4
#define MSGFONTID_MYTIME           5
#define MSGFONTID_YOURNAME         6
#define MSGFONTID_YOURTIME         7
#define H_MSGFONTID_MYMSG          8
#define H_MSGFONTID_MYMISC         9
#define H_MSGFONTID_YOURMSG       10
#define H_MSGFONTID_YOURMISC      11
#define H_MSGFONTID_MYNAME        12
#define H_MSGFONTID_MYTIME        13
#define H_MSGFONTID_YOURNAME      14
#define H_MSGFONTID_YOURTIME      15
#define MSGFONTID_MESSAGEAREA     16
#define H_MSGFONTID_STATUSCHANGES 17
#define H_MSGFONTID_DIVIDERS      18
#define MSGFONTID_ERROR           19
#define MSGFONTID_SYMBOLS_IN      20
#define MSGFONTID_SYMBOLS_OUT     21

#define IPFONTID_NICK              0
#define IPFONTID_UIN               1
#define IPFONTID_STATUS            2
#define IPFONTID_PROTO             3
#define IPFONTID_TIME              4

#define LOADHISTORY_UNREAD         0
#define LOADHISTORY_COUNT          1
#define LOADHISTORY_TIME           2

#define SRMSGSET_AUTOPOPUP         "AutoPopup"
#define SRMSGDEFSET_AUTOPOPUP      false
#define SRMSGSET_AUTOMIN           "AutoMin"
#define SRMSGDEFSET_AUTOMIN        0
#define SRMSGSET_SENDONENTER       "SendOnEnter"
#define SRMSGDEFSET_SENDONENTER    true
#define SRMSGSET_MSGTIMEOUT        "MessageTimeout"
#define SRMSGDEFSET_MSGTIMEOUT     30000
#define SRMSGSET_MSGTIMEOUT_MIN    5000 // minimum value (5 seconds)

#define SRMSGSET_LOADHISTORY       "LoadHistory"
#define SRMSGDEFSET_LOADHISTORY    LOADHISTORY_COUNT
#define SRMSGSET_LOADCOUNT         "LoadCount"
#define SRMSGDEFSET_LOADCOUNT      10
#define SRMSGSET_LOADTIME          "LoadTime"
#define SRMSGDEFSET_LOADTIME       10

#define SRMSGSET_BKGCOLOUR         "BkgColour"

#define SRMSGDEFSET_BKGCOLOUR      RGB(250,250,250)
//#define SRMSGDEFSET_BKGCOLOUR      GetSysColor(COLOR_WINDOW)
#define SRMSGDEFSET_BKGINCOLOUR    RGB(245,255,245)
#define SRMSGDEFSET_BKGOUTCOLOUR   RGB(245,245,255)

#define SRMSGSET_TYPING             "SupportTyping"
#define SRMSGSET_TYPINGNEW          "DefaultTyping"
#define SRMSGDEFSET_TYPINGNEW       1

#define SRMSGSET_TYPINGUNKNOWN      "UnknownTyping"
#define SRMSGDEFSET_TYPINGUNKNOWN   0

#define SRMSGSET_SHOWTYPING         "ShowTyping"
#define SRMSGDEFSET_SHOWTYPING      1

#define SRMSGSET_SHOWTYPINGWINFLASH "ShowTypingWinFlash"
#define SRMSGDEFSET_SHOWTYPINGWINFLASH 1

#define SRMSGSET_SHOWTYPINGNOWINOPEN "ShowTypingNoWinOpen"

#define SRMSGSET_SHOWTYPINGWINOPEN   "ShowTypingWinOpen"

#define SRMSGSET_SHOWTYPINGCLIST    "ShowTypingClist"
#define SRMSGDEFSET_SHOWTYPINGCLIST 1

// rtl support
#define SRMSGDEFSET_MOD_RTL  0

#define TIMEOUT_FLASHWND     900
#define TIMERID_HEARTBEAT    2
#define TIMEOUT_HEARTBEAT    20000
#define TIMERID_HOVER 10
#define TIMERID_HOVER_T 11

#define SRMSGMOD "SRMsg"
#define SRMSGMOD_T "Tab_SRMsg"
#define FONTMODULE "TabSRMM_Fonts"
#define CHAT_OLDFONTMODULE "TabSRMM_chat_Fonts"

#define IDM_STAYONTOP (WM_USER + 1)
#define IDM_NOTITLE (WM_USER + 2)
#define IDM_MOREOPTIONS (WM_USER +4)

// constants for the container management functions

#define CNT_ENUM_DELETE 1           // delete the target container...
#define CNT_ENUM_RENAME 2
#define CNT_ENUM_WRITEFLAGS 4

#define IDM_CONTAINERMENU 50500

#define EVENTTYPE_STATUSCHANGE 25368
#define EVENTTYPE_ERRMSG 25366

// hotkey modifiers...

#define HOTKEY_MODIFIERS_CTRLSHIFT 0
#define HOTKEY_MODIFIERS_CTRLALT 1
#define HOTKEY_MODIFIERS_ALTSHIFT 2

#include "../TabSRMM_icons/resource.h"         // icon pack values

struct TCpTable {
	UINT cpId;
	wchar_t *cpName;
};

#define LOI_TYPE_FLAG 1
#define LOI_TYPE_SETTING 2

struct TOptionListGroup
{
	LRESULT handle;
	wchar_t *szName;
};

struct TOptionListItem
{
	LRESULT handle;
	wchar_t *szName;
	UINT id;
	UINT uType;
	UINT_PTR lParam;
	UINT uGroup;
};

// sidebar button flags

#define SBI_TOP 1
#define SBI_BOTTOM 2
#define SBI_HIDDEN 4
#define SBI_DISABLED 8
#define SBI_TOGGLE   16
#define SBI_HANDLEBYCLIENT 32

// fixed stock button identifiers

#define IDC_SBAR_SLIST             1111
#define IDC_SBAR_FAVORITES         1112
#define IDC_SBAR_RECENT            1113
#define IDC_SBAR_SETUP             1114
#define IDC_SBAR_USERPREFS         1115
#define IDC_SBAR_TOGGLEFORMAT      1117
#define IDC_SBAR_CANCEL            1118

struct SIDEBARITEM {
	UINT    uId;
	DWORD   dwFlags;
	HICON   *hIcon, *hIconPressed, *hIconHover;
	wchar_t   *szName;
	void(*pfnAction)(ButtonItem *item, HWND hwndDlg, CSrmmWindow *dat, HWND hwndItem);
	void(*pfnCallback)(ButtonItem *item, HWND hwndDlg, CSrmmWindow *dat, HWND hwndItem);
	wchar_t   *tszTip;
};

#define FONTF_BOLD       1
#define FONTF_ITALIC     2
#define FONTF_UNDERLINE  4
#define FONTF_STRIKEOUT  8

#define RTFCACHELINESIZE 128

#define ID_EXTBKCONTAINER              0
#define ID_EXTBKBUTTONBAR              1
#define ID_EXTBKBUTTONSPRESSED         2
#define ID_EXTBKBUTTONSNPRESSED        3
#define ID_EXTBKBUTTONSMOUSEOVER       4
#define ID_EXTBKINFOPANEL              5
#define ID_EXTBKTITLEBUTTON            6
#define ID_EXTBKTITLEBUTTONMOUSEOVER   7
#define ID_EXTBKTITLEBUTTONPRESSED     8
#define ID_EXTBKTABPAGE                9
#define ID_EXTBKTABITEM               10
#define ID_EXTBKTABITEMACTIVE         11
#define ID_EXTBKTABITEMBOTTOM         12
#define ID_EXTBKTABITEMACTIVEBOTTOM   13
#define ID_EXTBKFRAME                 14
#define ID_EXTBKHISTORY               15
#define ID_EXTBKINPUTAREA             16
#define ID_EXTBKFRAMEINACTIVE         17
#define ID_EXTBKTABITEMHOTTRACK       18
#define ID_EXTBKTABITEMHOTTRACKBOTTOM 19
#define ID_EXTBKSTATUSBARPANEL        20
#define ID_EXTBKSTATUSBAR             21
#define ID_EXTBKUSERLIST              22
#define ID_EXTBKINFOPANELBG           23
#define ID_EXTBKSIDEBARBG             24
#define ID_EXTBK_LAST                 24

#define DEFAULT_SIDEBARWIDTH          30

#define THEME_READ_FONTS 1
#define THEME_READ_TEMPLATES 2
#define THEME_READ_ALL (THEME_READ_FONTS | THEME_READ_TEMPLATES)

#define IDC_TBFIRSTUID 10000            // first uId for custom buttons

#include "templates.h"

// callback for the user menu entry

#define MS_TABMSG_SETUSERPREFS	"SRMsg_MOD/SetUserPrefs"
#define MS_TABMSG_SLQMGR		"SRMsg_MOD/InvokeQmgr"

// show one of the tray menus
// wParam = 0 -> session list
// wParam = 1 -> tray menu
// lParam must be 0
#define MS_TABMSG_TRAYSUPPORT "SRMsg_MOD/Show_TrayMenu"

// the service which processes globally registered hotkeys
#define MS_TABMSG_HOTKEYPROCESS "SRMsg_MOD/ProcessHotkey"

#define MBF_DISABLED		0x01

#define TEMPLATES_MODULE "tabSRMM_Templates"
#define RTLTEMPLATES_MODULE "tabSRMM_RTLTemplates"

// custom tabSRMM events
#define tabMSG_WINDOW_EVT_CUSTOM_BEFORESEND 1

// temporary HPP API for emulating message log 
#define MS_HPP_EG_WINDOW "History++/ExtGrid/NewWindow"
#define MS_HPP_EG_EVENT  "History++/ExtGrid/Event"
#define MS_HPP_EG_UTILS  "History++/ExtGrid/Utils"
#define MS_HPP_EG_OPTIONSCHANGED "History++/ExtGrid/OptionsChanged"
#define MS_HPP_EG_NOTIFICATION   "History++/ExtGrid/Notification"

#define SB_CHAR_WIDTH			45					// default width for status bar panel #2
#define DEFAULT_CONTAINER_POS 	0x00400040			// default container position and size
#define DEFAULT_CONTAINER_SIZE 	0x019001f4

// core hotkey service ids
#define TABSRMM_HK_LASTUNREAD        2
#define TABSRMM_HK_LASTRECENT        4
#define TABSRMM_HK_PASTEANDSEND      8
#define TABSRMM_HK_SETUSERPREFS      9
#define TABSRMM_HK_CONTAINEROPTIONS 10
#define TABSRMM_HK_SENDFILE         12
#define TABSRMM_HK_QUOTEMSG         13
#define TABSRMM_HK_SEND             14
#define TABARMM_HK_TOGGLEINFOPANEL  16
#define TABSRMM_HK_HISTORY          17
#define TABSRMM_HK_TOGGLETOOLBAR    18
#define TABSRMM_HK_TOGGLEMULTISEND  19
#define TABSRMM_HK_TOGGLERTL        20
#define TABSRMM_HK_USERMENU         21
#define TABSRMM_HK_USERDETAILS      22
#define TABSRMM_HK_TOGGLEINFOPANEL  23
#define TABSRMM_HK_CLEARLOG         24
#define TABSRMM_HK_EDITNOTES        25
#define TABSRMM_HK_TOGGLESENDLATER  26
#define TABSRMM_HK_TOGGLESIDEBAR    27
#define TABSRMM_HK_CHANNELMGR       28
#define TABSRMM_HK_FILTERTOGGLE     29
#define TABSRMM_HK_LISTTOGGLE       30
#define TABSRMM_HK_MUC_SHOWSERVER   31
#define TABSRMM_HK_CLOSE_OTHER      32
#define TABSRMM_HK_SENDMENU         33
#define TABSRMM_HK_PROTOMENU        34

#define TABSRMM_HK_SECTION_IM LPGEN("Message windows - IM")
#define TABSRMM_HK_SECTION_GENERIC LPGEN("Message windows - all")
#define TABSRMM_HK_SECTION_GC LPGEN("Message windows - group chats")

// encryption status bar indicator
#define MSG_ICON_MODULE " TabSrmm"

#define MSG_ICON_SESSION 0
#define MSG_ICON_UTN     1
#define MSG_ICON_SOUND   2

int SI_InitStatusIcons();
int SI_DeinitStatusIcons();

struct SKINDESC
{
	ULONG   ulID;
	wchar_t tszName[30];
};

#define SKIN_NR_ELEMENTS 6
#define SKIN_VERSION	 2

// icon defintions (index into g_buttonBarIcons)
#define ICON_DEFAULT_SOUNDS 			22
#define ICON_DEFAULT_PULLDOWN			16
#define ICON_DEFAULT_LEFT				25
#define ICON_DEFAULT_RIGHT				28
#define ICON_DEFAULT_UP					26
#define ICON_DEFAULT_TYPING				5

#define ICON_BUTTON_ADD					0
#define ICON_BUTTON_CANCEL				6
#define ICON_BUTTON_SAVE				7

#endif /* _MSGS_H */
