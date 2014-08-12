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

HANDLE hIcoLibIconsChanged = NULL;
HANDLE hHookExtraIconsRebuild = NULL, hHookExtraIconsApply = NULL, hContactSettingChanged = NULL;
HANDLE hPrebuildContactMenu = NULL;
HANDLE hExtraIcon = NULL;
int hLangpack;

INT currentFilter = 0;

INT clistIcon = 0; //Icon slot to use

IGNOREITEMS ii[] = {
	{ LPGENT("All"),            IGNOREEVENT_ALL,           SKINICON_OTHER_FILLEDBLOB },
	{ LPGENT("Messages"),       IGNOREEVENT_MESSAGE,       SKINICON_EVENT_MESSAGE    },
	{ LPGENT("URL"),            IGNOREEVENT_URL,           SKINICON_EVENT_URL        },
	{ LPGENT("Files"),          IGNOREEVENT_FILE,          SKINICON_EVENT_FILE       },
	{ LPGENT("User Online"),    IGNOREEVENT_USERONLINE,    SKINICON_OTHER_USERONLINE },
	{ LPGENT("Authorization"),  IGNOREEVENT_AUTHORIZATION, SKINICON_OTHER_MIRANDA    },
	{ LPGENT("You Were Added"), IGNOREEVENT_YOUWEREADDED,  SKINICON_OTHER_ADDCONTACT },
	{ LPGENT("Typing Notify"),  IGNOREEVENT_TYPINGNOTIFY,  SKINICON_OTHER_TYPING     }
};

int nII = SIZEOF(ii);

/////////////////////////////////////////////////////////////////////////////////////////

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
	// A6872BCD-F2A1-41B8-B2F1-DD7CEC055734
	{0xa6872bcd, 0xf2a1, 0x41b8, {0xb2, 0xf1, 0xdd, 0x7c, 0xec, 0x05, 0x57, 0x34}}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
   g_hInst = hinstDLL;
   return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline BOOL checkState(int type)
{
	return ((currentFilter>>(type-1))&1);
}

INT_PTR isIgnored(MCONTACT hContact, int type)
{
	int all = 0, filtered = 0;

	if (type != IGNOREEVENT_ALL)
		return CallService(MS_IGNORE_ISIGNORED, hContact, (LPARAM)type);

	for (int i = 1; i < nII; i++)
		if (isIgnored(hContact, ii[i].type))
			ii[i].filtered ? filtered++ : all++;

	return (all+filtered == SIZEOF(ii)-1) ? 1 : (all > 0 ? -1 : 0) ;
}

void applyExtraImage(MCONTACT hContact)
{
	int ignore = isIgnored(hContact, IGNOREEVENT_ALL);
	if (ignore == 1)
		ExtraIcon_SetIcon(hExtraIcon, hContact, "ignore_full");
	else if (ignore == 0)
		ExtraIcon_Clear(hExtraIcon, hContact);
	else if (isIgnored(hContact, IGNOREEVENT_MESSAGE))
		ExtraIcon_SetIcon(hExtraIcon, hContact, "ignore_mess");
	else
		ExtraIcon_SetIcon(hExtraIcon, hContact, "ignore_part");
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Full Ignore"),    "ignore_full", IDI_IFULL },
	{ LPGEN("Partial Ignore"), "ignore_part", IDI_IPART },
	{ LPGEN("Message Ignore"), "ignore_mess", IDI_IMESS },
};

VOID fill_filter(void)
{
	bUseMirandaSettings = db_get_b(NULL, MODULENAME, "UseMirandaSettings", 0);

	currentFilter = bUseMirandaSettings ? db_get_dw(NULL, "Ignore", "Default1", 0) : db_get_dw(NULL, MODULENAME, "Filter", 0x8);

	for (int i=0; i < SIZEOF(ii); i++) {
		if (checkState((ii[i].type)))
			ii[i].filtered =  true;
		else
			ii[i].filtered =  false;
	}
}

int onModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	HookEvent(ME_OPT_INITIALISE, onOptInitialise);

	fill_filter();

	// Set initial value for all contacts
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		applyExtraImage(hContact);

	return 0;
}

int onContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	if ( !lstrcmpA(cws->szModule, "Ignore") && !lstrcmpA(cws->szSetting, "Mask1"))
		applyExtraImage(hContact);
	else if (hContact == 0) {
		if (( !lstrcmpA(cws->szModule, MODULENAME) && !lstrcmpA(cws->szSetting, "Filter")) ||
			(bUseMirandaSettings && !lstrcmpA(cws->szModule, "Ignore") && !lstrcmpA(cws->szSetting, "Default1")))
		{
			fill_filter();
		}
	}

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);

	//IcoLib support
	Icon_Register(g_hInst, LPGEN("Ignore State"), iconList, SIZEOF(iconList));

	hExtraIcon = ExtraIcon_Register("ignore", LPGEN("Ignore State"), "ignore_full");

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
