/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#include "commonheaders.h"

PLUGINLINK *pluginLink;
DWORD mirandaVer;


PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	__VERSION_DWORD,
	__PLUGIN_LONGDESC,
	__PLUGIN_AUTHOR,
	__PLUGIN_EMAIL,
	__PLUGIN_RIGHTS,
	__PLUGIN_URL,
	UNICODE_AWARE,
	0,
	MIID_CONSOLE
};


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	mirandaVer = mirandaVersion;
	return &pluginInfoEx;
}

// we implement service mode interface
static const MUUID interfaces[] = {MIID_LOGWINDOW, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

////////////////////////////////////////////////////////////////////////////////

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	if (mirandaVer < PLUGIN_MAKE_VERSION(0,8,0,3))
		return -1;
	else
	{
		InitCommonControls();
		InitConsole();
		return 0;
	}
}


int __declspec(dllexport) Unload(void)
{
    ShutdownConsole();
	return 0;
}
