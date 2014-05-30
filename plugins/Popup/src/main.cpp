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
*/

#include "headers.h"

WORD SETTING_MAXIMUMWIDTH_MAX = GetSystemMetrics(SM_CXSCREEN);

#define MENUCOMMAND_HISTORY "Popup/ShowHistory"
#define MENUCOMMAND_SVC "Popup/EnableDisableMenuCommand"

HANDLE hEventNotify;

//===== MessageBoxes =====
//void MB(char*); //This one is for Debug purposes
//void Log(char*); //This one is used to notify the user
//void HigherLower(int, int); //Used for integer input boxes (out of bound values).
//void NotifyError(DWORD, char*, int, char*); //Used to notify an error giving some error codes to report.

//===== Options =====
static int OptionsInitialize(WPARAM,LPARAM);
void UpgradeDb();

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
HANDLE htuText;
HANDLE htuTitle;

//===== Menu item =====
HGENMENU hMenuRoot;
HGENMENU hMenuItem;
HGENMENU hMenuItemHistory;

//===== Event Handles =====
HANDLE hTTButton;

//===== Options pages =====
static int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position      = 100000000;
	odp.groupPosition = 910000000;
	odp.hInstance     = hInst;
	odp.flags         = ODPF_BOLDGROUPS;
	odp.pszTitle      = MODULNAME_PLU;

	odp.pszTab      = LPGEN("General");
	odp.pfnDlgProc  = DlgProcPopupGeneral;
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
	odp.pfnDlgProc  = DlgProcPopupAdvOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_ADVANCED);
	Options_AddPage(wParam, &odp);

	odp.pszGroup    = LPGEN("Skins");
	odp.pszTab      = LPGEN(MODULNAME_PLU);
	odp.pfnDlgProc  = DlgProcPopSkinsOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN2);
	Options_AddPage(wParam, &odp);
	return 0;
}

static int FontsChanged(WPARAM, LPARAM)
{
	ReloadFonts();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	LoadActions();

	CLISTMENUITEM mi = { sizeof(mi) };
	if (PopupOptions.ModuleIsEnabled == TRUE) { //The module is enabled.
		//The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_ON, 0);
	}
	else { //The module is disabled.
		//The action to do is enable popups (show enabled), then write "disable popup" in the new item.
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_OFF, 0);
	}
	mi.flags = CMIM_ICON;
	Menu_ModifyItem(hMenuItem, &mi);
	Menu_ModifyItem(hMenuRoot, &mi);

	mi.hIcon = IcoLib_GetIcon(ICO_HISTORY, 0);
	mi.flags = CMIM_ICON;
	Menu_ModifyItem(hMenuItemHistory, &mi);
	return 0;
}

static int TTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.pszService = MENUCOMMAND_SVC;
	ttb.lParamUp = 1;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	if (PopupOptions.ModuleIsEnabled)
		ttb.dwFlags |= TTBBF_PUSHED;
	ttb.name = LPGEN("Toggle Popups");
	ttb.hIconHandleUp = Skin_GetIconHandle(ICO_TB_POPUP_OFF);
	ttb.hIconHandleDn = Skin_GetIconHandle(ICO_TB_POPUP_ON);
	ttb.pszTooltipUp = LPGEN("Enable Popups");
	ttb.pszTooltipDn = LPGEN("Disable Popups");
	hTTButton = TopToolbar_AddButton(&ttb);
	return 0;
}

//===== EnableDisableMenuCommand =====
INT_PTR svcEnableDisableMenuCommand(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	if (PopupOptions.ModuleIsEnabled) {
		//The module is enabled.
		//The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		PopupOptions.ModuleIsEnabled = FALSE;
		db_set_b(NULL, "Popup", "ModuleIsEnabled", FALSE);
		mi.ptszName = LPGENT("Enable Popups");
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_OFF,0);
	}
	else {
		//The module is disabled.
		//The action to do is enable popups (show enabled), then write "disable popup" in the new item.
		PopupOptions.ModuleIsEnabled = TRUE;
		db_set_b(NULL, "Popup", "ModuleIsEnabled", TRUE);
		mi.ptszName = LPGENT("Disable Popups");
		mi.hIcon = IcoLib_GetIcon(ICO_POPUP_ON,0);
	}
	mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
	Menu_ModifyItem(hMenuItem, &mi);
	mi.flags = CMIM_ICON;
	Menu_ModifyItem(hMenuRoot, &mi);

	if (hTTButton)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, (PopupOptions.ModuleIsEnabled) ? TTBST_PUSHED : TTBST_RELEASED);

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
	mi.hIcon		= IcoLib_GetIcon(PopupOptions.ModuleIsEnabled ? ICO_POPUP_ON : ICO_POPUP_OFF ,0);
	hMenuRoot		= Menu_AddMainMenuItem(&mi);

	// Add item to main menu
	mi.hParentMenu    = (HGENMENU)hMenuRoot;

	CreateServiceFunction(MENUCOMMAND_SVC, svcEnableDisableMenuCommand);
	mi.ptszName       = PopupOptions.ModuleIsEnabled ? LPGENT("Disable Popups") : LPGENT("Enable Popups");
	mi.pszService     = MENUCOMMAND_SVC;
	hMenuItem         = Menu_AddMainMenuItem(&mi);

	// Popup History
	CreateServiceFunction(MENUCOMMAND_HISTORY, svcShowHistory);
	mi.position       = 1000000000;
	mi.popupPosition  = 1999990000;
	mi.ptszName       = LPGENT("Popup History");
	mi.hIcon          = IcoLib_GetIcon(ICO_HISTORY, 0);
	mi.pszService     = MENUCOMMAND_HISTORY;
	hMenuItemHistory  = Menu_AddMainMenuItem(&mi);

}

//===== GetStatus =====
INT_PTR GetStatus(WPARAM, LPARAM)
{
	return PopupOptions.ModuleIsEnabled;
}


//register Hotkey
void LoadHotkey()
{
	HOTKEYDESC hk = { sizeof(hk) };
	hk.dwFlags = HKD_TCHAR;
	hk.pszName = "Toggle Popups";
	hk.ptszDescription = LPGENT("Toggle Popups");
	hk.ptszSection = LPGENT(MODULNAME_PLU);
	hk.pszService = MENUCOMMAND_SVC;
	Hotkey_Register(&hk);

	// 'Popup History' Hotkey
	hk.pszName = "Popup History";
	hk.ptszDescription = LPGENT("Popup History");
	hk.pszService = MENUCOMMAND_HISTORY;
	Hotkey_Register(&hk);
}

//menu
//Function which makes the initializations
static int ModulesLoaded(WPARAM,LPARAM)
{
	//check if History++ is installed
	gbHppInstalled = ServiceExists(MS_HPP_GETVERSION) && ServiceExists(MS_HPP_EG_WINDOW) &&
		(CallService(MS_HPP_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(1,5,0,112));
	//check if MText plugin is installed
	if (MText.Register) {
		htuText = MText.Register("Popup Plus/Text", MTEXT_FANCY_DEFAULT);
		htuTitle	= MText.Register("Popup Plus/Title",MTEXT_FANCY_DEFAULT);
	}
	else htuTitle = htuText = NULL;

	//check if OptionLoaded
	if (!OptionLoaded)
		LoadOptions();

	//Uninstalling purposes
	if (ServiceExists("PluginSweeper/Add"))
		CallService("PluginSweeper/Add", (WPARAM)Translate(MODULNAME), (LPARAM)MODULNAME);

	//load fonts / create hook
	InitFonts();
	HookEvent(ME_FONT_RELOAD, FontsChanged);

	//load actions and notifications
	LoadActions();
	LoadNotifications();
	//hook TopToolBar
	HookEvent(ME_TTB_MODULELOADED, TTBLoaded);
	//Folder plugin support
	folderId = FoldersRegisterCustomPathT(LPGEN("Skins"), LPGEN("Popup Plus"), MIRANDA_PATHT _T("\\Skins\\Popup"));
	//load skin
	skins.load(_T("dir"));
	const PopupSkin *skin;
	if (skin = skins.getSkin(PopupOptions.SkinPack)) {
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_tstrdup(skin->getName());
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
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

//===== MirandaPluginInfo =====
//Called by Miranda to get the information associated to this plugin.
//It only returns the PLUGININFOEX structure, without any test on the version
//@param mirandaVersion - The version of the application calling this function
MIRAPI PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

//ME_SYSTEM_PRESHUTDOWN event
//called before the app goes into shutdown routine to make sure everyone is happy to exit
static int OkToExit(WPARAM, LPARAM)
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
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);

	mir_getMTI(&MText);
	mir_getLP(&pluginInfoEx);

	CreateServiceFunction(MS_POPUP_GETSTATUS, GetStatus);

	#if defined(_DEBUG)
		PopupOptions.debug = db_get_b(NULL, MODULNAME, "debug", FALSE);
	#else
		PopupOptions.debug = false;
	#endif
	LoadGDIPlus();

	//Transparent and animation routines
	hDwmapiDll = LoadLibrary(_T("dwmapi.dll"));
	MyDwmEnableBlurBehindWindow = 0;
	if (hDwmapiDll)
		MyDwmEnableBlurBehindWindow = (HRESULT (WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(hDwmapiDll, "DwmEnableBlurBehindWindow");

	PopupHistoryLoad();
	LoadPopupThread();
	if (!LoadPopupWnd2()) {
		MessageBox(0, TranslateT("Error: I could not register the Popup Window class.\r\nThe plugin will not operate."), _T(MODULNAME_LONG), MB_ICONSTOP | MB_OK);
		return 0; //We couldn't register our Window Class, don't hook any event: the plugin will act as if it was disabled.
	}
	RegisterOptPrevBox();

	// Register in DBEditor++
	UpgradeDb();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OkToExit);

	hbmNoAvatar = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_NOAVATAR));

	if (!OptionLoaded)
		LoadOptions();

	//Service Functions
	hEventNotify = CreateHookableEvent(ME_POPUP_FILTER);

	CreateServiceFunction(MS_POPUP_ADDPOPUP,             Popup_AddPopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW,            Popup_AddPopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2,            Popup_AddPopup2);

	CreateServiceFunction(MS_POPUP_CHANGETEXTW,          Popup_ChangeTextW);

	CreateServiceFunction(MS_POPUP_CHANGEW,              Popup_ChangeW);
	CreateServiceFunction(MS_POPUP_CHANGEPOPUP2,         Popup_Change2);

	CreateServiceFunction(MS_POPUP_GETCONTACT,           Popup_GetContact);
	CreateServiceFunction(MS_POPUP_GETPLUGINDATA,        Popup_GetPluginData);

	CreateServiceFunction(MS_POPUP_SHOWMESSAGE,          Popup_ShowMessage);
	CreateServiceFunction(MS_POPUP_SHOWMESSAGEW,         Popup_ShowMessageW);
	CreateServiceFunction(MS_POPUP_QUERY,                Popup_Query);

	CreateServiceFunction(MS_POPUP_REGISTERACTIONS,      Popup_RegisterActions);
	CreateServiceFunction(MS_POPUP_REGISTERNOTIFICATION, Popup_RegisterNotification);

	CreateServiceFunction(MS_POPUP_UNHOOKEVENTASYNC,     Popup_UnhookEventAsync);

	CreateServiceFunction(MS_POPUP_REGISTERVFX,          Popup_RegisterVfx);

	CreateServiceFunction(MS_POPUP_REGISTERCLASS,        Popup_RegisterPopupClass);
	CreateServiceFunction(MS_POPUP_UNREGISTERCLASS,      Popup_UnregisterPopupClass);
	CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS,        Popup_CreateClassPopup);

	//load icons / create hook
	InitIcons();
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);
	//add menu items
	InitMenuItems();

	return 0;
}

//===== Unload =====
//Prepare the plugin to stop
//Called by Miranda when it will exit or when the plugin gets deselected

MIRAPI int Unload(void)
{
	DeleteObject(fonts.title);
	DeleteObject(fonts.clock);
	DeleteObject(fonts.text);
	DeleteObject(fonts.action);
	DeleteObject(fonts.actionHover);

	DeleteObject(hbmNoAvatar);

	FreeLibrary(hDwmapiDll);
	FreeLibrary(hUserDll);
	FreeLibrary(hMsimgDll);
	FreeLibrary(hGdiDll);

	DestroyHookableEvent(hEventNotify);

	mir_free(PopupOptions.SkinPack);
	mir_free(PopupOptions.Effect);

	OptAdv_UnregisterVfx();
	UnloadPopupThread();
	UnloadPopupWnd2();
	PopupHistoryUnload();
	SrmmMenu_Unload();

	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupWnd2),hInst);
	UnregisterClassW(L"PopupEditBox",hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupMenuHostWnd),hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupThreadManagerWnd),hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupPreviewBoxWndclass),hInst);
	UnregisterClass (MAKEINTATOM(g_wndClass.cPopupPlusDlgBox),hInst);

	UnloadGDIPlus();

	UnloadActions();
	UnloadTreeData();

	CloseHandle(hMainThread);

	return 0;
}
