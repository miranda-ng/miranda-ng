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

struct MODULEINFO : public GCModuleInfoBase {};
struct SESSION_INFO : public GCSessionInfoBase {};
struct LOGSTREAMDATA : public GCLogStreamDataBase {};

// special service for tweaking performance
#define MS_GC_GETEVENTPTR  "GChat/GetNewEventPtr"
typedef INT_PTR(*GETEVENTFUNC)(WPARAM wParam, LPARAM lParam);
struct GCPTRS
{
	GETEVENTFUNC pfnAddEvent;
};

extern HGENMENU hJoinMenuItem, hLeaveMenuItem;
extern GlobalLogSettingsBase *g_Settings;
extern int g_cbSession, g_cbModuleInfo, g_iFontMode, g_iChatLang;
extern TCHAR *g_szFontGroup;
extern mir_cs cs;

extern char* pLogIconBmpBits[14];
extern size_t logIconBmpSize[14];

// log.c
void   LoadMsgLogBitmaps(void);
void   FreeMsgLogBitmaps(void);
void   ValidateFilename (TCHAR *filename);
TCHAR* MakeTimeStamp(TCHAR *pszStamp, time_t time);
TCHAR* GetChatLogsFilename(SESSION_INFO *si, time_t tTime);
char*  Log_CreateRtfHeader(MODULEINFO *mi);
char*  Log_CreateRTF(LOGSTREAMDATA *streamData);
char*  Log_SetStyle(int style);

// clist.c
BOOL     AddEvent(MCONTACT hContact, HICON hIcon, MEVENT hEvent, int type, TCHAR* fmt, ...);
MCONTACT AddRoom(const char *pszModule, const TCHAR *pszRoom, const TCHAR *pszDisplayName, int iType);
MCONTACT FindRoom(const char *pszModule, const TCHAR *pszRoom);
BOOL     SetAllOffline(BOOL bHide, const char *pszModule);
BOOL     SetOffline(MCONTACT hContact, BOOL bHide);

int      RoomDoubleclicked(WPARAM wParam,LPARAM lParam);
INT_PTR  EventDoubleclicked(WPARAM wParam,LPARAM lParam);
INT_PTR  JoinChat(WPARAM wParam, LPARAM lParam);
INT_PTR  LeaveChat(WPARAM wParam, LPARAM lParam);
int      PrebuildContactMenu(WPARAM wParam, LPARAM lParam);
INT_PTR  PrebuildContactMenuSvc(WPARAM wParam, LPARAM lParam);

// colorchooser.c
void ColorChooser(SESSION_INFO *si, BOOL bFG, HWND hwndDlg, HWND hwndTarget, HWND hwndChooser);

// options.c
int    OptionsInit(void);
int    OptionsUnInit(void);
void   LoadMsgDlgFont(int i, LOGFONT * lf, COLORREF * colour);
void   LoadGlobalSettings(void);
HICON  LoadIconEx(char* pszIcoLibName, BOOL big);
void   LoadLogFonts(void);
void   SetIndentSize(void);
void   RegisterFonts(void);

// services.c
void   LoadChatIcons(void);
int    LoadChatModule(void);
void   UnloadChatModule(void);

// tools.c
int    DoRtfToTags(CMString &pszText, int iNumColors, COLORREF *pColors);
int    GetTextPixelSize(TCHAR* pszText, HFONT hFont, BOOL bWidth);
TCHAR *RemoveFormatting(const TCHAR* pszText);
BOOL   DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
int    GetColorIndex(const char *pszModule, COLORREF cr);
void   CheckColorsInModule(const char *pszModule);
int    GetRichTextLength(HWND hwnd);
BOOL   IsHighlighted(SESSION_INFO *si, GCEVENT *pszText);
UINT   CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText);
void   DestroyGCMenu(HMENU *hMenu, int iIndex);
BOOL   DoEventHookAsync(HWND hwnd, const TCHAR *pszID, const char *pszModule, int iType, const TCHAR* pszUID, const TCHAR* pszText, INT_PTR dwItem);
BOOL   DoEventHook(const TCHAR *pszID, const char *pszModule, int iType, const TCHAR *pszUID, const TCHAR* pszText, INT_PTR dwItem);
BOOL   IsEventSupported(int eventType);
BOOL   LogToFile(SESSION_INFO *si, GCEVENT *gce);
BOOL   DoTrayIcon(SESSION_INFO *si, GCEVENT *gce);
BOOL   DoPopup(SESSION_INFO *si, GCEVENT *gce);
int    ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, TCHAR* pszRoomName, COLORREF crBkg, const TCHAR* fmt, ...);

const TCHAR*  my_strstri(const TCHAR* s1, const TCHAR* s2);

#pragma comment(lib,"comctl32.lib")
