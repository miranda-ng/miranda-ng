#include "stdafx.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
}
g_plugin;

HANDLE hButtonTopToolbar;

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
	// {10896143-7249-4B36-A408-6501A6B6035A}
	{0x10896143, 0x7249, 0x4b36, {0xa4, 0x8, 0x65, 0x1, 0xa6, 0xb6, 0x3, 0x5a}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] = 
{
	{ LPGEN("Open Folder"), "open", IDI_FOLDER }
};

static INT_PTR MenuCommand_OpenFolder(WPARAM, LPARAM)
{
	wchar_t szMirandaPath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(nullptr), szMirandaPath, _countof(szMirandaPath));
	wchar_t *p = wcsrchr(szMirandaPath, '\\');
	if (p)
		p[1] = 0;

	if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		ShellExecute(nullptr, L"explore", szMirandaPath, nullptr, nullptr, SW_SHOWNORMAL);
	else
		ShellExecute(nullptr, L"open", szMirandaPath, nullptr, nullptr, SW_SHOWNORMAL);

	return 0;
}

// toptoolbar (if plugin is installed)
static int ToptoolBarHook(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.hIconHandleUp = iconList[0].hIcolib;
	ttb.pszService = MS_OPENFOLDER_OPEN;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.name = LPGEN("Open Folder");
	g_plugin.addTTB(&ttb);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	HookTemporaryEvent(ME_TTB_MODULELOADED, ToptoolBarHook);
	return 0;
}

HICON LoadIconExEx(const char* IcoLibName, int)
{
	char szSettingName[64];
	mir_snprintf(szSettingName, "%s_%s", OPENFOLDER_MODULE_NAME, IcoLibName);
	return IcoLib_GetIcon(szSettingName);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	CreateServiceFunction(MS_OPENFOLDER_OPEN, MenuCommand_OpenFolder);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	// icolib (0.7+)
	g_plugin.registerIcon(LPGEN("Open Folder"), iconList, OPENFOLDER_MODULE_NAME);
	
	// hotkeys service (0.8+)
	HOTKEYDESC hotkey = {};
	hotkey.dwFlags = HKD_UNICODE;
	hotkey.pszName = "Open Folder";
	hotkey.szDescription.w = LPGENW("Open Folder");
	hotkey.szSection.w = LPGENW("Main");
	hotkey.pszService = MS_OPENFOLDER_OPEN;
	hotkey.DefHotKey = MAKEWORD( 'O', HOTKEYF_SHIFT | HOTKEYF_ALT );
	g_plugin.addHotkey(&hotkey);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xbba6ad01, 0x755a, 0x4d01, 0x94, 0xee, 0x57, 0x84, 0x18, 0x70, 0x77, 0x4f);
	mi.position = 0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = LPGENW("Open Folder");
	mi.pszService = MS_OPENFOLDER_OPEN;
	Menu_AddMainMenuItem(&mi);

	return 0;
}
