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
 * $Id: clui.cpp 138 2010-11-01 10:51:15Z silvercircle $
 *
 */

#include <commonheaders.h>
#include <shobjidl.h>
#include <m_findadd.h>
#include <m_icq.h>
#include "../coolsb/coolscroll.h"

LRESULT (CALLBACK* CLUI::saveContactListWndProc )(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

int 			CLUI::fading_active = 0;
HPEN			CLUI::hPenFrames = 0;
WNDPROC 		CLUI::OldStatusBarProc = 0;
bool			CLUI::fInSizing = false;
RECT			CLUI::newRect = {0};
RECT 			CLUI::rcWPC = {0};
RECT			CLUI::cluiPos = {0};
HIMAGELIST 		CLUI::hExtraImages = 0;
TImageItem* 	CLUI::bgImageItem = 0, *CLUI::bgImageItem_nonAero = 0, *CLUI::bgClientItem = 0;
TButtonItem* 	CLUI::buttonItems = 0;

#ifdef _USE_D2D
	ID2D1HwndRenderTarget*	CLUI::renderTarget = 0;
#endif
UPDATELAYEREDWINDOWINFO CLUI::ulwInfo = {0};
BLENDFUNCTION			CLUI::bf = {0};

static RECT g_PreSizeRect;
static LONG g_CLUI_x_off, g_CLUI_y_off, g_CLUI_y1_off, g_CLUI_x1_off;

static int transparentFocus = 1;
static byte oldhideoffline;
static int disableautoupd = 1;
HANDLE hFrameContactTree;
extern PLUGININFOEX pluginInfo;

extern BOOL g_trayTooltipActive;
extern POINT tray_hover_pos;
extern HWND g_hwndViewModeFrame, g_hwndEventArea;

extern HBRUSH g_CLUISkinnedBkColor;
extern HWND g_hwndSFL;
extern COLORREF g_CLUISkinnedBkColorRGB;
extern FRAMEWND *wndFrameCLC;

static BYTE old_cliststate;

wchar_t *statusNames[12];

extern HANDLE hNotifyFrame;

void FLT_ShowHideAll(int showCmd);
void FLT_SnapToEdges(HWND hwnd);
void DestroyTrayMenu(HMENU hMenu);

extern HANDLE hSoundHook;
extern HANDLE hIcoLibChanged;
extern HANDLE hExtraImageListRebuilding, hExtraImageApplying;

SIZE g_oldSize = {0};
POINT g_oldPos = {0};
extern int dock_prevent_moving;

static HDC hdcLockedPoint = 0;
static HBITMAP hbmLockedPoint = 0, hbmOldLockedPoint = 0;

HICON overlayicons[10];

struct CluiTopButton top_buttons[] = {
	0, 0, 0, IDC_TBTOPMENU, IDI_TBTOPMENU, 0,           "CLN_topmenu", NULL, TOPBUTTON_PUSH | TOPBUTTON_SENDONDOWN, 1, LPGENT("Show menu"),
	0, 0, 0, IDC_TBHIDEOFFLINE, IDI_HIDEOFFLINE, 0,     "CLN_online", NULL, 0, 2, LPGENT("Show / hide offline contacts"),
	0, 0, 0, IDC_TBHIDEGROUPS, IDI_HIDEGROUPS, 0,       "CLN_groups", NULL, 0, 4, LPGENT("Toggle group mode"),
	0, 0, 0, IDC_TBFINDANDADD, IDI_FINDANDADD, 0,       "CLN_findadd", NULL, TOPBUTTON_PUSH, 8, LPGENT("Find and add contacts"),
	0, 0, 0, IDC_TBACCOUNTS, IDI_TBACCOUNTS, 0,         "CLN_accounts", NULL, TOPBUTTON_PUSH, 8192, LPGENT("Accounts"),
	0, 0, 0, IDC_TBOPTIONS, IDI_TBOPTIONS, 0,           "CLN_options", NULL, TOPBUTTON_PUSH, 16, LPGENT("Open preferences"),
	0, 0, 0, IDC_TBSOUND, IDI_SOUNDSON, IDI_SOUNDSOFF,  "CLN_sound", "CLN_soundsoff", 0, 32, LPGENT("Toggle sounds"),
	0, 0, 0, IDC_TBMINIMIZE, IDI_MINIMIZE, 0,           "CLN_minimize", NULL, TOPBUTTON_PUSH, 64, LPGENT("Minimize contact list"),
	0, 0, 0, IDC_TBTOPSTATUS, 0, 0,                     "CLN_topstatus", NULL, TOPBUTTON_PUSH  | TOPBUTTON_SENDONDOWN, 128, LPGENT("Status menu"),
	0, 0, 0, IDC_TABSRMMSLIST, IDI_TABSRMMSESSIONLIST, 0, "CLN_slist", NULL, TOPBUTTON_PUSH | TOPBUTTON_SENDONDOWN, 256, LPGENT("tabSRMM session list"),
	0, 0, 0, IDC_TABSRMMMENU, IDI_TABSRMMMENU, 0,       "CLN_menu", NULL, TOPBUTTON_PUSH | TOPBUTTON_SENDONDOWN, 512, LPGENT("tabSRMM Menu"),

	0, 0, 0, IDC_TBSELECTVIEWMODE, 0, 0,  				"CLN_CLVM_select", NULL, TOPBUTTON_PUSH | TOPBUTTON_SENDONDOWN, 1024, LPGENT("Select view mode"),
	0, 0, 0, IDC_TBCONFIGUREVIEWMODE, 0, 0, 			"CLN_CLVM_options", NULL, TOPBUTTON_PUSH, 2048, LPGENT("Setup view modes"),
	0, 0, 0, IDC_TBCLEARVIEWMODE, 0, 0,        			"CLN_CLVM_reset", NULL, TOPBUTTON_PUSH, 4096, LPGENT("Clear view mode"),

	0, 0, 0, IDC_TBGLOBALSTATUS, 0, 0, "", NULL, TOPBUTTON_PUSH | TOPBUTTON_SENDONDOWN, 0, LPGENT("Set status modes"),
	0, 0, 0, IDC_TBMENU, IDI_MINIMIZE, 0, "", NULL, TOPBUTTON_PUSH | TOPBUTTON_SENDONDOWN, 0, LPGENT("Open main menu"),
	(HWND) - 1, 0, 0, 0, 0, 0, 0, 0, 0
};

IconItemT myIcons[] = {
	{ LPGENT("Toggle show online/offline"), "CLN_online", IDI_HIDEOFFLINE },
	{ LPGENT("Toggle groups"), "CLN_groups", IDI_HIDEGROUPS },
	{ LPGENT("Find contacts"), "CLN_findadd", IDI_FINDANDADD },
	{ LPGENT("Open preferences"), "CLN_options", IDI_TBOPTIONS },
	{ LPGENT("Toggle sounds"), "CLN_sound", IDI_SOUNDSON },
	{ LPGENT("Minimize contact list"), "CLN_minimize", IDI_MINIMIZE },
	{ LPGENT("Show TabSRMM session list"), "CLN_slist", IDI_TABSRMMSESSIONLIST },
	{ LPGENT("Show TabSRMM menu"), "CLN_menu", IDI_TABSRMMMENU },
	{ LPGENT("Sounds are off"), "CLN_soundsoff", IDI_SOUNDSOFF },
	{ LPGENT("Select view mode"), "CLN_CLVM_select", IDI_CLVM_SELECT },
	{ LPGENT("Reset view mode"), "CLN_CLVM_reset", IDI_DELETE },
	{ LPGENT("Configure view modes"), "CLN_CLVM_options", IDI_CLVM_OPTIONS },
	{ LPGENT("Show menu"), "CLN_topmenu", IDI_TBTOPMENU },
	{ LPGENT("Setup accounts"), "CLN_accounts", IDI_TBACCOUNTS }
};

void CLUI::Tweak_It(const COLORREF clr)
{
#ifndef _USE_D2D
	if(!cfg::isAero)
		SetLayeredWindowAttributes(pcli->hwndContactList, clr, 0, LWA_COLORKEY);
#endif
	cfg::dat.colorkey = clr;
}

void CLUI::layoutButtons(HWND hwnd, RECT *rc)
{
	RECT rect;
	BYTE left_offset = Skin::metrics.cLeft - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0);
	BYTE right_offset = Skin::metrics.cRight - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0);
	BYTE delta = left_offset + right_offset;
	TButtonItem *btnItems = buttonItems;
	HDWP dwp = 0;
	bool fFrame = Skin::metrics.fHaveFrame;

	if (rc == NULL)
		GetClientRect(hwnd, &rect);
	else
		rect = *rc;

	rect.bottom -= Skin::metrics.cBottom;

	if (buttonItems || cfg::dat.dwFlags && CLUI_FRAME_SHOWBOTTOMBUTTONS) {
		LONG x, y;
		dwp = BeginDeferWindowPos(10);

		while (btnItems) {
			if(btnItems->dwFlags & BUTTON_FRAMELESS_ONLY) {
				if(Skin::metrics.fHaveFrame && IsWindowVisible(btnItems->hWnd))
					ShowWindow(btnItems->hWnd, SW_HIDE);
				else if(!Skin::metrics.fHaveFrame && !IsWindowVisible(btnItems->hWnd))
					ShowWindow(btnItems->hWnd, SW_SHOW);
			}
			fFrame = Skin::metrics.fHaveFrame || btnItems->dwFlags & BUTTON_FRAMELESS_ONLY;

			x = (btnItems->xOff >= 0) ? rect.left + btnItems->xOff + (!fFrame ? Skin::metrics.cFakeLeftBorder : 0) :
				 rect.right - abs(btnItems->xOff) - (!fFrame ? Skin::metrics.cFakeRightBorder : 0);
			if(btnItems->dwFlags & BUTTON_VALIGN_B)
				y = rect.bottom - Skin::metrics.bSBarHeight - abs(btnItems->yOff) - (!fFrame ? Skin::metrics.cFakeBtmBorder : 0);
			else
				y = rect.top + btnItems->yOff + (!fFrame ? Skin::metrics.cFakeCaption : 0);

			DeferWindowPos(dwp, btnItems->hWnd, 0, x, y, btnItems->width, btnItems->height,
						 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
			btnItems = btnItems->nextItem;
		}
		DeferWindowPos(dwp, top_buttons[15].hwnd, 0, 2 + left_offset, rect.bottom - Skin::metrics.bSBarHeight - Skin::metrics.cButtonHeight - 1,
					 BUTTON_WIDTH_D * 3, Skin::metrics.cButtonHeight + 1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
		DeferWindowPos(dwp, top_buttons[14].hwnd, 0, left_offset + (3 * BUTTON_WIDTH_D) + 3, rect.bottom - Skin::metrics.bSBarHeight - Skin::metrics.cButtonHeight - 1,
					 rect.right - delta - (3 * BUTTON_WIDTH_D + 5), Skin::metrics.cButtonHeight + 1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

		EndDeferWindowPos(dwp);
		return;
	}
}

static int FS_FontsChanged(WPARAM wParam, LPARAM lParam)
{
	pcli->pfnClcOptionsChanged();
	CLUI::Redraw();
	return 0;
}

/*
* create the CLC control, but not yet the frame. The frame containing the CLC should be created as the
* last frame of all.
*/

HWND CLUI::preCreateCLC(HWND parent)
{
	pcli->hwndContactTree = CreateWindow(_T(CLISTCONTROL_CLASS), L"",
										 WS_CHILD | CLS_CONTACTLIST
										 | (cfg::getByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) ? CLS_USEGROUPS : 0)
										 | CLS_HIDEOFFLINE
										 //|(DBGetContactSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT)?CLS_HIDEOFFLINE:0)
										 | (cfg::getByte(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) ? CLS_HIDEEMPTYGROUPS : 0)
										 | CLS_MULTICOLUMN
										 , 0, 0, 0, 0, parent, NULL, g_hInst, (LPVOID)0xff00ff00);

	cfg::clcdat = (struct ClcData *)GetWindowLongPtr(pcli->hwndContactTree, 0);

	return pcli->hwndContactTree;
}

/*
* create internal frames, including the last frame (actual CLC control)
*/

int CLUI::createCLC(HWND parent)
{
	reloadExtraIcons();
	CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM)oldhideoffline, 0);
	disableautoupd = 0;
	{
		CLISTFrame frame = {0};
		frame.cbSize = sizeof(frame);
		frame.tname = _T("EventArea");
		frame.TBtname = TranslateT("Event Area");
		frame.hIcon = 0;
		frame.height = 20;
		frame.Flags = F_VISIBLE | F_SHOWTBTIP | F_NOBORDER | F_TCHAR;
		frame.align = alBottom;
		frame.hWnd = CreateWindowExW(0, L"EventAreaClass", L"evt", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20, pcli->hwndContactList, (HMENU) 0, g_hInst, NULL);
		g_hwndEventArea = frame.hWnd;
		hNotifyFrame = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM) & frame, (LPARAM)0);
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)hNotifyFrame, FU_FMPOS);
		hideShowNotifyFrame();
		CreateViewModeFrame();
	}

	SetButtonToSkinned();

	{
		DWORD flags;
		CLISTFrame Frame;
		memset(&Frame, 0, sizeof(Frame));
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.hWnd = pcli->hwndContactTree;
		Frame.align = alClient;
		Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP | F_NOBORDER | F_TCHAR;
		Frame.tname = _T("My Contacts");
		Frame.TBtname = TranslateT("My Contacts");
		Frame.height = 200;
		hFrameContactTree = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM) & Frame, (LPARAM)0);
		//free(Frame.name);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBTIPNAME, hFrameContactTree), (LPARAM)Translate("My Contacts"));

		/*
		* ugly, but working hack. Prevent that annoying little scroll bar from appearing in the "My Contacts" title bar
		*/

		flags = (DWORD)CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hFrameContactTree), 0);
		flags |= F_VISIBLE;
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hFrameContactTree), flags);
	}
	return(0);
}

int CLUI::modulesLoaded(WPARAM wParam, LPARAM lParam)
{
	MTG_OnmodulesLoad(wParam, lParam);
	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
	return 0;
}

static HICON hIconSaved = 0;

void ClearIcons(int mode)
{
	int i;

	for (i = IDI_OVL_OFFLINE; i <= IDI_OVL_OUTTOLUNCH; i++) {
		if (overlayicons[i - IDI_OVL_OFFLINE] != 0) {
			if (mode)
				DestroyIcon(overlayicons[i - IDI_OVL_OFFLINE]);
			overlayicons[i - IDI_OVL_OFFLINE] = 0;
		}
	}
	hIconSaved = ImageList_GetIcon(CLUI::hExtraImages, 3, ILD_NORMAL);
	ImageList_RemoveAll(CLUI::hExtraImages);
}

static void CacheClientIcons()
{
	int i = 0;
	char szBuffer[128];

	ClearIcons(0);

	for (i = IDI_OVL_OFFLINE; i <= IDI_OVL_OUTTOLUNCH; i++) {
		mir_snprintf(szBuffer, sizeof(szBuffer), "cln_ovl_%d", ID_STATUS_OFFLINE + (i - IDI_OVL_OFFLINE));
		overlayicons[i - IDI_OVL_OFFLINE] = IcoLib_GetIcon(szBuffer);
	}
	ImageList_AddIcon(CLUI::hExtraImages, IcoLib_GetIcon("core_main_14"));
	ImageList_AddIcon(CLUI::hExtraImages, Skin_LoadIcon(SKINICON_EVENT_URL));
	ImageList_AddIcon(CLUI::hExtraImages, IcoLib_GetIcon("core_main_17"));
	if (hIconSaved != 0) {
		ImageList_AddIcon(CLUI::hExtraImages, hIconSaved);
		DestroyIcon(hIconSaved);
		hIconSaved = 0;
	} else
		ImageList_AddIcon(CLUI::hExtraImages, IcoLib_GetIcon("core_main_17"));
}

static void InitIcoLib()
{
	Icon_RegisterT(g_hInst, LPGENT("Contact list")_T("/")LPGENT("Default"),myIcons, _countof(myIcons));

	for (int i = IDI_OVL_OFFLINE; i <= IDI_OVL_OUTTOLUNCH; i++) {
		char szBuffer[128];
		mir_snprintf(szBuffer, _countof(szBuffer), "cln_ovl_%d", ID_STATUS_OFFLINE + (i - IDI_OVL_OFFLINE));
		IconItemT icon = { pcli->pfnGetStatusModeDescription(ID_STATUS_OFFLINE + (i - IDI_OVL_OFFLINE), 0), szBuffer, i };
		Icon_RegisterT(g_hInst, LPGENT("Contact list")_T("/")LPGENT("Overlay icons"), &icon, 1);
	}

	PROTOACCOUNT **accs = NULL;
	int p_count = 0;
	Proto_EnumAccounts(&p_count, &accs);
	for (int k = 0; k < p_count; k++) {
		if (!Proto_IsAccountEnabled(accs[k]) || CallProtoService(accs[k]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
			continue;

		TCHAR szDescr[128];
		mir_sntprintf(szDescr, _countof(szDescr), TranslateT("%s connecting"), accs[k]->tszAccountName);
		IconItemT icon = { szDescr, "conn", IDI_PROTOCONNECTING };
		Icon_RegisterT(g_hInst, LPGENT("Contact list")_T("/")LPGENT("Connecting icons"), &icon, 1, accs[k]->szModuleName);
	}
}

static int IcoLibChanged(WPARAM wParam, LPARAM lParam)
{
	IcoLibReloadIcons();
	return 0;
}

/*
* if mode != 0 we do first time init, otherwise only reload the extra icon stuff
*/

void CLN_LoadAllIcons(BOOL mode)
{
	if (mode) {
		InitIcoLib();
		HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibChanged);
	}
	CacheClientIcons();
	/*
	if (mode) {
		InitIcoLib();
		hIcoLibChanged = HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibChanged);
		cfg::dat.hIconVisible = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "CLN_visible");
		cfg::dat.hIconInvisible = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "CLN_invisible");
		cfg::dat.hIconChatactive = (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) "CLN_chatactive");
	}
	CacheClientIcons();
	*/
}

void CLUI::configureEventArea(HWND hwnd)
{
	int iCount = GetMenuItemCount(cfg::dat.hMenuNotify);
	DWORD dwFlags = cfg::dat.dwFlags;
	int oldstate = cfg::dat.notifyActive;
	int dwVisible = CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hNotifyFrame), 0) & F_VISIBLE;

	if (dwVisible) {
		if (dwFlags & CLUI_FRAME_AUTOHIDENOTIFY)
			cfg::dat.notifyActive = iCount > 0 ? 1 : 0;
		else
			cfg::dat.notifyActive = 1;
	} else
		cfg::dat.notifyActive = 0;

	if (oldstate != cfg::dat.notifyActive)
		hideShowNotifyFrame();
}

void CLUI::configureWindowLayout()
{
	int i;

	for (i = 0; ; i++) {
		if (top_buttons[i].szTooltip == NULL)
			break;
		if (top_buttons[i].hwnd == 0)
			continue;
		switch (top_buttons[i].id) {
			case IDC_TBMENU:
			case IDC_TBGLOBALSTATUS:
				ShowWindow(top_buttons[i].hwnd, cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? SW_SHOW : SW_HIDE);
				break;
			default:
				break;
		}
	}
}

void IcoLibReloadIcons()
{
	int i;
	HICON hIcon;

	for (i = 0; ; i++) {
		if (top_buttons[i].szTooltip == NULL)
			break;

		if (top_buttons[i].id == IDC_TBMENU || top_buttons[i].id == IDC_TBGLOBALSTATUS || top_buttons[i].id == IDC_TBTOPSTATUS)
			continue;

		hIcon = IcoLib_GetIcon(top_buttons[i].szIcoLibIcon);
		if (top_buttons[i].hwnd && IsWindow(top_buttons[i].hwnd)) {
			SendMessage(top_buttons[i].hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM) hIcon);
			InvalidateRect(top_buttons[i].hwnd, NULL, TRUE);
		}
	}
	cfg::dat.hIconVisible = IcoLib_GetIcon("CLN_visible");
	cfg::dat.hIconInvisible = IcoLib_GetIcon("CLN_invisible");
	cfg::dat.hIconChatactive = IcoLib_GetIcon("CLN_chatactive");
	CacheClientIcons();
	CLUI::reloadExtraIcons();

	// force client icons reload
	{
		int i;

		for (i = 0; i < cfg::nextCacheEntry; i++) {
			if (cfg::eCache[i].hContact)
				NotifyEventHooks(hExtraImageApplying, (WPARAM)cfg::eCache[i].hContact, 0);
		}
	}
	//
	pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
	SendMessage(g_hwndViewModeFrame, WM_USER + 100, 0, 0);
}

void CreateButtonBar(HWND hWnd)
{
	int i;
	HICON hIcon;

	for (i = 0; ; i++) {
		if (top_buttons[i].szTooltip == NULL)
			break;
		if (top_buttons[i].hwnd)
			continue;

		if (top_buttons[i].id != IDC_TBGLOBALSTATUS && top_buttons[i].id != IDC_TBMENU)
			continue;

		top_buttons[i].hwnd = CreateWindowEx(0, _T("CLCButtonClass"), _T(""), BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20, hWnd, (HMENU) top_buttons[i].id, g_hInst, NULL);

		hIcon = top_buttons[i].hIcon = IcoLib_GetIcon(top_buttons[i].szIcoLibIcon);
		if (top_buttons[i].szIcoLibAltIcon)
			top_buttons[i].hAltIcon = IcoLib_GetIcon(top_buttons[i].szIcoLibAltIcon);

		if (top_buttons[i].id == IDC_TBMENU) {
			SetWindowText(top_buttons[i].hwnd, TranslateT("Menu"));
			SendMessage(top_buttons[i].hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM) Skin_LoadIcon(SKINICON_OTHER_MIRANDA));
		}
		if (top_buttons[i].id == IDC_TBGLOBALSTATUS) {
			SetWindowText(top_buttons[i].hwnd, TranslateT("Offline"));
			SendMessage(top_buttons[i].hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM) Skin_LoadIcon(SKINICON_STATUS_OFFLINE));
		}
		SendMessage(top_buttons[i].hwnd, BUTTONADDTOOLTIP, (WPARAM) TranslateTS(top_buttons[i].szTooltip), 0);
	}
}

/*
 * to properly configure the CLUI window
 *
 * 1) applyBorderStyle()
 * 2) configureGeometry()
 */
void CLUI::configureGeometry(int mode)
{
	RECT rcStatus;

	Skin::metrics.cLeft = Skin::metrics.fHaveFrame ? (Skin::metrics.cLeftFramed + Skin::metrics.cLeftButtonset) :
			(Skin::metrics.cLeftSkinned + Skin::metrics.cFakeLeftBorder + Skin::metrics.cLeftButtonset);

	Skin::metrics.cRight = Skin::metrics.fHaveFrame ? (Skin::metrics.cRightFramed + Skin::metrics.cRightButtonset) :
			(Skin::metrics.cRightSkinned + Skin::metrics.cFakeRightBorder + Skin::metrics.cRightButtonset);

	Skin::metrics.cTop = Skin::metrics.fHaveFrame ? (Skin::metrics.cTopFramed + Skin::metrics.cTopButtonset) :
			(Skin::metrics.cTopSkinned + Skin::metrics.cFakeCaption + Skin::metrics.cTopButtonset);

	Skin::metrics.cBottom = Skin::metrics.fHaveFrame ? (Skin::metrics.cBottomFramed + Skin::metrics.cBottomButtonset) :
			(Skin::metrics.cBottomSkinned + Skin::metrics.cFakeBtmBorder + Skin::metrics.cBottomButtonset);

	if (mode) {
		if (cfg::dat.dwFlags & CLUI_FRAME_SBARSHOW) {
			SendMessage(pcli->hwndStatus, WM_SIZE, 0, 0);
			GetWindowRect(pcli->hwndStatus, &rcStatus);
			Skin::metrics.bSBarHeight = (rcStatus.bottom - rcStatus.top);
		} else
			Skin::metrics.bSBarHeight = 0;
	}

	Skin::metrics.dwTopOffset = Skin::metrics.cTop;
	Skin::metrics.dwBottomOffset = (cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? 2 + Skin::metrics.cButtonHeight : 0) + Skin::metrics.cBottom;

	if (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN) {
		Skin::metrics.dwTopOffset += 2;
		Skin::metrics.dwBottomOffset += 2;
		Skin::metrics.cLeft += 3;
		Skin::metrics.cRight += 3;
	}
}

/*
 * set the states of defined database action buttons (only if button is a toggle)
*/

void CLUI::setFrameButtonStates(MCONTACT hPassedContact)
{
	TButtonItem *buttonItem = buttonItems;
	MCONTACT hContact = 0, hFinalContact = 0;
	char *szModule, *szSetting;
	int sel = cfg::clcdat ? cfg::clcdat->selection : -1;
	struct ClcContact *contact = 0;

	if (sel != -1 && hPassedContact == 0) {
		sel = pcli->pfnGetRowByIndex(cfg::clcdat, cfg::clcdat->selection, &contact, NULL);
		if (contact && contact->type == CLCIT_CONTACT) {
			hContact = contact->hContact;
		}
	}

	while (buttonItem) {
		BOOL result = FALSE;

		if (!(buttonItem->dwFlags & BUTTON_ISTOGGLE && buttonItem->dwFlags & BUTTON_ISDBACTION)) {
			buttonItem = buttonItem->nextItem;
			continue;
		}
		szModule = buttonItem->szModule;
		szSetting = buttonItem->szSetting;
		if (buttonItem->dwFlags & BUTTON_DBACTIONONCONTACT || buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION) {
			if (hContact == 0) {
				SendMessage(buttonItem->hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
				buttonItem = buttonItem->nextItem;
				continue;
			}
			if (buttonItem->dwFlags & BUTTON_ISCONTACTDBACTION)
				szModule = GetContactProto(hContact);
			hFinalContact = hContact;
		} else
			hFinalContact = 0;

		if (buttonItem->type == DBVT_ASCIIZ) {
			DBVARIANT dbv = {0};

			if (!cfg::getString(hFinalContact, szModule, szSetting, &dbv)) {
				result = !strcmp((char *)buttonItem->bValuePush, dbv.pszVal);
				db_free(&dbv);
			}
		} else {
			switch (buttonItem->type) {
				case DBVT_BYTE: {
					BYTE val = cfg::getByte(hFinalContact, szModule, szSetting, 0);
					result = (val == buttonItem->bValuePush[0]);
					break;
				}
				case DBVT_WORD: {
					WORD val = cfg::getWord(hFinalContact, szModule, szSetting, 0);
					result = (val == *((WORD *) & buttonItem->bValuePush));
					break;
				}
				case DBVT_DWORD: {
					DWORD val = cfg::getDword(hFinalContact, szModule, szSetting, 0);
					result = (val == *((DWORD *) & buttonItem->bValuePush));
					break;
				}
			}
		}
		SendMessage(buttonItem->hWnd, BM_SETCHECK, (WPARAM)result, 0);
		buttonItem = buttonItem->nextItem;
	}
}

/*
 * set states of standard buttons (pressed/unpressed
 */
void CLUI::setButtonStates(HWND hwnd)
{
	BYTE iMode;
	TButtonItem *buttonItem = buttonItems;

	iMode = cfg::getByte("CList", "HideOffline", 0);
	while (buttonItem) {
		if (buttonItem->dwFlags & BUTTON_ISINTERNAL) {
			switch (buttonItem->uId) {
				case IDC_TBSOUND:
					SendMessage(buttonItem->hWnd, BM_SETCHECK, cfg::dat.soundsOff ? BST_UNCHECKED : BST_CHECKED, 0);
					break;
				case IDC_TBHIDEOFFLINE:
					SendMessage(buttonItem->hWnd, BM_SETCHECK, iMode ? BST_CHECKED : BST_UNCHECKED, 0);
					break;
				case IDC_TBHIDEGROUPS:
					SendMessage(buttonItem->hWnd, BM_SETCHECK, cfg::getByte("CList", "UseGroups", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
					break;
			}
		}
		buttonItem = buttonItem->nextItem;
	}
}


void ReloadThemedOptions()
{
	cfg::dat.bUsePerProto = 		cfg::getByte("CLCExt", "useperproto", 0);
	cfg::dat.bOverridePerStatusColors = cfg::getByte("CLCExt", "override_status", 0);
	cfg::dat.bRowSpacing = 			cfg::getByte("CLC", "RowGap", 0);
	cfg::dat.exIconScale = 			cfg::getByte("CLC", "ExIconScale", 16);
	cfg::dat.bApplyIndentToBg = 	cfg::getByte("CLCExt", "applyindentbg", 0);
	cfg::dat.gapBetweenFrames = 	(BYTE)cfg::getDword("CLUIFrames", "GapBetweenFrames", 1);
	cfg::dat.bUseDCMirroring = 		cfg::getByte("CLC", "MirrorDC", 0);
	cfg::dat.bGroupAlign = 			cfg::getByte("CLC", "GroupAlign", 0);
	cfg::dat.bUseFloater = 			cfg::getByte("CLUI", "FloaterMode", 0);
	cfg::dat.titleBarHeight = 		cfg::getByte("CLCExt", "frame_height", DEFAULT_TITLEBAR_HEIGHT);
	cfg::dat.group_padding = 		cfg::getDword("CLCExt", "grp_padding", 0);
}

static RECT rcWindow = {0};

static void sttProcessResize(HWND hwnd, NMCLISTCONTROL *nmc)
{
	RECT rcTree, rcWorkArea, rcOld;
	int maxHeight, newHeight;
	int winstyle, skinHeight = 0;

	if (disableautoupd)
		return;

	if (!cfg::getByte("CLUI", "AutoSize", 0))
		return;

	if (Docking_IsDocked(0, 0))
		return;
	if (hFrameContactTree == 0)
		return;

	maxHeight = cfg::getByte("CLUI", "MaxSizeHeight", 75);
	rcOld = rcWindow;

	GetWindowRect(hwnd, &rcWindow);
	GetWindowRect(pcli->hwndContactTree, &rcTree);
	winstyle = GetWindowLong(pcli->hwndContactTree, GWL_STYLE);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
	if (nmc->pt.y > (rcWorkArea.bottom - rcWorkArea.top)) {
		nmc->pt.y = (rcWorkArea.bottom - rcWorkArea.top);
	}

	if (winstyle & CLS_SKINNEDFRAME) {
		BOOL hasTitleBar = wndFrameCLC ? wndFrameCLC->TitleBar.ShowTitleBar : 0;
		TStatusItem *item = &Skin::statusItems[(hasTitleBar ? ID_EXTBKOWNEDFRAMEBORDERTB : ID_EXTBKOWNEDFRAMEBORDER)];
		skinHeight = item->IGNORED ? 0 : item->MARGIN_BOTTOM + item->MARGIN_TOP;
	}

	newHeight = max(nmc->pt.y, 3) + 1 + ((winstyle & WS_BORDER) ? 2 : 0) + skinHeight + (rcWindow.bottom - rcWindow.top) - (rcTree.bottom - rcTree.top);
	if (newHeight == (rcWindow.bottom - rcWindow.top))
		return;

	if (newHeight > (rcWorkArea.bottom - rcWorkArea.top)*maxHeight / 100)
		newHeight = (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100;
	if (cfg::getByte("CLUI", "AutoSizeUpward", 0)) {
		rcWindow.top = rcWindow.bottom - newHeight;
		if (rcWindow.top < rcWorkArea.top) rcWindow.top = rcWorkArea.top;
	} else {
		rcWindow.bottom = rcWindow.top + newHeight;
		if (rcWindow.bottom > rcWorkArea.bottom) rcWindow.bottom = rcWorkArea.bottom;
	}
	if (cfg::dat.szOldCTreeSize.cx != rcTree.right - rcTree.left) {
		cfg::dat.szOldCTreeSize.cx = rcTree.right - rcTree.left;
		return;
	}
	KillTimer(hwnd, TIMERID_AUTOSIZE);
	SetTimer(hwnd, TIMERID_AUTOSIZE, 50, 0);
}

BP_PAINTPARAMS bppp = {0, 0, 0, 0};

static char* _sbItemNames[] = {
		"@ScrollBackUpper",
		"@ScrollBackLower",
		"@ScrollThumb",
		"@ScrollButton",
		"@ScrollArrowUp",
		"@ScrollArrowDn"
};

#define ID_SBARSKIN_BACK_UPR 0
#define ID_SBARSKIN_BACK_LWR 1
#define ID_SBARSKIN_THUMB 2
#define ID_SBARSKIN_BUTTON 3
#define ID_SBARSKIN_ARROWUP 4
#define ID_SBARSKIN_ARROWDN 5

/**
 * draw the custom scroll bar
 * TODO Scrollbar: improve this to allow selective skinning of hovered/selected/pressed
 * elements.
 * @param nmcsbcd			custom control drawing structure
 * @return
 */
int CustomDrawScrollBars(NMCSBCUSTOMDRAW *nmcsbcd)
{
	switch (nmcsbcd->hdr.code) {
		case NM_COOLSB_CUSTOMDRAW: {
			static HDC hdcScroll = 0;
			//static HBITMAP hbmScroll, hbmScrollOld;
			static LONG scrollLeft, scrollRight, scrollHeight, scrollYmin, scrollYmax;

			switch (nmcsbcd->dwDrawStage) {
				case CDDS_PREPAINT:
					if (cfg::dat.bSkinnedScrollbar)
						return CDRF_SKIPDEFAULT;
					else
						return CDRF_DODEFAULT;
				case CDDS_POSTPAINT:
					return 0;
				case CDDS_ITEMPREPAINT: {
					HDC hdc = nmcsbcd->hdc;
					UINT uItemID = ID_SBARSKIN_BACK_UPR;
					RECT rcWindow;
					POINT pt;
					DWORD dfcFlags;
					GetWindowRect(pcli->hwndContactTree, &rcWindow);
					pt.x = rcWindow.left;
					pt.y = rcWindow.top;
					ScreenToClient(pcli->hwndContactList, &pt);

					bppp.cbSize = sizeof(BP_PAINTPARAMS);
					HANDLE hbp = Api::pfnBeginBufferedPaint(hdc, &nmcsbcd->rect, BPBF_TOPDOWNDIB, &bppp, &hdcScroll);

					//hdcScroll = hdc;
					BitBlt(hdcScroll, nmcsbcd->rect.left, nmcsbcd->rect.top, nmcsbcd->rect.right - nmcsbcd->rect.left,
						   nmcsbcd->rect.bottom - nmcsbcd->rect.top, cfg::dat.hdcBg, pt.x + nmcsbcd->rect.left, pt.y + nmcsbcd->rect.top, SRCCOPY);

					switch (nmcsbcd->uItem) {
						case HTSCROLL_UP:
						case HTSCROLL_DOWN:
							uItemID = (nmcsbcd->uState == CDIS_DEFAULT || nmcsbcd->uState == CDIS_DISABLED) ? ID_SBARSKIN_BUTTON :
									  (nmcsbcd->uState == CDIS_HOT ? ID_SBARSKIN_BUTTON : ID_SBARSKIN_BUTTON);
							break;
						case HTSCROLL_PAGEGDOWN:
						case HTSCROLL_PAGEGUP:
							uItemID = nmcsbcd->uItem == HTSCROLL_PAGEGUP ? ID_SBARSKIN_BACK_UPR : ID_SBARSKIN_BACK_LWR;
							break;
						case HTSCROLL_THUMB:
							uItemID = nmcsbcd->uState == CDIS_HOT ? ID_SBARSKIN_THUMB : ID_SBARSKIN_THUMB;
							uItemID = nmcsbcd->uState == CDIS_SELECTED ? ID_SBARSKIN_THUMB : ID_SBARSKIN_THUMB;
							break;
						default:
							break;
					}

					Skin::renderNamedImageItem(_sbItemNames[uItemID], &nmcsbcd->rect, hdcScroll);
					dfcFlags = DFCS_FLAT | (nmcsbcd->uState == CDIS_DISABLED ? DFCS_INACTIVE :
											(nmcsbcd->uState == CDIS_HOT ? DFCS_HOT :
											 (nmcsbcd->uState == CDIS_SELECTED ? DFCS_PUSHED : 0)));

					if (nmcsbcd->uItem == HTSCROLL_UP)
						uItemID = ID_SBARSKIN_ARROWUP;
					if (nmcsbcd->uItem == HTSCROLL_DOWN)
						uItemID = ID_SBARSKIN_ARROWDN;
					Skin::renderNamedImageItem(_sbItemNames[uItemID], &nmcsbcd->rect, hdcScroll);
					Api::pfnEndBufferedPaint(hbp, TRUE);
				}
				default:
					break;
			}
		}
		return 0;
	}
	return 0;
}

static int ServiceParamsOK(TButtonItem *item, WPARAM *wParam, LPARAM *lParam, MCONTACT hContact)
{
	if (item->dwFlags & BUTTON_PASSHCONTACTW || item->dwFlags & BUTTON_PASSHCONTACTL || item->dwFlags & BUTTON_ISCONTACTDBACTION) {
		if (hContact == 0)
			return 0;
		if (item->dwFlags & BUTTON_PASSHCONTACTW)
			*wParam = (WPARAM)hContact;
		else if (item->dwFlags & BUTTON_PASSHCONTACTL)
			*lParam = (LPARAM)hContact;
		return 1;
	}
	return 1;                                       // doesn't need a paramter
}
void CLUI::Show(HWND hwnd)
{
	int state = old_cliststate;
	int onTop = cfg::getByte("CList", "OnTop", SETTING_ONTOP_DEFAULT);

	SendMessage(hwnd, WM_SETREDRAW, FALSE, FALSE);
	if (state == SETTING_STATE_NORMAL) {
		ShowWindow(pcli->hwndContactList, SW_SHOWNORMAL);
		SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
		Redraw();
	} else if (state == SETTING_STATE_MINIMIZED) {
		cfg::dat.forceResize = TRUE;
		ShowWindow(pcli->hwndContactList, SW_HIDE);
	} else if (state == SETTING_STATE_HIDDEN) {
		cfg::dat.forceResize = TRUE;
		ShowWindow(pcli->hwndContactList, SW_HIDE);
	}
	SetWindowPos(pcli->hwndContactList, onTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
	if (cfg::dat.autosize) {
		SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
		SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);
	}
	SFL_Create();
	SFL_SetState(cfg::dat.bUseFloater & CLUI_FLOATER_AUTOHIDE ? (old_cliststate == SETTING_STATE_NORMAL ? 0 : 1) : 1);
}

#define M_CREATECLC  (WM_USER+1)
LRESULT CALLBACK CLUI::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CREATE: {
			int 		i;
			wchar_t		szSkinFile[MAX_PATH];
			wchar_t*	pszSkinToLoad = 0;
			DBVARIANT	dbv = {0};

			SetMenu(hwnd, 0);
			Skin::Unload();
			if(0 == cfg::getTString(0, SKIN_DB_MODULE, "gCurrentSkin", &dbv)) {
				mir_sntprintf(szSkinFile, MAX_PATH, L"%s%s", cfg::szProfileDir, dbv.ptszVal);
				if(PathFileExistsW(szSkinFile))
					pszSkinToLoad = szSkinFile;
				db_free(&dbv);
			}
			SkinLoader *sLoader = new SkinLoader(pszSkinToLoad);

			if(sLoader->isValid())
				sLoader->Load();
			else
				WarningDlg::show(WarningDlg::WARN_SKIN_LOADER_ERROR, WarningDlg::CWF_UNTRANSLATED|MB_OK|MB_ICONERROR);
			delete sLoader;

			Skin::updateAeroState();
			int flags = WS_CHILD | CCS_BOTTOM;
			flags |= cfg::getByte("CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
			pcli->hwndStatus = CreateWindowExW(0, STATUSCLASSNAME, NULL, flags, 0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
			//SendMessage(pcli->hwndStatus, SB_SETMINHEIGHT, 12, 0);
			if (flags & WS_VISIBLE) {
				ShowWindow(pcli->hwndStatus, SW_SHOW);
				SendMessage(pcli->hwndStatus, WM_SIZE, 0, 0);
			}
			OldStatusBarProc = (WNDPROC)SetWindowLongPtr(pcli->hwndStatus, GWLP_WNDPROC, (LONG_PTR)NewStatusBarWndProc);
			SetClassLong(pcli->hwndStatus, GCL_STYLE, GetClassLong(pcli->hwndStatus, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));

			g_oldSize.cx = g_oldSize.cy = 0;
			old_cliststate = cfg::getByte("CList", "State", SETTING_STATE_NORMAL);
			cfg::writeByte("CList", "State", SETTING_STATE_HIDDEN);
			SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_VISIBLE);
			SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_CLIPCHILDREN);
			if (!cfg::dat.bFirstRun)
				configureEventArea(hwnd);
			CluiProtocolStatusChanged(0, 0);

			for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
				statusNames[i - ID_STATUS_OFFLINE] = pcli->pfnGetStatusModeDescription(i, 0);

			SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | (WS_EX_LAYERED));

#ifndef _USE_D2D
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
#endif
			transparentFocus = 1;

			TranslateMenu(GetMenu(hwnd));
			PostMessage(hwnd, M_CREATECLC, 0, 0);
			bf.BlendOp = AC_SRC_OVER;
			bf.AlphaFormat = AC_SRC_ALPHA;
			bf.SourceConstantAlpha = 255;

			ulwInfo.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
			ulwInfo.pblend = &bf;
			Skin::setAeroMargins();
			return FALSE;
		}
		case WM_NCCREATE: {
			LPCREATESTRUCT p = (LPCREATESTRUCT)lParam;
			p->style &= ~(CS_HREDRAW | CS_VREDRAW);
			pcli->hwndContactList = hwnd;
		}
		break;
		case M_CREATECLC: {
			CreateButtonBar(hwnd);
			SendMessage(hwnd, WM_SETREDRAW, FALSE, FALSE);
			{
				ShowWindow(pcli->hwndContactList, SW_HIDE);
				applyBorderStyle();
				configureGeometry(0);
				SendMessage(hwnd, WM_SIZE, 0, 0);
				SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE);
			}

			if (cfg::dat.soundsOff)
				hSoundHook = HookEvent(ME_SKIN_PLAYINGSOUND, ClcSoundHook);
			configureWindowLayout();
			setButtonStates(hwnd);

			createCLC(hwnd);
			cfg::clcdat = (struct ClcData *)GetWindowLongPtr(pcli->hwndContactTree, 0);

			cfg::writeByte("CList", "State", old_cliststate);

			if (cfg::getByte("CList", "AutoApplyLastViewMode", 0)) {
				DBVARIANT dbv = {0};
				if (!db_get(NULL, "CList", "LastViewMode", &dbv)) {
					if (lstrlenA(dbv.pszVal) > 2) {
						if (cfg::getDword(NULL, CLVM_MODULE, dbv.pszVal, -1) != 0xffffffff)
							ApplyViewMode((char *)dbv.pszVal);
					}
					db_free(&dbv);
				}
			}
			Show(hwnd);
			return 0;
		}
		case WM_ERASEBKGND:
			return TRUE;

		case WM_PAINT: 	{
			PAINTSTRUCT 	ps;
			RECT 			rc, rcFrame, rcClient;
			HDC 			hdc;
			HDC 			hdcReal = BeginPaint(hwnd, &ps);
			TImageItem*		activeItem = cfg::isAero ? bgImageItem : bgImageItem_nonAero;

			GetClientRect(hwnd, &rcClient);
			CopyRect(&rc, &rcClient);

			if (!cfg::dat.hdcBg || rc.right != cfg::dat.dcSize.cx || rc.bottom + Skin::metrics.bSBarHeight != cfg::dat.dcSize.cy) {
				cfg::dat.dcSize.cy = rc.bottom + Skin::metrics.bSBarHeight;
				cfg::dat.dcSize.cx = rc.right;
				if (cfg::dat.hdcBg) {
					SelectObject(cfg::dat.hdcBg, cfg::dat.hbmBgOld);
					DeleteObject(cfg::dat.hbmBg);
					DeleteDC(cfg::dat.hdcBg);
				}
				cfg::dat.hdcBg = CreateCompatibleDC(hdcReal);
				cfg::dat.hbmBg = Gfx::createRGBABitmap(cfg::dat.dcSize.cx, cfg::dat.dcSize.cy);
				cfg::dat.hbmBgOld = reinterpret_cast<HBITMAP>(SelectObject(cfg::dat.hdcBg, cfg::dat.hbmBg));
			}

			if (cfg::shutDown) {
				EndPaint(hwnd, &ps);
				return 0;
			}

			hdc = cfg::dat.hdcBg;

			CopyRect(&rcFrame, &rcClient);

			cfg::dat.ptW.x = cfg::dat.ptW.y = 0;
			ClientToScreen(hwnd, &cfg::dat.ptW);

			if(cfg::isAero)
				FillRect(hdc, &rcClient, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
			else if (g_CLUISkinnedBkColor)
				FillRect(hdc, &rcClient, g_CLUISkinnedBkColor);
			if(!Skin::metrics.fHaveFrame) {
				if (activeItem) {
					Gfx::renderImageItem(hdc, activeItem, &rcFrame);
					goto skipbg;
				}
			}
			else {
				if(bgClientItem) {
					Gfx::renderImageItem(hdc, bgClientItem, &rcFrame);
					goto skipbg;
				}
			}

			rcFrame.left += (Skin::metrics.cLeft - 1);
			rcFrame.right -= (Skin::metrics.cRight - 1);
			rcFrame.bottom++;
			rcFrame.bottom -= Skin::metrics.bSBarHeight;
			rcFrame.top += (Skin::metrics.dwTopOffset - 1);

			if (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN) {
				InflateRect(&rcFrame, 1, 1);
				if (cfg::dat.bSkinnedButtonMode)
					rcFrame.bottom -= (Skin::metrics.dwBottomOffset);
				DrawEdge(hdc, &rcFrame, BDR_SUNKENOUTER, BF_RECT);
			}
skipbg:
#ifndef _USE_D2D
			BitBlt(hdcReal, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdc, 0, 0, SRCCOPY);
#endif
			EndPaint(hwnd, &ps);
			/*
			if(renderTarget) {
				renderTarget->BeginDraw();
				ID2D1SolidColorBrush* brush;
				D2D1_COLOR_F clr;
				clr.b = clr.r = clr.g = 0.0;
				clr.r = 25.0;
				clr.a = 0.0;

				D2D1_BRUSH_PROPERTIES bp;
				bp.transform = D2D1::Matrix3x2F();
				bp.opacity = 0.0;
				D2D1_RECT_F rect;
				rect.left = rect.top = 0;
				rect.right = rcClient.right;
				rect.bottom = rcClient.bottom;
				renderTarget->CreateSolidColorBrush(clr, bp, &brush);
				renderTarget->FillRectangle(&rect, brush);
				renderTarget->EndDraw();
				brush->Release();
			}
			*/
#ifdef _USE_D2D
			//updateLayers();
#endif
			return 0;
		}

		case WM_NCPAINT:
			if(Skin::metrics.fHaveFrame)
				break;
			return(0);

		case WM_ENTERSIZEMOVE: {
			RECT rc;
			POINT pt = {0};

			GetWindowRect(hwnd, &g_PreSizeRect);
			GetClientRect(hwnd, &rc);
			ClientToScreen(hwnd, &pt);
			g_CLUI_x_off = pt.x - g_PreSizeRect.left;
			g_CLUI_y_off = pt.y - g_PreSizeRect.top;
			pt.x = rc.right;
			ClientToScreen(hwnd, &pt);
			g_CLUI_x1_off = g_PreSizeRect.right - pt.x;
			pt.x = 0;
			pt.y = rc.bottom;
			ClientToScreen(hwnd, &pt);
			g_CLUI_y1_off = g_PreSizeRect.bottom - pt.y;
			break;
		}
		case WM_EXITSIZEMOVE:
			PostMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
			break;

		case WM_WINDOWPOSCHANGED:
			return(0);

		case WM_WINDOWPOSCHANGING: {
			WINDOWPOS *wp = (WINDOWPOS *)lParam;

			if (wp && wp->flags & SWP_NOSIZE)
				return FALSE;

			if (Docking_IsDocked(0, 0))
				break;

			if (pcli->hwndContactList != NULL) {

				//dsize.width = rcOld.right - rcOld.left;
				//dsize.height = rcOld.bottom - rcOld.top;

				/*
				if(0 == renderTarget) {
					D2D1_RENDER_TARGET_PROPERTIES rp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
						D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 0, 0, D2D1_RENDER_TARGET_USAGE_NONE,
						D2D1_FEATURE_LEVEL_DEFAULT);

					D2D1_HWND_RENDER_TARGET_PROPERTIES hp = D2D1::HwndRenderTargetProperties(hwnd, dsize, D2D1_PRESENT_OPTIONS_NONE);
					Gfx::pD2DFactory->CreateHwndRenderTarget(&rp, &hp, &renderTarget);
					if(0 == renderTarget)
						MessageBox(0, L"Creating render target failed", L"foo", MB_OK);

				}
				else
					renderTarget->Resize(&dsize);
				*/
				fInSizing = true;

				newRect.left = 0;
				newRect.right = wp->cx - (g_CLUI_x_off + g_CLUI_x1_off);
				newRect.top = 0;
				newRect.bottom = wp->cy - g_CLUI_y_off - g_CLUI_y1_off;

				if(!cfg::isAero)
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				if (cfg::dat.dwFlags & CLUI_FRAME_SBARSHOW) {
					RECT rcStatus;
					SetWindowPos(pcli->hwndStatus, 0, 0, newRect.bottom - 20 - (Skin::metrics.fHaveFrame ? 0 : Skin::metrics.cFakeBtmBorder), newRect.right, 20, SWP_NOZORDER);
					GetWindowRect(pcli->hwndStatus, &rcStatus);
					Skin::metrics.bSBarHeight = (rcStatus.bottom - rcStatus.top);
				} else
					Skin::metrics.bSBarHeight = 0;

				SizeFramesByWindowRect(&newRect);
				dock_prevent_moving = 0;
				layoutButtons(hwnd, &newRect);
				if (wp->cx != g_oldSize.cx)
					SendMessage(hwnd, CLUIINTM_STATUSBARUPDATE, 0, 0);
				dock_prevent_moving = 1;
				g_oldPos.x = wp->x;
				g_oldPos.y = wp->y;
				g_oldSize.cx = wp->cx;
				g_oldSize.cy = wp->cy;
				rcWPC = newRect;

				fInSizing = false;
			}
			fInSizing = false;
			return(0);
		}

		case WM_SIZE: {
			RECT rc;

			if ((wParam == 0 && lParam == 0) || Docking_IsDocked(0, 0)) {

				if (IsZoomed(hwnd))
					ShowWindow(hwnd, SW_SHOWNORMAL);

				if (pcli->hwndContactList != 0) {
					SendMessage(hwnd, WM_ENTERSIZEMOVE, 0, 0);
					GetWindowRect(hwnd, &rc);
					WINDOWPOS	wp = {0};
					wp.cx = rc.right - rc.left;
					wp.cy = rc.bottom - rc.top;
					wp.x = rc.left;
					wp.y = rc.top;
					wp.flags = 0;
					SendMessage(hwnd, WM_WINDOWPOSCHANGING, 0, (LPARAM)&wp);
					SendMessage(hwnd, WM_EXITSIZEMOVE, 0, 0);
				}
			}
		}
		case WM_MOVE:
			if (!IsIconic(hwnd)) {
				RECT rc;
				GetWindowRect(hwnd, &rc);

				if (!Docking_IsDocked(0, 0)) {
					cluiPos.bottom = (DWORD)(rc.bottom - rc.top);
					cluiPos.left = rc.left;
					cluiPos.top = rc.top;
				}
				cluiPos.right = rc.right - rc.left;
				if (cfg::dat.realTimeSaving) {
					RECT rc;
					GetWindowRect(hwnd, &rc);

					if (!CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0)) {     //if docked, dont remember pos (except for width)
						cfg::writeDword("CList", "Height", (DWORD)(rc.bottom - rc.top));
						cfg::writeDword("CList", "x", (DWORD) rc.left);
						cfg::writeDword("CList", "y", (DWORD) rc.top);
					}
					cfg::writeDword("CList", "Width", (DWORD)(rc.right - rc.left));
				}
			}
			return TRUE;

		case WM_SETFOCUS:
			SetFocus(pcli->hwndContactTree);
			return 0;

		case CLUIINTM_REMOVEFROMTASKBAR: {
			if (Skin::metrics.bWindowStyle == SETTING_WINDOWSTYLE_DEFAULT && cfg::getByte("CList", "AlwaysHideOnTB", 0))
				removeFromTaskBar(hwnd);
			return 0;
		}
		case WM_ACTIVATE:
			if (fading_active) {
				if (wParam != WA_INACTIVE && cfg::dat.isTransparent)
					transparentFocus = 1;
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			if (wParam == WA_INACTIVE) {
				if ((HWND) wParam != hwnd)
					if (cfg::dat.isTransparent)
						if (transparentFocus)
							SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
			} else {
				if (cfg::dat.isTransparent) {
					KillTimer(hwnd, TM_AUTOALPHA);
					setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
					transparentFocus = 1;
				}
				SetWindowPos(pcli->hwndContactList, cfg::getByte("CList", "OnTop", SETTING_ONTOP_DEFAULT) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
			}
			PostMessage(hwnd, CLUIINTM_REMOVEFROMTASKBAR, 0, 0);
			return DefWindowProc(hwnd, msg, wParam, lParam);

		case WM_SETCURSOR:
			if (cfg::dat.isTransparent) {
				if (!transparentFocus && GetForegroundWindow() != hwnd) {
					setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
					transparentFocus = 1;
					SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
				}
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_NCHITTEST: {
			LRESULT result;
			RECT r;
			POINT pt;

			GetWindowRect(hwnd, &r);
			GetCursorPos(&pt);
			if (pt.y <= r.bottom && pt.y >= r.bottom - 6 && !cfg::getByte("CLUI", "AutoSize", 0)) {
				if (pt.x > r.left + 10 && pt.x < r.right - 10)
					return HTBOTTOM;
				if (pt.x < r.left + 10)
					return HTBOTTOMLEFT;
				if (pt.x > r.right - 10)
					return HTBOTTOMRIGHT;

			} else if (pt.y >= r.top && pt.y <= r.top + 3 && !cfg::getByte("CLUI", "AutoSize", 0)) {
				if (pt.x > r.left + 10 && pt.x < r.right - 10)
					return HTTOP;
				if (pt.x < r.left + 10)
					return HTTOPLEFT;
				if (pt.x > r.right - 10)
					return HTTOPRIGHT;
			} else if (pt.x >= r.left && pt.x <= r.left + 6)
				return HTLEFT;
			else if (pt.x >= r.right - 6 && pt.x <= r.right)
				return HTRIGHT;

			result = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if (result == HTSIZE || result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT || result == HTBOTTOM || result == HTBOTTOMRIGHT || result == HTBOTTOMLEFT)
				if (cfg::dat.autosize)
					return HTCLIENT;
			return result;
		}

		case WM_TIMER:
			if ((int) wParam == TM_AUTOALPHA) {
				int inwnd;

				if (GetForegroundWindow() == hwnd) {
					KillTimer(hwnd, TM_AUTOALPHA);
					inwnd = 1;
				} else {
					POINT pt;
					HWND hwndPt;
					pt.x = (short) LOWORD(GetMessagePos());
					pt.y = (short) HIWORD(GetMessagePos());
					hwndPt = WindowFromPoint(pt);
					inwnd = (hwndPt == hwnd || GetParent(hwndPt) == hwnd);
				}
				if (inwnd != transparentFocus) {
					//change
					transparentFocus = inwnd;
					if (transparentFocus)
						setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
					else
						setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.autoalpha, LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
				}
				if (!transparentFocus)
					KillTimer(hwnd, TM_AUTOALPHA);
			} else if (wParam == TIMERID_AUTOSIZE) {
				KillTimer(hwnd, wParam);
				SetWindowPos(hwnd, 0, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
				SendMessage(hwnd, WM_SIZE, 0, 0);
				if(cfg::isAero)
					RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
				else
					PostMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
			}
			return TRUE;
		case WM_SHOWWINDOW: {
			static int noRecurse = 0;
			DWORD thisTick, startTick;
			int sourceAlpha, destAlpha;

			if (cfg::dat.forceResize && wParam != SW_HIDE) {
				cfg::dat.forceResize = FALSE;
				PostMessage(hwnd, WM_SIZE, 0, 0);
				PostMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
			}
			PostMessage(hwnd, CLUIINTM_REMOVEFROMTASKBAR, 0, 0);

			if (g_floatoptions.enabled) {
				if (wParam)
					FLT_ShowHideAll(SW_HIDE);
				else
					FLT_ShowHideAll(SW_SHOWNOACTIVATE);
			}

			if (!cfg::dat.fadeinout)
				SFL_SetState(-1);
			if (lParam)
				return DefWindowProc(hwnd, msg, wParam, lParam);
			if (noRecurse)
				return DefWindowProc(hwnd, msg, wParam, lParam);
			if (!cfg::dat.fadeinout)
				return DefWindowProc(hwnd, msg, wParam, lParam);

			fading_active = 1;

			if (wParam) {
				sourceAlpha = 0;
				destAlpha = cfg::dat.isTransparent ? cfg::dat.alpha : 255;
				setLayeredAttributes(Skin::metrics.fHaveColorkey ? (COLORREF)cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)sourceAlpha, LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
				noRecurse = 1;
				ShowWindow(hwnd, SW_SHOW);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
				noRecurse = 0;
			} else {
				sourceAlpha = cfg::dat.isTransparent ? (transparentFocus ? cfg::dat.alpha : cfg::dat.autoalpha) : 255;
				destAlpha = 0;
			}
			for (startTick = GetTickCount(); ;) {
				thisTick = GetTickCount();
				if (thisTick >= startTick + 200) {
					SFL_SetState(-1);
					setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)(destAlpha), LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
					fading_active = 0;
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}
				setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)(sourceAlpha + (destAlpha - sourceAlpha) * (int)(thisTick - startTick) / 200), LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
			}
			//setLayeredAttributes(Skin::metrics.fHaveColorkey ? cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)(destAlpha), LWA_ALPHA | (Skin::metrics.fHaveColorkey ? LWA_COLORKEY : 0));
			//return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		case WM_SYSCOMMAND:
			if(SETTING_WINDOWSTYLE_DEFAULT == Skin::metrics.bWindowStyle && SC_RESTORE == wParam) {
				CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
				SendMessage(hwnd, WM_SIZE, 0, 0);
				Redraw();
				cfg::writeByte("CList", "State", SETTING_STATE_NORMAL);
				break;
			}

			if (wParam == SC_MAXIMIZE)
				return 0;
			else if (wParam == SC_MINIMIZE) {
				if(SETTING_WINDOWSTYLE_DEFAULT == Skin::metrics.bWindowStyle) {
					cfg::writeByte("CList", "State", SETTING_STATE_MINIMIZED);
					break;
				}
				pcli->pfnShowHide(0, 0);
				return 0;
			}
			else if (wParam == SC_RESTORE) {
				pcli->pfnShowHide(0, 0);
				return(0);
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);

		case WM_COMMAND: {
			DWORD dwOldFlags = cfg::dat.dwFlags;
			if (HIWORD(wParam) == BN_CLICKED && lParam != 0) {
				if (LOWORD(wParam) == IDC_TBFIRSTUID - 1)
					break;
				else if (LOWORD(wParam) >= IDC_TBFIRSTUID) {                    // skinnable buttons handling
					TButtonItem *item    = buttonItems;
					WPARAM wwParam       = 0;
					LPARAM llParam       = 0;
					MCONTACT hContact    = 0;
					ClcContact *contact  = 0;
					int sel              = cfg::clcdat ? cfg::clcdat->selection : -1;
					int serviceFailure   = FALSE;

					if (sel != -1) {
						sel = pcli->pfnGetRowByIndex(cfg::clcdat, cfg::clcdat->selection, &contact, NULL);
						if (contact && contact->type == CLCIT_CONTACT) {
							hContact = contact->hContact;
						}
					}
					while (item) {
						if (item->uId == (DWORD)LOWORD(wParam)) {
							int contactOK = ServiceParamsOK(item, &wwParam, &llParam, hContact);

							if (item->dwFlags & BUTTON_ISSERVICE) {
								if (ServiceExists(item->szService) && contactOK)
									CallService(item->szService, wwParam, llParam);
								else if (contactOK)
									serviceFailure = TRUE;
							} else if (item->dwFlags & BUTTON_ISPROTOSERVICE && cfg::clcdat) {
								if (contactOK) {
									char szFinalService[512];

									mir_snprintf(szFinalService, 512, "%s/%s", GetContactProto(hContact), item->szService);
									if (ServiceExists(szFinalService))
										CallService(szFinalService, wwParam, llParam);
									else
										serviceFailure = TRUE;
								}
							} else if (item->dwFlags & BUTTON_ISDBACTION) {
								BYTE *pValue;
								char *szModule = item->szModule;
								char *szSetting = item->szSetting;
								MCONTACT finalhContact = 0;

								if (item->dwFlags & BUTTON_ISCONTACTDBACTION || item->dwFlags & BUTTON_DBACTIONONCONTACT) {
									contactOK = ServiceParamsOK(item, &wwParam, &llParam, hContact);
									if (contactOK && item->dwFlags & BUTTON_ISCONTACTDBACTION)
										szModule = GetContactProto(hContact);
									finalhContact = hContact;
								} else
									contactOK = 1;

								if (contactOK) {
									BOOL fDelete = FALSE;

									if (item->dwFlags & BUTTON_ISTOGGLE) {
										BOOL fChecked = (SendMessage(item->hWnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED);

										pValue = fChecked ? item->bValueRelease : item->bValuePush;
										if (fChecked && pValue[0] == 0)
											fDelete = TRUE;
									} else
										pValue = item->bValuePush;

									if (fDelete) {
										//_DebugTraceA("delete value: %s, %s ON %d", szModule, szSetting, finalhContact);
										db_unset(finalhContact, szModule, szSetting);
									} else {
										switch (item->type) {
											case DBVT_BYTE:
												cfg::writeByte(finalhContact, szModule, szSetting, pValue[0]);
												break;
											case DBVT_WORD:
												cfg::writeWord(finalhContact, szModule, szSetting, *((WORD *)&pValue[0]));
												//_DebugTraceA("set WORD value: %s, %s, %d ON %d", szModule, item->szSetting, *((WORD *)&pValue[0]), finalhContact);
												break;
											case DBVT_DWORD:
												cfg::writeDword(finalhContact, szModule, szSetting, *((DWORD *)&pValue[0]));
												break;
											case DBVT_ASCIIZ:
												cfg::writeString(finalhContact, szModule, szSetting, (char *)pValue);
												break;
										}
									}
								} else if (item->dwFlags & BUTTON_ISTOGGLE)
									SendMessage(item->hWnd, BM_SETCHECK, 0, 0);
							}
							if (!contactOK)
								MessageBox(0, _T("The requested action requires a valid contact selection. Please select a contact from the contact list and repeat"), _T("Parameter mismatch"), MB_OK);
							if (serviceFailure) {
								char szError[512];

								mir_snprintf(szError, 512, "The service %s specified by the %s button definition was not found. You may need to install additional plugins", item->szService, item->szName);
								MessageBoxA(0, szError, "Service failure", MB_OK);
							}
							break;
						}
						item = item->nextItem;
					}
					goto buttons_done;
				}
				switch (LOWORD(wParam)) {
					case IDC_TBMENU:
					case IDC_TBTOPMENU: {
						RECT rc;
						HMENU hMenu = Menu_GetMainMenu();

						GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
						TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, LOWORD(wParam) == IDC_TBMENU ? rc.top : rc.bottom, 0, hwnd, NULL);
						return 0;
					}
					case IDC_TBGLOBALSTATUS:
					case IDC_TBTOPSTATUS: {
						RECT rc;
						HMENU hmenu = Menu_GetStatusMenu();
						GetWindowRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
						TrackPopupMenu(hmenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, rc.top, 0, hwnd, NULL);
						return 0;
					}
					case IDC_TBSOUND: {
						cfg::dat.soundsOff = !cfg::dat.soundsOff;
						cfg::writeByte("CLUI", "NoSounds", (BYTE)cfg::dat.soundsOff);
						cfg::writeByte("Skin", "UseSound", (BYTE)(cfg::dat.soundsOff ? 0 : 1));
						return 0;
					}
					case IDC_TBSELECTVIEWMODE:
						SendMessage(g_hwndViewModeFrame, WM_COMMAND, IDC_SELECTMODE, lParam);
						break;
					case IDC_TBCLEARVIEWMODE:
						SendMessage(g_hwndViewModeFrame, WM_COMMAND, IDC_RESETMODES, lParam);
						break;
					case IDC_TBCONFIGUREVIEWMODE:
						SendMessage(g_hwndViewModeFrame, WM_COMMAND, IDC_CONFIGUREMODES, lParam);
						break;
					case IDC_TBFINDANDADD:
						CallService(MS_FINDADD_FINDADD, 0, 0);
						return 0;
					case IDC_TBACCOUNTS:
						CallService(MS_PROTO_SHOWACCMGR, 0, 0);
						break;
					case IDC_TBOPTIONS:
						CallService("Options/OptionsCommand", 0, 0);
						return 0;
				}
			} else if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_MAINMENU), (LPARAM)(HANDLE) NULL))
				return 0;

buttons_done:
			switch (LOWORD(wParam)) {
				case ID_TRAY_EXIT:
				case ID_ICQ_EXIT:
					cfg::shutDown = 1;
					if (CallService(MS_SYSTEM_OKTOEXIT, 0, 0))
						DestroyWindow(hwnd);
					break;
				case IDC_TBMINIMIZE:
				case ID_TRAY_HIDE:
					pcli->pfnShowHide(0, 0);
					break;
				case POPUP_NEWGROUP:
					SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
					CallService(MS_CLIST_GROUPCREATE, 0, 0);
					break;
				case POPUP_HIDEOFFLINE:
				case IDC_TBHIDEOFFLINE:
					CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM)(-1), 0);
					break;
				case POPUP_HIDEOFFLINEROOT:
					SendMessage(pcli->hwndContactTree, CLM_SETHIDEOFFLINEROOT, !SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0), 0);
					break;
				case POPUP_HIDEEMPTYGROUPS: {
					int newVal = !(GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
					cfg::writeByte("CList", "HideEmptyGroups", (BYTE) newVal);
					SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
					break;
				}
				case POPUP_DISABLEGROUPS:
				case IDC_TBHIDEGROUPS: {
					int newVal = !(GetWindowLong(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
					cfg::writeByte("CList", "UseGroups", (BYTE) newVal);
					SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
					CheckDlgButton(hwnd, IDC_TBHIDEGROUPS, newVal ? BST_CHECKED : BST_UNCHECKED);
					break;
				}
				case POPUP_HIDEMIRANDA:
					pcli->pfnShowHide(0, 0);
					break;
				case POPUP_VISIBILITY:
					cfg::dat.dwFlags ^= CLUI_SHOWVISI;
					break;
				case POPUP_FRAME:
					cfg::dat.dwFlags ^= CLUI_FRAME_CLISTSUNKEN;
					break;
				case POPUP_BUTTONS:
					cfg::dat.dwFlags ^= CLUI_FRAME_SHOWBOTTOMBUTTONS;
					break;
				case POPUP_SHOWSTATUSICONS:
					cfg::dat.dwFlags ^= CLUI_FRAME_STATUSICONS;
					break;
				case POPUP_FLOATER:
					cfg::dat.bUseFloater ^= CLUI_USE_FLOATER;
					if (cfg::dat.bUseFloater & CLUI_USE_FLOATER) {
						SFL_Create();
						SFL_SetState(-1);
					} else
						SFL_Destroy();
					cfg::writeByte("CLUI", "FloaterMode", cfg::dat.bUseFloater);
					break;
				case POPUP_FLOATER_AUTOHIDE:
					cfg::dat.bUseFloater ^= CLUI_FLOATER_AUTOHIDE;
					SFL_SetState(cfg::dat.bUseFloater & CLUI_FLOATER_AUTOHIDE ? (cfg::getByte("CList", "State", SETTING_STATE_NORMAL) == SETTING_STATE_NORMAL ? 0 : 1) : 1);
					cfg::writeByte("CLUI", "FloaterMode", cfg::dat.bUseFloater);
					break;
				case POPUP_FLOATER_EVENTS:
					cfg::dat.bUseFloater ^= CLUI_FLOATER_EVENTS;
					SFL_SetSize();
					SFL_Update(0, 0, 0, NULL, FALSE);
					cfg::writeByte("CLUI", "FloaterMode", cfg::dat.bUseFloater);
					break;
			}
			if (dwOldFlags != cfg::dat.dwFlags) {
				InvalidateRect(pcli->hwndContactTree, NULL, FALSE);
				cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);
				if ((dwOldFlags & (CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_CLISTSUNKEN)) != (cfg::dat.dwFlags & (CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_CLISTSUNKEN))) {
					configureWindowLayout();
					configureGeometry(1);
				}
				configureEventArea(pcli->hwndContactList);
				PostMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
				PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
			}
			return FALSE;
		}
		case WM_LBUTTONDOWN: {
			if (buttonItems) {
				POINT ptMouse, pt;
				RECT rcClient;

				GetCursorPos(&ptMouse);
				pt = ptMouse;
				if (buttonItems)
					return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
				ScreenToClient(hwnd, &ptMouse);
				GetClientRect(hwnd, &rcClient);
				rcClient.bottom = Skin::metrics.dwTopOffset;
				if (PtInRect(&rcClient, ptMouse))
					return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
			}
			break;
		}
		case WM_DISPLAYCHANGE:
			SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);   //forces it to send a cln_listsizechanged
			break;
		case WM_NOTIFY:
			if (((LPNMHDR) lParam)->hwndFrom == pcli->hwndContactTree) {
				switch (((LPNMHDR) lParam)->code) {
					case CLN_LISTSIZECHANGE:
						sttProcessResize(hwnd, (NMCLISTCONTROL*)lParam);
						return FALSE;

					case NM_CLICK: {
						NMCLISTCONTROL *nm = (NMCLISTCONTROL *) lParam;
						DWORD hitFlags;
						HANDLE hItem;

						hItem = (HANDLE)SendMessage(pcli->hwndContactTree, CLM_HITTEST, (WPARAM) & hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));

						if ((hitFlags & (CLCHT_NOWHERE | CLCHT_INLEFTMARGIN | CLCHT_BELOWITEMS)) == 0)
							break;
						if (cfg::getByte("CLUI", "ClientAreaDrag", SETTING_CLIENTDRAG_DEFAULT)) {
							POINT pt;
							pt = nm->pt;
							ClientToScreen(pcli->hwndContactTree, &pt);
							return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
						}
					}
					return FALSE;
				}
			}
			break;
		case WM_CONTEXTMENU: {
			RECT rc;
			POINT pt;

			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);
			// x/y might be -1 if it was generated by a kb click
			GetWindowRect(pcli->hwndContactTree, &rc);
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
				HMENU hMenu;
				hMenu = Menu_BuildGroupMenu();
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyTrayMenu(hMenu);
				return 0;
			}
			GetWindowRect(pcli->hwndStatus, &rc);
			if (PtInRect(&rc, pt)) {
				HMENU hMenu;
				if (cfg::getByte("CLUI", "SBarRightClk", 0))
					hMenu = Menu_GetMainMenu();
				else
					hMenu = Menu_GetStatusMenu();
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				return 0;
			}
		}
		break;

		case WM_MEASUREITEM:
			if (((LPMEASUREITEMSTRUCT) lParam)->itemData == MENU_MIRANDAMENU) {
				((LPMEASUREITEMSTRUCT) lParam)->itemWidth = CXSMICON * 4 / 3;
				((LPMEASUREITEMSTRUCT) lParam)->itemHeight = 0;
				return TRUE;
			}
			return Menu_MeasureItem((LPMEASUREITEMSTRUCT)lParam);
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;

			if (hbmLockedPoint == 0) {
				hdcLockedPoint = CreateCompatibleDC(dis->hDC);
				hbmLockedPoint = CreateCompatibleBitmap(dis->hDC, 5, 5);
				hbmOldLockedPoint = reinterpret_cast<HBITMAP>(SelectObject(hdcLockedPoint, hbmLockedPoint));
			}
			if (dis->hwndItem == pcli->hwndStatus) {
				ProtocolData *pd = (ProtocolData *)dis->itemData;
				int nParts = SendMessage(pcli->hwndStatus, SB_GETPARTS, 0, 0);
				char *szProto;
				int status, x;
				SIZE textSize;
				BYTE showOpts = cfg::getByte("CLUI", "SBarShow", 1);
				if (IsBadCodePtr((FARPROC)pd))
					return TRUE;
				if (cfg::shutDown)
					return TRUE;
				szProto = pd->RealName;
				status = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
				SetBkMode(dis->hDC, TRANSPARENT);
				x = dis->rcItem.left;

				if (showOpts & 1) {
					HICON hIcon;

					if (status >= ID_STATUS_CONNECTING && status < ID_STATUS_OFFLINE) {
						char szBuffer[128];
						mir_snprintf(szBuffer, 128, "%s_conn", pd->RealName);
						hIcon = IcoLib_GetIcon(szBuffer);
					} else
						hIcon = Skin_LoadProtoIcon(szProto, status);

					if (!(showOpts & 6) && cfg::dat.bEqualSections)
						x = (dis->rcItem.left + dis->rcItem.right - 16) >> 1;
					if (pd->protopos == 0)
						x += (cfg::dat.bEqualSections ? (Skin::metrics.cLeft / 2) : Skin::metrics.cLeft);
					else if (pd->protopos == nParts - 1)
						x -= (Skin::metrics.cRight / 2);
					DrawIconEx(dis->hDC, x, 2, hIcon, 16, 16, 0, NULL, DI_NORMAL);
					IcoLib_ReleaseIcon(hIcon);

					if (cfg::getByte("CLUI", "sbar_showlocked", 1)) {
						if (cfg::getByte(szProto, "LockMainStatus", 0)) {
							hIcon = Skin_LoadIcon(SKINICON_OTHER_STATUS_LOCKED);
							if (hIcon != NULL) {
								DrawIconEx(dis->hDC, x, 2, hIcon, 16, 16, 0, NULL, DI_NORMAL);
								IcoLib_ReleaseIcon(hIcon);
							}
						}
					}
					x += 18;
				} else {
					x += 2;
					if (pd->protopos == 0)
						x += (cfg::dat.bEqualSections ? (Skin::metrics.cLeft / 2) : Skin::metrics.cLeft);
					else if (pd->protopos == nParts - 1)
						x -= (Skin::metrics.cRight / 2);
				}
				dis->rcItem.bottom += (dis->rcItem.top + 16);
				if (showOpts & 2) {
					wchar_t szName[64];
					PROTOACCOUNT* pa = Proto_GetAccount( szProto );
					if ( pa ) {
						lstrcpyn(szName, pa->tszAccountName, _countof(szName));
						szName[_countof(szName) - 1] = 0;
					}
					else szName[0] = 0;

					if (lstrlen(szName) < sizeof(szName) - 1)
						lstrcat(szName, _T(" "));
					GetTextExtentPoint32(dis->hDC, szName, lstrlen(szName), &textSize);
					dis->rcItem.left += x;
					Gfx::renderText(dis->hDC, (HANDLE)dis->CtlID, szName, &dis->rcItem, DT_VCENTER | DT_SINGLELINE, 0);
					x += textSize.cx;
				}
				if (showOpts & 4) {
					wchar_t *szStatus = pcli->pfnGetStatusModeDescription( status, 0 );
					dis->rcItem.left += x;
					Gfx::renderText(dis->hDC, (HANDLE)dis->CtlID, szStatus, &dis->rcItem, DT_VCENTER | DT_SINGLELINE, 0);
				}
			} else if (dis->CtlType == ODT_MENU) {
				if (dis->itemData == MENU_MIRANDAMENU)
					break;
				return Menu_DrawItem(dis);
			}
			return 0;
		}

		case WM_CLOSE:
			if(SETTING_WINDOWSTYLE_DEFAULT == Skin::metrics.bWindowStyle && !cfg::getByte("CList", "AlwaysHideOnTB", 0)) {
				PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return(0);
			}
			pcli->pfnShowHide(0, 0);
			return(0);

		case CLUIINTM_REDRAW:
			Redraw();
			return 0;
		case CLUIINTM_STATUSBARUPDATE:
			CluiProtocolStatusChanged(0, 0);
			return 0;

		case WM_THEMECHANGED:
			Api::updateState();
			break;

		case WM_NCDESTROY:
#if _USE_D2D
			if(renderTarget) {
				renderTarget->Release();
				renderTarget = 0;
			}
#endif
			break;

		case WM_DESTROY:
			if (cfg::dat.hdcBg) {
				SelectObject(cfg::dat.hdcBg, cfg::dat.hbmBgOld);
				DeleteObject(cfg::dat.hbmBg);
				DeleteDC(cfg::dat.hdcBg);
				cfg::dat.hdcBg = NULL;
			}
			FreeProtocolData();
			if (hdcLockedPoint) {
				SelectObject(hdcLockedPoint, hbmOldLockedPoint);
				DeleteObject(hbmLockedPoint);
				DeleteDC(hdcLockedPoint);
			}
			/*
			 * if this has not yet been set, do it now.
			 * indicates that clist is shutting down and prevents various things
			 * from happening at shutdown.
			 */
			if (!cfg::shutDown)
				cfg::shutDown = 1;
			CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)hFrameContactTree, (LPARAM)0);
			break;
	}
	return saveContactListWndProc(hwnd, msg, wParam, lParam);
}

static int MetaChanged(WPARAM wParam, LPARAM lParam)
{
	pcli->pfnClcBroadcast(INTM_METACHANGEDEVENT, wParam, lParam);
	return 0;
}

static BOOL g_AboutDlgActive = 0;

INT_PTR CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;

	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			{
				int h;
				HFONT hFont;
				LOGFONT lf;

				g_AboutDlgActive = TRUE;
				hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_CLNICER, WM_GETFONT, 0, 0);
				GetObject(hFont, sizeof(lf), &lf);
				h = lf.lfHeight;
				lf.lfHeight = (int)(lf.lfHeight * 1.5);
				lf.lfWeight = FW_BOLD;
				hFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_CLNICER, WM_SETFONT, (WPARAM)hFont, 0);
				lf.lfHeight = h;
				hFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_SETFONT, (WPARAM)hFont, 0);
			}
			{
				char str[64];
				DWORD v = pluginInfo.version;
				mir_snprintf(str, sizeof(str), "%s %d.%d.%d.%d (Unicode)", Translate("Version"), HIBYTE(HIWORD(v)), LOBYTE(HIWORD(v)), HIBYTE(LOWORD(v)), LOBYTE(LOWORD(v)));
				SetDlgItemTextA(hwndDlg, IDC_VERSION, str);
				mir_snprintf(str, sizeof(str), Translate("Built %s %s"), __DATE__, __TIME__);
				SetDlgItemTextA(hwndDlg, IDC_BUILDTIME, str);
			}
			hIcon = LoadIcon(GetModuleHandleA("miranda32.exe"), MAKEINTRESOURCE(102));
			SendDlgItemMessage(hwndDlg, IDC_LOGO, STM_SETICON, (WPARAM)hIcon, 0);
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			DestroyIcon(hIcon);
			return TRUE;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hwndDlg);
					return TRUE;
				case IDC_SUPPORT:
					//CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://miranda-im.org/download/details.php?action=viewfile&id=2365");
					break;
			}
			break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_WHITERECT)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_CLNICER)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_VERSION)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_BUILDTIME)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_COPYRIGHT)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_SUPPORT)
					|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_LOGO)) {
				if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_CLNICER))
					SetTextColor((HDC)wParam, RGB(180, 10, 10));
				else if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_VERSION))
					SetTextColor((HDC)wParam, RGB(70, 70, 70));
				else
					SetTextColor((HDC)wParam, RGB(0, 0, 0));
				SetBkColor((HDC)wParam, RGB(255, 255, 255));
				return (INT_PTR)GetStockObject(WHITE_BRUSH);
			}
			break;
		case WM_DESTROY: {
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_CLNICER, WM_GETFONT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_CLNICER, WM_SETFONT, SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0), 0);
			DeleteObject(hFont);
			hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_GETFONT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_VERSION, WM_SETFONT, SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0), 0);
			DeleteObject(hFont);
			g_AboutDlgActive = FALSE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CLN_ShowAbout(WPARAM wParam, LPARAM lParam)
{
	if (!g_AboutDlgActive)
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CLNABOUT), 0, DlgProcAbout, 0);
	return 0;
}

static INT_PTR CLN_ShowMainMenu(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	POINT pt;

	hMenu = Menu_GetMainMenu();
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
	return 0;
}

static INT_PTR CLN_ShowStatusMenu(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	POINT pt;

	hMenu = Menu_GetStatusMenu();
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
	return 0;
}

#define MS_CLUI_SHOWMAINMENU    "CList/ShowMainMenu"
#define MS_CLUI_SHOWSTATUSMENU  "CList/ShowStatusMenu"

void CLUI::loadModule(void)
{
	WNDCLASSW wndclass;

	HookEvent(ME_SYSTEM_MODULESLOADED, CLUI::modulesLoaded);
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaChanged);
	HookEvent(ME_MC_SUBCONTACTSCHANGED, MetaChanged);

	InitGroupMenus();

	wndclass.style = 0;
	wndclass.lpfnWndProc = eventAreaWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_hInst;
	wndclass.hIcon = 0;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE);
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = L"EventAreaClass";
	RegisterClassW(&wndclass);

	oldhideoffline = cfg::getByte("CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	cluiPos.left = cfg::getDword("CList", "x", 600);
	cluiPos.top = cfg::getDword("CList", "y", 200);
	cluiPos.right = cfg::getDword("CList", "Width", 150);
	cluiPos.bottom = cfg::getDword("CList", "Height", 350);

	loadExtraIconModule();
	SFL_RegisterWindowClass();

	preCreateCLC(pcli->hwndContactList);
	//cfg::FrameMgr = new CLUIFrames();
	CreateServiceFunction("CLN/About", CLN_ShowAbout);
	CreateServiceFunction(MS_CLUI_SHOWMAINMENU, CLN_ShowMainMenu);
	CreateServiceFunction(MS_CLUI_SHOWSTATUSMENU, CLN_ShowStatusMenu);
}



void CLUI::removeFromTaskBar(HWND hWnd)
{
    ITaskbarList *pTaskbarList = NULL;

    if (SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList,
                                   (void **)(&pTaskbarList))) &&	pTaskbarList != NULL) {
        if (SUCCEEDED(pTaskbarList->HrInit())) {
            pTaskbarList->DeleteTab(hWnd);
        }
        pTaskbarList->Release();
    }
}

void CLUI::addToTaskBar(HWND hWnd)
{
    ITaskbarList *pTaskbarList = NULL;

    if (SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, IID_ITaskbarList,
                                   (void **)(&pTaskbarList))) &&	pTaskbarList != NULL) {
        if (SUCCEEDED(pTaskbarList->HrInit())) {
            pTaskbarList->AddTab(hWnd);
        }
        pTaskbarList->Release();
    }
}

void CLUI::updateLayers()
{
	SIZE 	sz;
	RECT 	rcWin;
	POINT 	ptSrc = {0}, ptDest;
	GetWindowRect(pcli->hwndContactList, &rcWin);
	sz.cx = rcWin.right - rcWin.left;
	sz.cy = rcWin.bottom - rcWin.top;
	ptDest.x = rcWin.left;
	ptDest.y = rcWin.top;
	ulwInfo.dwFlags = ULW_ALPHA;
	ulwInfo.prcDirty = 0;
	ulwInfo.hdcSrc = cfg::dat.hdcBg;
	ulwInfo.pptDst = &ptDest;
	ulwInfo.pptSrc = &ptSrc;
	ulwInfo.psize = &sz;
	Gfx::setBitmapAlpha(cfg::dat.hbmBg, 255);
	UpdateLayeredWindowIndirect(pcli->hwndContactList, &ulwInfo);
}

void CLUI::setLayeredAttributes(COLORREF clr, BYTE alpha, DWORD flags)
{
	if(cfg::isAero)
		flags &= LWA_ALPHA;

	SetLayeredWindowAttributes(pcli->hwndContactList, clr, alpha, flags);
}
