#include "headers.h"
#include "version.h"
#include "m_trigger.h"

HINSTANCE hInst;

int hLangpack;
TCHAR* profilePath;

HANDLE hFolder;
HANDLE hHooks[4];
HANDLE hServices[3];

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	__VERSION_DWORD,
	__PLUGIN_DESC,
	"chaos.persei, sje, Kildor, Billy_Bons",
	"chaos.persei@gmail.com",
	__COPYRIGHTS,
	"http://miranda-ng.org/",
	UNICODE_AWARE,		//doesn't replace anything built-in
    // Generate your own unique id for your plugin.
    // Do not use this UUID!
    // Use uuidgen.exe to generate the uuuid
    // {81C220A6-0226-4ad6-BFCA-217B17A16053}
	{ 0x81c220a6, 0x226, 0x4ad6, { 0xbf, 0xca, 0x21, 0x7b, 0x17, 0xa1, 0x60, 0x53 } }
};

struct
{
	TCHAR* szDescr;
	char* szName;
	int   defIconID;
}
static const iconList[] = {
	{	_T("Backup Profile"),           "backup",         IDI_ICON1    },
	{	_T("Save Profile As..."),       "saveas",         IDI_ICON1    }
};

INT_PTR BackupServiceTrgr(WPARAM wParam, LPARAM lParam)
{
	if(wParam & ACT_PERFORM) {
		return Backup(NULL);
	}
	return 0;
}

static int FoldersGetBackupPath(WPARAM wParam, LPARAM lParam)
{
	FoldersGetCustomPathT(hFolder, options.folder, MAX_PATH, DIR SUB_DIR);
	return 0;
}

static int FoldersInit(void)
{
	hFolder = (HANDLE) FoldersRegisterCustomPathT("Database Backups", "Backup Folder", DIR SUB_DIR);
	hHooks[0] = HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersGetBackupPath);
	FoldersGetBackupPath(0, 0);
	return 0;
}

static void IcoLibInit(void)
{
	TCHAR tszFile[MAX_PATH];
	GetModuleFileName(hInst, tszFile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = tszFile;
	sid.ptszSection = _T("Database/Database Backups");
	sid.flags = SIDF_ALL_TCHAR;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		sid.pszName = iconList[i].szName;
		sid.ptszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		Skin_AddIcon(&sid);
	}
}

static void MenuInit(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR;
	mi.hIcon = Skin_GetIcon("backup");
	mi.ptszPopupName = LPGENT("Database");

	mi.ptszName = LPGENT("Backup Profile");
	mi.pszService = MS_AB_BACKUP;
	mi.position = 500100000;
	Menu_AddMainMenuItem(&mi);

	mi.hIcon = Skin_GetIcon("saveas");
	mi.ptszName = LPGENT("Save Profile As...");
	mi.pszService = MS_AB_SAVEAS;
	mi.position = 500100001;
	Menu_AddMainMenuItem(&mi);
}

static void TriggerActionInit(void)
{
	ACTIONREGISTER ar = {0};
	ar.cbSize = sizeof(ACTIONREGISTER);
	ar.pszName = "Backup Database";
	ar.pszService = MS_AB_BACKUPTRGR;

	CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);
}

static int ModulesLoad(WPARAM wParam, LPARAM lParam)
{
	profilePath = Utils_ReplaceVarsT(_T("%miranda_userdata%"));

	IcoLibInit();
	if(ServiceExists(MS_FOLDERS_REGISTER_PATH))
		FoldersInit();
	LoadOptions();
	MenuInit();

	// register trigger action for triggerplugin
	if(ServiceExists(MS_TRIGGER_REGISTERACTION))
		TriggerActionInit();

	hHooks[1] = HookEvent(ME_OPT_INITIALISE, OptionsInit);
	if(options.backup_types & BT_START)
		Backup(NULL);
	return 0;
}

// can't do this on unload, since other plugins will be have already been unloaded, but their hooks
// for setting changed event not cleared. the backup on exit function will write to the db, calling those hooks.
int PreShutdown(WPARAM wParam, LPARAM lParam) {
	if(options.backup_types & BT_EXIT)
	{
		options.disable_popups = 1; // Don't try to show popups on exit
		Backup(NULL);
	}
	return 0;
}

void SysInit()
{
	mir_getLP( &pluginInfo );
	OleInitialize(0);

	hServices[0] = CreateServiceFunction(MS_AB_BACKUP, ABService);
	hServices[1] = CreateServiceFunction(MS_AB_BACKUPTRGR, BackupServiceTrgr);
	hServices[2] = CreateServiceFunction(MS_AB_SAVEAS, DBSaveAs);

	hHooks[2] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	hHooks[3] = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoad);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
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
	int i;

	OleUninitialize();

	for (i=0; i<SIZEOF(hHooks); ++i)
	{
		if (hHooks[i])
			UnhookEvent(hHooks[i]);
	}
	for (i=0; i<SIZEOF(hServices); ++i)
	{
		if (hServices[i])
			DestroyServiceFunction(hServices[i]);
	}

	return 0;
}

void ShowPopup(TCHAR* text, TCHAR* header)
{
	POPUPDATAT ppd = {0};

	lstrcpy(ppd.lptzText, text);
	lstrcpy(ppd.lptzContactName, header);
	ppd.lchIcon = Skin_GetIcon("backup");

	PUAddPopUpT(&ppd);
}

int CreateDirectoryTree(TCHAR *szDir)
{
	TCHAR *pszLastBackslash, szTestDir[ MAX_PATH ];

	lstrcpyn( szTestDir, szDir, SIZEOF( szTestDir ));
	pszLastBackslash = _tcsrchr( szTestDir, '\\' );
	if ( pszLastBackslash == NULL )
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTree( szTestDir );
	*pszLastBackslash = '\\';
	return ( CreateDirectory( szTestDir, NULL ) == 0 ) ? GetLastError() : 0;
}

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle)
{
	TOOLINFO ti = { 0 };
	HWND hwndTT;
    hwndTT = CreateWindowEx(WS_EX_TOPMOST,
        TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, NULL, hInst, NULL);

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
    ti.hwnd = hwndParent;
    ti.hinst = hInst;
    ti.lpszText = ptszText;
    GetClientRect (hwndParent, &ti.rect);
	ti.rect.left -= 80;

    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	SendMessage(hwndTT, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM)700);
	return hwndTT;
}