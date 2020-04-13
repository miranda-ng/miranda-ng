#include "stdafx.h"

MyColourID colors[] =
{
	{ "Incoming Messages",     "ColorMsgIn",   RGB(0xd6, 0xf5, 0xc0) },
	{ "Outgoing Messages",     "ColorMsgOut",  RGB(0xf5, 0xe7, 0xd8) },

	{ "Incoming Files",        "ColorFileIn",  RGB(0xe3, 0xee, 0x9b) },
	{ "Outgoing Files",        "ColorFileOut", RGB(0xe3, 0xee, 0x9b) },

	{ "Status changes",        "ColorStatus",  RGB(0xf0, 0xf0, 0xf0) },

	{ "Other Outgoing Events", "ColorOut",     RGB(0xff, 0xff, 0xff) },
	{ "Other Incoming Events", "ColorIn",      RGB(0xff, 0xff, 0xff) },

	{ "Selected Items",        "ColorSel",     RGB(0x60, 0x60, 0x60) },
	{ "Selected Items (Text)", "ColorSelTxt",  RGB(0xff, 0xff, 0xff) }
};

MyFontID fonts[] =
{
	{ "Incoming Messages",     "FontMsgIn"   },
	{ "Outgoing Messages",     "FontMsgOut"  },

	{ "Incoming Files",        "FontFileIn"  },
	{ "Outgoing Files",        "FontFileOut" },

	{ "Status changes",        "FontStatus"  },

	{ "Other Outgoing Events", "FontOut"     },
	{ "Other Incoming Events", "FontIn"      }
};

int evtFontsChanged(WPARAM, LPARAM)
{
	for (auto &it : colors)
		it.cl = Colour_Get(MODULENAME, it.szName);

	for (auto &it : fonts) {
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

	for (auto &it : colors) {
		cid.order = int(&it - colors);
		strncpy_s(cid.name, it.szName, _TRUNCATE);
		strncpy_s(cid.setting, it.szSetting, _TRUNCATE);
		cid.defcolour = it.defaultValue;
		g_plugin.addColor(&cid);
		it.cl = Colour_Get(cid.group, cid.name);
	}

	FontID fontid = {};
	strncpy_s(fontid.group, MODULENAME, _TRUNCATE);
	strncpy_s(fontid.dbSettingsGroup, MODULENAME, _TRUNCATE);

	for (auto &it : fonts) {
		fontid.order = int(&it - fonts);
		strncpy_s(fontid.name, it.szName, _TRUNCATE);
		strncpy_s(fontid.setting, it.szSetting, _TRUNCATE);
		g_plugin.addFont(&fontid);

		it.cl = Font_Get(MODULENAME, it.szName, &it.lf);
		it.hfnt = CreateFontIndirectA(&it.lf);
	}
}

void DestroyFonts()
{
	for (auto &it : fonts)
		DeleteObject(it.hfnt);
}
