/*
   IgnoreState plugin for Miranda-IM (www.miranda-im.org)
   (c) 2010 by Kildor

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
PLUGINLINK *pluginLink;
HANDLE hHookModulesLoaded = NULL, hSystemOKToExit = NULL, hOptInitialise = NULL, hIcoLibIconsChanged = NULL;
HANDLE hHookExtraIconsRebuild = NULL, hHookExtraIconsApply = NULL, hContactSettingChanged = NULL;
HANDLE hPrebuildContactMenu = NULL;
HANDLE hExtraIcon = NULL;
int hLangpack;
struct MM_INTERFACE mmi;

INT currentFilter = 0;
//static int bUseMirandaSettings = 0;

IconExtraColumn g_IECIgnoreFull = {0};
IconExtraColumn g_IECIgnorePart = {0};
IconExtraColumn g_IECIgnoreMess = {0};
IconExtraColumn g_IECClear = {0};




INT clistIcon = 0; //Icon slot to use
//DWORD dMask = IGNOREEVENT_USERONLINE; // by default hide online notification

//DWORD dIgnore = 0x0;

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
	0,
	{0xa6872bcd, 0xf2a1, 0x41b8, {0xb2, 0xf1, 0xdd, 0x7c, 0xec, 0x05, 0x57, 0x34}}
	// a6872bcd-f2a1-41b8-b2f1-dd7cec055734
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
   g_hInst = hinstDLL;
   return TRUE;
}

// плагининфо
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 0, 29))
	{
		MessageBox( NULL, TranslateT("The plugin requires Miranda IM 0.8.0.29 or later for correct applying its preferences."), TranslateT("IgnoreState"),
			MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TOPMOST );
	}

	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	static const MUUID interfaces[] = {MIID_IGNORESRATE, MIID_LAST};
	return interfaces;
}

int onSystemOKToExit(WPARAM wParam,LPARAM lParam)
{
   UnhookEvent(hHookModulesLoaded);
   UnhookEvent(hHookExtraIconsRebuild);
   UnhookEvent(hHookExtraIconsApply);
   UnhookEvent(hOptInitialise);
   UnhookEvent(hSystemOKToExit);
   if (hIcoLibIconsChanged) UnhookEvent(hIcoLibIconsChanged);
	if (hContactSettingChanged)
		UnhookEvent(hContactSettingChanged);

   return 0;
}


inline BOOL checkState(int type)
{
	return ((currentFilter>>(type-1))&1);
}


INT_PTR isIgnored(HANDLE hContact, int type)
{
	int i = 0, all = 0, filtered = 0;

	if (type == IGNOREEVENT_ALL)
	{
		int cii = SIZEOF(ii);
		for (i = 1; i < cii; i++)
		{
			if (isIgnored(hContact, ii[i].type))
			{
//				checkState(ii[i].type) ? filtered++ : all++;
				ii[i].filtered ? filtered++ : all++;
			}
		}
		return (all+filtered == cii-1) ? 1 : (all > 0 ? -1 : 0) ;
	}
	else
		return CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, (LPARAM)type);
}

int onExtraImageApplying(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	int ignore = isIgnored(hContact, IGNOREEVENT_ALL);
	if (hExtraIcon != NULL)
	{
		if (ignore == 1)
		{
		ExtraIcon_SetIcon(hExtraIcon, hContact, "ignore_full");
		}
  	else if (ignore == 0)
	  {
		ExtraIcon_SetIcon(hExtraIcon, hContact, "");
  	}
  	else if (isIgnored((HANDLE)wParam, IGNOREEVENT_MESSAGE))
  	{
		ExtraIcon_SetIcon(hExtraIcon, hContact, "ignore_mess");
	  }
  	else
	  {
		ExtraIcon_SetIcon(hExtraIcon, hContact, "ignore_part");
  	}
  }
  else
  {
		if (ignore == 1)
		{
			CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECIgnoreFull);
		}
		else if (ignore == 0)
	  {
			CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECClear);
	 	}
		else if (isIgnored((HANDLE)wParam, IGNOREEVENT_MESSAGE))
		{
			CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECIgnoreMess);
		}
		else
		{
			CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) wParam, (LPARAM) &g_IECIgnorePart);
		}
	}
  return 0;
}


int onExtraImageListRebuild(WPARAM wParam, LPARAM lParam)
{
  g_IECIgnoreMess.cbSize = sizeof(IconExtraColumn);
  g_IECIgnoreMess.ColumnType = clistIcon;
  g_IECIgnorePart.cbSize = sizeof(IconExtraColumn);
  g_IECIgnorePart.ColumnType = clistIcon;
  g_IECIgnoreFull.cbSize = sizeof(IconExtraColumn);
  g_IECIgnoreFull.ColumnType = clistIcon;

  if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
  {
      g_IECIgnoreMess.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"ignore_mess"), (LPARAM)0);
      g_IECIgnorePart.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"ignore_part"), (LPARAM)0);
      g_IECIgnoreFull.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"ignore_full"), (LPARAM)0);
  }
  return 0;
}

static VOID init_icolib (void)
{
	SKINICONDESC sid = {0};
	ZeroMemory(&sid, sizeof(sid));
	TCHAR tszFile[MAX_PATH];
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;
	int i = 0;

	sid.ptszSection = _T(MODULENAME);
	GetModuleFileName(g_hInst, tszFile, MAX_PATH);
	sid.ptszDefaultFile = tszFile;

	for ( i = 0; i < SIZEOF(iconList); i++ ) {
		sid.pszName = iconList[i].szName;
		sid.ptszDescription =  iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = (HANDLE) CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
//	hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, onExtraImageListRebuild);
}

VOID fill_filter(void)
{
	bUseMirandaSettings = DBGetContactSettingByte(NULL, MODULENAME, "UseMirandaSettings", 0);

	currentFilter = bUseMirandaSettings ? DBGetContactSettingDword(NULL, "Ignore", "Default1", 0) : DBGetContactSettingDword(NULL, MODULENAME, "Filter", 0x8);

	int i = 0, cii = SIZEOF(ii);

	for (; i< cii; i++)
  {
	  if (checkState((ii[i].type)))
  		ii[i].filtered =  true;
  	else
  		ii[i].filtered =  false;
  }
}

int onModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	hOptInitialise = HookEvent(ME_OPT_INITIALISE, onOptInitialise);

	//IcoLib support
	init_icolib();
	fill_filter();

	hExtraIcon = ExtraIcon_Register("ignore", "IgnoreState", "ignore_full");

	if (hExtraIcon != NULL)
	{
		// Set initial value for all contacts
		HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact != NULL)
		{
			onExtraImageApplying((WPARAM)hContact, NULL);
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


	// Updater support
	// switch off
	if ( ServiceExists(MS_UPDATE_REGISTER))
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
		update.szBetaUpdateURL      = "http://kildor.miranda.im/miranda/ignore_state.zip";
		update.szBetaVersionURL     = "http://kildor.miranda.im/miranda/ignore_state.txt";
		update.szBetaChangelogURL   = "http://kildor.miranda.im/miranda/IgnoreState-readme.txt";
		update.pbBetaVersionPrefix  = (BYTE *)MODULENAME " ";
		update.cpbBetaVersionPrefix = lstrlenA((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}
	return 0;
}


int onContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	if (!lstrcmpA(cws->szModule,"Ignore") & !lstrcmpA(cws->szSetting,"Mask1")) {
		onExtraImageApplying(wParam, lParam);
	}
	if ((HANDLE)wParam ==0)
		if (!lstrcmpA(cws->szModule,MODULENAME) & !lstrcmpA(cws->szSetting,"Filter") ||
			bUseMirandaSettings && (!lstrcmpA(cws->szModule,"Ignore") & !lstrcmpA(cws->szSetting,"Default1"))
			) {
			fill_filter();
		}

	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);

	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	hSystemOKToExit = HookEvent(ME_SYSTEM_OKTOEXIT,onSystemOKToExit);
	hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);

	clistIcon = DBGetContactSettingByte(NULL, MODULENAME, "AdvancedIcon", DefaultSlot);

	g_IECClear.cbSize = sizeof(IconExtraColumn);
	g_IECClear.ColumnType = clistIcon;
	g_IECClear.hImage = (HANDLE) -1;

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
