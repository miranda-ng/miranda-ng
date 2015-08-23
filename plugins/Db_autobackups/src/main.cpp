#include "stdafx.h"

int	hLangpack;
HINSTANCE g_hInstance;
TCHAR	*profilePath;
HANDLE	hFolder;

PLUGININFOEX pluginInfo = {
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
	{ 0x81c220a6, 0x226, 0x4ad6, { 0xbf, 0xca, 0x21, 0x7b, 0x17, 0xa1, 0x60, 0x53 } }
};

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

static INT_PTR ABService(WPARAM, LPARAM)
{
	BackupStart(NULL);
	return 0;
}

static INT_PTR DBSaveAs(WPARAM, LPARAM)
{
	TCHAR fname_buff[MAX_PATH], tszFilter[200];
	OPENFILENAME ofn = { 0 };
	CallService(MS_DB_GETPROFILENAMET, _countof(fname_buff), (LPARAM)fname_buff);

	mir_sntprintf(tszFilter, _countof(tszFilter), _T("%s (*.dat)%c*.dat%c%s (*.zip)%c*.zip%c%s (*.*)%c*%c"),
		TranslateT("Miranda NG databases"), 0, 0,
		TranslateT("Compressed Miranda NG databases"), 0, 0,
		TranslateT("All files"), 0, 0);

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = fname_buff;
	ofn.nMaxFile = _countof(fname_buff);
	ofn.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
	ofn.lpstrFilter = tszFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = _T("dat");

	if (GetSaveFileName(&ofn))
		BackupStart(fname_buff);
	return 0;
}

static int FoldersGetBackupPath(WPARAM, LPARAM)
{
	FoldersGetCustomPathT(hFolder, options.folder, _countof(options.folder), DIR SUB_DIR);
	return 0;
}

static int ModulesLoad(WPARAM, LPARAM)
{
	CMenuItem mi;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("Database"), 500100000);

	mi.name.a = LPGEN("Backup profile");
	mi.pszService = MS_AB_BACKUP;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.position = 500100000;
	Menu_AddMainMenuItem(&mi);

	mi.name.a = LPGEN("Save profile as...");
	mi.pszService = MS_AB_SAVEAS;
	mi.hIcolibItem = iconList[1].hIcolib;
	mi.position = 500100001;
	Menu_AddMainMenuItem(&mi);

	profilePath = Utils_ReplaceVarsT(_T("%miranda_userdata%"));

	if (hFolder = FoldersRegisterCustomPathT(LPGEN("Database backups"), LPGEN("Backup folder"), DIR SUB_DIR)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersGetBackupPath);
		FoldersGetBackupPath(0, 0);
	}

	if (options.backup_types & BT_START)
		BackupStart(NULL);
	return 0;
}

// can't do this on unload, since other plugins will be have already been unloaded, but their hooks
// for setting changed event not cleared. the backup on exit function will write to the db, calling those hooks.
static int PreShutdown(WPARAM, LPARAM)
{
	if (options.backup_types & BT_EXIT) {
		options.disable_popups = 1; // Don't try to show popups on exit
		BackupStart(NULL);
	}
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoad);

	Icon_Register(g_hInstance, LPGEN("Database") "/" LPGEN("Database backups"), iconList, _countof(iconList));

	CreateServiceFunction(MS_AB_BACKUP, ABService);
	CreateServiceFunction(MS_AB_SAVEAS, DBSaveAs);

	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	LoadOptions();

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}


