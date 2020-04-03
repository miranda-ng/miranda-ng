#ifndef __fonts_h__
#define __fonts_h__

enum
{
	COLOR_INMSG,
	COLOR_OUTMSG,
	COLOR_INFILE,
	COLOR_OUTFILE,
	COLOR_INURL,
	COLOR_OUTURL,
	COLOR_STATUS,
	COLOR_INOTHER,
	COLOR_OUTOTHER,
	COLOR_SELECTED,
	COLOR_SELTEXT,
	COLOR_COUNT
};

struct MyColourID
{
	const char* szName, * szSetting;
	COLORREF defaultValue, cl;
};



extern MyColourID colors[];

enum
{
	FONT_INMSG,
	FONT_OUTMSG,
	FONT_INFILE,
	FONT_OUTFILE,
	FONT_INURL,
	FONT_OUTURL,
	FONT_STATUS,
	FONT_INOTHER,
	FONT_OUTOTHER,
	FONT_COUNT
};

struct MyFontID
{
	const char* szName, * szSetting;

	COLORREF cl;
	LOGFONTA lf;
	HFONT    hfnt;
};

extern MyFontID fonts[];

void InitFonts();
void DestroyFonts();

#endif // __fonts_h__