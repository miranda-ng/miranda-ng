/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

#include "stdafx.h"
#include "cluiframes.h"

#pragma comment(lib, "shlwapi.lib")

HINSTANCE g_hInst = 0;
CLIST_INTERFACE *pcli, coreCli;

int hLangpack;

#define DEFAULT_TB_VISIBILITY (1 | 2 | 4 | 8 | 16 | 32 | 64 | 8192)

extern HICON overlayicons[10];

int Docking_ProcessWindowMessage(WPARAM wParam, LPARAM lParam);
int SetHideOffline(int iValue);

ClcContact *CreateClcContact(void);
void ReloadThemedOptions();
int TrayCalcChanged(const char *szChangedProto, int averageMode, int iProtoCount);
void LoadButtonModule();

void GetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour);
int GetWindowVisibleState(HWND hWnd, int iStepX, int iStepY);
int ShowHide(void);
int ClcShutdown(WPARAM wParam, LPARAM lParam);

CListEvent* AddEvent(CLISTEVENT *cle);
ClcGroup*   AddGroup(HWND hwnd, struct ClcData *dat, const wchar_t *szName, DWORD flags, int groupId, int calcTotalMembers);

ClcContact* AddContactToGroup(struct ClcData *dat, ClcGroup *group, MCONTACT hContact);
ClcContact* AddInfoItemToGroup(ClcGroup *group, int flags, const wchar_t *pszText);
LRESULT     ProcessExternalMessages(HWND hwnd, struct ClcData *dat, UINT msg, WPARAM wParam, LPARAM lParam);
int         RemoveEvent(MCONTACT hContact, MEVENT hDbEvent);
INT_PTR     TrayIconProcessMessage(WPARAM wParam, LPARAM lParam);
void        RecalcScrollBar(HWND hwnd, struct ClcData *dat);

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
	{ 0x8f79b4ee, 0xeb48, 0x4a03, { 0x87, 0x3e, 0x27, 0xbe, 0x6b, 0x7e, 0x9a, 0x25 } }
};

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
	g_hInst = hInstDLL;
	DisableThreadLibraryCalls(g_hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

int LoadContactListModule(void);
int LoadCLCModule(void);
void LoadCLUIModule(void);
void OnCreateClc(void);

static int systemModulesLoaded(WPARAM, LPARAM)
{
	GetSystemTime(&cfg::dat.st);
	SystemTimeToFileTime(&cfg::dat.st, &cfg::dat.ft);

	cfg::dat.bMetaEnabled = db_mc_isEnabled();

	cfg::dat.bAvatarServiceAvail = ServiceExists(MS_AV_GETAVATARBITMAP) ? TRUE : FALSE;
	if (cfg::dat.bAvatarServiceAvail)
		HookEvent(ME_AV_AVATARCHANGED, AvatarChanged);

	memset(&overlayicons, 0, sizeof(overlayicons));

	CLN_LoadAllIcons(1);
	return 0;
}

static int fnIconFromStatusMode(const char *szProto, int status, MCONTACT hContact)
{
	return IconFromStatusMode(szProto, status, hContact, NULL);
}

extern "C" int __declspec(dllexport) CListInitialise()
{
	mir_getLP(&pluginInfo);

	pcli = Clist_GetInterface();
	coreCli = *pcli;

	API::onInit();

	memset(&cfg::dat, 0, sizeof(cfg::dat));

	int iCount = db_get_contact_count();

	iCount += 20;
	if (iCount < 300)
		iCount = 300;

	cfg::dat.hMenuNotify = CreatePopupMenu();
	cfg::dat.wNextMenuID = 1;
	cfg::dat.sortTimer = db_get_dw(NULL, "CLC", "SortTimer", 150);
	cfg::dat.avatarBorder = (COLORREF)db_get_dw(NULL, "CLC", "avatarborder", 0);
	cfg::dat.avatarRadius = (COLORREF)db_get_dw(NULL, "CLC", "avatarradius", 4);
	cfg::dat.hBrushAvatarBorder = CreateSolidBrush(cfg::dat.avatarBorder);
	cfg::dat.avatarSize = db_get_w(NULL, "CList", "AvatarSize", 24);
	cfg::dat.dualRowMode = db_get_b(NULL, "CLC", "DualRowMode", 0);
	cfg::dat.avatarPadding = db_get_b(NULL, "CList", "AvatarPadding", 0);
	cfg::dat.isTransparent = db_get_b(NULL, "CList", "Transparent", 0);
	cfg::dat.alpha = db_get_b(NULL, "CList", "Alpha", SETTING_ALPHA_DEFAULT);
	cfg::dat.autoalpha = db_get_b(NULL, "CList", "AutoAlpha", SETTING_ALPHA_DEFAULT);
	cfg::dat.fadeinout = db_get_b(NULL, "CLUI", "FadeInOut", 0);
	cfg::dat.autosize = db_get_b(NULL, "CLUI", "AutoSize", 0);
	cfg::dat.bNoOfflineAvatars = db_get_b(NULL, "CList", "NoOfflineAV", 1);
	cfg::dat.bFullTransparent = db_get_b(NULL, "CLUI", "fulltransparent", 0);
	cfg::dat.bDblClkAvatars = db_get_b(NULL, "CLC", "dblclkav", 0);
	cfg::dat.bEqualSections = db_get_b(NULL, "CLUI", "EqualSections", 0);
	cfg::dat.bCenterStatusIcons = db_get_b(NULL, "CLC", "si_centered", 1);
	cfg::dat.boldHideOffline = -1;
	cfg::dat.bSecIMAvail = ServiceExists("SecureIM/IsContactSecured") ? 1 : 0;
	cfg::dat.bNoTrayTips = db_get_b(NULL, "CList", "NoTrayTips", 0);
	cfg::dat.bShowLocalTime = db_get_b(NULL, "CLC", "ShowLocalTime", 1);
	cfg::dat.bShowLocalTimeSelective = db_get_b(NULL, "CLC", "SelectiveLocalTime", 1);
	cfg::dat.bDontSeparateOffline = db_get_b(NULL, "CList", "DontSeparateOffline", 0);
	cfg::dat.bShowXStatusOnSbar = db_get_b(NULL, "CLUI", "xstatus_sbar", 0);
	cfg::dat.bLayeredHack = db_get_b(NULL, "CLUI", "layeredhack", 1);
	cfg::dat.bFirstRun = db_get_b(NULL, "CLUI", "firstrun", 1);
	cfg::dat.langPackCP = Langpack_GetDefaultCodePage();
	cfg::dat.realTimeSaving = db_get_b(NULL, "CLUI", "save_pos_always", 0);

	DWORD sortOrder = db_get_dw(NULL, "CList", "SortOrder", SORTBY_NAME);
	cfg::dat.sortOrder[0] = LOBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[1] = HIBYTE(LOWORD(sortOrder));
	cfg::dat.sortOrder[2] = LOBYTE(HIWORD(sortOrder));

	if (cfg::dat.bFirstRun)
		db_set_b(NULL, "CLUI", "firstrun", 0);

	ReloadThemedOptions();
	Reload3dBevelColors();

	cfg::dat.dwFlags = db_get_dw(NULL, "CLUI", "Frameflags", CLUI_FRAME_STATUSICONS | CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_BUTTONSFLAT | CLUI_FRAME_CLISTSUNKEN);
	cfg::dat.dwFlags |= (db_get_b(NULL, "CLUI", "ShowSBar", 1) ? CLUI_FRAME_SBARSHOW : 0);
	cfg::dat.soundsOff = db_get_b(NULL, "Skin", "UseSound", 1) ? 0 : 1;

	Profile_GetPathW(MAX_PATH, cfg::dat.tszProfilePath);
	wcslwr(cfg::dat.tszProfilePath);

	// get the clist interface
	pcli->hInst = g_hInst;
	pcli->pfnCluiProtocolStatusChanged = CluiProtocolStatusChanged;
	pcli->pfnCompareContacts = CompareContacts;
	pcli->pfnCreateClcContact = CreateClcContact;
	pcli->pfnDocking_ProcessWindowMessage = Docking_ProcessWindowMessage;
	pcli->pfnGetContactHiddenStatus = CLVM_GetContactHiddenStatus;
	pcli->pfnGetDefaultFontSetting = GetDefaultFontSetting;
	pcli->pfnGetRowBottomY = RowHeight::getItemBottomY;
	pcli->pfnGetRowHeight = RowHeight::getHeight;
	pcli->pfnGetRowTopY = RowHeight::getItemTopY;
	pcli->pfnGetRowTotalHeight = RowHeight::getTotalHeight;
	pcli->pfnGetWindowVisibleState = GetWindowVisibleState;
	pcli->pfnHitTest = HitTest;
	pcli->pfnOnCreateClc = OnCreateClc;
	pcli->pfnPaintClc = PaintClc;
	pcli->pfnRebuildEntireList = RebuildEntireList;
	pcli->pfnRowHitTest = RowHeight::hitTest;
	pcli->pfnScrollTo = ScrollTo;
	pcli->pfnTrayCalcChanged = TrayCalcChanged;
	pcli->pfnSetHideOffline = SetHideOffline;
	pcli->pfnShowHide = ShowHide;

	pcli->pfnAddContactToGroup = AddContactToGroup;

	pcli->pfnAddEvent = AddEvent;
	pcli->pfnRemoveEvent = RemoveEvent;

	pcli->pfnAddGroup = AddGroup;
	pcli->pfnAddInfoItemToGroup = AddInfoItemToGroup;
	pcli->pfnContactListControlWndProc = ContactListControlWndProc;
	pcli->pfnContactListWndProc = ContactListWndProc;
	pcli->pfnIconFromStatusMode = fnIconFromStatusMode;
	pcli->pfnLoadClcOptions = LoadClcOptions;
	pcli->pfnProcessExternalMessages = ProcessExternalMessages;
	pcli->pfnRecalcScrollBar = RecalcScrollBar;
	pcli->pfnTrayIconProcessMessage = TrayIconProcessMessage;

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
