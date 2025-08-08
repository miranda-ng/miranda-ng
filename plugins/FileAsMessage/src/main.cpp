#include "stdafx.h"

CMPlugin g_plugin;

MWindowList hFileList;

char *szServiceTitle = SERVICE_TITLE;
char *szServicePrefix = SERVICE_PREFIX;
HANDLE hHookDbSettingChange, hHookContactAdded, hHookSkinIconsChanged;

/////////////////////////////////////////////////////////////////////////////////////////

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
	// {34B5A402-1B79-4246-B041-43D0B590AE2C}
	{ 0x34b5a402, 0x1b79, 0x4246, { 0xb0, 0x41, 0x43, 0xd0, 0xb5, 0x90, 0xae, 0x2c } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
//  wParam - Section name
//  lParam - Icon ID

int OnSkinIconsChanged(WPARAM, LPARAM)
{
	WindowList_Broadcast(hFileList, WM_FE_SKINCHANGE, 0, 0);
	return 0;
}

int OnSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	if (hContact && !strcmp(cws->szSetting, "Status")) {
		HWND hwnd = WindowList_Find(hFileList, hContact);
		PostMessage(hwnd, WM_FE_STATUSCHANGE, 0, 0);
	}
	return 0;
}

INT_PTR OnRecvFile(WPARAM, LPARAM lParam)
{
	auto *clev = (CLISTEVENT *)lParam;

	HWND hwnd = WindowList_Find(hFileList, clev->hContact);
	if (IsWindow(hwnd)) {
		ShowWindow(hwnd, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}

	return 1;
}

INT_PTR OnSendFile(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hFileList, wParam);
	if (IsWindow(hwnd)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else {
		if (hwnd != nullptr) WindowList_Remove(hFileList, hwnd);
		FILEECHO *fe = new FILEECHO(wParam);
		fe->inSend = TRUE;
		hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MAIN), nullptr, DialogProc, (LPARAM)fe);
		if (hwnd == nullptr) {
			delete fe;
			return 0;
		}

		ShowWindow(hwnd, SW_SHOWNORMAL);
	}
	return 1;
}

INT_PTR OnRecvMessage(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	auto *dbei = (DB::EventInfo *)ccs->lParam;

	if (strncmp(dbei->pBlob, szServicePrefix, mir_strlen(szServicePrefix)))
		return Proto_ChainRecv(wParam, ccs);

	HWND hwnd = WindowList_Find(hFileList, ccs->hContact);
	if (!IsWindow(hwnd)) {
		if (hwnd != nullptr) WindowList_Remove(hFileList, hwnd);
		auto *fe = new FILEECHO(ccs->hContact);
		fe->inSend = FALSE;
		hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MAIN), nullptr, DialogProc, (LPARAM)fe);
		if (hwnd == nullptr) {
			delete fe;
			return 0;
		}
	}
	
	char *msg = mir_strdup(dbei->pBlob + mir_strlen(szServicePrefix));
	PostMessage(hwnd, WM_FE_MESSAGE, (WPARAM)ccs->hContact, (LPARAM)msg);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Startup initializing

static IconItem iconList[] =
{
	{ LPGEN("Play"),   "FePlay",    IDI_PLAY    },
	{ LPGEN("Pause"),  "FePause",   IDI_PAUSE   },
	{ LPGEN("Revive"), "FeRefresh", IDI_REFRESH },
	{ LPGEN("Stop"),   "FeStop",    IDI_STOP    },
	{ LPGEN("Main"),   "FeMain",    IDI_MAIN    },
};

static int ToolbarButtonPressed(WPARAM, LPARAM lParam)
{
	auto *pcbc = (CustomButtonClickData *)lParam;
	if (!mir_strcmp(pcbc->pszModule, MODULENAME))
		CallService(MODULENAME "/FESendFile", pcbc->hContact, 0);

	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	hHookSkinIconsChanged = HookEvent(ME_SKIN_ICONSCHANGED, OnSkinIconsChanged);

	// main menu item
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xe4a98d2a, 0xa54a, 0x4db1, 0x8d, 0x29, 0xd, 0x5c, 0xf1, 0x10, 0x69, 0x35);
	mi.position = 200011;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MAIN);
	mi.name.a = LPGEN("File As Message...");
	mi.pszService = MODULENAME "/FESendFile";
	mi.flags = CMIF_NOTOFFLINE;
	Menu_AddContactMenuItem(&mi);

	// toolbar button
	BBButton bbd = {};
	bbd.pszModuleName = MODULENAME;
	bbd.dwDefPos = 1500;
	bbd.pwszText = bbd.pwszTooltip = LPGENW("File As Message...");
	bbd.bbbFlags = BBBF_ISRSIDEBUTTON | BBBF_CANBEHIDDEN | BBBF_HIDDEN;
	bbd.hIcon = mi.hIcolibItem;
	g_plugin.addButton(&bbd);
	
	HookEvent(ME_MSG_BUTTONPRESSED, ToolbarButtonPressed);
	return 0;
}

int CMPlugin::Load()
{
	InitCRC32();

	g_plugin.registerIcon(MODULENAME, iconList);

	hFileList = WindowList_Create();

	CreateServiceFunction(MODULENAME PSR_MESSAGE, OnRecvMessage);
	CreateServiceFunction(MODULENAME "/FESendFile", OnSendFile);
	CreateServiceFunction(MODULENAME "/FERecvFile", OnRecvFile);

	Proto_RegisterModule(PROTOTYPE_FILTER, MODULENAME);

	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookDbSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);
	hHookSkinIconsChanged = nullptr;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload()
// Called by Miranda when Plugin is unloaded.

int CMPlugin::Unload()
{
	WindowList_Destroy(hFileList);
	if (hHookSkinIconsChanged != nullptr)
		UnhookEvent(hHookSkinIconsChanged);
	UnhookEvent(hHookDbSettingChange);
	UnhookEvent(hHookContactAdded);

	return 0;
}
