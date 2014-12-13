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
#include "db.h"
#include "options.h"

#define ACCOUNT_PROP_NAME             _T("{BF447EBA-27AE-4DB7-893C-FC42A3F74D75}")
#define DIALOG_INITIALIZED_PROP_NAME  _T("{5EE59FE5-679A-4A29-B0A1-03092E7AC20E}")

#define POPUPS_OPTIONS_GROUP   LPGENT("Popups")
#define NETWORK_OPTIONS_GROUP  LPGENT("Network")

#define NOTIFY_SETTINGS_FROM_MOD_NAME SHORT_PLUGIN_NAME ".NotifySettingsFromModName"

#define TEST_LETTER_SUBJECT LPGENT("Why C sucks")
#define TEST_LETTER_INBOX   LPGENT("brickstrace@gmail.com [1]")
#define TEST_LETTER_SENDER  LPGENT("    bems\n")
#define TEST_LETTER_SNIP    LPGENT("* Primitive type system\n* No overloading\n* Limited possibility of data abstraction, polymorphism, subtyping and code reuse\n* No metaprogramming except preprocessor macros\n* No exceptions")

extern HINSTANCE g_hInst;

void CheckControlsEnabled(HWND wnd)
{
	BOOL PopupsEnabled = (SendDlgItemMessage(wnd, IDC_POPUPSENABLED, BM_GETSTATE, 0, 0) & BST_CHECKED) == BST_CHECKED;
	EnableWindow(GetDlgItem(wnd, IDC_POPUPSINFULLSCREEN), PopupsEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_POPUPSINFULLSCREENLABEL), PopupsEnabled);

	BOOL CListEnabled = (SendDlgItemMessage(wnd, IDC_PSEUDOCONTACTENABLED, BM_GETSTATE, 0, 0) & BST_CHECKED) == BST_CHECKED;
	EnableWindow(GetDlgItem(wnd, IDC_CLEARPSEUDOCONTACTLOG), CListEnabled);

	EnableWindow(GetDlgItem(wnd, IDC_MARKEVENTREAD), PopupsEnabled && CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_ADDSNIP), PopupsEnabled || CListEnabled);

	EnableWindow(GetDlgItem(wnd, IDC_MAILBOXVIEWLABEL), PopupsEnabled || CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_UNKNOWNVIEW), PopupsEnabled || CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_STANDARDVIEW), PopupsEnabled || CListEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_HTMLVIEW), PopupsEnabled || CListEnabled);

	EnableWindow(GetDlgItem(wnd, IDC_MAILBOXVIEWLABEL), PopupsEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_UNKNOWNVIEW), PopupsEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_STANDARDVIEW), PopupsEnabled);
	EnableWindow(GetDlgItem(wnd, IDC_HTMLVIEW), PopupsEnabled);
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
	DWORD controls = db_get_dw(NULL, NOTIFY_SETTINGS_FROM_MOD_NAME, mod, 0);
	ReadCheckbox(wnd, IDC_POPUPSENABLED, controls);
	ReadCheckbox(wnd, IDC_PSEUDOCONTACTENABLED, controls);
	ReadCheckbox(wnd, IDC_CLEARPSEUDOCONTACTLOG, controls);
	ReadCheckbox(wnd, IDC_POPUPSINFULLSCREEN, controls);
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
		GetCheckboxSaveValue(wnd, IDC_MARKEVENTREAD) |
		GetCheckboxSaveValue(wnd, IDC_AUTHONMAILBOX) |
		GetCheckboxSaveValue(wnd, IDC_ADDSNIP) |
		GetCheckboxSaveValue(wnd, IDC_UNKNOWNVIEW) |
		GetCheckboxSaveValue(wnd, IDC_STANDARDVIEW) |
		GetCheckboxSaveValue(wnd, IDC_HTMLVIEW);

	db_set_dw(NULL, NOTIFY_SETTINGS_FROM_MOD_NAME, mod, controls);
}

/////////////////////////////////////////////////////////////////////////////////////////

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
		if (!((LPNMHDR)lParam)->idFrom && ((LPNMHDR)lParam)->code == PSN_APPLY)
			SaveControls(wnd, (LPCSTR)GetProp(wnd, ACCOUNT_PROP_NAME));
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void ShowTestPopup(HWND wnd)
{
	POPUPDATAT data = { 0 };
	mir_sntprintf(data.lptzContactName, SIZEOF(data.lptzContactName), TranslateTS(TEST_LETTER_INBOX));
	mir_sntprintf(data.lptzText, SIZEOF(data.lptzText), TranslateTS(FULL_NOTIFICATION_FORMAT), TranslateTS(TEST_LETTER_SUBJECT), TranslateTS(TEST_LETTER_SENDER), TranslateTS(TEST_LETTER_SNIP));

	int len = SendDlgItemMessage(wnd, IDC_TIMEOUTEDIT, WM_GETTEXTLENGTH, 0, 0) + 1;
	LPTSTR timeout = (LPTSTR)malloc(len * sizeof(TCHAR));
	__try {
		GetDlgItemText(wnd, IDC_TIMEOUTEDIT, timeout, len);
		data.iSeconds = _ttoi(timeout);
	}
	__finally {
		free(timeout);
	}

	extern HICON g_hPopupIcon;
	data.lchIcon = g_hPopupIcon;
	data.colorBack = (COLORREF)SendDlgItemMessage(wnd, IDC_BACKCOLORPICKER, CPM_GETCOLOUR, 0, 0);
	data.colorText = (COLORREF)SendDlgItemMessage(wnd, IDC_TEXTCOLORPICKER, CPM_GETCOLOUR, 0, 0);
	if (data.colorBack == data.colorText) {
		data.colorBack = 0;
		data.colorText = 0;
	}
	PUAddPopupT(&data);
}

INT_PTR CALLBACK PopupsOptionsDlgProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR timeout[20];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(wnd);
		SendDlgItemMessage(wnd, IDC_BACKCOLORPICKER, CPM_SETCOLOUR, 0, (LPARAM)db_get_dw(0, SHORT_PLUGIN_NAME, BACK_COLOR_SETTING, 0));
		SendDlgItemMessage(wnd, IDC_TEXTCOLORPICKER, CPM_SETCOLOUR, 0, (LPARAM)db_get_dw(0, SHORT_PLUGIN_NAME, TEXT_COLOR_SETTING, 0));

		_itot(db_get_dw(0, SHORT_PLUGIN_NAME, TIMEOUT_SETTING, 0), timeout, 10);
		SetDlgItemText(wnd, IDC_TIMEOUTEDIT, timeout);

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
		if (!((LPNMHDR)lParam)->idFrom && ((LPNMHDR)lParam)->code == PSN_APPLY)
			db_set_dw(0, SHORT_PLUGIN_NAME, BACK_COLOR_SETTING, (DWORD)SendDlgItemMessage(wnd, IDC_BACKCOLORPICKER, CPM_GETCOLOUR, 0, 0));
		db_set_dw(0, SHORT_PLUGIN_NAME, TEXT_COLOR_SETTING, (DWORD)SendDlgItemMessage(wnd, IDC_TEXTCOLORPICKER, CPM_GETCOLOUR, 0, 0));

		GetDlgItemText(wnd, IDC_TIMEOUTEDIT, timeout, SIZEOF(timeout));
		db_set_dw(0, SHORT_PLUGIN_NAME, TIMEOUT_SETTING, _ttoi(timeout));
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int OptionsInitialization(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.ptszTitle = MAIL_NOTIFICATIONS;
		odp.pfnDlgProc = PopupsOptionsDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPSETTINGS);
		odp.hInstance = g_hInst;
		odp.ptszGroup = POPUPS_OPTIONS_GROUP;
		odp.flags = ODPF_UNICODE | ODPF_USERINFOTAB;
		Options_AddPage(wParam, &odp);
	}

	for (int i=0; i < g_accs.getCount(); i++) {
		LPCSTR szProto = g_accs[i]->m_pa->szModuleName;
		PROTOACCOUNT *pa = ProtoGetAccount(szProto);
		if (pa != NULL) {
			OPTIONSDIALOGPAGE odp = { sizeof(odp) };
			odp.ptszTitle = pa->tszAccountName;
			odp.pfnDlgProc = AccOptionsDlgProc;
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAILSETTINGS);
			odp.hInstance = g_hInst;
			odp.ptszGroup = NETWORK_OPTIONS_GROUP;
			odp.flags = ODPF_UNICODE | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
			odp.ptszTab = MAIL_NOTIFICATIONS;
			odp.dwInitParam = (LPARAM)szProto;
			Options_AddPage(wParam, &odp);
		}
	}
	return 0;
}
