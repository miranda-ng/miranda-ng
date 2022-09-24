/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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

#define CLICKNICK_BEGIN L"~~++#"
#define CLICKNICK_END L"#++~~"

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
	uint32_t idleTimeStamp;
	wchar_t tszIdleMsg[60];
};

struct SESSION_INFO : public GCSessionInfoBase
{
	int iLogTrayFlags, iLogPopupFlags;
};

struct LOGSTREAMDATA : public GCLogStreamDataBase
{
	int crCount;
	CMsgDialog *dat;
};

struct TMUCSettings : public GlobalLogSettingsBase
{
	HICON    hIconOverlay;
	LONG     iNickListFontHeight;

	HFONT    UserListFonts[CHAT_STATUS_MAX];
	COLORREF UserListColors[CHAT_STATUS_MAX];

	COLORREF nickColors[8];
	HBRUSH   SelectionBGBrush;

	bool     bLogSymbols, bScaleIcons, bClickableNicks, bLogClassicIndicators, bColorizeNicksInLog;
	bool     bNewLineAfterNames, bAlternativeSorting, bColorizeNicks, bClassicIndicators;

	CMUCHighlight *Highlight;

	wchar_t *pwszAutoText;
};

extern TMUCSettings g_Settings;

#pragma comment(lib,"comctl32.lib")

//////////////////////////////////////////////////////////////////////////////////

// log.c
char* Log_CreateRtfHeader(void);
char* Log_CreateRTF(LOGSTREAMDATA *streamData);

// options.c
enum { FONTSECTION_AUTO, FONTSECTION_IM, FONTSECTION_IP };
void  LoadMsgDlgFont(int section, int i, LOGFONTW *lf, COLORREF *colour = nullptr);

void  AddIcons(void);
HICON LoadIconEx(char *pszIcoLibName);

// services.c
void ShowRoom(TContainerData *pContainer, SESSION_INFO *si);

// manager.c
SESSION_INFO* SM_FindSessionByHWND(HWND h);
SESSION_INFO* SM_FindSessionByHCONTACT(MCONTACT h);
SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const wchar_t* pszOriginal, const wchar_t* pszCurrent);

BOOL SM_ReconfigureFilters();

void OnCreateNick(const SESSION_INFO *si, const LOGINFO *lin, CMStringW &wszNick);

int UM_CompareItem(const USERINFO *u1, const USERINFO *u2);

// tools.c
BOOL     DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix);
bool     IsHighlighted(SESSION_INFO *si, GCEVENT *pszText);
char     GetIndicator(SESSION_INFO *si, LPCTSTR ptszNick, int *iNickIndex);
void     Chat_SetFilters(SESSION_INFO *si);
BOOL     DoPopup(SESSION_INFO *si, GCEVENT* gce);
int      ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, wchar_t* pszRoomName, COLORREF crBkg, const wchar_t* fmt, ...);

#include "chat_resource.h"

extern char szIndicators[];

#define DEFLOGFILENAME L"%miranda_logpath%\\%proto%\\%userid%.log"

#endif
