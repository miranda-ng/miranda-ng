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

#define APPPATH  _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s")
#define MUICACHE _T("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache")


// Additional languages that i could not find in Windows
struct {
	TCHAR *language;
	TCHAR *localized_name;
} aditionalLanguages[] = {
	{ _T("tl_PH"), LPGENT("Tagalog (Philippines)") },
	{ _T("de_frami_neu"), LPGENT("German (Germany)") }
};

struct {
	TCHAR *name;
	TCHAR *key;
} otherHunspellApps[] = {
	{ _T("Thunderbird"), _T("thunderbird.exe") },
	{ _T("Firefox"), _T("firefox.exe") }
};

struct {
	char *name;
	UINT codepage;
} codepages[] = {
	{ "ISO8859-1", 28591 },
	{ "UTF-7", CP_UTF7 },
	{ "UTF-8", CP_UTF8 },
	{ "UTF7", CP_UTF7 },
	{ "UTF8", CP_UTF8 },
	{ "ISO8859-2", 28592 },
	{ "ISO8859-3", 28593 },
	{ "ISO8859-4", 28594 },
	{ "ISO8859-5", 28595 },
	{ "ISO8859-6", 28596 },
	{ "ISO8859-7", 28597 },
	{ "ISO8859-8", 28598 },
	{ "ISO8859-9", 28599 },
	{ "ASMO-708", 708 },
	{ "DOS-720", 720 },
	{ "iso-8859-6", 28596 },
	{ "arabic", 28596 },
	{ "csISOLatinArabic", 28596 },
	{ "ECMA-114", 28596 },
	{ "ISO_8859-6", 28596 },
	{ "ISO_8859-6:1987", 28596 },
	{ "iso-ir-127", 28596 },
	{ "x-mac-arabic", 10004 },
	{ "windows-1256", 1256 },
	{ "cp1256", 1256 },
	{ "ibm775", 775 },
	{ "CP500", 775 },
	{ "iso-8859-4", 28594 },
	{ "csISOLatin4", 28594 },
	{ "ISO_8859-4", 28594 },
	{ "ISO_8859-4:1988", 28594 },
	{ "iso-ir-110", 28594 },
	{ "l4", 28594 },
	{ "latin4", 28594 },
	{ "windows-1257", 1257 },
	{ "ibm852", 852 },
	{ "cp852", 852 },
	{ "iso-8859-2", 28592 },
	{ "csISOLatin2", 28592 },
	{ "iso_8859-2", 28592 },
	{ "iso_8859-2:1987", 28592 },
	{ "iso-ir-101", 28592 },
	{ "l2", 28592 },
	{ "latin2", 28592 },
	{ "x-mac-ce", 10029 },
	{ "windows-1250", 1250 },
	{ "x-cp1250", 1250 },
	{ "EUC-CN", 51936 },
	{ "x-euc-cn", 51936 },
	{ "gb2312", 936 },
	{ "chinese", 936 },
	{ "CN-GB", 936 },
	{ "csGB2312", 936 },
	{ "csGB231280", 936 },
	{ "csISO58GB231280", 936 },
	{ "GB_2312-80", 936 },
	{ "GB231280", 936 },
	{ "GB2312-80", 936 },
	{ "GBK", 936 },
	{ "iso-ir-58", 936 },
	{ "hz-gb-2312", 52936 },
	{ "x-mac-chinesesimp", 10008 },
	{ "big5", 950 },
	{ "cn-big5", 950 },
	{ "csbig5", 950 },
	{ "x-x-big5", 950 },
	{ "x-Chinese-CNS", 20000 },
	{ "x-Chinese-Eten", 20002 },
	{ "x-mac-chinesetrad", 10002 },
	{ "cp866", 866 },
	{ "ibm866", 866 },
	{ "iso-8859-5", 28595 },
	{ "csISOLatin5", 28595 },
	{ "csISOLatinCyrillic", 28595 },
	{ "cyrillic", 28595 },
	{ "ISO_8859-5", 28595 },
	{ "ISO_8859-5:1988", 28595 },
	{ "iso-ir-144", 28595 },
	{ "l5", 28595 },
	{ "KOI8-R", 20866 },
	{ "csKOI8R", 20866 },
	{ "koi", 20866 },
	{ "koi8", 20866 },
	{ "koi8r", 20866 },
	{ "KOI8-U", 21866 },
	{ "koi8-ru", 21866 },
	{ "x-mac-cyrillic", 10007 },
	{ "windows-1251", 1251 },
	{ "Win1251", 1251 },
	{ "x-cp1251", 1251 },
	{ "x-Europa", 29001 },
	{ "x-IA5-German", 20106 },
	{ "ibm737", 737 },
	{ "iso-8859-7", 28597 },
	{ "csISOLatinGreek", 28597 },
	{ "ECMA-118", 28597 },
	{ "ELOT_928", 28597 },
	{ "greek", 28597 },
	{ "greek8", 28597 },
	{ "ISO_8859-7", 28597 },
	{ "ISO_8859-7:1987", 28597 },
	{ "iso-ir-126", 28597 },
	{ "x-mac-greek", 10006 },
	{ "windows-1253", 1253 },
	{ "ibm869", 869 },
	{ "DOS-862", 862 },
	{ "iso-8859-8-i", 38598 },
	{ "logical", 38598 },
	{ "iso-8859-8", 28598 },
	{ "csISOLatinHebrew", 28598 },
	{ "hebrew", 28598 },
	{ "ISO_8859-8", 28598 },
	{ "ISO_8859-8:1988", 28598 },
	{ "ISO-8859-8", 28598 },
	{ "iso-ir-138", 28598 },
	{ "visual", 28598 },
	{ "x-mac-hebrew", 10005 },
	{ "windows-1255", 1255 },
	{ "ISO_8859-8-I", 1255 },
	{ "ISO-8859-8", 1255 },
	{ "x-EBCDIC-Arabic", 20420 },
	{ "x-EBCDIC-CyrillicRussian", 20880 },
	{ "x-EBCDIC-CyrillicSerbianBulgarian", 21025 },
	{ "x-EBCDIC-DenmarkNorway", 20277 },
	{ "x-ebcdic-denmarknorway-euro", 1142 },
	{ "x-EBCDIC-FinlandSweden", 20278 },
	{ "x-ebcdic-finlandsweden-euro", 1143 },
	{ "X-EBCDIC-France", 1143 },
	{ "X-EBCDIC-France", 1143 },
	{ "x-ebcdic-france-euro", 1147 },
	{ "x-EBCDIC-Germany", 20273 },
	{ "x-ebcdic-germany-euro", 1141 },
	{ "x-EBCDIC-GreekModern", 875 },
	{ "x-EBCDIC-Greek", 20423 },
	{ "x-EBCDIC-Hebrew", 20424 },
	{ "x-EBCDIC-Icelandic", 20871 },
	{ "x-ebcdic-icelandic-euro", 1149 },
	{ "x-ebcdic-international-euro", 1148 },
	{ "x-EBCDIC-Italy", 20280 },
	{ "x-ebcdic-italy-euro", 1144 },
	{ "x-EBCDIC-JapaneseAndKana", 50930 },
	{ "x-EBCDIC-JapaneseAndJapaneseLatin", 50939 },
	{ "x-EBCDIC-JapaneseAndUSCanada", 50931 },
	{ "x-EBCDIC-JapaneseKatakana", 20290 },
	{ "x-EBCDIC-KoreanAndKoreanExtended", 50933 },
	{ "x-EBCDIC-KoreanExtended", 20833 },
	{ "CP870", 870 },
	{ "x-EBCDIC-SimplifiedChinese", 50935 },
	{ "X-EBCDIC-Spain", 20284 },
	{ "x-ebcdic-spain-euro", 1145 },
	{ "x-EBCDIC-Thai", 20838 },
	{ "x-EBCDIC-TraditionalChinese", 50937 },
	{ "CP1026", 1026 },
	{ "x-EBCDIC-Turkish", 20905 },
	{ "x-EBCDIC-UK", 20285 },
	{ "x-ebcdic-uk-euro", 1146 },
	{ "ebcdic-cp-us", 37 },
	{ "x-ebcdic-cp-us-euro", 1140 },
	{ "ibm861", 861 },
	{ "x-mac-icelandic", 10079 },
	{ "x-iscii-as", 57006 },
	{ "x-iscii-be", 57003 },
	{ "x-iscii-de", 57002 },
	{ "x-iscii-gu", 57010 },
	{ "x-iscii-ka", 57008 },
	{ "x-iscii-ma", 57009 },
	{ "x-iscii-or", 57007 },
	{ "x-iscii-pa", 57011 },
	{ "x-iscii-ta", 57004 },
	{ "x-iscii-te", 57005 },
	{ "euc-jp", 51932 },
	{ "csEUCPkdFmtJapanese", 51932 },
	{ "Extended_UNIX_Code_Packed_Format_for_Japanese", 51932 },
	{ "x-euc", 51932 },
	{ "x-euc-jp", 51932 },
	{ "iso-2022-jp", 50220 },
	{ "iso-2022-jp", 50222 },
	{ "_iso-2022-jp$SIO", 50222 },
	{ "csISO2022JP", 50221 },
	{ "_iso-2022-jp", 50221 },
	{ "x-mac-japanese", 10001 },
	{ "shift_jis", 932 },
	{ "csShiftJIS", 932 },
	{ "csWindows31J", 932 },
	{ "ms_Kanji", 932 },
	{ "shift-jis", 932 },
	{ "x-ms-cp932", 932 },
	{ "x-sjis", 932 },
	{ "ks_c_5601-1987", 949 },
	{ "csKSC56011987", 949 },
	{ "euc-kr", 949 },
	{ "iso-ir-149", 949 },
	{ "korean", 949 },
	{ "ks_c_5601", 949 },
	{ "ks_c_5601_1987", 949 },
	{ "ks_c_5601-1989", 949 },
	{ "KSC_5601", 949 },
	{ "KSC5601", 949 },
	{ "euc-kr", 51949 },
	{ "csEUCKR", 51949 },
	{ "iso-2022-kr", 50225 },
	{ "csISO2022KR", 50225 },
	{ "Johab", 1361 },
	{ "x-mac-korean", 10003 },
	{ "iso-8859-3", 28593 },
	{ "csISO", 28593 },
	{ "Latin3", 28593 },
	{ "ISO_8859-3", 28593 },
	{ "ISO_8859-3:1988", 28593 },
	{ "iso-ir-109", 28593 },
	{ "l3", 28593 },
	{ "latin3", 28593 },
	{ "iso-8859-15", 28605 },
	{ "csISO", 28605 },
	{ "Latin9", 28605 },
	{ "ISO_8859-15", 28605 },
	{ "l9", 28605 },
	{ "latin9", 28605 },
	{ "x-IA5-Norwegian", 20108 },
	{ "IBM437", 437 },
	{ "437", 437 },
	{ "cp437", 437 },
	{ "csPC8", 437 },
	{ "CodePage437", 437 },
	{ "x-IA5-Swedish", 20107 },
	{ "windows-874", 874 },
	{ "DOS-874", 874 },
	{ "iso-8859-11", 874 },
	{ "TIS-620", 874 },
	{ "ibm857", 857 },
	{ "iso-8859-9", 28599 },
	{ "csISO", 28599 },
	{ "Latin5", 28599 },
	{ "ISO_8859-9", 28599 },
	{ "ISO_8859-9:1989", 28599 },
	{ "iso-ir-148", 28599 },
	{ "l5", 28599 },
	{ "latin5", 28599 },
	{ "x-mac-turkish", 10081 },
	{ "windows-1254", 1254 },
	{ "ISO_8859-9", 1254 },
	{ "ISO_8859-9:1989", 1254 },
	{ "iso-8859-9", 1254 },
	{ "iso-ir-148", 1254 },
	{ "latin5", 1254 },
	{ "unicode", 1200 },
	{ "utf-16", 1200 },
	{ "unicodeFFFE", 1201 },
	{ "utf-7", 65000 },
	{ "csUnicode11UTF7", 65000 },
	{ "unicode-1-1-utf-7", 65000 },
	{ "x-unicode-2-0-utf-7", 65000 },
	{ "utf-8", 65001 },
	{ "unicode-1-1-utf-8", 65001 },
	{ "unicode-2-0-utf-8", 65001 },
	{ "x-unicode-2-0-utf-8", 65001 },
	{ "us-ascii", 20127 },
	{ "ANSI_X3.4-1968", 20127 },
	{ "ANSI_X3.4-1986", 20127 },
	{ "ascii", 20127 },
	{ "cp367", 20127 },
	{ "csASCII", 20127 },
	{ "IBM367", 20127 },
	{ "ISO_646.irv:1991", 20127 },
	{ "ISO646-US", 20127 },
	{ "iso-ir-6us", 20127 },
	{ "windows-1258", 1258 },
	{ "ibm850", 850 },
	{ "x-IA5", 20105 },
	{ "iso-8859-1", 28591 },
	{ "cp819", 28591 },
	{ "csISO", 28591 },
	{ "Latin1", 28591 },
	{ "ibm819", 28591 },
	{ "iso_8859-1", 28591 },
	{ "iso_8859-1:1987", 28591 },
	{ "iso-ir-100", 28591 },
	{ "l1", 28591 },
	{ "latin1", 28591 },
	{ "macintosh", 10000 },
	{ "Windows-1252", 1252 },
	{ "ANSI_X3.4-1968", 1252 },
	{ "ANSI_X3.4-1986", 1252 },
	{ "ascii", 1252 },
	{ "cp367", 1252 },
	{ "cp819", 1252 },
	{ "csASCII", 1252 },
	{ "IBM367", 1252 },
	{ "ibm819", 1252 },
	{ "ISO_646.irv:1991", 1252 },
	{ "iso_8859-1", 1252 },
	{ "iso_8859-1:1987", 1252 },
	{ "ISO646-US", 1252 },
	{ "iso-ir-100", 1252 },
	{ "iso-ir-6", 1252 },
	{ "latin1", 1252 },
	{ "us", 1252 },
	{ "us-ascii", 1252 },
	{ "x-ansi", 1252 },
	{ "microsoft-cp1251", 1251 }
};

#define LANGUAGE_NOT_LOADED		 1
#define LANGUAGE_LOADING		-1
#define LANGUAGE_LOADED			 0

void LoadThread(LPVOID hd);

class HunspellDictionary : public Dictionary
{
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
		mir_sntprintf(filename, SIZEOF(filename), _T("%s\\%s.cdic"), userPath, language);

		FILE *file = _tfopen(filename, _T("rb"));
		if (file != NULL) {
			char tmp[1024];
			char c;
			int pos = 0;
			while ((c = fgetc(file)) != EOF) {
				if (c == '\n' || c == '\r' || pos >= SIZEOF(tmp) - 1) {
					if (pos > 0) {
						tmp[pos] = '\0';
						hunspell->add(tmp);
					}

					pos = 0;
				}
				else {
					tmp[pos] = c;
					pos++;
				}
			}
			fclose(file);
		}
	}

	void appendToCustomDict(const TCHAR *word)
	{
		CreateDirectoryTreeT(userPath);

		TCHAR filename[1024];
		mir_sntprintf(filename, SIZEOF(filename), _T("%s\\%s.cdic"), userPath, language);

		FILE *file = _tfopen(filename, _T("ab"));
		if (file != NULL) {
			char tmp[1024];
			toHunspell(tmp, word, SIZEOF(tmp));
			fprintf(file, "%s\n", tmp);
			fclose(file);
		}
	}

	virtual void addWordInternal(const TCHAR * word)
	{
		if (loaded != LANGUAGE_LOADED)
			return;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, SIZEOF(hunspell_word));

		hunspell->add(hunspell_word);
	}

	void toHunspell(char *hunspellWord, const TCHAR *word, size_t hunspellWordLen)
	{
		WideCharToMultiByte(codePage, 0, word, -1, hunspellWord, (int)hunspellWordLen, NULL, NULL);
	}

	TCHAR* fromHunspell(const char *hunspellWord)
	{
		int len = MultiByteToWideChar(codePage, 0, hunspellWord, -1, NULL, 0);
		WCHAR *ret = (WCHAR *)malloc((len + 1) * sizeof(WCHAR));
		MultiByteToWideChar(codePage, 0, hunspellWord, -1, ret, len + 1);
		return ret;
	}

	TCHAR* fromHunspellAndFree(char *hunspellWord)
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
		mir_tstrncpy(language, aLanguage, SIZEOF(language));
		mir_tstrncpy(fileWithoutExtension, aFileWithoutExtension, SIZEOF(fileWithoutExtension));
		mir_tstrncpy(userPath, anUserPath, SIZEOF(userPath));
		if (aSource == NULL)
			source[0] = _T('\0');
		else
			mir_tstrncpy(source, aSource, SIZEOF(source));

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
		int len1 = (s1 == NULL ? 0 : mir_tstrlen(s1));
		int len2 = (s2 == NULL ? 0 : mir_tstrlen(s2));

		TCHAR *ret;
		if (len1 > 0 && len2 > 0) {
			ret = (TCHAR *)malloc(sizeof(TCHAR) * (len1 + len2 + 1));
			mir_tstrncpy(ret, s1, len1 + 1);
			mir_tstrncpy(&ret[len1], s2, len2 + 1);

			FREE(s1);
			FREE(s2);
		}
		else if (len1 > 0) {
			ret = s1;
			FREE(s2);
		}
		else if (len2 > 0) {
			ret = s2;
			FREE(s1);
		}
		else {
			ret = (TCHAR *)malloc(sizeof(TCHAR));
			ret[0] = 0;

			FREE(s1);
			FREE(s2);
		}

		// Remove duplicated chars
		int last = mir_tstrlen(ret) - 1;
		for (int i = 0; i <= last; i++) {
			TCHAR c = ret[i];
			for (int j = last; j > i; j--) {
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


		mir_snprintf(dic, SIZEOF(dic), "%S.dic", fileWithoutExtension);
		mir_snprintf(aff, SIZEOF(aff), "%S.aff", fileWithoutExtension);


		hunspell = new Hunspell(aff, dic);

		// Get codepage
		const char *dic_enc = hunspell->get_dic_encoding();

		TCHAR *hwordchars;
		if (strcmp(dic_enc, "UTF-8") == 0) {
			codePage = CP_UTF8;


			int wcs_len;
			hwordchars = fromHunspell((char *)hunspell->get_wordchars_utf16(&wcs_len));

		}
		else {
			for (int i = 0; i < SIZEOF(codepages); i++) {
				if (_strcmpi(codepages[i].name, dic_enc) == 0) {
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
		toHunspell(hunspell_word, word, SIZEOF(hunspell_word));

		return hunspell->spell(hunspell_word);
	}

	// Return a list of suggestions to a word
	virtual Suggestions suggest(const TCHAR * word)
	{
		Suggestions ret = { 0 };

		load();
		if (loaded != LANGUAGE_LOADED)
			return ret;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, SIZEOF(hunspell_word));

		char ** words = NULL;
		ret.count = hunspell->suggest(&words, hunspell_word);

		if (ret.count > 0 && words != NULL) {
			// Oki, lets make our array
			ret.words = (TCHAR **)malloc(ret.count * sizeof(TCHAR *));
			for (unsigned i = 0; i < ret.count; i++) {
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
		Suggestions ret = { 0 };

		load();
		if (loaded != LANGUAGE_LOADED)
			return ret;

		char hunspell_word[1024];
		toHunspell(hunspell_word, word, SIZEOF(hunspell_word));

		char ** words;
		int count = hunspell->suggest_auto(&words, hunspell_word);

		if (count <= 0)
			return ret;

		// Oki, lets make our array
		ret.count = count;
		ret.words = (TCHAR **)malloc(ret.count * sizeof(TCHAR *));
		for (int i = 0; i < count; i++) {
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
		toHunspell(hunspell_word, word, SIZEOF(hunspell_word));

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

		return _tcschr(wordChars, (_TINT)c) != NULL;
	}

	// Assert that all needed data is loaded
	virtual void load()
	{
		if (loaded == LANGUAGE_NOT_LOADED) {
			loaded = LANGUAGE_LOADING;
			mir_forkthread(LoadThread, this);
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

void LoadThread(LPVOID hd)
{
	HunspellDictionary *dict = (HunspellDictionary *)hd;
	dict->loadThread();
}



// To use with EnumLocalesProc :(
LIST<Dictionary> *tmp_dicts;

// To get the names of the languages
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	TCHAR *stopped = NULL;
	USHORT langID = (USHORT)_tcstol(lpLocaleString, &stopped, 16);

	TCHAR ini[32];
	TCHAR end[32];
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO639LANGNAME, ini, SIZEOF(ini));
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO3166CTRYNAME, end, SIZEOF(end));

	TCHAR name[64];
	mir_sntprintf(name, SIZEOF(name), _T("%s_%s"), ini, end);

	for (int i = 0; i < tmp_dicts->getCount(); i++) {
		Dictionary *dict = (*tmp_dicts)[i];
		if (lstrcmpi(dict->language, name) == 0) {
			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGLANGUAGE, dict->english_name, SIZEOF(dict->english_name));

			GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLANGUAGE, dict->localized_name, SIZEOF(dict->localized_name));
			if (dict->localized_name[0] == 0)
				GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SLOCALIZEDLANGUAGENAME, dict->localized_name, SIZEOF(dict->localized_name));
			if (dict->localized_name[0] == 0)
				GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SNATIVEDISPLAYNAME, dict->localized_name, SIZEOF(dict->localized_name));
			if (dict->localized_name[0] == 0 && dict->english_name[0] != 0) {
				TCHAR country[1024];
				GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SENGCOUNTRY, country, SIZEOF(country));

				TCHAR name[1024];
				if (country[0] != 0)
					mir_sntprintf(name, SIZEOF(name), _T("%s (%s)"), dict->english_name, country);
				else
					mir_tstrncpy(name, dict->english_name, SIZEOF(name));

				mir_tstrncpy(dict->localized_name, TranslateTS(name), SIZEOF(dict->localized_name));
			}

			if (dict->localized_name[0] != 0) {
				mir_sntprintf(dict->full_name, SIZEOF(dict->full_name), _T("%s [%s]"), dict->localized_name, dict->language);
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
	for (int i = 0; i < dicts.getCount(); i++) {
		Dictionary *dict = dicts[i];

		if (dict->full_name[0] == _T('\0')) {
			DBVARIANT dbv;

			char lang[128];
			WideCharToMultiByte(CP_ACP, 0, dict->language, -1, lang, sizeof(lang), NULL, NULL);
			if (!db_get_ts(NULL, MODULE_NAME, lang, &dbv)) {
				mir_tstrncpy(dict->localized_name, dbv.ptszVal, SIZEOF(dict->localized_name));
				db_free(&dbv);
			}

			if (dict->localized_name[0] == _T('\0')) {
				for (size_t j = 0; j < SIZEOF(aditionalLanguages); j++) {
					if (!lstrcmp(aditionalLanguages[j].language, dict->language)) {
						mir_tstrncpy(dict->localized_name, TranslateTS(aditionalLanguages[j].localized_name), SIZEOF(dict->localized_name));
						break;
					}
				}
			}

			if (dict->localized_name[0] != _T('\0')) {
				mir_sntprintf(dict->full_name, SIZEOF(dict->full_name), _T("%s [%s]"), dict->localized_name, dict->language);
			}
			else {
				mir_tstrncpy(dict->full_name, dict->language, SIZEOF(dict->full_name));
			}
		}
	}
}


void GetHunspellDictionariesFromFolder(LIST<Dictionary> &dicts, TCHAR *path, TCHAR *user_path, TCHAR *source)
{
	// Load the language files and create an array with then
	TCHAR file[1024];
	mir_sntprintf(file, SIZEOF(file), _T("%s\\*.dic"), path);

	BOOL found = FALSE;

	WIN32_FIND_DATA ffd = { 0 };
	HANDLE hFFD = FindFirstFile(file, &ffd);
	if (hFFD != INVALID_HANDLE_VALUE) {
		do {
			mir_sntprintf(file, SIZEOF(file), _T("%s\\%s"), path, ffd.cFileName);

			// Check .dic
			DWORD attrib = GetFileAttributes(file);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			// See if .aff exists too
			mir_tstrcpy(&file[mir_tstrlen(file) - 4], _T(".aff"));
			attrib = GetFileAttributes(file);
			if (attrib == 0xFFFFFFFF || (attrib & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			ffd.cFileName[mir_tstrlen(ffd.cFileName) - 4] = _T('\0');

			TCHAR *lang = ffd.cFileName;

			// Replace - for _
			for (int i = 0; i < mir_tstrlen(lang); i++)
				if (lang[i] == _T('-'))
					lang[i] = _T('_');

			// Check if dict is new
			bool exists = false;
			for (int i = 0; i < dicts.getCount() && !exists; i++)
				if (lstrcmp(dicts[i]->language, lang) == 0)
					exists = true;

			if (!exists) {
				found = TRUE;
				file[mir_tstrlen(file) - 4] = _T('\0');
				dicts.insert(new HunspellDictionary(lang, file, user_path, source));
			}
		}
		while (FindNextFile(hFFD, &ffd));

		FindClose(hFFD);
	}
}


// Return a list of avaible languages
void GetAvaibleDictionaries(LIST<Dictionary> &dicts, TCHAR *path, TCHAR *user_path)
{
	// Get miranda folder dicts
	GetHunspellDictionariesFromFolder(dicts, path, user_path, NULL);

	if (opts.use_other_apps_dicts) {
		// Get other apps dicts
		for (int i = 0; i < SIZEOF(otherHunspellApps); i++) {
			TCHAR key[1024];
			mir_sntprintf(key, SIZEOF(key), APPPATH, otherHunspellApps[i].key);

			HKEY hKey = 0;
			LONG lResult = 0;
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &hKey)) {
				DWORD size = SIZEOF(key);
				lResult = RegQueryValueEx(hKey, _T("Path"), NULL, NULL, (LPBYTE)key, &size);
				RegCloseKey(hKey);
			}
			else {
				// Not found in installed apps - Try MUICache
				lResult = RegOpenKeyEx(HKEY_CURRENT_USER, MUICACHE, 0, KEY_QUERY_VALUE, &hKey);
				if (ERROR_SUCCESS == lResult) {
					DWORD numValues;
					if (ERROR_SUCCESS != RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &numValues, NULL, NULL, NULL, NULL))
						numValues = 0;

					lResult = ERROR_NO_MORE_ITEMS;
					for (DWORD local = 0; local < numValues; local++) {
						DWORD cchValue = SIZEOF(key);
						if (ERROR_SUCCESS != RegEnumValue(hKey, local, key, &cchValue, NULL, NULL, NULL, NULL))
							break;
						key[cchValue] = 0;
						TCHAR *pos;
						if (pos = _tcsrchr(key, _T('\\'))) {
							if (!lstrcmpi(&pos[1], otherHunspellApps[i].key)) {
								pos[0] = 0;
								lResult = ERROR_SUCCESS;
								break;
							}
						}
					}
					RegCloseKey(hKey);
				}
			}

			if (ERROR_SUCCESS == lResult) {
				TCHAR folder[1024];
				mir_sntprintf(folder, SIZEOF(folder), _T("%s\\Dictionaries"), key);

				GetHunspellDictionariesFromFolder(languages, folder, user_path, otherHunspellApps[i].name);
			}
		}
	}

	GetDictsInfo(dicts);

	// Yeah, yeah, yeah, I know, but this is the easiest way...
	SortedList *sl = (SortedList *)&dicts;

	// Sort dicts
	for (int i = 0; i < dicts.getCount(); i++) {
		for (int j = i + 1; j < dicts.getCount(); j++) {
			if (lstrcmp(dicts[i]->full_name, dicts[j]->full_name) > 0) {
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
		delete dicts[i];

	dicts.destroy();
}

Dictionary::~Dictionary()
{
	delete autoReplace;
}

// Free the list returned by GetAvaibleDictionaries
void FreeSuggestions(Suggestions &suggestions)
{
	for (size_t i = 0; i < suggestions.count; i++)
		free(suggestions.words[i]);

	free(suggestions.words);

	suggestions.words = NULL;
	suggestions.count = 0;
}
