/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _CRT_SECURE_NO_DEPRECATE

#define _WIN32_WINNT 0x0501

#define MIRANDA_VER    0x0900
#define MIRANDA_CUSTOM_LP

#include <windows.h>
#include <commctrl.h>

#include "newpluginapi.h"
#include "m_utils.h"
#include "m_system.h"
#include "m_options.h"
#include "m_clc.h"
#include "m_database.h"
#include "m_langpack.h"
#include "m_icolib.h"
#include "m_message.h"
#include "m_clist.h"
#include "m_hotkeys.h"
#include "win2k.h"

#include "m_updater.h"
#include "m_toolbar.h"
#include "m_sessions.h"

#include "Utils.h"
#include "../Version.h"
#include "../Resource.h"

extern HANDLE hibChecked;
extern HANDLE hibNotChecked;

#define MIIM_STRING	0x00000040

int DelUserDefSession(int  ses_count);
int DeleteAutoSession(int ses_count);
int LoadSession(WPARAM ,LPARAM );
int SaveSessionHandles(WPARAM ,LPARAM );
INT_PTR SaveUserSessionHandles(WPARAM ,LPARAM );
int SaveUserSessionName(TCHAR*);
INT_PTR CloseCurrentSession(WPARAM ,LPARAM );
int SaveSessionDate();

extern HINSTANCE hinstance;
extern DWORD session_list_recovered[255];
extern DWORD session_list[255];
extern unsigned int ses_limit;
extern unsigned int g_ses_count;
extern BOOL g_bExclHidden;	
extern BOOL g_bWarnOnHidden;
extern BOOL g_bOtherWarnings;
extern BOOL g_bCrashRecovery;
extern BOOL g_bIncompletedSave;

#define TIMERID_LOAD		  12
#define TIMERID_SHOW          11
#define TIMERID_REBUILDAFTER   14

#ifdef _UNICODE
#define SESSIONS_NAME "sessions_u"
#else
#define SESSIONS_NAME "sessions_a"
#endif

#define SESSIONS_VERSION_URL "http://miranda.radicaled.ru/public/updater/sessions.txt"

#define SESSIONS_UPDATE_URL "http://miranda.radicaled.ru/public/sessions/"SESSIONS_NAME".zip"

#define SESSIONS_UPDATE_URL "http://miranda.radicaled.ru/public/sessions/"SESSIONS_NAME".zip"

#define SESSIONS_VERSION_PREFIX "Sessions "
