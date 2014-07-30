/*
Copyright (C) 2010 Mataes

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

#include "common.h"

INT_PTR CALLBACK UpdateNotifyOptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_UPDATEONSTARTUP, opts.bUpdateOnStartup);
		CheckDlgButton(hwndDlg, IDC_ONLYONCEADAY, opts.bOnlyOnceADay);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ONLYONCEADAY), opts.bUpdateOnStartup);
		CheckDlgButton(hwndDlg, IDC_UPDATEONPERIOD, opts.bUpdateOnPeriod);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PERIOD), opts.bUpdateOnPeriod);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODSPIN), opts.bUpdateOnPeriod);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), opts.bUpdateOnPeriod);
		CheckDlgButton(hwndDlg, IDC_SILENTMODE, opts.bSilentMode);
		if (db_get_b(NULL, MODNAME, "NeedRestart", 0))
			ShowWindow(GetDlgItem(hwndDlg, IDC_NEEDRESTARTLABEL), SW_SHOW);

		SendDlgItemMessage(hwndDlg, IDC_PERIODSPIN, UDM_SETRANGE, 0, MAKELONG(99, 1));
		SendDlgItemMessage(hwndDlg, IDC_PERIODSPIN, UDM_SETPOS, 0, (LPARAM)opts.Period);

		Edit_LimitText(GetDlgItem(hwndDlg, IDC_PERIOD), 2);

		ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), 0, TranslateT("hours"));
		ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), 1, TranslateT("days"));
		ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_PERIODMEASURE), opts.bPeriodMeasure);

		BYTE UpdateMode = db_get_b(NULL, MODNAME, "UpdateMode", UPDATE_MODE_STABLE);
		if (UpdateMode == UPDATE_MODE_STABLE) {
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL));
			CheckDlgButton(hwndDlg, IDC_STABLE, TRUE);
		}
		else if (UpdateMode == UPDATE_MODE_TRUNK) {
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL_TRUNK));
			CheckDlgButton(hwndDlg, IDC_TRUNK, TRUE);
		}
		else if (UpdateMode == UPDATE_MODE_TRUNK_SYMBOLS) {
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL_TRUNK_SYMBOLS));
			CheckDlgButton(hwndDlg, IDC_TRUNK_SYMBOLS, TRUE);
		}
		else {
			CheckDlgButton(hwndDlg, IDC_CUSTOM, TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), TRUE);
			DBVARIANT dbv;
			if (!db_get_s(NULL, MODNAME, "UpdateURL", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_CUSTOMURL, dbv.pszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL));
		}
	}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_UPDATEONSTARTUP:
			EnableWindow(GetDlgItem(hwndDlg, IDC_ONLYONCEADAY), IsDlgButtonChecked(hwndDlg, IDC_UPDATEONSTARTUP));

		case IDC_SILENTMODE:
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

		case IDC_TRUNK_SYMBOLS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL_TRUNK_SYMBOLS));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_TRUNK:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL_TRUNK));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_STABLE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), FALSE);
			SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_CUSTOM:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CUSTOMURL), TRUE);
			{
				DBVARIANT dbv;
				if (!db_get_ts(NULL, MODNAME, "UpdateURL", &dbv)) {
					SetDlgItemText(hwndDlg, IDC_CUSTOMURL, dbv.ptszVal);
					db_free(&dbv);
				}
				else
					SetDlgItemText(hwndDlg, IDC_CUSTOMURL, _T(DEFAULT_UPDATE_URL));
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PERIOD:
		case IDC_CUSTOMURL:
			if ((HWND)lParam == GetFocus() && (HIWORD(wParam) == EN_CHANGE))
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PERIODMEASURE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_LINK_HOTKEY:
			{
				OPENOPTIONSDIALOG ood = {0};
				ood.cbSize = sizeof(ood);
				ood.pszGroup = "Customize";
				ood.pszPage = "Hotkeys";
				Options_Open(&ood);
			}
			return true;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if (hdr && hdr->code == UDN_DELTAPOS)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			if (hdr && hdr->code == PSN_APPLY) {
				opts.bUpdateOnStartup = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONSTARTUP);
				opts.bOnlyOnceADay = IsDlgButtonChecked(hwndDlg, IDC_ONLYONCEADAY);

				opts.bUpdateOnPeriod = IsDlgButtonChecked(hwndDlg, IDC_UPDATEONPERIOD);
				opts.bSilentMode = IsDlgButtonChecked(hwndDlg, IDC_SILENTMODE);

				TCHAR buffer[3] = {0};
				Edit_GetText(GetDlgItem(hwndDlg, IDC_PERIOD), buffer, SIZEOF(buffer));
				opts.Period = _ttoi(buffer);

				opts.bPeriodMeasure = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_PERIODMEASURE));

				db_set_b(NULL, MODNAME, "UpdateOnStartup", opts.bUpdateOnStartup);
				db_set_b(NULL, MODNAME, "OnlyOnceADay", opts.bOnlyOnceADay);
				db_set_b(NULL, MODNAME, "UpdateOnPeriod", opts.bUpdateOnPeriod);
				db_set_b(NULL, MODNAME, "PeriodMeasure", opts.bPeriodMeasure);
				db_set_b(NULL, MODNAME, "SilentMode", opts.bSilentMode);
				db_set_dw(NULL, MODNAME, "Period", opts.Period);

				mir_forkthread(InitTimer, (void*)1);

				if ( IsDlgButtonChecked(hwndDlg, IDC_STABLE))
					db_set_b(NULL, MODNAME, "UpdateMode", UPDATE_MODE_STABLE);
				else if ( IsDlgButtonChecked(hwndDlg, IDC_TRUNK))
					db_set_b(NULL, MODNAME, "UpdateMode", UPDATE_MODE_TRUNK);
				else if ( IsDlgButtonChecked(hwndDlg, IDC_TRUNK_SYMBOLS)) {
					BYTE oldMode = db_get_b(NULL, MODNAME, "UpdateMode",0);
					 if(oldMode != UPDATE_MODE_TRUNK_SYMBOLS) {
						opts.bForceRedownload = true;
						db_set_b(NULL, MODNAME, "ForceRedownload", 1);
					}

					db_set_b(NULL, MODNAME, "UpdateMode", UPDATE_MODE_TRUNK_SYMBOLS);
				} else {
					char szUrl[100];
					GetDlgItemTextA(hwndDlg, IDC_CUSTOMURL, szUrl, SIZEOF(szUrl));
					db_set_s(NULL, MODNAME, "UpdateURL", szUrl);
					db_set_b(NULL, MODNAME, "UpdateMode", UPDATE_MODE_CUSTOM);
				}
			}
			break;
		}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgPopupOpts(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		//Colors
		if (PopupOptions.DefColors == byCOLOR_OWN) {
			CheckDlgButton(hdlg, IDC_USEOWNCOLORS, BST_CHECKED);
		}
		else if (PopupOptions.DefColors == byCOLOR_WINDOWS) {
			CheckDlgButton(hdlg, IDC_USEWINCOLORS, BST_CHECKED);
		}
		else if (PopupOptions.DefColors == byCOLOR_POPUP) {
			CheckDlgButton(hdlg, IDC_USEPOPUPCOLORS, BST_CHECKED);
		}
		for (int i = 0; i < POPUPS; i++) {
			SendDlgItemMessage(hdlg, (i+42071), CPM_SETCOLOUR, 0, PopupsList[i].colorBack);
			SendDlgItemMessage(hdlg, (i+41071), CPM_SETCOLOUR, 0, PopupsList[i].colorText);
			EnableWindow(GetDlgItem(hdlg, (i+42071)), (PopupOptions.DefColors == byCOLOR_OWN));
			EnableWindow(GetDlgItem(hdlg, (i+41071)), (PopupOptions.DefColors == byCOLOR_OWN));
		}
		//Timeout
		SendDlgItemMessage(hdlg, IDC_TIMEOUT_VALUE, EM_LIMITTEXT, 4, 0);
		SendDlgItemMessage(hdlg, IDC_TIMEOUT_VALUE_SPIN, UDM_SETRANGE32, -1, 9999);
		SetDlgItemInt(hdlg, IDC_TIMEOUT_VALUE, PopupOptions.Timeout, TRUE);
		//Mouse actions
		for (int i = 0; i < SIZEOF(PopupActions); i++) {
			SendMessage(GetDlgItem(hdlg, IDC_LC), CB_SETITEMDATA, SendMessage(GetDlgItem(hdlg, IDC_LC), CB_ADDSTRING, 0, (LPARAM)TranslateTS(PopupActions[i].Text)), PopupActions[i].Action);
			SendMessage(GetDlgItem(hdlg, IDC_RC), CB_SETITEMDATA, SendMessage(GetDlgItem(hdlg, IDC_RC), CB_ADDSTRING, 0, (LPARAM)TranslateTS(PopupActions[i].Text)), PopupActions[i].Action);
		}
		SendDlgItemMessage(hdlg, IDC_LC, CB_SETCURSEL, PopupOptions.LeftClickAction, 0);
		SendDlgItemMessage(hdlg, IDC_RC, CB_SETCURSEL, PopupOptions.RightClickAction, 0);

		//Popups nitified
		for (int i = 0; i < POPUPS; i++) {
			char str[20] = {0}, str2[20] = {0};
			mir_snprintf(str, SIZEOF(str), "Popups%d", i);
			mir_snprintf(str2, SIZEOF(str2), "Popups%dM", i);
			CheckDlgButton(hdlg, (i+40071), (db_get_b(NULL, MODNAME, str, DEFAULT_POPUP_ENABLED)) ? BST_CHECKED: BST_UNCHECKED);
			CheckDlgButton(hdlg, (i+1024), (db_get_b(NULL, MODNAME, str2, DEFAULT_MESSAGE_ENABLED)) ? BST_CHECKED: BST_UNCHECKED);
			if (IsDlgButtonChecked(hdlg, (i+40071)))
				EnableWindow(GetDlgItem(hdlg, (i+1024)), FALSE);
			else if (i > 0)
				EnableWindow(GetDlgItem(hdlg, (i+1024)), TRUE);
		}
		return TRUE;

	case WM_COMMAND:
		{
			WORD idCtrl = LOWORD(wParam), wNotifyCode = HIWORD(wParam);
			if (wNotifyCode == CPN_COLOURCHANGED) {
				if (idCtrl > 40070) 				{
					//It's a color picker change. idCtrl is the control id.
					COLORREF color = SendDlgItemMessage(hdlg, idCtrl, CPM_GETCOLOUR, 0, 0);
					int ctlID = idCtrl;
					if ((ctlID > 41070) && (ctlID < 42070)) //It's 41071 or above => Text color.
						PopupsList[ctlID-41071].colorText = color;
					else if (ctlID > 42070)//Background color.
						PopupsList[ctlID-42071].colorBack = color;
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
					return TRUE;
				}
			}

			if (wNotifyCode == CBN_SELCHANGE) {
				if (idCtrl == IDC_LC)
					PopupOptions.LeftClickAction = (BYTE)SendDlgItemMessage(hdlg, IDC_LC, CB_GETCURSEL, 0, 0);
				else if (idCtrl == IDC_RC)
					PopupOptions.RightClickAction = (BYTE)SendDlgItemMessage(hdlg, IDC_RC, CB_GETCURSEL, 0, 0);

				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				return TRUE;
			}
			switch(idCtrl) {
			case IDC_USEOWNCOLORS:
				if (wNotifyCode != BN_CLICKED)
					break;

				PopupOptions.DefColors = byCOLOR_OWN;

				for (int i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i+42071)), TRUE); //Background
					EnableWindow(GetDlgItem(hdlg, (i+41071)), TRUE); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEWINCOLORS:
				if (wNotifyCode != BN_CLICKED)
					break;

				//Use Windows colors
				PopupOptions.DefColors = byCOLOR_WINDOWS;
				for (int i = 0; i < POPUPS; i++) {
					EnableWindow(GetDlgItem(hdlg, (i+42071)), FALSE); //Background
					EnableWindow(GetDlgItem(hdlg, (i+41071)), FALSE); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_USEPOPUPCOLORS:
				if (wNotifyCode != BN_CLICKED)
					break;

				//Use Popup colors
				PopupOptions.DefColors = byCOLOR_POPUP;
				for (int i = 0; i < POPUPS; i++)  {
					EnableWindow(GetDlgItem(hdlg, (i+42071)), FALSE); //Background
					EnableWindow(GetDlgItem(hdlg, (i+41071)), FALSE); //Text
				}
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_PREVIEW:
				{//Declarations and initializations
					LPCTSTR Title = TranslateT("Plugin Updater");
					LPCTSTR Text = TranslateT("Test");
					for (int i = 0; i < POPUPS; i++) {
						if ((!IsDlgButtonChecked(hdlg, (i+40071))) || (!IsWindowEnabled(GetDlgItem(hdlg, (i+40071)))))
							continue;
						ShowPopup(Title, Text, i);
					}
				}
				break;

			case IDC_TIMEOUT_VALUE:
			case IDC_MSG_BOXES:
			case IDC_ERRORS:
				EnableWindow(GetDlgItem(hdlg, IDC_ERRORS_MSG), !IsDlgButtonChecked(hdlg, IDC_ERRORS));
				if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_INFO_MESSAGES:
				EnableWindow(GetDlgItem(hdlg, IDC_INFO_MESSAGES_MSG), !IsDlgButtonChecked(hdlg, IDC_INFO_MESSAGES));
				if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_PROGR_DLG:
				EnableWindow(GetDlgItem(hdlg, IDC_PROGR_DLG_MSG), !IsDlgButtonChecked(hdlg, IDC_PROGR_DLG));
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
			{
				//Text color
				char szSetting[20] = {0};
				DWORD ctlColor = 0;
				for (int i = 0; i < POPUPS; i++) {
					ctlColor = SendDlgItemMessage(hdlg, (i+42071), CPM_GETCOLOUR, 0, 0);
					PopupsList[i].colorBack = ctlColor;
					mir_snprintf(szSetting, SIZEOF(szSetting), "Popups%iBg", i);
					db_set_dw(NULL, MODNAME, szSetting, ctlColor);
					ctlColor = SendDlgItemMessage(hdlg, (i+41071), CPM_GETCOLOUR, 0, 0);
					PopupsList[i].colorText = ctlColor;
					mir_snprintf(szSetting, SIZEOF(szSetting), "Popups%iTx", i);
					db_set_dw(NULL, MODNAME, szSetting, ctlColor);
				}
				//Colors
				db_set_b(NULL, MODNAME, "DefColors", PopupOptions.DefColors);
				//Timeout
				PopupOptions.Timeout = GetDlgItemInt(hdlg, IDC_TIMEOUT_VALUE, 0, TRUE);
				db_set_dw(NULL, MODNAME, "Timeout", PopupOptions.Timeout);
				//Left mouse click
				db_set_b(NULL, MODNAME, "LeftClickAction", PopupOptions.LeftClickAction);
				//Right mouse click
				db_set_b(NULL, MODNAME, "RightClickAction", PopupOptions.RightClickAction);
				//Notified popups
				for (int i = 0; i < POPUPS; i++) {
					char str[20] = {0}, str2[20] = {0};
					mir_snprintf(str, SIZEOF(str), "Popups%d", i);
					db_set_b(NULL, MODNAME, str, (BYTE)(IsDlgButtonChecked(hdlg, (i+40071))));
					mir_snprintf(str2, SIZEOF(str2), "Popups%dM", i);
					db_set_b(NULL, MODNAME, str2, (BYTE)(IsDlgButtonChecked(hdlg, (i+1024))));
				}
				return TRUE;
			} //case PSN_APPLY
		} // switch code
		break; //End WM_NOTIFY
	} //switch message
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_UPDATENOTIFY);
	odp.ptszGroup = LPGENT("Services");
	odp.ptszTitle = LPGENT("Plugin Updater");
	odp.pfnDlgProc = UpdateNotifyOptsProc;
	Options_AddPage(wParam, &odp);

	if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUP);
		odp.ptszGroup = LPGENT("Popups");
		odp.ptszTitle = LPGENT("Plugin Updater");
		odp.pfnDlgProc = DlgPopupOpts;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptInit);
}