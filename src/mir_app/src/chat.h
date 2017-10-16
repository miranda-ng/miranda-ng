/*

Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include <m_smileyadd.h>
#include <m_popup.h>
#include <m_fontservice.h>

void Srmm_CreateToolbarIcons(HWND hwndDlg, int flags);
void Srmm_ProcessToolbarHotkey(MCONTACT hContact, INT_PTR iButtonFrom, HWND hwndDlg);

struct MODULEINFO : public GCModuleInfoBase {};
struct SESSION_INFO : public GCSessionInfoBase {};
struct LOGSTREAMDATA : public GCLogStreamDataBase {};

class CChatRoomDlg : public CSrmmBaseDialog
{
	CChatRoomDlg(); // just to suppress compiler's warnings, never implemented
};

extern HGENMENU hJoinMenuItem, hLeaveMenuItem;
extern GlobalLogSettingsBase *g_Settings;
extern int g_cbSession, g_cbModuleInfo, g_iFontMode, g_iChatLang;
extern wchar_t *g_szFontGroup;
extern mir_cs csChat;

extern HMENU g_hMenu;
extern HCURSOR g_hCurHyperlinkHand;
extern char* pLogIconBmpBits[14];
extern LIST<SESSION_INFO> g_arSessions;
extern MWindowList g_hWindowList;
extern HANDLE hevSendEvent, hevBuildMenuEvent;

// log.c
void     LoadMsgLogBitmaps(void);
void     FreeMsgLogBitmaps(void);
void     ValidateFilename (wchar_t *filename);
wchar_t* MakeTimeStamp(wchar_t *pszStamp, time_t time);
wchar_t* GetChatLogsFilename(SESSION_INFO *si, time_t tTime);
char*    Log_CreateRtfHeader();
char*    Log_CreateRTF(LOGSTREAMDATA *streamData);
char*    Log_SetStyle(int style);

// chat_manager.cpp
BOOL     SM_AddEvent(const wchar_t *pszID, const char *pszModule, GCEVENT *gce, bool bIsHighlighted);
BOOL     SM_ChangeNick(const wchar_t *pszID, const char *pszModule, GCEVENT *gce);
char*    SM_GetUsers(SESSION_INFO *si);
BOOL     SM_GiveStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus);
BOOL     SM_MoveUser(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID);
void     SM_RemoveAll(void);
int      SM_RemoveSession(const wchar_t *pszID, const char *pszModule, bool removeContact);
BOOL     SM_RemoveUser(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID);
BOOL     SM_SetContactStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, WORD wStatus);
BOOL     SM_SetOffline(const char *pszModule, SESSION_INFO *si);
BOOL     SM_SetStatus(const char *pszModule, SESSION_INFO *si, int wStatus);
BOOL     SM_TakeStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus);

SESSION_INFO* SM_FindSession(const wchar_t *pszID, const char *pszModule);

// clist.c
BOOL     AddEvent(MCONTACT hContact, HICON hIcon, MEVENT hEvent, int type, wchar_t* fmt, ...);
MCONTACT AddRoom(const char *pszModule, const wchar_t *pszRoom, const wchar_t *pszDisplayName, int iType);
MCONTACT FindRoom(const char *pszModule, const wchar_t *pszRoom);
BOOL     SetAllOffline(BOOL bHide, const char *pszModule);
BOOL     SetOffline(MCONTACT hContact, BOOL bHide);

int      RoomDoubleclicked(WPARAM wParam,LPARAM lParam);
INT_PTR  JoinChat(WPARAM wParam, LPARAM lParam);
INT_PTR  LeaveChat(WPARAM wParam, LPARAM lParam);
int      PrebuildContactMenu(WPARAM wParam, LPARAM lParam);

// options.c
int      OptionsInit(void);
int      OptionsUnInit(void);
void     LoadMsgDlgFont(int i, LOGFONT * lf, COLORREF * colour);
void     LoadGlobalSettings(void);
HICON    LoadIconEx(char* pszIcoLibName, bool big);
void     LoadLogFonts(void);
void     SetIndentSize(void);
void     RegisterFonts(void);

// services.c
void     LoadChatIcons(void);
int      LoadChatModule(void);
void     UnloadChatModule(void);

// tools.c
int      DoRtfToTags(CMStringW &pszText, int iNumColors, COLORREF *pColors);
wchar_t *RemoveFormatting(const wchar_t* pszText);
BOOL     DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
int      GetRichTextLength(HWND hwnd);
bool     IsHighlighted(SESSION_INFO *si, GCEVENT *pszText);
BOOL     IsEventSupported(int eventType);
BOOL     LogToFile(SESSION_INFO *si, GCEVENT *gce);
BOOL     DoTrayIcon(SESSION_INFO *si, GCEVENT *gce);
BOOL     DoPopup(SESSION_INFO *si, GCEVENT *gce);
int      ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, wchar_t* pszRoomName, COLORREF crBkg, const wchar_t* fmt, ...);

const wchar_t*  my_strstri(const wchar_t* s1, const wchar_t* s2);

#pragma comment(lib,"comctl32.lib")
