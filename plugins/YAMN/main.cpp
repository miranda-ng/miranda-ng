/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 * YAMN homepage: http://www.majvan.host.sk/Projekty/YAMN
 *
 * initializes all variables for further work
 *
 * (c) majvan 2002-2004
 */


#include "main.h"
#include "yamn.h"
#include "resources/resource.h"
#include <io.h>
//- imported ---------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//CRITICAL_SECTION MWCS;
//CRITICAL_SECTION ASCS;
//CRITICAL_SECTION PRCS;

extern LPCRITICAL_SECTION PluginRegCS;
extern HANDLE ExitEV;
extern HANDLE WriteToFileEV;

extern int PosX,PosY,SizeX,SizeY;
extern int HeadPosX,HeadPosY,HeadSizeX,HeadSizeY,HeadSplitPos;

//From account.cpp
extern LPCRITICAL_SECTION AccountStatusCS;
extern LPCRITICAL_SECTION FileWritingCS;
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

WCHAR	*ProfileName		= NULL;		//e.g. "majvan"
WCHAR	*UserDirectory		= NULL;		//e.g. "F:\WINNT\Profiles\UserXYZ"
char	*ProtoName			= YAMN_DBMODULE;
//char *AltProtoName;
char	*szMirandaDir		= NULL;
char	*szProfileDir		= NULL;

INT_PTR YAMN_STATUS;

BOOL UninstallPlugins;

HANDLE hAccountFolder;

HINSTANCE *hDllPlugins;
static int iDllPlugins=0;

PLUGINLINK *pluginLink;
YAMN_VARIABLES YAMNVar;

int hLangpack;
struct MM_INTERFACE mmi;

static const MUUID interfaces[] = {MUUID_YAMN_FORCECHECK, MIID_LAST};

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	YAMN_SHORTNAME,
	YAMN_VERSION,
	"Mail notifier and browser for Miranda IM. Included POP3 protocol. Mod for Mataes Pack.",
	"y_b tweety (majvan)",
	"francois.mean@skynet.be",
	"© (2002-2004 majvan) 2005-2007 tweety y_b Miranda community",
	"http://www.miranda-im.org/download/details.php?action=viewfile&id=3411", //"http://www.majvan.host.sk/Projekty/YAMN?fm=soft",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	{ 0xb047a7e5, 0x27a, 0x4cfc, { 0x8b, 0x18, 0xed, 0xa8, 0x34, 0x5d, 0x27, 0x90 } } // {B047A7E5-027A-4cfc-8B18-EDA8345D2790}

};

SKINSOUNDDESC NewMailSound={
	sizeof(SKINSOUNDDESC),
	YAMN_NEWMAILSOUND,	//name to refer to sound when playing and in db
	YAMN_NEWMAILSNDDESC,	//description for options dialog
	"",	   		//default sound file to use, without path
};

SKINSOUNDDESC ConnectFailureSound={
	sizeof(SKINSOUNDDESC),
	YAMN_CONNECTFAILSOUND,	//name to refer to sound when playing and in db
	YAMN_CONNECTFAILSNDDESC,//description for options dialog
	"",	   		//default sound file to use, without path
};

HANDLE hNewMailHook;
//HANDLE hUninstallPluginsHook;

HANDLE NoWriterEV;

HANDLE hTTButton;		//TopToolBar button

DWORD HotKeyThreadID;

UINT SecTimer;

BOOL bIcolibEmbededInCore = FALSE;

HICON hYamnIcons[ICONSNUMBER];
char *iconDescs[ICONSNUMBER]={ICONSDESCS};
char *iconNames[ICONSNUMBER]={ICONSNAMES};
 int iconIndexes[ICONSNUMBER]={ICONSINDS};

HANDLE hMenuItemMain = 0;
HANDLE hMenuItemCont = 0;
HANDLE hMenuItemContApp = 0;

BOOL     (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = 0;
HMODULE hUxTheme = 0;

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
    if((hUxTheme = LoadLibraryA("uxtheme.dll")) == 0)
        return 0;

    pfnIsThemeActive = (PITA)GetProcAddress(hUxTheme, "IsThemeActive");
    pfnOpenThemeData = (POTD)GetProcAddress(hUxTheme, "OpenThemeData");
    pfnDrawThemeBackground = (PDTB)GetProcAddress(hUxTheme, "DrawThemeBackground");
    pfnCloseThemeData = (PCTD)GetProcAddress(hUxTheme, "CloseThemeData");
    pfnDrawThemeText = (PDTT)GetProcAddress(hUxTheme, "DrawThemeText");
    
    MyEnableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
    if(pfnIsThemeActive != 0 && pfnOpenThemeData != 0 && pfnDrawThemeBackground != 0 && pfnCloseThemeData != 0 && pfnDrawThemeText != 0) {
        return 1;
    }
    return 0;
}

/*
 * unload uxtheme.dll
 */

int FreeVSApi()
{
    if(hUxTheme != 0)
        FreeLibrary(hUxTheme);
    return 0;
}


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static void GetProfileDirectory(char *szPath,int cbPath)
//This is copied from Miranda's sources. In 0.2.1.0 it is needed, in newer vesions of Miranda use MS_DB_GETPROFILEPATH service
{
	szProfileDir=new char[MAX_PATH];
	if (ServiceExists(MS_DB_GETPROFILEPATH)){
		if (!CallService(MS_DB_GETPROFILEPATH,(WPARAM)cbPath,(LPARAM)szPath)) {
			lstrcpy(szProfileDir,szPath);
			return; //success
		}
	}
	char szMirandaIni[MAX_PATH],szExpandedProfileDir[MAX_PATH];
	DWORD dwAttributes;

	lstrcpy(szMirandaIni,szMirandaDir);
	lstrcat(szMirandaIni,"\\mirandaboot.ini");
	GetPrivateProfileString("Database","ProfileDir",".",szProfileDir,sizeof(szProfileDir),szMirandaIni);
	ExpandEnvironmentStrings(szProfileDir,szExpandedProfileDir,sizeof(szExpandedProfileDir));
	_chdir(szMirandaDir);
	if(!_fullpath(szPath,szExpandedProfileDir,cbPath))
		lstrcpyn(szPath,szMirandaDir,cbPath);
	if(szPath[lstrlen(szPath)-1]=='\\') szPath[lstrlen(szPath)-1]='\0';
	if((dwAttributes=GetFileAttributes(szPath))!=0xffffffff&&dwAttributes&FILE_ATTRIBUTE_DIRECTORY) return;
	CreateDirectory(szPath,NULL);
}

void SetDefaultProtocolIcons()
{
	char szFileName[MAX_PATH+1];
	char oldname[] = YAMN_DBMODULE"4007_"; // the deprecated one
	char dllname[] = "plugins\\"YAMN_DBMODULE".dll,-xxxxx";

	// determine whether external icon file exists
	lstrcpy(szFileName, szMirandaDir);
	lstrcat(szFileName, "\\icons\\proto_"YAMN_DBMODULE".dll");
	BOOL isDllPresent = (_access(szFileName, 0) == 0);

	WORD statuses[4] = {ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_NA,ID_STATUS_OCCUPIED};
	BYTE  indices[4] = {7,                0,               3,           4};
	//From skinicons.c skinIconStatusToIdStatus[] 
	BYTE protoStatusInd[4] = {0,1,4,5};

	for (int i=0;i<4;i++){
		oldname[sizeof(oldname)-2]=protoStatusInd[i]+'1'; // "Out for lunch will not work here"
		if (isDllPresent){ // use the icons in proto_YAMN.dll and delete any user settings
			DBDeleteContactSetting(NULL, "Icons", oldname);
		} else {
			DBVARIANT dbv;
			if(!DBGetContactSetting(NULL,"SkinIcons",iconNames[indices[i]],&dbv)) 
			{// user won't be able to set status icons different from those in YAMN section
				DBWriteContactSettingString(NULL, "Icons", oldname, (char *)dbv.pszVal);			
				DBFreeVariant(&dbv);
			} else {
				_snprintf(&dllname[sizeof(dllname)-6],5,"%d",iconIndexes[indices[i]]);
				DBWriteContactSettingString(NULL, "Icons", oldname, dllname);			
			}
		}
	}
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	YAMNVar.hInst=hinstDLL;
	if(fdwReason==DLL_PROCESS_ATTACH)
	{
		if(NULL==(UserDirectory=new WCHAR[MAX_PATH]))
			return FALSE;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	if (mirandaVersion >= PLUGIN_MAKE_VERSION(0, 7, 0, 3))
		bIcolibEmbededInCore = TRUE;
	pluginInfo.cbSize = sizeof(PLUGININFO);//Miranda pre-0.7.0.17 does not load the plugin if cbSize does not match.
	return (PLUGININFO *) &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion >= PLUGIN_MAKE_VERSION(0, 7, 0, 3))
		bIcolibEmbededInCore = TRUE;
	pluginInfo.cbSize = sizeof(PLUGININFOEX);//Make sure cbSize is correct;
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

#ifdef YAMN_DEBUG
static char unknownCP[1500]={0};
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
    if(GetCPInfoEx(cp,0,&info)){
		#ifdef YAMN_DEBUG
		BOOLEAN found = FALSE;
		#endif
		for (int i=1;i<CPLENALL;i++) if (CodePageNamesAll[i].CP==cp) {
			CodePageNamesAll[i].isValid = TRUE;
			CPLENSUPP++;
			#ifdef YAMN_DEBUG
			found = TRUE;
			#endif
			break;
		}
		#ifdef YAMN_DEBUG
		if (!found) {
			strcat(unknownCP,info.CodePageName);
			strcat(unknownCP,"\n");
		}
		#endif
	}
    return TRUE;
}

int SystemModulesLoaded(WPARAM,LPARAM){
	if(ServiceExists(MS_SKIN2_ADDICON))
	{
		//MessageBox(NULL,"Icolib present","test",0);
		SKINICONDESC sid = {0};
		HICON temp;
		sid.cbSize = SKINICONDESC_SIZE; 
		sid.pszSection = "YAMN";
		sid.pszDefaultFile = NULL;
		for (int i=0; i<ICONSNUMBER; i++){
			sid.iDefaultIndex = -iconIndexes[i];
			sid.pszName = iconNames[i];
			sid.pszDescription = iconDescs[i];
			sid.hDefaultIcon = hYamnIcons[i];
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
			if (temp = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) iconNames[i]))hYamnIcons[i]=temp; 
		}
	}

	CLISTMENUITEM mi;

	//Insert "Check mail (YAMN)" item to Miranda's menu
	ZeroMemory(&mi,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 0xb0000000;
	mi.flags = 0;
	mi.hIcon = hYamnIcons[5];
	mi.pszName = "Check &mail (All Account)";
	mi.pszPopupName = NULL;//ProtoName;
	mi.pszService = MS_YAMN_FORCECHECK;
	if(DBGetContactSettingByte(NULL, YAMN_DBMODULE, YAMN_SHOWMAINMENU, 1))
		hMenuItemMain = (HANDLE) CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	mi.pszName = "Check &mail (This Account)";
	mi.pszContactOwner = ProtoName;
	mi.pszService = MS_YAMN_CLISTCONTEXT;
	hMenuItemCont = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	mi.hIcon = hYamnIcons[4];
	mi.pszName = "Launch application";
	mi.pszContactOwner = ProtoName;
	mi.pszService = MS_YAMN_CLISTCONTEXTAPP;
	hMenuItemContApp = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&mi);

	//Use for the Updater plugin
	if(ServiceExists(MS_UPDATE_REGISTER)) 
	{
		Update update = {0};
		char szVersion[16];
		char szUrl[250];

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionStringPlugin((PLUGININFO *)&pluginInfo, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);
		/*#ifdef YAMN_9x
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=2166";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=2166";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">YAMN tweety win9x ";
		#else
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=2165";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=2165";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">YAMN tweety ";
		#endif*/
		update.szUpdateURL = "http://addons.miranda-im.org/feed.php?dlfile=3411";
		update.szVersionURL = "http://addons.miranda-im.org/details.php?action=viewfile&id=3411";
		update.pbVersionPrefix = (BYTE *)"<span class=\"fileNameHeader\">YAMN 2in1 ";
		wsprintf(szUrl,"http://www.miranda-fr.net/tweety/yamn/%s.zip",YAMN_FILENAME);
	    update.szBetaUpdateURL = szUrl;
		update.szBetaVersionURL = "http://www.miranda-fr.net/tweety/yamn/yamn_beta.html";
		update.pbBetaVersionPrefix = (BYTE *)"YAMN version ";
		update.cpbVersionPrefix = (int)strlen((char *)update.pbVersionPrefix);
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	}
	if (ServiceExists(MS_FOLDERS_GET_PATH)){
		//char AccountFolder[MAX_PATH];
		//CallService(MS_DB_GETPROFILEPATH, (WPARAM) MAX_PATH, (LPARAM)AccountFolder);
		//sprintf(AccountFolder,"%s\\%s",AccountFolder,ProtoName);
		hAccountFolder = FoldersRegisterCustomPathW(ProtoName,YAMN_DBMODULE" Account Folder", UserDirectory);
		
		FoldersGetCustomPathW(hAccountFolder,  UserDirectory, MAX_PATH, UserDirectory);
		//MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,AccountFolder,-1,UserDirectory,strlen(AccountFolder)+1);
	}

	RegisterPOP3Plugin(0,0);

	return 0;
}

//int IcoLibIconsChanged(WPARAM wParam, LPARAM lParam); // implemented in services.cpp
extern HCURSOR hCurSplitNS, hCurSplitWE;
extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	UINT mod,vk;
	char pn[MAX_PATH+1];
	char *fc;
	int i,k;

	pluginLink=link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);

	YAMN_STATUS = ID_STATUS_OFFLINE;

	//	we get the Miranda Root Path
	szMirandaDir=new char[MAX_PATH];
	if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)){
		CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)".",(LPARAM)szMirandaDir);
	}
	else {
		char *str2;
		GetModuleFileName(GetModuleHandle(NULL),szMirandaDir,MAX_PATH);
		str2=strrchr(szMirandaDir,'\\');
		if(str2!=NULL) *str2=0;
	}

	//	we get the user path where our yamn-account.book.ini is stored from mirandaboot.ini file
	char szProfileDir[MAX_PATH+1];
	GetProfileDirectory(szProfileDir,sizeof(szProfileDir));
	MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,szProfileDir,-1,UserDirectory,(int)strlen(szProfileDir)+1);

	
	// Enumerate all the code pages available for the System Locale
	EnumSystemCodePages(EnumSystemCodePagesProc, CP_INSTALLED);
	CodePageNamesSupp = new _tcptable[CPLENSUPP];
	for (i=0,k=0;i<CPLENALL;i++) {
		if (CodePageNamesAll[i].isValid){
			CodePageNamesSupp[k]=CodePageNamesAll[i];
			k++;
		}
	}
	#ifdef YAMN_DEBUG
//unknownCP	0x6005a734
//20127 (US-ASCII)
//20261 (T.61)
//28605 (ISO 8859-15 Latin 9)
//737   (OEM - Greek 437G)
//874   (ANSI/OEM - Thai)
//932   (ANSI/OEM - Japanese Shift-JIS)
//936   (ANSI/OEM - Simplified Chinese GBK)
//949   (ANSI/OEM - Korean)
    MessageBox( NULL,unknownCP, TEXT("Unkown Code Page"), MB_OK);
	#endif

	HIMAGELIST CSImages = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK, 0, 3);
	{// workarround of 4bit forced images
		HBITMAP hScrBM = (HBITMAP)LoadImage(YAMNVar.hInst,MAKEINTRESOURCE(IDB_ICONS), IMAGE_BITMAP, 0, 0,LR_SHARED);
		ImageList_AddMasked(CSImages, hScrBM, RGB( 255, 0, 255 ));
		DeleteObject(hScrBM);
	}
	for (i=0,k=0; i<ICONSNUMBER; i++){
		switch (i){
			case 0: case 3: case 4: case 7: hYamnIcons[i] = LoadIcon(YAMNVar.hInst,MAKEINTRESOURCE(iconIndexes[i])); break;
			case 6: hYamnIcons[i] = hYamnIcons[4]; break;
			default: hYamnIcons[i] = ImageList_ExtractIcon(NULL, CSImages, k); k++;
		}
	}

	//Registering YAMN as protocol
	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize=PROTOCOLDESCRIPTOR_V3_SIZE;
	pd.szName=ProtoName;
	pd.type=PROTOTYPE_PROTOCOL;
	
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	if(NULL==(ProfileName=new WCHAR[MAX_PATH]))
		return 1;

	CallService(MS_DB_GETPROFILENAME,(WPARAM)sizeof(pn),(LPARAM)&(*pn));	//not to pass entire array to fcn
	if(NULL!=(fc=strrchr(pn,(int)'.')))
		*fc=0;
	MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,pn,-1,ProfileName,(int)strlen(pn)+1);

	if(NULL==(AccountStatusCS=new CRITICAL_SECTION))
		return 1;
	if(NULL==(FileWritingCS=new CRITICAL_SECTION))
		return 1;
	if(NULL==(PluginRegCS=new CRITICAL_SECTION))
		return 1;

	InitializeCriticalSection(AccountStatusCS);
	InitializeCriticalSection(FileWritingCS);
	InitializeCriticalSection(PluginRegCS);

	if(NULL==(NoWriterEV=CreateEvent(NULL,TRUE,TRUE,NULL)))
		return 1;
	if(NULL==(WriteToFileEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
		return 1;
	if(NULL==(ExitEV=CreateEvent(NULL,TRUE,FALSE,NULL)))
		return 1;
//	AccountWriterSO=new SCOUNTER(NoWriterEV);

	NewMailSound.pszDescription=Translate(YAMN_NEWMAILSNDDESC);
	ConnectFailureSound.pszDescription=Translate(YAMN_CONNECTFAILSNDDESC);

	PosX=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBPOSX,0);
	PosY=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBPOSY,0);
	SizeX=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBSIZEX,800);
	SizeY=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBSIZEY,200);

	HeadPosX=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGPOSX,0);
	HeadPosY=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGPOSY,0);
	HeadSizeX=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGSIZEX,690);
	HeadSizeY=DBGetContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGSIZEY,300);
	HeadSplitPos=DBGetContactSettingWord(NULL,YAMN_DBMODULE,YAMN_DBMSGPOSSPLIT,250);

	optDateTime=DBGetContactSettingByte(NULL,YAMN_DBMODULE,YAMN_DBTIMEOPTIONS,optDateTime);

//Create new window queues for broadcast messages
	YAMNVar.MessageWnds=(HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	YAMNVar.NewMailAccountWnd=(HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST,0,0);
	YAMNVar.Shutdown=FALSE;

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);

#ifdef YAMN_DEBUG
	InitDebug();
#endif


	CreateServiceFunctions();

	CallService(MS_SKIN_ADDNEWSOUND,0,(LPARAM)&NewMailSound);
	CallService(MS_SKIN_ADDNEWSOUND,0,(LPARAM)&ConnectFailureSound);

	/*
	// this does nothing - these event are never fired
	hNewMailHook=CreateHookableEvent(ME_YAMN_NEWMAIL);
	hUninstallPluginsHook=CreateHookableEvent(ME_YAMN_UNINSTALLPLUGINS);
	*/	

	HookEvents();

	if (!bIcolibEmbededInCore)
		SetDefaultProtocolIcons();

	LoadPlugins();

	InitVSApi();

	WordToModAndVk(DBGetContactSettingWord(NULL,YAMN_DBMODULE,YAMN_HKCHECKMAIL,YAMN_DEFAULTHK),&mod,&vk);
		
//Create thread for hotkey
	WORD HotKey = MAKEWORD((BYTE)vk,(BYTE)mod);
	CloseHandle(CreateThread(NULL,0,YAMNHotKeyThread,(LPVOID)HotKey,0,&HotKeyThreadID));
//Create thread that will be executed every second
	if(!(SecTimer=SetTimer(NULL,0,1000,(TIMERPROC)TimerProc)))
		return 1;


#ifdef YAMN_VER_BETA
	#ifdef YAMN_VER_BETA_CRASHONLY
	MessageBox(NULL,"This YAMN beta version is intended for testing. After crash, you should send report to author. Please read included readme when available. Thank you.","YAMN beta",MB_OK);
	#else
	MessageBox(NULL,"This YAMN beta version is intended for testing. You should inform author if it works or when it does not work. Please read included readme when available. Thank you.","YAMN beta",MB_OK);
	#endif
#endif
#ifdef YAMN_DEBUG
	MessageBox(NULL,"This YAMN creates extended debug logfiles. It is not intended for general use.","YAMN beta",MB_OK);
#endif

	return 0;
}

extern "C" int __declspec(dllexport) UninstallEx(PLUGINUNINSTALLPARAMS* ppup)
{
	const char* DocFiles[]={"YAMN-License.txt","YAMN-Readme.txt","YAMN-Readme.developers.txt",NULL};

	typedef int (* UNINSTALLFILTERFCN)();
	UNINSTALLFILTERFCN UninstallFilter;

	PUIRemoveSkinSound(YAMN_NEWMAILSOUND);
	PUIRemoveSkinSound(YAMN_CONNECTFAILSOUND);

	if(UninstallPlugins)
	{
		for(int i=0;i<iDllPlugins;i++)
		{
			if(NULL!=(UninstallFilter=(UNINSTALLFILTERFCN)GetProcAddress(hDllPlugins[i],"UninstallFilter")))
				UninstallFilter();

			FreeLibrary(hDllPlugins[i]);
			hDllPlugins[i]=NULL;				//for safety
		}
		iDllPlugins = 0;
		if(hDllPlugins){
			free((void *)hDllPlugins);
			hDllPlugins = NULL;
		}

//		NotifyEventHooks(ME_YAMN_UNINSTALLPLUGINS,0,0);
	}
	UninstallPOP3(ppup);

	MessageBoxA(NULL,"You have to delete manually YAMN plugins located in \"Plugins/YAMN\" folder.","YAMN uninstalling",MB_OK|MB_ICONINFORMATION);
	PUIRemoveFilesInDirectory(ppup->pszDocsPath,DocFiles);
	if(ppup->bDoDeleteSettings)
		PUIRemoveDbModule("YAMN");
	return 0;
}

int Shutdown(WPARAM,LPARAM)
{
	DBWriteContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGPOSX,HeadPosX);
	DBWriteContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGPOSY,HeadPosY);
	DBWriteContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGSIZEX,HeadSizeX);
	DBWriteContactSettingDword(NULL,YAMN_DBMODULE,YAMN_DBMSGSIZEY,HeadSizeY);
	DBWriteContactSettingWord(NULL,YAMN_DBMODULE,YAMN_DBMSGPOSSPLIT,HeadSplitPos);
	YAMNVar.Shutdown=TRUE;
//	CallService(MS_TTB_REMOVEBUTTON,(WPARAM)hTTButton,0);		//this often leads to deadlock in Miranda (bug in Miranda)
	KillTimer(NULL,SecTimer);

	UnregisterProtoPlugins();
	UnregisterFilterPlugins();
	return 0;
}

//We undo all things from Load()
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

	DeleteCriticalSection(AccountStatusCS);
	delete AccountStatusCS;
	DeleteCriticalSection(FileWritingCS);
	delete FileWritingCS;
	DeleteCriticalSection(PluginRegCS);

	delete PluginRegCS;
	UnhookEvents();
	DestroyServiceFunctions();

	UnloadPlugins();

	delete [] CodePageNamesSupp;
	delete [] szMirandaDir;
	delete [] UserDirectory;
	delete [] szProfileDir;
	delete [] ProfileName;
	return 0;
}

void LoadPlugins()
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	char szSearchPath[MAX_PATH];
	char szPluginPath[MAX_PATH];
	lstrcpy(szSearchPath,szMirandaDir);
	lstrcat(szSearchPath,"\\Plugins\\YAMN\\*.dll");
	typedef INT_PTR (*LOADFILTERFCN)(MIRANDASERVICE GetYAMNFcn);

	hDllPlugins=NULL;

	if(INVALID_HANDLE_VALUE!=(hFind=FindFirstFile(szSearchPath,&fd)))
	{
		do
		{	//rewritten from Miranda sources... Needed because Win32 API has a bug in FindFirstFile, search is done for *.dlllllll... too
			char *dot=strrchr(fd.cFileName,'.');
			if(dot)
			{ // we have a dot
				int len=(int)strlen(fd.cFileName); // find the length of the string
				char* end=fd.cFileName+len; // get a pointer to the NULL
				int safe=(end-dot)-1;	// figure out how many chars after the dot are "safe", not including NULL
		
				if((safe!=3) || (lstrcmpi(dot+1,"dll")!=0)) //not bound, however the "dll" string should mean only 3 chars are compared
					continue;
			}
			else
				continue;
			
			HINSTANCE hDll;
			LOADFILTERFCN LoadFilter;

			lstrcpy(szPluginPath,szMirandaDir);
			lstrcat(szPluginPath,"\\Plugins\\YAMN\\");
			lstrcat(szPluginPath,fd.cFileName);
			if((hDll=LoadLibrary(szPluginPath))==NULL) continue;
			LoadFilter=(LOADFILTERFCN)GetProcAddress(hDll,"LoadFilter");
			if(NULL==LoadFilter)
			{
				FreeLibrary(hDll);
				hDll=NULL;
				continue;
			}
			if(!(*LoadFilter)(GetFcnPtrSvc))
			{
				FreeLibrary(hDll);
				hDll=NULL;
			}

			if(hDll!=NULL)
			{
				hDllPlugins=(HINSTANCE *)realloc((void *)hDllPlugins,(iDllPlugins+1)*sizeof(HINSTANCE));
				hDllPlugins[iDllPlugins++]=hDll;
			}
		} while(FindNextFile(hFind,&fd));
	FindClose(hFind);
	}
}

void UnloadPlugins()
{
	for(int i=iDllPlugins-1;i>=0;i--) {
		if(FreeLibrary(hDllPlugins[i])){
			hDllPlugins[i]=NULL;				//for safety
			iDllPlugins --;
		}
	}
	if(hDllPlugins){
		free((void *)hDllPlugins);
		hDllPlugins = NULL;
	}
}

void GetIconSize(HICON hIcon, int* sizeX, int* sizeY)
{
    ICONINFO ii;
    BITMAP bm;
    GetIconInfo(hIcon, &ii);
    GetObject(ii.hbmColor, sizeof(bm), &bm);
    if (sizeX != NULL) *sizeX = bm.bmWidth;
    if (sizeY != NULL) *sizeY = bm.bmHeight;
    DeleteObject(ii.hbmMask);
    DeleteObject(ii.hbmColor);
}

HBITMAP LoadBmpFromIcon(HICON hIcon)
{
	HBITMAP hBmp, hoBmp;
    HDC hdc, hdcMem;
    HBRUSH hBkgBrush;

	int IconSizeX = 16;
    int IconSizeY = 16;

	//GetIconSize(hIcon, &IconSizeX, &IconSizeY);

	//DebugLog(SynchroFile,"Icon size %i %i\n",IconSizeX,IconSizeY);

    if ((IconSizeX == 0) || (IconSizeY == 0)) 
	{    
        IconSizeX = 16;
        IconSizeY = 16;
    }

    RECT rc;
    BITMAPINFOHEADER bih = {0};
    int widthBytes;

    hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    bih.biSize = sizeof(bih);
    bih.biBitCount = 24;
    bih.biPlanes = 1;
    bih.biCompression = BI_RGB;
    bih.biHeight = IconSizeY;
    bih.biWidth = IconSizeX; 
    widthBytes = ((bih.biWidth*bih.biBitCount + 31) >> 5) * 4;
    rc.top = rc.left = 0;
    rc.right = bih.biWidth;
    rc.bottom = bih.biHeight;
    hdc = GetDC(NULL);
    hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
    hdcMem = CreateCompatibleDC(hdc);
    hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
    FillRect(hdcMem, &rc, hBkgBrush);
    DrawIconEx(hdcMem, 0, 0, hIcon, bih.biWidth, bih.biHeight, 0, NULL, DI_NORMAL);
    SelectObject(hdcMem, hoBmp);

	return hBmp;
}

int AddTopToolbarIcon(WPARAM,LPARAM)
{
	TTBButton Button=
	{
		sizeof(TTBButton),
		NULL,
		NULL,
		NULL,
		MS_YAMN_FORCECHECK,
		TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP, // | TTBBF_DRAWBORDER,
		0,0,0,0,
		NULL
	};

	if(!DBGetContactSettingByte(NULL,YAMN_DBMODULE,YAMN_TTBFCHECK,1))
		return 1;
	
	Button.name=Translate("Check mail");
	
	Button.hbBitmapUp = LoadBmpFromIcon(hYamnIcons[5]);
	Button.hbBitmapDown = LoadBmpFromIcon(hYamnIcons[6]); //LoadBitmap(YAMNVar.hInst,MAKEINTRESOURCE(IDB_BMTTB));
	
	if((HANDLE)-1==(hTTButton=(HANDLE)CallService(MS_TTB_ADDBUTTON,(WPARAM)&Button,(LPARAM)0)))
		return 1;
	CallService(MS_TTB_SETBUTTONOPTIONS,MAKEWPARAM((WORD)TTBO_TIPNAME,(WORD)hTTButton),(LPARAM)Translate("Check mail"));
	return 0;
}

int UninstallQuestionSvc(WPARAM wParam,LPARAM)
{
//	if(strcmp((char *)wParam,Translate("Yet Another Mail Notifier")))
//		return 0;
	switch(MessageBoxA(NULL,Translate("Do you also want to remove native YAMN plugins settings?"),Translate("YAMN uninstalling"),MB_YESNOCANCEL|MB_ICONQUESTION))
	{
		case IDYES:
			UninstallPlugins=TRUE;
			break;
		case IDNO:
			UninstallPlugins=FALSE;
			break;
		case IDCANCEL:
			return 1;
	}
	return 0;
}
