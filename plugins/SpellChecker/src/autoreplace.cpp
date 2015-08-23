/*
Copyright (C) 2009-2010 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "commons.h"

AutoReplacement::AutoReplacement()
	: useVariables(FALSE)
{
}

AutoReplacement::AutoReplacement(const TCHAR *replace, BOOL useVariables)
	: replace(replace), useVariables(useVariables)
{
}


AutoReplaceMap::AutoReplaceMap(TCHAR *aFilename, Dictionary *dict)
{
	m_dict = dict;
	mir_tstrncpy(m_filename, aFilename, _countof(m_filename));
	loadAutoReplaceMap();
}

void AutoReplaceMap::loadAutoReplaceMap()
{
	FILE *file = _tfopen(m_filename, _T("rb"));
	if (file == NULL)
		return;

	char tmp[1024];
	int c, pos = 0;
	while ((c = fgetc(file)) != EOF) {
		if (c == '\n' || c == '\r' || pos >= _countof(tmp) - 1) {
			if (pos > 0) {
				tmp[pos] = '\0';

				// Get from
				BOOL useVars = FALSE;
				char *p;
				if ((p = strstr(tmp, "->")) != NULL) {
					*p = '\0';
					p += 2;
				}
				else if ((p = strstr(tmp, "-V>")) != NULL) {
					*p = '\0';
					p += 3;
					useVars = TRUE;
				}

				if (p != NULL) {
					Utf8ToTchar find(tmp);
					Utf8ToTchar replace(p);

					lstrtrim(find);
					lstrtrim(replace);

					if (find[0] != 0 && replace[0] != 0)
						m_replacements[find.get()] = AutoReplacement(replace, useVars);
				}
			}

			pos = 0;
		}
		else {
			tmp[pos] = (char)c;
			pos++;
		}
	}
	fclose(file);
}

void AutoReplaceMap::writeAutoReplaceMap()
{
	// Create path
	TCHAR *p = _tcsrchr(m_filename, _T('\\'));
	if (p != NULL) {
		*p = 0;
		CreateDirectoryTreeT(m_filename);
		*p = _T('\\');
	}

	// Write it
	FILE *file = _tfopen(m_filename, _T("wb"));
	if (file != NULL) {
		map<tstring, AutoReplacement>::iterator it = m_replacements.begin();
		for (; it != m_replacements.end(); it++) {
			AutoReplacement &ar = it->second;

			TcharToUtf8 find(it->first.c_str());
			TcharToUtf8 replace(ar.replace.c_str());

			if (ar.useVariables)
				fprintf(file, "%s-V>%s\n", (const char *)find, (const char *)replace);
			else
				fprintf(file, "%s->%s\n", (const char *)find, (const char *)replace);
		}
		fclose(file);
	}
}


BOOL AutoReplaceMap::isWordChar(TCHAR c)
{
	if (IsNumber(c))
		return TRUE;

	if (_tcschr(_T("-_.!@#$%&*()[]{}<>:?/\\=+"), c) != NULL)
		return TRUE;

	return m_dict->isWordChar(c);
}


TCHAR* AutoReplaceMap::autoReplace(const TCHAR * word)
{
	scoped_free<TCHAR> from = _tcslwr(_tcsdup(word));

	if (m_replacements.find(from.get()) == m_replacements.end())
		return NULL;

	AutoReplacement &ar = m_replacements[from.get()];

	TCHAR *to;
	if (ar.useVariables)
		to = variables_parsedup((TCHAR *)ar.replace.c_str(), (TCHAR *)word, NULL);
	else
		to = _tcsdup(ar.replace.c_str());

	// Wich case to use?
	size_t len = mir_tstrlen(word);
	size_t i;
	for (i = 0; i < len; i++)
		if (IsCharLower(word[i]))
			break;

	if (i <= 0) // All lower
		return to;

	if (i >= len)  // All upper
		return CharUpper(to);

	// First upper
	TCHAR tmp[2];
	tmp[0] = to[0];
	tmp[1] = _T('\0');
	CharUpper(tmp);
	to[0] = tmp[0];
	return to;
}

TCHAR* AutoReplaceMap::filterText(const TCHAR *find)
{
	TCHAR *ret = _tcsdup(find);
	int len = mir_tstrlen(ret);
	int pos = 0;
	for (int i = 0; i < len; i++)
		if (isWordChar(find[i]))
			ret[pos++] = ret[i];
	ret[pos] = 0;
	return CharLower(ret);
}

void AutoReplaceMap::add(const TCHAR * aFrom, const TCHAR * to, BOOL useVariables)
{
	scoped_free<TCHAR> from = filterText(aFrom);

	m_replacements[from.get()] = AutoReplacement(to, useVariables);

	writeAutoReplaceMap();
}

void AutoReplaceMap::copyMap(map<tstring, AutoReplacement> *replacements)
{
	*replacements = m_replacements;
}

void AutoReplaceMap::setMap(const map<tstring, AutoReplacement> &replacements)
{
	m_replacements.clear();

	map<tstring, AutoReplacement>::const_iterator it = replacements.begin();
	for (; it != replacements.end(); it++) {
		scoped_free<TCHAR> from = filterText(it->first.c_str());
		m_replacements[from.get()] = it->second;
	}

	writeAutoReplaceMap();
}
