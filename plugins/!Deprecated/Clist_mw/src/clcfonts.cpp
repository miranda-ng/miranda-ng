/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-10 Miranda ICQ/IM project,
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

#include "commonheaders.h"

struct
{
	const TCHAR *tszName;
	int iMask;
}
static clistFontDescr[] =
{
	{ LPGENT("Standard contacts"),                                        FIDF_CLASSGENERAL },
	{ LPGENT("Online contacts to whom you have a different visibility"),  FIDF_CLASSGENERAL },
	{ LPGENT("Offline contacts"),                                         FIDF_CLASSGENERAL },
	{ LPGENT("Contacts which are 'not on list'"),                         FIDF_CLASSGENERAL },
	{ LPGENT("Groups"),                                                   FIDF_CLASSHEADER  },
	{ LPGENT("Group member counts"),                                      FIDF_CLASSHEADER  },
	{ LPGENT("Dividers"),                                                 FIDF_CLASSSMALL   },
	{ LPGENT("Offline contacts to whom you have a different visibility"), FIDF_CLASSGENERAL },
	{ LPGENT("Status messages"),                                          FIDF_CLASSGENERAL },
	{ LPGENT("Group closed"),                                             FIDF_CLASSGENERAL },
	{ LPGENT("Hovered contacts"),                                         FIDF_CLASSGENERAL }
};

#define CLCDEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define CLCDEFAULT_SELTEXTCOLOUR GetSysColor(COLOR_HIGHLIGHTTEXT)

static int FS_FontsChanged(WPARAM wParam, LPARAM lParam)
{
	pcli->pfnClcOptionsChanged();
	return 0;
}

void RegisterCListFonts()
{
	FontIDT fontid = { sizeof(fontid) };
	fontid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWREREGISTER | FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS;
	strncpy(fontid.dbSettingsGroup, "CLC", sizeof(fontid.dbSettingsGroup));
	_tcsncpy(fontid.group, _T("Contact list"), SIZEOF(fontid.group));

	HDC hdc = GetDC(NULL);
	for (int i = 0; i < SIZEOF(clistFontDescr); i++) {
		LOGFONT lf;
		pcli->pfnGetFontSetting(i, &lf, &fontid.deffontsettings.colour);
		lf.lfHeight = -MulDiv(lf.lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);

		_tcsncpy_s(fontid.deffontsettings.szFace, lf.lfFaceName, _TRUNCATE);
		fontid.deffontsettings.charset = lf.lfCharSet;
		fontid.deffontsettings.size = (char)lf.lfHeight;
		fontid.deffontsettings.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0);

		fontid.flags &= ~FIDF_CLASSMASK;
		fontid.flags |= clistFontDescr[i].iMask;

		_tcsncpy(fontid.name, clistFontDescr[i].tszName, SIZEOF(fontid.name));

		char idstr[10];
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", i);
		strncpy(fontid.prefix, idstr, SIZEOF(fontid.prefix));
		fontid.order = i;
		FontRegisterT(&fontid);
	}
	ReleaseDC(NULL, hdc);

	// and colours
	ColourIDT colourid = { sizeof(colourid) };
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

	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
}
