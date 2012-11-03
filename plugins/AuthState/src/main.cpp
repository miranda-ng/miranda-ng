/*
   Authorization State plugin for Miranda-IM (www.miranda-im.org)
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

   File name      : $URL: http://svn.miranda.im/mainrepo/authstate/trunk/main.cpp $
   Revision       : $Rev: 1686 $
   Last change on : $Date: 2010-10-06 08:26:58 +0200 (Ср, 06 окт 2010) $
   Last change by : $Author: ghazan $

*/

#include "commonheaders.h"

HINSTANCE g_hInst;
static HANDLE hHookModulesLoaded = NULL, hSystemOKToExit = NULL, hOptInitialise = NULL, hIcoLibIconsChanged = NULL;
static HANDLE hHookExtraIconsRebuild = NULL, hHookExtraIconsApply = NULL, hContactSettingChanged = NULL, hContactAdded = NULL;
static HANDLE hPrebuildContactMenu = NULL, hAuthMenuSelected = NULL;
static HANDLE hUserMenu = NULL;
HANDLE hExtraIcon = NULL;
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

INT_PTR getIconToUse(HANDLE hContact, LPARAM lParam)
{
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
//	if (lParam == 1) return icon_none;
	if (!DBGetContactSettingByte(hContact,"AuthState","ShowIcons",!bIconsForRecentContacts)) return icon_none;

	if (DBGetContactSettingByte(0,"ICQ","UseServerCList",0))
		if (DBGetContactSettingWord(hContact,proto,"ServerId",1) == 0)
			return icon_both;

	if (bUseAuthIcon & bUseGrantIcon)
		if (DBGetContactSettingByte(hContact,proto,"Auth",0) && DBGetContactSettingByte(hContact,proto,"Grant",0))
			return icon_both;

	if (bUseAuthIcon)
		if (DBGetContactSettingByte(hContact,proto,"Auth",0))
			return icon_auth;

	if (bUseGrantIcon)
		if (DBGetContactSettingByte(hContact,proto,"Grant",0))
			return icon_grant;
	return icon_none;
}

int onExtraImageApplying(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
		return 0;

	int usedIcon = getIconToUse((HANDLE) wParam, lParam);

	const char *icon;
	switch (usedIcon) {
		case icon_both:   icon = "authgrant_icon";  break;
		case icon_grant:  icon = "grant_icon";  break;
		case icon_auth:   icon = "auth_icon";  break;
		default:          icon = NULL;  break;
	}
	ExtraIcon_SetIcon(hExtraIcon, (HANDLE)wParam, icon);
	return 0;
}

int onContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (!proto) return 0;

	if (!lstrcmpA(cws->szModule,proto))
		if (!lstrcmpA(cws->szSetting,"Auth") || !lstrcmpA(cws->szSetting,"Grant") || !lstrcmpA(cws->szSetting,"ServerId"))
			onExtraImageApplying(wParam, 1);

	return 0;
}

int onDBContactAdded(WPARAM wParam, LPARAM lParam)
{
	// A new contact added, mark it as recent
	DBWriteContactSettingByte((HANDLE)wParam, MODULENAME, "ShowIcons", 1);
	onExtraImageApplying(wParam, 0);

	return 0;
}

INT_PTR onAuthMenuSelected(WPARAM wParam, LPARAM lParam)
{
	byte enabled = DBGetContactSettingByte((HANDLE)wParam,"AuthState","ShowIcons",1);
	DBWriteContactSettingByte((HANDLE)wParam, MODULENAME, "ShowIcons", !enabled);

	onExtraImageApplying(wParam, 0);
	return 0;
}

int onPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	if (!proto) return 0;

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(CLISTMENUITEM);

	if (!DBGetContactSettingByte((HANDLE)wParam,proto,"Auth",0) && !DBGetContactSettingByte((HANDLE)wParam,proto,"Grant",0) && DBGetContactSettingWord((HANDLE)wParam,proto,"ServerId",0))
		mi.flags = CMIF_TCHAR | CMIM_FLAGS | CMIF_HIDDEN;
	else
		mi.flags = CMIF_TCHAR | CMIM_FLAGS;

	if (DBGetContactSettingByte((HANDLE)wParam,"AuthState","ShowIcons",1))
	{
		mi.flags |= CMIF_TCHAR | CMIM_NAME;
		mi.ptszName = _T("Disable AuthState icons");
	}
	else
	{
		mi.flags |= CMIF_TCHAR | CMIM_NAME;
		mi.ptszName = _T("Enable AuthState icons");
	}

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hUserMenu, (LPARAM)&mi);

	return 0;
}

int onModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	// IcoLib support
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;

	sid.ptszSection = _T("Auth State");
	sid.ptszDefaultFile = szFile;

	sid.ptszDescription = _T("Auth");
	sid.pszName = "auth_icon";
	sid.iDefaultIndex = -IDI_AUTH;
	Skin_AddIcon(&sid);

	sid.ptszDescription = _T("Grant");
	sid.pszName = "grant_icon";
	sid.iDefaultIndex = -IDI_GRANT;
	Skin_AddIcon(&sid);

	sid.ptszDescription = _T("Auth & Grant");
	sid.pszName = "authgrant_icon";
	sid.iDefaultIndex = -IDI_AUTHGRANT;
	Skin_AddIcon(&sid);

	// extra icons
	hExtraIcon = ExtraIcon_Register("authstate", "Auth State", "authgrant_icon");

	// Set initial value for all contacts
	HANDLE hContact = db_find_first();
	while (hContact != NULL) {
		onExtraImageApplying((WPARAM)hContact, 1);
		hContact = db_find_next(hContact);
	}

	hOptInitialise = HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	if (bContactMenuItem) hPrebuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);

	return 0;
}

int onSystemOKToExit(WPARAM wParam,LPARAM lParam)
{
	UnhookEvent(hHookModulesLoaded);
	UnhookEvent(hHookExtraIconsRebuild);
	UnhookEvent(hHookExtraIconsApply);
	UnhookEvent(hOptInitialise);
	UnhookEvent(hSystemOKToExit);
	UnhookEvent(hContactSettingChanged);
	if (hIcoLibIconsChanged) UnhookEvent(hIcoLibIconsChanged);
	if (hContactAdded) UnhookEvent(hContactAdded);
	if (hPrebuildContactMenu)
	{
		UnhookEvent(hPrebuildContactMenu);
		UnhookEvent(hUserMenu);
	}
	DestroyServiceFunction(hAuthMenuSelected);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	hSystemOKToExit = HookEvent(ME_SYSTEM_OKTOEXIT,onSystemOKToExit);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);

	bUseAuthIcon = DBGetContactSettingByte(NULL, MODULENAME, "EnableAuthIcon", 1);
	bUseGrantIcon = DBGetContactSettingByte(NULL, MODULENAME, "EnableGrantIcon", 1);
	bContactMenuItem = DBGetContactSettingByte(NULL, MODULENAME, "MenuItem", 0);
	bIconsForRecentContacts = DBGetContactSettingByte(NULL, MODULENAME, "EnableOnlyForRecent", 0);

	hContactAdded = HookEvent(ME_DB_CONTACT_ADDED, onDBContactAdded);

	if (bContactMenuItem)
	{
		hAuthMenuSelected = CreateServiceFunction("AuthState/MenuItem", onAuthMenuSelected);

		CLISTMENUITEM mi = {0};
		mi.cbSize = sizeof(CLISTMENUITEM);
		mi.position = -1999901005;
		mi.flags = CMIF_TCHAR;
		mi.ptszName = _T("Enable AuthState icons");
		mi.pszService = "AuthState/MenuItem";
		hUserMenu = Menu_AddContactMenuItem(&mi);
	}

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
