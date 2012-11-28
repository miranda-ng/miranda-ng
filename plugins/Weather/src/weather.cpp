/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

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




// plugin info
// VER = version, AUTH = author, defined in weather.h
static const PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	"Weather Protocol",
	__VERSION_DWORD,
	"Retrieves weather information and displays it in your contact list.",
	AUTH,
	"borkra@miranda-im.org",
	"(c) 2002-2005 NoName, 2005-2010 Boris Krasnovskiy",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	MIID_WEATHER
};

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfoEx;
}

// MirandaInterfaces - returns the protocol interface to the core
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
{
	hInst = hinstDLL;
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
		db_unset(NULL, WEATHERPROTONAME, "TriggerText");
	if (lastver < PLUGIN_MAKE_VERSION(0,3,3,13))
		db_unset(NULL, "KnownModules", "Weather");

	db_set_dw(NULL, WEATHERPROTONAME, "Version", __VERSION_DWORD);
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
	timerId = SetTimer(NULL, 0, 5000, timerProc2);  // first update is 5 sec after load

	// weather user detail
	HookEvent(ME_USERINFO_INITIALISE, UserInfoInit);

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
		db_set_dw(hContact, "Ignore", "Mask", 8);
		db_set_dw(hContact, "Ignore", "Mask1", 8);
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
	timerId = 0;
	opt.DefStn = NULL;
	ModuleLoaded = FALSE;
}

// unload function
extern "C" int __declspec(dllexport) Unload(void) 
{
	DestroyMwin();
	DestroyWindow(hPopupWindow);

	DestroyHookableEvent(hHookWeatherUpdated);
	DestroyHookableEvent(hHookWeatherError);

	NetlibHttpDisconnect();
	Netlib_CloseHandle(hNetlibUser);

	DestroyUpdateList();
	DestroyOptions();
	DestroyWIList();				// unload all ini data from memory

	CloseHandle(hUpdateMutex);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void) 
{
	DWORD lastver;

	mir_getLP(&pluginInfoEx);

	// initialize global variables
	InitVar();

	// load options and set defaults
	LoadOptions();

	// upgrade check
	// I only support version check and upgrade for my own version, so check if the author is my name
	if (strstr(AUTH, "NoName") != NULL) 
	{
		lastver = db_get_dw(NULL, WEATHERPROTONAME, "Version", PLUGIN_MAKE_VERSION(0,3,1,8));
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
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, WeatherInit);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_CLIST_DOUBLECLICKED, BriefInfo);
	HookEvent(ME_WEATHER_UPDATED, WeatherPopup);
	HookEvent(ME_WEATHER_ERROR, WeatherError);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, WeatherShutdown);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildContactMenu);

	hUpdateMutex = CreateMutex(NULL, FALSE, NULL);

	// register weather protocol
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = WEATHERPROTONAME;
	pd.type = (opt.NoProtoCondition) ? PROTOTYPE_VIRTUAL : PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0, (LPARAM)&pd);

	// initialize weather protocol services
	InitServices();

	// add our modules to the KnownModules list 
	db_set_s(NULL, "KnownModules", "Weather Protocol", "Weather,WeatherCondition,Current");

	// add sound event
	SkinAddNewSoundExT("weatherupdated", _T(WEATHERPROTONAME), LPGENT("Weather Condition Changed"));
	SkinAddNewSoundExT("weatheralert", _T(WEATHERPROTONAME), LPGENT("Weather Alert Issued"));

	// window needed for popup commands
	TCHAR SvcFunc[100];
	mir_sntprintf( SvcFunc, SIZEOF(SvcFunc), _T("%s__PopupWindow"), _T(WEATHERPROTONAME));
	hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW,_T("static"),SvcFunc,0,CW_USEDEFAULT,CW_USEDEFAULT,
		CW_USEDEFAULT,CW_USEDEFAULT,HWND_DESKTOP,NULL, hInst,NULL);
	SetWindowLongPtr(hPopupWindow, GWLP_WNDPROC, (LONG_PTR)PopupWndProc);

	return 0; 
}
