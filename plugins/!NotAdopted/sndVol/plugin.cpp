#define _WIN32_WINNT  0x0500
#include "plugin.h"
#include "playSnd.h"
#include "optionsdlg.h"
#include "resource.h"
#include "m_cluiframes.h"

#include "IcoLib.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HWND hwndFrame = NULL, hwndMute = NULL, hwndSlider = NULL, hwndOptSlider = NULL;
HANDLE hHookSkinPlayingSound, hHookDbSettingChange, hHookSkinIconsChanged;
HHOOK hHookMouse;
bool showMute = TRUE;

char szTitle[] = "sndVol";
bool isEnabled = FALSE;
HICON hSoundOn = NULL, hSoundOff = NULL;
int isMuteVisible = FALSE;
int autoPreview = FALSE;

char *szIconId[2] = {"sndVol_Off", "sndVol_On"};
char *szIconName[2] = {"Off", "On"};
int iIconId[2] = {-IDI_SND_OFF, -IDI_SND_ON};

PLUGININFO pluginInfo={
	sizeof( PLUGININFO ),
	szTitle,
	PLUGIN_MAKE_VERSION( 0,0,1,1 ),
	"Volume control 4 Miranda's events sound",
	"Denis Stanishevskiy // StDenis",
	"stdenformiranda(at)fromru(dot)com",
	"Copyright (c) 2005, Denis Stanishevskiy",
	"",
	0, 0
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	if(fdwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(GetModuleHandle("sndvol"));
	return TRUE;
}

__declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
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

	int useSound = DBGetContactSettingByte(NULL,"Skin","UseSound",1);
	DBWriteContactSettingByte(NULL,"Skin","UseSound",useSound);
	SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(useSound?hSoundOn:hSoundOff));

	return 0;
}

int OnSettingChanged(WPARAM wParam,LPARAM lParam)
{
	if(wParam != 0 || lParam == NULL) return 0;
	{
		DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
		if(!strcmp(dbcws->szModule,"Skin"))
			if(!strcmp(dbcws->szSetting,"UseSound"))
			{
				int useSound = dbcws->value.bVal;
				SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(useSound?hSoundOn:hSoundOff));

				return 0;
			};
	}

	return 0;
}

static int OnOptionsInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	
	ZeroMemory(&odp, sizeof(odp));

	odp.cbSize = sizeof(odp);
	odp.pszGroup = Translate("Events");
	odp.position = -200000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPT);
	odp.pszTitle = Translate("Sounds Volume");
	odp.pfnDlgProc = (DLGPROC)OptionsDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

BOOL __inline IsMouseInside(HWND hwnd)
{
	POINT pt;
	HWND hwndAtPt;

	GetCursorPos(&pt);
	hwndAtPt = WindowFromPoint(pt);
	while(hwndAtPt != NULL)
	{
		if(hwndAtPt == hwnd)
			return TRUE;
		hwndAtPt = GetParent(hwndAtPt);
	}
	return FALSE;
	//return (hwndAtPt == hwnd) || (GetParent(hwndAtPt) == hwnd);
	//GetWindowRect(hwnd, &rc);
	//return PtInRect(&rc, pt);
}

LRESULT CALLBACK OnMouseHook(int code, WPARAM wParam, LPARAM lParam)
{
	if(code >= 0)
	{
		if(wParam == WM_MOUSEWHEEL)
		{
			//MOUSEHOOKSTRUCTEX *mhk = (MOUSEHOOKSTRUCTEX*)lParam;
			MSLLHOOKSTRUCT *mhk = (MSLLHOOKSTRUCT*)lParam;

			if(IsMouseInside(hwndSlider))
			{
				int count = (short)HIWORD(mhk->mouseData);

				PostMessage(hwndSlider, WM_MOUSEWHEEL, MAKEWPARAM(0,-count), MAKELPARAM(mhk->pt.x, mhk->pt.y));

				return 1;
			}
			
		}
	}
	return CallNextHookEx(hHookMouse, code, wParam, lParam);
}


static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
#define MUTE_W	20
#define MUTE_H	20
			isMuteVisible = DBGetContactSettingByte(NULL,SERVICENAME,"MuteBtn", TRUE);
			hwndMute = CreateWindow(MIRANDABUTTONCLASS,
						"",
						WS_CHILD | (isMuteVisible?WS_VISIBLE:0),
						1,
						1,
						MUTE_W,
						MUTE_H,
						hwnd,
						(HMENU)0,
						(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
						NULL);
			SendMessage(hwndMute, BUTTONSETASFLATBTN,0,0);				
			SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hSoundOn);
			hwndSlider = CreateWindow(TRACKBAR_CLASS,
						"",
						WS_CHILD | WS_VISIBLE | TBS_NOTICKS,
						MUTE_W+1,
						1,
						120,
						MUTE_W,
						hwnd,
						(HMENU)0,
						(HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE), 
						NULL);
			//SendMessage(hwndSlider, TBM_SETRANGE, FALSE, MAKELONG(-10000/3,1000));
			SendMessage(hwndSlider, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN, SLIDER_MAX/SLIDER_DIV));
			SendMessage(hwndSlider, TBM_SETPOS, TRUE, DBGetContactSettingDword(NULL,SERVICENAME,"Volume",1000)/SLIDER_DIV);

			//SetWindowLong(hwndSlider, GWL_USERDATA, GetWindowLong(hwndSlider, GWL_WNDPROC));
			//SetWindowLong(hwndSlider, GWL_WNDPROC, (LONG)SliderWndProc);

			break;
		}
		case WM_COMMAND:
			if((HWND)lParam == hwndMute)
			{
				int useSound = !DBGetContactSettingByte(NULL,"Skin","UseSound",1);
				DBWriteContactSettingByte(NULL,"Skin","UseSound",useSound);
				SendMessage(hwndMute, BM_SETIMAGE,IMAGE_ICON,(LPARAM)(useSound?hSoundOn:hSoundOff));
			}
			break;
		/*
		case WM_MOUSEWHEEL:
		{
			int count = HIWORD(wParam);
			break;
		}
		*/
		case WM_USER:
			ShowWindow(hwndMute, wParam?SW_SHOW:SW_HIDE);
			isMuteVisible = wParam;
		case WM_HSCROLL:
		{
			int value = (DWORD)SendMessage(hwndSlider, TBM_GETPOS, 0, 0)*SLIDER_DIV;
			if(hwndOptSlider)
				SendMessage(hwndOptSlider, TBM_SETPOS, TRUE, value);
			else
				DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
			playSnd::SetVolume(value);
			SkinPlaySound("RecvMsg");

			break;
		}			
		case WM_SIZE:
		{
			RECT rect;
			// = DBGetContactSettingByte(NULL,SERVICENAME,"MuteBtn", TRUE);//IsWindowVisible(hwndMute);

			GetClientRect(hwnd,&rect);
#define SLIDER_H	18
			SetWindowPos(hwndSlider, 0,
				rect.left+(isMuteVisible?MUTE_W:0)+1, rect.top+1+ (MUTE_H-SLIDER_H)/2, rect.right-rect.left-3-(isMuteVisible?MUTE_W:0), SLIDER_H,
				SWP_NOZORDER);
			InvalidateRect(hwnd, &rect, FALSE);
			return 0;
		}
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_RELEASEDCAPTURE:
				{
					int value = (DWORD)SendMessage(hwndSlider, TBM_GETPOS, 0, 0)*SLIDER_DIV;

					DBWriteContactSettingDword(NULL, SERVICENAME, "Volume", value);
					if(hwndOptSlider) SendMessage(hwndOptSlider, TBM_SETPOS, TRUE, value);
					playSnd::SetVolume(value);
					//SkinPlaySound("RecvMsg");

					break;
				}
			}
			break;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam); 
}
#define WndClass "SNDVOL_WND"

int OnModulesLoaded(WPARAM wparam,LPARAM lparam)
{
	if(!ServiceExists(MS_SKIN2_ADDICON))
	{
		hSoundOn = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SND_ON),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
		hSoundOff = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SND_OFF),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
	}
	else
	{
		SKINICONDESC sid;
		char ModuleName[MAX_PATH];

		sid.cbSize = sizeof(sid);
		sid.pszSection = Translate("Sound Volume");
		GetModuleFileName(hInst, ModuleName, sizeof(ModuleName));
		sid.pszDefaultFile = ModuleName;
#define myLOAD_ICON(n, result)\
		sid.pszName = szIconId[n];\
		sid.pszDescription = szIconName[n];\
		sid.iDefaultIndex = iIconId[n];\
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);\
		result = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)szIconId[n]);

		myLOAD_ICON(0, hSoundOff);
		myLOAD_ICON(1, hSoundOn);		
		
		hHookSkinIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, OnSkinIconsChanged);
	}
	if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
	{
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
		//
		RegisterClassEx(&wcx);
		//
		hwndFrame = CreateWindow(WndClass,
							NULL,
							WS_CHILD|WS_VISIBLE,
							0,
							0,
							120,
							MUTE_H+2,
							(HWND)CallService( MS_CLUI_GETHWND, 0, 0 ),
							NULL,
							hInst,
							NULL);
		//
		ZeroMemory(&f, sizeof(CLISTFrame));
		f.align = alBottom;
		f.cbSize = sizeof(CLISTFrame);
		f.height = MUTE_H+2;
		f.Flags = F_VISIBLE;
		f.hWnd = hwndFrame;
		f.name = "SoundVolume";
		CallService(MS_CLIST_FRAMES_ADDFRAME,(int)&f,0);
	}
	return 0;
}


int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink=link;

	playSnd::Init();
	hHookSkinPlayingSound = HookEvent(ME_SKIN_PLAYINGSOUND, OnSkinPlayingSound);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHookDbSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);
	hHookMouse = SetWindowsHookEx(WH_MOUSE_LL, &OnMouseHook, hInst, 0);
	//hHookMouse = SetWindowsHookEx(WH_MOUSE, &OnMouseHook, hInst, 0);

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	if(!ServiceExists(MS_SKIN2_ADDICON))
	{
		DestroyIcon(hSoundOff);
		DestroyIcon(hSoundOn);
		UnhookEvent(hHookSkinIconsChanged);
	}
	UnhookEvent(hHookSkinPlayingSound);
	playSnd::Destroy();
	return 0;
}