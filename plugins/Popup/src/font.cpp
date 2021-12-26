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

#include "stdafx.h"

PopupFonts fonts = {};

void InitFonts()
{
	// Fonts
	FontIDW fid = {};
	mir_wstrncpy(fid.group, _A2W(PU_FNT_AND_COLOR), _countof(fid.group));
	mir_strncpy(fid.dbSettingsGroup, PU_FNT_AND_COLOR_DB, _countof(fid.dbSettingsGroup));
	fid.flags = FIDF_DEFAULTVALID;
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.size = -11;
	mir_wstrncpy(fid.backgroundGroup, _A2W(PU_FNT_AND_COLOR), _countof(fid.backgroundGroup));
	mir_wstrncpy(fid.backgroundName, PU_COL_BACK_NAME, _countof(fid.backgroundName));
	mir_wstrncpy(fid.deffontsettings.szFace, L"Tahoma", _countof(fid.deffontsettings.szFace));

	mir_wstrncpy(fid.name, _A2W(PU_FNT_NAME_TITLE), _countof(fid.name));
	mir_snprintf(fid.setting, PU_FNT_PREFIX, PU_FNT_NAME_TITLE);
	fid.deffontsettings.style = DBFONTF_BOLD;
	fid.deffontsettings.colour = RGB(0, 0, 0);
	g_plugin.addFont(&fid);

	mir_wstrncpy(fid.name, _A2W(PU_FNT_NAME_CLOCK), _countof(fid.name));
	mir_snprintf(fid.setting, PU_FNT_PREFIX, PU_FNT_NAME_CLOCK);
	g_plugin.addFont(&fid);

	mir_wstrncpy(fid.name, _A2W(PU_FNT_NAME_TEXT), _countof(fid.name));
	mir_snprintf(fid.setting, PU_FNT_PREFIX, PU_FNT_NAME_TEXT);
	fid.deffontsettings.style = 0;
	g_plugin.addFont(&fid);

	mir_wstrncpy(fid.name, _A2W(PU_FNT_NAME_ACTION), _countof(fid.name));
	mir_snprintf(fid.setting, PU_FNT_PREFIX, PU_FNT_NAME_ACTION);
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
	fid.deffontsettings.colour = RGB(0, 0, 255);
	g_plugin.addFont(&fid);

	mir_wstrncpy(fid.name, _A2W(PU_FNT_NAME_HOVERED_ACTION), _countof(fid.name));
	mir_snprintf(fid.setting, PU_FNT_PREFIX, PU_FNT_NAME_HOVERED_ACTION);
	fid.deffontsettings.style = DBFONTF_UNDERLINE;
	g_plugin.addFont(&fid);

	ColourIDW cid = {};
	mir_wstrncpy(cid.group, _A2W(PU_FNT_AND_COLOR), _countof(cid.group));
	mir_strncpy(cid.dbSettingsGroup, PU_FNT_AND_COLOR_DB, _countof(cid.dbSettingsGroup));

	mir_wstrncpy(cid.name, PU_COL_BACK_NAME, _countof(cid.name));
	mir_strncpy(cid.setting, PU_COL_BACK_SETTING, _countof(cid.setting));
	cid.defcolour = SETTING_BACKCOLOUR_DEFAULT;
	g_plugin.addColor(&cid);

	mir_wstrncpy(cid.name, PU_COL_AVAT_NAME, _countof(cid.name));
	mir_strncpy(cid.setting, PU_COL_AVAT_SETTING, _countof(cid.setting));
	cid.defcolour = SETTING_TEXTCOLOUR_DEFAULT;
	g_plugin.addColor(&cid);

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
	fonts.clTitle = Font_GetW(_A2W(PU_FNT_AND_COLOR), _A2W(PU_FNT_NAME_TITLE), &lf);
	fonts.title = CreateFontIndirect(&lf);

	fonts.clClock = Font_GetW(_A2W(PU_FNT_AND_COLOR), _A2W(PU_FNT_NAME_CLOCK), &lf);
	fonts.clock = CreateFontIndirect(&lf);

	fonts.clText = Font_GetW(_A2W(PU_FNT_AND_COLOR), _A2W(PU_FNT_NAME_TEXT), &lf);
	fonts.text = CreateFontIndirect(&lf);

	fonts.clAction = Font_GetW(_A2W(PU_FNT_AND_COLOR), _A2W(PU_FNT_NAME_ACTION), &lf);
	fonts.action = CreateFontIndirect(&lf);

	fonts.clActionHover = Font_GetW(_A2W(PU_FNT_AND_COLOR), _A2W(PU_FNT_NAME_HOVERED_ACTION), &lf);
	fonts.actionHover = CreateFontIndirect(&lf);

	fonts.clBack = Colour_GetW(_A2W(PU_FNT_AND_COLOR), PU_COL_BACK_NAME);
	fonts.clAvatarBorder = Colour_GetW(_A2W(PU_FNT_AND_COLOR), PU_COL_AVAT_NAME);

	// update class popups (only temp at this point, must rework)
	char setting[256];
	for (auto &it : gTreeData) {
		if (it->typ == 2) {
			mir_snprintf(setting, "%s/TextCol", it->pupClass.pszName);
			it->colorText = it->pupClass.colorText = (COLORREF)db_get_dw(0, PU_MODULCLASS, setting, (uint32_t)fonts.clText);
			
			mir_snprintf(setting, "%s/BgCol", it->pupClass.pszName);
			it->colorBack = it->pupClass.colorBack = (COLORREF)db_get_dw(0, PU_MODULCLASS, setting, (uint32_t)fonts.clBack/*pc->colorBack*/);
		}
	}
}
