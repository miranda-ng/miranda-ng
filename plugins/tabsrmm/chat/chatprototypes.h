/*
Chat module plugin for Miranda IM

Copyright (C) 2003-2009 Joergen Persson and others

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

$Id: chatprototypes.h 13184 2010-12-07 14:16:58Z silvercircle $

*/

void LoadIcons(void);
void LoadLogIcons(void);
void FreeIcons(void);
void UpgradeCheck(void);

//colorchooser.c
INT_PTR CALLBACK DlgProcColorToolWindow(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

//log.c
void   Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO* si, BOOL bRedraw, BOOL bPhaseTwo);
void   LoadMsgLogBitmaps(void);
void   FreeMsgLogBitmaps(void);
TCHAR* GetChatLogsFilename(SESSION_INFO *si, time_t tTime);
TCHAR* MakeTimeStamp(TCHAR* pszStamp, time_t time);
char*  Log_CreateRtfHeader(MODULEINFO * mi);

//window.c
INT_PTR CALLBACK RoomWndProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
int GetTextPixelSize( TCHAR* pszText, HFONT hFont, BOOL bWidth);

//options.c
enum { FONTSECTION_AUTO, FONTSECTION_CHAT, FONTSECTION_IM, FONTSECTION_IP };
int   OptionsInit(void);
int   OptionsUnInit(void);
void  LoadMsgDlgFont(int section, int i, LOGFONT * lf, COLORREF * colour, char* szMod );
void  LoadGlobalSettings(void);
void  AddIcons(void);
HICON LoadIconEx(int iIndex, char * pszIcoLibName, int iX, int iY);

//services.c
void HookEvents(void);
void UnhookEvents(void);
int  CreateServiceFunctions(void);
void DestroyServiceFunctions(void);
void DestroyHookableEvents(void);
void CreateHookableEvents(void);
void TabsInit(void);
int  ModulesLoaded(WPARAM wParam,LPARAM lParam);
int  SmileyOptionsChanged(WPARAM wParam,LPARAM lParam);
int  PreShutdown(WPARAM wParam,LPARAM lParam);
int  IconsChanged(WPARAM wParam,LPARAM lParam);
void ShowRoom(SESSION_INFO* si, WPARAM wp, BOOL bSetForeground);
INT_PTR  Service_AddEvent(WPARAM wParam, LPARAM lParam);
int  Service_ItemData(WPARAM wParam, LPARAM lParam);
int  Service_SetSBText(WPARAM wParam, LPARAM lParam);
int  Service_SetVisibility(WPARAM wParam, LPARAM lParam);
INT_PTR  Service_GetCount(WPARAM wParam,LPARAM lParam);

HWND CreateNewRoom(TContainerData *pContainer, SESSION_INFO *si, BOOL bActivateTab, BOOL bPopupContainer, BOOL bWantPopup);

//manager.c
void          SetActiveSession(const TCHAR* pszID, const char* pszModule);
void          SetActiveSessionEx(SESSION_INFO* si);
SESSION_INFO* GetActiveSession(void);
SESSION_INFO* SM_AddSession(const TCHAR* pszID, const char* pszModule);
int           SM_RemoveSession(const TCHAR* pszID, const char* pszModule);
SESSION_INFO* SM_FindSession(const TCHAR* pszID, const char* pszModule);
USERINFO*     SM_AddUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus);
SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const TCHAR* pszOriginal, const TCHAR* pszCurrent);
BOOL          SM_ChangeUID(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszNewUID);
BOOL          SM_ChangeNick(const TCHAR* pszID, const char* pszModule, GCEVENT * gce);
BOOL          SM_RemoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID);
BOOL          SM_SetOffline(const TCHAR* pszID, const char* pszModule);
HICON         SM_GetStatusIcon(SESSION_INFO* si, USERINFO* ui, char* szIndicator);
BOOL          SM_SetStatus(const TCHAR* pszID, const char* pszModule, int wStatus);
BOOL          SM_SetStatusEx(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText, int flags );
BOOL          SM_SendUserMessage(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText);
STATUSINFO*   SM_AddStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszStatus);
BOOL          SM_AddEventToAllMatchingUID(GCEVENT * gce, BOOL bisHighLight = FALSE);
BOOL          SM_AddEvent(const TCHAR* pszID, const char* pszModule, GCEVENT * gce, BOOL bIsHighlighted);
LRESULT       SM_SendMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL          SM_PostMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL          SM_BroadcastMessage(const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync);
BOOL          SM_RemoveAll (void);
BOOL          SM_GiveStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus);
BOOL          SM_SetContactStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, WORD pszStatus);
BOOL          SM_TakeStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus);
BOOL          SM_MoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID);
void          SM_AddCommand(const TCHAR* pszID, const char* pszModule, const char* lpNewCommand);
char*         SM_GetPrevCommand(const TCHAR* pszID, const char* pszModule);
char*         SM_GetNextCommand(const TCHAR* pszID, const char* pszModule);
int           SM_GetCount(const char* pszModule);
SESSION_INFO* SM_FindSessionByHWND(HWND h);
SESSION_INFO* SM_FindSessionByHCONTACT(HANDLE h);
SESSION_INFO* SM_FindSessionByIndex(const char* pszModule, int iItem);
char*         SM_GetUsers(SESSION_INFO* si);
USERINFO*     SM_GetUserFromIndex(const TCHAR* pszID, const char* pszModule, int index);
BOOL		  SM_ReconfigureFilters();
BOOL		  SM_InvalidateLogDirectories();
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
INT_PTR       CList_EventDoubleclicked(WPARAM wParam,LPARAM lParam);
INT_PTR       CList_JoinChat(WPARAM wParam, LPARAM lParam);
INT_PTR       CList_LeaveChat(WPARAM wParam, LPARAM lParam);
int           CList_PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
INT_PTR		  CList_PrebuildContactMenuSvc(WPARAM wParam, LPARAM lParam);
void          CList_CreateGroup(TCHAR* group);
BOOL          CList_AddEvent(HANDLE hContact, HICON Icon, HANDLE event, int type, const TCHAR* fmt, ... ) ;
HANDLE        CList_FindRoom (const char* pszModule, const TCHAR* pszRoom) ;
int           WCCmp(TCHAR* wild, TCHAR*string);

//tools.c
TCHAR*        RemoveFormatting(const TCHAR* pszText, bool fLower = false, bool fStripCR = false, TCHAR* buf = 0, const size_t len = 0);
BOOL          DoSoundsFlashPopupTrayStuff(SESSION_INFO* si, GCEVENT * gce, BOOL bHighlight, int bManyFix);
int           Chat_GetColorIndex(const char* pszModule, COLORREF cr);
void          CheckColorsInModule(const char* pszModule);
TCHAR*        my_strstri(const TCHAR* s1, const TCHAR* s2) ;
int           GetRichTextLength(HWND hwnd);
BOOL          IsHighlighted(SESSION_INFO* si, const TCHAR* pszText);
UINT          CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO* si, TCHAR* pszUID, TCHAR* pszWordText);
void          DestroyGCMenu(HMENU *hMenu, int iIndex);
BOOL          DoEventHookAsync(HWND hwnd, const TCHAR* pszID, const char* pszModule, int iType, TCHAR* pszUID, TCHAR* pszText, DWORD dwItem);
BOOL          DoEventHook(const TCHAR* pszID, const char* pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, DWORD dwItem);
BOOL          IsEventSupported(int eventType);
BOOL          LogToFile(SESSION_INFO* si, GCEVENT * gce);
void          Chat_SetFilters(SESSION_INFO *si);
void 		  TSAPI DoFlashAndSoundWorker(FLASH_PARAMS* p);
// message.c
char*         Chat_Message_GetFromStream(HWND hwndDlg, SESSION_INFO* si);
TCHAR*        Chat_DoRtfToTags( char* pszRtfText, SESSION_INFO* si);
