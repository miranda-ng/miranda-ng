#include "openFolder.h"

int hLangpack = 0;
HINSTANCE hInst;
HANDLE hServiceOpenFolder, hButtonTopToolbar;

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {10896143-7249-4B36-A408-6501A6B6035A}
	{0x10896143, 0x7249, 0x4b36, {0xa4, 0x8, 0x65, 0x1, 0xa6, 0xb6, 0x3, 0x5a}}
};

static IconItem icon = { LPGEN("Open Folder"), "open", IDI_FOLDER };

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static INT_PTR MenuCommand_OpenFolder(WPARAM wParam, LPARAM lParam)
{
	TCHAR szMirandaPath[MAX_PATH];
	GetModuleFileName( GetModuleHandle(NULL), szMirandaPath, SIZEOF(szMirandaPath));
	TCHAR* p = _tcsrchr( szMirandaPath, '\\' );
	if ( p && p + 1 )
		*( p + 1 ) = 0;

	if ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 )
		ShellExecute(0, _T("explore"), szMirandaPath, 0, 0, SW_SHOWNORMAL);
	else
		ShellExecute(0, _T("open"), szMirandaPath, 0, 0, SW_SHOWNORMAL);

	return 0;
}

// toptoolbar (if plugin is installed)
static int ToptoolBarHook(WPARAM wParam, LPARAM lParam)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.hIconHandleUp = icon.hIcolib;
	ttb.pszService = MS_OPENFOLDER_OPEN;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.name = LPGEN("Open Folder");
	TopToolbar_AddButton(&ttb);
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_TTB_MODULELOADED, ToptoolBarHook);
	
	// hotkeys service (0.8+)
	HOTKEYDESC hotkey = { 0 };
	hotkey.cbSize = sizeof(hotkey);
	hotkey.dwFlags = HKD_TCHAR;
	hotkey.pszName = "Open Folder";
	hotkey.ptszDescription = LPGENT("Open Folder");
	hotkey.ptszSection = LPGENT("Main");
	hotkey.pszService = MS_OPENFOLDER_OPEN;
	hotkey.DefHotKey = MAKEWORD( 'O', HOTKEYF_SHIFT | HOTKEYF_ALT );
	Hotkey_Register(&hotkey);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 0x7FFFFFFF;
	mi.flags = CMIF_TCHAR;
	mi.icolibItem = icon.hIcolib;
	mi.ptszName = LPGENT("Open Folder");
	mi.pszService = MS_OPENFOLDER_OPEN;
	Menu_AddMainMenuItem(&mi);
	return 0;
}

HICON LoadIconExEx( const char* IcoLibName, int NonIcoLibIcon )
{
	char szSettingName[64];
	mir_snprintf( szSettingName, sizeof( szSettingName ), "%s_%s", OPENFOLDER_MODULE_NAME, IcoLibName );
	return Skin_GetIcon(szSettingName);
}

extern "C" __declspec( dllexport ) PLUGININFOEX* MirandaPluginInfoEx( DWORD mirandaVersion )
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec( dllexport ) Load()
{
	mir_getLP(&pluginInfoEx);

	hServiceOpenFolder = CreateServiceFunction(MS_OPENFOLDER_OPEN, MenuCommand_OpenFolder);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	// icolib (0.7+)
	Icon_Register(hInst, LPGEN("Open Folder"), &icon, 1, OPENFOLDER_MODULE_NAME);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec( dllexport ) Unload()
{
	DestroyServiceFunction(hServiceOpenFolder);
	return 0;
}
