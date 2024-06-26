/*
	Authorization State plugin for Miranda NG (www.miranda-ng.org)
	(c) 2006-2010 by Thief

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/

#include "stdafx.h"

static HGENMENU hUserMenu;
static HANDLE hExtraIcon;

CMPlugin g_plugin;

static IconItem iconList[] =
{
	{ LPGEN("Request"), "auth_icon", IDI_AUTH },
	{ LPGEN("Grant"), "grant_icon", IDI_GRANT },
	{ LPGEN("Request & Grant"), "authgrant_icon", IDI_AUTHGRANT }
};

enum
{
	ICON_NONE = -1,
	ICON_AUTH,
	ICON_GRANT,
	ICON_BOTH
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {DACE7D41-DFA9-4772-89AE-A59A6153E6B2}
	{0xdace7d41, 0xdfa9, 0x4772, {0x89, 0xae, 0xa5, 0x9a, 0x61, 0x53, 0xe6, 0xb2}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bUseAuthIcon(MODULENAME, "EnableAuthIcon", 1),
	bUseGrantIcon(MODULENAME, "EnableGrantIcon", 1),
	bIconsForRecentContacts(MODULENAME, "EnableOnlyForRecent", 0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR getIconToUse(MCONTACT hContact, LPARAM)
{
	const char *proto = Proto_GetBaseAccountName(hContact);
	//	if (lParam == 1) return icon_none;
	if (!g_plugin.getByte(hContact, "ShowIcons", !g_plugin.bIconsForRecentContacts))
		return ICON_NONE;

	// Facebook contact type
	int type = db_get_b(hContact, proto, "ContactType", 0);

	if (g_plugin.bUseAuthIcon & g_plugin.bUseGrantIcon)
		if ((db_get_b(hContact, proto, "Auth", 0) && db_get_b(hContact, proto, "Grant", 0)) || type == 2)
			return ICON_BOTH;

	if (g_plugin.bUseAuthIcon)
		if (db_get_b(hContact, proto, "Auth", 0) || type == 3)
			return ICON_AUTH;

	if (g_plugin.bUseGrantIcon)
		if (db_get_b(hContact, proto, "Grant", 0) || type == 4)
			return ICON_GRANT;

	return ICON_NONE;
}

int onExtraImageApplying(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;
	int iIcon = getIconToUse((MCONTACT)hContact, lParam);
	ExtraIcon_SetIcon(hExtraIcon, (MCONTACT)hContact, iIcon == -1 ? nullptr : iconList[iIcon].hIcolib);
	return 0;
}

int onContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	const char *proto = Proto_GetBaseAccountName((MCONTACT)hContact);
	if (!proto)
		return 0;

	if (!strcmp(cws->szModule, proto))
		if (!strcmp(cws->szSetting, "Auth") || !strcmp(cws->szSetting, "Grant") || !strcmp(cws->szSetting, "ServerId") || !strcmp(cws->szSetting, "ContactType"))
			onExtraImageApplying(hContact, 1);

	return 0;
}

int onDBContactAdded(WPARAM hContact, LPARAM)
{
	g_plugin.setByte(hContact, "ShowIcons", 1);
	onExtraImageApplying(hContact, 0);
	return 0;
}

INT_PTR onAuthMenuSelected(WPARAM hContact, LPARAM)
{
	g_plugin.setByte(hContact, "ShowIcons", 1 - g_plugin.getByte(hContact, "ShowIcons", 1));
	onExtraImageApplying(hContact, 0);
	return 0;
}

int onPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	char *proto = Proto_GetBaseAccountName((MCONTACT)hContact);
	if (!proto)
		return 0;

	if (g_plugin.getByte(hContact, "ShowIcons", 1))
		Menu_ModifyItem(hUserMenu, LPGENW("Disable AuthState icons"));
	else
		Menu_ModifyItem(hUserMenu, LPGENW("Enable AuthState icons"));

	Menu_ShowItem(hUserMenu, db_get_b((MCONTACT)hContact, proto, "Auth", 0) || db_get_b((MCONTACT)hContact, proto, "Grant", 0) || !db_get_dw((MCONTACT)hContact, proto, "ServerId", 0));
	return 0;
}


int onModulesLoaded(WPARAM, LPARAM)
{
	// extra icons
	hExtraIcon = ExtraIcon_RegisterIcolib("authstate", LPGEN("Auth state"), iconList[ICON_BOTH].hIcolib);

	// Set initial value for all contacts
	for (auto &hContact : Contacts())
		onExtraImageApplying((WPARAM)hContact, 1);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);
	return 0;
}

int CMPlugin::Load()
{
	// IcoLib support
	g_plugin.registerIcon(LPGEN("Auth state"), iconList);

	HookEvent(ME_DB_CONTACT_ADDED, onDBContactAdded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
	HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);

	CreateServiceFunction("AuthState/MenuItem", onAuthMenuSelected);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xc5a784ea, 0x8b07, 0x4b95, 0xa2, 0xb2, 0x84, 0x9d, 0x87, 0x43, 0x7e, 0xda);
	mi.position = -1999901005;
	mi.name.a = LPGEN("Enable AuthState icons");
	mi.pszService = "AuthState/MenuItem";
	hUserMenu = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hUserMenu, MCI_OPT_DISABLED, TRUE);
	return 0;
}
