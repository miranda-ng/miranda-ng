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
	TCHAR *name;
	UINT controlId;
	int alignment;
	int spacing;
	int width;
};

struct ErrorWindowData
{
	TCHAR *szName;
	TCHAR *szDescription;
	TCHAR *szText;
	MessageSendQueueItem *queueItem;
	HWND hwndParent;
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
	int windowWasCascaded;
	TabCtrlData *tabCtrlDat;
	BOOL isChat;
	ParentWindowData *prev, *next;
};

struct MessageWindowTabData
{
	HWND hwnd;
	MCONTACT hContact;
	char *szProto;
	ParentWindowData *parent;
	HICON hIcon;
};

#define NMWLP_INCOMING 1

struct NewMessageWindowLParam
{
	MCONTACT hContact;
	BOOL isChat;
	int isWchar;
	LPCSTR szInitialText;
	int flags;
};

#define CWDF_RTF_INPUT 1

struct CommonWindowData
{
	MCONTACT hContact;
	int codePage;
	DWORD flags;
	HWND hwndLog;
	int minLogBoxHeight, minEditBoxHeight;
	TCmdList *cmdList, *cmdListCurrent;
};

struct SrmmWindowData
{
	HWND hwnd;
	int tabId;
	ParentWindowData *parent;
	HWND hwndParent;
	HANDLE hDbEventFirst, hDbEventLast, hDbUnreadEventFirst;
	int splitterPos;
	int desiredInputAreaHeight;
	SIZE toolbarSize;
	int windowWasCascaded;
	int nTypeSecs;
	int nTypeMode;
	HBITMAP avatarPic;
	DWORD nLastTyping;
	int showTyping;
	int showUnread;
	DWORD lastMessage;
	char *szProto;
	WORD wStatus;
	time_t startTime;
	time_t lastEventTime;
	int lastEventType;
	DWORD flags;
	int messagesInProgress;
	struct avatarCacheEntry *ace;
	int isMixed;
	int sendAllConfirm;
	HICON statusIcon;
	HICON statusIconBig;
	HICON statusIconOverlay;
	CommonWindowData windowData;
	InfobarWindowData *infobarData;
};


#define HM_DBEVENTADDED			(WM_USER+10)
#define DM_REMAKELOG			(WM_USER+11)
#define DM_CASCADENEWWINDOW		(WM_USER+13)
#define DM_OPTIONSAPPLIED		(WM_USER+14)
#define DM_SPLITTERMOVED		(WM_USER+15)
#define DM_APPENDTOLOG			(WM_USER+17)
#define DM_ERRORDECIDED			(WM_USER+18)
#define DM_SCROLLLOGTOBOTTOM	(WM_USER+19)
#define DM_TYPING				(WM_USER+20)
#define DM_UPDATELASTMESSAGE	(WM_USER+22)
#define DM_USERNAMETOCLIP		(WM_USER+23)
#define DM_CHANGEICONS			(WM_USER+24)
#define DM_UPDATEICON			(WM_USER+25)
#define DM_GETAVATAR			(WM_USER+27)
#define HM_ACKEVENT				(WM_USER+29)

#define DM_SENDMESSAGE			(WM_USER+30)
#define DM_STARTMESSAGESENDING	(WM_USER+31)
#define DM_SHOWMESSAGESENDING	(WM_USER+32)
#define DM_STOPMESSAGESENDING	(WM_USER+33)
#define DM_SHOWERRORMESSAGE		(WM_USER+34)

#define DM_CLEARLOG				(WM_USER+46)
#define DM_SWITCHSTATUSBAR		(WM_USER+47)
#define DM_SWITCHTOOLBAR		(WM_USER+48)
#define DM_SWITCHTITLEBAR		(WM_USER+49)
#define DM_SWITCHINFOBAR		(WM_USER+50)
#define DM_SWITCHRTL			(WM_USER+51)
#define DM_SWITCHTYPING			(WM_USER+53)
#define DM_MESSAGESENDING		(WM_USER+54)
#define DM_GETWINDOWSTATE		(WM_USER+55)
#define DM_STATUSICONCHANGE		(WM_USER+56)

#define DM_MYAVATARCHANGED		(WM_USER+62)
#define DM_PROTOAVATARCHANGED	(WM_USER+63)
#define DM_AVATARCHANGED		(WM_USER+64)

#define EM_SUBCLASSED			(WM_USER+0x101)
#define EM_UNSUBCLASSED			(WM_USER+0x102)

#define EVENTTYPE_JABBER_CHATSTATES	2000
#define EVENTTYPE_JABBER_PRESENCE	2001

struct CREOleCallback : public IRichEditOleCallback
{
	CREOleCallback() : refCount(0) {}
	unsigned refCount;
	IStorage *pictStg;
	int nextStgId;

	STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR *lplpObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);
	
	STDMETHOD(ContextSensitiveHelp) (BOOL fEnterMode);
	STDMETHOD(GetNewStorage) (LPSTORAGE FAR *lplpstg);
	STDMETHOD(GetInPlaceContext) (LPOLEINPLACEFRAME FAR *lplpFrame, LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(ShowContainerUI) (BOOL fShow);
	STDMETHOD(QueryInsertObject) (LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp);
	STDMETHOD(DeleteObject) (LPOLEOBJECT lpoleobj);
	STDMETHOD(QueryAcceptData) (LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
	STDMETHOD(GetClipboardData) (CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj);
	STDMETHOD(GetDragDropEffect) (BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect);
	STDMETHOD(GetContextMenu) (WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg, HMENU FAR *lphmenu);
};

struct CREOleCallback2 : public CREOleCallback
{
	STDMETHOD(QueryAcceptData) (LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
};

INT_PTR CALLBACK DlgProcParentWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int DbEventIsShown(DBEVENTINFO *dbei, SrmmWindowData *dat);
int DbEventIsCustomForMsgWindow(DBEVENTINFO *dbei);
int DbEventIsMessageOrCustom(DBEVENTINFO *dbei);
int safe_wcslen(wchar_t *msg, int maxLen);
void StreamInEvents(HWND hwndDlg, HANDLE hDbEventFirst, int count, int fAppend);
void LoadMsgLogIcons(void);
void FreeMsgLogIcons(void);
TCHAR *GetNickname(MCONTACT hContact, const char *szProto);
int IsAutoPopup(MCONTACT hContact);

#define MSGFONTID_MYMSG			0
#define MSGFONTID_YOURMSG		1
#define MSGFONTID_MYNAME		2
#define MSGFONTID_MYTIME		3
#define MSGFONTID_MYCOLON		4
#define MSGFONTID_YOURNAME		5
#define MSGFONTID_YOURTIME		6
#define MSGFONTID_YOURCOLON		7
#define MSGFONTID_MESSAGEAREA	8
#define MSGFONTID_NOTICE		9
#define MSGFONTID_MYURL			10
#define MSGFONTID_YOURURL		11
#define MSGFONTID_INFOBAR_NAME	12
#define MSGFONTID_INFOBAR_STATUS 13

void LoadMsgDlgFont(int i, LOGFONT *lf, COLORREF *colour);
extern int fontOptionsListSize;

#define LOADHISTORY_UNREAD		0
#define LOADHISTORY_COUNT		1
#define LOADHISTORY_TIME		2

#define SRMMMOD						"SRMM"

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
#define SRMSGSET_LIMITNAMESLEN_MIN	0
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
#define SRMSGSET_MSGTIMEOUT_MIN		4000 // minimum value (4 seconds)
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
