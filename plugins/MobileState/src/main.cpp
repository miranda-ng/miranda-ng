/*
   Mobile State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2012-17 by Robert Pösel, 2017-22 Miranda NG team

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

CMPlugin g_plugin;

HANDLE hExtraIcon = nullptr;

static IconItem iconList[] = 
{
	{ LPGEN("Mobile State"), "mobile_icon", IDI_MOBILE }
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
	// {F0BA32D0-CD07-4A9C-926B-5A1FF21C3C10}
	{ 0xf0ba32d0, 0xcd07, 0x4a9c, { 0x92, 0x6b, 0x5a, 0x1f, 0xf2, 0x1c, 0x3c, 0x10 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("MobileState", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

bool hasMobileClient(MCONTACT hContact, LPARAM)
{
	char *proto = Proto_GetBaseAccountName(hContact);

	ptrW client(db_get_wsa(hContact, proto, "MirVer"));
	if (client) {
		// Make client lower-case
		wcslwr(client);
		
		// Find whether this client contain something from clients list
		for (size_t i = 0; i < _countof(clients); i++)
			if (wcsstr(client, clients[i]))
				return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ExtraIconsApply(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
		return 0;

	if (hasMobileClient(wParam, lParam))
		ExtraIcon_SetIconByName(hExtraIcon, wParam, "mobile_icon");
	else
		ExtraIcon_Clear(hExtraIcon, wParam);

	return 0;
}

static int onContactSettingChanged(WPARAM wParam, LPARAM lParam)
{	
	char *proto = Proto_GetBaseAccountName(wParam);
	if (!proto)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(cws->szModule, proto) && !strcmp(cws->szSetting, "MirVer"))
		ExtraIconsApply(wParam, 1);

	return 0;
}

static int onModulesLoaded(WPARAM, LPARAM)
{
	// Set initial value for all contacts
	for (auto &hContact : Contacts())
		ExtraIconsApply(hContact, 1);

	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ExtraIconsApply);

	// IcoLib support
	g_plugin.registerIcon("Mobile State", iconList);

	// Extra icons
	hExtraIcon = ExtraIcon_RegisterIcolib("mobilestate", LPGEN("Mobile State"), "mobile_icon");
	return 0;
}
