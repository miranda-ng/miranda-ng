/*
   Mobile State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2012-15 by Robert Pösel

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
int hLangpack;
HANDLE hExtraIcon = NULL;

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
	// {F0BA32D0-CD07-4A9C-926B-5A1FF21C3C10}
	{ 0xf0ba32d0, 0xcd07, 0x4a9c, { 0x92, 0x6b, 0x5a, 0x1f, 0xf2, 0x1c, 0x3c, 0x10 } }
};

static IconItem icon = { LPGEN("Mobile State"), "mobile_icon", IDI_MOBILE };

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

bool hasMobileClient(MCONTACT hContact, LPARAM)
{
	char *proto = GetContactProto(hContact);

	DBVARIANT dbv;
	if (!db_get_ts(hContact, proto, "MirVer", &dbv)) {
		TCHAR *client = _tcslwr(NEWTSTR_ALLOCA(dbv.ptszVal));
		db_free(&dbv);

		for (size_t i = 0; i < SIZEOF(clients); i++)
			if (_tcsstr(client, clients[i]))
				return true;
	}
	return false;
}

int ExtraIconsApply(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
		return 0;

	if (hasMobileClient(wParam, lParam))
		ExtraIcon_SetIcon(hExtraIcon, wParam, "mobile_icon");
	else
		ExtraIcon_Clear(hExtraIcon, wParam);

	return 0;
}

int onContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *proto = GetContactProto(wParam);
	if (!proto)
		return 0;

	if (!mir_strcmp(cws->szModule, proto))
		if (!mir_strcmp(cws->szSetting, "MirVer"))
			ExtraIconsApply(wParam, 1);

	return 0;
}

int onModulesLoaded(WPARAM, LPARAM)
{
	// Set initial value for all contacts
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		ExtraIconsApply(hContact, 1);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ExtraIconsApply);

	// IcoLib support
	Icon_Register(g_hInst, "Mobile State", &icon, 1);

	// Extra icons
	hExtraIcon = ExtraIcon_Register("mobilestate", LPGEN("Mobile State"), "mobile_icon");

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
