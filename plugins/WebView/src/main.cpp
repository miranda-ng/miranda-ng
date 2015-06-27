/*
 * A plugin for Miranda IM which displays web page text in a window.
 * Copyright (C) 2005 Vincent Joyce.
 * 
 * Miranda IM: the free icq client for MS Windows 
 * Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "stdafx.h"
#include "webview.h"

MWindowList hWindowList;
HANDLE hNetlibUser;
HANDLE hHookDisplayDataAlert, hHookAlertPopup, hHookAlertWPopup, hHookErrorPopup, hHookAlertOSD;

int    hLangpack = 0;
CLIST_INTERFACE *pcli;
static HMODULE hRichEd = NULL;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {CD5427FB-5320-4f65-B4BF-86B7CF7B5087}
	{0xcd5427fb, 0x5320, 0x4f65, {0xb4, 0xbf, 0x86, 0xb7, 0xcf, 0x7b, 0x50, 0x87}}
};

/*****************************************************************************/
void InitServices()
{
	CreateProtoServiceFunction(MODULENAME, PS_GETCAPS, GetCaps);
	CreateProtoServiceFunction(MODULENAME, PS_GETNAME, GetName);
	CreateProtoServiceFunction(MODULENAME, PS_LOADICON, BPLoadIcon);
	CreateProtoServiceFunction(MODULENAME, PS_SETSTATUS, SetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_GETSTATUS, GetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_BASICSEARCH, BasicSearch);
	CreateProtoServiceFunction(MODULENAME, PS_ADDTOLIST, AddToList);
	CreateProtoServiceFunction(MODULENAME, PSS_GETINFO, GetInfo);
}

/*****************************************************************************/
void ChangeContactStatus(int con_stat)
{
   WORD status_code = 0;
   if (con_stat == 0)
      status_code = ID_STATUS_OFFLINE;
   if (con_stat == 1)
      status_code = ID_STATUS_ONLINE;
   if (con_stat == 2)
      status_code = ID_STATUS_AWAY;
   if (con_stat == 3)
      status_code = ID_STATUS_NA;

	for (MCONTACT hContact = db_find_first(MODULENAME); hContact != NULL; hContact = db_find_next(hContact, MODULENAME))
		db_set_w(hContact, MODULENAME, "Status", status_code);
}

/*****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   hInst = hinstDLL;
   return TRUE;
}

/*****************************************************************************/
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

/*****************************************************************************/
extern "C" int __declspec(dllexport) Unload(void)
{
   ChangeContactStatus(0);

   KillTimer(NULL, timerId);
   KillTimer(NULL, Countdown);

   db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 0);
   SavewinSettings();
   if (hRichEd)
		FreeLibrary(hRichEd);

	if (hNetlibUser) {
		Netlib_CloseHandle(hNetlibUser);
		hNetlibUser = NULL;
	}
	
	if (hHookDisplayDataAlert)
		DestroyHookableEvent(hHookDisplayDataAlert);
	if (hHookAlertPopup)
		DestroyHookableEvent(hHookAlertPopup);
	if (hHookAlertWPopup)
		DestroyHookableEvent(hHookAlertWPopup);
	
	if (h_font != NULL)
		DeleteObject(h_font);
	if (hMenu)
		DestroyMenu(hMenu);    
	WindowList_Destroy(hWindowList);
	return 0;
}

/*****************************************************************************/
extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

	HookEvent(ME_CLIST_DOUBLECLICKED, Doubleclick);
   
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	hRichEd = LoadLibrary(_T("Msftedit.dll"));

	/*TIMERS*/
	if ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, TIME) != 0)) {  
		timerId = SetTimer(NULL, 0, ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, TIME)) * MINUTE), timerfunc);
		db_set_dw(NULL, MODULENAME, COUNTDOWN_KEY, 0); 
		Countdown = SetTimer(NULL, 0, MINUTE, Countdownfunc);
	}

	InitialiseGlobals();

	// register netlib handle
   char tempNdesc[50];
	mir_snprintf(tempNdesc, _countof(tempNdesc), "%s connection settings", MODULENAME);

	NETLIBUSER nlu = { sizeof(nlu) };
   nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
   nlu.szSettingsModule = MODULENAME;
   nlu.szDescriptiveName = tempNdesc;
   hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) & nlu);

	// register webview protocol
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULENAME;
	pd.type = PROTOTYPE_PROTOCOL;
	Proto_RegisterModule(&pd);

	//protocol services
	InitServices();

	//add sound event to options
	SkinAddNewSoundExT("webviewalert", _T(MODULENAME), LPGENT("Alert event"));

	//value is 1 if menu is disabled
	db_set_b(NULL, MODULENAME, MENU_IS_DISABLED_KEY, 1);

	CLISTMENUITEM mi = { 0 };
	mi.flags = CMIF_TCHAR;
	if ( db_get_b(NULL, MODULENAME, MENU_OFF, 0)) {
		//value is 0 if menu is enabled
		db_set_b(NULL, MODULENAME, MENU_IS_DISABLED_KEY, 0);

		/*DISABLE WEBVIEW*/
		CreateServiceFunction("DisableWebview", AutoUpdateMCmd);

		mi.hParentMenu = Menu_CreateRoot(MO_MAIN, _T(MODULENAME), 20200001);
		mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
		if (db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0))
			mi.name.t = LPGENT("Auto update disabled");
		else
			mi.name.t = LPGENT("Auto update enabled"); 

		mi.pszService = "DisableWebview";
		hMenuItem1 = Menu_AddMainMenuItem(&mi);

		// Update all webview contacts
		CreateServiceFunction("UpdateAll", UpdateAllMenuCommand);
		mi.position = 500090002;
		mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
		mi.name.t = LPGENT("Update all Webview sites");
		mi.pszService = "UpdateAll";
		Menu_AddMainMenuItem(&mi);

		// Mark All Webview Sites As Read
		CreateServiceFunction("MarkAllSitesRead", MarkAllReadMenuCommand);
		mi.position = 500090099;
		mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MARKALLREAD));
		mi.name.t = LPGENT("Mark all Webview sites as read");
		mi.pszService = "MarkAllSitesRead";
		Menu_AddMainMenuItem(&mi);

		// open cache directory
		CreateServiceFunction("OpenCacheFolder", OpenCacheDir);
		mi.position = 500090099;
		mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOLDER));
		mi.name.t = LPGENT("Open cache folder");
		mi.pszService = "OpenCacheFolder";
		Menu_AddMainMenuItem(&mi);

		// Countdown test
		CreateServiceFunction("Countdown", CountdownMenuCommand);
		mi.flags |= CMIF_KEEPUNTRANSLATED;
		TCHAR countername[100];
		mir_sntprintf(countername, _countof(countername), TranslateT("%d minutes to update"), db_get_dw(NULL, MODULENAME, COUNTDOWN_KEY, 0));
		mi.position = 600090099;
		mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
		mi.name.t = countername;
		mi.pszService = "Countdown";
		hMenuItemCountdown = Menu_AddMainMenuItem(&mi);
	}

	// contact menu
	mi.flags = CMIF_TCHAR;

	CreateServiceFunction("Open web page", WebsiteMenuCommand);
	mi.position = 100;
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_URL));
	mi.pszService = "Open web page";
	mi.name.t = LPGENT("Open web page");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	CreateServiceFunction("OpenClose Window", DataWndMenuCommand);
	mi.pszService = "OpenClose Window";
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SHOW_HIDE));
	mi.name.t = LPGENT("Open/Close window");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	mi.position = 2222220;
	mi.pszService = "UpdateData";
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATE));
	mi.name.t = LPGENT("Update data");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	CreateServiceFunction("ContactOptions", CntOptionsMenuCommand);
	mi.pszService = "ContactOptions";
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_OPTIONS));
	mi.name.t = LPGENT("Contact options");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	CreateServiceFunction("ContactAlertOpts", CntAlertMenuCommand);
	mi.pszService = "ContactAlertOpts";
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ALERT));
	mi.name.t = LPGENT("Contact alert options");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	CreateServiceFunction("PingWebsite", PingWebsiteMenuCommand);
	mi.pszService = "PingWebsite";
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PING));
	mi.name.t = LPGENT("Ping web site");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	CreateServiceFunction("StopDataProcessing", StpPrcssMenuCommand);
	mi.pszService = "StopDataProcessing";
	mi.icolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_STOP));
	mi.name.t = LPGENT("Stop data processing");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	hWindowList = WindowList_Create();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, SiteDeleted);

	db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 1);
	return 0;
}
