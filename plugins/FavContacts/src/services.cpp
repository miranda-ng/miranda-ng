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

		BYTE fav = db_get_b(event->hContact, "FavContacts", "IsFavourite", 0);
		StatusIconData sid = {};
		sid.szModule = "FavContacts";
		sid.flags = fav ? 0 : MBF_DISABLED;
		Srmm_ModifyIcon(event->hContact, &sid);

		if (event->hContact == hContactToActivate) {
			HWND hwndRoot = event->hwndWindow;
			while (HWND hwndParent = GetParent(hwndRoot))
				hwndRoot = hwndParent;

			AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), NULL), GetCurrentThreadId(), TRUE);
			SetForegroundWindow(hwndRoot);
			SetActiveWindow(hwndRoot);
			SetFocus(hwndRoot);
			AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), NULL), GetCurrentThreadId(), FALSE);
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
	if (mir_strcmp(sicd->szModule, "FavContacts")) return 0;

	if (!hContact)
		return 0;

	if (sicd->flags & MBCF_RIGHTBUTTON) {
		BYTE fav = !db_get_b(hContact, "FavContacts", "IsFavourite", 0);
		db_set_b(hContact, "FavContacts", "IsFavourite", fav);
		if (fav) CallService(MS_AV_GETAVATARBITMAP, hContact, 0);

		StatusIconData sid = {};
		sid.szModule = "FavContacts";
		sid.flags = fav ? 0 : MBF_DISABLED;
		Srmm_ModifyIcon(hContact, &sid);
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
	TTBButton ttb = { 0 };
	ttb.pszTooltipUp = ttb.name = LPGEN("Favorite Contacts");
	ttb.pszService = MS_FAVCONTACTS_SHOWMENU;
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;
	ttb.hIconHandleUp = iconList[0].hIcolib;
	TopToolbar_AddButton(&ttb);
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
	sid.szModule = "FavContacts";
	sid.szTooltip = LPGEN("Favorite Contacts");
	sid.hIcon = IcoLib_GetIconByHandle(iconList[0].hIcolib);
	sid.hIconDisabled = IcoLib_GetIconByHandle(iconList[1].hIcolib);
	Srmm_AddIcon(&sid);

	HookEvent(ME_MSG_ICONPRESSED, ProcessSrmmIconClick);
	HookEvent(ME_MSG_WINDOWEVENT, ProcessSrmmEvent);

	/////////////////////////////////////////////////////////////////////////////////////

	FontIDW fontid = { sizeof(fontid) };
	mir_wstrcpy(fontid.group, LPGENW("Favorite Contacts"));
	mir_strcpy(fontid.dbSettingsGroup, "FavContacts");
	mir_wstrcpy(fontid.backgroundGroup, LPGENW("Favorite Contacts"));
	fontid.flags = FIDF_DEFAULTVALID;
	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.size = -11;
	mir_wstrcpy(fontid.deffontsettings.szFace, L"MS Shell Dlg");
	fontid.deffontsettings.style = 0;

	mir_wstrcpy(fontid.backgroundName, LPGENW("Background"));

	mir_wstrcpy(fontid.name, LPGENW("Contact name"));
	mir_strcpy(fontid.prefix, "fntName");
	fontid.deffontsettings.colour = GetSysColor(COLOR_MENUTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	Font_RegisterW(&fontid);

	mir_wstrcpy(fontid.name, LPGENW("Second line"));
	mir_strcpy(fontid.prefix, "fntSecond");
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_MENUTEXT), GetSysColor(COLOR_MENU));
	fontid.deffontsettings.style = 0;
	Font_RegisterW(&fontid);

	mir_wstrcpy(fontid.backgroundName, LPGENW("Selected background"));

	mir_wstrcpy(fontid.name, LPGENW("Selected contact name (color)"));
	mir_strcpy(fontid.prefix, "fntNameSel");
	fontid.deffontsettings.colour = GetSysColor(COLOR_HIGHLIGHTTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	Font_RegisterW(&fontid);

	mir_wstrcpy(fontid.name, LPGENW("Selected second line (color)"));
	mir_strcpy(fontid.prefix, "fntSecondSel");
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_HIGHLIGHT));
	fontid.deffontsettings.style = 0;
	Font_RegisterW(&fontid);

	/////////////////////////////////////////////////////////////////////////////////////

	ColourIDW colourid = { sizeof(colourid) };
	mir_wstrcpy(colourid.group, LPGENW("Favorite Contacts"));
	mir_strcpy(colourid.dbSettingsGroup, "FavContacts");

	mir_wstrcpy(colourid.name, LPGENW("Background"));
	mir_strcpy(colourid.setting, "BackColour");
	colourid.defcolour = GetSysColor(COLOR_MENU);
	Colour_RegisterW(&colourid);

	mir_wstrcpy(colourid.name, LPGENW("Selected background"));
	mir_strcpy(colourid.setting, "SelectedColour");
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	Colour_RegisterW(&colourid);

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
	Hotkey_Register(&hotkey);

	if (ServiceExists(MS_AV_GETAVATARBITMAP)) {
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			if (db_get_b(hContact, "FavContacts", "IsFavourite", 0))
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
