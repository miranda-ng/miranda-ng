#include "stdafx.h"

MyColourID g_colorTable[COLOR_COUNT] =
{
	{ LPGEN("Incoming name"),              "ColorNickIn",  RGB(0xc8, 0x3f, 0x6b) },
	{ LPGEN("Outgoing name"),              "ColorNickOut", RGB(0x08, 0x60, 0xbd) },

	{ LPGEN("Incoming messages"),          "ColorMsgIn",   RGB(0xd6, 0xf5, 0xc0) },
	{ LPGEN("Outgoing messages"),          "ColorMsgOut",  RGB(0xf5, 0xe7, 0xd8) },
																			  
	{ LPGEN("Incoming files"),             "ColorFileIn",  RGB(0xe3, 0xee, 0x9b) },
	{ LPGEN("Outgoing files"),             "ColorFileOut", RGB(0xe3, 0xee, 0x9b) },
																			  
	{ LPGEN("Status changes"),             "ColorStatus",  RGB(0xf0, 0xf0, 0xf0) },
																			  
	{ LPGEN("Other incoming events"),      "ColorIn",      RGB(0xff, 0xff, 0xff) },
	{ LPGEN("Other outgoing events"),      "ColorOut",     RGB(0xff, 0xff, 0xff) },
																			  
	{ LPGEN("Selected item's text"),       "ColorSelTxt",  RGB(0xff, 0xff, 0xff) },
	{ LPGEN("Selected item's background"), "ColorSel",     GetSysColor(COLOR_HIGHLIGHT) },
	{ LPGEN("Selected item's frame"),      "ColorSelFrm",  GetSysColor(COLOR_HIGHLIGHTTEXT) },
																			  
	{ LPGEN("Grid background"),            "Background",   RGB(0xff, 0xff, 0xff) },
	{ LPGEN("Separator"),                  "Separator",    RGB(0x60, 0x60, 0x60) },
};

MyFontID g_fontTable[FONT_COUNT] =
{
	{ LPGEN("Incoming messages"),     "FontMsgIn"   },
	{ LPGEN("Outgoing messages"),     "FontMsgOut"  },

	{ LPGEN("Incoming files"),        "FontFileIn"  },
	{ LPGEN("Outgoing files"),        "FontFileOut" },

	{ LPGEN("Status changes"),        "FontStatus"  },

	{ LPGEN("Other incoming events"), "FontIn"      },
	{ LPGEN("Other outgoing events"), "FontOut"     }
};

int evtFontsChanged(WPARAM, LPARAM)
{
	for (auto &it : g_colorTable)
		it.cl = Colour_Get(MODULENAME, it.szName);

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
		fontid.deffontsettings.colour = it.defaultValue;
		g_plugin.addFont(&fontid);

		it.cl = Font_Get(MODULENAME, it.szName, &it.lf);
		it.hfnt = CreateFontIndirectA(&it.lf);
	}
}

void DestroyFonts()
{
	for (auto &it : g_fontTable)
		DeleteObject(it.hfnt);
}
