/*
Weather Protocol plugin for Miranda NG
Copyright (C) 2012-25 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This file contains the includes, weather constants/declarations,
   the structs, and the primitives for some of the functions.
*/

#pragma once

#include <share.h>
#include <time.h>
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <malloc.h>

#include <vector>

#include <newpluginapi.h>
#include <m_acc.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_clc.h>
#include <m_cluiframes.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_findadd.h>
#include <m_fontservice.h>
#include <m_history.h>
#include <m_icolib.h>
#include <m_ignore.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_skin_eng.h>
#include <m_userinfo.h>
#include <m_xstatus.h>

#include <m_tipper.h>
#include <m_toptoolbar.h>

#include "resource.h"
#include "version.h"
#include "proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CONSTANTS

// name
#define MODULENAME         "Weather"
#define DEFCURRENTWEATHER  "WeatherCondition"
#define WEATHERCONDITION   "Current"

// weather conditions
enum EWeatherCondition
{
	SUNNY,
	NA,
	PCLOUDY,
	CLOUDY,
	RAIN,
	RSHOWER,
	FOG,
	SNOW,
	SSHOWER,
	LIGHT,
	MAX_COND
};

// limits
#define MAX_TEXT_SIZE   4096
#define MAX_DATA_LEN    1024

// db info mangement mode
#define WDBM_REMOVE			1
#define WDBM_DETAILDISPLAY	2

// more info list column width
#define LIST_COLUMN		 150

// others
#define NODATA			TranslateT("N/A")
#define UM_SETCONTACT	40000

// weather update error codes
#define INVALID_ID_FORMAT  10
#define INVALID_SVC        11
#define INVALID_ID         12
#define SVC_NOT_FOUND      20
#define NETLIB_ERROR       30
#define DATA_EMPTY         40
#define DOC_NOT_FOUND      42
#define DOC_TOO_SHORT      43
#define UNKNOWN_ERROR      99

#define SM_WEATHERALERT		16
#define WM_UPDATEDATA      (WM_USER + 2687)

/////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES

extern HWND hPopupWindow;

extern MWindowList hDataWindowList, hWindowList;

extern HANDLE hTBButton;

extern HGENMENU hMwinMenu;

extern bool g_bIsUtf;

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_conv.c

void ClearStatusIcons();

void CaseConv(wchar_t *str);
void TrimString(char *str);
void TrimString(wchar_t *str);
void ConvertBackslashes(char *str);
char *GetSearchStr(char *dis);

wchar_t *GetDisplay(WEATHERINFO *w, const wchar_t *dis, wchar_t* str);

wchar_t *GetError(int code);

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_data.c

void DBDataManage(MCONTACT hContact, uint16_t Mode, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// functions in weather_info.c

const wchar_t *GetDefaultText(int c);

/////////////////////////////////////////////////////////////////////////////////////////
// function from multiwin module

void UpdateMwinData(MCONTACT hContact);

/////////////////////////////////////////////////////////////////////////////////////////
// utils

class JsonReply
{
	JSONNode *m_root = nullptr;
	int m_errorCode = 0;

public:
	JsonReply(MHttpResponse *);
	~JsonReply();

	__forceinline int error() const { return m_errorCode; }
	__forceinline JSONNode &data() const { return *m_root; }
	__forceinline operator bool() const { return m_errorCode == 200; }
};
