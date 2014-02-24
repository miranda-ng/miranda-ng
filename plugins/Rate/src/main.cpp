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

#include "commonheaders.h"

HINSTANCE g_hInst;

static HANDLE hExtraIcon = NULL;
byte bRate = 0;
int hLangpack;

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
	// {45230488-977B-405B-856D-EA276D7083B7}
   {0x45230488, 0x977b, 0x405b, {0x85, 0x6d, 0xea, 0x27, 0x6d, 0x70, 0x83, 0xb7}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   g_hInst = hinstDLL;
   return TRUE;
}

// плагининфо
extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

///////////////////////////////////////////////////////////////////////////////

static void setExtraIcon(MCONTACT hContact, int bRate = -1, BOOL clear = TRUE)
{
	if (hContact == NULL)
		return;

	if (bRate < 0)
		bRate = db_get_b(hContact, "CList", "Rate", 0);

	const char *icon = NULL;
	switch(bRate) {
		case 1: icon = "rate_low";  break;
		case 2: icon = "rate_medium";  break;
		case 3: icon = "rate_high";  break;
	}

	if (icon != NULL || clear)
		ExtraIcon_SetIcon(hExtraIcon, hContact, icon);
}

///////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Rate high"),   "rate_high",   IDI_RATEHI },
	{ LPGEN("Rate medium"), "rate_medium", IDI_RATEME },
	{ LPGEN("Rate low"),    "rate_low",    IDI_RATELO },
};

int onModulesLoaded(WPARAM wParam,LPARAM lParam)
{
   // IcoLib support
	Icon_Register(g_hInst, LPGEN("Contact rate"), iconList, SIZEOF(iconList));

	// Extra icon support
	hExtraIcon = ExtraIcon_Register("contact_rate", LPGEN("Contact rate"), "rate_high");

	// Set initial value for all contacts
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		setExtraIcon(hContact, -1, FALSE);

	return 0;
}

int onContactSettingChanged(WPARAM hContact,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;

	if (hContact != NULL && !lstrcmpA(cws->szModule,"CList") && !lstrcmpA(cws->szSetting,"Rate"))
		setExtraIcon(hContact, cws->value.type == DBVT_DELETED ? 0 : cws->value.bVal);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

   HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
   HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
   return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
   return 0;
}
