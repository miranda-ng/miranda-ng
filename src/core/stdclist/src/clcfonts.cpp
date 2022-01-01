/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "m_fontservice.h"

struct
{
	const wchar_t *tszName;
	int iMask;
}
static clistFontDescr[] =
{
	{ LPGENW("Standard contacts"),                                        FIDF_CLASSGENERAL },
	{ LPGENW("Online contacts to whom you have a different visibility"),  FIDF_CLASSGENERAL },
	{ LPGENW("Offline contacts"),                                         FIDF_CLASSGENERAL },
	{ LPGENW("Contacts which are 'not on list'"),                         FIDF_CLASSGENERAL },
	{ LPGENW("Groups"),                                                   FIDF_CLASSHEADER  },
	{ LPGENW("Group member counts"),                                      FIDF_CLASSHEADER  },
	{ LPGENW("Dividers"),                                                 FIDF_CLASSSMALL   },
	{ LPGENW("Offline contacts to whom you have a different visibility"), FIDF_CLASSGENERAL }
};

static int FS_FontsChanged(WPARAM, LPARAM)
{
	Clist_ClcOptionsChanged();
	return 0;
}

void RegisterCListFonts()
{
	FontIDW fontid = {};
	fontid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWREREGISTER | FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS;
	strncpy_s(fontid.dbSettingsGroup, "CLC", _TRUNCATE);
	wcsncpy_s(fontid.group, LPGENW("Contact list"), _TRUNCATE);

	HDC hdc = GetDC(nullptr);
	for (int i = 0; i < _countof(clistFontDescr); i++) {
		LOGFONT lf;
		Clist_GetFontSetting(i, &lf, &fontid.deffontsettings.colour);
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

		wcsncpy_s(fontid.deffontsettings.szFace, lf.lfFaceName, _TRUNCATE);
		fontid.deffontsettings.charset = lf.lfCharSet;
		fontid.deffontsettings.size = (char)lf.lfHeight;
		fontid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0);

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= clistFontDescr[i].iMask;

		wcsncpy_s(fontid.name, clistFontDescr[i].tszName, _TRUNCATE);

		mir_snprintf(fontid.setting, "Font%d", i);
		fontid.order = i;
		g_plugin.addFont(&fontid);
	}
	ReleaseDC(nullptr, hdc);

	// and colours
	ColourIDW colourid = {};
	strncpy_s(colourid.dbSettingsGroup, "CLC", _TRUNCATE);

	strncpy_s(colourid.setting, "BkColour", _TRUNCATE);
	wcsncpy_s(colourid.name, LPGENW("Background"), _TRUNCATE);
	wcsncpy_s(colourid.group, LPGENW("Contact list"), _TRUNCATE);
	colourid.defcolour = CLCDEFAULT_BKCOLOUR;
	g_plugin.addColor(&colourid);

	strncpy_s(colourid.setting, "SelTextColour", _TRUNCATE);
	wcsncpy_s(colourid.name, LPGENW("Selected text"), _TRUNCATE);
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_SELTEXTCOLOUR;
	g_plugin.addColor(&colourid);

	strncpy_s(colourid.setting, "HotTextColour", _TRUNCATE);
	wcsncpy_s(colourid.name, LPGENW("Hottrack text"), _TRUNCATE);
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_HOTTEXTCOLOUR;
	g_plugin.addColor(&colourid);

	strncpy_s(colourid.setting, "QuickSearchColour", _TRUNCATE);
	wcsncpy_s(colourid.name, LPGENW("Quicksearch text"), _TRUNCATE);
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_QUICKSEARCHCOLOUR;
	g_plugin.addColor(&colourid);

	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
}

void LoadClcOptions(HWND hwnd, struct ClcData *dat, BOOL bFirst)
{
	HDC hdc = GetDC(hwnd);
	for (int i = 0; i <= FONTID_MAX; i++) {
		if (!dat->fontInfo[i].changed)
			DeleteObject(dat->fontInfo[i].hFont);

		LOGFONT lf;
		Clist_GetFontSetting(i, &lf, &dat->fontInfo[i].colour);
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

		dat->fontInfo[i].hFont = CreateFontIndirect(&lf);
		dat->fontInfo[i].changed = 0;

		HFONT holdfont = (HFONT)SelectObject(hdc, dat->fontInfo[i].hFont);
		SIZE fontSize;
		GetTextExtentPoint32(hdc, L"x", 1, &fontSize);
		SelectObject(hdc, holdfont);

		dat->fontInfo[i].fontHeight = fontSize.cy;
	}
	ReleaseDC(hwnd, hdc);

	coreCli.pfnLoadClcOptions(hwnd, dat, bFirst);
}
