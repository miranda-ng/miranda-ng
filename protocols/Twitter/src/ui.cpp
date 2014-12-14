/*
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "ui.h"

#include "proto.h"
#include "twitter.h"

static const TCHAR *sites[] = {
	_T("https://api.twitter.com/"),
	_T("https://identi.ca/api/")
};

INT_PTR CALLBACK first_run_dialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TwitterProto *proto;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = reinterpret_cast<TwitterProto*>(lParam);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		DBVARIANT dbv;
		if (!db_get_ts(0, proto->ModuleName(), TWITTER_KEY_GROUP, &dbv)) {
			SetDlgItemText(hwndDlg, IDC_GROUP, dbv.ptszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hwndDlg, IDC_GROUP, L"Twitter");

		if (!db_get_s(0, proto->ModuleName(), TWITTER_KEY_UN, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_USERNAME, dbv.pszVal);
			db_free(&dbv);
		}

		for (size_t i = 0; i < SIZEOF(sites); i++)
			SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(sites[i]));

		if (!db_get_s(0, proto->ModuleName(), TWITTER_KEY_BASEURL, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_SERVER, dbv.pszVal);
			db_free(&dbv);
		}
		else SendDlgItemMessage(hwndDlg, IDC_SERVER, CB_SETCURSEL, 0, 0);
		return true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_NEWACCOUNTLINK) {
			CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, reinterpret_cast<LPARAM>("https://twitter.com/signup"));
			return true;
		}

		if (GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) { // Window is done initializing
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
			case CBN_EDITCHANGE:
			case CBN_SELCHANGE:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY: // might be able to get rid of this bit?
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY) {
			proto = reinterpret_cast<TwitterProto*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			char str[128];
			TCHAR tstr[128];

			GetDlgItemTextA(hwndDlg, IDC_SERVER, str, SIZEOF(str) - 1);
			if (str[strlen(str) - 1] != '/')
				strncat(str, "/", sizeof(str));
			db_set_s(0, proto->ModuleName(), TWITTER_KEY_BASEURL, str);

			GetDlgItemText(hwndDlg, IDC_GROUP, tstr, SIZEOF(tstr));
			db_set_ts(0, proto->ModuleName(), TWITTER_KEY_GROUP, tstr);

			return true;
		}
		break;
	}

	return false;
}

INT_PTR CALLBACK tweet_proc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TwitterProto *proto;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = reinterpret_cast<TwitterProto*>(lParam);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwndDlg, IDC_TWEETMSG, EM_LIMITTEXT, 140, 0);
		SetDlgItemText(hwndDlg, IDC_CHARACTERS, _T("140"));

		// Set window title
		TCHAR title[512];
		mir_sntprintf(title, SIZEOF(title), _T("Send Tweet for %s"), proto->m_tszUserName);
		SendMessage(hwndDlg, WM_SETTEXT, 0, (LPARAM)title);
		return true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			TCHAR msg[141];
			proto = reinterpret_cast<TwitterProto*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

			GetDlgItemText(hwndDlg, IDC_TWEETMSG, msg, SIZEOF(msg));
			ShowWindow(hwndDlg, SW_HIDE);

			char *narrow = mir_t2a_cp(msg, CP_UTF8);
			proto->ForkThread(&TwitterProto::SendTweetWorker, narrow);

			EndDialog(hwndDlg, wParam);
			return true;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwndDlg, wParam);
			return true;
		}
		else if (LOWORD(wParam) == IDC_TWEETMSG && HIWORD(wParam) == EN_CHANGE) {
			size_t len = SendDlgItemMessage(hwndDlg, IDC_TWEETMSG, WM_GETTEXTLENGTH, 0, 0);
			char str[16];
			mir_snprintf(str, SIZEOF(str), "%d", 140 - len);
			SetDlgItemTextA(hwndDlg, IDC_CHARACTERS, str);

			return true;
		}

		break;
	case WM_SETREPLY:
		char foo[512];
		mir_snprintf(foo, SIZEOF(foo), "@%s ", (char*)wParam);
		size_t len = strlen(foo);

		SetDlgItemTextA(hwndDlg, IDC_TWEETMSG, foo);
		SendDlgItemMessage(hwndDlg, IDC_TWEETMSG, EM_SETSEL, len, len);

		char str[16];
		mir_snprintf(str, SIZEOF(str), "%d", 140 - len);
		SetDlgItemTextA(hwndDlg, IDC_CHARACTERS, str);

		return true;
	}

	return false;
}

INT_PTR CALLBACK options_proc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TwitterProto *proto;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = reinterpret_cast<TwitterProto*>(lParam);

		DBVARIANT dbv;
		if (!db_get_s(0, proto->ModuleName(), TWITTER_KEY_UN, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_UN, dbv.pszVal);
			db_free(&dbv);
		}

		CheckDlgButton(hwndDlg, IDC_CHATFEED, db_get_b(0, proto->ModuleName(), TWITTER_KEY_CHATFEED, 0) ? BST_CHECKED : BST_UNCHECKED);

		for (size_t i = 0; i < SIZEOF(sites); i++)
			SendDlgItemMessage(hwndDlg, IDC_BASEURL, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(sites[i]));

		if (!db_get_s(0, proto->ModuleName(), TWITTER_KEY_BASEURL, &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_BASEURL, dbv.pszVal);
			db_free(&dbv);
		}
		else SendDlgItemMessage(hwndDlg, IDC_BASEURL, CB_SETCURSEL, 0, 0);

		char pollrate_str[32];
		mir_snprintf(pollrate_str, SIZEOF(pollrate_str), "%d", db_get_dw(0, proto->ModuleName(), TWITTER_KEY_POLLRATE, 80));
		SetDlgItemTextA(hwndDlg, IDC_POLLRATE, pollrate_str);

		CheckDlgButton(hwndDlg, IDC_TWEET_MSG, db_get_b(0, proto->ModuleName(), TWITTER_KEY_TWEET_TO_MSG, 0) ? BST_CHECKED : BST_UNCHECKED);


		// Do this last so that any events propagated by pre-filling the form don't
		// instigate a PSM_CHANGED message
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		break;
	case WM_COMMAND:
		if (GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) // Window is done initializing
		{
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
			case BN_CLICKED:
			case CBN_EDITCHANGE:
			case CBN_SELCHANGE:
				switch (LOWORD(wParam)) {
				case IDC_UN:
				case IDC_PW:
				case IDC_BASEURL:
					ShowWindow(GetDlgItem(hwndDlg, IDC_RECONNECT), SW_SHOW);
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}

		break;
	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY) {
			proto = reinterpret_cast<TwitterProto*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			char str[128];

			GetDlgItemTextA(hwndDlg, IDC_UN, str, SIZEOF(str));
			db_set_s(0, proto->ModuleName(), TWITTER_KEY_UN, str);

			/*GetDlgItemTextA(hwndDlg,IDC_PW,str,SIZEOF(str));
			CallService(MS_DB_CRYPT_ENCODESTRING,sizeof(str),reinterpret_cast<LPARAM>(str));
			db_set_s(0,proto->ModuleName(),TWITTER_KEY_PASS,str);*/

			GetDlgItemTextA(hwndDlg, IDC_BASEURL, str, SIZEOF(str) - 1);
			if (str[strlen(str) - 1] != '/')
				strncat(str, "/", sizeof(str));
			db_set_s(0, proto->ModuleName(), TWITTER_KEY_BASEURL, str);

			db_set_b(0, proto->ModuleName(), TWITTER_KEY_CHATFEED, IsDlgButtonChecked(hwndDlg, IDC_CHATFEED));

			GetDlgItemTextA(hwndDlg, IDC_POLLRATE, str, SIZEOF(str));
			int rate = atoi(str);
			if (rate == 0)
				rate = 80;
			db_set_dw(0, proto->ModuleName(), TWITTER_KEY_POLLRATE, rate);

			db_set_b(0, proto->ModuleName(), TWITTER_KEY_TWEET_TO_MSG, IsDlgButtonChecked(hwndDlg, IDC_TWEET_MSG));

			proto->UpdateSettings();
			return true;
		}
	}

	return false;
}

namespace popup_options
{
	static int get_timeout(HWND hwndDlg)
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_TIMEOUT_PERMANENT))
			return -1;
		else if (IsDlgButtonChecked(hwndDlg, IDC_TIMEOUT_CUSTOM)) {
			char str[32];
			GetDlgItemTextA(hwndDlg, IDC_TIMEOUT, str, SIZEOF(str));
			return atoi(str);
		}
		else // Default checked (probably)
			return 0;
	}

	static COLORREF get_text_color(HWND hwndDlg, bool for_db)
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_COL_WINDOWS)) {
			if (for_db)
				return -1;
			else
				return GetSysColor(COLOR_WINDOWTEXT);
		}
		else if (IsDlgButtonChecked(hwndDlg, IDC_COL_CUSTOM))
			return (COLORREF)SendDlgItemMessage(hwndDlg, IDC_COLTEXT, CPM_GETCOLOUR, 0, 0);
		else // Default checked (probably)
			return 0;
	}

	static COLORREF get_back_color(HWND hwndDlg, bool for_db)
	{
		if (IsDlgButtonChecked(hwndDlg, IDC_COL_WINDOWS)) {
			if (for_db)
				return -1;
			else
				return GetSysColor(COLOR_WINDOW);
		}
		else if (IsDlgButtonChecked(hwndDlg, IDC_COL_CUSTOM))
			return (COLORREF)SendDlgItemMessage(hwndDlg, IDC_COLBACK, CPM_GETCOLOUR, 0, 0);
		else // Default checked (probably)
			return 0;
	}

	struct
	{
		TCHAR *name;
		TCHAR *text;
	} const quotes[] = {
	{
		_T("Dorothy Parker"), _T("If, with the literate, I am\n")
			_T("Impelled to try an epigram,\n")
			_T("I never seek to take the credit;\n")
			_T("We all assume that Oscar said it.") },
		{ _T("Steve Ballmer"), _T("I have never, honestly, thrown a chair in my life.") },
		{ _T("James Joyce"), _T("I think I would know Nora's fart anywhere. I think ")
			_T("I could pick hers out in a roomful of farting women.") },
		{ _T("Brooke Shields"), _T("Smoking kills. If you're killed, you've lost a very ")
			_T("important part of your life.") },
		{ _T("Yogi Berra"), _T("Always go to other peoples' funerals, otherwise ")
			_T("they won't go to yours.") },
	};

	static void preview(HWND hwndDlg)
	{
		POPUPDATAT popup = {};

		// Pick a random contact
		MCONTACT hContact = 0;
		int n_contacts = (int)CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);

		if (n_contacts != 0) {
			int contact = rand() % n_contacts;
			hContact = db_find_first();
			for (int i = 0; i < contact; i++)
				hContact = db_find_next(hContact);
		}

		// Pick a random quote
		int q = rand() % SIZEOF(quotes);
		_tcsncpy(popup.lptzContactName, quotes[q].name, MAX_CONTACTNAME);
		_tcsncpy(popup.lptzText, quotes[q].text, MAX_SECONDLINE);

		popup.lchContact = hContact;
		popup.iSeconds = get_timeout(hwndDlg);
		popup.colorText = get_text_color(hwndDlg, false);
		popup.colorBack = get_back_color(hwndDlg, false);

		PUAddPopupT(&popup);
	}
}

void CheckAndUpdateDlgButton(HWND hWnd, int button, BOOL check)
{
	CheckDlgButton(hWnd, button, check ? BST_CHECKED : BST_UNCHECKED);
	SendMessage(hWnd, WM_COMMAND, MAKELONG(button, BN_CLICKED),
		(LPARAM)GetDlgItem(hWnd, button));
}

INT_PTR CALLBACK popup_options_proc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	using namespace popup_options;
	TwitterProto *proto;

	int text_color, back_color, timeout;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = reinterpret_cast<TwitterProto*>(lParam);

		CheckAndUpdateDlgButton(hwndDlg, IDC_SHOWPOPUPS, db_get_b(0, proto->ModuleName(), TWITTER_KEY_POPUP_SHOW, 0));
		CheckDlgButton(hwndDlg, IDC_NOSIGNONPOPUPS, !db_get_b(0, proto->ModuleName(), TWITTER_KEY_POPUP_SIGNON, 0) ? BST_CHECKED : BST_UNCHECKED);

		// ***** Get color information
		back_color = db_get_dw(0, proto->ModuleName(), TWITTER_KEY_POPUP_COLBACK, 0);
		text_color = db_get_dw(0, proto->ModuleName(), TWITTER_KEY_POPUP_COLTEXT, 0);

		SendDlgItemMessage(hwndDlg, IDC_COLBACK, CPM_SETCOLOUR, 0, RGB(255, 255, 255));
		SendDlgItemMessage(hwndDlg, IDC_COLTEXT, CPM_SETCOLOUR, 0, RGB(0, 0, 0));

		if (back_color == -1 && text_color == -1)            // Windows defaults
			CheckAndUpdateDlgButton(hwndDlg, IDC_COL_WINDOWS, true);
		else if (back_color == 0 && text_color == 0)         // Popup defaults
			CheckAndUpdateDlgButton(hwndDlg, IDC_COL_POPUP, true);
		else {                                               // Custom colors
			CheckAndUpdateDlgButton(hwndDlg, IDC_COL_CUSTOM, true);
			SendDlgItemMessage(hwndDlg, IDC_COLBACK, CPM_SETCOLOUR, 0, back_color);
			SendDlgItemMessage(hwndDlg, IDC_COLTEXT, CPM_SETCOLOUR, 0, text_color);
		}

		// ***** Get timeout information
		timeout = db_get_dw(0, proto->ModuleName(), TWITTER_KEY_POPUP_TIMEOUT, 0);
		SetDlgItemTextA(hwndDlg, IDC_TIMEOUT, "5");

		if (timeout == 0)
			CheckAndUpdateDlgButton(hwndDlg, IDC_TIMEOUT_DEFAULT, true);
		else if (timeout < 0)
			CheckAndUpdateDlgButton(hwndDlg, IDC_TIMEOUT_PERMANENT, true);
		else {
			char str[32];
			mir_snprintf(str, SIZEOF(str), "%d", timeout);
			SetDlgItemTextA(hwndDlg, IDC_TIMEOUT, str);
			CheckAndUpdateDlgButton(hwndDlg, IDC_TIMEOUT_CUSTOM, true);
		}

		SendDlgItemMessage(hwndDlg, IDC_TIMEOUT_SPIN, UDM_SETRANGE32, 1, INT_MAX);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		return true;
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_SHOWPOPUPS:
				EnableWindow(GetDlgItem(hwndDlg, IDC_NOSIGNONPOPUPS), IsDlgButtonChecked(hwndDlg, IDC_SHOWPOPUPS));
				break;

			case IDC_COL_CUSTOM:
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLBACK), true);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLTEXT), true);
				break;
			case IDC_COL_WINDOWS:
			case IDC_COL_POPUP:
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLBACK), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLTEXT), false);
				break;

			case IDC_TIMEOUT_CUSTOM:
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEOUT), true);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEOUT_SPIN), true);
				break;
			case IDC_TIMEOUT_DEFAULT:
			case IDC_TIMEOUT_PERMANENT:
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEOUT), false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TIMEOUT_SPIN), false);
				break;

			case IDC_PREVIEW:
				preview(hwndDlg);
				break;
			}

		case EN_CHANGE:
			if (GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) // Window is done initializing
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY) {
			proto = reinterpret_cast<TwitterProto*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

			db_set_b(0, proto->ModuleName(), TWITTER_KEY_POPUP_SHOW, IsDlgButtonChecked(hwndDlg, IDC_SHOWPOPUPS));
			db_set_b(0, proto->ModuleName(), TWITTER_KEY_POPUP_SIGNON, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_NOSIGNONPOPUPS));

			// ***** Write color settings
			db_set_dw(0, proto->ModuleName(), TWITTER_KEY_POPUP_COLBACK, get_back_color(hwndDlg, true));
			db_set_dw(0, proto->ModuleName(), TWITTER_KEY_POPUP_COLTEXT, get_text_color(hwndDlg, true));

			// ***** Write timeout setting
			db_set_dw(0, proto->ModuleName(), TWITTER_KEY_POPUP_TIMEOUT, get_timeout(hwndDlg));

			return true;
		}
		break;
	}

	return false;
}

INT_PTR CALLBACK pin_proc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TwitterProto *proto;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		return true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			proto = reinterpret_cast<TwitterProto*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			char str[128];

			GetDlgItemTextA(hwndDlg, IDC_PIN, str, SIZEOF(str));

			db_set_s(0, proto->ModuleName(), TWITTER_KEY_OAUTH_PIN, str);
			EndDialog(hwndDlg, wParam);
			return true;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwndDlg, wParam);
			return true;
		}
		break;
	}

	return false;
}
