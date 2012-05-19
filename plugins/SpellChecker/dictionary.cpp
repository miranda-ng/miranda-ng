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


#include "commons.h"

#include "hunspell/config.h"
#include "hunspell/hunspell.hxx"
#include "hunspell/csutil.hxx"


#include "codepages.cpp"


DWORD WINAPI LoadThread(LPVOID hd);

// Additional languages that i could not find in Windows
TCHAR *aditionalLanguages[] = {
	_T("tl_PH"), _T("Tagalog (Philippines)"),
	_T("de_frami_neu"), _T("German (Germany)")
};



#define LANGUAGE_NOT_LOADED		 1
#define LANGUAGE_LOADING		-1
#define LANGUAGE_LOADED			 0



class HunspellDictionary : public Dictionary {
protected:
	TCHAR fileWithoutExtension[1024];
	TCHAR userPath[1024];
	volatile int loaded;
	Hunspell *hunspell;
	TCHAR *wordChars;
	UINT codePage;

	void loadCustomDict()
	{
		TCHAR filename[1024];
		mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.cdic"), userPath, language);

		FILE *file = _tfopen(filename, _T("rb"));
		if (file != NULL) 
		{
			char tmp[1024];
			char c;
			int pos = 0;
			while((c = fgetc(file)) != EOF) 
			{
				if (c == '\n' || c == '\r' || pos >= MAX_REGS(tmp) - 1) 
				{
					if (pos > 0)
					{
						tmp[pos] = '\0';
						hunspell->add(tmp);
					}

					pos = 0;
				}
				else
				{
					tmp[pos] = c;
					pos ++;
				}
			}
			fclose(file);
		}
	}

	void appendToCustomDict(const TCHAR *word)
	{
		CreatePath(userPath);

		TCHAR filename[1024];
		mir_sntprintf(filename, MAX_REGS(filename), _T("%s\\%s.cdic"), userPath, language);

		FILE *file = _tfopen(filename, _T("ab"));
		if (file != NULL) 
		{
			char tmp[1024];
			toHunspell(tmp, word, MAX_REGS(tmp));
			fprintf(file, "%s\n", tmp);
			fclose(file);
		}
	}

	virtual void addWordInternal(const TCHAR * word)
	{
		if (loaded != LANGUAGE_LOADED)
			return;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		hunspell->add(hunspell_word);
	}

	void toHunspell(char *hunspellWord, const TCHAR *word, size_t hunspellWordLen)
	{
#ifdef UNICODE
		WideCharToMultiByte(codePage, 0, word, -1, hunspellWord, hunspellWordLen, NULL, NULL);
#else
		// TODO
		strncpy(hunspellWord, word, hunspellWordLen);
#endif
	}

	TCHAR * fromHunspell(const char *hunspellWord)
	{
#ifdef UNICODE
		int len = MultiByteToWideChar(codePage, 0, hunspellWord, -1, NULL, 0);
		WCHAR *ret = (WCHAR *) malloc((len + 1) * sizeof(WCHAR));
		MultiByteToWideChar(codePage, 0, hunspellWord, -1, ret, len + 1);
		return ret;
#else
		// TODO
		return strdup(hunspellWord);
#endif
	}

	TCHAR * fromHunspellAndFree(char *hunspellWord)
	{
		if (hunspellWord == NULL)
			return NULL;
		
		TCHAR *ret = fromHunspell(hunspellWord);
		free(hunspellWord);
		return ret;
	}

public:
	HunspellDictionary(TCHAR *aLanguage, TCHAR *aFileWithoutExtension, TCHAR *anUserPath, TCHAR *aSource)
	{
		lstrcpyn(language, aLanguage, MAX_REGS(language));
		lstrcpyn(fileWithoutExtension, aFileWithoutExtension, MAX_REGS(fileWithoutExtension));
		lstrcpyn(userPath, anUserPath, MAX_REGS(userPath));
		if (aSource == NULL)
			source[0] = _T('\0');
		else
			lstrcpyn(source, aSource, MAX_REGS(source));

		loaded = LANGUAGE_NOT_LOADED;
		localized_name[0] = _T('\0');
		english_name[0] = _T('\0');
		full_name[0] = _T('\0');
		hunspell = NULL;
		wordChars = NULL;
		codePage = CP_ACP;
		autoReplace = NULL;
	}

	virtual ~HunspellDictionary()
	{
		if (hunspell != NULL)
			delete hunspell;
		if (wordChars != NULL)
			free(wordChars);
	}

	TCHAR * merge(TCHAR * s1, TCHAR *s2)
	{
		int len1 = (s1 == NULL ? 0 : lstrlen(s1));
		int len2 = (s2 == NULL ? 0 : lstrlen(s2));

		TCHAR *ret;
		if (len1 > 0 && len2 > 0) 
		{
			ret = (TCHAR *) malloc(sizeof(TCHAR) * (len1 + len2 + 1));
			lstrcpyn(ret, s1, len1+1);
			lstrcpyn(&ret[len1], s2, len2+1);

			FREE(s1);
			FREE(s2);
		}
		else if (len1 > 0)
		{
			ret = s1;
			FREE(s2);
		}
		else if (len2 > 0)
		{
			ret = s2;
			FREE(s1);
		}
		else
		{
			ret = (TCHAR *) malloc(sizeof(TCHAR));
			ret[0] = 0;

			FREE(s1);
			FREE(s2);
		}

		// Remove duplicated chars
		int last = lstrlen(ret) - 1;
		for(int i = 0; i <= last; i++)
		{
			TCHAR c = ret[i];
			for(int j = last; j > i; j--)
			{
				if (c != ret[j])
					continue;
				if (j != last)
					ret[j] = ret[last];
				ret[last] = _T('\0');
				last--;
			}
		}

		return ret;
	}


	void loadThread()
	{
		char dic[1024];
		char aff[1024];

#ifdef UNICODE
		mir_snprintf(dic, MAX_REGS(dic), "%S.dic", fileWithoutExtension);
		mir_snprintf(aff, MAX_REGS(aff), "%S.aff", fileWithoutExtension);
#else
		mir_snprintf(dic, MAX_REGS(dic), "%s.dic", fileWithoutExtension);
		mir_snprintf(aff, MAX_REGS(aff), "%s.aff", fileWithoutExtension);
#endif

		hunspell = new Hunspell(aff, dic);

		// Get codepage
		const char *dic_enc = hunspell->get_dic_encoding();

		TCHAR *hwordchars;
		if (strcmp(dic_enc, "UTF-8") == 0)
		{
			codePage = CP_UTF8;

#ifdef UNICODE
			int wcs_len;
			hwordchars = fromHunspell((char *) hunspell->get_wordchars_utf16(&wcs_len));
#else
			// No option
			hwordchars = NULL;
#endif
		}
		else
		{
			for (int i = 0; i < MAX_REGS(codepages); i++)
			{
				if (_strcmpi(codepages[i].name, dic_enc) == 0)
				{
					if (IsValidCodePage(codepages[i].codepage))
						codePage = codepages[i].codepage;
					break;
				}
			}
			
			hwordchars = fromHunspell(hunspell->get_wordchars());
		}

		TCHAR *casechars = fromHunspellAndFree(get_casechars(dic_enc));
		TCHAR *try_string = fromHunspellAndFree(hunspell->get_try_string());

		wordChars = merge(merge(casechars, hwordchars), try_string);

		// Make a suggestion to load hunspell internalls
		char ** words = NULL;
		int count = hunspell->suggest(&words, "asdf");
		for (int i = 0; i < count; i++)
			free(words[i]);
		if (words != NULL) 
			free(words);

		loadCustomDict();

		loaded = LANGUAGE_LOADED;
	}

	// Return TRUE if the word is correct
	virtual BOOL spell(const TCHAR *word)
	{
		load();
		if (loaded != LANGUAGE_LOADED)
			return TRUE;

		// TODO Check if it was generated by auto-replacement

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		return hunspell->spell(hunspell_word);
	}

	// Return a list of suggestions to a word
	virtual Suggestions suggest(const TCHAR * word)
	{
		Suggestions ret = {0};

		load();
		if (loaded != LANGUAGE_LOADED)
			return ret;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		char ** words = NULL;
		ret.count = hunspell->suggest(&words, hunspell_word);

		if (ret.count > 0)
		{
			// Oki, lets make our array
			ret.words = (TCHAR **) malloc(ret.count * sizeof(TCHAR *));
			for (unsigned i = 0; i < ret.count; i++)
			{
				ret.words[i] = fromHunspell(words[i]);
				free(words[i]);
			}
		}

		if (words != NULL)
			free(words);

		return ret;
	}

	// Return a list of auto suggestions to a word
	virtual Suggestions autoSuggest(const TCHAR * word)
	{
		Suggestions ret = {0};

		load();
		if (loaded != LANGUAGE_LOADED)
			return ret;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		char ** words;
		int count = hunspell->suggest_auto(&words, hunspell_word);

		if (count <= 0)
			return ret;

		// Oki, lets make our array
		ret.count = count;
		ret.words = (TCHAR **) malloc(ret.count * sizeof(TCHAR *));
		for (int i = 0; i < count; i++)
		{
			ret.words[i] = fromHunspell(words[i]);
			free(words[i]);
		}
		free(words);

		return ret;
	}

	// Return a list of auto suggestions to a word
	// You have to free the list AND each item
	virtual TCHAR * autoSuggestOne(const TCHAR * word)
	{
		load();
		if (loaded != LANGUAGE_LOADED)
			return NULL;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, MAX_REGS(hunspell_word));

		char ** words;
		int count = hunspell->suggest_auto(&words, hunspell_word);

		if (count <= 0)
			return NULL;

		TCHAR *ret = fromHunspell(words[0]);

		// Oki, lets make our array
		for (int i = 0; i < count; i++)
			free(words[i]);
		free(words);

		return ret;
	}

	// Return TRUE if the char is a word char
	virtual BOOL isWordChar(TCHAR c)
	{
		if (c == 0)
			return FALSE;

		load();
		if (loaded != LANGUAGE_LOADED)
			return TRUE;

		return _tcschr(wordChars, (_TINT) c) != NULL;
	}

	// Assert that all needed data is loaded
	virtual void load()	
	{
		if (loaded == LANGUAGE_NOT_LOADED)
		{
			loaded = LANGUAGE_LOADING;

			DWORD thread_id;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) LoadThread, 
				(LPVOID) this, 0, &thread_id);
		}
	}

	virtual BOOL isLoaded()
	{
		return loaded == LANGUAGE_LOADED;
	}


	// Add a word to the user custom dict
	virtual void addWord(const TCHAR * word)
	{
		addWordInternal(word);
		appendToCustomDict(word);
	}
	
	// Add a word to the list of ignored words
	virtual void ignoreWord(const TCHAR * word)
	{
		addWordInternal(word);
	}
};


DWORD WINAPI LoadThread(LPVOID hd)
{
	HunspellDictionary *dict = (HunspellDictionary *) hd;
	dict->loadThread();
	return 0;
}



// To use with EnumLocalesProc :(
LIST<Dictionary> *tmp_dicts;

// To get the names of the languages
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	TCHAR *stopped = NULL;
	USHORT langID = (USHORT) _tcstol(lpLocaleString, &stopped, 16);

	TCHAR ini[32];
	TCHAR end[32];
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO639LANGNAME, ini, MAX_REGS(ini));
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO3166CTRYNAME, end, MAX_REGS(end));

	TCHAR name[64];
	mir_sntprintf(name, MAX_REGS(name), _T("%s_%s"), ini, end);

	for(int i = 0; i < tmp_dicts->getCount(); i++)
	{
		Dictionary *dict = (*tmp_dicts)[i];
		if (lstrcmpi(dict->language, name) == 0)
		{
#define LOCALE_SLOCALIZEDLANGUAGENAME 0x0000006f
#define LOCALE_SNATIVEDISPLAYNAME 0x00000073
			
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, dict->english_name, MAX_REGS(dict->english_name));

			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLANGUAGE, dict->localized_name, MAX_REGS(dict->localized_name));
			if (dict->localized_name[0] == 0)
				GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLOCALIZEDLANGUAGENAME, dict->localized_name, MAX_REGS(dict->localized_name));
			if (dict->localized_name[0] == 0)
				GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SNATIVEDISPLAYNAME, dict->localized_name, MAX_REGS(dict->localized_name));
			if (dict->localized_name[0] == 0 && dict->english_name[0] != 0)
			{
				TCHAR country[1024];
				GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGCOUNTRY, country, MAX_REGS(country));

				TCHAR name[1024];
				if (country[0] != 0)
					mir_sntprintf(name, MAX_REGS(name), _T("%s (%s)"), dict->english_name, country);
				else
					lstrcpyn(name, dict->english_name, MAX_REGS(name));

				lstrcpyn(dict->localized_name, TranslateTS(name), MAX_REGS(dict->localized_name));
			}

			if (dict->localized_name[0] != 0)
			{
				mir_sntprintf(dict->full_name, MAX_REGS(dict->full_name), _T("%s [%s]"), dict->localized_name, dict->language);
			}
			break;
		}
	}
	return TRUE;
}


void GetDictsInfo(LIST<Dictionary> &dicts)
{
	tmp_dicts = &dicts;
	EnumSystemLocales(EnumLocalesProc, LCID_SUPPORTED);

	// Try to get name from DB
	for(int i = 0; i < dicts.getCount(); i++)
	{
		Dictionary *dict = dicts[i];
		
		if (dict->full_name[0] == _T('\0'))
		{
			DBVARIANT dbv;
#ifdef UNICODE
			char lang[128];
			WideCharToMultiByte(CP_ACP, 0, dict->language, -1, lang, sizeof(lang), NULL, NULL);
			if (!DBGetContactSettingTString(NULL, MODULE_NAME, lang, &dbv))
#else
			if (!DBGetContactSettingTString(NULL, MODULE_NAME, dict->language, &dbv))
#endif
			{
				lstrcpyn(dict->localized_name, dbv.ptszVal, MAX_REGS(dict->localized_name));
				DBFreeVariant(&dbv);
			}

			if (dict->localized_name[0] == _T('\0'))
			{
				for(size_t j = 0; j < MAX_REGS(aditionalLanguages); j+=2)
				{
					if (lstrcmp(aditionalLanguages[j], dict->language) == 0)
					{
						lstrcpyn(dict->localized_name, aditionalLanguages[j+1], MAX_REGS(dict->localized_name));
						break;
					}
				}
			}

			if (dict->localized_name[0] != _T('\0'))
			{
				mir_sntprintf(dict->full_name, MAX_REGS(dict->full_name), _T("%s [%s]"), dict->localized_name, dict->language);
			}
			else
			{
				lstrcpyn(dict->full_name, dict->language, MAX_REGS(dict->full_name));
			}
		}
	}
}


void GetHunspellDictionariesFromFolder(LIST<Dictionary> &dicts, TCHAR *path, TCHAR *user_path, TCHAR *source)
{
	// Load the language files and create an array with then
	TCHAR file[1024];
	mir_sntprintf(file, MAX_REGS(file), _T("%s\\*.dic"), path);

	BOOL found = FALSE;

	WIN32_FIND_DATA ffd = {0};
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE)
	{
		do
		{
			mir_sntprintf(file, MAX_REGS(file), _T("%s\\%s"), path, ffd.cFileName);

			// Check .dic
			DWORD attrib = GetFileAttributes(file);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// See if .aff exists too
			lstrcpy(&file[lstrlen(file) - 4], _T(".aff"));
			attrib = GetFileAttributes(file);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			ffd.cFileName[lstrlen(ffd.cFileName)-4] = _T('\0');

			TCHAR *lang = ffd.cFileName;

			// Replace - for _
			int i;
			for(i = 0; i < lstrlen(lang); i++)
				if (lang[i] == _T('-'))
					lang[i] = _T('_');

			// Check if dict is new
			BOOL exists = FALSE;
			for(i = 0; i < dicts.getCount() && !exists; i++)
				if (lstrcmp(dicts[i]->language, lang) == 0)
					exists = TRUE;

			if (!exists)
			{
				found = TRUE;
				file[lstrlen(file) - 4] = _T('\0');
				dicts.insert(new HunspellDictionary(lang, file, user_path, source));
			}
		}
		while(FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}
}


// Return a list of avaible languages
void GetAvaibleDictionaries(LIST<Dictionary> &dicts, TCHAR *path, TCHAR *user_path)
{
	// Get miranda folder dicts
	GetHunspellDictionariesFromFolder(dicts, path, user_path, NULL);

	if (opts.use_other_apps_dicts)
	{
		TCHAR *otherHunspellApps[] = { _T("Thunderbird"), _T("thunderbird.exe"), 
									   _T("Firefox"), _T("firefox.exe") };

#define APPPATH  _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s")
#define MUICACHE _T("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache")

		// Get other apps dicts
		for (int i = 0; i < MAX_REGS(otherHunspellApps); i += 2)
		{
			TCHAR key[1024];
			mir_sntprintf(key, MAX_REGS(key), APPPATH, otherHunspellApps[i+1]);

			HKEY hKey = 0;
			LONG lResult = 0;
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &hKey))
			{
				DWORD size = MAX_REGS(key);
				lResult = RegQueryValueEx(hKey, _T("Path"), NULL, NULL, (LPBYTE)key, &size);
				RegCloseKey(hKey);
			}
			else 
			{ 
				// Not found in installed apps - Try MUICache
				lResult = RegOpenKeyEx(HKEY_CURRENT_USER, MUICACHE, 0, KEY_QUERY_VALUE, &hKey);
				if (ERROR_SUCCESS == lResult)
				{
					DWORD numValues;
					if (ERROR_SUCCESS != RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &numValues, NULL, NULL, NULL, NULL)) 
						numValues = 0;

					lResult = ERROR_NO_MORE_ITEMS;
					for (DWORD local = 0; local < numValues; local++) 
					{
						DWORD cchValue = MAX_REGS(key);
						if (ERROR_SUCCESS != RegEnumValue(hKey, local, key, &cchValue, NULL, NULL, NULL, NULL)) 
							break;
						key[cchValue] = 0;
						TCHAR *pos;
						if (pos = _tcsrchr(key, _T('\\')))
						{
							if (lstrcmpi(&pos[1], otherHunspellApps[i+1]) == 0)
							{
								pos[0] = 0;
								lResult = ERROR_SUCCESS;
								break;
							}
						}
					}
					RegCloseKey(hKey);
				} 
			}

			if (ERROR_SUCCESS == lResult)
			{
				TCHAR folder[1024];
				mir_sntprintf(folder, MAX_REGS(folder), _T("%s\\Dictionaries"), key);

				GetHunspellDictionariesFromFolder(languages, folder, user_path, otherHunspellApps[i]);
			}       
		}
	}

	GetDictsInfo(dicts);

	// Yeah, yeah, yeah, I know, but this is the easiest way...
	SortedList *sl = (SortedList *) &dicts;

	// Sort dicts
	for(int i = 0; i < dicts.getCount(); i++)
	{
		for(int j = i + 1; j < dicts.getCount(); j++)
		{
			if (lstrcmp(dicts[i]->full_name, dicts[j]->full_name) > 0)
			{
				Dictionary *dict = dicts[i];
				sl->items[i] = dicts[j];
				sl->items[j] = dict;
			}
		}
	}
}


// Free the list returned by GetAvaibleDictionaries
void FreeDictionaries(LIST<Dictionary> &dicts)
{
	for (int i = 0; i < dicts.getCount(); i++)
	{
		delete dicts[i];
	}
	dicts.destroy();
}


// Free the list returned by GetAvaibleDictionaries
void FreeSuggestions(Suggestions &suggestions)
{
	for (size_t i = 0; i < suggestions.count; i++)
	{
		free(suggestions.words[i]);
	}
	free(suggestions.words);

	suggestions.words = NULL;
	suggestions.count = 0;
}
