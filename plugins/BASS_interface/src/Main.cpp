/*
Copyright (C) 2010, 2011 tico-tico
*/

#include "stdafx.h"

#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#include <delayimp.h>

#include "bass.h"

#pragma comment(lib, "delayimp.lib")
#ifdef _WIN64
	#pragma comment(lib, "src\\bass64.lib")
#else
	#pragma comment(lib, "src\\bass.lib")
#endif

static HINSTANCE hBass = nullptr;

FARPROC WINAPI delayHook(unsigned dliNotify, PDelayLoadInfo dli)
{
	switch (dliNotify)
	{
	case dliNotePreGetProcAddress:
		return GetProcAddress(hBass, dli->dlp.szProcName);
	}
	return nullptr;
}

extern "C" PfnDliHook __pfnDliNotifyHook2 = delayHook;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2F07EA05-05B5-4FF0-875D-C590DA2DDAC1}
	{ 0x2f07ea05, 0x05b5, 0x4ff0, { 0x87, 0x5d, 0xc5, 0x90, 0xda, 0x2d, 0xda, 0xc1 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

#define MAXCHAN 5
static wchar_t CurrBassPath[MAX_PATH], tmp[MAX_PATH];
static int sndNSnd = 0, sndLimSnd;
static HSTREAM sndSSnd[MAXCHAN] = { 0 };

static uint16_t TimeWrd1, TimeWrd2, StatMask;
static BOOL QuietTime, Preview, EnPreview;
static int Volume;
static int device = -1;
static int newBass = 0;
static int frame_id = 0;

HWND hwndSlider = nullptr, hwndMute = nullptr, hwndOptSlider = nullptr, hwnd_plugin = nullptr;
COLORREF clBack = 0;
HBRUSH hBkgBrush = nullptr;
HANDLE hBASSFolder = nullptr, hPlaySound = nullptr;

static int OnPlaySnd(WPARAM wParam, LPARAM lParam)
{
	wchar_t* ptszFile = (wchar_t*)lParam;
	SYSTEMTIME systime;
	BOOL doPlay = TRUE;

	GetLocalTime(&systime);
	uint16_t currtime = MAKEWORD(systime.wMinute, systime.wHour);

	uint16_t currstat = 1;
	switch (CallService(MS_CLIST_GETSTATUSMODE, 0, 0)) {
	case ID_STATUS_INVISIBLE:    currstat <<= 1;
	case ID_STATUS_FREECHAT:     currstat <<= 1;
	case ID_STATUS_DND:          currstat <<= 1;
	case ID_STATUS_OCCUPIED:     currstat <<= 1;
	case ID_STATUS_NA:           currstat <<= 1;
	case ID_STATUS_AWAY:         currstat <<= 1;
	case ID_STATUS_ONLINE:       currstat <<= 1;
	}

	if (!db_get_b(0, "Skin", "UseSound", 0))
		doPlay = FALSE;

	if (QuietTime)
		if (
			((TimeWrd1 < TimeWrd2) && (TimeWrd1 <= currtime && currtime < TimeWrd2)) ||
			((TimeWrd2 < TimeWrd1) && (TimeWrd1 <= currtime || currtime < TimeWrd2))
			) doPlay = FALSE;

	if (!(currstat & StatMask))
		doPlay = FALSE;

	if (Preview || (int)wParam == 1)
		doPlay = TRUE;

	if (!ptszFile)
		doPlay = FALSE;

	if (doPlay) {
		BASS_StreamFree(sndSSnd[sndNSnd]);
		sndSSnd[sndNSnd] = BASS_StreamCreateFile(FALSE, ptszFile, 0, 0, BASS_STREAM_AUTOFREE);
		BASS_ChannelPlay(sndSSnd[sndNSnd], FALSE);
		sndNSnd = (sndNSnd + 1) % sndLimSnd;
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
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_RESETCONTENT, 0, 0);
		for (int i = 1; i <= MAXCHAN; i++)
			SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_ADDSTRING, 0, (LPARAM)_itow(i, tmp, 10));

		SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_SETCURSEL, sndLimSnd - 1, 0);

		SetDlgItemText(hwndDlg, IDC_CURRPATH, CurrBassPath);

		hwndOptSlider = GetDlgItem(hwndDlg, IDC_VOLUME);
		SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN, SLIDER_MAX));
		SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPOS, TRUE, Volume);
		SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_SETPAGESIZE, 0, 5);
		{
			SYSTEMTIME systime = { 0 };
			systime.wHour = HIBYTE(TimeWrd1);
			systime.wMinute = LOBYTE(TimeWrd1);
			systime.wYear = 2000;
			systime.wMonth = 1;
			systime.wDay = 1;
			SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_SETFORMAT, 0, (LPARAM)L"HH:mm");
			SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&systime);
			systime.wHour = HIBYTE(TimeWrd2);
			systime.wMinute = LOBYTE(TimeWrd2);
			SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_SETFORMAT, 0, (LPARAM)L"HH:mm");
			SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&systime);
		}

		CheckDlgButton(hwndDlg, IDC_PREVIEW, EnPreview ? BST_CHECKED : BST_UNCHECKED);

		for (int i = IDC_CHECKBOX1; i <= IDC_CHECKBOX8; i++)
			if (StatMask & (1 << (i - IDC_CHECKBOX1)))
				CheckDlgButton(hwndDlg, i, BST_CHECKED);

		if (QuietTime) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_TIME1), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TIME2), TRUE);
			CheckDlgButton(hwndDlg, IDC_QUIETTIME, BST_CHECKED);
		}

		if (hBass == nullptr) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_VOLUME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXCHANNEL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OUTDEVICE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_QUIETTIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PREVIEW), FALSE);

			for (int i = IDC_CHECKBOX1; i <= IDC_CHECKBOX8; i++)
				EnableWindow(GetDlgItem(hwndDlg, i), FALSE);
		}
		else {
			uint32_t bassver = BASS_GetVersion();
			mir_snwprintf(tmp, TranslateT("Un4seen's BASS version: %d.%d.%d.%d"), bassver >> 24, (bassver >> 16) & 0xff, (bassver >> 8) & 0xff, bassver & 0xff);
			SetDlgItemText(hwndDlg, IDC_BASSVERSION, tmp);

			SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_ADDSTRING, 0, (LPARAM)TranslateT("--default device--"));
			SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_SETCURSEL, 0, 0);

			BASS_DEVICEINFO info;
			ptrW tszDeviceName(g_plugin.getWStringA(OPT_OUTDEVICE));
			for (int i = 1; BASS_GetDeviceInfo(i + newBass, &info); i++) {
				SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_ADDSTRING, 0, _A2T(info.name));
				if (!mir_wstrcmp(tszDeviceName, _A2T(info.name)))
					SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_SETCURSEL, i, 0);
			}
		}
		return TRUE;

	case WM_HSCROLL:
		if (hBass != nullptr)
			if (LOWORD(wParam) == SB_ENDSCROLL || LOWORD(wParam) == SB_THUMBTRACK) {
				Volume = SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
				BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
				SendMessage(hwndSlider, TBM_SETPOS, TRUE, Volume);
				Preview = TRUE;
				if (EnPreview)
					Skin_PlaySound("AlertMsg");
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			if (hBass != nullptr) {
				SYSTEMTIME systime = { 0 };

				GetDlgItemText(hwndDlg, IDC_OUTDEVICE, tmp, _countof(tmp));
				g_plugin.setWString(OPT_OUTDEVICE, tmp);

				Volume = (uint32_t)SendDlgItemMessage(hwndDlg, IDC_VOLUME, TBM_GETPOS, 0, 0);
				g_plugin.setByte(OPT_VOLUME, Volume);

				sndLimSnd = SendDlgItemMessage(hwndDlg, IDC_MAXCHANNEL, CB_GETCURSEL, 0, 0) + 1;
				if (sndLimSnd > MAXCHAN)
					sndLimSnd = MAXCHAN;
				g_plugin.setByte(OPT_MAXCHAN, sndLimSnd);

				QuietTime = IsDlgButtonChecked(hwndDlg, IDC_QUIETTIME) == BST_CHECKED;
				g_plugin.setByte(OPT_QUIETTIME, QuietTime);

				SendDlgItemMessage(hwndDlg, IDC_TIME1, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
				TimeWrd1 = MAKEWORD(systime.wMinute, systime.wHour);
				g_plugin.setWord(OPT_TIME1, TimeWrd1);

				SendDlgItemMessage(hwndDlg, IDC_TIME2, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
				TimeWrd2 = MAKEWORD(systime.wMinute, systime.wHour);
				g_plugin.setWord(OPT_TIME2, TimeWrd2);

				EnPreview = IsDlgButtonChecked(hwndDlg, IDC_PREVIEW) == BST_CHECKED;
				g_plugin.setByte(OPT_PREVIEW, EnPreview);

				StatMask = 0;
				for (int i = IDC_CHECKBOX8; i >= IDC_CHECKBOX1; i--) {
					StatMask <<= 1;
					if (IsDlgButtonChecked(hwndDlg, i) == BST_CHECKED)
						StatMask |= 1;
				}

				g_plugin.setWord(OPT_STATUS, StatMask);

				device = SendDlgItemMessage(hwndDlg, IDC_OUTDEVICE, CB_GETCURSEL, 0, 0);
				if (device == 0)
					device = -1;
				else
					device += newBass;
			}
			return 1;

		case PSN_RESET:
			if (hBass != nullptr)
				BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
			return 1;

		case DTN_DATETIMECHANGE:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return 1;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_QUIETTIME:
			{
				BOOL b = IsDlgButtonChecked(hwndDlg, IDC_QUIETTIME) == BST_CHECKED;
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIME1), b);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIME2), b);
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
		case IDC_PREVIEW:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_GETBASS:
			Utils_OpenUrl("https://www.un4seen.com/");
			break;
		}
		break;
	}
	return 0;
}

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.a = MODULENAME;
	odp.pfnDlgProc = OptionsProc;
	odp.szGroup.a = LPGEN("Sounds");
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Frame

static void EnableFrameIcon(bool bEnable)
{
	if (bEnable) {
		SendMessage(hwndMute, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("BASSSoundOn"));
		SendMessage(hwndMute, BUTTONADDTOOLTIP, (WPARAM)Translate("Disable sounds"), 0);
	}
	else {
		SendMessage(hwndMute, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("BASSSoundOff"));
		SendMessage(hwndMute, BUTTONADDTOOLTIP, (WPARAM)Translate("Enable sounds"), 0);
	}
}

static void fnPainter(MButtonCtrl *btn, HDC hdc)
{
	DrawIconEx(hdc, 0, 0, btn->hIcon, 16, 16, 0, hBkgBrush, DI_NORMAL);
}

static LRESULT CALLBACK SliderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		return TRUE;
	}
	return mir_callNextSubclass(hwnd, SliderWndProc, msg, wParam, lParam);
}

static LRESULT CALLBACK FrameWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_CREATE:
		hwndMute = CreateWindow(MIRANDABUTTONCLASS, L"", WS_CHILD | WS_VISIBLE, 1, 1, 16, 16, hwnd,
			nullptr, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), nullptr);
		SendMessage(hwndMute, BUTTONSETASFLATBTN, 1, 0);
		SendMessage(hwndMute, BUTTONSETCUSTOMPAINT, 0, (LPARAM)&fnPainter);

		EnableFrameIcon(db_get_b(0, "Skin", "UseSound", 0) != 0);

		hwndSlider = CreateWindow(TRACKBAR_CLASS, L"", WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_TOOLTIPS, 21, 1, 100, 20,
			hwnd, (HMENU)nullptr, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), nullptr);
		SendMessage(hwndSlider, TBM_SETRANGE, FALSE, MAKELONG(SLIDER_MIN, SLIDER_MAX));
		SendMessage(hwndSlider, TBM_SETPOS, TRUE, Volume);
		mir_subclassWindow(hwndSlider, SliderWndProc);
		break;

	case WM_COMMAND:
		if ((HWND)lParam == hwndMute) {
			bool useSound = !db_get_b(0, "Skin", "UseSound", 0);
			db_set_b(0, "Skin", "UseSound", useSound);
			EnableFrameIcon(useSound);
		}
		break;

	case WM_HSCROLL:
		if (hBass != nullptr)
			if (LOWORD(wParam) == SB_ENDSCROLL || LOWORD(wParam) == SB_THUMBTRACK) {
				Volume = (uint32_t)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
				g_plugin.setByte(OPT_VOLUME, Volume);
				BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
				SendMessage(hwndOptSlider, TBM_SETPOS, TRUE, Volume);
				Preview = TRUE;
				if (EnPreview)
					Skin_PlaySound("AlertMsg");
			}
		break;

	case WM_SIZE:
		GetClientRect(hwnd, &rc);
		if (hwndMute)
			MoveWindow(hwndMute, rc.right - 20, 2, 16, 16, FALSE);
		SetWindowPos(hwndSlider, nullptr, 1, rc.top + 1 + (20 - 18) / 2, rc.right - rc.left - 1 - 20, 18, SWP_NOZORDER);
		InvalidateRect(hwnd, &rc, FALSE);
		return 0;

	case WM_ERASEBKGND:
		GetClientRect(hwnd, &rc);
		FillRect((HDC)wParam, &rc, hBkgBrush);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == hwndSlider) {
			HDC dc = (HDC)wParam;
			SetBkColor(dc, clBack);
			return (INT_PTR)hBkgBrush;
		}
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return TRUE;
}

int ReloadColors(WPARAM, LPARAM)
{
	clBack = Colour_GetW(_A2W(MODULENAME), LPGENW("Frame background"));

	if (hBkgBrush)
		DeleteObject(hBkgBrush);
	hBkgBrush = CreateSolidBrush(clBack);
	HWND hwnd = GetFocus();
	InvalidateRect(hwndSlider, nullptr, TRUE);
	SetFocus(hwndSlider);
	RedrawWindow(hwnd_plugin, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
	SetFocus(hwnd);
	return 0;
}

void CreateFrame()
{
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		return;

	WNDCLASS wndclass = { 0 };
	wndclass.lpfnWndProc = FrameWindowProc;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.lpszClassName = L"BassInterfaceFrame";
	RegisterClass(&wndclass);

	hwnd_plugin = CreateWindow(L"BassInterfaceFrame", TranslateT("BASS Interface"),
		WS_CHILD | WS_CLIPCHILDREN, 0, 0, 10, 10, g_clistApi.hwndContactList, nullptr, g_plugin.getInst(), nullptr);

	CLISTFrame Frame = { sizeof(CLISTFrame) };
	Frame.szName.a = LPGEN("BASS Interface");
	Frame.hWnd = hwnd_plugin;
	Frame.align = alBottom;
	Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP;
	Frame.height = 22;
	Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	frame_id = g_plugin.addFrame(&Frame);

	ColourIDW colourid = {};
	strcpy_s(colourid.dbSettingsGroup, MODULENAME);
	strcpy_s(colourid.setting, "ColorFrame");
	wcscpy_s(colourid.name, LPGENW("Frame background"));
	wcscpy_s(colourid.group, _A2W(MODULENAME));

	colourid.defcolour = GetSysColor(COLOR_3DFACE);
	g_plugin.addColor(&colourid);

	HookEvent(ME_COLOUR_RELOAD, ReloadColors);
	ReloadColors(0, 0);
}

void DeleteFrame()
{
	if (hBkgBrush)
		DeleteObject(hBkgBrush);

	CallService(MS_CLIST_FRAMES_REMOVEFRAME, frame_id, 0);
}

void LoadBassLibrary(const wchar_t *ptszPath)
{
	hBass = LoadLibrary(ptszPath);

	if (hBass != nullptr) 
	{
		newBass = (BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, TRUE) != 0); // will use new "Default" device

		DBVARIANT dbv = { 0 };

		BASS_DEVICEINFO info;
		if (!g_plugin.getWString(OPT_OUTDEVICE, &dbv))
			for (size_t i = 1; BASS_GetDeviceInfo((uint32_t)i, &info); i++)
				if (!mir_wstrcmp(dbv.pwszVal, _A2T(info.name)))
					device = (int)i;

		db_free(&dbv);

		sndLimSnd = g_plugin.getByte(OPT_MAXCHAN, MAXCHAN);
		if (sndLimSnd > MAXCHAN)
			sndLimSnd = MAXCHAN;

		TimeWrd1 = g_plugin.getWord(OPT_TIME1, 0);
		TimeWrd2 = g_plugin.getWord(OPT_TIME2, 0);
		QuietTime = g_plugin.getByte(OPT_QUIETTIME, 0);
		EnPreview = g_plugin.getByte(OPT_PREVIEW, 0);
		StatMask = g_plugin.getWord(OPT_STATUS, 0x3ff);

		BASS_Init(device, 44100, 0, g_clistApi.hwndContactList, nullptr);

		Volume = g_plugin.getByte(OPT_VOLUME, 33);
		BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, Volume * 100);
		hPlaySound = HookEvent(ME_SKIN_PLAYINGSOUND, OnPlaySnd);
		CreateFrame();
	}
}

int OnFoldersChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hBASSFolder, CurrBassPath, MAX_PATH, L"");
	mir_wstrcat(CurrBassPath, L"\\bass.dll");

	if (hBass != nullptr) {
		BASS_Free();
		FreeLibrary(hBass);
		hBass = nullptr;
		UnhookEvent(hPlaySound);
		DeleteFrame();
	}

	LoadBassLibrary(CurrBassPath);
	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	if (hBASSFolder = FoldersRegisterCustomPathW(LPGEN("Bass Interface"), LPGEN("Bass library"), PLUGINS_PATHW L"\\BASS")) {
		FoldersGetCustomPathW(hBASSFolder, CurrBassPath, MAX_PATH, L"");
		mir_wstrcat(CurrBassPath, L"\\bass.dll");
	}
	else {
		DBVARIANT dbv;
		if (g_plugin.getWString(OPT_BASSPATH, &dbv)) {
			mir_wstrncpy(CurrBassPath, VARSW(L"Plugins\\BASS\\bass.dll"), _countof(CurrBassPath));
			g_plugin.setWString(OPT_BASSPATH, CurrBassPath);
		}
		else {
			mir_wstrcpy(CurrBassPath, dbv.pwszVal);
			db_free(&dbv);
		}
	}

	LoadBassLibrary(CurrBassPath);

	HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);
	return 0;
}

int OnSettingChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0 || lParam == NULL)
		return 0;

	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(dbcws->szModule, "Skin")) {
		if (!strcmp(dbcws->szSetting, "UseSound")) {
			EnableFrameIcon(dbcws->value.bVal != 0);
			return 0;
		}
	}
	return 0;
}

int OnShutdown(WPARAM, LPARAM)
{
	if (hBass != nullptr) {
		BASS_Free();
		FreeLibrary(hBass);
	}

	DeleteFrame();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Sounds enabled"), "BASSSoundOn", IDI_BASSSoundOn },
	{ LPGEN("Sounds disabled"), "BASSSoundOff", IDI_BASSSoundOff }
};

int CMPlugin::Load()
{
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);

	g_plugin.registerIcon(MODULENAME, iconList);
	return 0;
}
