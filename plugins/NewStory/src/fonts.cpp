#include "stdafx.h"

MyColourID colors[] = 
{
	{0, {MODULETITLE, "Incoming Messages",     MODULENAME, "ColorMsgIn", 0,   RGB(0xff, 0xff, 0xff),  0}},
	{0, {MODULETITLE, "Outgoing Messages",     MODULENAME, "ColorMsgOut", 0,  RGB(0xff, 0xff, 0xff),  1}},

	{0, {MODULETITLE, "Incoming Files",        MODULENAME, "ColorFileIn", 0,  RGB(0xff, 0xff, 0xff),  2}},
	{0, {MODULETITLE, "Outgoing Files",        MODULENAME, "ColorFileOut", 0, RGB(0xff, 0xff, 0xff),  3}},

	{0, {MODULETITLE, "Incoming URLs",         MODULENAME, "ColorURLIn", 0,   RGB(0xff, 0xff, 0xff),  4}},
	{0, {MODULETITLE, "Outgoing URLs",         MODULENAME, "ColorURLOut", 0,  RGB(0xff, 0xff, 0xff),  5}},

	{0, {MODULETITLE, "Status changes",        MODULENAME, "ColorStatus", 0,  RGB(0xff, 0xff, 0xff),  6}},

	{0, {MODULETITLE, "Other Outgoing Events", MODULENAME, "ColorOut", 0,     RGB(0xff, 0xff, 0xff),  7}},
	{0, {MODULETITLE, "Other Incoming Events", MODULENAME, "ColorIn", 0,      RGB(0xff, 0xff, 0xff),  8}},

	{0, {MODULETITLE, "Selected Items",        MODULENAME, "ColorSel", 0,     RGB(0x60, 0x60, 0x60),  9}},
	{0, {MODULETITLE, "Selected Items (Text)", MODULENAME, "ColorSelTxt", 0,  RGB(0xff, 0xff, 0xff), 10}}
};

MyFontID fonts[] = 
{
	{0, 0, 0, {MODULETITLE, "Incoming Messages",     MODULENAME, "FontMsgIn", 0,   {0}, 0}},
	{0, 0, 0, {MODULETITLE, "Outgoing Messages",     MODULENAME, "FontMsgOut", 0,  {0}, 1}},

	{0, 0, 0, {MODULETITLE, "Incoming Files",        MODULENAME, "FontFileIn", 0,  {0}, 2}},
	{0, 0, 0, {MODULETITLE, "Outgoing Files",        MODULENAME, "FontFileOut", 0, {0}, 3}},

	{0, 0, 0, {MODULETITLE, "Incoming URLs",         MODULENAME, "FontURLIn", 0,   {0}, 4}},
	{0, 0, 0, {MODULETITLE, "Outgoing URLs",         MODULENAME, "FontURLOut", 0,  {0}, 5}},

	{0, 0, 0, {MODULETITLE, "Status changes",        MODULENAME, "FontStatus", 0,  {0}, 6}},

	{0, 0, 0, {MODULETITLE, "Other Outgoing Events", MODULENAME, "FontOut", 0,     {0}, 7}},
	{0, 0, 0, {MODULETITLE, "Other Incoming Events", MODULENAME, "FontIn", 0,      {0}, 8}}

//	{sizeof(FontID), MODULETITLE, "Default Text",  MODULENAME, "FontDef", FIDF_SAVEPOINTSIZE, {0}, 0},
//	{sizeof(FontID), MODULETITLE, "Selected Item", MODULENAME, "FontSel", FIDF_SAVEPOINTSIZE, {0}, 1}
};

int evtFontsChanged(WPARAM, LPARAM)
{
	int i;
	for (i = 0; i < COLOR_COUNT; i++)
		colors[i].cl = Colour_Get(colors[i].info.group, colors[i].info.name);

	for (i = 0; i < FONT_COUNT; i++) {
		fonts[i].cl = (COLORREF)Font_Get(fonts[i].info, fonts[i].lf);
		DeleteObject(fonts[i].hfnt);
		fonts[i].hfnt = CreateFontIndirectA(fonts[i].lf);
	}
//	WindowList_Broadcast(hNewstoryWindows, UM_REDRAWLISTH, 0, 0);
	return 0;
}

void InitFonts()
{
	bool isFSInstalled = /*ServiceExists(MS_FONT_GET) ? */true/* : false*/;

	int i; // damn msvc can't handle for(int i...) correctly! =(

	HookEvent(ME_FONT_RELOAD, evtFontsChanged);
	HookEvent(ME_COLOUR_RELOAD, evtFontsChanged);

	if (isFSInstalled)
	{
		for (i = 0; i < COLOR_COUNT; i++)
		{
			g_plugin.addColor(&colors[i].info);
			colors[i].cl = Colour_Get(colors[i].info.group, colors[i].info.name);
		}
		for (i = 0; i < FONT_COUNT; i++)
		{
			//CallService(MS_FONT_REGISTER, (WPARAM)&fonts[i].info, 0);
			g_plugin.addFont(&fonts[i].info);
			fonts[i].lf = (LOGFONTA *)malloc(sizeof(LOGFONTA));
			fonts[i].cl = (COLORREF)Font_Get(fonts[i].info, fonts[i].lf);
			fonts[i].hfnt = CreateFontIndirectA(fonts[i].lf);
			//MessageBox(0, fonts[i].lf->lfFaceName, _T(""), MB_OK);
		}
	} else
	{
		LOGFONTA lfText = {0};
		lfText.lfHeight = -11;
		lfText.lfWidth = lfText.lfEscapement = lfText.lfOrientation = 0;
		lfText.lfItalic = lfText.lfUnderline = lfText.lfStrikeOut = FALSE;
		lfText.lfCharSet = DEFAULT_CHARSET;
		lfText.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lfText.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lfText.lfQuality = DEFAULT_QUALITY;
		lfText.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS; 
		lstrcpyA(lfText.lfFaceName, "MS Shell Dlg");
		lfText.lfWeight = FW_REGULAR;

		for (i = 0; i < COLOR_COUNT; i++)
		{
			colors[i].cl = colors[i].info.defcolour;
		}
		for (i = 0; i < FONT_COUNT; i++)
		{
			fonts[i].lf = (LOGFONTA *)malloc(sizeof(LOGFONTA));
			*fonts[i].lf = lfText;
			fonts[i].cl = RGB(0,0,0);
			fonts[i].hfnt = CreateFontIndirectA(fonts[i].lf);
		}
	}
}

void DestroyFonts()
{
	int i;
	for (i = 0; i < FONT_COUNT; i++)
	{
		DeleteObject(fonts[i].hfnt);
		free(fonts[i].lf);
	}
}