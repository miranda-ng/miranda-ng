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

CLIST_INTERFACE *pcli;

HINSTANCE g_hInst;

int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {CE2C0401-F9E0-40D7-8E95-1A4197D7AB04}
	{0xce2c0401, 0xf9e0, 0x40d7, {0x8e, 0x95, 0x1a, 0x41, 0x97, 0xd7, 0xab, 0x4}}
};

static IconItem iconList[] =
{
	{ LPGEN("Favorite Contact"), "favcontacts_favorite", IDI_FAVORITE },
	{ LPGEN("Regular Contact"), "favcontacts_regular", IDI_REGULAR },
};

#define MS_FAVCONTACTS_SHOWMENU				"FavContacts/ShowMenu"
#define MS_FAVCONTACTS_SHOWMENU_CENTERED	"FavContacts/ShowMenuCentered"
#define MS_FAVCONTACTS_OPEN_CONTACT			"FavContacts/OpenContact"

HWND g_hwndMenuHost = NULL;
static LRESULT CALLBACK MenuHostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void sttLoadOptions();
static void sttSaveOptions();

int sttShowMenu(bool centered);

INT_PTR svcOpenContact(WPARAM wParam, LPARAM lParam);

INT_PTR svcShowMenu(WPARAM wParam, LPARAM lParam);
INT_PTR svcShowMenuCentered(WPARAM wParam, LPARAM lParam);
int ProcessSrmmEvent(WPARAM wParam, LPARAM lParam);
int ProcessSrmmIconClick(WPARAM wParam, LPARAM lParam);

float g_widthMultiplier = 0;
UINT  g_maxItemWidth = 0;

CContactCache *g_contactCache = NULL;
TCHAR g_filter[1024] = { 0 };

Options g_Options = { 0 };

static HANDLE hDialogsList = NULL;
static MCONTACT hContactToActivate = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static __forceinline COLORREF sttShadeColor(COLORREF clLine1, COLORREF clBack)
{
	return RGB(
		(GetRValue(clLine1) * 66UL + GetRValue(clBack) * 34UL) / 100,
		(GetGValue(clLine1) * 66UL + GetGValue(clBack) * 34UL) / 100,
		(GetBValue(clLine1) * 66UL + GetBValue(clBack) * 34UL) / 100);
}

static TCHAR* sttGetGroupName(int id)
{
	if (id == 1) {
		if (g_Options.bUseGroups)
			return TranslateT("<no group>");
		return TranslateT("Favorite Contacts");
	}

	return pcli->pfnGetGroupName(id - 1, NULL);
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
	lstrcpy(fontid.group, LPGENT("Favorite Contacts"));
	lstrcpy(fontid.name, LPGENT("Contact name"));
	g_Options.clLine1 = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);
	g_Options.hfntName = CreateFontIndirect(&lf);

	lstrcpy(fontid.name, LPGENT("Second line"));
	g_Options.clLine2 = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);
	g_Options.hfntSecond = CreateFontIndirect(&lf);

	lstrcpy(fontid.name, LPGENT("Selected contact name (color)"));
	g_Options.clLine1Sel = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);

	lstrcpy(fontid.name, LPGENT("Selected second line (color)"));
	g_Options.clLine2Sel = CallService(MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)&lf);

	ColourIDT colourid = { sizeof(colourid) };
	lstrcpy(colourid.group, LPGENT("Favorite Contacts"));
	lstrcpy(colourid.name, LPGENT("Background"));
	g_Options.clBack = CallService(MS_COLOUR_GETT, (WPARAM)&colourid, (LPARAM)&lf);

	lstrcpy(colourid.name, LPGENT("Selected background"));
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
	lstrcpy(fontid.group, LPGENT("Favorite Contacts"));
	lstrcpyA(fontid.dbSettingsGroup, "FavContacts");
	lstrcpy(fontid.backgroundGroup, LPGENT("Favorite Contacts"));
	fontid.flags = FIDF_DEFAULTVALID;
	fontid.deffontsettings.charset = DEFAULT_CHARSET;
	fontid.deffontsettings.size = -11;
	lstrcpy(fontid.deffontsettings.szFace, _T("MS Shell Dlg"));
	fontid.deffontsettings.style = 0;

	lstrcpy(fontid.backgroundName, LPGENT("Background"));

	lstrcpy(fontid.name, LPGENT("Contact name"));
	lstrcpyA(fontid.prefix, "fntName");
	fontid.deffontsettings.colour = GetSysColor(COLOR_MENUTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	FontRegisterT(&fontid);

	lstrcpy(fontid.name, LPGENT("Second line"));
	lstrcpyA(fontid.prefix, "fntSecond");
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_MENUTEXT), GetSysColor(COLOR_MENU));
	fontid.deffontsettings.style = 0;
	FontRegisterT(&fontid);

	lstrcpy(fontid.backgroundName, LPGENT("Selected background"));

	lstrcpy(fontid.name, LPGENT("Selected contact name (color)"));
	lstrcpyA(fontid.prefix, "fntNameSel");
	fontid.deffontsettings.colour = GetSysColor(COLOR_HIGHLIGHTTEXT);
	fontid.deffontsettings.style = DBFONTF_BOLD;
	FontRegisterT(&fontid);

	lstrcpy(fontid.name, LPGENT("Selected second line (color)"));
	lstrcpyA(fontid.prefix, "fntSecondSel");
	fontid.deffontsettings.colour = sttShadeColor(GetSysColor(COLOR_HIGHLIGHTTEXT), GetSysColor(COLOR_HIGHLIGHT));
	fontid.deffontsettings.style = 0;
	FontRegisterT(&fontid);

	/////////////////////////////////////////////////////////////////////////////////////

	ColourIDT colourid = { sizeof(colourid) };
	lstrcpy(colourid.group, LPGENT("Favorite Contacts"));
	lstrcpyA(colourid.dbSettingsGroup, "FavContacts");

	lstrcpy(colourid.name, LPGENT("Background"));
	lstrcpyA(colourid.setting, "BackColour");
	colourid.defcolour = GetSysColor(COLOR_MENU);
	ColourRegisterT(&colourid);

	lstrcpy(colourid.name, LPGENT("Selected background"));
	lstrcpyA(colourid.setting, "SelectedColour");
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

int ProcessOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Contacts");
	odp.pszTitle = LPGEN("Favorites");
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = OptionsDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	g_contactCache = new CContactCache;

	WNDCLASSEX wcl = { sizeof(wcl) };
	wcl.lpfnWndProc = MenuHostWndProc;
	wcl.hInstance = g_hInst;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszClassName = _T("FavContactsMenuHostWnd");
	RegisterClassEx(&wcl);

	g_hwndMenuHost = CreateWindow(_T("FavContactsMenuHostWnd"), NULL, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, g_hInst, NULL);
	SetWindowPos(g_hwndMenuHost, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_DEFERERASE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

	sttLoadOptions();

	CreateServiceFunction(MS_FAVCONTACTS_SHOWMENU, svcShowMenu);
	CreateServiceFunction(MS_FAVCONTACTS_SHOWMENU_CENTERED, svcShowMenuCentered);
	CreateServiceFunction(MS_FAVCONTACTS_OPEN_CONTACT, svcOpenContact);

	HookEvent(ME_OPT_INITIALISE, ProcessOptInitialise);
	HookEvent(ME_SYSTEM_MODULESLOADED, ProcessModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, UnloadHttpApi);

	/////////////////////////////////////////////////////////////////////////////////////

	Icon_Register(g_hInst, LPGEN("Favorites"), iconList, SIZEOF(iconList));

	LoadHttpApi();

#ifdef _DEBUG
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS);
	mi.position = 1900000000;
	mi.pszName = LPGEN("&Favorite Contacts...");
	mi.pszService = MS_FAVCONTACTS_SHOWMENU;
	Menu_AddMainMenuItem(&mi);
#endif

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hDialogsList);

	if (g_hwndMenuHost) DestroyWindow(g_hwndMenuHost);
	if (g_Options.hfntName) DeleteObject(g_Options.hfntName);
	if (g_Options.hfntSecond) DeleteObject(g_Options.hfntSecond);

	delete g_contactCache;
	return 0;
}

static void sttLoadOptions()
{
	g_Options.bSecondLine = db_get_b(NULL, "FavContacts", "SecondLine", 1);
	g_Options.bAvatars = db_get_b(NULL, "FavContacts", "Avatars", 1);
	g_Options.bAvatarBorder = db_get_b(NULL, "FavContacts", "AvatarBorder", 0);
	g_Options.wAvatarRadius = db_get_w(NULL, "FavContacts", "AvatarRadius", 3);
	g_Options.bNoTransparentBorder = db_get_b(NULL, "FavContacts", "NoTransparentBorder",
															!db_get_b(NULL, "FavContacts", "AvatarBorderTransparent", 1));
	g_Options.bSysColors = db_get_b(NULL, "FavContacts", "SysColors", 0);
	g_Options.bCenterHotkey = db_get_b(NULL, "FavContacts", "CenterHotkey", 1);
	g_Options.bUseGroups = db_get_b(NULL, "FavContacts", "UseGroups", 0);
	g_Options.bUseColumns = db_get_b(NULL, "FavContacts", "UseColumns", 1);
	g_Options.bRightAvatars = db_get_b(NULL, "FavContacts", "RightAvatars", 0);
	g_Options.bDimIdle = db_get_b(NULL, "FavContacts", "DimIdle", 1);

	g_Options.wMaxRecent = db_get_b(NULL, "FavContacts", "MaxRecent", 10);
}

static void sttSaveOptions()
{
	db_set_b(NULL, "FavContacts", "SecondLine", g_Options.bSecondLine);
	db_set_b(NULL, "FavContacts", "Avatars", g_Options.bAvatars);
	db_set_b(NULL, "FavContacts", "AvatarBorder", g_Options.bAvatarBorder);
	db_set_w(NULL, "FavContacts", "AvatarRadius", g_Options.wAvatarRadius);
	db_set_b(NULL, "FavContacts", "NoTransparentBorder", g_Options.bNoTransparentBorder);
	db_set_b(NULL, "FavContacts", "SysColors", g_Options.bSysColors);
	db_set_b(NULL, "FavContacts", "CenterHotkey", g_Options.bCenterHotkey);
	db_set_b(NULL, "FavContacts", "UseGroups", g_Options.bUseGroups);
	db_set_b(NULL, "FavContacts", "UseColumns", g_Options.bUseColumns);
	db_set_b(NULL, "FavContacts", "RightAvatars", g_Options.bRightAvatars);
	db_set_b(NULL, "FavContacts", "DimIdle", g_Options.bDimIdle);
	db_set_w(NULL, "FavContacts", "MaxRecent", g_Options.wMaxRecent);
}

static BOOL sttMeasureItem_Group(LPMEASUREITEMSTRUCT lpmis, Options *options)
{
	if (true) {
		HDC hdc = GetDC(g_hwndMenuHost);
		HFONT hfntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
		TCHAR *name = sttGetGroupName(lpmis->itemData);
		SIZE sz;
		if (!options->bSysColors)
			SelectObject(hdc, g_Options.hfntName);
		GetTextExtentPoint32(hdc, name, lstrlen(name), &sz);
		lpmis->itemHeight = sz.cy + 8;
		lpmis->itemWidth = sz.cx + 10;
		SelectObject(hdc, hfntSave);
		ReleaseDC(g_hwndMenuHost, hdc);
	}

	return TRUE;
}

static BOOL sttMeasureItem_Contact(LPMEASUREITEMSTRUCT lpmis, Options *options)
{
	MCONTACT hContact = (MCONTACT)lpmis->itemData;

	lpmis->itemHeight = 4;
	lpmis->itemWidth = 8 + 10;

	if (true) {
		lpmis->itemWidth += 20;
	}

	if (true) {
		SIZE sz;
		int textWidth = 0;

		HDC hdc = GetDC(g_hwndMenuHost);
		HFONT hfntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

		if (options->bSecondLine) {
			DBVARIANT dbv;
			TCHAR *title;
			bool bFree = false;
			if (db_get_ts(hContact, "CList", "StatusMsg", &dbv) || !*dbv.ptszVal) {
				char *proto = GetContactProto(hContact);
				int status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
				title = pcli->pfnGetStatusModeDescription(status, 0);
			}
			else {
				title = dbv.ptszVal;
				bFree = true;
			}

			if (!options->bSysColors) SelectObject(hdc, g_Options.hfntSecond);
			GetTextExtentPoint32(hdc, title, lstrlen(title), &sz);
			if (bFree) db_free(&dbv);
			textWidth = sz.cx;
			lpmis->itemHeight += sz.cy + 3;
		}

		TCHAR *name = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

		if (!options->bSysColors) SelectObject(hdc, g_Options.hfntName);
		GetTextExtentPoint32(hdc, name, lstrlen(name), &sz);
		textWidth = max(textWidth, sz.cx);

		SelectObject(hdc, hfntSave);
		ReleaseDC(g_hwndMenuHost, hdc);

		lpmis->itemWidth += textWidth;
		lpmis->itemHeight += sz.cy;
	}

	if (options->bAvatars) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
		if (ace && (ace != (AVATARCACHEENTRY *)CALLSERVICE_NOTFOUND)) {
			int avatarWidth = lpmis->itemHeight;
			if (ace->bmWidth < ace->bmHeight)
				avatarWidth = lpmis->itemHeight * ace->bmWidth / ace->bmHeight;

			lpmis->itemWidth += avatarWidth + 5;
		}
	}

	if (lpmis->itemHeight < 18) lpmis->itemHeight = 18;

	return TRUE;
}

static BOOL sttMeasureItem(LPMEASUREITEMSTRUCT lpmis, Options *options = NULL)
{
	if (!options) options = &g_Options;

	if (!lpmis->itemData)
		return FALSE;

	BOOL res = FALSE;
	if (lpmis->itemData == 1)
		res = sttMeasureItem_Group(lpmis, options);
	else if (CallService(MS_DB_CONTACT_IS, lpmis->itemData, 0))
		res = sttMeasureItem_Contact(lpmis, options);

	if (res && (lpmis->itemWidth > g_maxItemWidth)) lpmis->itemWidth = g_maxItemWidth;
	if (res && g_widthMultiplier) lpmis->itemWidth *= g_widthMultiplier;

	return FALSE;
}

static BOOL sttDrawItem_Group(LPDRAWITEMSTRUCT lpdis, Options *options = NULL)
{
	lpdis->rcItem.top++;
	lpdis->rcItem.bottom--;

	HFONT hfntSave = (HFONT)SelectObject(lpdis->hDC, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(lpdis->hDC, TRANSPARENT);
	if (options->bSysColors) {
		FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else {
		HBRUSH hbr = CreateSolidBrush(g_Options.clBackSel);
		FillRect(lpdis->hDC, &lpdis->rcItem, hbr);
		DeleteObject(hbr);
		SetTextColor(lpdis->hDC, g_Options.clLine1Sel);
	}

	TCHAR *name = sttGetGroupName(lpdis->itemData);
	if (!options->bSysColors)
		SelectObject(lpdis->hDC, g_Options.hfntName);
	DrawText(lpdis->hDC, name, lstrlen(name), &lpdis->rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	SelectObject(lpdis->hDC, hfntSave);

	return TRUE;
}

void ImageList_DrawDimmed(HIMAGELIST himl, int i, HDC hdc, int left, int top, UINT fStyle)
{
	int dx, dy;
	ImageList_GetIconSize(himl, &dx, &dy);

	HDC dcMem = CreateCompatibleDC(hdc);
	HBITMAP hbm = CreateCompatibleBitmap(hdc, dx, dy);
	HBITMAP hbmOld = (HBITMAP)SelectObject(dcMem, hbm);
	BitBlt(dcMem, 0, 0, dx, dx, hdc, left, top, SRCCOPY);
	ImageList_Draw(himl, i, dcMem, 0, 0, fStyle);
	BLENDFUNCTION bf = { 0 };
	bf.SourceConstantAlpha = 180;
	GdiAlphaBlend(hdc, left, top, dx, dy, dcMem, 0, 0, dx, dy, bf);
	SelectObject(dcMem, hbmOld);
	DeleteObject(hbm);
	DeleteDC(dcMem);
}

static BOOL sttDrawItem_Contact(LPDRAWITEMSTRUCT lpdis, Options *options = NULL)
{
	MCONTACT hContact = (MCONTACT)lpdis->itemData;

	HDC hdcTemp = CreateCompatibleDC(lpdis->hDC);
	HBITMAP hbmTemp = CreateCompatibleBitmap(lpdis->hDC, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top);
	HBITMAP hbmSave = (HBITMAP)SelectObject(hdcTemp, hbmTemp);
	RECT rcSave = lpdis->rcItem;

	OffsetRect(&lpdis->rcItem, -lpdis->rcItem.left, -lpdis->rcItem.top);

	HFONT hfntSave = (HFONT)SelectObject(hdcTemp, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(hdcTemp, TRANSPARENT);
	COLORREF clBack, clLine1, clLine2;
	if (lpdis->itemState & ODS_SELECTED) {
		if (options->bSysColors) {
			FillRect(hdcTemp, &lpdis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
			clBack = GetSysColor(COLOR_HIGHLIGHT);
			clLine1 = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else {
			clBack = g_Options.clBackSel;
			clLine1 = g_Options.clLine1Sel;
			clLine2 = g_Options.clLine2Sel;
		}
	}
	else {
		if (options->bSysColors) {
			FillRect(hdcTemp, &lpdis->rcItem, GetSysColorBrush(COLOR_MENU));
			clBack = GetSysColor(COLOR_MENU);
			clLine1 = GetSysColor(COLOR_MENUTEXT);
		}
		else {
			clBack = g_Options.clBack;
			clLine1 = g_Options.clLine1;
			clLine2 = g_Options.clLine2;
		}
	}
	if (options->bSysColors) {
		clLine2 = RGB(
			(GetRValue(clLine1) * 66UL + GetRValue(clBack) * 34UL) / 100,
			(GetGValue(clLine1) * 66UL + GetGValue(clBack) * 34UL) / 100,
			(GetBValue(clLine1) * 66UL + GetBValue(clBack) * 34UL) / 100);
	}
	else {
		HBRUSH hbr = CreateSolidBrush(clBack);
		FillRect(hdcTemp, &lpdis->rcItem, hbr);
		DeleteObject(hbr);
	}

	lpdis->rcItem.left += 4;
	lpdis->rcItem.right -= 4;

	lpdis->rcItem.top += 2;
	lpdis->rcItem.bottom -= 2;

	char *proto = GetContactProto(hContact);

	HIMAGELIST hIml = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
	int iIcon = CallService(MS_CLIST_GETCONTACTICON, hContact, 0);

	if (db_get_dw(hContact, proto, "IdleTS", 0)) {
		ImageList_DrawDimmed(hIml, iIcon, hdcTemp,
									lpdis->rcItem.left, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
									ILD_TRANSPARENT);
	}
	else {
		ImageList_Draw(hIml, iIcon, hdcTemp,
							lpdis->rcItem.left, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
							ILD_TRANSPARENT);
	}

	lpdis->rcItem.left += 20;

	if (options->wMaxRecent && db_get_b(hContact, "FavContacts", "IsFavourite", 0)) {
		DrawIconEx(hdcTemp, lpdis->rcItem.right - 18, (lpdis->rcItem.top + lpdis->rcItem.bottom - 16) / 2,
					  Skin_GetIconByHandle(iconList[0].hIcolib), 16, 16, 0, NULL, DI_NORMAL);
		lpdis->rcItem.right -= 20;
	}

	if (options->bAvatars) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
		if (ace && (ace != (AVATARCACHEENTRY *)CALLSERVICE_NOTFOUND)) {
			int avatarWidth = lpdis->rcItem.bottom - lpdis->rcItem.top;
			if (ace->bmWidth < ace->bmHeight)
				avatarWidth = (lpdis->rcItem.bottom - lpdis->rcItem.top) * ace->bmWidth / ace->bmHeight;

			AVATARDRAWREQUEST avdr = { 0 };
			avdr.cbSize = sizeof(avdr);
			avdr.hContact = hContact;
			avdr.hTargetDC = hdcTemp;
			avdr.rcDraw = lpdis->rcItem;
			if (options->bRightAvatars)
				avdr.rcDraw.left = avdr.rcDraw.right - avatarWidth;
			else
				avdr.rcDraw.right = avdr.rcDraw.left + avatarWidth;
			avdr.dwFlags = AVDRQ_FALLBACKPROTO;
			if (options->bAvatarBorder) {
				avdr.dwFlags |= AVDRQ_DRAWBORDER;
				avdr.clrBorder = clLine1;
				if (options->bNoTransparentBorder)
					avdr.dwFlags |= AVDRQ_HIDEBORDERONTRANSPARENCY;
				if (options->wAvatarRadius) {
					avdr.dwFlags |= AVDRQ_ROUNDEDCORNER;
					avdr.radius = (unsigned char)options->wAvatarRadius;
				}
			}
			avdr.alpha = 255;
			CallService(MS_AV_DRAWAVATAR, 0, (LPARAM)&avdr);

			if (options->bRightAvatars)
				lpdis->rcItem.right += avatarWidth + 5;
			else
				lpdis->rcItem.left += avatarWidth + 5;
		}
	}

	if (true) {
		TCHAR *name = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

		if (!options->bSysColors) SelectObject(hdcTemp, g_Options.hfntName);
		SetTextColor(hdcTemp, clLine1);
		DrawText(hdcTemp, name, lstrlen(name), &lpdis->rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_TOP | DT_LEFT);

		SIZE sz; GetTextExtentPoint32(hdcTemp, name, lstrlen(name), &sz);
		lpdis->rcItem.top += sz.cy + 3;
	}

	if (options->bSecondLine) {
		DBVARIANT dbv;
		TCHAR *title;
		bool bFree = false;
		if (db_get_ts(hContact, "CList", "StatusMsg", &dbv) || !*dbv.ptszVal) {
			int status = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE);
			title = pcli->pfnGetStatusModeDescription(status, 0);
		}
		else {
			title = dbv.ptszVal;
			bFree = true;
		}

		if (!options->bSysColors) SelectObject(hdcTemp, g_Options.hfntSecond);
		SetTextColor(hdcTemp, clLine2);
		DrawText(hdcTemp, title, lstrlen(title), &lpdis->rcItem, DT_NOPREFIX | DT_SINGLELINE | DT_TOP | DT_LEFT);

		if (bFree) db_free(&dbv);
	}

	SelectObject(hdcTemp, hfntSave);

	BitBlt(lpdis->hDC,
			 rcSave.left, rcSave.top,
			 rcSave.right - rcSave.left, rcSave.bottom - rcSave.top,
			 hdcTemp, 0, 0, SRCCOPY);

	SelectObject(hdcTemp, hbmSave);
	DeleteObject(hbmTemp);
	DeleteDC(hdcTemp);

	return TRUE;
}

static BOOL sttDrawItem(LPDRAWITEMSTRUCT lpdis, Options *options = NULL)
{
	if (!options) options = &g_Options;

	if (!lpdis->itemData)
		return FALSE;

	if (lpdis->itemData == 1)
		return sttDrawItem_Group(lpdis, options);

	if (CallService(MS_DB_CONTACT_IS, lpdis->itemData, 0))
		return sttDrawItem_Contact(lpdis, options);

	return FALSE;
}

static LRESULT CALLBACK MenuHostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hContact = NULL;

	switch (message) {
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
			if (lpmis->CtlType != ODT_MENU)
				return FALSE;

			if ((lpmis->itemID >= CLISTMENUIDMIN) && (lpmis->itemID <= CLISTMENUIDMAX))
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

			return sttMeasureItem(lpmis);
		}

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlType != ODT_MENU)
				return FALSE;

			if ((lpdis->itemID >= CLISTMENUIDMIN) && (lpdis->itemID <= CLISTMENUIDMAX))
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

			return sttDrawItem(lpdis);
		}

	case WM_MENUCHAR:
		while (GetMenuItemCount((HMENU)lParam) > 1)
			RemoveMenu((HMENU)lParam, 1, MF_BYPOSITION);

		if (LOWORD(wParam) == VK_BACK) {
			if (int l = lstrlen(g_filter))
				g_filter[l - 1] = 0;
		}
		else if (_istalnum(LOWORD(wParam))) {
			if (lstrlen(g_filter) < SIZEOF(g_filter) - 1) {
				TCHAR s[] = { LOWORD(wParam), 0 };
				lstrcat(g_filter, s);
			}
		}
		{
			int maxRecent = g_Options.wMaxRecent ? g_Options.wMaxRecent : 10;
			for (int i = 0, nRecent = 0; nRecent < maxRecent; ++i) {
				MCONTACT hContact = g_contactCache->get(i);
				if (!hContact) break;
				if (!g_contactCache->filter(i, g_filter)) continue;

				AppendMenu((HMENU)lParam, MF_OWNERDRAW, nRecent + 1, (LPCTSTR)hContact);
				++nRecent;
			}
		}
		return MAKELRESULT(1, MNC_SELECT);

	case WM_MENURBUTTONUP:
		MENUITEMINFO mii = { sizeof(mii) };
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo((HMENU)lParam, wParam, TRUE, &mii);
		MCONTACT hContact = (MCONTACT)mii.dwItemData;
		if (!CallService(MS_DB_CONTACT_IS, mii.dwItemData, 0))
			return FALSE;

		HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, hContact, 0);

		POINT pt;
		GetCursorPos(&pt);
		HWND hwndSave = GetForegroundWindow();
		SetForegroundWindow(g_hwndMenuHost);
		int res = TrackPopupMenu(hMenu, TPM_RECURSE | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, g_hwndMenuHost, NULL);
		SetForegroundWindow(hwndSave);
		DestroyMenu(hMenu);

		CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(res, MPCF_CONTACTMENU), hContact);
		return TRUE;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

int sttShowMenu(bool centered)
{
	TFavContacts favList;
	HMENU hMenu = CreatePopupMenu();
	SIZE szMenu = { 0 };
	SIZE szColumn = { 0 };
	TCHAR *prevGroup = NULL;
	int i, idItem = 100;
	MCONTACT hContact;

	favList.build();

	g_widthMultiplier = 0;

	g_maxItemWidth = GetSystemMetrics(SM_CXSCREEN);
	if (g_Options.bUseColumns) g_maxItemWidth /= favList.groupCount();

	prevGroup = NULL;
	for (i = 0; i < favList.getCount(); ++i) {
		hContact = favList[i]->getHandle();

		MEASUREITEMSTRUCT mis = { 0 };
		mis.CtlID = 0;
		mis.CtlType = ODT_MENU;

		if (!prevGroup || lstrcmp(prevGroup, favList[i]->getGroup())) {
			if (prevGroup && g_Options.bUseColumns) {
				szMenu.cx += szColumn.cx;
				szMenu.cy = max(szMenu.cy, szColumn.cy);
				szColumn.cx = szColumn.cy = 0;
			}

			DWORD groupID = (DWORD)Clist_GroupExists(favList[i]->getGroup()) + 1;

			AppendMenu(hMenu,
						  MF_OWNERDRAW | MF_SEPARATOR | ((prevGroup && g_Options.bUseColumns) ? MF_MENUBREAK : 0),
						  ++idItem, (LPCTSTR)groupID);

			mis.itemData = groupID;
			mis.itemID = idItem;
			sttMeasureItem(&mis);
			szColumn.cx = max(szColumn.cx, (int)mis.itemWidth);
			szColumn.cy += mis.itemHeight;
		}

		AppendMenu(hMenu, MF_OWNERDRAW, ++idItem, (LPCTSTR)hContact);

		mis.itemData = (DWORD)hContact;
		mis.itemID = idItem;
		sttMeasureItem(&mis);
		szColumn.cx = max(szColumn.cx, (int)mis.itemWidth);
		szColumn.cy += mis.itemHeight;

		prevGroup = favList[i]->getGroup();
	}
	szMenu.cx += szColumn.cx;
	szMenu.cy = max(szMenu.cy, szColumn.cy);
	szColumn.cx = szColumn.cy = 0;

	int maxWidth = GetSystemMetrics(SM_CXSCREEN) * db_get_b(NULL, "FavContacts", "MenuWidth", 66) / 100;
	if (szMenu.cx > maxWidth) {
		g_widthMultiplier = (float)maxWidth / szMenu.cx;
		szMenu.cx *= g_widthMultiplier;
	}

	POINT pt;
	if (centered) {
		if ((pt.x = (GetSystemMetrics(SM_CXSCREEN) - szMenu.cx) / 2) < 0) pt.x = 0;
		if ((pt.y = (GetSystemMetrics(SM_CYSCREEN) - szMenu.cy) / 2) < 0) pt.y = 0;
	}
	else GetCursorPos(&pt);

	HWND hwndSave = GetForegroundWindow();
	SetForegroundWindow(g_hwndMenuHost);
	hContact = NULL;
	g_filter[0] = 0;

	if (int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, g_hwndMenuHost, NULL)) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(hMenu, res, FALSE, &mii);
		hContact = (MCONTACT)mii.dwItemData;
	}
	SetForegroundWindow(hwndSave);
	DestroyMenu(hMenu);

	if (hContact)
		CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);

	return 0;
}

INT_PTR svcShowMenu(WPARAM wParam, LPARAM lParam)
{
	sttShowMenu(false);
	return 0;
}

INT_PTR svcShowMenuCentered(WPARAM wParam, LPARAM lParam)
{
	sttShowMenu(g_Options.bCenterHotkey ? true : false);
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
	if (lstrcmpA(sicd->szModule, "FavContacts")) return 0;

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
	else sttShowMenu(false);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options

static void sttResetListOptions(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETBKBITMAP, 0, (LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList, CLM_SETBKCOLOR, GetSysColor(COLOR_WINDOW), 0);
	SendMessage(hwndList, CLM_SETGREYOUTFLAGS, 0, 0);
	SendMessage(hwndList, CLM_SETLEFTMARGIN, 4, 0);
	SendMessage(hwndList, CLM_SETINDENT, 10, 0);
	SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, 1, 0);
	SendMessage(hwndList, CLM_SETHIDEOFFLINEROOT, 1, 0);
	for (int i = 0; i <= FONTID_MAX; ++i)
		SendMessage(hwndList, CLM_SETTEXTCOLOR, i, GetSysColor(COLOR_WINDOWTEXT));
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInitialized = false;
	static MCONTACT hSelectedContact = 0;

	switch (msg) {
	case WM_INITDIALOG:
		bInitialized = false;

		TranslateDialogDefault(hwnd);

		CheckDlgButton(hwnd, IDC_CHK_GROUPS, g_Options.bUseGroups ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_GROUPCOLUMS, g_Options.bUseColumns ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_SECONDLINE, g_Options.bSecondLine ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_AVATARS, g_Options.bAvatars ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_AVATARBORDER, g_Options.bAvatarBorder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_NOTRANSPARENTBORDER, g_Options.bNoTransparentBorder ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_SYSCOLORS, g_Options.bSysColors ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_CENTERHOTKEY, g_Options.bCenterHotkey ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_RIGHTAVATARS, g_Options.bRightAvatars ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CHK_DIMIDLE, g_Options.bDimIdle ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwnd, IDC_TXT_RADIUS, g_Options.wAvatarRadius, FALSE);
		SetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, g_Options.wMaxRecent, FALSE);

		SetWindowLongPtr(GetDlgItem(hwnd, IDC_CLIST), GWL_STYLE,
							  GetWindowLongPtr(GetDlgItem(hwnd, IDC_CLIST), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		SendMessage(GetDlgItem(hwnd, IDC_CLIST), CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
		sttResetListOptions(GetDlgItem(hwnd, IDC_CLIST));

		hSelectedContact = db_find_first();
		{
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
				SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETCHECKMARK,
				SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0),
				db_get_b(hContact, "FavContacts", "IsFavourite", 0));
		}

		bInitialized = true;
		PostMessage(hwnd, WM_APP, 0, 0);
		return TRUE;

	case WM_APP:
		{
			BOOL bGroups = IsDlgButtonChecked(hwnd, IDC_CHK_GROUPS);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_GROUPCOLUMS), bGroups);

			BOOL bAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARS);
			BOOL bBorders = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARBORDER);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_AVATARBORDER), bAvatars);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_RIGHTAVATARS), bAvatars);
			EnableWindow(GetDlgItem(hwnd, IDC_CHK_NOTRANSPARENTBORDER), bAvatars && bBorders);
			EnableWindow(GetDlgItem(hwnd, IDC_TXT_RADIUS), bAvatars && bBorders);
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlID == IDC_CANVAS) {
				MEASUREITEMSTRUCT mis = { 0 };
				DRAWITEMSTRUCT dis = *lpdis;

				FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(COLOR_BTNFACE));
				if (hSelectedContact) {
					Options options;
					options.bSecondLine = IsDlgButtonChecked(hwnd, IDC_CHK_SECONDLINE);
					options.bAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARS);
					options.bAvatarBorder = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARBORDER);
					options.bNoTransparentBorder = IsDlgButtonChecked(hwnd, IDC_CHK_NOTRANSPARENTBORDER);
					options.bSysColors = IsDlgButtonChecked(hwnd, IDC_CHK_SYSCOLORS);
					options.bCenterHotkey = IsDlgButtonChecked(hwnd, IDC_CHK_CENTERHOTKEY);
					options.bRightAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_RIGHTAVATARS);
					options.bDimIdle = IsDlgButtonChecked(hwnd, IDC_CHK_DIMIDLE);
					options.wAvatarRadius = GetDlgItemInt(hwnd, IDC_TXT_RADIUS, NULL, FALSE);
					options.wMaxRecent = GetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, NULL, FALSE);

					mis.CtlID = 0;
					mis.CtlType = ODT_MENU;
					mis.itemData = (DWORD)hSelectedContact;
					sttMeasureItem(&mis, &options);
					dis.rcItem.bottom = dis.rcItem.top + mis.itemHeight;

					dis.CtlID = 0;
					dis.CtlType = ODT_MENU;
					dis.itemData = (DWORD)hSelectedContact;
					sttDrawItem(&dis, &options);

					RECT rc = lpdis->rcItem;
					rc.bottom = rc.top + mis.itemHeight;
					FrameRect(lpdis->hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
				}
				return TRUE;
			}
		}
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHK_SECONDLINE:
		case IDC_CHK_AVATARS:
		case IDC_CHK_AVATARBORDER:
		case IDC_CHK_NOTRANSPARENTBORDER:
		case IDC_CHK_SYSCOLORS:
		case IDC_CHK_CENTERHOTKEY:
		case IDC_CHK_GROUPS:
		case IDC_CHK_GROUPCOLUMS:
		case IDC_CHK_RIGHTAVATARS:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), NULL, NULL, RDW_INVALIDATE);
			PostMessage(hwnd, WM_APP, 0, 0);
			break;

		case IDC_BTN_FONTS:
			{
				OPENOPTIONSDIALOG ood = { sizeof(ood) };
				ood.pszGroup = "Customize";
				ood.pszPage = "Fonts and colors";
				ood.pszTab = NULL;
				Options_Open(&ood);
			}
			break;

		case IDC_TXT_RADIUS:
			if ((HIWORD(wParam) == EN_CHANGE) && bInitialized) {
				RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), NULL, NULL, RDW_INVALIDATE);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_TXT_MAXRECENT:
			if ((HIWORD(wParam) == EN_CHANGE) && bInitialized)
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if ((((LPNMHDR)lParam)->idFrom == 0) && (((LPNMHDR)lParam)->code == PSN_APPLY)) {
			g_Options.bSecondLine = IsDlgButtonChecked(hwnd, IDC_CHK_SECONDLINE);
			g_Options.bAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARS);
			g_Options.bAvatarBorder = IsDlgButtonChecked(hwnd, IDC_CHK_AVATARBORDER);
			g_Options.bNoTransparentBorder = IsDlgButtonChecked(hwnd, IDC_CHK_NOTRANSPARENTBORDER);
			g_Options.bSysColors = IsDlgButtonChecked(hwnd, IDC_CHK_SYSCOLORS);
			g_Options.bCenterHotkey = IsDlgButtonChecked(hwnd, IDC_CHK_CENTERHOTKEY);
			g_Options.bUseGroups = IsDlgButtonChecked(hwnd, IDC_CHK_GROUPS);
			g_Options.bUseColumns = IsDlgButtonChecked(hwnd, IDC_CHK_GROUPCOLUMS);
			g_Options.bRightAvatars = IsDlgButtonChecked(hwnd, IDC_CHK_RIGHTAVATARS);
			g_Options.bDimIdle = IsDlgButtonChecked(hwnd, IDC_CHK_DIMIDLE);
			g_Options.wAvatarRadius = GetDlgItemInt(hwnd, IDC_TXT_RADIUS, NULL, FALSE);
			g_Options.wMaxRecent = GetDlgItemInt(hwnd, IDC_TXT_MAXRECENT, NULL, FALSE);

			sttSaveOptions();

			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				BYTE fav = SendDlgItemMessage(hwnd, IDC_CLIST, CLM_GETCHECKMARK,
														SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0), 0);
				if (fav != db_get_b(hContact, "FavContacts", "IsFavourite", 0))
					db_set_b(hContact, "FavContacts", "IsFavourite", fav);
				if (fav) CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
			}
		}
		else if (((LPNMHDR)lParam)->idFrom == IDC_CLIST) {
			int iSelection;

			switch (((LPNMHDR)lParam)->code) {
			case CLN_OPTIONSCHANGED:
				sttResetListOptions(GetDlgItem(hwnd, IDC_CLIST));
				break;

			case CLN_NEWCONTACT:
				iSelection = (int)((NMCLISTCONTROL *)lParam)->hItem;
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					if (SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0) == iSelection) {
						SendDlgItemMessage(hwnd, IDC_CLIST, CLM_SETCHECKMARK, iSelection,
												 db_get_b(hContact, "FavContacts", "IsFavourite", 0));
						break;
					}
				}
				break;

			case CLN_CHECKCHANGED:
				iSelection = (int)((NMCLISTCONTROL *)lParam)->hItem;
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					if (SendDlgItemMessage(hwnd, IDC_CLIST, CLM_FINDCONTACT, hContact, 0) == iSelection) {
						hSelectedContact = hContact;
						RedrawWindow(GetDlgItem(hwnd, IDC_CANVAS), NULL, NULL, RDW_INVALIDATE);
					}
				}
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
		break;
	}

	return FALSE;
}
