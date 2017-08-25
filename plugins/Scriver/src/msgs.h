/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#ifndef SRMM_MSGS_H
#define SRMM_MSGS_H

#define MSGERROR_CANCEL	0
#define MSGERROR_RETRY	1
#define MSGERROR_DONE	2

struct ToolbarButton
{
	wchar_t *name;
	UINT controlId;
	int alignment;
	int spacing;
	int width;
};

struct TabCtrlData
{
	int lastClickTime;
	WPARAM clickWParam;
	LPARAM clickLParam;
	POINT mouseLBDownPos;
	HIMAGELIST hDragImageList;
	int bDragging;
	int bDragged;
	int destTab;
	int srcTab;
};

struct ParentWindowData
{
	HWND hwnd;
	MCONTACT hContact;
	int childrenCount;
	HWND hwndActive;
	HWND hwndStatus;
	HWND hwndTabs;
	DWORD flags2;
	RECT childRect;
	POINT mouseLBDownPos;
	int mouseLBDown;
	int nFlash;
	int nFlashMax;
	int bMinimized;
	int bVMaximized;
	int bTopmost;
	int iSplitterX, iSplitterY;

	int windowWasCascaded;
	TabCtrlData *tabCtrlDat;
	BOOL isChat;
	ParentWindowData *prev, *next;
};

#define NMWLP_INCOMING 1

class CScriverWindow : public CSrmmBaseDialog
{
	typedef CSrmmBaseDialog CSuper;

protected:
	CScriverWindow(int iDialog, SESSION_INFO* = nullptr);

	int InputAreaShortcuts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual void CloseTab() override;
	virtual void LoadSettings() override;
	virtual void SetStatusText(const wchar_t*, HICON) override;

	void Reattach(HWND hwndContainer);

	ParentWindowData *m_pParent;
	int m_minLogBoxHeight, m_minEditBoxHeight;
	HWND m_hwndIeview;
	TCmdList *cmdList, *cmdListCurrent;
};

class CSrmmWindow : public CScriverWindow
{
	typedef CScriverWindow CSuper;

	CCtrlButton m_btnOk, m_btnAdd, m_btnUserMenu, m_btnQuote, m_btnDetails;
	CSplitter m_splitter;

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam);

	bool   m_bIncoming, m_bShowTyping;
	
	MEVENT m_hDbEventFirst, m_hDbEventLast, m_hDbUnreadEventFirst;
	SIZE   m_toolbarSize;
	int    m_iWindowWasCascaded;
	int    m_nTypeSecs, m_nTypeMode, m_nLastTyping;
	int    m_iShowUnread;
	WORD   m_wStatus;
	DWORD  m_lastMessage;
	int    m_iMessagesInProgress;
	int    m_iSendAllConfirm;
	HICON  m_hStatusIcon, m_hStatusIconBig, m_hStatusIconOverlay;

	void   GetContactUniqueId(char *buf, int maxlen);
	HICON  GetTabIcon();
	void   GetTitlebarIcon(struct TitleBarData *tbd);
	void   MessageDialogResize(int w, int h);
	void   ShowAvatar();
	void   SetDialogToType();
	void   SetStatusIcon();
	void   StreamInEvents(MEVENT hDbEventFirst, int count, int fAppend);
	void   UpdateReadChars();

	bool   IsTypingNotificationEnabled();
	bool   IsTypingNotificationSupported();
	void   NotifyTyping(int mode);

public:  // info bar support
	HWND   m_hwndInfo;
	HWND   m_hXStatusTip;

	void   CreateInfobar();
	void   SetupInfobar();
	void   RefreshInfobar();

public:
	char *m_szProto;
	time_t m_startTime, m_lastEventTime;
	int m_lastEventType;
	int m_isMixed;
	bool m_bUseRtl, m_bUseIEView;

	wchar_t *m_wszInitialText;
	HBITMAP m_hbmpAvatarPic;
	AVATARCACHEENTRY *m_ace;

public:
	CSrmmWindow(MCONTACT hContact, bool bIncoming);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual void UpdateStatusBar() override;
	virtual void UpdateTitle() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void onClick_Ok(CCtrlButton*);
	void onClick_Add(CCtrlButton*);
	void onClick_Quote(CCtrlButton*);
	void onClick_Details(CCtrlButton*);
	void onClick_History(CCtrlButton*);
	void onClick_UserMenu(CCtrlButton*);

	void onChange_Message(CCtrlEdit*);

	void onChanged_Splitter(CSplitter*);
};

class CChatRoomDlg : public CScriverWindow
{
	typedef CScriverWindow CSuper;

	CCtrlButton m_btnOk;
	CSplitter m_splitterX, m_splitterY;

	void MessageDialogResize(int w, int h);
	void TabAutoComplete(void);

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam) override;

	static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	wchar_t m_wszSearch[255];
	wchar_t *m_wszSearchQuery, *m_wszSearchResult;
	SESSION_INFO *m_pLastSession;

public:
	CChatRoomDlg(SESSION_INFO *si);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	
	virtual void RedrawLog() override;
	virtual void ScrollToBottom() override;
	virtual void ShowFilterMenu() override;
	virtual void StreamInEvents(LOGINFO* lin, bool bRedraw) override;
	virtual void UpdateNickList() override;
	virtual void UpdateOptions() override;
	virtual void UpdateStatusBar() override;
	virtual void UpdateTitle() override;

	void onChange_Message(CCtrlEdit*);

	void onClick_Ok(CCtrlButton*);
	void onClick_Filter(CCtrlButton*);
	void onClick_ShowList(CCtrlButton*);

	void OnSplitterX(CSplitter*);
	void OnSplitterY(CSplitter*);

	void FixTabIcons();
};

#define HM_DBEVENTADDED        (WM_USER+10)
#define DM_REMAKELOG           (WM_USER+11)
#define DM_CASCADENEWWINDOW    (WM_USER+13)
#define DM_OPTIONSAPPLIED      (WM_USER+14)
#define DM_APPENDTOLOG         (WM_USER+17)
#define DM_ERRORDECIDED        (WM_USER+18)
#define DM_SCROLLLOGTOBOTTOM   (WM_USER+19)
#define DM_TYPING              (WM_USER+20)
#define DM_UPDATELASTMESSAGE   (WM_USER+22)
#define DM_USERNAMETOCLIP      (WM_USER+23)
#define DM_CHANGEICONS         (WM_USER+24)
#define DM_UPDATEICON          (WM_USER+25)
#define DM_GETAVATAR           (WM_USER+27)
#define HM_ACKEVENT            (WM_USER+29)

#define DM_SENDMESSAGE         (WM_USER+30)
#define DM_STARTMESSAGESENDING (WM_USER+31)
#define DM_SHOWMESSAGESENDING  (WM_USER+32)
#define DM_STOPMESSAGESENDING  (WM_USER+33)
#define DM_SHOWERRORMESSAGE    (WM_USER+34)

#define DM_CLEARLOG            (WM_USER+46)
#define DM_SWITCHSTATUSBAR     (WM_USER+47)
#define DM_SWITCHTOOLBAR       (WM_USER+48)
#define DM_SWITCHTITLEBAR      (WM_USER+49)
#define DM_SWITCHINFOBAR       (WM_USER+50)
#define DM_SWITCHRTL           (WM_USER+51)
#define DM_SWITCHTYPING        (WM_USER+53)
#define DM_MESSAGESENDING      (WM_USER+54)
#define DM_STATUSICONCHANGE    (WM_USER+56)

#define DM_MYAVATARCHANGED     (WM_USER+62)
#define DM_PROTOAVATARCHANGED  (WM_USER+63)
#define DM_AVATARCHANGED       (WM_USER+64)

#define EM_SUBCLASSED          (WM_USER+0x101)
#define EM_UNSUBCLASSED        (WM_USER+0x102)

#define EVENTTYPE_JABBER_CHATSTATES	2000
#define EVENTTYPE_JABBER_PRESENCE	2001

class CErrorDlg : public CDlgBase
{
	ptrW m_wszText;
	CMStringW m_wszName, m_wszDescr;
	MessageSendQueueItem *m_queueItem;
	HWND m_hwndParent;

	CCtrlButton m_btnOk, m_btnCancel;

protected:
	virtual void OnInitDialog() override;

public:
	CErrorDlg(const wchar_t *pwszDescr, HWND, MessageSendQueueItem*);

	void onOk(CCtrlButton*);
	void onCancel(CCtrlButton*);
};

int DbEventIsShown(DBEVENTINFO &dbei);
int DbEventIsCustomForMsgWindow(DBEVENTINFO *dbei);
int DbEventIsMessageOrCustom(DBEVENTINFO *dbei);
void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);
int IsAutoPopup(MCONTACT hContact);

#define MSGFONTID_MYMSG           0
#define MSGFONTID_YOURMSG         1
#define MSGFONTID_MYNAME          2
#define MSGFONTID_MYTIME          3
#define MSGFONTID_MYCOLON         4
#define MSGFONTID_YOURNAME        5
#define MSGFONTID_YOURTIME        6
#define MSGFONTID_YOURCOLON       7
#define MSGFONTID_MESSAGEAREA     8
#define MSGFONTID_NOTICE          9
#define MSGFONTID_MYURL          10
#define MSGFONTID_YOURURL        11
#define MSGFONTID_INFOBAR_NAME   12
#define MSGFONTID_INFOBAR_STATUS 13

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour);
extern int fontOptionsListSize;

#define LOADHISTORY_UNREAD		0
#define LOADHISTORY_COUNT		1
#define LOADHISTORY_TIME		2

#define SRMM_MODULE						"SRMM"

#define SRMSGSET_USETABS			"UseTabs"
#define SRMSGDEFSET_USETABS			1
#define SRMSGSET_TABSATBOTTOM		"TabsPosition"
#define SRMSGDEFSET_TABSATBOTTOM	0
#define SRMSGSET_TABCLOSEBUTTON		"TabCloseButton"
#define SRMSGDEFSET_TABCLOSEBUTTON	0
#define SRMSGSET_LIMITNAMES			"LimitNamesOnTabs"
#define SRMSGDEFSET_LIMITNAMES		1
#define SRMSGSET_LIMITNAMESLEN		"LimitNamesLength"
#define SRMSGDEFSET_LIMITNAMESLEN	20
#define SRMSGSET_HIDEONETAB			"HideOneTab"
#define SRMSGDEFSET_HIDEONETAB		1
#define SRMSGSET_SEPARATECHATSCONTAINERS "SeparateChatsContainers"
#define SRMSGDEFSET_SEPARATECHATSCONTAINERS 0
#define SRMSGSET_LIMITTABS			"LimitTabs"
#define SRMSGDEFSET_LIMITTABS		0
#define SRMSGSET_LIMITTABSNUM		"LimitTabsNum"
#define SRMSGDEFSET_LIMITTABSNUM	10
#define SRMSGSET_LIMITCHATSTABS		"LimitChatsTabs"
#define SRMSGDEFSET_LIMITCHATSTABS	0
#define SRMSGSET_LIMITCHATSTABSNUM	"LimitChatsTabsNum"
#define SRMSGDEFSET_LIMITCHATSTABSNUM 10

#define SRMSGSET_CASCADE			"Cascade"
#define SRMSGDEFSET_CASCADE			1
#define SRMSGSET_SAVEPERCONTACT		"SavePerContact"
#define SRMSGDEFSET_SAVEPERCONTACT	0
#define SRMSGSET_SHOWTITLEBAR		"ShowTitleBar"
#define SRMSGDEFSET_SHOWTITLEBAR	1
#define SRMSGSET_SHOWSTATUSBAR		"ShowStatusBar"
#define SRMSGDEFSET_SHOWSTATUSBAR	1
#define SRMSGSET_SHOWINFOBAR		"ShowInfoBar"
#define SRMSGDEFSET_SHOWINFOBAR		1
#define SRMSGSET_TOPMOST			"Topmost"
#define SRMSGDEFSET_TOPMOST			0
#define SRMSGSET_POPFLAGS			"PopupFlags"
#define SRMSGDEFSET_POPFLAGS		0
#define SRMSGSET_SHOWBUTTONLINE		"ShowButtonLine"
#define SRMSGDEFSET_SHOWBUTTONLINE	1
#define SRMSGSET_SHOWINFOLINE		"ShowInfoLine"
#define SRMSGDEFSET_SHOWINFOLINE	1
#define SRMSGSET_SHOWPROGRESS		"ShowProgress"
#define SRMSGDEFSET_SHOWPROGRESS	0
#define SRMSGSET_AUTOPOPUP			"AutoPopupMsg"
#define SRMSGDEFSET_AUTOPOPUP		0
#define SRMSGSET_STAYMINIMIZED		"StayMinimized"
#define SRMSGDEFSET_STAYMINIMIZED	0
#define SRMSGSET_SWITCHTOACTIVE		"SwitchToActiveTab"
#define SRMSGDEFSET_SWITCHTOACTIVE	0
#define SRMSGSET_AUTOMIN			"AutoMin"
#define SRMSGDEFSET_AUTOMIN			0
#define SRMSGSET_AUTOCLOSE			"AutoClose"
#define SRMSGDEFSET_AUTOCLOSE		0
#define SRMSGSET_SENDONENTER		"SendOnEnter"
#define SRMSGDEFSET_SENDONENTER		1
#define SRMSGSET_SENDONDBLENTER		"SendOnDblEnter"
#define SRMSGDEFSET_SENDONDBLENTER	0
#define SRMSGSET_STATUSICON			"UseStatusWinIcon"
#define SRMSGDEFSET_STATUSICON		0
#define SRMSGSET_SENDBUTTON			"UseSendButton"
#define SRMSGDEFSET_SENDBUTTON		0
#define SRMSGSET_CHARCOUNT			"ShowCharCount"
#define SRMSGDEFSET_CHARCOUNT		0
#define SRMSGSET_CTRLSUPPORT		"SupportCtrlUpDn"
#define SRMSGDEFSET_CTRLSUPPORT		1
#define SRMSGSET_DELTEMP			"DeleteTempCont"
#define SRMSGDEFSET_DELTEMP			0
#define SRMSGSET_MSGTIMEOUT			"MessageTimeout"
#define SRMSGDEFSET_MSGTIMEOUT		10000
#define SRMSGSET_FLASHCOUNT			"FlashMax"
#define SRMSGDEFSET_FLASHCOUNT		3

#define SRMSGSET_LOADHISTORY		"LoadHistory"
#define SRMSGDEFSET_LOADHISTORY		LOADHISTORY_UNREAD
#define SRMSGSET_LOADCOUNT			"LoadCount"
#define SRMSGDEFSET_LOADCOUNT		10
#define SRMSGSET_LOADTIME			"LoadTime"
#define SRMSGDEFSET_LOADTIME		10

#define SRMSGSET_USELONGDATE		"UseLongDate"
#define SRMSGDEFSET_USELONGDATE		0
#define SRMSGSET_SHOWSECONDS		"ShowSeconds"
#define SRMSGDEFSET_SHOWSECONDS		1
#define SRMSGSET_USERELATIVEDATE	"UseRelativeDate"
#define SRMSGDEFSET_USERELATIVEDATE	0

#define SRMSGSET_GROUPMESSAGES		"GroupMessages"
#define SRMSGDEFSET_GROUPMESSAGES	0
#define SRMSGSET_MARKFOLLOWUPS		"MarkFollowUps"
#define SRMSGDEFSET_MARKFOLLOWUPS	0
#define SRMSGSET_MESSAGEONNEWLINE	"MessageOnNewLine"
#define SRMSGDEFSET_MESSAGEONNEWLINE 0
#define SRMSGSET_DRAWLINES			"DrawLines"
#define SRMSGDEFSET_DRAWLINES		0

#define SRMSGSET_INDENTTEXT			"IndentText"
#define SRMSGDEFSET_INDENTTEXT		0
#define SRMSGSET_INDENTSIZE			"IndentSize"
#define SRMSGDEFSET_INDENTSIZE		0

#define SRMSGSET_SHOWLOGICONS		"ShowLogIcon"
#define SRMSGDEFSET_SHOWLOGICONS	1
#define SRMSGSET_HIDENAMES			"HideNames"
#define SRMSGDEFSET_HIDENAMES		1
#define SRMSGSET_SHOWTIME			"ShowTime"
#define SRMSGDEFSET_SHOWTIME		1
#define SRMSGSET_SHOWDATE			"ShowDate"
#define SRMSGDEFSET_SHOWDATE		0
#define SRMSGSET_BKGCOLOUR				"BkgColour"
#define SRMSGDEFSET_BKGCOLOUR			GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INPUTBKGCOLOUR			"InputBkgColour"
#define SRMSGDEFSET_INPUTBKGCOLOUR		GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INCOMINGBKGCOLOUR		"IncomingBkgColour"
#define SRMSGDEFSET_INCOMINGBKGCOLOUR	GetSysColor(COLOR_WINDOW)
#define SRMSGSET_OUTGOINGBKGCOLOUR		"OutgoingBkgColour"
#define SRMSGDEFSET_OUTGOINGBKGCOLOUR	GetSysColor(COLOR_WINDOW)
#define SRMSGSET_INFOBARBKGCOLOUR		"InfobarBkgColour"
#define SRMSGDEFSET_INFOBARBKGCOLOUR	GetSysColor(COLOR_3DLIGHT)
#define SRMSGSET_LINECOLOUR				"LineColour"
#define SRMSGDEFSET_LINECOLOUR			GetSysColor(COLOR_WINDOW)

#define SRMSGSET_USEIEVIEW			"UseIEView"
#define SRMSGDEFSET_USEIEVIEW		1


#define SRMSGSET_TYPING				"SupportTyping"
#define SRMSGSET_TYPINGNEW			"DefaultTyping"
#define SRMSGDEFSET_TYPINGNEW		1
#define SRMSGSET_TYPINGUNKNOWN		"UnknownTyping"
#define SRMSGDEFSET_TYPINGUNKNOWN	0
#define SRMSGSET_SHOWTYPING			"ShowTyping"
#define SRMSGDEFSET_SHOWTYPING		1
#define SRMSGSET_SHOWTYPINGWIN		"ShowTypingWin"
#define SRMSGDEFSET_SHOWTYPINGWIN	1
#define SRMSGSET_SHOWTYPINGNOWIN	"ShowTypingTray"
#define SRMSGDEFSET_SHOWTYPINGNOWIN	0
#define SRMSGSET_SHOWTYPINGCLIST	"ShowTypingClist"
#define SRMSGDEFSET_SHOWTYPINGCLIST	1
#define SRMSGSET_SHOWTYPINGSWITCH	"ShowTypingSwitch"
#define SRMSGDEFSET_SHOWTYPINGSWITCH 1

#define SRMSGSET_AVATARENABLE		"AvatarEnable"
#define SRMSGDEFSET_AVATARENABLE	1

#define SRMSGSET_USETRANSPARENCY	"UseTransparency"
#define SRMSGDEFSET_USETRANSPARENCY	0
#define SRMSGSET_ACTIVEALPHA		"ActiveAlpha"
#define SRMSGDEFSET_ACTIVEALPHA		0
#define SRMSGSET_INACTIVEALPHA		"InactiveAlpha"
#define SRMSGDEFSET_INACTIVEALPHA	0
#define SRMSGSET_WINDOWTITLE		"WindowTitle"
#define SRMSGSET_SAVEDRAFTS			"SaveDrafts"
#define SRMSGDEFSET_SAVEDRAFTS		0
#define SRMSGSET_BUTTONVISIBILITY	"ButtonVisibility"
#define SRMSGDEFSET_BUTTONVISIBILITY 0xFFFF
#define SRMSGSET_CHATBUTTONVISIBILITY "ChatButtonVisibility"
#define SRMSGDEFSET_CHATBUTTONVISIBILITY 0xFFFF

#define SRMSGSET_HIDECONTAINERS		"HideContainers"
#define SRMSGDEFSET_HIDECONTAINERS	0

#define SRMSGSET_AUTORESIZELINES	"AutoResizeLines"
#define SRMSGDEFSET_AUTORESIZELINES	2
#endif
