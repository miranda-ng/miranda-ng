/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 * YAMN homepage: http://www.majvan.host.sk/Projekty/YAMN
 *
 * initializes all variables for further work
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"

//--------------------------------------------------------------------------------------------------

TCHAR ProfileName[MAX_PATH];
TCHAR UserDirectory[MAX_PATH];

TCHAR	szMirandaDir[MAX_PATH];
TCHAR	szProfileDir[MAX_PATH];

int YAMN_STATUS;

BOOL UninstallPlugins;

HANDLE hAccountFolder;

HINSTANCE *hDllPlugins;
static int iDllPlugins = 0;


YAMN_VARIABLES YAMNVar;

int hLangpack;

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
	// {B047A7E5-027A-4CFC-8B18-EDA8345D2790}
	{0xb047a7e5, 0x27a, 0x4cfc, {0x8b, 0x18, 0xed, 0xa8, 0x34, 0x5d, 0x27, 0x90}}
};

HANDLE hNewMailHook;
HANDLE NoWriterEV;
HANDLE hTTButton;

UINT SecTimer;

HGENMENU hMenuItemMain = 0;
HGENMENU hMenuItemCont = 0;
HGENMENU hMenuItemContApp = 0;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

static void GetProfileDirectory(TCHAR *szPath, int cbPath)
//This is copied from Miranda's sources. In 0.2.1.0 it is needed, in newer vesions of Miranda use MS_DB_GETPROFILEPATH service
{
	TCHAR tszOldPath[MAX_PATH];
	CallService(MS_DB_GETPROFILEPATHT, SIZEOF(tszOldPath), (LPARAM)tszOldPath);
	_tcscat(tszOldPath, _T("\\*.book"));

	VARST ptszNewPath( _T("%miranda_userdata%"));

	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_MOVE,
		tszOldPath,
		ptszNewPath,
		FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_SILENT,
		false,
		0,
		_T("") };
	SHFileOperation(&file_op);

	_tcsncpy(szPath, ptszNewPath, cbPath);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	YAMNVar.hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
static TCHAR unknownCP[1500] = {0};
#endif
// The callback function
BOOL CALLBACK EnumSystemCodePagesProc(LPTSTR cpStr)
{
    //Convert code page string to number
    UINT cp = _ttoi(cpStr);
    if (!IsValidCodePage(cp))
        return TRUE;

    //Get Code Page name
    CPINFOEX info;
    if (GetCPInfoEx(cp, 0, &info)) {
		#ifdef _DEBUG
		BOOLEAN found = FALSE;
		#endif
		for (int i = 1;i<CPLENALL;i++) if (CodePageNamesAll[i].CP == cp) {
			CodePageNamesAll[i].isValid = TRUE;
			CPLENSUPP++;
			#ifdef _DEBUG
			found = TRUE;
			#endif
			break;
		}
		#ifdef _DEBUG
		if (!found) {
			_tcscat(unknownCP, info.CodePageName);
			_tcscat(unknownCP, _T("\n"));
		}
		#endif
	}
    return TRUE;
}

void CheckMenuItems()
{
	Menu_ShowItem(hMenuItemMain, db_get_b(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, 1) != 0);
}

int SystemModulesLoaded(WPARAM, LPARAM)
{
	//Insert "Check mail (YAMN)" item to Miranda's menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 0xb0000000;
	mi.icolibItem = g_GetIconHandle(0);
	mi.pszName = LPGEN("Check &mail (All Account)");
	mi.pszPopupName = NULL;//YAMN_DBMODULE;
	mi.pszService = MS_YAMN_FORCECHECK;
	hMenuItemMain = Menu_AddMainMenuItem(&mi);

	mi.pszName = LPGEN("Check &mail (This Account)");
	mi.pszContactOwner = YAMN_DBMODULE;
	mi.pszService = MS_YAMN_CLISTCONTEXT;
	hMenuItemCont = Menu_AddContactMenuItem(&mi);

	mi.icolibItem = g_GetIconHandle(1);
	mi.pszName = LPGEN("Launch application");
	mi.pszContactOwner = YAMN_DBMODULE;
	mi.pszService = MS_YAMN_CLISTCONTEXTAPP;
	hMenuItemContApp = Menu_AddContactMenuItem(&mi);

	CheckMenuItems();

	if (hAccountFolder = FoldersRegisterCustomPathT(LPGEN("YAMN"), LPGEN("YAMN Account Folder"), UserDirectory))
		FoldersGetCustomPathT(hAccountFolder, UserDirectory, MAX_PATH, UserDirectory);

	RegisterPOP3Plugin(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Check mail"),         "YAMN_Check",       IDI_CHECKMAIL  },
	{ LPGEN("Launch application"), "YAMN_Launch",      IDI_LAUNCHAPP  },
	{ LPGEN("New Mail"),           "YAMN_NewMail",     IDI_NEWMAIL    },
	{ LPGEN("Connect Fail"),       "YAMN_ConnectFail", IDI_BADCONNECT },
};

static void LoadIcons()
{
	Icon_Register(YAMNVar.hInst, "YAMN", iconList, SIZEOF(iconList));
}

HANDLE WINAPI g_GetIconHandle( int idx )
{
	if ( idx >= SIZEOF(iconList))
		return NULL;
	return iconList[idx].hIcolib;
}

HICON WINAPI g_LoadIconEx( int idx, bool big )
{
	if ( idx >= SIZEOF(iconList))
		return NULL;
	return Skin_GetIcon(iconList[idx].szName, big);
}

void WINAPI g_ReleaseIcon( HICON hIcon )
{
	if ( hIcon ) Skin_ReleaseIcon(hIcon);
}

static void LoadPlugins()
{
	TCHAR szSearchPath[MAX_PATH];
	mir_sntprintf(szSearchPath, MAX_PATH, _T("%s\\Plugins\\YAMN\\*.dll"), szMirandaDir);

	hDllPlugins = NULL;

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(szSearchPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			//rewritten from Miranda sources... Needed because Win32 API has a bug in FindFirstFile, search is done for *.dlllllll... too
			TCHAR *dot = _tcsrchr(fd.cFileName, '.');
			if (dot == NULL )
				continue;

			// we have a dot
			int len = mir_tstrlen(fd.cFileName); // find the length of the string
			TCHAR* end = fd.cFileName+len; // get a pointer to the NULL
			int safe = (end-dot)-1;	// figure out how many chars after the dot are "safe", not including NULL

			if ((safe != 3) || (lstrcmpi(dot+1, _T("dll")) != 0)) //not bound, however the "dll" string should mean only 3 chars are compared
				continue;

			TCHAR szPluginPath[MAX_PATH];
			mir_sntprintf(szPluginPath, MAX_PATH,_T("%s\\Plugins\\YAMN\\%s"), szMirandaDir, fd.cFileName);
			HINSTANCE hDll = LoadLibrary(szPluginPath);
			if (hDll == NULL)
				continue;

			LOADFILTERFCN LoadFilter = (LOADFILTERFCN) GetProcAddress(hDll, "LoadFilter");
			if (NULL == LoadFilter) {
				FreeLibrary(hDll);
				hDll = NULL;
				continue;
			}

			if (!(*LoadFilter)(GetFcnPtrSvc)) {
				FreeLibrary(hDll);
				hDll = NULL;
			}

			if (hDll != NULL) {
				hDllPlugins = (HINSTANCE *)realloc((void *)hDllPlugins, (iDllPlugins+1)*sizeof(HINSTANCE));
				hDllPlugins[iDllPlugins++] = hDll;
			}
		}
			while(FindNextFile(hFind, &fd));

		FindClose(hFind);
	}
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	YAMN_STATUS = ID_STATUS_OFFLINE;

	//	we get the Miranda Root Path
	PathToAbsoluteT( _T("."), szMirandaDir);

	// retrieve the current profile name
	CallService(MS_DB_GETPROFILENAMET, (WPARAM)SIZEOF(ProfileName), (LPARAM)ProfileName);	//not to pass entire array to fcn
	TCHAR *fc = _tcsrchr(ProfileName, '.');
	if ( fc != NULL ) *fc = 0;

	//	we get the user path where our yamn-account.book.ini is stored from mirandaboot.ini file
	GetProfileDirectory(UserDirectory, SIZEOF(UserDirectory));

	// Enumerate all the code pages available for the System Locale
	EnumSystemCodePages(EnumSystemCodePagesProc, CP_INSTALLED);
	CodePageNamesSupp = new _tcptable[CPLENSUPP];
	for (int i = 0, k = 0; i < CPLENALL; i++) {
		if (CodePageNamesAll[i].isValid) {
			CodePageNamesSupp[k] = CodePageNamesAll[i];
			k++;
	}	}

	// Registering YAMN as protocol
	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = YAMN_DBMODULE;
	pd.type = PROTOTYPE_VIRTUAL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	InitializeCriticalSection(&AccountStatusCS);
	InitializeCriticalSection(&FileWritingCS);
	InitializeCriticalSection(&PluginRegCS);

	if (NULL == (NoWriterEV = CreateEvent(NULL, TRUE, TRUE, NULL)))
		return 1;
	if (NULL == (WriteToFileEV = CreateEvent(NULL, FALSE, FALSE, NULL)))
		return 1;
	if (NULL == (ExitEV = CreateEvent(NULL, TRUE, FALSE, NULL)))
		return 1;

	PosX = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBPOSX, 0);
	PosY = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBPOSY, 0);
	SizeX = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBSIZEX, 800);
	SizeY = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBSIZEY, 200);

	HeadPosX = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSX, 0);
	HeadPosY = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSY, 0);
	HeadSizeX = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEX, 690);
	HeadSizeY = db_get_dw(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEY, 300);
	HeadSplitPos = db_get_w(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSSPLIT, 250);

	optDateTime = db_get_b(NULL, YAMN_DBMODULE, YAMN_DBTIMEOPTIONS, optDateTime);

	// Create new window queues for broadcast messages
	YAMNVar.MessageWnds = WindowList_Create();
	YAMNVar.NewMailAccountWnd = WindowList_Create();
	YAMNVar.Shutdown = FALSE;

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);

#ifdef _DEBUG
	InitDebug();
#endif

	CreateServiceFunctions();

	SkinAddNewSoundEx(YAMN_NEWMAILSOUND, YAMN_DBMODULE, YAMN_NEWMAILSNDDESC);
	SkinAddNewSoundEx(YAMN_CONNECTFAILSOUND, YAMN_DBMODULE, YAMN_CONNECTFAILSNDDESC);

	HookEvents();

	LoadIcons();
	LoadPlugins();

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.pszName = "YAMN_hotkey";
	hkd.pszService = MS_YAMN_FORCECHECK;
	hkd.pszSection = YAMN_DBMODULE;
	hkd.pszDescription = LPGEN("Check mail");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F11);
	Hotkey_Register(&hkd);

	//Create thread that will be executed every second
	if (!(SecTimer = SetTimer(NULL, 0, 1000, TimerProc)))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void UnloadPlugins()
{
	if (hDllPlugins == NULL)
		return;
	for (int i = iDllPlugins - 1; i >= 0; i --) {
		if (FreeLibrary(hDllPlugins[i])) {
			hDllPlugins[i] = NULL;				//for safety
			iDllPlugins --;
		}
	}
	free((void *)hDllPlugins);
	hDllPlugins = NULL;
}

extern "C" int __declspec(dllexport) Unload(void)
{
#ifdef _DEBUG
	UnInitDebug();
#endif

	WindowList_Destroy(YAMNVar.MessageWnds);
	WindowList_Destroy(YAMNVar.NewMailAccountWnd);

	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurSplitWE);

	CloseHandle(NoWriterEV);
	CloseHandle(WriteToFileEV);
	CloseHandle(ExitEV);

	DeleteCriticalSection(&AccountStatusCS);
	DeleteCriticalSection(&FileWritingCS);
	DeleteCriticalSection(&PluginRegCS);

	UnhookEvents();
	DestroyServiceFunctions();

	UnloadPlugins();

	delete [] CodePageNamesSupp;
	return 0;
}
