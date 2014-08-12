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

#include "commonheaders.h"

HINSTANCE g_hInst;
static HANDLE hOptInitialise;
static HANDLE hHookExtraIconsRebuild, hHookExtraIconsApply;
static HANDLE hAuthMenuSelected;
static HGENMENU hUserMenu;
HANDLE hExtraIcon;
int hLangpack;

BYTE bUseAuthIcon = 0, bUseGrantIcon = 0, bContactMenuItem = 0, bIconsForRecentContacts = 0, bUseAuthGroup = 0;

enum {
icon_none,
icon_auth,
icon_grant,
icon_both
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
	{0xdace7d41, 0xdfa9, 0x4772, {0x89, 0xae, 0xa5, 0x9a, 0x61, 0x53, 0xe6, 0xb2}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

INT_PTR getIconToUse(MCONTACT hContact, LPARAM lParam)
{
	char *proto = GetContactProto(hContact);
//	if (lParam == 1) return icon_none;
	if (!db_get_b(hContact,"AuthState","ShowIcons",!bIconsForRecentContacts)) return icon_none;

	if (db_get_b(0,"ICQ","UseServerCList",0))
		if (db_get_dw(hContact,proto,"ServerId",1) == 0)
			return icon_both;

	// Facebook contact type
	int type = db_get_b(hContact, proto, "ContactType", 0);

	if (bUseAuthIcon & bUseGrantIcon)
		if ((db_get_b(hContact,proto,"Auth",0) && db_get_b(hContact,proto,"Grant",0)) || type == 2)
			return icon_both;

	if (bUseAuthIcon)
		if (db_get_b(hContact,proto,"Auth",0) || type == 3)
			return icon_auth;

	if (bUseGrantIcon)
		if (db_get_b(hContact,proto,"Grant",0) || type == 4)
			return icon_grant;

	return icon_none;
}

int onExtraImageApplying(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	int usedIcon = getIconToUse(hContact, lParam);

	const char *icon;
	switch (usedIcon) {
		case icon_both:   icon = "authgrant_icon";  break;
		case icon_grant:  icon = "grant_icon";  break;
		case icon_auth:   icon = "auth_icon";  break;
		default:          icon = NULL;  break;
	}
	ExtraIcon_SetIcon(hExtraIcon, hContact, icon);
	return 0;
}

int onContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	char *proto = GetContactProto(hContact);
	if (!proto) return 0;

	if (!lstrcmpA(cws->szModule,proto))
		if (!lstrcmpA(cws->szSetting,"Auth") || !lstrcmpA(cws->szSetting,"Grant") || !lstrcmpA(cws->szSetting,"ServerId") || !lstrcmpA(cws->szSetting,"ContactType"))
			onExtraImageApplying(hContact, 1);

	return 0;
}

int onDBContactAdded(WPARAM hContact, LPARAM lParam)
{
	// A new contact added, mark it as recent
	db_set_b(hContact, MODULENAME, "ShowIcons", 1);
	onExtraImageApplying(hContact, 0);

	return 0;
}

INT_PTR onAuthMenuSelected(WPARAM hContact, LPARAM lParam)
{
	byte enabled = db_get_b(hContact,"AuthState","ShowIcons",1);
	db_set_b(hContact, MODULENAME, "ShowIcons", !enabled);

	onExtraImageApplying(hContact, 0);
	return 0;
}

int onPrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	char *proto = GetContactProto(hContact);
	if (!proto)
		return 0;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR | CMIM_NAME;
	if (db_get_b(hContact,"AuthState","ShowIcons",1))
		mi.ptszName = LPGENT("Disable AuthState icons");
	else
		mi.ptszName = LPGENT("Enable AuthState icons");
	Menu_ModifyItem(hUserMenu, &mi);

	Menu_ShowItem(hUserMenu, db_get_b(hContact,proto,"Auth",0) || db_get_b(hContact,proto,"Grant",0) || !db_get_dw(hContact,proto,"ServerId",0));
	return 0;
}

static IconItem iconList[] = 
{
	{ LPGEN("Auth"),  "auth_icon", IDI_AUTH },
	{ LPGEN("Grant"), "grant_icon", IDI_GRANT },
	{ LPGEN("Auth & Grant"), "authgrant_icon", IDI_AUTHGRANT }
};

int onModulesLoaded(WPARAM, LPARAM)
{
	// Set initial value for all contacts
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		onExtraImageApplying(hContact, 1);

	hOptInitialise = HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	if (bContactMenuItem)
		HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);

	return 0;
}

static int onShutdown(WPARAM, LPARAM)
{
	DestroyServiceFunction(hAuthMenuSelected);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);

	bUseAuthIcon = db_get_b(NULL, MODULENAME, "EnableAuthIcon", 1);
	bUseGrantIcon = db_get_b(NULL, MODULENAME, "EnableGrantIcon", 1);
	bContactMenuItem = db_get_b(NULL, MODULENAME, "MenuItem", 0);
	bIconsForRecentContacts = db_get_b(NULL, MODULENAME, "EnableOnlyForRecent", 0);

	HookEvent(ME_DB_CONTACT_ADDED, onDBContactAdded);

	if (bContactMenuItem) {
		hAuthMenuSelected = CreateServiceFunction("AuthState/MenuItem", onAuthMenuSelected);

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position = -1999901005;
		mi.flags = CMIF_TCHAR;
		mi.ptszName = LPGENT("Enable AuthState icons");
		mi.pszService = "AuthState/MenuItem";
		hUserMenu = Menu_AddContactMenuItem(&mi);
	}

	// IcoLib support
	Icon_Register(g_hInst, "Auth State", iconList, SIZEOF(iconList));

	// extra icons
	hExtraIcon = ExtraIcon_Register("authstate", LPGEN("Auth State"), "authgrant_icon");

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
