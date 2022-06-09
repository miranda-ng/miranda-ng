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

#include "stdafx.h"

#define DEF_COLOR_BACK  0xCEF7AD
#define DEF_COLOR_FORE  0x000000

extern HICON hIcon;
extern time_t getLastSeen(MCONTACT);
extern time_t getLastInputMsg(MCONTACT);
extern bool isContactGoneFor(MCONTACT, int);
Options options;

void LoadOptions()
{
	options.iAbsencePeriod = g_plugin.getDword("iAbsencePeriod", 14);
	options.iAbsencePeriod2 = g_plugin.getDword("iAbsencePeriod2", 30 * 3);
	options.iSilencePeriod = g_plugin.getDword("iSilencePeriod", 30);

	options.iShowPopup = g_plugin.getByte("iShowPopUp", 1);
	options.iShowEvent = g_plugin.getByte("iShowEvent", 0);
	options.iShowUDetails = g_plugin.getByte("iShowUDetails", 0);
	options.iShowMessageWindow = g_plugin.getByte("iShowMessageWindow", 1);

	options.iPopupColorBack = g_plugin.getDword("iPopUpColorBack", DEF_COLOR_BACK);
	options.iPopupColorFore = g_plugin.getDword("iPopUpColorFore", DEF_COLOR_FORE);

	options.iUsePopupColors = g_plugin.getByte("iUsePopupColors", 0);
	options.iUseWinColors = g_plugin.getByte("iUseWinColors", 0);
	options.iPopupDelay = g_plugin.getByte("iPopUpDelay", 0);

	options.iShowPopup2 = g_plugin.getByte("iShowPopUp2", 1);
	options.iShowEvent2 = g_plugin.getByte("iShowEvent2", 0);
	options.action2 = (GoneContactAction)g_plugin.getByte("Action2", (uint8_t)GCA_NOACTION);
	options.notifyFirstOnline = g_plugin.getByte("bShowFirstSight", 0) ? true : false;
	options.hideInactive = g_plugin.getByte("bHideInactive", 0) ? true : false;
	options.enableMissYou = g_plugin.getByte("bMissYouEnabled", 1) ? true : false;
}

void SaveOptions()
{
	g_plugin.setDword("iAbsencePeriod", options.iAbsencePeriod);
	g_plugin.setDword("iAbsencePeriod2", options.iAbsencePeriod2);
	g_plugin.setDword("iSilencePeriod", options.iSilencePeriod);

	g_plugin.setByte("iShowPopUp", options.iShowPopup);
	g_plugin.setByte("iShowEvent", options.iShowEvent);
	g_plugin.setByte("iShowUDetails", options.iShowUDetails);
	g_plugin.setByte("iShowMessageWindow", options.iShowMessageWindow);

	g_plugin.setByte("iShowPopUp2", options.iShowPopup2);
	g_plugin.setByte("iShowEvent2", options.iShowEvent2);
	g_plugin.setByte("Action2", (uint8_t)options.action2);
	g_plugin.setByte("bShowFirstSight", options.notifyFirstOnline ? 1 : 0);
	g_plugin.setByte("bHideInactive", options.hideInactive ? 1 : 0);
	g_plugin.setByte("bMissYouEnabled", options.enableMissYou ? 1 : 0);
}

void SavePopupOptions()
{
	g_plugin.setDword("iPopUpColorBack", options.iPopupColorBack);
	g_plugin.setDword("iPopUpColorFore", options.iPopupColorFore);

	g_plugin.setByte("iUsePopupColors", options.iUsePopupColors);
	g_plugin.setByte("iUseWinColors", options.iUseWinColors);
	g_plugin.setByte("iPopUpDelay", options.iPopupDelay);
}

/**
 * Options panel function
 */
static INT_PTR CALLBACK OptionsFrameProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		// iAbsencePeriod
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
		switch (((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			//iAbsencePeriod
			int num = GetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE, nullptr, FALSE);
			switch (SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD, CB_GETCURSEL, 0, 0)) {
			case 1: options.iAbsencePeriod = 7 * num; break;
			case 2: options.iAbsencePeriod = 30 * num; break;
			case 3: options.iAbsencePeriod = 365 * num; break;
			default: options.iAbsencePeriod = num; break;
			}

			//iAbsencePeriod2
			num = GetDlgItemInt(hwndDlg, IDC_EDIT_ABSENCE2, nullptr, FALSE);
			switch (SendDlgItemMessage(hwndDlg, IDC_COMBO_PERIOD2, CB_GETCURSEL, 0, 0)) {
			case 1: options.iAbsencePeriod2 = 7 * num; break;
			case 2: options.iAbsencePeriod2 = 30 * num; break;
			case 3: options.iAbsencePeriod2 = 365 * num; break;
			default: options.iAbsencePeriod2 = num; break;
			}

			//iSilencePeriod
			num = GetDlgItemInt(hwndDlg, IDC_EDIT_SILENTFOR, nullptr, FALSE);
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
			for (auto &hContact : Contacts())
				if (g_plugin.getByte(hContact, "StillAbsentNotified", 0))
					g_plugin.setByte(hContact, "StillAbsentNotified", 0);

			// restart timer & run check
			KillTimer(nullptr, timer_id);
			timer_id = SetTimer(nullptr, 0, 1000 * 60 * 60 * 4, TimerProc); // check every 4 hours
			TimerProc(nullptr, 0, 0, 0);
			return TRUE;
		}
		break;
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
			POPUPDATAW ppd;

			//iPopupDelay
			options.iPopupDelay = GetDlgItemInt(hwndDlg, IDC_EDIT_POPUPDELAY, nullptr, FALSE);
			if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_PERM) == BST_CHECKED)
				options.iPopupDelay = -1;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAY_DEF) == BST_CHECKED)
				options.iPopupDelay = 0;

			ppd.lchContact = NULL;
			ppd.lchIcon = hIcon;
			wcsncpy(ppd.lpwzContactName, TranslateT("Contact name"), MAX_CONTACTNAME);
			wchar_t szPreviewText[250];
			mir_snwprintf(szPreviewText, TranslateT("has returned after being absent since %d days"), rand() % 30);
			wcsncpy(ppd.lpwzText, szPreviewText, MAX_SECONDLINE);

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
			ppd.PluginData = nullptr;
			ppd.iSeconds = options.iPopupDelay;
			PUAddPopupW(&ppd, APF_NO_HISTORY);

			wcsncpy(ppd.lpwzText, TranslateT("You awaited this contact!"), MAX_SECONDLINE);
			ppd.lchIcon = IcoLib_GetIcon("enabled_icon");
			PUAddPopupW(&ppd, APF_NO_HISTORY);
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
		NMHDR *nmhdr = (NMHDR *)lParam;
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
			options.iPopupDelay = GetDlgItemInt(hwndDlg, IDC_EDIT_POPUPDELAY, nullptr, FALSE);
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
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Init options

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Contacts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPANEL);
	odp.szTitle.w = LPGENW("Buddy Expectator");
	odp.pfnDlgProc = OptionsFrameProc;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.w = LPGENW("Popups");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPPANEL);
	odp.pfnDlgProc = PopupOptionsFrameProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// UserInfo initalization

struct UserinfoDlg : public CUserInfoPageDlg
{
	UserinfoDlg() :
		CUserInfoPageDlg(g_plugin, IDD_USERINFO)
	{}

	bool OnInitDialog() override
	{
		time_t tmpTime;
		wchar_t tmpBuf[51] = { 0 };
		tmpTime = getLastSeen(m_hContact);
		if (tmpTime == -1)
			SetDlgItemTextW(m_hwnd, IDC_EDIT_LASTSEEN, TranslateT("not detected"));
		else {
			wcsftime(tmpBuf, 50, L"%#x, %#X", gmtime(&tmpTime));
			SetDlgItemTextW(m_hwnd, IDC_EDIT_LASTSEEN, tmpBuf);
		}

		tmpTime = getLastInputMsg(m_hContact);
		if (tmpTime == -1)
			SetDlgItemTextW(m_hwnd, IDC_EDIT_LASTINPUT, TranslateT("not found"));
		else {
			wcsftime(tmpBuf, 50, L"%#x, %#X", gmtime(&tmpTime));
			SetDlgItemTextW(m_hwnd, IDC_EDIT_LASTINPUT, tmpBuf);
		}

		unsigned int AbsencePeriod = g_plugin.getDword(m_hContact, "iAbsencePeriod", options.iAbsencePeriod);

		SendDlgItemMessage(m_hwnd, IDC_SPINABSENCE, UDM_SETRANGE, 0, MAKELONG(999, 1));
		SetDlgItemInt(m_hwnd, IDC_EDITABSENCE, AbsencePeriod, FALSE);

		if (isContactGoneFor(m_hContact, options.iAbsencePeriod2))
			SetDlgItemText(m_hwnd, IDC_EDIT_WILLNOTICE, TranslateT("This contact has been absent for an extended period of time."));
		else
			SetDlgItemText(m_hwnd, IDC_EDIT_WILLNOTICE, L"");

		CheckDlgButton(m_hwnd, IDC_CHECK_MISSYOU, g_plugin.getByte(m_hContact, "MissYou") ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHECK_NOTIFYALWAYS, g_plugin.getByte(m_hContact, "MissYouNotifyAlways") ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHECK_NEVERHIDE, g_plugin.getByte(m_hContact, "NeverHide") ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		if (m_hContact) {
			g_plugin.setDword(m_hContact, "iAbsencePeriod", GetDlgItemInt(m_hwnd, IDC_EDITABSENCE, nullptr, FALSE));
			g_plugin.setByte(m_hContact, "MissYou", (IsDlgButtonChecked(m_hwnd, IDC_CHECK_MISSYOU) == BST_CHECKED) ? 1 : 0);
			g_plugin.setByte(m_hContact, "MissYouNotifyAlways", (IsDlgButtonChecked(m_hwnd, IDC_CHECK_NOTIFYALWAYS) == BST_CHECKED) ? 1 : 0);
			g_plugin.setByte(m_hContact, "NeverHide", (IsDlgButtonChecked(m_hwnd, IDC_CHECK_NEVERHIDE) == BST_CHECKED) ? 1 : 0);
		}
		return true;
	}
};

int UserinfoInit(WPARAM wparam, LPARAM hContact)
{
	if (hContact > 0) {
		USERINFOPAGE uip = {};
		uip.szTitle.a = LPGEN("Buddy Expectator");
		uip.pDialog = new UserinfoDlg();
		g_plugin.addUserInfo(wparam, &uip);
	}
	return 0;
}
