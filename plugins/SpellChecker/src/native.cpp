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

struct NativeDictionary : public Dictionary
{
	CComPtr<ISpellChecker> m_speller;

	NativeDictionary(const wchar_t *wszLanguage) :
		Dictionary(wszLanguage, 0)
	{
		if (auto *p = wcschr(language, '-'))
			*p = '_';

		g_plugin.m_spellFactory->CreateSpellChecker(wszLanguage, &m_speller);

		if (!GetInfo())
			if (auto *p = wcschr(full_name, '_'))
				*p = '-';
	}

	virtual ~NativeDictionary()
	{
	}

	// Return TRUE if the word is correct
	virtual BOOL spell(const wchar_t *word)
	{
		CComPtr<IEnumSpellingError> error;
		if (FAILED(m_speller->Check(word, &error)))
			return FALSE;

		CComPtr<ISpellingError> err;
		if (FAILED(error->Next(&err)))
			return FALSE;

		return err == nullptr;
	}

	// Return a list of suggestions to a word
	virtual Suggestions suggest(const wchar_t *word)
	{
		Suggestions ret;

		CComPtr<IEnumString> suggestions;
		if (SUCCEEDED(m_speller->Suggest(word, &suggestions))) {
			wchar_t *ws;
			ULONG fetched;
			while (true) {
				suggestions->Next(1, &ws, &fetched);
				if (fetched == 0)
					break;
				ret.push_back(ws);
			}
		}
		return ret;
	}

	// Return a list of auto suggestions to a word
	// You have to free the list AND each item
	virtual wchar_t* autoSuggestOne(const wchar_t * word)
	{
		CComPtr<IEnumString> suggestions;
		if (SUCCEEDED(m_speller->Suggest(word, &suggestions))) {
			wchar_t *ws;
			ULONG fetched;
			suggestions->Next(1, &ws, &fetched);
			if (fetched) {
				wchar_t *p = (wchar_t *)malloc(sizeof(wchar_t) * (mir_wstrlen(ws) + 1));
				mir_wstrcpy(p, ws);
				return p;
			}
		}
		return nullptr;
	}

	// Return TRUE if the char is a word char
	virtual BOOL isWordChar(wchar_t c)
	{
		if (c == 0)
			return FALSE;

		return iswalpha(c);
	}

	// Assert that all needed data is loaded
	virtual void load()
	{
	}

	virtual BOOL isLoaded()
	{
		return TRUE;
	}

	// Add a word to the user custom dict
	virtual void addWord(const wchar_t *word)
	{
		m_speller->Add(word);
	}

	// Add a word to the list of ignored words
	virtual void ignoreWord(const wchar_t * word)
	{
		m_speller->Ignore(word);
	}
};

// Return a list of avaible languages
void GetNativeDictionaries(OBJLIST<Dictionary> &dicts)
{
	CComPtr<IEnumString> ptr;
	g_plugin.m_spellFactory->get_SupportedLanguages(&ptr);

	ULONG fetched;
	wchar_t *ws;
	while (true) {
		ptr->Next(1, &ws, &fetched);
		if (fetched == 0)
			break;

		auto *pNew = new NativeDictionary(ws);
		if (!mir_wstrcmpi(ws, pNew->full_name))
			delete pNew;
		else 
			dicts.insert(pNew);
	}
}
