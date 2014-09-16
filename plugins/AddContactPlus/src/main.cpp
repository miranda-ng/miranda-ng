/*
AddContact+ plugin for Miranda NG

Copyright (C) 2007-11 Bartosz 'Dezeath' Bia³ek
Copyright (C) 2012-14 Miranda NG Team

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
#include "addcontactplus.h"

CLIST_INTERFACE *pcli;
HINSTANCE hInst;
int hLangpack;
static HANDLE hMainMenuItem = 0, hToolBarItem = 0;
HWND hAddDlg;

static IconItem icon = { LPGEN("Add contact"), ICON_ADD, IDI_ADDCONTACT };

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {6471D451-2FE0-4EE2-850E-9F84F3C0D187}
	{0x6471d451, 0x2fe0, 0x4ee2, {0x85, 0xe, 0x9f, 0x84, 0xf3, 0xc0, 0xd1, 0x87}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR AddContactPlusDialog(WPARAM, LPARAM)
{
	if (hAddDlg) {
		SetForegroundWindow(hAddDlg);
		SetFocus(hAddDlg);
	}
	else hAddDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDCONTACT), NULL, AddContactDlgProc, 0);

	return 0;
}

static int OnAccListChanged(WPARAM, LPARAM)
{
	PROTOACCOUNT** pAccounts;
	int iRealAccCount, iAccCount = 0;

	ProtoEnumAccounts(&iRealAccCount, &pAccounts);
	for (int i = 0; i < iRealAccCount; i++) {
		if (!IsAccountEnabled(pAccounts[i]))
			continue;

		DWORD dwCaps = (DWORD)CallProtoService(pAccounts[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
		if (dwCaps & PF1_BASICSEARCH || dwCaps & PF1_EXTSEARCH || dwCaps & PF1_SEARCHBYEMAIL || dwCaps & PF1_SEARCHBYNAME)
			iAccCount++;
	}

	if (iAccCount) {
		if (hMainMenuItem)
			return 0;

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position = 500020001;
		mi.flags = CMIF_TCHAR;
		mi.icolibItem = icon.hIcolib;
		mi.ptszName = LPGENT("&Add contact...");
		mi.pszService = MS_ADDCONTACTPLUS_SHOW;
		hMainMenuItem = Menu_AddMainMenuItem(&mi);
	}
	else {
		if (!hMainMenuItem)
			return 0;

		CallService(MO_REMOVEMENUITEM, (WPARAM)hMainMenuItem, 0);
		CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hToolBarItem, 0);
		hMainMenuItem = 0;
	}

	return 0;
}

static int CreateButton(WPARAM, LPARAM)
{
	TTBButton tbb = { sizeof(tbb) };
	tbb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	tbb.pszService = MS_ADDCONTACTPLUS_SHOW;
	tbb.name = tbb.pszTooltipUp = LPGEN("Add contact");
	tbb.hIconHandleUp = icon.hIcolib;
	hToolBarItem = TopToolbar_AddButton(&tbb);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.dwFlags = HKD_TCHAR;
	hkd.pszName = "AddContactPlus_OpenDialog";
	hkd.ptszDescription = LPGENT("Open add contact dialog");
	hkd.ptszSection = LPGENT("Main");
	hkd.pszService = MS_ADDCONTACTPLUS_SHOW;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'C') | HKF_MIRANDA_LOCAL;
	Hotkey_Register(&hkd);

	OnAccListChanged(0, 0);

	HookEvent(ME_TTB_MODULELOADED, CreateButton);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_USEREX_CLASSES };
	InitCommonControlsEx(&icex);

	Icon_Register(hInst, LPGEN("AddContact+"), &icon, 1);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccListChanged);

	CreateServiceFunction(MS_ADDCONTACTPLUS_SHOW, AddContactPlusDialog);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
