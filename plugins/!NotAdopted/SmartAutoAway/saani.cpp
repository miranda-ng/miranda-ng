/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-06 Miranda ICQ/IM project,
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

//#ifdef VCL_VC60
#include "AggressiveOptimize.h"
//#endif
#include <windows.h>
#include <newpluginapi.h>

PLUGININFOEX pluginInfo = {
    sizeof(PLUGININFOEX),
	"Smart Auto Away NI",
    PLUGIN_MAKE_VERSION(0, 0, 3, 3),
    "Just to suppress the build-in \"Idle\" module",
    "YB",
    "y_b" /* these */ "@" /* fucking spammers */ "saaplugin.no-ip.info" /* gather everything */,
    "© 2005-2009 YB; 2000-2006 Miranda-IM",
    "http://saaplugin.no-ip.info/",
	0,
	// 91bb7ad7-7ed8-40b9-b150-3f376b6b67fe
	{ 0x91bb7ad7,0x7ed8,0x40b9,{0xb1, 0x50, 0x3f, 0x37, 0x6b, 0x6b, 0x67, 0xfe}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	//return 0;
	pluginInfo.cbSize = sizeof( PLUGININFOEX );
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_IDLE, MIID_LAST};

extern "C" int __declspec(dllexport) Load()
{
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
//	AutoAwayShutdown(0,0);
    return 0; //if 1 we dont want to shutdown because we have hooked ME_SYSTEM_SHUTDOWN
}
