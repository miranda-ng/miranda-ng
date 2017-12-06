/*
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

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

#include "..\stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Basic options

static INT_PTR CALLBACK DlgProcKSBasicOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			LVCOLUMN lvCol;
			LVITEM lvItem;
			DBVARIANT dbv;

			SetDlgItemInt(hwndDlg, IDC_MAXRETRIES, db_get_b(0, KSMODULENAME, SETTING_MAXRETRIES, DEFAULT_MAXRETRIES), FALSE);
			SetDlgItemInt(hwndDlg, IDC_INITDELAY, db_get_dw(0, KSMODULENAME, SETTING_INITDELAY, DEFAULT_INITDELAY), FALSE);
			CheckDlgButton(hwndDlg, IDC_CHECKCONNECTION, db_get_b(0, KSMODULENAME, SETTING_CHECKCONNECTION, FALSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWCONNECTIONPOPUPS, (db_get_b(0, KSMODULENAME, SETTING_SHOWCONNECTIONPOPUPS, FALSE) && ServiceExists(MS_POPUP_SHOWMESSAGE)) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CHKINET, db_get_b(0, KSMODULENAME, SETTING_CHKINET, FALSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_CONTCHECK, db_get_b(0, KSMODULENAME, SETTING_CONTCHECK, FALSE) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_BYPING, db_get_b(0, KSMODULENAME, SETTING_BYPING, FALSE) ? BST_CHECKED : BST_UNCHECKED);
			if (!db_get_s(0, KSMODULENAME, SETTING_PINGHOST, &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_PINGHOST, dbv.pszVal);
				db_free(&dbv);
			}

			// proto list
			HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
			memset(&lvCol, 0, sizeof(lvCol));
			lvCol.mask = LVCF_WIDTH | LVCF_TEXT;
			lvCol.pszText = TranslateT("Protocol");
			lvCol.cx = 118;
			ListView_InsertColumn(hList, 0, &lvCol);
			// fill the list
			memset(&lvItem, 0, sizeof(lvItem));
			lvItem.mask = LVIF_TEXT | LVIF_PARAM;
			lvItem.iItem = 0;
			lvItem.iSubItem = 0;

			int count;
			PROTOACCOUNT** protos;
			Proto_EnumAccounts(&count, &protos);

			for (int i = 0; i < count; i++) {
				if (!IsSuitableProto(protos[i]))
					continue;

				lvItem.pszText = protos[i]->tszAccountName;
				lvItem.lParam = (LPARAM)protos[i]->szModuleName;
				ListView_InsertItem(hList, &lvItem);

				char dbSetting[128];
				mir_snprintf(dbSetting, "%s_enabled", protos[i]->szModuleName);
				ListView_SetCheckState(hList, lvItem.iItem, db_get_b(0, KSMODULENAME, dbSetting, TRUE));
				lvItem.iItem++;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXRETRIES), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCONNECTIONPOPUPS), ServiceExists(MS_POPUP_SHOWMESSAGE) && IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_INITDELAY), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOCOLLIST), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKINET), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONTCHECK), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BYPING), (IsDlgButtonChecked(hwndDlg, IDC_CONTCHECK) && IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION)) ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PINGHOST), (IsDlgButtonChecked(hwndDlg, IDC_CONTCHECK)) && (IsDlgButtonChecked(hwndDlg, IDC_BYPING) && IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION)) ? TRUE : FALSE);
		}
		break;

	case WM_COMMAND:
		if (((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)) && ((HWND)lParam == GetFocus()))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		// something changed
		switch (LOWORD(wParam)) {
		case IDC_CHECKCONNECTION:
		case IDC_CONTCHECK:
		case IDC_BYPING:
			BOOL bEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXRETRIES), bEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCONNECTIONPOPUPS), ServiceExists(MS_POPUP_SHOWMESSAGE) && bEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_INITDELAY), bEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOCOLLIST), bEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHKINET), bEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONTCHECK), bEnabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BYPING), (IsDlgButtonChecked(hwndDlg, IDC_CONTCHECK) && bEnabled) ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PINGHOST), (IsDlgButtonChecked(hwndDlg, IDC_CONTCHECK)) && (IsDlgButtonChecked(hwndDlg, IDC_BYPING) && bEnabled) ? TRUE : FALSE);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((NMHDR*)lParam)->idFrom == IDC_PROTOCOLLIST) {
			switch (((NMHDR*)lParam)->code) {
			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
					if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_PROTOCOLLIST)) && ((nmlv->uNewState^nmlv->uOldState)&LVIS_STATEIMAGEMASK))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			}
		}

		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int i;
			LVITEM lvItem;

			db_set_b(0, KSMODULENAME, SETTING_MAXRETRIES, (BYTE)GetDlgItemInt(hwndDlg, IDC_MAXRETRIES, nullptr, FALSE));
			db_set_b(0, KSMODULENAME, SETTING_CHECKCONNECTION, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			db_set_b(0, KSMODULENAME, SETTING_SHOWCONNECTIONPOPUPS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWCONNECTIONPOPUPS));
			db_set_dw(0, KSMODULENAME, SETTING_INITDELAY, (DWORD)GetDlgItemInt(hwndDlg, IDC_INITDELAY, nullptr, FALSE));
			db_set_b(0, KSMODULENAME, SETTING_CHKINET, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CHKINET));
			db_set_b(0, KSMODULENAME, SETTING_CONTCHECK, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CONTCHECK));
			db_set_b(0, KSMODULENAME, SETTING_BYPING, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_BYPING));
			if (IsDlgButtonChecked(hwndDlg, IDC_BYPING)) {
				char *host;

				int len = SendDlgItemMessage(hwndDlg, IDC_PINGHOST, WM_GETTEXTLENGTH, 0, 0);
				if (len > 0) {
					host = (char*)malloc(len + 1);
					if (host != nullptr) {
						memset(host, '\0', len + 1);
						GetDlgItemTextA(hwndDlg, IDC_PINGHOST, host, len + 1);
						db_set_s(0, KSMODULENAME, SETTING_PINGHOST, host);
					}
				}
			}
			HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);
			memset(&lvItem, 0, sizeof(lvItem));
			lvItem.mask = LVIF_PARAM;
			for (i = 0; i < ListView_GetItemCount(hList); i++) {
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hList, &lvItem);

				char dbSetting[128];
				mir_snprintf(dbSetting, "%s_enabled", (char *)lvItem.lParam);
				db_set_b(0, KSMODULENAME, dbSetting, (BYTE)ListView_GetCheckState(hList, lvItem.iItem));
			}
		}
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Advanced options

static INT_PTR CALLBACK DlgProcKSAdvOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetDlgItemInt(hwndDlg, IDC_MAXDELAY, db_get_dw(0, KSMODULENAME, SETTING_MAXDELAY, DEFAULT_MAXDELAY), FALSE);
		SetDlgItemInt(hwndDlg, IDC_MAXCONNECTINGTIME, db_get_dw(0, KSMODULENAME, SETTING_MAXCONNECTINGTIME, 0), FALSE);
		CheckDlgButton(hwndDlg, IDC_INCREASEEXPONENTIAL, db_get_b(0, KSMODULENAME, SETTING_INCREASEEXPONENTIAL, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CNCOTHERLOC, (db_get_b(0, KSMODULENAME, SETTING_CNCOTHERLOC, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOGINERR, db_get_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_NOTHING) == LOGINERR_NOTHING ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECKAPMRESUME, (db_get_b(0, KSMODULENAME, SETTING_CHECKAPMRESUME, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FIRSTOFFLINE, (db_get_b(0, KSMODULENAME, SETTING_FIRSTOFFLINE, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOLOCKED, (db_get_b(0, KSMODULENAME, SETTING_NOLOCKED, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_LOGINERR_DELAY, db_get_dw(0, KSMODULENAME, SETTING_LOGINERR_DELAY, DEFAULT_MAXDELAY), FALSE);
		SetDlgItemInt(hwndDlg, IDC_PINGCOUNT, db_get_w(0, KSMODULENAME, SETTING_PINGCOUNT, DEFAULT_PINGCOUNT), FALSE);
		SetDlgItemInt(hwndDlg, IDC_CNTDELAY, db_get_dw(0, KSMODULENAME, SETTING_CNTDELAY, CHECKCONTIN_DELAY), FALSE);

		switch (db_get_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_CANCEL)) {
		case LOGINERR_SETDELAY:
			CheckRadioButton(hwndDlg, IDC_LOGINERR_CANCEL, IDC_LOGINERR_SETDELAY, IDC_LOGINERR_SETDELAY);
			break;
		default:
		case LOGINERR_CANCEL:
			CheckRadioButton(hwndDlg, IDC_LOGINERR_CANCEL, IDC_LOGINERR_SETDELAY, IDC_LOGINERR_CANCEL);
			break;
		}
		break;

	case WM_COMMAND:
		if (((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)) && ((HWND)lParam == GetFocus()))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		// something changed
		switch (LOWORD(wParam)) {
		case IDC_LOGINERR_CANCEL:
		case IDC_LOGINERR_SETDELAY:
			CheckRadioButton(hwndDlg, IDC_LOGINERR_CANCEL, IDC_LOGINERR_SETDELAY, LOWORD(wParam));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_DELAY), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR_SETDELAY) && IsDlgButtonChecked(hwndDlg, IDC_LOGINERR) && IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			db_set_b(0, KSMODULENAME, SETTING_INCREASEEXPONENTIAL, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_INCREASEEXPONENTIAL));
			db_set_dw(0, KSMODULENAME, SETTING_MAXDELAY, (DWORD)GetDlgItemInt(hwndDlg, IDC_MAXDELAY, nullptr, FALSE));
			db_set_dw(0, KSMODULENAME, SETTING_MAXCONNECTINGTIME, (DWORD)GetDlgItemInt(hwndDlg, IDC_MAXCONNECTINGTIME, nullptr, FALSE));
			db_set_b(0, KSMODULENAME, SETTING_FIRSTOFFLINE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_FIRSTOFFLINE));
			db_set_b(0, KSMODULENAME, SETTING_NOLOCKED, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOLOCKED));
			db_set_b(0, KSMODULENAME, SETTING_CNCOTHERLOC, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CNCOTHERLOC));
			db_set_b(0, KSMODULENAME, SETTING_LOGINERR, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
			if (IsDlgButtonChecked(hwndDlg, IDC_LOGINERR)) {
				if (IsDlgButtonChecked(hwndDlg, IDC_LOGINERR_SETDELAY)) {
					db_set_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_SETDELAY);
					db_set_dw(0, KSMODULENAME, SETTING_LOGINERR_DELAY, GetDlgItemInt(hwndDlg, IDC_LOGINERR_DELAY, nullptr, FALSE));
				}
				else db_set_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_CANCEL);
			}
			else db_set_b(0, KSMODULENAME, SETTING_LOGINERR, LOGINERR_NOTHING);

			db_set_b(0, KSMODULENAME, SETTING_CHECKAPMRESUME, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CHECKAPMRESUME));
			db_set_w(0, KSMODULENAME, SETTING_PINGCOUNT, (WORD)GetDlgItemInt(hwndDlg, IDC_PINGCOUNT, nullptr, FALSE));
			db_set_dw(0, KSMODULENAME, SETTING_CNTDELAY, (DWORD)GetDlgItemInt(hwndDlg, IDC_CNTDELAY, nullptr, FALSE) == 0 ? CHECKCONTIN_DELAY : GetDlgItemInt(hwndDlg, IDC_CNTDELAY, nullptr, FALSE));
		}
		break;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

static INT_PTR CALLBACK PopupOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bFreeze = false;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		bFreeze = true;

		// left action
		switch (db_get_b(0, KSMODULENAME, SETTING_POPUP_LEFTCLICK, POPUP_ACT_CANCEL)) {
		case POPUP_ACT_CLOSEPOPUP:
			CheckDlgButton(hwndDlg, IDC_LCLOSE, BST_CHECKED);
			break;

		case POPUP_ACT_CANCEL:
			CheckDlgButton(hwndDlg, IDC_LCANCEL, BST_CHECKED);
			break;

		case POPUP_ACT_NOTHING:
		default:
			CheckDlgButton(hwndDlg, IDC_LNOTHING, BST_CHECKED);
			break;
		}
		// right action
		switch (db_get_b(0, KSMODULENAME, SETTING_POPUP_RIGHTCLICK, POPUP_ACT_CANCEL)) {
		case POPUP_ACT_CLOSEPOPUP:
			CheckDlgButton(hwndDlg, IDC_RCLOSE, BST_CHECKED);
			break;

		case POPUP_ACT_CANCEL:
			CheckDlgButton(hwndDlg, IDC_RCANCEL, BST_CHECKED);
			break;

		case POPUP_ACT_NOTHING:
		default:
			CheckDlgButton(hwndDlg, IDC_RNOTHING, BST_CHECKED);
			break;
		}
		// delay
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYCUSTOM), ServiceExists(MS_POPUP_ADDPOPUPT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYFROMPU), ServiceExists(MS_POPUP_ADDPOPUPT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYPERMANENT), ServiceExists(MS_POPUP_ADDPOPUPT));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), ServiceExists(MS_POPUP_ADDPOPUPT));
		switch (db_get_b(0, KSMODULENAME, SETTING_POPUP_DELAYTYPE, POPUP_DELAYFROMPU)) {
		case POPUP_DELAYCUSTOM:
			CheckDlgButton(hwndDlg, IDC_DELAYCUSTOM, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), ServiceExists(MS_POPUP_ADDPOPUPT));
			break;

		case POPUP_DELAYPERMANENT:
			CheckDlgButton(hwndDlg, IDC_DELAYPERMANENT, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), FALSE);
			break;

		case POPUP_DELAYFROMPU:
		default:
			CheckDlgButton(hwndDlg, IDC_DELAYFROMPU, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), FALSE);
			break;
		}
		// delay
		SetDlgItemInt(hwndDlg, IDC_DELAY, db_get_dw(0, KSMODULENAME, SETTING_POPUP_TIMEOUT, 0), FALSE);
		// back color
		SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, db_get_dw(0, KSMODULENAME, SETTING_POPUP_BACKCOLOR, 0xAAAAAA));
		// text
		SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, db_get_dw(0, KSMODULENAME, SETTING_POPUP_TEXTCOLOR, 0x0000CC));
		// wincolors
		CheckDlgButton(hwndDlg, IDC_WINCOLORS, db_get_b(0, KSMODULENAME, SETTING_POPUP_USEWINCOLORS, 0) ? BST_CHECKED : BST_UNCHECKED);
		// defaultcolors
		CheckDlgButton(hwndDlg, IDC_DEFAULTCOLORS, ((db_get_b(0, KSMODULENAME, SETTING_POPUP_USEDEFCOLORS, 0)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS))) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), ((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS))));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), ((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS))));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS)));
		// popup types
		CheckDlgButton(hwndDlg, IDC_PUCONNLOST, db_get_b(0, KSMODULENAME, SETTING_PUCONNLOST, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PUOTHER, db_get_b(0, KSMODULENAME, SETTING_PUOTHER, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PUCONNRETRY, db_get_b(0, KSMODULENAME, SETTING_PUCONNRETRY, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PURESULT, db_get_b(0, KSMODULENAME, SETTING_PURESULT, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PUSHOWEXTRA, db_get_b(0, KSMODULENAME, SETTING_PUSHOWEXTRA, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		bFreeze = false;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_WINCOLORS:
		case IDC_DEFAULTCOLORS:
			EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), ((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS))));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), ((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS))));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)));
			EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS)));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_DELAYFROMPU:
		case IDC_DELAYPERMANENT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), FALSE);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_DELAYCUSTOM:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), TRUE);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_BGCOLOR:
		case IDC_TEXTCOLOR:
		case IDC_DELAY:
		case IDC_LNOTHING:
		case IDC_LCLOSE:
		case IDC_LCANCEL:
		case IDC_RNOTHING:
		case IDC_RCLOSE:
		case IDC_RCANCEL:
		case IDC_PUCONNLOST:
		case IDC_PUOTHER:
		case IDC_PUCONNRETRY:
		case IDC_PURESULT:
		case IDC_PUSHOWEXTRA:
			if (!bFreeze)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PREV:
			{
				POPUPDATAT ppd = {};
				ppd.lchIcon = Skin_LoadIcon(SKINICON_STATUS_OFFLINE);
				wcsncpy(ppd.lptzContactName, TranslateT("Keep status"), MAX_CONTACTNAME);
				wcsncpy(ppd.lptzText, TranslateT("You broke the Internet!"), MAX_SECONDLINE);
				if (IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) {
					ppd.colorBack = GetSysColor(COLOR_BTNFACE);
					ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS)) {
					ppd.colorBack = 0;
					ppd.colorText = 0;
				}
				else {
					ppd.colorBack = SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0);
					ppd.colorText = SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0);
				}
				ppd.PluginWindowProc = KSPopupDlgProc;
				ppd.PluginData = nullptr;
				if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU)) {
					ppd.iSeconds = 0;
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM)) {
					ppd.iSeconds = GetDlgItemInt(hwndDlg, IDC_DELAY, nullptr, FALSE);
				}
				else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT)) {
					ppd.iSeconds = -1;
				}
				PUAddPopupT(&ppd);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			// left action
			if (IsDlgButtonChecked(hwndDlg, IDC_LNOTHING))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_LEFTCLICK, POPUP_ACT_NOTHING);
			else if (IsDlgButtonChecked(hwndDlg, IDC_LCLOSE))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_LEFTCLICK, POPUP_ACT_CLOSEPOPUP);
			else if (IsDlgButtonChecked(hwndDlg, IDC_LCANCEL))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_LEFTCLICK, POPUP_ACT_CANCEL);
			// right action
			if (IsDlgButtonChecked(hwndDlg, IDC_RNOTHING))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_RIGHTCLICK, POPUP_ACT_NOTHING);
			else if (IsDlgButtonChecked(hwndDlg, IDC_RCLOSE))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_RIGHTCLICK, POPUP_ACT_CLOSEPOPUP);
			else if (IsDlgButtonChecked(hwndDlg, IDC_RCANCEL))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_RIGHTCLICK, POPUP_ACT_CANCEL);
			// delay
			if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_DELAYTYPE, POPUP_DELAYFROMPU);
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_DELAYTYPE, POPUP_DELAYCUSTOM);
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
				db_set_b(0, KSMODULENAME, SETTING_POPUP_DELAYTYPE, POPUP_DELAYPERMANENT);
			// delay
			db_set_dw(0, KSMODULENAME, SETTING_POPUP_TIMEOUT, GetDlgItemInt(hwndDlg, IDC_DELAY, nullptr, FALSE));
			// back color
			db_set_dw(0, KSMODULENAME, SETTING_POPUP_BACKCOLOR, SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0));
			// text color
			db_set_dw(0, KSMODULENAME, SETTING_POPUP_TEXTCOLOR, SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0));
			// use win
			db_set_b(0, KSMODULENAME, SETTING_POPUP_USEWINCOLORS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
			// use def
			db_set_b(0, KSMODULENAME, SETTING_POPUP_USEDEFCOLORS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
			// store types
			db_set_b(0, KSMODULENAME, SETTING_PUCONNLOST, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_PUCONNLOST));
			db_set_b(0, KSMODULENAME, SETTING_PUOTHER, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_PUOTHER));
			db_set_b(0, KSMODULENAME, SETTING_PUCONNRETRY, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_PUCONNRETRY));
			db_set_b(0, KSMODULENAME, SETTING_PURESULT, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_PURESULT));
			db_set_b(0, KSMODULENAME, SETTING_PUSHOWEXTRA, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_PUSHOWEXTRA));
		}
		break;
	}

	return 0;
}

int KeepStatusOptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.hInstance = hInst;
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Keep status");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Basic");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KS_BASIC);
	odp.pfnDlgProc = DlgProcKSBasicOpts;
	Options_AddPage(wparam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KS_ADV);
	odp.pfnDlgProc = DlgProcKSAdvOpts;
	Options_AddPage(wparam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		memset(&odp, 0, sizeof(odp));
		odp.position = 150000000;
		odp.szGroup.a = LPGEN("Popups");
		odp.hInstance = hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_PUOPT_KEEPSTATUS);
		odp.szTitle.a = LPGEN("Keep status");
		odp.pfnDlgProc = PopupOptDlgProc;
		odp.flags = ODPF_BOLDGROUPS;
		Options_AddPage(wparam, &odp);
	}
	return 0;
}
