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
#include <m_findadd.h>
#include "../cluiframes/cluiframes.h"
#include "../coolsb/coolscroll.h"

#define TM_AUTOALPHA  1
#define TIMERID_AUTOSIZE 100
#define MENU_MIRANDAMENU 0xFFFF1234

int g_fading_active = 0;

static RECT g_PreSizeRect, g_SizingRect;
static int g_sizingmethod;
static LONG g_CLUI_x_off, g_CLUI_y_off, g_CLUI_y1_off, g_CLUI_x1_off;
static RECT rcWPC;

static int transparentFocus = 1;
static byte oldhideoffline;
static int disableautoupd = 1;
HANDLE hFrameContactTree;
extern HIMAGELIST hCListImages;
extern PLUGININFOEX pluginInfo;
extern RECT old_window_rect, new_window_rect;

extern BOOL g_trayTooltipActive;
extern POINT tray_hover_pos;
extern HWND g_hwndViewModeFrame, g_hwndEventArea, g_hwndToolbarFrame;

extern ImageItem *g_CLUIImageItem;
extern HBRUSH g_CLUISkinnedBkColor;
extern HWND g_hwndSFL;
extern ButtonItem *g_ButtonItems;
extern COLORREF g_CLUISkinnedBkColorRGB;
extern FRAMEWND *wndFrameCLC;
extern HPEN g_hPenCLUIFrames;

static BYTE old_cliststate, show_on_first_autosize = FALSE;

RECT cluiPos;

TCHAR *statusNames[12];

extern LRESULT CALLBACK EventAreaWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern HANDLE hNotifyFrame;

int SortList(WPARAM wParam, LPARAM lParam);
int LoadCluiServices(void);
void InitGroupMenus();
void FS_RegisterFonts();
void LoadExtraIconModule();
void RemoveFromTaskBar(HWND hWnd);
void DestroyTrayMenu(HMENU hMenu);

extern LONG g_cxsmIcon, g_cysmIcon;

SIZE g_oldSize = {0};
POINT g_oldPos = {0};
int during_sizing = 0;
extern int dock_prevent_moving;

static HDC hdcLockedPoint = 0;
static HBITMAP hbmLockedPoint = 0, hbmOldLockedPoint = 0;

HICON overlayicons[10];

static IconItem myIcons[] = {
	{ LPGEN("Toggle show online/offline"),	"CLN_online",		IDI_HIDEOFFLINE },
	{ LPGEN("Toggle groups"),				"CLN_groups",		IDI_HIDEGROUPS },
	{ LPGEN("Find contacts"),				"CLN_findadd",		IDI_FINDANDADD },
	{ LPGEN("Open preferences"),			"CLN_options",		IDI_TBOPTIONS },
	{ LPGEN("Toggle sounds"),				"CLN_sound",		IDI_SOUNDSON },
	{ LPGEN("Minimize contact list"),		"CLN_minimize",		IDI_MINIMIZE },
	{ LPGEN("Show TabSRMM session list"),	"CLN_slist",		IDI_TABSRMMSESSIONLIST },
	{ LPGEN("Show TabSRMM menu"),			"CLN_menu",			IDI_TABSRMMMENU },
	{ LPGEN("Sounds are off"),				"CLN_soundsoff",	IDI_SOUNDSOFF },
	{ LPGEN("Select view mode"),			"CLN_CLVM_select",	IDI_CLVM_SELECT },
	{ LPGEN("Reset view mode"),				"CLN_CLVM_reset",	IDI_DELETE },
	{ LPGEN("Configure view modes"),		"CLN_CLVM_options",	IDI_CLVM_OPTIONS },
	{ LPGEN("Show menu"),					"CLN_topmenu",		IDI_TBTOPMENU },
	{ LPGEN("Setup accounts"),				"CLN_accounts",		IDI_TBACCOUNTS }
};

HWND hTbMenu, hTbGlobalStatus;

static void Tweak_It(COLORREF clr)
{
	SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(pcli->hwndContactList, clr, 0, LWA_COLORKEY);
	cfg::dat.colorkey = clr;
}

static void LayoutButtons(HWND hwnd, RECT *rc)
{
	RECT rect;
	BYTE rightButton = 1, leftButton = 0;
	BYTE left_offset = cfg::dat.bCLeft - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0);
	BYTE right_offset = cfg::dat.bCRight - (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? 3 : 0);
	BYTE delta = left_offset + right_offset;
	ButtonItem *btnItems = g_ButtonItems;

	if (rc == NULL)
		GetClientRect(hwnd, &rect);
	else
		rect = *rc;

	rect.bottom -= cfg::dat.bCBottom;

	if (g_ButtonItems) {
		while (btnItems) {
			LONG x = (btnItems->xOff >= 0) ? rect.left + btnItems->xOff : rect.right - abs(btnItems->xOff);
			LONG y = (btnItems->yOff >= 0) ? rect.top + btnItems->yOff : rect.bottom - cfg::dat.statusBarHeight;

			SetWindowPos(btnItems->hWnd, 0, x, y, btnItems->width, btnItems->height, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);
			btnItems = btnItems->nextItem;
		}
	}

	SetWindowPos(hTbMenu, 0, 2 + left_offset, rect.bottom - cfg::dat.statusBarHeight - 21 - 1,
		21 * 3, 21 + 1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

	SetWindowPos(hTbGlobalStatus, 0, left_offset + (3 * 21) + 3, rect.bottom - cfg::dat.statusBarHeight - 21 - 1,
		rect.right - delta - (3 * 21 + 5), 21 + 1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS | SWP_NOREDRAW);

}

static int FS_FontsChanged(WPARAM wParam, LPARAM lParam)
{
	COLORREF clr_cluiframes = cfg::getDword("CLUI", "clr_frameborder", RGB(40, 40, 40));

	if (g_hPenCLUIFrames)
		DeleteObject(g_hPenCLUIFrames);
	g_hPenCLUIFrames = CreatePen(PS_SOLID, 1, clr_cluiframes);

	pcli->pfnClcOptionsChanged();
	RedrawWindow(pcli->hwndContactList, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	return 0;
}

// create the CLC control, but not yet the frame. The frame containing the CLC should be created as the
// last frame of all.
static HWND PreCreateCLC(HWND parent)
{
	pcli->hwndContactTree = CreateWindow(_T(CLISTCONTROL_CLASS), _T(""),
		WS_CHILD | CLS_CONTACTLIST
		| (cfg::getByte(NULL, "CList", "UseGroups", SETTING_USEGROUPS_DEFAULT) ? CLS_USEGROUPS : 0)
		| CLS_HIDEOFFLINE
		//| (db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? CLS_HIDEOFFLINE : 0)
		| (cfg::getByte(NULL, "CList", "HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) ? CLS_HIDEEMPTYGROUPS : 0)
		| CLS_MULTICOLUMN,
		0, 0, 0, 0, parent, NULL, g_hInst, (LPVOID)0xff00ff00);

	cfg::clcdat = (struct ClcData *)GetWindowLongPtr(pcli->hwndContactTree, 0);
	return pcli->hwndContactTree;
}

// create internal frames, including the last frame (actual CLC control)
static int CreateCLC(HWND parent)
{
	pcli->pfnReloadExtraIcons();
	CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM)oldhideoffline, 0);
	disableautoupd = 0;

	{
		CLISTFrame frame = {0};
		frame.cbSize = sizeof(frame);
		frame.tname = _T("EventArea");
		frame.TBtname = TranslateT("Event area");
		frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
		frame.height = 20;
		frame.Flags = F_VISIBLE | F_SHOWTBTIP | F_NOBORDER | F_TCHAR;
		frame.align = alBottom;
		frame.hWnd = CreateWindowExA(0, "EventAreaClass", "evt", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20, pcli->hwndContactList, (HMENU)0, g_hInst, NULL);
		g_hwndEventArea = frame.hWnd;
		hNotifyFrame = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&frame, 0);
		CallService(MS_CLIST_FRAMES_UPDATEFRAME, (WPARAM)hNotifyFrame, FU_FMPOS);
		HideShowNotifyFrame();
		CreateViewModeFrame();
	}

	{
		CLISTFrame Frame = {0};
		memset(&Frame, 0, sizeof(Frame));
		Frame.cbSize = sizeof(CLISTFrame);
		Frame.hWnd = pcli->hwndContactTree;
		Frame.align = alClient;
		Frame.hIcon = LoadSkinnedIcon(SKINICON_OTHER_FRAME);
		Frame.Flags = F_VISIBLE | F_SHOWTB | F_SHOWTBTIP | F_NOBORDER | F_TCHAR;
		Frame.tname = _T("My contacts");
		Frame.TBtname = TranslateT("My contacts");
		Frame.height = 200;
		hFrameContactTree = (HWND)CallService(MS_CLIST_FRAMES_ADDFRAME, (WPARAM)&Frame, 0);
		//free(Frame.name);
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_TBTIPNAME, hFrameContactTree), (LPARAM)Translate("My contacts"));

		// ugly, but working hack. Prevent that annoying little scroll bar from appearing in the "My Contacts" title bar
		DWORD flags = (DWORD)CallService(MS_CLIST_FRAMES_GETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hFrameContactTree), 0);
		flags |= F_VISIBLE;
		CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, MAKEWPARAM(FO_FLAGS, hFrameContactTree), flags);
	}

	SetButtonToSkinned();
	return 0;
}

static int CluiModulesLoaded(WPARAM, LPARAM)
{
	FS_RegisterFonts();
	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
	return 0;
}

static HICON hIconSaved = 0;

void ClearIcons(int mode)
{
	for (int i = IDI_OVL_OFFLINE; i <= IDI_OVL_OUTTOLUNCH; i++) {
		if (overlayicons[i - IDI_OVL_OFFLINE] != 0) {
			if (mode)
				DestroyIcon(overlayicons[i - IDI_OVL_OFFLINE]);
			overlayicons[i - IDI_OVL_OFFLINE] = 0;
		}
	}
}

static void CacheClientIcons()
{
	ClearIcons(0);

	for (int i = IDI_OVL_OFFLINE; i <= IDI_OVL_OUTTOLUNCH; i++) {
		char szBuffer[128];
		mir_snprintf(szBuffer, sizeof(szBuffer), "cln_ovl_%d", ID_STATUS_OFFLINE + (i - IDI_OVL_OFFLINE));
		overlayicons[i - IDI_OVL_OFFLINE] = Skin_GetIcon(szBuffer);
	}
}

static void InitIcoLib()
{
	Icon_Register(g_hInst, LPGEN("Contact list")"/"LPGEN("Default"), myIcons, SIZEOF(myIcons));

	for (int i = IDI_OVL_OFFLINE; i <= IDI_OVL_OUTTOLUNCH; i++) {
		char szBuffer[128];
		mir_snprintf(szBuffer, sizeof(szBuffer), "cln_ovl_%d", ID_STATUS_OFFLINE + (i - IDI_OVL_OFFLINE));
		IconItemT icon = { pcli->pfnGetStatusModeDescription(ID_STATUS_OFFLINE + (i - IDI_OVL_OFFLINE), GSMDF_TCHAR), szBuffer, i };
		Icon_RegisterT(g_hInst, LPGENT("Contact list")_T("/")LPGENT("Overlay icons"), &icon, 1);
	}

	PROTOACCOUNT **accs = NULL;
	int p_count = 0;
	ProtoEnumAccounts(&p_count, &accs);
	for (int k = 0; k < p_count; k++) {
		if (!IsAccountEnabled(accs[k]) || CallProtoService(accs[k]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
			continue;

		TCHAR szDescr[128];
		mir_sntprintf(szDescr, SIZEOF(szDescr), TranslateT("%s connecting"), accs[k]->tszAccountName);
		IconItemT icon = { szDescr, "conn", IDI_PROTOCONNECTING };
		Icon_RegisterT(g_hInst, LPGENT("Contact list")_T("/")LPGENT("Connecting icons"), &icon, 1, accs[k]->szModuleName);
	}
}

static int IcoLibChanged(WPARAM, LPARAM)
{
	IcoLibReloadIcons();
	return 0;
}

void CreateButtonBar(HWND hWnd)
{
	hTbMenu = CreateWindowEx(0, MIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20, hWnd, (HMENU)IDC_TBMENU, g_hInst, NULL);
	CustomizeButton(hTbMenu, false, false, false);
	SetWindowText(hTbMenu, TranslateT("Menu"));
	SendMessage(hTbMenu, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_OTHER_MAINMENU));
	SendMessage(hTbMenu, BUTTONSETSENDONDOWN, TRUE, 0);
	SendMessage(hTbMenu, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Open main menu"), 0);

	hTbGlobalStatus = CreateWindowEx(0, MIRANDABUTTONCLASS, _T(""), BS_PUSHBUTTON | WS_CHILD | WS_TABSTOP, 0, 0, 20, 20, hWnd, (HMENU)IDC_TBGLOBALSTATUS, g_hInst, NULL);
	CustomizeButton(hTbGlobalStatus, false, false, false);
	SetWindowText(hTbGlobalStatus, TranslateT("Offline"));
	SendMessage(hTbGlobalStatus, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(SKINICON_STATUS_OFFLINE));
	SendMessage(hTbGlobalStatus, BUTTONSETSENDONDOWN, TRUE, 0);
	SendMessage(hTbGlobalStatus, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Set status modes"), 0);
}

// if mode != 0 we do first time init, otherwise only reload the extra icon stuff
void CLN_LoadAllIcons(BOOL mode)
{
	if (mode) {
		InitIcoLib();
		HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibChanged);
	}
	CacheClientIcons();
}

void ConfigureEventArea(HWND hwnd)
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
	}
	else
		cfg::dat.notifyActive = 0;

	if (oldstate != cfg::dat.notifyActive)
		HideShowNotifyFrame();
}

void ConfigureFrame()
{
	int show = cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? SW_SHOW : SW_HIDE;
	ShowWindow(hTbMenu,show);
	ShowWindow(hTbGlobalStatus, show);
}

void IcoLibReloadIcons()
{
	CacheClientIcons();
	pcli->pfnReloadExtraIcons();
	pcli->pfnSetAllExtraIcons(NULL);

	pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0);
	SendMessage(g_hwndViewModeFrame, WM_USER + 100, 0, 0);
}

void ConfigureCLUIGeometry(int mode)
{
	RECT rcStatus;
	DWORD clmargins = cfg::getDword("CLUI", "clmargins", 0);

	cfg::dat.bCLeft = LOBYTE(LOWORD(clmargins));
	cfg::dat.bCRight = HIBYTE(LOWORD(clmargins));
	cfg::dat.bCTop = LOBYTE(HIWORD(clmargins));
	cfg::dat.bCBottom = HIBYTE(HIWORD(clmargins));

	if (mode) {
		if (cfg::dat.dwFlags & CLUI_FRAME_SBARSHOW) {
			SendMessage(pcli->hwndStatus, WM_SIZE, 0, 0);
			GetWindowRect(pcli->hwndStatus, &rcStatus);
			cfg::dat.statusBarHeight = (rcStatus.bottom - rcStatus.top);
		}
		else
			cfg::dat.statusBarHeight = 0;
	}

	cfg::dat.topOffset = cfg::dat.bCTop;
	cfg::dat.bottomOffset = (cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? 2 + 21 : 0) + cfg::dat.bCBottom;

	if (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN) {
		cfg::dat.topOffset += 2;
		cfg::dat.bottomOffset += 2;
		cfg::dat.bCLeft += 3;
		cfg::dat.bCRight += 3;
	}
}

// set the states of defined database action buttons (only if button is a toggle)
void SetDBButtonStates(MCONTACT hPassedContact)
{
	ButtonItem *buttonItem = g_ButtonItems;
	MCONTACT hContact = 0, hFinalContact = 0;
	char *szModule, *szSetting;
	int sel = cfg::clcdat ? cfg::clcdat->selection : -1;
	ClcContact *contact = 0;

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
		}
		else
			hFinalContact = 0;

		if (buttonItem->type == DBVT_ASCIIZ) {
			DBVARIANT dbv = {0};

			if (!cfg::getString(hFinalContact, szModule, szSetting, &dbv)) {
				result = !strcmp((char *)buttonItem->bValuePush, dbv.pszVal);
				db_free(&dbv);
			}
		}
		else {
			switch (buttonItem->type) {
				case DBVT_BYTE: {
					BYTE val = cfg::getByte(hFinalContact, szModule, szSetting, 0);
					result = (val == buttonItem->bValuePush[0]);
					break;
				}
				case DBVT_WORD: {
					WORD val = cfg::getWord(hFinalContact, szModule, szSetting, 0);
					result = (val == *((WORD *)&buttonItem->bValuePush));
					break;
				}
				case DBVT_DWORD: {
					DWORD val = cfg::getDword(hFinalContact, szModule, szSetting, 0);
					result = (val == *((DWORD *)&buttonItem->bValuePush));
					break;
				}
			}
		}
		SendMessage(buttonItem->hWnd, BM_SETCHECK, (WPARAM)result, 0);
		buttonItem = buttonItem->nextItem;
	}
}

// set states of standard buttons (pressed/unpressed)
void SetButtonStates(HWND hwnd)
{
	ButtonItem *buttonItem = g_ButtonItems;

	if (g_ButtonItems) {
		while (buttonItem) {
			if (buttonItem->dwFlags & BUTTON_ISINTERNAL) {
				switch (buttonItem->uId) {
					case IDC_STBSOUND:
							SendMessage(buttonItem->hWnd, BM_SETCHECK, cfg::dat.soundsOff ? BST_CHECKED : BST_UNCHECKED, 0);
							break;
					case IDC_STBHIDEOFFLINE:
							SendMessage(buttonItem->hWnd, BM_SETCHECK, cfg::getByte("CList", "HideOffline", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
							break;
					case IDC_STBHIDEGROUPS:
							SendMessage(buttonItem->hWnd, BM_SETCHECK, cfg::getByte("CList", "UseGroups", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
							break;
				}
			}
			buttonItem = buttonItem->nextItem;
		}
	}
}

void BlitWallpaper(HDC hdc, RECT *rc, RECT *rcPaint, struct ClcData *dat)
{
	int x, y;
	int bitx, bity;
	int maxx, maxy;
	int destw, desth, height, width;
	BITMAP *bmp = &cfg::dat.bminfoBg;
	LONG clip = cfg::dat.bClipBorder;

	if (dat == 0)
		return;

	SetStretchBltMode(hdc, HALFTONE);

	y = rc->top;

	rc->left = max(rc->left, clip);
	rc->right = min(rc->right - clip, rc->right);
	rc->top = max(rc->top, clip);
	rc->bottom = min(rc->bottom - clip, rc->bottom);

	width = rc->right - rc->left;
	height = rc->bottom - rc->top;
	HRGN my_rgn = CreateRectRgn(rc->left, rc->top, rc->right, rc->bottom);
	SelectClipRgn(hdc, my_rgn);
	maxx = dat->backgroundBmpUse & CLBF_TILEH ? rc->right : rc->left + 1;
	maxy = dat->backgroundBmpUse & CLBF_TILEV ? maxy = rc->bottom : y + 1;
	switch (dat->backgroundBmpUse & CLBM_TYPE) {
		case CLB_STRETCH:
			if (dat->backgroundBmpUse & CLBF_PROPORTIONAL) {
				if (width * bmp->bmHeight < height * bmp->bmWidth) {
					desth = height;
					destw = desth * bmp->bmWidth / bmp->bmHeight;
				}
				else {
					destw = width;
					desth = destw * bmp->bmHeight / bmp->bmWidth;
				}
			}
			else {
				destw = width;
				desth = height;
			}
			break;
		case CLB_STRETCHH:
			if (dat->backgroundBmpUse & CLBF_PROPORTIONAL) {
				destw = width;
				desth = destw * bmp->bmHeight / bmp->bmWidth;
			}
			else {
				destw = width;
				desth = bmp->bmHeight;
			}
			break;
		case CLB_STRETCHV:
			if (dat->backgroundBmpUse & CLBF_PROPORTIONAL) {
				desth = height;
				destw = desth * bmp->bmWidth / bmp->bmHeight;
			}
			else {
				destw = bmp->bmWidth;
				desth = height;
			}
			break;
		default:
			//clb_topleft
			destw = bmp->bmWidth;
			desth = bmp->bmHeight;
			break;
	}

	bitx = 0;
	bity = 0;
	for (; y < maxy; y += desth) {
		for (x = rc->left; x < maxx; x += destw)
			StretchBlt(hdc, x, y, destw, desth, cfg::dat.hdcPic, bitx, bity, bmp->bmWidth, bmp->bmHeight, SRCCOPY);
	}
	SelectClipRgn(hdc, NULL);
	DeleteObject(my_rgn);
}

void ReloadThemedOptions()
{
	cfg::dat.bSkinnedStatusBar = 	cfg::getByte("CLUI", "sb_skinned", 0);
	cfg::dat.bUsePerProto = 		cfg::getByte("CLCExt", "useperproto", 0);
	cfg::dat.bOverridePerStatusColors = cfg::getByte("CLCExt", "override_status", 0);
	cfg::dat.bRowSpacing = 			cfg::getByte("CLC", "RowGap", 0);
	cfg::dat.bApplyIndentToBg = 	cfg::getByte("CLCExt", "applyindentbg", 0);
	cfg::dat.bWallpaperMode = 		cfg::getByte("CLUI", "UseBkSkin", 1);
	cfg::dat.bClipBorder = 			cfg::getByte("CLUI", "clipborder", 0);
	cfg::dat.cornerRadius = 		cfg::getByte("CLCExt", "CornerRad", 6);
	cfg::dat.gapBetweenFrames = 	(BYTE)cfg::getDword("CLUIFrames", "GapBetweenFrames", 1);
	cfg::dat.bUseDCMirroring = 		cfg::getByte("CLC", "MirrorDC", 0);
	cfg::dat.bGroupAlign = 			cfg::getByte("CLC", "GroupAlign", 0);
	if (cfg::dat.hBrushColorKey)
		DeleteObject(cfg::dat.hBrushColorKey);
	cfg::dat.hBrushColorKey = CreateSolidBrush(RGB(255, 0, 255));
	cfg::dat.bWantFastGradients = 	cfg::getByte("CLCExt", "FastGradients", 0);
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
	winstyle = GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
	HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMon, &mi))
		rcWorkArea = mi.rcWork;

	if (nmc->pt.y > (rcWorkArea.bottom - rcWorkArea.top)) {
		nmc->pt.y = (rcWorkArea.bottom - rcWorkArea.top);
	}

	if (winstyle & CLS_SKINNEDFRAME) {
		BOOL hasTitleBar = wndFrameCLC ? wndFrameCLC->TitleBar.ShowTitleBar : 0;
		StatusItems_t *item = arStatusItems[(hasTitleBar ? ID_EXTBKOWNEDFRAMEBORDERTB : ID_EXTBKOWNEDFRAMEBORDER) - ID_STATUS_OFFLINE];
		skinHeight = item->IGNORED ? 0 : item->MARGIN_BOTTOM + item->MARGIN_TOP;
	}

	newHeight = max(nmc->pt.y, 3) + 1 + ((winstyle & WS_BORDER) ? 2 : 0) + skinHeight + (rcWindow.bottom - rcWindow.top) - (rcTree.bottom - rcTree.top);
	if (newHeight == (rcWindow.bottom - rcWindow.top) && show_on_first_autosize == FALSE)
		return;

	if (newHeight > (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100)
		newHeight = (rcWorkArea.bottom - rcWorkArea.top) * maxHeight / 100;
	if (cfg::getByte("CLUI", "AutoSizeUpward", 0)) {
		rcWindow.top = rcWindow.bottom - newHeight;
		if (rcWindow.top < rcWorkArea.top) rcWindow.top = rcWorkArea.top;
	}
	else {
		rcWindow.bottom = rcWindow.top + newHeight;
		if (rcWindow.bottom > rcWorkArea.bottom) rcWindow.bottom = rcWorkArea.bottom;
	}
	if (cfg::dat.szOldCTreeSize.cx != rcTree.right - rcTree.left) {
		cfg::dat.szOldCTreeSize.cx = rcTree.right - rcTree.left;
		return;
	}
	KillTimer(hwnd, TIMERID_AUTOSIZE);
	SetTimer(hwnd, TIMERID_AUTOSIZE, 100, 0);
}

int CustomDrawScrollBars(NMCSBCUSTOMDRAW *nmcsbcd)
{
	switch (nmcsbcd->hdr.code) {
	case NM_COOLSB_CUSTOMDRAW:
		static HDC hdcScroll = 0;
		static HBITMAP hbmScroll, hbmScrollOld;
		static LONG scrollLeft, scrollRight, scrollHeight, scrollYmin, scrollYmax;

		switch (nmcsbcd->dwDrawStage) {
		case CDDS_PREPAINT:
			if (cfg::dat.bSkinnedScrollbar) // XXX fix (verify skin items to be complete, otherwise don't draw
				return CDRF_SKIPDEFAULT;
			return CDRF_DODEFAULT;

		case CDDS_POSTPAINT:
			return 0;

		case CDDS_ITEMPREPAINT:
			HDC hdc = nmcsbcd->hdc;
			StatusItems_t *item = 0, *arrowItem = 0;
			UINT uItemID = ID_EXTBKSCROLLBACK;
			RECT rcWindow;
			POINT pt;
			DWORD dfcFlags;
			HRGN rgn = 0;
			GetWindowRect(pcli->hwndContactTree, &rcWindow);
			pt.x = rcWindow.left;
			pt.y = rcWindow.top;
			ScreenToClient(pcli->hwndContactList, &pt);
			hdcScroll = hdc;
			BitBlt(hdcScroll, nmcsbcd->rect.left, nmcsbcd->rect.top, nmcsbcd->rect.right - nmcsbcd->rect.left,
				nmcsbcd->rect.bottom - nmcsbcd->rect.top, cfg::dat.hdcBg, pt.x + nmcsbcd->rect.left, pt.y + nmcsbcd->rect.top, SRCCOPY);

			switch (nmcsbcd->uItem) {
			case HTSCROLL_UP:
			case HTSCROLL_DOWN:
				uItemID = (nmcsbcd->uState == CDIS_DEFAULT || nmcsbcd->uState == CDIS_DISABLED) ? ID_EXTBKSCROLLBUTTON :
					(nmcsbcd->uState == CDIS_HOT ? ID_EXTBKSCROLLBUTTONHOVER : ID_EXTBKSCROLLBUTTONPRESSED);
				break;
			case HTSCROLL_PAGEGDOWN:
			case HTSCROLL_PAGEGUP:
				uItemID = nmcsbcd->uItem == HTSCROLL_PAGEGUP ? ID_EXTBKSCROLLBACK : ID_EXTBKSCROLLBACKLOWER;
				rgn = CreateRectRgn(nmcsbcd->rect.left, nmcsbcd->rect.top, nmcsbcd->rect.right, nmcsbcd->rect.bottom);
				SelectClipRgn(hdcScroll, rgn);
				break;
			case HTSCROLL_THUMB:
				uItemID = nmcsbcd->uState == CDIS_SELECTED ? ID_EXTBKSCROLLTHUMBPRESSED : ID_EXTBKSCROLLTHUMB;
				break;
			default:
				break;
			}

			uItemID -= ID_STATUS_OFFLINE;
			item = arStatusItems[uItemID];
			if (!item->IGNORED) {
				int alpha = nmcsbcd->uState == CDIS_DISABLED ? item->ALPHA - 50 : item->ALPHA;
				DrawAlpha(hdcScroll, &nmcsbcd->rect, item->COLOR, alpha, item->COLOR2, item->COLOR2_TRANSPARENT,
					item->GRADIENT, item->CORNER, item->BORDERSTYLE, item->imageItem);
			}
			dfcFlags = DFCS_FLAT | (nmcsbcd->uState == CDIS_DISABLED ? DFCS_INACTIVE :
				(nmcsbcd->uState == CDIS_HOT ? DFCS_HOT :
				(nmcsbcd->uState == CDIS_SELECTED ? DFCS_PUSHED : 0)));

			if (nmcsbcd->uItem == HTSCROLL_UP)
				arrowItem = arStatusItems[ID_EXTBKSCROLLARROWUP - ID_STATUS_OFFLINE];
			if (nmcsbcd->uItem == HTSCROLL_DOWN)
				arrowItem = arStatusItems[ID_EXTBKSCROLLARROWDOWN - ID_STATUS_OFFLINE];
			if (arrowItem && !arrowItem->IGNORED)
				DrawAlpha(hdcScroll, &nmcsbcd->rect, arrowItem->COLOR, arrowItem->ALPHA, arrowItem->COLOR2, arrowItem->COLOR2_TRANSPARENT,
					arrowItem->GRADIENT, arrowItem->CORNER, arrowItem->BORDERSTYLE, arrowItem->imageItem);
			else if (arrowItem)
				DrawFrameControl(hdcScroll, &nmcsbcd->rect, DFC_SCROLL, (nmcsbcd->uItem == HTSCROLL_UP ? DFCS_SCROLLUP : DFCS_SCROLLDOWN) | dfcFlags);

			if (rgn) {
				SelectClipRgn(hdcScroll, NULL);
				DeleteObject(rgn);
			}
		}
	}
	return 0;
}

extern LRESULT(CALLBACK *saveContactListWndProc)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static int ServiceParamsOK(ButtonItem *item, WPARAM *wParam, LPARAM *lParam, MCONTACT hContact)
{
	if (item->dwFlags & BUTTON_PASSHCONTACTW || item->dwFlags & BUTTON_PASSHCONTACTL || item->dwFlags & BUTTON_ISCONTACTDBACTION) {
		if (hContact == 0)
			return 0;

		if (item->dwFlags & BUTTON_PASSHCONTACTW)
			*wParam = hContact;
		else if (item->dwFlags & BUTTON_PASSHCONTACTL)
			*lParam = hContact;
	}
	return 1;
}

static void ShowCLUI(HWND hwnd)
{
	int state = old_cliststate;
	int onTop = cfg::getByte("CList", "OnTop", SETTING_ONTOP_DEFAULT);

	SendMessage(hwnd, WM_SETREDRAW, FALSE, FALSE);
	if (!cfg::getByte("CLUI", "ShowMainMenu", SETTING_SHOWMAINMENU_DEFAULT))
		SetMenu(pcli->hwndContactList, NULL);
	if (state == SETTING_STATE_NORMAL) {
		SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
		ShowWindow(pcli->hwndContactList, SW_SHOWNORMAL);
		SendMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
	}
	else if (state == SETTING_STATE_MINIMIZED) {
		cfg::dat.forceResize = TRUE;
		ShowWindow(pcli->hwndContactList, SW_HIDE);
	}
	else if (state == SETTING_STATE_HIDDEN) {
		cfg::dat.forceResize = TRUE;
		ShowWindow(pcli->hwndContactList, SW_HIDE);
	}
	SetWindowPos(pcli->hwndContactList, onTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
	DrawMenuBar(hwnd);
	if (cfg::dat.autosize) {
		SendMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
		SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0);
	}
}

static void GetButtonRect(HWND hwnd, RECT *rc)
{
	if (hwnd)
		GetWindowRect(hwnd, rc);
	else {
		POINT pt;
		GetCursorPos(&pt);
		rc->bottom = rc->top = pt.y;
		rc->left = rc->right = pt.x;
	}
}

LRESULT CALLBACK ContactListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		{
			int flags = WS_CHILD | CCS_BOTTOM;
			flags |= cfg::getByte("CLUI", "ShowSBar", 1) ? WS_VISIBLE : 0;
			flags |= cfg::getByte("CLUI", "ShowGrip", 1) ? SBARS_SIZEGRIP : 0;
			pcli->hwndStatus = CreateWindow(STATUSCLASSNAME, NULL, flags, 0, 0, 0, 0, hwnd, NULL, g_hInst, NULL);
			if (flags & WS_VISIBLE) {
				ShowWindow(pcli->hwndStatus, SW_SHOW);
				SendMessage(pcli->hwndStatus, WM_SIZE, 0, 0);
			}
			mir_subclassWindow(pcli->hwndStatus, NewStatusBarWndProc);
			SetClassLong(pcli->hwndStatus, GCL_STYLE, GetClassLong(pcli->hwndStatus, GCL_STYLE) & ~(CS_VREDRAW | CS_HREDRAW));
		}
		g_oldSize.cx = g_oldSize.cy = 0;
		old_cliststate = cfg::getByte("CList", "State", SETTING_STATE_NORMAL);
		cfg::writeByte("CList", "State", SETTING_STATE_HIDDEN);
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_VISIBLE);
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_CLIPCHILDREN);
		if (!cfg::dat.bFirstRun)
			ConfigureEventArea(hwnd);
		ConfigureCLUIGeometry(0);
		CluiProtocolStatusChanged(0, 0);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++)
			statusNames[i - ID_STATUS_OFFLINE] = pcli->pfnGetStatusModeDescription(i, 0);

		//delay creation of CLC so that it can get the status icons right the first time (needs protocol modules loaded)
		if (cfg::dat.bLayeredHack) {
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | (WS_EX_LAYERED));
			SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
		}

		if (cfg::dat.isTransparent) {
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
		}
		transparentFocus = 1;

		TranslateMenu(GetMenu(hwnd));
		PostMessage(hwnd, M_CREATECLC, 0, 0);
		return FALSE;

	case WM_NCCREATE:
		{
			LPCREATESTRUCT p = (LPCREATESTRUCT)lParam;
			p->style &= ~(CS_HREDRAW | CS_VREDRAW);
		}
		break;

	case M_CREATECLC: {
		if (cfg::getByte("CLUI", "useskin", 0))
			IMG_LoadItems();
		CreateButtonBar(hwnd);
		//FYR: to be checked: otherwise it raises double xStatus items
		//NotifyEventHooks(pcli->hPreBuildStatusMenuEvent, 0, 0);
		SendMessage(hwnd, WM_SETREDRAW, FALSE, FALSE);
		{
			LONG style;
			BYTE windowStyle = cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_TOOLWINDOW);
			ShowWindow(pcli->hwndContactList, SW_HIDE);
			style = GetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE);
			if (windowStyle != SETTING_WINDOWSTYLE_DEFAULT) {
				style |= WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE;
				style &= ~WS_EX_APPWINDOW;
			}
			else {
				style &= ~(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE);
				if (cfg::getByte("CList", "AlwaysHideOnTB", 1))
					style &= ~WS_EX_APPWINDOW;
				else
					style |= WS_EX_APPWINDOW;
			}

			SetWindowLongPtr(pcli->hwndContactList, GWL_EXSTYLE, style);
			ApplyCLUIBorderStyle(pcli->hwndContactList);

			SetWindowPos(pcli->hwndContactList, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE);
		}

		if (cfg::dat.bSkinnedButtonMode)
			SetButtonToSkinned();
		ConfigureFrame();
		SetButtonStates(hwnd);

		CreateCLC(hwnd);
		cfg::clcdat = (struct ClcData *)GetWindowLongPtr(pcli->hwndContactTree, 0);

		if (cfg::dat.bFullTransparent) {
			if (g_CLUISkinnedBkColorRGB)
				Tweak_It(g_CLUISkinnedBkColorRGB);
			else if (cfg::dat.bClipBorder || (cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME))
				Tweak_It(RGB(255, 0, 255));
			else
				Tweak_It(cfg::clcdat->bkColour);
		}

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
		if (!cfg::dat.autosize)
			ShowCLUI(hwnd);
		else {
			show_on_first_autosize = TRUE;
			RecalcScrollBar(pcli->hwndContactTree, cfg::clcdat);
		}
		return 0;
	}
	case WM_ERASEBKGND:
		return TRUE;
		/*
		if (cfg::dat.bSkinnedButtonMode)
			return TRUE;
		return DefWindowProc(hwnd, msg, wParam, lParam);
		*/

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT rc, rcFrame, rcClient;
			HDC hdc;
			HRGN rgn = 0;
			HDC hdcReal = BeginPaint(hwnd, &ps);

			if (during_sizing)
				rcClient = rcWPC;
			else
				GetClientRect(hwnd, &rcClient);
			CopyRect(&rc, &rcClient);

			if (!cfg::dat.hdcBg || rc.right > cfg::dat.dcSize.cx || rc.bottom + cfg::dat.statusBarHeight > cfg::dat.dcSize.cy) {
				RECT rcWorkArea;

				SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
				HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO mi;
				mi.cbSize = sizeof(mi);
				if (GetMonitorInfo(hMon, &mi))
					rcWorkArea = mi.rcWork;

				cfg::dat.dcSize.cy = max(rc.bottom + cfg::dat.statusBarHeight, rcWorkArea.bottom - rcWorkArea.top);
				cfg::dat.dcSize.cx = max(rc.right, (rcWorkArea.right - rcWorkArea.left) / 2);

				if (cfg::dat.hdcBg) {
					SelectObject(cfg::dat.hdcBg, cfg::dat.hbmBgOld);
					DeleteObject(cfg::dat.hbmBg);
					DeleteDC(cfg::dat.hdcBg);
				}
				cfg::dat.hdcBg = CreateCompatibleDC(hdcReal);
				cfg::dat.hbmBg = CreateCompatibleBitmap(hdcReal, cfg::dat.dcSize.cx, cfg::dat.dcSize.cy);
				cfg::dat.hbmBgOld = reinterpret_cast<HBITMAP>(SelectObject(cfg::dat.hdcBg, cfg::dat.hbmBg));
			}

			if (cfg::shutDown) {
				EndPaint(hwnd, &ps);
				return 0;
			}

			hdc = cfg::dat.hdcBg;

			CopyRect(&rcFrame, &rcClient);
			if (g_CLUISkinnedBkColor) {
				if (cfg::dat.fOnDesktop) {
					HDC dc = GetDC(0);
					RECT rcWin;

					GetWindowRect(hwnd, &rcWin);
					BitBlt(hdc, 0, 0, rcClient.right, rcClient.bottom, dc, rcWin.left, rcWin.top, SRCCOPY);
				}
				else
					FillRect(hdc, &rcClient, g_CLUISkinnedBkColor);
			}

			if (cfg::dat.bClipBorder != 0 || cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME) {
				int docked = CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0);
				int clip = cfg::dat.bClipBorder;

				if (!g_CLUISkinnedBkColor)
					FillRect(hdc, &rcClient, cfg::dat.hBrushColorKey);
				if (cfg::dat.dwFlags & CLUI_FRAME_ROUNDEDFRAME)
					rgn = CreateRoundRectRgn(clip, docked ? 0 : clip, rcClient.right - clip + 1, rcClient.bottom - (docked ? 0 : clip - 1), 8 + clip, 8 + clip);
				else
					rgn = CreateRectRgn(clip, docked ? 0 : clip, rcClient.right - clip, rcClient.bottom - (docked ? 0 : clip));
				SelectClipRgn(hdc, rgn);
			}

			if (g_CLUIImageItem) {
				IMG_RenderImageItem(hdc, g_CLUIImageItem, &rcFrame);
				cfg::dat.ptW.x = cfg::dat.ptW.y = 0;
				ClientToScreen(hwnd, &cfg::dat.ptW);
				goto skipbg;
			}

			if (cfg::dat.bWallpaperMode)
				FillRect(hdc, &rcClient, cfg::dat.hBrushCLCBk);
			else
				FillRect(hdc, &rcClient, GetSysColorBrush(COLOR_3DFACE));

			rcFrame.left += (cfg::dat.bCLeft - 1);
			rcFrame.right -= (cfg::dat.bCRight - 1);
			rcFrame.bottom++;
			rcFrame.bottom -= cfg::dat.statusBarHeight;
			rcFrame.top += (cfg::dat.topOffset - 1);

			if (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN) {
				if (cfg::dat.bWallpaperMode && cfg::clcdat != NULL) {
					InflateRect(&rcFrame, -1, -1);
					if (cfg::dat.bmpBackground)
						BlitWallpaper(hdc, &rcFrame, &ps.rcPaint, cfg::clcdat);
					cfg::dat.ptW.x = cfg::dat.ptW.y = 0;
					ClientToScreen(hwnd, &cfg::dat.ptW);
				}
				InflateRect(&rcFrame, 1, 1);
				if (cfg::dat.bSkinnedButtonMode)
					rcFrame.bottom -= (cfg::dat.bottomOffset);
				DrawEdge(hdc, &rcFrame, BDR_SUNKENOUTER, BF_RECT);
			}
			else if (cfg::dat.bWallpaperMode && cfg::clcdat != NULL) {
				if (cfg::dat.bmpBackground)
					BlitWallpaper(hdc, &rcFrame, &ps.rcPaint, cfg::clcdat);
				cfg::dat.ptW.x = cfg::dat.ptW.y = 0;
				ClientToScreen(hwnd, &cfg::dat.ptW);
			}
skipbg:
			BitBlt(hdcReal, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdc, 0, 0, SRCCOPY);
			if (rgn) {
				SelectClipRgn(hdc, NULL);
				DeleteObject(rgn);
			}
			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		{
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
		}
		break;

	case WM_EXITSIZEMOVE:
		PostMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
		break;

	case WM_SIZING:
		break;
		/*
		RECT *szrect = (RECT *)lParam;
		if (Docking_IsDocked(0, 0))
			break;
		g_SizingRect = *((RECT *)lParam);
		if (wParam != WMSZ_BOTTOM && wParam != WMSZ_BOTTOMRIGHT && wParam != WMSZ_BOTTOMLEFT)
			szrect->bottom = g_PreSizeRect.bottom;
		if (wParam != WMSZ_RIGHT && wParam != WMSZ_BOTTOMRIGHT && wParam != WMSZ_TOPRIGHT)
			szrect->right = g_PreSizeRect.right;
		*/
		return TRUE;

	case WM_WINDOWPOSCHANGED:
		if (Docking_IsDocked(0, 0))
			break;

	case WM_WINDOWPOSCHANGING:
		if (pcli->hwndContactList != NULL) {
			WINDOWPOS *wp = (WINDOWPOS *)lParam;
			if (wp && wp->flags & SWP_NOSIZE)
				return FALSE;

			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			during_sizing = true;

			new_window_rect.left = 0;
			new_window_rect.right = wp->cx - (g_CLUI_x_off + g_CLUI_x1_off);
			new_window_rect.top = 0;
			new_window_rect.bottom = wp->cy - g_CLUI_y_off - g_CLUI_y1_off;

			if (cfg::dat.dwFlags & CLUI_FRAME_SBARSHOW) {
				RECT rcStatus;
				SetWindowPos(pcli->hwndStatus, 0, 0, new_window_rect.bottom - 20, new_window_rect.right, 20, SWP_NOZORDER);
				GetWindowRect(pcli->hwndStatus, &rcStatus);
				cfg::dat.statusBarHeight = (rcStatus.bottom - rcStatus.top);
				if (wp->cx != g_oldSize.cx)
					SendMessage(hwnd, CLUIINTM_STATUSBARUPDATE, 0, 0);
				RedrawWindow(pcli->hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			else
				cfg::dat.statusBarHeight = 0;

			SizeFramesByWindowRect(&new_window_rect);
			dock_prevent_moving = 0;
			LayoutButtons(hwnd, &new_window_rect);
			dock_prevent_moving = 1;
			g_oldPos.x = wp->x;
			g_oldPos.y = wp->y;
			g_oldSize.cx = wp->cx;
			g_oldSize.cy = wp->cy;
			rcWPC = new_window_rect;
		}
		during_sizing = false;
		return 0;

	case WM_SIZE:
		if ((wParam == 0 && lParam == 0) || Docking_IsDocked(0, 0)) {

			if (IsZoomed(hwnd))
				ShowWindow(hwnd, SW_SHOWNORMAL);

			if (pcli->hwndContactList != 0) {
				SendMessage(hwnd, WM_ENTERSIZEMOVE, 0, 0);
				RECT rc;
				GetWindowRect(hwnd, &rc);
				WINDOWPOS wp = {0};
				wp.cx = rc.right - rc.left;
				wp.cy = rc.bottom - rc.top;
				wp.x = rc.left;
				wp.y = rc.top;
				wp.flags = 0;
				SendMessage(hwnd, WM_WINDOWPOSCHANGING, 0, (LPARAM)&wp);
				SendMessage(hwnd, WM_EXITSIZEMOVE, 0, 0);
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

				// if docked, dont remember pos (except for width)
				if (!CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0)) {
					cfg::writeDword("CList", "Height", (DWORD)(rc.bottom - rc.top));
					cfg::writeDword("CList", "x", (DWORD)rc.left);
					cfg::writeDword("CList", "y", (DWORD)rc.top);
				}
				cfg::writeDword("CList", "Width", (DWORD)(rc.right - rc.left));
			}
		}
		return TRUE;

	case WM_SETFOCUS:
		SetFocus(pcli->hwndContactTree);
		return 0;

	case CLUIINTM_REMOVEFROMTASKBAR: {
		BYTE windowStyle = cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_DEFAULT);
		if (windowStyle == SETTING_WINDOWSTYLE_DEFAULT && cfg::getByte("CList", "AlwaysHideOnTB", 0))
			RemoveFromTaskBar(hwnd);
		return 0;
	}
	case WM_ACTIVATE:
		if (g_fading_active) {
			if (wParam != WA_INACTIVE && cfg::dat.isTransparent)
				transparentFocus = 1;
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		if (wParam == WA_INACTIVE) {
			if ((HWND)wParam != hwnd)
				if (cfg::dat.isTransparent)
					if (transparentFocus)
						SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
		}
		else {
			if (cfg::dat.isTransparent) {
				KillTimer(hwnd, TM_AUTOALPHA);
				SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
				transparentFocus = 1;
			}
			SetWindowPos(pcli->hwndContactList, cfg::getByte("CList", "OnTop", SETTING_ONTOP_DEFAULT) ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOSENDCHANGING);
		}
		PostMessage(hwnd, CLUIINTM_REMOVEFROMTASKBAR, 0, 0);
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (cfg::dat.isTransparent) {
			if (!transparentFocus && GetForegroundWindow() != hwnd) {
				SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
				transparentFocus = 1;
				SetTimer(hwnd, TM_AUTOALPHA, 250, NULL);
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NCHITTEST: {
		LRESULT result;
		RECT r;
		POINT pt;
		int k = 0;
		int clip = cfg::dat.bClipBorder;

		GetWindowRect(hwnd, &r);
		GetCursorPos(&pt);
		if (pt.y <= r.bottom && pt.y >= r.bottom - clip - 6 && !cfg::getByte("CLUI", "AutoSize", 0)) {
			if (pt.x > r.left + clip + 10 && pt.x < r.right - clip - 10)
				return HTBOTTOM;
			if (pt.x < r.left + clip + 10)
				return HTBOTTOMLEFT;
			if (pt.x > r.right - clip - 10)
				return HTBOTTOMRIGHT;
		}
		else if (pt.y >= r.top && pt.y <= r.top + 3 && !cfg::getByte("CLUI", "AutoSize", 0)) {
			if (pt.x > r.left + clip + 10 && pt.x < r.right - clip - 10)
				return HTTOP;
			if (pt.x < r.left + clip + 10)
				return HTTOPLEFT;
			if (pt.x > r.right - clip - 10)
				return HTTOPRIGHT;
		}
		else if (pt.x >= r.left && pt.x <= r.left + clip + 6)
			return HTLEFT;
		else if (pt.x >= r.right - clip - 6 && pt.x <= r.right)
			return HTRIGHT;

		result = DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if (result == HTSIZE || result == HTTOP || result == HTTOPLEFT || result == HTTOPRIGHT || result == HTBOTTOM || result == HTBOTTOMRIGHT || result == HTBOTTOMLEFT)
			if (cfg::dat.autosize)
				return HTCLIENT;
		return result;
	}

	case WM_TIMER:
		if ((int)wParam == TM_AUTOALPHA) {
			int inwnd;

			if (GetForegroundWindow() == hwnd) {
				KillTimer(hwnd, TM_AUTOALPHA);
				inwnd = 1;
			}
			else {
				POINT pt;
				HWND hwndPt;
				pt.x = (short)LOWORD(GetMessagePos());
				pt.y = (short)HIWORD(GetMessagePos());
				hwndPt = WindowFromPoint(pt);
				inwnd = (hwndPt == hwnd || GetParent(hwndPt) == hwnd);
			}
			if (inwnd != transparentFocus) {
				//change
				transparentFocus = inwnd;
				if (transparentFocus)
					SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.alpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
				else
					SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), cfg::dat.autoalpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
			}
			if (!transparentFocus)
				KillTimer(hwnd, TM_AUTOALPHA);
		}
		else if (wParam == TIMERID_AUTOSIZE) {
			KillTimer(hwnd, wParam);
			SetWindowPos(hwnd, 0, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
			PostMessage(hwnd, WM_SIZE, 0, 0);
			PostMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
		}
		return TRUE;

	case WM_SHOWWINDOW:
		{
			static int noRecurse = 0;
			DWORD thisTick, startTick;
			int sourceAlpha, destAlpha;

			if (cfg::dat.forceResize && wParam != SW_HIDE) {
				cfg::dat.forceResize = FALSE;
				if (0) { //!g_CluiData.fadeinout && MySetLayeredWindowAttributes && g_CluiData.bLayeredHack) {
					SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), 0, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
					SendMessage(hwnd, WM_SIZE, 0, 0);
					ShowWindow(hwnd, SW_SHOW);
					RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
					SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), 255, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
				}
				else {
					SendMessage(hwnd, WM_SIZE, 0, 0);
					PostMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
				}
			}
			PostMessage(hwnd, CLUIINTM_REMOVEFROMTASKBAR, 0, 0);

			if (lParam)
				return DefWindowProc(hwnd, msg, wParam, lParam);
			if (noRecurse)
				return DefWindowProc(hwnd, msg, wParam, lParam);
			if (!cfg::dat.fadeinout)
				return DefWindowProc(hwnd, msg, wParam, lParam);

			g_fading_active = 1;

			if (wParam) {
				sourceAlpha = 0;
				destAlpha = cfg::dat.isTransparent ? cfg::dat.alpha : 255;
				SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? (COLORREF)cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)sourceAlpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
				noRecurse = 1;
				ShowWindow(hwnd, SW_SHOW);
				RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
				noRecurse = 0;
			}
			else {
				sourceAlpha = cfg::dat.isTransparent ? (transparentFocus ? cfg::dat.alpha : cfg::dat.autoalpha) : 255;
				destAlpha = 0;
			}
			for (startTick = GetTickCount(); ;) {
				thisTick = GetTickCount();
				if (thisTick >= startTick + 200) {
					SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)destAlpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
					g_fading_active = 0;
					return DefWindowProc(hwnd, msg, wParam, lParam);
				}
				SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)(sourceAlpha + (destAlpha - sourceAlpha) * (int)(thisTick - startTick) / 200), LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
			}
			SetLayeredWindowAttributes(hwnd, cfg::dat.bFullTransparent ? cfg::dat.colorkey : RGB(0, 0, 0), (BYTE)destAlpha, LWA_ALPHA | (cfg::dat.bFullTransparent ? LWA_COLORKEY : 0));
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_SYSCOMMAND:
		{
			BYTE bWindowStyle = cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_DEFAULT);
			if (SETTING_WINDOWSTYLE_DEFAULT == bWindowStyle) {
				if (wParam == SC_RESTORE) {
					CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
					SendMessage(hwnd, WM_SIZE, 0, 0);
					SendMessage(hwnd, CLUIINTM_REDRAW, 0, 0);
					SendMessage(hwnd, CLUIINTM_STATUSBARUPDATE, 0, 0);
					cfg::writeByte("CList", "State", SETTING_STATE_NORMAL);
					break;
				}
			}

			if (wParam == SC_MAXIMIZE)
				return 0;

			if (wParam == SC_MINIMIZE) {
				if (SETTING_WINDOWSTYLE_DEFAULT == bWindowStyle && !cfg::getByte("CList", "AlwaysHideOnTB", 0)) {
					cfg::writeByte("CList", "State", SETTING_STATE_MINIMIZED);
					break;
				}
				pcli->pfnShowHide(0, 0);
				return 0;
			}
			if (wParam == SC_RESTORE) {
				pcli->pfnShowHide(0, 0);
				return 0;
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_COMMAND:
		{
			DWORD dwOldFlags = cfg::dat.dwFlags;
			if (HIWORD(wParam) == BN_CLICKED && lParam != 0) {
				if (LOWORD(wParam) == IDC_TBFIRSTUID - 1)
					break;

				else if (LOWORD(wParam) >= IDC_TBFIRSTUID) { // skinnable buttons handling
					ButtonItem *item = g_ButtonItems;
					WPARAM wwParam = 0;
					LPARAM llParam = 0;
					MCONTACT hContact = 0;
					ClcContact *contact = 0;
					int sel = cfg::clcdat ? cfg::clcdat->selection : -1;
					int serviceFailure = FALSE;

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
							}
							else if (item->dwFlags & BUTTON_ISPROTOSERVICE && cfg::clcdat) {
								if (contactOK) {
									char *szProto = GetContactProto(hContact);
									if (ProtoServiceExists(szProto, item->szService))
										ProtoCallService(szProto, item->szService, wwParam, llParam);
									else
										serviceFailure = TRUE;
								}
							}
							else if (item->dwFlags & BUTTON_ISDBACTION) {
								BYTE *pValue;
								char *szModule = item->szModule;
								char *szSetting = item->szSetting;
								MCONTACT finalhContact = 0;

								if (item->dwFlags & BUTTON_ISCONTACTDBACTION || item->dwFlags & BUTTON_DBACTIONONCONTACT) {
									contactOK = ServiceParamsOK(item, &wwParam, &llParam, hContact);
									if (contactOK && item->dwFlags & BUTTON_ISCONTACTDBACTION)
										szModule = GetContactProto(hContact);
									finalhContact = hContact;
								}
								else
									contactOK = 1;

								if (contactOK) {
									BOOL fDelete = FALSE;

									if (item->dwFlags & BUTTON_ISTOGGLE) {
										BOOL fChecked = (SendMessage(item->hWnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED);

										pValue = fChecked ? item->bValueRelease : item->bValuePush;
										if (fChecked && pValue[0] == 0)
											fDelete = TRUE;
									}
									else
										pValue = item->bValuePush;

									if (fDelete)
										db_unset(finalhContact, szModule, szSetting);
									else {
										switch (item->type) {
										case DBVT_BYTE:
											cfg::writeByte(finalhContact, szModule, szSetting, pValue[0]);
											break;
										case DBVT_WORD:
											cfg::writeWord(finalhContact, szModule, szSetting, *((WORD *)&pValue[0]));
											break;
										case DBVT_DWORD:
											cfg::writeDword(finalhContact, szModule, szSetting, *((DWORD *)&pValue[0]));
											break;
										case DBVT_ASCIIZ:
											cfg::writeString(finalhContact, szModule, szSetting, (char *)pValue);
											break;
										}
									}
								}
								else if (item->dwFlags & BUTTON_ISTOGGLE)
									SendMessage(item->hWnd, BM_SETCHECK, 0, 0);
							}
							if (!contactOK)
								MessageBox(0, TranslateT("The requested action requires a valid contact selection. Please select a contact from the contact list and repeat."), TranslateT("Parameter mismatch"), MB_OK);
							if (serviceFailure) {
								char szError[512];

								mir_snprintf(szError, SIZEOF(szError), Translate("The service %s specified by the %s button definition was not found. You may need to install additional plugins."), item->szService, item->szName);
								MessageBoxA(0, szError, Translate("Service failure"), MB_OK);
							}
							break;
						}
						item = item->nextItem;
					}
					goto buttons_done;
				}

				switch (LOWORD(wParam)) {
				case IDC_TBMENU:
				case IDC_TBTOPMENU:
				case IDC_STBTOPMENU:
					{
						RECT rc;
						HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
						GetButtonRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
						TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, LOWORD(wParam) == IDC_TBMENU ? rc.top : rc.bottom, 0, hwnd, NULL);
					}
					return 0;

				case IDC_TBTOPSTATUS:
				case IDC_STBTOPSTATUS:
				case IDC_TBGLOBALSTATUS:
					{
						RECT rc;
						HMENU hmenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
						GetButtonRect(GetDlgItem(hwnd, LOWORD(wParam)), &rc);
						TrackPopupMenu(hmenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, rc.left, LOWORD(wParam) == IDC_TBGLOBALSTATUS ? rc.top : rc.bottom, 0, hwnd, NULL);
					}
					return 0;


				case IDC_TBSOUND:
				case IDC_STBSOUND:
					cfg::dat.soundsOff = !cfg::dat.soundsOff;
					cfg::writeByte("CLUI", "NoSounds", (BYTE)cfg::dat.soundsOff);
					cfg::writeByte("Skin", "UseSound", (BYTE)(cfg::dat.soundsOff ? 0 : 1));
					return 0;

				case IDC_TBSELECTVIEWMODE:
				case IDC_STBSELECTVIEWMODE:
					SendMessage(g_hwndViewModeFrame, WM_COMMAND, IDC_SELECTMODE, lParam);
					break;
				case IDC_TBCLEARVIEWMODE:
				case IDC_STBCLEARVIEWMODE:
					SendMessage(g_hwndViewModeFrame, WM_COMMAND, IDC_RESETMODES, lParam);
					break;
				case IDC_TBCONFIGUREVIEWMODE:
				case IDC_STBCONFIGUREVIEWMODE:
					SendMessage(g_hwndViewModeFrame, WM_COMMAND, IDC_CONFIGUREMODES, lParam);
					break;
				case IDC_TBFINDANDADD:
				case IDC_STBFINDANDADD:
					CallService(MS_FINDADD_FINDADD, 0, 0);
					return 0;
				case IDC_TBACCOUNTS:
				case IDC_STBACCOUNTS:
					CallService(MS_PROTO_SHOWACCMGR, 0, 0);
					break;
				case IDC_TBOPTIONS:
				case IDC_STBOPTIONS:
					CallService("Options/OptionsCommand", 0, 0);
					return 0;
				}
			}
			else if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_MAINMENU), (LPARAM)(HANDLE)NULL))
				return 0;

buttons_done:
			switch (LOWORD(wParam)) {
			case ID_TRAY_EXIT:
			case ID_ICQ_EXIT:
				cfg::shutDown = 1;
				if (CallService(MS_SYSTEM_OKTOEXIT, 0, 0))
					DestroyWindow(hwnd);
				break;
			case ID_TRAY_HIDE:
			case IDC_TBMINIMIZE:
			case IDC_STBMINIMIZE:
				pcli->pfnShowHide(0, 0);
				break;
			case POPUP_NEWGROUP:
				SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
				SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, 1, 0);
				Clist_CreateGroup(NULL, NULL);
				break;
			case POPUP_HIDEOFFLINE:
			case IDC_TBHIDEOFFLINE:
			case IDC_STBHIDEOFFLINE:
				CallService(MS_CLIST_SETHIDEOFFLINE, (WPARAM)(-1), 0);
				break;
			case POPUP_HIDEOFFLINEROOT:
				SendMessage(pcli->hwndContactTree, CLM_SETHIDEOFFLINEROOT, !SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0), 0);
				break;
			case POPUP_HIDEEMPTYGROUPS:
				{
					int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
					cfg::writeByte("CList", "HideEmptyGroups", (BYTE)newVal);
					SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
				}
				break;
			case IDC_TBHIDEGROUPS:
			case IDC_STBHIDEGROUPS:
			case POPUP_DISABLEGROUPS:
				{
					int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
					cfg::writeByte("CList", "UseGroups", (BYTE)newVal);
					SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
					CheckDlgButton(hwnd, IDC_TBHIDEGROUPS, newVal ? BST_CHECKED : BST_UNCHECKED);
				}
				break;
			case POPUP_HIDEMIRANDA:
				pcli->pfnShowHide(0, 0);
				break;
			case POPUP_SHOWMETAICONS:
				cfg::dat.dwFlags ^= CLUI_USEMETAICONS;
				pcli->pfnInitAutoRebuild(hwnd);
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
			}
			if (dwOldFlags != cfg::dat.dwFlags) {
				InvalidateRect(pcli->hwndContactTree, NULL, FALSE);
				cfg::writeDword("CLUI", "Frameflags", cfg::dat.dwFlags);
				if ((dwOldFlags & (CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_CLISTSUNKEN)) != (cfg::dat.dwFlags & (CLUI_FRAME_SHOWBOTTOMBUTTONS | CLUI_FRAME_CLISTSUNKEN))) {
					ConfigureFrame();
					ConfigureCLUIGeometry(1);
				}
				ConfigureEventArea(pcli->hwndContactList);
				PostMessage(pcli->hwndContactList, WM_SIZE, 0, 0);
				PostMessage(pcli->hwndContactList, CLUIINTM_REDRAW, 0, 0);
			}
		}
		return FALSE;

	case WM_LBUTTONDOWN:
		if (g_ButtonItems) {
			POINT ptMouse, pt;

			GetCursorPos(&ptMouse);
			pt = ptMouse;
			return SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, MAKELPARAM(pt.x, pt.y));
		}
		break;

	case WM_DISPLAYCHANGE:
		SendMessage(pcli->hwndContactTree, WM_SIZE, 0, 0); //forces it to send a cln_listsizechanged
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->hwndFrom == pcli->hwndContactTree) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_LISTSIZECHANGE:
				sttProcessResize(hwnd, (NMCLISTCONTROL *)lParam);
				return FALSE;

			case NM_CLICK:
				{
					NMCLISTCONTROL *nm = (NMCLISTCONTROL *)lParam;
					DWORD hitFlags;
					HANDLE hItem;

					hItem = (HANDLE)SendMessage(pcli->hwndContactTree, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x, nm->pt.y));

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

	case WM_CONTEXTMENU:
		{
			RECT rc;
			POINT pt;

			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
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
				hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDGROUP, 0, 0);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				DestroyTrayMenu(hMenu);
				return 0;
			}
			GetWindowRect(pcli->hwndStatus, &rc);
			if (PtInRect(&rc, pt)) {
				HMENU hMenu;
				if (cfg::getByte("CLUI", "SBarRightClk", 0))
					hMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
				else
					hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				return 0;
			}
		}
		break;

	case WM_MEASUREITEM:
		if (((LPMEASUREITEMSTRUCT)lParam)->itemData == MENU_MIRANDAMENU) {
			((LPMEASUREITEMSTRUCT)lParam)->itemWidth = g_cxsmIcon * 4 / 3;
			((LPMEASUREITEMSTRUCT)lParam)->itemHeight = 0;
			return TRUE;
		}
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

			if (hbmLockedPoint == 0) {
				RECT rc = {0, 0, 5, 5};

				hdcLockedPoint = CreateCompatibleDC(dis->hDC);
				hbmLockedPoint = CreateCompatibleBitmap(dis->hDC, 5, 5);
				hbmOldLockedPoint = reinterpret_cast<HBITMAP>(SelectObject(hdcLockedPoint, hbmLockedPoint));
			}
			if (dis->hwndItem == pcli->hwndStatus) {
				ProtocolData *pd = (ProtocolData *)dis->itemData;
				if (IsBadCodePtr((FARPROC)pd))
					return TRUE;
				if (cfg::shutDown)
					return TRUE;

				int nParts = SendMessage(pcli->hwndStatus, SB_GETPARTS, 0, 0);
				SIZE textSize;
				BYTE showOpts = cfg::getByte("CLUI", "SBarShow", 1);
				char *szProto = pd->RealName;
				int status = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
				SetBkMode(dis->hDC, TRANSPARENT);
				int x = dis->rcItem.left;

				if (showOpts & 1) {
					HICON hIcon;

					if (status >= ID_STATUS_CONNECTING && status < ID_STATUS_OFFLINE) {
						char szBuffer[128];
						mir_snprintf(szBuffer, SIZEOF(szBuffer), "%s_conn", pd->RealName);
						hIcon = Skin_GetIcon(szBuffer);
					}
					else if (cfg::dat.bShowXStatusOnSbar && status > ID_STATUS_OFFLINE) {
						int xStatus;
						CUSTOM_STATUS cst = { sizeof(cst) };
						cst.flags = CSSF_MASK_STATUS;
						cst.status = &xStatus;
						if (ProtoServiceExists(pd->RealName, PS_GETCUSTOMSTATUSEX) && !ProtoCallService(pd->RealName, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cst) && xStatus > 0)
							hIcon = (HICON)CallProtoService(pd->RealName, PS_GETCUSTOMSTATUSICON, 0, LR_SHARED); // get OWN xStatus icon (if set)
						else
							hIcon = LoadSkinnedProtoIcon(szProto, status);
					}
					else
						hIcon = LoadSkinnedProtoIcon(szProto, status);

					if (!(showOpts & 6) && cfg::dat.bEqualSections)
						x = (dis->rcItem.left + dis->rcItem.right - 16) >> 1;
					if (pd->protopos == 0)
						x += (cfg::dat.bEqualSections ? (cfg::dat.bCLeft / 2) : cfg::dat.bCLeft);
					else if (pd->protopos == nParts - 1)
						x -= (cfg::dat.bCRight / 2);
					DrawIconEx(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - 16) >> 1, hIcon, 16, 16, 0, NULL, DI_NORMAL);
					Skin_ReleaseIcon(hIcon);

					if (cfg::getByte("CLUI", "sbar_showlocked", 1)) {
						if (cfg::getByte(szProto, "LockMainStatus", 0)) {
							hIcon = LoadSkinnedIcon(SKINICON_OTHER_STATUS_LOCKED);
							if (hIcon != NULL) {
								DrawIconEx(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - 16) >> 1, hIcon, 16, 16, 0, NULL, DI_NORMAL);
								Skin_ReleaseIcon(hIcon);
							}
						}
					}
					x += 18;
				}
				else {
					x += 2;
					if (pd->protopos == 0)
						x += (cfg::dat.bEqualSections ? (cfg::dat.bCLeft / 2) : cfg::dat.bCLeft);
					else if (pd->protopos == nParts - 1)
						x -= (cfg::dat.bCRight / 2);
				}

				if (showOpts & 2) {
					TCHAR szName[64];
					PROTOACCOUNT *pa = ProtoGetAccount(szProto);
					if (pa) {
						lstrcpyn(szName, pa->tszAccountName, SIZEOF(szName));
						szName[SIZEOF(szName) - 1] = 0;
					}
					else
						szName[0] = 0;

					if (lstrlen(szName) < sizeof(szName) - 1)
						lstrcat(szName, _T(" "));
					GetTextExtentPoint32(dis->hDC, szName, lstrlen(szName), &textSize);
					TextOut(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, szName, lstrlen(szName));
					x += textSize.cx;
				}
				if (showOpts & 4) {
					TCHAR *szStatus = pcli->pfnGetStatusModeDescription(status, 0);
					GetTextExtentPoint32(dis->hDC, szStatus, lstrlen(szStatus), &textSize);
					TextOut(dis->hDC, x, (dis->rcItem.top + dis->rcItem.bottom - textSize.cy) >> 1, szStatus, lstrlen(szStatus));
				}
			}
			else if (dis->CtlType == ODT_MENU) {
				if (dis->itemData == MENU_MIRANDAMENU)
					break;
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			}
		}
		return 0;

	case WM_CLOSE:
		if (SETTING_WINDOWSTYLE_DEFAULT == cfg::getByte("CLUI", "WindowStyle", SETTING_WINDOWSTYLE_DEFAULT) && !cfg::getByte("CList", "AlwaysHideOnTB", 0)) {
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
			return 0;
		}
		pcli->pfnShowHide(0, 0);
		return 0;

	case CLUIINTM_REDRAW:
		if (show_on_first_autosize) {
			show_on_first_autosize = FALSE;
			ShowCLUI(hwnd);
		}
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		return 0;

	case CLUIINTM_STATUSBARUPDATE:
		CluiProtocolStatusChanged(0, 0);
		return 0;

	case WM_THEMECHANGED:
		API::updateState();
		break;

	case WM_DESTROY:
		if (cfg::dat.hdcBg) {
			SelectObject(cfg::dat.hdcBg, cfg::dat.hbmBgOld);
			DeleteObject(cfg::dat.hbmBg);
			DeleteDC(cfg::dat.hdcBg);
			cfg::dat.hdcBg = NULL;
		}
		if (cfg::dat.bmpBackground) {
			SelectObject(cfg::dat.hdcPic, cfg::dat.hbmPicOld);
			DeleteDC(cfg::dat.hdcPic);
			DeleteObject(cfg::dat.bmpBackground);
			cfg::dat.bmpBackground = NULL;
		}
		FreeProtocolData();
		if (hdcLockedPoint) {
			SelectObject(hdcLockedPoint, hbmOldLockedPoint);
			DeleteObject(hbmLockedPoint);
			DeleteDC(hdcLockedPoint);
		}
		// if this has not yet been set, do it now.
		// indicates that clist is shutting down and prevents various things
		// from happening at shutdown.
		if (!cfg::shutDown)
			cfg::shutDown = 1;
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, (WPARAM)hFrameContactTree, 0);
		break;
	}

	return saveContactListWndProc(hwnd, msg, wParam, lParam);
}

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW 0x00020000
#endif

static int MetaChanged(WPARAM wParam, LPARAM lParam)
{
	pcli->pfnClcBroadcast(INTM_METACHANGEDEVENT, wParam, lParam);
	return 0;
}

static BOOL g_AboutDlgActive = 0;

INT_PTR CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	COLORREF url_visited = RGB(128, 0, 128);
	COLORREF url_unvisited = RGB(0, 0, 255);

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
			mir_snprintf(str, sizeof(str), "%s %d.%d.%d.%d", Translate("Version"), HIBYTE(HIWORD(v)), LOBYTE(HIWORD(v)), HIBYTE(LOWORD(v)), LOBYTE(LOWORD(v)));
			SetDlgItemTextA(hwndDlg, IDC_VERSION, str);
		}
		{
			HICON hIcon = LoadIcon(GetModuleHandleA("miranda32.exe"), MAKEINTRESOURCE(102));
			SendDlgItemMessage(hwndDlg, IDC_LOGO, STM_SETICON, (WPARAM)hIcon, 0);
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			DestroyIcon(hIcon);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_SUPPORT:
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://miranda-ng.org/");
			break;
		}
		break;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_WHITERECT)
			|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_CLNICER)
			|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_VERSION)
			|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_COPYRIGHT)
			|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_SUPPORT)
			|| (HWND)lParam == GetDlgItem(hwndDlg, IDC_LOGO))
		{
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

	case WM_DESTROY:
		{
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

	hMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
	return 0;
}

static INT_PTR CLN_ShowStatusMenu(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	POINT pt;

	hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
	return 0;
}

#define MS_CLUI_SHOWMAINMENU	"CList/ShowMainMenu"
#define MS_CLUI_SHOWSTATUSMENU	"CList/ShowStatusMenu"

void LoadCLUIModule(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, CluiModulesLoaded);

	WNDCLASS wndclass;
	wndclass.style = 0;
	wndclass.lpfnWndProc = EventAreaWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_hInst;
	wndclass.hIcon = 0;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)COLOR_3DFACE;
	wndclass.lpszMenuName = 0;
	wndclass.lpszClassName = _T("EventAreaClass");
	RegisterClass(&wndclass);

	oldhideoffline = cfg::getByte("CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	cluiPos.left = cfg::getDword("CList", "x", 600);
	cluiPos.top = cfg::getDword("CList", "y", 200);
	cluiPos.right = cfg::getDword("CList", "Width", 150);
	cluiPos.bottom = cfg::getDword("CList", "Height", 350);

	LoadExtraIconModule();
	LoadCLUIFramesModule();

	CreateServiceFunction("CLN/About", CLN_ShowAbout);
	CreateServiceFunction(MS_CLUI_SHOWMAINMENU, CLN_ShowMainMenu);
	CreateServiceFunction(MS_CLUI_SHOWSTATUSMENU, CLN_ShowStatusMenu);

	if (cfg::getByte("CLUI", "FloaterMode", 0)) {
		MessageBox(NULL,
			TranslateT("You need the FloatingContacts plugin, cause the embedded floating contacts were removed."),
			TranslateT("Warning"), MB_OK | MB_ICONWARNING);
		db_unset(NULL, "CLUI", "FloaterMode");
	}
}

void OnCreateClc()
{
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaChanged);
	HookEvent(ME_MC_SUBCONTACTSCHANGED, MetaChanged);

	InitGroupMenus();
	LoadExtBkSettingsFromDB();
	PreCreateCLC(pcli->hwndContactList);
}

struct
{
	const TCHAR *tszName;
	int iMask;
}
static clistFontDescr[] = {
	{ LPGENT("Standard contacts"),											FIDF_CLASSGENERAL },
	{ LPGENT("Online contacts to whom you have a different visibility"),	FIDF_CLASSGENERAL },
	{ LPGENT("Offline contacts"),											FIDF_CLASSGENERAL },
	{ LPGENT("Contacts which are 'not on list'"),							FIDF_CLASSGENERAL },
	{ LPGENT("Groups"),														FIDF_CLASSHEADER  },
	{ LPGENT("Group member counts"),										FIDF_CLASSHEADER  },
	{ LPGENT("Dividers"),													FIDF_CLASSSMALL   },
	{ LPGENT("Offline contacts to whom you have a different visibility"),	FIDF_CLASSGENERAL },
	{ LPGENT("Status mode"),												FIDF_CLASSGENERAL },
	{ LPGENT("Frame titles"),												FIDF_CLASSGENERAL },
	{ LPGENT("Event area"),													FIDF_CLASSGENERAL },
	{ LPGENT("Contact list local time"),									FIDF_CLASSGENERAL }
};

void FS_RegisterFonts()
{
	FontIDT fid = { sizeof(fid) };
	_tcsncpy(fid.group, LPGENT("Contact list"), SIZEOF(fid.group));
	strncpy(fid.dbSettingsGroup, "CLC", 5);
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS | FIDF_APPENDNAME | FIDF_SAVEPOINTSIZE;

	HDC hdc = GetDC(NULL);
	for (int i = 0; i < SIZEOF(clistFontDescr); i++) {
		LOGFONT lf;
		pcli->pfnGetFontSetting(i, &lf, &fid.deffontsettings.colour);
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

		_tcsncpy_s(fid.deffontsettings.szFace, SIZEOF(fid.deffontsettings.szFace), lf.lfFaceName, _TRUNCATE);
		fid.deffontsettings.charset = lf.lfCharSet;
		fid.deffontsettings.size = (char)lf.lfHeight;
		fid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0);

		fid.flags &= ~FIDF_CLASSMASK;
		fid.flags |= clistFontDescr[i].iMask;

		_tcsncpy(fid.name, clistFontDescr[i].tszName, SIZEOF(fid.name));
		
		char idstr[10];
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", i);
		strncpy(fid.prefix, idstr, SIZEOF(fid.prefix));
		fid.order = i;
		FontRegisterT(&fid);
	}
	ReleaseDC(NULL, hdc);

	// and colours
	ColourIDT colourid = {0};
	colourid.cbSize = sizeof(colourid);
	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "CLC", sizeof(colourid.dbSettingsGroup));

	strncpy(colourid.setting, "BkColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Background"), SIZEOF(colourid.name));
	_tcsncpy(colourid.group, LPGENT("Contact list"), SIZEOF(colourid.group));
	colourid.defcolour = CLCDEFAULT_BKCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "SelTextColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Selected text"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_SELTEXTCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "HotTextColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Hottrack text"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_HOTTEXTCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "QuickSearchColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Quicksearch text"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_QUICKSEARCHCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.dbSettingsGroup, "CLUI", sizeof(colourid.dbSettingsGroup));
	strncpy(colourid.setting, "clr_frameborder", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Embedded frames border"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = RGB(40, 40, 40);
	ColourRegisterT(&colourid);
}
