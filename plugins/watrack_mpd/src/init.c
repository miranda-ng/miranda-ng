// Copyright © 2009-2010 sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"


#define PLUGIN_NAME	"Watrack_MPD"

HINSTANCE hInst;
BOOL bWatrackService = FALSE;
int hLangpack = 0;
static int OnModulesLoaded(WPARAM wParam,LPARAM lParam);
extern char *date();
extern int WaMpdOptInit(WPARAM wParam,LPARAM lParam);


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	0,
	PLUGIN_MAKE_VERSION(0,0,0,4),
	"Music Player Daemon support for watrack",
	"sss, others..",
	"sss123next@list.ru",
	"© 2009 sss, others...",
	"http://sss.chaoslab.ru:81/tracker/mim_plugs/",
	1,		//unicode
    { 0x692e87d0, 0x6c71, 0x4cdc, { 0x9e, 0x36, 0x2b, 0x2d, 0x69, 0xfb, 0xdc, 0x4c } }

};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	{
	  static char plugname[52];
	  strcpy(plugname, PLUGIN_NAME" [");
	  strcat(plugname, date());
	  strcat(plugname, " ");
	  strcat(plugname, __TIME__);
	  strcat(plugname, "]");
	  pluginInfo.shortName = plugname;
	}
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_SERVICEMODE, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
}

void InitVars()
{
	gbPort = DBGetContactSettingWord(NULL, szModuleName, "Port", 6600);
	gbPassword = (TCHAR*)malloc(64*sizeof(TCHAR));
	gbHost = (TCHAR*)malloc(128*sizeof(TCHAR));
	gbHost = UniGetContactSettingUtf(NULL, szModuleName, "Server", _T("127.0.0.1"));
	gbPassword = UniGetContactSettingUtf(NULL, szModuleName, "Password", _T(""));
}


extern void RegisterPlayer();
static int OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	HANDLE hHookOptionInit;
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = (NUF_OUTGOING | NUF_HTTPCONNS);
	nlu.szDescriptiveName = "Watrack MPD connection";
	nlu.szSettingsModule = PLUGIN_NAME;
	ghNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	InitVars();
	hHookOptionInit = HookEvent(ME_OPT_INITIALISE, WaMpdOptInit);
	if (ServiceExists("WATrack/Player"))
		bWatrackService = TRUE;
	RegisterPlayer();

	return 0;
}


int __declspec(dllexport) Unload(void)
{
	free(gbHost);
	free(gbPassword);
	return 0;
}
