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
	COLORREF cl;
	ColourID info;
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
//	FONT_DEFAULT,
//	FONT_SELECTED,
	FONT_COUNT
};

struct MyFontID
{
	COLORREF cl;
	LOGFONTA *lf;
	HFONT hfnt;
	FontID info;
};

extern MyFontID fonts[];

void InitFonts();
void DestroyFonts();

#endif // __fonts_h__