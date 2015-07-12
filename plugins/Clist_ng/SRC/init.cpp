/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: init.cpp 138 2010-11-01 10:51:15Z silvercircle $
 *
 */

#include <commonheaders.h>

HINSTANCE 			g_hInst 	=  0;
CLIST_INTERFACE		*pcli, coreCli;
int hLangpack;

#define DEFAULT_TB_VISIBILITY (1 | 2 | 4 | 8 | 16 | 32 | 64 | 8192)

wchar_t*			szNoevents = L"No events...";
extern HICON 		overlayicons[10];

extern int Docking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
extern void RegisterCLUIFrameClasses();

HMENU  BuildGroupPopupMenu(struct ClcGroup* group);
void   ReloadThemedOptions();
void   TrayIconUpdateBase(const char *szChangedProto);

void GetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour);
int  GetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);
int  ShowHide(WPARAM wParam, LPARAM lParam);

#define __MAJOR_VERSION          0
#define __MINOR_VERSION          9
#define __RELEASE_NUM            2
#define __BUILD_NUM              4

#include <stdver.h>

#define __PLUGIN_NAME            "Clist NG"
#define __FILENAME               "Clist_NG.dll"
#define __DESCRIPTION            "Displays contacts, event notifications, protocol status."
#define __AUTHOR                 "Pixel, egoDust, cyreve, Nightwish"
#define __AUTHOREMAIL            ""
#define __AUTHORWEB              "http://miranda-ng.org/p/Clist_nicer/"
#define __COPYRIGHT              "Copyright 2000-2015 Miranda-IM project"

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
	// {8F79B4EE-EB48-4A03-873E-27BE6B7E9A25}
	{ 0xe3a15605, 0x62be, 0x4a1b, { 0xa1, 0x10, 0xf8, 0x89, 0xfa, 0x4b, 0x2f, 0x0 } }
};

int _DebugTraceW(const wchar_t *fmt, ...)
{
	wchar_t 	debug[2048];
	int     	ibsize = 2047;
	SYSTEMTIME	st;
	va_list 	va;
	char		tszTime[50];
	va_start(va, fmt);

	GetLocalTime(&st);

	mir_snprintf(tszTime, 50, "%02d.%02d.%04d - %02d:%02d:%02d.%04d: ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);


	_vsnwprintf(debug, ibsize - 10, fmt, va);
//#ifdef _DEBUG
	OutputDebugStringW(debug);
//#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "clg_debug.log");
		f = fopen(szLogFileName, "a+");
		if(f) {
			char *szDebug = Utf8EncodeW(debug);
			fputs(tszTime, f);
			fputs(szDebug, f);
			fputs("\n", f);
			fclose(f);
			if(szDebug)
				mir_free(szDebug);
		}
	}
//#endif
	return 0;
}

int _DebugTraceA(const char *fmt, ...)
{
	char    debug[2048];
	int     ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	lstrcpyA(debug, "CLIST_NG: ");
	_vsnprintf(&debug[9], ibsize - 10, fmt, va);
#ifdef _DEBUG
	OutputDebugStringA(debug);
#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "clg_debugA.log");
		f = fopen(szLogFileName, "a+");
		if(f) {
			fputs(debug, f);
			fputs("\n", f);
			fclose(f);
		}
	}
#endif
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

int  LoadContactListModule(void);
int  LoadCLCModule(void);

static int systemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	GetSystemTime(&cfg::dat.st);
	SystemTimeToFileTime(&cfg::dat.st, &cfg::dat.ft);

	cfg::dat.bMetaEnabled = db_mc_isEnabled();

	cfg::dat.bAvatarServiceAvail = ServiceExists(MS_AV_GETAVATARBITMAP) ? TRUE : FALSE;
	if (cfg::dat.bAvatarServiceAvail)
		HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	cfg::dat.tabSRMM_Avail = ServiceExists("SRMsg_MOD/GetWindowFlags") ? TRUE : FALSE;

	ZeroMemory((void *)overlayicons, sizeof(HICON) * 10);

	CLN_LoadAllIcons(1);
	return 0;
}

int CLC::fnIconFromStatusMode(const char* szProto, int status, MCONTACT hContact)
{
	return IconFromStatusMode(szProto, status, hContact, NULL);
}

extern "C" int __declspec(dllexport) CListInitialise()
{
	int 		rc = 0;
	DBVARIANT 	dbv = {0};
	int       	i;
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	mir_getLP( &pluginInfo );
	mir_getCLI();
	coreCli = *pcli;

	if(!SUCCEEDED(Api::onInit())) {
		MessageBoxW(0, L"Unsupported operating system", L"Clist NG fatal", MB_OK);
		return(1);
	}
	if(!SUCCEEDED(cfg::onInit())) {
		MessageBoxW(0, L"cfg::OnInit() failed", L"Clist NG fatal", MB_OK);
		return(1);
	}
	Gfx::D2D_Init();
	LoadCLCButtonModule();
	RegisterCLUIFrameClasses();
	ZeroMemory((void*) &cfg::dat, sizeof(cfg::dat));

	int iCount = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

	iCount += 20;
	if(iCount < 300)
		iCount = 300;

	cfg::eCache = reinterpret_cast<TExtraCache *>(malloc(sizeof(TExtraCache) * iCount));
	ZeroMemory(cfg::eCache, sizeof(struct TExtraCache) * iCount);
	cfg::nextCacheEntry = 0;
	cfg::maxCacheEntry = iCount;
	cfg::initCache();

	cfg::dat.hMenuNotify = 				CreatePopupMenu();
	cfg::dat.wNextMenuID = 				1;
	cfg::dat.szNoEvents = 				TranslateTS(szNoevents);
	cfg::dat.avatarBorder =	(COLORREF)cfg::getDword("CLC", "avatarborder", 0);
	cfg::dat.hBrushAvatarBorder = 		CreateSolidBrush(cfg::dat.avatarBorder);
	cfg::dat.avatarSize = 				cfg::getWord("CList", "AvatarSize", 24);
	cfg::dat.dualRowMode = 				cfg::getByte("CLC", "DualRowMode", 0);
	cfg::dat.avatarPadding = 			cfg::getByte("CList", "AvatarPadding", 0);
	cfg::dat.isTransparent = 			cfg::getByte("CList", "Transparent", 0);
	cfg::dat.alpha = 					cfg::getByte("CList", "Alpha", SETTING_ALPHA_DEFAULT);
	cfg::dat.autoalpha = 				cfg::getByte("CList", "AutoAlpha", SETTING_ALPHA_DEFAULT);
	cfg::dat.fadeinout = 				cfg::getByte("CLUI", "FadeInOut", 0);
	cfg::dat.autosize = 				cfg::getByte("CLUI", "AutoSize", 0);
	cfg::dat.dwExtraImageMask = 		cfg::getDword("CLUI", "ximgmask", 0);
	cfg::dat.bNoOfflineAvatars = 		cfg::getByte("CList", "NoOfflineAV", 1);
	cfg::dat.bDblClkAvatars = 			cfg::getByte("CLC", "dblclkav", 0);
	cfg::dat.bEqualSections = 			cfg::getByte("CLUI", "EqualSections", 0);
	cfg::dat.bCenterStatusIcons = 		cfg::getByte("CLC", "si_centered", 1);
	cfg::dat.boldHideOffline = 			-1;
	cfg::dat.bNoTrayTips = 				cfg::getByte("CList", "NoTrayTips", 0);
	cfg::dat.bShowLocalTime = 			cfg::getByte("CLC", "ShowLocalTime", 1);
	cfg::dat.bShowLocalTimeSelective = 	cfg::getByte("CLC", "SelectiveLocalTime", 1);
	cfg::dat.bDontSeparateOffline = 	cfg::getByte("CList", "DontSeparateOffline", 0);
	cfg::dat.bFirstRun = 				cfg::getByte("CLUI", "firstrun", 1);
	cfg::dat.langPackCP =				Langpack_GetDefaultCodePage();
	cfg::dat.realTimeSaving = 			cfg::getByte("CLUI", "save_pos_always", 0);

	DWORD sortOrder = cfg::getDword("CList", "SortOrder", SORTBY_NAME);
	cfg::dat.sortOrder[0] = LOBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[1] = HIBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[2] = LOBYTE(HIWORD(sortOrder));

	if(cfg::dat.bFirstRun)
		cfg::writeByte("CLUI", "firstrun", 0);

	_tzset();

	if(!cfg::getString(NULL, "CLUI", "exIconOrder", &dbv)) {
		if(lstrlenA(dbv.pszVal) < EXICON_COUNT) {
			for(i = 1; i <= EXICON_COUNT; i++)
				cfg::dat.exIconOrder[i - 1] = i;
		} else {
			for(i = 0; i < EXICON_COUNT; i++)
				if(dbv.pszVal[i] < EXICON_COUNT + 1 && dbv.pszVal[i] > 0)
					cfg::dat.exIconOrder[i] = dbv.pszVal[i];
				else
					cfg::dat.exIconOrder[i] = i + 1;
		}
		db_free(&dbv);
	} else {
		for(i = 1; i <= EXICON_COUNT; i++)
			cfg::dat.exIconOrder[i - 1] = i;
	}
	ReloadThemedOptions();
	FLT_ReadOptions();

	CLUI::hExtraImages = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 30, 2);
	ImageList_SetIconSize(CLUI::hExtraImages, cfg::dat.exIconScale, cfg::dat.exIconScale);

	cfg::dat.dwFlags = cfg::getDword("CLUI", "Frameflags", CLUI_FRAME_STATUSICONS |
									 CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_BUTTONSFLAT | CLUI_FRAME_CLISTSUNKEN);
	cfg::dat.dwFlags |= (cfg::getByte("CLUI", "ShowSBar", 1) ? CLUI_FRAME_SBARSHOW : 0);
	cfg::dat.soundsOff = cfg::getByte("CLUI", "NoSounds", 0);

	if(cfg::getByte("Skin", "UseSound", 0) != cfg::dat.soundsOff)
		cfg::writeByte("Skin", "UseSound", (BYTE)(cfg::dat.soundsOff ? 0 : 1));

	// get the clist interface
	pcli = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);

	pcli->pfnBuildGroupPopupMenu = BuildGroupPopupMenu;
	pcli->pfnCluiProtocolStatusChanged = CluiProtocolStatusChanged;
	pcli->pfnCompareContacts = CLC::CompareContacts;
	pcli->pfnCreateClcContact = CLC::CreateClcContact;
	pcli->pfnCreateEvent = CLC::fnCreateEvent;
	pcli->pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
	pcli->pfnGetDefaultFontSetting = GetDefaultFontSetting;
	pcli->pfnGetRowBottomY = RowHeight::getItemBottomY;
	pcli->pfnGetRowHeight = RowHeight::getHeight;
	pcli->pfnGetRowTopY = RowHeight::getItemTopY;
	pcli->pfnGetRowTotalHeight = RowHeight::getTotalHeight;
	pcli->pfnGetWindowVisibleState = GetWindowVisibleState;
	pcli->pfnHitTest = CLC::HitTest;
	pcli->pfnLoadContactTree = CLC::LoadContactTree;
	pcli->pfnOnCreateClc = CLUI::loadModule;
	pcli->pfnPaintClc = CLC::Paint;
	pcli->pfnRebuildEntireList = CLC::RebuildEntireList;
	pcli->pfnRowHitTest = RowHeight::hitTest;
	pcli->pfnScrollTo = CLC::ScrollTo;
	pcli->pfnTrayIconUpdateBase = TrayIconUpdateBase;
	pcli->pfnSetHideOffline = CLC::SetHideOffline;
	pcli->pfnShowHide = ShowHide;
	pcli->pfnBeginRenameSelection = CLC::BeginRenameSelection;
	pcli->pfnAddContactToGroup = CLC::AddContactToGroup;

	pcli->pfnRemoveItemFromGroup = CLC::RemoveItemFromGroup;

	pcli->pfnAddEvent = CLC::AddEvent;

	pcli->pfnRemoveEvent = CLC::RemoveEvent;

	pcli->pfnAddGroup = CLC::AddGroup;

	pcli->pfnAddInfoItemToGroup = CLC::AddInfoItemToGroup;

	CLC::saveContactListControlWndProc = pcli->pfnContactListControlWndProc;
	pcli->pfnContactListControlWndProc = CLC::wndProc;

	CLUI::saveContactListWndProc = pcli->pfnContactListWndProc;
	pcli->pfnContactListWndProc = CLUI::wndProc;

	pcli->pfnIconFromStatusMode = CLC::fnIconFromStatusMode;

	pcli->pfnLoadClcOptions = CLC::LoadClcOptions;

	pcli->pfnProcessExternalMessages = CLC::ProcessExternalMessages;

	pcli->pfnRecalcScrollBar = CLC::RecalcScrollBar;

	pcli->pfnTrayIconProcessMessage = CLC::TrayIconProcessMessage;

	rc = LoadContactListModule();
	if(rc == 0)
		rc = CLC::loadModule();
	
	LoadCLUIFramesModule();
	HookEvent(ME_SYSTEM_MODULESLOADED, systemModulesLoaded);
	return rc;
}

// a plugin loader aware of CList exports will never call this.
extern "C" int __declspec(dllexport) Load()
{
	return 1;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if(IsWindow(pcli->hwndContactList))
		DestroyWindow(pcli->hwndContactList);

	ImageList_Destroy(CLUI::hExtraImages);
	CLC::shutDown(0, 0);
	UnLoadCLUIFramesModule();
	Skin::Unload();
	Api::onUnload();
	WarningDlg::destroyAll();
	DeleteCriticalSection(&cfg::cachecs);
	UnregisterClassW(L"CLCButtonClass", g_hInst);
	free(cfg::eCache);
	Gfx::shutDown();
	Gfx::D2D_Release();
	return 0;
}

