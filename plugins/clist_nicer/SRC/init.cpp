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
 * part of clist_nicer plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: init.cpp 13596 2011-04-15 19:07:23Z george.hazan $
 *
 */

#include <commonheaders.h>
#include "../cluiframes/cluiframes.h"

HINSTANCE g_hInst = 0;
PLUGINLINK *pluginLink;
CLIST_INTERFACE* pcli = NULL;
int hLangpack;

#define DEFAULT_TB_VISIBILITY (1 | 2 | 4 | 8 | 16 | 32 | 64 | 8192)
TCHAR *szNoevents = _T("No events...");
//extern HICON im_clienthIcons[NR_CLIENTS];
extern HICON overlayicons[10];

extern int Docking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
extern int SetHideOffline(WPARAM wParam, LPARAM lParam);

extern DWORD g_gdiplusToken;
extern HIMAGELIST himlExtraImages;

struct LIST_INTERFACE li;
struct MM_INTERFACE mmi;
TIME_API tmi;

HMENU  BuildGroupPopupMenu( struct ClcGroup* group );
struct ClcContact* CreateClcContact( void );
struct CListEvent* fnCreateEvent( void );
void   ReloadThemedOptions();
void   TrayIconUpdateBase(const char *szChangedProto);
void    RegisterCLUIFrameClasses();

void GetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour);
int  GetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);
int  ShowHide(WPARAM wParam, LPARAM lParam);
int  ClcShutdown(WPARAM wParam, LPARAM lParam);

void ( *saveLoadClcOptions )(HWND hwnd,struct ClcData *dat);
void LoadClcOptions(HWND hwnd,struct ClcData *dat);

int ( *saveAddContactToGroup )(struct ClcData *dat, struct ClcGroup *group, HANDLE hContact);
int AddContactToGroup(struct ClcData *dat, struct ClcGroup *group, HANDLE hContact);

struct ClcGroup* ( *saveRemoveItemFromGroup )(HWND hwnd, struct ClcGroup *group, struct ClcContact *contact, int updateTotalCount);
struct ClcGroup* RemoveItemFromGroup(HWND hwnd, struct ClcGroup *group, struct ClcContact *contact, int updateTotalCount);

struct CListEvent* ( *saveAddEvent )(CLISTEVENT *cle);
struct CListEvent* AddEvent(CLISTEVENT *cle);

int ( *saveAddInfoItemToGroup )(struct ClcGroup *group, int flags, const TCHAR *pszText);
int AddInfoItemToGroup(struct ClcGroup *group, int flags, const TCHAR *pszText);

struct ClcGroup* ( *saveAddGroup )(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers);
struct ClcGroup* AddGroup(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers);

LRESULT ( CALLBACK *saveContactListWndProc )(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT ( CALLBACK *saveContactListControlWndProc )(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int ( *saveIconFromStatusMode )( const char *szProto, int status, HANDLE hContact );

LRESULT ( *saveProcessExternalMessages )(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT ProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

int ( *saveRemoveEvent )(HANDLE hContact, HANDLE hDbEvent);
int RemoveEvent(HANDLE hContact, HANDLE hDbEvent);

INT_PTR ( *saveTrayIconProcessMessage )(WPARAM wParam, LPARAM lParam);
INT_PTR TrayIconProcessMessage(WPARAM wParam, LPARAM lParam);

void ( *saveRecalcScrollBar )(HWND hwnd, struct ClcData *dat);
void RecalcScrollBar(HWND hwnd, struct ClcData *dat);

PLUGININFOEX pluginInfo = {
#if defined(_UNICODE)
		sizeof(PLUGININFOEX), "CList Nicer+ (Unicode)", PLUGIN_MAKE_VERSION(0, 9, 2, 1),
#else
		sizeof(PLUGININFOEX), "CList Nicer+", PLUGIN_MAKE_VERSION(0, 9, 2, 1),
#endif
		"Display contacts, event notifications, protocol status",
		"Pixel, egoDust, cyreve, Nightwish", "", "Copyright 2000-2010 Miranda-IM project", "http://www.miranda-im.org",
		UNICODE_AWARE,
		DEFMOD_CLISTALL,
#if defined(_UNICODE)
		{0x8f79b4ee, 0xeb48, 0x4a03, { 0x87, 0x3e, 0x27, 0xbe, 0x6b, 0x7e, 0x9a, 0x25 }} //{8F79B4EE-EB48-4a03-873E-27BE6B7E9A25}
#else
		{0x5a070cec, 0xb2ab, 0x4bbe, { 0x8e, 0x48, 0x9c, 0x8d, 0xcd, 0xda, 0x14, 0xc3 }} //{5A070CEC-B2AB-4bbe-8E48-9C8DCDDA14C3}
#endif
};

#if defined(_UNICODE)
void _DebugTraceW(const wchar_t *fmt, ...)
{
#ifdef _DEBUG
    wchar_t debug[2048];
    int     ibsize = 2047;
    va_list va;
    va_start(va, fmt);

	lstrcpyW(debug, L"CLN: ");

    _vsnwprintf(&debug[5], ibsize - 10, fmt, va);
    OutputDebugStringW(debug);
#endif
}
#endif

void _DebugTraceA(const char *fmt, ...)
{
    char    debug[2048];
    int     ibsize = 2047;
    va_list va;
    va_start(va, fmt);

	lstrcpyA(debug, "CLN: ");
	_vsnprintf(&debug[5], ibsize - 10, fmt, va);
#ifdef _DEBUG
    OutputDebugStringA(debug);
#else
    {
        char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
        FILE *f;

        CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
        mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "clist_nicer.log");
        f = fopen(szLogFileName, "a+");
        if(f) {
            fputs(debug, f);
            fputs("\n", f);
            fclose(f);
        }
    }
#endif
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 9, 0, 0)) {
		MessageBox(0, _T("The installed version of the Clist Nicer+ plugin is not compatible with this Miranda installation. It requires Miranda core 0.9.0.0 or later"), _T("Clist Nicer+ error"), MB_OK);
		return NULL;
	}
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_CLIST, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

int  LoadContactListModule(void);
int  LoadCLCModule(void);
void LoadCLUIModule( void );

static int systemModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	GetSystemTime(&cfg::dat.st);
	SystemTimeToFileTime(&cfg::dat.st, &cfg::dat.ft);
	cfg::dat.bMetaAvail = ServiceExists(MS_MC_GETDEFAULTCONTACT) ? TRUE : FALSE;
	if(cfg::dat.bMetaAvail)
		mir_snprintf(cfg::dat.szMetaName, 256, "%s", (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0));
	else
		strncpy(cfg::dat.szMetaName, "MetaContacts", 255);

	if(ServiceExists(MS_MC_DISABLEHIDDENGROUP))
		CallService(MS_MC_DISABLEHIDDENGROUP, 1, 0);
	cfg::dat.bMetaEnabled = cfg::getByte(cfg::dat.szMetaName, "Enabled", 1);

	cfg::dat.bAvatarServiceAvail = ServiceExists(MS_AV_GETAVATARBITMAP) ? TRUE : FALSE;
	if(cfg::dat.bAvatarServiceAvail)
		HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);
	cfg::dat.tabSRMM_Avail = ServiceExists("SRMsg_MOD/GetWindowFlags") ? TRUE : FALSE;
	cfg::dat.IcoLib_Avail = ServiceExists(MS_SKIN2_ADDICON) ? TRUE : FALSE;

	ZeroMemory((void *)overlayicons, sizeof(HICON) * 10);

	CLN_LoadAllIcons(1);
	LoadExtBkSettingsFromDB();
	return 0;
}

static int fnIconFromStatusMode( const char* szProto, int status, HANDLE hContact )
{	return IconFromStatusMode( szProto, status, hContact, NULL );
}

extern "C" int __declspec(dllexport) CListInitialise(PLUGINLINK * link)
{
	int rc = 0;
	DBVARIANT dbv;
	int       i;
	char	  szProfilePath[MAX_PATH];

	pluginLink = link;
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getTMI(&tmi);
	mir_getLP( &pluginInfo );

	API::onInit();
	LoadCLCButtonModule();
	RegisterCLUIFrameClasses();

	ZeroMemory((void*) &cfg::dat, sizeof(cfg::dat));

	int iCount = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

	iCount += 20;
	if( iCount < 300 )
		iCount = 300;

	cfg::eCache = reinterpret_cast<TExtraCache *>(malloc(sizeof(TExtraCache) * iCount));
	ZeroMemory(cfg::eCache, sizeof(struct TExtraCache) * iCount);
	cfg::nextCacheEntry = 0;
	cfg::maxCacheEntry = iCount;
	cfg::init();

	cfg::dat.toolbarVisibility = 		cfg::getDword("CLUI", "TBVisibility", DEFAULT_TB_VISIBILITY);
	cfg::dat.hMenuButtons = 			GetSubMenu(LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 3);
	cfg::dat.hMenuNotify = 				CreatePopupMenu();
	cfg::dat.wNextMenuID = 				1;
	cfg::dat.sortTimer = 				cfg::getDword("CLC", "SortTimer", 150);
	cfg::dat.szNoEvents = 				TranslateTS(szNoevents);
	cfg::dat.avatarBorder = 			(COLORREF)cfg::getDword("CLC", "avatarborder", 0);
	cfg::dat.avatarRadius = 			(COLORREF)cfg::getDword("CLC", "avatarradius", 4);
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
	cfg::dat.bFullTransparent = 		cfg::getByte("CLUI", "fulltransparent", 0);
	cfg::dat.bDblClkAvatars = 			cfg::getByte("CLC", "dblclkav", 0);
	cfg::dat.bEqualSections = 			cfg::getByte("CLUI", "EqualSections", 0);
	cfg::dat.bCenterStatusIcons = 		cfg::getByte("CLC", "si_centered", 1);
	cfg::dat.boldHideOffline = 			-1;
	cfg::dat.bSecIMAvail = 				ServiceExists("SecureIM/IsContactSecured") ? 1 : 0;
	cfg::dat.bNoTrayTips = 				cfg::getByte("CList", "NoTrayTips", 0);
	cfg::dat.bShowLocalTime = 			cfg::getByte("CLC", "ShowLocalTime", 1);
	cfg::dat.bShowLocalTimeSelective = 	cfg::getByte("CLC", "SelectiveLocalTime", 1);
	cfg::dat.bDontSeparateOffline = 	cfg::getByte("CList", "DontSeparateOffline", 0);
	cfg::dat.bShowXStatusOnSbar = 		cfg::getByte("CLUI", "xstatus_sbar", 0);
	cfg::dat.bLayeredHack = 			cfg::getByte("CLUI", "layeredhack", 1);
	cfg::dat.bFirstRun = 				cfg::getByte("CLUI", "firstrun", 1);
	cfg::dat.langPackCP = 				CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	cfg::dat.realTimeSaving = 			cfg::getByte("CLUI", "save_pos_always", 0);

	DWORD sortOrder = cfg::getDword("CList", "SortOrder", SORTBY_NAME);
	cfg::dat.sortOrder[0] = LOBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[1] = HIBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[2] = LOBYTE(HIWORD(sortOrder));

	if(cfg::dat.bFirstRun)
		cfg::writeByte("CLUI", "firstrun", 0);

	if(!cfg::getString(NULL, "CLUI", "exIconOrder", &dbv)) {
		if(lstrlenA(dbv.pszVal) < EXICON_COUNT) {
			for(i = 1; i <= EXICON_COUNT; i++)
				cfg::dat.exIconOrder[i - 1] = i;
		} else {
			for(i = 0; i < EXICON_COUNT; i++)
				if(dbv.pszVal[i] < EXICON_COUNT+1 && dbv.pszVal[i] >0)
					cfg::dat.exIconOrder[i] = dbv.pszVal[i];
				else
					cfg::dat.exIconOrder[i] = i+1;
		}
		DBFreeVariant(&dbv);
	} else {
		for(i = 1; i <= EXICON_COUNT; i++)
			cfg::dat.exIconOrder[i - 1] = i;
	}
	ReloadThemedOptions();
	FLT_ReadOptions();
	Reload3dBevelColors();
	himlExtraImages = ImageList_Create(16, 16, ILC_MASK | (IsWinVerXPPlus() ? ILC_COLOR32 : ILC_COLOR16), 30, 2);
	ImageList_SetIconSize(himlExtraImages, cfg::dat.exIconScale, cfg::dat.exIconScale);

	cfg::dat.dwFlags = cfg::getDword("CLUI", "Frameflags", CLUI_FRAME_SHOWTOPBUTTONS | CLUI_FRAME_STATUSICONS |
                                                  CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_BUTTONSFLAT | CLUI_FRAME_CLISTSUNKEN);
	cfg::dat.dwFlags |= (cfg::getByte("CLUI", "ShowSBar", 1) ? CLUI_FRAME_SBARSHOW : 0);
	cfg::dat.soundsOff = cfg::getByte("CLUI", "NoSounds", 0);

	CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szProfilePath);

#if defined(_UNICODE)
	MultiByteToWideChar(CP_ACP, 0, szProfilePath, MAX_PATH, cfg::dat.tszProfilePath, MAX_PATH);
	cfg::dat.tszProfilePath[MAX_PATH - 1] = 0;
#else
	mir_sntprintf(cfg::dat.tszProfilePath, MAX_PATH, "%s", szProfilePath);
#endif

	_tcslwr(cfg::dat.tszProfilePath);

	if(cfg::getByte("Skin", "UseSound", 0) != cfg::dat.soundsOff)
		cfg::writeByte("Skin", "UseSound", (BYTE)(cfg::dat.soundsOff ? 0 : 1));

	// get the clist interface
	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)g_hInst);
	if ( (INT_PTR)pcli == CALLSERVICE_NOTFOUND ) {
LBL_Error:
		MessageBoxA( NULL, "This plugin requires Miranda IM 0.8.0.9 or later", "Fatal error", MB_OK );
		return 1;
	}
	if ( pcli->version < 6 ) // don't join it with the previous if()
		goto LBL_Error;

	pcli->pfnBuildGroupPopupMenu = BuildGroupPopupMenu;
	pcli->pfnCluiProtocolStatusChanged = CluiProtocolStatusChanged;
	pcli->pfnCompareContacts = CompareContacts;
	pcli->pfnCreateClcContact = CreateClcContact;
	pcli->pfnCreateEvent = fnCreateEvent;
	pcli->pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
	pcli->pfnGetDefaultFontSetting = GetDefaultFontSetting;
	pcli->pfnGetRowBottomY = RowHeight::getItemBottomY;
	pcli->pfnGetRowHeight = RowHeight::getHeight;
	pcli->pfnGetRowTopY = RowHeight::getItemTopY;
	pcli->pfnGetRowTotalHeight = RowHeight::getTotalHeight;
	pcli->pfnGetWindowVisibleState = GetWindowVisibleState;
	pcli->pfnHitTest = HitTest;
	pcli->pfnLoadContactTree = LoadContactTree;
	pcli->pfnOnCreateClc = LoadCLUIModule;
	pcli->pfnPaintClc = PaintClc;
	pcli->pfnRebuildEntireList = RebuildEntireList;
	pcli->pfnRowHitTest = RowHeight::hitTest;
	pcli->pfnScrollTo = ScrollTo;
	pcli->pfnTrayIconUpdateBase = TrayIconUpdateBase;
	pcli->pfnSetHideOffline = SetHideOffline;
	pcli->pfnShowHide = ShowHide;

	saveAddContactToGroup = pcli->pfnAddContactToGroup; pcli->pfnAddContactToGroup = AddContactToGroup;
	saveRemoveItemFromGroup = pcli->pfnRemoveItemFromGroup; pcli->pfnRemoveItemFromGroup = RemoveItemFromGroup;

	saveAddEvent = pcli->pfnAddEvent; pcli->pfnAddEvent = AddEvent;
	saveRemoveEvent = pcli->pfnRemoveEvent; pcli->pfnRemoveEvent = RemoveEvent;

	saveAddGroup = pcli->pfnAddGroup; pcli->pfnAddGroup = AddGroup;
	saveAddInfoItemToGroup = pcli->pfnAddInfoItemToGroup; pcli->pfnAddInfoItemToGroup = AddInfoItemToGroup;
	saveContactListControlWndProc = pcli->pfnContactListControlWndProc; pcli->pfnContactListControlWndProc = ContactListControlWndProc;
	saveContactListWndProc = pcli->pfnContactListWndProc; pcli->pfnContactListWndProc = ContactListWndProc;
	saveIconFromStatusMode = pcli->pfnIconFromStatusMode; pcli->pfnIconFromStatusMode = fnIconFromStatusMode;
	saveLoadClcOptions = pcli->pfnLoadClcOptions; pcli->pfnLoadClcOptions = LoadClcOptions;
	saveProcessExternalMessages = pcli->pfnProcessExternalMessages; pcli->pfnProcessExternalMessages = ProcessExternalMessages;
	saveRecalcScrollBar = pcli->pfnRecalcScrollBar; pcli->pfnRecalcScrollBar = RecalcScrollBar;
	saveTrayIconProcessMessage = pcli->pfnTrayIconProcessMessage; pcli->pfnTrayIconProcessMessage = TrayIconProcessMessage;

	rc = LoadContactListModule();
	if (rc == 0)
		rc = LoadCLCModule();
	HookEvent(ME_SYSTEM_MODULESLOADED, systemModulesLoaded);
	return rc;
}

// a plugin loader aware of CList exports will never call this.
extern "C" int __declspec(dllexport) Load(PLUGINLINK * link)
{
	return 1;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (IsWindow(pcli->hwndContactList))
		DestroyWindow(pcli->hwndContactList);
	ImageList_Destroy(himlExtraImages);
	ClcShutdown(0, 0);
	UnLoadCLUIFramesModule();
	return 0;
}

