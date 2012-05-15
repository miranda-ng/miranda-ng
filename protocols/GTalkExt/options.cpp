//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "stdafx.h"
#include "options.h"

static const LPTSTR ACCOUNT_PROP_NAME = _T("{BF447EBA-27AE-4DB7-893C-FC42A3F74D75}");
static const LPTSTR DIALOG_INITIALIZED_PROP_NAME = _T("{5EE59FE5-679A-4A29-B0A1-03092E7AC20E}");

static const LPTSTR POPUPS_OPTIONS_GROUP = _T("Popups");
static const LPTSTR NETWORK_OPTIONS_GROUP = _T("Network");

static const LPSTR NOTIFY_SETTINGS_FROM_MOD_NAME = SHORT_PLUGIN_NAME ".NotifySettingsFromModName";

static const LPTSTR TEST_LETTER_SUBJECT = _T("Why C sucks");
static const LPTSTR TEST_LETTER_INBOX = _T("brickstrace@gmail.com [1]");
static const LPTSTR TEST_LETTER_SENDER = _T("    bems <bems@vingrad.ru>\n");
static const LPTSTR TEST_LETTER_SNIP =
	_T("* Primitive type system\n")
	_T("* No overloading\n")
	_T("* Limited possibility of data abstraction, polymorphism, subtyping and code reuse\n")
	_T("* No metaprogramming except preprocessor macros\n")
	_T("* No exceptions");

HANDLE hOptionsHook = 0;
extern HINSTANCE hInst;

void CheckControlsEnabled(HWND wnd)
{
	BOOL PopupsEnabled = (SendMessage(GetDlgItem(wnd, IDC_POPUPSENABLED), BM_GETSTATE, 0, 0) & BST_CHECKED) == BST_CHECKED;
	EnableWindow(GetDlgItem(wnd, IDC_POPUPSINFULLSCREEN), PopupsEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_POPUPSINFULLSCREENLABEL), PopupsEnabled);

	BOOL CListEnabled = (SendMessage(GetDlgItem(wnd, IDC_PSEUDOCONTACTENABLED), BM_GETSTATE, 0, 0) & BST_CHECKED) == BST_CHECKED;
	EnableWindow(GetDlgItem(wnd, IDC_CLEARPSEUDOCONTACTLOG), CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_SUPRESSFOREIGN), CListEnabled);

	EnableWindow(GetDlgItem(wnd, IDC_MARKEVENTREAD), PopupsEnabled && CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_ADDSNIP), PopupsEnabled || CListEnabled);

	EnableWindow(GetDlgItem(wnd, IDC_MAILBOXVIEWLABEL), PopupsEnabled || CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_UNKNOWNVIEW), PopupsEnabled || CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_STANDARDVIEW), PopupsEnabled || CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_HTMLVIEW), PopupsEnabled || CListEnabled);
}

BOOL ReadCheckbox(HWND wnd, int id, DWORD controls)
{
	BOOL result = ((controls >> (id - IDC_BASE)) & 1);
	if (id != IDC_STANDARDVIEW && id != IDC_HTMLVIEW) result = !result;
	if (wnd) Button_SetCheck(GetDlgItem(wnd, id), result);
	return result;
}

DWORD ReadCheckboxes(HWND wnd, LPCSTR mod)
{
	DWORD controls = DBGetContactSettingDword(0, NOTIFY_SETTINGS_FROM_MOD_NAME, mod, 0);
	ReadCheckbox(wnd, IDC_POPUPSENABLED, controls);
	ReadCheckbox(wnd, IDC_PSEUDOCONTACTENABLED, controls);
	ReadCheckbox(wnd, IDC_CLEARPSEUDOCONTACTLOG, controls);
	ReadCheckbox(wnd, IDC_POPUPSINFULLSCREEN, controls);
	ReadCheckbox(wnd, IDC_SUPRESSFOREIGN, controls);
	ReadCheckbox(wnd, IDC_MARKEVENTREAD, controls);
	ReadCheckbox(wnd, IDC_AUTHONMAILBOX, controls);
	ReadCheckbox(wnd, IDC_ADDSNIP, controls);
	ReadCheckbox(wnd, IDC_UNKNOWNVIEW, controls);
	ReadCheckbox(wnd, IDC_STANDARDVIEW, controls);
	ReadCheckbox(wnd, IDC_HTMLVIEW, controls);
	return controls;
}

DWORD GetCheckboxSaveValue(HWND wnd, int id)
{
	BOOL val = Button_GetCheck(GetDlgItem(wnd, id));
	if (id != IDC_STANDARDVIEW && id != IDC_HTMLVIEW) val = !val;
	return val ? (1 << (id - IDC_BASE)) : 0;
}

void SaveControls(HWND wnd, LPCSTR mod)
{
	DWORD controls = GetCheckboxSaveValue(wnd, IDC_CLEARPSEUDOCONTACTLOG) |
		GetCheckboxSaveValue(wnd, IDC_POPUPSINFULLSCREEN) |
		GetCheckboxSaveValue(wnd, IDC_POPUPSENABLED) |
		GetCheckboxSaveValue(wnd, IDC_PSEUDOCONTACTENABLED) |
		GetCheckboxSaveValue(wnd, IDC_SUPRESSFOREIGN) |
		GetCheckboxSaveValue(wnd, IDC_MARKEVENTREAD) |
		GetCheckboxSaveValue(wnd, IDC_AUTHONMAILBOX) |
		GetCheckboxSaveValue(wnd, IDC_ADDSNIP) |
		GetCheckboxSaveValue(wnd, IDC_UNKNOWNVIEW) |
		GetCheckboxSaveValue(wnd, IDC_STANDARDVIEW) |
		GetCheckboxSaveValue(wnd, IDC_HTMLVIEW);

	DBWriteContactSettingDword(0, NOTIFY_SETTINGS_FROM_MOD_NAME, mod, controls);
}

INT_PTR CALLBACK AccOptionsDlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			SetProp(wnd, ACCOUNT_PROP_NAME, (HANDLE)lParam);
			TranslateDialogDefault(wnd);
			ReadCheckboxes(wnd, (LPCSTR)lParam);
			CheckControlsEnabled(wnd);
			break;

		case WM_CTLCOLORSTATIC:
			if (GetDlgItem(wnd, IDC_WARNBAR) == (HWND)lParam)
				return (INT_PTR)CreateSolidBrush(0x55AAFF); // orange
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_POPUPSENABLED:
				case IDC_PSEUDOCONTACTENABLED:
					if (HIWORD(wParam) == BN_CLICKED) CheckControlsEnabled(wnd);
					// no break

				case IDC_CLEARPSEUDOCONTACTLOG:
				case IDC_POPUPSINFULLSCREEN:
				case IDC_SUPRESSFOREIGN:
				case IDC_MARKEVENTREAD:
				case IDC_AUTHONMAILBOX:
				case IDC_ADDSNIP:
				case IDC_UNKNOWNVIEW:
				case IDC_STANDARDVIEW:
				case IDC_HTMLVIEW:
					if (HIWORD(wParam) == BN_CLICKED) PropSheet_Changed(GetParent(wnd), wnd);
			}
			break;

		case WM_NOTIFY:
			if (!((LPNMHDR)lParam)->idFrom  && ((LPNMHDR)lParam)->code == PSN_APPLY)
				SaveControls(wnd, (LPCSTR)GetProp(wnd, ACCOUNT_PROP_NAME));
			break;
	}
	return 0;
}

void ShowTestPopup(HWND wnd)
{
	POPUPDATAT data = {0};
	wsprintf(&data.lptzContactName[0], TEST_LETTER_INBOX);
	wsprintf(&data.lptzText[0], TranslateT(FULL_NOTIFICATION_FORMAT),
		TEST_LETTER_SUBJECT, TEST_LETTER_SENDER, TEST_LETTER_SNIP);

	int len = SendMessage(GetDlgItem(wnd, IDC_TIMEOUTEDIT), WM_GETTEXTLENGTH, 0, 0) + 1;
	LPTSTR timeout = (LPTSTR)malloc(len * sizeof(TCHAR));
	__try {
		SendMessage(GetDlgItem(wnd, IDC_TIMEOUTEDIT), WM_GETTEXT, len, (LPARAM)timeout);
		data.iSeconds = _ttoi(timeout);
	}
	__finally {
		free(timeout);
	}

	extern HICON g_hPopupIcon;
	data.lchIcon = g_hPopupIcon;
	data.colorBack = (COLORREF)SendMessage(GetDlgItem(wnd, IDC_BACKCOLORPICKER), CPM_GETCOLOUR, 0, 0);
	data.colorText = (COLORREF)SendMessage(GetDlgItem(wnd, IDC_TEXTCOLORPICKER), CPM_GETCOLOUR, 0, 0);
	if (data.colorBack == data.colorText) {
		data.colorBack = 0;
		data.colorText = 0;
	}
	CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&data, 0);
}

INT_PTR CALLBACK PopupsOptionsDlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(wnd);
			SendMessage(GetDlgItem(wnd, IDC_BACKCOLORPICKER), CPM_SETCOLOUR, 0,
				(LPARAM)DBGetContactSettingDword(0, SHORT_PLUGIN_NAME, BACK_COLOR_SETTING, 0));
			SendMessage(GetDlgItem(wnd, IDC_TEXTCOLORPICKER), CPM_SETCOLOUR, 0,
				(LPARAM)DBGetContactSettingDword(0, SHORT_PLUGIN_NAME, TEXT_COLOR_SETTING, 0));

			{LPTSTR timeout = (LPTSTR)malloc(11 * sizeof(TCHAR));
			__try {
				wsprintf(timeout, _T("%d"), DBGetContactSettingDword(0, SHORT_PLUGIN_NAME, TIMEOUT_SETTING, 0));
				SendMessage(GetDlgItem(wnd, IDC_TIMEOUTEDIT), WM_SETTEXT, 0, (LPARAM)timeout);
			}
			__finally {
				free(timeout);
			}}

			SetProp(wnd, DIALOG_INITIALIZED_PROP_NAME, (HANDLE)TRUE);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_TESTBUTTON && HIWORD(wParam) == BN_CLICKED)
				ShowTestPopup(wnd);

			if (GetProp(wnd, DIALOG_INITIALIZED_PROP_NAME))
				switch (LOWORD(wParam)) {
					case IDC_BACKCOLORPICKER:
					case IDC_TEXTCOLORPICKER:
						if (HIWORD(wParam) == CPN_COLOURCHANGED) PropSheet_Changed(GetParent(wnd), wnd);
						break;

					case IDC_TIMEOUTEDIT:
						if (HIWORD(wParam) == EN_CHANGE) PropSheet_Changed(GetParent(wnd), wnd);
				}
			break;

		case WM_NOTIFY:
			if (!((LPNMHDR)lParam)->idFrom  && ((LPNMHDR)lParam)->code == PSN_APPLY)
				DBWriteContactSettingDword(0, SHORT_PLUGIN_NAME, BACK_COLOR_SETTING,
					(DWORD)SendMessage(GetDlgItem(wnd, IDC_BACKCOLORPICKER), CPM_GETCOLOUR, 0, 0));
				DBWriteContactSettingDword(0, SHORT_PLUGIN_NAME, TEXT_COLOR_SETTING,
					(DWORD)SendMessage(GetDlgItem(wnd, IDC_TEXTCOLORPICKER), CPM_GETCOLOUR, 0, 0));

			int len = SendMessage(GetDlgItem(wnd, IDC_TIMEOUTEDIT), WM_GETTEXTLENGTH, 0, 0) + 1;
			LPTSTR timeout = (LPTSTR)malloc(len * sizeof(TCHAR));
			__try {
				SendMessage(GetDlgItem(wnd, IDC_TIMEOUTEDIT), WM_GETTEXT, len, (LPARAM)timeout);
				DBWriteContactSettingDword(0, SHORT_PLUGIN_NAME, TIMEOUT_SETTING, _ttoi(timeout));
			}
			__finally {
				free(timeout);
			}
			break;
	}
	return 0;
}

void AddPopupsPage(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.ptszTitle = _T(MAIL_NOTIFICATIONS);
	odp.pfnDlgProc = PopupsOptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPSETTINGS);
	odp.hInstance = hInst;
	odp.ptszGroup = POPUPS_OPTIONS_GROUP;
	odp.flags = ODPF_UNICODE | ODPF_USERINFOTAB;

	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
}

void AddAccPage(LPCTSTR acc, LPCSTR mod, WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.pszTitle = (LPSTR)acc;
	odp.pfnDlgProc = AccOptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAILSETTINGS);
	odp.hInstance = hInst;
	odp.ptszGroup = NETWORK_OPTIONS_GROUP;
	odp.flags = ODPF_UNICODE | ODPF_USERINFOTAB;
	odp.ptszTab =_T(MAIL_NOTIFICATIONS);
	odp.dwInitParam = (LPARAM)mod;

	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
}

int OptionsInitialization(WPARAM wParam, LPARAM lParam)
{
	int count;
	PROTOACCOUNT **accs;
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&accs);
	for (int i = 0; i < count; i++)
		if (getJabberApi(accs[i]->szModuleName)) AddAccPage(accs[i]->tszAccountName, accs[i]->szModuleName, wParam);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) AddPopupsPage(wParam);
	return FALSE;
}

BOOL HookOptionsInitialization()
{
	return (hOptionsHook = HookEvent(ME_OPT_INITIALISE, OptionsInitialization)) != 0;
}

void UnhookOptionsInitialization()
{
	if (hOptionsHook) {
		UnhookEvent(hOptionsHook);
		hOptionsHook = 0;
	}
}