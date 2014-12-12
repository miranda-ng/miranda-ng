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

#include <windows.h>
#include <commctrl.h>

#include <malloc.h>

#include <newpluginapi.h>
#include <m_options.h>
#include <m_clc.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_message.h>
#include <m_clist.h>
#include <m_hotkeys.h>
#include <win2k.h>

#include <m_toptoolbar.h>
#include <m_sessions.h>

#include "Utils.h"
#include "Version.h"
#include "Resource.h"

#define MODNAME "Sessions"

#define MS_SESSIONS_LAUNCHME "Sessions/Service/LaunchMe"

extern IconItem iconList[];

#define MIIM_STRING	0x00000040

int DelUserDefSession(int ses_count);
int DeleteAutoSession(int ses_count);
int LoadSession(WPARAM, LPARAM);
int SaveSessionHandles(WPARAM, LPARAM);
INT_PTR SaveUserSessionHandles(WPARAM, LPARAM);
int SaveUserSessionName(TCHAR*);
INT_PTR CloseCurrentSession(WPARAM, LPARAM);
int SaveSessionDate();

extern HINSTANCE g_hInst;
extern MCONTACT session_list_recovered[255];
extern MCONTACT session_list[255];
extern int g_ses_limit;
extern size_t g_ses_count;
extern bool g_bExclHidden;	
extern bool g_bWarnOnHidden;
extern bool g_bOtherWarnings;
extern bool g_bCrashRecovery;
extern bool g_bIncompletedSave;

#define TIMERID_LOAD		  12
#define TIMERID_SHOW          11
#define TIMERID_REBUILDAFTER   14
