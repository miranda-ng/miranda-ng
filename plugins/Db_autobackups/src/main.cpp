#include "headers.h"

HINSTANCE hInst;

TCHAR *profilePath;
int hLangpack;

HANDLE hFolder;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {81C220A6-0226-4Ad6-BFCA-217B17A16053}
	{0x81c220a6, 0x226, 0x4ad6, {0xbf, 0xca, 0x21, 0x7b, 0x17, 0xa1, 0x60, 0x53}}
};

static IconItem iconList[] = {
	{ LPGEN("Backup profile"),     "backup", IDI_ICON1 },
	{ LPGEN("Save profile as..."), "saveas", IDI_ICON1 }
};

static int FoldersGetBackupPath(WPARAM, LPARAM)
{
	FoldersGetCustomPathT(hFolder, options.folder, MAX_PATH, DIR SUB_DIR);
	return 0;
}

static void FoldersInit(void)
{
	if (hFolder = FoldersRegisterCustomPathT(LPGEN("Database backups"), LPGEN("Backup folder"), DIR SUB_DIR)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersGetBackupPath);
		FoldersGetBackupPath(0, 0);
	}
}

static void MenuInit(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszPopupName = LPGEN("Database");

	mi.pszName = LPGEN("Backup profile");
	mi.pszService = MS_AB_BACKUP;
	mi.icolibItem = iconList[0].hIcolib;
	mi.position = 500100000;
	Menu_AddMainMenuItem(&mi);

	mi.pszName = LPGEN("Save profile as...");
	mi.pszService = MS_AB_SAVEAS;
	mi.icolibItem = iconList[1].hIcolib;
	mi.position = 500100001;
	Menu_AddMainMenuItem(&mi);
}

static int ModulesLoad(WPARAM, LPARAM)
{
	profilePath = Utils_ReplaceVarsT(_T("%miranda_userdata%"));

	MenuInit();
	FoldersInit();
	LoadOptions();

	if (options.backup_types & BT_START)
		mir_forkthread(BackupThread, NULL);
	return 0;
}

// can't do this on unload, since other plugins will be have already been unloaded, but their hooks
// for setting changed event not cleared. the backup on exit function will write to the db, calling those hooks.
int PreShutdown(WPARAM, LPARAM)
{
	if (options.backup_types & BT_EXIT) {
		options.disable_popups = 1; // Don't try to show popups on exit
		mir_forkthread(BackupThread, NULL);
	}
	return 0;
}

void SysInit()
{
	mir_getLP(&pluginInfo);
	OleInitialize(0);

	CreateServiceFunction(MS_AB_BACKUP, ABService);
	CreateServiceFunction(MS_AB_SAVEAS, DBSaveAs);

	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoad);

	Icon_Register(hInst, LPGEN("Database")"/"LPGEN("Database backups"), iconList, SIZEOF(iconList));
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	SysInit();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	OleUninitialize();
	return 0;
}

LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_COMMAND:
		{
			TCHAR* ptszPath = (TCHAR*)PUGetPluginData(hWnd);
			if (ptszPath != 0)
				ShellExecute(0, _T("open"), ptszPath, NULL, NULL, SW_SHOW);

			PUDeletePopup(hWnd);
			break;
		}
		case WM_CONTEXTMENU:
			PUDeletePopup(hWnd);
			break;

		case UM_FREEPLUGINDATA:
			TCHAR* ptszPath = (TCHAR*)PUGetPluginData(hWnd);
			mir_free(ptszPath);
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void ShowPopup(TCHAR* ptszText, TCHAR* ptszHeader, TCHAR* ptszPath)
{
	POPUPDATAT ppd = {0};

	lstrcpy(ppd.lptzText, ptszText);
	lstrcpy(ppd.lptzContactName, ptszHeader);
	if (ptszPath != NULL)
		ppd.PluginData = (void*)mir_tstrdup(ptszPath);
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = Skin_GetIconByHandle(iconList[0].hIcolib);

	PUAddPopupT(&ppd);
}

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, hInst, NULL);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFO ti = {0};
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
	ti.hwnd = hwndParent;
	ti.hinst = hInst;
	ti.lpszText = ptszText;
	GetClientRect (hwndParent, &ti.rect);
	ti.rect.left = -80;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	SendMessage(hwndTT, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM)650);
	return hwndTT;
}
