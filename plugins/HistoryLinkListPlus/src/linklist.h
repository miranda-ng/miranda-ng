// History Linklist Plus
// Copyright (C) 2010 Thomas Wendel, gureedo
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#pragma once

#ifndef _LINKLIST_H
#define _LINKLIST_H

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <richedit.h>
#include <commctrl.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_langpack.h>
#include <win2k.h>
#include <m_options.h>

#include "language.h"
#include "linklist_dlg.h"
#include "resource.h"
#include "Version.h"


// Filter Flags
#define WLL_URL		0x01
#define WLL_MAIL	0x02
#define WLL_FILE	0x04
#define WLL_IN		0x08
#define WLL_OUT		0x10
#define WLL_ALL		(WLL_URL | WLL_MAIL | WLL_FILE | WLL_IN | WLL_OUT)
#define SLL_DEEP	0x20

// String length
#define LINK_MAX	1024
#define DIR_SIZE	6
#define TYPE_SIZE	5
#define DATE_SIZE	11
#define TIME_SIZE	15

// Link types
#define LINK_UNKNOWN	0x00
#define LINK_URL	0x01
#define LINK_MAIL	0x02
#define LINK_FILE	0x03

// Directions
#define DIRECTION_IN	1
#define DIRECTION_OUT	2


#define FILTERTEXT	125

#define IN_COL_DEF	0x005050A0
#define	OUT_COL_DEF	0x00206020
#define BG_COL_DEF	0x00EAFFFF
#define TXT_COL_DEF	0x00000000

struct LISTELEMENT {
	BYTE direction;
	BYTE type;
	TCHAR date[DATE_SIZE];
	TCHAR time[TIME_SIZE];
	TCHAR link[LINK_MAX];
	HANDLE hEvent;
	int	linePos;
	struct LISTELEMENT *nextElement;
} ;

typedef struct {
	BYTE openNewWindow;
	BYTE updateWindow;
	BYTE mouseEvent;
	BYTE saveSpecial;
	BYTE showDate;
	BYTE showLine;
	BYTE showTime;
	BYTE showDirection;
	BYTE showType;
} LISTOPTIONS;

typedef struct {
	DWORD incoming;
	DWORD outgoing;
	DWORD background;
	DWORD text;
} MYCOLOURSET;

#include "linklist_fct.h"

#define LINKLIST_MODULE			"HistoryLinklist"
#define LINKLIST_IN_COL			"InColour"
#define LINKLIST_OUT_COL		"OutColour"
#define LINKLIST_BG_COL			"BGColour"
#define LINKLIST_TXT_COL		"TxtColour"
#define LINKLIST_USE_DEF		"UseMirandaDefault"
#define LINKLIST_OPEN_WINDOW		"OpenNewWindow"
#define LINKLIST_UPDATE_WINDOW		"UpdateWindow"
#define LINKLIST_MOUSE_EVENT		"MessageView"
#define LINKLIST_LEFT			"WindowLeft"
#define LINKLIST_RIGHT			"WindowRight"
#define LINKLIST_BOTTOM			"WindowBottom"
#define LINKLIST_TOP			"WindowTop"
#define LINKLIST_SPLITPOS		"SplitterPos"
#define LINKLIST_SAVESPECIAL		"SavePosSpecial"
#define LINKLIST_FIRST			"FirstStartup"
#define LINKLIST_SHOW_DATE		"ShowDate"
#define LINKLIST_SHOW_LINE		"ShowLine"
#define LINKLIST_SHOW_TIME		"ShowTime"
#define LINKLIST_SHOW_DIRECTION		"ShowMessageDirection"
#define LINKLIST_SHOW_TYPE		"ShowMessageType"


#define _mstrlen(x) (SIZEOF(x) - 1)
#define MAKE_TXT_COL(BGCol) ((DWORD)~BGCol & 0x00FFFFFF)

#define DM_LINKSPLITTER			WM_USER+99

typedef struct LISTELEMENT LISTELEMENT;

// Dialogbox Parameter
typedef struct {
	MCONTACT hContact;
	LISTELEMENT *listStart;
	UINT findMessage;
	CHARRANGE chrg;
	int splitterPosNew;
	int splitterPosOld;
	SIZE minSize;
} DIALOGPARAM;


static INT_PTR LinkList_Main(WPARAM, LPARAM);
int InitOptionsDlg(WPARAM, LPARAM);
int DBUpdate(WPARAM, LPARAM);
int ExtractURI(DBEVENTINFO*, HANDLE, LISTELEMENT*);
int RemoveList(LISTELEMENT*);
int ListCount(LISTELEMENT*);

#endif //_LINKLIST_H