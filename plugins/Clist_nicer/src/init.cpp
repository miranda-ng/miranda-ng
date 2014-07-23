/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <commonheaders.h>
#include "../cluiframes/cluiframes.h"

HINSTANCE g_hInst = 0;
CLIST_INTERFACE *pcli = NULL;
int hLangpack;

#define DEFAULT_TB_VISIBILITY (1 | 2 | 4 | 8 | 16 | 32 | 64 | 8192)

extern HICON overlayicons[10];

extern int Docking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
extern int SetHideOffline(WPARAM wParam, LPARAM lParam);

extern DWORD g_gdiplusToken;

TIME_API tmi;

HMENU BuildGroupPopupMenu(ClcGroup *group);
ClcContact *CreateClcContact(void);
CListEvent *fnCreateEvent(void);
void ReloadThemedOptions();
int TrayCalcChanged(const char *szChangedProto, int averageMode, int iProtoCount);
void RegisterCLUIFrameClasses();
void LoadButtonModule();

void GetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour);
int GetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);
int ShowHide(WPARAM wParam, LPARAM lParam);
int ClcShutdown(WPARAM wParam, LPARAM lParam);

void (*saveLoadClcOptions)(HWND hwnd, struct ClcData *dat);
void LoadClcOptions(HWND hwnd, struct ClcData *dat);

int (*saveAddContactToGroup)(struct ClcData *dat, ClcGroup *group, MCONTACT hContact);
int AddContactToGroup(struct ClcData *dat, ClcGroup *group, MCONTACT hContact);

CListEvent *(*saveAddEvent)(CLISTEVENT *cle);
CListEvent *AddEvent(CLISTEVENT *cle);

int (*saveAddInfoItemToGroup)(ClcGroup *group, int flags, const TCHAR *pszText);
int AddInfoItemToGroup(ClcGroup *group, int flags, const TCHAR *pszText);

ClcGroup *(*saveAddGroup)(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers);
ClcGroup *AddGroup(HWND hwnd, struct ClcData *dat, const TCHAR *szName, DWORD flags, int groupId, int calcTotalMembers);

LRESULT (CALLBACK *saveContactListWndProc)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT (CALLBACK *saveContactListControlWndProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int (*saveIconFromStatusMode)(const char *szProto, int status, MCONTACT hContact);

LRESULT (*saveProcessExternalMessages)(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT ProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);

int (*saveRemoveEvent)(MCONTACT hContact, HANDLE hDbEvent);
int RemoveEvent(MCONTACT hContact, HANDLE hDbEvent);

INT_PTR (*saveTrayIconProcessMessage)(WPARAM wParam, LPARAM lParam);
INT_PTR TrayIconProcessMessage(WPARAM wParam, LPARAM lParam);

void (*saveRecalcScrollBar)(HWND hwnd, struct ClcData *dat);
void RecalcScrollBar(HWND hwnd, struct ClcData *dat);

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
	{0x8f79b4ee, 0xeb48, 0x4a03, {0x87, 0x3e, 0x27, 0xbe, 0x6b, 0x7e, 0x9a, 0x25}}
};


void _DebugTraceW(const wchar_t *fmt, ...)
{
#ifdef _DEBUG
	wchar_t debug[2048];
	int ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	lstrcpyW(debug, L"CLN: ");

	mir_vsnwprintf(&debug[5], ibsize - 10, fmt, va);
	OutputDebugStringW(debug);
#endif
}


void _DebugTraceA(const char *fmt, ...)
{
	char debug[2048];
	int ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	lstrcpyA(debug, "CLN: ");
	mir_vsnprintf(&debug[5], ibsize - 10, fmt, va);
#ifdef _DEBUG
	OutputDebugStringA(debug);
#else
	{
		char szLogFileName[MAX_PATH], szDataPath[MAX_PATH];
		FILE *f;

		CallService(MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szDataPath);
		mir_snprintf(szLogFileName, MAX_PATH, "%s\\%s", szDataPath, "clist_nicer.log");
		f = fopen(szLogFileName, "a+");
		if (f) {
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

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

int PreloadContactListModule(void);
int LoadContactListModule(void);
int LoadCLCModule(void);
void LoadCLUIModule(void);
void OnCreateClc(void);

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

static int fnIconFromStatusMode(const char *szProto, int status, MCONTACT hContact)
{
	return IconFromStatusMode(szProto, status, hContact, NULL);
}

extern "C" int __declspec(dllexport) CListInitialise()
{
	mir_getLP( &pluginInfo );
	mir_getCLI();
	mir_getTMI(&tmi);

	API::onInit();
	RegisterCLUIFrameClasses();

	ZeroMemory((void *)&cfg::dat, sizeof(cfg::dat));

	int iCount = CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

	iCount += 20;
	if (iCount < 300)
		iCount = 300;

	cfg::init();

	cfg::dat.hMenuNotify = CreatePopupMenu();
	cfg::dat.wNextMenuID = 1;
	cfg::dat.sortTimer = cfg::getDword("CLC", "SortTimer", 150);
	cfg::dat.avatarBorder = (COLORREF)cfg::getDword("CLC", "avatarborder", 0);
	cfg::dat.avatarRadius = (COLORREF)cfg::getDword("CLC", "avatarradius", 4);
	cfg::dat.hBrushAvatarBorder = CreateSolidBrush(cfg::dat.avatarBorder);
	cfg::dat.avatarSize = cfg::getWord("CList", "AvatarSize", 24);
	cfg::dat.dualRowMode = cfg::getByte("CLC", "DualRowMode", 0);
	cfg::dat.avatarPadding = cfg::getByte("CList", "AvatarPadding", 0);
	cfg::dat.isTransparent = cfg::getByte("CList", "Transparent", 0);
	cfg::dat.alpha = cfg::getByte("CList", "Alpha", SETTING_ALPHA_DEFAULT);
	cfg::dat.autoalpha = cfg::getByte("CList", "AutoAlpha", SETTING_ALPHA_DEFAULT);
	cfg::dat.fadeinout = cfg::getByte("CLUI", "FadeInOut", 0);
	cfg::dat.autosize = cfg::getByte("CLUI", "AutoSize", 0);
	cfg::dat.bNoOfflineAvatars = cfg::getByte("CList", "NoOfflineAV", 1);
	cfg::dat.bFullTransparent = cfg::getByte("CLUI", "fulltransparent", 0);
	cfg::dat.bDblClkAvatars = cfg::getByte("CLC", "dblclkav", 0);
	cfg::dat.bEqualSections = cfg::getByte("CLUI", "EqualSections", 0);
	cfg::dat.bCenterStatusIcons = cfg::getByte("CLC", "si_centered", 1);
	cfg::dat.boldHideOffline = -1;
	cfg::dat.bSecIMAvail = ServiceExists("SecureIM/IsContactSecured") ? 1 : 0;
	cfg::dat.bNoTrayTips = cfg::getByte("CList", "NoTrayTips", 0);
	cfg::dat.bShowLocalTime = cfg::getByte("CLC", "ShowLocalTime", 1);
	cfg::dat.bShowLocalTimeSelective = cfg::getByte("CLC", "SelectiveLocalTime", 1);
	cfg::dat.bDontSeparateOffline = cfg::getByte("CList", "DontSeparateOffline", 0);
	cfg::dat.bShowXStatusOnSbar = cfg::getByte("CLUI", "xstatus_sbar", 0);
	cfg::dat.bLayeredHack = cfg::getByte("CLUI", "layeredhack", 1);
	cfg::dat.bFirstRun = cfg::getByte("CLUI", "firstrun", 1);
	cfg::dat.langPackCP = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	cfg::dat.realTimeSaving = cfg::getByte("CLUI", "save_pos_always", 0);

	DWORD sortOrder = cfg::getDword("CList", "SortOrder", SORTBY_NAME);
	cfg::dat.sortOrder[0] = LOBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[1] = HIBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[2] = LOBYTE(HIWORD(sortOrder));

	if (cfg::dat.bFirstRun)
		cfg::writeByte("CLUI", "firstrun", 0);

	ReloadThemedOptions();
	Reload3dBevelColors();

	cfg::dat.dwFlags = cfg::getDword("CLUI", "Frameflags", CLUI_FRAME_STATUSICONS | CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_BUTTONSFLAT | CLUI_FRAME_CLISTSUNKEN);
	cfg::dat.dwFlags |= (cfg::getByte("CLUI", "ShowSBar", 1) ? CLUI_FRAME_SBARSHOW : 0);
	cfg::dat.soundsOff = cfg::getByte("Skin", "UseSound", 1) ? 0 : 1;

	CallService(MS_DB_GETPROFILEPATHT, MAX_PATH, (LPARAM)cfg::dat.tszProfilePath);
	_tcslwr(cfg::dat.tszProfilePath);

	PreloadContactListModule();

	// get the clist interface
	pcli->hInst = g_hInst;
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
	pcli->pfnOnCreateClc = OnCreateClc;
	pcli->pfnPaintClc = PaintClc;
	pcli->pfnRebuildEntireList = RebuildEntireList;
	pcli->pfnRowHitTest = RowHeight::hitTest;
	pcli->pfnScrollTo = ScrollTo;
	pcli->pfnTrayCalcChanged = TrayCalcChanged;
	pcli->pfnSetHideOffline = SetHideOffline;
	pcli->pfnShowHide = ShowHide;

	saveAddContactToGroup = pcli->pfnAddContactToGroup; pcli->pfnAddContactToGroup = AddContactToGroup;

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

	int rc = LoadContactListModule();
	if (rc == 0)
		rc = LoadCLCModule();
	LoadCLUIModule();
	LoadButtonModule();

	HookEvent(ME_SYSTEM_MODULESLOADED, systemModulesLoaded);
	return rc;
}

// a plugin loader aware of CList exports will never call this.
extern "C" int __declspec(dllexport) Load(void)
{
	return 1;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (IsWindow(pcli->hwndContactList))
		DestroyWindow(pcli->hwndContactList);
	ClcShutdown(0, 0);
	UnLoadCLUIFramesModule();
	return 0;
}
