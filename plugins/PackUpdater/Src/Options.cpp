/*
Copyright (C) 2011-22 Mataes

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

LRESULT CALLBACK MyEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_KEYDOWN:
		SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
		break;
	}
	return mir_callNextSubclass(hwnd, MyEditProc, message, wParam, lParam);
}

INT_PTR CALLBACK UpdateNotifyOptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_UPDATEONSTARTUP, UpdateOnStartup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONLYONCEADAY, OnlyOnceADay ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ONLYONCEADAY), UpdateOnStartup);
		CheckDlgButton(hwndDlg, IDC_UPDATEONPERIOD, UpdateOnPeriod ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PERIOD), UpdateOnPeriod);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODSPIN), UpdateOnPeriod);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), UpdateOnPeriod);

		SendDlgItemMessage(hwndDlg, IDC_PERIODSPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));
		SendDlgItemMessage(hwndDlg, IDC_PERIODSPIN, UDM_SETPOS, 0, (LPARAM)Period);

		Edit_LimitText(GetDlgItem(hwndDlg, IDC_PERIOD), 2);
		mir_subclassWindow(GetDlgItem(hwndDlg, IDC_PERIOD), MyEditProc);

		ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), 0, TranslateT("hours"));
		ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), 1, TranslateT("days"));
		ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), PeriodMeasure);

		CheckDlgButton(hwndDlg, IDC_REMINDER, Reminder ? BST_CHECKED : BST_UNCHECKED);

		ShowWindow(GetDlgItem(hwndDlg, IDC_NOTIFY2), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_MSG_BOXES2), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_ERRORS2), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_INFO_MESSAGES2), SW_HIDE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_PROGR_DLG2), SW_HIDE);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UPDATEONSTARTUP:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ONLYONCEADAY), IsDlgButtonChecked(hwndDlg, IDC_UPDATEONSTARTUP));
			break;

		case IDC_ONLYONCEADAY:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_UPDATEONPERIOD:
		{
			BOOL value = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONPERIOD);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PERIOD), value);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODSPIN), value);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), value);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

		case IDC_PERIODMEASURE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_REMINDER:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_LINK_HOTKEY:
			g_plugin.openOptions(L"Customize", L"Hotkeys");
			return true;

		case IDC_MSG_BOXES2:
		case IDC_ERRORS2:
		case IDC_INFO_MESSAGES2:
		case IDC_PROGR_DLG2:
			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
	{
		NMHDR *hdr = (NMHDR *)lParam;
		if (hdr && hdr->code == UDN_DELTAPOS)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		if (hdr && hdr->code == PSN_APPLY) {
			UpdateOnStartup = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONSTARTUP);
			OnlyOnceADay = IsDlgButtonChecked(hwndDlg, IDC_ONLYONCEADAY);

			UpdateOnPeriod = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONPERIOD);

			char buffer[3] = { 0 };
			Edit_GetText(GetDlgItem(hwndDlg, IDC_PERIOD), (LPWSTR)&buffer, 2);
			Period = atoi(buffer);

			PeriodMeasure = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_PERIODMEASURE));

			InitTimer();

			g_plugin.setByte("UpdateOnStartup", UpdateOnStartup);
			g_plugin.setByte("OnlyOnceADay", OnlyOnceADay);
			g_plugin.setByte("UpdateOnPeriod", UpdateOnPeriod);
			g_plugin.setDword("Period", Period);
			g_plugin.setByte("PeriodMeasure", PeriodMeasure);
			Reminder = IsDlgButtonChecked(hwndDlg, IDC_REMINDER);
			g_plugin.setByte("Reminder", Reminder);
		}
		break;
	}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgPopupOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	uint16_t idCtrl;
	char str[20] = { 0 }, str2[20] = { 0 };

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		//Colors
		if (MyOptions.DefColors == byCOLOR_OWN) {
			CheckDlgButton(hdlg, IDC_USEOWNCOLORS, BST_CHECKED);
			CheckDlgButton(hdlg, IDC_USEPOPUPCOLORS, BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_USEWINCOLORS, BST_UNCHECKED);
		}
		if (MyOptions.DefColors == byCOLOR_WINDOWS) {
			CheckDlgButton(hdlg, IDC_USEOWNCOLORS, BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_USEPOPUPCOLORS, BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_USEWINCOLORS, BST_CHECKED);
		}
		if (MyOptions.DefColors == byCOLOR_POPUP) {
			CheckDlgButton(hdlg, IDC_USEOWNCOLORS, BST_UNCHECKED);
			CheckDlgButton(hdlg, IDC_USEPOPUPCOLORS, BST_CHECKED);
			CheckDlgButton(hdlg, IDC_USEWINCOLORS, BST_UNCHECKED);
		}
		for (i = 0; i < POPUPS; i++) {
			SendDlgItemMessage(hdlg, (i + 42071), CPM_SETCOLOUR, 0, PopupsList[i].colorBack);
			SendDlgItemMessage(hdlg, (i + 41071), CPM_SETCOLOUR, 0, PopupsList[i].colorText);
			EnableWindow(GetDlgItem(hdlg, (i + 42071)), (MyOptions.DefColors == byCOLOR_OWN));
			EnableWindow(GetDlgItem(hdlg, (i + 41071)), (MyOptions.DefColors == byCOLOR_OWN));
		}
		//Timeout
		SendDlgItemMessage(hdlg, IDC_TIMEOUT_VALUE, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hdlg, IDC_TIMEOUT_VALUE_SPIN, UDM_SETRANGE32, -1, 9999);
		SetDlgItemInt(hdlg, IDC_TIMEOUT_VALUE, MyOptions.Timeout, TRUE);
		//Mouse actions
		for (i = 0; i < _countof(PopupActions); i++) {
			SendDlgItemMessage(hdlg, IDC_LC, CB_SETITEMDATA, SendDlgItemMessage(hdlg, IDC_LC, CB_ADDSTRING, 0, (LPARAM)TranslateW(PopupActions[i].Text)), PopupActions[i].Action);
			SendDlgItemMessage(hdlg, IDC_RC, CB_SETITEMDATA, SendDlgItemMessage(hdlg, IDC_RC, CB_ADDSTRING, 0, (LPARAM)TranslateW(PopupActions[i].Text)), PopupActions[i].Action);
		}
		SendDlgItemMessage(hdlg, IDC_LC, CB_SETCURSEL, MyOptions.LeftClickAction, 0);
		SendDlgItemMessage(hdlg, IDC_RC, CB_SETCURSEL, MyOptions.RightClickAction, 0);
		//Popups nitified
		for (i = 0; i < POPUPS; i++) {
			mir_snprintf(str, "Popups%d", i);
			mir_snprintf(str2, "Popups%dM", i);
			CheckDlgButton(hdlg, (i + 40071), (g_plugin.getByte(str, DEFAULT_POPUP_ENABLED)) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hdlg, (i + 1024), (g_plugin.getByte(str2, DEFAULT_MESSAGE_ENABLED)) ? BST_CHECKED : BST_UNCHECKED);
			if (IsDlgButtonChecked(hdlg, (i + 40071)))
				EnableWindow(GetDlgItem(hdlg, (i + 1024)), FALSE);
			else if (i > 0)
				EnableWindow(GetDlgItem(hdlg, (i + 1024)), TRUE);
		}
		if (!(db_get_dw(0, "Popup", "Actions", 0) & 1))
			EnableWindow(GetDlgItem(hdlg, (40071)), FALSE);
		else
			EnableWindow(GetDlgItem(hdlg, (40071)), TRUE);

		return TRUE;

	case WM_SHOWWINDOW:
		if (!(db_get_dw(0, "Popup", "Actions", 0) & 1))
			EnableWindow(GetDlgItem(hdlg, (40071)), FALSE);
		else
			EnableWindow(GetDlgItem(hdlg, (40071)), TRUE);
		return TRUE;

	case WM_COMMAND:
		idCtrl = LOWORD(wParam);
		if (HIWORD(wParam) == CPN_COLOURCHANGED) {
			if (idCtrl > 40070) {
				//It's a color picker change. idCtrl is the control id.
				COLORREF color = SendDlgItemMessage(hdlg, idCtrl, CPM_GETCOLOUR, 0, 0);
				int ctlID = idCtrl;
				if ((ctlID > 41070) && (ctlID < 42070)) //It's 41071 or above => Text color.
					PopupsList[ctlID - 41071].colorText = color;
				else if (ctlID > 42070)//Background color.
					PopupsList[ctlID - 42071].colorBack = color;
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				return TRUE;
			}
		}
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			if (idCtrl == IDC_LC)
				MyOptions.LeftClickAction = (uint8_t)SendDlgItemMessage(hdlg, IDC_LC, CB_GETCURSEL, 0, 0);
			else if (idCtrl == IDC_RC)
				MyOptions.RightClickAction = (uint8_t)SendDlgItemMessage(hdlg, IDC_RC, CB_GETCURSEL, 0, 0);

			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}

		switch (idCtrl) {
		case IDC_USEOWNCOLORS:
			if (HIWORD(wParam) == BN_CLICKED) {
				MyOptions.DefColors = byCOLOR_OWN;
				BOOL bEnableOthers = TRUE;
				for (i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i + 42071)), bEnableOthers); //Background
					EnableWindow(GetDlgItem(hdlg, (i + 41071)), bEnableOthers); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			}
			break;
		case IDC_USEWINCOLORS:
			if (HIWORD(wParam) == BN_CLICKED) {
				//Use Windows colors
				MyOptions.DefColors = byCOLOR_WINDOWS;
				BOOL bEnableOthers = FALSE;
				for (i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i + 42071)), bEnableOthers); //Background
					EnableWindow(GetDlgItem(hdlg, (i + 41071)), bEnableOthers); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_USEPOPUPCOLORS:
			if (HIWORD(wParam) == BN_CLICKED) {
				//Use Popup colors
				MyOptions.DefColors = byCOLOR_POPUP;
				BOOL bEnableOthers = FALSE;
				for (i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i + 42071)), bEnableOthers); //Background
					EnableWindow(GetDlgItem(hdlg, (i + 41071)), bEnableOthers); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_PREVIEW:
			Title = TranslateT("Pack Updater");
			Text = TranslateT("Test");
			for (i = 0; i < POPUPS; i++) {
				if ((BST_UNCHECKED == IsDlgButtonChecked(hdlg, (i + 40071))) || (!IsWindowEnabled(GetDlgItem(hdlg, (i + 40071)))))
					continue;
				show_popup(nullptr, Title, Text, i, 0);
			}
			break;

		case IDC_TIMEOUT_VALUE:
		case IDC_MSG_BOXES:
		case IDC_ERRORS:
			if (BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_ERRORS))
				EnableWindow(GetDlgItem(hdlg, IDC_ERRORS_MSG), TRUE);
			else
				EnableWindow(GetDlgItem(hdlg, IDC_ERRORS_MSG), FALSE);
			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_INFO_MESSAGES:
			if (BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_INFO_MESSAGES))
				EnableWindow(GetDlgItem(hdlg, IDC_INFO_MESSAGES_MSG), TRUE);
			else
				EnableWindow(GetDlgItem(hdlg, IDC_INFO_MESSAGES_MSG), FALSE);
			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PROGR_DLG:
			if (BST_UNCHECKED == IsDlgButtonChecked(hdlg, IDC_PROGR_DLG))
				EnableWindow(GetDlgItem(hdlg, IDC_PROGR_DLG_MSG), TRUE);
			else
				EnableWindow(GetDlgItem(hdlg, IDC_PROGR_DLG_MSG), FALSE);
			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_MSG_BOXES_MSG:
		case IDC_ERRORS_MSG:
		case IDC_INFO_MESSAGES_MSG:
		case IDC_PROGR_DLG_MSG:
			if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_RESET:
			//Restore the options stored in memory.
			LoadOptions();
			InitPopupList();
			return TRUE;

		case PSN_APPLY:
			//Text color
			char szSetting[20] = { 0 };
			uint32_t ctlColor = 0;
			for (i = 0; i <= POPUPS - 1; i++) {
				ctlColor = SendDlgItemMessage(hdlg, (i + 42071), CPM_GETCOLOUR, 0, 0);
				PopupsList[i].colorBack = ctlColor;
				mir_snprintf(szSetting, "Popups%iBg", i);
				g_plugin.setDword(szSetting, ctlColor);
				ctlColor = SendDlgItemMessage(hdlg, (i + 41071), CPM_GETCOLOUR, 0, 0);
				PopupsList[i].colorText = ctlColor;
				mir_snprintf(szSetting, "Popups%iTx", i);
				g_plugin.setDword(szSetting, ctlColor);
			}
			//Colors
			g_plugin.setByte("DefColors", MyOptions.DefColors);
			//Timeout
			MyOptions.Timeout = GetDlgItemInt(hdlg, IDC_TIMEOUT_VALUE, nullptr, TRUE);
			g_plugin.setDword("Timeout", MyOptions.Timeout);
			//Left mouse click
			g_plugin.setByte("LeftClickAction", MyOptions.LeftClickAction);
			//Right mouse click
			g_plugin.setByte("RightClickAction", MyOptions.RightClickAction);
			//Notified popups
			for (i = 0; i < POPUPS; i++) {
				mir_snprintf(str, "Popups%d", i);
				g_plugin.setByte(str, (uint8_t)(IsDlgButtonChecked(hdlg, (i + 40071))));
				mir_snprintf(str2, "Popups%dM", i);
				g_plugin.setByte(str2, (uint8_t)(IsDlgButtonChecked(hdlg, (i + 1024))));
			}
			return TRUE;
		}
		break; //End WM_NOTIFY
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_UPDATENOTIFY);
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = LPGENW("Pack Updater");
	odp.pfnDlgProc = UpdateNotifyOptsProc;
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
	odp.szGroup.w = LPGENW("Popups");
	odp.szTitle.w = LPGENW("Pack Updater");
	odp.pfnDlgProc = DlgPopupOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
