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

#include "stdafx.h"

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
	next = nullptr;
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
	m_next = nullptr;
	m_tokens = nullptr;
	m_text = mir_strdup(text);
	m_name = mir_strdup(name);
	tokenize();
}

Template::~Template()
{
	mir_free(m_text);
	mir_free(m_name);
	Token *ptr = m_tokens, *ptr2;
	m_tokens = nullptr;
	for (; ptr != nullptr; ptr = ptr2) {
		ptr2 = ptr->getNext();
		delete ptr;
	}
}

bool Template::equals(const char *name)
{
	if (!mir_strcmp(m_name, name))
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
};

void Template::tokenize()
{
	if (m_text == nullptr)
		return;

	char *str = mir_strdup(m_text);
	Token *lastToken = nullptr;
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
				newToken = new Token(lastTokenType, nullptr, lastTokenEscape);

			if (lastToken != nullptr)
				lastToken->setNext(newToken);
			else
				m_tokens = newToken;

			lastToken = newToken;
			lastTokenStart = i;
		}
		lastTokenEscape = newTokenEscape;
		lastTokenType = newTokenType;
		i += newTokenSize;
	}
	mir_free(str);
}

TemplateMap* TemplateMap::m_mapList = nullptr;

TemplateMap::TemplateMap(const char *name)
{
	m_entries = nullptr;
	m_next = nullptr;
	m_filename = nullptr;
	m_name = mir_strdup(name);
	m_grouping = false;
	m_rtl = false;
}

TemplateMap::~TemplateMap()
{
	mir_free(m_name);
	mir_free(m_filename);

	clear();
}

TemplateMap* TemplateMap::add(const char *id, const char *filename)
{
	TemplateMap *map;
	for (map = m_mapList; map != nullptr; map = map->m_next) {
		if (!mir_strcmp(map->m_name, id)) {
			map->clear();
			map->setFilename(filename);
			return map;
		}
	}
	map = new TemplateMap(id);
	map->setFilename(filename);
	map->m_next = m_mapList;
	m_mapList = map;
	return map;
}

void TemplateMap::addTemplate(const char *name, const char *text)
{
	Template *tmplate = new Template(name, text);
	tmplate->m_next = m_entries;
	m_entries = tmplate;
}

void TemplateMap::clear()
{
	Template *ptr, *ptr2;
	ptr = m_entries;
	m_entries = nullptr;
	for (; ptr != nullptr; ptr = ptr2) {
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
	if (filename == nullptr || mir_strlen(filename) == 0)
		return nullptr;

	FILE *fh = fopen(filename, "rt");
	if (fh == nullptr)
		return nullptr;

	TemplateMap *tmap;
	if (!onlyInfo)
		tmap = TemplateMap::add(id, filename);
	else
		tmap = new TemplateMap(id);

	char store[4096];
	bool wasTemplate = false;
	CMStringA templateText;
	while (fgets(store, sizeof(store), fh) != nullptr) {
		if (sscanf(store, "%s", tmp2) == EOF) continue;
		//template start
		bool bFound = false;
		for (auto &it : templateNames)
			if (!strncmp(store, it.tokenString, it.tokenLen)) {
				bFound = true;
				break;
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
	tmap->m_grouping = true;
	for (auto &it : groupTemplates) {
		if (tmap->getTemplate(it) == nullptr) {
			tmap->m_grouping = false;
			break;
		}
	}
	static const char *rtlTemplates[] = { "MessageInRTL", "MessageOutRTL" }; //"HTMLStartRTL",
	tmap->m_rtl = true;
	for (auto &it : rtlTemplates) {
		if (tmap->getTemplate(it) == nullptr) {
			tmap->m_rtl = false;
			break;
		}
	}
	return tmap;
}

Template* TemplateMap::getTemplate(const char *text)
{
	for (Template *ptr = m_entries; ptr != nullptr; ptr = ptr->getNext())
		if (ptr->equals(text))
			return ptr;

	return nullptr;
}

Template* TemplateMap::getTemplate(const char *proto, const char *text)
{
	for (TemplateMap *ptr = m_mapList; ptr != nullptr; ptr = ptr->m_next)
		if (!mir_strcmp(ptr->m_name, proto))
			return ptr->getTemplate(text);

	return nullptr;
}

TemplateMap* TemplateMap::getTemplateMap(const char *proto)
{
	for (TemplateMap *ptr = m_mapList; ptr != nullptr; ptr = ptr->m_next)
		if (!mir_strcmp(ptr->m_name, proto))
			return ptr;

	return nullptr;
}

void TemplateMap::setFilename(const char *filename)
{
	replaceStr(m_filename, filename);
	Utils::convertPath(m_filename);
}

TemplateMap* TemplateMap::loadTemplates(const char *id, const char *filename, bool onlyInfo)
{
	return loadTemplateFile(id, filename, onlyInfo);
}

void TemplateMap::dropTemplates()
{
	for (TemplateMap *p = m_mapList, *p1; p != nullptr; p = p1) {
		p1 = p->m_next;
		delete p;
	}
}
