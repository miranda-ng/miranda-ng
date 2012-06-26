/*==========================================================================*/
/*
    FILE DESCRIPTION: Rate main

    AUTHOR:    Kildor
    GROUP:     The NULL workgroup
    PROJECT:   Contact`s rate
    PART:      Main
    VERSION:   1.0
    CREATED:   20.12.2006 23:11:41

    EMAIL:     kostia@ngs.ru
    WWW:       http://kildor.miranda.im

    COPYRIGHT: (C) 2006 The NULL workgroup. All Rights Reserved.
*/
/*--------------------------------------------------------------------------*/
/*
    Copyright (C) 2006 The NULL workgroup

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
/*--------------------------------------------------------------------------*/
/*
    FILE ID: $Id$

    CHANGE LOG:

    $Log$
*/
#include "commonheaders.h"

HINSTANCE g_hInst;
PLUGINLINK *pluginLink;
static HANDLE hHookModulesLoaded = NULL, hSystemOKToExit = NULL, hOptInitialise = NULL, hIcoLibIconsChanged = NULL;
static HANDLE hHookExtraIconsRebuild = NULL, hHookExtraIconsApply = NULL, hContactSettingChanged = NULL;
static HANDLE hPrebuildContactMenu = NULL;
static HANDLE hExtraIcon = NULL;
IconExtraColumn g_IECRateHigh = {0};
IconExtraColumn g_IECRateMedium = {0};
IconExtraColumn g_IECRateLow = {0};
IconExtraColumn g_IECClear = {0};
int clistIcon = 0; //Icon slot to use
byte bRate = 0;
int hLangpack;

extern int onOptInitialise(WPARAM wParam, LPARAM lParam);


PLUGININFOEX pluginInfo={
   sizeof(PLUGININFOEX),
   "Contact`s Rate",
   PLUGIN_MAKE_VERSION(0,0,2,1),
   "Show rating of contact in contact list (if presents).",
   "Kildor, Thief",
   "kostia@ngs.ru",
   "© 2006-2009 Kostia Romanov, based on AuthState by Alexander Turyak",
   "http://kildor.miranda.im/",
   UNICODE_AWARE,      // is not unicode
   0,      //doesn't replace anything built-in
   {0x45230488, 0x977b, 0x405b, {0x85, 0x6d, 0xea, 0x27, 0x6d, 0x70, 0x83, 0xb7}}
/* 45230488-977b-405b-856d-ea276d7083b7 */

};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
   g_hInst = hinstDLL;
   return TRUE;
}

// плагининфо
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	static const MUUID interfaces[] = {MIID_CONTACTSRATE, MIID_LAST};
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

void setExtaIcon(HANDLE hContact, int bRate = -1, BOOL clear = TRUE)
{
	if (hContact == NULL)
		return;

	if (bRate < 0)
		bRate = DBGetContactSettingByte(hContact, "CList", "Rate", 0);

	if (hExtraIcon != NULL)
	{
		const char *icon;
		switch(bRate)
		{
			case 3: icon = "rate_high";  break;
			case 2: icon = "rate_medium";  break;
			case 1: icon = "rate_low";  break;
			default: icon = NULL;  break;
		}

		if (icon == NULL && !clear)
			return;

		ExtraIcon_SetIcon(hExtraIcon, hContact, icon);
	}
	else
	{
		switch(bRate)
		{
			case 3: CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &g_IECRateHigh); break;
			case 2: CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &g_IECRateMedium); break;
			case 1: CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &g_IECRateLow); break;
			default: CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &g_IECClear); break;
		}
	}
}


int onExtraImageApplying(WPARAM wParam, LPARAM lParam)
{
	setExtaIcon((HANDLE) wParam);
	return 0;
}


int onExtraImageListRebuild(WPARAM wParam, LPARAM lParam)
{
  g_IECRateHigh.cbSize = sizeof(IconExtraColumn);
  g_IECRateHigh.ColumnType = clistIcon;
  g_IECRateMedium.cbSize = sizeof(IconExtraColumn);
  g_IECRateMedium.ColumnType = clistIcon;
  g_IECRateLow.cbSize = sizeof(IconExtraColumn);
  g_IECRateLow.ColumnType = clistIcon;

  if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
  {
      g_IECRateHigh.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"rate_high"), (LPARAM)0);
      g_IECRateMedium.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"rate_medium"), (LPARAM)0);
      g_IECRateLow.hImage = (HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM)(HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM)"rate_low"), (LPARAM)0);
  }
  return 0;
}


static void init_icolib (void)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(g_hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.pszSection = Translate("Contact Rate");
	sid.pszDefaultFile = szFile;
	sid.flags = SIDF_PATH_TCHAR;

	for (int i = 0; i < SIZEOF(iconList); i++ ) {
		sid.pszName = iconList[i].szName;
		sid.pszDescription =  iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = Skin_AddIcon(&sid);
	}
}


int onModulesLoaded(WPARAM wParam,LPARAM lParam)
{
   //IcoLib support
   init_icolib();


   // Updater support
   // switch off
   if ( ServiceExists(MS_UPDATE_REGISTER))
   {
      Update update = {0};
      char szVersion[16];

      update.cbSize = sizeof(Update);

      update.szComponentName = pluginInfo.shortName;
      update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
      update.cpbVersion = (int)strlen((char *)update.pbVersion);

      update.szUpdateURL = UPDATER_AUTOREGISTER;

      // these are the three lines that matter - the archive, the page containing the version string, and the text (or data)
      // before the version that we use to locate it on the page
      // (note that if the update URL and the version URL point to standard file listing entries, the backend xml
      // data will be used to check for updates rather than the actual web page - this is not true for beta urls)
      update.szBetaUpdateURL      = "http://kildor.miranda.im/miranda/rate.zip";
      update.szBetaVersionURL     = "http://kildor.miranda.im/miranda/rate.txt";
      update.szBetaChangelogURL   = "http://kildor.miranda.im/miranda/rate_changes.txt";
      update.pbBetaVersionPrefix  = (BYTE *)"Rate ";
      update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

      CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
   }

	// Extra icon support
	hExtraIcon = ExtraIcon_Register("contact_rate", "Contact rate", "rate_high");

	if (hExtraIcon != NULL)
	{
		// Set initial value for all contacts
		HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact != NULL)
		{
			setExtaIcon(hContact, -1, FALSE);
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	}
	else
	{
		hOptInitialise = HookEvent(ME_OPT_INITIALISE, onOptInitialise);
		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, onExtraImageListRebuild);
		hHookExtraIconsRebuild = HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, onExtraImageListRebuild);
		hHookExtraIconsApply = HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
		onExtraImageListRebuild(0,0);
	}

	return 0;
}


int onContactSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;

	if (wParam != NULL && !lstrcmp(cws->szModule,"CList") && !lstrcmp(cws->szSetting,"Rate"))
		setExtaIcon((HANDLE)wParam, cws->value.type == DBVT_DELETED ? 0 : cws->value.bVal);

	return 0;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getLP(&pluginInfo);
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
