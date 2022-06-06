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

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

Options opts;

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK AutoreplaceDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static OptPageControl optionsControls[] = {
	{ &opts.auto_replace_dict, CONTROL_CHECKBOX, IDC_AUTO_DICT, "AutoReplaceDict", FALSE },
	{ &opts.ignore_with_numbers, CONTROL_CHECKBOX, IDC_IGNORE_NUMBERS, "IgnoreWithNumbers", FALSE },
	{ &opts.ignore_uppercase, CONTROL_CHECKBOX, IDC_IGNORE_UPPERCASE, "IgnoreUppercase", FALSE },
	{ &opts.ask_when_sending_with_error, CONTROL_CHECKBOX, IDC_ASK_ON_ERROR, "AskWhenSendingWithError", FALSE },
	{ &opts.underline_type, CONTROL_COMBO, IDC_UNDERLINE_TYPE, "UnderlineType", CFU_UNDERLINEWAVE - CFU_UNDERLINEDOUBLE },
	{ &opts.cascade_corrections, CONTROL_CHECKBOX, IDC_CASCADE_CORRECTIONS, "CascadeCorrections", FALSE },
	{ &opts.show_all_corrections, CONTROL_CHECKBOX, IDC_SHOW_ALL_CORRECTIONS, "ShowAllCorrections", FALSE },
	{ &opts.show_wrong_word, CONTROL_CHECKBOX, IDC_SHOW_WRONG_WORD, "ShowWrongWord", TRUE },
	{ &opts.use_flags, CONTROL_CHECKBOX, IDC_USE_FLAGS, "UseFlags", TRUE },
	{ &opts.auto_locale, CONTROL_CHECKBOX, IDC_AUTO_LOCALE, "AutoLocale", FALSE },
	{ &opts.use_other_apps_dicts, CONTROL_CHECKBOX, IDC_OTHER_PROGS, "UseOtherAppsDicts", TRUE },
	{ &opts.handle_underscore, CONTROL_CHECKBOX, IDC_HANDLE_UNDERSCORE, "HandleUnderscore", FALSE },
};

static UINT optionsExpertControls[] = {
	IDC_ADVANCED, IDC_UNDERLINE_TYPE_L, IDC_UNDERLINE_TYPE, IDC_CASCADE_CORRECTIONS, IDC_SHOW_ALL_CORRECTIONS,
	IDC_SHOW_WRONG_WORD, IDC_USE_FLAGS, IDC_AUTO_LOCALE, IDC_OTHER_PROGS, IDC_HANDLE_UNDERSCORE
};

static OptPageControl autoReplaceControls[] = {
	{ &opts.auto_replace_user, CONTROL_CHECKBOX, IDC_AUTO_USER, "AutoReplaceUser", TRUE },
};

// Functions //////////////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Spell Checker");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);

	memset(&odp, 0, sizeof(odp));
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Auto-replacements");
	odp.pfnDlgProc = AutoreplaceDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_REPLACEMENTS);
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void LoadOptions()
{
	LoadOpts(optionsControls, _countof(optionsControls), MODULENAME);
	LoadOpts(autoReplaceControls, _countof(autoReplaceControls), MODULENAME);

	if (languages.getCount() <= 0) {
		opts.default_language[0] = '\0';
		return;
	}

	DBVARIANT dbv;
	if (!g_plugin.getWString("DefaultLanguage", &dbv)) {
		mir_wstrncpy(opts.default_language, dbv.pwszVal, _countof(opts.default_language));
		db_free(&dbv);
	}

	for (auto &it : languages)
		if (mir_wstrcmp(it->language, opts.default_language) == 0)
			return;

	mir_wstrcpy(opts.default_language, languages[0]->language);
}

static void DrawItem(LPDRAWITEMSTRUCT lpdis, Dictionary *dict)
{
	TEXTMETRIC tm;
	RECT rc;

	GetTextMetrics(lpdis->hDC, &tm);

	int foreIndex, backIndex;
	if (lpdis->itemState & ODS_DISABLED) {
		foreIndex = COLOR_GRAYTEXT;
		backIndex = COLOR_BTNFACE;
	}
	else if (lpdis->itemState & ODS_SELECTED) {
		foreIndex = COLOR_HIGHLIGHTTEXT;
		backIndex = COLOR_HIGHLIGHT;
	}
	else {
		foreIndex = COLOR_WINDOWTEXT;
		backIndex = COLOR_WINDOW;
	}

	COLORREF clrfore = SetTextColor(lpdis->hDC, GetSysColor(foreIndex));
	COLORREF clrback = SetBkColor(lpdis->hDC, GetSysColor(backIndex));

	FillRect(lpdis->hDC, &lpdis->rcItem, GetSysColorBrush(backIndex));

	rc.left = lpdis->rcItem.left + 2;

	// Draw icon
	if (opts.use_flags) {
		HICON hFlag = IcoLib_GetIconByHandle(dict->hIcolib);

		rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - ICON_SIZE) / 2;
		DrawIconEx(lpdis->hDC, rc.left, rc.top, hFlag, 16, 16, 0, nullptr, DI_NORMAL);

		rc.left += ICON_SIZE + 4;

		IcoLib_ReleaseIcon(hFlag);
	}

	// Draw text
	rc.right = lpdis->rcItem.right - 2;
	rc.top = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;
	rc.bottom = rc.top + tm.tmHeight;
	DrawText(lpdis->hDC, dict->full_name, -1, &rc, DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

	// Restore old colors
	SetTextColor(lpdis->hDC, clrfore);
	SetBkColor(lpdis->hDC, clrback);
}

static void MeasureItem(HWND hwndDlg, LPMEASUREITEMSTRUCT lpmis)
{
	TEXTMETRIC tm;
	GetTextMetrics(GetDC(hwndDlg), &tm);

	if (opts.use_flags)
		lpmis->itemHeight = max(ICON_SIZE, tm.tmHeight);
	else
		lpmis->itemHeight = tm.tmHeight;
}

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		int sel = -1;
		for (int i = 0; i < languages.getCount(); i++) {
			SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_ADDSTRING, 0, (LPARAM)languages[i]->full_name);
			SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETITEMDATA, i, (LPARAM)languages[i]);

			if (!mir_wstrcmp(opts.default_language, languages[i]->language))
				sel = i;
		}
		SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_SETCURSEL, sel, 0);

		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Line"));
		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Dotted"));
		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Dash"));
		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Dash dot"));
		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Dash dot dot"));
		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Wave"));
		SendDlgItemMessage(hwndDlg, IDC_UNDERLINE_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Thick"));
	}
	break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_GETMORE)
			Utils_OpenUrl("https://miranda-ng.org/p/SpellChecker");

		if (LOWORD(wParam) == IDC_DEF_LANG && (HIWORD(wParam) == CBN_SELCHANGE && (HWND)lParam == GetFocus())) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return 0;
		}

		break;

	case WM_NOTIFY:
	{
		LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY && languages.getCount() > 0) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_DEF_LANG, CB_GETCURSEL, 0, 0);
			if (sel >= languages.getCount())
				sel = 0;
			g_plugin.setWString("DefaultLanguage",
				(wchar_t *)languages[sel]->language);
			mir_wstrcpy(opts.default_language, languages[sel]->language);
		}
	}
	break;

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->CtlID != IDC_DEF_LANG)
			break;
		if (lpdis->itemID == -1)
			break;

		Dictionary *dict = (Dictionary *)lpdis->itemData;
		DrawItem(lpdis, dict);
	}
	return TRUE;

	case WM_MEASUREITEM:
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
		if (lpmis->CtlID != IDC_DEF_LANG)
			break;

		MeasureItem(hwndDlg, lpmis);
		return TRUE;
	}

	return SaveOptsDlgProc(optionsControls, _countof(optionsControls), MODULENAME, hwndDlg, msg, wParam, lParam);
}

struct AutoreplaceData
{
	Dictionary *dict;
	map<std::wstring, AutoReplacement> autoReplaceMap;
	BOOL initialized;
	BOOL changed;

	AutoreplaceData(Dictionary *dict) : dict(dict), initialized(FALSE), changed(FALSE) {}

	void RemoveWord(const wchar_t *aWord)
	{
		map<std::wstring, AutoReplacement>::iterator it = autoReplaceMap.find(aWord);
		if (it != autoReplaceMap.end())
			autoReplaceMap.erase(it);
		changed = TRUE;
	}

	void AddWord(const wchar_t *find, const wchar_t *replace, BOOL useVars)
	{
		autoReplaceMap[find] = AutoReplacement(replace, useVars);
		changed = TRUE;
	}
};

static void EnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_AUTO_USER);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LANGUAGE_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LANGUAGE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REPLACEMENTS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ADD), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), enabled);
	if (!enabled)
		return;

	HWND hList = GetDlgItem(hwndDlg, IDC_REPLACEMENTS);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT), ListView_GetSelectedCount(hList) == 1);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVE), ListView_GetSelectedCount(hList) > 0);
}

static void LoadReplacements(HWND hwndDlg)
{
	HWND hList = GetDlgItem(hwndDlg, IDC_REPLACEMENTS);

	ListView_DeleteAllItems(hList);
	ListView_SetItemCount(hList, 0);

	int sel = SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		return;

	AutoreplaceData *data = (AutoreplaceData *)SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETITEMDATA, sel, 0);
	if (!data->initialized) {
		data->dict->autoReplace->copyMap(&data->autoReplaceMap);
		data->initialized = TRUE;
	}

	map<std::wstring, AutoReplacement>::iterator it = data->autoReplaceMap.begin();
	for (int i = 0; it != data->autoReplaceMap.end(); it++, i++) {
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = i;
		item.iSubItem = 0;
		item.pszText = (wchar_t *)it->first.c_str();
		item.lParam = i;

		ListView_InsertItem(hList, &item);

		ListView_SetItemText(hList, i, 1, (wchar_t *)it->second.replace.c_str());
	}

	EnableDisableCtrls(hwndDlg);
}

static void SaveNewReplacements(BOOL canceled, Dictionary*,
	const wchar_t *find, const wchar_t *replace, BOOL useVariables,
	const wchar_t *original_find, void *param)
{
	if (canceled)
		return;

	AutoreplaceData *data = (AutoreplaceData *)param;

	if (mir_wstrlen(original_find) > 0)
		data->RemoveWord(original_find);

	data->AddWord(find, replace, useVariables);
}

static void ShowAddReplacement(HWND hwndDlg, int item = -1)
{
	int sel = SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
	if (sel < 0)
		return;

	AutoreplaceData *data = (AutoreplaceData *)SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETITEMDATA, sel, 0);

	wchar_t find[256];
	const wchar_t *replace = nullptr;
	BOOL useVariables = FALSE;

	if (item < 0)
		find[0] = 0;
	else
		ListView_GetItemText(GetDlgItem(hwndDlg, IDC_REPLACEMENTS), item, 0, find, _countof(find));

	if (mir_wstrlen(find) > 0) {
		AutoReplacement &ar = data->autoReplaceMap[find];
		replace = ar.replace.c_str();
		useVariables = ar.useVariables;
	}

	if (ShowAutoReplaceDialog(hwndDlg, TRUE, data->dict, find, replace, useVariables, FALSE, &SaveNewReplacements, data)) {
		LoadReplacements(hwndDlg);
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
	}

	EnableDisableCtrls(hwndDlg);
}

static INT_PTR CALLBACK AutoreplaceDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		BOOL ret = SaveOptsDlgProc(autoReplaceControls, _countof(autoReplaceControls), MODULENAME, hwndDlg, msg, wParam, lParam);

		int sel = -1;
		for (int i = 0; i < languages.getCount(); i++) {
			Dictionary *p = languages[i];
			SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_ADDSTRING, 0, (LPARAM)p->full_name);
			SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_SETITEMDATA, i, (LPARAM)new AutoreplaceData(p));

			if (!mir_wstrcmp(opts.default_language, p->language))
				sel = i;
		}
		SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_SETCURSEL, sel, 0);

		HWND hList = GetDlgItem(hwndDlg, IDC_REPLACEMENTS);

		ListView_SetExtendedListViewStyle(hList, ListView_GetExtendedListViewStyle(hList) | LVS_EX_FULLROWSELECT);

		LVCOLUMN col = { 0 };
		col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		col.fmt = LVCFMT_LEFT;
		col.cx = 175;
		col.pszText = TranslateT("Wrong word");
		ListView_InsertColumn(hList, 0, &col);

		col.pszText = TranslateT("Correction");
		ListView_InsertColumn(hList, 1, &col);

		LoadReplacements(hwndDlg);
		return ret;
	}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LANGUAGE && HIWORD(wParam) == CBN_SELCHANGE)
			LoadReplacements(hwndDlg);
		else if (LOWORD(wParam) == IDC_ADD)
			ShowAddReplacement(hwndDlg);
		else if (LOWORD(wParam) == IDC_EDIT) {
			HWND hList = GetDlgItem(hwndDlg, IDC_REPLACEMENTS);
			if (ListView_GetSelectedCount(hList) != 1)
				break;

			int sel = SendMessage(hList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			if (sel < 0)
				break;

			ShowAddReplacement(hwndDlg, sel);
		}
		else if (LOWORD(wParam) == IDC_REMOVE) {
			HWND hList = GetDlgItem(hwndDlg, IDC_REPLACEMENTS);

			int sel = SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
			if (sel < 0)
				break;

			AutoreplaceData *data = (AutoreplaceData *)SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETITEMDATA, sel, 0);

			BOOL changed = FALSE;

			sel = SendMessage(hList, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			while (sel >= 0) {
				wchar_t tmp[256];
				ListView_GetItemText(hList, sel, 0, tmp, _countof(tmp));

				data->RemoveWord(tmp);
				changed = TRUE;

				sel = SendMessage(hList, LVM_GETNEXTITEM, sel, LVNI_SELECTED);
			}

			if (changed) {
				LoadReplacements(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
	{
		LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY && languages.getCount() > 0) {
			for (int i = 0; i < languages.getCount(); i++) {
				AutoreplaceData *data = (AutoreplaceData *)SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_GETITEMDATA, i, 0);
				if (data->changed) {
					data->dict->autoReplace->setMap(data->autoReplaceMap);
					data->changed = FALSE;
				}
			}
		}
		else if (lpnmhdr->idFrom == IDC_REPLACEMENTS) {
			switch (lpnmhdr->code) {
			case LVN_ITEMCHANGED:
			case NM_CLICK:
				EnableDisableCtrls(hwndDlg);
				break;

			case NM_DBLCLK:
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
				if (lpnmitem->iItem >= 0)
					ShowAddReplacement(hwndDlg, lpnmitem->iItem);
				break;
			}
		}
		else if (lpnmhdr->idFrom == IDC_AUTO_USER)
			EnableDisableCtrls(hwndDlg);
	}
	break;

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->CtlID != IDC_LANGUAGE)
			break;
		if (lpdis->itemID == -1)
			break;

		AutoreplaceData *data = (AutoreplaceData *)lpdis->itemData;
		DrawItem(lpdis, data->dict);
	}
	return TRUE;

	case WM_MEASUREITEM:
		LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
		if (lpmis->CtlID != IDC_LANGUAGE)
			break;

		MeasureItem(hwndDlg, lpmis);
		return TRUE;
	}

	return SaveOptsDlgProc(autoReplaceControls, _countof(autoReplaceControls), MODULENAME, hwndDlg, msg, wParam, lParam);
}
