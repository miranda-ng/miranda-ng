/*

Chat module interface for Miranda NG

Copyright (c) 2014 George Hazan

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

#pragma once

#ifndef M_CHAT_INT_H__
#define M_CHAT_INT_H__ 1

#pragma warning(disable:4512)

#include <time.h>

#ifndef M_CORE_H__
#include <m_core.h>
#endif

#include <m_string.h>
#include <m_chat.h>
#include <m_gui.h>
#include <m_utils.h>

#define OPTIONS_FONTCOUNT 20
#define STATUSICONCOUNT 6

#define CHAT_MODULE "Chat"
#define CHATFONT_MODULE "ChatFonts"

#define GC_FAKE_EVENT MEVENT(0xBABABEDA)

#define TIMERID_FLASHWND       1

#define GCW_TABROOM            10
#define GCW_TABPRIVMSG         11

#define GC_EVENT_HIGHLIGHT     0x1000
#define STATE_TALK             0x0001

#define ICON_ACTION            0
#define ICON_ADDSTATUS         1
#define ICON_HIGHLIGHT         2
#define ICON_INFO              3
#define ICON_JOIN              4
#define ICON_KICK              5
#define ICON_MESSAGE           6
#define ICON_MESSAGEOUT        7
#define ICON_NICK              8
#define ICON_NOTICE            9
#define ICON_PART             10
#define ICON_QUIT             11
#define ICON_REMSTATUS        12
#define ICON_TOPIC            13

#define ICON_STATUS0          14
#define ICON_STATUS1          15
#define ICON_STATUS2          16
#define ICON_STATUS3          17
#define ICON_STATUS4          18
#define ICON_STATUS5          19

//structs

struct SESSION_INFO;
struct MODULEINFO;
struct LOGSTREAMDATA;

class CChatRoomDlg;

struct GCModuleInfoBase
{
	char     *pszModule;
	wchar_t  *ptszModDispName;
	char     *pszHeader;
	
	bool      bBold, bItalics, bUnderline;
	bool      bColor, bBkgColor;
	bool      bChanMgr, bAckMsg;
	
	int       iMaxText;
};

struct COMMANDINFO
{
	char *lpCommand;
	COMMANDINFO *last, *next;
};

struct FONTINFO
{
	LOGFONT  lf;
	COLORREF color;
};

struct LOGINFO
{
	wchar_t *ptszText;
	wchar_t *ptszNick;
	wchar_t *ptszUID;
	wchar_t *ptszStatus;
	wchar_t *ptszUserInfo;
	BOOL     bIsMe;
	BOOL     bIsHighlighted;
	time_t   time;
	int      iType;
	DWORD    dwFlags;
	LOGINFO *next, *prev;
};

struct STATUSINFO
{
	wchar_t    *pszGroup;
	HICON       hIcon;
	WORD        Status;
	STATUSINFO *next;
};

struct USERINFO
{
	wchar_t* pszNick;
	wchar_t* pszUID;
	WORD     Status;
	int      iStatusEx;
	WORD     ContactStatus;
	USERINFO *next;
};

struct GCSessionInfoBase
{
	MCONTACT    hContact;

	bool        bInitDone;
	bool        bHasToolTip;
	bool        bTrimmed;

	char*       pszModule;
	wchar_t*    ptszID;
	wchar_t*    ptszName;
	wchar_t*    ptszStatusbarText;
	wchar_t*    ptszTopic;

	int         iType;
	int         nUsersInNicklist;
	int         iEventCount;
	int         iStatusCount;

	WORD        wStatus;
	WORD        wState;
	WORD        wCommandsNum;
	void*       pItemData;
	time_t      LastTime;

	int         currentHovered;

	CChatRoomDlg *pDlg;
	COMMANDINFO *lpCommands, *lpCurrentCommand;
	LOGINFO *pLog, *pLogEnd;
	USERINFO *pUsers, *pMe;
	STATUSINFO *pStatuses;

	wchar_t pszLogFileName[MAX_PATH];
};

struct GCLogStreamDataBase
{
	char*         buffer;
	int           bufferOffset, bufferLen;
	HWND          hwnd;
	LOGINFO*      lin;
	BOOL          bStripFormat;
	BOOL          bRedraw;
	SESSION_INFO *si;
};

struct GlobalLogSettingsBase
{
	bool     bShowTime;
	bool     bShowTimeIfChanged;
	bool     bLoggingEnabled;
	bool     bFlashWindow, bFlashWindowHighlight;
	bool     bHighlightEnabled;
	bool     bLogIndentEnabled;
	bool     bStripFormat;
	bool     bSoundsFocus;
	bool     bPopupInactiveOnly;
	bool     bTrayIconInactiveOnly;
	bool     bAddColonToAutoComplete;
	bool     bLogLimitNames;
	bool     bTimeStampEventColour;
	bool		bShowContactStatus;
	bool		bContactStatusFirst;
	bool     bDoubleClick4Privat;    // send a private message on dblclick in a nick list
	DWORD    dwIconFlags;
	DWORD    dwTrayIconFlags;
	DWORD    dwPopupFlags;
	int      LogIconSize;
	int      LogTextIndent;
	int      LoggingLimit;
	int      iEventLimit;
	int      iPopupStyle;
	int      iPopupTimeout;
	int      iWidth;
	int      iHeight;
	wchar_t *pszTimeStamp;
	wchar_t *pszTimeStampLog;
	wchar_t *pszIncomingNick;
	wchar_t *pszOutgoingNick;
	wchar_t *pszHighlightWords;
	wchar_t *pszLogDir;
	HFONT    UserListFont;
	HFONT    UserListHeadingsFont;
	HFONT    NameFont;
	COLORREF crLogBackground;
	COLORREF crUserListColor;
	COLORREF crUserListBGColor;
	COLORREF crUserListSelectedBGColor;
	COLORREF crUserListHeadingsColor;
	COLORREF crPUTextColour;
	COLORREF crPUBkgColour;
};

/////////////////////////////////////////////////////////////////////////////////////////

#define FONTMODE_USE   1
#define FONTMODE_SKIP  2
#define FONTMODE_ALTER 3

struct CHAT_MANAGER_INITDATA
{
	GlobalLogSettingsBase *pSettings;
	int cbModuleInfo, cbSession;
	wchar_t *szFontGroup;
	int iFontMode;
};

typedef BOOL (*pfnDoTrayIcon)(SESSION_INFO *si, GCEVENT *gce);
typedef BOOL (*pfnDoPopup)(SESSION_INFO *si, GCEVENT *gce);

struct CHAT_MANAGER
{
	CHAT_MANAGER();

	void          (*SetActiveSession)(SESSION_INFO *si);
	SESSION_INFO* (*GetActiveSession)(void);
	SESSION_INFO* (*SM_FindSession)(const wchar_t *pszID, const char *pszModule);
	HICON         (*SM_GetStatusIcon)(SESSION_INFO *si, USERINFO * ui);
	BOOL          (*SM_PostMessage)(const wchar_t *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL          (*SM_BroadcastMessage)(const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync);
	void          (*SM_AddCommand)(const wchar_t *pszID, const char *pszModule, const char* lpNewCommand);
	char*         (*SM_GetPrevCommand)(const wchar_t *pszID, const char *pszModule);
	char*         (*SM_GetNextCommand)(const wchar_t *pszID, const char *pszModule);
	int           (*SM_GetCount)(const char *pszModule);
	SESSION_INFO* (*SM_FindSessionByIndex)(const char *pszModule, int iItem);
	USERINFO*     (*SM_GetUserFromIndex)(const wchar_t *pszID, const char *pszModule, int index);
	void          (*SM_InvalidateLogDirectories)(void);

	MODULEINFO*   (*MM_AddModule)(const char *pszModule);
	MODULEINFO*   (*MM_FindModule)(const char *pszModule);
	void          (*MM_FontsChanged)(void);
	void          (*MM_IconsChanged)(void);
	BOOL          (*MM_RemoveAll)(void);

	STATUSINFO*   (*TM_AddStatus)(STATUSINFO **ppStatusList, const wchar_t *pszStatus, int* iCount);
	STATUSINFO*   (*TM_FindStatus)(STATUSINFO *pStatusList, const wchar_t *pszStatus);
	WORD          (*TM_StringToWord)(STATUSINFO *pStatusList, const wchar_t *pszStatus);
	wchar_t*      (*TM_WordToString)(STATUSINFO *pStatusList, WORD Status);
	BOOL          (*TM_RemoveAll)(STATUSINFO** pStatusList);

	BOOL          (*UM_SetStatusEx)(USERINFO *pUserList, const wchar_t* pszText, int onlyMe);
	USERINFO*     (*UM_AddUser)(STATUSINFO *pStatusList, USERINFO **pUserList, const wchar_t *pszUID, const wchar_t *pszNick, WORD wStatus);
	USERINFO*     (*UM_SortUser)(USERINFO **ppUserList, const wchar_t *pszUID);
	USERINFO*     (*UM_FindUser)(USERINFO *pUserList, const wchar_t *pszUID);
	USERINFO*     (*UM_FindUserFromIndex)(USERINFO *pUserList, int index);
	USERINFO*     (*UM_GiveStatus)(USERINFO *pUserList, const wchar_t *pszUID, WORD status);
	USERINFO*     (*UM_SetContactStatus)(USERINFO *pUserList, const wchar_t *pszUID, WORD status);
	USERINFO*     (*UM_TakeStatus)(USERINFO *pUserList, const wchar_t *pszUID, WORD status);
	wchar_t*      (*UM_FindUserAutoComplete)(USERINFO *pUserList, const wchar_t* pszOriginal, const wchar_t* pszCurrent);
	BOOL          (*UM_RemoveUser)(USERINFO **pUserList, const wchar_t *pszUID);
	BOOL          (*UM_RemoveAll)(USERINFO **ppUserList);
	int           (*UM_CompareItem)(USERINFO *u1, const wchar_t *pszNick, WORD wStatus);

	LOGINFO*      (*LM_AddEvent)(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd);
	BOOL          (*LM_TrimLog)(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd, int iCount);
	BOOL          (*LM_RemoveAll)(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd);

	BOOL          (*SetOffline)(MCONTACT hContact, BOOL bHide);
	BOOL          (*SetAllOffline)(BOOL bHide, const char *pszModule);
	BOOL          (*AddEvent)(MCONTACT hContact, HICON hIcon, MEVENT hEvent, int type, wchar_t* fmt, ...);
	MCONTACT      (*FindRoom)(const char *pszModule, const wchar_t *pszRoom);

	char*         (*Log_CreateRTF)(LOGSTREAMDATA *streamData);
	char*         (*Log_CreateRtfHeader)(void);
	void          (*LoadMsgDlgFont)(int i, LOGFONT *lf, COLORREF *color);
	wchar_t*      (*MakeTimeStamp)(wchar_t *pszStamp, time_t time);

	BOOL          (*DoSoundsFlashPopupTrayStuff)(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
	BOOL          (*DoTrayIcon)(SESSION_INFO *si, GCEVENT *gce);
	BOOL          (*DoPopup)(SESSION_INFO *si, GCEVENT *gce);
	int           (*ShowPopup)(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, wchar_t* pszRoomName, COLORREF crBkg, const wchar_t* fmt, ...);
	BOOL          (*LogToFile)(SESSION_INFO *si, GCEVENT *gce);
	wchar_t*      (*GetChatLogsFilename)(SESSION_INFO *si, time_t tTime);
	char*         (*Log_SetStyle)(int style);

	bool          (*IsHighlighted)(SESSION_INFO *si, GCEVENT *pszText);
	wchar_t*      (*RemoveFormatting)(const wchar_t *pszText);
	void          (*ReloadSettings)(void);

	int           (*DoRtfToTags)(CMStringW &pszText, int iNumColors, COLORREF *pColors);

	int logPixelSY, logPixelSX;
	char *szActiveWndModule;
	wchar_t *szActiveWndID;
	HICON  hIcons[30];
	HBRUSH hListBkgBrush, hListSelectedBkgBrush;
	HANDLE hevWinPopup, hevPreCreate;
	FONTINFO aFonts[OPTIONS_FONTCOUNT];
	LIST<SESSION_INFO> &arSessions;
	char **pLogIconBmpBits;

	// user-defined custom callbacks
	void (*OnCreateModule)(MODULEINFO*);
	void (*OnDestroyModule)(MODULEINFO*);

	void (*OnCreateSession)(SESSION_INFO*, MODULEINFO*);
	void (*OnOfflineSession)(SESSION_INFO*);
	void (*OnReplaceSession)(SESSION_INFO*);

	void (*ShowRoom)(SESSION_INFO*);
	void (*OnGetLogName)(SESSION_INFO*, const wchar_t*);
	void (*OnEventBroadcast)(SESSION_INFO *si, GCEVENT *gce);
	
	void (*OnSetTopic)(SESSION_INFO*);

	void (*OnAddUser)(SESSION_INFO*, USERINFO*);
	void (*OnNewUser)(SESSION_INFO*, USERINFO*);
	void (*OnRemoveUser)(SESSION_INFO*, USERINFO*);
	void (*OnChangeNick)(SESSION_INFO*);

	void (*OnAddStatus)(SESSION_INFO*, STATUSINFO*);
	void (*OnSetStatus)(SESSION_INFO*, int);

	void (*OnLoadSettings)(void);
	void (*OnFlashWindow)(SESSION_INFO*, int bInactive);
	void (*OnFlashHighlight)(SESSION_INFO*, int bInactive);
};

#ifndef MIR_APP_EXPORTS
extern CHAT_MANAGER *pci;
#endif

EXTERN_C MIR_APP_DLL(CHAT_MANAGER*) Chat_GetInterface(CHAT_MANAGER_INITDATA *pData = NULL, int = hLangpack);

/////////////////////////////////////////////////////////////////////////////////////////

// receives LOGSTREAMDATA* as the first parameter
EXTERN_C MIR_APP_DLL(DWORD) CALLBACK Srmm_LogStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

// sends a message to all SRMM windows
EXTERN_C MIR_APP_DLL(void) Srmm_Broadcast(UINT, WPARAM, LPARAM);

// finds a SRMM window using hContact
EXTERN_C MIR_APP_DLL(HWND) Srmm_FindWindow(MCONTACT hContact);

// updates options for all windows
EXTERN_C MIR_APP_DLL(void) Chat_UpdateOptions();

// runs ME_GC_EVENT with the parameters passed
EXTERN_C MIR_APP_DLL(BOOL) Chat_DoEventHook(SESSION_INFO *si, int iType, const USERINFO *pUser, const wchar_t* pszText, INT_PTR dwItem);

// chat menu creation / destruction
EXTERN_C MIR_APP_DLL(UINT) Chat_CreateGCMenu(HWND hwnd, HMENU hMenu, POINT pt, SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszWordText);
EXTERN_C MIR_APP_DLL(void) Chat_DestroyGCMenu(HMENU hMenu, int iIndex);

// calculates width or height of a string
EXTERN_C MIR_APP_DLL(int) Chat_GetTextPixelSize(const wchar_t *pszText, HFONT hFont, bool bWidth);

// message procedures' stubs
EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubLogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubMessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubNicklistProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////

#include <chat_resource.h>

class MIR_APP_EXPORT CSrmmBaseDialog : public CDlgBase
{
	CSrmmBaseDialog(const CSrmmBaseDialog&);
	CSrmmBaseDialog& operator=(const CSrmmBaseDialog&);

protected:
	CSrmmBaseDialog(HINSTANCE hInst, int idDialog, SESSION_INFO *si = nullptr);

	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	int  NotifyEvent(int code);
	bool ProcessHotkeys(int key, bool bShift, bool bCtrl, bool bAlt);
	void RefreshButtonStatus(void);
	void RunUserMenu(HWND hwndOwner, USERINFO *ui, const POINT &pt);

protected:
	CCtrlRichEdit m_message, m_log;
	SESSION_INFO *m_si;
	COLORREF m_clrInputBG, m_clrInputFG;
	time_t m_iLastEnterTime;

	CCtrlListBox m_nickList;
	CCtrlButton m_btnColor, m_btnBkColor;
	CCtrlButton m_btnBold, m_btnItalic, m_btnUnderline;
	CCtrlButton m_btnHistory, m_btnChannelMgr, m_btnNickList, m_btnFilter;

	void onClick_BIU(CCtrlButton*);
	void onClick_Color(CCtrlButton*);
	void onClick_BkColor(CCtrlButton*);

	void onClick_ChanMgr(CCtrlButton*);
	void onClick_History(CCtrlButton*);

	void onDblClick_List(CCtrlListBox*);

public:
	MCONTACT m_hContact;
	int m_iLogFilterFlags;
	bool m_bFilterEnabled, m_bNicklistEnabled;
	bool m_bFGSet, m_bBGSet;
	COLORREF m_iFG, m_iBG;

	void ClearLog();
	void RedrawLog2();
	void ShowColorChooser(int iCtrlId);

	virtual void AddLog();
	virtual void CloseTab() {}
	virtual void LoadSettings() PURE;
	virtual void RedrawLog() {}
	virtual void ScrollToBottom() {}
	virtual void SetStatusText(const wchar_t*, HICON) {}
	virtual void ShowFilterMenu() {}
	virtual void StreamInEvents(LOGINFO*, bool) {}
	virtual void UpdateNickList() {}
	virtual void UpdateOptions() {}
	virtual void UpdateStatusBar() {}
	virtual void UpdateTitle() PURE;

	virtual LRESULT WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam);

	__forceinline bool isChat() const { return m_si != nullptr; }

	__inline void* operator new(size_t size){ return calloc(1, size); }
	__inline void operator delete(void* p) { free(p); }
};

#endif // M_CHAT_INT_H__
