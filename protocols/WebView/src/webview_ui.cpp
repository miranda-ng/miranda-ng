/*
* A plugin for Miranda IM which displays web page text in a window 
* Copyright (C) 2005 Vincent Joyce.
*
* Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
* Richard Hughes, Roland Rabien & Tristan Van de Vreede
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "webview.h"

static const wchar_t *szTrackerBarDescr[] = {
	LPGENW("No whitespace removal"),
	LPGENW("Minimal level of whitespace removal"),
	LPGENW("Medium level of whitespace removal"),
	LPGENW("Large level of whitespace removal"),
	LPGENW("Remove all whitespace")
};

static wchar_t *AlertTypes[] = { LPGENW("Popup plugin"), LPGENW("Log to file"), LPGENW("Open data display window"), LPGENW("Use OSD plugin") };
static wchar_t *EventTypes[] = { LPGENW("A string is present"), LPGENW("The web page changes"), LPGENW("A specific part of web page changes") };

#define M_FILLSCRIPTCOMBO    (WM_USER+16)

wchar_t* FixButtonText(wchar_t *url, size_t len)
{
	wchar_t buttontext[256], stringbefore[256], newbuttontext[256];
	wcsncpy_s(buttontext, url, _TRUNCATE);
	wcsncpy_s(newbuttontext, url, _TRUNCATE);

	if (wcschr(newbuttontext, '&') != nullptr) {
		while (true) {
			if (wcschr(newbuttontext, '&') == nullptr)
				break;

			wcsncpy_s(buttontext, newbuttontext, _TRUNCATE);
			wchar_t *stringafter = wcschr(buttontext, '&');
			int pos = (stringafter - buttontext);
			int posbefore = (stringafter - buttontext) - 1;
			int posafter = (stringafter - buttontext) + 1;
			strdelw(stringafter, 1);
			wcsncpy_s(stringbefore, pos, buttontext, _TRUNCATE);
			mir_snwprintf(newbuttontext, L"%s!!%s", stringbefore, stringafter);

			posafter = 0;
			posbefore = 0;
		}

		while (true) {
			if (wcschr(newbuttontext, '!') != nullptr) {
				wchar_t *stringafter = wcschr(newbuttontext, '!');
				int pos = (stringafter - newbuttontext);
				newbuttontext[pos] = '&';
			}
			if (wcschr(newbuttontext, '!') == nullptr)
				break;
		}
	}

	wcsncpy_s(url, len, newbuttontext, _TRUNCATE);
	return url;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcAlertOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND ParentHwnd;
	DBVARIANT dbv;
	int alertIndex = 0;
	int eventIndex = 0;
	static int test;
	MCONTACT hContact;

	ParentHwnd = GetParent(hwndDlg);

	switch (msg) {
	case WM_INITDIALOG:
		test = 0;
		TranslateDialogDefault(hwndDlg);
		hContact = lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		SetWindowText(hwndDlg, TranslateT("Alert options"));

		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ALERT)));

		EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 0);

		if (!g_plugin.getWString(hContact, ALERT_STRING_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_ALERT_STRING, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, ALRT_S_STRING_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_START2, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, ALRT_E_STRING_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_END2, dbv.pwszVal);
			db_free(&dbv);
		}
		CheckDlgButton(hwndDlg, IDC_ENABLE_ALERTS, g_plugin.getByte(hContact, ENABLE_ALERTS_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ADD_DATE_NAME, g_plugin.getByte(hContact, APND_DATE_NAME_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_24_HOUR, g_plugin.getByte(hContact, USE_24_HOUR_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYS_LOG, g_plugin.getByte(hContact, ALWAYS_LOG_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(hwndDlg, IDC_ALERT_TYPE, TranslateW(AlertTypes[g_plugin.getByte(hContact, ALRT_INDEX_KEY, 0)]));
		SetDlgItemText(hwndDlg, IDC_EVENT_TYPE, TranslateW(EventTypes[g_plugin.getByte(hContact, EVNT_INDEX_KEY, 0)]));

		for (auto &it: AlertTypes)
			SendDlgItemMessage(hwndDlg, IDC_ALERT_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));

		for (auto &it: EventTypes)
			SendDlgItemMessage(hwndDlg, IDC_EVENT_TYPE, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));

		if (g_plugin.getByte(hContact, ENABLE_ALERTS_KEY, 0)) {
			CheckDlgButton(hwndDlg, IDC_ENABLE_ALERTS, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ADD_DATE_NAME, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_24_HOUR, BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_PREFIX, BST_UNCHECKED);
		}
		EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_DATE_NAME), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

		if (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), 0);
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_START2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_END2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYS_LOG), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

		if (!g_plugin.getString(hContact, EVNT_INDEX_KEY, &dbv)) {
			eventIndex = g_plugin.getByte(hContact, EVNT_INDEX_KEY, 0);
			db_free(&dbv);
		}
		if (!g_plugin.getString(hContact, ALRT_INDEX_KEY, &dbv)) {
			alertIndex = g_plugin.getByte(hContact, ALRT_INDEX_KEY, 0);
			db_free(&dbv);
		}

		// alerts
		if (alertIndex == 0) // Popup
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
			}
		}
		else if (alertIndex == 1) // file
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
			}
		}
		else if (alertIndex == 2) // datawindow
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
			}
		}
		else if (alertIndex == 3) // osd
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
			}
		}

		// events
		if (eventIndex == 0) // string is present
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 1);
			}
		}
		else if (eventIndex == 1) // webpage changed
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
			}
		}
		else if (eventIndex == 2) // part of page changed
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
			}
		}

		if (!g_plugin.getWString(hContact, FILE_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_FILENAME, dbv.pwszVal);
			db_free(&dbv);
		}

		CheckDlgButton(hwndDlg, IDC_APPEND, g_plugin.getByte(hContact, APPEND_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SAVE_AS_RAW, g_plugin.getByte(hContact, SAVE_AS_RAW_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);

		if (g_plugin.getByte(hContact, CONTACT_PREFIX_KEY, 1) == 1)
			CheckRadioButton(hwndDlg, IDC_PREFIX, IDC_SUFFIX, IDC_PREFIX);
		else
			CheckRadioButton(hwndDlg, IDC_PREFIX, IDC_SUFFIX, IDC_SUFFIX);

		if (g_plugin.getByte(hContact, ALWAYS_LOG_KEY, 0)) {
			if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BROWSE:
			{
				wchar_t szFileName[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILENAME, szFileName, _countof(szFileName));

				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = nullptr;
				ofn.lpstrFilter = L"TEXT Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
				ofn.lpstrFile = szFileName;
				ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
				ofn.nMaxFile = _countof(szFileName);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = L"txt";
				if (!GetSaveFileName(&ofn))
					break;

				SetDlgItemText(hwndDlg, IDC_FILENAME, szFileName);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			}
			break;

		case IDC_ADD_DATE_NAME:
			EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			break;

		case IDC_24_HOUR:
		case IDC_SUFFIX:
		case IDC_PREFIX:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			break;

		case IDC_ALERT_STRING:
			if (HIWORD(wParam) == EN_CHANGE)
				test++;
			if (test > 1)
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			break;

		case IDC_START2:
		case IDC_END2:
			if (HIWORD(wParam) == EN_CHANGE)
				test++;
			if (test > 3)
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			break;

		case IDC_APPEND:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			break;

		case IDC_SAVE_AS_RAW:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			break;

		case IDC_ENABLE_ALERTS:
			hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_DATE_NAME), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

			if (IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_PREFIX), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SUFFIX), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_24_HOUR), 0);
			}

			EnableWindow(GetDlgItem(hwndDlg, IDC_EVENT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_TYPE), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYS_LOG), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

			eventIndex = g_plugin.getByte(hContact, EVNT_INDEX_KEY, 0);
			alertIndex = g_plugin.getByte(hContact, ALRT_INDEX_KEY, 0);

			if (eventIndex == 2) { // part of webpage changed
				EnableWindow(GetDlgItem(hwndDlg, IDC_START2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				EnableWindow(GetDlgItem(hwndDlg, IDC_END2), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
			}

			// //////// 
			if (alertIndex == 0) { // popup
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

					if (eventIndex == 2) // part of webpage changed
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
					if (eventIndex == 1) // webpage changed
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
					if (eventIndex == 0) // string present
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				}
			}
			else if (alertIndex == 1) { // log to file
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);

					if (eventIndex == 1) // webpage changed
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
					if (eventIndex == 0) // string present
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
				}
			}
			else if (alertIndex == 2) { // display window
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

					if (eventIndex == 1) // webpage changed
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
					if (eventIndex == 0) // string present
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				}
			}
			else if (alertIndex == 3) { // osd
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);

					if (eventIndex == 1) // webpage changed
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
					if (eventIndex == 0) // string present
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));
				}
			}

			if (eventIndex == 0) // string present
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)));

			EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);

			if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)) {
				if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
				}
			}
			break;

		case IDC_ALWAYS_LOG:
			EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
			break;

		case IDC_ALERT_TYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE) {
				hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				alertIndex = SendDlgItemMessage(hwndDlg, IDC_ALERT_TYPE, CB_GETCURSEL, 0, 0);

				if (HIWORD(wParam) == CBN_SELCHANGE) {
					g_plugin.setByte(hContact, ALRT_INDEX_KEY, alertIndex);
					if (alertIndex == 0) {
						// PopUp
						EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
					}
					else if (alertIndex == 1) {
						// log to file
						EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 1);
					}
					else if (alertIndex == 2) {
						// data window
						EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
					}
					else if (alertIndex == 3) {
						// OSD
						EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), 0);
					}

					if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_APPEND), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
						EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAME), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
						EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAVE_AS_RAW), (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG)));
					}
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			}
			break;

		case IDC_EVENT_TYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE) {
				hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				eventIndex = SendDlgItemMessage(hwndDlg, IDC_EVENT_TYPE, CB_GETCURSEL, 0, 0);

				if (HIWORD(wParam) == CBN_SELCHANGE) {
					g_plugin.setByte(hContact, EVNT_INDEX_KEY, eventIndex);
					if (eventIndex == 0) {
						// event when string is present
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
					}
					else if (eventIndex == 1) {
						// event when web page changes
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 0);
					}
					else if (eventIndex == 2) {
						// event when part of web page changes
						EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_STRING), 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_START2), 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_END2), 1);
					}
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 1);
			}
			break;

		case IDC_ALERT_APPLY:
		case IDC_OK2:
			{
				hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				eventIndex = g_plugin.getByte(hContact, EVNT_INDEX_KEY, 0);
				alertIndex = g_plugin.getByte(hContact, ALRT_INDEX_KEY, 0);

				g_plugin.setByte(hContact, ENABLE_ALERTS_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS));
				g_plugin.setByte(hContact, APND_DATE_NAME_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ADD_DATE_NAME));
				g_plugin.setByte(hContact, USE_24_HOUR_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_24_HOUR));
				g_plugin.setByte(hContact, ALWAYS_LOG_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG));

				//if alerts is unticked delete the cache
				if (!(IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)))
					SiteDeleted(hContact, 0);

				if (eventIndex == 0) // string present
					if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_ALERT_STRING))))
						if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
							MessageBox(nullptr, TranslateT("You need to supply a search string."), _A2W(MODULENAME), MB_OK);
							break;
						}

				if (eventIndex == 2) // part of web page changed
					if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
						if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_START2)))) {
							MessageBox(nullptr, TranslateT("You need to supply a start string."), _A2W(MODULENAME), MB_OK);
							break;
						}
						if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_END2)))) {
							MessageBox(nullptr, TranslateT("You need to supply an end string."), _A2W(MODULENAME), MB_OK);
							break;
						}
					}

				if (alertIndex == 1) // log to file
					if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FILENAME))))
						if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
							MessageBox(nullptr, TranslateT("You need to supply a file name and path."), _A2W(MODULENAME), MB_OK);
							break;
						}

				if (IsDlgButtonChecked(hwndDlg, IDC_ALWAYS_LOG))
					if (!(GetWindowTextLength(GetDlgItem(hwndDlg, IDC_FILENAME))))
						if (IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ALERTS)) {
							MessageBox(nullptr, TranslateT("You need to supply a file name and path."), _A2W(MODULENAME), MB_OK);
							break;
						}

				wchar_t buf[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILENAME, buf, _countof(buf));
				g_plugin.setWString(hContact, FILE_KEY, buf);

				g_plugin.setByte(hContact, APPEND_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_APPEND));
				g_plugin.setByte(hContact, SAVE_AS_RAW_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SAVE_AS_RAW));

				GetDlgItemText(hwndDlg, IDC_ALERT_STRING, buf, _countof(buf));
				g_plugin.setWString(hContact, ALERT_STRING_KEY, buf);

				GetDlgItemText(hwndDlg, IDC_START2, buf, _countof(buf));
				g_plugin.setWString(hContact, ALRT_S_STRING_KEY, buf);

				GetDlgItemText(hwndDlg, IDC_END2, buf, _countof(buf));
				g_plugin.setWString(hContact, ALRT_E_STRING_KEY, buf);

				g_plugin.setByte(hContact, CONTACT_PREFIX_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_PREFIX));

				EnableWindow(GetDlgItem(hwndDlg, IDC_ALERT_APPLY), 0);

				if (LOWORD(wParam) == IDC_OK2)
					EndDialog(hwndDlg, 1);
			}
			break;

		case IDC_ALERT_CANCEL:
		case IDC_CANCEL:
			EndDialog(hwndDlg, 0);
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcContactOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	wchar_t url[300];
	HWND ParentHwnd = GetParent(hwndDlg);
	static int test;
	static int test2;
	MCONTACT hContact;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		hContact = (MCONTACT)lParam;

		test = 0;
		test2 = 0;

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)hContact);

		SetWindowText(hwndDlg, TranslateT("Contact options"));

		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_OPTIONS)));

		EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 0);

		if (!g_plugin.getWString(hContact, URL_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_URL, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, START_STRING_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_START, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, END_STRING_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_END, dbv.pwszVal);
			db_free(&dbv);
		}
		if (g_plugin.getWString(hContact, PRESERVE_NAME_KEY, &dbv)) {
			db_free(&dbv);
			db_get_ws(hContact, "CList", "MyHandle", &dbv);
			g_plugin.setWString(hContact, PRESERVE_NAME_KEY, dbv.pwszVal);
			db_free(&dbv);
		}
		if (!g_plugin.getWString(hContact, PRESERVE_NAME_KEY, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_SITE_NAME, dbv.pwszVal);
			db_free(&dbv);
		}

		CheckDlgButton(hwndDlg, IDC_CLEAN, g_plugin.getByte(hContact, CLEAR_DISPLAY_KEY, 0) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_SETRANGE, FALSE, MAKELONG(0, 4));
		SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_SETPOS, TRUE, g_plugin.getByte(hContact, RWSPACE_KEY, 0));
		SetDlgItemText(hwndDlg, IDC_RWSPC_TEXT, TranslateW(szTrackerBarDescr[SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_GETPOS, 0, 0)]));

		EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPACE), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPC_TEXT), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));

		if (g_plugin.getByte(hContact, DBLE_WIN_KEY, 1) == 1)
			CheckRadioButton(hwndDlg, IDC_DBLE_WEB, IDC_DBLE_WIN, IDC_DBLE_WIN);
		else
			CheckRadioButton(hwndDlg, IDC_DBLE_WEB, IDC_DBLE_WIN, IDC_DBLE_WEB);

		if (g_plugin.getByte(hContact, U_ALLSITE_KEY, 0) == 1) {
			CheckRadioButton(hwndDlg, IDC_U_SE_STRINGS, IDC_U_ALLSITE, IDC_U_ALLSITE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_START), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_END), 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), 0);
		}
		else {
			CheckRadioButton(hwndDlg, IDC_U_SE_STRINGS, IDC_U_ALLSITE, IDC_U_SE_STRINGS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_START), 1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_END), 1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), 1);
			//EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STR_TXT), 1);
		}
		break;

	case WM_HSCROLL:
		SetDlgItemText(hwndDlg, IDC_RWSPC_TEXT, TranslateW(szTrackerBarDescr[SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_GETPOS, 0, 0)]));
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case WM_CLOSE:
		case IDCANCEL:
			if (hwndDlg) {
				DestroyWindow(hwndDlg);
				hwndDlg = nullptr;
			}
			return TRUE;

		case IDC_SITE_NAME:
		case IDC_URL:
			if (HIWORD(wParam) == EN_CHANGE)
				test++;
			if (test > 2)
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
			break;

		case IDC_DBLE_WEB:
		case IDC_DBLE_WIN:
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
			break;

		case IDC_START:
		case IDC_END:
			if (HIWORD(wParam) == EN_CHANGE)
				test2++;
			if (test2 > 2)
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
			break;

		case IDC_CPY_STRINGS:
			{
				wchar_t string[128];
				hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				GetDlgItemText(hwndDlg, IDC_START, string, _countof(string));
				g_plugin.setWString(hContact, ALRT_S_STRING_KEY, string);

				GetDlgItemText(hwndDlg, IDC_END, string, _countof(string));
				g_plugin.setWString(hContact, ALRT_E_STRING_KEY, string);

				g_plugin.setWord(hContact, EVNT_INDEX_KEY, 2);
			}
			break;

		case IDC_CLEAN:
			EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPACE), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_RWSPC_TEXT), (IsDlgButtonChecked(hwndDlg, IDC_CLEAN)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
			break;

		case IDC_U_SE_STRINGS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_START), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_END), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
			break;

		case IDC_U_ALLSITE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_START), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_END), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CPY_STRINGS), (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 1);
			break;

		case IDC_OPT_APPLY:
		case IDOK:
			{
				wchar_t str[128], contactname[128];
				if (!GetWindowTextLength(GetDlgItem(hwndDlg, IDC_URL))) {
					MessageBox(nullptr, TranslateT("You need to supply a URL."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (!GetWindowTextLength(GetDlgItem(hwndDlg, IDC_START))) {
					if (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)) {
						MessageBox(nullptr, TranslateT("You need to supply a start string."), _A2W(MODULENAME), MB_OK);
						break;
					}
				}
				if (!GetWindowTextLength(GetDlgItem(hwndDlg, IDC_END))) {
					if (IsDlgButtonChecked(hwndDlg, IDC_U_SE_STRINGS)) {
						MessageBox(nullptr, TranslateT("You need to supply an end string."), _A2W(MODULENAME), MB_OK);
						break;
					}
				}
				if (!GetWindowTextLength(GetDlgItem(hwndDlg, IDC_SITE_NAME))) {
					MessageBox(nullptr, TranslateT("You need to supply a name for the contact."), _A2W(MODULENAME), MB_OK);
					break;
				}

				GetDlgItemText(hwndDlg, IDC_SITE_NAME, contactname, _countof(contactname));
				if (wcschr(contactname, '\\') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '/') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, ':') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '*') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '?') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '\"') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '<') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '>') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}
				if (wcschr(contactname, '|') != nullptr) {
					MessageBox(nullptr, TranslateT("Invalid symbol present in contact name."), _A2W(MODULENAME), MB_OK);
					break;
				}

				hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				GetDlgItemText(hwndDlg, IDC_URL, url, _countof(url));
				g_plugin.setWString(hContact, URL_KEY, url);
				g_plugin.setWString(hContact, "Homepage", url);

				GetDlgItemText(hwndDlg, IDC_START, str, _countof(str));
				g_plugin.setWString(hContact, START_STRING_KEY, str);

				GetDlgItemText(hwndDlg, IDC_END, str, _countof(str));
				g_plugin.setWString(hContact, END_STRING_KEY, str);

				GetDlgItemText(hwndDlg, IDC_SITE_NAME, str, _countof(str));
				db_set_ws(hContact, "CList", "MyHandle", str);

				g_plugin.setByte(hContact, DBLE_WIN_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DBLE_WIN));
				g_plugin.setByte(hContact, U_ALLSITE_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_U_ALLSITE));

				g_plugin.setByte(hContact, CLEAR_DISPLAY_KEY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CLEAN));
				g_plugin.setByte(hContact, RWSPACE_KEY, (uint8_t)(SendDlgItemMessage(hwndDlg, IDC_RWSPACE, TBM_GETPOS, 0, 0)));

				SetDlgItemText(ParentHwnd, IDC_OPEN_URL, FixButtonText(url, _countof(url)));

				SetWindowText(ParentHwnd, str);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_APPLY), 0);

				if (LOWORD(wParam) == IDOK)
					EndDialog(hwndDlg, 1);
			}
			break;

		case IDC_OPT_CANCEL:
			EndDialog(hwndDlg, 0);
		}
		break;
	}
	return FALSE;
}
