/*

AddContact+ plugin for Miranda IM

Copyright (C) 2007-2011 Bartosz 'Dezeath' Bia³ek

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

HINSTANCE hInst;
int hLangpack;
static HANDLE hModulesLoaded = 0, hChangedIcons = 0, hAccListChanged = 0,
			  hMainMenuItem = 0, hToolBarItem = 0, hService = 0;
HANDLE hIconLibItem;

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
	// {6471D451-2FE0-4ee2-850E-9F84F3C0D187}
	{ 0x6471d451, 0x2fe0, 0x4ee2, { 0x85, 0xe, 0x9f, 0x84, 0xf3, 0xc0, 0xd1, 0x87 } }
};

static const MUUID interfaces[] = {MIID_ADDCONTACTPLUS, MIID_LAST};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

INT_PTR AddContactPlusDialog(WPARAM, LPARAM)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDCONTACT), (HWND)NULL, AddContactDlgProc, 0);
	return 0;
}

static int OnIconsChanged(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = {0};

	if (!hMainMenuItem)
		return 0;

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_ICON | CMIF_ICONFROMICOLIB | CMIF_TCHAR;
	mi.icolibItem = hIconLibItem;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMainMenuItem, (LPARAM)&mi);

	return 0;
}

static int OnAccListChanged(WPARAM, LPARAM)
{
	PROTOACCOUNT** pAccounts;
	int iRealAccCount, iAccCount = 0;
	DWORD dwCaps;

	ProtoEnumAccounts(&iRealAccCount, &pAccounts);
	for (int i = 0; i < iRealAccCount; i++)
	{
		if (!IsAccountEnabled(pAccounts[i])) continue;
		dwCaps = (DWORD)CallProtoService(pAccounts[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
		if (dwCaps & PF1_BASICSEARCH || dwCaps & PF1_EXTSEARCH || dwCaps & PF1_SEARCHBYEMAIL || dwCaps & PF1_SEARCHBYNAME)
			iAccCount++;
	}

	if (iAccCount)
	{
		CLISTMENUITEM mi = {0};

		if (hMainMenuItem)
			return 0;

		mi.cbSize = sizeof(mi);
		mi.position = 500020001;
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_TCHAR;
		mi.icolibItem = hIconLibItem;
		mi.ptszName = LPGENT("&Add Contact...");
		mi.pszService = MS_ADDCONTACTPLUS_SHOW;
		hMainMenuItem = Menu_AddMainMenuItem(&mi);
	}
	else
	{
		if (!hMainMenuItem)
			return 0;

		CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)hMainMenuItem, 0);
		CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hToolBarItem, 0);

		hMainMenuItem = 0;
	}

	return 0;
}

static int CreateButton(WPARAM, LPARAM)
{
	TTBButton tbb = {0};
	tbb.cbSize = sizeof(tbb);
	tbb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	tbb.name = "Add Contact";
	tbb.pszService = MS_ADDCONTACTPLUS_SHOW;
	tbb.pszTooltipUp = "Add Contact";
	tbb.hIconHandleUp = hIconLibItem;
	hToolBarItem = TopToolbar_AddButton(&tbb);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	if (ServiceExists(MS_UPDATE_REGISTERFL))
#if defined(_WIN64)
		CallService(MS_UPDATE_REGISTERFL, 4414, (LPARAM)&pluginInfo);
#else
		CallService(MS_UPDATE_REGISTERFL, 3842, (LPARAM)&pluginInfo);
#endif

	SKINICONDESC sid = {0};
	char szFile[MAX_PATH];
	GetModuleFileNameA(hInst, szFile, MAX_PATH);
	sid.cbSize = sizeof(SKINICONDESC);
	sid.flags = SIDF_TCHAR;
	sid.pszDefaultFile = szFile;
	sid.ptszSection = _T("AddContact+");
	sid.iDefaultIndex = -IDI_ADDCONTACT;
	sid.ptszDescription = LPGENT("Add Contact");
	sid.pszName = ICON_ADD;
	hIconLibItem = Skin_AddIcon(&sid);
	hChangedIcons = HookEvent(ME_SKIN2_ICONSCHANGED, OnIconsChanged);

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.dwFlags = HKD_TCHAR;
	hkd.pszName = "AddContactPlus_OpenDialog";
	hkd.ptszDescription = LPGENT("Open Add Contact Dialog");
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

	INITCOMMONCONTROLSEX icex = {0};
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icex);

	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hAccListChanged = HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccListChanged);
	hService = CreateServiceFunction(MS_ADDCONTACTPLUS_SHOW, AddContactPlusDialog);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hChangedIcons);
	UnhookEvent(hAccListChanged);
	DestroyServiceFunction(hService);

	return 0;
}
