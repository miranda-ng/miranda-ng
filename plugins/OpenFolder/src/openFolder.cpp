#include "openFolder.h"

int hLangpack = 0;
HINSTANCE hInst;
HANDLE hServiceOpenFolder, hButtonTopToolbar, hIconOpenFolder;

PLUGININFOEX pluginInfoEx =
{
	sizeof( PLUGININFOEX ),
	"Open Miranda Folder",
	OPENFOLDER_VERSION,
	OPENFOLDER_DESCRIPTION,
	"jarvis, Kreisquadratur",
	"jarvis@jabber.cz, djui@kreisquadratur.de",
	"© 2008 jarvis, © 2004 Kreisquadratur",
	"http://miranda-ng.org/",
	UNICODE_AWARE,    //not transient
	MIID_OPENFOLDER // {10896143-7249-4b36-A408-6501A6B6035A}
};

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
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
	TTBButton ttbb = { 0 };
	ttbb.cbSize = sizeof( ttbb );
	ttbb.hIconHandleUp = hIconOpenFolder;
	ttbb.pszService = MS_OPENFOLDER_OPEN;
	ttbb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttbb.name = LPGEN("Open Folder");
	TopToolbar_AddButton(&ttbb);
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName( hInst, szFile, MAX_PATH );

	char szSettingName[64];
	mir_snprintf(szSettingName, sizeof( szSettingName ), "%s_%s", OPENFOLDER_MODULE_NAME, "open");

	// icolib (0.7+)
	SKINICONDESC sid = { 0 };
	sid.cbSize = sizeof( SKINICONDESC );
	sid.ptszDefaultFile = szFile;
	sid.flags = SIDF_PATH_TCHAR;
	sid.cx = sid.cy = 16;
	sid.pszSection = LPGEN("Open Folder");
	sid.pszName = szSettingName;
	sid.pszDescription = LPGEN("Open Folder");
	sid.iDefaultIndex = -IDI_FOLDER;
	hIconOpenFolder = Skin_AddIcon(&sid);

	// hotkeys service (0.8+)
	HOTKEYDESC hotkey = { 0 };
	hotkey.cbSize = sizeof( hotkey );
	hotkey.pszName = LPGEN("Open Folder");
	hotkey.pszDescription = LPGEN("Open Folder");
	hotkey.pszSection = "Main";
	hotkey.pszService = MS_OPENFOLDER_OPEN;
	hotkey.DefHotKey = MAKEWORD( 'O', HOTKEYF_SHIFT | HOTKEYF_ALT );
	Hotkey_Register(&hotkey);

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof( mi );
	mi.position = 0x7FFFFFFF;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = hIconOpenFolder;
	mi.pszName = LPGEN("Open Folder");
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
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec( dllexport ) Unload()
{
	DestroyServiceFunction(hServiceOpenFolder);
	return 0;
}
