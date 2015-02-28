/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

char ModuleName[] = "WhenWasIt";
HINSTANCE hInstance;
HWND hBirthdaysDlg = NULL;
HWND hUpcomingDlg = NULL;
HANDLE hAddBirthdayWndsList = NULL;
int hLangpack;

HANDLE hmCheckBirthdays = NULL;
HANDLE hmBirthdayList = NULL;
HANDLE hmRefreshDetails = NULL;
HANDLE hmAddChangeBirthday = NULL;
HANDLE hmImportBirthdays = NULL;
HANDLE hmExportBirthdays = NULL;


CommonData commonData = {0};

CLIST_INTERFACE *pcli;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2FF96C84-B0B5-470E-BBF9-907B9F3F5D2F}
	{0x2ff96c84, 0xb0b5, 0x470e, {0xbb, 0xf9, 0x90, 0x7b, 0x9f, 0x3f, 0x5d, 0x2f}}
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	Log("%s", "Entering function " __FUNCTION__);

	mir_getLP(&pluginInfo);
	mir_getCLI();

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icex);
	
	AddIcons();

	Log("%s", "Creating service functions ...");
	InitServices();

	Log("%s", "Hooking events ...");	
	HookEvents();
	
	hAddBirthdayWndsList = WindowList_Create();

	CLISTMENUITEM cl = { sizeof(cl) };
	cl.position = 10000000;
	cl.pszPopupName = LPGEN("Birthdays (When Was It)");

	cl.pszService = MS_WWI_CHECK_BIRTHDAYS;
	cl.icolibItem = hCheckMenu;
	cl.pszName = LPGEN("Check for birthdays");
	hmCheckBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_LIST_SHOW;
	cl.pszName = LPGEN("Birthday list");
	cl.icolibItem = hListMenu;
	hmBirthdayList = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_REFRESH_USERDETAILS;
	cl.position = 10100000;
	cl.pszName = LPGEN("Refresh user details");
	cl.icolibItem = hRefreshUserDetails;
	hmRefreshDetails = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_IMPORT_BIRTHDAYS;
	cl.position = 10200000;
	cl.pszName = LPGEN("Import birthdays");
	cl.icolibItem = hImportBirthdays;
	hmImportBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_EXPORT_BIRTHDAYS;
	cl.pszName = LPGEN("Export birthdays");
	cl.icolibItem = hExportBirthdays;
	hmExportBirthdays = Menu_AddMainMenuItem(&cl);
	
	cl.pszService = MS_WWI_ADD_BIRTHDAY;
	cl.position = 10000000;
	cl.icolibItem = hAddBirthdayContact;
	cl.pszName = LPGEN("Add/change user &birthday");
	hmAddChangeBirthday = Menu_AddContactMenuItem(&cl);

	// Register hotkeys
	HOTKEYDESC hotkey = { sizeof(hotkey) };
	hotkey.pszSection = LPGEN("Birthdays");

	hotkey.pszName = "wwi_birthday_list";
	hotkey.pszDescription = LPGEN("Birthday list");
	hotkey.pszService = MS_WWI_LIST_SHOW;
	Hotkey_Register(&hotkey);
		
	hotkey.pszName = "wwi_check_birthdays";
	hotkey.pszDescription = LPGEN("Check for birthdays");
	hotkey.pszService = MS_WWI_CHECK_BIRTHDAYS;
	Hotkey_Register(&hotkey);
	
	
	SkinAddNewSoundExT(BIRTHDAY_NEAR_SOUND, LPGENT("WhenWasIt"), LPGENT("Birthday near"));
	SkinAddNewSoundExT(BIRTHDAY_TODAY_SOUND, LPGENT("WhenWasIt"), LPGENT("Birthday today"));
	
	Log("%s", "Leaving function " __FUNCTION__);
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	Log("%s", "Entering function " __FUNCTION__);
	
	if (hBirthdaysDlg)
		SendMessage(hBirthdaysDlg, WM_CLOSE, 0, 0);
	
	if (hUpcomingDlg)
		SendMessage(hUpcomingDlg, WM_CLOSE, 0, 0);
	
	WindowList_Broadcast(hAddBirthdayWndsList, WM_CLOSE, 0, 0);
	WindowList_Destroy(hAddBirthdayWndsList);

	Log("%s", "Killing timers ...");
	KillTimers();
	
	Log("%s", "Unhooking events ...");
	UnhookEvents();
	
	Log("%s", "Destroying service functions ...");
	DestroyServices();

	Log("%s", "Leaving function " __FUNCTION__);
	return 0;
}

bool WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	return TRUE;
}
