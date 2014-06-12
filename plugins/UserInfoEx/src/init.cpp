/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {9C23A24B-E6AA-43C6-B0B8-D6C36D2F7B57}
	{0x9c23a24b, 0xe6aa, 0x43c6, {0xb0, 0xb8, 0xd6, 0xc3, 0x6d, 0x2f, 0x7b, 0x57}}
};

int hLangpack;

/*
============================================================================================
	event hooks
============================================================================================
*/

/**
 * This function is called by the ME_TTB_MODULELOADED event.
 * It adds a set of buttons to the TopToolbar plugin.
 *
 * @param	wParam	- not used
 * @param	lParam	- not used
 *
 * @return	always 0
 **/
static int OnTopToolBarLoaded(WPARAM wParam, LPARAM lParam)
{
	DlgAnniversaryListOnTopToolBarLoaded();
	SvcReminderOnTopToolBarLoaded();
	return 0;
}

/**
 * This function is called by Miranda just after loading all system modules.
 *
 * @param	wParam	- not used
 * @param	lParam	- not used
 *
 * @return	always 0
 **/
static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	myGlobals.PopupActionsExist = ServiceExists(MS_POPUP_REGISTERACTIONS);

	// options
	OptionsLoadModule();

	// create services to receive string lists of languages and timezones
	SvcConstantsLoadModule();

	// load module to remind user about birthday and a anniversary
	SvcReminderOnModulesLoaded();

	// load extended intagration services
	SvcHomepageLoadModule();
	SvcFlagsOnModulesLoaded();

	// build contact's menuitems
	RebuildMenu();
	HookEvent(ME_CLIST_PREBUILDSTATUSMENU, (MIRANDAHOOK)RebuildAccount);
	return 0;
}

static int OnShutdown(WPARAM wParam, LPARAM lParam)
{
	DlgContactInfoUnLoadModule();
	SvcReminderUnloadModule();

	// uninitialize classes
	CtrlContactUnLoadModule();
	CtrlButtonUnloadModule();

	SvcConstantsUnloadModule();
	SvcEMailUnloadModule();
	SvcFlagsUnloadModule();
	SvcGenderUnloadModule();
	SvcHomepageUnloadModule();
	SvcPhoneUnloadModule();

	mir_free(hMenuItemAccount);
	return 0;
}

/*
============================================================================================
	plugin interface & DllEntrypoint
============================================================================================
*/

/**
 * This function is called by Miranda to get some information about this plugin.
 *
 * @return	pointer to pluginInfo struct
 **/
extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	myGlobals.mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

/**
 * This function returns the provided interfaces.
 *
 * @return	array of interfaces
 **/
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {
	MIID_UIUSERINFO,		// replace the default userinfo module
	MIID_SREMAIL,			// Send/Receive E-Mail service is provided
	MIID_LAST
};

/**
 * This function is called by Miranda just to make it possible to unload some memory, ...
 *
 * @return	0
 **/
extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

/**
 * This function is called by Miranda to initialize the plugin.
 *
 * @return	0
 **/
extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	// init common controls
	INITCOMMONCONTROLSEX ccEx;
	ccEx.dwSize = sizeof(ccEx);
	ccEx.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES;
	InitCommonControlsEx(&ccEx);

	ZeroMemory(&myGlobals, sizeof(MGLOBAL));

	// init clist interface
	mir_getCLI();

	// init new miranda timezone interface
	mir_getTMI(&tmi);

	// init freeimage interface
	INT_PTR result = CALLSERVICE_NOTFOUND;
	if (ServiceExists(MS_IMG_GETINTERFACE))
		result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&FIP);

	if (FIP == NULL || result != S_OK) {
		MessageBoxEx(NULL, TranslateT("Fatal error, image services not found. Flags module will be disabled."), _T("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
		return 1;
	}

	if (IsWinVerVistaPlus()) {
		HMODULE hDwmApi = LoadLibraryA("dwmapi.dll");
		if (hDwmApi)
			dwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)GetProcAddress(hDwmApi, "DwmIsCompositionEnabled");
	}

	// check for dbx_tree
	myGlobals.UseDbxTree = ServiceExists("DBT/Entity/GetRoot");

	// load icon library
	IcoLib_LoadModule();

	SvcFlagsLoadModule();
	tmi.getTimeZoneTime ? SvcTimezoneLoadModule() : SvcTimezoneLoadModule_old();
	SvcContactInfoLoadModule();
	SvcEMailLoadModule();
	SvcRefreshContactInfoLoadModule();

	SvcPhoneEnableExtraIcons();
	SvcGenderEnableExtraIcons();
	SvcHomepageEnableExtraIcons();

	CtrlContactLoadModule();
	// load my button class
	CtrlButtonLoadModule();
	// initializes the Ex/Import Services
	SvcExImport_LoadModule();
	// load the UserInfoPropertySheet module
	DlgContactInfoLoadModule();

	// Anniversary stuff
	DlgAnniversaryListLoadModule();
	SvcReminderLoadModule();

	// Now the module is loaded! Start initializing certain things
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_TTB_MODULELOADED, OnTopToolBarLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	return 0;
}

/**
 * Windows needs it for loading.
 *
 * @return	TRUE
 **/
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		ghInst = hinst;

	return TRUE;
}
