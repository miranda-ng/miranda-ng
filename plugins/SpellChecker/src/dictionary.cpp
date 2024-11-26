/*
Copyright (C) 2006-2010 Ricardo Pescuma Domenecci

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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Dictionary class members

Dictionary::Dictionary(const wchar_t *aLanguage, const wchar_t *aSource)
{
	mir_wstrncpy(language, aLanguage, _countof(language));

	if (aSource == nullptr)
		source[0] = '\0';
	else
		mir_wstrncpy(source, aSource, _countof(source));

	localized_name[0] = '\0';
	english_name[0] = '\0';
	full_name[0] = '\0';
}

Dictionary::~Dictionary()
{
	delete autoReplace;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Additional languages that i could not find in Windows

struct
{
	wchar_t *language;
	wchar_t *localized_name;
}
static aditionalLanguages[] = {
	{ L"tl_PH", LPGENW("Tagalog (Philippines)") },
	{ L"de_frami_neu", LPGENW("German (Germany)") }
};

/////////////////////////////////////////////////////////////////////////////////////////
// To get the names of the languages

bool Dictionary::GetInfo()
{
	for (auto &it : g_plugin.locales) {
		if (mir_wstrcmpi(language, it.first.c_str()) == 0) {
			int langID = it.second;
			GetLocaleInfoW(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, english_name, _countof(english_name));

			GetLocaleInfoW(MAKELCID(langID, 0), LOCALE_SLANGUAGE, localized_name, _countof(localized_name));
			if (localized_name[0] == 0)
				GetLocaleInfoW(MAKELCID(langID, 0), LOCALE_SLOCALIZEDLANGUAGENAME, localized_name, _countof(localized_name));
			if (localized_name[0] == 0)
				GetLocaleInfoW(MAKELCID(langID, 0), LOCALE_SNATIVEDISPLAYNAME, localized_name, _countof(localized_name));
			if (localized_name[0] == 0 && english_name[0] != 0) {
				wchar_t country[1024];
				GetLocaleInfoW(MAKELCID(langID, 0), LOCALE_SENGCOUNTRY, country, _countof(country));

				wchar_t localName[1024];
				if (country[0] != 0)
					mir_snwprintf(localName, L"%s (%s)", english_name, country);
				else
					mir_wstrncpy(localName, english_name, _countof(localName));

				mir_wstrncpy(localized_name, TranslateW(localName), _countof(localized_name));
			}

			mir_snwprintf(full_name, L"%s [%s]", localized_name, language);
			return true;
		}
	}

	DBVARIANT dbv;

	char lang[128];
	WideCharToMultiByte(CP_ACP, 0, language, -1, lang, sizeof(lang), nullptr, nullptr);
	if (!g_plugin.getWString(lang, &dbv)) {
		mir_wstrncpy(localized_name, dbv.pwszVal, _countof(localized_name));
		db_free(&dbv);
	}

	if (localized_name[0] == '\0') {
		for (auto &it : aditionalLanguages) {
			if (!mir_wstrcmp(it.language, language)) {
				mir_wstrncpy(localized_name, TranslateW(it.localized_name), _countof(localized_name));
				break;
			}
		}
	}

	if (localized_name[0] != '\0') {
		mir_snwprintf(full_name, L"%s [%s]", localized_name, language);
		return true;
	}

	mir_wstrncpy(full_name, language, _countof(full_name));
	return false;
}
