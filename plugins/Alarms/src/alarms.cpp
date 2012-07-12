/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "common.h"
#include "alarms.h"


#define SERVICENAME _T("mp")
#define COMMANDPREFIX _T("/") SERVICENAME

#define WMP_PAUSE	32808
#define WMP_NEXT	0x497B

TCHAR szGamePrefix[] = COMMANDPREFIX;

HINSTANCE hInst;
int hLangpack;

HANDLE hTopToolbarButton;

typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

//bool right_window = false;
WNDPROC old_clist_wndproc;

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
	{ 0x4dd7762b, 0xd612, 0x4f84, { 0xaa, 0x86, 0x6, 0x8f, 0x17, 0x85, 0x9b, 0x6d } } // {4DD7762B-D612-4f84-AA86-068F17859B6D}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_ALARMS, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		case UM_FREEPLUGINDATA: {
			//MY_PLUGIN_DATA * mpd = NULL;
			//mpd = (MY_PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
			//if (mdp > 0) free(mpd);
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowPopup(HANDLE hContact, const TCHAR *msg) {
	if (ServiceExists(MS_POPUP_ADDPOPUP)) {
		POPUPDATAT ppd;
		TCHAR *lpzContactName;

		ZeroMemory(&ppd, sizeof(ppd)); //This is always a good thing to do.
		ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
		ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);

		lpzContactName = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
	
		lstrcpy(ppd.lptzContactName, lpzContactName);
		lstrcpy(ppd.lptzText, msg);
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);;
		ppd.colorText = RGB(0,0,0);
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
		ppd.PluginData = 0;
		ppd.iSeconds = 3;

		//Now that every field has been filled, we want to see the popup.
		CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
	}
}

static int PluginSendMessage(WPARAM wParam,LPARAM lParam) {
	CCSDATA css;
	css.hContact = (HANDLE)wParam;
	css.szProtoService = PSS_MESSAGE;
	css.wParam = 0;
	css.lParam = lParam;

	CallService(MS_PROTO_CALLCONTACTSERVICE, 0, (LPARAM)&css);
	return 0;
}

static int PluginMessageReceived(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = ( PROTORECVEVENT * )pccsd->lParam;
	TCHAR *savedMsg;
	HWND hWnd;
	TCHAR response[256];
	
	TCHAR msg[1024], buff[1024];

	if (_tcsncmp(ppre->tszMessage, szGamePrefix, _tcslen(szGamePrefix)))
		return CallService( MS_PROTO_CHAINRECV, wParam, lParam );

	_tcscpy(msg, ppre->tszMessage + _tcslen(szGamePrefix));

	savedMsg = ppre->tszMessage;
	
	if (!_tcscmp(msg, _T(" ffw"))) {
		mir_sntprintf(buff, SIZEOF(buff), _T("%s"), _T("Fast forward!"));
		
		hWnd = FindWindow(0, _T("Windows Media Player"));
		PostMessage(hWnd, WM_COMMAND, WMP_NEXT, 0);
	} else {
		mir_sntprintf(buff, SIZEOF(buff), _T("Unknown command issued: \"%s\""), msg);
	}
	
	/*
	ppre->szMessage = (char *)Translate(buff);
	retval = CallService( MS_PROTO_CHAINRECV, wParam, lParam );
	ppre->szMessage = savedMsg;
	*/

	ShowPopup(pccsd->hContact, buff);

	_tcscpy(response, buff);
	PluginSendMessage((WPARAM)pccsd->hContact, (LPARAM)response);

	return 0;
}

HBITMAP LoadBmpFromIcon(int IdRes)
{
	HBITMAP hBmp, hoBmp;
    HDC hdc, hdcMem;
    HBRUSH hBkgBrush;
	HICON hIcon;

	hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IdRes));

    RECT rc;
    BITMAPINFOHEADER bih = {0};
    int widthBytes;

    hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
    bih.biSize = sizeof(bih);
    bih.biBitCount = 24;
    bih.biPlanes = 1;
    bih.biCompression = BI_RGB;
    bih.biHeight = 16;
    bih.biWidth = 20; 
    widthBytes = ((bih.biWidth*bih.biBitCount + 31) >> 5) * 4;
    rc.top = rc.left = 0;
    rc.right = bih.biWidth;
    rc.bottom = bih.biHeight;
    hdc = GetDC(NULL);
    hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
    hdcMem = CreateCompatibleDC(hdc);
    hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
    FillRect(hdcMem, &rc, hBkgBrush);
    DrawIconEx(hdcMem, 2, 0, hIcon, 16, 16, 0, NULL, DI_NORMAL);


    SelectObject(hdcMem, hoBmp);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);

	DeleteObject(hIcon);

	return hBmp;
}

static int InitTopToolbarButton(WPARAM wParam, LPARAM lParam)
{
	TTBButton ttb = {0};
	ttb.cbSize = sizeof(ttb);
	ttb.hIconUp = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TBUP));
	ttb.hIconDn = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TBDN));
	ttb.pszService = MODULE "/NewAlarm";
	ttb.dwFlags = TTBBF_VISIBLE;
	ttb.name = ttb.pszTooltipUp = LPGEN("Set Alarm");
	
	hTopToolbarButton = TopToolbar_AddButton(&ttb);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButton, (LPARAM)TTBST_RELEASED);
	return 0;
}

static int MainInit(WPARAM wparam,LPARAM lparam)
{
	if (ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/alarms.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_alarms.html";
		update.pbBetaVersionPrefix = (BYTE *)" version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	/*
	PROTOCOLDESCRIPTOR pd;
	HANDLE hContact;

	memset( &pd, 0, sizeof( PROTOCOLDESCRIPTOR ));
	pd.cbSize = sizeof( PROTOCOLDESCRIPTOR );
	pd.szName = SERVICENAME;
	//pd.type = PROTOTYPE_ENCRYPTION - 9;
	pd.type = PROTOTYPE_FILTER;
	CallService( MS_PROTO_REGISTERMODULE, 0, ( LPARAM ) &pd );

	CreateServiceFunction( SERVICENAME PSR_MESSAGE, PluginMessageReceived );

	hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

	while( hContact )
	{
		if ( !CallService( MS_PROTO_ISPROTOONCONTACT, ( WPARAM )hContact, ( LPARAM )SERVICENAME ))
			CallService( MS_PROTO_ADDTOCONTACT, ( WPARAM )hContact, ( LPARAM )SERVICENAME );

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT, ( WPARAM )hContact, 0 );
	}
	*/

	// initialize icons
	InitIcons();

	// Hotkey
	/*
	SKINHOTKEYDESCEX hk;
	hk.cbSize = sizeof(hk);
	hk.pszName = Translate("Set Alarm");
	hk.pszDescription = Translate("Set a new alarm");
	hk.pszSection = Translate("Alarms");
	hk.pszService = MODULE "/NewAlarm";
	hk.DefHotKey = (WORD)'A' + HOTKEYF_ALT * 256;
	CallService(MS_SKIN_ADDHOTKEY, 0, (LPARAM)&hk);
	*/

	// TopToolbar support
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbarButton);

	InitFrames();

	// TriggerPlugin support
	LoadTriggerSupport();

	return 0;
}

static int MainDeInit(WPARAM wParam, LPARAM lParam)
{
	DeinitFrames();
	DeinitList();
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	// ensure datetime picker is loaded
	INITCOMMONCONTROLSEX ccx;
	ccx.dwSize = sizeof(ccx);
	ccx.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ccx);

	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainDeInit);

	LoadOptions();
	InitList();

	HookEvent(ME_OPT_INITIALISE, OptInit );

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

