#define _WIN32_WINNT  0x0500
#include "plugin.h"
#include "playSnd.h"
#include "optionsdlg.h"
#include "resource.h"
#include "m_cluiframes.h"

#include "IcoLib.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HWND hwndFrame = NULL, hwndMute = NULL, hwndSlider = NULL, hwndOptSlider = NULL, hwndMRadio = 0;
HANDLE hHookSkinPlayingSound, hHookDbSettingChange, hHookSkinIconsChanged;
HHOOK hHookMouse;
bool showMute = TRUE;
extern void LoadBackgroundSettings();

HBRUSH g_HBRback = 0;
COLORREF g_clrback = 0;

#if defined(UNICODE)
	char szTitle[] = "sndVol (UNICODE)";
#else
	char szTitle[] = "sndVol";
#endif

bool isEnabled = FALSE;
HICON hSoundOn = NULL, hSoundOff = NULL;
HICON hIconMRadio = 0, hIconMRadio_off = 0;

int isMuteVisible = FALSE;
int autoPreview = FALSE;

char *szIconId[4] = {"sndVol_Off", "sndVol_On", "mradio_off", "mradio_on"};
char *szIconName[4] = {"Off", "On", "mRadio off", "mRadio on"};
int iIconId[4] = {-IDI_SND_OFF, -IDI_SND_ON, -IDI_MRAADIO_OFF, -IDI_MRADIO};
int g_mRadioMuted = 0;

/*
PLUGININFO pluginInfo={
	sizeof( PLUGININFO ),
	szTitle,
	PLUGIN_MAKE_VERSION( 0,0,2,2 ),
#if defined(UNICODE)
	"Volume control for Mirandas event sounds and mRadio (UNICODE)",
#else
	"Volume control for Mirandas event sounds and mRadio",
#endif
	"Denis Stanishevskiy / StDenis",
	"stdenformiranda(at)fromru(dot)com",
	"Copyright (c) 2005-2006, Denis Stanishevskiy, modifications and mRadio support by Nightwish - silvercircle(at)gmail(dot)com",
	"",
	0, 0
};

*/

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	szTitle,
	PLUGIN_MAKE_VERSION(0, 0, 3, 0),
#if defined(UNICODE)
	"Volume control for Mirandas event sounds and mRadio (UNICODE)",
#else
	"Volume control for Mirandas event sounds and mRadio",
#endif
	"Denis Stanishevskiy / StDenis",
	"stdenformiranda(at)fromru(dot)com",
	"Copyright (c) 2005-2006, Denis Stanishevskiy, modifications and mRadio support by Nightwish - silvercircle(at)gmail(dot)com",
	"",
	UNICODE_AWARE,
	0,
#ifdef _UNICODE
	{ 0xb0733420, 0x7bf2, 0x4c08, { 0xb2, 0xbf, 0xf2, 0x63, 0xdf, 0x72, 0x42, 0xc9 }}
#else
	{ 0x35235260, 0x3d3d, 0x4b81, { 0xbe, 0x2e, 0x7f, 0x95, 0xdb, 0xad, 0x1a, 0x23 }}
#endif
};

void SetRadioVolume(int value)
{
	if(ServiceExists(MS_RADIO_SETVOL) && DBGetContactSettingByte(NULL,SERVICENAME,"mRadioAdjust", FALSE)) {
		int useSound = DBGetContactSettingByte(NULL, "Skin", "UseSound", 1);

		if(g_mRadioMuted)
			return;

		if(useSound || !DBGetContactSettingByte(NULL, SERVICENAME, "mRadioMute", 0) || !g_mRadioMuted)
			CallService(MS_RADIO_SETVOL, (WPARAM)value, 0);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	if(fdwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(GetModuleHandle(_T("sndvol")));
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 0, 0))
		return NULL;
	return &pluginInfo;
}

#define MIID_SNDVOL {0x50ea8870, 0x1c68, 0x4962, { 0xbc, 0xc1, 0x57, 0x73, 0x45, 0x4e, 0x3d, 0x26 }}

extern "C" static const MUUID interfaces[] = {MIID_TESTPLUGIN, MIID_SNDVOL, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

HANDLE hMenuItem;

int OnSkinPlayingSound(WPARAM wParam,LPARAM lParam)
{
	char * pszFile = (char *) lParam;
	if(pszFile && (DBGetContactSettingByte(NULL,"Skin","UseSound",1) || (int)wParam==1))
		playSnd::PlaySound(pszFile, NULL, SND_ASYNC | SND_FILENAME | SND_NOWAIT);
	return 0;
}

int OnSkinIconsChanged(WPARAM wParam,LPARAM lParam)
{
	hSoundOff = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[0]);
	hSoundOn = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[1]);	
	hIconMRadio_off = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[2]);
	hIconMRadio = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[3]);

	int useSound = DBGetContactSettingByte(NULL,"Skin","UseSound",1);
	DBWriteContactSettingByte(NULL,"Skin","UseSound",useSound);
	SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(useSound?hSoundOn:hSoundOff));
	SendMessage(hwndMRadio, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(g_mRadioMuted ? hIconMRadio_off : hIconMRadio));

	return 0;
}

int OnSettingChanged(WPARAM wParam,LPARAM lParam)
{
	if(wParam != 0 || lParam == NULL) 
		return 0;
	{
		DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
		if(!strcmp(dbcws->szModule,"Skin")) {
			if(!strcmp(dbcws->szSetting,"UseSound")) {
				int useSound = dbcws->value.bVal;
				SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(useSound?hSoundOn:hSoundOff));
				if(ServiceExists(MS_RADIO_SETVOL) && DBGetContactSettingByte(NULL,SERVICENAME,"mRadioMute", FALSE)) {
					if(useSound) {
						int value = (int)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
						CallService(MS_RADIO_SETVOL, (WPARAM)value, 0);
					}
					else
						CallService(MS_RADIO_SETVOL, 0, 0);
				}
				return 0;
			}
		}
		else if(!strcmp(dbcws->szModule, "mRadio") && !strcmp(dbcws->szSetting, "Volume")) {
			g_mRadioMuted = (DBGetContactSettingByte(NULL, "mRadio", "Volume", 50) == 0) ? TRUE : FALSE;
			SendMessage(hwndMRadio, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(g_mRadioMuted ? hIconMRadio_off : hIconMRadio));
			return 0;
		}
	}
	return 0;
}

static int OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	
	ZeroMemory(&odp, sizeof(odp));

	odp.cbSize = sizeof(odp);
	odp.ptszGroup = TranslateT("Events");
	odp.position = -200000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.ptszTitle = TranslateT("Sounds Volume");
	odp.pfnDlgProc = (DLGPROC)OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

BOOL __inline IsMouseInside(HWND hwnd)
{
	POINT pt;
	HWND hwndAtPt;

	GetCursorPos(&pt);
	hwndAtPt = WindowFromPoint(pt);
	while(hwndAtPt != NULL)	{
		if(hwndAtPt == hwnd)
			return TRUE;
		hwndAtPt = GetParent(hwndAtPt);
	}
	return FALSE;
}

LRESULT CALLBACK OnMouseHook(int code, WPARAM wParam, LPARAM lParam)
{
	if(code >= 0) {
		if(wParam == WM_MOUSEWHEEL)	{
			MOUSEHOOKSTRUCTEX *mhk = (MOUSEHOOKSTRUCTEX*)lParam;

			if(IsMouseInside(hwndSlider)) {
				int count = (short)HIWORD(mhk->mouseData);
				PostMessage(hwndSlider, WM_MOUSEWHEEL, MAKEWPARAM(0,-count), MAKELPARAM(mhk->pt.x, mhk->pt.y));
				return 1;
			}
		}
	}
	return CallNextHookEx(hHookMouse, code, wParam, lParam);
}

static WNDPROC OldSliderWndProc = 0;

static LRESULT CALLBACK SliderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)	{
		case WM_ERASEBKGND:
			return TRUE;
	}
	return(CallWindowProc(OldSliderWndProc, hwnd, msg, wParam, lParam));
}
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)	{
		case WM_CREATE:
		{
#define MUTE_W	20
#define MUTE_H	20
			isMuteVisible = DBGetContactSettingByte(NULL,SERVICENAME,"MuteBtn", TRUE);
			hwndMute = CreateWindow(MIRANDABUTTONCLASS,	_T(""),	WS_CHILD | (isMuteVisible?WS_VISIBLE:0), 1,	1,
									MUTE_W,	MUTE_H,	hwnd, (HMENU)0,	(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(hwndMute, BUTTONSETASFLATBTN,0,0);				
			SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hSoundOn);

			hwndSlider = CreateWindow(TRACKBAR_CLASS, _T(""), WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_TOOLTIPS, MUTE_W+1, 1,	120,
									  MUTE_W, hwnd, (HMENU)0, (HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
			SendMessage(hwndSlider, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN, SLIDER_MAX));
			SendMessage(hwndSlider, TBM_SETPOS, TRUE, DBGetContactSettingDword(NULL,SERVICENAME,"Volume",80));
			OldSliderWndProc = (WNDPROC)SetWindowLong(hwndSlider, GWL_WNDPROC, (LONG)SliderWndProc);
			SendMessage(hwndMute, BUTTONADDTOOLTIP, (WPARAM)Translate("Click to toggle all sounds"), 0);

			if(ServiceExists(MS_RADIO_SETVOL)) {
				hwndMRadio = CreateWindow(MIRANDABUTTONCLASS,	_T(""),	WS_CHILD | (isMuteVisible?WS_VISIBLE:0), 1,	1,
							   			  MUTE_W,	MUTE_H,	hwnd, (HMENU)0,	(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), NULL);
				SendMessage(hwndMRadio, BUTTONSETASFLATBTN,0,0);
				SendMessage(hwndMRadio, BUTTONADDTOOLTIP, (WPARAM)Translate("Click to mute mRadio only, right click for mRadio settings"), 0);
				SendMessage(hwndMRadio, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(g_mRadioMuted ? hIconMRadio_off : hIconMRadio));
			}
			break;
		}
		case WM_COMMAND:
			if((HWND)lParam == hwndMute) {
				int useSound = !DBGetContactSettingByte(NULL,"Skin","UseSound",1);
				DBWriteContactSettingByte(NULL,"Skin","UseSound",useSound);
				SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(useSound?hSoundOn:hSoundOff));
			}
			else if((HWND)lParam == hwndMRadio) {
				int value;

				g_mRadioMuted = !g_mRadioMuted;
				value = g_mRadioMuted ? 0 : (int)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
				CallService(MS_RADIO_SETVOL, (WPARAM)value, 0);
				DBWriteContactSettingByte(NULL, "mRadio", "Volume", (BYTE)value);
				SendMessage(hwndMRadio, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(g_mRadioMuted ? hIconMRadio_off : hIconMRadio));
			}
			break;
		case WM_CONTEXTMENU:
			if(IsMouseInside(hwndMRadio))
				CallService("mRadio/Settings", 0, 0);
			break;
		case WM_USER:
			ShowWindow(hwndMute, wParam?SW_SHOW:SW_HIDE);
			isMuteVisible = wParam;
		case WM_HSCROLL:
		{
			int value = (DWORD)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
			if(hwndOptSlider)
				SendMessage(hwndOptSlider, TBM_SETPOS, TRUE, value);
			else
				DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
			playSnd::SetVolume(value);
			if(DBGetContactSettingByte(NULL,SERVICENAME,"AutoPreview", FALSE))
				SkinPlaySound("AlertMsg");

			SetRadioVolume(value);
			break;
		}			
		case WM_SIZE:
		{
			RECT rect;

			GetClientRect(hwnd,&rect);
			#define SLIDER_H	18
			if(hwndMute)
				MoveWindow(hwndMute, 1, 1, MUTE_W, MUTE_H, FALSE);
			SetWindowPos(hwndSlider, 0,
				rect.left+(isMuteVisible?MUTE_W:0)+1, rect.top+1+ (MUTE_H-SLIDER_H)/2, rect.right-rect.left-3-(isMuteVisible?MUTE_W:0)-(ServiceExists(MS_RADIO_SETVOL) ? MUTE_W:0), SLIDER_H,
				SWP_NOZORDER);
			if(hwndMRadio)
				MoveWindow(hwndMRadio, rect.right - MUTE_W, 1, MUTE_W, MUTE_H, FALSE);
			InvalidateRect(hwnd, &rect, FALSE);
			return 0;
		}
		case WM_CTLCOLORSTATIC:
			{
				if((HANDLE)lParam == hwndSlider) {
					HDC dc = (HDC)wParam;

					SetBkColor(dc, g_clrback);
					return((BOOL)g_HBRback);
				}
				break;
			}
		case WM_ERASEBKGND:
			{
				HDC dc = (HDC)wParam;
				RECT rc;

				GetClientRect(hwnd, &rc);
				FillRect(dc, &rc, g_HBRback);
				return TRUE;
			}
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_RELEASEDCAPTURE:
				{
					int value = (DWORD)SendMessage(hwndSlider, TBM_GETPOS, 0, 0); //*SLIDER_DIV;

					DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
					if(hwndOptSlider) SendMessage(hwndOptSlider, TBM_SETPOS, TRUE, value);
					playSnd::SetVolume(value);
					SetRadioVolume(value);
					break;
				}
			}
			break;
		case WM_DESTROY:
			if(hwndSlider && IsWindow(hwndSlider) && OldSliderWndProc != 0)
				SetWindowLong(hwndSlider, GWL_WNDPROC, (LONG)OldSliderWndProc);
			break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam); 
}
#define WndClass _T("SNDVOL_WND")

static HANDLE g_ShutDownEvent;
static HANDLE g_hFrame = 0;

static int ShutDownPlugin(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(g_ShutDownEvent);

	if(!ServiceExists(MS_SKIN2_ADDICON))
	{
		DestroyIcon(hSoundOff);
		DestroyIcon(hSoundOn);
		UnhookEvent(hHookSkinIconsChanged);
	}
	UnhookEvent(hHookSkinPlayingSound);
	playSnd::Destroy();
	if(g_HBRback)
		DeleteObject(g_HBRback);

	CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)g_hFrame, 0);
	return 0;
}

static int OnModulesLoaded(WPARAM wparam,LPARAM lparam)
{
	int useSound;

	g_mRadioMuted = (DBGetContactSettingByte(NULL, "mRadio", "Volume", 50) == 0) ? TRUE : FALSE;

	g_ShutDownEvent = HookEvent(ME_SYSTEM_OKTOEXIT, ShutDownPlugin);

	if(!ServiceExists(MS_SKIN2_ADDICON)) {
		hSoundOn = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SND_ON),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
		hSoundOff = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SND_OFF),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
		hIconMRadio = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_MRADIO),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
		hIconMRadio_off = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_MRAADIO_OFF),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
	}
	else {
		SKINICONDESC sid;
		char ModuleName[MAX_PATH];

		sid.cbSize = sizeof(sid);
		sid.pszSection = Translate("Sound Volume");
		GetModuleFileNameA(hInst, ModuleName, sizeof(ModuleName));
		sid.pszDefaultFile = ModuleName;
#define myLOAD_ICON(n, result)\
		sid.pszName = szIconId[n];\
		sid.pszDescription = szIconName[n];\
		sid.iDefaultIndex = iIconId[n];\
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);\
		result = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[n]);

		myLOAD_ICON(0, hSoundOff);
		myLOAD_ICON(1, hSoundOn);		
		myLOAD_ICON(2, hIconMRadio_off);
		myLOAD_ICON(3, hIconMRadio);

		hHookSkinIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, OnSkinIconsChanged);
	}
	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))	{
		WNDCLASSEX wcx;
		CLISTFrame f;

		ZeroMemory( &wcx, sizeof( wcx ) );

		wcx.cbSize			=	sizeof(WNDCLASSEX);
		wcx.style			=	0;
		wcx.lpfnWndProc		=	WndProc;
		wcx.cbClsExtra		=	0;
		wcx.cbWndExtra		=	0;
		wcx.hInstance		=	hInst;
		wcx.hIcon			=	NULL;
		wcx.hCursor			=	LoadCursor(hInst, IDC_ARROW);
		wcx.hbrBackground	=	(HBRUSH)(COLOR_3DFACE+1);
		wcx.lpszMenuName	=	NULL;
		wcx.lpszClassName	=	WndClass;
		wcx.hIconSm			=	NULL;

		RegisterClassEx(&wcx);

		hwndFrame = CreateWindow(WndClass, NULL, WS_CHILD | WS_VISIBLE,	0, 0, 120, MUTE_H+2,
								 (HWND)CallService( MS_CLUI_GETHWND, 0, 0 ), NULL, hInst, NULL);

		ZeroMemory(&f, sizeof(CLISTFrame));
		f.align = alBottom;
		f.cbSize = sizeof(CLISTFrame);
		f.height = MUTE_H+2;
		f.Flags = F_VISIBLE;
		f.hWnd = hwndFrame;
		f.name = "SoundVolume";
		g_hFrame = (HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME,(int)&f,0);
	}
	else
		MessageBox(0, _T("Frame services not found. The sound volume plugin needs a multiwindow-enabled contact list"), _T("Critical"), MB_OK | MB_ICONERROR);

	useSound = DBGetContactSettingByte(NULL, "Skin", "UseSound", 1);
	DBWriteContactSettingByte(NULL, "Skin", "UseSound", !useSound);
	DBWriteContactSettingByte(NULL, "Skin", "UseSound", useSound);

	LoadBackgroundSettings();
	return 0;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;

	playSnd::Init();
	hHookSkinPlayingSound = HookEvent(ME_SKIN_PLAYINGSOUND, OnSkinPlayingSound);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookDbSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);
	hHookMouse = SetWindowsHookEx(WH_MOUSE, OnMouseHook, hInst, 0);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
