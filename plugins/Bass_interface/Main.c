/*
Copyright (C) 2010, 2011 tico-tico
*/

#include "bass_interface.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
int hLangpack;

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
	0,
	MIID_BASSINT
};

struct MM_INTERFACE mmi;

static HANDLE hHooks[5] = {0};
static HANDLE hService;
static HINSTANCE hBass = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_BASSINT, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

#define MAXCHAN	5
static TCHAR CurrBassPath[MAX_PATH], tmp[MAX_PATH];
static int sndNSnd = 0, sndLimSnd;
static HSTREAM sndSSnd[MAXCHAN] = {0};

static WORD TimeWrd1, TimeWrd2, StatMask;
static BOOL QuietTime, Preview, EnPreview;
static int Volume;
static int device = -1;
static int newBass = 0;
static HWND ClistHWND;

static int OnPlaySnd(WPARAM wParam, LPARAM lParam)
{
	char * pszFile = (char *) lParam; SYSTEMTIME systime; WORD currtime, currstat; BOOL doPlay = TRUE;

	GetLocalTime(&systime);
	currtime = MAKEWORD(systime.wMinute, systime.wHour);

	currstat = 1;
	switch (CallService(MS_CLIST_GETSTATUSMODE, 0, 0))
	{
		case ID_STATUS_OUTTOLUNCH:	currstat <<= 1;
		case ID_STATUS_ONTHEPHONE:	currstat <<= 1;
		case ID_STATUS_INVISIBLE:	currstat <<= 1;
		case ID_STATUS_FREECHAT:	currstat <<= 1;
		case ID_STATUS_DND:			currstat <<= 1;
		case ID_STATUS_OCCUPIED:	currstat <<= 1;
		case ID_STATUS_NA:			currstat <<= 1;
		case ID_STATUS_AWAY:		currstat <<= 1;
		case ID_STATUS_ONLINE:		currstat <<= 1;
	}

	if ( !DBGetContactSettingByte(NULL,"Skin","UseSound",0) ) doPlay = FALSE;

	if ( QuietTime )
		if (
			((TimeWrd1 < TimeWrd2) && (TimeWrd1 <= currtime && currtime < TimeWrd2)) ||
			((TimeWrd2 < TimeWrd1) && (TimeWrd1 <= currtime || currtime < TimeWrd2))
				) doPlay = FALSE;
		
	if ( !(currstat & StatMask) ) doPlay = FALSE;

	if ( Preview || (int)wParam==1 ) doPlay = TRUE;

	if ( !pszFile ) doPlay = FALSE;

	if ( doPlay )
	{
		BASS_StreamFree(sndSSnd[sndNSnd]);
		sndSSnd[sndNSnd] = BASS_StreamCreateFile(FALSE, pszFile, 0, 0, BASS_STREAM_AUTOFREE);
		BASS_ChannelPlay(sndSSnd[sndNSnd], FALSE);
		sndNSnd = (sndNSnd+1)%sndLimSnd;
	}

	Preview = FALSE;

	return 0;
}

#define SLIDER_MIN  0
#define SLIDER_MAX  100

INT_PTR CALLBACK OptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			{
				int i; SYSTEMTIME systime = {0};

				TranslateDialogDefault(hwndDlg);

				SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_RESETCONTENT, 0, 0);
				for(i=1; i<=MAXCHAN; i++)
					SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_ADDSTRING, 0, (LPARAM)_itot(i, tmp, 10));

				SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_SETCURSEL, sndLimSnd - 1, 0);
				
				SendDlgItemMessage(hwndDlg, IDC_CURRPATH, WM_SETTEXT, 0, (LPARAM)CurrBassPath);

				SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN,SLIDER_MAX));
				SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPOS, TRUE, Volume);
				SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPAGESIZE, 0, 5);

				systime.wHour = HIBYTE(TimeWrd1);
				systime.wMinute = LOBYTE(TimeWrd1);
				systime.wYear = 2000;
				systime.wMonth = 1;
				systime.wDay = 1;
				SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_SETFORMAT, 0, (LPARAM)_T("HH:mm"));
				SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&systime);
				systime.wHour = HIBYTE(TimeWrd2);
				systime.wMinute = LOBYTE(TimeWrd2);
				SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_SETFORMAT, 0, (LPARAM)_T("HH:mm"));
				SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&systime);
				
				CheckDlgButton(hwndDlg, IDC_PREVIEW, EnPreview ? BST_CHECKED : BST_UNCHECKED);

				for(i = IDC_CHECKBOX1; i < IDC_CHECKBOX10+1; i++)
				{
					if (StatMask & (1 << (i-IDC_CHECKBOX1))) 
						CheckDlgButton(hwndDlg, i, BST_CHECKED);
				}

				if (QuietTime)
				{
					EnableWindow(GetDlgItem(hwndDlg,IDC_TIME1), TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_TIME2), TRUE);
					CheckDlgButton(hwndDlg, IDC_QUIETTIME, BST_CHECKED);
				}
				if (hBass == NULL)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_VOLUME), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_MAXCHANNEL), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_OUTDEVICE), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_QUIETTIME), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), FALSE);

					for(i = IDC_CHECKBOX1; i < IDC_CHECKBOX10+1; i++)
					{
						EnableWindow(GetDlgItem(hwndDlg, i), FALSE);
					}
				}
				else
				{
					DBVARIANT dbv = {0}; BASS_DEVICEINFO info; DWORD bassver = BASS_GetVersion();

					mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("un4seen's bass version: %d.%d.%d.%d"), bassver >> 24, (bassver >> 16) & 0xff, (bassver >> 8) & 0xff, bassver & 0xff);
					SendDlgItemMessage(hwndDlg, IDC_BASSVERSION, WM_SETTEXT, 0, (LPARAM)tmp);

					SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_RESETCONTENT, 0, 0);
					SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_ADDSTRING, 0, (LPARAM)TranslateT("--default device--"));
					SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_SETCURSEL, 0, 0);
					DBGetContactSettingString(NULL, ModuleName, OPT_OUTDEVICE, &dbv);
					for (i=1; BASS_GetDeviceInfo(i+newBass, &info); i++)
					{
						SendDlgItemMessageA(hwndDlg, IDC_OUTDEVICE, CB_ADDSTRING, 0, (LPARAM)info.name);
						if (!lstrcmpA(dbv.pszVal, info.name))
							SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_SETCURSEL, i, 0);
					}
					DBFreeVariant(&dbv);
				}

				return 1;
			}

		case WM_HSCROLL:
			if (hBass != NULL)				
				if (LOWORD(wParam) == SB_ENDSCROLL || LOWORD(wParam) == SB_THUMBTRACK)
				{
					BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0) * 100);
					Preview = TRUE;
					if (EnPreview) SkinPlaySound("AlertMsg");
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
					if (hBass != NULL)
					{
						int i; SYSTEMTIME systime;

						SendDlgItemMessageA(hwndDlg, IDC_OUTDEVICE, WM_GETTEXT, sizeof(tmp), (LPARAM)tmp);
						DBWriteContactSettingString(NULL, ModuleName, OPT_OUTDEVICE, (char *)tmp);
						
						Volume = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
						DBWriteContactSettingByte(NULL, ModuleName, OPT_VOLUME, Volume);

						sndLimSnd = SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_GETCURSEL, 0, 0) + 1;
						if ( sndLimSnd > MAXCHAN ) sndLimSnd = MAXCHAN;
						DBWriteContactSettingByte(NULL, ModuleName, OPT_MAXCHAN, sndLimSnd);

						QuietTime = IsDlgButtonChecked(hwndDlg, IDC_QUIETTIME) == BST_CHECKED;
						DBWriteContactSettingByte(NULL, ModuleName, OPT_QUIETTIME, QuietTime);
						
						SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
						TimeWrd1 = MAKEWORD(systime.wMinute, systime.wHour);
						DBWriteContactSettingWord(NULL, ModuleName, OPT_TIME1, TimeWrd1);
						
						SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
						TimeWrd2 = MAKEWORD(systime.wMinute, systime.wHour);
						DBWriteContactSettingWord(NULL, ModuleName, OPT_TIME2, TimeWrd2);
						
						EnPreview = IsDlgButtonChecked(hwndDlg, IDC_PREVIEW) == BST_CHECKED;
						DBWriteContactSettingByte(NULL, ModuleName, OPT_PREVIEW, EnPreview);

						StatMask = 0;
						for(i = IDC_CHECKBOX10; i > IDC_CHECKBOX1-1; i--)
						{
							StatMask <<= 1;
							if(IsDlgButtonChecked(hwndDlg, i) == BST_CHECKED)
								StatMask |= 1;
						}

						DBWriteContactSettingWord(NULL, ModuleName, OPT_STATUS, StatMask);

						device = SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_GETCURSEL, 0, 0);
						if (device == 0)	device = -1;
						else				device += newBass;
						if (CallService(MS_TB_GETBUTTONSTATEBYID, (WPARAM)"BASSSoundOnOff", 0)==TBST_RELEASED)
						{
							BASS_Free();
							BASS_Init(device, 44100, 0, ClistHWND, NULL);
							BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100 );
						}
					}
					return 1;

				case PSN_RESET:
					if (hBass != NULL)
					{
						BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
					}
					return 1;
					
				case DTN_DATETIMECHANGE:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return 1;
			}
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_QUIETTIME:
					{
						BOOL b = IsDlgButtonChecked(hwndDlg, IDC_QUIETTIME) == BST_CHECKED;
						EnableWindow(GetDlgItem(hwndDlg,IDC_TIME1), b);
						EnableWindow(GetDlgItem(hwndDlg,IDC_TIME2), b);
					}
				case IDC_MAXCHANNEL:
				case IDC_OUTDEVICE:
				case IDC_CHECKBOX1:
				case IDC_CHECKBOX2:
				case IDC_CHECKBOX3:
				case IDC_CHECKBOX4:
				case IDC_CHECKBOX5:
				case IDC_CHECKBOX6:
				case IDC_CHECKBOX7:
				case IDC_CHECKBOX8:
				case IDC_CHECKBOX9:
				case IDC_CHECKBOX10:
				case IDC_PREVIEW:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_GETBASS:
					CallService(MS_UTILS_OPENURL , 1, (LPARAM)"http://www.un4seen.com/");
					break;
			}
			break;
		
		//case WM_CLOSE:
		//	EndDialog(hwndDlg, 0);
	}
	return 0;
}

struct
{
	char* szDescr;
	char* szName;
	int   defIconID;
}
static const iconList[] =
{
	{ "Audio device is opened", "BASSSoundOnOffUp", IDI_BASSSoundOnOffUp },
	{ "Audio device is closed", "BASSSoundOnOffDown", IDI_BASSSoundOnOffDown }
};

static HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons(void)
{
	int i; char szSettingName[100]; SKINICONDESC sid = {0}; TCHAR szFile[MAX_PATH];

	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.cx = sid.cy = 16;
	sid.pszName = szSettingName;
	sid.pszSection = ModuleName;
	sid.flags = SIDF_PATH_TCHAR;

	for (i = 0; i < SIZEOF(iconList); i++) 
	{
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", ModuleName, iconList[i].szName);

		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}	
}

int OptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszTitle = _T(ModuleName);
	odp.pfnDlgProc = OptionsProc;
	odp.ptszGroup = _T("Customize");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

INT_PTR BASSSoundOnOff(WPARAM wParam, LPARAM lParam)
{
	if (hBass != NULL)
	{
		BOOL opened = CallService(MS_TB_GETBUTTONSTATEBYID, (WPARAM)"BASSSoundOnOff", 0) == TBST_RELEASED;

		if ( opened )		
		{
			BASS_Free();				// Close Device
		}
		else
		{
			BASS_Init(device, 44100, 0, ClistHWND, NULL);
			BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100 );
		}
		
		CallService(MS_TB_SETBUTTONSTATEBYID, (WPARAM)"BASSSoundOnOff", opened ? TBST_PUSHED : TBST_RELEASED);
		DBWriteContactSettingByte(NULL, ModuleName, OPT_DEVOPEN, !opened);
	}
	return 0;
}

int OnToolbarLoaded(WPARAM wParam, LPARAM lParam) 
{
	TBButton tbb				= {0};
	tbb.cbSize					= sizeof(TBButton);
	tbb.pszButtonID				= "BASSSoundOnOff";
	tbb.pszButtonName			= Translate("Open/close audio device");
	tbb.pszServiceName			= "BASSinterface/BASSSoundOnOff";
	tbb.pszTooltipUp			= Translate("Audio device is opened");
	tbb.pszTooltipDn			= Translate("Audio device is closed");
	tbb.hPrimaryIconHandle		= hIconLibItem[0];
	tbb.hSecondaryIconHandle	= hIconLibItem[1];
	tbb.tbbFlags				= TBBF_SHOWTOOLTIP;
	tbb.defPos					= 1000;
	
	CallService(MS_TB_ADDBUTTON, 0, (LPARAM)&tbb);

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) 
{
	DBVARIANT dbv = {0};

	if (DBGetContactSettingTString(NULL, ModuleName, OPT_BASSPATH, &dbv)) 
	{
		DBWriteContactSettingTString(NULL, ModuleName, OPT_BASSPATH, _T("Plugins\\Bass\\Bass.dll"));
		lstrcpy(tmp, _T("Plugins\\Bass\\Bass.dll"));
	}
	else
	{
		lstrcpy(tmp, dbv.ptszVal);
	}
	
	DBFreeVariant(&dbv);
	
	CallService(MS_UTILS_PATHTOABSOLUTET, (WPARAM)tmp, (LPARAM)CurrBassPath);
	hBass = LoadLibrary(CurrBassPath);

	if (hBass != NULL)
	{
		if (LOADBASSFUNCTION(BASS_Init) != NULL && LOADBASSFUNCTION(BASS_SetConfig) != NULL &&
			LOADBASSFUNCTION(BASS_ChannelPlay) != NULL && LOADBASSFUNCTION(BASS_StreamCreateFile) != NULL &&
			LOADBASSFUNCTION(BASS_GetVersion) != NULL && LOADBASSFUNCTION(BASS_StreamFree) != NULL &&
			LOADBASSFUNCTION(BASS_GetDeviceInfo) != NULL && LOADBASSFUNCTION(BASS_Free))
		{
			int i; BASS_DEVICEINFO info;

			newBass = (BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, TRUE) != 0); // will use new "Default" device			

			if (!DBGetContactSettingString(NULL, ModuleName, OPT_OUTDEVICE, &dbv))
			{
				for (i=1; BASS_GetDeviceInfo(i, &info); i++)
					if (!lstrcmpA(dbv.pszVal, info.name))
						device = i;
			}
			DBFreeVariant(&dbv);

			sndLimSnd = DBGetContactSettingByte(NULL, ModuleName, OPT_MAXCHAN, MAXCHAN);
			if ( sndLimSnd > MAXCHAN ) sndLimSnd = MAXCHAN;
			TimeWrd1 = DBGetContactSettingWord(NULL, ModuleName, OPT_TIME1, 0);
			TimeWrd2 = DBGetContactSettingWord(NULL, ModuleName, OPT_TIME2, 0);
			QuietTime = DBGetContactSettingByte(NULL, ModuleName, OPT_QUIETTIME, 0);
			EnPreview = DBGetContactSettingByte(NULL, ModuleName, OPT_PREVIEW, 0);

			StatMask = DBGetContactSettingWord(NULL, ModuleName, OPT_STATUS, 0x3ff);

			ClistHWND = (HWND)CallService("CLUI/GetHwnd", 0, 0);
			if (DBGetContactSettingByte(NULL, ModuleName, OPT_DEVOPEN, 1))
				BASS_Init(device, 44100, 0, ClistHWND, NULL);
			else
				CallService(MS_TB_SETBUTTONSTATEBYID, (WPARAM)"BASSSoundOnOff", TBST_PUSHED);

			Volume = DBGetContactSettingByte(NULL, ModuleName, OPT_VOLUME, 33);
			BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100 );
			hHooks[3] = HookEvent(ME_SKIN_PLAYINGSOUND, OnPlaySnd);
		}
		else
		{
			FreeLibrary(hBass);
			hBass = NULL;
		}
	}

	hHooks[4] = HookEvent(ME_OPT_INITIALISE, OptionsInit);

	return 0;
}

int OnShutdown(WPARAM wParam, LPARAM lParam) 
{
	if(hBass != NULL)
	{
		BASS_Free();
		FreeLibrary(hBass);
	}

	return 0;
}

int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);

	hHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hHooks[1] = HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	hHooks[2] = HookEvent(ME_TB_MODULELOADED, OnToolbarLoaded);
	
	hService = CreateServiceFunction("BASSinterface/BASSSoundOnOff", BASSSoundOnOff);

	InitIcons();

	return 0;
}

int __declspec(dllexport) Unload(void)
{
	int i;
	for (i = 0; i < SIZEOF(hHooks); i++)
	{
		if (hHooks[i])
			UnhookEvent(hHooks[i]);
	}

	DestroyServiceFunction(hService);

	return 0;
}