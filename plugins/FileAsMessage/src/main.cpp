#include "main.h"

PLUGININFOEX pluginInfo =
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
	// {34B5A402-1B79-4246-B041-43D0B590AE2C}
	{0x34b5a402, 0x1b79, 0x4246, {0xb0, 0x41, 0x43, 0xd0, 0xb5, 0x90, 0xae, 0x2c}}
};

HANDLE hFileList;
HINSTANCE hInst;
int hLangpack;

char *szServiceTitle = SERVICE_TITLE;
char *szServicePrefix = SERVICE_PREFIX;
HANDLE hHookDbSettingChange, hHookContactAdded, hHookSkinIconsChanged;

HICON hIcons[5];

static IconItem iconList[] = 
{
	{ LPGEN("Play"), "FePlay", IDI_PLAY },
	{ LPGEN("Pause"), "FePause", IDI_PAUSE },
	{ LPGEN("Revive"), "FeRefresh", IDI_REFRESH },
	{ LPGEN("Stop"), "FeStop", IDI_STOP },
	{ LPGEN("Main"), "FeMain", IDI_SMALLICON },
};

int iIconId[5] = {3,2,4,1,0};

//
//  wParam - Section name
//  lParam - Icon ID
//
int OnSkinIconsChanged(WPARAM wParam,LPARAM lParam)
{
	for(int indx = 0; indx < SIZEOF(hIcons); indx++)
		hIcons[indx] = Skin_GetIconByHandle( iconList[indx].hIcolib );

	WindowList_Broadcast(hFileList, WM_FE_SKINCHANGE, 0,0);

	return 0;
}

int OnSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;

	HWND hwnd = WindowList_Find(hFileList, hContact);
	PostMessage(hwnd, WM_FE_STATUSCHANGE, 0,0);
	return 0;
}

INT_PTR OnRecvFile(WPARAM wParam, LPARAM lParam)
{
	CLISTEVENT *clev = (CLISTEVENT*)lParam;

	HWND hwnd = WindowList_Find(hFileList, clev->hContact);
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

INT_PTR OnSendFile(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = WindowList_Find(hFileList, wParam);
	if(IsWindow(hwnd))
	{
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else
	{
		if(hwnd != 0) WindowList_Remove(hFileList, hwnd);
		FILEECHO *fe = new FILEECHO(wParam);
		fe->inSend = TRUE;
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, (LPARAM)fe);
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

INT_PTR OnRecvMessage(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = ( PROTORECVEVENT * )pccsd->lParam;

	if(strncmp(ppre->szMessage, szServicePrefix, strlen(szServicePrefix)))
		return CallService(MS_PROTO_CHAINRECV, wParam, lParam);

	HWND hwnd = WindowList_Find(hFileList, pccsd->hContact);
	if(!IsWindow(hwnd))
	{
		if(hwnd != 0) WindowList_Remove(hFileList, hwnd);
		FILEECHO *fe = new FILEECHO(pccsd->hContact);
		fe->inSend = FALSE;
		hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, (LPARAM)fe);
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
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszTitle = _T(SERVICE_TITLE);
	odp.ptszGroup = LPGENT("Events");
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.pfnDlgProc = OptionsDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

//
// MirandaPluginInfo()
// Called by Miranda to get Version
//
extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD dwVersion)
{
	return &pluginInfo;
}

//
// Startup initializing
//

static int OnModulesLoaded(WPARAM wparam,LPARAM lparam)
{
	for(int indx = 0; indx < SIZEOF(hIcons); indx++)
		hIcons[indx] = Skin_GetIconByHandle( iconList[indx].hIcolib );

	hHookSkinIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, OnSkinIconsChanged);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 200011;
	mi.hIcon = hIcons[ICON_MAIN];
	mi.pszName = LPGEN("File As Message...");
	mi.pszService = SERVICE_NAME "/FESendFile";
	mi.pszContactOwner = NULL;
	mi.flags = CMIF_NOTOFFLINE;
	Menu_AddContactMenuItem(&mi);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	InitCRC32();

	Icon_Register(hInst, "fileAsMessage", iconList, SIZEOF(iconList));

	hFileList = WindowList_Create();

	CreateServiceFunction(SERVICE_NAME PSR_MESSAGE, OnRecvMessage);
	CreateServiceFunction(SERVICE_NAME "/FESendFile", OnSendFile);
	CreateServiceFunction(SERVICE_NAME "/FERecvFile", OnRecvFile);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = SERVICE_NAME;
	pd.type = PROTOTYPE_FILTER;
	CallService(MS_PROTO_REGISTERMODULE, 0, ( LPARAM ) &pd);

	HookEvent(ME_OPT_INITIALISE, OnOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookDbSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);
	hHookSkinIconsChanged = NULL;

	return 0;
}

//
// Unload()
// Called by Miranda when Plugin is unloaded.
//
extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hFileList);
	if(hHookSkinIconsChanged != NULL)
		UnhookEvent(hHookSkinIconsChanged);
	UnhookEvent(hHookDbSettingChange);
	UnhookEvent(hHookContactAdded);

	return 0;
}

//
// DllMain()
//
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
{
	hInst = hInstance;
	return TRUE;
}
