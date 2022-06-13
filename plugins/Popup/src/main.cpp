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

#include "stdafx.h"

uint16_t SETTING_MAXIMUMWIDTH_MAX = GetSystemMetrics(SM_CXSCREEN);

#define MENUCOMMAND_HISTORY "Popup/ShowHistory"
#define MENUCOMMAND_SVC "Popup/EnableDisableMenuCommand"

HANDLE hEventNotify;

//===== Options =========================================================================
static int OptionsInitialize(WPARAM, LPARAM);
void UpgradeDb();

//===== Initializations =================================================================
static int OkToExit(WPARAM, LPARAM);
bool OptionLoaded = false;

CMPlugin g_plugin;

//===== Global variables ================================================================
HMODULE  hUserDll = nullptr;
HMODULE  hMsimgDll = nullptr;
HMODULE  hKernelDll = nullptr;
HMODULE  hGdiDll = nullptr;
HMODULE  hDwmapiDll = nullptr;

GLOBAL_WND_CLASSES g_wndClass = { 0 };

HANDLE   htuText;
HANDLE   htuTitle;

HGENMENU hMenuRoot;
HGENMENU hMenuItem;
HGENMENU hMenuItemHistory;

HANDLE   hTTButton;

//===== Options pages ===================================================================

static int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = MODULNAME_PLU;

	odp.szTab.a = LPGEN("General");
	odp.pfnDlgProc = DlgProcPopupGeneral;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_GENERAL);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Classes");
	odp.pfnDlgProc = DlgProcOptsClasses;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_NOTIFICATIONS);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Actions");
	odp.pfnDlgProc = DlgProcPopupActions;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ACTIONS);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Contacts");
	odp.pfnDlgProc = DlgProcContactOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTACTS);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pfnDlgProc = DlgProcPopupAdvOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_ADVANCED);
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.a = LPGEN("Skins");
	odp.szTab.a = LPGEN(MODULNAME_PLU);
	odp.pfnDlgProc = DlgProcPopSkinsOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN2);
	g_plugin.addOptions(wParam, &odp);
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

	HANDLE hIcon = Popup_Enabled() ? Skin_GetIconHandle(SKINICON_OTHER_POPUP) : Skin_GetIconHandle(SKINICON_OTHER_NOPOPUP);

	Menu_ModifyItem(hMenuItem, nullptr, hIcon);
	Menu_ModifyItem(hMenuRoot, nullptr, hIcon);

	Menu_ModifyItem(hMenuItemHistory, nullptr, g_plugin.getIconHandle(IDI_HISTORY));
	return 0;
}

static int TTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.pszService = MENUCOMMAND_SVC;
	ttb.lParamUp = 1;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	if (Popup_Enabled())
		ttb.dwFlags |= TTBBF_PUSHED;
	ttb.name = LPGEN("Toggle popups");
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_NOPOPUP);
	ttb.hIconHandleDn = Skin_GetIconHandle(SKINICON_OTHER_POPUP);
	ttb.pszTooltipUp = LPGEN("Enable popups");
	ttb.pszTooltipDn = LPGEN("Disable popups");
	hTTButton = g_plugin.addTTB(&ttb);
	return 0;
}

//===== EnableDisableMenuCommand ========================================================
INT_PTR svcEnableDisableMenuCommand(WPARAM, LPARAM)
{
	HANDLE hIcon;
	if (Popup_Enabled()) {
		// The module is enabled.
		// The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		Popup_Enable(false);
		Menu_ModifyItem(hMenuItem, LPGENW("Enable popups"), hIcon = Skin_GetIconHandle(SKINICON_OTHER_NOPOPUP));
	}
	else {
		// The module is disabled.
		// The action to do is enable popups (show enabled), then write "disable popup" in the new item.
		Popup_Enable(true);
		Menu_ModifyItem(hMenuItem, LPGENW("Disable popups"), hIcon = Skin_GetIconHandle(SKINICON_OTHER_POPUP));
	}

	Menu_ModifyItem(hMenuRoot, nullptr, hIcon);

	if (hTTButton)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, Popup_Enabled() ? TTBST_PUSHED : 0);

	return 0;
}

INT_PTR svcShowHistory(WPARAM, LPARAM)
{
	PopupHistoryShow();
	return 0;
}

void InitMenuItems(void)
{
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE;

	HANDLE hIcon = Skin_GetIconHandle(Popup_Enabled() ? SKINICON_OTHER_POPUP : SKINICON_OTHER_NOPOPUP);

	// Build main menu
	hMenuRoot = mi.root = g_plugin.addRootMenu(MO_MAIN, MODULNAME_PLUW, -1000000000, hIcon);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "3F5B5AB5-46D8-469E-8951-50B287C59037");

	// Add item to main menu
	SET_UID(mi, 0x4353d44e, 0x177, 0x4843, 0x88, 0x30, 0x25, 0x5d, 0x91, 0xad, 0xdf, 0x3f);
	mi.pszService = MENUCOMMAND_SVC;
	CreateServiceFunction(mi.pszService, svcEnableDisableMenuCommand);
	mi.name.w = Popup_Enabled() ? LPGENW("Disable popups") : LPGENW("Enable popups");
	mi.hIcolibItem = hIcon;
	hMenuItem = Menu_AddMainMenuItem(&mi);

	// Popup history
	SET_UID(mi, 0x92c386ae, 0x6e81, 0x452d, 0xb5, 0x71, 0x87, 0x46, 0xe9, 0x2, 0x66, 0xe9);
	mi.pszService = MENUCOMMAND_HISTORY;
	CreateServiceFunction(mi.pszService, svcShowHistory);
	mi.position = 1000000000;
	mi.name.w = LPGENW("Popup history");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_HISTORY);
	hMenuItemHistory = Menu_AddMainMenuItem(&mi);
}

// register Hotkey
void LoadHotkey()
{
	HOTKEYDESC hk = {};
	hk.dwFlags = HKD_UNICODE;
	hk.pszName = "Toggle Popups";
	hk.szDescription.w = LPGENW("Toggle popups");
	hk.szSection.w = MODULNAME_PLUW;
	hk.pszService = MENUCOMMAND_SVC;
	g_plugin.addHotkey(&hk);

	// 'Popup history' Hotkey
	hk.pszName = "Popup History";
	hk.szDescription.w = LPGENW("Popup history");
	hk.pszService = MENUCOMMAND_HISTORY;
	g_plugin.addHotkey(&hk);
}

// menu
// Function which makes the initializations
static int ModulesLoaded(WPARAM, LPARAM)
{
	// check if History++ is installed
	gbHppInstalled = ServiceExists(MS_HPP_GETVERSION) && ServiceExists(MS_HPP_EG_WINDOW) &&
		(CallService(MS_HPP_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(1, 5, 0, 112));

	// check if MText plugin is installed
	htuText = MTextRegister("Popup Plus/Text", MTEXT_FANCY_DEFAULT);
	htuTitle = MTextRegister("Popup Plus/Title", MTEXT_FANCY_DEFAULT);

	// check if OptionLoaded
	if (!OptionLoaded)
		LoadOptions();

	// Uninstalling purposes
	if (ServiceExists("PluginSweeper/Add"))
		CallService("PluginSweeper/Add", (WPARAM)Translate(MODULENAME), (LPARAM)MODULENAME);

	// load actions and notifications
	LoadActions();
	LoadNotifications();
	// hook TopToolBar
	HookEvent(ME_TTB_MODULELOADED, TTBLoaded);
	// Folder plugin support
	folderId = FoldersRegisterCustomPathW(LPGEN("Skins"), LPGEN("Popup Plus"), MIRANDA_PATHW L"\\Skins\\Popup");
	// load skin
	skins.load();
	const PopupSkin *skin;
	if (skin = skins.getSkin(PopupOptions.SkinPack)) {
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_wstrdup(skin->getName());
		skin->loadOpts();
	}
	// init PopupEfects
	PopupEfectsInitialize();
	// MessageAPI support
	SrmmMenu_Load();
	// Hotkey
	LoadHotkey();

	gbPopupLoaded = TRUE;
	return 0;
}

//===== MirandaPluginInfo ===============================================================
// Called by Miranda to get the information associated to this plugin.
// It only returns the PLUGININFOEX structure, without any test on the version
// @param mirandaVersion - The version of the application calling this function

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
	// {67716225-21F1-4A03-A69B-5FE12F128A2B}
  { 0x67716225, 0x21f1, 0x4a03, 0xa6, 0x9b, 0x5f, 0xe1, 0x2f, 0x12, 0x8a, 0x2b }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_POPUP, MIID_LAST };

//===== Load ============================================================================
// Initializes the services provided and the link to those needed
// Called when the plugin is loaded into Miranda

static int OkToExit(WPARAM, LPARAM)
{
	closing = TRUE;
	StopPopupThread();
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	UnloadPopupThread();
	UnloadPopupWnd2();
	return 0;
}

int CMPlugin::Load()
{
#if defined(_DEBUG)
	PopupOptions.debug = g_plugin.getByte("debug", FALSE);
#else
	PopupOptions.debug = false;
#endif
	LoadGDIPlus();

	// Transparent and animation routines
	hDwmapiDll = LoadLibrary(L"dwmapi.dll");
	MyDwmEnableBlurBehindWindow = nullptr;
	if (hDwmapiDll)
		MyDwmEnableBlurBehindWindow = (HRESULT(WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(hDwmapiDll, "DwmEnableBlurBehindWindow");

	PopupHistoryLoad();
	LoadPopupThread();
	if (!LoadPopupWnd2()) {
		MessageBox(nullptr, TranslateT("Error: I could not register the Popup Window class.\r\nThe plugin will not operate."), MODULNAME_LONG, MB_ICONSTOP | MB_OK);
		return 0; // We couldn't register our Window Class, don't hook any event: the plugin will act as if it was disabled.
	}
	RegisterOptPrevBox();

	// Register in DBEditor++
	UpgradeDb();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OkToExit);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);

	hbmNoAvatar = LoadBitmap(g_plugin.getInst(), MAKEINTRESOURCE(IDB_NOAVATAR));

	if (!OptionLoaded)
		LoadOptions();

	// Service Functions
	hEventNotify = CreateHookableEvent(ME_POPUP_FILTER);

	CreateServiceFunction(MS_POPUP_ADDPOPUP, Popup_AddPopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, Popup_AddPopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, Popup_AddPopup2);

	CreateServiceFunction(MS_POPUP_CHANGETEXTW, Popup_ChangeTextW);

	CreateServiceFunction(MS_POPUP_CHANGEW, Popup_ChangeW);
	CreateServiceFunction(MS_POPUP_CHANGEPOPUP2, Popup_Change2);

	CreateServiceFunction(MS_POPUP_GETCONTACT, Popup_GetContact);
	CreateServiceFunction(MS_POPUP_GETPLUGINDATA, Popup_GetPluginData);

	CreateServiceFunction(MS_POPUP_SHOWMESSAGE, Popup_ShowMessage);
	CreateServiceFunction(MS_POPUP_SHOWMESSAGEW, Popup_ShowMessageW);

	CreateServiceFunction(MS_POPUP_REGISTERACTIONS, Popup_RegisterActions);
	CreateServiceFunction(MS_POPUP_REGISTERNOTIFICATION, Popup_RegisterNotification);

	CreateServiceFunction(MS_POPUP_UNHOOKEVENTASYNC, Popup_UnhookEventAsync);

	CreateServiceFunction(MS_POPUP_REGISTERVFX, Popup_RegisterVfx);

	CreateServiceFunction(MS_POPUP_REGISTERCLASS, Popup_RegisterPopupClass);
	CreateServiceFunction(MS_POPUP_UNREGISTERCLASS, Popup_UnregisterPopupClass);
	CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS, Popup_CreateClassPopup);

	CreateServiceFunction(MS_POPUP_DESTROYPOPUP, Popup_DeletePopup);

	CreateServiceFunction("Popup/LoadSkin", Popup_LoadSkin);

	// load fonts / create hook
	InitFonts();
	HookEvent(ME_FONT_RELOAD, FontsChanged);
	Miranda_WaitOnHandle(ReloadFonts);

	// load icons / create hook
	InitIcons();
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);

	// add menu items
	InitMenuItems();
	return 0;
}

//===== Unload ==========================================================================
// Prepare the plugin to stop
// Called by Miranda when it will exit or when the plugin gets deselected

int CMPlugin::Unload()
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
	PopupHistoryUnload();
	SrmmMenu_Unload();

	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupWnd2), g_plugin.getInst());
	UnregisterClass(L"PopupEditBox", g_plugin.getInst());
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupMenuHostWnd), g_plugin.getInst());
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupThreadManagerWnd), g_plugin.getInst());
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupPreviewBoxWndclass), g_plugin.getInst());
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupPlusDlgBox), g_plugin.getInst());

	UnloadGDIPlus();

	UnloadActions();
	UnloadTreeData();
	return 0;
}
