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
#include "Options.h"
#include "resource.h"
#include "PasteToWeb.h"
#include "PasteToWeb1.h"

extern HINSTANCE hInst;

#define MODULE				"PasteIt"

Options *Options::instance;
extern PasteToWeb* pasteToWebs[PasteToWeb::pages];

struct TCpTable {
	UINT cpId;
	TCHAR *cpName;
}
cpTable[] = {
	{ CP_ACP,	_T("Use default codepage")	 },
	{ CP_UTF8,	_T("UTF-8")	 },
	{ 874,	_T("Thai")	 },
	{ 932,	_T("Japanese") },
	{ 936,	_T("Simplified Chinese") },
	{ 949,	_T("Korean") },
	{ 950,	_T("Traditional Chinese") },
	{ 1250,	_T("Central European") },
	{ 1251,	_T("Cyrillic") },
	{ 20866,  _T("Cyrillic KOI8-R") },
	{ 1252,	_T("Latin I") },
	{ 1253,	_T("Greek") },
	{ 1254,	_T("Turkish") },
	{ 1255,	_T("Hebrew") },
	{ 1256,	_T("Arabic") },
	{ 1257,	_T("Baltic") },
	{ 1258,	_T("Vietnamese") },
	{ 1361,	_T("Korean (Johab)") },
	{ CP_UTF7,	_T("UTF-7")	 },
	{ 1200,	_T("UTF-16") },
	{ 1201,	_T("UTF-16BE")	 }
};

Options::Options()
{
	defWeb = 0;
	codepage = CP_ACP;
	autoUTF = true;
	confDlg = true;
	autoSend = false;
	for(int i = 0 ; i < PasteToWeb::pages; ++i)
	{
		webOptions[i] = new WebOptions(i);
	}
}


Options::~Options()
{
	for(int i = 0 ; i < PasteToWeb::pages; ++i)
	{
		delete webOptions[i];
		webOptions[i] = NULL;
	}
}

void DefWebPageChanged();

INT_PTR CALLBACK Options::DlgProcOptsMain(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_AUTOUTF, instance->autoUTF ? 1 : 0);
			CheckDlgButton(hwndDlg, IDC_CONFDLG, instance->confDlg ? 1 : 0);
			CheckDlgButton(hwndDlg, IDC_AUTOSEND, instance->autoSend ? 1 : 0);

			for(int i = 0; i < PasteToWeb::pages; ++i)
			{
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_WEBLIST), pasteToWebs[i]->GetName());
			}

			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_WEBLIST), instance->defWeb);	

			InitCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), instance->codepage);
			return TRUE;
		}
		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDC_CODEPAGE)
			{
				if(HIWORD(wParam) == CBN_KILLFOCUS)
				{
					GetCodepageCB(GetDlgItem(hwndDlg, IDC_CODEPAGE), true, instance->codepage);
				}
			}

			if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam)==CBN_SELCHANGE || HIWORD(wParam)==CBN_EDITCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		case WM_NOTIFY:
		{
			if(((LPNMHDR)lParam)->code == PSN_APPLY) 
			{
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
		for(int i = 0 ; i < PasteToWeb::pages; ++i)
		{
			webOptions[i] = Options::instance->webOptions[i]->Copy();
		}
	}

	~OptsPagesData()
	{
		for(int i = 0 ; i < PasteToWeb::pages; ++i)
		{
			delete webOptions[i];
			webOptions[i] = NULL;
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
	while(ComboBox_DeleteString(hWnd, 0) > 0);
	int sel = 0;
	int i = 0;
	for(std::list<PasteFormat>::iterator it = wo->formats.begin(); it != wo->formats.end(); ++it)
	{
		ComboBox_AddString(hWnd, it->name.c_str());
		if(it->id == wo->defFormatId)
			sel = i;
		++i;
	}
	if(!wo->formats.empty())
	{
		ComboBox_SetCurSel(hWnd, sel);
	}
}

void GetPagesSettings(HWND hwndDlg, OptsPagesData* optsPagesData)
{
	int selected = optsPagesData->lastPage;
	int format = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_DEFFORMAT));
	if(format >= 0 && format < optsPagesData->webOptions[selected]->formats.size())
	{
		for(std::list<PasteFormat>::iterator it = optsPagesData->webOptions[selected]->formats.begin(); it != optsPagesData->webOptions[selected]->formats.end(); ++it)
		{
			if(format-- <= 0)
			{
				optsPagesData->webOptions[selected]->defFormatId = it->id;
				break;
			}
		}
	}
	
	if(optsPagesData->webOptions[selected]->isSendFileName)
		optsPagesData->webOptions[selected]->sendFileName = IsDlgButtonChecked(hwndDlg, IDC_AUTOFORMAT) ? true : false;
	if(optsPagesData->webOptions[selected]->isPublicPaste)
		optsPagesData->webOptions[selected]->publicPaste = IsDlgButtonChecked(hwndDlg, IDC_PUBLICPASTE) ? true : false;
	if(optsPagesData->webOptions[selected]->isCombo1)
	{
		int sel = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBO1));	
		if(sel >= 0 && sel < optsPagesData->webOptions[selected]->combo1Values.size())
		{
			for(std::list<PasteFormat>::iterator it = optsPagesData->webOptions[selected]->combo1Values.begin(); it != optsPagesData->webOptions[selected]->combo1Values.end(); ++it)
			{
				if(sel-- <= 0)
				{
					optsPagesData->webOptions[selected]->combo1 = it->id;
					break;
				}
			}
		}
	}
	if(optsPagesData->webOptions[selected]->isPublicPaste)
	{
		if(IsDlgButtonChecked(hwndDlg, IDC_GUEST))
		{
			optsPagesData->webOptions[selected]->pastebinUserKey = _T("");
		}
		else
		{
			TCHAR buf[100];
			Edit_GetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), buf, 100);
			optsPagesData->webOptions[selected]->pastebinUserKey = buf;
		}
	}
}

INT_PTR CALLBACK Options::DlgProcOptsPages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			OptsPagesData* optsPagesData = new OptsPagesData();
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)optsPagesData);
			
			HWND btnhwnd = GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN);
			int btSize = Button_GetTextLength(btnhwnd);
			if(btSize > 20)
			{
				SetWindowPos(btnhwnd, NULL, 0, 0, 115, 34, SWP_NOZORDER | SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOACTIVATE | SWP_NOREDRAW);
			}

			for(int i = 0; i < PasteToWeb::pages; ++i)
			{
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_WEBPAGE), pasteToWebs[i]->GetName());
			}

			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_WEBPAGE), 0);	
			ReloadFormatsCombo(GetDlgItem(hwndDlg, IDC_DEFFORMAT), optsPagesData->webOptions[0]);
			if(optsPagesData->webOptions[0]->isSendFileName)
				CheckDlgButton(hwndDlg, IDC_AUTOFORMAT, optsPagesData->webOptions[0]->sendFileName ? 1 : 0);
			else
				ShowWindow(GetDlgItem(hwndDlg,IDC_AUTOFORMAT),SW_HIDE);

			if(Options::instance->webOptions[0]->isPublicPaste)
				CheckDlgButton(hwndDlg, IDC_PUBLICPASTE, Options::instance->webOptions[0]->publicPaste ? 1 : 0);
			else
				ShowWindow(GetDlgItem(hwndDlg,IDC_PUBLICPASTE),SW_HIDE);

			if(Options::instance->webOptions[0]->isCombo1)
			{
				int sel = 0;
				int i = 0;
				std::wstring &val = Options::instance->webOptions[0]->combo1;
				HWND cb = GetDlgItem(hwndDlg, IDC_COMBO1);
				for(std::list<PasteFormat>::iterator it = Options::instance->webOptions[0]->combo1Values.begin(); it != Options::instance->webOptions[0]->combo1Values.end(); ++it)
				{
					ComboBox_AddString(cb, it->name.c_str());
					if(val == it->id)
						sel = i;
					++i;
				}
				ComboBox_SetCurSel(cb, sel);	
				Static_SetText(GetDlgItem(hwndDlg,IDC_COMBO1_DESC), Options::instance->webOptions[0]->combo1Desc.c_str());
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1),SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1_DESC),SW_HIDE);
			}

			Edit_LimitText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), 99);
			if(Options::instance->webOptions[0]->isPastebin)
			{
				std::wstring pastebinUserKey = Options::instance->webOptions[0]->pastebinUserKey;
				if(pastebinUserKey.empty())
				{
					CheckDlgButton(hwndDlg, IDC_GUEST, TRUE);
					Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), FALSE);
					Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), FALSE);
					Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), FALSE);
				}
				else
				{
					Edit_SetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), pastebinUserKey.c_str());
				}
			}
			else
			{
				ShowWindow(GetDlgItem(hwndDlg, IDC_GUEST), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), SW_HIDE);
			}

			optsPagesData->init = true;
			return TRUE;
		}
		case WM_COMMAND:
		{
			OptsPagesData* optsPagesData = (OptsPagesData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_CONFIGURE)
			{
				GetPagesSettings(hwndDlg, optsPagesData);
				OptsConfigureData d;
				d.page = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_WEBPAGE));
				WebOptions* wo = optsPagesData->webOptions[d.page];
				d.tempFormats = wo->formats;
				if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_CONFIGURE), hwndDlg, DlgProcOptsConfigure, (LPARAM)&d) == IDOK)
				{
					wo->formats = d.tempFormats;
					ReloadFormatsCombo(GetDlgItem(hwndDlg, IDC_DEFFORMAT), wo);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			else if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_PASTEBIN_LOGIN)
			{
				OptsLoginData d;
				if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DLG_PASTEBIN_LOGIN), hwndDlg, DlgProcOptsLogin, (LPARAM)&d) == IDOK)
				{
					PasteToWeb1 *web = (PasteToWeb1*)pasteToWebs[optsPagesData->lastPage];
					Edit_SetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), web->GetUserKey(d.user, d.password).c_str());
				}
			}
			else if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_GUEST)
			{
				BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_GUEST) ? FALSE : TRUE;
				Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), enabled);
				Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), enabled);
				Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), enabled);
			}
			else if(HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_WEBPAGE)
			{
				GetPagesSettings(hwndDlg, optsPagesData);
				optsPagesData->init = false;
				optsPagesData->lastPage = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_WEBPAGE));
				ReloadFormatsCombo(GetDlgItem(hwndDlg, IDC_DEFFORMAT), optsPagesData->webOptions[optsPagesData->lastPage]);
				if(optsPagesData->webOptions[optsPagesData->lastPage]->isSendFileName)
				{
					ShowWindow(GetDlgItem(hwndDlg,IDC_AUTOFORMAT),SW_SHOW);
					CheckDlgButton(hwndDlg, IDC_AUTOFORMAT, optsPagesData->webOptions[optsPagesData->lastPage]->sendFileName ? 1 : 0);
				}
				else
					ShowWindow(GetDlgItem(hwndDlg,IDC_AUTOFORMAT),SW_HIDE);

				if(optsPagesData->webOptions[optsPagesData->lastPage]->isPublicPaste)
				{
					ShowWindow(GetDlgItem(hwndDlg,IDC_PUBLICPASTE),SW_SHOW);
					CheckDlgButton(hwndDlg, IDC_PUBLICPASTE, optsPagesData->webOptions[optsPagesData->lastPage]->publicPaste ? 1 : 0);
				}
				else
					ShowWindow(GetDlgItem(hwndDlg,IDC_PUBLICPASTE),SW_HIDE);

				if(optsPagesData->webOptions[optsPagesData->lastPage]->isCombo1)
				{
					ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1),SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1_DESC),SW_SHOW);
					int sel = 0;
					int i = 0;
					std::wstring &val = optsPagesData->webOptions[optsPagesData->lastPage]->combo1;
					HWND cb = GetDlgItem(hwndDlg, IDC_COMBO1);
					while(ComboBox_DeleteString(cb, 0) > 0);
					for(std::list<PasteFormat>::iterator it = optsPagesData->webOptions[optsPagesData->lastPage]->combo1Values.begin(); it != optsPagesData->webOptions[optsPagesData->lastPage]->combo1Values.end(); ++it)
					{
						ComboBox_AddString(cb, it->name.c_str());
						if(val == it->id)
							sel = i;
						++i;
					}
					ComboBox_SetCurSel(cb, sel);	
					SetWindowText(GetDlgItem(hwndDlg,IDC_COMBO1_DESC), optsPagesData->webOptions[optsPagesData->lastPage]->combo1Desc.c_str());
				}
				else
				{
					ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1),SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg,IDC_COMBO1_DESC),SW_HIDE);
				}

				
				if(optsPagesData->webOptions[optsPagesData->lastPage]->isPastebin)
				{
					ShowWindow(GetDlgItem(hwndDlg, IDC_GUEST), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), SW_SHOW);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), SW_SHOW);
					std::wstring pastebinUserKey = optsPagesData->webOptions[optsPagesData->lastPage]->pastebinUserKey;
					if(pastebinUserKey.empty())
					{
						CheckDlgButton(hwndDlg, IDC_GUEST, TRUE);
						Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), FALSE);
						Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), FALSE);
						Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), FALSE);
					}
					else
					{
						CheckDlgButton(hwndDlg, IDC_GUEST, FALSE);
						Edit_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), TRUE);
						Static_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), TRUE);
						Button_Enable(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), TRUE);
					}
					
					Edit_SetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), pastebinUserKey.c_str());
				}
				else
				{
					ShowWindow(GetDlgItem(hwndDlg, IDC_GUEST), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_KEY_DESC), SW_HIDE);
					ShowWindow(GetDlgItem(hwndDlg, IDC_PASTEBIN_LOGIN), SW_HIDE);
				}

				optsPagesData->init = true;
			}
			
			if (optsPagesData->init && ((HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) != IDC_CONFIGURE && LOWORD(wParam) != IDC_PASTEBIN_LOGIN) || (HIWORD(wParam)==CBN_SELCHANGE && LOWORD(wParam) != IDC_WEBPAGE) || HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		case WM_NOTIFY:
		{
			if(((LPNMHDR)lParam)->code == PSN_APPLY) 
			{
				OptsPagesData* optsPagesData = (OptsPagesData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				GetPagesSettings(hwndDlg, optsPagesData);
				for(int i = 0 ; i < PasteToWeb::pages; ++i)
				{
					*Options::instance->webOptions[i] = *optsPagesData->webOptions[i];
				}

				Options::instance->Save();
			}
			return TRUE;
		}
		case WM_CLOSE:
		{
			OptsPagesData* optsPagesData = (OptsPagesData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			delete optsPagesData;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, NULL);
			break;
		}
	}

	return FALSE;
}

void SelectLbConfigure(HWND hwndDlg, int sel, OptsConfigureData* optsConfigureData)
{
	if(sel >= 0 && sel < optsConfigureData->tempFormats.size())
	{
		Button_Enable(GetDlgItem(hwndDlg, IDC_DELETE), TRUE);
		if(sel == 0)
			Button_Enable(GetDlgItem(hwndDlg, IDC_UP), FALSE);
		else
			Button_Enable(GetDlgItem(hwndDlg, IDC_UP), TRUE);
		if(sel + 1 == optsConfigureData->tempFormats.size())
			Button_Enable(GetDlgItem(hwndDlg, IDC_DOWN), FALSE);
		else
			Button_Enable(GetDlgItem(hwndDlg, IDC_DOWN), TRUE);
	}
	else
	{
		Button_Enable(GetDlgItem(hwndDlg, IDC_DELETE), FALSE);
		Button_Enable(GetDlgItem(hwndDlg, IDC_UP), FALSE);
		Button_Enable(GetDlgItem(hwndDlg, IDC_DOWN), FALSE);
	}
}

INT_PTR CALLBACK Options::DlgProcOptsConfigure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			OptsConfigureData* optsConfigureData = (OptsConfigureData*)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			
			HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
			for(std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it)
			{
				ListBox_AddString(lb, it->name.c_str());
			}

			SelectLbConfigure(hwndDlg, -1, optsConfigureData);
			return TRUE;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED) 
			{
				if (LOWORD(wParam) == IDOK)
				{
					EndDialog(hwndDlg, IDOK);
				}
				else if (LOWORD(wParam) == IDCANCEL)
				{
					EndDialog(hwndDlg, IDCANCEL);
				}
				else if (LOWORD(wParam) == IDC_DELETE)
				{
					HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
					OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int sel = ListBox_GetCurSel(lb);
					if(sel >= 0)
					{
						ListBox_DeleteString(lb, sel);
						int i = sel;
						for(std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it)
						{
							if(i-- <= 0)
							{
								optsConfigureData->tempFormats.erase(it);
								ListBox_SetCurSel(lb, sel);
								SelectLbConfigure(hwndDlg, sel, optsConfigureData);
								break;
							}
						}
					}
				}
				else if (LOWORD(wParam) == IDC_UP)
				{
					HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
					OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int sel = ListBox_GetCurSel(lb);
					if(sel > 0)
					{
						int i = sel;
						for(std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it)
						{
							if(i-- <= 0)
							{
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
				else if (LOWORD(wParam) == IDC_DOWN)
				{
					HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
					OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					int sel = ListBox_GetCurSel(lb);
					if(sel >= 0 && sel + 1 < optsConfigureData->tempFormats.size())
					{
						int i = sel;
						for(std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it)
						{
							if(i-- <= 0)
							{
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
				else if(LOWORD(wParam) == IDC_DOWNLOAD)
				{
					OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
					while(ListBox_DeleteString(lb, 0) > 0);
					SelectLbConfigure(hwndDlg, -1, optsConfigureData);
					optsConfigureData->tempFormats = pasteToWebs[optsConfigureData->page]->GetFormats();
					for(std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it)
					{
						ListBox_AddString(lb, it->name.c_str());
					}
				}
				else if(LOWORD(wParam) == IDC_RESTORE)
				{
					OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					HWND lb = GetDlgItem(hwndDlg, IDC_FORMATTING);
					while(ListBox_DeleteString(lb, 0) > 0);
					SelectLbConfigure(hwndDlg, -1, optsConfigureData);
					optsConfigureData->tempFormats = pasteToWebs[optsConfigureData->page]->GetDefFormats();
					for(std::list<PasteFormat>::iterator it = optsConfigureData->tempFormats.begin(); it != optsConfigureData->tempFormats.end(); ++it)
					{
						ListBox_AddString(lb, it->name.c_str());
					}
				}
			}
			else if(HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_FORMATTING)
			{
				OptsConfigureData* optsConfigureData = (OptsConfigureData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				int sel = ListBox_GetCurSel(GetDlgItem(hwndDlg, IDC_FORMATTING));
				SelectLbConfigure(hwndDlg, sel, optsConfigureData);
			}
			return TRUE;
		}
		case WM_VKEYTOITEM:
			if(LOWORD(wParam) == VK_DELETE && (HWND)lParam == GetDlgItem(hwndDlg, IDC_FORMATTING))
			{
				DlgProcOptsConfigure(hwndDlg, WM_COMMAND, MAKELONG(IDC_DELETE, BN_CLICKED), NULL);
				return -2;
			}
			return -1;
		case WM_NOTIFY:
		{
			return TRUE;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK Options::DlgProcOptsLogin(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			Edit_LimitText(GetDlgItem(hwndDlg, IDC_PASTEBIN_USER), 99);
			Edit_LimitText(GetDlgItem(hwndDlg, IDC_PASTEBIN_PASSWORD), 99);
			return TRUE;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED) 
			{
				if (LOWORD(wParam) == IDOK)
				{
					TCHAR buf[100];
					OptsLoginData* optsLoginData = (OptsLoginData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					Edit_GetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_USER), buf, 100);
					optsLoginData->user = buf;
					Edit_GetText(GetDlgItem(hwndDlg, IDC_PASTEBIN_PASSWORD), buf, 100);
					optsLoginData->password = buf;
					EndDialog(hwndDlg, IDOK);
				}
				else if (LOWORD(wParam) == IDCANCEL)
				{
					EndDialog(hwndDlg, IDCANCEL);
				}
			}
			return TRUE;
		}
		case WM_NOTIFY:
		{
			return TRUE;
		}
	}

	return FALSE;
}

unsigned int Options::GetCodepageCB(HWND hwndCB, bool errorReport, unsigned int defCp)
{
	int selCpIdx = ComboBox_GetCurSel(hwndCB);
	if(selCpIdx < 0)
	{
		TCHAR text[128];
		ComboBox_GetText(hwndCB, text, 128);
		TCHAR * stopOn = NULL;
		long cp = _tcstol(text, &stopOn, 10);
		if(errorReport && (stopOn == text || *stopOn != '\0' || cp < 0 || cp > 0xffff))
		{
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
	DBWriteContactSettingWString(0, MODULE, "defWeb", pasteToWebs[web]->GetName());
	DefWebPageChanged();
}

void Options::InitCodepageCB(HWND hwndCB, unsigned int codepage)
{
	int cpCount = sizeof(cpTable) / sizeof(cpTable[0]);
	int selCpIdx = -1;
	for(int i = 0; i < cpCount; ++i)
	{
		ComboBox_AddString(hwndCB, TranslateTS(cpTable[i].cpName));
		if(cpTable[i].cpId == codepage)
			selCpIdx = i;
	}

	if(selCpIdx == -1)
	{
		TCHAR buf[10];
		_stprintf_s(buf, 10, _T("%d"), codepage);
		ComboBox_SetText(hwndCB, buf);	
	}
	else
	{
		ComboBox_SetCurSel(hwndCB, selCpIdx);	
	}

	ComboBox_LimitText(hwndCB, 127);
}

void Options::SetCodepageCB(HWND hwndCB, unsigned int codepage)
{
	int cpCount = sizeof(cpTable) / sizeof(cpTable[0]);
	int selCpIdx = -1;
	for(int i = 0; i < cpCount; ++i)
	{
		if(cpTable[i].cpId == codepage)
			selCpIdx = i;
	}

	if(selCpIdx == -1)
	{
		TCHAR buf[10];
		_stprintf_s(buf, 10, _T("%d"), codepage);
		ComboBox_SetText(hwndCB, buf);	
	}
	else
	{
		ComboBox_SetCurSel(hwndCB, selCpIdx);	
	}
}

int Options::InitOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};

	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.ptszTitle = LPGENT("Paste It");
	odp.ptszGroup = LPGENT("Services");

	odp.ptszTab = LPGENT("Main");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = Options::DlgProcOptsMain;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	odp.ptszTab = LPGENT("Web page");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PAGES);
	odp.pfnDlgProc = Options::DlgProcOptsPages;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}

void Options::Save()
{
	DBWriteContactSettingWString(0, MODULE, "defWeb", pasteToWebs[defWeb]->GetName());
	DBWriteContactSettingDword(0, MODULE, "codepage", codepage);
	DBWriteContactSettingByte(0, MODULE, "autoUTF", autoUTF ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "confDlg", confDlg ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "autoSend", autoSend ? 1 : 0);
	for(int i = 0 ; i < PasteToWeb::pages; ++i)
	{
		char buf[256];
		TCHAR* name = pasteToWebs[i]->GetName();
		int j = 0;
		while(name[j])
		{
			buf[j] = (char)name[j];
			++j;
		}
		buf[j++] = '_';

		strcpy_s(buf + j, 256 - j, "formats");
		std::wstring forms;
		for(std::list<PasteFormat>::iterator it = webOptions[i]->formats.begin(); it != webOptions[i]->formats.end(); ++it)
		{
			forms += it->id + L'=' + it->name + L';';
		}

		DBWriteContactSettingWString(0, MODULE, buf, forms.c_str());

		strcpy_s(buf + j, 256 - j, "defFormatId");
		DBWriteContactSettingWString(0, MODULE, buf, webOptions[i]->defFormatId.c_str());

		if(webOptions[i]->isSendFileName)
		{
			strcpy_s(buf + j, 256 - j, "sendFileName");
			DBWriteContactSettingByte(0, MODULE, buf, webOptions[i]->sendFileName ? 1 : 0);
		}
		
		if(webOptions[i]->isPublicPaste)
		{
			strcpy_s(buf + j, 256 - j, "publicPaste");
			DBWriteContactSettingByte(0, MODULE, buf, webOptions[i]->publicPaste ? 1 : 0);
		}

		if(webOptions[i]->isCombo1)
		{
			strcpy_s(buf + j, 256 - j, "combo1");
			DBWriteContactSettingWString(0, MODULE, buf, webOptions[i]->combo1.c_str());
		}

		if(webOptions[i]->isPastebin)
		{
			strcpy_s(buf + j, 256 - j, "pastebinUserKey");
			DBWriteContactSettingWString(0, MODULE, buf, webOptions[i]->pastebinUserKey.c_str());
		}
	}
}

void Options::Load()
{
	DBVARIANT defWebV;
	if(!DBGetContactSettingWString(0, MODULE, "defWeb", &defWebV))
	{
		for(int i = 0; i < PasteToWeb::pages; ++i)
		{
			if(!wcscmp(pasteToWebs[i]->GetName(), defWebV.pwszVal))
			{
				defWeb = i;
				break;
			}
		}
		DBFreeVariant(&defWebV);
	}
	codepage = DBGetContactSettingDword(0, MODULE, "codepage", CP_ACP);
	autoUTF = DBGetContactSettingByte(0, MODULE, "autoUTF", 1) ? true : false;
	confDlg = DBGetContactSettingByte(0, MODULE, "confDlg", 1) ? true : false;
	autoSend = DBGetContactSettingByte(0, MODULE, "autoSend", 0) ? true : false;
	for(int i = 0 ; i < PasteToWeb::pages; ++i)
	{
		char buf[256];
		TCHAR* name = pasteToWebs[i]->GetName();
		int j = 0;
		while(name[j])
		{
			buf[j] = (char)name[j];
			++j;
		}
		buf[j++] = '_';

		strcpy_s(buf + j, 256 - j, "formats");
		DBVARIANT forms;
		if(!DBGetContactSettingWString(0, MODULE, buf, &forms))
		{
			webOptions[i]->formats.clear();
			int k = 0;
			wchar_t * id = forms.pwszVal;
			wchar_t * name = NULL;
			while(forms.pwszVal[k])
			{
				if(forms.pwszVal[k] == L'=')
				{
					forms.pwszVal[k] = 0;
					name = forms.pwszVal + k + 1;
				}
				else if(forms.pwszVal[k] == L';')
				{
					forms.pwszVal[k] = 0;
					PasteFormat pf;
					pf.id = id;
					pf.name = name;
					webOptions[i]->formats.push_back(pf);
					id = forms.pwszVal + k + 1;
				}

				++k;
			}

			DBFreeVariant(&forms);
		}

		strcpy_s(buf + j, 256 - j, "defFormatId");
		DBVARIANT defForm;
		if(!DBGetContactSettingWString(0, MODULE, buf, &defForm))
		{
			webOptions[i]->defFormatId = defForm.pwszVal;
			DBFreeVariant(&defForm);
		}

		if(webOptions[i]->isSendFileName)
		{
			strcpy_s(buf + j, 256 - j, "sendFileName");
			webOptions[i]->sendFileName = DBGetContactSettingByte(0, MODULE, buf, 1) ? true : false;
		}
		
		if(webOptions[i]->isPublicPaste)
		{
			strcpy_s(buf + j, 256 - j, "publicPaste");
			webOptions[i]->publicPaste = DBGetContactSettingByte(0, MODULE, buf, 0) ? true : false;
		}

		if(webOptions[i]->isCombo1)
		{
			strcpy_s(buf + j, 256 - j, "combo1");
			DBVARIANT combo1;
			if(!DBGetContactSettingWString(0, MODULE, buf, &combo1))
			{
				webOptions[i]->combo1 = combo1.pwszVal;
				DBFreeVariant(&combo1);
			}
		}

		if(webOptions[i]->isPastebin)
		{
			strcpy_s(buf + j, 256 - j, "pastebinUserKey");
			DBVARIANT pastebinUserKey;
			if(!DBGetContactSettingWString(0, MODULE, buf, &pastebinUserKey))
			{
				webOptions[i]->pastebinUserKey = pastebinUserKey.pwszVal;
				DBFreeVariant(&pastebinUserKey);
			}
		}
	}
}

WebOptions::WebOptions(int serv)
	:server(serv),
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
