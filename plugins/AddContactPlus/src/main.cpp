/*
AddContact+ plugin for Miranda NG

Copyright (C) 2007-11 Bartosz 'Dezeath' Białek
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdafx.h"

CMPlugin g_plugin;
static HANDLE hToolBarItem = nullptr;
static HGENMENU hMainMenuItem = nullptr;
HWND hAddDlg;

static IconItem icon[] =
{
	{ LPGEN("Add contact"), ICON_ADD, IDI_ADDCONTACT }
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {6471D451-2FE0-4EE2-850E-9F84F3C0D187}
	{0x6471d451, 0x2fe0, 0x4ee2, {0x85, 0xe, 0x9f, 0x84, 0xf3, 0xc0, 0xd1, 0x87}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR AddContactPlusDialog(WPARAM, LPARAM)
{
	if (hAddDlg) {
		SetForegroundWindow(hAddDlg);
		SetFocus(hAddDlg);
	}
	else hAddDlg = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ADDCONTACT), nullptr, AddContactDlgProc, 0);

	return 0;
}

static int OnAccListChanged(WPARAM, LPARAM)
{
	int iAccCount = 0;
	for (auto &pa : Accounts()) {
		if (!pa->IsEnabled())
			continue;

		uint32_t dwCaps = (uint32_t)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
		if (dwCaps & PF1_BASICSEARCH || dwCaps & PF1_EXTSEARCH || dwCaps & PF1_SEARCHBYEMAIL || dwCaps & PF1_SEARCHBYNAME)
			iAccCount++;
	}

	if (iAccCount) {
		if (hMainMenuItem)
			return 0;

		CMenuItem mi(&g_plugin);
		SET_UID(mi, 0xb19db907, 0x870e, 0x49fa, 0xa7, 0x1e, 0x43, 0x5e, 0xa8, 0xe5, 0x9b, 0xbd);
		mi.position = 500020001;
		mi.flags = CMIF_UNICODE;
		mi.hIcolibItem = icon[0].hIcolib;
		mi.name.w = LPGENW("&Add contact...");
		mi.pszService = MS_ADDCONTACTPLUS_SHOW;
		hMainMenuItem = Menu_AddMainMenuItem(&mi);
	}
	else {
		if (!hMainMenuItem)
			return 0;

		Menu_RemoveItem(hMainMenuItem);
		CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hToolBarItem, 0);
		hMainMenuItem = nullptr;
	}

	return 0;
}

static int CreateButton(WPARAM, LPARAM)
{
	TTBButton tbb = {};
	tbb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	tbb.pszService = MS_ADDCONTACTPLUS_SHOW;
	tbb.name = tbb.pszTooltipUp = LPGEN("Add contact");
	tbb.hIconHandleUp = icon[0].hIcolib;
	hToolBarItem = g_plugin.addTTB(&tbb);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HOTKEYDESC hkd = {};
	hkd.dwFlags = HKD_UNICODE;
	hkd.pszName = "AddContactPlus_OpenDialog";
	hkd.szDescription.w = LPGENW("Open add contact dialog");
	hkd.szSection.w = LPGENW("Main");
	hkd.pszService = MS_ADDCONTACTPLUS_SHOW;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'C') | HKF_MIRANDA_LOCAL;
	g_plugin.addHotkey(&hkd);

	OnAccListChanged(0, 0);

	HookEvent(ME_TTB_MODULELOADED, CreateButton);
	return 0;
}

int CMPlugin::Load()
{
	INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_USEREX_CLASSES };
	InitCommonControlsEx(&icex);

	g_plugin.registerIcon(LPGEN("AddContact+"), icon);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccListChanged);

	CreateServiceFunction(MS_ADDCONTACTPLUS_SHOW, AddContactPlusDialog);
	return 0;
}
