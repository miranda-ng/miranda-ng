/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

HANDLE ehmissed = nullptr, ehuserinfo = nullptr, ehmissed_proto = nullptr;
HANDLE g_hShutdownEvent;
MWindowList g_pUserInfo;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2D506D46-C94E-4EF8-8537-F11233A80381}
	{ 0x2d506d46, 0xc94e, 0x4ef8, { 0x85, 0x37, 0xf1, 0x12, 0x33, 0xa8, 0x03, 0x81 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bUsePopups(MODULENAME, "UsePopups", false)
{}

/////////////////////////////////////////////////////////////////////////////////////////

#define TRANSNUMBER 2
DBVTranslation idleTr[TRANSNUMBER] = {
	{ any_to_IdleNotidleUnknown, L"Any to Idle/Not Idle/Unknown", 0 },
	{ any_to_Idle, L"Any to /Idle or empty", 0 }
};

static int CompareProtos(const char *p1, const char *p2)
{	return mir_strcmp(p1, p2);
}

BOOL includeIdle;
LIST<char> arWatchedProtos(10, CompareProtos);
LIST<logthread_info> arContacts(16, NumericKeySortT);
mir_cs csContacts;

void UninitHistoryDialog(void);

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR GetParsedFormat(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	ptrW wszStamp(g_plugin.getWStringA("MenuStamp"));

	CMStringW wszRet(ParseString(wszStamp ? wszStamp : DEFAULT_MENUSTAMP, hContact));

	return (INT_PTR)wszRet.Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////

static int MainInit(WPARAM, LPARAM)
{
	if (g_bFileActive = g_plugin.getByte("FileOutput", 0))
		InitFileOutput();

	if (g_plugin.getByte("MenuItem", 1))
		InitMenuitem();

	if (g_plugin.getByte("UserinfoTab", 1))
		ehuserinfo = HookEvent(ME_USERINFO_INITIALISE, UserinfoInit);

	if (ServiceExists(MS_TIPPER_ADDTRANSLATION))
		for (int i = 0; i < TRANSNUMBER; i++)
			CallService(MS_TIPPER_ADDTRANSLATION, 0, (LPARAM)&idleTr[i]);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	SetEvent(g_hShutdownEvent);
	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Clock"), "clock", IDI_CLOCK },
};

int CMPlugin::Load()
{
	registerIcon(MODULENAME, iconList);
	
	g_pUserInfo = WindowList_Create();
	g_hShutdownEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, UpdateValues);
	HookEvent(ME_PROTO_ACK, ModeChange);

	includeIdle = (BOOL)g_plugin.getByte("IdleSupport", 1);

	if (g_plugin.getByte("MissedOnes", 0))
		ehmissed_proto = HookEvent(ME_PROTO_ACK, ModeChange_mo);

	LoadWatchedProtos();

	g_plugin.addSound("LastSeenTrackedStatusChange", LPGENW("LastSeen"), LPGENW("User status change"));
	g_plugin.addSound("LastSeenTrackedStatusOnline", LPGENW("LastSeen"), LPGENW("Changed to Online"));
	g_plugin.addSound("LastSeenTrackedStatusOffline", LPGENW("LastSeen"), LPGENW("User Logged Off"));
	g_plugin.addSound("LastSeenTrackedStatusFromOffline", LPGENW("LastSeen"), LPGENW("User Logged In"));

	CreateServiceFunction(MS_LASTSEEN_GET, GetParsedFormat);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UninitFileOutput();
	UnloadWatchedProtos();

	WindowList_Destroy(g_pUserInfo);

	if (ehmissed)
		UnhookEvent(ehmissed);

	CloseHandle(g_hShutdownEvent);
	UninitHistoryDialog();
	return 0;
}
