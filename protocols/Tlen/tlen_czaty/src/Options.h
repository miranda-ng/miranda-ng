/*

MUCC Group Chat GUI Plugin for Miranda NG
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
#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
#include "FontList.h"
#include "mucc.h"

extern int MUCCOptInit(WPARAM wParam, LPARAM lParam);

class Options:public FontList {
private:
	static COLORREF colorInputBg;
	static COLORREF	colorLogBg;
	static COLORREF	colorListBg;
	static HBRUSH   brushInputBg;
	static HBRUSH   brushLogBg;
	static HBRUSH   brushListBg;
	static int		logLimit;
	static int		chatContainerOptions;
	static int		chatWindowOptions;
	static COLORREF	chatWindowFontColor;
	static int		chatWindowFont;
	static int		chatWindowFontSize;
	static int		chatWindowFontStyle;
public:
	static void		setListBgColor(COLORREF c);
	static void		setLogBgColor(COLORREF c);
	static void		setInputBgColor(COLORREF c);
	static COLORREF getListBgColor();
	static COLORREF getLogBgColor();
	static COLORREF getInputBgColor();
	static HBRUSH	getListBgBrush();
	static HBRUSH	getInputBgBrush();
	static HBRUSH	getLogBgBrush();
	static void		setLogLimit(int l);
	static int		getLogLimit();
	static void		setChatContainerOptions(int o);
	static int		getChatContainerOptions();
	static void		setChatWindowOptions(int o);
	static int		getChatWindowOptions();
	static void		setChatWindowFontColor(COLORREF o);
	static COLORREF	getChatWindowFontColor();
	static void		setChatWindowFont(int o);
	static int		getChatWindowFont();
	static void		setChatWindowFontSize(int o);
	static int		getChatWindowFontSize();
	static void		setChatWindowFontStyle(int o);
	static int		getChatWindowFontStyle();
	static void		setExpandFlags(int l);
	static int		getExpandFlags();
	static void		init();
	static void		loadSettings();
	static void		saveSettings();
	
};

#endif
