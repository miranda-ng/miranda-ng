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

#include "stdafx.h"

MWindowList hAddBirthdayWndsList = nullptr;

CMPlugin g_plugin;

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
	// {2FF96C84-B0B5-470E-BBF9-907B9F3F5D2F}
	{ 0x2ff96c84, 0xb0b5, 0x470e, { 0xbb, 0xf9, 0x90, 0x7b, 0x9f, 0x3f, 0x5d, 0x2f } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	foreground(MODULENAME, "Foreground", FOREGROUND_COLOR),
	background(MODULENAME, "Background", BACKGROUND_COLOR),
	checkInterval(MODULENAME, "CheckInterval", CHECK_INTERVAL),
	daysInAdvance(MODULENAME, "DaysInAdvance", DAYS_TO_NOTIFY),
	daysAfter(MODULENAME, "DaysAfter", DAYS_TO_NOTIFY_AFTER),
	popupTimeout(MODULENAME, "PopupTimeout", POPUP_TIMEOUT),
	popupTimeoutToday(MODULENAME, "PopupTimeoutToday", POPUP_TIMEOUT),
	bUsePopups(MODULENAME, "UsePopups", true),
	bUseDialog(MODULENAME, "UseDialog", true),
	bIgnoreSubcontacts(MODULENAME, "IgnoreSubcontacts", false),
	cShowAgeMode(MODULENAME, "ShowCurrentAge", false),
	bNoBirthdaysPopup(MODULENAME, "NoBirthdaysPopup", false),
	bOpenInBackground(MODULENAME, "OpenInBackground", false),
	cSoundNearDays(MODULENAME, "SoundNearDays", BIRTHDAY_NEAR_DEFAULT_DAYS),
	lPopupClick(MODULENAME, "PopupLeftClick", 2),
	rPopupClick(MODULENAME, "PopupRightClick", 1),
	bOncePerDay(MODULENAME, "OncePerDay", false),
	cDlgTimeout(MODULENAME, "DlgTimeout", POPUP_TIMEOUT),
	notifyFor(MODULENAME, "NotifyFor", 0)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	Log("%s", "Entering function " __FUNCTION__);

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icex);

	AddIcons();

	Log("%s", "Creating service functions ...");
	InitServices();

	Log("%s", "Hooking events ...");
	HookEvents();

	CheckConvert();

	hAddBirthdayWndsList = WindowList_Create();

	CMenuItem mi(&g_plugin);
	mi.position = 10000000;
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Birthdays (When Was It)"), mi.position);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "95D842AE-FCCE-43C9-87E3-C28546B7E00E");

	SET_UID(mi, 0x4efbd640, 0xabbd, 0x470e, 0x9a, 0xa, 0x64, 0x76, 0x1a, 0x74, 0xf3, 0x24);
	mi.pszService = MS_WWI_CHECK_BIRTHDAYS;
	mi.hIcolibItem = hCheckMenu;
	mi.name.a = LPGEN("Check for birthdays");
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x3379c184, 0x5bfa, 0x456d, 0x80, 0xf7, 0xb3, 0x73, 0xd8, 0xe4, 0x4a, 0x33);
	mi.pszService = MS_WWI_LIST_SHOW;
	mi.name.a = LPGEN("Birthday list");
	mi.hIcolibItem = hListMenu;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x57c88bdd, 0x389f, 0x40b2, 0xbb, 0xc5, 0x24, 0xcd, 0xef, 0xeb, 0xc8, 0x4);
	mi.pszService = MS_WWI_REFRESH_USERDETAILS;
	mi.position = 10100000;
	mi.name.a = LPGEN("Refresh user details");
	mi.hIcolibItem = hRefreshUserDetails;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x402c5566, 0x6272, 0x4ef8, 0x89, 0x7d, 0x2e, 0x23, 0x8b, 0xbd, 0xf1, 0x2d);
	mi.pszService = MS_WWI_IMPORT_BIRTHDAYS;
	mi.position = 10200000;
	mi.name.a = LPGEN("Import birthdays");
	mi.hIcolibItem = hImportBirthdays;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xa1a1fd44, 0xc43a, 0x4615, 0xab, 0x5f, 0x4f, 0xd7, 0x57, 0x69, 0x19, 0xf6);
	mi.pszService = MS_WWI_EXPORT_BIRTHDAYS;
	mi.name.a = LPGEN("Export birthdays");
	mi.hIcolibItem = hExportBirthdays;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x6f844569, 0xad06, 0x4b27, 0x8f, 0xd8, 0x1, 0x24, 0xe7, 0x1e, 0xcc, 0x43);
	mi.root = nullptr;
	mi.pszService = MS_WWI_ADD_BIRTHDAY;
	mi.position = 10000000;
	mi.hIcolibItem = hAddBirthdayContact;
	mi.name.a = LPGEN("Add/change user &birthday");
	Menu_AddContactMenuItem(&mi);

	// Register hotkeys
	HOTKEYDESC hotkey = {};
	hotkey.szSection.a = LPGEN("Birthdays");

	hotkey.pszName = "wwi_birthday_list";
	hotkey.szDescription.a = LPGEN("Birthday list");
	hotkey.pszService = MS_WWI_LIST_SHOW;
	g_plugin.addHotkey(&hotkey);

	hotkey.pszName = "wwi_check_birthdays";
	hotkey.szDescription.a = LPGEN("Check for birthdays");
	hotkey.pszService = MS_WWI_CHECK_BIRTHDAYS;
	g_plugin.addHotkey(&hotkey);

	g_plugin.addSound(BIRTHDAY_NEAR_SOUND, LPGENW("WhenWasIt"), LPGENW("Birthday near"));
	g_plugin.addSound(BIRTHDAY_TODAY_SOUND, LPGENW("WhenWasIt"), LPGENW("Birthday today"));

	Log("%s", "Leaving function " __FUNCTION__);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	Log("%s", "Entering function " __FUNCTION__);

	Log("%s", "Unhooking events ...");
	UnhookEvents();

	Log("%s", "Leaving function " __FUNCTION__);
	return 0;
}
