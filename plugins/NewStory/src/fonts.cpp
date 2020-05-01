#include "stdafx.h"

MyColourID g_colorTable[COLOR_COUNT] =
{
	{ "Incoming messages",     "ColorMsgIn",   RGB(0xd6, 0xf5, 0xc0) },
	{ "Outgoing messages",     "ColorMsgOut",  RGB(0xf5, 0xe7, 0xd8) },

	{ "Incoming files",        "ColorFileIn",  RGB(0xe3, 0xee, 0x9b) },
	{ "Outgoing files",        "ColorFileOut", RGB(0xe3, 0xee, 0x9b) },

	{ "Status changes",        "ColorStatus",  RGB(0xf0, 0xf0, 0xf0) },

	{ "Other incoming events", "ColorIn",      RGB(0xff, 0xff, 0xff) },
	{ "Other outgoing events", "ColorOut",     RGB(0xff, 0xff, 0xff) },

	{ "Selected items",        "ColorSel",     RGB(0x60, 0x60, 0x60) },
	{ "Selected items (text)", "ColorSelTxt",  RGB(0xff, 0xff, 0xff) }
};

MyFontID g_fontTable[FONT_COUNT] =
{
	{ "Incoming nick",     "FontNickIn", RGB(0xc8, 0x3f, 0x6b)    },
	{ "Outgoing nick",     "FontNickOut", RGB(0x08, 0x60, 0xbd)  },

	{ "Incoming messages",     "FontMsgIn"   },
	{ "Outgoing messages",     "FontMsgOut"  },

	{ "Incoming files",        "FontFileIn"  },
	{ "Outgoing files",        "FontFileOut" },

	{ "Status changes",        "FontStatus"  },

	{ "Other incoming events", "FontIn"      },
	{ "Other outgoing events", "FontOut"     }
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

	WindowList_Broadcast(hNewstoryWindows, UM_REDRAWLISTH, 0, 0);
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

	FontID fontid = {};
	strncpy_s(fontid.group, MODULENAME, _TRUNCATE);
	strncpy_s(fontid.dbSettingsGroup, MODULENAME, _TRUNCATE);

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
