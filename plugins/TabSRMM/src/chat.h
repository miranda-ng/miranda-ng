/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

#ifndef _CHAT_H_
#define _CHAT_H_

//defines

enum TChatStatusEx
{
	CHAT_STATUS_NORMAL,
	CHAT_STATUS_AWAY,
	CHAT_STATUS_OFFLINE,
	CHAT_STATUS_MAX
};

class CMUCHighlight;

// structs

struct MODULEINFO : public GCModuleInfoBase
{
	DWORD idleTimeStamp;
	DWORD lastIdleCheck;
	wchar_t tszIdleMsg[60];
};

struct SESSION_INFO : public GCSessionInfoBase
{
	int iLogTrayFlags, iLogPopupFlags, iDiskLogFlags;
};

struct LOGSTREAMDATA : public GCLogStreamDataBase
{
	int crCount;
	CChatRoomDlg *dat;
};

struct TMUCSettings : public GlobalLogSettingsBase
{
	HICON    hIconOverlay;
	DWORD    dwIconFlags;
	LONG     iNickListFontHeight;

	HFONT    UserListFonts[CHAT_STATUS_MAX];
	COLORREF UserListColors[CHAT_STATUS_MAX];

	COLORREF nickColors[8];
	HBRUSH   SelectionBGBrush;
	bool     bOpenInDefault, bBBCodeInPopups;
	bool     bShowContactStatus, bContactStatusFirst;

	bool     bLogClassicIndicators, bAlternativeSorting, bAnnoyingHighlight, bCreateWindowOnHighlight;
	bool     bLogSymbols, bClassicIndicators, bClickableNicks, bColorizeNicks, bColorizeNicksInLog;
	bool     bScaleIcons, bUseCommaAsColon, bNewLineAfterNames;

	CMUCHighlight* Highlight;
};

struct FLASH_PARAMS
{
	MCONTACT hContact;
	const char* sound;
	int   iEvent;
	HICON hNotifyIcon;
	bool  bActiveTab, bHighlight, bInactive, bMustFlash, bMustAutoswitch;
	HWND  hWnd;
};

extern TMUCSettings g_Settings;

#pragma comment(lib,"comctl32.lib")

//////////////////////////////////////////////////////////////////////////////////

// log.c
char* Log_CreateRtfHeader(void);
char* Log_CreateRTF(LOGSTREAMDATA *streamData);

// options.c
enum { FONTSECTION_AUTO, FONTSECTION_IM, FONTSECTION_IP };
void  LoadMsgDlgFont(int section, int i, LOGFONT *lf, COLORREF *colour, char *szModule);
void  LoadLogfont(int section, int i, LOGFONTA *lf, COLORREF *colour, char *szModule);

void  AddIcons(void);
HICON LoadIconEx(char *pszIcoLibName);

// services.c
void ShowRoom(TContainerData *pContainer, SESSION_INFO *si);

// manager.c
SESSION_INFO* SM_FindSessionByHWND(HWND h);
SESSION_INFO* SM_FindSessionByHCONTACT(MCONTACT h);
SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const wchar_t* pszOriginal, const wchar_t* pszCurrent);

BOOL SM_ReconfigureFilters();

int UM_CompareItem(USERINFO *u1, const wchar_t* pszNick, WORD wStatus);

// tools.c
BOOL     DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
wchar_t* my_strstri(const wchar_t* s1, const wchar_t* s2);
bool     IsHighlighted(SESSION_INFO *si, GCEVENT *pszText);
char     GetIndicator(SESSION_INFO *si, LPCTSTR ptszNick, int *iNickIndex);
void     Chat_SetFilters(SESSION_INFO *si);
void     DoFlashAndSoundWorker(FLASH_PARAMS* p);
BOOL     DoPopup(SESSION_INFO *si, GCEVENT* gce);
int      ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, wchar_t* pszRoomName, COLORREF crBkg, const wchar_t* fmt, ...);
BOOL     LogToFile(SESSION_INFO *si, GCEVENT *gce);

#include "chat_resource.h"

extern char szIndicators[];

#define DEFLOGFILENAME L"%miranda_logpath%\\%proto%\\%userid%.log"

#endif
