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
PLUGINLINK *pluginLink;
static HANDLE hHookModulesLoaded = NULL, hSystemOKToExit = NULL, hOptInitialise = NULL, hIcoLibIconsChanged = NULL;
static HANDLE hHookExtraIconsRebuild = NULL, hHookExtraIconsApply = NULL, hContactSettingChanged = NULL, hContactAdded = NULL;
static HANDLE hPrebuildContactMenu = NULL, hAuthMenuSelected = NULL;
static HANDLE hUserMenu = NULL;
HANDLE hExtraIcon = NULL;
struct MM_INTERFACE mmi;
int hLangpack;

IconExtraColumn g_IECAuth = {0};
IconExtraColumn g_IECGrant = {0};
IconExtraColumn g_IECAuthGrant = {0};
IconExtraColumn g_IECClear = {0};
INT clistIcon = 0; //Icon slot to use
BYTE bUseAuthIcon = 0, bUseGrantIcon = 0, bContactMenuItem = 0, bIconsForRecentContacts = 0, bUseAuthGroup = 0;

enum {
icon_none,
icon_auth,
icon_grant,
icon_both
};

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
	0, //doesn't replace anything built-in
	{0xdace7d41, 0xdfa9, 0x4772, {0x89, 0xae, 0xa5, 0x9a, 0x61, 0x53, 0xe6, 0xb2}}
/* DACE7D41-DFA9-4772-89AE-A59A6153E6B2 */
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

/* 236CBB9A-57D2-419D-B5CB-0DF5926E921C */
#define MIID_AUTHSTATE {0x236cbb9a, 0x57d2, 0x419d, {0xb5, 0xcb, 0x0d, 0xf5, 0x92, 0x6e, 0x92, 0x1c}}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	static const MUUID interfaces[] = {MIID_AUTHSTATE, MIID_LAST};
	return interfaces;
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

	int usedIcon;
	usedIcon = getIconToUse((HANDLE) wParam, lParam);

	if (hExtraIcon != NULL)
	{
		const char *icon;
		switch (usedIcon)
		{
			case icon_both:   icon = "authgrant_icon";  break;
			case icon_grant:  icon = "grant_icon";  break;
			case icon_auth:   icon = "auth_icon";  break;
			default:          icon = NULL;  break;
		}
		ExtraIcon_SetIcon(hExtraIcon, (HANDLE)wParam, icon);
	}
	else
	{
		switch (usedIcon)
		{
			case icon_both:   CallService(MS_CLIST_EXTRA_SET_ICON, wParam, (LPARAM) &g_IECAuthGrant); break;
			case icon_grant:  CallService(MS_CLIST_EXTRA_SET_ICON, wParam, (LPARAM) &g_IECGrant); break;
			case icon_auth:   CallService(MS_CLIST_EXTRA_SET_ICON, wParam, (LPARAM) &g_IECAuth); break;
			default:          CallService(MS_CLIST_EXTRA_SET_ICON, wParam, (LPARAM) &g_IECClear); break;
		}
	}

	return 0;
}

int onExtraImageListRebuild(WPARAM wParam, LPARAM lParam)
{
	g_IECAuth.cbSize = sizeof(IconExtraColumn);
	g_IECAuth.ColumnType = clistIcon;
	g_IECGrant.cbSize = sizeof(IconExtraColumn);
	g_IECGrant.ColumnType = clistIcon;
	g_IECAuthGrant.cbSize = sizeof(IconExtraColumn);
	g_IECAuthGrant.ColumnType = clistIcon;

	if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
	{
		if (ServiceExists(MS_SKIN2_ADDICON))
		{
			g_IECAuth.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"auth_icon"), (LPARAM)0);
			g_IECGrant.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"grant_icon"), (LPARAM)0);
			g_IECAuthGrant.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"authgrant_icon"), (LPARAM)0);
		}
		else
		{
			g_IECAuth.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_AUTH), IMAGE_ICON, 16, 16, LR_SHARED), (LPARAM)0);
			g_IECGrant.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_GRANT), IMAGE_ICON, 16, 16, LR_SHARED), (LPARAM)0);
			g_IECAuthGrant.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_AUTHGRANT), IMAGE_ICON, 16, 16, LR_SHARED), (LPARAM)0);
		}
	}

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

	if (enabled)
		CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECClear);
	else
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
	// hHookExtraIconsRebuild = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, onExtraImageListRebuild);
	// hHookExtraIconsApply = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
	// IcoLib support
	if (ServiceExists(MS_SKIN2_ADDICON))
	{
		SKINICONDESC sid = {0};
		ZeroMemory(&sid, sizeof(sid));
		TCHAR szFile[MAX_PATH];
		sid.cbSize = sizeof(sid);
		sid.flags = SIDF_ALL_TCHAR;

		sid.ptszSection = _T("Auth State");
		GetModuleFileName(g_hInst, szFile, MAX_PATH);
		sid.ptszDefaultFile = szFile;

		sid.ptszDescription = _T("Auth");
		sid.pszName = "auth_icon";
		sid.iDefaultIndex = -IDI_AUTH;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.ptszDescription = _T("Grant");
		sid.pszName = "grant_icon";
		sid.iDefaultIndex = -IDI_GRANT;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.ptszDescription = _T("Auth & Grant");
		sid.pszName = "authgrant_icon";
		sid.iDefaultIndex = -IDI_AUTHGRANT;
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

	// hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, onExtraImageListRebuild);
	}
	hExtraIcon = ExtraIcon_Register("authstate", "Auth State", "authgrant_icon");

	if (hExtraIcon != NULL)
	{
		// Set initial value for all contacts
		HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact != NULL)
		{
			onExtraImageApplying((WPARAM)hContact, 1);
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	}
	else
	{
		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, onExtraImageListRebuild);
		hHookExtraIconsRebuild = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, onExtraImageListRebuild);
		hHookExtraIconsApply = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
		onExtraImageListRebuild(0,0);
	}
	hOptInitialise = HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	if (bContactMenuItem) hPrebuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onPrebuildContactMenu);

	// Updater support
	if (ServiceExists(MS_UPDATE_REGISTER))
	{
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = lstrlenA((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;

		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data)
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL  = "http://thief.miranda.im/authstate.zip";
		update.szBetaVersionURL = "http://thief.miranda.im/updater/authstate_version.txt";
		update.szBetaChangelogURL = "http://thief.miranda.im";
		update.pbBetaVersionPrefix = (BYTE *)"AuthState ";

		update.cpbBetaVersionPrefix = lstrlenA((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

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

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);

	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	hSystemOKToExit = HookEvent(ME_SYSTEM_OKTOEXIT,onSystemOKToExit);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);

	clistIcon = DBGetContactSettingByte(NULL, MODULENAME, "AdvancedIcon", DefaultSlot);
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
		hUserMenu = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
	}

	g_IECClear.cbSize = sizeof(IconExtraColumn);
	g_IECClear.ColumnType = clistIcon;
	g_IECClear.hImage = (HANDLE) -1;

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
