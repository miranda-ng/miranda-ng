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

#ifndef _MSGS_H
#define _MSGS_H

/*
 * required for MingW32 compatibility
 */

#define MSGERROR_CANCEL	0
#define MSGERROR_RETRY	    1
#define MSGERROR_SENDLATER  2

#define EVENT_QUEUE_SIZE 10

#define CONTAINER_NAMELEN 25
#define TITLE_FORMATLEN 30

#define MWF_LOG_BBCODE		       0x01
#define MWF_LOG_LOCALTIME	       0x40
#define MWF_LOG_NORMALTEMPLATES  0x200
#define MWF_LOG_SHOWTIME         0x400
#define MWF_LOG_SHOWSECONDS      0x800
#define MWF_LOG_SHOWDATES       0x1000
#define MWF_LOG_NEWLINE         0x2000
#define MWF_LOG_INDENT          0x4000
#define MWF_LOG_RTL             0x8000
#define MWF_LOG_UNDERLINE      0x10000
#define MWF_LOG_SWAPNICK       0x20000
#define MWF_LOG_SHOWICONS      0x40000
#define MWF_LOG_SYMBOLS       0x200000
#define MWF_LOG_TEXTFORMAT   0x2000000
#define MWF_LOG_GRID         0x4000000
#define MWF_LOG_INOUTICONS  0x10000000
#define MWF_LOG_GROUPMODE   0x80000000

 /*
  * custom dialog window messages
  */

#define TM_USER                  (WM_USER+300)

#define EM_SEARCHSCROLLER        (TM_USER+0x103)
#define EM_VALIDATEBOTTOM        (TM_USER+0x104)
#define EM_THEMECHANGED          (TM_USER+0x105)
#define EM_REFRESHWITHOUTCLIP    (TM_USER+0x106)

#define HM_EVENTSENT             (TM_USER+10)
#define HM_DBEVENTADDED          (TM_USER+12)
#define DM_SETINFOPANEL          (TM_USER+13)
#define DM_OPTIONSAPPLIED        (TM_USER+14)
#define DM_SPLITSENDACK          (TM_USER+19)
#define DM_UPDATEWINICON         (TM_USER+21)
#define DM_UPDATELASTMESSAGE     (TM_USER+22)

#define DM_STATUSICONCHANGE      (TM_USER+25)
#define DM_CREATECONTAINER       (TM_USER+26)
#define DM_QUERYLASTUNREAD       (TM_USER+28)
#define DM_UPDATEPICLAYOUT       (TM_USER+30)
#define DM_APPENDMCEVENT         (TM_USER+34)
#define DM_CHECKINFOTIP          (TM_USER+35)
#define DM_SAVESIZE              (TM_USER+36)
#define DM_CHECKSIZE             (TM_USER+37)
#define DM_FORCEREDRAW           (TM_USER+38)
#define DM_QUERYHCONTACT         (TM_USER+41)
#define DM_STATUSMASKSET         (TM_USER+51)
#define DM_UPDATESTATUSMSG       (TM_USER+53)
#define DM_OWNNICKCHANGED        (TM_USER+55)
#define DM_CONFIGURETOOLBAR      (TM_USER+56)
#define DM_FORCEDREMAKELOG       (TM_USER+62)
#define DM_STATUSBARCHANGED      (TM_USER+64)
#define DM_CHECKQUEUEFORCLOSE    (TM_USER+70)
#define DM_CHECKAUTOHIDE         (TM_USER+71)
#define DM_HANDLECLISTEVENT      (TM_USER+73)
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

enum
{
	TMPL_MSGIN = 0,
	TMPL_MSGOUT,
	TMPL_GRPSTARTIN,
	TMPL_GRPSTARTOUT,
	TMPL_GRPINNERIN,
	TMPL_GRPINNEROUT,
	TMPL_STATUSCHG,
	TMPL_ERRMSG,
	TMPL_MAX
};

#define TEMPLATE_LENGTH 150
#define CUSTOM_COLORS 5

struct TTemplateSet {
	BOOL valid;             // all templates populated (may still contain crap.. so it's only half-assed safety :)
	wchar_t szTemplates[TMPL_MAX][TEMPLATE_LENGTH];      // the template strings
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

class CContactCache;
class CMenuBar;
class CMsgDialog;
class CProxyWindow;
class CSideBar;
class CTaskbarInteract;
class CThumbBase;

#define STICK_ICON_MSG 10

struct TLogTheme
{
	COLORREF  inbg, outbg, bg, oldinbg, oldoutbg, statbg, inputbg;
	COLORREF  hgrid;
	COLORREF  custom_colors[5];
	uint32_t  dwFlags;
	uint32_t  left_indent, right_indent;
	LOGFONTW *logFonts;
	COLORREF *fontColors;
	char     *rtfFonts;
	bool      isPrivate;
};

union TContainerFlags
{
	uint32_t dw;
	struct {
		bool m_bUnused1 : 1;
		bool m_bNoTitle : 1;
		bool m_bHideTabs : 1;
		bool m_bSideBar : 1;
		bool m_bNoFlash : 1;
		bool m_bSticky : 1;
		bool m_bDontReport : 1;
		bool m_bFlashAlways : 1;
		bool m_bTransparent : 1;
		bool m_bAvatarsOnTaskbar : 1;
		bool m_bDontReportFocused : 1;
		bool m_bGlobalSize : 1;
		bool m_bInfoPanel : 1;
		bool m_bNoSound : 1;
		bool m_bAutoSplitter : 1;
		bool m_bDeferredConfigure : 1;
		bool m_bCreateMinimized : 1;
		bool m_bNeedsUpdateTitle : 1;
		bool m_bDeferredResize : 1;
		bool m_bDontReportUnfocused : 1;
		bool m_bAlwaysReportInactive : 1;
		bool m_bNewContainerFlags : 1;
		bool m_bDeferredTabSelect : 1;
		bool m_bCreateCloned : 1;
		bool m_bNoStatusBar : 1;
		bool m_bNoMenuBar : 1;
		bool m_bTabsBottom : 1;
		bool m_bUnused2 : 1;
		bool m_bBottomToolbar : 1;
		bool m_bHideToolbar : 1;
		bool m_bUinStatusBar : 1;
		bool m_bVerticalMax : 1;
	};
};

union TContainerFlagsEx
{
	uint32_t dw;
	struct
	{
		bool m_bTabFlat : 1;
		bool m_bUnused1 : 1;
		bool m_bTabCloseButton : 1;
		bool m_bTabFlashIcon : 1;
		bool m_bTabFlashLabel : 1;
		bool m_bTabSingleRow : 1;
		bool m_bUnused2 : 1;
		bool m_bUnused3 : 1;
		bool m_bTabSBarLeft : 1;
		bool m_bTabSBarRight : 1;

		bool m_bSoundMinimized : 1;
		bool m_bSoundUnfocused : 1;
		bool m_bSoundInactive : 1;
		bool m_bSoundFocused : 1;
	};
};

struct TContainerSettings
{
	TContainerFlags flags;
	TContainerFlagsEx flagsEx;

	uint32_t   dwTransparency;
	uint32_t   panelheight;
	int     iSplitterX, iSplitterY;
	wchar_t szTitleFormat[TITLE_FORMATLEN + 2];
	uint16_t    avatarMode;
	uint16_t    ownAvatarMode;
	uint16_t    autoCloseSeconds;
	bool    fPrivate;
};

struct ButtonItem;

struct TContainerData : public MZeroedObject
{
	TContainerData() {}
	~TContainerData();

	TContainerData *pNext;
	TContainerFlags m_flags;
	TContainerFlagsEx m_flagsEx;

	HWND     m_hwndActive; // active message window
	HWND     m_hwnd;       // the container handle
	HWND     m_hwndTabs;   // tab control handle
	int      m_iTabIndex;  // next tab id
	int	   m_iChilds;
	int      m_iContainerIndex;
	bool	   m_bHidden;
	HWND     m_hwndTip;         // tab - tooltips...
	BOOL     m_bDontSmartClose; // if set, do not search and select the next possible tab after closing one.
	LONG     m_uChildMinHeight;
	int      m_tBorder;
	int	   m_tBorder_outer_left, m_tBorder_outer_right, m_tBorder_outer_top, m_tBorder_outer_bottom;
	MCONTACT m_hContactFrom;
	BOOL     m_isCloned;
	HWND     m_hwndStatus;
	int      m_statusBarHeight;
	uint32_t m_dwLastActivity;
	int      m_hIcon;               // current window icon stick indicator
	HICON	   m_hIconTaskbarOverlay; // contains a "sticky" taskbar overlay (e.g. new message icon)
	uint32_t m_dwFlashingStarted;
	HWND     m_hWndOptions;
	BOOL     m_bSizingLoop;
	HDC      m_cachedDC;
	HBITMAP  m_cachedHBM, m_oldHBM;
	SIZE     m_oldDCSize;
	RECT     m_rcClose, m_rcMin, m_rcMax;
	TitleBtn m_buttons[3];
	TitleBtn m_oldbuttons[3];
	int      m_ncActive;
	HWND     m_hwndSaved;
	RECT     m_rcSaved, m_rcLogSaved;
	POINT	   m_ptLogSaved;
	uint32_t m_exFlags;
	BOOL	   m_fPrivateThemeChanged;
	MARGINS  m_mOld;
	HDC      m_cachedToolbarDC;
	HBITMAP  m_hbmToolbarBG, m_oldhbmToolbarBG;
	SIZE	   m_szOldToolbarSize;
	SIZE     m_oldSize, m_preSIZE;
	uint16_t	m_avatarMode, m_ownAvatarMode;
	uint8_t	m_bTBRenderingMode;
	TLogTheme m_theme;
	CMenuBar *m_pMenuBar;
	CSideBar *m_pSideBar;
	ButtonItem *m_buttonItems;
	TTemplateSet *m_ltr_templates, *m_rtl_templates;
	CTaskbarInteract *m_pTaskBar;
	TContainerSettings *m_pSettings;

	wchar_t m_wszName[CONTAINER_NAMELEN + 4];		// container name
	wchar_t m_szRelThemeFile[MAX_PATH], m_szAbsThemeFile[MAX_PATH];

	void ActivateExistingTab(CMsgDialog *dat);
	void AdjustTabClientRect(RECT &rc);
	void ApplySetting(bool fForceResize = false);
	void BroadCastContainer(UINT message, WPARAM wParam, LPARAM lParam) const;
	void CloseTabByMouse(POINT *);
	void Configure();
	void FlashContainer(int iMode, int iCount);
	void InitDialog(HWND);
	void InitRedraw(void);
	void LoadOverrideTheme(void);
	void LoadThemeDefaults(void);
	void OptionsDialog(void);
	void QueryClientArea(RECT &rc);
	void QueryPending(void);
	void ReflashContainer(void);
	void Resize(bool, int newWidth);
	void RestoreWindowPos(void);
	void SelectTab(int iCommand, int idx = 0);
	void SetAeroMargins(void);
	void SetIcon(CMsgDialog *pDlg, HICON hIcon);
	void UpdateTabs(void);
	void UpdateTitle(MCONTACT, CMsgDialog* = nullptr);

	void ClearMargins()
	{	memset(&m_mOld, 0xfe, sizeof(m_mOld));
	}

	bool IsActive()
	{	return GetActiveWindow() == m_hwnd && GetForegroundWindow() == m_hwnd;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CLogWindow - built-in log window

class CLogWindow : public CRtfLogWindow
{
	typedef CRtfLogWindow CSuper;

public:
	CLogWindow(CMsgDialog &pDlg) :
		CSuper(pDlg)
	{
	}

	void Attach() override;
	void LogEvents(MEVENT hDbEventFirst, int count, bool bAppend) override;
	void LogEvents(struct LOGINFO *, bool) override;
	void ScrollToBottom() override;
	void UpdateOptions() override;

	void DisableStaticEdge()
	{
		SetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(m_rtf.GetHwnd(), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);
	}

	char* GetRichTextRtf(bool bText, bool bSelection)
	{
		return m_rtf.GetRichTextRtf(bText, bSelection);
	}

	void  LogEvents(MEVENT hDbEventFirst, int count, bool bAppend, DBEVENTINFO *dbei);
	void  ReplaceIcons(LONG startAt, int fAppend, BOOL isSent);
	void  ScrollToBottom(bool, bool);

	INT_PTR WndProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CMsgDialog - SRMM window class

class CMsgDialog : public CSrmmBaseDialog
{
	typedef CSrmmBaseDialog CSuper;
	friend class CInfoPanel;
	friend class CLogWindow;
	friend class CProxyWindow;

	std::map<CustomButtonData *, bool> mapHidden;

	__forceinline bool BB_IsDisplayed(CustomButtonData *cbd) const
	{
		return (!isChat() && cbd->m_bIMButton) || (isChat() && cbd->m_bChatButton);
	}

	void     BB_InitDlgButtons(void);
	void     BB_RefreshTheme(void);
	BOOL     BB_SetButtonsPos(void);
	void     BB_RedrawButtons(void);

	void     CB_DestroyAllButtons(void);
	void     CB_DestroyButton(uint32_t dwButtonCID, uint32_t dwFlags);
	void     CB_ChangeButton(CustomButtonData *cbd);
			   
	void     DM_AddDivider(void);
	HWND     DM_CreateClist(void);
	void     DM_DismissTip(const POINT& pt);
	void     DM_ErrorDetected(int type, int flag);
	void     DM_EventAdded(WPARAM wParam, LPARAM lParam);
	void     DM_FreeTheme(void);
	bool     DM_GenericHotkeysCheck(MSG *message);
	void     DM_HandleAutoSizeRequest(REQRESIZE *rr);
	void     DM_InitRichEdit(void);
	void     DM_InitTip(void);
	LRESULT  DM_MouseWheelHandler(WPARAM wParam, LPARAM lParam);
	LRESULT  DM_MsgWindowCmdHandler(UINT cmd, WPARAM wParam, LPARAM lParam);
	void     DM_NotifyTyping(int mode);
	void     DM_SetDBButtonStates();
	int      DM_SplitterGlobalEvent(WPARAM wParam, LPARAM lParam);
	void     DM_ThemeChanged(void);
	void     DM_Typing(bool fForceOff);
	void     DM_UpdateLastMessage(void) const;
			   
	void     AdjustBottomAvatarDisplay(void);
	void     CalcDynamicAvatarSize(BITMAP *bminfo);
	void     DetermineMinHeight(void);
	BOOL     DoRtfToTags(CMStringW &pszText) const;
	void     FindFirstEvent(void);
	int      FindRTLLocale(void);
	void     FlashOnClist(MEVENT hEvent, DBEVENTINFO *dbei);
	void     FlashTab(bool bInvertMode);
	LRESULT  GetSendButtonState();
	void     GetSendFormat(void);
	HICON    GetXStatusIcon() const;
	void     HandlePasteAndSend(void);
	void     Init(void);
	void     LoadContactAvatar(void);
	void     LoadOwnAvatar(void);
	void     LoadSplitter(void);
	void     PlayIncomingSound(void) const;
	LRESULT  ProcessHotkeysByMsgFilter(const CCtrlBase &pCtrl, UINT msg, WPARAM wParam, LPARAM lParam);
	void     SaveAvatarToFile(HBITMAP hbm, int isOwnPic);
	void 	   SendHBitmapAsFile(HBITMAP hbmp) const;
	void     ShowPopupMenu(const CCtrlBase&, POINT pt);
	void     StreamEvents(MEVENT, int, bool);
	void     UpdateWindowIcon(void);
	void     UpdateWindowState(UINT msg);
	void     VerifyProxy(void);
	LRESULT  WMCopyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);
			   
	uint16_t m_wStatus, m_wOldStatus;
	size_t   m_iSendBufferSize;
	int      m_iSendLength;				// message length in utf-8 octets
	HICON    m_hSmileyIcon;
	HWND     m_hwndContactPic, m_hwndPanelPic, m_hwndPanelPicParent;
	UINT     m_bbLSideWidth, m_bbRSideWidth;
	uint8_t  kstate[256];
			  
	RECT     m_rcNick, m_rcUIN, m_rcStatus, m_rcPic;
	int      m_originalSplitterY;
	SIZE     m_minEditBoxSize;
	uint32_t m_lastMessage;
	uint32_t m_dwTickLastEvent;
	HBITMAP  m_hOwnPic;
	SIZE     m_pic;

	CMStringW m_szStatusText;
	HICON    m_szStatusIcon;
	bool     m_bStatusSet;
			  
	bool     m_bShowInfoAvatar, m_bShowUIElements;
	bool     m_bFlashClist, m_bScrollingDisabled, m_bAwayMsgTimer;
	bool     m_bDelayedSplitter, m_bWarnClose;
	bool     m_bUseOffset;
	bool     m_bkeyProcessed;
	bool     m_fLimitedUpdate;
	bool     m_bClrAdded;
	bool     m_bInsertMode, m_bInitMode = true;
	bool     m_bDeferredScroll, m_bDeferredRemakeLog;
	bool     m_bWasBackgroundCreate;

	int      m_iRealAvatarHeight;
	int      m_iButtonBarReallyNeeds;
	uint32_t m_dwLastActivity;
	MEVENT   m_hFlashingEvent;
	int      m_SendFormat;
	LCID     m_lcid;
	wchar_t  m_lcID[10];
	int      m_iPanelAvatarX, m_iPanelAvatarY;
	HWND     m_hwndTip;
	uint32_t m_panelStatusCX;
	int      m_textLen;         // current text len
	LONG     m_ipFieldHeight;
	WPARAM   m_wParam;          // used for "delayed" actions like moved splitters in minimized windows
	LPARAM   m_lParam;
	int      m_iHaveRTLLang;
			  
	uint32_t m_iSplitterSaved;
	POINT    m_ptTipActivation;

protected:
	void     GetMYUIN();

public:
	char*    m_szProto;
	int      m_iTabID;
	int      m_iLogMode;

	bool     m_bIsHistory, m_bNotOnList, m_bIsIdle;
	bool     m_bActualHistory;
	bool     m_bIsAutosizingInput;
	bool     m_bCanFlashTab, m_bTabFlash;
	bool     m_bEditNotesActive;
	bool     m_bShowAvatar;
	bool     m_bSaveBtn, m_bNeedCheckSize;
	bool     m_bForcedClose;
	bool     m_bErrorState;
	bool     m_bDividerWanted, m_bDividerSet;
	bool     m_bSplitterOverride;
	bool     m_bRtlText;
	bool     m_bLastParaDeleted;

	int      m_sendMode;
	HKL      m_hkl;                                    // keyboard layout identifier
	uint32_t m_idle;
	uint32_t m_dwFlags;
	uint32_t m_dwUnread;
	HANDLE   m_hTheme, m_hThemeIP, m_hThemeToolbar;
	HICON    m_hXStatusIcon, m_hTabStatusIcon, m_hTabIcon, m_iFlashIcon, m_hTaskbarIcon, m_hClientIcon;
	MEVENT   m_hDbEventFirst, m_hDbEventLast;
	HANDLE   m_hTimeZone;
	MEVENT*  m_hHistoryEvents;
	time_t   m_lastEventTime;
	int      m_iLastEventType;
	int      m_iOpenJobs;
	int      m_iInputAreaHeight = -1;
	int      m_maxHistory, m_curHistory;
	int      m_iCurrentQueueError;
	int      m_iSplitterY, m_dynaSplitter;
	int      m_savedSplitterY, m_savedDynaSplit;
	char*    m_sendBuffer;

	wchar_t  m_wszMyNickname[130];
	wchar_t  m_wszStatus[50];
	wchar_t  m_wszTitle[130];        // tab title...
	wchar_t  m_myUin[80];
	wchar_t  m_wszStatusBar[100];
	char     m_szMicroLf[128];

	int      m_iMultiSplit;
	int      msgTop, rcLogBottom;
	bool     m_bActivate, m_bWantPopup, m_bIsMeta;

	wchar_t* wszInitialText;
	TOOLINFO ti;
	CInfoPanel m_pPanel;
	CProxyWindow *m_pWnd;	// proxy window object (win7+, for taskbar support).
	CContactCache *m_cache;
	TContainerData *m_pContainer;		// parent container description structure
	AVATARCACHEENTRY *m_ace, *m_ownAce;

	static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool TabAutoComplete();

	HWND m_hwndFilter;
	int m_iSearchItem;
	BOOL m_iSavedSpaces;
	wchar_t m_wszSearch[255];
	wchar_t *m_wszSearchQuery, *m_wszSearchResult;
	SESSION_INFO *m_pLastSession;

	CTimer timerAwayMsg;
	CCtrlButton m_btnAdd, m_btnQuote, m_btnCancelAdd;

public:
	CMsgDialog(int dlgId, MCONTACT hContact);
	CMsgDialog(SESSION_INFO *si);
	~CMsgDialog();

	void onClick_Ok(CCtrlButton *);
	void onClick_Add(CCtrlButton *);
	void onClick_Quote(CCtrlButton *);
	void onClick_Filter(CCtrlButton *);
	void onClick_CancelAdd(CCtrlButton *);
	void onClick_ShowNickList(CCtrlButton *);

	void onType(CTimer *);
	void onFlash(CTimer *);
	void onAwayMsg(CTimer *);

	void onChange_Message(CCtrlEdit *);

	void onDblClick_List(CCtrlListBox *);

	int OnFilter(MSGFILTER *);

	bool OnInitDialog() override;
	void OnDestroy() override;
	int Resizer(UTILRESIZECONTROL *urc) override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void AddLog() override;
	void CloseTab() override;
	bool IsActive() const override;
	void LoadSettings() override;
	void SetStatusText(const wchar_t *, HICON) override;
	void ShowFilterMenu() override;
	void UpdateNickList() override;
	void UpdateOptions() override;
	void UpdateStatusBar() override;
	void UpdateTitle() override;

	CThumbBase* tabCreateThumb(CProxyWindow*) const;
	void tabClearLog();
	void tabUpdateStatusBar() const;

	static LONG_PTR CALLBACK StatusBarSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	__forceinline CCtrlRichEdit& GetEntry() { return m_message; }

	__forceinline void ActivateTab() {
		m_pContainer->ActivateExistingTab(this);
	}

	__forceinline void ClearTyping() {
		m_nTypeSecs = 0;
		m_bShowTyping = 0;
		m_wszStatusBar[0] = 0;
		PostMessage(m_hwnd, DM_UPDATELASTMESSAGE, 0, 0);
	}

	__forceinline CLogWindow* LOG() {
		return ((CLogWindow *)m_pLog);
	}

	void LogEvent(DBEVENTINFO &dbei);

	void    DM_OptionsApplied(bool bRemakeLog = true);
	void    DM_RecalcPictureSize(void);
	void    DM_ScrollToBottom(WPARAM wParam, LPARAM lParam);
		     
	void    ActivateTooltip(int iCtrlId, const wchar_t *pwszMessage);
	void    CheckStatusIconClick(POINT pt, const RECT &rc, int gap, int code);
	void    DrawStatusIcons(HDC hDC, const RECT &rc, int gap);
	void    EnableSendButton(bool bMode) const;
	void    EnableSending(bool bMode) const;
	void    FormatRaw(CMStringW&, int flags, bool isSent);
	bool    FormatTitleBar(const wchar_t *szFormat, CMStringW &dest);
	bool    GetAvatarVisibility(void);
	void    GetClientIcon(void);
	HICON   GetMyContactIcon(LPCSTR szSetting);
	void    GetMyNick(void);
	HICON   IconFromAvatar(void) const;
	void    KbdState(bool &isShift, bool &isControl, bool &isAlt);
	void    LimitMessageText(int iLen);
	int     LoadLocalFlags(void);
	bool    MustPlaySound(void) const;
	void    NotifyDeliveryFailure(void) const;
	void    RemakeLog(void);
	void    SaveSplitter(void);
	void    SelectContainer(void);
	void    SetDialogToType(void);
	void    ShowPicture(bool showNewPic);
	void    SplitterMoved(int x, HWND hwnd);
	void    SwitchToContainer(const wchar_t *szNewName);
	int     Typing(int secs);
	void    UpdateReadChars(void) const;
	void    UpdateSaveAndSendButton(void);

	int     MsgWindowDrawHandler(DRAWITEMSTRUCT *dis);
	int     MsgWindowMenuHandler(int selection, int menuId);
	int     MsgWindowUpdateMenu(HMENU submenu, int menuID);
		     
	void    RenderToolbarBG(HDC hdc, const RECT &rcWindow) const;
	void    UpdateToolbarBG(void);
};

extern LIST<void> g_arUnreadWindows;

#define MESSAGE_WINDOW_DATA_SIZE offsetof(_MessageWindowData, hdbEventFirst);

/*
 * configuration data for custom tab ctrl
 */

struct myTabCtrl
{
	HPEN    m_hPenShadow, m_hPenItemShadow, m_hPenLight;
	HFONT   m_hMenuFont;
	COLORREF colors[10];
	HBRUSH  m_brushes[4];
	uint32_t   m_fixedwidth;
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
	wchar_t *szName;
	wchar_t *szDesc;
	HICON   *phIcon;       // where the handle is saved...
	INT_PTR  uId;           // icon ID
	BOOL     bForceSmall;   // true: force 16x16
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

#define MIN_PANELHEIGHT 20

#define CNT_TRANS_DEFAULT 0x00ff00ff

#define CNT_FLAGSEX_DEFAULT (m_bTabFlashIcon | m_bSoundMinimized | m_bSoundUnfocused | m_bSoundInactive | m_bSoundFocused)

#define CNT_CREATEFLAG_CLONED 1
#define CNT_CREATEFLAG_MINIMIZED 2

#define CNT_EX_CLOSEWARN 1

#define MWF_LOG_ALL (MWF_LOG_NORMALTEMPLATES | MWF_LOG_SHOWTIME | MWF_LOG_SHOWSECONDS | \
        MWF_LOG_SHOWDATES | MWF_LOG_INDENT | MWF_LOG_TEXTFORMAT | MWF_LOG_SYMBOLS | MWF_LOG_INOUTICONS | \
        MWF_LOG_SHOWICONS | MWF_LOG_GRID | MWF_LOG_GROUPMODE | \
 		MWF_LOG_RTL | MWF_LOG_BBCODE | MWF_LOG_LOCALTIME/*MAD:*/ | \
		MWF_LOG_NEWLINE|MWF_LOG_UNDERLINE|MWF_LOG_SWAPNICK /*_MAD*/)

#define MWF_LOG_DEFAULT (MWF_LOG_GROUPMODE | MWF_LOG_SHOWTIME | MWF_LOG_NORMALTEMPLATES | MWF_LOG_SHOWDATES | MWF_LOG_SYMBOLS | MWF_LOG_GRID | MWF_LOG_INOUTICONS)

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
	HTREEITEM handle;
	wchar_t *szName;
};

struct TOptionListItem
{
	HTREEITEM handle;
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

struct SIDEBARITEM
{
	UINT    uId;
	uint32_t   dwFlags;
	HICON *hIcon, *hIconPressed, *hIconHover;
	wchar_t *szName;
	void (*pfnAction)(ButtonItem *item, HWND hwndDlg, CMsgDialog *dat, HWND hwndItem);
	void (*pfnCallback)(ButtonItem *item, HWND hwndDlg, CMsgDialog *dat, HWND hwndItem);
	wchar_t *tszTip;
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

// callback for the user menu entry

#define MS_TABMSG_SETUSERPREFS  "SRMsg_MOD/SetUserPrefs"
#define MS_TABMSG_SLQMGR        "SRMsg_MOD/InvokeQmgr"

// the service which processes globally registered hotkeys
#define MS_TABMSG_HOTKEYPROCESS "SRMsg_MOD/ProcessHotkey"

#define MBF_DISABLED		0x01

#define TEMPLATES_MODULE "tabSRMM_Templates"
#define RTLTEMPLATES_MODULE "tabSRMM_RTLTemplates"

// custom tabSRMM events
#define tabMSG_WINDOW_EVT_CUSTOM_BEFORESEND 1

#define SB_CHAR_WIDTH                45               // default width for status bar panel #2

#define DEFAULT_CONTAINER_POS        0x00400040       // default container position and size
#define DEFAULT_CONTAINER_SIZE       0x019001f4

// core hotkey service ids
#define TABSRMM_HK_LASTUNREAD        2
#define TABSRMM_HK_LASTRECENT        4
#define TABSRMM_HK_PASTEANDSEND      8
#define TABSRMM_HK_SETUSERPREFS      9
#define TABSRMM_HK_CONTAINEROPTIONS 10
#define TABSRMM_HK_SENDFILE         12
#define TABSRMM_HK_QUOTEMSG         13
#define TABSRMM_HK_CLEARMSG         15
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

#define MSG_ICON_UTN     0
#define MSG_ICON_SOUND   1

int SI_InitStatusIcons();

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

struct CMPlugin : public PLUGIN<CMPlugin>
{
	HANDLE hLogger;

	CMOption<bool> bPopups;

	CMPlugin();

	int Load() override;
	int Unload() override;
};

#endif /* _MSGS_H */
