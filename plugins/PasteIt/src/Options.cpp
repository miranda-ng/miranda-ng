/*
Paste It plugin
Copyright (C) 2011 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"

Options *Options::instance;

struct TCpTable
{
	UINT cpId;
	wchar_t *cpName;
}
cpTable[] = {
	{ CP_ACP, LPGENW("Use default codepage") },
	{ CP_UTF8, LPGENW("UTF-8") },
	{ 874, LPGENW("Thai") },
	{ 932, LPGENW("Japanese") },
	{ 936, LPGENW("Simplified Chinese") },
	{ 949, LPGENW("Korean") },
	{ 950, LPGENW("Traditional Chinese") },
	{ 1250, LPGENW("Central European") },
	{ 1251, LPGENW("Cyrillic") },
	{ 20866, LPGENW("Cyrillic KOI8-R") },
	{ 1252, LPGENW("Latin I") },
	{ 1253, LPGENW("Greek") },
	{ 1254, LPGENW("Turkish") },
	{ 1255, LPGENW("Hebrew") },
	{ 1256, LPGENW("Arabic") },
	{ 1257, LPGENW("Baltic") },
	{ 1258, LPGENW("Vietnamese") },
	{ 1361, LPGENW("Korean (Johab)") },
	{ CP_UTF7, LPGENW("UTF-7") },
	{ 1200, LPGENW("UTF-16") },
	{ 1201, LPGENW("UTF-16BE") }
};

Options::Options()
{
	defWeb = 0;
	codepage = CP_ACP;
	autoUTF = true;
	confDlg = true;
	autoSend = false;
	for (int i = 0; i < PasteToWeb::pages; ++i) {
		webOptions[i] = new WebOptions(i);
	}
}


Options::~Options()
{
	for (int i = 0; i < PasteToWeb::pages; ++i) {
		delete webOptions[i];
		webOptions[i] = nullptr;
	}
}

void DefWebPageChanged();

INT_PTR CALLBACK Options::DlgProcOptsMain(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			CheckDlgButton(hwndDlg, IDC_AUTOUTF, instance->autoUTF ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONFDLG, instance->confDlg ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOSEND, instance->autoSend ? BST_CHECKED : BST_UNCHECKED);

			for (int i = 0; i < PasteToWeb::pages; ++i) {
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_WEBLIST), pasteToWebs[i]->GetName());
			}

			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_WEBLIST), instance->defWeb);

			InitCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), instance->codepage);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_CODEPAGE) {
			if (HIWORD(wParam) == CBN_KILLFOCUS) {
				GetCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), true, instance->codepage);
			}
		}

		if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			instance->codepage = GetCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), false, instance->codepage);
			instance->autoUTF = IsDlgButtonChecked(hwndDlg, IDC_AUTOUTF) ? true : false;
			instance->confDlg = IsDlgButtonChecked(hwndDlg, IDC_CONFDLG) ? true : false;
			instance->autoSend = IsDlgButtonChecked(hwndDlg, IDC_AUTOSEND) ? true : false;
			instance->defWeb = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_WEBLIST));

			DefWebPageChanged();

			Options::instance->Save();
		}
		return TRUE;
	}

	return FALSE;
}

class OptsPagesData
{
public:
	OptsPagesData()
		: lastPage(0)
	{
		init = false;
		for (int i = 0; i < PasteToWeb::pages; ++i) {
			webOptions[i] = Options::instance->webOptions[i]->Copy();
		}
	}

	~OptsPagesData()
	{
		for (int i = 0; i < PasteToWeb::pages; ++i) {
			delete webOptions[i];
			webOptions[i] = nullptr;
		}
	}

	WebOptions* webOptions[PasteToWeb::pages];
	int lastPage;
	bool init;
};

struct OptsConfigureData
{
	int page;
	std::list<PasteFormat> tempFormats;
};

struct OptsLoginData
{
	std::wstring user;
	std::wstring password;
};

void ReloadFormatsCombo(HWND hWnd, WebOptions* wo)
{
	while (ComboBox_DeleteString(hWnd, 0) > 0);
	int sel = 0;
	int i = 0;
	for (std::list<PasteFormat>::iterator it = wo->formats.begin(); it != wo->formats.end(); ++it) {
		ComboBox_AddString(hWnd, it->name.c_str());
		if (it->id == wo->defFormatId)
			sel = i;
		++i;
	}
	
	if (!wo->formats.empty())
		ComboBox_SetCurSel(hWnd, sel);
}

void GetPagesSettings(HWND hwndDlg, OptsPagesData* optsPagesData)
{
	int selected = optsPagesData->lastPage;
	int format = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_DEFFORMAT));
	if (format >= 0 && format < (int)optsPagesData->webOptions[selected]->formats.size()) {
		for (std::list<PasteFormat>::iterator it = optsPagesData->webOptions[selected]->formats.begin(); it != optsPagesData->webOptions[selected]->formats.end(); ++it) {
			if (format-- <= 0) {
				optsPagesData->webOptions[selected]->defFormatId = it->id;
				break;
			}
		}
	}

	if (optsPagesData->webOptions[selected]->isSendFileName)
		optsPagesData->webOptions[selected]->sendFileName = IsDlgButtonChecked(hwndDlg, IDC_AUTOFORMAT) ? true : false;
	if (optsPagesData->webOptions[selected]->isPublicPaste)
		optsPagesData->webOptions[selected]->publicPaste = IsDlgButtonChecked(hwndDlg, IDC_PUBLICPASTE) ? true : false;
	if (optsPagesData->webOptions[selected]->isCombo1) {
		int sel = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO1));
		if (sel >= 0 && sel < (int)optsPagesData->webOptions[selected]->combo1Values.size()) {
			for (std::list<PasteFormat>::iterator it = optsPagesData->webOptions[selected]->combo1Values.begin(); it != optsPagesData->webOptions[selected]->combo1Values.end(); ++it) {
				if (sel-- <= 0) {
					optsPagesData->webOptions[selected]->combo1 = it->id;
					break;
				}
			}
		}
	}
	
	if (optsPagesData->webOptions[selected]->isPublicPaste) {
		if (IsDlgButtonChecked(hwndDlg, IDC_GUEST)) {
			optsPagesData->webOptions[selected]->pastebinUserKey = L"";
		}
		else {
			wchar_t buf[100];
			Edit_GetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), buf, 100);
			optsPagesData->webOptions[selected]->pastebinUserKey = buf;
		}
	}
}

INT_PTR CALLBACK Options::DlgProcOptsPages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			OptsPagesData* optsPagesData = new OptsPagesData();
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)optsPagesData);

			HWND btnhwnd = GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN);
			int btSize = Button_GetTextLength(btnhwnd);
			if (btSize > 20)
				SetWindowPos(btnhwnd, nullptr, 0, 0, 115, 34, SWP_NOZORDER | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOACTIVATE | SWP_NOREDRAW);

			for (int i = 0; i < PasteToWeb::pages; ++i)
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_WEBPAGE), pasteToWebs[i]->GetName());

			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_WEBPAGE), 0);
			ReloadFormatsCombo(GetDlgItem(hwndDlg, IDC_DEFFORMAT), optsPagesData->webOptions[0]);
			if (optsPagesData->webOptions[0]->isSendFileName)
				CheckDlgButton(hwndDlg, IDC_AUTOFORMAT, optsPagesData->webOptions[0]->sendFileName ? BST_CHECKED : BST_UNCHECKED);
			else
				ShowWindow(GetDlgItem(hwndDlg, IDC_AUTOFORMAT), SW_HIDE);

			if (Options::instance->webOptions[0]->isPublicPaste)
				CheckDlgButton(hwndDlg, IDC_PUBLICPASTE, Options::instance->webOptions[0]->publicPaste ? BST_CHECKED : BST_UNCHECKED);
			else
				ShowWindow(GetDlgItem(hwndDlg, IDC_PUBLICPASTE), SW_HIDE);

			if (Options::instance->webOptions[0]->isCombo1) {
				int sel = 0;
				int i = 0;
				std::wstring &val = Options::instance->webOptions[0]->combo1;
				HWND cb = GetDlgItem(hwndDlg, IDC_COMBO1);
				for (std::list<PasteFormat>::iterator it = Options::instance->webOptions[0]->combo1Values.begin(); it != Options::instance->webOptions[0]->combo1Values.end(); ++it) {
					ComboBox_AddString(cb, it->name.c_str());
					if (val == it->id)
						sel = i;
					++i;
				}
				ComboBox_SetCurSel(cb, sel);
				Static_SetText(GetDlgItem(hwndDlg, IDC_COMBO1_DESC), Options::instance->webOptions[0]->combo1Desc.c_str());
			}
			else {
				ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1_DESC), SW_HIDE);
			}

			Edit_LimitText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), 99);
			if (Options::instance->webOptions[0]->isPastebin) {
				std::wstring pastebinUserKey = Options::instance->webOptions[0]->pastebinUserKey;
				if (pastebinUserKey.empty()) {
					CheckDlgButton(hwndDlg, IDC_GUEST, BST_CHECKED);
					Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), FALSE);
					Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), FALSE);
					Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), FALSE);
				}
				else Edit_SetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), pastebinUserKey.c_str());
			}
			else {
				ShowWindow(GetDlgItem(hwndDlg, IDC_GUEST), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), SW_HIDE);
			}

			optsPagesData->init = true;
		}
		return TRUE;

	case WM_COMMAND:
		{
			OptsPagesData* optsPagesData = (OptsPagesData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_CONFIGURE) {
				GetPagesSettings(hwndDlg, optsPagesData);
				OptsConfigureData d;
				d.page = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_WEBPAGE));
				WebOptions* wo = optsPagesData->webOptions[d.page];
				d.tempFormats = wo->formats;
				if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DLG_CONFIGURE), hwndDlg, DlgProcOptsConfigure, (LPARAM)&d) == IDOK) {
					wo->formats = d.tempFormats;
					ReloadFormatsCombo(GetDlgItem(hwndDlg, IDC_DEFFORMAT), wo);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_PASTEBIN_LOGIN) {
				OptsLoginData d;
				if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_DLG_PASTEBIN_LOGIN), hwndDlg, DlgProcOptsLogin, (LPARAM)&d) == IDOK) {
					PasteToWeb1 *web = (PasteToWeb1*)pasteToWebs[optsPagesData->lastPage];
					Edit_SetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), web->GetUserKey(d.user, d.password).c_str());
				}
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_GUEST) {
				BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_GUEST) ? FALSE : TRUE;
				Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), enabled);
				Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), enabled);
				Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), enabled);
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_WEBPAGE) {
				GetPagesSettings(hwndDlg, optsPagesData);
				optsPagesData->init = false;
				optsPagesData->lastPage = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_WEBPAGE));
				ReloadFormatsCombo(GetDlgItem(hwndDlg, IDC_DEFFORMAT), optsPagesData->webOptions[optsPagesData->lastPage]);
				if (optsPagesData->webOptions[optsPagesData->lastPage]->isSendFileName) {
					ShowWindow(GetDlgItem(hwndDlg, IDC_AUTOFORMAT), SW_SHOW);
					CheckDlgButton(hwndDlg, IDC_AUTOFORMAT, optsPagesData->webOptions[optsPagesData->lastPage]->sendFileName ? BST_CHECKED : BST_UNCHECKED);
				}
				else ShowWindow(GetDlgItem(hwndDlg, IDC_AUTOFORMAT), SW_HIDE);

				if (optsPagesData->webOptions[optsPagesData->lastPage]->isPublicPaste) {
					ShowWindow(GetDlgItem(hwndDlg, IDC_PUBLICPASTE), SW_SHOW);
					CheckDlgButton(hwndDlg, IDC_PUBLICPASTE, optsPagesData->webOptions[optsPagesData->lastPage]->publicPaste ? BST_CHECKED : BST_UNCHECKED);
				}
				else ShowWindow(GetDlgItem(hwndDlg, IDC_PUBLICPASTE), SW_HIDE);

				if (optsPagesData->webOptions[optsPagesData->lastPage]->isCombo1) {
					ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1_DESC), SW_SHOW);
					int sel = 0;
					int i = 0;
					std::wstring &val = optsPagesData->webOptions[optsPagesData->lastPage]->combo1;
					HWND cb = GetDlgItem(hwndDlg, IDC_COMBO1);
					while (ComboBox_DeleteString(cb, 0) > 0);
					for (std::list<PasteFormat>::iterator it = optsPagesData->webOptions[optsPagesData->lastPage]->combo1Values.begin(); it != optsPagesData->webOptions[optsPagesData->lastPage]->combo1Values.end(); ++it) {
						ComboBox_AddString(cb, it->name.c_str());
						if (val == it->id)
							sel = i;
						++i;
					}
					ComboBox_SetCurSel(cb, sel);
					SetDlgItemText(hwndDlg, IDC_COMBO1_DESC, optsPagesData->webOptions[optsPagesData->lastPage]->combo1Desc.c_str());
				}
				else {
					ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_COMBO1_DESC), SW_HIDE);
				}

				if (optsPagesData->webOptions[optsPagesData->lastPage]->isPastebin) {
					ShowWindow(GetDlgItem(hwndDlg, IDC_GUEST), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), SW_SHOW);
					std::wstring pastebinUserKey = optsPagesData->webOptions[optsPagesData->lastPage]->pastebinUserKey;
					if (pastebinUserKey.empty()) {
						CheckDlgButton(hwndDlg, IDC_GUEST, BST_CHECKED);
						Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), FALSE);
						Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), FALSE);
						Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), FALSE);
					}
					else {
						CheckDlgButton(hwndDlg, IDC_GUEST, BST_UNCHECKED);
						Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), TRUE);
						Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), TRUE);
						Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), TRUE);
					}

					Edit_SetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), pastebinUserKey.c_str());
				}
				else {
					ShowWindow(GetDlgItem(hwndDlg, IDC_GUEST), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), SW_HIDE);
				}

				optsPagesData->init = true;
			}

			if (optsPagesData->init && ((HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) != IDC_CONFIGURE && LOWORD(wParam) != IDC_PASTEBIN_LOGIN) || (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) != IDC_WEBPAGE) || HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			OptsPagesData* optsPagesData = (OptsPagesData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			GetPagesSettings(hwndDlg, optsPagesData);
			for (int i = 0; i < PasteToWeb::pages; ++i) {
				*Options::instance->webOptions[i] = *optsPagesData->webOptions[i];
			}

			Options::instance->Save();
		}
		return TRUE;

	case WM_CLOSE:
		OptsPagesData *optsPagesData = (OptsPagesData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		delete optsPagesData;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

void SelectLbConfigure(HWND hwndDlg, int sel, OptsConfigureData* optsConfigureData)
{
	if (sel >= 0 && sel < (int)optsConfigureData->tempFormats.size()) {
		Button_Enable(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
		if (sel == 0)
			Button_Enable(GetDlgItem(hwndDlg, IDC_UP), FALSE);
		else
			Button_Enable(GetDlgItem(hwndDlg, IDC_UP), TRUE);
		if (sel + 1 == optsConfigureData->tempFormats.size())
			Button_Enable(GetDlgItem(hwndDlg, IDC_DOWN), FALSE);
		else
			Button_Enable(GetDlgItem(hwndDlg, IDC_DOWN), TRUE);
	}
	else {
		Button_Enable(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
		Button_Enable(GetDlgItem(hwndDlg, IDC_UP), FALSE);
		Button_Enable(GetDlgItem(hwndDlg, IDC_DOWN), FALSE);
	}
}

INT_PTR CALLBACK Options::DlgProcOptsConfigure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			OptsConfigureData* optsConfigureData = (OptsConfigureData*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
			for (std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it) {
				ListBox_AddString(lb, it->name.c_str());
			}

			SelectLbConfigure(hwndDlg, -1, optsConfigureData);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) == IDOK) {
				EndDialog(hwndDlg, IDOK);
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDlg, IDCANCEL);
			}
			else if (LOWORD(wParam) == IDC_DELETE) {
				HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
				OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int sel = ListBox_GetCurSel(lb);
				if (sel >= 0) {
					ListBox_DeleteString(lb, sel);
					int i = sel;
					for (std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it) {
						if (i-- <= 0) {
							optsConfigureData->tempFormats.erase(it);
							ListBox_SetCurSel(lb, sel);
							SelectLbConfigure(hwndDlg, sel, optsConfigureData);
							break;
						}
					}
				}
			}
			else if (LOWORD(wParam) == IDC_UP) {
				HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
				OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int sel = ListBox_GetCurSel(lb);
				if (sel > 0) {
					int i = sel;
					for (std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it) {
						if (i-- <= 0) {
							PasteFormat pf = *it;
							std::list<PasteFormat>::iterator prevIt = it;
							--prevIt;
							optsConfigureData->tempFormats.erase(it);
							optsConfigureData->tempFormats.insert(prevIt, pf);
							ListBox_DeleteString(lb, sel--);
							ListBox_InsertString(lb, sel, pf.name.c_str());
							ListBox_SetCurSel(lb, sel);
							SelectLbConfigure(hwndDlg, sel, optsConfigureData);
							break;
						}
					}
				}
			}
			else if (LOWORD(wParam) == IDC_DOWN) {
				HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
				OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int sel = ListBox_GetCurSel(lb);
				if (sel >= 0 && sel + 1 < (int)optsConfigureData->tempFormats.size()) {
					int i = sel;
					for (std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it) {
						if (i-- <= 0) {
							PasteFormat pf = *it;
							std::list<PasteFormat>::iterator nextIt = it;
							++(++nextIt);
							optsConfigureData->tempFormats.erase(it);
							optsConfigureData->tempFormats.insert(nextIt, pf);
							ListBox_DeleteString(lb, sel++);
							ListBox_InsertString(lb, sel, pf.name.c_str());
							ListBox_SetCurSel(lb, sel);
							SelectLbConfigure(hwndDlg, sel, optsConfigureData);
							break;
						}
					}
				}
			}
			else if (LOWORD(wParam) == IDC_DOWNLOAD) {
				OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
				while (ListBox_DeleteString(lb, 0) > 0);
				SelectLbConfigure(hwndDlg, -1, optsConfigureData);
				optsConfigureData->tempFormats = pasteToWebs[optsConfigureData->page]->GetFormats();
				for (std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it) {
					ListBox_AddString(lb, it->name.c_str());
				}
			}
			else if (LOWORD(wParam) == IDC_RESTORE) {
				OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
				while (ListBox_DeleteString(lb, 0) > 0);
				SelectLbConfigure(hwndDlg, -1, optsConfigureData);
				optsConfigureData->tempFormats = pasteToWebs[optsConfigureData->page]->GetDefFormats();
				for (std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it) {
					ListBox_AddString(lb, TranslateW(it->name.c_str()));
				}
			}
		}
		else if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_FORMATTING) {
			OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			int sel = ListBox_GetCurSel(GetDlgItem(hwndDlg, IDC_FORMATTING));
			SelectLbConfigure(hwndDlg, sel, optsConfigureData);
		}
		return TRUE;

	case WM_VKEYTOITEM:
		if (LOWORD(wParam) == VK_DELETE && (HWND)lParam == GetDlgItem(hwndDlg, IDC_FORMATTING)) {
			DlgProcOptsConfigure(hwndDlg, WM_COMMAND, MAKELONG(IDC_DELETE, BN_CLICKED), NULL);
			return -2;
		}
		return -1;

	case WM_NOTIFY:
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK Options::DlgProcOptsLogin(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_PASTEBIN_USER), 99);
		Edit_LimitText(GetDlgItem(hwndDlg, IDC_PASTEBIN_PASSWORD), 99);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) == IDOK) {
				wchar_t buf[100];
				OptsLoginData* optsLoginData = (OptsLoginData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				Edit_GetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_USER), buf, 100);
				optsLoginData->user = buf;
				Edit_GetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_PASSWORD), buf, 100);
				optsLoginData->password = buf;
				EndDialog(hwndDlg, IDOK);
			}
			else if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDlg, IDCANCEL);
			}
		}
		return TRUE;

	case WM_NOTIFY:
		return TRUE;
	}

	return FALSE;
}

unsigned int Options::GetCodepageCB(HWND hwndCB, bool errorReport, unsigned int defCp)
{
	int selCpIdx = ComboBox_GetCurSel(hwndCB);
	if (selCpIdx < 0) {
		wchar_t text[128];
		ComboBox_GetText(hwndCB, text, 128);
		wchar_t * stopOn = nullptr;
		long cp = wcstol(text, &stopOn, 10);
		if (errorReport && (stopOn == text || *stopOn != '\0' || cp < 0 || cp > 0xffff)) {
			MessageBox(GetParent(hwndCB), TranslateT("You've entered invalid codepage. Select codepage from combo box or enter correct number."), TranslateT("Invalid codepage"), MB_OK | MB_ICONERROR);
			SetFocus(hwndCB);
			SetCodepageCB(hwndCB, defCp);
			return defCp;
		}

		return cp;
	}
	else return cpTable[selCpIdx].cpId;
}

void Options::SetDefWeb(int web)
{
	defWeb = web;
	g_plugin.setWString("defWeb", pasteToWebs[web]->GetName());
	DefWebPageChanged();
}

void Options::InitCodepageCB(HWND hwndCB, unsigned int codepage)
{
	int cpCount = sizeof(cpTable) / sizeof(cpTable[0]);
	int selCpIdx = -1;
	for (int i = 0; i < cpCount; ++i) {
		ComboBox_AddString(hwndCB, TranslateW(cpTable[i].cpName));
		if (cpTable[i].cpId == codepage)
			selCpIdx = i;
	}

	if (selCpIdx == -1) {
		wchar_t buf[10];
		mir_snwprintf(buf, L"%d", codepage);
		ComboBox_SetText(hwndCB, buf);
	}
	else {
		ComboBox_SetCurSel(hwndCB, selCpIdx);
	}

	ComboBox_LimitText(hwndCB, 127);
}

void Options::SetCodepageCB(HWND hwndCB, unsigned int codepage)
{
	int cpCount = sizeof(cpTable) / sizeof(cpTable[0]);
	int selCpIdx = -1;
	for (int i = 0; i < cpCount; ++i) {
		if (cpTable[i].cpId == codepage)
			selCpIdx = i;
	}

	if (selCpIdx == -1) {
		wchar_t buf[10];
		mir_snwprintf(buf, L"%d", codepage);
		ComboBox_SetText(hwndCB, buf);
	}
	else {
		ComboBox_SetCurSel(hwndCB, selCpIdx);
	}
}

int Options::InitOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szTitle.w = LPGENW("Paste It");
	odp.szGroup.w = LPGENW("Services");

	odp.szTab.w = LPGENW("Main");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = Options::DlgProcOptsMain;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Web page");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PAGES);
	odp.pfnDlgProc = Options::DlgProcOptsPages;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void Options::Save()
{
	g_plugin.setWString("defWeb", pasteToWebs[defWeb]->GetName());
	g_plugin.setDword("codepage", codepage);
	g_plugin.setByte("autoUTF", autoUTF ? 1 : 0);
	g_plugin.setByte("confDlg", confDlg ? 1 : 0);
	g_plugin.setByte("autoSend", autoSend ? 1 : 0);
	for (int i = 0; i < PasteToWeb::pages; ++i) {
		char buf[256];
		wchar_t* name = pasteToWebs[i]->GetName();
		int j = 0;
		while (name[j]) {
			buf[j] = (char)name[j];
			++j;
		}
		buf[j++] = '_';

		mir_strncpy(buf + j, "formats", _countof(buf) - j);
		std::wstring forms;
		for (std::list<PasteFormat>::iterator it = webOptions[i]->formats.begin(); it != webOptions[i]->formats.end(); ++it) {
			forms += it->id + L'=' + it->name + L';';
		}

		g_plugin.setWString(buf, forms.c_str());

		mir_strncpy(buf + j, "defFormatId", _countof(buf) - j);
		g_plugin.setWString(buf, webOptions[i]->defFormatId.c_str());

		if (webOptions[i]->isSendFileName) {
			mir_strncpy(buf + j, "sendFileName", _countof(buf) - j);
			g_plugin.setByte(buf, webOptions[i]->sendFileName ? 1 : 0);
		}

		if (webOptions[i]->isPublicPaste) {
			mir_strncpy(buf + j, "publicPaste", _countof(buf) - j);
			g_plugin.setByte(buf, webOptions[i]->publicPaste ? 1 : 0);
		}

		if (webOptions[i]->isCombo1) {
			mir_strncpy(buf + j, "combo1", _countof(buf) - j);
			g_plugin.setWString(buf, webOptions[i]->combo1.c_str());
		}

		if (webOptions[i]->isPastebin) {
			mir_strncpy(buf + j, "pastebinUserKey", _countof(buf) - j);
			g_plugin.setWString(buf, webOptions[i]->pastebinUserKey.c_str());
		}
	}
}

void Options::Load()
{
	DBVARIANT defWebV;
	if (!g_plugin.getWString("defWeb", &defWebV)) {
		for (int i = 0; i < PasteToWeb::pages; ++i) {
			if (!mir_wstrcmp(pasteToWebs[i]->GetName(), defWebV.pwszVal)) {
				defWeb = i;
				break;
			}
		}
		db_free(&defWebV);
	}
	codepage = g_plugin.getDword("codepage", CP_ACP);
	autoUTF = g_plugin.getByte("autoUTF", 1) ? true : false;
	confDlg = g_plugin.getByte("confDlg", 1) ? true : false;
	autoSend = g_plugin.getByte("autoSend", 0) ? true : false;
	for (int i = 0; i < PasteToWeb::pages; ++i) {
		char buf[256];
		int j = 0;
		{
			wchar_t *name = pasteToWebs[i]->GetName();
			while (name[j]) {
				buf[j] = (char)name[j];
				++j;
			}
			buf[j++] = '_';
		}

		mir_strncpy(buf + j, "formats", _countof(buf) - j);
		DBVARIANT forms;
		if (!g_plugin.getWString(buf, &forms)) {
			webOptions[i]->formats.clear();
			int k = 0;
			wchar_t *id = forms.pwszVal;
			wchar_t *name = nullptr;
			while (forms.pwszVal[k]) {
				if (forms.pwszVal[k] == L'=') {
					forms.pwszVal[k] = 0;
					name = forms.pwszVal + k + 1;
				}
				else if (forms.pwszVal[k] == L';') {
					forms.pwszVal[k] = 0;
					PasteFormat pf;
					pf.id = id;
					pf.name = name;
					webOptions[i]->formats.push_back(pf);
					id = forms.pwszVal + k + 1;
				}

				++k;
			}

			db_free(&forms);
		}

		mir_strncpy(buf + j, "defFormatId", _countof(buf) - j);
		DBVARIANT defForm;
		if (!g_plugin.getWString(buf, &defForm)) {
			webOptions[i]->defFormatId = defForm.pwszVal;
			db_free(&defForm);
		}

		if (webOptions[i]->isSendFileName) {
			mir_strncpy(buf + j, "sendFileName", _countof(buf) - j);
			webOptions[i]->sendFileName = g_plugin.getByte(buf, 1) ? true : false;
		}

		if (webOptions[i]->isPublicPaste) {
			mir_strncpy(buf + j, "publicPaste", _countof(buf) - j);
			webOptions[i]->publicPaste = g_plugin.getByte(buf, 0) ? true : false;
		}

		if (webOptions[i]->isCombo1) {
			mir_strncpy(buf + j, "combo1", _countof(buf) - j);
			DBVARIANT combo1;
			if (!g_plugin.getWString(buf, &combo1)) {
				webOptions[i]->combo1 = combo1.pwszVal;
				db_free(&combo1);
			}
		}

		if (webOptions[i]->isPastebin) {
			mir_strncpy(buf + j, "pastebinUserKey", _countof(buf) - j);
			DBVARIANT pastebinUserKey;
			if (!g_plugin.getWString(buf, &pastebinUserKey)) {
				webOptions[i]->pastebinUserKey = pastebinUserKey.pwszVal;
				db_free(&pastebinUserKey);
			}
		}
	}
}

WebOptions::WebOptions(int serv) :
	server(serv),
	formats(pasteToWebs[serv]->GetDefFormats()),
	sendFileName(true),
	isSendFileName(true),
	publicPaste(false),
	isPublicPaste(false),
	isCombo1(false),
	isPastebin(false)
{
	defFormatId = formats.front().id;
}

WebOptions::~WebOptions()
{
}

WebOptions* WebOptions::Copy()
{
	return new WebOptions(*this);
}
