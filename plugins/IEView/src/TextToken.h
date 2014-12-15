/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

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

#ifndef TEXTTOKEN_INCLUDED
#define TEXTTOKEN_INCLUDED
#include "IEView.h"
class TextToken {
private:
	int  type;
	bool end;
	int  tag;
	DWORD value;
	wchar_t *wtext;
	char *text;
	wchar_t *wlink;
	char *link;
	TextToken *next;
	static TextToken* 	tokenizeBBCodes(const wchar_t *text, int len);
public:
	enum TOKENS {
		END = 0,
		TEXT,
		LINK,
		WWWLINK,
		SMILEY,
		BBCODE,
		MATH,
	};
	enum BBCODES {
		BB_B = 0,
		BB_I,
		BB_U,
		BB_S,
		BB_COLOR,
		BB_SIZE,
		BB_IMG,
		BB_BIMG,
		BB_URL,
		BB_CODE,
		BB_BACKGROUND
	};
	TextToken(int type, const char *text, int len);
	TextToken(int type, const wchar_t *wtext, int len);
	~TextToken();
	int 				getType();
	const char *		getText();
	const wchar_t*      getTextW();
	const char *		getLink();
	const wchar_t *		getLinkW();
	void 				setLink(const char *link);
	void 				setLink(const wchar_t *wlink);
	int 				getTag();
	void                setTag(int);
	bool 				isEnd();
	void                setEnd(bool);
	TextToken *			getNext();
	void   				setNext(TextToken *);
	//	void				toString(char **str, int *sizeAlloced);
	void				toString(wchar_t **str, int *sizeAlloced);
	//	static char *		htmlEncode(const char *str);
	//	static char *		urlEncode2(const char *str);
	//	static TextToken* 	tokenizeLinks(const char *text);
	//	static TextToken*	tokenizeSmileys(const char *proto, const char *text);
	// UNICODE
	wchar_t *			htmlEncode(const wchar_t *str);
	static TextToken* 	tokenizeLinks(const wchar_t *wtext);
	static TextToken* 	tokenizeSmileys(MCONTACT hContact, const char *proto, const wchar_t *wtext, bool isSent);
	static TextToken* 	tokenizeBBCodes(const wchar_t *text);
	static TextToken*	tokenizeChatFormatting(const wchar_t *text);
};
#endif
