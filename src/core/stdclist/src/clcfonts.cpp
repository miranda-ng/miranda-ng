/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-12 Miranda IM, 2012-13 Miranda NG project,
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

#include "m_fontservice.h"

#define CLIST_FONTID_CONTACTS    0
#define CLIST_FONTID_INVIS       1
#define CLIST_FONTID_OFFLINE     2
#define CLIST_FONTID_NOTONLIST   3
#define CLIST_FONTID_GROUPS      4
#define CLIST_FONTID_GROUPCOUNTS 5
#define CLIST_FONTID_DIVIDERS    6
#define CLIST_FONTID_OFFINVIS    7
#define CLIST_FONTID_MAX         7

static const TCHAR* szClistFontIdDescr[CLIST_FONTID_MAX + 1] =
{
	LPGENT( "Standard contacts"),
	LPGENT( "Online contacts to whom you have a different visibility"),
	LPGENT( "Offline contacts"),
	LPGENT( "Contacts which are 'not on list'"),
	LPGENT( "Groups"),
	LPGENT( "Group member counts"),
	LPGENT( "Dividers"),
	LPGENT( "Offline contacts to whom you have a different visibility" )
};

static int fontListOrder[CLIST_FONTID_MAX + 1] =
{
	CLIST_FONTID_CONTACTS,
	CLIST_FONTID_INVIS,
	CLIST_FONTID_OFFLINE,
	CLIST_FONTID_OFFINVIS,
	CLIST_FONTID_NOTONLIST,
	CLIST_FONTID_GROUPS,
	CLIST_FONTID_GROUPCOUNTS,
	CLIST_FONTID_DIVIDERS
};

static int FS_FontsChanged(WPARAM wParam, LPARAM lParam)
{
	pcli->pfnClcOptionsChanged();
	return 0;
}

void RegisterCListFonts()
{
	FontIDT fontid = {0};
	ColourIDT colourid = {0};
	char idstr[10];
	int i;

	fontid.cbSize = sizeof(FontIDT);
	fontid.flags = FIDF_ALLOWREREGISTER | FIDF_APPENDNAME | FIDF_NOAS | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS;

	for (i = 0; i <= CLIST_FONTID_MAX; i++) {

		switch (fontListOrder[i])
		{
		case FONTID_GROUPS:
		case FONTID_GROUPCOUNTS:
			fontid.flags &= ~FIDF_CLASSMASK;
			fontid.flags |= FIDF_CLASSHEADER;
			break;
		case FONTID_DIVIDERS:
			fontid.flags &= ~FIDF_CLASSMASK;
			fontid.flags |= FIDF_CLASSSMALL;
			break;
		default:
			fontid.flags &= ~FIDF_CLASSMASK;
			fontid.flags |= FIDF_CLASSGENERAL;
			break;
		}

		strncpy(fontid.dbSettingsGroup, "CLC", sizeof(fontid.dbSettingsGroup));
		_tcsncpy(fontid.group, _T("Contact List"), SIZEOF(fontid.group));
		_tcsncpy(fontid.name, szClistFontIdDescr[fontListOrder[i]], SIZEOF(fontid.name));
		mir_snprintf(idstr, SIZEOF(idstr), "Font%d", fontListOrder[i]);
		strncpy(fontid.prefix, idstr, SIZEOF(fontid.prefix));
		fontid.order = fontListOrder[i];

		FontRegisterT(&fontid);
	}

	// and colours
	colourid.cbSize = sizeof(ColourIDT);
	colourid.order = 0;
	strncpy(colourid.dbSettingsGroup, "CLC", sizeof(colourid.dbSettingsGroup));

	strncpy(colourid.setting, "BkColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Background"), SIZEOF(colourid.name));
	_tcsncpy(colourid.group, LPGENT("Contact List"), SIZEOF(colourid.group));
	colourid.defcolour = CLCDEFAULT_BKCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "SelTextColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Selected Text"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_SELTEXTCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "HotTextColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Hottrack Text"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_HOTTEXTCOLOUR;
	ColourRegisterT(&colourid);

	strncpy(colourid.setting, "QuickSearchColour", sizeof(colourid.setting));
	_tcsncpy(colourid.name, LPGENT("Quicksearch Text"), SIZEOF(colourid.name));
	colourid.order = 1;
	colourid.defcolour = CLCDEFAULT_QUICKSEARCHCOLOUR;
	ColourRegisterT(&colourid);

	HookEvent(ME_FONT_RELOAD, FS_FontsChanged);
}
