/*
Copyright (C) 2010, 2011 tico-tico
*/

#include "bass_interface.h"
#define BASSDEF(f) (WINAPI *f)
#include "bass.h"

#define LOADBASSFUNCTION(f) (*((void**)&f)=(void*)GetProcAddress(hBass,#f))

HINSTANCE hInst;
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
	// {2F07EA05-05B5-4FF0-875D-C590DA2DDAC1}
	{0x2f07ea05, 0x05b5, 0x4ff0, {0x87, 0x5d, 0xc5, 0x90, 0xda, 0x2d, 0xda, 0xc1}}
};

static HINSTANCE hBass = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

#define MAXCHAN 5
static TCHAR CurrBassPath[MAX_PATH], tmp[MAX_PATH];
static int sndNSnd = 0, sndLimSnd;
static HSTREAM sndSSnd[MAXCHAN] = {0};

static WORD TimeWrd1, TimeWrd2, StatMask;
static BOOL QuietTime, Preview, EnPreview;
static int Volume;
static int device = -1;
static int newBass = 0;
static HWND ClistHWND;

HWND hwndSlider = NULL, hwndMute = NULL, hwndOptSlider = NULL, hwnd_plugin = NULL;
COLORREF clBack = 0;
HBRUSH hBkgBrush = 0;
HANDLE frame_id = NULL;
HANDLE hBASSFolder = NULL, hPlaySound = NULL;

static int OnPlaySnd(WPARAM wParam, LPARAM lParam)
{
	TCHAR* ptszFile = (TCHAR*)lParam;
	SYSTEMTIME systime;
	BOOL doPlay = TRUE;

	GetLocalTime(&systime);
	WORD currtime = MAKEWORD(systime.wMinute, systime.wHour);

	WORD currstat = 1;
	switch (CallService(MS_CLIST_GETSTATUSMODE, 0, 0)) {
		case ID_STATUS_OUTTOLUNCH:   currstat <<= 1;
		case ID_STATUS_ONTHEPHONE:   currstat <<= 1;
		case ID_STATUS_INVISIBLE:    currstat <<= 1;
		case ID_STATUS_FREECHAT:     currstat <<= 1;
		case ID_STATUS_DND:          currstat <<= 1;
		case ID_STATUS_OCCUPIED:     currstat <<= 1;
		case ID_STATUS_NA:           currstat <<= 1;
		case ID_STATUS_AWAY:         currstat <<= 1;
		case ID_STATUS_ONLINE:       currstat <<= 1;
	}

	if ( !db_get_b(NULL, "Skin", "UseSound", 0))
		doPlay = FALSE;

	if (QuietTime)
		if (
			((TimeWrd1 < TimeWrd2) && (TimeWrd1 <= currtime && currtime < TimeWrd2)) ||
			((TimeWrd2 < TimeWrd1) && (TimeWrd1 <= currtime || currtime < TimeWrd2))
				) doPlay = FALSE;

	if ( !(currstat & StatMask))
		doPlay = FALSE;

	if (Preview || (int)wParam == 1)
		doPlay = TRUE;

	if ( !ptszFile)
		doPlay = FALSE;

	if (doPlay) {
		BASS_StreamFree(sndSSnd[sndNSnd]);
		sndSSnd[sndNSnd] = BASS_StreamCreateFileW(FALSE, ptszFile, 0, 0, BASS_STREAM_AUTOFREE);
		BASS_ChannelPlay(sndSSnd[sndNSnd], FALSE);
		sndNSnd = (sndNSnd + 1)%sndLimSnd;
	}

	Preview = FALSE;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options

#define SLIDER_MIN 0
#define SLIDER_MAX 100

INT_PTR CALLBACK OptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SYSTEMTIME systime = {0};

			SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_RESETCONTENT, 0, 0);
			for(int i = 1; i <= MAXCHAN; i++)
				SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_ADDSTRING, 0, (LPARAM)_itot(i, tmp, 10));

			SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_SETCURSEL, sndLimSnd - 1, 0);

			SetDlgItemText(hwndDlg, IDC_CURRPATH, CurrBassPath);

			hwndOptSlider = GetDlgItem(hwndDlg, IDC_VOLUME);
			SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN, SLIDER_MAX));
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

			for(int i = IDC_CHECKBOX1; i < IDC_CHECKBOX10 + 1; i++)
				if (StatMask & (1 << (i - IDC_CHECKBOX1)))
					CheckDlgButton(hwndDlg, i, BST_CHECKED);

			if (QuietTime) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIME1), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIME2), TRUE);
				CheckDlgButton(hwndDlg, IDC_QUIETTIME, BST_CHECKED);
			}

			if (hBass == NULL) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_VOLUME), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_MAXCHANNEL), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OUTDEVICE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_QUIETTIME), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), FALSE);

				for(int i = IDC_CHECKBOX1; i < IDC_CHECKBOX10 + 1; i++)
					EnableWindow(GetDlgItem(hwndDlg, i), FALSE);
			}
			else {
				DBVARIANT dbv = {0}; BASS_DEVICEINFO info; DWORD bassver = BASS_GetVersion();

				mir_sntprintf(tmp, SIZEOF(tmp), TranslateT("un4seen's bass version: %d.%d.%d.%d"), bassver >> 24, (bassver >> 16) & 0xff, (bassver >> 8) & 0xff, bassver & 0xff);
				SetDlgItemText(hwndDlg, IDC_BASSVERSION, tmp);

				SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_ADDSTRING, 0, (LPARAM)TranslateT("--default device--"));
				SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_SETCURSEL, 0, 0);
				db_get_ts(NULL, ModuleName, OPT_OUTDEVICE, &dbv);
				for (int i = 1; BASS_GetDeviceInfo(i + newBass, &info); i++) {
					SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_ADDSTRING, 0, (LPARAM)(TCHAR*)_A2T(info.name));
					if ( !mir_tstrcmp(dbv.ptszVal, _A2T(info.name)))
						SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_SETCURSEL, i, 0);
				}
				db_free(&dbv);
			}
		}
		return TRUE;

	case WM_HSCROLL:
		if (hBass != NULL)
			if (LOWORD(wParam) == SB_ENDSCROLL || LOWORD(wParam) == SB_THUMBTRACK) {
				Volume = SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
				BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
				SendMessage(hwndSlider, TBM_SETPOS, TRUE, Volume);
				Preview = TRUE;
				if (EnPreview)
					SkinPlaySound("AlertMsg");
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			if (hBass != NULL) {
				SYSTEMTIME systime = {0};

				SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, WM_GETTEXT, SIZEOF(tmp), (LPARAM)tmp);
				db_set_ts(NULL, ModuleName, OPT_OUTDEVICE, tmp);

				Volume = (DWORD)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
				db_set_b(NULL, ModuleName, OPT_VOLUME, Volume);

				sndLimSnd = SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_GETCURSEL, 0, 0) + 1;
				if (sndLimSnd > MAXCHAN)
					sndLimSnd = MAXCHAN;
				db_set_b(NULL, ModuleName, OPT_MAXCHAN, sndLimSnd);

				QuietTime = IsDlgButtonChecked(hwndDlg, IDC_QUIETTIME) == BST_CHECKED;
				db_set_b(NULL, ModuleName, OPT_QUIETTIME, QuietTime);

				SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
				TimeWrd1 = MAKEWORD(systime.wMinute, systime.wHour);
				db_set_w(NULL, ModuleName, OPT_TIME1, TimeWrd1);

				SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
				TimeWrd2 = MAKEWORD(systime.wMinute, systime.wHour);
				db_set_w(NULL, ModuleName, OPT_TIME2, TimeWrd2);

				EnPreview = IsDlgButtonChecked(hwndDlg, IDC_PREVIEW) == BST_CHECKED;
				db_set_b(NULL, ModuleName, OPT_PREVIEW, EnPreview);

				StatMask = 0;
				for(int i = IDC_CHECKBOX10; i > IDC_CHECKBOX1 - 1; i--)
				{
					StatMask <<= 1;
					if (IsDlgButtonChecked(hwndDlg, i) == BST_CHECKED)
						StatMask |= 1;
				}

				db_set_w(NULL, ModuleName, OPT_STATUS, StatMask);

				device = SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_GETCURSEL, 0, 0);
				if (device == 0)
					device = -1;
				else
					device += newBass;
			}
			return 1;

		case PSN_RESET:
			if (hBass != NULL)
				BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
			return 1;

		case DTN_DATETIMECHANGE:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return 1;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
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
			CallService(MS_UTILS_OPENURL , OUF_NEWWINDOW, (LPARAM)"http://www.un4seen.com/");
			break;
		}
		break;
	}
	return 0;
}

int OptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = ModuleName;
	odp.pfnDlgProc = OptionsProc;
	odp.pszGroup = LPGEN("Sounds");
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Frame

static void EnableFrameIcon(bool bEnable)
{
	if (bEnable) {
		SendMessage(hwndMute, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon("BASSSoundOn"));
		SendMessage(hwndMute, BUTTONADDTOOLTIP, (WPARAM)Translate("Disable sounds"), 0);
	} else {
		SendMessage(hwndMute, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_GetIcon("BASSSoundOff"));
		SendMessage(hwndMute, BUTTONADDTOOLTIP, (WPARAM)Translate("Enable sounds"), 0);
	}
}

static void fnPainter(MButtonCtrl *btn, HDC hdc)
{
	DrawIconEx(hdc, 0, 0, btn->hIcon, 16, 16, 0, hBkgBrush, DI_NORMAL);
}

static LRESULT CALLBACK SliderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_ERASEBKGND:
		return TRUE;
	}
	return mir_callNextSubclass(hwnd, SliderWndProc, msg, wParam, lParam);
}

static LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CREATE:
		hwndMute = CreateWindow(MIRANDABUTTONCLASS, _T(""), WS_CHILD | WS_VISIBLE, 1, 1, 16, 16, hwnd,
			0, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		SendMessage(hwndMute, BUTTONSETASFLATBTN, 1, 0);
		SendMessage(hwndMute, BUTTONSETCUSTOMPAINT, 0, (LPARAM)&fnPainter);

		EnableFrameIcon( db_get_b(NULL, "Skin", "UseSound", 0) != 0);

		hwndSlider = CreateWindow(TRACKBAR_CLASS, _T(""), WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_TOOLTIPS, 21, 1, 100, 20,
			hwnd, (HMENU)0, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		SendMessage(hwndSlider, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN, SLIDER_MAX));
		SendMessage(hwndSlider, TBM_SETPOS, TRUE, Volume);
		mir_subclassWindow(hwndSlider, SliderWndProc);
		break;

	case WM_COMMAND:
		if ((HWND)lParam == hwndMute) {
			bool useSound = !db_get_b(NULL, "Skin", "UseSound", 0);
			db_set_b(NULL, "Skin", "UseSound", useSound);
			EnableFrameIcon(useSound);
		}
		break;

	case WM_HSCROLL:
		if (hBass != NULL)
			if (LOWORD(wParam) == SB_ENDSCROLL || LOWORD(wParam) == SB_THUMBTRACK) {
				Volume = (DWORD)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
				db_set_b(NULL, ModuleName, OPT_VOLUME, Volume);
				BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
				SendMessage(hwndOptSlider, TBM_SETPOS, TRUE, Volume);
				Preview = TRUE;
				if (EnPreview)
					SkinPlaySound("AlertMsg");
			}
		break;

	case WM_SIZE:
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			if (hwndMute)
				MoveWindow(hwndMute, rect.right - 20, 2, 16, 16, FALSE);
			SetWindowPos(hwndSlider, 0, 1, rect.top + 1 + (20-18)/2, rect.right - rect.left - 1 - 20, 18, SWP_NOZORDER);
			InvalidateRect(hwnd, &rect, FALSE);
			return 0;
		}

	case WM_ERASEBKGND:
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			FillRect((HDC)wParam, &rc, hBkgBrush);
			return TRUE;
		}

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hwndSlider) {
			HDC dc = (HDC)wParam;
			SetBkColor(dc, clBack);
			return (BOOL)hBkgBrush;
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

int ReloadColors(WPARAM wParam, LPARAM lParam)
{
	ColourIDT colourid = {0};
	colourid.cbSize = sizeof(colourid);
	_tcscpy(colourid.group, _T(ModuleName));
	_tcscpy(colourid.name, LPGENT("Frame background"));
	clBack = CallService(MS_COLOUR_GETT, (WPARAM)&colourid, 0);

	if (hBkgBrush)
		DeleteObject(hBkgBrush);
	hBkgBrush = CreateSolidBrush(clBack);
	HWND hwnd = GetFocus();
	InvalidateRect(hwndSlider, NULL, TRUE);
	SetFocus(hwndSlider);
	RedrawWindow(hwnd_plugin, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
	SetFocus(hwnd);
	return 0;
}

void CreateFrame()
{
	if ( !ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		return;

	WNDCLASS wndclass = {0};
	wndclass.style = 0;
	wndclass.lpfnWndProc = FrameWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = 0;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = _T("BassInterfaceFrame");
	RegisterClass(&wndclass);

	hwnd_plugin = CreateWindow(_T("BassInterfaceFrame"), TranslateT("Bass Interface"),
		WS_CHILD | WS_CLIPCHILDREN, 0, 0, 10, 10, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), NULL, hInst, NULL);

	CLISTFrame Frame = {sizeof(CLISTFrame)};
	Frame.tname = TranslateT("Bass Interface");
	Frame.hWnd = hwnd_plugin;
	Frame.align = alBottom;
	Frame.Flags = F_TCHAR | F_VISIBLE | F_SHOWTB | F_SHOWTBTIP;
	Frame.height = 22;
	Frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
	frame_id = (HANDLE)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);

	ColourIDT colourid = {0};
	colourid.cbSize = sizeof(ColourIDT);
	strcpy(colourid.dbSettingsGroup, ModuleName);
	strcpy(colourid.setting, "ColorFrame");
	_tcscpy(colourid.name, LPGENT("Frame background"));
	_tcscpy(colourid.group, _T(ModuleName));
	colourid.defcolour = GetSysColor(COLOR_3DFACE);
	ColourRegisterT(&colourid);

	HookEvent(ME_COLOUR_RELOAD, ReloadColors);
	ReloadColors(0, 0);
}

void DeleteFrame()
{
	if (hBkgBrush)
		DeleteObject(hBkgBrush);

	CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)frame_id, 0);
}

void LoadBassLibrary(TCHAR CurrBassPath[MAX_PATH])
{
	hBass = LoadLibrary(CurrBassPath);
	if (hBass != NULL) {
		if (LOADBASSFUNCTION(BASS_Init) != NULL && LOADBASSFUNCTION(BASS_SetConfig) != NULL &&
			LOADBASSFUNCTION(BASS_ChannelPlay) != NULL && LOADBASSFUNCTION(BASS_StreamCreateFile) != NULL &&
			LOADBASSFUNCTION(BASS_GetVersion) != NULL && LOADBASSFUNCTION(BASS_StreamFree) != NULL &&
			LOADBASSFUNCTION(BASS_GetDeviceInfo) != NULL && LOADBASSFUNCTION(BASS_Free))
		{
			BASS_DEVICEINFO info;

			newBass = (BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, TRUE) != 0); // will use new "Default" device

			DBVARIANT dbv = {0};

			if ( !db_get_ts(NULL, ModuleName, OPT_OUTDEVICE, &dbv))
				for (int i = 1; BASS_GetDeviceInfo(i, &info); i++)
					if ( !mir_tstrcmp(dbv.ptszVal, _A2T(info.name)))
						device = i;

			db_free(&dbv);

			sndLimSnd = db_get_b(NULL, ModuleName, OPT_MAXCHAN, MAXCHAN);
			if (sndLimSnd > MAXCHAN)
				sndLimSnd = MAXCHAN;
			TimeWrd1 = db_get_w(NULL, ModuleName, OPT_TIME1, 0);
			TimeWrd2 = db_get_w(NULL, ModuleName, OPT_TIME2, 0);
			QuietTime = db_get_b(NULL, ModuleName, OPT_QUIETTIME, 0);
			EnPreview = db_get_b(NULL, ModuleName, OPT_PREVIEW, 0);

			StatMask = db_get_w(NULL, ModuleName, OPT_STATUS, 0x3ff);

			ClistHWND = (HWND)CallService("CLUI/GetHwnd", 0, 0);
			BASS_Init(device, 44100, 0, ClistHWND, NULL);

			Volume = db_get_b(NULL, ModuleName, OPT_VOLUME, 33);
			BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100 );
			hPlaySound = HookEvent(ME_SKIN_PLAYINGSOUND, OnPlaySnd);
			CreateFrame();
		}
		else {
			FreeLibrary(hBass);
			hBass = NULL;
		}
	}
}

int OnFoldersChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathT(hBASSFolder, CurrBassPath, MAX_PATH, _T(""));
	_tcscat(CurrBassPath, _T("\\bass.dll"));

	if (hBass != NULL) {
		BASS_Free();
		FreeLibrary(hBass);
		UnhookEvent(hPlaySound);
		DeleteFrame();
	}
	LoadBassLibrary(CurrBassPath);

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	if (hBASSFolder = FoldersRegisterCustomPathT(LPGEN("Bass Interface"), LPGEN("Bass library"), PLUGINS_PATHT _T("\\Bass"))) {
		FoldersGetCustomPathT(hBASSFolder, CurrBassPath, MAX_PATH, _T(""));
		_tcscat(CurrBassPath, _T("\\bass.dll"));
	}
	else {
		DBVARIANT dbv;
		if ( db_get_ts(NULL, ModuleName, OPT_BASSPATH, &dbv)) {
			mir_tstrncpy(CurrBassPath, VARST( _T("Plugins\\Bass\\bass.dll")), SIZEOF(CurrBassPath));
			db_set_ts(NULL, ModuleName, OPT_BASSPATH, CurrBassPath);
		}
		else {
			mir_tstrcpy(CurrBassPath, dbv.ptszVal);
			db_free(&dbv);
		}
	}

	LoadBassLibrary(CurrBassPath);

	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);
	return 0;
}

int OnSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0 || lParam == NULL)
		return 0;

	DBCONTACTWRITESETTING *dbcws=(DBCONTACTWRITESETTING*)lParam;
	if ( !strcmp(dbcws->szModule, "Skin")) {
		if ( !strcmp(dbcws->szSetting, "UseSound")) {
			EnableFrameIcon(dbcws->value.bVal != 0);
			return 0;
		}
	}
	return 0;
}

int OnShutdown(WPARAM wParam, LPARAM lParam)
{
	if (hBass != NULL) {
		BASS_Free();
		FreeLibrary(hBass);
	}

	DeleteFrame();
	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Sounds enabled"),  "BASSSoundOn",  IDI_BASSSoundOn  },
	{ LPGEN("Sounds disabled"), "BASSSoundOff", IDI_BASSSoundOff }
};

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);

	Icon_Register(hInst, ModuleName, iconList, SIZEOF(iconList));
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
