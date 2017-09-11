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
HNETLIBUSER hNetlibUser;
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
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
   hInst = hinstDLL;
   return TRUE;
}

/*****************************************************************************/
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
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
	pcli = Clist_GetInterface();

	HookEvent(ME_CLIST_DOUBLECLICKED, Doubleclick);
   
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	hRichEd = LoadLibrary(L"Msftedit.dll");

	/*TIMERS*/
	if ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, TIME) != 0)) {  
		timerId = SetTimer(NULL, 0, ((db_get_dw(NULL, MODULENAME, REFRESH_KEY, TIME)) * MINUTE), timerfunc);
		db_set_dw(NULL, MODULENAME, COUNTDOWN_KEY, 0); 
		Countdown = SetTimer(NULL, 0, MINUTE, Countdownfunc);
	}

	InitialiseGlobals();

	// register netlib handle
   char tempNdesc[50];
	mir_snprintf(tempNdesc, "%s connection settings", MODULENAME);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.a = tempNdesc;
	hNetlibUser = Netlib_RegisterUser(&nlu);

	// register webview protocol
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULENAME;
	pd.type = PROTOTYPE_PROTOCOL;
	Proto_RegisterModule(&pd);

	//protocol services
	InitServices();

	//add sound event to options
	Skin_AddSound("webviewalert", _A2W(MODULENAME), LPGENW("Alert event"));

	//value is 1 if menu is disabled
	db_set_b(NULL, MODULENAME, MENU_IS_DISABLED_KEY, 1);

	CMenuItem mi;
	mi.flags = CMIF_UNICODE;
	if ( db_get_b(NULL, MODULENAME, MENU_OFF, 0)) {
		//value is 0 if menu is enabled
		db_set_b(NULL, MODULENAME, MENU_IS_DISABLED_KEY, 0);

		mi.root = Menu_CreateRoot(MO_MAIN, _A2W(MODULENAME), 20200001);
		Menu_ConfigureItem(mi.root, MCI_OPT_UID, "403BE07B-7954-4F3E-B318-4301571776B8");

		/*DISABLE WEBVIEW*/
		SET_UID(mi, 0xdedeb697, 0xfc10, 0x4622, 0x8b, 0x97, 0x74, 0x39, 0x32, 0x68, 0xa7, 0x7b);
		CreateServiceFunction("DisableWebview", AutoUpdateMCmd);
		mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SITE));
		if (db_get_b(NULL, MODULENAME, DISABLE_AUTOUPDATE_KEY, 0))
			mi.name.w = LPGENW("Auto update disabled");
		else
			mi.name.w = LPGENW("Auto update enabled"); 
		mi.pszService = "DisableWebview";
		hMenuItem1 = Menu_AddMainMenuItem(&mi);

		// Update all webview contacts
		SET_UID(mi, 0xf324ede, 0xfdf, 0x498a, 0x8f, 0x49, 0x6d, 0x2a, 0x9f, 0xda, 0x58, 0x6);
		CreateServiceFunction("UpdateAll", UpdateAllMenuCommand);
		mi.position = 500090002;
		mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
		mi.name.w = LPGENW("Update all Webview sites");
		mi.pszService = "UpdateAll";
		Menu_AddMainMenuItem(&mi);

		// Mark All Webview Sites As Read
		SET_UID(mi, 0x1fa5fa21, 0x2ee1, 0x4372, 0xae, 0x3e, 0x3b, 0x96, 0xac, 0xd, 0xe8, 0x49);
		CreateServiceFunction("MarkAllSitesRead", MarkAllReadMenuCommand);
		mi.position = 500090099;
		mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MARKALLREAD));
		mi.name.w = LPGENW("Mark all Webview sites as read");
		mi.pszService = "MarkAllSitesRead";
		Menu_AddMainMenuItem(&mi);

		// open cache directory
		SET_UID(mi, 0xfed046a8, 0xaae5, 0x4cbe, 0xa8, 0xc, 0x3c, 0x50, 0x3e, 0x3e, 0x9b, 0x15);
		CreateServiceFunction("OpenCacheFolder", OpenCacheDir);
		mi.position = 500090099;
		mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FOLDER));
		mi.name.w = LPGENW("Open cache folder");
		mi.pszService = "OpenCacheFolder";
		Menu_AddMainMenuItem(&mi);

		// Countdown test
		SET_UID(mi, 0xbb1a94a9, 0xca63, 0x4966, 0x98, 0x48, 0x8b, 0x3f, 0x9d, 0xac, 0x6a, 0x10);
		CreateServiceFunction("Countdown", CountdownMenuCommand);
		mi.flags |= CMIF_KEEPUNTRANSLATED;
		wchar_t countername[100];
		mir_snwprintf(countername, TranslateT("%d minutes to update"), db_get_dw(NULL, MODULENAME, COUNTDOWN_KEY, 0));
		mi.position = 600090099;
		mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATEALL));
		mi.name.w = countername;
		mi.pszService = "Countdown";
		hMenuItemCountdown = Menu_AddMainMenuItem(&mi);
	}

	// contact menu
	mi.flags = CMIF_UNICODE;

	SET_UID(mi, 0xadc6a9a4, 0xdf7, 0x4f63, 0x89, 0x11, 0x8e, 0x42, 0x1d, 0xd6, 0x29, 0x31);
	CreateServiceFunction("Open web page", WebsiteMenuCommand);
	mi.position = 100;
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_URL));
	mi.pszService = "Open web page";
	mi.name.w = LPGENW("Open web page");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0x9d803e61, 0xc929, 0x4c6e, 0x9e, 0x7, 0x93, 0x0, 0xab, 0x14, 0x13, 0x50);
	CreateServiceFunction("OpenClose Window", DataWndMenuCommand);
	mi.pszService = "OpenClose Window";
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SHOW_HIDE));
	mi.name.w = LPGENW("Open/Close window");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0x3840cc71, 0xcc85, 0x448d, 0xb5, 0xc8, 0x1a, 0x7d, 0xfe, 0xf0, 0x8, 0x85);
	mi.position = 2222220;
	mi.pszService = "UpdateData";
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_UPDATE));
	mi.name.w = LPGENW("Update data");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0xd1ab586c, 0x2c71, 0x429c, 0xb1, 0x79, 0x7b, 0x3a, 0x1d, 0x4a, 0xc1, 0x7d);
	CreateServiceFunction("ContactOptions", CntOptionsMenuCommand);
	mi.pszService = "ContactOptions";
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_OPTIONS));
	mi.name.w = LPGENW("Contact options");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0xe4cda597, 0x9def, 0x4f54, 0x8a, 0xc6, 0x69, 0x3b, 0x5a, 0x7d, 0x77, 0xb6);
	CreateServiceFunction("ContactAlertOpts", CntAlertMenuCommand);
	mi.pszService = "ContactAlertOpts";
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ALERT));
	mi.name.w = LPGENW("Contact alert options");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0x63fdeed8, 0xf880, 0x423f, 0x95, 0xae, 0x20, 0x8c, 0x86, 0x3c, 0x5, 0xd8);
	CreateServiceFunction("PingWebsite", PingWebsiteMenuCommand);
	mi.pszService = "PingWebsite";
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PING));
	mi.name.w = LPGENW("Ping web site");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	SET_UID(mi, 0x28fd36de, 0x6ce1, 0x43d0, 0xa1, 0x6e, 0x98, 0x71, 0x53, 0xe8, 0xc9, 0xf4);
	CreateServiceFunction("StopDataProcessing", StpPrcssMenuCommand);
	mi.pszService = "StopDataProcessing";
	mi.hIcolibItem = LoadIcon(hInst, MAKEINTRESOURCE(IDI_STOP));
	mi.name.w = LPGENW("Stop data processing");
	Menu_AddContactMenuItem(&mi, MODULENAME);

	hWindowList = WindowList_Create();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, SiteDeleted);

	db_set_b(NULL, MODULENAME, HAS_CRASHED_KEY, 1);
	return 0;
}
