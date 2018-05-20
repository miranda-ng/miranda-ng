/*
Favorite Contacts for Miranda IM

Copyright 2007 Victor Pavlychko

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#include "http_api.h"

CMPlugin g_plugin;
int &hLangpack(g_plugin.m_hLang);

IconItem iconList[] =
{
	{ LPGEN("Favorite Contact"), "favcontacts_favorite", IDI_FAVORITE },
	{ LPGEN("Regular Contact"), "favcontacts_regular", IDI_REGULAR },
};

CContactCache *g_contactCache = nullptr;

Options g_Options = { 0 };

/////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {CE2C0401-F9E0-40D7-8E95-1A4197D7AB04}
	{0xce2c0401, 0xf9e0, 0x40d7, {0x8e, 0x95, 0x1a, 0x41, 0x97, 0xd7, 0xab, 0x4}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("FavContacts", pluginInfoEx)
{}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	g_contactCache = new CContactCache;

	InitMenu();
	InitServices();
	LoadOptions();

	/////////////////////////////////////////////////////////////////////////////////////

	g_plugin.registerIcon(LPGEN("Favorites"), iconList);

	LoadHttpApi();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitServices();
	UninitMenu();
	UnloadHttpApi();

	if (g_Options.hfntName) DeleteObject(g_Options.hfntName);
	if (g_Options.hfntSecond) DeleteObject(g_Options.hfntSecond);

	delete g_contactCache;
	return 0;
}
