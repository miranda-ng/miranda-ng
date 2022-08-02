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

#include "stdafx.h"

CMPlugin g_plugin;
HMODULE hDwmApi;

MGLOBAL myGlobals;
pfnDwmIsCompositionEnabled	dwmIsCompositionEnabled;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {9C23A24B-E6AA-43C6-B0B8-D6C36D2F7B57}
	{0x9c23a24b, 0xe6aa, 0x43c6, {0xb0, 0xb8, 0xd6, 0xc3, 0x6d, 0x2f, 0x7b, 0x57}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),

	// Common options
	bUseUnknownFlag(MODULENAME, "UseUnknownFlag", true),
	bShowStatusIconFlag(MODULENAME, "ShowStatusIconFlag", true),

	// Advanced options
	bCheckVersion(MODULENAME, "CheckIconPackVersion", true),
	bMetaScan(MODULENAME, "MetaScan", true),
	bButtonIcons(MODULENAME, "ButtonIcons", true),
	bEmailService(MODULENAME, "emailEx", true),

	// Propsheet options
	bAero(MODULENAME, "AeroAdaption", true),
	bReadOnly(MODULENAME, "PBCIReadOnly", false),
	bSortTree(MODULENAME, "TreeSortItems", false),
	bTreeGroups(MODULENAME, "TreeGroups", true),
	bShowColours(MODULENAME, "ShowColours", true),
	bChangeDetails(MODULENAME, "ChangeMyDetails", false),

	clrBoth(MODULENAME, "colourBoth", RGB(0, 160, 10)),
	clrMeta(MODULENAME, "colourMeta", RGB(120, 40, 130)),
	clrCustom(MODULENAME, "colourUser", RGB(0, 10, 130)),
	clrNormal(MODULENAME, "colourNormal", RGB(90, 90, 90)),
	clrChanged(MODULENAME, "colourChanged", RGB(190, 0, 0)),

	// Reminder options
	bRemindExtraIcon(MODULENAME, "RemindExtraIcon", true),
	bRemindFlashIcon(MODULENAME, "RemindFlashIcon", false),
	bRemindMenuEnabled(MODULENAME, "RemindMenuEnabled", true),
	bRemindStartupCheck(MODULENAME, "RemindStartupCheck", false),
	bRemindCheckVisible(MODULENAME, "RemindCheckVisible", false),

	iRemindState(MODULENAME, "RemindEnabled", REMIND_ALL),
	wRemindOffset(MODULENAME, "RemindOffset", 9),
	wRemindSoundOffset(MODULENAME, "RemindSoundOffset", 3),
	wRemindNotifyInterval(MODULENAME, "RemindNotifyInterval", 12),

	// Popup options
	bPopupEnabled(MODULENAME, "PopupEnabled", true),
	bPopupMsgbox(MODULENAME, "PopupMsgBox", false),
	bPopupProgress(MODULENAME, "PopupProgress", false),

	iPopupDelay(MODULENAME, "PopupDelay", 0),
	iAnnivClrType(MODULENAME, "PopupAnnivClrType", POPUP_COLOR_CUSTOM),
	iBirthClrType(MODULENAME, "PopupBirthClrType", POPUP_COLOR_CUSTOM),

	clrAback(MODULENAME, "PopupAnnivClrBack", RGB(90, 190, 130)),
	clrAtext(MODULENAME, "PopupAnnivClrText", RGB(0, 0, 0)),
	clrBback(MODULENAME, "PopupBirthClrBirthBack", RGB(192, 180, 30)),
	clrBtext(MODULENAME, "PopupBirthClrBirthText", RGB(0, 0, 0))
{}

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
static int OnTopToolBarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_USERINFO_SHOWDIALOG;
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_MAIN);
	ttb.name = ttb.pszTooltipUp = LPGEN("User &details");
	g_plugin.addTTB(&ttb);

	ttb.dwFlags = TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_USERINFO_REMINDER_LIST;
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_ANNIVERSARY);
	ttb.name = ttb.pszTooltipUp = LPGEN("Anniversary list");
	g_plugin.addTTB(&ttb);

	ttb.pszService = MS_USERINFO_REMINDER_CHECK;
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_BIRTHDAY);
	ttb.name = ttb.pszTooltipUp = LPGEN("Check anniversaries");
	g_plugin.addTTB(&ttb);
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
static int OnModulesLoaded(WPARAM, LPARAM)
{
	// create services to receive string lists of languages and timezones
	SvcConstantsLoadModule();

	// load module to remind user about birthday and a anniversary
	SvcReminderOnModulesLoaded();

	// load extended intagration services
	SvcFlagsOnModulesLoaded();

	// build contact's menuitems
	RebuildMenu();
	HookEvent(ME_CLIST_PREBUILDSTATUSMENU, (MIRANDAHOOK)RebuildAccount);
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	DlgContactInfoUnLoadModule();
	SvcReminderUnloadModule();

	// uninitialize classes
	CtrlContactUnLoadModule();
	CtrlButtonUnloadModule();

	SvcConstantsUnloadModule();
	SvcFlagsUnloadModule();

	mir_free(hMenuItemAccount);
	return 0;
}

//============================================================================================
//	plugin interface
//============================================================================================


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
int CMPlugin::Unload()
{
	FreeLibrary(hDwmApi);
	return 0;
}

/**
 * This function is called by Miranda to initialize the plugin.
 *
 * @return	0
 **/
int CMPlugin::Load()
{
	// init common controls
	INITCOMMONCONTROLSEX ccEx;
	ccEx.dwSize = sizeof(ccEx);
	ccEx.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES;
	InitCommonControlsEx(&ccEx);

	memset(&myGlobals, 0, sizeof(MGLOBAL));

	if (IsWinVerVistaPlus()) {
		hDwmApi = LoadLibraryA("dwmapi.dll");
		if (hDwmApi)
			dwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)GetProcAddress(hDwmApi, "DwmIsCompositionEnabled");
	}

	// load icon library
	IcoLib_LoadModule();

	SvcFlagsLoadModule();
	SvcTimezoneLoadModule();
	SvcRefreshContactInfoLoadModule();

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
	HookEvent(ME_OPT_INITIALISE, OnInitOptions);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_TTB_MODULELOADED, OnTopToolBarLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	return 0;
}
