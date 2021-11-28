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

#pragma once

#include <m_smileyadd.h>
#include <m_popup.h>
#include <m_fontservice.h>

void Srmm_CreateToolbarIcons(HWND hwndDlg, int flags);
void Srmm_ProcessToolbarHotkey(MCONTACT hContact, INT_PTR iButtonFrom, HWND hwndDlg);

class CLogWindow : public CSrmmLogWindow {};

extern HPLUGIN  g_pChatPlugin;
extern int      g_cbSession, g_cbModuleInfo, g_iFontMode;
extern wchar_t *g_szFontGroup;
extern mir_cs   csChat;

extern HICON    g_hChatIcons[20];
extern DWORD    g_dwDiskLogFlags;
extern HCURSOR  g_hCurHyperlinkHand;
extern char*    pLogIconBmpBits[14];
extern HANDLE   hevSendEvent, hevBuildMenuEvent;

extern MWindowList g_hWindowList;
extern LIST<SESSION_INFO> g_arSessions;
extern GlobalLogSettingsBase *g_Settings;

extern CMOption<bool> g_bChatTrayInactive, g_bChatPopupInactive;

// log.c
void          LoadMsgLogBitmaps(void);
void          FreeMsgLogBitmaps(void);
void          RedrawLog2(SESSION_INFO *si);
void          ValidateFilename (wchar_t *filename);
wchar_t*      MakeTimeStamp(wchar_t *pszStamp, time_t time);
wchar_t*      GetChatLogsFilename(SESSION_INFO *si, time_t tTime);
char*         Log_CreateRtfHeader();
char*         Log_CreateRTF(LOGSTREAMDATA *streamData);
char*         Log_SetStyle(int style);

// chat_manager.cpp
MODULEINFO*   MM_AddModule(const char *pszModule);
MODULEINFO*   MM_FindModule(const char *pszModule);

BOOL          SM_AddEvent(const wchar_t *pszID, const char *pszModule, GCEVENT *gce, bool bIsHighlighted);
BOOL          SM_ChangeNick(const wchar_t *pszID, const char *pszModule, GCEVENT *gce);
char*         SM_GetUsers(SESSION_INFO *si);
BOOL          SM_GiveStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus);
void          SM_RemoveAll(void);
int           SM_RemoveSession(const wchar_t *pszID, const char *pszModule, bool removeContact);
BOOL          SM_RemoveUser(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID);
BOOL          SM_SetContactStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, WORD wStatus);
BOOL          SM_SetOffline(const char *pszModule, SESSION_INFO *si);
BOOL          SM_SetStatus(const char *pszModule, SESSION_INFO *si, int wStatus);
BOOL          SM_TakeStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus);
BOOL          SM_UserTyping(GCEVENT* gce);

SESSION_INFO* SM_FindSession(const wchar_t *pszID, const char *pszModule);
SESSION_INFO* SM_FindSessionByIndex(const char *pszModule, int iItem);

STATUSINFO*   TM_AddStatus(STATUSINFO **ppStatusList, const wchar_t *pszStatus, int *iCount);
WORD          TM_StringToWord(STATUSINFO *pStatusList, const wchar_t *pszStatus);

BOOL          UM_RemoveAll(SESSION_INFO *si);
BOOL          UM_SetStatusEx(SESSION_INFO *si, const wchar_t* pszText, int flags);
void          UM_SortKeys(SESSION_INFO *si);
void          UM_SortUser(SESSION_INFO *si);

// clist.c
MCONTACT      AddRoom(const char *pszModule, const wchar_t *pszRoom, const wchar_t *pszDisplayName, int iType);
MCONTACT      FindRoom(const char *pszModule, const wchar_t *pszRoom);
BOOL          SetAllOffline(BOOL bHide, const char *pszModule);
BOOL          SetOffline(MCONTACT hContact, BOOL bHide);
		        
int           RoomDoubleclicked(WPARAM wParam,LPARAM lParam);

// options.c
void          ChatOptionsInit(WPARAM wParam);

int           OptionsInit(void);
int           OptionsUnInit(void);
void          LoadMsgDlgFont(int i, LOGFONT * lf, COLORREF * colour);
void          LoadGlobalSettings(void);
HICON         LoadIconEx(char* pszIcoLibName, bool big);
void          LoadLogFonts(void);
void          SetIndentSize(void);
void          RegisterFonts(void);

// services.c
void          LoadChatIcons(void);
int           LoadChatModule(void);
void          UnloadChatModule(void);

// tools.c
int           DoRtfToTags(CMStringW &pszText, int iNumColors, COLORREF *pColors);
wchar_t*      RemoveFormatting(const wchar_t* pszText);
BOOL          DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
int           GetRichTextLength(HWND hwnd);
bool          IsHighlighted(SESSION_INFO *si, GCEVENT *pszText);
BOOL          IsEventSupported(int eventType);
BOOL          LogToFile(SESSION_INFO *si, GCEVENT *gce);
BOOL          DoTrayIcon(SESSION_INFO *si, GCEVENT *gce);
BOOL          DoPopup(SESSION_INFO *si, GCEVENT *gce);
int           ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, wchar_t* pszRoomName, COLORREF crBkg, const wchar_t* fmt, ...);

CSrmmLogWindow *Srmm_GetLogWindow(CMsgDialog *pDlg);

void          Chat_RemoveContact(MCONTACT hContact);

#pragma comment(lib,"comctl32.lib")
