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
class TemplateMap;
class Template;
#ifndef TEMPLATE_INCLUDED
#define TEMPLATE_INCLUDED

#include "ieview_common.h"

class TokenDef {
public:
	const char *tokenString;
	int 		token;
	int         tokenLen;
	int         escape;
	TokenDef(const char *tokenString);
	TokenDef(const char *tokenString, int token, int escape);
};

class Token {
private:
	int  escape;
	int  type;
	char *text;
	Token *next;
public:
	enum TOKENS {
		END = 0,
		BASE,
		PLAIN,
		TEXT,
		INAME,
		TIME,
		DATE,
		AVATAR,
		CID,
		PROTO,
		AVATARIN,
		AVATAROUT,
		NAMEIN,
		NAMEOUT,
		UIN,
		UININ,
		UINOUT,
		STATUSMSG,
		NICKIN,
		NICKOUT,
		FILEDESC,
	};
	Token(int, const char *, int);
	~Token();
	int getType();
	int getEscape();
	const char *getText();
	Token *getNext();
	void   setNext(Token *);
};

class Template {
private:
	char *name;
	char *text;
	Template *next;
	Token *tokens;
protected:
	friend class TemplateMap;
	bool        equals(const char *name);
	void        tokenize();
	Template *	getNext();
	Template(const char *name, const char *text);
public:
	~Template();
	const char *getText();
	const char *getName();
	Token *getTokens();
};

class TemplateMap {
private:
	static TemplateMap *mapList;
	char *name;
	char *filename;
	bool  grouping;
	bool  rtl;
	Template *entries;
	TemplateMap *next;
	TemplateMap(const char *name);
	void addTemplate(const char *name, const char *text);
	void setFilename(const char *filename);
	void clear();
	static TemplateMap* add(const char *id, const char *filename);
	static void appendText(char **str, int *sizeAlloced, const char *fmt, ...);
	static TemplateMap* loadTemplateFile(const char *proto, const char *filename, bool onlyInfo);
public:
	~TemplateMap();
	static Template* getTemplate(const char *id, const char *name);
	static TemplateMap* getTemplateMap(const char *id);
	static TemplateMap* loadTemplates(const char *id, const char *filename, bool onlyInfo);
	static void dropTemplates();
	Template* getTemplate(const char *text);
	const char* getFilename();
	bool isGrouping();
	bool isRTL();
};


#endif
