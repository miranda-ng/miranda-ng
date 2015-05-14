/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

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

PopupFonts fonts = { 0 };

void InitFonts() {
	// Fonts
	FontIDT fid = { 0 };
	fid.cbSize = sizeof(FontIDT);
	mir_tstrncpy(fid.group, _T(PU_FNT_AND_COLOR), SIZEOF(fid.group));
	mir_strncpy(fid.dbSettingsGroup, PU_FNT_AND_COLOR_DB, SIZEOF(fid.dbSettingsGroup));
	fid.flags = FIDF_DEFAULTVALID;
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.size = -11;
	mir_tstrncpy(fid.backgroundGroup, _T(PU_FNT_AND_COLOR), SIZEOF(fid.backgroundGroup));
	mir_tstrncpy(fid.backgroundName, PU_COL_BACK_NAME, SIZEOF(fid.backgroundName));
	mir_tstrncpy(fid.deffontsettings.szFace, _T("Tahoma"), SIZEOF(fid.deffontsettings.szFace));

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_TITLE), SIZEOF(fid.name));
	mir_snprintf(fid.prefix, SIZEOF(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_TITLE);
	fid.deffontsettings.style = DBFONTF_BOLD;
	fid.deffontsettings.colour = RGB(0, 0, 0);
	FontRegisterT(&fid);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_CLOCK), SIZEOF(fid.name));
	mir_snprintf(fid.prefix, SIZEOF(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_CLOCK);
	FontRegisterT(&fid);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_TEXT), SIZEOF(fid.name));
	mir_snprintf(fid.prefix, SIZEOF(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_TEXT);
	fid.deffontsettings.style = 0;
	FontRegisterT(&fid);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_ACTION), SIZEOF(fid.name));
	mir_snprintf(fid.prefix, SIZEOF(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_ACTION);
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
	fid.deffontsettings.colour = RGB(0, 0, 255);
	FontRegisterT(&fid);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_HOVERED_ACTION), SIZEOF(fid.name));
	mir_snprintf(fid.prefix, SIZEOF(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_HOVERED_ACTION);
	fid.deffontsettings.style = DBFONTF_UNDERLINE;
	FontRegisterT(&fid);

	ColourIDT cid = { 0 };
	cid.cbSize = sizeof(ColourIDT);
	mir_tstrncpy(cid.group, _T(PU_FNT_AND_COLOR), SIZEOF(cid.group));
	mir_strncpy(cid.dbSettingsGroup, PU_FNT_AND_COLOR_DB, SIZEOF(cid.dbSettingsGroup));

	mir_tstrncpy(cid.name, PU_COL_BACK_NAME, SIZEOF(cid.name));
	mir_strncpy(cid.setting, PU_COL_BACK_SETTING, SIZEOF(cid.setting));
	cid.defcolour = SETTING_BACKCOLOUR_DEFAULT;
	ColourRegisterT(&cid);

	mir_tstrncpy(cid.name, PU_COL_AVAT_NAME, SIZEOF(cid.name));
	mir_strncpy(cid.setting, PU_COL_AVAT_SETTING, SIZEOF(cid.setting));
	cid.defcolour = SETTING_TEXTCOLOUR_DEFAULT;
	ColourRegisterT(&cid);

	ReloadFonts();
}


void ReloadFonts()
{
	if (fonts.title)		DeleteObject(fonts.title);
	if (fonts.clock)		DeleteObject(fonts.clock);
	if (fonts.text)			DeleteObject(fonts.text);
	if (fonts.action)		DeleteObject(fonts.action);
	if (fonts.actionHover)	DeleteObject(fonts.actionHover);

	LOGFONT lf = { 0 };
	FontIDT fid = { 0 };
	fid.cbSize = sizeof(FontIDT);
	mir_tstrncpy(fid.group, _T(PU_FNT_AND_COLOR), SIZEOF(fid.name));

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_TITLE), SIZEOF(fid.name));
	fonts.clTitle = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.title = CreateFontIndirect(&lf);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_CLOCK), SIZEOF(fid.name));
	fonts.clClock = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.clock = CreateFontIndirect(&lf);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_TEXT), SIZEOF(fid.name));
	fonts.clText = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.text = CreateFontIndirect(&lf);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_ACTION), SIZEOF(fid.name));
	fonts.clAction = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.action = CreateFontIndirect(&lf);

	mir_tstrncpy(fid.name, _T(PU_FNT_NAME_HOVERED_ACTION), SIZEOF(fid.name));
	fonts.clActionHover = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.actionHover = CreateFontIndirect(&lf);

	ColourIDT cid = { 0 };
	cid.cbSize = sizeof(ColourIDT);
	mir_tstrncpy(cid.group, _T(PU_FNT_AND_COLOR), SIZEOF(cid.group));
	mir_tstrncpy(cid.name, PU_COL_BACK_NAME, SIZEOF(cid.name));
	fonts.clBack = (COLORREF)CallService(MS_COLOUR_GETT, (WPARAM)&cid, (LPARAM)&lf);

	mir_tstrncpy(cid.group, _T(PU_FNT_AND_COLOR), SIZEOF(cid.group));
	mir_tstrncpy(cid.name, PU_COL_AVAT_NAME, SIZEOF(cid.name));
	fonts.clAvatarBorder = (COLORREF)CallService(MS_COLOUR_GETT, (WPARAM)&cid, (LPARAM)&lf);

	// update class popupps(only temp at this point, must rework)
	char setting[256];
	for (int i = 0; i < gTreeData.getCount(); i++) {
		if (gTreeData[i]->typ == 2) {
			mir_snprintf(setting, SIZEOF(setting), "%s/TextCol", gTreeData[i]->pupClass.pszName);
			gTreeData[i]->colorText = gTreeData[i]->pupClass.colorText =
				(COLORREF)db_get_dw(0, PU_MODULCLASS, setting, (DWORD)fonts.clText);
			mir_snprintf(setting, SIZEOF(setting), "%s/BgCol", gTreeData[i]->pupClass.pszName);
			gTreeData[i]->colorBack = gTreeData[i]->pupClass.colorBack =
				(COLORREF)db_get_dw(0, PU_MODULCLASS, setting, (DWORD)fonts.clBack/*pc->colorBack*/);
		}
	}
}
