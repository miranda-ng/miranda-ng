/* 
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "commons.h"

HINSTANCE hInst;
int hLangpack = 0;

bool g_bAvsExist;

// Plugin data ////////////////////////////////////////////////////////////////////////////////////

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
	// {A82BAEB3-A33C-4036-B837-7803A5B6C2AB}
	{0xa82baeb3, 0xa33c, 0x4036, {0xb8, 0x37, 0x78, 0x3, 0xa5, 0xb6, 0xc2, 0xab}}
};

static IconItem iconList[] =
{
	{ LPGEN("Listening to"), "LISTENING_TO_ICON", IDI_LISTENINGTO },
	{ LPGEN("Previous account"), "MYDETAILS_PREV_PROTOCOL", IDI_LEFT_ARROW },
	{ LPGEN("Next account"), "MYDETAILS_NEXT_PROTOCOL", IDI_RIGHT_ARROW }
};

// Functions //////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

// Services ///////////////////////////////////////////////////////////////////////////////////////

static INT_PTR Menu_SetMyAvatarUI(WPARAM wParam, LPARAM lParam)
{
	return PluginCommand_SetMyAvatarUI(0, 0);
}

static INT_PTR Menu_SetMyNicknameUI(WPARAM wParam, LPARAM lParam)
{
	return PluginCommand_SetMyNicknameUI(0, 0);
}

static INT_PTR Menu_SetMyStatusMessageUI(WPARAM wParam, LPARAM lParam)
{
	return PluginCommand_SetMyStatusMessageUI(0, 0);
}

// Hook called after init
static int MainInit(WPARAM wparam, LPARAM lparam)
{
	g_bAvsExist = ServiceExists(MS_AV_GETMYAVATAR) != 0;
	g_bFramesExist = ServiceExists(MS_CLIST_FRAMES_ADDFRAME) != 0;

	InitProtocolData();

	// Add options to menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.popupPosition = 500050000;
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_USERDETAILS);
	mi.ptszName = LPGENT("My details");
	HANDLE hMenuRoot = Menu_AddMainMenuItem(&mi);

	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;
	mi.ptszPopupName = (TCHAR *)hMenuRoot;
	mi.popupPosition = 0;
	mi.icolibItem = NULL;

	if (protocols->CanSetAvatars()) {
		mi.position = 100001;
		mi.ptszName = LPGENT("Set my avatar...");
		CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYAVATARUI, Menu_SetMyAvatarUI);
		mi.pszService = "MENU_" MS_MYDETAILS_SETMYAVATARUI;
		Menu_AddMainMenuItem(&mi);
	}

	mi.position = 100002;
	mi.ptszName = LPGENT("Set my nickname...");
	CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYNICKNAMEUI, Menu_SetMyNicknameUI);
	mi.pszService = "MENU_" MS_MYDETAILS_SETMYNICKNAMEUI;
	Menu_AddMainMenuItem(&mi);

	mi.position = 100003;
	mi.ptszName = LPGENT("Set my status message...");
	CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYSTATUSMESSAGEUI, Menu_SetMyStatusMessageUI);
	mi.pszService = "MENU_" MS_MYDETAILS_SETMYSTATUSMESSAGEUI;
	Menu_AddMainMenuItem(&mi);

	// Set protocols to show frame
	mi.position = 200001;
	mi.ptszName = LPGENT("Show next account");
	mi.pszService = MS_MYDETAILS_SHOWNEXTPROTOCOL;
	Menu_AddMainMenuItem(&mi);

	InitFrames();
	return 0;
}

static int MainUninit(WPARAM wParam, LPARAM lParam) 
{
	DeInitFrames();
	return 0;
}

// Load ///////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load()
{
	mir_getLP(&pluginInfo);

	// Hook event to load messages and show first one
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainUninit);

	// Options
	InitOptions();

	if (Skin_GetIcon("LISTENING_TO_ICON") == NULL)
		Icon_Register(hInst, LPGEN("Contact list"), iconList, 1);

	Icon_Register(hInst, LPGEN("My details"), iconList + 1, SIZEOF(iconList) - 1);

	// Register services
	CreateServiceFunction(MS_MYDETAILS_SETMYNICKNAME, PluginCommand_SetMyNickname);
	CreateServiceFunction(MS_MYDETAILS_SETMYNICKNAMEUI, PluginCommand_SetMyNicknameUI);
	CreateServiceFunction(MS_MYDETAILS_SETMYAVATAR, PluginCommand_SetMyAvatar);
	CreateServiceFunction(MS_MYDETAILS_SETMYAVATARUI, PluginCommand_SetMyAvatarUI);
	CreateServiceFunction(MS_MYDETAILS_GETMYNICKNAME, PluginCommand_GetMyNickname);
	CreateServiceFunction(MS_MYDETAILS_GETMYAVATAR, PluginCommand_GetMyAvatar);
	CreateServiceFunction(MS_MYDETAILS_SETMYSTATUSMESSAGEUI, PluginCommand_SetMyStatusMessageUI);
	CreateServiceFunction(MS_MYDETAILS_SHOWNEXTPROTOCOL, PluginCommand_ShowNextProtocol);
	CreateServiceFunction(MS_MYDETAILS_SHOWPREVIOUSPROTOCOL, PluginCommand_ShowPreviousProtocol);
	CreateServiceFunction(MS_MYDETAILS_SHOWPROTOCOL, PluginCommand_ShowProtocol);
	CreateServiceFunction(MS_MYDETAILS_CYCLE_THROUGH_PROTOCOLS, PluginCommand_CycleThroughtProtocols);
	return 0;
}

// Unload /////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload(void)
{
	DeInitProtocolData();
	return 0;
}
