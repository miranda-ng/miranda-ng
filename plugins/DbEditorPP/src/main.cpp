#include "stdafx.h"

MIDatabase *g_db;

HANDLE hTTBButt = nullptr;
bool g_bServiceMode = false;
bool g_bUsePopups;

CMPlugin g_plugin;
uint8_t nameOrder[NAMEORDERCOUNT];
HGENMENU hUserMenu;
MCONTACT hRestore;

extern HWND hwnd2watchedVarsWindow;

#pragma comment(lib, "shlwapi.lib")

/////////////////////////////////////////////////////////////////////////////////////////
//  MirandaPluginInfo

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
	// {A8A417EF-07AA-4F37-869F-7BFD74886534}
	{ 0xa8a417ef, 0x7aa, 0x4f37, { 0x86, 0x9f, 0x7b, 0xfd, 0x74, 0x88, 0x65, 0x34 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	iPopupDelay(MODULENAME, "PopupDelay", 4),
	iPopupBkColor(MODULENAME, "PopupColour", RGB(255, 0, 0)),
	iPopupTxtColor(MODULENAME, "PopupTextColour", RGB(0, 0, 0)),
	bWarnOnDelete(MODULENAME, "WarnOnDelete", true),
	bRestoreOnOpen(MODULENAME, "RestoreOnOpen", true),
	bExpandSettingsOnOpen(MODULENAME, "ExpandSettingsOnOpen", false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// we implement service mode interface

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static int DBSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

	// setting list
	if (g_pMainWindow)
		g_pMainWindow->onSettingChanged(hContact, cws);

	// watch list
	if (!hwnd2watchedVarsWindow && !g_bUsePopups)
		return 0;

	if (WatchedArrayIndex(hContact, cws->szModule, cws->szSetting, 0) >= 0) {
		if (g_bUsePopups) popupWatchedVar(hContact, cws->szModule, cws->szSetting);
		PopulateWatchedWindow();
	}

	return 0;
}

INT_PTR DBEditorppMenuCommand(WPARAM wParam, LPARAM)
{
	if (!g_pMainWindow) { // so only opens 1 at a time
		hRestore = wParam;
		(new CMainDlg())->Create();
	}
	else {
		ShowWindow(g_pMainWindow->GetHwnd(), SW_RESTORE);
		SetForegroundWindow(g_pMainWindow->GetHwnd());
		if (!hRestore && wParam) {
			hRestore = wParam;
			refreshTree(4);
		}
	}

	if (hTTBButt)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTBButt, (LPARAM)0);

	return 0;
}

static int OnTTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = "DBEditorpp/MenuCommand";
	ttb.name = LPGEN("Database Editor++");
	ttb.hIconHandleUp = g_plugin.getIconHandle(ICO_DBE_BUTT);
	ttb.pszTooltipUp = LPGEN("Open Database Editor");
	hTTBButt = g_plugin.addTTB(&ttb);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	// Register menu item
	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000);

	SET_UID(mi, 0xe298849c, 0x1a8c, 0x4fc7, 0xa0, 0xf4, 0x78, 0x18, 0xf, 0xe2, 0xf7, 0xc9);
	mi.position = 1900000001;
	mi.hIcolibItem = g_plugin.getIconHandle(ICO_DBE_BUTT);
	mi.name.a = modFullname;
	mi.pszService = "DBEditorpp/MenuCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x2fed8613, 0xac43, 0x4148, 0xbd, 0x5c, 0x44, 0x88, 0xaf, 0x68, 0x69, 0x10);
	mi.root = nullptr;
	mi.hIcolibItem = g_plugin.getIconHandle(ICO_REGUSER);
	mi.name.a = LPGEN("Open user tree in DBE++");
	mi.pszService = "DBEditorpp/MenuCommand";
	hUserMenu = Menu_AddContactMenuItem(&mi);

	// Register hotkeys
	HOTKEYDESC hkd = {};
	hkd.pszName = "hk_dbepp_open";
	hkd.pszService = "DBEditorpp/MenuCommand";
	hkd.szSection.a = modFullname;
	hkd.szDescription.a = LPGEN("Open Database Editor");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_EXT, 'D');
	g_plugin.addHotkey(&hkd);

	g_bUsePopups = g_plugin.getByte("UsePopUps", 0) != 0;

	HookEvent(ME_TTB_MODULELOADED, OnTTBLoaded);
	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	if (hwnd2watchedVarsWindow)
		DestroyWindow(hwnd2watchedVarsWindow);
	if (g_pMainWindow)
		g_pMainWindow->Close();
	return 0;
}

static INT_PTR ServiceMode(WPARAM, LPARAM)
{
	g_bServiceMode = true;

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	DBEditorppMenuCommand(0, 0);

	return SERVICE_ONLYDB;  // load database and then call us
}

static INT_PTR ImportFromFile(WPARAM wParam, LPARAM lParam)
{
	ImportSettingsFromFileMenuItem(wParam, (char *)lParam);
	return 0;
}

int CMPlugin::Load()
{
	hRestore = NULL;
	g_db = db_get_current();

	IcoLibRegister();

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	CreateServiceFunction("DBEditorpp/MenuCommand", DBEditorppMenuCommand);
	CreateServiceFunction("DBEditorpp/Import", ImportFromFile);

	CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceMode);

	// Ensure that the common control DLL is loaded.
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	// Load the name order
	for (int i = 0; i < NAMEORDERCOUNT; i++)
		nameOrder[i] = i;

	DBVARIANT dbv = {};
	if (!db_get_s(0, "Contact", "NameOrder", &dbv, DBVT_BLOB)) {
		memcpy(nameOrder, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	freeAllWatches();
	return 0;
}
