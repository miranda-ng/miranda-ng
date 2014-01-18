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

#include <m_chat.h>

#define OPTIONS_FONTCOUNT 20
#define STATUSICONCOUNT 6

#define GC_FAKE_EVENT HANDLE(0xBABABEDA)

#define GC_UPDATETITLE         (WM_USER+100)
#define GC_SPLITTERMOVED       (WM_USER+101)
#define GC_CLOSEWINDOW         (WM_USER+103)
#define GC_GETITEMDATA         (WM_USER+104)
#define GC_SETITEMDATA         (WM_USER+105)
#define GC_UPDATESTATUSBAR     (WM_USER+106)
#define GC_SETVISIBILITY       (WM_USER+107)
#define GC_SETWNDPROPS         (WM_USER+108)
#define GC_REDRAWLOG           (WM_USER+109)
#define GC_FIREHOOK            (WM_USER+110)
#define GC_FILTERFIX           (WM_USER+111)
#define GC_CHANGEFILTERFLAG    (WM_USER+112)
#define GC_SHOWFILTERMENU      (WM_USER+113)
#define GC_SETWINDOWPOS        (WM_USER+114)
#define GC_SAVEWNDPOS          (WM_USER+115)
#define GC_REDRAWWINDOW        (WM_USER+118)
#define GC_SHOWCOLORCHOOSER    (WM_USER+119)
#define GC_ADDLOG              (WM_USER+120)
#define GC_ACKMESSAGE          (WM_USER+121)
#define GC_UPDATENICKLIST      (WM_USER+125)
#define GC_TABCHANGE           (WM_USER+127)
#define GC_ADDTAB              (WM_USER+128)
#define GC_SCROLLTOBOTTOM      (WM_USER+129)
#define GC_REMOVETAB           (WM_USER+130)
#define GC_SESSIONNAMECHANGE   (WM_USER+131)
#define GC_FIXTABICONS         (WM_USER+132)
#define GC_DROPPEDTAB          (WM_USER+133)
#define GC_TABCLICKED          (WM_USER+134)
#define GC_SWITCHNEXTTAB       (WM_USER+135)
#define GC_SWITCHPREVTAB       (WM_USER+136)
#define GC_SWITCHTAB           (WM_USER+137)
#define GC_SETTABHIGHLIGHT     (WM_USER+138)
#define GC_SETMESSAGEHIGHLIGHT (WM_USER+139)
#define GC_REDRAWLOG2          (WM_USER+140)
#define GC_REDRAWLOG3          (WM_USER+141)

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

struct GCModuleInfoBase
{
	char*     pszModule;
	TCHAR*    ptszModDispName;
	char*     pszHeader;
	bool      bBold;
	bool      bUnderline;
	bool      bItalics;
	bool      bColor;
	bool      bBkgColor;
	bool      bChanMgr;
	bool      bAckMsg;
	bool      bSingleFormat;
	bool      bFontSize;
	int       nColorCount;
	COLORREF* crColors;
	HICON     hOnlineIcon;
	HICON     hOfflineIcon;
	HICON     hOnlineTalkIcon;
	HICON     hOfflineTalkIcon;
	int       iMaxText;
	MODULEINFO* next;
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
   TCHAR*  ptszText;
   TCHAR*  ptszNick;
   TCHAR*  ptszUID;
   TCHAR*  ptszStatus;
   TCHAR*  ptszUserInfo;
   BOOL    bIsMe;
   BOOL    bIsHighlighted;
   time_t  time;
   int     iType;
	DWORD   dwFlags;
   LOGINFO *next, *prev;
};

struct STATUSINFO
{
   TCHAR*  pszGroup;
   HICON   hIcon;
   WORD    Status;
   STATUSINFO *next;
};

struct USERINFO
{
   TCHAR* pszNick;
   TCHAR* pszUID;
   WORD   Status;
   int    iStatusEx;
   WORD   ContactStatus;
   USERINFO *next;
};

struct GCSessionInfoBase
{
   HWND        hWnd;

   BOOL        bFGSet;
   BOOL        bBGSet;
   BOOL        bFilterEnabled;
   BOOL        bNicklistEnabled;
   BOOL        bInitDone;
	BOOL        bTrimmed;

   char*       pszModule;
   TCHAR*      ptszID;
   TCHAR*      ptszName;
   TCHAR*      ptszStatusbarText;
   TCHAR*      ptszTopic;

   int         iType;
   int         iFG;
   int         iBG;
   int         iSplitterY;
   int         iSplitterX;
   int         iLogFilterFlags;
   int         nUsersInNicklist;
   int         iEventCount;
   int         iX;
   int         iY;
   int         iWidth;
   int         iHeight;
   int         iStatusCount;

   WORD        wStatus;
   WORD        wState;
   WORD        wCommandsNum;
   DWORD       dwItemData;
   DWORD       dwFlags;
   HANDLE      hContact;
   HWND        hwndStatus;
   time_t      LastTime;

   COMMANDINFO* lpCommands;
   COMMANDINFO* lpCurrentCommand;
   LOGINFO*    pLog;
   LOGINFO*    pLogEnd;
   USERINFO*   pUsers;
   USERINFO*   pMe;
   STATUSINFO* pStatuses;

   HWND        hwndTooltip;
   int         iOldItemID;

	TCHAR       pszLogFileName[MAX_PATH];
	SESSION_INFO *next;
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
	BOOL     ShowTime;
	BOOL     ShowTimeIfChanged;
	BOOL     LoggingEnabled;
	BOOL     FlashWindow;
	BOOL     HighlightEnabled;
	BOOL     LogIndentEnabled;
	BOOL     StripFormat;
	BOOL     SoundsFocus;
	BOOL     PopupInactiveOnly;
	BOOL     TrayIconInactiveOnly;
	BOOL     AddColonToAutoComplete;
	BOOL     LogLimitNames;
	BOOL     TimeStampEventColour;
	DWORD    dwIconFlags;
	DWORD    dwTrayIconFlags;
	DWORD    dwPopupFlags;
	int      LogTextIndent;
	int      LoggingLimit;
	int      iEventLimit;
	int      iPopupStyle;
	int      iPopupTimeout;
	int      iSplitterX;
	int      iSplitterY;
	int      iX;
	int      iY;
	int      iWidth;
	int      iHeight;
	TCHAR*   pszTimeStamp;
	TCHAR*   pszTimeStampLog;
	TCHAR*   pszIncomingNick;
	TCHAR*   pszOutgoingNick;
	TCHAR*   pszHighlightWords;
	TCHAR*   pszLogDir;
	HFONT    UserListFont;
	HFONT    UserListHeadingsFont;
	HFONT    MessageBoxFont;
	HFONT    NameFont;
	COLORREF crLogBackground;
	COLORREF crUserListColor;
	COLORREF crUserListBGColor;
	COLORREF crUserListSelectedBGColor;
	COLORREF crUserListHeadingsColor;
	COLORREF crPUTextColour;
	COLORREF crPUBkgColour;
	BOOL		ShowContactStatus;
	BOOL		ContactStatusFirst;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CHAT_MANAGER_INITDATA
{
	GlobalLogSettingsBase *pSettings;
	int cbModuleInfo, cbSession;
	TCHAR *szFontGroup;
};

struct CHAT_MANAGER
{
	void          (*SetActiveSession)(const TCHAR *pszID, const char *pszModule);
	void          (*SetActiveSessionEx)(SESSION_INFO *si);
	SESSION_INFO* (*GetActiveSession)(void);
	SESSION_INFO* (*SM_AddSession)(const TCHAR *pszID, const char *pszModule);
	int           (*SM_RemoveSession)(const TCHAR *pszID, const char *pszModule, BOOL removeContact);
	SESSION_INFO* (*SM_FindSession)(const TCHAR *pszID, const char *pszModule);
	USERINFO*     (*SM_AddUser)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszNick, WORD wStatus);
	BOOL          (*SM_ChangeUID)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR* pszNewUID);
	BOOL          (*SM_ChangeNick)(const TCHAR *pszID, const char *pszModule, GCEVENT *gce);
	BOOL          (*SM_RemoveUser)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID);
	BOOL          (*SM_SetOffline)(const TCHAR *pszID, const char *pszModule);
	BOOL          (*SM_SetTabbedWindowHwnd)(SESSION_INFO *si, HWND hwnd);
	HICON         (*SM_GetStatusIcon)(SESSION_INFO *si, USERINFO * ui);
	BOOL          (*SM_SetStatus)(const TCHAR *pszID, const char *pszModule, int wStatus);
	BOOL          (*SM_SetStatusEx)(const TCHAR *pszID, const char *pszModule, const TCHAR* pszText, int flags);
	BOOL          (*SM_SendUserMessage)(const TCHAR *pszID, const char *pszModule, const TCHAR* pszText);
	STATUSINFO*   (*SM_AddStatus)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszStatus);
	BOOL          (*SM_AddEventToAllMatchingUID)(GCEVENT *gce);
	BOOL          (*SM_AddEvent)(const TCHAR *pszID, const char *pszModule, GCEVENT *gce, BOOL bIsHighlighted);
	LRESULT       (*SM_SendMessage)(const TCHAR *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL          (*SM_PostMessage)(const TCHAR *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL          (*SM_BroadcastMessage)(const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync);
	BOOL          (*SM_RemoveAll)(void);
	BOOL          (*SM_GiveStatus)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszStatus);
	BOOL          (*SM_SetContactStatus)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, WORD pszStatus);
	BOOL          (*SM_TakeStatus)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszStatus);
	BOOL          (*SM_MoveUser)(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID);
	void          (*SM_AddCommand)(const TCHAR *pszID, const char *pszModule, const char* lpNewCommand);
	char*         (*SM_GetPrevCommand)(const TCHAR *pszID, const char *pszModule);
	char*         (*SM_GetNextCommand)(const TCHAR *pszID, const char *pszModule);
	int           (*SM_GetCount)(const char *pszModule);
	SESSION_INFO* (*SM_FindSessionByIndex)(const char *pszModule, int iItem);
	char*         (*SM_GetUsers)(SESSION_INFO *si);
	USERINFO*     (*SM_GetUserFromIndex)(const TCHAR *pszID, const char *pszModule, int index);
	void          (*SM_InvalidateLogDirectories)(void);

	MODULEINFO*   (*MM_AddModule)(const char *pszModule);
	MODULEINFO*   (*MM_FindModule)(const char *pszModule);
	void          (*MM_FixColors)();
	void          (*MM_FontsChanged)(void);
	void          (*MM_IconsChanged)(void);
	BOOL          (*MM_RemoveAll)(void);

	STATUSINFO*   (*TM_AddStatus)(STATUSINFO** ppStatusList, const TCHAR *pszStatus, int* iCount);
	STATUSINFO*   (*TM_FindStatus)(STATUSINFO* pStatusList, const TCHAR *pszStatus);
	WORD          (*TM_StringToWord)(STATUSINFO* pStatusList, const TCHAR *pszStatus);
	TCHAR*        (*TM_WordToString)(STATUSINFO* pStatusList, WORD Status);
	BOOL          (*TM_RemoveAll)(STATUSINFO** pStatusList);

	BOOL          (*UM_SetStatusEx)(USERINFO* pUserList, const TCHAR* pszText, int onlyMe);
	USERINFO*     (*UM_AddUser)(STATUSINFO* pStatusList, USERINFO** pUserList, const TCHAR *pszUID, const TCHAR *pszNick, WORD wStatus);
	USERINFO*     (*UM_SortUser)(USERINFO** ppUserList, const TCHAR *pszUID);
	USERINFO*     (*UM_FindUser)(USERINFO* pUserList, const TCHAR *pszUID);
	USERINFO*     (*UM_FindUserFromIndex)(USERINFO* pUserList, int index);
	USERINFO*     (*UM_GiveStatus)(USERINFO* pUserList, const TCHAR *pszUID, WORD status);
	USERINFO*     (*UM_SetContactStatus)(USERINFO* pUserList, const TCHAR *pszUID, WORD status);
	USERINFO*     (*UM_TakeStatus)(USERINFO* pUserList, const TCHAR *pszUID, WORD status);
	TCHAR*        (*UM_FindUserAutoComplete)(USERINFO* pUserList, const TCHAR* pszOriginal, const TCHAR* pszCurrent);
	BOOL          (*UM_RemoveUser)(USERINFO** pUserList, const TCHAR *pszUID);
	BOOL          (*UM_RemoveAll)(USERINFO** ppUserList);

	LOGINFO*      (*LM_AddEvent)(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd);
	BOOL          (*LM_TrimLog)(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd, int iCount);
	BOOL          (*LM_RemoveAll)(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd);

	HANDLE        (*AddRoom)(const char *pszModule, const TCHAR *pszRoom, const TCHAR *pszDisplayName, int iType);
	BOOL          (*SetOffline)(HANDLE hContact, BOOL bHide);
	BOOL          (*SetAllOffline)(BOOL bHide, const char *pszModule);
	BOOL          (*AddEvent)(HANDLE hContact, HICON hIcon, HANDLE hEvent, int type, TCHAR* fmt, ...);
	HANDLE        (*FindRoom)(const char *pszModule, const TCHAR *pszRoom);

	char*         (*Log_CreateRTF)(LOGSTREAMDATA *streamData);
	char*         (*Log_CreateRtfHeader)(MODULEINFO *mi);
	void          (*LoadMsgDlgFont)(int i, LOGFONT *lf, COLORREF *color);
	TCHAR*        (*MakeTimeStamp)(TCHAR *pszStamp, time_t time);

	BOOL          (*DoEventHook)(const TCHAR *pszID, const char *pszModule, int iType, const TCHAR *pszUID, const TCHAR* pszText, INT_PTR dwItem);
	BOOL          (*DoEventHookAsync)(HWND hwnd, const TCHAR *pszID, const char *pszModule, int iType, TCHAR* pszUID, TCHAR* pszText, INT_PTR dwItem);

	BOOL          (*DoSoundsFlashPopupTrayStuff)(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
	BOOL          (*DoTrayIcon)(SESSION_INFO *si, GCEVENT *gce);
	BOOL          (*DoPopup)(SESSION_INFO *si, GCEVENT *gce);
	int           (*ShowPopup)(HANDLE hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, TCHAR* pszRoomName, COLORREF crBkg, const TCHAR* fmt, ...);
	BOOL          (*LogToFile)(SESSION_INFO *si, GCEVENT *gce);
	TCHAR*        (*GetChatLogsFilename)(SESSION_INFO *si, time_t tTime);

	BOOL          (*IsHighlighted)(SESSION_INFO *si, GCEVENT *pszText);
	TCHAR*        (*RemoveFormatting)(const TCHAR *pszText);
	void          (*ReloadSettings)(void);

	int logPixelSY, logPixelSX;
	char *szActiveWndModule;
	TCHAR *szActiveWndID;
	HICON  hIcons[30];
	HBRUSH hListBkgBrush, hListSelectedBkgBrush;
	HANDLE hBuildMenuEvent, hSendEvent;
	FONTINFO aFonts[OPTIONS_FONTCOUNT];
	SESSION_INFO *wndList;

	// user-defined custom callbacks
	void (*OnCreateModule)(MODULEINFO*);

	void (*OnCreateSession)(SESSION_INFO*, MODULEINFO*);
	void (*OnSessionDblClick)(SESSION_INFO*);
	void (*OnSessionOffline)(SESSION_INFO*);
	void (*OnSessionRemove)(SESSION_INFO*);
	void (*OnSessionRename)(SESSION_INFO*);
	void (*OnSessionReplace)(SESSION_INFO*);

	void (*ShowRoom)(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground);
	void (*OnAddLog)(SESSION_INFO*, int);
	void (*OnClearLog)(SESSION_INFO*);
	void (*OnEventBroadcast)(SESSION_INFO *si, GCEVENT *gce);
	
	void (*OnSetStatusBar)(SESSION_INFO*);
	void (*OnSetTopic)(SESSION_INFO*);

	void (*OnAddUser)(SESSION_INFO*, USERINFO*);
	void (*OnNewUser)(SESSION_INFO*, USERINFO*);
	void (*OnRemoveUser)(SESSION_INFO *si, USERINFO*);
	void (*OnChangeNick)(SESSION_INFO *si);

	void (*OnAddStatus)(SESSION_INFO *si, STATUSINFO*);
	void (*OnSetStatus)(SESSION_INFO *si, int);

	void (*OnLoadSettings)(void);
	void (*OnFlashWindow)(SESSION_INFO *si, int);
};

extern CHAT_MANAGER ci, *pci;

__forceinline void mir_getCI(CHAT_MANAGER_INITDATA *pData)
{
	pci = (CHAT_MANAGER*)CallService("GChat/GetInterface", 0, (LPARAM)pData);
}

#endif // M_CHAT_INT_H__
