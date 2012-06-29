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
#include "FontList.h"
#include "Utils.h"

#define FONTCOUNT  12
static Font defFonts[] = {
	Font("Timestamp","Verdana", DEFAULT_CHARSET, -9, Font::BOLD, 0x000000),
//	Font("Colon","Verdana", DEFAULT_CHARSET, -13, 0, 0x808080),
	Font("Others names","Verdana", DEFAULT_CHARSET, -13, 0, 0x723732),
	Font("Your name","Verdana", DEFAULT_CHARSET, -13, 0, 0x4453A4),
	Font("Incoming message","Verdana", DEFAULT_CHARSET, -13, 0, 0x000000),
	Font("Outgoing message","Verdana", DEFAULT_CHARSET, -13, 0, 0x505080),
	Font("User has joined","Verdana", DEFAULT_CHARSET, -11, Font::ITALIC, 0x0F880F),
	Font("User has left","Verdana", DEFAULT_CHARSET, -11, Font::ITALIC, 0x1313B9),
	Font("The topic is ...","Verdana", DEFAULT_CHARSET, -13, Font::ITALIC, 0x991A53),
	Font("Error","Verdana", DEFAULT_CHARSET, -13, Font::BOLD, 0x1010D0),
	Font("Message typing area","Verdana", DEFAULT_CHARSET, -13, 0, 0x000000),
	Font("User list","Tahoma", DEFAULT_CHARSET, -11, 0, 0x000000),
	Font("User list group","Tahoma", DEFAULT_CHARSET, -11, Font::BOLD, 0x000000)

};

Font::Font() {
	name = NULL;
	face = NULL;
	size = 0;
	style = 0;
	color = 0;
	charSet = DEFAULT_CHARSET;
}

Font::Font(const char *n, const char *f, int cs, int s, int st, COLORREF c) {
	name = NULL;
	face = NULL;
	size = 0;
	style = 0;
	color = 0;
	setName(n);
	setFace(f);
	setCharSet(cs);
	setSize(s);
	setStyle(st);
	setColor(c);
}


COLORREF Font::getColor() {
	return color;
}

void Font::setColor(COLORREF c) {
	color = c;
}

int Font::getSize() {
	return size;
}

void Font::setSize(int s) {
	size = s;
}

int Font::getStyle() {
	return style;
}

void Font::setStyle(int s) {
	style = s;
}

int Font::getCharSet() {
	return charSet;
}

void Font::setCharSet(int s) {
	charSet = s;
}

void Font::setName(const char *s) {
	Utils::copyString(&name, s);
}

const char *Font::getName() {
	return name;
}

void Font::setFace(const char *s) {
	Utils::copyString(&face, s);
}

const char *Font::getFace() {
	return face;
}

Font *FontList::fonts = NULL;
Font *FontList::fontsSettings = NULL;

Font *FontList::getFont(int index) {
	if (index < FONTCOUNT)	return &fonts[index];
	else return NULL;
}

Font *FontList::getFontSettings(int index) {
	if (index < FONTCOUNT)	return &fontsSettings[index];
	else return NULL;
}

void FontList::loadSettings() {
	char fontParam[512];
	for (int i=0;i<FONTCOUNT;i++) {
		DBVARIANT dbv;
		fontsSettings[i].setName(defFonts[i].getName());
		fontsSettings[i].setFace(defFonts[i].getFace());
		fontsSettings[i].setCharSet(defFonts[i].getCharSet());
		fontsSettings[i].setSize(defFonts[i].getSize());
		fontsSettings[i].setStyle(defFonts[i].getStyle());
		fontsSettings[i].setColor(defFonts[i].getColor());
		sprintf(fontParam, "Font%dFace", i);
		if (!DBGetContactSetting(NULL, muccModuleName, fontParam, &dbv)) {
			fontsSettings[i].setFace(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		sprintf(fontParam, "Font%dSet", i);
		if (!DBGetContactSetting(NULL, muccModuleName, fontParam, &dbv)) {
			fontsSettings[i].setCharSet(dbv.bVal);
			DBFreeVariant(&dbv);
		}
		sprintf(fontParam, "Font%dSize", i);
		if (!DBGetContactSetting(NULL, muccModuleName, fontParam, &dbv)) {
			fontsSettings[i].setSize(dbv.lVal);
			DBFreeVariant(&dbv);
		}
		sprintf(fontParam, "Font%dCol", i);
		if (!DBGetContactSetting(NULL, muccModuleName, fontParam, &dbv)) {
			fontsSettings[i].setColor(dbv.lVal);
			DBFreeVariant(&dbv);
		}
		sprintf(fontParam, "Font%dStyle", i);
		if (!DBGetContactSetting(NULL, muccModuleName, fontParam, &dbv)) {
			fontsSettings[i].setStyle(dbv.bVal);
			DBFreeVariant(&dbv);
		}
		fonts[i].setName(fontsSettings[i].getName());
		fonts[i].setFace(fontsSettings[i].getFace());
		fonts[i].setCharSet(fontsSettings[i].getCharSet());
		fonts[i].setSize(fontsSettings[i].getSize());
		fonts[i].setStyle(fontsSettings[i].getStyle());
		fonts[i].setColor(fontsSettings[i].getColor());
	}
}

void FontList::saveSettings() {
	char fontParam[512];
	for (int i=0;i<FONTCOUNT;i++) {
		sprintf(fontParam, "Font%dFace", i);
		DBWriteContactSettingString(NULL, muccModuleName, fontParam, fontsSettings[i].getFace());
		sprintf(fontParam, "Font%dSet", i);
		DBWriteContactSettingByte(NULL, muccModuleName, fontParam, fontsSettings[i].getCharSet());
		sprintf(fontParam, "Font%dSize", i);
		DBWriteContactSettingDword(NULL, muccModuleName, fontParam, fontsSettings[i].getSize());
		sprintf(fontParam, "Font%dCol", i);
		DBWriteContactSettingDword(NULL, muccModuleName, fontParam, fontsSettings[i].getColor());
		sprintf(fontParam, "Font%dStyle", i);
		DBWriteContactSettingByte(NULL, muccModuleName, fontParam, fontsSettings[i].getStyle());
		fonts[i].setName(fontsSettings[i].getName());
		fonts[i].setFace(fontsSettings[i].getFace());
		fonts[i].setCharSet(fontsSettings[i].getCharSet());
		fonts[i].setStyle(fontsSettings[i].getStyle());
		fonts[i].setSize(fontsSettings[i].getSize());
		fonts[i].setColor(fontsSettings[i].getColor());
	}
}

void FontList::init() {
	fonts = new Font[FONTCOUNT];
	fontsSettings = new Font[FONTCOUNT];
	loadSettings();
}

void FontList::release() {
	delete[] fonts;
	delete[] fontsSettings;
}

int FontList::getFontNum() {
	return FONTCOUNT;
}
