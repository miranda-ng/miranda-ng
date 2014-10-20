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

#include "seen.h"

HINSTANCE hInstance;
HANDLE ehmissed = NULL, ehuserinfo = NULL, ehmissed_proto = NULL;
HANDLE g_hShutdownEvent;

int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2D506D46-C94E-4EF8-8537-F11233A80381}
	{0x2d506d46, 0xc94e, 0x4ef8, {0x85, 0x37, 0xf1, 0x12, 0x33, 0xa8, 0x03, 0x81}}
};

#define TRANSNUMBER 2
DBVTranslation idleTr[TRANSNUMBER]={
	{any_to_IdleNotidleUnknown, _T("Any to Idle/Not Idle/Unknown"),0},
	{any_to_Idle, _T("Any to /Idle or empty"),0}
};

BOOL includeIdle;
LIST<logthread_info> arContacts(16, NumericKeySortT);
CRITICAL_SECTION csContacts;

void UninitHistoryDialog(void);

int MainInit(WPARAM,LPARAM)
{
	if ( ServiceExists(MS_TIPPER_ADDTRANSLATION))
		for (int i=0; i < TRANSNUMBER; i++)
			CallService(MS_TIPPER_ADDTRANSLATION, 0, (LPARAM)&idleTr[i]);

	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	SetEvent(g_hShutdownEvent);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	g_hShutdownEvent = CreateEvent(0, TRUE, FALSE, 0); 
	InitializeCriticalSection(&csContacts);

	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, UpdateValues);
	HookEvent(ME_PROTO_ACK,ModeChange);

	includeIdle = (BOOL )db_get_b(NULL,S_MOD,"IdleSupport",1);
		
	if ( db_get_b(NULL,S_MOD,"MenuItem",1))
		InitMenuitem();

	if ( db_get_b(NULL,S_MOD,"UserinfoTab",1))
		ehuserinfo = HookEvent(ME_USERINFO_INITIALISE,UserinfoInit);

	if ( db_get_b(NULL,S_MOD,"FileOutput",0))
		InitFileOutput();

	if ( db_get_b(NULL,S_MOD,"MissedOnes",0))
		ehmissed_proto = HookEvent(ME_PROTO_ACK,ModeChange_mo);

	SkinAddNewSoundExT("LastSeenTrackedStatusChange", LPGENT("LastSeen"), LPGENT("User status change"));
	SkinAddNewSoundExT("LastSeenTrackedStatusOnline", LPGENT("LastSeen"), LPGENT("Changed to Online"));
	SkinAddNewSoundExT("LastSeenTrackedStatusOffline", LPGENT("LastSeen"), LPGENT("User Logged Off"));
	SkinAddNewSoundExT("LastSeenTrackedStatusFromOffline", LPGENT("LastSeen"), LPGENT("User Logged In"));
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	if (ehmissed)
		UnhookEvent(ehmissed);

	DeleteCriticalSection(&csContacts);
	CloseHandle(g_hShutdownEvent);
	UninitHistoryDialog();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance=hinst;
	return 1;
}
