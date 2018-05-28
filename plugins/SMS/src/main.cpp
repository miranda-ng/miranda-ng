/*
Miranda-IM SMS Plugin
Copyright (C) 2001-2  Richard Hughes
Copyright (C) 2007-18  Rozhuk Ivan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------

This was the original words.
This plugin was modified by Ariel Shulman (NuKe007).
For any comments, problems, etc. contact me at Miranda-IM forums or E-Mail or ICQ.
All the information needed you can find at www.nuke007.tk
Enjoy the code and use it smartly!
*/

#include "stdafx.h"

CMPlugin g_plugin;

SMS_SETTINGS ssSMSSettings;

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
	// {CF97FD5D-B911-47A8-AF03-D21968B5B894}
	{0xcf97fd5d, 0xb911, 0x47a8, {0xaf, 0x3, 0xd2, 0x19, 0x68, 0xb5, 0xb8, 0x94}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static void VersionConversions()
{
	WCHAR wsztm[MAX_PATH];

	if (DB_SMS_GetStaticStringW(NULL, "UseSignature", wsztm, _countof(wsztm), nullptr))
		DB_SMS_SetByte(NULL, "UseSignature", (wsztm[0] == '0'));
	else
		DB_SMS_SetByte(NULL, "UseSignature", SMS_DEFAULT_USESIGNATURE);

	if (DB_SMS_GetStaticStringW(NULL, "SignaturePos", wsztm, _countof(wsztm), nullptr))
		DB_SMS_SetByte(NULL, "SignaturePos", (wsztm[0] == '0'));
	else
		DB_SMS_SetByte(NULL, "SignaturePos", SMS_DEFAULT_SIGNATUREPOS);

	if (DB_SMS_GetStaticStringW(NULL, "ShowACK", wsztm, _countof(wsztm), nullptr))
		DB_SMS_SetByte(NULL, "ShowACK", (wsztm[0] == '0'));
	else
		DB_SMS_SetByte(NULL, "ShowACK", SMS_DEFAULT_SHOWACK);
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	VersionConversions();
	LoadModules();
	return 0;
}

static int OnPreShutdown(WPARAM, LPARAM)
{
	RecvSMSWindowDestroy();
	SendSMSWindowDestroy();
	FreeAccountList();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	switch(dwReason) {
	case DLL_PROCESS_ATTACH:
		memset(&ssSMSSettings, 0, sizeof(ssSMSSettings));
		ssSMSSettings.hInstance = hInstance;
		ssSMSSettings.hHeap = HeapCreate(0, 0, 0);
		DisableThreadLibraryCalls((HMODULE)hInstance);
		break;
	
	case DLL_PROCESS_DETACH:
		HeapDestroy(ssSMSSettings.hHeap);
		ssSMSSettings.hHeap = nullptr;
	
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN,OnPreShutdown);

	SendSMSWindowInitialize();
	RecvSMSWindowInitialize();

	LoadServices();
	return 0;
}
