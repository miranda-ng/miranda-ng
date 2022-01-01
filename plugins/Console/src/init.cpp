/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

uint32_t mirandaVer;
CMPlugin g_plugin;

static IconItem iconList[] =
{
	{ "Show", "Console_Up", IDI_BTN_UP },
	{ "Hide", "Console_Down", IDI_BTN_DN },
};

HANDLE LoadIcon(int iIconID)
{
	for (int i = 0; i < _countof(iconList); i++)
		if (iconList[i].defIconID == iIconID)
			return iconList[i].hIcolib;
	return nullptr;
}

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
	// {23D4F302-D513-45B7-9027-445F29557311}
{ 0x23d4f302, 0xd513, 0x45b7,{ 0x90, 0x27, 0x44, 0x5f, 0x29, 0x55, 0x73, 0x11 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	g_plugin.registerIcon(MODULENAME, iconList);

	InitCommonControls();
	InitConsole();
	return 0;
}

int CMPlugin::Unload()
{
	ShutdownConsole();

	if (hConsoleThread)
		WaitForSingleObject(hConsoleThread, INFINITE);

	return 0;
}
