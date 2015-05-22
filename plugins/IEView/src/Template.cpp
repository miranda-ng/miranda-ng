/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010 Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "ieview_common.h"

TokenDef::TokenDef(const char *tokenString)
{
	this->tokenString = tokenString;
	this->tokenLen = (int)mir_strlen(tokenString);
	this->token = 0;
	this->escape = 0;
}

TokenDef::TokenDef(const char *tokenString, int token, int escape)
{
	this->tokenString = tokenString;
	this->token = token;
	this->tokenLen = (int)mir_strlen(tokenString);
	this->escape = escape;
}

Token::Token(int type, const char *text, int escape)
{
	next = NULL;
	this->type = type;
	this->escape = escape;
	this->text = mir_strdup(text);
}

Token::~Token()
{
	mir_free(text);
}

Token* Token::getNext()
{
	return next;
}

void Token::setNext(Token *ptr)
{
	next = ptr;
}

int Token::getType()
{
	return type;
}

int Token::getEscape()
{
	return escape;
}

const char* Token::getText()
{
	return text;
}

Template::Template(const char *name, const char *text)
{
	next = NULL;
	tokens = NULL;
	this->text = mir_strdup(text);
	this->name = mir_strdup(name);
	tokenize();
}

Template::~Template()
{
	mir_free(text);
	mir_free(name);
	Token *ptr = tokens, *ptr2;
	tokens = NULL;
	for (; ptr != NULL; ptr = ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}

const char* Template::getText()
{
	return text;
}

const char* Template::getName()
{
	return name;
}

Template* Template::getNext()
{
	return next;
}

bool Template::equals(const char *name)
{
	if (!strcmp(name, this->name))
		return true;

	return false;
}

static TokenDef tokenNames[] = {
	TokenDef("%name%", Token::INAME, 0),
	TokenDef("%time%", Token::TIME, 0),
	TokenDef("%text%", Token::TEXT, 0),
	TokenDef("%date%", Token::DATE, 0),
	TokenDef("%base%", Token::BASE, 0),
	TokenDef("%avatar%", Token::AVATAR, 0),
	TokenDef("%cid%", Token::CID, 0),
	TokenDef("%proto%", Token::PROTO, 0),
	TokenDef("%avatarIn%", Token::AVATARIN, 0),
	TokenDef("%avatarOut%", Token::AVATAROUT, 0),
	TokenDef("%nameIn%", Token::NAMEIN, 0),
	TokenDef("%nameOut%", Token::NAMEOUT, 0),
	TokenDef("%uin%", Token::UIN, 0),
	TokenDef("%uinIn%", Token::UININ, 0),
	TokenDef("%uinOut%", Token::UINOUT, 0),
	TokenDef("%nickIn%", Token::NICKIN, 0),
	TokenDef("%nickOut%", Token::NICKOUT, 1),
	TokenDef("%statusMsg%", Token::STATUSMSG, 0),
	TokenDef("%fileDesc%", Token::FILEDESC, 0),

	TokenDef("%\\name%", Token::INAME, 1),
	TokenDef("%\\time%", Token::TIME, 1),
	TokenDef("%\\text%", Token::TEXT, 1),
	TokenDef("%\\date%", Token::DATE, 1),
	TokenDef("%\\base%", Token::BASE, 1),
	TokenDef("%\\avatar%", Token::AVATAR, 1),
	TokenDef("%\\cid%", Token::CID, 1),
	TokenDef("%\\proto%", Token::PROTO, 1),
	TokenDef("%\\avatarIn%", Token::AVATARIN, 1),
	TokenDef("%\\avatarOut%", Token::AVATAROUT, 1),
	TokenDef("%\\nameIn%", Token::NAMEIN, 1),
	TokenDef("%\\nameOut%", Token::NAMEOUT, 1),
	TokenDef("%\\uin%", Token::UIN, 1),
	TokenDef("%\\uinIn%", Token::UININ, 1),
	TokenDef("%\\uinOut%", Token::UINOUT, 1),
	TokenDef("%\\nickIn%", Token::NICKIN, 1),
	TokenDef("%\\nickOut%", Token::NICKOUT, 1),
	TokenDef("%\\statusMsg%", Token::STATUSMSG, 1),
	TokenDef("%\\fileDesc%", Token::FILEDESC, 1)
};

void Template::tokenize()
{
	if (text == NULL)
		return;

	char *str = mir_strdup(text);
	Token *lastToken = NULL;
	int lastTokenType = Token::PLAIN;
	int lastTokenEscape = 0;
	int l = (int)mir_strlen(str);
	for (int i = 0, lastTokenStart = 0; i <= l;) {
		Token *newToken;
		int newTokenType = 0, newTokenSize = 0, newTokenEscape = 0;
		if (str[i] == '\0') {
			newTokenType = Token::END;
			newTokenSize = 1;
			newTokenEscape = 0;
		}
		else {
			bool found = false;
			for (unsigned int j = 0; j < (sizeof(tokenNames) / sizeof(tokenNames[0])); j++) {
				if (!strncmp(str + i, tokenNames[j].tokenString, tokenNames[j].tokenLen)) {
					newTokenType = tokenNames[j].token;
					newTokenSize = tokenNames[j].tokenLen;
					newTokenEscape = tokenNames[j].escape;
					found = true;
					break;
				}
			}
			if (!found) {
				newTokenType = Token::PLAIN;
				newTokenSize = 1;
				newTokenEscape = 0;
			}
		}
		if (newTokenType != Token::PLAIN) {
			if (str[i + newTokenSize] == '%') {
				//newTokenSize++;
			}
			str[i] = '\0';
		}
		if ((lastTokenType != newTokenType || lastTokenEscape != newTokenEscape) && i != lastTokenStart) {
			if (lastTokenType == Token::PLAIN)
				newToken = new Token(lastTokenType, str + lastTokenStart, lastTokenEscape);
			else
				newToken = new Token(lastTokenType, NULL, lastTokenEscape);

			if (lastToken != NULL)
				lastToken->setNext(newToken);
			else
				tokens = newToken;

			lastToken = newToken;
			lastTokenStart = i;
		}
		lastTokenEscape = newTokenEscape;
		lastTokenType = newTokenType;
		i += newTokenSize;
	}
	mir_free(str);
}

Token* Template::getTokens()
{
	return tokens;
}

TemplateMap* TemplateMap::mapList = NULL;

TemplateMap::TemplateMap(const char *name)
{
	entries = NULL;
	next = NULL;
	filename = NULL;
	this->name = mir_strdup(name);
	this->grouping = false;
	this->rtl = false;
}

TemplateMap::~TemplateMap()
{
	mir_free(name);
	mir_free(filename);

	clear();
}

TemplateMap* TemplateMap::add(const char *id, const char *filename)
{
	TemplateMap *map;
	for (map = mapList; map != NULL; map = map->next) {
		if (!strcmp(map->name, id)) {
			map->clear();
			map->setFilename(filename);
			return map;
		}
	}
	map = new TemplateMap(id);
	map->setFilename(filename);
	map->next = mapList;
	mapList = map;
	return map;
}

void TemplateMap::addTemplate(const char *name, const char *text)
{
	Template *tmplate = new Template(name, text);
	tmplate->next = entries;
	entries = tmplate;
}

void TemplateMap::clear()
{
	Template *ptr, *ptr2;
	ptr = entries;
	entries = NULL;
	for (; ptr != NULL; ptr = ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}

static TokenDef templateNames[] = {
	TokenDef("<!--HTMLStart-->"),
	TokenDef("<!--MessageIn-->"),
	TokenDef("<!--MessageOut-->"),
	TokenDef("<!--hMessageIn-->"),
	TokenDef("<!--hMessageOut-->"),
	TokenDef("<!--File-->"),
	TokenDef("<!--hFile-->"),
	TokenDef("<!--URL-->"),
	TokenDef("<!--hURL-->"),
	TokenDef("<!--Status-->"),
	TokenDef("<!--hStatus-->"),
	TokenDef("<!--MessageInGroupStart-->"),
	TokenDef("<!--MessageInGroupInner-->"),
	TokenDef("<!--MessageInGroupEnd-->"),
	TokenDef("<!--hMessageInGroupStart-->"),
	TokenDef("<!--hMessageInGroupInner-->"),
	TokenDef("<!--hMessageInGroupEnd-->"),
	TokenDef("<!--MessageOutGroupStart-->"),
	TokenDef("<!--MessageOutGroupInner-->"),
	TokenDef("<!--MessageOutGroupEnd-->"),
	TokenDef("<!--hMessageOutGroupStart-->"),
	TokenDef("<!--hMessageOutGroupInner-->"),
	TokenDef("<!--hMessageOutGroupEnd-->"),
	TokenDef("<!--FileIn-->"),
	TokenDef("<!--hFileIn-->"),
	TokenDef("<!--FileOut-->"),
	TokenDef("<!--hFileOut-->"),
	TokenDef("<!--URLIn-->"),
	TokenDef("<!--hURLIn-->"),
	TokenDef("<!--URLOut-->"),
	TokenDef("<!--hURLOut-->"),

	TokenDef("<!--HTMLStartRTL-->"),
	TokenDef("<!--MessageInRTL-->"),
	TokenDef("<!--MessageOutRTL-->"),
	TokenDef("<!--hMessageInRTL-->"),
	TokenDef("<!--hMessageOutRTL-->"),
	TokenDef("<!--MessageInGroupStartRTL-->"),
	TokenDef("<!--MessageInGroupInnerRTL-->"),
	TokenDef("<!--MessageInGroupEndRTL-->"),
	TokenDef("<!--hMessageInGroupStartRTL-->"),
	TokenDef("<!--hMessageInGroupInnerRTL-->"),
	TokenDef("<!--hMessageInGroupEndRTL-->"),
	TokenDef("<!--MessageOutGroupStartRTL-->"),
	TokenDef("<!--MessageOutGroupInnerRTL-->"),
	TokenDef("<!--MessageOutGroupEndRTL-->"),
	TokenDef("<!--hMessageOutGroupStartRTL-->"),
	TokenDef("<!--hMessageOutGroupInnerRTL-->"),
	TokenDef("<!--hMessageOutGroupEndRTL-->")
};

TemplateMap* TemplateMap::loadTemplateFile(const char *id, const char *filename, bool onlyInfo)
{
	char lastTemplate[1024], tmp2[1024];
	unsigned int i = 0;
	if (filename == NULL || mir_strlen(filename) == 0)
		return NULL;

	FILE *fh = fopen(filename, "rt");
	if (fh == NULL)
		return NULL;

	TemplateMap *tmap;
	if (!onlyInfo)
		tmap = TemplateMap::add(id, filename);
	else
		tmap = new TemplateMap(id);

	char store[4096];
	bool wasTemplate = false;
	CMStringA templateText;
	while (fgets(store, sizeof(store), fh) != NULL) {
		if (sscanf(store, "%s", tmp2) == EOF) continue;
		//template start
		bool bFound = false;
		for (unsigned i = 0; i < SIZEOF(templateNames); i++) {
			if (!strncmp(store, templateNames[i].tokenString, templateNames[i].tokenLen)) {
				bFound = true;
				break;
			}
		}
		if (bFound) {
			if (wasTemplate)
				tmap->addTemplate(lastTemplate, templateText);

			templateText.Empty();
			wasTemplate = true;
			sscanf(store, "<!--%[^-]", lastTemplate);
		}
		else if (wasTemplate)
			templateText.Append(store);
	}
	if (wasTemplate)
		tmap->addTemplate(lastTemplate, templateText);
	templateText.Empty();

	fclose(fh);
	static const char *groupTemplates[] = { "MessageInGroupStart", "MessageInGroupInner",
		"hMessageInGroupStart", "hMessageInGroupInner",
		"MessageOutGroupStart", "MessageOutGroupInner",
		"hMessageOutGroupStart", "hMessageOutGroupInner" };
	tmap->grouping = true;
	for (i = 0; i < SIZEOF(groupTemplates); i++) {
		if (tmap->getTemplate(groupTemplates[i]) == NULL) {
			tmap->grouping = false;
			break;
		}
	}
	static const char *rtlTemplates[] = { "MessageInRTL", "MessageOutRTL" }; //"HTMLStartRTL",
	tmap->rtl = true;
	for (i = 0; i < SIZEOF(rtlTemplates); i++) {
		if (tmap->getTemplate(rtlTemplates[i]) == NULL) {
			tmap->rtl = false;
			break;
		}
	}
	return tmap;
}

bool TemplateMap::isGrouping()
{
	return grouping;
}

bool TemplateMap::isRTL()
{
	return rtl;
}

Template* TemplateMap::getTemplate(const char *text)
{
	for (Template *ptr = entries; ptr != NULL; ptr = ptr->getNext())
		if (ptr->equals(text))
			return ptr;

	return 0;
}

Template* TemplateMap::getTemplate(const char *proto, const char *text)
{
	for (TemplateMap *ptr = mapList; ptr != NULL; ptr = ptr->next)
		if (!strcmp(ptr->name, proto))
			return ptr->getTemplate(text);

	return NULL;
}

TemplateMap* TemplateMap::getTemplateMap(const char *proto)
{
	for (TemplateMap *ptr = mapList; ptr != NULL; ptr = ptr->next)
		if (!strcmp(ptr->name, proto))
			return ptr;

	return NULL;
}

const char *TemplateMap::getFilename()
{
	return filename;
}

void TemplateMap::setFilename(const char *_filename)
{
	replaceStr(filename, _filename);
	Utils::convertPath(filename);
}

TemplateMap* TemplateMap::loadTemplates(const char *id, const char *filename, bool onlyInfo)
{
	return loadTemplateFile(id, filename, onlyInfo);
}

void TemplateMap::dropTemplates()
{
	for (TemplateMap *p = mapList, *p1; p != NULL; p = p1) {
		p1 = p->next;
		delete p;
	}
}
