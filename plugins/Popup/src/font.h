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

#ifndef __font_h__
#define __font_h__

//basic constants for all popup plugins
#define PU_FNT_AND_COLOR		LPGEN("Popups")			//common main group for customice\font&color
#define PU_FNT_AND_COLOR_DB		MODULNAME		//use eg strcpy(fid.dbSettingsGroup, PU_FNT_GROUP_DB);

#define PU_FNT_PREFIX			"fnt%s"				//use eg mir_snprintf(fid.prefix, sizeof(fid.prefix), PU_FNT_PREFIX, PU_FNT_NAME_....);
#define PU_FNT_NAME_TITLE		LPGEN("Title")				//use eg lstrcpy(fid.name, _T(FNT_NAME_....)) for FontIDT
#define PU_FNT_NAME_CLOCK		LPGEN("Clock")
#define PU_FNT_NAME_TEXT		LPGEN("Text")
#define PU_FNT_NAME_ACTION		LPGEN("Action")
#define PU_FNT_NAME_HOVERED_ACTION LPGEN("Hovered action")

#define PU_COL_BACK_NAME		LPGENT("Background")
#define PU_COL_BACK_SETTING		"ColourBg"
#define PU_COL_BORD_NAME		LPGENT("Border")
#define PU_COL_BORD_SETTING		"ColourBorder"
#define PU_COL_SIDE_NAME		LPGENT("Sidebar")
#define PU_COL_SIDE_SETTING		"ColourSidebar"
#define PU_COL_LINE_NAME		LPGENT("Title underline")
#define PU_COL_LINE_SETTING		"ColourUnderline"
#define PU_COL_AVAT_NAME		LPGENT("Avatar border")
#define PU_COL_AVAT_SETTING		"ColourAvatarBorder"

struct PopupFonts
{
	HFONT title;
	HFONT text;
	HFONT clock;
	HFONT action;
	HFONT actionHover;

	COLORREF clTitle;
	COLORREF clText;
	COLORREF clClock;
	COLORREF clBack;
	COLORREF clAction;
	COLORREF clActionHover;
	COLORREF clAvatarBorder;

};

extern PopupFonts fonts;

void InitFonts();

void ReloadFonts();


#endif // __font_h__
