/*
	Buddy Expectator+ plugin for Miranda-IM (www.miranda-im.org)
	(c)2005 Anar Ibragimoff (ai91@mail.ru)
	(c)2006 Scott Ellis (mail@scottellis.com.au)
	(c)2007,2008 Alexander Turyak (thief@miranda-im.org.ua)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/

#include "common.h"

#define DEF_COLOR_BACK  0xCEF7AD
#define DEF_COLOR_FORE  0x000000

extern HICON hIcon;
extern time_t getLastSeen(MCONTACT);
extern time_t getLastInputMsg(MCONTACT);
extern bool isContactGoneFor(MCONTACT, int);
Options options;

void LoadOptions()
{
	options.iAbsencePeriod = db_get_dw(NULL, MODULE_NAME, "iAbsencePeriod", 14);
	options.iAbsencePeriod2 = db_get_dw(NULL, MODULE_NAME, "iAbsencePeriod2", 30 * 3);
	options.iSilencePeriod = db_get_dw(NULL, MODULE_NAME, "iSilencePeriod", 30);

	options.iShowPopup = db_get_b(NULL, MODULE_NAME, "iShowPopUp", 1);
	options.iShowEvent = db_get_b(NULL, MODULE_NAME, "iShowEvent", 0);
	options.iShowUDetails = db_get_b(NULL, MODULE_NAME, "iShowUDetails", 0);
	options.iShowMessageWindow = db_get_b(NULL, MODULE_NAME, "iShowMessageWindow", 1);

	options.iPopupColorBack = db_get_dw(NULL, MODULE_NAME, "iPopUpColorBack", DEF_COLOR_BACK);
	options.iPopupColorFore = db_get_dw(NULL, MODULE_NAME, "iPopUpColorFore", DEF_COLOR_FORE);

	options.iUsePopupColors = db_get_b(NULL, MODULE_NAME, "iUsePopupColors", 0);
	options.iUseWinColors = db_get_b(NULL, MODULE_NAME, "iUseWinColors", 0);
	options.iPopupDelay = db_get_b(NULL, MODULE_NAME, "iPopUpDelay", 0);

	options.iShowPopup2 = db_get_b(NULL, MODULE_NAME, "iShowPopUp2", 1);
	options.iShowEvent2 = db_get_b(NULL, MODULE_NAME, "iShowEvent2", 0);
	options.action2 = (GoneContactAction)db_get_b(NULL, MODULE_NAME, "Action2", (BYTE)GCA_NOACTION);
	options.notifyFirstOnline = db_get_b(NULL, MODULE_NAME, "bShowFirstSight", 0) ? true : false;
	options.hideInactive = db_get_b(NULL, MODULE_NAME, "bHideInactive", 0) ? true : false;
	options.enableMissYou = db_get_b(NULL, MODULE_NAME, "bMissYouEnabled", 1) ? true : false;
}

void SaveOptions()
{
	db_set_dw(NULL, MODULE_NAME, "iAbsencePeriod", options.iAbsencePeriod);
	db_set_dw(NULL, MODULE_NAME, "iAbsencePeriod2", options.iAbsencePeriod2);
	db_set_dw(NULL, MODULE_NAME, "iSilencePeriod", options.iSilencePeriod);

	db_set_b(NULL, MODULE_NAME, "iShowPopUp", options.iShowPopup);
	db_set_b(NULL, MODULE_NAME, "iShowEvent", options.iShowEvent);
	db_set_b(NULL, MODULE_NAME, "iShowUDetails", options.iShowUDetails);
	db_set_b(NULL, MODULE_NAME, "iShowMessageWindow", options.iShowMessageWindow);

	db_set_b(NULL, MODULE_NAME, "iShowPopUp2", options.iShowPopup2);
	db_set_b(NULL, MODULE_NAME, "iShowEvent2", options.iShowEvent2);
	db_set_b(NULL, MODULE_NAME, "Action2", (BYTE)options.action2);
	db_set_b(NULL, MODULE_NAME, "bShowFirstSight", options.notifyFirstOnline ? 1 : 0);
	db_set_b(NULL, MODULE_NAME, "bHideInactive", options.hideInactive ? 1 : 0);
	db_set_b(NULL, MODULE_NAME, "bMissYouEnabled", options.enableMissYou ? 1 : 0);
}

void SavePopupOptions()
{
	db_set_dw(NULL, MODULE_NAME, "iPopUpColorBack", options.iPopupColorBack);
	db_set_dw(NULL, MODULE_NAME, "iPopUpColorFore", options.iPopupColorFore);

	db_set_b(NULL, MODULE_NAME, "iUsePopupColors", options.iUsePopupColors);
	db_set_b(NULL, MODULE_NAME, "iUseWinColors", options.iUseWinColors);
	db_set_b(NULL, MODULE_NAME, "iPopUpDelay", options.iPopupDelay);
}

/**
 * Options panel function
 */
static INT_PTR CALLBACK OptionsFrameProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		if (!ServiceExists(MS_POPUP_ADDPOPUPT))
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_POPUP), FALSE);

		//iAbsencePeriod
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("days"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("weeks"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("months"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_ADDSTRING, 0, (LPARAM)TranslateT("years"));
		if (options.iAbsencePeriod % 365 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE, options.iAbsencePeriod / 365, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_SETCURSEL, 3, 0);
		}
		else if (options.iAbsencePeriod % 30 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE, options.iAbsencePeriod / 30, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_SETCURSEL, 2, 0);
		}
		else if (options.iAbsencePeriod % 7 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE, options.iAbsencePeriod / 7, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_SETCURSEL, 1, 0);
		}
		else {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE, options.iAbsencePeriod, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_SETCURSEL, 0, 0);
		}

		//iAbsencePeriod2
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_ADDSTRING, 0, (LPARAM)TranslateT("days"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_ADDSTRING, 0, (LPARAM)TranslateT("weeks"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_ADDSTRING, 0, (LPARAM)TranslateT("months"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_ADDSTRING, 0, (LPARAM)TranslateT("years"));

		if (options.iAbsencePeriod2 % 365 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE2, options.iAbsencePeriod2 / 365, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_SETCURSEL, 3, 0);
		}
		else if (options.iAbsencePeriod2 % 30 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE2, options.iAbsencePeriod2 / 30, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_SETCURSEL, 2, 0);
		}
		else if (options.iAbsencePeriod2 % 7 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE2, options.iAbsencePeriod2 / 7, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_SETCURSEL, 1, 0);
		}
		else {
			SetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE2, options.iAbsencePeriod2, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_SETCURSEL, 0, 0);
		}

		//iSilencePeriod
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_ADDSTRING, 0, (LPARAM)TranslateT("days"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_ADDSTRING, 0, (LPARAM)TranslateT("weeks"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_ADDSTRING, 0, (LPARAM)TranslateT("months"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_ADDSTRING, 0, (LPARAM)TranslateT("years"));

		if (options.iSilencePeriod % 365 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_SILENTFOR, options.iSilencePeriod / 365, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_SETCURSEL, 3, 0);
		}
		else if (options.iSilencePeriod % 30 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_SILENTFOR, options.iSilencePeriod / 30, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_SETCURSEL, 2, 0);
		}
		else if (options.iSilencePeriod % 7 == 0) {
			SetDlgItemInt(hwndDlg, IDC_EDIT_SILENTFOR, options.iSilencePeriod / 7, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_SETCURSEL, 1, 0);
		}
		else {
			SetDlgItemInt(hwndDlg, IDC_EDIT_SILENTFOR, options.iSilencePeriod, FALSE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_SETCURSEL, 0, 0);
		}

		CheckDlgButton(hwndDlg, IDC_CHECK_POPUP, options.iShowPopup > 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_FLASHICON, options.iShowEvent > 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_UDETAILS, (options.iShowUDetails > 0 ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHECK_MSGWINDOW, (options.iShowMessageWindow > 0 ? BST_CHECKED : BST_UNCHECKED));
		CheckDlgButton(hwndDlg, IDC_CHECK_FIRSTSIGHT, options.notifyFirstOnline ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_NOMSGS, options.hideInactive ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_MISSYOU, options.enableMissYou ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_CHECK_POPUP2, options.iShowPopup2 > 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_FLASHICON2, options.iShowEvent2 > 0 ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_RESETCONTENT, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Do nothing"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Delete the contact"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Open User Details"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Open message window"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_SETCURSEL, options.action2, 0);
		return TRUE;

	case WM_COMMAND:
		if ((HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == CBN_SELCHANGE)
			|| ((HIWORD(wParam) == EN_CHANGE) && (SendDlgItemMessage(hwndDlg, IDC_EDIT_ABSENCE, EM_GETMODIFY, 0, 0)))
			|| ((HIWORD(wParam) == EN_CHANGE) && (SendDlgItemMessage(hwndDlg, IDC_EDIT_ABSENCE2, EM_GETMODIFY, 0, 0)))
			|| ((HIWORD(wParam) == EN_CHANGE) && (SendDlgItemMessage(hwndDlg, IDC_EDIT_SILENTFOR, EM_GETMODIFY, 0, 0))))
		{
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
	{
		NMHDR* nmhdr = (NMHDR*)lParam;
		switch (nmhdr->code) {
		case PSN_APPLY:
			//iAbsencePeriod
			int num = GetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE, 0, FALSE);
			switch (SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_GETCURSEL, 0, 0)) {
			case 1: options.iAbsencePeriod = 7 * num; break;
			case 2: options.iAbsencePeriod = 30 * num; break;
			case 3: options.iAbsencePeriod = 365 * num; break;
			default: options.iAbsencePeriod = num; break;
			}

			//iAbsencePeriod2
			num = GetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE2, 0, FALSE);
			switch (SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_GETCURSEL, 0, 0)) {
			case 1: options.iAbsencePeriod2 = 7 * num; break;
			case 2: options.iAbsencePeriod2 = 30 * num; break;
			case 3: options.iAbsencePeriod2 = 365 * num; break;
			default: options.iAbsencePeriod2 = num; break;
			}

			//iSilencePeriod
			num = GetDlgItemInt(hwndDlg, IDC_EDIT_SILENTFOR, 0, FALSE);
			switch (SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD3, CB_GETCURSEL, 0, 0)) {
			case 1: options.iSilencePeriod = 7 * num; break;
			case 2: options.iSilencePeriod = 30 * num; break;
			case 3: options.iSilencePeriod = 365 * num; break;
			default: options.iSilencePeriod = num; break;
			}

			options.iShowPopup = IsDlgButtonChecked(hwndDlg, IDC_CHECK_POPUP) == BST_CHECKED ? 1 : 0;
			options.iShowEvent = IsDlgButtonChecked(hwndDlg, IDC_CHECK_FLASHICON) == BST_CHECKED ? 1 : 0;
			options.iShowUDetails = IsDlgButtonChecked(hwndDlg, IDC_CHECK_UDETAILS) == BST_CHECKED ? 1 : 0;
			options.iShowMessageWindow = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MSGWINDOW) == BST_CHECKED ? 1 : 0;
			options.notifyFirstOnline = IsDlgButtonChecked(hwndDlg, IDC_CHECK_FIRSTSIGHT) == BST_CHECKED ? true : false;
			options.hideInactive = IsDlgButtonChecked(hwndDlg, IDC_CHECK_NOMSGS) == BST_CHECKED ? true : false;
			options.enableMissYou = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MISSYOU) == BST_CHECKED ? true : false;

			options.iShowPopup2 = IsDlgButtonChecked(hwndDlg, IDC_CHECK_POPUP2) == BST_CHECKED ? 1 : 0;
			options.iShowEvent2 = IsDlgButtonChecked(hwndDlg, IDC_CHECK_FLASHICON2) == BST_CHECKED ? 1 : 0;

			options.action2 = (GoneContactAction)SendDlgItemMessage(hwndDlg, IDC_COMBO_ACTIONS, CB_GETCURSEL, 0, 0);

			// save values to the DB
			SaveOptions();

			// clear all notified settings
			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
				if (db_get_b(hContact, MODULE_NAME, "StillAbsentNotified", 0))
					db_set_b(hContact, MODULE_NAME, "StillAbsentNotified", 0);

			// restart timer & run check
			KillTimer(0, timer_id);
			timer_id = SetTimer(0, 0, 1000 * 60 * 60 * 4, TimerProc); // check every 4 hours
			TimerProc(0, 0, 0, 0);
			return TRUE;
		}
		break;
	}
	}
	return 0;
}

/**
 * Popup Options panel function
 */
static INT_PTR CALLBACK PopupOptionsFrameProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int ChangeLock = 0;
	switch (uMsg) {
	case WM_INITDIALOG:
		ChangeLock++;
		TranslateDialogDefault(hwndDlg);

		//iPopupColorBack
		SendDlgItemMessage(hwndDlg, IDC_COLOR_BGR, CPM_SETCOLOUR, 0, options.iPopupColorBack);

		//iPopupColorFore
		SendDlgItemMessage(hwndDlg, IDC_COLOR_FRG, CPM_SETCOLOUR, 0, options.iPopupColorFore);

		if (options.iUsePopupColors) {
			CheckDlgButton(hwndDlg, IDC_COLORS_POPUP, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BGR), false);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_FRG), false);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLORS_WIN), false);
		}

		if (options.iUseWinColors) {
			CheckDlgButton(hwndDlg, IDC_COLORS_WIN, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BGR), false);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_FRG), false);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COLORS_POPUP), false);
		}

		//iPopupDelay
		SetDlgItemInt(hwndDlg, IDC_EDIT_POPUPDELAY, 5, FALSE);
		if (options.iPopupDelay < 0) {
			CheckDlgButton(hwndDlg, IDC_DELAY_PERM, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_POPUPDELAY), false);
		}
		else if (options.iPopupDelay == 0) {
			CheckDlgButton(hwndDlg, IDC_DELAY_DEF, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_POPUPDELAY), false);
		}
		else {
			CheckDlgButton(hwndDlg, IDC_DELAY_CUST, BST_CHECKED);
			SetDlgItemInt(hwndDlg, IDC_EDIT_POPUPDELAY, options.iPopupDelay, FALSE);
		}

		ChangeLock--;
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_PREVIEW) {
			POPUPDATAT ppd;
			memset(&ppd, 0, sizeof(ppd));

			//iPopupDelay
			options.iPopupDelay = GetDlgItemInt(hwndDlg, IDC_EDIT_POPUPDELAY, 0, FALSE);
			if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERM) == BST_CHECKED)
				options.iPopupDelay = -1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_DEF) == BST_CHECKED)
				options.iPopupDelay = 0;

			ppd.lchContact = NULL;
			ppd.lchIcon = hIcon;
			_tcsncpy(ppd.lptzContactName, TranslateT("Contact name"), MAX_CONTACTNAME);
			TCHAR szPreviewText[250];
			mir_sntprintf(szPreviewText, SIZEOF(szPreviewText), TranslateT("has returned after being absent since %d days"), rand() % 30);
			_tcsncpy(ppd.lptzText, szPreviewText, MAX_SECONDLINE);

			// Get current popups colors options
			if (IsDlgButtonChecked(hwndDlg, IDC_COLORS_POPUP))
				ppd.colorBack = ppd.colorText = 0;
			else if (IsDlgButtonChecked(hwndDlg, IDC_COLORS_WIN)) {
				ppd.colorBack = GetSysColor(COLOR_BTNFACE);
				ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			}
			else {
				ppd.colorBack = SendDlgItemMessage(hwndDlg, IDC_COLOR_BGR, CPM_GETCOLOUR, 0, 0);
				ppd.colorText = SendDlgItemMessage(hwndDlg, IDC_COLOR_FRG, CPM_GETCOLOUR, 0, 0);
			}
			ppd.PluginData = NULL;
			ppd.iSeconds = options.iPopupDelay;

			CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, APF_NO_HISTORY);

			_tcsncpy(ppd.lptzText, TranslateT("You awaited this contact!"), MAX_SECONDLINE);
			ppd.lchIcon = Skin_GetIcon("enabled_icon");

			CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, APF_NO_HISTORY);
		}
		else {
			if ((HIWORD(wParam) == BN_CLICKED) || (HIWORD(wParam) == CBN_SELCHANGE) || ((HIWORD(wParam) == EN_CHANGE) && !ChangeLock))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			if (LOWORD(wParam) == IDC_COLORS_POPUP) {
				if (IsDlgButtonChecked(hwndDlg, IDC_COLORS_POPUP)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BGR), false);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_FRG), false);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLORS_WIN), false);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BGR), true);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_FRG), true);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLORS_WIN), true);
				}
			}
			if (LOWORD(wParam) == IDC_COLORS_WIN) {
				if (IsDlgButtonChecked(hwndDlg, IDC_COLORS_WIN)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BGR), false);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_FRG), false);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLORS_POPUP), false);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_BGR), true);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLOR_FRG), true);
					EnableWindow(GetDlgItem(hwndDlg, IDC_COLORS_POPUP), true);
				}
			}

			if (LOWORD(wParam) == IDC_DELAY_DEF || LOWORD(wParam) == IDC_DELAY_PERM)
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_POPUPDELAY), false);
			else if (LOWORD(wParam) == IDC_DELAY_CUST)
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_POPUPDELAY), true);
		}

		break;
	case WM_NOTIFY:
	{
		NMHDR* nmhdr = (NMHDR*)lParam;
		switch (nmhdr->code) {
		case PSN_APPLY:
			if (IsDlgButtonChecked(hwndDlg, IDC_COLORS_POPUP)) {
				options.iUsePopupColors = 1;
				options.iUseWinColors = 0;
			}
			else if (IsDlgButtonChecked(hwndDlg, IDC_COLORS_WIN)) {
				options.iUseWinColors = 1;
				options.iUsePopupColors = 0;
				options.iPopupColorBack = GetSysColor(COLOR_BTNFACE);
				options.iPopupColorFore = GetSysColor(COLOR_WINDOWTEXT);
			}
			else {
				options.iUseWinColors = options.iUsePopupColors = 0;
				options.iPopupColorBack = SendDlgItemMessage(hwndDlg, IDC_COLOR_BGR, CPM_GETCOLOUR, 0, 0);
				options.iPopupColorFore = SendDlgItemMessage(hwndDlg, IDC_COLOR_FRG, CPM_GETCOLOUR, 0, 0);
			}

			//iPopupDelay
			options.iPopupDelay = GetDlgItemInt(hwndDlg, IDC_EDIT_POPUPDELAY, 0, FALSE);
			if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERM) == BST_CHECKED)
				options.iPopupDelay = -1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_DEF) == BST_CHECKED)
				options.iPopupDelay = 0;

			// save value to the DB
			SavePopupOptions();

			return TRUE;
		}
		break;
	}
	}
	return 0;
}

/**
 * Init options panel
 */
static int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.ptszGroup = LPGENT("Contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPANEL);
	odp.ptszTitle = LPGENT("Buddy Expectator");
	odp.pfnDlgProc = OptionsFrameProc;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.ptszGroup = LPGENT("Popups");
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPPANEL);
		odp.pfnDlgProc = PopupOptionsFrameProc;
		Options_AddPage(wParam, &odp);
	}

	return 0;
}

INT_PTR CALLBACK UserinfoDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		time_t tmpTime;
		TCHAR tmpBuf[51] = { 0 };
		tmpTime = getLastSeen((MCONTACT)lparam);
		if (tmpTime == -1)
			SetDlgItemText(hdlg, IDC_EDIT_LASTSEEN, TranslateT("not detected"));
		else {
			_tcsftime(tmpBuf, 50, _T("%#x"), gmtime(&tmpTime));
			SetDlgItemText(hdlg, IDC_EDIT_LASTSEEN, tmpBuf);
		}

		tmpTime = getLastInputMsg((MCONTACT)lparam);
		if (tmpTime == -1)
			SetDlgItemText(hdlg, IDC_EDIT_LASTINPUT, TranslateT("not found"));
		else {
			_tcsftime(tmpBuf, 50, _T("%#x"), gmtime(&tmpTime));
			SetDlgItemText(hdlg, IDC_EDIT_LASTINPUT, tmpBuf);
		}

		unsigned int AbsencePeriod = db_get_dw((MCONTACT)lparam, MODULE_NAME, "iAbsencePeriod", options.iAbsencePeriod);

		SendDlgItemMessage(hdlg, IDC_SPINABSENCE, UDM_SETRANGE, 0, MAKELONG(999, 1));
		SetDlgItemInt(hdlg, IDC_EDITABSENCE, AbsencePeriod, FALSE);

		if (isContactGoneFor((MCONTACT)lparam, options.iAbsencePeriod2))
			SetDlgItemText(hdlg, IDC_EDIT_WILLNOTICE, TranslateT("This contact has been absent for an extended period of time."));
		else
			SetDlgItemText(hdlg, IDC_EDIT_WILLNOTICE, _T(""));

		CheckDlgButton(hdlg, IDC_CHECK_MISSYOU, db_get_b((MCONTACT)lparam, MODULE_NAME, "MissYou", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_NOTIFYALWAYS, db_get_b((MCONTACT)lparam, MODULE_NAME, "MissYouNotifyAlways", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_CHECK_NEVERHIDE, db_get_b((MCONTACT)lparam, MODULE_NAME, "NeverHide", 0) ? BST_CHECKED : BST_UNCHECKED);

		TranslateDialogDefault(hdlg);
		return TRUE;
	}

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->idFrom)
		{
		case 0:
			switch (((LPNMHDR)lparam)->code)
			{
			case (PSN_APPLY) :
			{
				MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lparam)->lParam;
				if (hContact)
				{
					db_set_dw(hContact, MODULE_NAME, "iAbsencePeriod", GetDlgItemInt(hdlg, IDC_EDITABSENCE, 0, FALSE));
					db_set_b(hContact, MODULE_NAME, "MissYou", (IsDlgButtonChecked(hdlg, IDC_CHECK_MISSYOU) == BST_CHECKED) ? 1 : 0);
					db_set_b(hContact, MODULE_NAME, "MissYouNotifyAlways", (IsDlgButtonChecked(hdlg, IDC_CHECK_NOTIFYALWAYS) == BST_CHECKED) ? 1 : 0);
					db_set_b(hContact, MODULE_NAME, "NeverHide", (IsDlgButtonChecked(hdlg, IDC_CHECK_NEVERHIDE) == BST_CHECKED) ? 1 : 0);
				}
				break;
			}
			}
			break;
		}
		break;

	case WM_COMMAND:
		if (wparam == MAKEWPARAM(IDC_EDITABSENCE, EN_CHANGE))
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		else if (LOWORD(wparam) == IDCANCEL)
			SendMessage(GetParent(hdlg), msg, wparam, lparam);
		break;
	}

	return FALSE;
}

int UserinfoInit(WPARAM wparam, LPARAM lparam)
{
	if (lparam > 0) {
		OPTIONSDIALOGPAGE uip = { sizeof(uip) };
		uip.hInstance = hInst;
		uip.pszTemplate = MAKEINTRESOURCEA(IDD_USERINFO);
		uip.flags = ODPF_TCHAR;
		uip.ptszTitle = LPGENT("Buddy Expectator");
		uip.pfnDlgProc = UserinfoDlgProc;
		UserInfo_AddPage(wparam, &uip);
	}
	return 0;
}


void InitOptions()
{
	LoadOptions();
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
}
