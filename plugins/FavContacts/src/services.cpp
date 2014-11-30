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

#include "headers.h"

static MCONTACT hContactToActivate;
static HANDLE hDialogsList;

INT_PTR svcShowMenu(WPARAM wParam, LPARAM lParam)
{
	ShowMenu(false);
	return 0;
}

INT_PTR svcShowMenuCentered(WPARAM wParam, LPARAM lParam)
{
	ShowMenu(g_Options.bCenterHotkey ? true : false);
	return 0;
}

INT_PTR svcOpenContact(WPARAM wParam, LPARAM lParam)
{
	hContactToActivate = wParam;
	CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)hContactToActivate, 0);
	return 0;
}

int ProcessSrmmEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		if (!hDialogsList)
			hDialogsList = WindowList_Create();
		WindowList_Add(hDialogsList, event->hwndWindow, event->hContact);

		BYTE fav = db_get_b(event->hContact, "FavContacts", "IsFavourite", 0);
		StatusIconData sid = { sizeof(sid) };
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

		StatusIconData sid = { sizeof(sid) };
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

int ProcessTBLoaded(WPARAM wParam, LPARAM lParam)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.pszTooltipUp = ttb.name = LPGEN("Favorite Contacts");
	ttb.pszService = MS_FAVCONTACTS_SHOWMENU;
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;
	ttb.hIconHandleUp = iconList[0].hIcolib;
	TopToolbar_AddButton(&ttb);
	return 0;
}

int ProcessReloadFonts(WPARAM wParam, LPARAM lParam)
{
	if (g_Options.hfntName) DeleteObject(g_Options.hfntName);
	if (g_Options.hfntSecond) DeleteObject(g_Options.hfntSecond);

	LOGFONT lf = { 0 };
	FontIDT fontid = { sizeof(fontid) };
	mir_tstrcpy(fontid.group, LPGENT("Favorite Contacts"));
	mir_tstrcpy(fontid.name, LPGENT("Contact name"));
	g_Options.clLine1 = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);
	g_Options.hfntName = CreateFontIndirect(&lf);

	mir_tstrcpy(fontid.name, LPGENT("Second line"));
	g_Options.clLine2 = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);
	g_Options.hfntSecond = CreateFontIndirect(&lf);

	mir_tstrcpy(fontid.name, LPGENT("Selected contact name (color)"));
	g_Options.clLine1Sel = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);

	mir_tstrcpy(fontid.name, LPGENT("Selected second line (color)"));
	g_Options.clLine2Sel = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);

	ColourIDT colourid = { sizeof(colourid) };
	mir_tstrcpy(colourid.group, LPGENT("Favorite Contacts"));
	mir_tstrcpy(colourid.name, LPGENT("Background"));
	g_Options.clBack = CallService(MS_COLOUR_GETT, (WPARAM)&colourid, (LPARAM)&lf);

	mir_tstrcpy(colourid.name, LPGENT("Selected background"));
	g_Options.clBackSel = CallService(MS_COLOUR_GETT, (WPARAM)&colourid, (LPARAM)&lf);

	return 0;
}

int ProcessModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_TTB_MODULELOADED, ProcessTBLoaded);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = "FavContacts";
	sid.szTooltip = LPGEN("Favorite Contacts");
	sid.hIcon = Skin_GetIconByHandle(iconList[0].hIcolib);
	sid.hIconDisabled = Skin_GetIconByHandle(iconList[1].hIcolib);
	Srmm_AddIcon(&sid);

	HookEvent(ME_MSG_ICONPRESSED, ProcessSrmmIconClick);
	HookEvent(ME_MSG_WINDOWEVENT, ProcessSrmmEvent);

	/////////////////////////////////////////////////////////////////////////////////////

	FontIDT fontid = { sizeof(fontid) };
	mir_tstrcpy(fontid.group, LPGENT("Favorite Contacts"));
	mir_strcpy(fontid.dbSettingsGroup, "FavContacts");
	mir_tstrcpy(fontid.backgroundGroup, LPGENT("Favorite Contacts"));
	fontid.flags = FIDF_DEFAULTVALID;
	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.size = -11;
	mir_tstrcpy(fontid.deffontsettings.szFace, _T("MS Shell Dlg"));
	fontid.deffontsettings.style = 0;

	mir_tstrcpy(fontid.backgroundName, LPGENT("Background"));

	mir_tstrcpy(fontid.name, LPGENT("Contact name"));
	mir_strcpy(fontid.prefix, "fntName");
	fontid.deffontsettings.colour = GetSysColor(COLOR_MENUTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	FontRegisterT(&fontid);

	mir_tstrcpy(fontid.name, LPGENT("Second line"));
	mir_strcpy(fontid.prefix, "fntSecond");
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_MENUTEXT), GetSysColor(COLOR_MENU));
	fontid.deffontsettings.style = 0;
	FontRegisterT(&fontid);

	mir_tstrcpy(fontid.backgroundName, LPGENT("Selected background"));

	mir_tstrcpy(fontid.name, LPGENT("Selected contact name (color)"));
	mir_strcpy(fontid.prefix, "fntNameSel");
	fontid.deffontsettings.colour = GetSysColor(COLOR_HIGHLIGHTTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	FontRegisterT(&fontid);

	mir_tstrcpy(fontid.name, LPGENT("Selected second line (color)"));
	mir_strcpy(fontid.prefix, "fntSecondSel");
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_HIGHLIGHT));
	fontid.deffontsettings.style = 0;
	FontRegisterT(&fontid);

	/////////////////////////////////////////////////////////////////////////////////////

	ColourIDT colourid = { sizeof(colourid) };
	mir_tstrcpy(colourid.group, LPGENT("Favorite Contacts"));
	mir_strcpy(colourid.dbSettingsGroup, "FavContacts");

	mir_tstrcpy(colourid.name, LPGENT("Background"));
	mir_strcpy(colourid.setting, "BackColour");
	colourid.defcolour = GetSysColor(COLOR_MENU);
	ColourRegisterT(&colourid);

	mir_tstrcpy(colourid.name, LPGENT("Selected background"));
	mir_strcpy(colourid.setting, "SelectedColour");
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	ColourRegisterT(&colourid);

	HookEvent(ME_FONT_RELOAD, ProcessReloadFonts);
	HookEvent(ME_COLOUR_RELOAD, ProcessReloadFonts);
	ProcessReloadFonts(0, 0);

	/////////////////////////////////////////////////////////////////////////////////////

	HOTKEYDESC hotkey = { sizeof(hotkey) };
	hotkey.pszName = "FavContacts/ShowMenu";
	hotkey.pszDescription = LPGEN("Show favorite contacts");
	hotkey.pszSection = "Contacts";
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
