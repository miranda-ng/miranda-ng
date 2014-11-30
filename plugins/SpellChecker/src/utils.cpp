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

typedef void(*FoundWrongWordCallback)(TCHAR *word, CHARRANGE pos, void *param);

typedef map<HWND, Dialog *> DialogMapType;
DialogMapType dialogs;
DialogMapType menus;

void SetUnderline(Dialog *dlg, int pos_start, int pos_end)
{
	dlg->re->SetSel(pos_start, pos_end);

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_UNDERLINE | CFM_UNDERLINETYPE;
	cf.dwEffects = CFE_UNDERLINE;
	cf.bUnderlineType = opts.underline_type + CFU_UNDERLINEDOUBLE;
	cf.bUnderlineColor = 0x05;
	dlg->re->SendMessage(EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);

	dlg->markedSomeWord = TRUE;
}

BOOL IsMyUnderline(const CHARFORMAT2 &cf)
{
	return (cf.dwEffects & CFE_UNDERLINE)
		&& (cf.bUnderlineType & 0x0F) >= CFU_UNDERLINEDOUBLE
		&& (cf.bUnderlineType & 0x0F) <= CFU_UNDERLINETHICK
		&& (cf.bUnderlineColor) == 5;
}

void SetNoUnderline(RichEdit *re, int pos_start, int pos_end)
{
	if (opts.handle_underscore) {
		for (int i = pos_start; i <= pos_end; i++) {
			re->SetSel(i, min(i + 1, pos_end));

			CHARFORMAT2 cf;
			cf.cbSize = sizeof(cf);
			re->SendMessage(EM_GETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);

			BOOL mine = IsMyUnderline(cf);
			if (mine) {
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_UNDERLINE | CFM_UNDERLINETYPE;
				cf.dwEffects = 0;
				cf.bUnderlineType = CFU_UNDERLINE;
				cf.bUnderlineColor = 0;
				re->SendMessage(EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
			}
		}
	}
	else {
		re->SetSel(pos_start, pos_end);

		CHARFORMAT2 cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_UNDERLINE | CFM_UNDERLINETYPE;
		cf.dwEffects = 0;
		cf.bUnderlineType = CFU_UNDERLINE;
		cf.bUnderlineColor = 0;
		re->SendMessage(EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
	}
}

void SetNoUnderline(Dialog *dlg)
{
	dlg->re->Stop();
	SetNoUnderline(dlg->re, 0, dlg->re->GetTextLength());
	dlg->markedSomeWord = FALSE;
	dlg->re->Start();
}

inline BOOL IsNumber(TCHAR c)
{
	return c >= _T('0') && c <= _T('9');
}

inline BOOL IsURL(TCHAR c)
{
	return (c >= _T('a') && c <= _T('z'))
		|| (c >= _T('A') && c <= _T('Z'))
		|| IsNumber(c)
		|| c == _T('.') || c == _T('/')
		|| c == _T('\\') || c == _T('?')
		|| c == _T('=') || c == _T('&')
		|| c == _T('%') || c == _T('-')
		|| c == _T('_') || c == _T(':')
		|| c == _T('@') || c == _T('#');
}

int FindURLEnd(Dialog *dlg, TCHAR *text, int start_pos, int *checked_until = NULL)
{
	int num_slashes = 0;
	int num_ats = 0;
	int num_dots = 0;

	int i = start_pos;

	for (; IsURL(text[i]) || dlg->lang->isWordChar(text[i]); i++) {
		TCHAR c = text[i];
		if (c == _T('\\') || c == _T('/'))
			num_slashes++;
		else if (c == _T('.'))
			num_dots++;
		else if (c == _T('@'))
			num_ats++;
	}

	if (checked_until != NULL)
		*checked_until = i;

	if (num_slashes <= 0 && num_ats <= 0 && num_dots <= 0)
		return -1;

	if (num_slashes == 0 && num_ats == 0 && num_dots < 2)
		return -1;

	if (i - start_pos < 2)
		return -1;

	return i;
}


int ReplaceWord(Dialog *dlg, CHARRANGE &sel, TCHAR *new_word)
{
	dlg->re->Stop();
	dlg->re->ResumeUndo();

	int dif = dlg->re->Replace(sel.cpMin, sel.cpMax, new_word);

	dlg->re->SuspendUndo();
	dlg->re->Start();

	return dif;
}

class TextParser
{
public:
	virtual ~TextParser() {}

	/// @return true when finished an word
	virtual bool feed(int pos, TCHAR c) = 0;
	virtual int getFirstCharPos() = 0;
	virtual void reset() = 0;
	virtual void deal(const TCHAR *text, bool *mark, bool *replace, TCHAR **replacement) = 0;
};

class SpellParser : public TextParser
{
	Dictionary *dict;
	int last_pos;
	BOOL found_real_char;

public:
	SpellParser(Dictionary *dict) : dict(dict)
	{
		reset();
	}

	void reset()
	{
		last_pos = -1;
		found_real_char = FALSE;
	}

	bool feed(int pos, TCHAR c)
	{
		// Is inside a word?
		if (dict->isWordChar(c) || IsNumber(c)) {
			if (last_pos == -1)
				last_pos = pos;

			if (c != _T('-') && !IsNumber(c))
				found_real_char = TRUE;

			return false;
		}

		if (!found_real_char)
			last_pos = -1;

		return (last_pos != -1);
	}

	int getFirstCharPos()
	{
		if (!found_real_char)
			return -1;
		else
			return last_pos;
	}

	void deal(const TCHAR *text, bool *mark, bool *replace, TCHAR **replacement)
	{
		// Is it correct?
		if (dict->spell(text))
			return;

		// Has to auto-correct?
		if (opts.auto_replace_dict) {
			*replacement = dict->autoSuggestOne(text);
			if (*replacement != NULL) {
				*replace = true;
				return;
			}
		}

		*mark = true;
	}
};

class AutoReplaceParser : public TextParser
{
	AutoReplaceMap *ar;
	int last_pos;

public:
	AutoReplaceParser(AutoReplaceMap *ar) : ar(ar)
	{
		reset();
	}

	void reset()
	{
		last_pos = -1;
	}

	bool feed(int pos, TCHAR c)
	{
		// Is inside a word?
		if (ar->isWordChar(c)) {
			if (last_pos == -1)
				last_pos = pos;
			return false;
		}

		return (last_pos != -1);
	}

	int getFirstCharPos()
	{
		return last_pos;
	}

	void deal(const TCHAR *text, bool *mark, bool *replace, TCHAR **replacement)
	{
		*replacement = ar->autoReplace(text);
		if (*replacement != NULL)
			*replace = true;
	}
};

int CheckTextLine(Dialog *dlg, int line, TextParser *parser,
						BOOL ignore_upper, BOOL ignore_with_numbers, BOOL test_urls,
						const CHARRANGE &ignored, FoundWrongWordCallback callback, void *param)
{
	int errors = 0;
	TCHAR text[1024];
	dlg->re->GetLine(line, text, SIZEOF(text));
	int len = mir_tstrlen(text);
	int first_char = dlg->re->GetFirstCharOfLine(line);

	// Now lets get the words
	int next_char_for_url = 0;
	for (int pos = 0; pos < len; pos++) {
		int url_end = pos;
		if (pos >= next_char_for_url) {
			url_end = FindURLEnd(dlg, text, pos, &next_char_for_url);
			next_char_for_url++;
		}

		if (url_end > pos) {
			BOOL ignore_url = FALSE;

			if (test_urls) {
				// All the url must be handled by the parser
				parser->reset();

				BOOL feed = FALSE;
				for (int j = pos; !feed && j <= url_end; j++)
					feed = parser->feed(j, text[j]);

				if (feed || parser->getFirstCharPos() != pos)
					ignore_url = TRUE;
			}
			else ignore_url = TRUE;

			pos = url_end;

			if (ignore_url) {
				parser->reset();
				continue;
			}
		}
		else {
			TCHAR c = text[pos];

			BOOL feed = parser->feed(pos, c);
			if (!feed) {
				if (pos >= len - 1)
					pos = len; // To check the last block
				else
					continue;
			}
		}

		int last_pos = parser->getFirstCharPos();
		parser->reset();

		if (last_pos < 0)
			continue;

		// We found a word
		CHARRANGE sel = { first_char + last_pos, first_char + pos };

		// Is in ignored range?
		if (sel.cpMin <= ignored.cpMax && sel.cpMax >= ignored.cpMin)
			continue;

		if (ignore_upper) {
			BOOL upper = TRUE;
			for (int i = last_pos; i < pos && upper; i++)
				upper = !IsCharLower(text[i]);
			if (upper)
				continue;
		}

		if (ignore_with_numbers) {
			BOOL hasNumbers = FALSE;
			for (int i = last_pos; i < pos && !hasNumbers; i++)
				hasNumbers = IsNumber(text[i]);
			if (hasNumbers)
				continue;
		}

		text[pos] = 0;

		bool mark = false;
		bool replace = false;
		TCHAR *replacement = NULL;
		parser->deal(&text[last_pos], &mark, &replace, &replacement);

		if (replace) {
			// Replace in rich edit
			int dif = dlg->re->Replace(sel.cpMin, sel.cpMax, replacement);
			if (dif != 0) {
				// Read line again
				dlg->re->GetLine(line, text, SIZEOF(text));
				len = mir_tstrlen(text);

				int old_first_char = first_char;
				first_char = dlg->re->GetFirstCharOfLine(line);

				pos = max(-1, pos + dif + old_first_char - first_char);
			}

			free(replacement);
		}
		else if (mark) {
			SetUnderline(dlg, sel.cpMin, sel.cpMax);

			if (callback != NULL)
				callback(&text[last_pos], sel, param);

			errors++;
		}
	}

	return errors;
}

// Checks for errors in all text
int CheckText(Dialog *dlg, BOOL check_all, FoundWrongWordCallback callback = NULL, void *param = NULL)
{
	int errors = 0;

	dlg->re->Stop();

	if (dlg->re->GetTextLength() > 0) {
		int lines = dlg->re->GetLineCount();
		int line = 0;
		CHARRANGE cur_sel = { -1, -1 };

		if (!check_all) {
			// Check only the current line, one up and one down
			int current_line = dlg->re->GetLineFromChar(dlg->re->GetSel().cpMin);
			line = max(line, current_line - 1);
			lines = min(lines, current_line + 2);
			cur_sel = dlg->re->GetSel();
		}

		for (; line < lines; line++) {
			int first_char = dlg->re->GetFirstCharOfLine(line);

			SetNoUnderline(dlg->re, first_char, first_char + dlg->re->GetLineLength(line));

			if (opts.auto_replace_user)
				errors += CheckTextLine(dlg, line, &AutoReplaceParser(dlg->lang->autoReplace),
				FALSE, FALSE, TRUE,
				cur_sel, callback, param);

			errors += CheckTextLine(dlg, line, &SpellParser(dlg->lang),
											opts.ignore_uppercase, opts.ignore_with_numbers, FALSE,
											cur_sel, callback, param);
		}
	}

	// Fix last char
	int len = dlg->re->GetTextLength();
	SetNoUnderline(dlg->re, len, len);

	dlg->re->Start();
	return errors;
}

void ToLocaleID(TCHAR *szKLName, size_t size)
{
	TCHAR *stopped = NULL;
	USHORT langID = (USHORT)_tcstol(szKLName, &stopped, 16);

	TCHAR ini[32], end[32];
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO639LANGNAME, ini, SIZEOF(ini));
	GetLocaleInfo(MAKELCID(langID, 0), LOCALE_SISO3166CTRYNAME, end, SIZEOF(end));

	mir_sntprintf(szKLName, size, _T("%s_%s"), ini, end);
}

void LoadDictFromKbdl(Dialog *dlg)
{
	TCHAR szKLName[KL_NAMELENGTH + 1];

	// Use default input language
	HKL hkl = GetKeyboardLayout(0);
	mir_sntprintf(szKLName, SIZEOF(szKLName), _T("%x"), (int)LOWORD(hkl));
	ToLocaleID(szKLName, SIZEOF(szKLName));

	int d = GetClosestLanguage(szKLName);
	if (d >= 0) {
		dlg->lang = languages[d];
		dlg->lang->load();

		if (dlg->srmm)
			ModifyIcon(dlg);
	}
}

int TimerCheck(Dialog *dlg, BOOL forceCheck = FALSE)
{
	KillTimer(dlg->hwnd, TIMER_ID);

	if (!dlg->enabled || dlg->lang == NULL)
		return -1;

	if (!dlg->lang->isLoaded()) {
		SetTimer(dlg->hwnd, TIMER_ID, 500, NULL);
		return -1;
	}

	// Don't check if field is read-only
	if (dlg->re->IsReadOnly())
		return -1;

	int len = dlg->re->GetTextLength();
	if (!forceCheck && len == dlg->old_text_len && !dlg->changed)
		return -1;

	dlg->old_text_len = len;
	dlg->changed = FALSE;

	return CheckText(dlg, TRUE);
}

LRESULT CALLBACK OwnerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogMapType::iterator dlgit = dialogs.find(hwnd);
	if (dlgit == dialogs.end())
		return -1;

	Dialog *dlg = dlgit->second;

	if (msg == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == 1624)) {
		if (opts.ask_when_sending_with_error) {
			int errors = TimerCheck(dlg, TRUE);
			if (errors > 0) {
				TCHAR text[500];
				mir_sntprintf(text, SIZEOF(text), TranslateT("There are %d spelling errors. Are you sure you want to send this message?"), errors);
				if (MessageBox(hwnd, text, TranslateT("Spell Checker"), MB_ICONQUESTION | MB_YESNO) == IDNO)
					return TRUE;
			}
		}
		else if (opts.auto_replace_dict || opts.auto_replace_user) {
			// Fix all
			TimerCheck(dlg);
		}

		if (dlg->markedSomeWord)
			// Remove underline
			SetNoUnderline(dlg);

		// Schedule to re-parse
		KillTimer(dlg->hwnd, TIMER_ID);
		SetTimer(dlg->hwnd, TIMER_ID, 100, NULL);

		dlg->changed = TRUE;
	}

	return mir_callNextSubclass(hwnd, OwnerProc, msg, wParam, lParam);
}

void ToggleEnabled(Dialog *dlg)
{
	dlg->enabled = !dlg->enabled;
	db_set_b(dlg->hContact, MODULE_NAME, dlg->name, dlg->enabled);

	if (!dlg->enabled)
		SetNoUnderline(dlg);
	else {
		dlg->changed = TRUE;
		SetTimer(dlg->hwnd, TIMER_ID, 100, NULL);
	}

	if (dlg->srmm)
		ModifyIcon(dlg);
}

LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogMapType::iterator dlgit = dialogs.find(hwnd);
	if (dlgit == dialogs.end())
		return -1;

	Dialog *dlg = dlgit->second;
	if (dlg == NULL)
		return -1;

	// Hotkey support
	MSG msgData = { 0 };
	msgData.hwnd = hwnd;
	msgData.message = msg;
	msgData.wParam = wParam;
	msgData.lParam = lParam;

	int action = CallService(MS_HOTKEY_CHECK, (WPARAM)&msgData, (LPARAM)"Spell Checker");
	if (action == HOTKEY_ACTION_TOGGLE) {
		ToggleEnabled(dlg);
		return 1;
	}

	LRESULT ret = mir_callNextSubclass(hwnd, EditProc, msg, wParam, lParam);
	if ((dlgit = dialogs.find(hwnd)) == dialogs.end())
		return ret;

	switch (msg) {
	case WM_KEYDOWN:
		if (wParam != VK_DELETE)
			break;

	case WM_CHAR:
		if (dlg->re->IsStopped())
			break;

		if (lParam & (1 << 28))	// ALT key
			break;

		if (GetKeyState(VK_CONTROL) & 0x8000)	// CTRL key
			break;

		{
			TCHAR c = (TCHAR)wParam;
			BOOL deleting = (c == VK_BACK || c == VK_DELETE);

			// Need to do that to avoid changing the word while typing
			KillTimer(hwnd, TIMER_ID);
			SetTimer(hwnd, TIMER_ID, 1000, NULL);

			dlg->changed = TRUE;

			if (!deleting && (lParam & 0xFF) > 1)	// Repeat rate
				break;

			if (!dlg->enabled || dlg->lang == NULL || !dlg->lang->isLoaded())
				break;

			// Don't check if field is read-only
			if (dlg->re->IsReadOnly())
				break;

			if (!deleting && !dlg->lang->isWordChar(c))
				CheckText(dlg, FALSE);
			else {
				// Remove underline of current word
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(cf);
				dlg->re->SendMessage(EM_GETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);

				if (IsMyUnderline(cf)) {
					dlg->re->Stop();

					CHARRANGE sel = dlg->re->GetSel();

					TCHAR text[1024];
					int first_char;
					GetWordCharRange(dlg, sel, text, SIZEOF(text), first_char);

					SetNoUnderline(dlg->re, sel.cpMin, sel.cpMax);

					dlg->re->Start();
				}
			}
		}
		break;

	case EM_REPLACESEL:
	case WM_SETTEXT:
	case EM_SETTEXTEX:
	case EM_PASTESPECIAL:
	case WM_PASTE:
		if (dlg->re->IsStopped())
			break;

		KillTimer(hwnd, TIMER_ID);
		SetTimer(hwnd, TIMER_ID, 100, NULL);

		dlg->changed = TRUE;
		break;

	case WM_TIMER:
		if (wParam == TIMER_ID)
			TimerCheck(dlg);
		break;

	case WMU_DICT_CHANGED:
		KillTimer(hwnd, TIMER_ID);
		SetTimer(hwnd, TIMER_ID, 100, NULL);

		dlg->changed = TRUE;
		break;

	case WMU_KBDL_CHANGED:
		if (opts.auto_locale) {
			KillTimer(hwnd, TIMER_ID);
			SetTimer(hwnd, TIMER_ID, 100, NULL);

			dlg->changed = TRUE;

			LoadDictFromKbdl(dlg);
		}
		break;

	case WM_INPUTLANGCHANGE:
		// Allow others to process this message and we get only the result
		PostMessage(hwnd, WMU_KBDL_CHANGED, 0, 0);
		break;
	}

	return ret;
}

int GetClosestLanguage(TCHAR *lang_name)
{
	int i;

	// Search the language by name
	for (i = 0; i < languages.getCount(); i++)
		if (lstrcmpi(languages[i]->language, lang_name) == 0)
			return i;

	// Try searching by the prefix only
	TCHAR lang[128];
	mir_tstrncpy(lang, lang_name, SIZEOF(lang));

	TCHAR *p = _tcschr(lang, _T('_'));
	if (p != NULL)
		*p = _T('\0');

	// First check if there is a language that is only the prefix
	for (i = 0; i < languages.getCount(); i++)
		if (lstrcmpi(languages[i]->language, lang) == 0)
			return i;

	// Now try any suffix
	size_t len = mir_tstrlen(lang);
	for (i = 0; i < languages.getCount(); i++) {
		TCHAR *p = _tcschr(languages[i]->language, _T('_'));
		if (p == NULL)
			continue;

		int prefix_len = p - languages[i]->language;
		if (prefix_len != len)
			continue;

		if (_tcsnicmp(languages[i]->language, lang_name, len) == 0)
			return i;
	}

	return -1;
}

void GetUserProtoLanguageSetting(Dialog *dlg, MCONTACT hContact, char *group, char *setting, BOOL isProtocol = TRUE)
{
	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_TCHAR;

	DBCONTACTGETSETTING cgs = { 0 };
	cgs.szModule = group;
	cgs.szSetting = setting;
	cgs.pValue = &dbv;

	INT_PTR rc;

	int caps = (isProtocol ? CallProtoService(group, PS_GETCAPS, PFLAGNUM_4, 0) : 0);
	if (caps & PF4_INFOSETTINGSVC)
		rc = CallProtoService(group, PS_GETINFOSETTING, hContact, (LPARAM)&cgs);
	else {
		rc = CallService(MS_DB_CONTACT_GETSETTING_STR_EX, hContact, (LPARAM)&cgs);
		if (rc == CALLSERVICE_NOTFOUND)
			rc = db_get_ts(hContact, group, setting, &dbv);
	}

	if (!rc && dbv.type == DBVT_TCHAR && dbv.ptszVal != NULL) {
		TCHAR *lang = dbv.ptszVal;

		for (int i = 0; i < languages.getCount(); i++) {
			Dictionary *dict = languages[i];
			if (lstrcmpi(dict->localized_name, lang) == 0
				 || lstrcmpi(dict->english_name, lang) == 0
				 || lstrcmpi(dict->language, lang) == 0) {
				mir_tstrncpy(dlg->lang_name, dict->language, SIZEOF(dlg->lang_name));
				break;
			}
		}
	}

	if (!rc)
		db_free(&dbv);
}

void GetUserLanguageSetting(Dialog *dlg, char *setting)
{
	char *proto = GetContactProto(dlg->hContact);
	if (proto == NULL)
		return;

	GetUserProtoLanguageSetting(dlg, dlg->hContact, proto, setting);
	if (dlg->lang_name[0] != _T('\0'))
		return;

	GetUserProtoLanguageSetting(dlg, dlg->hContact, "UserInfo", setting, FALSE);
	if (dlg->lang_name[0] != _T('\0'))
		return;

	// If not found and is inside meta, try to get from the meta
	MCONTACT hMetaContact = db_mc_getMeta(dlg->hContact);
	if (hMetaContact != NULL) {
		GetUserProtoLanguageSetting(dlg, hMetaContact, META_PROTO, setting);
		if (dlg->lang_name[0] != _T('\0'))
			return;

		GetUserProtoLanguageSetting(dlg, hMetaContact, "UserInfo", setting, FALSE);
	}
}

void GetContactLanguage(Dialog *dlg)
{
	DBVARIANT dbv = { 0 };

	dlg->lang_name[0] = _T('\0');

	if (dlg->hContact == NULL) {
		if (!db_get_ts(NULL, MODULE_NAME, dlg->name, &dbv)) {
			mir_tstrncpy(dlg->lang_name, dbv.ptszVal, SIZEOF(dlg->lang_name));
			db_free(&dbv);
		}
	}
	else {
		if (!db_get_ts(dlg->hContact, MODULE_NAME, "TalkLanguage", &dbv)) {
			mir_tstrncpy(dlg->lang_name, dbv.ptszVal, SIZEOF(dlg->lang_name));
			db_free(&dbv);
		}

		if (dlg->lang_name[0] == _T('\0') && !db_get_ts(dlg->hContact, "eSpeak", "TalkLanguage", &dbv)) {
			mir_tstrncpy(dlg->lang_name, dbv.ptszVal, SIZEOF(dlg->lang_name));
			db_free(&dbv);
		}

		// Try from metacontact
		if (dlg->lang_name[0] == _T('\0')) {
			MCONTACT hMetaContact = db_mc_getMeta(dlg->hContact);
			if (hMetaContact != NULL) {
				if (!db_get_ts(hMetaContact, MODULE_NAME, "TalkLanguage", &dbv)) {
					mir_tstrncpy(dlg->lang_name, dbv.ptszVal, SIZEOF(dlg->lang_name));
					db_free(&dbv);
				}

				if (dlg->lang_name[0] == _T('\0') && !db_get_ts(hMetaContact, "eSpeak", "TalkLanguage", &dbv)) {
					mir_tstrncpy(dlg->lang_name, dbv.ptszVal, SIZEOF(dlg->lang_name));
					db_free(&dbv);
				}
			}
		}

		// Try to get from Language info
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language");
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language1");
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language2");
		if (dlg->lang_name[0] == _T('\0'))
			GetUserLanguageSetting(dlg, "Language3");

		// Use default lang
		if (dlg->lang_name[0] == _T('\0'))
			mir_tstrncpy(dlg->lang_name, opts.default_language, SIZEOF(dlg->lang_name));
	}

	int i = GetClosestLanguage(dlg->lang_name);
	if (i < 0) {
		// Lost a dict?
		mir_tstrncpy(dlg->lang_name, opts.default_language, SIZEOF(dlg->lang_name));
		i = GetClosestLanguage(dlg->lang_name);
	}

	if (i >= 0) {
		dlg->lang = languages[i];
		dlg->lang->load();
	}
	else dlg->lang = NULL;
}

void ModifyIcon(Dialog *dlg)
{
	StatusIconData sid = { sizeof(sid) };
	sid.szModule = MODULE_NAME;

	for (int i = 0; i < languages.getCount(); i++) {
		sid.dwId = i;

		if (languages[i] == dlg->lang)
			sid.flags = (dlg->enabled ? 0 : MBF_DISABLED);
		else
			sid.flags = MBF_HIDDEN;

		Srmm_ModifyIcon(dlg->hContact, &sid);
	}
}

INT_PTR AddContactTextBoxService(WPARAM wParam, LPARAM lParam)
{
	SPELLCHECKER_ITEM *sci = (SPELLCHECKER_ITEM *)wParam;
	if (sci == NULL || sci->cbSize != sizeof(SPELLCHECKER_ITEM))
		return -1;

	return AddContactTextBox(sci->hContact, sci->hwnd, sci->window_name, FALSE, NULL);
}

int AddContactTextBox(MCONTACT hContact, HWND hwnd, char *name, BOOL srmm, HWND hwndOwner)
{
	if (languages.getCount() <= 0)
		return 0;

	if (dialogs.find(hwnd) == dialogs.end()) {
		// Fill dialog data
		Dialog *dlg = (Dialog *)malloc(sizeof(Dialog));
		ZeroMemory(dlg, sizeof(Dialog));

		dlg->re = new RichEdit(hwnd);
		if (!dlg->re->IsValid()) {
			delete dlg->re;
			free(dlg);
			return 0;
		}

		dlg->hContact = hContact;
		dlg->hwnd = hwnd;
		strncpy(dlg->name, name, sizeof(dlg->name));
		dlg->enabled = db_get_b(dlg->hContact, MODULE_NAME, dlg->name, 1);
		dlg->srmm = srmm;

		GetContactLanguage(dlg);

		if (opts.auto_locale)
			LoadDictFromKbdl(dlg);

		mir_subclassWindow(dlg->hwnd, EditProc);
		dialogs[hwnd] = dlg;

		if (dlg->srmm && hwndOwner != NULL) {
			dlg->hwnd_owner = hwndOwner;
			mir_subclassWindow(dlg->hwnd_owner, OwnerProc);
			dialogs[dlg->hwnd_owner] = dlg;

			ModifyIcon(dlg);
		}

		if (dlg->lang != NULL)
			dlg->lang->load();

		SetTimer(hwnd, TIMER_ID, 1000, NULL);
	}

	return 0;
}

#define DESTROY_MENY(_m_)	if (_m_ != NULL) { DestroyMenu(_m_); _m_ = NULL; }

void FreePopupData(Dialog *dlg)
{
	DESTROY_MENY(dlg->hLanguageSubMenu);
	DESTROY_MENY(dlg->hWrongWordsSubMenu);

	if (dlg->wrong_words != NULL) {
		for (unsigned i = 0; i < dlg->wrong_words->size(); i++) {
			FREE((*dlg->wrong_words)[i].word);

			DESTROY_MENY((*dlg->wrong_words)[i].hMeSubMenu);
			DESTROY_MENY((*dlg->wrong_words)[i].hCorrectSubMenu);
			DESTROY_MENY((*dlg->wrong_words)[i].hReplaceSubMenu);

			FreeSuggestions((*dlg->wrong_words)[i].suggestions);
		}

		delete dlg->wrong_words;
		dlg->wrong_words = NULL;
	}
}

INT_PTR RemoveContactTextBoxService(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = (HWND)wParam;
	if (hwnd == NULL)
		return -1;

	return RemoveContactTextBox(hwnd);
}

int RemoveContactTextBox(HWND hwnd)
{
	DialogMapType::iterator dlgit = dialogs.find(hwnd);
	if (dlgit != dialogs.end()) {
		Dialog *dlg = dlgit->second;

		KillTimer(hwnd, TIMER_ID);

		mir_unsubclassWindow(hwnd, EditProc);
		dialogs.erase(hwnd);
		if (dlg->hwnd_owner != NULL)
			dialogs.erase(dlg->hwnd_owner);

		delete dlg->re;
		FreePopupData(dlg);
		free(dlg);
	}

	return 0;
}

// TODO Make this better
BOOL GetWordCharRange(Dialog *dlg, CHARRANGE &sel, TCHAR *text, size_t text_len, int &first_char)
{
	// Get line
	int line = dlg->re->GetLineFromChar(sel.cpMin);

	// Get text
	dlg->re->GetLine(line, text, text_len);
	first_char = dlg->re->GetFirstCharOfLine(line);

	// Find the word
	sel.cpMin--;
	while (sel.cpMin >= first_char && (dlg->lang->isWordChar(text[sel.cpMin - first_char])
		|| IsNumber(text[sel.cpMin - first_char])))
		sel.cpMin--;
	sel.cpMin++;

	while (text[sel.cpMax - first_char] != _T('\0') && (dlg->lang->isWordChar(text[sel.cpMax - first_char])
		|| IsNumber(text[sel.cpMax - first_char])))
		sel.cpMax++;

	// Has a word?
	if (sel.cpMin >= sel.cpMax)
		return FALSE;

	// See if it has only '-'s
	BOOL has_valid_char = FALSE;
	for (int i = sel.cpMin; i < sel.cpMax && !has_valid_char; i++)
		has_valid_char = (text[i - first_char] != _T('-'));

	return has_valid_char;
}

TCHAR *GetWordUnderPoint(Dialog *dlg, POINT pt, CHARRANGE &sel)
{
	// Get text
	if (dlg->re->GetTextLength() <= 0)
		return NULL;

	// Get pos
	sel.cpMin = sel.cpMax = dlg->re->GetCharFromPos(pt);

	// Get text
	TCHAR text[1024];
	int first_char;

	if (!GetWordCharRange(dlg, sel, text, SIZEOF(text), first_char))
		return NULL;

	// copy the word
	text[sel.cpMax - first_char] = _T('\0');
	return _tcsdup(&text[sel.cpMin - first_char]);
}


void AppendSubmenu(HMENU hMenu, HMENU hSubMenu, TCHAR *name)
{
	MENUITEMINFO mii = { sizeof(mii) };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mii.hSubMenu = hSubMenu;
	mii.dwTypeData = name;
	mii.cch = mir_tstrlen(name);
	InsertMenuItem(hMenu, 0, TRUE, &mii);
}

void AppendMenuItem(HMENU hMenu, int id, TCHAR *name, HICON hIcon, BOOL checked)
{
	ICONINFO iconInfo;
	GetIconInfo(hIcon, &iconInfo);

	MENUITEMINFO mii = { sizeof(mii) };
	mii.fMask = MIIM_CHECKMARKS | MIIM_TYPE | MIIM_STATE;
	mii.fType = MFT_STRING;
	mii.fState = (checked ? MFS_CHECKED : 0);
	mii.wID = id;
	mii.hbmpChecked = iconInfo.hbmColor;
	mii.hbmpUnchecked = iconInfo.hbmColor;
	mii.dwTypeData = name;
	mii.cch = mir_tstrlen(name);
	InsertMenuItem(hMenu, 0, TRUE, &mii);
}

#define LANGUAGE_MENU_ID_BASE 10
#define WORD_MENU_ID_BASE 100
#define AUTOREPLACE_MENU_ID_BASE 50

void AddMenuForWord(Dialog *dlg, TCHAR *word, CHARRANGE &pos, HMENU hMenu, BOOL in_submenu, UINT base)
{
	if (dlg->wrong_words == NULL)
		dlg->wrong_words = new vector<WrongWordPopupMenuData>(1);
	else
		dlg->wrong_words->resize(dlg->wrong_words->size() + 1);

	WrongWordPopupMenuData &data = (*dlg->wrong_words)[dlg->wrong_words->size() - 1];
	ZeroMemory(&data, sizeof(WrongWordPopupMenuData));

	// Get suggestions
	data.word = word;
	data.pos = pos;
	data.suggestions = dlg->lang->suggest(word);

	Suggestions &suggestions = data.suggestions;

	if (in_submenu) {
		data.hMeSubMenu = CreatePopupMenu();
		AppendSubmenu(hMenu, data.hMeSubMenu, word);
		hMenu = data.hMeSubMenu;
	}

	data.hReplaceSubMenu = CreatePopupMenu();

	InsertMenu(data.hReplaceSubMenu, 0, MF_BYPOSITION, base + AUTOREPLACE_MENU_ID_BASE + suggestions.count, TranslateT("Other..."));
	if (suggestions.count > 0) {
		InsertMenu(data.hReplaceSubMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
		for (int i = (int)suggestions.count - 1; i >= 0; i--)
			InsertMenu(data.hReplaceSubMenu, 0, MF_BYPOSITION, base + AUTOREPLACE_MENU_ID_BASE + i, suggestions.words[i]);
	}

	AppendSubmenu(hMenu, data.hReplaceSubMenu, TranslateT("Always replace with"));

	InsertMenu(hMenu, 0, MF_BYPOSITION, base + suggestions.count + 1, TranslateT("Ignore all"));
	InsertMenu(hMenu, 0, MF_BYPOSITION, base + suggestions.count, TranslateT("Add to dictionary"));

	if (suggestions.count > 0) {
		HMENU hSubMenu;
		if (opts.cascade_corrections) {
			hSubMenu = data.hCorrectSubMenu = CreatePopupMenu();
			AppendSubmenu(hMenu, hSubMenu, TranslateT("Corrections"));
		}
		else {
			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
			hSubMenu = hMenu;
		}

		for (int i = (int)suggestions.count - 1; i >= 0; i--)
			InsertMenu(hSubMenu, 0, MF_BYPOSITION, base + i, suggestions.words[i]);
	}

	if (!in_submenu && opts.show_wrong_word) {
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

		TCHAR text[128];
		mir_sntprintf(text, SIZEOF(text), TranslateT("Wrong word: %s"), word);
		InsertMenu(hMenu, 0, MF_BYPOSITION, 0, text);
	}
}

struct FoundWrongWordParam
{
	Dialog *dlg;
	int count;
};

void FoundWrongWord(TCHAR *word, CHARRANGE pos, void *param)
{
	FoundWrongWordParam *p = (FoundWrongWordParam*)param;

	p->count++;

	AddMenuForWord(p->dlg, _tcsdup(word), pos, p->dlg->hWrongWordsSubMenu, TRUE, WORD_MENU_ID_BASE * p->count);
}

void AddItemsToMenu(Dialog *dlg, HMENU hMenu, POINT pt, HWND hwndOwner)
{
	FreePopupData(dlg);
	if (opts.use_flags) {
		dlg->hwnd_menu_owner = hwndOwner;
		menus[hwndOwner] = dlg;
	}

	BOOL wrong_word = FALSE;

	// Make menu
	if (GetMenuItemCount(hMenu) > 0)
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);

	if (languages.getCount() > 0 && dlg->enabled) {
		dlg->hLanguageSubMenu = CreatePopupMenu();

		if (dlg->hwnd_menu_owner != NULL)
			mir_subclassWindow(dlg->hwnd_menu_owner, MenuWndProc);

		// First add languages
		for (int i = 0; i < languages.getCount(); i++)
			AppendMenu(dlg->hLanguageSubMenu, MF_STRING | (languages[i] == dlg->lang ? MF_CHECKED : 0),
			LANGUAGE_MENU_ID_BASE + i, languages[i]->full_name);

		AppendSubmenu(hMenu, dlg->hLanguageSubMenu, TranslateT("Language"));
	}

	InsertMenu(hMenu, 0, MF_BYPOSITION, 1, TranslateT("Enable spell checking"));
	CheckMenuItem(hMenu, 1, MF_BYCOMMAND | (dlg->enabled ? MF_CHECKED : MF_UNCHECKED));

	// Get text
	if (dlg->lang != NULL && dlg->enabled) {
		if (opts.show_all_corrections) {
			dlg->hWrongWordsSubMenu = CreatePopupMenu();

			FoundWrongWordParam p = { dlg, 0 };
			CheckText(dlg, TRUE, FoundWrongWord, &p);

			if (p.count > 0)
				AppendSubmenu(hMenu, dlg->hWrongWordsSubMenu, TranslateT("Wrong words"));
		}
		else {
			CHARRANGE sel;
			TCHAR *word = GetWordUnderPoint(dlg, pt, sel);
			if (word != NULL && !dlg->lang->spell(word)) {
				InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
				AddMenuForWord(dlg, word, sel, hMenu, FALSE, WORD_MENU_ID_BASE);
			}
		}
	}
}


static void AddWordToDictCallback(BOOL canceled, Dictionary *dict,
	const TCHAR *find, const TCHAR *replace, BOOL useVariables,
	const TCHAR *original_find, void *param)
{
	if (canceled)
		return;

	dict->autoReplace->add(find, replace, useVariables);

	HWND hwndParent = (HWND)param;
	if (hwndParent != NULL)
		PostMessage(hwndParent, WMU_DICT_CHANGED, 0, 0);
}


BOOL HandleMenuSelection(Dialog *dlg, POINT pt, unsigned selection)
{
	BOOL ret = FALSE;

	if (selection == 1) {
		ToggleEnabled(dlg);
		ret = TRUE;
	}
	else if (selection >= LANGUAGE_MENU_ID_BASE && selection < LANGUAGE_MENU_ID_BASE + (unsigned)languages.getCount()) {
		SetNoUnderline(dlg);

		if (dlg->hContact == NULL)
			db_set_ts(NULL, MODULE_NAME, dlg->name,
			languages[selection - LANGUAGE_MENU_ID_BASE]->language);
		else
			db_set_ts(dlg->hContact, MODULE_NAME, "TalkLanguage",
			languages[selection - LANGUAGE_MENU_ID_BASE]->language);

		GetContactLanguage(dlg);

		if (dlg->srmm)
			ModifyIcon(dlg);

		ret = TRUE;
	}
	else if (selection > 0 && dlg->wrong_words != NULL
				&& selection >= WORD_MENU_ID_BASE
				&& selection < (dlg->wrong_words->size() + 1) * WORD_MENU_ID_BASE) {
		int pos = selection / WORD_MENU_ID_BASE;
		selection -= pos * WORD_MENU_ID_BASE;
		pos--; // 0 based
		WrongWordPopupMenuData &data = (*dlg->wrong_words)[pos];

		if (selection < data.suggestions.count) {
			// TODO Assert that text hasn't changed
			ReplaceWord(dlg, data.pos, data.suggestions.words[selection]);
			ret = TRUE;
		}
		else if (selection == data.suggestions.count) {
			dlg->lang->addWord(data.word);
			ret = TRUE;
		}
		else if (selection == data.suggestions.count + 1) {
			dlg->lang->ignoreWord(data.word);
			ret = TRUE;
		}
		else if (selection >= AUTOREPLACE_MENU_ID_BASE && selection < AUTOREPLACE_MENU_ID_BASE + data.suggestions.count + 1) {
			selection -= AUTOREPLACE_MENU_ID_BASE;
			if (selection == data.suggestions.count) {
				ShowAutoReplaceDialog(dlg->hwnd_owner != NULL ? dlg->hwnd_owner : dlg->hwnd, FALSE,
											 dlg->lang, data.word, NULL, FALSE,
											 TRUE, &AddWordToDictCallback, dlg->hwnd);
			}
			else {
				// TODO Assert that text hasn't changed
				ReplaceWord(dlg, data.pos, data.suggestions.words[selection]);
				dlg->lang->autoReplace->add(data.word, data.suggestions.words[selection]);
				ret = TRUE;
			}
		}
	}

	if (ret) {
		KillTimer(dlg->hwnd, TIMER_ID);
		SetTimer(dlg->hwnd, TIMER_ID, 100, NULL);

		dlg->changed = TRUE;
	}

	FreePopupData(dlg);
	return ret;
}

int MsgWindowPopup(WPARAM wParam, LPARAM lParam)
{
	MessageWindowPopupData *mwpd = (MessageWindowPopupData *)lParam;
	if (mwpd == NULL || mwpd->cbSize < sizeof(MessageWindowPopupData) || mwpd->uFlags != MSG_WINDOWPOPUP_INPUT)
		return 0;

	DialogMapType::iterator dlgit = dialogs.find(mwpd->hwnd);
	if (dlgit == dialogs.end())
		return -1;

	Dialog *dlg = dlgit->second;

	POINT pt = mwpd->pt;
	ScreenToClient(dlg->hwnd, &pt);

	if (mwpd->uType == MSG_WINDOWPOPUP_SHOWING)
		AddItemsToMenu(dlg, mwpd->hMenu, pt, dlg->hwnd_owner);
	else if (mwpd->uType == MSG_WINDOWPOPUP_SELECTED)
		HandleMenuSelection(dlg, pt, mwpd->selection);

	return 0;
}

INT_PTR ShowPopupMenuService(WPARAM wParam, LPARAM lParam)
{
	SPELLCHECKER_POPUPMENU *scp = (SPELLCHECKER_POPUPMENU *)wParam;
	if (scp == NULL || scp->cbSize != sizeof(SPELLCHECKER_POPUPMENU))
		return -1;

	return ShowPopupMenu(scp->hwnd, scp->hMenu, scp->pt, scp->hwndOwner == NULL ? scp->hwnd : scp->hwndOwner);
}

int ShowPopupMenu(HWND hwnd, HMENU hMenu, POINT pt, HWND hwndOwner)
{
	DialogMapType::iterator dlgit = dialogs.find(hwnd);
	if (dlgit == dialogs.end())
		return -1;

	Dialog *dlg = dlgit->second;

	if (pt.x == 0xFFFF && pt.y == 0xFFFF) {
		CHARRANGE sel;
		SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);

		// Get current cursor pos
		SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
	}
	else ScreenToClient(hwnd, &pt);

	BOOL create_menu = (hMenu == NULL);
	if (create_menu)
		hMenu = CreatePopupMenu();

	// Make menu
	AddItemsToMenu(dlg, hMenu, pt, hwndOwner);

	// Show menu
	POINT client = pt;
	ClientToScreen(hwnd, &pt);
	int selection = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndOwner, NULL);

	// Do action
	if (HandleMenuSelection(dlg, client, selection))
		selection = 0;

	if (create_menu)
		DestroyMenu(hMenu);

	return selection;
}

int MsgWindowEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;
	if (event == NULL)
		return 0;

	if (event->cbSize < sizeof(MessageWindowEventData))
		return 0;

	if (event->uType == MSG_WINDOW_EVT_OPEN)
		AddContactTextBox(event->hContact, event->hwndInput, "DefaultSRMM", TRUE, event->hwndWindow);
	else if (event->uType == MSG_WINDOW_EVT_CLOSING)
		RemoveContactTextBox(event->hwndInput);

	return 0;
}


int IconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (sicd == NULL || strcmp(sicd->szModule, MODULE_NAME) != 0)
		return 0;

	if (hContact == NULL)
		return 0;

	// Find the dialog
	HWND hwnd = NULL;
	Dialog *dlg;
	for (DialogMapType::iterator it = dialogs.begin(); it != dialogs.end(); it++) {
		dlg = it->second;
		if (dlg->srmm && dlg->hContact == hContact) {
			hwnd = it->first;
			break;
		}
	}

	if (hwnd == NULL)
		return 0;

	if ((sicd->flags & MBCF_RIGHTBUTTON) == 0) {
		FreePopupData(dlg);

		// Show the menu
		HMENU hMenu = CreatePopupMenu();

		if (languages.getCount() > 0) {
			if (opts.use_flags) {
				menus[dlg->hwnd] = dlg;
				dlg->hwnd_menu_owner = dlg->hwnd;
				mir_subclassWindow(dlg->hwnd_menu_owner, MenuWndProc);
			}

			// First add languages
			for (int i = 0; i < languages.getCount(); i++)
				AppendMenu(hMenu, MF_STRING | (languages[i] == dlg->lang ? MF_CHECKED : 0), LANGUAGE_MENU_ID_BASE + i, languages[i]->full_name);

			InsertMenu(hMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, 0);
		}

		InsertMenu(hMenu, 0, MF_BYPOSITION, 1, TranslateT("Enable spell checking"));
		CheckMenuItem(hMenu, 1, MF_BYCOMMAND | (dlg->enabled ? MF_CHECKED : MF_UNCHECKED));

		// Show menu
		int selection = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, dlg->hwnd, NULL);
		HandleMenuSelection(dlg, sicd->clickLocation, selection);
		DestroyMenu(hMenu);
	}
	else {
		// Enable / disable
		HandleMenuSelection(dlg, sicd->clickLocation, 1);
	}

	return 0;
}

LRESULT CALLBACK MenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DialogMapType::iterator dlgit = menus.find(hwnd);
	if (dlgit == menus.end())
		return -1;

	Dialog *dlg = dlgit->second;

	switch (msg) {
	case WM_INITMENUPOPUP:
		{
			HMENU hMenu = (HMENU)wParam;

			int count = GetMenuItemCount(hMenu);
			for (int i = 0; i < count; i++) {
				unsigned id = GetMenuItemID(hMenu, i);
				if (id < LANGUAGE_MENU_ID_BASE || id >= LANGUAGE_MENU_ID_BASE + (unsigned)languages.getCount())
					continue;

				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STATE;
				GetMenuItemInfo(hMenu, id, FALSE, &mii);

				// Make ownerdraw
				ModifyMenu(hMenu, id, mii.fState | MF_BYCOMMAND | MF_OWNERDRAW, id, NULL);
			}
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlType != ODT_MENU || lpdis->itemID < LANGUAGE_MENU_ID_BASE || lpdis->itemID >= LANGUAGE_MENU_ID_BASE + (unsigned)languages.getCount())
				break;

			int pos = lpdis->itemID - LANGUAGE_MENU_ID_BASE;

			Dictionary *dict = languages[pos];

			COLORREF clrfore = SetTextColor(lpdis->hDC,
													  GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_MENUTEXT));
			COLORREF clrback = SetBkColor(lpdis->hDC,
													GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_MENU));

			FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_MENU));

			RECT rc = lpdis->rcItem;
			rc.left += 2;

			// Checked?
			rc.right = rc.left + bmpChecked.bmWidth;

			if (lpdis->itemState & ODS_CHECKED) {
				rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - bmpChecked.bmHeight) / 2;
				rc.bottom = rc.top + bmpChecked.bmHeight;

				HDC hdcTemp = CreateCompatibleDC(lpdis->hDC);
				HBITMAP oldBmp = (HBITMAP)SelectObject(hdcTemp, hCheckedBmp);

				BitBlt(lpdis->hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcTemp, 0, 0, SRCCOPY);

				SelectObject(hdcTemp, oldBmp);
				DeleteDC(hdcTemp);
			}

			rc.left += bmpChecked.bmWidth + 2;

			// Draw icon
			if (dict->hIcolib) {
				HICON hFlag = Skin_GetIconByHandle(dict->hIcolib);

				rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - ICON_SIZE) / 2;
				DrawIconEx(lpdis->hDC, rc.left, rc.top, hFlag, 16, 16, 0, NULL, DI_NORMAL);

				Skin_ReleaseIcon(hFlag);

				rc.left += ICON_SIZE + 4;
			}

			// Draw text
			RECT rc_text = { 0, 0, 0xFFFF, 0xFFFF };
			DrawText(lpdis->hDC, dict->full_name, mir_tstrlen(dict->full_name), &rc_text, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT);

			rc.right = lpdis->rcItem.right - 2;
			rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - (rc_text.bottom - rc_text.top)) / 2;
			rc.bottom = rc.top + rc_text.bottom - rc_text.top;
			DrawText(lpdis->hDC, dict->full_name, mir_tstrlen(dict->full_name), &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_TOP | DT_SINGLELINE);

			// Restore old colors
			SetTextColor(lpdis->hDC, clrfore);
			SetBkColor(lpdis->hDC, clrback);
		}
		return TRUE;

	case WM_MEASUREITEM:
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
		if (lpmis->CtlType != ODT_MENU || lpmis->itemID < LANGUAGE_MENU_ID_BASE || lpmis->itemID >= LANGUAGE_MENU_ID_BASE + (unsigned)languages.getCount())
			break;

		int pos = lpmis->itemID - LANGUAGE_MENU_ID_BASE;

		Dictionary *dict = languages[pos];

		HDC hdc = GetDC(hwnd);

		NONCLIENTMETRICS info;
		ZeroMemory(&info, sizeof(info));
		info.cbSize = sizeof(info);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		HFONT hFont = CreateFontIndirect(&info.lfMenuFont);
		HFONT hFontOld = (HFONT)SelectObject(hdc, hFont);

		RECT rc = { 0, 0, 0xFFFF, 0xFFFF };

		DrawText(hdc, dict->full_name, mir_tstrlen(dict->full_name), &rc, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_TOP | DT_CALCRECT);

		lpmis->itemHeight = max(ICON_SIZE, max(bmpChecked.bmHeight, rc.bottom));
		lpmis->itemWidth = 2 + bmpChecked.bmWidth + 2 + ICON_SIZE + 4 + rc.right + 2;

		SelectObject(hdc, hFontOld);
		DeleteObject(hFont);
		ReleaseDC(hwnd, hdc);

		return TRUE;
	}

	return mir_callNextSubclass(hwnd, MenuWndProc, msg, wParam, lParam);
}

TCHAR* lstrtrim(TCHAR *str)
{
	int len = mir_tstrlen(str);

	int i;
	for (i = len - 1; i >= 0 && (str[i] == ' ' || str[i] == '\t'); --i);
	if (i < len - 1) {
		++i;
		str[i] = _T('\0');
		len = i;
	}

	for (i = 0; i < len && (str[i] == ' ' || str[i] == '\t'); ++i);
	if (i > 0)
		memmove(str, &str[i], (len - i + 1) * sizeof(TCHAR));

	return str;
}

BOOL lstreq(TCHAR *a, TCHAR *b, size_t len)
{
	a = CharLower(_tcsdup(a));
	b = CharLower(_tcsdup(b));
	BOOL ret;
	if (len >= 0)
		ret = !_tcsncmp(a, b, len);
	else
		ret = !_tcscmp(a, b);
	free(a);
	free(b);
	return ret;
}
