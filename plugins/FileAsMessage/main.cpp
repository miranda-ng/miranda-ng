#include "main.h"
#include "dialog.h"
#include "resource.h"

#include "IcoLib.h"

#include <stdio.h>
#include <time.h>

PLUGINLINK *pluginLink;
PLUGININFO pluginInfo =
{
	sizeof( PLUGININFO ),
	SERVICE_TITLE,
	PLUGIN_MAKE_VERSION( 0,0,2,4 ),
	"file tranfer by using the messaging services - as plain text",
	"Denis Stanishevskiy // StDenis",
	"stdenformiranda(at)fromru(dot)com",
	"Copyright (c) 2004, Denis Stanishevskiy",
	PLUGIN_URL,
	0, 0
};

HANDLE hFileList;
HINSTANCE hInst;

char *szServiceTitle = SERVICE_TITLE;
char *szServicePrefix = SERVICE_PREFIX;
HANDLE hHookDbSettingChange, hHookContactAdded, hHookSkinIconsChanged;

extern LRESULT CALLBACK OptionsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

int idIcons[5] = {IDI_PLAY, IDI_PAUSE, IDI_REFRESH, IDI_STOP, IDI_SMALLICON};
HICON hIcons[5];

char *szIconId[5] =
{
	"FePlay",
	"FePause",
	"FeRefresh",
	"FeStop",
	"FeMain"
};
char *szIconName[5] =
{
	"Play",
	"Pause",
	"Revive",
	"Stop",
	"Main"
};
/*
char *szIconGroup[5] =
{
	"gr1",
	"gr3",
	"gr2",
	"gr3",
	"gr1"
};
*/
int iIconId[5] = {3,2,4,1,0};

//
//  wParam - Section name
//  lParam - Icon ID
//
int OnSkinIconsChanged(WPARAM wParam,LPARAM lParam)
{
	int indx;
/*
	if(lParam == NULL)
		return 0;
	for(indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
	{
		if(strcmp((char*)lParam, szIconId[indx]) == 0)
		{
			hIcons[indx] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[indx]);
			break;
		}
	}
*/
	for(indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
		hIcons[indx] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[indx]);

	WindowList_Broadcast(hFileList, WM_FE_SKINCHANGE, 0,0);

	return 0;
}

int OnSettingChanged(WPARAM wParam,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	HWND hwnd;

	hwnd = WindowList_Find(hFileList,(HANDLE)wParam);
	PostMessage(hwnd, WM_FE_STATUSCHANGE, 0,0);
	//OnSkinIconsChanged(0,0);
	//PostMessage(hwnd, WM_FE_SKINCHANGE, 0,0);

	return 0;
}

int OnContactAdded(WPARAM wParam,LPARAM lParam)
{
	CallService(MS_PROTO_ADDTOCONTACT, wParam, (LPARAM)SERVICE_NAME);
	return 0;
}

int OnRecvFile(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	CLISTEVENT *clev = (CLISTEVENT*)lParam;

	hwnd = WindowList_Find(hFileList,(HANDLE)clev->hContact);
	if(IsWindow(hwnd))
	{
		ShowWindow(hwnd, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	/*
	else
	{
		if(hwnd != 0) WindowList_Remove(hFileList, hwnd);
		FILEECHO *fe = new FILEECHO((HANDLE)clev->hContact);
		fe->inSend = FALSE;
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DialogProc, (LPARAM)fe);
		if(hwnd == NULL)
		{
			delete fe;
			return 0;
		}
		//SendMessage(hwnd, WM_FE_SERVICE, 0, TRUE);
		ShowWindow(hwnd, SW_SHOWNORMAL);
	}
	*/
	return 1;
}

int OnSendFile(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;

	hwnd = WindowList_Find(hFileList,(HANDLE)wParam);
	if(IsWindow(hwnd))
	{
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else
	{
		if(hwnd != 0) WindowList_Remove(hFileList, hwnd);
		FILEECHO *fe = new FILEECHO((HANDLE)wParam);
		fe->inSend = TRUE;
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DialogProc, (LPARAM)fe);
		if(hwnd == NULL)
		{
			delete fe;
			return 0;
		}
		//SendMessage(hwnd, WM_FE_SERVICE, 0, TRUE);
		ShowWindow(hwnd, SW_SHOWNORMAL);
	}
	return 1;
}

int OnRecvMessage( WPARAM wParam, LPARAM lParam )
{
	CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = ( PROTORECVEVENT * )pccsd->lParam;
	HWND hwnd;

	if(strncmp(ppre->szMessage, szServicePrefix, strlen(szServicePrefix)))
		return CallService( MS_PROTO_CHAINRECV, wParam, lParam );

	hwnd = (HWND)WindowList_Find(hFileList, (HANDLE)pccsd->hContact);
	if(!IsWindow(hwnd))
	{
		if(hwnd != 0) WindowList_Remove(hFileList, hwnd);
		FILEECHO *fe = new FILEECHO((HANDLE)pccsd->hContact);
		fe->inSend = FALSE;
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DialogProc, (LPARAM)fe);
		if(hwnd == NULL)
		{
			delete fe;
			return 0;
		}
	}
	char *msg = strdup(ppre->szMessage + strlen(szServicePrefix));
	PostMessage(hwnd, WM_FE_MESSAGE, (WPARAM)pccsd->hContact, (LPARAM)msg);

	return 0;
}

int OnOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));

	odp.cbSize = sizeof(odp);
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle = Translate(SERVICE_TITLE);
	odp.pszGroup = Translate("Plugins");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = (DLGPROC)OptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

//
// MirandaPluginInfo()
// Called by Miranda to get Version
//
PLUGININFO *MirandaPluginInfo( DWORD dwVersion )
{
/*
	if(MessageBox(0,
		"Achtung! This plugin is technology demo only. It didn't tested carefully.\n"
		"Do you want to continue usage of this plugin?", SERVICE_TITLE,
		MB_YESNO) != IDYES) return 0;
//*/
	if( dwVersion < PLUGIN_MAKE_VERSION( 0,3,3,0 ))return 0;
	return &pluginInfo;
}
/*
DWORD CreateSetting(char *name, DWORD defvalue)
{
	if(DBGetContactSettingDword(NULL, SERVICE_NAME, name, -1) == -1)
		DBWriteContactSettingDword(NULL, SERVICE_NAME, name, defvalue);
	else
		defvalue = DBGetContactSettingDword(NULL, SERVICE_NAME, name, defvalue);
	return defvalue;
}
*/

int OnModulesLoaded(WPARAM wparam,LPARAM lparam)
{
	if(!ServiceExists(MS_SKIN2_ADDICON))
	{
		for(int indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
			hIcons[indx] = (HICON)LoadImage(hInst,MAKEINTRESOURCE(idIcons[indx]),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
	}
	else
	{
		int indx;
		SKINICONDESC sid;
		char ModuleName[MAX_PATH];

		ZeroMemory(&sid, sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.pszSection = Translate("fileAsMessage");
		GetModuleFileName(hInst, ModuleName, sizeof(ModuleName));
		for(indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
		{
			//sid.pszSection = szIconGroup[indx];
			sid.pszName = szIconId[indx];
			sid.pszDescription = szIconName[indx];
			sid.pszDefaultFile = ModuleName;
			sid.iDefaultIndex = iIconId[indx];
			CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
		}
		for(indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
			hIcons[indx] = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[indx]);

		hHookSkinIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, OnSkinIconsChanged);
	}
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact)
	{
		if(!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)SERVICE_NAME))
			CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)SERVICE_NAME);
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	CLISTMENUITEM mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(CLISTMENUITEM);
	mi.position = 200011;
	mi.hIcon = hIcons[ICON_MAIN];
	mi.pszName = Translate("File As Message...");
	mi.pszService = SERVICE_NAME "/FESendFile";
	mi.pszContactOwner = NULL;
	mi.flags = CMIF_NOTOFFLINE;
	CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, ( LPARAM )&mi);

	return 0;
}

//
// Startup initializing
//
int Load( PLUGINLINK *link )
{
	pluginLink = link;

	InitCRC32();

//	for(int indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
//		hIcons[indx] = (HICON)LoadImage(hInst,MAKEINTRESOURCE(idIcons[indx]),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
		
	hFileList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	//CreateServiceFunction( SERVICE_NAME PS_GETCAPS, FEGetCaps );
	CreateServiceFunction(SERVICE_NAME PSR_MESSAGE, OnRecvMessage);
	CreateServiceFunction(SERVICE_NAME "/FESendFile", OnSendFile);
	CreateServiceFunction(SERVICE_NAME "/FERecvFile", OnRecvFile);

	PROTOCOLDESCRIPTOR pd;
	memset(&pd, 0, sizeof( PROTOCOLDESCRIPTOR));
	pd.cbSize = sizeof(PROTOCOLDESCRIPTOR);
	pd.szName = SERVICE_NAME;
	pd.type = PROTOTYPE_FILTER;
	CallService(MS_PROTO_REGISTERMODULE, 0, ( LPARAM ) &pd);

	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookDbSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);
	hHookContactAdded = HookEvent(ME_DB_CONTACT_ADDED, OnContactAdded);
	hHookSkinIconsChanged = NULL;

	return 0;
}

//
// Unload()
// Called by Miranda when Plugin is unloaded.
//
int Unload( void )
{
//	if(hFileList)
//		WindowList_Broadcast(hFileList, WM_CLOSE, 0,0);
	if(!ServiceExists(MS_SKIN2_ADDICON))
		for(int indx = 0; indx < ARRAY_SIZE(hIcons); indx++)
			DestroyIcon(hIcons[indx]);

	if(hHookSkinIconsChanged != NULL)
		UnhookEvent(hHookSkinIconsChanged);
	UnhookEvent(hHookDbSettingChange);
	UnhookEvent(hHookContactAdded);

	return 0;
}

//
// DllMain()
//
int WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved )
{
	hInst = hInstance;

	if( dwReason == DLL_PROCESS_ATTACH )
		DisableThreadLibraryCalls(hInstance);

	return 1;
}
