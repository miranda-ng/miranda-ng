/*
Copyright (C) 2006 Ricardo Pescuma Domenecci
Based on work (C) Heiko Schillinger

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

int &hLangpack(g_plugin.m_hLang);
CMPlugin g_plugin;

HANDLE hModulesLoaded = nullptr;
HANDLE hEventAdded = nullptr;
HANDLE hHotkeyPressed = nullptr;

INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam);
void FreeContacts();

int hksModule = 0;
int hksAction = 0;

// Functions ////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {F93BA59C-4F48-4F2E-8A91-77A2801527A3}
	{0xf93ba59c, 0x4f48, 0x4f2e, {0x8a, 0x91, 0x77, 0xa2, 0x80, 0x15, 0x27, 0xa3}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int ModulesLoaded(WPARAM, LPARAM)
{
	InitOptions();

	// Get number of protocols
	Proto_EnumAccounts(&opts.num_protos, nullptr);

	// Add hotkey to multiple services
	HOTKEYDESC hkd = {};
	hkd.dwFlags = HKD_UNICODE;
	hkd.pszName = "Quick Contacts/Open dialog";
	hkd.szDescription.w = LPGENW("Open dialog");
	hkd.szSection.w = LPGENW("Quick Contacts");
	hkd.pszService = MS_QC_SHOW_DIALOG;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_ALT, 'Q');
	Hotkey_Register(&hkd);

	hkd.pszService = nullptr;

	hkd.lParam = HOTKEY_FILE;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'F');
	hkd.pszName = "Quick Contacts/File";
	hkd.szDescription.w = LPGENW("Send file");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_URL;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'U');
	hkd.pszName = "Quick Contacts/URL";
	hkd.szDescription.w = LPGENW("Send URL");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_INFO;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'I');
	hkd.pszName = "Quick Contacts/Info";
	hkd.szDescription.w = LPGENW("Open user info");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_HISTORY;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'H');
	hkd.pszName = "Quick Contacts/History";
	hkd.szDescription.w = LPGENW("Open history");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_MENU;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'M');
	hkd.pszName = "Quick Contacts/Menu";
	hkd.szDescription.w = LPGENW("Open contact menu");
	Hotkey_Register(&hkd);

	hkd.lParam = HOTKEY_ALL_CONTACTS;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, 'A');
	hkd.pszName = "Quick Contacts/All Contacts";
	hkd.szDescription.w = LPGENW("Show all contacts");
	Hotkey_Register(&hkd);

	if (ServiceExists(MS_SKIN_ADDHOTKEY)) {
		SKINHOTKEYDESCEX hk = { 0 };
		hk.cbSize = sizeof(hk);
		hk.pszSection = Translate("Quick Contacts");
		hk.pszName = Translate("Open dialog");
		hk.pszDescription = Translate("Open dialog");
		hk.pszService = MS_QC_SHOW_DIALOG;
		hk.DefHotKey = 0;
		CallService(MS_SKIN_ADDHOTKEY, 0, (LPARAM)&hk);
	}

	// Add menu item
	CMenuItem mi;
	SET_UID(mi, 0x3a3f768a, 0xcf47, 0x43d5, 0x92, 0x16, 0xe4, 0xeb, 0x93, 0xf6, 0x72, 0xfa);
	mi.position = 500100001;
	mi.flags = CMIF_UNICODE;
	mi.name.w = LPGENW("Quick Contacts...");
	mi.pszService = MS_QC_SHOW_DIALOG;
	Menu_AddMainMenuItem(&mi);
	return 0;
}

// called when a message/file/url was sent
// handle of contact is set as window-userdata
static int EventAdded(WPARAM wparam, LPARAM hDbEvent)
{
	DBEVENTINFO dbei = {};
	db_event_get(hDbEvent, &dbei);
	if (!(dbei.flags & DBEF_SENT) || (dbei.flags & DBEF_READ)
		|| !db_get_b(NULL, MODULENAME, "EnableLastSentTo", 0)
		|| db_get_w(NULL, MODULENAME, "MsgTypeRec", TYPE_GLOBAL) != TYPE_GLOBAL)
		return 0;

	db_set_dw(NULL, MODULENAME, "LastSentTo", (UINT_PTR)wparam);
	return 0;
}

extern "C" __declspec(dllexport) int Load()
{
	CreateServiceFunction(MS_QC_SHOW_DIALOG, ShowDialog);

	// hooks
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hEventAdded = HookEvent(ME_DB_EVENT_ADDED, EventAdded);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload(void)
{
	FreeContacts();

	DeInitOptions();

	UnhookEvent(hModulesLoaded);
	UnhookEvent(hEventAdded);
	return 0;
}
