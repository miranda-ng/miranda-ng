/*
Weather Protocol plugin for Miranda IM
Copyright (C) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Main file for the Weather Protocol, includes loading, unloading,
upgrading, support for plugin uninsaller, and anything that doesn't
belong to any other file.
*/

#include "weather.h"

//============  GLOBAL VARIABLES  ============

WIDATALIST *WIHead;
WIDATALIST *WITail;

HINSTANCE hInst;
HWND hPopupWindow;

HANDLE hHookWeatherUpdated;
HANDLE hHookWeatherError;

static HANDLE hHooks[9];

HANDLE hDataWindowList;
HANDLE hWindowList;

HANDLE hUpdateMutex;

unsigned status;
unsigned old_status;

UINT_PTR timerId;
int hLangpack;

MYOPTIONS opt;

// check if weather is currently updating
BOOL ThreadRunning;

// variable to determine if module loaded
BOOL ModuleLoaded;

struct MM_INTERFACE   mmi;
struct UTF8_INTERFACE utfi;


PLUGINLINK *pluginLink;

// plugin info
// VER = version, AUTH = author, defined in weather.h
static const PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
#ifdef _WIN64
	"Weather Protocol x64",
#else
	"Weather Protocol",
#endif
	__VERSION_DWORD,
	"Retrieve weather information and display them in your contact list",
	AUTH,
	"borkra@miranda-im.org",
	"(c) 2002-2005 NoName, 2005-2010 Boris Krasnovskiy",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=2322",
	UNICODE_AWARE,
	0,
	MIID_WEATHER
};

__declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0,8,0,0)) {
		MessageBox(NULL, "Weather Protocol requires Miranda 0.8.0.0 or later to run.", "Weather Protocol", MB_OK|MB_ICONERROR|MB_APPLMODAL);
		return NULL;
	}
	// if Miranda version is higher than 0.7.0
	else	
		return &pluginInfoEx;
}

// MirandaPluginInterfaces - returns the protocol interface to the core
static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
		break;
	}

	return TRUE;
}


int WeatherShutdown(WPARAM wParam,LPARAM lParam) 
{
	KillTimer(NULL, timerId);		// kill update timer

	SaveOptions();					// save options once more
	status = ID_STATUS_OFFLINE;		// set status to offline

	CallService(MS_NETLIB_SHUTDOWN, (WPARAM)hNetlibHttp, 0);

	WindowList_Broadcast(hWindowList, WM_CLOSE, 0, 0);
	WindowList_Broadcast(hDataWindowList, WM_CLOSE, 0, 0);
	SendMessage(hWndSetup, WM_CLOSE, 0, 0);

	return 0;
}

// update some settings/db values for new version
// lastver = dword value for the last version made by PLUGIN_MAKE_VERSION
void Upgrade(DWORD lastver)
{
	// for version below v0.3.2.3, remove the "TriggerText" setting
	if (lastver < PLUGIN_MAKE_VERSION(0,3,2,3))
		DBDeleteContactSetting(NULL, WEATHERPROTONAME, "TriggerText");
	if (lastver < PLUGIN_MAKE_VERSION(0,3,3,13))
		DBDeleteContactSetting(NULL, "KnownModules", "Weather");

	DBWriteContactSettingDword(NULL, WEATHERPROTONAME, "Version", __VERSION_DWORD);
}

// weather protocol initialization function
// run after the event ME_SYSTEM_MODULESLOADED occurs
int WeatherInit(WPARAM wParam,LPARAM lParam) 
{
	// initialize netlib
	NetlibInit();

	InitIcons();
	InitMwin();

	// load weather menu items
	AddMenuItems();

	// timer for the first update
	timerId = SetTimer(NULL, 0, 5000, (TIMERPROC)timerProc2);  // first update is 5 sec after load

	// weather user detail
	hHooks[0] = HookEvent(ME_USERINFO_INITIALISE, UserInfoInit);

	hDataWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);

	return 0;
}

// update some settings/db values for new version, this one is for contact
// lastver = dword value for the last version made by PLUGIN_MAKE_VERSION
// hContact = current contact
void UpgradeContact(DWORD lastver, HANDLE hContact) 
{
	// for version below v0.3.2.3, suppress online notifications for all weather contacts
	if (lastver < PLUGIN_MAKE_VERSION(0,3,2,3)) 
	{
		DBWriteContactSettingDword(hContact, "Ignore", "Mask", 8);
		DBWriteContactSettingDword(hContact, "Ignore", "Mask1", 8);
	}
}

//============  MISC FUNCTIONS  ============

// initialize the global variables at startup
void InitVar() 
{
	// setup the linklist for weather update list
	UpdateListTail = NULL;
	UpdateListHead = NULL;

	// other settings
	timerId=0;
	opt.DefStn = NULL;
	ModuleLoaded = FALSE;
}

// unload function
int __declspec(dllexport) Unload(void) 
{
	unsigned i;

	DestroyMwin();
	DestroyWindow(hPopupWindow);

	for (i = sizeof(hHooks)/sizeof(HANDLE); i--; )
		UnhookEvent(hHooks[i]);

	DestroyHookableEvent(hHookWeatherUpdated);
	DestroyHookableEvent(hHookWeatherError);

	DestroyServices();

	NetlibHttpDisconnect();
	Netlib_CloseHandle(hNetlibUser);

	DestroyUpdateList();
	DestroyOptions();
	DestroyWIList();				// unload all ini data from memory

	CloseHandle(hUpdateMutex);

	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link) 
{
	PROTOCOLDESCRIPTOR pd = {0};
	char SvcFunc[100];
	DWORD lastver;

	pluginLink = link;

	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLP(&pluginInfoEx);

	// initialize global variables
	InitVar();
	InitUniConv();

	// load options and set defaults
	LoadOptions();

	// upgrade check
	// I only support version check and upgrade for my own version, so check if the author is my name
	if (strstr(AUTH, "NoName") != NULL) 
	{
		lastver = DBGetContactSettingDword(NULL, WEATHERPROTONAME, "Version", PLUGIN_MAKE_VERSION(0,3,1,8));
		if (lastver < __VERSION_DWORD)	Upgrade(lastver);
	}
	else	// if it is not my build, ignore upgrade procedure
		lastver = PLUGIN_MAKE_VERSION(255,255,255,255);

	// reset the weather data at startup for individual contacts
	EraseAllInfo(lastver);

	// load weather update data
	LoadWIData(TRUE);

	// set status to online if "Do not display weather condition as protocol status" is enabled
	old_status = status = ID_STATUS_OFFLINE;

	// add an event on weather update and error
	hHookWeatherUpdated = CreateHookableEvent(ME_WEATHER_UPDATED);
	hHookWeatherError = CreateHookableEvent(ME_WEATHER_ERROR);

	// initialize options and network
	hHooks[1] = HookEvent(ME_OPT_INITIALISE, OptInit);
	hHooks[2] = HookEvent(ME_SYSTEM_MODULESLOADED, WeatherInit);
	hHooks[3] = HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	hHooks[4] = HookEvent(ME_CLIST_DOUBLECLICKED, BriefInfo);
	hHooks[5] = HookEvent(ME_WEATHER_UPDATED, WeatherPopup);
	hHooks[6] = HookEvent(ME_WEATHER_ERROR, WeatherError);
	hHooks[7] = HookEvent(ME_SYSTEM_PRESHUTDOWN, WeatherShutdown);
	hHooks[8] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildContactMenu);

	hUpdateMutex = CreateMutex(NULL, FALSE, NULL);

	// register weather protocol
	pd.cbSize = sizeof(pd);
	pd.szName = WEATHERPROTONAME;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	// initialize weather protocol services
	InitServices();

	// add our modules to the KnownModules list 
	DBWriteContactSettingString(NULL, "KnownModules", "Weather Protocol", "Weather,WeatherCondition,Current");

	// add sound event
	SkinAddNewSound("weatherupdated", Translate("Weather Condition Changed"), "");
	SkinAddNewSound("weatheralert", Translate("Weather Alert Issued"), "");

	// window needed for popup commands
	strcpy(SvcFunc, WEATHERPROTONAME);
	strcat(SvcFunc, "_PopupWindow");
	hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW,"static",SvcFunc,0,CW_USEDEFAULT,CW_USEDEFAULT,
		CW_USEDEFAULT,CW_USEDEFAULT,HWND_DESKTOP,NULL,hInst,NULL);
	SetWindowLongPtr(hPopupWindow, GWLP_WNDPROC, (LONG_PTR)PopupWndProc);

	return 0; 
}
