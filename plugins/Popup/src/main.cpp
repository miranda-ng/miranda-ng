/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK

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

===============================================================================

File name      : $HeadURL: http://svn.miranda.im/mainrepo/popup/trunk/src/main.cpp $
Revision       : $Revision: 1652 $
Last change on : $Date: 2010-07-15 20:48:21 +0300 (Ð§Ñ‚, 15 Ð¸ÑŽÐ» 2010) $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"
#include <shellapi.h>
#include <malloc.h>

WORD SETTING_MAXIMUMWIDTH_MAX = GetSystemMetrics(SM_CXSCREEN);

#define MENUCOMMAND_HISTORY "PopUp/ShowHistory"
#define MENUCOMMAND_SVC "PopUp/EnableDisableMenuCommand"


//===== MessageBoxes =====
//void MB(char*); //This one is for Debug purposes
//void Log(char*); //This one is used to notify the user
//void HigherLower(int, int); //Used for integer input boxes (out of bound values).
//void NotifyError(DWORD, char*, int, char*); //Used to notify an error giving some error codes to report.

//===== Options =====
static int OptionsInitialize(WPARAM,LPARAM);

//===== Initializations =====
static int OkToExit(WPARAM,LPARAM);
bool OptionLoaded = false;
int hLangpack = 0;

//===== Global variables =====
//===== DLLs =====
HMODULE hUserDll	= 0;
HMODULE hMsimgDll	= 0;
HMODULE hKernelDll	= 0;
HMODULE hGdiDll		= 0;
HMODULE hDwmapiDll	= 0;

//===== User wnd class =====
GLOBAL_WND_CLASSES g_wndClass = {0};

//===== MTextControl ====
HANDLE htuText				= NULL;
HANDLE htuTitle				= NULL;

//===== Menu item =====
HANDLE hMenuRoot			= NULL;
HANDLE hMenuItem			= NULL;
HANDLE hMenuItemHistory		= NULL;

//==== ServiceFunctions Handles ====
HANDLE hShowHistory			= NULL;
HANDLE hTogglePopup			= NULL;
HANDLE hGetStatus			= NULL;

//===== Event Handles =====
HANDLE hOptionsInitialize;
//HANDLE hNotifyOptionsInitialize;		deprecatet
HANDLE hModulesLoaded;
HANDLE hTTBLoaded;
HANDLE hTBLoaded;
HANDLE hOkToExit;
HANDLE hIconsChanged, hFontsChanged;
HANDLE hEventStatusChanged; //To automatically disable on status change.
int hTTButton = -1;
HANDLE hTButton = NULL;

GLOBAL g_popup = {0};

static struct {
	char *name;
	INT_PTR (*func)(WPARAM, LPARAM);
	HANDLE handle;
} popupServices[] =
{
	{MS_POPUP_ADDPOPUP,				PopUp_AddPopUp,				0},
	{MS_POPUP_ADDPOPUPEX,			PopUp_AddPopUpEx,			0},
	{MS_POPUP_ADDPOPUPW,			PopUp_AddPopUpW,			0},
	{MS_POPUP_ADDPOPUP2,			PopUp_AddPopUp2,			0},

	{MS_POPUP_CHANGETEXT,			PopUp_ChangeText,			0},
	{MS_POPUP_CHANGETEXTW,			PopUp_ChangeTextW,			0},
	{MS_POPUP_CHANGE,				PopUp_Change,				0},
	{MS_POPUP_CHANGEW,				PopUp_ChangeW,				0},
	{MS_POPUP_CHANGEPOPUP2,			PopUp_Change2,				0},

	{MS_POPUP_GETCONTACT,			PopUp_GetContact,			0},
	{MS_POPUP_GETPLUGINDATA,		PopUp_GetPluginData,		0},
	{MS_POPUP_ISSECONDLINESHOWN,	PopUp_IsSecondLineShown,	0},
	
	{MS_POPUP_SHOWMESSAGE,			PopUp_ShowMessage,			0},
	{MS_POPUP_SHOWMESSAGEW,			PopUp_ShowMessageW,			0},
	{MS_POPUP_QUERY,				PopUp_Query,				0},

	{MS_POPUP_REGISTERACTIONS,		PopUp_RegisterActions,		0},
	{MS_POPUP_REGISTERNOTIFICATION,	PopUp_RegisterNotification,	0},

	{MS_POPUP_UNHOOKEVENTASYNC,		PopUp_UnhookEventAsync,		0},

	{MS_POPUP_REGISTERVFX,			PopUp_RegisterVfx,			0},

	{MS_POPUP_REGISTERCLASS,		PopUp_RegisterPopupClass,	0},
	{MS_POPUP_ADDPOPUPCLASS,		PopUp_CreateClassPopup,		0},

};

//===== Options pages =====
static int OptionsInitialize(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize			= sizeof(odp);
	odp.hInstance		= hInst;
	odp.position		= 100000000;
	odp.groupPosition	= 910000000;
	odp.flags			= ODPF_TCHAR | ODPF_BOLDGROUPS;
	odp.ptszTitle		= LPGENT(MODULNAME_PLU);

	odp.ptszTab			= LPGENT("General");
	odp.pfnDlgProc		= DlgProcPopUpGeneral;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_POPUP_GENERAL);
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab			= LPGENT("Classes");
	odp.pfnDlgProc		= DlgProcOptsClasses;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_NOTIFICATIONS);
	CallService( MS_OPT_ADDPAGE, wParam,(LPARAM)&odp );

	odp.ptszTab			= LPGENT("Actions");
	odp.pfnDlgProc		= DlgProcPopupActions;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_ACTIONS);
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab			= LPGENT("Contacts");
	odp.pfnDlgProc		= DlgProcContactOpts;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_CONTACTS);
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszTab			= LPGENT("Advanced");
	odp.pfnDlgProc		= DlgProcPopUpAdvOpts;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_POPUP_ADVANCED);
	odp.flags			|= ODPF_EXPERTONLY;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	odp.ptszGroup		= LPGENT("Skins");
	odp.ptszTab			= LPGENT(MODULNAME_PLU);
	odp.pfnDlgProc		= DlgProcPopSkinsOpts;
	odp.pszTemplate		= MAKEINTRESOURCEA(IDD_OPT_SKIN2);
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	//Test page
	//odp.ptszTab			= LPGEN("General (old)");
	//odp.pfnDlgProc		= DlgProcPopUpOpts;
	//odp.pszTemplate		= MAKEINTRESOURCE(IDD_OPT_POPUP);
	//CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}

static int FontsChanged(WPARAM wParam,LPARAM lParam)
{
	ReloadFonts();
	return 0;
}

static int IconsChanged(WPARAM wParam,LPARAM lParam){
	LoadActions();

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);

	if (PopUpOptions.ModuleIsEnabled == TRUE) { //The module is enabled.
		//The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_ON,0);
	}
	else { //The module is disabled.
		//The action to do is enable popups (show enabled), then write "disable popup" in the new item.
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_OFF,0);
	}
	mi.flags = CMIM_ICON;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItem,(LPARAM)&mi);
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuRoot,(LPARAM)&mi);

	mi.hIcon = IcoLib_GetIcon(ICO_HISTORY,0);
	mi.flags = CMIM_ICON;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItemHistory,(LPARAM)&mi);

	if (hTTButton >= 0) {
		TTBButtonV2 btn = {0};
		btn.cbSize = sizeof(btn);
		btn.pszServiceUp = btn.pszServiceDown = MENUCOMMAND_SVC;
		btn.lParamUp = 1;
		btn.lParamDown = 0;
		btn.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
		btn.name = Translate("Toggle Popups");
		btn.tooltipUp = Translate("Popups are disabled");
		btn.tooltipDn = Translate("Popups are enabled");
		btn.hIconUp = IcoLib_GetIcon(ICO_POPUP_OFF,0);
		btn.hIconDn = IcoLib_GetIcon(ICO_POPUP_ON,0);
		hTTButton = CallService(MS_TTB_SETBUTTONOPTIONS, MAKEWPARAM(TTBO_ALLDATA, hTTButton), (LPARAM)&btn);
	}

	return 0;
}

static int TTBLoaded(WPARAM wParam,LPARAM lParam){
	if (hTTButton < 0) {
		TTBButtonV2 btn		= {0};
		btn.cbSize			= sizeof(btn);
		btn.pszServiceUp	= btn.pszServiceDown = MENUCOMMAND_SVC;
		btn.lParamUp		= 1;
		btn.lParamDown		= 0;
		btn.dwFlags			= TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
		btn.name			= Translate("Toggle Popups");
		btn.tooltipUp		= Translate("Popups are disabled");
		btn.tooltipDn		= Translate("Popups are enabled");
		btn.hIconUp			= IcoLib_GetIcon(ICO_POPUP_OFF,0);
		btn.hIconDn			= IcoLib_GetIcon(ICO_POPUP_ON,0);
		hTTButton = CallService(MS_TTB_ADDBUTTON, (WPARAM)&btn, 0);
	}
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, PopUpOptions.ModuleIsEnabled?TTBST_RELEASED:TTBST_PUSHED);
	return 0;
}

//register Modern Toolbarbutton
static int ToolbarSet(WPARAM, LPARAM){
	if (hTButton == NULL){
		TBButton tbb				= {0};
		tbb.cbSize					= sizeof(TBButton);
		tbb.pszButtonID				= "PopupToogle";
		tbb.pszButtonName			= Translate("Toggle Popups");
		tbb.pszServiceName			= MENUCOMMAND_SVC;
		tbb.pszTooltipUp			= Translate("Popups are disabled");
		tbb.pszTooltipDn			= Translate("Popups are enabled");
		tbb.hPrimaryIconHandle		= (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)ICO_TB_POPUP_OFF);
		tbb.hSecondaryIconHandle	= (HANDLE)CallService(MS_SKIN2_GETICONHANDLE, 0, (LPARAM)ICO_TB_POPUP_ON);
		tbb.tbbFlags				= TBBF_VISIBLE;
		tbb.defPos					= 10000;
		hTButton = (HANDLE)CallService(MS_TB_ADDBUTTON,0, (LPARAM)&tbb);
	}
	CallService(MS_TB_SETBUTTONSTATEBYID, (WPARAM)"PopupToogle", PopUpOptions.ModuleIsEnabled?TBST_PUSHED:TBST_RELEASED);
	return 0;
}

//===== EnableDisableMenuCommand =====
INT_PTR svcEnableDisableMenuCommand(WPARAM wp, LPARAM lp)
{
	int iResult = 0;
	int iResultRoot = 0;
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);

	if (PopUpOptions.ModuleIsEnabled)
	{ //The module is enabled.
		//The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		PopUpOptions.ModuleIsEnabled = FALSE;
		DBWriteContactSettingByte(NULL, MODULNAME, "ModuleIsEnabled", FALSE);
		mi.ptszName = LPGENT("Enable &popup module");
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_OFF,0);
	} else
	{ //The module is disabled.
		//The action to do is enable popups (show enabled), then write "disable popup" in the new item.
		PopUpOptions.ModuleIsEnabled = TRUE;
		DBWriteContactSettingByte(NULL, MODULNAME, "ModuleIsEnabled", TRUE);
		mi.ptszName = LPGENT("Disable &popup module");
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_ON,0);
	}
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	iResult = CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuItem,(LPARAM)&mi);
	mi.flags = CMIM_ICON;
	iResultRoot = CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenuRoot,(LPARAM)&mi);
	TTBLoaded(0,0);
	ToolbarSet(0,0);
	if(iResult && iResultRoot)
		return 1;
	else
		return 0;
}

INT_PTR svcShowHistory(WPARAM, LPARAM)
{
	PopupHistoryShow();
	return 0;
}

void InitMenuItems(void) {
	CLISTMENUITEM mi={0};

	// Common
	mi.cbSize = sizeof(mi);
	// support new genmenu style
	mi.flags		= CMIF_ROOTHANDLE|CMIF_TCHAR;
	mi.hParentMenu	= HGENMENU_ROOT;

	// Build main menu
	mi.position		= -1000000000 /*1000001*/;
	mi.ptszName		= LPGENT(MODULNAME_PLU);
	mi.hIcon		= IcoLib_GetIcon(PopUpOptions.ModuleIsEnabled ? ICO_POPUP_ON : ICO_POPUP_OFF ,0);
	hMenuRoot		= (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

	// Add item to main menu
	mi.hParentMenu		= (HGENMENU)hMenuRoot;

	hTogglePopup = CreateServiceFunction(MENUCOMMAND_SVC, svcEnableDisableMenuCommand);
	mi.ptszName			= PopUpOptions.ModuleIsEnabled ? LPGENT("Disable &popup module") : LPGENT("Enable &popup module");
	mi.pszService		= MENUCOMMAND_SVC;
	hMenuItem			= (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, (WPARAM)0, (LPARAM)&mi);

	// Popup History
	hShowHistory = CreateServiceFunction(MENUCOMMAND_HISTORY, svcShowHistory);
	mi.position			= 1000000000;
	mi.popupPosition	= 1999990000;
	mi.ptszName			= LPGENT("Popup History");
	mi.hIcon			= IcoLib_GetIcon(ICO_HISTORY, 0);
	mi.pszService		= MENUCOMMAND_HISTORY;
	hMenuItemHistory	= (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, (WPARAM)0, (LPARAM)&mi);

}

//===== GetStatus =====
INT_PTR GetStatus(WPARAM wp, LPARAM lp)
{
	return PopUpOptions.ModuleIsEnabled;
}

//register Updatersupport
void registerUpdate(){	
	Update update = {0};
	char szVersion[16];
	update.cbSize				= sizeof(Update);
	update.szComponentName		= pluginInfoEx.shortName;
	update.pbVersion			= (BYTE *)CreateVersionStringPluginEx(&pluginInfoEx, szVersion);
	update.cpbVersion			= (int)strlen((char *)update.pbVersion);

	update.szUpdateURL			= __FLUpdateURL /*UPDATER_AUTOREGISTER*/;
	update.szVersionURL			= __FLVersionURL;
	update.pbVersionPrefix		= (BYTE *)__FLVersionPrefix;
	update.cpbVersionPrefix		= (int)strlen((char *)update.pbVersionPrefix);

	update.szBetaUpdateURL		= __BetaUpdateURL;
	update.szBetaVersionURL		= __BetaVersionURL;
	// bytes occuring in VersionURL before the version, used to locate the version information within the URL data
	update.pbBetaVersionPrefix	= (BYTE *)__BetaVersionPrefix;
	update.cpbBetaVersionPrefix	= (int)strlen((char *)update.pbBetaVersionPrefix);

	update.szBetaChangelogURL	= __BetaChangelogURL;

	CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);
}

//register Hotkey
void LoadHotkey(){	
	HOTKEYDESC hk        = {0};
	hk.cbSize            = sizeof(hk);
	hk.dwFlags			 = HKD_TCHAR;
	hk.pszName            = "Toggle Popups";
	hk.ptszDescription    = LPGENT("Toggle Popups");
	hk.ptszSection        = LPGENT(MODULNAME_PLU);
	hk.pszService        = MENUCOMMAND_SVC;
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM) &hk);

	// 'Popup History' Hotkey
	hk.pszName			= "Popup History";
	hk.ptszDescription	= LPGENT("Popup History");
	hk.pszService		= MENUCOMMAND_HISTORY;
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM) &hk);
}

//menu
//Function which makes the initializations
static int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	//check if 'Icon Library Manager' and 'FontService' exist (load icon and font later)
	if (!ServiceExists(MS_FONT_REGISTER) || !ServiceExists(MS_SKIN2_ADDICON)) {
		LPTSTR msg = TranslateTS(
			_T("This version of Popup Plus requires\r\n")
			_T("'Icon Library Manager' and 'FontService'\r\n")
			_T("modules.\r\n")
			_T("\r\n")
			_T("You always can download them at\r\n")
			_T("http://addons.miranda-im.org/\r\n")
			_T("\r\n")
			_T("Do you want to visit Miranda IM homepage now?\r\n") );
		if (MessageBox(NULL, msg, _T("Popup Plus Error"), MB_YESNO|MB_ICONSTOP) == IDYES)
			ShellExecute(NULL, _T("open"), _T("http://addons.miranda-im.org/"), NULL, NULL, SW_SHOWNORMAL);
		return 0;
	}

	//check if History++ is installed
	gbHppInstalled = ServiceExists(MS_HPP_GETVERSION) && ServiceExists(MS_HPP_EG_WINDOW) &&
		(CallService(MS_HPP_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(1,5,0,112));
	//check if MText plugin is installed
	if (MText.Register) {
		htuText		= MText.Register("PopUp Plus/Text", MTEXT_FANCY_DEFAULT);
		htuTitle	= MText.Register("PopUp Plus/Title",MTEXT_FANCY_DEFAULT);
	}
	else {
		htuTitle = htuText = NULL;
	}
	// init meta contacts
	INT_PTR ptr = CallService(MS_MC_GETPROTOCOLNAME, 0, 0);
	if (ptr != CALLSERVICE_NOTFOUND) {
		gszMetaProto = (LPCSTR)ptr;
	}

	//check if OptionLoaded
	if(!OptionLoaded){
		LoadOptions();
	}
/*/deprecatet stuff
	notifyLink = ServiceExists(MS_NOTIFY_GETLINK) ? (MNOTIFYLINK *)CallService(MS_NOTIFY_GETLINK, 0, 0) : NULL;
	LoadNotifyImp();
	hNotifyOptionsInitialize = HookEvent(ME_NOTIFY_OPT_INITIALISE, NotifyOptionsInitialize);

	HookEvent(ME_CONTACTSETTINGS_INITIALISE, ContactSettingsInitialise);
*/
	//Uninstalling purposes
	if (ServiceExists("PluginSweeper/Add")) {
		CallService("PluginSweeper/Add",(WPARAM)Translate(MODULNAME),(LPARAM)MODULNAME);
	}
	//load fonts / create hook
	InitFonts();
	hFontsChanged = HookEvent(ME_FONT_RELOAD,FontsChanged);

	//load actions and notifications
	LoadActions();
	LoadNotifications();
	//hook TopToolBar
	hTTBLoaded = HookEvent(ME_TTB_MODULELOADED, TTBLoaded);
	//Folder plugin support
	LPTSTR pszPath = mir_a2t(MIRANDA_PATH "\\Skins\\PopUp");
	folderId = FoldersRegisterCustomPathT(MODULNAME_LONG, "Skins", pszPath);
	mir_free(pszPath);
	//load skin
	skins.load(_T("dir"));
	const PopupSkin *skin;
	if (skin = skins.getSkin(PopUpOptions.SkinPack)) {
		mir_free(PopUpOptions.SkinPack);
		PopUpOptions.SkinPack = mir_tstrdup(skin->getName());
		skin->loadOpts();
	}
	//init PopupEfects
	PopupEfectsInitialize();
	//MessageAPI support
	SrmmMenu_Load();
	//Hotkey
	LoadHotkey();
	//Modern Toolbar support
	if(ServiceExists(MS_TB_ADDBUTTON)) {
		hTBLoaded = HookEvent(ME_TB_MODULELOADED, ToolbarSet);
		ToolbarSet(0,0);
	}
	//Updater support
	if(ServiceExists(MS_UPDATE_REGISTER)) registerUpdate();

	gbPopupLoaded = TRUE;
	return 0;
}

//=== DllMain =====
//DLL entry point, Required to store the instance handle
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

//===== MirandaPluginInfo =====
//Called by Miranda to get the information associated to this plugin.
//It only returns the PLUGININFOEX structure, without any test on the version
//@param mirandaVersion - The version of the application calling this function
MIRAPI PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_popup.MirVer = mirandaVersion;

#if defined(_UNICODE)
	pluginInfoEx.flags = UNICODE_AWARE;
#else
	if (GetProcAddress(GetModuleHandle(_T("user32")), "DrawTextExW"))
		pluginInfoEx.flags = 1; // dynamic UNICODE_AWARE
#endif

	return &pluginInfoEx;
}

//Miranda PluginInterfaces
MIRAPI const MUUID *MirandaPluginInterfaces(void)
{
	static const MUUID interfaces[] = { MIID_POPUPS, MIID_LAST };
	return interfaces;
}

//ME_SYSTEM_OKTOEXIT event
//called before the app goes into shutdown routine to make sure everyone is happy to exit
static int OkToExit(WPARAM wParam, LPARAM lParam)
{
	closing = TRUE;
	StopPopupThread();
	return 0;
}

//===== Load =====
//Initializes the services provided and the link to those needed
//Called when the plugin is loaded into Miranda
MIRAPI int Load(PLUGINLINK *link)
{
	char ver[1024];
	pluginLink=link;

	g_popup.isOsUnicode = (GetVersion() & 0x80000000) == 0;

	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM) sizeof(ver), (LPARAM) ver);
	g_popup.isMirUnicode = strstr(ver, "Unicode") != NULL;

	hGetStatus = CreateServiceFunction(MS_POPUP_GETSTATUS, GetStatus);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);

	mir_getLI	(&li);
	mir_getMMI	(&mmi);
	mir_getUTFI	(&utfi);
	mir_getMTI	(&MText);
	mir_getLP(&pluginInfoEx);

	#if defined(_DEBUG)
		PopUpOptions.debug = DBGetContactSettingByte(NULL, MODULNAME, "debug", FALSE);
	#else
		PopUpOptions.debug = false;
	#endif
	LoadGDIPlus();

	//Transparent and animation routines
	OSVERSIONINFO osvi = { 0 };
	BOOL bResult = FALSE;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bResult = GetVersionEx(&osvi);

// MLU layer for ansi release
#if !defined(_UNICODE)
	MySetLayeredWindowAttributes = 0;
	MyUpdateLayeredWindow = 0;
	MyAnimateWindow = 0;
	MyDrawTextW = 0;
	MyDrawTextExW = 0;
	MySetWindowTextW = 0;
	MySendMessageW = 0;
	MyCallWindowProcW = 0;
	MyCreateWindowExW = 0;

	MyGetMonitorInfo = 0;
	MyMonitorFromWindow = 0;

	hUserDll = LoadLibrary(_T("user32.dll"));
	if (hUserDll) {
		MySetLayeredWindowAttributes = (BOOL (WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
		MyUpdateLayeredWindow = (BOOL (WINAPI *)(HWND, HDC, POINT *, SIZE *, HDC, POINT *, COLORREF, BLENDFUNCTION *, DWORD))GetProcAddress(hUserDll, "UpdateLayeredWindow");
		MyAnimateWindow = (BOOL (WINAPI*)(HWND,DWORD,DWORD))GetProcAddress(hUserDll,"AnimateWindow");
		MyDrawTextW = (int (WINAPI *)(HDC, LPCWSTR, int, LPRECT, UINT))GetProcAddress(hUserDll,"DrawTextW");
		MyDrawTextExW = (int (WINAPI*)(HDC,LPCWSTR,int,LPRECT,UINT,LPDRAWTEXTPARAMS))GetProcAddress(hUserDll,"DrawTextExW");
		MySetWindowTextW = (BOOL (WINAPI*)(HWND, LPCWSTR))GetProcAddress(hUserDll,"SetWindowTextW");
		MySendMessageW = (LRESULT (WINAPI *)(HWND, UINT, WPARAM, LPARAM))GetProcAddress(hUserDll,"SendMessageW");
		MyCallWindowProcW = (LRESULT (WINAPI  *)(WNDPROC, HWND, UINT, WPARAM, LPARAM))GetProcAddress(hUserDll,"CallWindowProcW");
		MyCreateWindowExW = (HWND (WINAPI*)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID))GetProcAddress(hUserDll,"CreateWindowExW");

		if (LOWORD(GetVersion())!=4) { //Windows 98, ME, 2000, XP, and later support multimonitor configuration.
			if (bResult) {
				if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
				{ //2000 or XP
					#ifdef UNICODE
						MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUserDll,("GetMonitorInfoW"));
					#else
						MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUserDll,("GetMonitorInfoA"));
					#endif //UNICODE
				} else
				{ //98 or ME
					// attempt to fix multimonitor on 9x
					MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUserDll,("GetMonitorInfoA"));
					if (!MyGetMonitorInfo)
						MyGetMonitorInfo = (BOOL (WINAPI*)(HMONITOR,LPMONITORINFO))GetProcAddress(hUserDll,("GetMonitorInfo"));
				}
			} //There's no need for an else branch.
			if (MyGetMonitorInfo)
				MyMonitorFromWindow = (HMONITOR (WINAPI*)(HWND,DWORD))GetProcAddress(hUserDll, ("MonitorFromWindow"));
		}
	}

	hGdiDll = LoadLibrary(_T("gdi32.dll"));
	if (hGdiDll) {
		MyGetTextExtentPoint32W = (BOOL (WINAPI *)(HDC, LPCWSTR, int, LPSIZE))GetProcAddress(hGdiDll,"GetTextExtentPoint32W");
	}
	else{
		MyGetTextExtentPoint32W = 0;
	}
	hMsimgDll = LoadLibrary(_T("msimg32.dll"));
	MyAlphaBlend = 0;
	MyTransparentBlt = 0;
	if (hMsimgDll)
	{
		MyTransparentBlt = (BOOL (WINAPI *)(HDC, int, int, int, int, HDC, int, int, int, int, UINT))
			GetProcAddress(hMsimgDll, "TransparentBlt");
		MyAlphaBlend = (BOOL (WINAPI *)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION))
			GetProcAddress(hMsimgDll, "AlphaBlend");
	}
#endif

	hDwmapiDll = LoadLibrary(_T("dwmapi.dll"));
	MyDwmEnableBlurBehindWindow = 0;
	if (hDwmapiDll) {
		MyDwmEnableBlurBehindWindow = (HRESULT (WINAPI *)(HWND, DWM_BLURBEHIND *))
			GetProcAddress(hDwmapiDll, "DwmEnableBlurBehindWindow");
	}

	PopupHistoryLoad();
	LoadPopupThread();
	if (!LoadPopupWnd2())
	{
		MessageBox(0, TranslateTS(
			_T("Error: I could not register the PopUp Window class.\r\n")
			_T("The plugin will not operate.")
			),
			_T(MODULNAME_LONG), MB_ICONSTOP|MB_OK);
		return 0; //We couldn't register our Window Class, don't hook any event: the plugin will act as if it was disabled.
	}
	RegisterOptPrevBox();

	// Register in DBEditor++
	DBVARIANT dbv;
	if (DBGetContactSetting(NULL, "KnownModules", MODULNAME, &dbv))
		DBWriteContactSettingString(NULL, "KnownModules", pluginInfoEx.shortName, MODULNAME);
	DBFreeVariant(&dbv);
	
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	hOkToExit = HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);
//	hEventStatusChanged = HookEvent(ME_CLIST_STATUSMODECHANGE,StatusModeChanged);

	hbmNoAvatar = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_NOAVATAR));

	if(!OptionLoaded){
		LoadOptions();
	}

	//Service Functions
	for (int i = SIZEOF(popupServices); i--; )
		popupServices[i].handle = CreateServiceFunction(popupServices[i].name, popupServices[i].func);

	//load icons / create hook
	InitIcons();
	hIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED,IconsChanged);
	//add menu items
	InitMenuItems();

	return 0;
}

//===== Unload =====
//Prepare the plugin to stop
//Called by Miranda when it will exit or when the plugin gets deselected

MIRAPI int Unload(void)
{
	int i;

	for (i = SIZEOF(popupServices); i--; )
		DestroyServiceFunction(popupServices[i].handle);

	SrmmMenu_Unload();

	UnhookEvent(hOptionsInitialize);
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hOkToExit);
	UnhookEvent(hEventStatusChanged);
	UnhookEvent(hIconsChanged);
	UnhookEvent(hFontsChanged);
	UnhookEvent(hTBLoaded);

	DestroyServiceFunction(hShowHistory);
	DestroyServiceFunction(hTogglePopup);
	DestroyServiceFunction(hGetStatus);
	DestroyServiceFunction(hSquareFad);

	DeleteObject(fonts.title);
	DeleteObject(fonts.clock);
	DeleteObject(fonts.text);
	DeleteObject(fonts.action);
	DeleteObject(fonts.actionHover);

	DeleteObject(hbmNoAvatar);

	FreeLibrary(hDwmapiDll);
	FreeLibrary(hUserDll);
	FreeLibrary(hMsimgDll);
//	FreeLibrary(hKernelDll);
	FreeLibrary(hGdiDll);

	if(PopUpOptions.SkinPack) mir_free(PopUpOptions.SkinPack);
	mir_free(PopUpOptions.Effect);

	OptAdv_UnregisterVfx();
	UnloadPopupThread();
	UnloadPopupWnd2();
	PopupHistoryUnload();

	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupWnd2),hInst);
	UnregisterClassW(L"PopupEditBox",hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupMenuHostWnd),hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupThreadManagerWnd),hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupPreviewBoxWndclass),hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupPlusDlgBox),hInst);

	UnloadGDIPlus();

	UnloadActions();

	CloseHandle(hMainThread);

	return 0;
}
