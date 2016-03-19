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

HINSTANCE g_hInst;
static HGENMENU hUserMenu;
HANDLE hExtraIcon;
int hLangpack;

Opts Options;

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

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {DACE7D41-DFA9-4772-89AE-A59A6153E6B2}
	{ 0xdace7d41, 0xdfa9, 0x4772, { 0x89, 0xae, 0xa5, 0x9a, 0x61, 0x53, 0xe6, 0xb2 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

INT_PTR getIconToUse(MCONTACT hContact, LPARAM)
{
	const char *proto = GetContactProto(hContact);
	//	if (lParam == 1) return icon_none;
	if (!db_get_b(hContact, "AuthState", "ShowIcons", !Options.bIconsForRecentContacts)) return ICON_NONE;

	if (db_get_b(0, "ICQ", "UseServerCList", 0))
		if (db_get_dw(hContact, proto, "ServerId", 1) == 0)
			return ICON_BOTH;

	// Facebook contact type
	int type = db_get_b(hContact, proto, "ContactType", 0);

	if (Options.bUseAuthIcon & Options.bUseGrantIcon)
		if ((db_get_b(hContact, proto, "Auth", 0) && db_get_b(hContact, proto, "Grant", 0)) || type == 2)
			return ICON_BOTH;

	if (Options.bUseAuthIcon)
		if (db_get_b(hContact, proto, "Auth", 0) || type == 3)
			return ICON_AUTH;

	if (Options.bUseGrantIcon)
		if (db_get_b(hContact, proto, "Grant", 0) || type == 4)
			return ICON_GRANT;

	return ICON_NONE;
}

int onExtraImageApplying(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;
	int iIcon = getIconToUse((MCONTACT)hContact, lParam);
	ExtraIcon_SetIcon(hExtraIcon, (MCONTACT)hContact, iIcon == -1 ? NULL : iconList[iIcon].hIcolib);
	return 0;
}

int onContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	const char *proto = GetContactProto((MCONTACT)hContact);
	if (!proto) return 0;

	if (!strcmp(cws->szModule, proto))
		if (!strcmp(cws->szSetting, "Auth") || !strcmp(cws->szSetting, "Grant") || !strcmp(cws->szSetting, "ServerId") || !strcmp(cws->szSetting, "ContactType"))
			onExtraImageApplying(hContact, 1);

	return 0;
}

int onDBContactAdded(WPARAM hContact, LPARAM)
{
	db_set_b((MCONTACT)hContact, MODULENAME, "ShowIcons", 1);
	onExtraImageApplying(hContact, 0);
	return 0;
}

INT_PTR onAuthMenuSelected(WPARAM hContact, LPARAM)
{
	db_set_b((MCONTACT)hContact, MODULENAME, "ShowIcons", 1 - db_get_b((MCONTACT)hContact, "AuthState", "ShowIcons", 1));
	onExtraImageApplying(hContact, 0);
	return 0;
}

int onPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!Options.bContactMenuItem)
	{
		Menu_ShowItem(hUserMenu, false);
		return 0;
	}

	char *proto = GetContactProto((MCONTACT)hContact);
	if (!proto)
		return 0;

	if (db_get_b((MCONTACT)hContact, "AuthState", "ShowIcons", 1))
		Menu_ModifyItem(hUserMenu, LPGENT("Disable AuthState icons"));
	else
		Menu_ModifyItem(hUserMenu, LPGENT("Enable AuthState icons"));

	Menu_ShowItem(hUserMenu, db_get_b((MCONTACT)hContact, proto, "Auth", 0) || db_get_b((MCONTACT)hContact, proto, "Grant", 0) || !db_get_dw((MCONTACT)hContact, proto, "ServerId", 0));
	return 0;
}


int onModulesLoaded(WPARAM, LPARAM)
{
	// extra icons
	hExtraIcon = ExtraIcon_RegisterIcolib("authstate", LPGEN("Auth state"), iconList[ICON_BOTH].szName);

	// Set initial value for all contacts
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		onExtraImageApplying((WPARAM)hContact, 1);

	HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
	HookEvent(ME_DB_CONTACT_ADDED, onDBContactAdded);
	CreateServiceFunction("AuthState/MenuItem", onAuthMenuSelected);

	Options.Load();

	CMenuItem mi;
	SET_UID(mi, 0xc5a784ea, 0x8b07, 0x4b95, 0xa2, 0xb2, 0x84, 0x9d, 0x87, 0x43, 0x7e, 0xda);
	mi.position = -1999901005;
	mi.flags = CMIF_TCHAR;
	mi.name.t = LPGENT("Enable AuthState icons");
	mi.pszService = "AuthState/MenuItem";
	hUserMenu = Menu_AddContactMenuItem(&mi);

	// IcoLib support
	Icon_Register(g_hInst, LPGEN("Auth state"), iconList, _countof(iconList));

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
