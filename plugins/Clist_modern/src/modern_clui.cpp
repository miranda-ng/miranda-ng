/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "modern_static_clui.h"
#include "modern_clcpaint.h"
#include "modern_sync.h"

struct PROTOTICKS
{
	~PROTOTICKS()
	{
		ImageList_Destroy(himlIconList);
	}

	ptrA szProto;
	int  nIconsCount;
	int  nCycleStartTick;
	int  nIndex;
	BOOL bTimerCreated;
	BOOL bGlobal;
	HIMAGELIST himlIconList;
};

static OBJLIST<PROTOTICKS> arTicks(1);

int ContactSettingChanged(WPARAM, LPARAM);
int MetaStatusChanged(WPARAM, LPARAM);

HRESULT(WINAPI *g_proc_DWMEnableBlurBehindWindow)(HWND hWnd, DWM_BLURBEHIND *pBlurBehind);
BOOL CALLBACK ProcessCLUIFrameInternalMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT &result);

// new sources
#include <crtdbg.h>

/* Global variables */

UINT    g_dwMainThreadID = 0;
HANDLE  g_hAwayMsgThread = nullptr, g_hGetTextAsyncThread = nullptr, g_hSmoothAnimationThread = nullptr;

bool    g_bSizing = false;
bool    g_bTrimText = true;
bool    g_bChangingMode = false;
bool    g_bTransparentFlag = false;
bool    g_bOnModulesLoadedCalled = false;

RECT    g_rcEdgeSizingRect = { 0 };

/* Module global variables */

static uint8_t bAlphaEnd;
static int bOldHideOffline;
static int bOldUseGroups;

static uint16_t wBehindEdgeShowDelay,
wBehindEdgeHideDelay,
wBehindEdgeBorderSize;

static BOOL mutex_bAnimationInProgress = FALSE,
mutex_bShowHideCalledFromAnimation = FALSE,
mutex_bIgnoreActivation = FALSE,
mutex_bDisableAutoUpdate = TRUE,
mutex_bDuringSizing = FALSE,
mutex_bDelayedSizing = FALSE;  // TBC is it need?

static BOOL flag_bFirstTimeCall = FALSE;

static BOOL bTransparentFocus = TRUE,
bNeedFixSizingRect = FALSE,
bShowEventStarted = FALSE;

static HGENMENU hRenameMenuItem, hShowAvatarMenuItem, hHideAvatarMenuItem;

static UINT uMsgGetProfile = 0;

static int nLastRequiredHeight = 0, nRequiredHeight = 0, nMirMenuState = 0, nStatusMenuState = 0;

static RECT rcSizingRect = { 0 }, rcCorrectSizeRect = { 0 };

static HANDLE hFrameContactTree;

static int nAnimatedIconStep = 100;

HIMAGELIST hAvatarOverlays = nullptr;

OVERLAYICONINFO g_pAvatarOverlayIcons[MAX_STATUS_COUNT] =
{
	{ "AVATAR_OVERLAY_OFFLINE", LPGEN("Offline"), IDI_AVATAR_OVERLAY_OFFLINE, -1 },
	{ "AVATAR_OVERLAY_ONLINE", LPGEN("Online"), IDI_AVATAR_OVERLAY_ONLINE, -1 },
	{ "AVATAR_OVERLAY_AWAY", LPGEN("Away"), IDI_AVATAR_OVERLAY_AWAY, -1 },
	{ "AVATAR_OVERLAY_DND", LPGEN("Do not disturb"), IDI_AVATAR_OVERLAY_DND, -1 },
	{ "AVATAR_OVERLAY_NA", LPGEN("Not available"), IDI_AVATAR_OVERLAY_NA, -1 },
	{ "AVATAR_OVERLAY_OCCUPIED", LPGEN("Occupied"), IDI_AVATAR_OVERLAY_OCCUPIED, -1 },
	{ "AVATAR_OVERLAY_CHAT", LPGEN("Free for chat"), IDI_AVATAR_OVERLAY_CHAT, -1 },
	{ "AVATAR_OVERLAY_INVISIBLE", LPGEN("Invisible"), IDI_AVATAR_OVERLAY_INVISIBLE, -1 }
};

OVERLAYICONINFO g_pStatusOverlayIcons[MAX_STATUS_COUNT] =
{
	{ "STATUS_OVERLAY_OFFLINE", LPGEN("Offline"), IDI_STATUS_OVERLAY_OFFLINE, -1 },
	{ "STATUS_OVERLAY_ONLINE", LPGEN("Online"), IDI_STATUS_OVERLAY_ONLINE, -1 },
	{ "STATUS_OVERLAY_AWAY", LPGEN("Away"), IDI_STATUS_OVERLAY_AWAY, -1 },
	{ "STATUS_OVERLAY_DND", LPGEN("Do not disturb"), IDI_STATUS_OVERLAY_DND, -1 },
	{ "STATUS_OVERLAY_NA", LPGEN("Not available"), IDI_STATUS_OVERLAY_NA, -1 },
	{ "STATUS_OVERLAY_OCCUPIED", LPGEN("Occupied"), IDI_STATUS_OVERLAY_OCCUPIED, -1 },
	{ "STATUS_OVERLAY_CHAT", LPGEN("Free for chat"), IDI_STATUS_OVERLAY_CHAT, -1 },
	{ "STATUS_OVERLAY_INVISIBLE", LPGEN("Invisible"), IDI_STATUS_OVERLAY_INVISIBLE, -1 }
};

//////////////// CLUI CLASS IMPLEMENTATION // ///////////////////////////////
#include "modern_clui.h"

CLUI *CLUI::m_pCLUI = nullptr;
BOOL CLUI::m_fMainMenuInited = FALSE;
HWND CLUI::m_hWnd = nullptr;

static wchar_t tszFolderPath[MAX_PATH];

void CLUI::cliOnCreateClc(void)
{
	_ASSERT(m_pCLUI);
	m_pCLUI->CreateCluiFrames();
}

int CLUI::OnEvent_ModulesLoaded(WPARAM, LPARAM)
{
	cliCluiProtocolStatusChanged(0, nullptr);
	SleepEx(0, TRUE);
	g_bOnModulesLoadedCalled = true;

	SendMessage(g_clistApi.hwndContactList, UM_CREATECLC, 0, 0); // $$$
	InitSkinHotKeys();
	g_CluiData.bSTATE = STATE_NORMAL;
	ske_RedrawCompleteWindow();
	return 0;
}

int CLUI::OnEvent_FontReload(WPARAM wParam, LPARAM lParam)
{
	Clist_Broadcast(INTM_RELOADOPTIONS, wParam, lParam);

	g_CluiData.dwKeyColor = db_get_dw(0, "ModernSettings", "KeyColor", (uint32_t)SETTING_KEYCOLOR_DEFAULT);

	cliInvalidateRect(g_clistApi.hwndContactList, nullptr, 0);
	return 0;
}

int CLUI::OnEvent_ContactMenuPreBuild(WPARAM, LPARAM)
{
	if (MirandaExiting())
		return 0;

	HWND hwndClist = GetFocus();
	wchar_t cls[128];
	GetClassName(hwndClist, cls, _countof(cls));
	if (mir_wstrcmp(CLISTCONTROL_CLASSW, cls))
		hwndClist = g_clistApi.hwndContactList;

	MCONTACT hItem = (MCONTACT)SendMessage(hwndClist, CLM_GETSELECTION, 0, 0);
	Menu_ShowItem(hRenameMenuItem, hItem != 0);

	if (!hItem || !IsHContactContact(hItem) || !g_plugin.getByte("AvatarsShow", SETTINGS_SHOWAVATARS_DEFAULT)) {
		Menu_ShowItem(hShowAvatarMenuItem, false);
		Menu_ShowItem(hHideAvatarMenuItem, false);
	}
	else {
		bool bHideAvatar = g_plugin.getByte(hItem, "HideContactAvatar") != 0;
		Menu_ShowItem(hShowAvatarMenuItem, bHideAvatar);
		Menu_ShowItem(hHideAvatarMenuItem, !bHideAvatar);
	}

	return 0;
}

INT_PTR CLUI::Service_ShowMainMenu(WPARAM, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(Menu_GetMainMenu(), TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
	return 0;
}

INT_PTR CLUI::Service_ShowStatusMenu(WPARAM, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);
	TrackPopupMenu(Menu_GetStatusMenu(), TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
	return 0;
}

INT_PTR CLUI::Service_Menu_ShowContactAvatar(WPARAM hContact, LPARAM)
{
	g_plugin.setByte(hContact, "HideContactAvatar", 0);

	Clist_Broadcast(INTM_AVATARCHANGED, hContact, 0);
	return 0;
}

INT_PTR CLUI::Service_Menu_HideContactAvatar(WPARAM hContact, LPARAM)
{
	g_plugin.setByte(hContact, "HideContactAvatar", 1);

	Clist_Broadcast(INTM_AVATARCHANGED, hContact, 0);
	return 0;
}

HRESULT CLUI::CreateCluiFrames()
{
	CreateCLCWindow(g_clistApi.hwndContactList);

	CLUI_ChangeWindowMode();

	RegisterAvatarMenu();

	CLUI_ReloadCLUIOptions();

	CreateUIFrames();

	HookEvent(ME_SYSTEM_MODULESLOADED, CLUI::OnEvent_ModulesLoaded);
	HookEvent(ME_SKIN_ICONSCHANGED, CLUI_IconsChanged);
	HookEvent(ME_FONT_RELOAD, CLUI::OnEvent_FontReload);
	return S_OK;
}

CLUI::CLUI() :
	m_hDwmapiDll(nullptr)
{
	m_pCLUI = this;
	g_CluiData.bSTATE = STATE_CLUI_LOADING;
	LoadDllsRuntime();
	hFrameContactTree = nullptr;

	CLUIServices_LoadModule();

	CreateServiceFunction(MS_CLUI_SHOWMAINMENU, Service_ShowMainMenu);
	CreateServiceFunction(MS_CLUI_SHOWSTATUSMENU, Service_ShowStatusMenu);

	// TODO Add Row template loading here.

	nLastRequiredHeight = 0;

	LoadCLUIFramesModule();

	g_CluiData.boldHideOffline = -1;
	bOldHideOffline = Clist::HideOffline;

	g_CluiData.bOldUseGroups = -1;
	bOldUseGroups = Clist::UseGroups;
}

CLUI::~CLUI()
{
	FreeLibrary(m_hDwmapiDll);
	m_pCLUI = nullptr;
}

HRESULT CLUI::LoadDllsRuntime()
{
	g_CluiData.fLayered = !db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT);
	g_CluiData.fSmoothAnimation = db_get_b(0, "CLUI", "FadeInOut", SETTING_FADEIN_DEFAULT) != 0;
	g_CluiData.fLayered = (g_CluiData.fLayered*db_get_b(0, "ModernData", "EnableLayering", g_CluiData.fLayered)) && !db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT);

	if (IsWinVerVistaPlus() && !IsWinVer8Plus()) {
		m_hDwmapiDll = LoadLibrary(L"dwmapi.dll");
		if (m_hDwmapiDll)
			g_proc_DWMEnableBlurBehindWindow = (HRESULT(WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(m_hDwmapiDll, "DwmEnableBlurBehindWindow");
	}

	g_CluiData.fAeroGlass = FALSE;
	return S_OK;
}

static IconItem iconItem[] =
{
	{ LPGEN("Show avatar"), "ShowAvatar", IDI_SHOW_AVATAR },
	{ LPGEN("Hide avatar"), "HideAvatar", IDI_HIDE_AVATAR }
};

HRESULT CLUI::RegisterAvatarMenu()
{
	g_plugin.registerIcon(LPGEN("Contact list"), iconItem);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x1cc99858, 0x40ca, 0x4558, 0xae, 0x10, 0xba, 0x81, 0xaf, 0x4c, 0x67, 0xb5);
	CreateServiceFunction("CList/ShowContactAvatar", CLUI::Service_Menu_ShowContactAvatar);
	mi.position = 2000150000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.name.a = LPGEN("Show contact &avatar");
	mi.pszService = "CList/ShowContactAvatar";
	hShowAvatarMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x13f93d1b, 0xd470, 0x45de, 0x86, 0x8f, 0x22, 0x3b, 0x89, 0x4f, 0x4f, 0xa3);
	CreateServiceFunction("CList/HideContactAvatar", CLUI::Service_Menu_HideContactAvatar);
	mi.position = 2000150001;
	mi.hIcolibItem = iconItem[1].hIcolib;
	mi.name.a = LPGEN("Hide contact &avatar");
	mi.pszService = "CList/HideContactAvatar";
	hHideAvatarMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CLUI::OnEvent_ContactMenuPreBuild);
	return S_OK;
}

HRESULT CLUI::CreateCLCWindow(const HWND hwndClui)
{
	g_clistApi.hwndContactTree = CreateWindow(CLISTCONTROL_CLASSW, L"",
		WS_CHILD | WS_CLIPCHILDREN | CLS_CONTACTLIST | (Clist::UseGroups ? CLS_USEGROUPS : 0) | (Clist::HideOffline ? CLS_HIDEOFFLINE : 0) | (Clist::HideEmptyGroups ? CLS_HIDEEMPTYGROUPS : 0 | CLS_MULTICOLUMN),
		0, 0, 0, 0, hwndClui, nullptr, g_plugin.getInst(), nullptr);

	return S_OK;
}

HRESULT CLUI::CreateUIFrames()
{
	EventArea_Create(g_clistApi.hwndContactList);
	CreateViewModeFrame();
	g_clistApi.hwndStatus = StatusBar_Create(g_clistApi.hwndContactList);

	return S_OK;
}

HRESULT CLUI::FillAlphaChannel(HDC hDC, RECT *prcParent)
{
	RECT rcWindow;
	GetWindowRect(m_hWnd, &rcWindow);

	HRGN hRgn = CreateRectRgn(0, 0, 0, 0);

	if (GetWindowRgn(m_hWnd, hRgn) == ERROR) {
		DeleteObject(hRgn);
		hRgn = CreateRectRgn(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);
	}

	OffsetRgn(hRgn, -prcParent->left, -prcParent->top);

	RECT rcBounds;
	GetRgnBox(hRgn, &rcBounds);

	if (IsRectEmpty(&rcBounds)) {
		DeleteObject(hRgn);
		return S_FALSE;
	}

	uint32_t dwRgnSize = GetRegionData(hRgn, 0, nullptr);
	RGNDATA *rgnData = (RGNDATA *)malloc(dwRgnSize);
	GetRegionData(hRgn, dwRgnSize, rgnData);

	RECT *pRect = (RECT *)rgnData->Buffer;

	for (uint32_t i = 0; i < rgnData->rdh.nCount; i++)
		ske_SetRectOpaque(hDC, &pRect[i]);

	free(rgnData);
	DeleteObject(hRgn);

	return S_OK;
}

HRESULT CLUI::CreateCLC()
{
	CLISTFrame Frame = { sizeof(Frame) };
	Frame.hWnd = g_clistApi.hwndContactTree;
	Frame.align = alClient;
	Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	Frame.Flags = F_VISIBLE | F_SHOWTBTIP | F_NO_SUBCONTAINER;
	Frame.szName.a = LPGEN("My contacts");
	Frame.szTBname.a = LPGEN("My contacts");
	hFrameContactTree = (HWND)g_plugin.addFrame(&Frame);

	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)Frame.hWnd, (LPARAM)CLCPaint::PaintCallbackProc);
	CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBTIPNAME, hFrameContactTree), (LPARAM)Translate("My Contacts"));

	ExtraIcon_Reload();

	nLastRequiredHeight = 0;
	if (g_CluiData.current_viewmode[0] == '\0') {
		g_clistApi.pfnSetHideOffline((bOldHideOffline == -1) ? false : bOldHideOffline);
		CallService(MS_CLIST_SETUSEGROUPS, (bOldUseGroups == -1) ? false : bOldUseGroups, 0);
	}
	nLastRequiredHeight = 0;
	mutex_bDisableAutoUpdate = 0;

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaStatusChanged);
	return S_OK;
}

HRESULT CLUI::SnappingToEdge(WINDOWPOS *lpWindowPos)
{
	if (db_get_b(0, "CLUI", "SnapToEdges", SETTING_SNAPTOEDGES_DEFAULT)) {
		HMONITOR curMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monInfo;
		monInfo.cbSize = sizeof(monInfo);
		GetMonitorInfo(curMonitor, &monInfo);

		RECT *dr = &monInfo.rcWork;

		// Left side
		if (lpWindowPos->x < dr->left + SNAPTOEDGESENSIVITY && lpWindowPos->x > dr->left - SNAPTOEDGESENSIVITY && g_CluiData.bBehindEdgeSettings != 1)
			lpWindowPos->x = dr->left;

		// Right side
		if (dr->right - lpWindowPos->x - lpWindowPos->cx  < SNAPTOEDGESENSIVITY && dr->right - lpWindowPos->x - lpWindowPos->cx > -SNAPTOEDGESENSIVITY && g_CluiData.bBehindEdgeSettings != 2)
			lpWindowPos->x = dr->right - lpWindowPos->cx;

		// Top side
		if (lpWindowPos->y < dr->top + SNAPTOEDGESENSIVITY && lpWindowPos->y > dr->top - SNAPTOEDGESENSIVITY)
			lpWindowPos->y = dr->top;

		// Bottom side
		if (dr->bottom - lpWindowPos->y - lpWindowPos->cy  < SNAPTOEDGESENSIVITY && dr->bottom - lpWindowPos->y - lpWindowPos->cy > -SNAPTOEDGESENSIVITY)
			lpWindowPos->y = dr->bottom - lpWindowPos->cy;
	}
	return S_OK;
}

HICON GetMainStatusOverlay(int STATUS)
{
	return ImageList_GetIcon(hAvatarOverlays, g_pStatusOverlayIcons[STATUS - ID_STATUS_OFFLINE].listID, ILD_NORMAL);
}

void UnloadAvatarOverlayIcon()
{
	for (auto &it : g_pAvatarOverlayIcons)
		it.listID = -1;
	for (auto &it : g_pStatusOverlayIcons)
		it.listID = -1;

	ImageList_Destroy(hAvatarOverlays);
	hAvatarOverlays = nullptr;
	DestroyIcon_protect(g_hListeningToIcon);
	g_hListeningToIcon = nullptr;
}

BOOL CLUI_CheckOwnedByClui(HWND hWnd)
{
	if (!hWnd)
		return FALSE;

	HWND hWndClui = g_clistApi.hwndContactList;
	HWND hWndMid = GetAncestor(hWnd, GA_ROOTOWNER);
	if (hWndMid == hWndClui)
		return TRUE;

	wchar_t buf[255];
	GetClassName(hWndMid, buf, 254);
	if (!mir_wstrcmpi(buf, CLUIFrameSubContainerClassName))
		return TRUE;

	return FALSE;
}

int CLUI_ShowWindowMod(HWND hWnd, int nCmd)
{
	if (hWnd == g_clistApi.hwndContactList) {
		if (nCmd == SW_HIDE || nCmd == SW_MINIMIZE) {
			AniAva_InvalidateAvatarPositions(0);
			AniAva_RemoveInvalidatedAvatars();
		}

		if (!g_bChangingMode && !g_CluiData.fLayered) {
			if (nCmd == SW_HIDE && g_plugin.getByte("WindowShadow", SETTING_WINDOWSHADOW_DEFAULT)) {
				ShowWindow(hWnd, SW_MINIMIZE); // removing of shadow
				return ShowWindow(hWnd, nCmd);
			}

			if (nCmd == SW_RESTORE && g_CluiData.fSmoothAnimation && !g_bTransparentFlag) {
				if (g_plugin.getByte("WindowShadow", SETTING_WINDOWSHADOW_DEFAULT))
					CLUI_SmoothAlphaTransition(hWnd, 255, 1);
				else {
					int ret = ShowWindow(hWnd, nCmd);
					CLUI_SmoothAlphaTransition(hWnd, 255, 1);
					return ret;
				}
			}
		}
	}
	return ShowWindow(hWnd, nCmd);
}

static BOOL CLUI_WaitThreadsCompletion()
{
	static uint8_t bEntersCount = 0;
	static const uint8_t bcMAX_AWAITING_RETRY = 10; // repeat awaiting only 10 times
	TRACE("CLUI_WaitThreadsCompletion Enter");
	if (bEntersCount < bcMAX_AWAITING_RETRY && (g_CluiData.mutexPaintLock || g_hAwayMsgThread || g_hGetTextAsyncThread || g_hSmoothAnimationThread) && !Miranda_IsTerminated()) {
		TRACE("Waiting threads");
		TRACEVAR("g_CluiData.mutexPaintLock: %x", g_CluiData.mutexPaintLock);
		TRACEVAR("g_hAwayMsgThread: %x", g_hAwayMsgThread);
		TRACEVAR("g_hGetTextAsyncThread: %x", g_hGetTextAsyncThread);
		TRACEVAR("g_hSmoothAnimationThread: %x", g_hSmoothAnimationThread);

		bEntersCount++;
		SleepEx(10, TRUE);
		return TRUE;
	}

	return FALSE;
}

void CLUI_UpdateLayeredMode()
{
	g_CluiData.fDisableSkinEngine = db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT) != 0;

	bool tLayeredFlag = db_get_b(0, "ModernData", "EnableLayering", SETTING_ENABLELAYERING_DEFAULT) != 0 && !g_CluiData.fDisableSkinEngine;
	if (g_CluiData.fLayered != tLayeredFlag) {
		BOOL fWasVisible = IsWindowVisible(g_clistApi.hwndContactList);
		if (fWasVisible)
			ShowWindow(g_clistApi.hwndContactList, SW_HIDE);

		//change layered mode
		LONG_PTR exStyle = GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE);
		if (tLayeredFlag)
			exStyle |= WS_EX_LAYERED;
		else
			exStyle &= ~WS_EX_LAYERED;

		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, exStyle);
		g_CluiData.fLayered = tLayeredFlag;
		Sync(CLUIFrames_SetLayeredMode, tLayeredFlag, g_clistApi.hwndContactList);
		CLUI_ChangeWindowMode();
		Sync(CLUIFrames_OnClistResize_mod, 0, 0);
		if (fWasVisible)
			ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
	}
}

void CLUI_UpdateAeroGlass()
{
	bool tAeroGlass = db_get_b(0, "ModernData", "AeroGlass", SETTING_AEROGLASS_DEFAULT) && g_CluiData.fLayered;
	if (g_proc_DWMEnableBlurBehindWindow && (tAeroGlass != g_CluiData.fAeroGlass)) {
		if (g_CluiData.hAeroGlassRgn) {
			DeleteObject(g_CluiData.hAeroGlassRgn);
			g_CluiData.hAeroGlassRgn = nullptr;
		}

		DWM_BLURBEHIND bb = { 0 };
		bb.dwFlags = DWM_BB_ENABLE;
		bb.fEnable = tAeroGlass;

		if (tAeroGlass) {
			g_CluiData.hAeroGlassRgn = ske_CreateOpaqueRgn(AEROGLASS_MINALPHA, true);
			bb.hRgnBlur = g_CluiData.hAeroGlassRgn;
			bb.dwFlags |= DWM_BB_BLURREGION;
		}

		g_proc_DWMEnableBlurBehindWindow(g_clistApi.hwndContactList, &bb);
		g_CluiData.fAeroGlass = tAeroGlass;
	}
}

extern int CLUIFrames_UpdateBorders();

void CLUI_ChangeWindowMode()
{
	BOOL storedVisMode = FALSE;
	LONG_PTR style, styleEx;
	LONG_PTR oldStyle, oldStyleEx;
	LONG_PTR styleMask = WS_CLIPCHILDREN | WS_BORDER | WS_CAPTION | WS_MINIMIZEBOX | WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_THICKFRAME | WS_SYSMENU;
	LONG_PTR styleMaskEx = WS_EX_TOOLWINDOW | WS_EX_LAYERED;
	LONG_PTR curStyle, curStyleEx;
	if (!g_clistApi.hwndContactList) return;

	g_bChangingMode = true;
	g_bTransparentFlag = g_plugin.getByte("Transparent", SETTING_TRANSPARENT_DEFAULT);
	g_CluiData.fSmoothAnimation = db_get_b(0, "CLUI", "FadeInOut", SETTING_FADEIN_DEFAULT) != 0;
	if (g_bTransparentFlag == 0 && g_CluiData.bCurrentAlpha != 0)
		g_CluiData.bCurrentAlpha = 255;

	// 2 - Calculate STYLES and STYLESEX
	if (!g_CluiData.fLayered) {
		style = 0;
		styleEx = 0;
		if (g_plugin.getByte("ThinBorder", SETTING_THINBORDER_DEFAULT) || (g_plugin.getByte("NoBorder", SETTING_NOBORDER_DEFAULT))) {
			style = WS_CLIPCHILDREN | (g_plugin.getByte("ThinBorder", SETTING_THINBORDER_DEFAULT) ? WS_BORDER : 0);
			styleEx = WS_EX_TOOLWINDOW;
			styleMaskEx |= WS_EX_APPWINDOW;
		}
		else if (db_get_b(0, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT) && g_plugin.getByte("ToolWindow", SETTING_TOOLWINDOW_DEFAULT)) {
			styleEx = WS_EX_TOOLWINDOW/*|WS_EX_WINDOWEDGE*/;
			style = WS_CAPTION | WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_THICKFRAME;
			styleMaskEx |= WS_EX_APPWINDOW;
		}
		else if (db_get_b(0, "CLUI", "ShowCaption", SETTING_SHOWCAPTION_DEFAULT))
			style = WS_CAPTION | WS_SYSMENU | WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_THICKFRAME | WS_MINIMIZEBOX;
		else {
			style = WS_POPUPWINDOW | WS_CLIPCHILDREN | WS_THICKFRAME;
			styleEx = WS_EX_TOOLWINDOW/*|WS_EX_WINDOWEDGE*/;
			styleMaskEx |= WS_EX_APPWINDOW;
		}
	}
	else {
		style = WS_CLIPCHILDREN;
		styleEx = WS_EX_TOOLWINDOW;
		styleMaskEx |= WS_EX_APPWINDOW;
	}

	// 3 - TODO Update Layered mode
	if (g_bTransparentFlag && g_CluiData.fLayered)
		styleEx |= WS_EX_LAYERED;

	// 4 - Set Title
	wchar_t titleText[255] = { 0 };
	DBVARIANT dbv;
	if (g_plugin.getWString("TitleText", &dbv))
		wcsncpy_s(titleText, _A2W(MIRANDANAME), _TRUNCATE);
	else {
		wcsncpy_s(titleText, dbv.pwszVal, _TRUNCATE);
		db_free(&dbv);
	}
	SetWindowText(g_clistApi.hwndContactList, titleText);

	// < ->
	// 1 - If visible store it and hide
	if (g_CluiData.fLayered && (g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT))) {
		SetParent(g_clistApi.hwndContactList, nullptr);
		Sync(CLUIFrames_SetParentForContainers, (HWND)nullptr);
		UpdateWindow(g_clistApi.hwndContactList);
		g_CluiData.fOnDesktop = false;
	}

	// 5 - TODO Apply Style
	oldStyleEx = curStyleEx = GetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE);
	oldStyle = curStyle = GetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE);

	curStyleEx = (curStyleEx & ~styleMaskEx) | styleEx;
	curStyle = (curStyle & ~styleMask) | style;
	if (oldStyleEx != curStyleEx || oldStyle != curStyle) {
		if (IsWindowVisible(g_clistApi.hwndContactList)) {
			storedVisMode = TRUE;
			mutex_bShowHideCalledFromAnimation = TRUE;
			ShowWindow(g_clistApi.hwndContactList, SW_HIDE);
			Sync(CLUIFrames_OnShowHide, 0);
		}
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_EXSTYLE, curStyleEx);
		SetWindowLongPtr(g_clistApi.hwndContactList, GWL_STYLE, curStyle);
	}

	CLUI_UpdateAeroGlass();

	if (g_CluiData.fLayered || !db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT))
		SetMenu(g_clistApi.hwndContactList, nullptr);
	else
		SetMenu(g_clistApi.hwndContactList, g_clistApi.hMenuMain);

	if (g_CluiData.fLayered && (g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT)))
		ske_UpdateWindowImage();

	// 6 - Pin to desktop mode
	if (g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT)) {
		HWND hProgMan = FindWindow(L"Progman", nullptr);
		if (IsWindow(hProgMan)) {
			SetParent(g_clistApi.hwndContactList, hProgMan);
			Sync(CLUIFrames_SetParentForContainers, (HWND)hProgMan);
			g_CluiData.fOnDesktop = true;
		}
	}
	else {
		SetParent(g_clistApi.hwndContactList, nullptr);
		Sync(CLUIFrames_SetParentForContainers, (HWND)nullptr);
		g_CluiData.fOnDesktop = false;
	}

	// 7 - if it was visible - show
	if (storedVisMode) {
		ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
		Sync(CLUIFrames_OnShowHide, 1);
	}
	mutex_bShowHideCalledFromAnimation = FALSE;

	CLUIFrames_UpdateBorders();

	if (!g_CluiData.fLayered) {
		RECT r;
		int w = 10;
		GetWindowRect(g_clistApi.hwndContactList, &r);
		int h = (r.right - r.left) > (w * 2) ? w : (r.right - r.left);
		int v = (r.bottom - r.top) > (w * 2) ? w : (r.bottom - r.top);
		h = (h < v) ? h : v;
		HRGN hRgn1 = CreateRoundRectRgn(0, 0, (r.right - r.left + 1), (r.bottom - r.top + 1), h, h);
		if (db_get_b(0, "CLC", "RoundCorners", SETTING_ROUNDCORNERS_DEFAULT) && !Clist_IsDocked())
			SetWindowRgn(g_clistApi.hwndContactList, hRgn1, 1);
		else {
			DeleteObject(hRgn1);
			SetWindowRgn(g_clistApi.hwndContactList, nullptr, 1);
		}

		RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	g_bChangingMode = false;
	flag_bFirstTimeCall = true;
	AniAva_UpdateParent();
}

struct  _tagTimerAsync
{
	HWND hwnd;
	int ID;
	int Timeout;
	TIMERPROC proc;
};

static UINT_PTR SetTimerSync(WPARAM wParam, LPARAM)
{
	struct  _tagTimerAsync *call = (struct  _tagTimerAsync *)wParam;
	return SetTimer(call->hwnd, call->ID, call->Timeout, call->proc);
}

UINT_PTR CLUI_SafeSetTimer(HWND hwnd, int ID, int Timeout, TIMERPROC proc)
{
	struct  _tagTimerAsync param = { hwnd, ID, Timeout, proc };
	return Sync(SetTimerSync, (WPARAM)&param, 0);
}

int CLUI_UpdateTimer()
{
	if (g_CluiData.nBehindEdgeState == 0) {
		KillTimer(g_clistApi.hwndContactList, TM_BRINGOUTTIMEOUT);
		CLUI_SafeSetTimer(g_clistApi.hwndContactList, TM_BRINGOUTTIMEOUT, wBehindEdgeHideDelay * 100, nullptr);
	}

	if (bShowEventStarted == 0 && g_CluiData.nBehindEdgeState > 0) {
		KillTimer(g_clistApi.hwndContactList, TM_BRINGINTIMEOUT);
		bShowEventStarted = (BOOL)CLUI_SafeSetTimer(g_clistApi.hwndContactList, TM_BRINGINTIMEOUT, wBehindEdgeShowDelay * 100, nullptr);
	}
	return 0;
}

int CLUI_HideBehindEdge()
{
	int method = g_CluiData.bBehindEdgeSettings;
	if (method) {
		RECT rcScreen;
		RECT rcWindow;
		//Need to be moved out of screen
		bShowEventStarted = 0;
		//1. get work area rectangle
		Docking_GetMonitorRectFromWindow(g_clistApi.hwndContactList, &rcScreen);
		//SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen,FALSE);
		//2. move out
		int bordersize = wBehindEdgeBorderSize;
		GetWindowRect(g_clistApi.hwndContactList, &rcWindow);
		switch (method) {
		case 1: // left
			rcWindow.left = rcScreen.left - (rcWindow.right - rcWindow.left) + bordersize;
			break;
		case 2: // right
			rcWindow.left = rcScreen.right - bordersize;
			break;
		}
		g_CluiData.mutexPreventDockMoving = 0;
		SetWindowPos(g_clistApi.hwndContactList, nullptr, rcWindow.left, rcWindow.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		Sync(CLUIFrames_OnMoving, g_clistApi.hwndContactList, &rcWindow);
		g_CluiData.mutexPreventDockMoving = 1;

		//3. store setting
		db_set_b(0, "ModernData", "BehindEdge", method);
		g_CluiData.nBehindEdgeState = method;
		return 1;
	}
	return 0;
}


int CLUI_ShowFromBehindEdge()
{
	int method = g_CluiData.bBehindEdgeSettings;
	bShowEventStarted = 0;
	if (g_bOnTrayRightClick) {
		g_bOnTrayRightClick= false;
		return 0;
	}

	if (method) {
		// Need to be moved out of screen
		// 1. get work area rectangle
		RECT rcScreen;
		Docking_GetMonitorRectFromWindow(g_clistApi.hwndContactList, &rcScreen);

		// 2. move out
		RECT rcWindow;
		GetWindowRect(g_clistApi.hwndContactList, &rcWindow);
		switch (method) {
		case 1: // left
			rcWindow.left = rcScreen.left;
			break;
		case 2: // right
			rcWindow.left = rcScreen.right - (rcWindow.right - rcWindow.left);
			break;
		}
		g_CluiData.mutexPreventDockMoving = 0;
		SetWindowPos(g_clistApi.hwndContactList, nullptr, rcWindow.left, rcWindow.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		Sync(CLUIFrames_OnMoving, g_clistApi.hwndContactList, &rcWindow);
		g_CluiData.mutexPreventDockMoving = 1;

		// 3. store setting
		db_set_b(0, "ModernData", "BehindEdge", 0);
		g_CluiData.nBehindEdgeState = 0;
	}
	return 0;
}

int CLUI_IsInMainWindow(HWND hwnd)
{
	if (hwnd == g_clistApi.hwndContactList) return 1;
	if (GetParent(hwnd) == g_clistApi.hwndContactList) return 2;
	return 0;
}

int CLUI_OnSkinLoad(WPARAM, LPARAM)
{
	ske_LoadSkinFromDB();
	return 0;
}

static int CLUI_GetConnectingIconForProtoCount(char *szAccoName)
{
	int count;
	wchar_t fileFull[MAX_PATH];

	if (!tszFolderPath[0]) {
		wchar_t szRelativePath[MAX_PATH];
		GetModuleFileName(GetModuleHandle(nullptr), szRelativePath, MAX_PATH);
		wchar_t *str = wcsrchr(szRelativePath, '\\');
		if (str != nullptr)
			*str = 0;
		PathToAbsoluteW(szRelativePath, tszFolderPath);
	}

	if (szAccoName) {
		// first of all try to find by account name( or empty - global )
		mir_snwprintf(fileFull, L"%s\\Icons\\proto_conn_%S.dll", tszFolderPath, szAccoName);
		if (count = ExtractIconEx(fileFull, -1, nullptr, nullptr, 1))
			return count;

		if (szAccoName[0]) {
			// second try to find by protocol name
			PROTOACCOUNT *acc = Proto_GetAccount(szAccoName);
			if (acc && !acc->bOldProto) {
				mir_snwprintf(fileFull, L"%s\\Icons\\proto_conn_%S.dll", tszFolderPath, acc->szProtoName);
				if (count = ExtractIconEx(fileFull, -1, nullptr, nullptr, 1))
					return count;
			}
		}
	}

	// third try global
	mir_snwprintf(fileFull, L"%s\\Icons\\proto_conn.dll", tszFolderPath);
	if (count = ExtractIconEx(fileFull, -1, nullptr, nullptr, 1))
		return count;

	return 8;
}

static HICON CLUI_LoadIconFromExternalFile(wchar_t *filename, int i)
{
	wchar_t szPath[MAX_PATH], szFullPath[MAX_PATH];
	mir_snwprintf(szPath, L"Icons\\%s", filename);
	PathToAbsoluteW(szPath, szFullPath);
	if (_waccess(szFullPath, 0))
		return nullptr;

	HICON hIcon = nullptr;
	ExtractIconEx(szFullPath, i, nullptr, &hIcon, 1);
	return hIcon;
}

static HICON CLUI_GetConnectingIconForProto(char *szAccoName, int idx)
{
	wchar_t szFullPath[MAX_PATH];
	HICON hIcon;

	if (szAccoName) {
		mir_snwprintf(szFullPath, L"proto_conn_%S.dll", szAccoName);
		if (hIcon = CLUI_LoadIconFromExternalFile(szFullPath, idx))
			return hIcon;

		if (szAccoName[0]) {
			// second try to find by protocol name
			PROTOACCOUNT *acc = Proto_GetAccount(szAccoName);
			if (acc && !acc->bOldProto) {
				mir_snwprintf(szFullPath, L"proto_conn_%S.dll", acc->szProtoName);
				if (hIcon = CLUI_LoadIconFromExternalFile(szFullPath, idx))
					return hIcon;
			}
		}
	}

	// third try global
	wcsncpy_s(szFullPath, L"proto_conn.dll", _TRUNCATE);
	if (hIcon = CLUI_LoadIconFromExternalFile(szFullPath, idx))
		return hIcon;

	return LoadSmallIcon(g_plugin.getInst(), -IDI_ICQC1 - idx);
}

static PROTOTICKS* CLUI_GetProtoTicksByProto(char *szProto)
{
	for (auto &it : arTicks)
		if (!mir_strcmp(it->szProto, szProto))
			return it;

	PROTOTICKS *pt = new PROTOTICKS();
	pt->szProto = mir_strdup(szProto);
	pt->nCycleStartTick = 0;
	pt->nIndex = arTicks.getCount();
	pt->bGlobal = (szProto[0] == 0);
	pt->himlIconList = nullptr;
	arTicks.insert(pt);
	return pt;
}

static bool StartTicksTimer(PROTOTICKS *pt)
{
	int cnt = CLUI_GetConnectingIconForProtoCount(pt->szProto);
	if (cnt == 0)
		return false;

	pt->nIconsCount = cnt;
	pt->himlIconList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, cnt, 1);
	for (int i = 0; i < cnt; i++) {
		HICON ic = CLUI_GetConnectingIconForProto(pt->szProto, i);
		if (ic) {
			ImageList_AddIcon(pt->himlIconList, ic);
			DestroyIcon(ic);
		}
	}
	CLUI_SafeSetTimer(g_clistApi.hwndContactList, TM_STATUSBARUPDATE + pt->nIndex, nAnimatedIconStep, nullptr);
	pt->bTimerCreated = 1;
	pt->nCycleStartTick = GetTickCount();
	return true;
}

INT_PTR CLUI_GetConnectingIconService(WPARAM wParam, LPARAM)
{
	char *szProto = (char*)wParam;
	if (!szProto || !g_StatusBarData.bConnectingIcon)
		return 0;

	PROTOTICKS *pt = CLUI_GetProtoTicksByProto(szProto);
	if (pt->nCycleStartTick == 0)
		StartTicksTimer(pt);

	if (pt->nCycleStartTick != 0 && pt->nIconsCount != 0) {
		int b = ((GetTickCount() - pt->nCycleStartTick) / (nAnimatedIconStep)) % (pt->nIconsCount);
		if (pt->himlIconList)
			return (INT_PTR)ske_ImageList_GetIcon(pt->himlIconList, b);
	}

	return 0;
}

static BOOL CALLBACK BroadcastEnumChildProc(HWND hwndChild, LPARAM lParam)
{
	MSG *pMsg = (MSG*)lParam;
	SendNotifyMessage(hwndChild, pMsg->message, pMsg->wParam, pMsg->lParam);
	EnumChildWindows(hwndChild, BroadcastEnumChildProc, lParam);
	return TRUE;
}

static LRESULT BroadCastMessageToChild(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	MSG msg = {};
	msg.hwnd = hwnd;
	msg.lParam = lParam;
	msg.wParam = wParam;
	msg.message = message;
	EnumChildWindows(hwnd, BroadcastEnumChildProc, (LPARAM)&msg);
	return 1;
}

int CLUI_ReloadCLUIOptions()
{
	KillTimer(g_clistApi.hwndContactList, TM_UPDATEBRINGTIMER);
	g_CluiData.bBehindEdgeSettings = db_get_b(0, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT);
	wBehindEdgeShowDelay = db_get_w(0, "ModernData", "ShowDelay", SETTING_SHOWDELAY_DEFAULT);
	wBehindEdgeHideDelay = db_get_w(0, "ModernData", "HideDelay", SETTING_HIDEDELAY_DEFAULT);
	wBehindEdgeBorderSize = db_get_w(0, "ModernData", "HideBehindBorderSize", SETTING_HIDEBEHINDBORDERSIZE_DEFAULT);

	g_CluiData.fAutoSize = db_get_b(0, "CLUI", "AutoSize", SETTING_AUTOSIZE_DEFAULT) != 0;
	g_CluiData.bInternalAwayMsgDiscovery = db_get_b(0, "ModernData", "InternalAwayMsgDiscovery", SETTING_INTERNALAWAYMSGREQUEST_DEFAULT);
	g_CluiData.bRemoveAwayMessageForOffline = db_get_b(0, "ModernData", "RemoveAwayMessageForOffline", SETTING_REMOVEAWAYMSGFOROFFLINE_DEFAULT);

	// window borders
	if (g_CluiData.fDisableSkinEngine) {
		g_CluiData.LeftClientMargin = 0;
		g_CluiData.RightClientMargin = 0;
		g_CluiData.TopClientMargin = 0;
		g_CluiData.BottomClientMargin = 0;
	}
	else {
		// window borders
		g_CluiData.LeftClientMargin = db_get_b(0, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT);
		g_CluiData.RightClientMargin = db_get_b(0, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT);
		g_CluiData.TopClientMargin = db_get_b(0, "CLUI", "TopClientMargin", SETTING_TOPCLIENTMARIGN_DEFAULT);
		g_CluiData.BottomClientMargin = db_get_b(0, "CLUI", "BottomClientMargin", SETTING_BOTTOMCLIENTMARIGN_DEFAULT);
	}
	BroadCastMessageToChild(g_clistApi.hwndContactList, WM_THEMECHANGED, 0, 0);

	NotifyEventHooks(g_CluiData.hEventBkgrChanged, 0, 0);
	return 0;
}

void CLUI_DisconnectAll()
{
	for (auto &pa : Accounts())
		if (pa->IsEnabled())
			CallProtoService(pa->szModuleName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
}

static int CLUI_DrawMenuBackGround(HWND hwnd, HDC hdc, int item, int state)
{
	RECT ra, r1;
	HRGN treg, treg2;

	ClcData *dat = (ClcData*)GetWindowLongPtr(g_clistApi.hwndContactTree, 0);
	if (!dat)
		return 1;

	GetWindowRect(hwnd, &ra);

	MENUBARINFO mbi = { sizeof(MENUBARINFO) };
	GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbi);
	if (!(mbi.rcBar.right - mbi.rcBar.left > 0 && mbi.rcBar.bottom - mbi.rcBar.top > 0))
		return 1;

	r1 = mbi.rcBar;
	r1.bottom += !db_get_b(0, "CLUI", "LineUnderMenu", SETTING_LINEUNDERMENU_DEFAULT);
	if (item < 1) {
		treg = CreateRectRgn(mbi.rcBar.left, mbi.rcBar.top, mbi.rcBar.right, r1.bottom);
		if (item == 0) { // should remove item clips
			for (int t = 1; t <= 2; t++) {
				GetMenuBarInfo(hwnd, OBJID_MENU, t, &mbi);
				treg2 = CreateRectRgn(mbi.rcBar.left, mbi.rcBar.top, mbi.rcBar.right, mbi.rcBar.bottom);
				CombineRgn(treg, treg, treg2, RGN_DIFF);
				DeleteObject(treg2);
			}
		}
	}
	else {
		GetMenuBarInfo(hwnd, OBJID_MENU, item, &mbi);
		treg = CreateRectRgn(mbi.rcBar.left, mbi.rcBar.top, mbi.rcBar.right, mbi.rcBar.bottom + !db_get_b(0, "CLUI", "LineUnderMenu", SETTING_LINEUNDERMENU_DEFAULT));
	}
	OffsetRgn(treg, -ra.left, -ra.top);
	r1.left -= ra.left;
	r1.top -= ra.top;
	r1.bottom -= ra.top;
	r1.right -= ra.left;

	SelectClipRgn(hdc, treg);
	DeleteObject(treg);

	RECT rc;
	GetWindowRect(g_clistApi.hwndContactList, &rc);
	OffsetRect(&rc, -rc.left, -rc.top);
	FillRect(hdc, &r1, GetSysColorBrush(COLOR_MENU));
	ske_SetRectOpaque(hdc, &r1);

	if (!g_CluiData.fDisableSkinEngine)
		SkinDrawGlyph(hdc, &r1, &r1, "Main,ID=MenuBar");
	else {
		HBRUSH hbr = nullptr;
		if (dat->hMenuBackground) {
			RECT clRect = r1;

			// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
			SetStretchBltMode(hdc, HALFTONE);

			BITMAP bmp;
			GetObject(dat->hMenuBackground, sizeof(bmp), &bmp);
			HDC hdcBmp = CreateCompatibleDC(hdc);
			HBITMAP oldbm = (HBITMAP)SelectObject(hdcBmp, dat->hMenuBackground);
			int y = clRect.top, x = clRect.left, destw, desth;
			int maxx = (dat->MenuBmpUse & CLBF_TILEH) ? r1.right : x + 1;
			int maxy = (dat->MenuBmpUse & CLBF_TILEV) ? r1.bottom : y + 1;

			switch (dat->MenuBmpUse & CLBM_TYPE) {
			case CLB_STRETCH:
				if (dat->MenuBmpUse & CLBF_PROPORTIONAL) {
					if (clRect.right - clRect.left*bmp.bmHeight < clRect.bottom - clRect.top*bmp.bmWidth) {
						desth = clRect.bottom - clRect.top;
						destw = desth*bmp.bmWidth / bmp.bmHeight;
					}
					else {
						destw = clRect.right - clRect.left;
						desth = destw*bmp.bmHeight / bmp.bmWidth;
					}
				}
				else {
					destw = clRect.right - clRect.left;
					desth = clRect.bottom - clRect.top;
				}
				break;

			case CLB_STRETCHH:
				if (dat->MenuBmpUse & CLBF_PROPORTIONAL) {
					destw = clRect.right - clRect.left;
					desth = destw * bmp.bmHeight / bmp.bmWidth;
				}
				else {
					destw = clRect.right - clRect.left;
					desth = bmp.bmHeight;
				}
				break;

			case CLB_STRETCHV:
				if (dat->MenuBmpUse & CLBF_PROPORTIONAL) {
					desth = clRect.bottom - clRect.top;
					destw = desth * bmp.bmWidth / bmp.bmHeight;
				}
				else {
					destw = bmp.bmWidth;
					desth = clRect.bottom - clRect.top;
				}
				break;

			default:    // clb_topleft
				destw = bmp.bmWidth;
				desth = bmp.bmHeight;
				break;
			}
			if (desth && destw)
				for (y = clRect.top; y < maxy; y += desth)
					for (x = clRect.left; x < maxx; x += destw)
						StretchBlt(hdc, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

			SelectObject(hdcBmp, oldbm);
			DeleteDC(hdcBmp);
		}
		else {
			hbr = CreateSolidBrush(dat->MenuBkColor);
			FillRect(hdc, &r1, hbr);
			DeleteObject(hbr);
		}
		if (item != 0 && state & (ODS_SELECTED)) {
			hbr = CreateSolidBrush(dat->MenuBkHiColor);
			FillRect(hdc, &r1, hbr);
			DeleteObject(hbr);
		}
	}

	SelectClipRgn(hdc, nullptr);
	return 0;
}

int CLUI_SizingGetWindowRect(HWND hwnd, RECT *rc)
{
	if (mutex_bDuringSizing && hwnd == g_clistApi.hwndContactList)
		*rc = rcSizingRect;
	else
		GetWindowRect(hwnd, rc);
	return 1;
}


int CLUI_IconsChanged(WPARAM, LPARAM)
{
	if (MirandaExiting())
		return 0;

	DrawMenuBar(g_clistApi.hwndContactList);
	ExtraIcon_Reload();
	ExtraIcon_SetAll();

	// need to update tray cause it use combined icons
	Clist_TrayIconIconsChanged();  // TODO: remove as soon as core will include icolib
	ske_RedrawCompleteWindow();
	return 0;
}

void CLUI_cli_LoadCluiGlobalOpts()
{
	BOOL tLayeredFlag = TRUE;
	tLayeredFlag &= db_get_b(0, "ModernData", "EnableLayering", tLayeredFlag);
	if (tLayeredFlag) {
		if (g_plugin.getByte("WindowShadow", SETTING_WINDOWSHADOW_DEFAULT) == 1)
			g_plugin.setByte("WindowShadow", 2);
	}
	else {
		if (g_plugin.getByte("WindowShadow", SETTING_WINDOWSHADOW_DEFAULT) == 2)
			g_plugin.setByte("WindowShadow", 1);
	}
	corecli.pfnLoadCluiGlobalOpts();
}


int CLUI_TestCursorOnBorders()
{
	HWND hwnd = g_clistApi.hwndContactList;
	HCURSOR hCurs1 = nullptr;
	POINT pt;
	int k = 0, fx, fy;
	BOOL mouse_in_window = 0;
	HWND gf = GetForegroundWindow();
	GetCursorPos(&pt);
	HWND hAux = WindowFromPoint(pt);
	if (CLUI_CheckOwnedByClui(hAux)) {
		if (g_bTransparentFlag) {
			if (!bTransparentFocus && gf != hwnd) {
				CLUI_SmoothAlphaTransition(hwnd, g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT), 1);
				bTransparentFocus = 1;
				CLUI_SafeSetTimer(hwnd, TM_AUTOALPHA, 250, nullptr);
			}
		}
	}

	mutex_bIgnoreActivation = 0;

	RECT r;
	GetWindowRect(hwnd, &r);

	// Size borders offset (contract)
	r.top += db_get_dw(0, "ModernSkin", "SizeMarginOffset_Top", SKIN_OFFSET_TOP_DEFAULT);
	r.bottom -= db_get_dw(0, "ModernSkin", "SizeMarginOffset_Bottom", SKIN_OFFSET_BOTTOM_DEFAULT);
	r.left += db_get_dw(0, "ModernSkin", "SizeMarginOffset_Left", SKIN_OFFSET_LEFT_DEFAULT);
	r.right -= db_get_dw(0, "ModernSkin", "SizeMarginOffset_Right", SKIN_OFFSET_RIGHT_DEFAULT);

	if (r.right < r.left) r.right = r.left;
	if (r.bottom < r.top) r.bottom = r.top;

	// End of size borders offset (contract)
	hAux = WindowFromPoint(pt);
	while (hAux != nullptr) {
		if (hAux == hwnd) { mouse_in_window = 1; break; }
		hAux = GetParent(hAux);
	}
	fx = GetSystemMetrics(SM_CXFULLSCREEN);
	fy = GetSystemMetrics(SM_CYFULLSCREEN);
	if (g_CluiData.fDocked || g_CluiData.nBehindEdgeState == 0) {
		if (pt.y <= r.bottom && pt.y >= r.bottom - SIZING_MARGIN && !g_CluiData.fAutoSize) k = 6;
		else if (pt.y >= r.top && pt.y <= r.top + SIZING_MARGIN && !g_CluiData.fAutoSize) k = 3;
		if (pt.x <= r.right && pt.x >= r.right - SIZING_MARGIN && g_CluiData.bBehindEdgeSettings != 2) k += 2;
		else if (pt.x >= r.left && pt.x <= r.left + SIZING_MARGIN && g_CluiData.bBehindEdgeSettings != 1) k += 1;
		if (!(pt.x >= r.left && pt.x <= r.right && pt.y >= r.top && pt.y <= r.bottom)) k = 0;
		k *= mouse_in_window;
		hCurs1 = LoadCursor(nullptr, IDC_ARROW);
		if (g_CluiData.nBehindEdgeState <= 0 && (!(db_get_b(0, "CLUI", "LockSize", SETTING_LOCKSIZE_DEFAULT))))
			switch (k) {
			case 1:
			case 2:
				if (!g_CluiData.fDocked || (g_CluiData.fDocked == 2 && k == 1) || (g_CluiData.fDocked == 1 && k == 2)) { hCurs1 = LoadCursor(nullptr, IDC_SIZEWE); break; }
			case 3: if (!g_CluiData.fDocked) { hCurs1 = LoadCursor(nullptr, IDC_SIZENS); break; }
			case 4: if (!g_CluiData.fDocked) { hCurs1 = LoadCursor(nullptr, IDC_SIZENWSE); break; }
			case 5: if (!g_CluiData.fDocked) { hCurs1 = LoadCursor(nullptr, IDC_SIZENESW); break; }
			case 6: if (!g_CluiData.fDocked) { hCurs1 = LoadCursor(nullptr, IDC_SIZENS); break; }
			case 7: if (!g_CluiData.fDocked) { hCurs1 = LoadCursor(nullptr, IDC_SIZENESW); break; }
			case 8: if (!g_CluiData.fDocked) { hCurs1 = LoadCursor(nullptr, IDC_SIZENWSE); break; }
		}
		if (hCurs1) SetCursor(hCurs1);
		return k;
	}

	return 0;
}

int CLUI_SizingOnBorder(POINT pt, int PerformSize)
{
	if (!(db_get_b(0, "CLUI", "LockSize", SETTING_LOCKSIZE_DEFAULT))) {
		RECT r;
		HWND hwnd = g_clistApi.hwndContactList;
		int sizeOnBorderFlag = 0;
		GetWindowRect(hwnd, &r);

		// Size borders offset (contract)
		r.top += db_get_dw(0, "ModernSkin", "SizeMarginOffset_Top", SKIN_OFFSET_TOP_DEFAULT);
		r.bottom -= db_get_dw(0, "ModernSkin", "SizeMarginOffset_Bottom", SKIN_OFFSET_BOTTOM_DEFAULT);
		r.left += db_get_dw(0, "ModernSkin", "SizeMarginOffset_Left", SKIN_OFFSET_LEFT_DEFAULT);
		r.right -= db_get_dw(0, "ModernSkin", "SizeMarginOffset_Right", SKIN_OFFSET_RIGHT_DEFAULT);

		if (r.right < r.left) r.right = r.left;
		if (r.bottom < r.top) r.bottom = r.top;

		// End of size borders offset (contract)
		if (!g_CluiData.fAutoSize) {
			if (pt.y <= r.bottom && pt.y >= r.bottom - SIZING_MARGIN)
				sizeOnBorderFlag = SCF_BOTTOM;
			else if (pt.y >= r.top && pt.y <= r.top + SIZING_MARGIN)
				sizeOnBorderFlag = SCF_TOP;
		}

		if (pt.x <= r.right && pt.x >= r.right - SIZING_MARGIN)
			sizeOnBorderFlag += SCF_RIGHT;
		else if (pt.x >= r.left && pt.x <= r.left + SIZING_MARGIN)
			sizeOnBorderFlag += SCF_LEFT;

		if (!(pt.x >= r.left && pt.x <= r.right && pt.y >= r.top && pt.y <= r.bottom))  sizeOnBorderFlag = SCF_NONE;

		if (sizeOnBorderFlag && PerformSize) {
			ReleaseCapture();
			SendMessage(hwnd, WM_SYSCOMMAND, SC_SIZE + sizeOnBorderFlag, MAKELPARAM(pt.x, pt.y));
		}
		return sizeOnBorderFlag;
	}
	return SCF_NONE;
}

static int CLUI_SyncSmoothAnimation(WPARAM, LPARAM)
{
	return CLUI_SmoothAlphaThreadTransition();
}

static void CLUI_SmoothAnimationThreadProc(void *param)
{
	if (mutex_bAnimationInProgress) {
		do {
			if (!g_mutex_bLockUpdating) {
				if (MirandaExiting())
					break;

				Sync(CLUI_SyncSmoothAnimation, 0, (LPARAM)param);
				SleepEx(20, TRUE);
				if (MirandaExiting())
					break;
			}
			else SleepEx(0, TRUE);
		} while (mutex_bAnimationInProgress);
	}

	g_hSmoothAnimationThread = nullptr;
}

static int CLUI_SmoothAlphaThreadTransition()
{
	int step = (g_CluiData.bCurrentAlpha > bAlphaEnd) ? -1 * ANIMATION_STEP : ANIMATION_STEP;
	int a = g_CluiData.bCurrentAlpha + step;
	if ((step >= 0 && a >= bAlphaEnd) || (step <= 0 && a <= bAlphaEnd)) {
		mutex_bAnimationInProgress = 0;
		g_CluiData.bCurrentAlpha = bAlphaEnd;
		if (g_CluiData.bCurrentAlpha == 0) {
			g_CluiData.bCurrentAlpha = 1;
			ske_JustUpdateWindowImage();
			mutex_bShowHideCalledFromAnimation = 1;
			CLUI_ShowWindowMod(g_clistApi.hwndContactList, 0);
			Sync(CLUIFrames_OnShowHide, 0);
			mutex_bShowHideCalledFromAnimation = 0;
			g_CluiData.bCurrentAlpha = 0;
			if (!g_CluiData.fLayered) RedrawWindow(g_clistApi.hwndContactList, nullptr, nullptr, RDW_ERASE | RDW_FRAME);
			return 0;
		}
	}
	else g_CluiData.bCurrentAlpha = a;
	ske_JustUpdateWindowImage();
	return 1;
}

int CLUI_SmoothAlphaTransition(HWND hwnd, uint8_t GoalAlpha, BOOL wParam)
{
	if (!g_CluiData.fLayered && (!g_CluiData.fSmoothAnimation && !g_bTransparentFlag)) {
		if (GoalAlpha > 0 && wParam != 2) {
			if (!IsWindowVisible(hwnd)) {
				mutex_bShowHideCalledFromAnimation = 1;
				CLUI_ShowWindowMod(g_clistApi.hwndContactList, SW_RESTORE);
				Sync(CLUIFrames_OnShowHide, 1);
				mutex_bShowHideCalledFromAnimation = 0;
				g_CluiData.bCurrentAlpha = GoalAlpha;
				ske_UpdateWindowImage();

			}
		}
		else if (GoalAlpha == 0 && wParam != 2) {
			if (IsWindowVisible(hwnd)) {
				mutex_bShowHideCalledFromAnimation = 1;
				CLUI_ShowWindowMod(g_clistApi.hwndContactList, 0);
				Sync(CLUIFrames_OnShowHide, 0);
				g_CluiData.bCurrentAlpha = GoalAlpha;
				mutex_bShowHideCalledFromAnimation = 0;

			}
		}
		return 0;
	}
	if (mutex_bShowHideCalledFromAnimation)
		return 0;

	if (wParam != 2) {  // not from timer
		bAlphaEnd = GoalAlpha;
		if (!mutex_bAnimationInProgress) {
			if ((!IsWindowVisible(hwnd) || g_CluiData.bCurrentAlpha == 0) && bAlphaEnd > 0) {
				mutex_bShowHideCalledFromAnimation = 1;
				CLUI_ShowWindowMod(g_clistApi.hwndContactList, SW_SHOWNA);
				Sync(CLUIFrames_OnShowHide, SW_SHOW);
				mutex_bShowHideCalledFromAnimation = 0;
				g_CluiData.bCurrentAlpha = 1;
				ske_UpdateWindowImage();
			}
			if (IsWindowVisible(hwnd) && !g_hSmoothAnimationThread) {
				mutex_bAnimationInProgress = 1;
				if (g_CluiData.fSmoothAnimation)
					g_hSmoothAnimationThread = mir_forkthread(CLUI_SmoothAnimationThreadProc, g_clistApi.hwndContactList);
			}
		}
	}

	int step = (g_CluiData.bCurrentAlpha > bAlphaEnd) ? -1 * ANIMATION_STEP : ANIMATION_STEP;
	int a = g_CluiData.bCurrentAlpha + step;
	if ((step >= 0 && a >= bAlphaEnd) || (step <= 0 && a <= bAlphaEnd) || g_CluiData.bCurrentAlpha == bAlphaEnd || !g_CluiData.fSmoothAnimation) { // stop animation;
		KillTimer(hwnd, TM_SMOTHALPHATRANSITION);
		mutex_bAnimationInProgress = 0;
		if (bAlphaEnd == 0) {
			g_CluiData.bCurrentAlpha = 1;
			ske_UpdateWindowImage();
			mutex_bShowHideCalledFromAnimation = 1;
			CLUI_ShowWindowMod(g_clistApi.hwndContactList, 0);
			Sync(CLUIFrames_OnShowHide, 0);
			mutex_bShowHideCalledFromAnimation = 0;
			g_CluiData.bCurrentAlpha = 0;
		}
		else {
			g_CluiData.bCurrentAlpha = bAlphaEnd;
			ske_UpdateWindowImage();
		}
	}
	else {
		g_CluiData.bCurrentAlpha = a;
		ske_UpdateWindowImage();
	}

	return 0;
}

BOOL cliInvalidateRect(HWND hWnd, CONST RECT *lpRect, BOOL bErase)
{
	if (CLUI_IsInMainWindow(hWnd) && g_CluiData.fLayered) {
		if (IsWindowVisible(hWnd))
			return SkinInvalidateFrame(hWnd, lpRect);

		g_bFullRepaint = true;
		return 0;
	}

	return InvalidateRect(hWnd, lpRect, bErase);
}

static BOOL FileExists(wchar_t *tszFilename)
{
	FILE *f = _wfopen(tszFilename, L"r");
	if (f == nullptr) return FALSE;
	fclose(f);
	return TRUE;
}

HANDLE RegisterIcolibIconHandle(char *szIcoID, char *szSectionName, char *szDescription, wchar_t *tszDefaultFile, int iDefaultIndex, HINSTANCE hDefaultModuleInst, int iDefaultResource)
{
	if (hDefaultModuleInst == nullptr)
		return Skin_GetIconHandle(iDefaultResource);

	wchar_t fileFull[MAX_PATH] = { 0 };

	SKINICONDESC sid = {};
	sid.section.a = szSectionName;
	sid.pszName = szIcoID;
	sid.flags |= SIDF_PATH_UNICODE;
	sid.description.a = szDescription;
	sid.defaultFile.w = fileFull;

	if (tszDefaultFile) {
		PathToAbsoluteW(tszDefaultFile, fileFull);
		if (!FileExists(fileFull))
			fileFull[0] = '\0';
	}

	if (fileFull[0] != '\0')
		sid.iDefaultIndex = -iDefaultIndex;
	else {
		GetModuleFileName(hDefaultModuleInst, fileFull, _countof(fileFull));
		sid.iDefaultIndex = -iDefaultResource;
	}

	return g_plugin.addIcon(&sid);
}

// MAIN WINPROC MESSAGE HANDLERS
LRESULT CLUI::PreProcessWndProc(UINT msg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// proxy CLUI Messages
	LRESULT result = 0;
	if (ProcessCLUIFrameInternalMsg(m_hWnd, msg, wParam, lParam, result)) {
		bHandled = TRUE;
		return result;
	}

	// This registers a window message with RegisterWindowMessage() and then waits for such a message,
	// if it gets it, it tries to open a file mapping object and then maps it to this process space,
	// it expects 256 bytes of data (incl. nullptr) it will then write back the profile it is using the DB to fill in the answer.
	// 
	// The caller is expected to create this mapping object and tell us the ID we need to open ours.
	if (g_CluiData.bSTATE == STATE_EXITING && msg != WM_DESTROY) {
		bHandled = TRUE;
		return 0;
	}

	if (msg == uMsgGetProfile && wParam != 0) { // got IPC message
		int rc = 0;
		char szName[MAX_PATH];
		mir_snprintf(szName, "Miranda::%u", wParam); // caller will tell us the ID of the map
		HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, szName);
		if (hMap != nullptr) {
			void *hView = nullptr;
			hView = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, MAX_PATH);
			if (hView) {
				char szFilePath[MAX_PATH], szProfile[MAX_PATH];
				Profile_GetPathA(MAX_PATH, szFilePath);
				Profile_GetNameA(MAX_PATH, szProfile);
				mir_snprintf((char*)hView, MAX_PATH, "%s\\%s", szFilePath, szProfile);
				UnmapViewOfFile(hView);
				rc = 1;
			}
			CloseHandle(hMap);
		}
		bHandled = TRUE;
		return rc;
	}
	return FALSE;
}


LRESULT CLUI::OnSizingMoving(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (g_CluiData.fLayered) {
		if (msg == WM_SIZING) {
			static int a = 0;
			RECT* wp = (RECT*)lParam;
			if (bNeedFixSizingRect && (rcCorrectSizeRect.bottom != 0 || rcCorrectSizeRect.top != 0)) {
				if (wParam != WMSZ_BOTTOM) wp->bottom = rcCorrectSizeRect.bottom;
				if (wParam != WMSZ_TOP) wp->top = rcCorrectSizeRect.top;
			}
			bNeedFixSizingRect = 0;
			rcSizingRect = *wp;
			mutex_bDuringSizing = 1;
			return 1;
		}

		if (msg == WM_WINDOWPOSCHANGING) {
			RECT work_rect = { 0 };
			RECT temp_rect = { 0 };
			WINDOWPOS *wp = (WINDOWPOS*)lParam;

			RECT rcOldWindowRect;
			GetWindowRect(m_hWnd, &rcOldWindowRect);

			// Прилипание к краям by ZorG
			CLUI::SnappingToEdge(wp);

			if ((rcOldWindowRect.bottom - rcOldWindowRect.top != wp->cy || rcOldWindowRect.right - rcOldWindowRect.left != wp->cx) && !(wp->flags & SWP_NOSIZE)) {
				if (!(wp->flags & SWP_NOMOVE)) {
					work_rect.left = wp->x;
					work_rect.top = wp->y;
				}
				else {
					work_rect.left = rcOldWindowRect.left;
					work_rect.top = rcOldWindowRect.top;
				}
				work_rect.right = work_rect.left + wp->cx;
				work_rect.bottom = work_rect.top + wp->cy;

				// resize frames (batch)
				HDWP PosBatch = BeginDeferWindowPos(1);
				SizeFramesByWindowRect(&work_rect, &PosBatch, 0);

				// Check rect after frames resize
				GetWindowRect(m_hWnd, &temp_rect);

				// Here work_rect should be changed to fit possible changes in cln_listsizechange
				if (bNeedFixSizingRect) {
					work_rect = rcSizingRect;
					wp->x = work_rect.left;
					wp->y = work_rect.top;
					wp->cx = work_rect.right - work_rect.left;
					wp->cy = work_rect.bottom - work_rect.top;
					wp->flags &= ~(SWP_NOMOVE);
				}
				// reposition buttons and new size applying
				ModernSkinButton_ReposButtons(m_hWnd, SBRF_DO_NOT_DRAW, &work_rect);
				ske_PrepareImageButDontUpdateIt(&work_rect);
				g_CluiData.mutexPreventDockMoving = 0;
				ske_UpdateWindowImageRect(&work_rect);
				EndDeferWindowPos(PosBatch);
				g_CluiData.mutexPreventDockMoving = 1;

				Sleep(0);
				mutex_bDuringSizing = 0;
				DefWindowProc(m_hWnd, msg, wParam, lParam);
				return SendMessage(m_hWnd, WM_WINDOWPOSCHANGED, wParam, lParam);
			}
			else SetRect(&rcCorrectSizeRect, 0, 0, 0, 0);

			return DefWindowProc(m_hWnd, msg, wParam, lParam);
		}
	}

	else if (msg == WM_WINDOWPOSCHANGING) {
		// Snaping if it is not in LayeredMode
		WINDOWPOS *wp = (WINDOWPOS *)lParam;
		CLUI::SnappingToEdge(wp);
		return DefWindowProc(m_hWnd, msg, wParam, lParam);
	}
	switch (msg) {
	case WM_DISPLAYCHANGE:
		SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);
		return TRUE;

	case WM_EXITSIZEMOVE:
		{
			int res = DefWindowProc(m_hWnd, msg, wParam, lParam);
			ReleaseCapture();
			TRACE("WM_EXITSIZEMOVE\n");
			SendMessage(m_hWnd, WM_ACTIVATE, (WPARAM)WA_ACTIVE, (LPARAM)m_hWnd);
			return res;
		}

	case WM_SIZING:
		return DefWindowProc(m_hWnd, msg, wParam, lParam);

	case WM_MOVE:
		{
			RECT rc;
			CallWindowProc(DefWindowProc, m_hWnd, msg, wParam, lParam);
			mutex_bDuringSizing = 0;
			GetWindowRect(m_hWnd, &rc);
			CheckFramesPos(&rc);
			Sync(CLUIFrames_OnMoving, m_hWnd, &rc);
			if (!IsIconic(m_hWnd)) {
				if (!Clist_IsDocked()) { // if g_CluiData.fDocked, dont remember pos (except for width)
					g_plugin.setDword("Height", (uint32_t)(rc.bottom - rc.top));
					g_plugin.setDword("x", (uint32_t)rc.left);
					g_plugin.setDword("y", (uint32_t)rc.top);
				}
				g_plugin.setDword("Width", (uint32_t)(rc.right - rc.left));
			}
		}
		return TRUE;

	case WM_SIZE:
		if (g_bSizing) return 0;
		if (wParam != SIZE_MINIMIZED /* &&  IsWindowVisible(m_hWnd)*/) {
			if (g_clistApi.hwndContactList == nullptr)
				return 0;

			if (!g_CluiData.fLayered && !g_CluiData.fDisableSkinEngine)
				ske_ReCreateBackImage(TRUE, nullptr);

			RECT rc;
			GetWindowRect(m_hWnd, &rc);
			CheckFramesPos(&rc);
			ModernSkinButton_ReposButtons(m_hWnd, SBRF_DO_NOT_DRAW, &rc);
			ModernSkinButton_ReposButtons(m_hWnd, SBRF_REDRAW, nullptr);
			if (g_CluiData.fLayered)
				CallService(MS_SKINENG_UPTATEFRAMEIMAGE, (WPARAM)m_hWnd, 0);

			if (!g_CluiData.fLayered) {
				g_bSizing = true;
				Sync(CLUIFrames_OnClistResize_mod, (WPARAM)m_hWnd, 1);
				CLUIFrames_ApplyNewSizes(2);
				CLUIFrames_ApplyNewSizes(1);
				SendMessage(m_hWnd, CLN_LISTSIZECHANGE, 0, 0);
				g_bSizing = false;
			}

			// if g_CluiData.fDocked, dont remember pos (except for width)
			if (!Clist_IsDocked()) {
				g_plugin.setDword("Height", (uint32_t)(rc.bottom - rc.top));
				g_plugin.setDword("x", (uint32_t)rc.left);
				g_plugin.setDword("y", (uint32_t)rc.top);
			}
			else SetWindowRgn(m_hWnd, nullptr, 0);
			g_plugin.setDword("Width", (uint32_t)(rc.right - rc.left));

			if (!g_CluiData.fLayered) {
				HRGN hRgn1;
				RECT r;
				int w = 10;
				GetWindowRect(m_hWnd, &r);
				int h = (r.right - r.left) > (w * 2) ? w : (r.right - r.left);
				int v = (r.bottom - r.top) > (w * 2) ? w : (r.bottom - r.top);
				h = (h < v) ? h : v;
				hRgn1 = CreateRoundRectRgn(0, 0, (r.right - r.left + 1), (r.bottom - r.top + 1), h, h);
				if (db_get_b(0, "CLC", "RoundCorners", SETTING_ROUNDCORNERS_DEFAULT) && !Clist_IsDocked())
					SetWindowRgn(m_hWnd, hRgn1, FALSE);
				else {
					DeleteObject(hRgn1);
					SetWindowRgn(m_hWnd, nullptr, FALSE);
				}
				RedrawWindow(m_hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
		}
		else {
			if (g_plugin.getByte("Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
				CLUI_ShowWindowMod(m_hWnd, SW_HIDE);
				g_plugin.setByte("State", SETTING_STATE_HIDDEN);
			}
			else g_plugin.setByte("State", SETTING_STATE_MINIMIZED);
			SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		}
		return TRUE;
	}
	return 0;
}

LRESULT CLUI::OnThemeChanged(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	xpt_OnWM_THEMECHANGED();
	return FALSE;
}

LRESULT CLUI::OnDwmCompositionChanged(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	g_CluiData.fAeroGlass = false;
	CLUI_UpdateAeroGlass();
	return FALSE;
}

LRESULT CLUI::OnUpdate(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (g_bPostWasCanceled)
		return FALSE;
	return ske_ValidateFrameImageProc(nullptr);
}

LRESULT CLUI::OnInitMenu(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (!CLUI::IsMainMenuInited()) {
		Menu_GetMainMenu();
		CLUI::m_fMainMenuInited = TRUE;
	}
	return FALSE;
}

LRESULT CLUI::OnNcPaint(UINT msg, WPARAM wParam, LPARAM lParam)
{
	int lRes = DefWindowProc(m_hWnd, msg, wParam, lParam);
	if (!g_CluiData.fLayered && db_get_b(0, "CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT)) {
		HDC hdc = nullptr;
		if (msg == WM_PRINT) hdc = (HDC)wParam;
		if (!hdc) hdc = GetWindowDC(m_hWnd);
		CLUI_DrawMenuBackGround(m_hWnd, hdc, 0, 0);
		if (msg != WM_PRINT) ReleaseDC(m_hWnd, hdc);
	}
	return lRes;
}

LRESULT CLUI::OnEraseBkgnd(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return TRUE;
}

LRESULT CLUI::OnNcCreate(UINT msg, WPARAM wParam, LPARAM lParam)
{
	((LPCREATESTRUCT)lParam)->style &= ~(CS_HREDRAW | CS_VREDRAW);
	return DefCluiWndProc(msg, wParam, lParam);
}

LRESULT CLUI::OnPaint(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!g_CluiData.fLayered && IsWindowVisible(m_hWnd)) {
		RECT w = { 0 };
		RECT w2 = { 0 };
		PAINTSTRUCT ps = {};

		GetClientRect(m_hWnd, &w);
		if (!(w.right > 0 && w.bottom > 0))
			return DefWindowProc(m_hWnd, msg, wParam, lParam);

		if (!g_CluiData.fDisableSkinEngine) {
			HDC paintDC = GetDC(m_hWnd);
			w2 = w;
			HDC hdc = CreateCompatibleDC(paintDC);
			HBITMAP hbmp = ske_CreateDIB32(w.right, w.bottom);
			HBITMAP oldbmp = (HBITMAP)SelectObject(hdc, hbmp);
			ske_ReCreateBackImage(FALSE, nullptr);
			BitBlt(paintDC, w2.left, w2.top, w2.right - w2.left, w2.bottom - w2.top, g_pCachedWindow->hBackDC, w2.left, w2.top, SRCCOPY);
			SelectObject(hdc, oldbmp);
			DeleteObject(hbmp);
			DeleteDC(hdc);
			ReleaseDC(m_hWnd, paintDC);
		}
		else {
			HDC hdc = BeginPaint(m_hWnd, &ps);
			ske_BltBackImage(m_hWnd, hdc, &ps.rcPaint);
			ps.fErase = FALSE;
			EndPaint(m_hWnd, &ps);
		}

		ValidateRect(m_hWnd, nullptr);
	}

	if (0 && (db_get_dw(0, "CLUIFrames", "GapBetweenFrames", SETTING_GAPFRAMES_DEFAULT) || db_get_dw(0, "CLUIFrames", "GapBetweenTitleBar", SETTING_GAPTITLEBAR_DEFAULT))) {
		if (IsWindowVisible(m_hWnd)) {
			if (g_CluiData.fLayered)
				SkinInvalidateFrame(m_hWnd, nullptr);
			else {
				RECT w = { 0 };
				RECT w2 = { 0 };
				PAINTSTRUCT ps = {};
				GetWindowRect(m_hWnd, &w);
				OffsetRect(&w, -w.left, -w.top);
				BeginPaint(m_hWnd, &ps);
				if ((ps.rcPaint.bottom - ps.rcPaint.top) * (ps.rcPaint.right - ps.rcPaint.left) == 0)
					w2 = w;
				else
					w2 = ps.rcPaint;
				SkinDrawGlyph(ps.hdc, &w, &w2, "Main,ID=Background,Opt=Non-Layered");
				ps.fErase = FALSE;
				EndPaint(m_hWnd, &ps);
			}
		}
	}
	return DefWindowProc(m_hWnd, msg, wParam, lParam);
}

LRESULT CLUI::OnCreate(UINT, WPARAM, LPARAM)
{
	cliCluiProtocolStatusChanged(0, nullptr);

	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE | MIIM_DATA;
	mii.dwItemData = MENU_MIRANDAMENU;

	mii.fType = MFT_OWNERDRAW;
	mii.dwItemData = MENU_STATUSMENU;
	SetMenuItemInfo(g_clistApi.hMenuMain, 1, TRUE, &mii);

	mii.fType = MFT_OWNERDRAW;
	mii.dwItemData = MENU_MINIMIZE;
	SetMenuItemInfo(g_clistApi.hMenuMain, 2, TRUE, &mii);

	uMsgGetProfile = RegisterWindowMessage(L"Miranda::GetProfile"); // don't localise
	bTransparentFocus = 1;
	return FALSE;
}

LRESULT CLUI::OnSetAllExtraIcons(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return FALSE;
}

LRESULT CLUI::OnCreateClc(UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CreateCLC();
	if (g_plugin.getByte("ShowOnStart", SETTING_SHOWONSTART_DEFAULT))
		cliShowHide(true);
	Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
	return FALSE;
}

LRESULT CLUI::OnLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt = UNPACK_POINT(lParam);
	ClientToScreen(m_hWnd, &pt);

	if (CLUI_SizingOnBorder(pt, 1)) {
		mutex_bIgnoreActivation = TRUE;
		return FALSE;
	}
	return DefCluiWndProc(msg, wParam, lParam);
}

LRESULT CLUI::OnParentNotify(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case WM_LBUTTONDOWN:
		POINT pt = UNPACK_POINT(lParam);
		ClientToScreen(m_hWnd, &pt);
		wParam = 0;
		lParam = 0;

		if (CLUI_SizingOnBorder(pt, 1)) {
			mutex_bIgnoreActivation = TRUE;
			return 0;
		}
	}
	return DefWindowProc(m_hWnd, msg, wParam, lParam);
}

LRESULT CLUI::OnSetFocus(UINT, WPARAM, LPARAM)
{
	if (hFrameContactTree && (!CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLOATING, hFrameContactTree), 0)))
		SetFocus(g_clistApi.hwndContactTree);

	return FALSE;
}

LRESULT CLUI::OnStatusBarUpdateTimer(UINT msg, WPARAM wParam, LPARAM lParam)
{
	for (auto &pt : arTicks) {
		if (!pt->bTimerCreated)
			continue;

		int status;
		if (pt->bGlobal)
			status = g_bMultiConnectionMode ? ID_STATUS_CONNECTING : 0;
		else
			status = Proto_GetStatus(pt->szProto);

		if (!IsStatusConnecting(status)) {
			pt->nCycleStartTick = 0;
			ImageList_Destroy(pt->himlIconList);
			pt->himlIconList = nullptr;
			KillTimer(m_hWnd, TM_STATUSBARUPDATE + pt->nIndex);
			pt->bTimerCreated = 0;
		}
	}

	PROTOTICKS *pt = &arTicks[wParam - TM_STATUSBARUPDATE];

	if (IsWindowVisible(g_clistApi.hwndStatus))
		g_clistApi.pfnInvalidateRect(g_clistApi.hwndStatus, nullptr, 0);
	if (pt->bGlobal)
		Clist_TrayIconUpdateBase(g_szConnectingProto);
	else
		Clist_TrayIconUpdateBase(pt->szProto);

	g_clistApi.pfnInvalidateRect(g_clistApi.hwndStatus, nullptr, TRUE);
	return DefCluiWndProc(msg, wParam, lParam);
}

LRESULT CLUI::OnAutoAlphaTimer(UINT, WPARAM, LPARAM)
{
	int inwnd;

	if (GetForegroundWindow() == m_hWnd) {
		KillTimer(m_hWnd, TM_AUTOALPHA);
		inwnd = TRUE;
	}
	else {
		POINT pt = UNPACK_POINT(GetMessagePos());
		HWND hwndPt = WindowFromPoint(pt);

		inwnd = CLUI_CheckOwnedByClui(hwndPt);
		if (!inwnd)
			inwnd = (GetCapture() == g_clistApi.hwndContactList);
	}

	if (inwnd != bTransparentFocus) {
		// change
		bTransparentFocus = inwnd;
		if (bTransparentFocus)
			CLUI_SmoothAlphaTransition(m_hWnd, (uint8_t)g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT), 1);
		else
			CLUI_SmoothAlphaTransition(m_hWnd, (uint8_t)(g_bTransparentFlag ? g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT) : 255), 1);
	}
	if (!bTransparentFocus)
		KillTimer(m_hWnd, TM_AUTOALPHA);
	return TRUE;
}

LRESULT CLUI::OnSmoothAlphaTransitionTimer(UINT, WPARAM, LPARAM)
{
	CLUI_SmoothAlphaTransition(m_hWnd, 0, 2);
	return TRUE;
}

LRESULT CLUI::OnDelayedSizingTimer(UINT, WPARAM, LPARAM)
{
	if (mutex_bDelayedSizing && !mutex_bDuringSizing) {
		mutex_bDelayedSizing = 0;
		KillTimer(m_hWnd, TM_DELAYEDSIZING);
		Clist_Broadcast(INTM_SCROLLBARCHANGED, 0, 0);
	}
	return TRUE;
}

LRESULT CLUI::OnBringOutTimer(UINT, WPARAM, LPARAM)
{
	// hide
	KillTimer(m_hWnd, TM_BRINGINTIMEOUT);
	KillTimer(m_hWnd, TM_BRINGOUTTIMEOUT);
	bShowEventStarted = 0;
	POINT pt; GetCursorPos(&pt);
	HWND hAux = WindowFromPoint(pt);
	BOOL mouse_in_window = CLUI_CheckOwnedByClui(hAux);
	if (!mouse_in_window && GetForegroundWindow() != m_hWnd)
		CLUI_HideBehindEdge();
	return TRUE;
}

LRESULT CLUI::OnBringInTimer(UINT, WPARAM, LPARAM)
{
	// show
	KillTimer(m_hWnd, TM_BRINGINTIMEOUT);
	bShowEventStarted = 0;
	KillTimer(m_hWnd, TM_BRINGOUTTIMEOUT);
	POINT pt; GetCursorPos(&pt);
	HWND hAux = WindowFromPoint(pt);
	BOOL mouse_in_window = FALSE;
	while (hAux != nullptr) {
		if (hAux == m_hWnd) {
			mouse_in_window = TRUE;
			break;
		}
		hAux = GetParent(hAux);
	}
	if (mouse_in_window)
		CLUI_ShowFromBehindEdge();
	return TRUE;
}

LRESULT CLUI::OnUpdateBringTimer(UINT, WPARAM, LPARAM)
{
	CLUI_UpdateTimer();
	return TRUE;
}

LRESULT CLUI::OnTimer(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (MirandaExiting())
		return FALSE;

	if (wParam >= TM_STATUSBARUPDATE && wParam <= TM_STATUSBARUPDATE + 64) {
		if (!g_clistApi.hwndStatus)
			return FALSE;
		return OnStatusBarUpdateTimer(msg, wParam, lParam);
	}

	switch (wParam) {
	case TM_AUTOALPHA:				return OnAutoAlphaTimer(msg, wParam, lParam);
	case TM_SMOTHALPHATRANSITION:	return OnSmoothAlphaTransitionTimer(msg, wParam, lParam);
	case TM_DELAYEDSIZING:			return OnDelayedSizingTimer(msg, wParam, lParam);
	case TM_BRINGOUTTIMEOUT:		return OnBringOutTimer(msg, wParam, lParam);
	case TM_BRINGINTIMEOUT:			return OnBringInTimer(msg, wParam, lParam);
	case TM_UPDATEBRINGTIMER:		return OnUpdateBringTimer(msg, wParam, lParam);
	}
	return DefCluiWndProc(msg, wParam, lParam);
}


LRESULT CLUI::OnActivate(UINT msg, WPARAM wParam, LPARAM lParam)
{
	SetCursor(LoadCursor(nullptr, IDC_ARROW));
	SendMessage(g_clistApi.hwndContactTree, WM_ACTIVATE, wParam, lParam);
	if (db_get_b(0, "ModernData", "HideBehind", SETTING_HIDEBEHIND_DEFAULT)) {
		if (wParam == WA_INACTIVE && ((HWND)lParam != m_hWnd) && GetParent((HWND)lParam) != m_hWnd) {
			if (!g_bCalledFromShowHide) CLUI_UpdateTimer();
		}
		else if (!g_bCalledFromShowHide)
			CLUI_ShowFromBehindEdge();
	}

	if (!IsWindowVisible(m_hWnd) || mutex_bShowHideCalledFromAnimation) {
		KillTimer(m_hWnd, TM_AUTOALPHA);
		return 0;
	}

	if (wParam == WA_INACTIVE && ((HWND)lParam != m_hWnd) && !CLUI_CheckOwnedByClui((HWND)lParam)) {
		if (g_bTransparentFlag && bTransparentFocus)
			CLUI_SafeSetTimer(m_hWnd, TM_AUTOALPHA, 250, nullptr);
	}
	else {
		if (!g_plugin.getByte("OnTop", SETTING_ONTOP_DEFAULT))
			Sync(CLUIFrames_ActivateSubContainers, TRUE);
		if (g_bTransparentFlag) {
			KillTimer(m_hWnd, TM_AUTOALPHA);
			CLUI_SmoothAlphaTransition(m_hWnd, g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT), 1);
			bTransparentFocus = 1;
		}
	}
	RedrawWindow(m_hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
	if (g_bTransparentFlag) {
		uint8_t alpha;
		if (wParam != WA_INACTIVE || CLUI_CheckOwnedByClui((HWND)lParam) || ((HWND)lParam == m_hWnd) || GetParent((HWND)lParam) == m_hWnd)
			alpha = g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT);
		else
			alpha = g_bTransparentFlag ? g_plugin.getByte("AutoAlpha", SETTING_AUTOALPHA_DEFAULT) : 255;
		CLUI_SmoothAlphaTransition(m_hWnd, alpha, 1);
		return 1;
	}
	return DefWindowProc(m_hWnd, msg, wParam, lParam);
}

LRESULT CLUI::OnSetCursor(UINT, WPARAM, LPARAM)
{
	HWND gf = GetForegroundWindow();
	if (g_CluiData.nBehindEdgeState >= 0)  CLUI_UpdateTimer();
	if (g_bTransparentFlag) {
		if (!bTransparentFocus && gf != m_hWnd) {
			CLUI_SmoothAlphaTransition(m_hWnd, g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT), 1);
			bTransparentFocus = 1;
			CLUI_SafeSetTimer(m_hWnd, TM_AUTOALPHA, 250, nullptr);
		}
	}
	int k = CLUI_TestCursorOnBorders();
	return k ? k : 1;
}

LRESULT CLUI::OnMouseActivate(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (mutex_bIgnoreActivation) {
		mutex_bIgnoreActivation = 0;
		return MA_NOACTIVATEANDEAT;
	}

	int lRes = DefWindowProc(m_hWnd, msg, wParam, lParam);
	CLUIFrames_RepaintSubContainers();
	return lRes;
}

LRESULT CLUI::OnNcLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT pt = UNPACK_POINT(lParam);
	int k = CLUI_SizingOnBorder(pt, 1);
	return k ? k : DefWindowProc(m_hWnd, msg, wParam, lParam);
}

LRESULT CLUI::OnNcLButtonDblClk(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == HTMENU || wParam == HTCAPTION) {
		RECT rc;
		GetWindowRect(m_hWnd, &rc);

		POINT pt = UNPACK_POINT(lParam);
		if (pt.x > rc.right - 16 && pt.x < rc.right)
			return g_clistApi.pfnShowHide();
	}
	return DefCluiWndProc(msg, wParam, lParam);
}

LRESULT CLUI::OnNcHitTest(UINT, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = DefWindowProc(m_hWnd, WM_NCHITTEST, wParam, lParam);

	if (g_CluiData.fAutoSize)
		if (result == HTSIZE || result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT || result == HTBOTTOM || result == HTBOTTOMRIGHT || result == HTBOTTOMLEFT)
			return HTCLIENT;

	if (result == HTMENU) {
		POINT pt = UNPACK_POINT(lParam);
		int t = MenuItemFromPoint(m_hWnd, g_clistApi.hMenuMain, pt);
		if (t == -1 && (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)))
			return HTCAPTION;
	}

	if (result == HTCLIENT) {
		POINT pt = UNPACK_POINT(lParam);
		int k = CLUI_SizingOnBorder(pt, 0);
		if (!k && (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)))
			return HTCAPTION;
		else return k + 9;
	}
	return result;
}

LRESULT CLUI::OnShowWindow(UINT, WPARAM wParam, LPARAM lParam)
{
	if (lParam) return 0;
	if (mutex_bShowHideCalledFromAnimation) return 1;

	uint8_t gAlpha = (!wParam) ? 0 : (g_plugin.getByte("Transparent", SETTING_TRANSPARENT_DEFAULT) ? g_plugin.getByte("Alpha", SETTING_ALPHA_DEFAULT) : 255);
	if (wParam) {
		g_CluiData.bCurrentAlpha = 0;
		Sync(CLUIFrames_OnShowHide, 1);
		ske_RedrawCompleteWindow();
	}
	CLUI_SmoothAlphaTransition(m_hWnd, gAlpha, 1);
	return FALSE;
}

LRESULT CLUI::OnSysCommand(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case SC_MAXIMIZE:
		return 0;

	case SC_CLOSE:
		PostMessage(m_hWnd, msg, SC_MINIMIZE, lParam);
		return 0;
	}

	DefWindowProc(m_hWnd, msg, wParam, lParam);
	if (g_plugin.getByte("OnDesktop", SETTING_ONDESKTOP_DEFAULT))
		Sync(CLUIFrames_ActivateSubContainers, TRUE);
	return FALSE;
}

LRESULT CLUI::OnKeyDown(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_F5)
		Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
	return DefCluiWndProc(msg, wParam, lParam);
}

LRESULT CLUI::OnGetMinMaxInfo(UINT msg, WPARAM wParam, LPARAM lParam)
{
	DefWindowProc(m_hWnd, msg, wParam, lParam);
	((LPMINMAXINFO)lParam)->ptMinTrackSize.x = max(db_get_w(0, "CLUI", "MinWidth", SETTING_MINWIDTH_DEFAULT), max(18, db_get_b(0, "CLUI", "LeftClientMargin", SETTING_LEFTCLIENTMARIGN_DEFAULT) + db_get_b(0, "CLUI", "RightClientMargin", SETTING_RIGHTCLIENTMARIGN_DEFAULT) + 18));
	if (nRequiredHeight == 0)
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = CLUIFramesGetMinHeight();

	return FALSE;
}

LRESULT CLUI::OnMoving(UINT msg, WPARAM wParam, LPARAM lParam)
{
	CallWindowProc(DefWindowProc, m_hWnd, msg, wParam, lParam);
	if (FALSE)  // showcontents is turned on
		Sync(CLUIFrames_OnMoving, m_hWnd, (RECT*)lParam);

	return TRUE;
}

LRESULT CLUI::OnListSizeChangeNotify(NMCLISTCONTROL *pnmc)
{
	// TODO: Check and refactor possible problem of clist resized to full screen problem
	static RECT rcWindow, rcTree, rcTree2, rcWorkArea, rcOld;
	int maxHeight, minHeight, newHeight;
	int winstyle;
	if (mutex_bDisableAutoUpdate)
		return FALSE;
	if (mutex_bDuringSizing)
		rcWindow = rcSizingRect;
	else
		GetWindowRect(m_hWnd, &rcWindow);
	if (!g_CluiData.fAutoSize || g_clistApi.hwndContactTree == nullptr || Clist_IsDocked())
		return FALSE;

	maxHeight = db_get_b(0, "CLUI", "MaxSizeHeight", SETTING_MAXSIZEHEIGHT_DEFAULT);
	minHeight = db_get_b(0, "CLUI", "MinSizeHeight", SETTING_MINSIZEHEIGHT_DEFAULT);
	rcOld = rcWindow;
	GetWindowRect(g_clistApi.hwndContactTree, &rcTree);

	FRAMEWND *frm = FindFrameByItsHWND(g_clistApi.hwndContactTree);
	if (frm)
		rcTree2 = frm->wndSize;
	else
		SetRect(&rcTree2, 0, 0, 0, 0);

	winstyle = GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
	HMONITOR hMon = MonitorFromWindow(g_clistApi.hwndContactTree, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { 0 };
	if (GetMonitorInfo(hMon, &mi))
		rcWorkArea = mi.rcWork;

	if (pnmc->pt.y > (rcWorkArea.bottom - rcWorkArea.top))
		pnmc->pt.y = (rcWorkArea.bottom - rcWorkArea.top);

	newHeight = max(CLUIFramesGetMinHeight(), max(pnmc->pt.y, 3) + 1 + ((winstyle & WS_BORDER) ? 2 : 0) + (rcWindow.bottom - rcWindow.top) - (rcTree.bottom - rcTree.top));
	if (newHeight < (rcWorkArea.bottom - rcWorkArea.top) * minHeight / 100)
		newHeight = (rcWorkArea.bottom - rcWorkArea.top) * minHeight / 100;

	if (newHeight > (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100)
		newHeight = (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100;

	// if nothing was changed - return
	if (newHeight == (rcWindow.bottom - rcWindow.top))
		return 0;

	if (db_get_b(0, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT)) {
		rcWindow.top = rcWindow.bottom - newHeight;
		if (rcWindow.top < rcWorkArea.top)
			rcWindow.top = rcWorkArea.top;
	}
	else {
		rcWindow.bottom = rcWindow.top + newHeight;
		if (rcWindow.bottom > rcWorkArea.bottom)
			rcWindow.bottom = rcWorkArea.bottom;
	}

	if (nRequiredHeight == 1) {
		nLastRequiredHeight = newHeight;
		return FALSE;
	}

	nRequiredHeight = 1;
	if (mutex_bDuringSizing) {
		bNeedFixSizingRect = 1;
		rcSizingRect.top = rcWindow.top;
		rcSizingRect.bottom = rcWindow.bottom;
		rcCorrectSizeRect = rcSizingRect;
	}
	else bNeedFixSizingRect = 0;

	SetWindowPos(m_hWnd, nullptr, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOZORDER | SWP_NOACTIVATE);

	// during total resize contact list tree's height was somehow changed
	/* if (nLastRequiredHeight != newHeight)
		SetWindowPos(m_hWnd, nullptr, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, nLastRequiredHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		*/

	nRequiredHeight = 0;
	return FALSE;
}

LRESULT CLUI::OnClickNotify(NMCLISTCONTROL *pnmc)
{
	uint32_t hitFlags;
	HANDLE hItem = (HANDLE)SendMessage(g_clistApi.hwndContactTree, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(pnmc->pt.x, pnmc->pt.y));
	if (hItem && !(hitFlags & CLCHT_NOWHERE))
		return DefCluiWndProc(WM_NOTIFY, 0, (LPARAM)pnmc);

	if ((hitFlags & (CLCHT_NOWHERE | CLCHT_INLEFTMARGIN | CLCHT_BELOWITEMS)) == 0)
		return DefCluiWndProc(WM_NOTIFY, 0, (LPARAM)pnmc);

	if (db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
		POINT pt;
		int res;
		pt = pnmc->pt;
		ClientToScreen(g_clistApi.hwndContactTree, &pt);
		res = PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		return res;
	}

	if (db_get_b(0, "CLUI", "DragToScroll", SETTING_DRAGTOSCROLL_DEFAULT) && !db_get_b(0, "CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT))
		return ClcEnterDragToScroll(g_clistApi.hwndContactTree, pnmc->pt.y);

	return 0;
}

LRESULT CLUI::OnNotify(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmhdr = (LPNMHDR)lParam;
	if (pnmhdr->hwndFrom != g_clistApi.hwndContactTree)
		return DefCluiWndProc(msg, wParam, lParam);

	switch (pnmhdr->code) {
	case CLN_LISTSIZECHANGE: return OnListSizeChangeNotify((NMCLISTCONTROL *)pnmhdr);
	case NM_CLICK:           return OnClickNotify((NMCLISTCONTROL *)pnmhdr);
	}
	return DefCluiWndProc(msg, wParam, lParam);
}

LRESULT CLUI::OnContextMenu(UINT, WPARAM, LPARAM lParam)
{
	POINT pt = UNPACK_POINT(lParam);
	// x/y might be -1 if it was generated by a kb click
	RECT rc;
	GetWindowRect(g_clistApi.hwndContactTree, &rc);
	if (pt.x == -1 && pt.y == -1) {
		// all this is done in screen-coords!
		GetCursorPos(&pt);
		// the mouse isnt near the window, so put it in the middle of the window
		if (!PtInRect(&rc, pt)) {
			pt.x = rc.left + (rc.right - rc.left) / 2;
			pt.y = rc.top + (rc.bottom - rc.top) / 2;
		}
	}
	if (PtInRect(&rc, pt)) {
		HMENU hMenu = Menu_BuildGroupMenu();
		TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hWnd, nullptr);
		Menu_DestroyNestedMenu(hMenu);
	}
	return FALSE;
}

LRESULT CLUI::OnMeasureItem(UINT, WPARAM, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT)lParam;
	switch (pmis->itemData) {
	case MENU_MIRANDAMENU:
		pmis->itemWidth = GetSystemMetrics(SM_CXSMICON) * 4 / 3;
		pmis->itemHeight = 0;
		return TRUE;

	case MENU_STATUSMENU:
		HDC hdc = GetDC(m_hWnd);
		wchar_t *ptszStr = TranslateT("Status");
		SIZE textSize;
		GetTextExtentPoint32(hdc, ptszStr, (int)mir_wstrlen(ptszStr), &textSize);
		pmis->itemWidth = textSize.cx;
		pmis->itemHeight = 0;
		ReleaseDC(m_hWnd, hdc);
		return TRUE;
	}
	return Menu_MeasureItem(lParam);
}

LRESULT CLUI::OnDrawItem(UINT, WPARAM, LPARAM lParam)
{
	ClcData *dat = (ClcData*)GetWindowLongPtr(g_clistApi.hwndContactTree, 0);
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
	if (!dat || dis->CtlType != ODT_MENU)
		return 0;

	if (dis->itemData == MENU_MIRANDAMENU) {
		if (!g_CluiData.fLayered) {
			char buf[255];
			short offset = 1 + (dis->itemState & ODS_SELECTED ? 1 : 0) - (dis->itemState & ODS_HOTLIGHT ? 1 : 0);

			HICON hIcon = Skin_LoadIcon(SKINICON_OTHER_MAINMENU);

			CLUI_DrawMenuBackGround(m_hWnd, dis->hDC, 1, dis->itemState);
			mir_snprintf(buf, "Main,ID=MainMenu,Selected=%s,Hot=%s", (dis->itemState & ODS_SELECTED) ? "True" : "False", (dis->itemState & ODS_HOTLIGHT) ? "True" : "False");
			SkinDrawGlyph(dis->hDC, &dis->rcItem, &dis->rcItem, buf);

			int x = (dis->rcItem.right + dis->rcItem.left - GetSystemMetrics(SM_CXSMICON)) / 2 + offset;
			int y = (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2 + offset;

			DrawState(dis->hDC, nullptr, nullptr, (LPARAM)hIcon, 0, x, y, 0, 0,
				DST_ICON | (dis->itemState & ODS_INACTIVE && (((FALSE))) ? DSS_DISABLED : DSS_NORMAL));

			IcoLib_ReleaseIcon(hIcon);
			nMirMenuState = dis->itemState;
		}
		else {
			nMirMenuState = dis->itemState;
			g_clistApi.pfnInvalidateRect(m_hWnd, nullptr, 0);
		}
		return TRUE;
	}

	if (dis->itemData == MENU_STATUSMENU) {
		if (!g_CluiData.fLayered) {
			char buf[255] = { 0 };
			RECT rc = dis->rcItem;
			short dx = 1 + (dis->itemState & ODS_SELECTED ? 1 : 0) - (dis->itemState & ODS_HOTLIGHT ? 1 : 0);
			if (dx > 1) {
				rc.left += dx;
				rc.top += dx;
			}
			else if (dx == 0) {
				rc.right -= 1;
				rc.bottom -= 1;
			}
			CLUI_DrawMenuBackGround(m_hWnd, dis->hDC, 2, dis->itemState);
			SetBkMode(dis->hDC, TRANSPARENT);
			mir_snprintf(buf, "Main,ID=StatusMenu,Selected=%s,Hot=%s", (dis->itemState & ODS_SELECTED) ? "True" : "False", (dis->itemState & ODS_HOTLIGHT) ? "True" : "False");
			SkinDrawGlyph(dis->hDC, &dis->rcItem, &dis->rcItem, buf);
			SetTextColor(dis->hDC, (dis->itemState & ODS_SELECTED/*|dis->itemState & ODS_HOTLIGHT*/) ? dat->MenuTextHiColor : dat->MenuTextColor);
			DrawText(dis->hDC, TranslateT("Status"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			nStatusMenuState = dis->itemState;
		}
		else {
			nStatusMenuState = dis->itemState;
			g_clistApi.pfnInvalidateRect(m_hWnd, nullptr, 0);
		}
		return TRUE;
	}

	if (dis->itemData == MENU_MINIMIZE && !g_CluiData.fLayered) {
		//TODO check if caption is visible
		char buf[255] = { 0 };
		short dx = 1 + (dis->itemState & ODS_SELECTED ? 1 : 0) - (dis->itemState & ODS_HOTLIGHT ? 1 : 0);
		HICON hIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		CLUI_DrawMenuBackGround(m_hWnd, dis->hDC, 3, dis->itemState);
		mir_snprintf(buf, "Main,ID=MainMenu,Selected=%s,Hot=%s", (dis->itemState & ODS_SELECTED) ? "True" : "False", (dis->itemState & ODS_HOTLIGHT) ? "True" : "False");
		SkinDrawGlyph(dis->hDC, &dis->rcItem, &dis->rcItem, buf);
		DrawState(dis->hDC, nullptr, nullptr, (LPARAM)hIcon, 0, (dis->rcItem.right + dis->rcItem.left - GetSystemMetrics(SM_CXSMICON)) / 2 + dx, (dis->rcItem.bottom + dis->rcItem.top - GetSystemMetrics(SM_CYSMICON)) / 2 + dx, 0, 0, DST_ICON);
		IcoLib_ReleaseIcon(hIcon);
		nMirMenuState = dis->itemState;
	}

	return Menu_DrawItem(lParam);
}

LRESULT CLUI::OnDestroy(UINT, WPARAM, LPARAM)
{
	int state = g_plugin.getByte("State", SETTING_STATE_NORMAL);
	BOOL wait = FALSE;

	AniAva_UnloadModule();
	TRACE("CLUI.c: WM_DESTROY\n");
	g_CluiData.bSTATE = STATE_EXITING;
	CLUI_DisconnectAll();
	//fire the "Away Message" Event to wake the thread so it can die.
	//fire the "Get Text Async" Event to wake the thread so it can die.
	if (amWakeThread())
		wait = TRUE;

	if (wait) {
		//need to give them a little time to exit.
		Sleep(50);
	}

	TRACE("CLUI.c: WM_DESTROY - WaitThreadsCompletion \n");
	while (CLUI_WaitThreadsCompletion()); // stop all my threads
	TRACE("CLUI.c: WM_DESTROY - WaitThreadsCompletion DONE\n");

	arTicks.destroy();

	if (state == SETTING_STATE_NORMAL)
		CLUI_ShowWindowMod(m_hWnd, SW_HIDE);
	UnLoadContactListModule();

	RemoveMenu(g_clistApi.hMenuMain, 0, MF_BYPOSITION);
	RemoveMenu(g_clistApi.hMenuMain, 0, MF_BYPOSITION);
	DestroyMenu(g_clistApi.hMenuMain);

	Clist_TrayIconDestroy(m_hWnd);
	mutex_bAnimationInProgress = 0;
	CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)hFrameContactTree, 0);
	TRACE("CLUI.c: WM_DESTROY - hFrameContactTree removed\n");
	g_clistApi.hwndContactTree = nullptr;
	g_clistApi.hwndStatus = nullptr;

	if (g_CluiData.fAutoSize && !g_CluiData.fDocked) {
		RECT r;
		GetWindowRect(g_clistApi.hwndContactList, &r);
		if (db_get_b(0, "CLUI", "AutoSizeUpward", SETTING_AUTOSIZEUPWARD_DEFAULT))
			r.top = r.bottom - CLUIFrames_GetTotalHeight();
		else
			r.bottom = r.top + CLUIFrames_GetTotalHeight();
		g_plugin.setDword("y", r.top);
		g_plugin.setDword("Height", r.bottom - r.top);
	}

	UnLoadCLUIFramesModule();
	//ExtFrames_Uninit();
	TRACE("CLUI.c: WM_DESTROY - UnLoadCLUIFramesModule DONE\n");
	g_plugin.setByte("State", (uint8_t)state);
	ske_UnloadSkin(&g_SkinObjectList);

	delete m_pCLUI;

	g_clistApi.hwndContactList = nullptr;
	g_clistApi.hwndStatus = nullptr;
	PostQuitMessage(0);
	return 0;
}
