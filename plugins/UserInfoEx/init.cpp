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

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/init.cpp $
Revision       : $Revision: 210 $
Last change on : $Date: 2010-10-02 22:27:36 +0400 (Сб, 02 окт 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/
#include "commonheaders.h"
#include "res/version.h"
#include "m_updater.h"

#include "mir_menuitems.h"
#include "ctrl_base.h"
#include "ctrl_button.h"
#include "ctrl_contact.h"
#include "dlg_propsheet.h"
#include "dlg_anniversarylist.h"
#include "psp_options.h"
#include "ex_import/svc_ExImport.h"
//#include "ex_import/svc_ExImVCF.h"
#include "svc_avatar.h"
#include "svc_contactinfo.h"
#include "svc_email.h"
#include "svc_gender.h"
#include "svc_homepage.h"
#include "svc_phone.h"
#include "svc_refreshci.h"
#include "svc_reminder.h"
#include "svc_timezone.h"
#include "svc_timezone_old.h"
#include "flags/svc_flags.h"

static PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	__VERSION_DWORD,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	MIID_UIUSERINFOEX
};

static HANDLE ghModulesLoadedHook		= NULL;
static HANDLE ghTopToolBarLoaded		= NULL;
static HANDLE ghModernToolBarLoaded		= NULL;
static HANDLE ghShutdownHook			= NULL;
static HANDLE ghPrebuildStatusMenu		= NULL;
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
static INT OnTopToolBarLoaded(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(ghTopToolBarLoaded);
	DlgAnniversaryListOnTopToolBarLoaded();
	SvcReminderOnTopToolBarLoaded();
	return 0;
}

/**
 * This function is called by the ME_TB_MODULELOADED event.
 * It adds a set of buttons to the Toolbar of the Modern Contact List.
 *
 * @param	wParam	- not used
 * @param	lParam	- not used
 *
 * @return	always 0
 **/
static INT OnModernToolBarLoaded(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(ghModernToolBarLoaded);
	DlgAnniversaryListOnToolBarLoaded();
	SvcReminderOnToolBarLoaded();
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
static INT OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	INT_PTR ptr;

	UnhookEvent(ghModulesLoadedHook);

	myGlobals.HaveCListExtraIcons		= ServiceExists(MS_CLIST_EXTRA_SET_ICON);
	myGlobals.ExtraIconsServiceExist	= ServiceExists(MS_EXTRAICON_REGISTER);
	myGlobals.PopUpActionsExist			= ServiceExists(MS_POPUP_REGISTERACTIONS);
	myGlobals.MsgAddIconExist			= ServiceExists(MS_MSG_ADDICON);

	// init meta contacts
	ptr = CallService(MS_MC_GETPROTOCOLNAME, 0, 0);
	myGlobals.szMetaProto = (ptr != CALLSERVICE_NOTFOUND) ? (LPCSTR)ptr : NULL;

	// options
	OptionsLoadModule();
	// create services to receive string lists of languages and timezones
	SvcConstantsLoadModule();
	// load module to remind user about birthday and a anniversary
	SvcReminderOnModulesLoaded();
	// load extended intagration services
	SvcEMailOnModulesLoaded();
	SvcHomepageLoadModule();
	SvcPhoneLoadModule();
	SvcGenderLoadModule();
	SvcFlagsOnModulesLoaded();

#ifdef _DEBUG // new feature, not in release jet
	NServices::NAvatar::OnModulesLoaded();
#endif

	// build contact's menuitems
	RebuildMenu();
	ghPrebuildStatusMenu = HookEvent( ME_CLIST_PREBUILDSTATUSMENU, (MIRANDAHOOK)RebuildAccount);

	// finally register for updater
	if (ServiceExists(MS_UPDATE_REGISTER))
	{
		Update update = {0};
		CHAR szVersion[16];
		update.cbSize			= sizeof(Update);
		update.szComponentName	= pluginInfo.shortName;
		update.pbVersion		= (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion		= (INT)strlen((LPSTR)update.pbVersion);

		update.szUpdateURL		= UPDATER_AUTOREGISTER;

		update.szBetaVersionURL		= "http://userinfoex.googlecode.com/svn/trunk/changelog.txt";
		// bytes occuring in VersionURL before the version, used to locate the version information within the URL data
		// e.g. change '[0.8.1.0 (in work)]' to 'UserinfoEx: 0.8.1.0' for beta versions bump
		update.pbBetaVersionPrefix	= (BYTE *)"UserinfoEx: ";
		update.cpbBetaVersionPrefix	= (INT)strlen((LPSTR)update.pbBetaVersionPrefix);

#ifdef _WIN64

		update.szBetaUpdateURL		= "http://userinfoex.googlecode.com/files/uinfoex64.zip";

#else

		update.szBetaUpdateURL		= "http://userinfoex.googlecode.com/files/uinfoexW.zip";

#endif

		// url for displaying changelog for beta versions
		update.szBetaChangelogURL	 = "http://userinfoex.googlecode.com/svn/trunk/changelog.txt";

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}
	// install known modules strings to database
	DB::Setting::WriteAString(NULL, "KnownModules", MODULELONGNAME, USERINFO","MODNAME","MOD_MBIRTHDAY","MODNAMEFLAGS);

	return 0;
}

static INT OnShutdown(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(ghShutdownHook);
	services.destroy();
	DlgContactInfoUnLoadModule();
	SvcReminderUnloadModule();

	// uninitialize classes
	CtrlContactUnLoadModule();
	CtrlButtonUnloadModule();

	SvcConstantsUnloadModule();
	UnhookEvent(ghPrebuildStatusMenu);
	SvcEMailUnloadModule();
	SvcFlagsUnloadModule();
	SvcGenderUnloadModule();
	SvcHomepageUnloadModule();
	SvcPhoneUnloadModule();

	mir_free(hMenuItemAccount);
	return 0;
}

static BOOL CoreCheck()
{
	BOOL	bOk = TRUE;
	CHAR	szVer[260];
	TCHAR	tszExePath[1024];

	GetModuleFileName(GetModuleHandle(NULL), tszExePath, SIZEOF(tszExePath));
	CallService(MS_SYSTEM_GETVERSIONTEXT, SIZEOF(szVer), (LPARAM)szVer);

	strlwr(szVer);
	_tcslwr(tszExePath);


	bOk *= (GetVersion() & 0x80000000) == 0;
	bOk *= strstr(szVer, "unicode") != 0;


	bOk *= _tcsstr(_tcsrchr(tszExePath, '\\'), _T("miranda")) != 0;
	bOk *= !strstr(szVer, "coffee") && strncmp(szVer, "1.", 2) && !strstr(szVer, " 1.");
	bOk *= myGlobals.mirandaVersion < PLUGIN_MAKE_VERSION(1,0,0,0);
	return bOk;
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
extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	static const MUUID interfaces[] = {
		MIID_UIUSERINFOEX,		// this is just me
		MIID_UIUSERINFO,		// replace the default userinfo module
		MIID_CONTACTINFO,		// indicate, that MS_CONTACT_GETCONTACTINFO service is provided
		MIID_REMINDER,			// indicate an Reminder of being provided
		MIID_SREMAIL,			// Send/Receive E-Mail service is provided
		MIID_LAST
	};
	return interfaces;
}

/**
 * This function is called by Miranda just to make it possible to unload some memory, ...
 *
 * @return	0
 **/
extern "C" INT __declspec(dllexport) Unload(VOID)
{
	return 0;
}

/**
 * This function is called by Miranda to initialize the plugin.
 *
 * @return	0
 **/
extern "C" INT __declspec(dllexport) Load(void)
{
	INITCOMMONCONTROLSEX ccEx;


	mir_getLP(&pluginInfo);
	if (CoreCheck())
	{
		// init common controls
		ccEx.dwSize = sizeof(ccEx);
		ccEx.dwICC = ICC_WIN95_CLASSES|ICC_DATE_CLASSES;
		InitCommonControlsEx(&ccEx);

		ZeroMemory(&myGlobals, sizeof(MGLOBAL));

		// init clist interface
		pcli = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)0);

		// init new miranda timezone interface
		mir_getTMI(&tmi);

		// init freeimage interface
		INT_PTR result = CALLSERVICE_NOTFOUND;
		if(ServiceExists(MS_IMG_GETINTERFACE))
			result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&FIP);

		if(FIP == NULL || result != S_OK) {
			MessageBoxEx(NULL, TranslateT("Fatal error, image services not found. Flags Module will be disabled."), _T("Error"), MB_OK | MB_ICONERROR | MB_APPLMODAL, 0);
			return 1;
		}

		if (IsWinVerVistaPlus())
		{
			HMODULE hDwmApi = LoadLibraryA("dwmapi.dll");
			if (hDwmApi)
				dwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)GetProcAddress(hDwmApi,"DwmIsCompositionEnabled");
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
		ghModulesLoadedHook		= HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
		ghTopToolBarLoaded		= HookEvent(ME_TTB_MODULELOADED, OnTopToolBarLoaded);
		ghModernToolBarLoaded	= HookEvent(ME_TB_MODULELOADED, OnModernToolBarLoaded);
		ghShutdownHook			= HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
		return 0;
	}
	return 1;
}

/**
 * Windows needs it for loading.
 *
 * @return	TRUE
 **/
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			ghInst = hinst;
			break;
	}
	return TRUE;
}
