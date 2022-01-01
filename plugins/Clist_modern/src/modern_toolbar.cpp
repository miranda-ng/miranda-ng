/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
Copyright 2007 Artem Shpynov

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
#include "m_skinbutton.h"
#include <m_toptoolbar.h>
#include "modern_sync.h"

struct
{
	char *pszButtonID, *pszButtonName, *pszServiceName;
	char *pszTooltipUp, *pszTooltipDn;
	int icoDefIdx, defResource, defResource2;
	BOOL bVisByDefault;
	HANDLE hButton;
}
static BTNS[] =
{
	{ "MainMenu", LPGEN("Main menu"), "CList/ShowMainMenu", LPGEN("Main menu"), nullptr, 100, IDI_RESETVIEW, IDI_RESETVIEW, TRUE },
	{ "StatusMenu", LPGEN("Status menu"), "CList/ShowStatusMenu", LPGEN("Status menu"), nullptr, 105, IDI_RESETVIEW, IDI_RESETVIEW, TRUE },
	{ "AccoMgr", LPGEN("Accounts"), MS_PROTO_SHOWACCMGR, LPGEN("Accounts..."), nullptr, 282, IDI_ACCMGR, IDI_ACCMGR, TRUE },
	{ "ShowHideOffline", LPGEN("Show/Hide offline contacts"), MS_CLIST_TOGGLEHIDEOFFLINE, LPGEN("Hide offline contacts"), LPGEN("Show offline contacts"), 110, IDI_RESETVIEW, IDI_RESETVIEW, TRUE },
	{ "FindUser", LPGEN("Find user"), "FindAdd/FindAddCommand", LPGEN("Find user"), nullptr, 140, IDI_RESETVIEW, IDI_RESETVIEW, TRUE },
	{ "Options", "Options", "Options/OptionsCommand", "Options", nullptr, 150, IDI_RESETVIEW, IDI_RESETVIEW, TRUE },
	{ "UseGroups", LPGEN("Use/Disable groups"), MS_CLIST_TOGGLEGROUPS, LPGEN("Use groups"), LPGEN("Disable groups"), 160, IDI_RESETVIEW, IDI_RESETVIEW, FALSE },
	{ "EnableSounds", LPGEN("Enable/Disable sounds"), MS_CLIST_TOGGLESOUNDS, LPGEN("Enable sounds"), LPGEN("Disable sounds"), 170, IDI_RESETVIEW, IDI_RESETVIEW, FALSE },
	{ "Minimize", LPGEN("Minimize"), "CList/ShowHide", LPGEN("Minimize"), nullptr, 180, IDI_RESETVIEW, IDI_RESETVIEW, FALSE }
};

static void SetButtonPressed(int i, int state)
{
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)BTNS[i].hButton, state ? TTBST_PUSHED : 0);
}

void Modern_InitButtons()
{
	for (int i = 0; i < _countof(BTNS); i++) {
		TTBButton tbb = {};

		if (BTNS[i].pszButtonID) {
			tbb.name = LPGEN(BTNS[i].pszButtonName);
			tbb.pszService = BTNS[i].pszServiceName;
			tbb.pszTooltipUp = LPGEN(BTNS[i].pszTooltipUp);
			tbb.pszTooltipDn = LPGEN(BTNS[i].pszTooltipDn);

			char buf[255];
			if (i != 0) {
				mir_snprintf(buf, "%s%s%s", TTB_OPTDIR, BTNS[i].pszButtonID, "_dn");
				tbb.hIconHandleUp = RegisterIcolibIconHandle(buf, "Toolbar", BTNS[i].pszTooltipUp, L"icons\\toolbar_icons.dll", BTNS[i].icoDefIdx, g_plugin.getInst(), BTNS[i].defResource);
			}
			else tbb.hIconHandleUp = RegisterIcolibIconHandle(buf, "Toolbar", BTNS[i].pszTooltipUp, nullptr, 0, nullptr, SKINICON_OTHER_MAINMENU);

			if (BTNS[i].pszTooltipDn) {
				mir_snprintf(buf, "%s%s%s", TTB_OPTDIR, BTNS[i].pszButtonID, "_up");
				tbb.hIconHandleDn = RegisterIcolibIconHandle(buf, "Toolbar", BTNS[i].pszTooltipDn, L"icons\\toolbar_icons.dll", BTNS[i].icoDefIdx + 1, g_plugin.getInst(), BTNS[i].defResource2);
			}
			else tbb.hIconHandleDn = nullptr;
		}
		else tbb.dwFlags |= TTBBF_ISSEPARATOR;

		tbb.dwFlags |= (BTNS[i].bVisByDefault ? TTBBF_VISIBLE : 0);
		BTNS[i].hButton = g_plugin.addTTB(&tbb);
	}

	SetButtonPressed(3, Clist::HideOffline);
	SetButtonPressed(6, Clist::UseGroups);
	SetButtonPressed(7, db_get_b(0, "Skin", "UseSound", SETTING_ENABLESOUNDS_DEFAULT));
}

///////////////////////////////////////////////////////////////////////////////

#define MTBM_LAYEREDPAINT (WM_USER+20)

struct ModernToolbarCtrl : public TTBCtrl
{
	XPTHANDLE mtbXPTheme;
};

struct
{
	HBITMAP  mtb_hBmpBackground;
	COLORREF mtb_bkColour;
	uint16_t     mtb_backgroundBmpUse;
	BOOL     mtb_useWinColors;
}
static tbdat = { nullptr, CLCDEFAULT_BKCOLOUR, CLCDEFAULT_BKBMPUSE, CLCDEFAULT_USEWINDOWSCOLOURS };

static int ehhToolBarSettingsChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact != 0)
		return 0;

	if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "HideOffline"))
			SetButtonPressed(3, cws->value.bVal);
		else if (!strcmp(cws->szSetting, "UseGroups"))
			SetButtonPressed(6, cws->value.bVal);
	}
	else if (!strcmp(cws->szModule, "Skin")) {
		if (!strcmp(cws->szSetting, "UseSound"))
			SetButtonPressed(7, cws->value.bVal);
	}

	return 0;
}

static int ehhToolBarBackgroundSettingsChanged(WPARAM, LPARAM)
{
	if (tbdat.mtb_hBmpBackground) {
		DeleteObject(tbdat.mtb_hBmpBackground);
		tbdat.mtb_hBmpBackground = nullptr;
	}

	if (g_CluiData.fDisableSkinEngine) {
		tbdat.mtb_bkColour = cliGetColor("ToolBar", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (db_get_b(0, "ToolBar", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmapName(db_get_wsa(0, "ToolBar", "BkBitmap"));
			if (tszBitmapName)
				tbdat.mtb_hBmpBackground = Bitmap_Load(tszBitmapName);
		}
		tbdat.mtb_useWinColors = db_get_b(0, "ToolBar", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
		tbdat.mtb_backgroundBmpUse = db_get_b(0, "ToolBar", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}
	PostMessage(g_clistApi.hwndContactList, WM_SIZE, 0, 0);
	return 0;
}

static BOOL sttDrawToolBarBackground(HWND hwnd, HDC hdc, RECT *rect, ModernToolbarCtrl* pMTBInfo)
{
	BOOL bFloat = (GetParent(hwnd) != g_clistApi.hwndContactList);
	if (g_CluiData.fDisableSkinEngine || !g_CluiData.fLayered || bFloat) {
		HBRUSH hbr;

		RECT rc;
		if (rect)
			rc = *rect;
		else
			GetClientRect(hwnd, &rc);

		if (!(tbdat.mtb_backgroundBmpUse && tbdat.mtb_hBmpBackground) && tbdat.mtb_useWinColors) {
			HRESULT hr = S_FALSE;
			if (xpt_IsThemed(pMTBInfo->mtbXPTheme))
				hr = xpt_DrawTheme(pMTBInfo->mtbXPTheme, pMTBInfo->hWnd, hdc, 0, 0, &rc, &rc);

			if (hr == S_FALSE) {
				hbr = GetSysColorBrush(COLOR_3DFACE);
				FillRect(hdc, &rc, hbr);
			}
		}
		else if (!tbdat.mtb_hBmpBackground && !tbdat.mtb_useWinColors) {
			hbr = CreateSolidBrush(tbdat.mtb_bkColour);
			FillRect(hdc, &rc, hbr);
			DeleteObject(hbr);
		}
		else DrawBackGround(hwnd, hdc, tbdat.mtb_hBmpBackground, tbdat.mtb_bkColour, tbdat.mtb_backgroundBmpUse);
	}
	return TRUE;
}

static void sttDrawNonLayeredSkinedBar(HWND hwnd, HDC hdc)
{
	RECT rc = { 0 };
	GetClientRect(hwnd, &rc);
	rc.right++;
	rc.bottom++;
	HDC hdc2 = CreateCompatibleDC(hdc);
	HBITMAP hbmp = ske_CreateDIB32(rc.right, rc.bottom);
	HBITMAP hbmpo = (HBITMAP)SelectObject(hdc2, hbmp);
	if (GetParent(hwnd) != g_clistApi.hwndContactList) {
		HBRUSH br = GetSysColorBrush(COLOR_3DFACE);
		FillRect(hdc2, &rc, br);
	}
	else ske_BltBackImage(hwnd, hdc2, &rc);

	SendMessage(hwnd, MTBM_LAYEREDPAINT, (WPARAM)hdc2, 0);
	BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc2, rc.left, rc.top, SRCCOPY);
	SelectObject(hdc2, hbmpo);
	DeleteObject(hbmp);
	DeleteDC(hdc2);

	SelectObject(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
	ValidateRect(hwnd, nullptr);
}

static LRESULT CALLBACK toolbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ModernToolbarCtrl* pMTBInfo = (ModernToolbarCtrl*)GetWindowLongPtr(hwnd, 0);

	switch (msg) {
	case WM_ERASEBKGND:
		return (g_CluiData.fDisableSkinEngine) ? sttDrawToolBarBackground(hwnd, (HDC)wParam, nullptr, pMTBInfo) : 0;

	case WM_NCPAINT:
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BOOL bFloat = (GetParent(hwnd) != g_clistApi.hwndContactList);
			if (g_CluiData.fDisableSkinEngine || !g_CluiData.fLayered || bFloat) {
				BeginPaint(hwnd, &ps);
				if ((!g_CluiData.fLayered || bFloat) && !g_CluiData.fDisableSkinEngine)
					sttDrawNonLayeredSkinedBar(hwnd, ps.hdc);
				else
					sttDrawToolBarBackground(hwnd, ps.hdc, &ps.rcPaint, pMTBInfo);
				EndPaint(hwnd, &ps);
			}
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == BUTTONNEEDREDRAW)
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case MTBM_LAYEREDPAINT:
		{
			RECT MyRect = { 0 };
			HDC hDC = (HDC)wParam;
			GetWindowRect(hwnd, &MyRect);

			RECT rcClient;
			GetClientRect(hwnd, &rcClient);
			SkinDrawGlyph(hDC, &rcClient, &rcClient, "Bar,ID=ToolBar,Part=Background");

			for (int i = 0; i < pMTBInfo->pButtonList->realCount; i++) {
				RECT childRect;
				POINT Offset;
				TTBCtrlButton* mtbi = (TTBCtrlButton*)pMTBInfo->pButtonList->items[i];
				GetWindowRect(mtbi->hWindow, &childRect);
				Offset.x = childRect.left - MyRect.left;
				Offset.y = childRect.top - MyRect.top;
				SendMessage(mtbi->hWindow, BUTTONDRAWINPARENT, (WPARAM)hDC, (LPARAM)&Offset);
			}
		}
		return 0;

	case WM_DESTROY:
		xpt_FreeThemeForWindow(hwnd);
		CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)hwnd, 0);
		break;
	}

	return mir_callNextSubclass(hwnd, toolbarWndProc, msg, wParam, lParam);
}

static int ToolBar_LayeredPaintProc(HWND hWnd, HDC hDC, RECT *, HRGN, uint32_t, void *)
{
	return SendMessage(hWnd, MTBM_LAYEREDPAINT, (WPARAM)hDC, 0);
}

void CustomizeToolbar(HWND hwnd)
{
	mir_subclassWindow(hwnd, toolbarWndProc);
	SendMessage(hwnd, TTB_SETCUSTOMDATASIZE, 0, sizeof(ModernToolbarCtrl));

	ModernToolbarCtrl* pMTBInfo = (ModernToolbarCtrl*)GetWindowLongPtr(hwnd, 0);

	CLISTFrame Frame = { sizeof(Frame) };
	Frame.szName.a = "Toolbar";
	Frame.hWnd = hwnd;
	Frame.align = alTop;
	Frame.Flags = F_VISIBLE | F_NOBORDER | F_LOCKED | F_NO_SUBCONTAINER;
	Frame.height = 18;
	Frame.hIcon = Skin_LoadIcon(SKINICON_OTHER_FRAME);
	pMTBInfo->hFrame = g_plugin.addFrame(&Frame);

	CallService(MS_SKINENG_REGISTERPAINTSUB, (WPARAM)hwnd, (LPARAM)ToolBar_LayeredPaintProc);

	pMTBInfo->mtbXPTheme = xpt_AddThemeHandle(hwnd, L"TOOLBAR");
	pMTBInfo->bHardUpdate = TRUE;

	Modern_InitButtons();
}

#define TTB_OPTDIR "TopToolBar"

static char szUrl[] = "https://miranda-ng.org/p/TopToolBar";

static wchar_t szWarning[] = LPGENW("To view a toolbar in Clist_modern you need the TopToolBar plugin. Click Yes to download it or No to continue");

static void CopySettings(const char* to, const char* from, int defValue)
{
	db_set_b(0, TTB_OPTDIR, to, db_get_b(0, "ModernToolBar", from, defValue));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CustomizeButton(HANDLE ttbid, HWND hWnd, LPARAM lParam);

static int Toolbar_ModuleReloaded(WPARAM wParam, LPARAM)
{
	auto &pInfo = HPLUGIN(wParam)->getInfo();
	if (!mir_strcmpi(pInfo.shortName, "TopToolBar"))
		TopToolbar_SetCustomProc(CustomizeButton, 0);

	return 0;
}

static int Toolbar_ModulesLoaded(WPARAM, LPARAM)
{
	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("Toolbar background")"/ToolBar"), 0);

	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ehhToolBarSettingsChanged);

	TopToolbar_SetCustomProc(CustomizeButton, 0);

	uint8_t bOldSetting = 0;
	if (!db_get_b(0, "Compatibility", "TTB_Upgrade", 0)) {
		if (bOldSetting = db_get_b(0, "CLUI", "ShowButtonBar", 1)) {
			CopySettings("BUTTWIDTH", "option_Bar0_BtnWidth", 20);
			CopySettings("BUTTHEIGHT", "option_Bar0_BtnHeight", 20);
			CopySettings("BUTTGAP", "option_Bar0_BtnSpace", 1);
			CopySettings("BUTTAUTOSIZE", "option_Bar0_Autosize", 1);
			CopySettings("BUTTMULTI", "option_Bar0_Multiline", 1);

			db_unset(0, "CLUI", "ShowButtonBar");

			db_delete_module(0, "ModernToolBar");
		}
		db_set_b(0, "Compatibility", "TTB_Upgrade", 1);
	}

	if (!ServiceExists(MS_TTB_REMOVEBUTTON) && bOldSetting == 1)
		if (IDYES == MessageBox(nullptr, TranslateW(szWarning), TranslateT("Toolbar upgrade"), MB_ICONQUESTION | MB_YESNO))
			Utils_OpenUrl(szUrl);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT ToolbarLoadModule()
{
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, ehhToolBarBackgroundSettingsChanged);
	HookEvent(ME_SYSTEM_MODULELOAD, Toolbar_ModuleReloaded);
	HookEvent(ME_SYSTEM_MODULESLOADED, Toolbar_ModulesLoaded);
	return S_OK;
}
