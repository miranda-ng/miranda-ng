/*
Favorite Contacts for Miranda IM

Copyright 2007 Victor Pavlychko

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

static MCONTACT hContactToActivate;
static MWindowList hDialogsList;

INT_PTR svcShowMenu(WPARAM, LPARAM)
{
	ShowMenu(false);
	return 0;
}

INT_PTR svcShowMenuCentered(WPARAM, LPARAM)
{
	ShowMenu(g_Options.bCenterHotkey ? true : false);
	return 0;
}

INT_PTR svcOpenContact(WPARAM wParam, LPARAM)
{
	hContactToActivate = wParam;
	Clist_ContactDoubleClicked(hContactToActivate);
	return 0;
}

int ProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		if (!hDialogsList)
			hDialogsList = WindowList_Create();
		WindowList_Add(hDialogsList, event->hwndWindow, event->hContact);

		uint8_t fav = g_plugin.getByte(event->hContact, "IsFavourite");
		Srmm_SetIconFlags(event->hContact, MODULENAME, 0, fav ? 0 : MBF_DISABLED);

		if (event->hContact == hContactToActivate) {
			HWND hwndRoot = event->hwndWindow;
			while (HWND hwndParent = GetParent(hwndRoot))
				hwndRoot = hwndParent;

			AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), nullptr), GetCurrentThreadId(), TRUE);
			SetForegroundWindow(hwndRoot);
			SetActiveWindow(hwndRoot);
			SetFocus(hwndRoot);
			AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), nullptr), GetCurrentThreadId(), FALSE);
		}

		hContactToActivate = NULL;
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSING) {
		if (hDialogsList)
			WindowList_Remove(hDialogsList, event->hwndWindow);
	}

	return 0;
}

int ProcessSrmmIconClick(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (mir_strcmp(sicd->szModule, MODULENAME))
		return 0;

	if (!hContact)
		return 0;

	if (sicd->flags & MBCF_RIGHTBUTTON) {
		uint8_t fav = !g_plugin.getByte(hContact, "IsFavourite");
		g_plugin.setByte(hContact, "IsFavourite", fav);
		if (fav)
			CallService(MS_AV_GETAVATARBITMAP, hContact, 0);

		Srmm_SetIconFlags(hContact, MODULENAME, 0, fav ? 0 : MBF_DISABLED);
	}
	else ShowMenu(false);

	return 0;
}

static __forceinline COLORREF sttShadeColor(COLORREF clLine1, COLORREF clBack)
{
	return RGB(
		(GetRValue(clLine1) * 66UL + GetRValue(clBack) * 34UL) / 100,
		(GetGValue(clLine1) * 66UL + GetGValue(clBack) * 34UL) / 100,
		(GetBValue(clLine1) * 66UL + GetBValue(clBack) * 34UL) / 100);
}

int ProcessTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.pszTooltipUp = ttb.name = LPGEN("Favorite Contacts");
	ttb.pszService = MS_FAVCONTACTS_SHOWMENU;
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;
	ttb.hIconHandleUp = iconList[0].hIcolib;
	g_plugin.addTTB(&ttb);
	return 0;
}

int ProcessReloadFonts(WPARAM, LPARAM)
{
	if (g_Options.hfntName) DeleteObject(g_Options.hfntName);
	if (g_Options.hfntSecond) DeleteObject(g_Options.hfntSecond);

	LOGFONT lf = { 0 };
	g_Options.clLine1 = Font_GetW(LPGENW("Favorite Contacts"), LPGENW("Contact name"), &lf);
	g_Options.hfntName = CreateFontIndirect(&lf);

	g_Options.clLine2 = Font_GetW(LPGENW("Favorite Contacts"), LPGENW("Second line"), &lf);
	g_Options.hfntSecond = CreateFontIndirect(&lf);

	g_Options.clLine1Sel = Font_GetW(LPGENW("Favorite Contacts"), LPGENW("Selected contact name (color)"), &lf);
	g_Options.clLine2Sel = Font_GetW(LPGENW("Favorite Contacts"), LPGENW("Selected second line (color)"), &lf);

	g_Options.clBack = Colour_GetW(LPGENW("Favorite Contacts"), LPGENW("Background"));
	g_Options.clBackSel = Colour_GetW(LPGENW("Favorite Contacts"), LPGENW("Selected background"));

	return 0;
}

int ProcessModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_TTB_MODULELOADED, ProcessTBLoaded);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.szTooltip.a = LPGEN("Favorite Contacts");
	sid.hIcon = IcoLib_GetIconByHandle(iconList[0].hIcolib);
	sid.hIconDisabled = IcoLib_GetIconByHandle(iconList[1].hIcolib);
	Srmm_AddIcon(&sid, &g_plugin);

	HookEvent(ME_MSG_ICONPRESSED, ProcessSrmmIconClick);
	HookEvent(ME_MSG_WINDOWEVENT, ProcessSrmmEvent);

	/////////////////////////////////////////////////////////////////////////////////////

	FontIDW fontid = {};
	wcsncpy_s(fontid.group, LPGENW("Favorite Contacts"), _TRUNCATE);
	strncpy_s(fontid.dbSettingsGroup, MODULENAME, _TRUNCATE);
	wcsncpy_s(fontid.backgroundGroup, LPGENW("Favorite Contacts"), _TRUNCATE);
	fontid.flags = FIDF_DEFAULTVALID;
	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.size = -11;
	wcsncpy_s(fontid.deffontsettings.szFace, L"MS Shell Dlg", _TRUNCATE);
	fontid.deffontsettings.style = 0;

	wcsncpy_s(fontid.backgroundName, LPGENW("Background"), _TRUNCATE);

	wcsncpy_s(fontid.name, LPGENW("Contact name"), _TRUNCATE);
	strncpy_s(fontid.setting, "fntName", _TRUNCATE);
	fontid.deffontsettings.colour = GetSysColor(COLOR_MENUTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	g_plugin.addFont(&fontid);

	wcsncpy_s(fontid.name, LPGENW("Second line"), _TRUNCATE);
	strncpy_s(fontid.setting, "fntSecond", _TRUNCATE);
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_MENUTEXT), GetSysColor(COLOR_MENU));
	fontid.deffontsettings.style = 0;
	g_plugin.addFont(&fontid);

	wcsncpy_s(fontid.backgroundName, LPGENW("Selected background"), _TRUNCATE);

	wcsncpy_s(fontid.name, LPGENW("Selected contact name (color)"), _TRUNCATE);
	strncpy_s(fontid.setting, "fntNameSel", _TRUNCATE);
	fontid.deffontsettings.colour = GetSysColor(COLOR_HIGHLIGHTTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	g_plugin.addFont(&fontid);

	wcsncpy_s(fontid.name, LPGENW("Selected second line (color)"), _TRUNCATE);
	strncpy_s(fontid.setting, "fntSecondSel", _TRUNCATE);
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_HIGHLIGHT));
	fontid.deffontsettings.style = 0;
	g_plugin.addFont(&fontid);

	/////////////////////////////////////////////////////////////////////////////////////

	ColourIDW colourid = {};
	wcsncpy_s(colourid.group, LPGENW("Favorite Contacts"), _TRUNCATE);
	strncpy_s(colourid.dbSettingsGroup, MODULENAME, _TRUNCATE);

	wcsncpy_s(colourid.name, LPGENW("Background"), _TRUNCATE);
	strncpy_s(colourid.setting, "BackColour", _TRUNCATE);
	colourid.defcolour = GetSysColor(COLOR_MENU);
	g_plugin.addColor(&colourid);

	wcsncpy_s(colourid.name, LPGENW("Selected background"), _TRUNCATE);
	strncpy_s(colourid.setting, "SelectedColour", _TRUNCATE);
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	g_plugin.addColor(&colourid);

	HookEvent(ME_FONT_RELOAD, ProcessReloadFonts);
	HookEvent(ME_COLOUR_RELOAD, ProcessReloadFonts);
	ProcessReloadFonts(0, 0);

	/////////////////////////////////////////////////////////////////////////////////////

	HOTKEYDESC hotkey = {};
	hotkey.pszName = "FavContacts/ShowMenu";
	hotkey.szDescription.a = LPGEN("Show favorite contacts");
	hotkey.szSection.a = "Contacts";
	hotkey.pszService = MS_FAVCONTACTS_SHOWMENU_CENTERED;
	hotkey.DefHotKey = MAKEWORD('Q', HOTKEYF_EXT);
	g_plugin.addHotkey(&hotkey);

	if (ServiceExists(MS_AV_GETAVATARBITMAP)) {
		for (auto &hContact : Contacts())
			if (g_plugin.getByte(hContact, "IsFavourite"))
				CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
	}

	return 0;
}

void InitServices()
{
	CreateServiceFunction(MS_FAVCONTACTS_SHOWMENU, svcShowMenu);
	CreateServiceFunction(MS_FAVCONTACTS_SHOWMENU_CENTERED, svcShowMenuCentered);
	CreateServiceFunction(MS_FAVCONTACTS_OPEN_CONTACT, svcOpenContact);

	HookEvent(ME_OPT_INITIALISE, ProcessOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, ProcessModulesLoaded);
}

void UninitServices()
{
	WindowList_Destroy(hDialogsList);
}
