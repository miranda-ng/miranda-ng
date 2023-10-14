/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

MyColourID g_colorTable[COLOR_COUNT] =
{
	{ LPGEN("Incoming name"),              "ColorNickIn",    RGB(0xc8, 0x3f, 0x6b) },
	{ LPGEN("Outgoing name"),              "ColorNickOut",   RGB(0x08, 0x60, 0xbd) },
																			   
	{ LPGEN("Incoming messages"),          "ColorMsgIn",     RGB(0xd6, 0xf5, 0xc0) },
	{ LPGEN("Outgoing messages"),          "ColorMsgOut",    RGB(0xf5, 0xe7, 0xd8) },
																			   
	{ LPGEN("Incoming files"),             "ColorFileIn",    RGB(0xe3, 0xee, 0x9b) },
	{ LPGEN("Outgoing files"),             "ColorFileOut",   RGB(0xe3, 0xee, 0x9b) },
																			   
	{ LPGEN("Status changes"),             "ColorStatus",    RGB(0xf0, 0xf0, 0xf0) },
																			   
	{ LPGEN("Other incoming events"),      "ColorIn",        RGB(0xff, 0xff, 0xff) },
	{ LPGEN("Other outgoing events"),      "ColorOut",       RGB(0xff, 0xff, 0xff) },
																			   
	{ LPGEN("Selected item's text"),       "ColorSelTxt",    RGB(0xff, 0xff, 0xff) },
	{ LPGEN("Selected item's background"), "ColorSel",       GetSysColor(COLOR_HIGHLIGHT) },
	{ LPGEN("Selected item's frame"),      "ColorSelFrm",    GetSysColor(COLOR_HIGHLIGHTTEXT) },

	{ LPGEN("Highlighted messages"),                  "ColorHighlight", RGB(0xf0, 0xf0, 0xf0) },
	{ LPGEN("Grid background"),            "Background",     RGB(0xff, 0xff, 0xff) },
	{ LPGEN("Separator"),                  "Separator",      RGB(0x60, 0x60, 0x60) },
};

MyFontID g_fontTable[FONT_COUNT] =
{
	{ LPGEN("Incoming messages"),     "FontMsgIn"     },
	{ LPGEN("Outgoing messages"),     "FontMsgOut"    },
																     
	{ LPGEN("Incoming files"),        "FontFileIn"    },
	{ LPGEN("Outgoing files"),        "FontFileOut"   },

	{ LPGEN("Status changes"),        "FontStatus"    },
	{ LPGEN("Highlighted messages"),             "FontHighlight", DBFONTF_BOLD, RGB(0x7f, 0, 0) },

	{ LPGEN("Other incoming events"), "FontIn"        },
	{ LPGEN("Other outgoing events"), "FontOut"       },
};

int evtFontsChanged(WPARAM, LPARAM)
{
	for (auto &it : g_colorTable)
		it.cl = Colour_Get(MODULENAME, it.szName);

	DeleteObject(g_plugin.hBackBrush);
	g_plugin.hBackBrush = CreateSolidBrush(g_colorTable[COLOR_SELBACK].cl);

	for (auto &it : g_fontTable) {
		it.cl = (COLORREF)Font_Get(MODULENAME, it.szName, &it.lf);

		DeleteObject(it.hfnt);
		it.hfnt = CreateFontIndirectA(&it.lf);
	}

	WindowList_Broadcast(g_hNewstoryWindows, UM_REDRAWLISTH, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitFonts()
{
	HookEvent(ME_FONT_RELOAD, evtFontsChanged);
	HookEvent(ME_COLOUR_RELOAD, evtFontsChanged);

	ColourID cid = {};
	strncpy_s(cid.group, MODULENAME, _TRUNCATE);
	strncpy_s(cid.dbSettingsGroup, MODULENAME, _TRUNCATE);

	for (auto &it : g_colorTable) {
		cid.order = int(&it - g_colorTable);
		strncpy_s(cid.name, it.szName, _TRUNCATE);
		strncpy_s(cid.setting, it.szSetting, _TRUNCATE);
		cid.defcolour = it.defaultValue;
		g_plugin.addColor(&cid);
		it.cl = Colour_Get(cid.group, cid.name);
	}

	g_plugin.hBackBrush = CreateSolidBrush(g_colorTable[COLOR_SELBACK].cl);

	LOGFONT lfDefault;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfDefault), &lfDefault, FALSE);

	FontID fontid = {};
	fontid.flags = FIDF_DEFAULTVALID;
	strncpy_s(fontid.group, MODULENAME, _TRUNCATE);
	strncpy_s(fontid.dbSettingsGroup, MODULENAME, _TRUNCATE);
	strncpy_s(fontid.deffontsettings.szFace, _T2A(lfDefault.lfFaceName), _TRUNCATE);
	fontid.deffontsettings.size = -12;

	for (auto &it : g_fontTable) {
		fontid.order = int(&it - g_fontTable);
		strncpy_s(fontid.name, it.szName, _TRUNCATE);
		strncpy_s(fontid.setting, it.szSetting, _TRUNCATE);
		fontid.deffontsettings.style = it.style;
		fontid.deffontsettings.colour = it.defaultValue;
		g_plugin.addFont(&fontid);

		it.cl = Font_Get(MODULENAME, it.szName, &it.lf);
		it.hfnt = CreateFontIndirectA(&it.lf);
	}
}

void DestroyFonts()
{
	DeleteObject(g_plugin.hBackBrush);

	for (auto &it : g_fontTable)
		DeleteObject(it.hfnt);
}
