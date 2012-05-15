/* 
Copyright (C) 2007 Dmitry Titkov (C) 2010 tico-tico, Mataes

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

#include "TranslitSwitcher.h"

HINSTANCE hInst = NULL;
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;
HANDLE hOnButtonPressed;
int hLangpack;

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
	0,
	MIID_TS
};

HANDLE hHook;
HANDLE hService, hService2, hService3;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

static const MUUID interfaces[] = {MIID_TS, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

//-------------------------------------------------------------------------------------------------------
#define MS_TS_SWITCHLAYOUT "TranslitSwitcher/SwitchLayout"
INT_PTR ServiceSwitch(WPARAM wParam, LPARAM lParam)
{
	SwitchLayout(lParam);
	return 0;
}

#define MS_TS_TRANSLITLAYOUT "TranslitSwitcher/TranslitLayout"
INT_PTR ServiceTranslit(WPARAM wParam, LPARAM lParam)
{
	TranslitLayout(lParam);
	return 0;
}

#define MS_TS_INVERTCASE "TranslitSwitcher/InvertCase"
INT_PTR ServiceInvert(WPARAM wParam, LPARAM lParam)
{
	InvertCase(lParam);
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	HICON hSwitchIcon = NULL, hTranslitIcon = NULL, hInvertIcon = NULL;

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.dwFlags = HKD_TCHAR;

	hService = CreateServiceFunction(MS_TS_SWITCHLAYOUT, ServiceSwitch);
	hService2 = CreateServiceFunction(MS_TS_TRANSLITLAYOUT, ServiceTranslit);
	hService3 = CreateServiceFunction(MS_TS_INVERTCASE, ServiceInvert);

	hkd.pszName = "TranslitSwitcher/ConvertAllOrSelected";
	hkd.ptszDescription = _T("Convert All / Selected");
	hkd.ptszSection = _T("TranslitSwitcher");
	hkd.pszService = MS_TS_SWITCHLAYOUT;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL+HKCOMB_A, 'R') | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;

	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "TranslitSwitcher/ConvertLastOrSelected";
	hkd.ptszDescription = _T("Convert Last / Selected");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_ALT+HKCOMB_A, 'R') | HKF_MIRANDA_LOCAL;
	hkd.lParam = TRUE;

	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "TranslitSwitcher/TranslitAllOrSelected";
	hkd.ptszDescription = _T("Translit All / Selected");
	hkd.pszService = MS_TS_TRANSLITLAYOUT;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL+HKCOMB_A, 'T') | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;

	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "TranslitSwitcher/TranslitLastOrSelected";
	hkd.ptszDescription = _T("Translit Last / Selected");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_ALT+HKCOMB_A, 'T') | HKF_MIRANDA_LOCAL;
	hkd.lParam = TRUE;

	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "TranslitSwitcher/InvertCaseAllOrSelected";
	hkd.ptszDescription = _T("Invert Case All / Selected");
	hkd.pszService = MS_TS_INVERTCASE;
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL+HKCOMB_A, 'Y') | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;

	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hkd.pszName = "TranslitSwitcher/InvertCaseLastOrSelected";
	hkd.ptszDescription = _T("Invert Case Last / Selected");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_ALT+HKCOMB_A, 'Y') | HKF_MIRANDA_LOCAL;
	hkd.lParam = TRUE;

	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	hOnButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, OnButtonPressed); 
	if (ServiceExists(MS_BB_ADDBUTTON) && ServiceExists(MS_SKIN2_ADDICON))
	{
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.flags = SIDF_TCHAR;
		sid.ptszSection = _T("TabSRMM/TranslitSwitcher");
		sid.cx = sid.cy = 16;
		sid.ptszDescription = _T("SwitchLayout and Send");
		sid.pszName = "SwitchLayout and Send";
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SWITCHSEND));
		hSwitchIcon = (HICON)CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);

		sid.ptszDescription = _T("Translit and Send");
		sid.pszName = "Translit and Send";
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRANSLITSEND));
		hTranslitIcon = (HICON)CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);

		sid.ptszDescription = _T("Invert Case and Send");
		sid.pszName = "Invert Case and Send";
		sid.hDefaultIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_INVERTSEND));
		hInvertIcon = (HICON)CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);
	}
	else if (ServiceExists(MS_BB_ADDBUTTON))
	{
		hSwitchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SWITCHSEND));
		hTranslitIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRANSLITSEND));
		hInvertIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_INVERTSEND));
	}

	if (ServiceExists(MS_BB_ADDBUTTON))
	{
		BBButton bbd = {0};
		bbd.cbSize = sizeof(BBButton);
		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISRSIDEBUTTON;
		bbd.pszModuleName = "SwitchLayout and Send";
		bbd.ptszTooltip = TranslateT("SwitchLayout and Send");
		bbd.hIcon = (HANDLE)hSwitchIcon;
		bbd.dwButtonID = 1;
		bbd.dwDefPos = 30;
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		bbd.pszModuleName = "Translit and Send";
		bbd.ptszTooltip = TranslateT("Translit and Send");
		bbd.hIcon = (HANDLE)hTranslitIcon;
		bbd.dwButtonID = 1;
		bbd.dwDefPos = 40;
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

		bbd.pszModuleName = "Invert Case and Send";
		bbd.ptszTooltip = TranslateT("Invert Case and Send");
		bbd.hIcon = (HANDLE)hInvertIcon;
		bbd.dwButtonID = 1;
		bbd.dwDefPos = 50;
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);
	}

	DestroyIcon(hSwitchIcon);
	DestroyIcon(hTranslitIcon);
	DestroyIcon(hInvertIcon);

	return 0;
}

//-------------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLP(&pluginInfoEx);

	hHook = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyServiceFunction(hService);
	DestroyServiceFunction(hService2);
	DestroyServiceFunction(hService3);
	UnhookEvent(hHook);
	UnhookEvent(hOnButtonPressed);

	return 0;
}