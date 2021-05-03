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

#include "stdafx.h"

CMPlugin g_plugin;

bool g_bAvsExist;

static IconItem iconList[] =
{
	{ LPGEN("Listening to"), "LISTENING_TO_ICON", IDI_LISTENINGTO },
	{ LPGEN("Previous account"), "MYDETAILS_PREV_PROTOCOL", IDI_LEFT_ARROW },
	{ LPGEN("Next account"), "MYDETAILS_NEXT_PROTOCOL", IDI_RIGHT_ARROW }
};

// Plugin data ////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A82BAEB3-A33C-4036-B837-7803A5B6C2AB}
	{ 0xa82baeb3, 0xa33c, 0x4036, { 0xb8, 0x37, 0x78, 0x3, 0xa5, 0xb6, 0xc2, 0xab } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

// Services ///////////////////////////////////////////////////////////////////////////////////////

static INT_PTR Menu_SetMyAvatarUI(WPARAM, LPARAM)
{
	return PluginCommand_SetMyAvatarUI(0, 0);
}

static INT_PTR Menu_SetMyNicknameUI(WPARAM, LPARAM)
{
	return PluginCommand_SetMyNicknameUI(0, 0);
}

static INT_PTR Menu_SetMyStatusMessageUI(WPARAM, LPARAM)
{
	return PluginCommand_SetMyStatusMessageUI(0, 0);
}

// Hook called after init
static int MainInit(WPARAM, LPARAM)
{
	g_bAvsExist = ServiceExists(MS_AV_GETMYAVATAR) != 0;
	g_bFramesExist = ServiceExists(MS_CLIST_FRAMES_ADDFRAME) != 0;

	InitProtocolData();

	// Add options to menu
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x73ff525d, 0x7b8, 0x49cc, 0xa2, 0xdf, 0xc5, 0xad, 0x13, 0xf3, 0x8e, 0x2d);
	mi.position = 500050000;
	mi.flags =  CMIF_UNICODE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_USERDETAILS);
	mi.name.w = LPGENW("My details");
	HGENMENU hMenuRoot = Menu_AddMainMenuItem(&mi);

	mi.flags =  CMIF_UNICODE;
	mi.root = hMenuRoot;
	mi.hIcolibItem = nullptr;

	if (protocols.CanSetAvatars()) {
		SET_UID(mi, 0xe5b2d79e, 0xd25a, 0x4f72, 0xa4, 0x1a, 0x21, 0xfd, 0x48, 0x6b, 0xb5, 0x6);
		mi.position = 100001;
		mi.name.w = LPGENW("Set my avatar...");
		CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYAVATARUI, Menu_SetMyAvatarUI);
		mi.pszService = "MENU_" MS_MYDETAILS_SETMYAVATARUI;
		Menu_AddMainMenuItem(&mi);
	}

	SET_UID(mi, 0xa327838f, 0xea6, 0x4ba5, 0x95, 0x92, 0xd8, 0xc4, 0x80, 0x84, 0x3, 0x50);
	mi.position = 100002;
	mi.name.w = LPGENW("Set my nickname...");
	CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYNICKNAMEUI, Menu_SetMyNicknameUI);
	mi.pszService = "MENU_" MS_MYDETAILS_SETMYNICKNAMEUI;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x57c0d407, 0x61e1, 0x4c08, 0x8f, 0xcb, 0x65, 0x6a, 0x73, 0x3e, 0x20, 0xa8);
	mi.position = 100003;
	mi.name.w = LPGENW("Set my status message...");
	CreateServiceFunction("MENU_" MS_MYDETAILS_SETMYSTATUSMESSAGEUI, Menu_SetMyStatusMessageUI);
	mi.pszService = "MENU_" MS_MYDETAILS_SETMYSTATUSMESSAGEUI;
	Menu_AddMainMenuItem(&mi);

	// Set protocols to show frame
	SET_UID(mi, 0x248530a2, 0xfc37, 0x413a, 0x87, 0x62, 0xb1, 0xd1, 0xa8, 0x87, 0x39, 0x5c);
	mi.position = 200001;
	mi.name.w = LPGENW("Show next account");
	mi.pszService = MS_MYDETAILS_SHOWNEXTPROTOCOL;
	Menu_AddMainMenuItem(&mi);

	InitFrames();
	return 0;
}

static int MainUninit(WPARAM, LPARAM)
{
	DeInitFrames();
	return 0;
}

// Load ///////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	g_plugin.registerIcon(LPGEN("My details"), iconList);

	// Hook event to load messages and show first one
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainUninit);
	HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);

	// Options
	LoadOptions();

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

int CMPlugin::Unload()
{
	DeInitProtocolData();
	return 0;
}
