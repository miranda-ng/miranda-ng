/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

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
#ifndef FONTLIST_INCLUDED
#define FONTLIST_INCLUDED
#include "mucc.h"


class Font {
private:
	COLORREF	color;
	char *		face;
	char *		name;
	int			size;
	int			style;
	int			charSet;
public:
	enum STYLES {
		BOLD		= 1,
		ITALIC		= 2,
		UNDERLINE	= 4,
		STROKE		= 8,
		UNDERWAVE	= 16
	};
	Font();
	Font(const char *name, const char *face, int charSet, int size, int style, COLORREF color);
	void		setColor(COLORREF c);
	COLORREF	getColor();
	void		setSize(int s);
	int			getSize();
	void		setStyle(int s);
	int			getStyle();
	void		setFace(const char *s);
	const char *getFace();
	void		setName(const char *s);
	const char *getName();
	void		setCharSet(int s);
	int			getCharSet();
};

class FontList {
private:
	static Font *fonts;
	static Font *fontsSettings;
public:
	enum FONTIDEX {
		FONT_TIMESTAMP		= 0,
//		FONT_COLON			= 1,
		FONT_OTHERSNAMES	= 1,
		FONT_MYNAME			= 2,
		FONT_INMESSAGE		= 3,
		FONT_OUTMESSAGE		= 4,
		FONT_JOINED			= 5,
		FONT_LEFT			= 6,
		FONT_TOPIC			= 7,
//		FONT_NOTIFICATION	= 9,
		FONT_ERROR			= 8,
		FONT_TYPING			= 9,
		FONT_USERLIST		= 10,
		FONT_USERLISTGROUP	= 11
	};
	static Font *getFont(int index);
	static Font *getFontSettings(int index);
	static void	release();
	static void	init();
	static int	getFontNum();
	static void loadSettings();
	static void saveSettings();
};

#endif


