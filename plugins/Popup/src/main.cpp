/*
Popup Plus plugin for Miranda IM

Copyright	� 2002 Luca Santarelli,
			� 2004-2007 Victor Pavlychko
			� 2010 MPK

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
Last change on : $Date: 2010-07-15 20:48:21 +0300 (Чт, 15 июл 2010) $
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
HANDLE hModulesLoaded;
HANDLE hOkToExit;
HANDLE hIconsChanged, hFontsChanged;
HANDLE hEventStatusChanged; //To automatically disable on status change.
HANDLE hTTButton = NULL;

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
	odp.cbSize      = sizeof(odp);
	odp.position      = 100000000;
	odp.groupPosition = 910000000;
	odp.hInstance     = hInst;
	odp.flags         = ODPF_BOLDGROUPS;
	odp.pszTitle      = MODULNAME_PLU;

	odp.pszTab      = LPGEN("General");
	odp.pfnDlgProc  = DlgProcPopUpGeneral;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_GENERAL);
	Options_AddPage(wParam, &odp);

	odp.pszTab      = LPGEN("Classes");
	odp.pfnDlgProc  = DlgProcOptsClasses;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_NOTIFICATIONS);
	Options_AddPage(wParam, &odp);

	odp.pszTab      = LPGEN("Actions");
	odp.pfnDlgProc  = DlgProcPopupActions;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ACTIONS);
	Options_AddPage(wParam, &odp);

	odp.pszTab      = LPGEN("Contacts");
	odp.pfnDlgProc  = DlgProcContactOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTACTS);
	Options_AddPage(wParam, &odp);

	odp.pszTab     = LPGEN("Advanced");
	odp.pfnDlgProc  = DlgProcPopUpAdvOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_ADVANCED);
	Options_AddPage(wParam, &odp);

	odp.pszGroup    = LPGEN("Skins");
	odp.pszTab      = LPGEN(MODULNAME_PLU);
	odp.pfnDlgProc  = DlgProcPopSkinsOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN2);
	Options_AddPage(wParam, &odp);
	return 0;
}

static int FontsChanged(WPARAM wParam,LPARAM lParam)
{
	ReloadFonts();
	return 0;
}

static int IconsChanged(WPARAM wParam,LPARAM lParam)
{
	LoadActions();

	CLISTMENUITEM mi = { sizeof(mi) };
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
	return 0;
}

static int TTBLoaded(WPARAM wParam,LPARAM lParam)
{
	if ( !hTTButton) {
		TTBButton btn = {0};
		btn.cbSize = sizeof(btn);
		btn.pszService = MENUCOMMAND_SVC;
		btn.lParamUp = 1;
		btn.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
		btn.name = LPGEN("Toggle Popups");
		btn.hIconHandleUp = Skin_GetIconHandle(ICO_TB_POPUP_OFF);
		btn.hIconHandleDn = Skin_GetIconHandle(ICO_TB_POPUP_ON);
		btn.pszTooltipUp = LPGEN("Enable popups");
		btn.pszTooltipDn = LPGEN("Disable popups");
		hTTButton = TopToolbar_AddButton(&btn);
	}

	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, (PopUpOptions.ModuleIsEnabled) ? TTBST_PUSHED : TTBST_RELEASED);
	return 0;
}

//===== EnableDisableMenuCommand =====
INT_PTR svcEnableDisableMenuCommand(WPARAM wp, LPARAM lp)
{
	int iResult = 0;
	int iResultRoot = 0;

	CLISTMENUITEM mi = { sizeof(mi) };
	if (PopUpOptions.ModuleIsEnabled) {
		//The module is enabled.
		//The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		PopUpOptions.ModuleIsEnabled = FALSE;
		DBWriteContactSettingByte(NULL, MODULNAME, "ModuleIsEnabled", FALSE);
		mi.ptszName = LPGENT("Enable &popup module");
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_OFF,0);
	}
	else {
		//The module is disabled.
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

void InitMenuItems(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	// support new genmenu style
	mi.flags		= CMIF_ROOTHANDLE|CMIF_TCHAR;
	mi.hParentMenu	= HGENMENU_ROOT;

	// Build main menu
	mi.position		= -1000000000 /*1000001*/;
	mi.ptszName		= LPGENT(MODULNAME_PLU);
	mi.hIcon		= IcoLib_GetIcon(PopUpOptions.ModuleIsEnabled ? ICO_POPUP_ON : ICO_POPUP_OFF ,0);
	hMenuRoot		= Menu_AddMainMenuItem(&mi);

	// Add item to main menu
	mi.hParentMenu		= (HGENMENU)hMenuRoot;

	hTogglePopup = CreateServiceFunction(MENUCOMMAND_SVC, svcEnableDisableMenuCommand);
	mi.ptszName			= PopUpOptions.ModuleIsEnabled ? LPGENT("Disable &popup module") : LPGENT("Enable &popup module");
	mi.pszService		= MENUCOMMAND_SVC;
	hMenuItem			= Menu_AddMainMenuItem(&mi);

	// Popup History
	hShowHistory = CreateServiceFunction(MENUCOMMAND_HISTORY, svcShowHistory);
	mi.position			= 1000000000;
	mi.popupPosition	= 1999990000;
	mi.ptszName			= LPGENT("Popup History");
	mi.hIcon			= IcoLib_GetIcon(ICO_HISTORY, 0);
	mi.pszService		= MENUCOMMAND_HISTORY;
	hMenuItemHistory	= Menu_AddMainMenuItem(&mi);

}

//===== GetStatus =====
INT_PTR GetStatus(WPARAM wp, LPARAM lp)
{
	return PopUpOptions.ModuleIsEnabled;
}


//register Hotkey
void LoadHotkey() {
	HOTKEYDESC hk        = {0};
	hk.cbSize            = sizeof(hk);
	hk.dwFlags			 = HKD_TCHAR;
	hk.pszName            = "Toggle Popups";
	hk.ptszDescription    = LPGENT("Toggle Popups");
	hk.ptszSection        = LPGENT(MODULNAME_PLU);
	hk.pszService        = MENUCOMMAND_SVC;
	Hotkey_Register( &hk);

	// 'Popup History' Hotkey
	hk.pszName			= "Popup History";
	hk.ptszDescription	= LPGENT("Popup History");
	hk.pszService		= MENUCOMMAND_HISTORY;
	Hotkey_Register( &hk);
}

//menu
//Function which makes the initializations
static int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
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
	if (!OptionLoaded){
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
	HookEvent(ME_TTB_MODULELOADED, TTBLoaded);
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

	return &pluginInfoEx;
}

//Miranda PluginInterfaces
MIRAPI const MUUID MirandaInterfaces[] = { MIID_POPUPS, MIID_LAST };

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
MIRAPI int Load(void)
{
	g_popup.isOsUnicode = (GetVersion() & 0x80000000) == 0;
	g_popup.isMirUnicode = true;

	hGetStatus = CreateServiceFunction(MS_POPUP_GETSTATUS, GetStatus);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);

	mir_getMTI(&MText);
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

	if (!OptionLoaded){
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
