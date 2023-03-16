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

#pragma once

#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>

#include <malloc.h>
#include <vector>

#include <newpluginapi.h>
#include <m_clc.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_hotkeys.h>
#include <m_icolib.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_metacontacts.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_srmm_int.h>
#include <m_utils.h>

#include <m_toptoolbar.h>
#include <m_sessions.h>

#include "Utils.h"
#include "version.h"
#include "Resource.h"

#define MODULENAME "Sessions"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMOption<int> g_lastUserId, g_lastDateId;
	CMOption<bool> bExclHidden, bWarnOnHidden, bOtherWarnings, bCrashRecovery;
	CMOption<uint16_t> iTrackCount, iStartupDelay;

	CMPlugin();

	void CheckImport();

	int Load() override;
};

/////////////////////////////////////////////////////////////////////////////////////////

struct CSession
{
	int id;
	CMStringW wszName;
	std::vector<MCONTACT> contacts;
	bool bIsFavorite = false, bIsUser = false;

	void fromString(const char *str);
	std::string toString() const;

	CMStringA getSetting() const;
	void remove();
	void save();
};

extern OBJLIST<CSession> g_arUserSessions, g_arDateSessions;

/////////////////////////////////////////////////////////////////////////////////////////

int  LoadSession(CSession *pSession);

void CALLBACK LaunchSessions();

INT_PTR CloseCurrentSession(WPARAM, LPARAM);
INT_PTR LoadLastSession(WPARAM, LPARAM);
INT_PTR OpenSessionsManagerWindow(WPARAM, LPARAM);
INT_PTR SaveUserSessionHandles(WPARAM, LPARAM);

extern MCONTACT session_list_recovered[255];
extern MCONTACT session_list[255];

extern HWND g_hDlg, g_hSDlg;
extern bool g_bLastSessionPresent;
extern bool g_bIncompletedSave;

#define TIMERID_SHOW 11
#define TIMERID_LOAD 12

