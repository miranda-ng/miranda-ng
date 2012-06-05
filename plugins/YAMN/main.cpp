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
#include "main.h"
#include "resources/resource.h"
#include <io.h>

#include "m_hotkeys.h"

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

PLUGINLINK *pluginLink;
YAMN_VARIABLES YAMNVar;

int hLangpack;
MM_INTERFACE mmi;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX), 
	YAMN_SHORTNAME, 
	YAMN_VERSION, 
	"Mail notifier and browser for Miranda IM. Included POP3 protocol.", 
	"y_b tweety (majvan)", 
	"francois.mean@skynet.be", 
	"© (2002-2004 majvan) 2005-2007 tweety y_b Miranda community", 
	"http://www.miranda-im.org/download/details.php?action = viewfile&id = 3411", //"http://www.majvan.host.sk/Projekty/YAMN?fm = soft", 
	UNICODE_AWARE, 
	0, 		//doesn't replace anything built-in
	{ 0xb047a7e5, 0x27a, 0x4cfc, { 0x8b, 0x18, 0xed, 0xa8, 0x34, 0x5d, 0x27, 0x90 } } // {B047A7E5-027A-4cfc-8B18-EDA8345D2790}

};

SKINSOUNDDESC NewMailSound  = 
{
	sizeof(SKINSOUNDDESC), 
	YAMN_NEWMAILSOUND, 	//name to refer to sound when playing and in db
	YAMN_NEWMAILSNDDESC, 	//description for options dialog
	"", 	   		//default sound file to use, without path
};

SKINSOUNDDESC ConnectFailureSound  = 
{
	sizeof(SKINSOUNDDESC), 
	YAMN_CONNECTFAILSOUND, 	//name to refer to sound when playing and in db
	YAMN_CONNECTFAILSNDDESC, //description for options dialog
	"", 	   		//default sound file to use, without path
};

HANDLE hNewMailHook;
HANDLE NoWriterEV;
HANDLE hTTButton, hTButton;

UINT SecTimer;

BOOL bIcolibEmbededInCore = FALSE;

HANDLE hMenuItemMain = 0;
HANDLE hMenuItemCont = 0;
HANDLE hMenuItemContApp = 0;

HMODULE hUxTheme = 0;
BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = 0;

// function pointers, use typedefs for casting to shut up the compiler when using GetProcAddress()

typedef BOOL (WINAPI *PITA)();
typedef HANDLE (WINAPI *POTD)(HWND, LPCWSTR);
typedef UINT (WINAPI *PDTB)(HANDLE, HDC, int, int, RECT *, RECT *);
typedef UINT (WINAPI *PCTD)(HANDLE);
typedef UINT (WINAPI *PDTT)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, RECT *);

PITA pfnIsThemeActive = 0;
POTD pfnOpenThemeData = 0;
PDTB pfnDrawThemeBackground = 0;
PCTD pfnCloseThemeData = 0;
PDTT pfnDrawThemeText = 0;

#define FIXED_TAB_SIZE 100                  // default value for fixed width tabs

/*
 * visual styles support (XP+)
 * returns 0 on failure
 */

int InitVSApi()
{
	if ((hUxTheme = LoadLibraryA("uxtheme.dll"))  ==  0)
		return 0;

	pfnIsThemeActive = (PITA)GetProcAddress(hUxTheme, "IsThemeActive");
	pfnOpenThemeData = (POTD)GetProcAddress(hUxTheme, "OpenThemeData");
	pfnDrawThemeBackground = (PDTB)GetProcAddress(hUxTheme, "DrawThemeBackground");
	pfnCloseThemeData = (PCTD)GetProcAddress(hUxTheme, "CloseThemeData");
	pfnDrawThemeText = (PDTT)GetProcAddress(hUxTheme, "DrawThemeText");

	MyEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
	if (pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0 && pfnDrawThemeText != 0)
		return 1;

	return 0;
}

/*
 * unload uxtheme.dll
 */

int FreeVSApi()
{
	if (hUxTheme != 0)
		FreeLibrary(hUxTheme);
	return 0;
}

//--------------------------------------------------------------------------------------------------

static void GetProfileDirectory(TCHAR *szPath, int cbPath)
//This is copied from Miranda's sources. In 0.2.1.0 it is needed, in newer vesions of Miranda use MS_DB_GETPROFILEPATH service
{
	if (ServiceExists(MS_DB_GETPROFILEPATH))
		if (!CallService(MS_DB_GETPROFILEPATHT, (WPARAM)cbPath, (LPARAM)szPath)) {
			lstrcpy(szProfileDir, szPath);
			return; //success
		}

	TCHAR szMirandaIni[MAX_PATH], szExpandedProfileDir[MAX_PATH];
	DWORD dwAttributes;

	lstrcpy(szMirandaIni, szMirandaDir);
	lstrcat(szMirandaIni, _T("\\mirandaboot.ini"));
	GetPrivateProfileString( _T("Database"), _T("ProfileDir"), _T("."), szProfileDir, sizeof(szProfileDir), szMirandaIni);
	ExpandEnvironmentStrings(szProfileDir, szExpandedProfileDir, sizeof(szExpandedProfileDir));
	_tchdir(szMirandaDir);
	if (!_tfullpath(szPath, szExpandedProfileDir, cbPath))
		lstrcpyn(szPath, szMirandaDir, cbPath);
	if (szPath[lstrlen(szPath)-1] == '\\') szPath[lstrlen(szPath)-1] = '\0';
	if ((dwAttributes = GetFileAttributes(szPath))!=0xffffffff&&dwAttributes&FILE_ATTRIBUTE_DIRECTORY) return;
	CreateDirectory(szPath, NULL);
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
	if (mirandaVersion >= PLUGIN_MAKE_VERSION(0, 7, 0, 3))
		bIcolibEmbededInCore = TRUE;
	pluginInfo.cbSize = sizeof(PLUGININFOEX);//Make sure cbSize is correct;
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const MUUID interfaces[] = {MUUID_YAMN_FORCECHECK, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef YAMN_DEBUG
static char unknownCP[1500] = {0};
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
		#ifdef YAMN_DEBUG
		BOOLEAN found = FALSE;
		#endif
		for (int i = 1;i<CPLENALL;i++) if (CodePageNamesAll[i].CP == cp) {
			CodePageNamesAll[i].isValid = TRUE;
			CPLENSUPP++;
			#ifdef YAMN_DEBUG
			found = TRUE;
			#endif
			break;
		}
		#ifdef YAMN_DEBUG
		if (!found) {
			strcat(unknownCP, info.CodePageName);
			strcat(unknownCP, "\n");
		}
		#endif
	}
    return TRUE;
}

void CheckMenuItems()
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS;
	if ( !DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, 1))
		clmi.flags |= CMIF_HIDDEN;

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMenuItemMain, ( LPARAM )&clmi );
}

int SystemModulesLoaded(WPARAM, LPARAM)
{
	//Insert "Check mail (YAMN)" item to Miranda's menu
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = 0xb0000000;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = g_GetIconHandle(5);
	mi.pszName = "Check &mail (All Account)";
	mi.pszPopupName = NULL;//YAMN_DBMODULE;
	mi.pszService = MS_YAMN_FORCECHECK;
	hMenuItemMain = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&mi);

	mi.pszName = "Check &mail (This Account)";
	mi.pszContactOwner = YAMN_DBMODULE;
	mi.pszService = MS_YAMN_CLISTCONTEXT;
	hMenuItemCont = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mi.icolibItem = g_GetIconHandle(4);
	mi.pszName = "Launch application";
	mi.pszContactOwner = YAMN_DBMODULE;
	mi.pszService = MS_YAMN_CLISTCONTEXTAPP;
	hMenuItemContApp = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	CheckMenuItems();

	//Use for the Updater plugin
	if (ServiceExists(MS_UPDATE_REGISTER)) {
		Update update = {0};
		char szVersion[16], szUrl[250];
		wsprintfA(szUrl, "http://www.miranda-fr.net/tweety/yamn/%s.zip", YAMN_FILENAME);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO *)&pluginInfo, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile = 3411";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action = viewfile&id = 3411";
		update.pbVersionPrefix = (BYTE *)"<span class = \"fileNameHeader\">YAMN 2in1 ";
		update.szBetaUpdateURL = szUrl;
		update.szBetaVersionURL = "http://www.miranda-fr.net/tweety/yamn/yamn_beta.html";
		update.pbBetaVersionPrefix = (BYTE *)"YAMN version ";
		update.cpbVersionPrefix = (int)strlen((char *)update.pbVersionPrefix);
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);
		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	if (ServiceExists(MS_FOLDERS_GET_PATH)) {
		hAccountFolder = FoldersRegisterCustomPathT(YAMN_DBMODULE, YAMN_DBMODULE" Account Folder", UserDirectory);
		FoldersGetCustomPathT(hAccountFolder, UserDirectory, MAX_PATH, UserDirectory);
	}

	RegisterPOP3Plugin(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct TIconListItem
{
	char*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIcon;
};

static TIconListItem iconList[] = 
{
	{ LPGEN("Neutral"),            "YAMN_Neutral",        IDI_ONLINE,   0 },
	{ LPGEN("YAMN"),               "YAMN",                IDI_ICOYAMN1, 0 },
	{ LPGEN("New Mail"),           "YAMN_NewMail",        IDI_ICOYAMN2, 0 },
	{ LPGEN("Connect Fail"),       "YAMN_ConnectFail",    IDI_NA,       0 },
	{ LPGEN("Launch Application"), "YAMN_ApplExec",       IDI_OCCUPIED, 0 },
	{ LPGEN("TopToolBar UP"),      "YAMN_TopToolBarUp",   IDI_ICOTTBUP, 0 },
	{ LPGEN("TopToolBar Down"),    "YAMN_TopToolBarDown", IDI_OCCUPIED, 0 },
	{ LPGEN("Offline"),            "YAMN_Offline",        IDI_OFFLINE,  0 }
};

static void LoadIcons()
{
	HIMAGELIST CSImages = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 3);
	{
		HBITMAP hScrBM = (HBITMAP)LoadImage(YAMNVar.hInst, MAKEINTRESOURCE(IDB_ICONS), IMAGE_BITMAP, 0, 0, LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);
	}

	char szFile[MAX_PATH];
	GetModuleFileNameA(YAMNVar.hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszDefaultFile = szFile;
	sid.pszSection = "YAMN";

	for (int i = 0, k = 0; i < ICONSNUMBER; i++) {
		switch (i){
		case 1: case 2: case 5: 
			sid.hDefaultIcon = ImageList_ExtractIcon(NULL, CSImages, k); k++;
			break;
		default:
			sid.hDefaultIcon = LoadIcon(YAMNVar.hInst, MAKEINTRESOURCE(iconList[i].defIconID)); break;
		}
		sid.pszName = iconList[i].szName;
		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIcon = ( HANDLE )CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
}	}

HANDLE WINAPI g_GetIconHandle( int idx )
{
	if ( idx >= SIZEOF(iconList))
		return NULL;
	return iconList[idx].hIcon;
}

HICON WINAPI g_LoadIconEx( int idx, bool big )
{
	if ( idx >= SIZEOF(iconList))
		return NULL;
	return ( HICON )CallService(MS_SKIN2_GETICON, big, (LPARAM)iconList[idx].szName);
}

void WINAPI g_ReleaseIcon( HICON hIcon )
{
	if ( hIcon ) CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}

static void LoadPlugins()
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	TCHAR szSearchPath[MAX_PATH];
	TCHAR szPluginPath[MAX_PATH];
	lstrcpy(szSearchPath, szMirandaDir);
	lstrcat(szSearchPath, _T("\\Plugins\\YAMN\\*.dll"));
	typedef INT_PTR (*LOADFILTERFCN)(MIRANDASERVICE GetYAMNFcn);

	hDllPlugins = NULL;

	if (INVALID_HANDLE_VALUE!=(hFind = FindFirstFile(szSearchPath, &fd))) {
		do {
			//rewritten from Miranda sources... Needed because Win32 API has a bug in FindFirstFile, search is done for *.dlllllll... too
			TCHAR *dot = _tcsrchr(fd.cFileName, '.');
			if (dot == NULL )
				continue;
			
			// we have a dot
			int len = (int)lstrlen(fd.cFileName); // find the length of the string
			TCHAR* end = fd.cFileName+len; // get a pointer to the NULL
			int safe = (end-dot)-1;	// figure out how many chars after the dot are "safe", not including NULL
		
			if ((safe!=3) || (lstrcmpi(dot+1, _T("dll"))!=0)) //not bound, however the "dll" string should mean only 3 chars are compared
				continue;
			
			HINSTANCE hDll;
			LOADFILTERFCN LoadFilter;

			lstrcpy(szPluginPath, szMirandaDir);
			lstrcat(szPluginPath, _T("\\Plugins\\YAMN\\"));
			lstrcat(szPluginPath, fd.cFileName);
			if ((hDll = LoadLibrary(szPluginPath)) == NULL) continue;
			LoadFilter = (LOADFILTERFCN)GetProcAddress(hDll, "LoadFilter");
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

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	int i, k;

	pluginLink = link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);

	YAMN_STATUS = ID_STATUS_OFFLINE;

	//	we get the Miranda Root Path
	if (ServiceExists(MS_UTILS_PATHTOABSOLUTET))
		CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)_T("."), (LPARAM)szMirandaDir);
	else {
		GetModuleFileName(GetModuleHandle(NULL), szMirandaDir, MAX_PATH);
		TCHAR* str2 = _tcsrchr(szMirandaDir, '\\');
		if (str2!=NULL) *str2 = 0;
	}

	//	we get the user path where our yamn-account.book.ini is stored from mirandaboot.ini file
	GetProfileDirectory(UserDirectory, SIZEOF(UserDirectory));
	
	// Enumerate all the code pages available for the System Locale
	EnumSystemCodePages(EnumSystemCodePagesProc, CP_INSTALLED);
	CodePageNamesSupp = new _tcptable[CPLENSUPP];
	for (i = 0, k = 0; i < CPLENALL; i++) {
		if (CodePageNamesAll[i].isValid){
			CodePageNamesSupp[k] = CodePageNamesAll[i];
			k++;
	}	}

	// Registering YAMN as protocol
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = PROTOCOLDESCRIPTOR_V3_SIZE;
	pd.szName = YAMN_DBMODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CallService(MS_DB_GETPROFILENAMET, (WPARAM)SIZEOF(ProfileName), (LPARAM)ProfileName);	//not to pass entire array to fcn
	TCHAR *fc = _tcsrchr(ProfileName, '.');
	if ( fc != NULL ) *fc = 0;

	InitializeCriticalSection(&AccountStatusCS);
	InitializeCriticalSection(&FileWritingCS);
	InitializeCriticalSection(&PluginRegCS);

	if (NULL == (NoWriterEV = CreateEvent(NULL, TRUE, TRUE, NULL)))
		return 1;
	if (NULL == (WriteToFileEV = CreateEvent(NULL, FALSE, FALSE, NULL)))
		return 1;
	if (NULL == (ExitEV = CreateEvent(NULL, TRUE, FALSE, NULL)))
		return 1;

	NewMailSound.pszDescription = Translate(YAMN_NEWMAILSNDDESC);
	ConnectFailureSound.pszDescription = Translate(YAMN_CONNECTFAILSNDDESC);

	PosX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBPOSX, 0);
	PosY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBPOSY, 0);
	SizeX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBSIZEX, 800);
	SizeY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBSIZEY, 200);

	HeadPosX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSX, 0);
	HeadPosY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSY, 0);
	HeadSizeX = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEX, 690);
	HeadSizeY = DBGetContactSettingDword(NULL, YAMN_DBMODULE, YAMN_DBMSGSIZEY, 300);
	HeadSplitPos = DBGetContactSettingWord(NULL, YAMN_DBMODULE, YAMN_DBMSGPOSSPLIT, 250);

	optDateTime = DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_DBTIMEOPTIONS, optDateTime);

	// Create new window queues for broadcast messages
	YAMNVar.MessageWnds = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	YAMNVar.NewMailAccountWnd = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	YAMNVar.Shutdown = FALSE;

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);

#ifdef YAMN_DEBUG
	InitDebug();
#endif

	CreateServiceFunctions();

	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&NewMailSound);
	CallService(MS_SKIN_ADDNEWSOUND, 0, (LPARAM)&ConnectFailureSound);

	HookEvents();

	LoadIcons();
	LoadPlugins();
	InitVSApi();

	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.pszService = MS_YAMN_FORCECHECK;
	hkd.pszSection = YAMN_DBMODULE;
	hkd.pszDescription = LPGEN("Check mail");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F11);
	CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hkd);

	//Create thread that will be executed every second
	if (!(SecTimer = SetTimer(NULL, 0, 1000, (TIMERPROC)TimerProc)))
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void UnloadPlugins()
{
	for (int i = iDllPlugins-1;i>=0;i--) {
		if (FreeLibrary(hDllPlugins[i])) {
			hDllPlugins[i] = NULL;				//for safety
			iDllPlugins --;
		}
	}
	if (hDllPlugins){
		free((void *)hDllPlugins);
		hDllPlugins = NULL;
	}
}

extern "C" int __declspec(dllexport) Unload(void)
{
#ifdef YAMN_DEBUG
	UnInitDebug();
#endif
	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurSplitWE);

	CloseHandle(NoWriterEV);
	CloseHandle(WriteToFileEV);
	CloseHandle(ExitEV);

	FreeVSApi();

	DeleteCriticalSection(&AccountStatusCS);
	DeleteCriticalSection(&FileWritingCS);
	DeleteCriticalSection(&PluginRegCS);

	UnhookEvents();
	DestroyServiceFunctions();

	UnloadPlugins();

	delete [] CodePageNamesSupp;
	return 0;
}
