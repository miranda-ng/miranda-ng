/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * This code is based on and still contains large parts of the the
 * original chat module for Miranda NG, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 */

#ifndef _CHAT_H_
#define _CHAT_H_

//defines
#define OPTIONS_FONTCOUNT 20
#define GC_UPDATETITLE			(WM_USER+100)
#define GC_CLOSEWINDOW			(WM_USER+103)
#define GC_GETITEMDATA			(WM_USER+104)
#define GC_SETITEMDATA			(WM_USER+105)
#define GC_UPDATESTATUSBAR		(WM_USER+106)
#define GC_SETVISIBILITY		(WM_USER+107)
#define GC_SETWNDPROPS			(WM_USER+108)
#define GC_REDRAWLOG			(WM_USER+109)
#define GC_FIREHOOK				(WM_USER+110)
#define GC_FILTERFIX			(WM_USER+111)
#define GC_CHANGEFILTERFLAG		(WM_USER+112)
#define GC_SHOWFILTERMENU		(WM_USER+113)
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
#define GC_SCROLLTOBOTTOM		(WM_USER+129)
#define GC_SESSIONNAMECHANGE	(WM_USER+131)
#define GC_SETMESSAGEHIGHLIGHT	(WM_USER+139)
#define GC_REDRAWLOG2			(WM_USER+140)
#define GC_REDRAWLOG3			(WM_USER+141)

#define EM_ACTIVATE				(WM_USER+202)

#define GC_EVENT_HIGHLIGHT		0x1000
#define STATE_TALK				0x0001

#define ICON_ACTION				0
#define ICON_ADDSTATUS			1
#define ICON_HIGHLIGHT			2
#define ICON_INFO				3
#define ICON_JOIN				4
#define ICON_KICK				5
#define ICON_MESSAGE			6
#define ICON_MESSAGEOUT			7
#define ICON_NICK				8
#define ICON_NOTICE				9
#define ICON_PART				10
#define ICON_QUIT				11
#define ICON_REMSTATUS			12
#define ICON_TOPIC				13

#define ICON_STATUS1			14
#define ICON_STATUS2			15
#define ICON_STATUS3			16
#define ICON_STATUS4			17
#define ICON_STATUS0			18
#define ICON_STATUS5			19

enum TChatStatusEx
{
	CHAT_STATUS_NORMAL,
	CHAT_STATUS_AWAY,
	CHAT_STATUS_OFFLINE,
	CHAT_STATUS_MAX
};

// special service for tweaking performance
#define MS_GC_GETEVENTPTR  "GChat/GetNewEventPtr"
typedef INT_PTR (*GETEVENTFUNC)(WPARAM wParam, LPARAM lParam);
typedef struct  {
	GETEVENTFUNC pfnAddEvent;
}GCPTRS;

class CMUCHighlight;

//structs

struct MODULEINFO
{
	char          *pszModule;
	TCHAR         *ptszModDispName;
	char          *pszHeader;
	bool           bBold, bUnderline, bItalics, bColor, bBkgColor, bChanMgr, bAckMsg;
	int            nColorCount;
	COLORREF      *crColors;
				      
	int            iMaxText;
	DWORD          idleTimeStamp;
	DWORD          lastIdleCheck;
	TCHAR          tszIdleMsg[60];
	CMUCHighlight* Highlight;
	MODULEINFO *   next;
};

struct COMMAND_INFO
{
	char*  lpCommand;
	COMMAND_INFO *last, *next;
};

struct FONTINFO
{
	LOGFONT  lf;
	COLORREF color;
};

struct LOGINFO
{
	TCHAR   *ptszText, *ptszNick, *ptszUID, *ptszStatus, *ptszUserInfo;
	bool     bIsMe, bIsHighlighted;
	time_t   time;
	int      iType;
	DWORD    dwFlags;
	LOGINFO *next, *prev;
};

struct STATUSINFO
{
	TCHAR      *pszGroup;
	HICON       hIcon;
	WORD        Status;
	STATUSINFO *next;
};

struct USERINFO
{
	TCHAR        *pszNick, *pszUID;
	WORD          Status;
	WORD          ContactStatus;
	TChatStatusEx iStatusEx;
	USERINFO     *next;
};

struct SESSION_INFO
{
	HWND            hWnd;
				       
	bool            bFGSet, bBGSet, bFilterEnabled, bNicklistEnabled, bInitDone;
				       
	char           *pszModule;
	TCHAR          *ptszID;
	TCHAR          *ptszName;
	TCHAR          *ptszStatusbarText;
	TCHAR          *ptszTopic;
	TCHAR           pszLogFileName[MAX_PATH + 50];
				       
	char           *pszID;      // ugly fix for returning static ANSI strings in GC_INFO
	char           *pszName;   // just to fix a bug quickly, should die after porting IRC to Unicode
				       
	int             iType;
	int             iFG;
	int             iBG;
	int             iSplitterY;
	int             iSplitterX;
	int             iLogFilterFlags;
	int             iLogPopupFlags;
	int             iLogTrayFlags;
	int             iDiskLogFlags;
	int             nUsersInNicklist;
	int             iEventCount;
	int             iStatusCount;
				       
	WORD            wStatus;
	WORD            wState;
	WORD            wCommandsNum;
	DWORD           dwItemData;
	DWORD           dwFlags;
	HANDLE          hContact;
	HWND            hwndFilter;
	time_t          LastTime;
	int             iSearchItem;
	TCHAR           szSearch[255];
	CMUCHighlight  *Highlight;
	COMMAND_INFO   *lpCommands;
	COMMAND_INFO   *lpCurrentCommand;
	LOGINFO        *pLog;
	LOGINFO        *pLogEnd;
	USERINFO       *pUsers;
	USERINFO       *pMe;
	STATUSINFO     *pStatuses;
	TContainerData *pContainer;
	TWindowData    *dat;
	int             wasTrimmed;
	SESSION_INFO   *next;
};

struct LOGSTREAMDATA
{
	char         *buffer;
	int           bufferOffset, bufferLen;
	HWND          hwnd;
	LOGINFO      *lin;
	bool          bStripFormat, bRedraw;
	SESSION_INFO *si;
	int           crCount;
	TWindowData  *dat;
};

struct TMUCSettings
{
	HICON       hIconOverlay;
	bool        bShowTime, bShowTimeIfChanged, bLoggingEnabled;
	bool        bFlashWindow, bFlashWindowHightlight;
	bool        bOpenInDefault;
	bool        bLogIndentEnabled;
	bool        bStripFormat;
	bool        bBBCodeInPopups;
	bool        bTrayIconInactiveOnly;
	bool        bAddColonToAutoComplete;
	bool        bLogLimitNames;
	bool        bTimeStampEventColour;
	DWORD       dwIconFlags;
	int         LogTextIndent;
	long        LoggingLimit;
	int         iEventLimit;
	int         iEventLimitThreshold;
	int         iPopupStyle;
	int         iPopupTimeout;
	int         iSplitterX;
	int         iSplitterY;
	TCHAR      *pszTimeStamp;
	TCHAR      *pszTimeStampLog;
	TCHAR      *pszIncomingNick;
	TCHAR      *pszOutgoingNick;
	TCHAR       pszLogDir[MAX_PATH + 20];
	LONG        iNickListFontHeight;
	HFONT       NameFont;
	COLORREF    crUserListBGColor;
	COLORREF    crPUTextColour;
	COLORREF    crPUBkgColour;

	HFONT       UserListFonts[CHAT_STATUS_MAX];
	COLORREF    UserListColors[CHAT_STATUS_MAX];

	COLORREF    nickColors[8];
	HBRUSH      SelectionBGBrush;
	bool        bDoubleClick4Privat, bShowContactStatus, bContactStatusFirst;

	bool        bLogClassicIndicators, bAlternativeSorting, bAnnoyingHighlight, bCreateWindowOnHighlight;
	bool        bLogSymbols, bClassicIndicators, bClickableNicks, bColorizeNicks, bColorizeNicksInLog;
	bool        bScaleIcons, bUseDividers, bDividersUsePopupConfig, bMathMod;

	HANDLE      hGroup;
	CMUCHighlight* Highlight;
};

struct FLASH_PARAMS
{
	HANDLE hContact;
	const char* sound;
	int   iEvent;
	HICON hNotifyIcon;
	bool  bActiveTab, bHighlight, bInactive, bMustFlash, bMustAutoswitch;
	HWND  hWnd;
};

extern TMUCSettings g_Settings;

struct COLORCHOOSER
{
	MODULEINFO *pModule;
	int   xPosition, yPosition;
	HWND  hWndTarget;
	bool  bForeground;
	SESSION_INFO *si;
};

#pragma comment(lib,"comctl32.lib")

//////////////////////////////////////////////////////////////////////////////////

#include "chatprototypes.h"
#include "chat_resource.h"

TCHAR* a2tf(const TCHAR* str, int flags, DWORD cp = 0);

extern char *szChatIconString;

#define DEFLOGFILENAME _T("%miranda_logpath%\\%proto%\\%userid%.log")

#endif
