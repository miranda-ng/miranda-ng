/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project, 
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
    sizeof(PLUGININFO),
	"Smart Auto Away NI",
#include "version.txt"
	0,
    DEFMOD_RNDIDLE,            // replace internal version (if any)
	{ 0x91bb7ad7,0x7ed8,0x40b9,{0xb1, 0x50, 0x3f, 0x37, 0x6b, 0x6b, 0x67, 0xfe}/* 91bb7ad7-7ed8-40b9-b150-3f376b6b67fe */}
};


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
static	char description[1023];
sprintf(char *, const char *, ...);
__declspec(dllexport) PLUGININFOEX *MirandaPluginInfo(DWORD mirandaVersion)
{
	sprintf(description,"%s","Just to suppress the build-in \"Idle\" module");
	if (mirandaVersion<0x00030400) {
		sprintf(description,"%s\r\n%s",description,"Miranda %d.%d.%d.%d has no build-in \"Idle\" support and this plugin is useless");
		sprintf(description,description, // Yes, I realize this is strange approach...
			mirandaVersion>>24&0xFF,
			mirandaVersion>>16&0xFF,
			mirandaVersion>>8&0xFF,
			mirandaVersion&0xFF
		);
	}
	sprintf(description,"%s\r\n[Build %s %s]",description, __DATE__,__TIME__);
	pluginInfo.description = description;
    return &pluginInfo;
}
__declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	//return 0;
	pluginInfo.cbSize = sizeof( PLUGININFOEX );
	return MirandaPluginInfo(mirandaVersion);
}
  	 
extern "C" __declspec(dllexport) const MUUID interfaces[] = {MIID_IDLE, MIID_LAST};
__declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 0;
}

int __declspec(dllexport) Unload(void)
{
//	AutoAwayShutdown(0,0);
    return 0; //if 1 we dont want to shutdown because we have hooked ME_SYSTEM_SHUTDOWN
}

