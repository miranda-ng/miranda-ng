/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#ifndef _CHAT_H_
#define _CHAT_H_

#include <m_protomod.h>
#include <m_chat.h>

#ifndef TVM_GETITEMSTATE
#define TVM_GETITEMSTATE        (TV_FIRST + 39)
#endif

#ifndef TreeView_GetItemState
#define TreeView_GetItemState(hwndTV, hti, mask) \
   (UINT)SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), (LPARAM)(mask))
#endif

#ifndef CFM_BACKCOLOR
#define CFM_BACKCOLOR		0x04000000
#endif

//defines
#define OPTIONS_FONTCOUNT 17
#define GC_SPLITTERMOVED		(WM_USER+101)
#define GC_CLOSEWINDOW			(WM_USER+103)
#define GC_GETITEMDATA			(WM_USER+104)
#define GC_SETITEMDATA			(WM_USER+105)
#define GC_SETVISIBILITY		(WM_USER+107)
#define GC_SETWNDPROPS			(WM_USER+108)
#define GC_REDRAWLOG			(WM_USER+109)
#define GC_FIREHOOK				(WM_USER+110)
#define GC_FILTERFIX			(WM_USER+111)
#define GC_CHANGEFILTERFLAG		(WM_USER+112)
#define GC_SHOWFILTERMENU		(WM_USER+113)
#define GC_SETWINDOWPOS			(WM_USER+114)
//#define	GC_NICKLISTCLEAR		(WM_USER+117)
#define GC_REDRAWWINDOW			(WM_USER+118)
#define GC_SHOWCOLORCHOOSER		(WM_USER+119)
#define GC_ADDLOG				(WM_USER+120)
#define GC_ACKMESSAGE			(WM_USER+121)
//#define GC_ADDUSER				(WM_USER+122)
//#define GC_REMOVEUSER			(WM_USER+123)
//#define GC_NICKCHANGE			(WM_USER+124)
#define GC_UPDATENICKLIST		(WM_USER+125)
//#define GC_MODECHANGE			(WM_USER+126)
#define GC_TABCHANGE			(WM_USER+127)
#define GC_SCROLLTOBOTTOM		(WM_USER+129)
#define GC_FIXTABICONS			(WM_USER+132)
#define GC_SETTABHIGHLIGHT		(WM_USER+138)
#define GC_SETMESSAGEHIGHLIGHT	(WM_USER+139)
#define GC_REDRAWLOG2			(WM_USER+140)
#define GC_REDRAWLOG3			(WM_USER+141)

//#define EM_SUBCLASSED			(WM_USER+200)
//#define EM_UNSUBCLASSED			(WM_USER+201)
#define EM_ACTIVATE				(WM_USER+202)

#define GCW_TABROOM				10
#define GCW_TABPRIVMSG			11

#define GC_EVENT_HIGHLIGHT		0x1000
#define STATE_TALK				0x0001


// special service for tweaking performance
#define MS_GC_GETEVENTPTR  "GChat/GetNewEventPtr"
typedef INT_PTR (*GETEVENTFUNC)(WPARAM wParam, LPARAM lParam);
typedef struct  {
	GETEVENTFUNC pfnAddEvent;
}GCPTRS;

//structs

typedef struct  MODULE_INFO_TYPE
{
	char*		pszModule;
	TCHAR*		ptszModDispName;
	BOOL		bBold;
	BOOL		bUnderline;
	BOOL		bItalics;
	BOOL		bColor;
	BOOL		bBkgColor;
	BOOL		bFontSize;
	BOOL		bChanMgr;
	BOOL		bAckMsg;
	BOOL		bSingleFormat;
	int			nColorCount;
	COLORREF*	crColors;
	HICON		hOnlineIcon;
	HICON		hOfflineIcon;
	HICON		hOnlineIconBig;
	HICON		hOfflineIconBig;
	HICON		hOnlineTalkIcon;
	HICON		hOfflineTalkIcon;
	int			iMaxText;
	struct MODULE_INFO_TYPE *next;
}
	MODULEINFO;

typedef struct COMMAND_INFO_TYPE
{
	char*  lpCommand;
	struct COMMAND_INFO_TYPE *prev, *next;
}
	COMMAND_INFO;

typedef struct
{
	LOGFONT  lf;
	COLORREF color;
}
	FONTINFO;

typedef struct LOG_INFO_TYPE
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
	struct  LOG_INFO_TYPE *next;
	struct  LOG_INFO_TYPE *prev;
}
	LOGINFO;

typedef struct STATUSINFO_TYPE
{
	TCHAR*  pszGroup;
	HICON   hIcon;
	WORD    Status;
	struct  STATUSINFO_TYPE *next;
}
	STATUSINFO;

typedef struct  USERINFO_TYPE
{
	TCHAR* pszNick;
	TCHAR* pszUID;
	WORD   Status;
	int    iStatusEx;
	WORD   ContactStatus;
	struct USERINFO_TYPE *next;
}
	USERINFO;

typedef struct  TABLIST_TYPE
{
	TCHAR* pszID;
	char*  pszModule;
	struct TABLIST_TYPE *next;
}
	TABLIST;

typedef struct SESSION_INFO_TYPE
{
	HWND        hWnd;

	BOOL        bFGSet;
	BOOL        bBGSet;
	BOOL        bFilterEnabled;
	BOOL        bNicklistEnabled;
	BOOL        bInitDone;

	char*       pszModule;
	char*		pszHeader;
	TCHAR*      ptszID;
	TCHAR*      ptszName;
	TCHAR*      ptszStatusbarText;
	TCHAR*      ptszTopic;

	#if defined( _UNICODE )
		char*    pszID;		// ugly fix for returning static ANSI strings in GC_INFO
		char*    pszName;   // just to fix a bug quickly, should die after porting IRC to Unicode
	#endif

	int         iType;
	int         iFG;
	int         iBG;
	int         iSplitterY;
	int			desiredInputAreaHeight;
	int         iSplitterX;
	int         iLogFilterFlags;
	int         nUsersInNicklist;
	int         iEventCount;
	int         iStatusCount;

	WORD        wStatus;
	WORD        wState;
	WORD        wCommandsNum;
	DWORD       dwItemData;
	DWORD       dwFlags;
	time_t      LastTime;
	CommonWindowData windowData;
	LOGINFO*       pLog;
	LOGINFO*       pLogEnd;
	USERINFO*      pUsers;
	USERINFO*      pMe;
	STATUSINFO*    pStatuses;
	TCHAR          szSearch[255];

	struct SESSION_INFO_TYPE *next;

}SESSION_INFO;

typedef struct
{
	char*         buffer;
	int           bufferOffset, bufferLen;
	HWND          hwnd;
	LOGINFO*      lin;
	BOOL          bStripFormat;
	BOOL          bRedraw;
	BOOL		  isFirst;
	SESSION_INFO* si;
}
	LOGSTREAMDATA;

struct GlobalLogSettings_t {
	BOOL        ShowTime;
	BOOL        ShowTimeIfChanged;
	BOOL        LoggingEnabled;
	BOOL        FlashWindow;
	BOOL        HighlightEnabled;
	BOOL        LogIndentEnabled;
	BOOL        StripFormat;
	BOOL        SoundsFocus;
	BOOL        PopUpInactiveOnly;
	BOOL        TrayIconInactiveOnly;
	BOOL        AddColonToAutoComplete;
	BOOL        LogLimitNames;
	BOOL        TimeStampEventColour;
	DWORD       dwIconFlags;
	DWORD       dwTrayIconFlags;
	DWORD       dwPopupFlags;
	int         LogTextIndent;
	int         LoggingLimit;
	int         iEventLimit;
	int         iPopupStyle;
	int         iPopupTimeout;
	int         iSplitterX;
	int         iSplitterY;
	TCHAR*      pszTimeStamp;
	TCHAR*      pszTimeStampLog;
	TCHAR*      pszIncomingNick;
	TCHAR*      pszOutgoingNick;
	TCHAR*      pszHighlightWords;
	TCHAR*      pszLogDir;
	HFONT       UserListFont;
	HFONT       UserListHeadingsFont;
	HFONT       MessageBoxFont;
	HFONT       NameFont;
	COLORREF    crLogBackground;
	COLORREF    crUserListColor;
	COLORREF    crUserListBGColor;
	COLORREF    crUserListSelectedBGColor;
	COLORREF    crUserListHeadingsColor;
	COLORREF    crPUTextColour;
	COLORREF    crPUBkgColour;
	BOOL		ShowContactStatus;
	BOOL		ContactStatusFirst;
};
extern struct GlobalLogSettings_t g_Settings;

typedef struct{
	MODULEINFO*   pModule;
	int           xPosition;
	int           yPosition;
	HWND          hWndTarget;
	BOOL          bForeground;
	SESSION_INFO* si;
}
	COLORCHOOSER;

//main.c
void LoadIcons(void);
void FreeIcons(void);
void UpgradeCheck(void);

//colorchooser.c
INT_PTR CALLBACK DlgProcColorToolWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//log.c
void   Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO* si, BOOL bRedraw);
void   LoadMsgLogBitmaps(void);
void   FreeMsgLogBitmaps(void);
TCHAR* GetChatLogsFilename (HANDLE  hContact, time_t tTime);
TCHAR* MakeTimeStamp(TCHAR* pszStamp, time_t time);
char*  Log_CreateRtfHeader(MODULEINFO * mi, SESSION_INFO* si);

//window.c
INT_PTR CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int GetTextPixelSize( TCHAR* pszText, HFONT hFont, BOOL bWidth);

//options.c
int    OptionsInit(void);
int    OptionsUnInit(void);
void   LoadGlobalSettings(void);
void   LoadLogFonts(void);
void   SetIndentSize();

//services.c
void   HookEvents(void);
void   UnhookEvents(void);
void   CreateServiceFunctions(void);
void   DestroyServiceFunctions(void);
void   DestroyHookableEvents(void);
void   CreateHookableEvents(void);
int    Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam);
int    Chat_FontsChanged(WPARAM wParam,LPARAM lParam);
int    Chat_SmileyOptionsChanged(WPARAM wParam,LPARAM lParam);
int    Chat_PreShutdown(WPARAM wParam,LPARAM lParam);
int    Chat_IconsChanged(WPARAM wParam,LPARAM lParam);
void   ShowRoom(SESSION_INFO* si, WPARAM wp, BOOL bSetForeground);

//manager.c
void          SetActiveSession(const TCHAR* pszID, const char* pszModule);
void          SetActiveSessionEx(SESSION_INFO* si);
SESSION_INFO* GetActiveSession(void);
SESSION_INFO* SM_AddSession(const TCHAR* pszID, const char* pszModule);
int           SM_RemoveSession(const TCHAR* pszID, const char* pszModule);
SESSION_INFO* SM_FindSession(const TCHAR* pszID, const char* pszModule);
HWND          SM_FindWindowByContact(HANDLE hContact);
USERINFO*     SM_AddUser(SESSION_INFO* si, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus);
BOOL          SM_ChangeUID(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszNewUID);
BOOL          SM_ChangeNick(const TCHAR* pszID, const char* pszModule, GCEVENT * gce);
BOOL          SM_RemoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID);
BOOL          SM_SetOffline(const TCHAR* pszID, const char* pszModule);
HICON         SM_GetStatusIcon(SESSION_INFO* si, USERINFO * ui);
BOOL          SM_SetStatus(const TCHAR* pszID, const char* pszModule, int wStatus);
BOOL          SM_SetStatusEx(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText, int flags );
BOOL          SM_SendUserMessage(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText);
STATUSINFO*   SM_AddStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszStatus);
BOOL          SM_AddEventToAllMatchingUID(GCEVENT * gce);
BOOL          SM_AddEvent(const TCHAR* pszID, const char* pszModule, GCEVENT * gce, BOOL bIsHighlighted);
LRESULT       SM_SendMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL          SM_PostMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL          SM_BroadcastMessage(const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync);
BOOL          SM_RemoveAll (void);
BOOL          SM_GiveStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus);
BOOL          SM_SetContactStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, WORD pszStatus);
BOOL          SM_TakeStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus);
BOOL          SM_MoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID);
int           SM_GetCount(const char* pszModule);
SESSION_INFO* SM_FindSessionByIndex(const char* pszModule, int iItem);
char*         SM_GetUsers(SESSION_INFO* si);
USERINFO*     SM_GetUserFromIndex(const TCHAR* pszID, const char* pszModule, int index);
char          SM_GetStatusIndicator(SESSION_INFO* si, USERINFO * ui);
SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const TCHAR* pszOriginal, const TCHAR* pszCurrent);
MODULEINFO*   MM_AddModule(const char* pszModule);
MODULEINFO*   MM_FindModule(const char* pszModule);
void          MM_FixColors();
void          MM_FontsChanged(void);
void          MM_IconsChanged(void);
BOOL          MM_RemoveAll (void);
BOOL          TabM_AddTab(const TCHAR* pszID, const char* pszModule);
BOOL          TabM_RemoveAll (void);
STATUSINFO*   TM_AddStatus(STATUSINFO** ppStatusList, const TCHAR* pszStatus, int* iCount);
STATUSINFO*   TM_FindStatus(STATUSINFO* pStatusList, const TCHAR* pszStatus);
WORD          TM_StringToWord(STATUSINFO* pStatusList, const TCHAR* pszStatus);
TCHAR*        TM_WordToString(STATUSINFO* pStatusList, WORD Status);
BOOL          TM_RemoveAll (STATUSINFO** pStatusList);
BOOL          UM_SetStatusEx(USERINFO* pUserList,const TCHAR* pszText, int onlyMe );
USERINFO*     UM_AddUser(STATUSINFO* pStatusList, USERINFO** pUserList, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus);
USERINFO*     UM_SortUser(USERINFO** ppUserList, const TCHAR* pszUID);
USERINFO*     UM_FindUser(USERINFO* pUserList, const TCHAR* pszUID);
USERINFO*     UM_FindUserFromIndex(USERINFO* pUserList, int index);
USERINFO*     UM_GiveStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status);
USERINFO*     UM_SetContactStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status);
USERINFO*     UM_TakeStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status);
TCHAR*        UM_FindUserAutoComplete(USERINFO* pUserList, const TCHAR* pszOriginal, const TCHAR* pszCurrent);
BOOL          UM_RemoveUser(USERINFO** pUserList, const TCHAR* pszUID);
BOOL          UM_RemoveAll (USERINFO** ppUserList);
LOGINFO*      LM_AddEvent(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd);
BOOL          LM_TrimLog(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd, int iCount);
BOOL          LM_RemoveAll (LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd);

//clist.c
HANDLE        CList_AddRoom(const char* pszModule, const TCHAR* pszRoom, const TCHAR* pszDisplayName, int iType);
BOOL          CList_SetOffline(HANDLE hContact, BOOL bHide);
BOOL          CList_SetAllOffline(BOOL bHide, const char *pszModule);
int           CList_RoomDoubleclicked(WPARAM wParam,LPARAM lParam);
int           CList_EventDoubleclicked(WPARAM wParam,LPARAM lParam);
INT_PTR       CList_EventDoubleclickedSvc(WPARAM wParam,LPARAM lParam);
INT_PTR       CList_JoinChat(WPARAM wParam, LPARAM lParam);
INT_PTR       CList_LeaveChat(WPARAM wParam, LPARAM lParam);
INT_PTR		  CList_PrebuildContactMenuSvc(WPARAM wParam, LPARAM lParam);
int           CList_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
void          CList_CreateGroup(TCHAR* group);
BOOL          CList_AddEvent(HANDLE hContact, HICON Icon, HANDLE event, int type, TCHAR* fmt, ... ) ;
HANDLE        CList_FindRoom (const char* pszModule, const TCHAR* pszRoom) ;
int           WCCmp(TCHAR* wild, TCHAR*string);

//tools.c
TCHAR*        RemoveFormatting(const TCHAR* pszText);
BOOL          DoSoundsFlashPopupTrayStuff(SESSION_INFO* si, GCEVENT * gce, BOOL bHighlight, int bManyFix);
int           GetColorIndex(const char* pszModule, COLORREF cr);
void          CheckColorsInModule(const char* pszModule);
TCHAR*        my_strstri(const TCHAR* s1, const TCHAR* s2) ;
BOOL          IsHighlighted(SESSION_INFO* si, const TCHAR* pszText);
UINT          CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO* si, TCHAR* pszUID, TCHAR* pszWordText);
void          DestroyGCMenu(HMENU *hMenu, int iIndex);
BOOL          DoEventHookAsync(HWND hwnd, const TCHAR* pszID, const char* pszModule, int iType, TCHAR* pszUID, TCHAR* pszText, DWORD dwItem);
BOOL          DoEventHook(const TCHAR* pszID, const char* pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, DWORD dwItem);
BOOL          IsEventSupported(int eventType);
BOOL          LogToFile(SESSION_INFO* si, GCEVENT * gce);

// message.c
TCHAR*        DoRtfToTags( char* pszRtfText, SESSION_INFO* si);

//////////////////////////////////////////////////////////////////////////////////

TCHAR* a2tf( const TCHAR* str, int flags );
TCHAR* replaceStr( TCHAR** dest, const TCHAR* src );
char*  replaceStrA( char** dest, const char* src );

#define DEFLOGFILENAME _T("%miranda_logpath%\\%proto%\\%userid%.log")
#endif
