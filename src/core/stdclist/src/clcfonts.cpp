/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
	pcli->pfnClcOptionsChanged();
	return 0;
}

void RegisterCListFonts()
{
	FontIDW fontid = { sizeof(fontid) };
	fontid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWREREGISTER | FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS;
	strncpy(fontid.dbSettingsGroup, "CLC", sizeof(fontid.dbSettingsGroup));
	wcsncpy(fontid.group, LPGENW("Contact list"), _countof(fontid.group));

	HDC hdc = GetDC(NULL);
	for (int i = 0; i < _countof(clistFontDescr); i++) {
		LOGFONT lf;
		pcli->pfnGetFontSetting(i, &lf, &fontid.deffontsettings.colour);
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

		wcsncpy_s(fontid.deffontsettings.szFace, lf.lfFaceName, _TRUNCATE);
		fontid.deffontsettings.charset = lf.lfCharSet;
		fontid.deffontsettings.size = (char)lf.lfHeight;
		fontid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0);

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= clistFontDescr[i].iMask;

		wcsncpy(fontid.name, clistFontDescr[i].tszName, _countof(fontid.name));

		char idstr[10];
		mir_snprintf(idstr, "Font%d", i);
		strncpy(fontid.prefix, idstr, _countof(fontid.prefix));
		fontid.order = i;
		Font_RegisterW(&fontid);
	}
	ReleaseDC(NULL, hdc);

	// and colours
	ColourIDW colourid = { 0 };
	colourid.cbSize = sizeof(ColourIDW);
	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "CLC", sizeof(colourid.dbSettingsGroup));

	strncpy(colourid.setting, "BkColour", sizeof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Background"), _countof(colourid.name));
	wcsncpy(colourid.group, LPGENW("Contact list"), _countof(colourid.group));
	colourid.defcolour = CLCDEFAULT_BKCOLOUR;
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "SelTextColour", sizeof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Selected text"), _countof(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_SELTEXTCOLOUR;
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "HotTextColour", sizeof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Hottrack text"), _countof(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_HOTTEXTCOLOUR;
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "QuickSearchColour", sizeof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Quicksearch text"), _countof(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_QUICKSEARCHCOLOUR;
	Colour_RegisterW(&colourid);

	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
}

void LoadClcOptions(HWND hwnd, struct ClcData *dat, BOOL bFirst)
{
	HDC hdc = GetDC(hwnd);
	for (int i = 0; i <= FONTID_MAX; i++) {
		if (!dat->fontInfo[i].changed)
			DeleteObject(dat->fontInfo[i].hFont);

		LOGFONT lf;
		pcli->pfnGetFontSetting(i, &lf, &dat->fontInfo[i].colour);
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
