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

PopupFonts fonts = {0};

void InitFonts() {
	// Fonts
	FontIDT fid = {0};
	fid.cbSize = sizeof(FontIDT);
	lstrcpy(fid.group, _T(PU_FNT_AND_COLOR));
	strcpy(fid.dbSettingsGroup, PU_FNT_AND_COLOR_DB);
	fid.flags = FIDF_DEFAULTVALID;
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.size = -11;
	lstrcpy(fid.backgroundGroup,_T(PU_FNT_AND_COLOR));
	lstrcpy(fid.backgroundName,_T(PU_COL_BACK_NAME));
	lstrcpy(fid.deffontsettings.szFace, _T("Tahoma"));

	lstrcpy(fid.name, _T(PU_FNT_NAME_TITLE));
	mir_snprintf(fid.prefix, sizeof(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_TITLE);
	fid.deffontsettings.style  = DBFONTF_BOLD;
	fid.deffontsettings.colour = RGB(0,0,0);
	FontRegisterT(&fid);

	lstrcpy(fid.name, _T(PU_FNT_NAME_CLOCK));
	mir_snprintf(fid.prefix, sizeof(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_CLOCK);
	//fid.deffontsettings.style  = DBFONTF_BOLD;
	//fid.deffontsettings.colour = RGB(0,0,0);
	FontRegisterT(&fid);

	lstrcpy(fid.name, _T(PU_FNT_NAME_TEXT));
	mir_snprintf(fid.prefix, sizeof(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_TEXT);
	fid.deffontsettings.style  = 0;
	//fid.deffontsettings.colour = RGB(0,0,0);
	FontRegisterT(&fid);

	lstrcpy(fid.name, _T("Action"));
	mir_snprintf(fid.prefix, sizeof(fid.prefix), PU_FNT_PREFIX, "Action");
	//fid.deffontsettings.style  = 0;
	fid.deffontsettings.colour = RGB(0,0,255);
	FontRegisterT(&fid);

	lstrcpy(fid.name, _T("Hovered Action"));
	mir_snprintf(fid.prefix, sizeof(fid.prefix), PU_FNT_PREFIX, "Hovered Action");
	fid.deffontsettings.style  = DBFONTF_UNDERLINE;
	//fid.deffontsettings.colour = RGB(0,0,255);
	FontRegisterT(&fid);

	ColourIDT cid = {0};
	cid.cbSize = sizeof(ColourIDT);
	lstrcpy(cid.group, _T(PU_FNT_AND_COLOR));
	strcpy(cid.dbSettingsGroup, PU_FNT_AND_COLOR_DB);

	lstrcpy(cid.name, _T(PU_COL_BACK_NAME));
	strcpy(cid.setting, PU_COL_BACK_SETTING);
	cid.defcolour = SETTING_BACKCOLOUR_DEFAULT;
	ColourRegisterT(&cid);

	lstrcpy(cid.name, _T(PU_COL_AVAT_NAME));
	strcpy(cid.setting, PU_COL_AVAT_SETTING);
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

	LOGFONT lf	= {0};
	FontIDT fid	= {0};
	fid.cbSize	= sizeof(FontIDT);
	lstrcpy(fid.group, _T(PU_FNT_AND_COLOR));

	lstrcpy(fid.name, _T(PU_FNT_NAME_TITLE));
	fonts.clTitle	= (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.title		= CreateFontIndirect(&lf);

	lstrcpy(fid.name, _T(PU_FNT_NAME_CLOCK));
	fonts.clClock	= (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.clock		= CreateFontIndirect(&lf);

	lstrcpy(fid.name, _T(PU_FNT_NAME_TEXT));
	fonts.clText	= (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.text		= CreateFontIndirect(&lf);

	lstrcpy(fid.name, _T("Action"));
	fonts.clAction	= (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.action	= CreateFontIndirect(&lf);

	lstrcpy(fid.name, _T("Hovered Action"));
	fonts.clActionHover	= (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)&lf);
	fonts.actionHover	= CreateFontIndirect(&lf);

	ColourIDT cid = {0};
	cid.cbSize = sizeof(ColourIDT);
	lstrcpy(cid.group, _T(PU_FNT_AND_COLOR));
	lstrcpy(cid.name,  _T(PU_COL_BACK_NAME));
	fonts.clBack = (COLORREF)CallService(MS_COLOUR_GETT, (WPARAM)&cid, (LPARAM)&lf);

	lstrcpy(cid.group, _T(PU_FNT_AND_COLOR));
	lstrcpy(cid.name,  _T(PU_COL_AVAT_NAME));
	fonts.clAvatarBorder = (COLORREF)CallService(MS_COLOUR_GETT, (WPARAM)&cid, (LPARAM)&lf);

	//update class popupps(only temp at this point, must rework)
	char setting[256];
	for(int i=0; i < gTreeData.getCount(); i++) {
		if (gTreeData[i]->typ == 2) {
			mir_snprintf(setting, 256, "%s/TextCol", gTreeData[i]->pupClass.pszName);
			gTreeData[i]->colorText = gTreeData[i]->pupClass.colorText = 
				(COLORREF)DBGetContactSettingDword(0, PU_MODULCLASS, setting, (DWORD)fonts.clText);
			mir_snprintf(setting, 256, "%s/BgCol",   gTreeData[i]->pupClass.pszName);
			gTreeData[i]->colorBack = gTreeData[i]->pupClass.colorBack = 
				(COLORREF)DBGetContactSettingDword(0, PU_MODULCLASS, setting, (DWORD)fonts.clBack/*pc->colorBack*/);
		}
	}
}
