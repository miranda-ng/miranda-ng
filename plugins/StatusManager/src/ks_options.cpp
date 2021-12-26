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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Basic options

static INT_PTR CALLBACK DlgProcKSBasicOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetDlgItemInt(hwndDlg, IDC_MAXRETRIES, KSPlugin.getByte(SETTING_MAXRETRIES, DEFAULT_MAXRETRIES), FALSE);
		SetDlgItemInt(hwndDlg, IDC_INITDELAY, KSPlugin.getDword(SETTING_INITDELAY, DEFAULT_INITDELAY), FALSE);
		SetDlgItemTextA(hwndDlg, IDC_PINGHOST, KSPlugin.getMStringA(SETTING_PINGHOST));
		CheckDlgButton(hwndDlg, IDC_CHECKCONNECTION, KSPlugin.getByte(SETTING_CHECKCONNECTION, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWCONNECTIONPOPUPS, KSPlugin.getByte(SETTING_SHOWCONNECTIONPOPUPS, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHKINET, KSPlugin.getByte(SETTING_CHKINET, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CONTCHECK, KSPlugin.getByte(SETTING_CONTCHECK, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_BYPING, KSPlugin.getByte(SETTING_BYPING, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		{
			// proto list
			HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			LVCOLUMN lvCol;
			memset(&lvCol, 0, sizeof(lvCol));
			lvCol.mask = LVCF_WIDTH | LVCF_TEXT;
			lvCol.pszText = TranslateT("Protocol");
			lvCol.cx = 118;
			ListView_InsertColumn(hList, 0, &lvCol);

			// fill the list
			LVITEM lvItem;
			memset(&lvItem, 0, sizeof(lvItem));
			lvItem.mask = LVIF_TEXT | LVIF_PARAM;
			lvItem.iItem = 0;
			lvItem.iSubItem = 0;

			for (auto &pa : Accounts()) {
				if (!IsSuitableProto(pa))
					continue;

				lvItem.pszText = pa->tszAccountName;
				lvItem.lParam = (LPARAM)pa->szModuleName;
				ListView_InsertItem(hList, &lvItem);

				char dbSetting[128];
				mir_snprintf(dbSetting, "%s_enabled", pa->szModuleName);
				ListView_SetCheckState(hList, lvItem.iItem, KSPlugin.getByte(dbSetting, TRUE));
				lvItem.iItem++;
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_MAXRETRIES), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCONNECTIONPOPUPS), IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
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
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWCONNECTIONPOPUPS), bEnabled);
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
				NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
				if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_PROTOCOLLIST)) && ((nmlv->uNewState^nmlv->uOldState)&LVIS_STATEIMAGEMASK))
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}

		switch (((LPNMHDR)lParam)->code) {
		case PSN_WIZFINISH:
			KSLoadOptions();
			break;

		case PSN_APPLY:
			KSPlugin.setByte(SETTING_MAXRETRIES, (uint8_t)GetDlgItemInt(hwndDlg, IDC_MAXRETRIES, nullptr, FALSE));
			KSPlugin.setByte(SETTING_CHECKCONNECTION, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CHECKCONNECTION));
			KSPlugin.setByte(SETTING_SHOWCONNECTIONPOPUPS, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_SHOWCONNECTIONPOPUPS));
			KSPlugin.setDword(SETTING_INITDELAY, (uint32_t)GetDlgItemInt(hwndDlg, IDC_INITDELAY, nullptr, FALSE));
			KSPlugin.setByte(SETTING_CHKINET, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CHKINET));
			KSPlugin.setByte(SETTING_CONTCHECK, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CONTCHECK));
			KSPlugin.setByte(SETTING_BYPING, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_BYPING));
			if (IsDlgButtonChecked(hwndDlg, IDC_BYPING)) {
				int len = SendDlgItemMessage(hwndDlg, IDC_PINGHOST, WM_GETTEXTLENGTH, 0, 0);
				if (len > 0) {
					ptrA host((char*)mir_alloc(len + 1));
					GetDlgItemTextA(hwndDlg, IDC_PINGHOST, host, len + 1);
					KSPlugin.setString(SETTING_PINGHOST, host);
				}
			}

			HWND hList = GetDlgItem(hwndDlg, IDC_PROTOCOLLIST);
			LVITEM lvItem;
			lvItem.mask = LVIF_PARAM;
			for (int i = 0; i < ListView_GetItemCount(hList); i++) {
				lvItem.iItem = i;
				lvItem.iSubItem = 0;
				ListView_GetItem(hList, &lvItem);

				char dbSetting[128];
				mir_snprintf(dbSetting, "%s_enabled", (char *)lvItem.lParam);
				KSPlugin.setByte(dbSetting, (uint8_t)ListView_GetCheckState(hList, lvItem.iItem));
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
		SetDlgItemInt(hwndDlg, IDC_MAXDELAY, KSPlugin.getDword(SETTING_MAXDELAY, DEFAULT_MAXDELAY), FALSE);
		SetDlgItemInt(hwndDlg, IDC_MAXCONNECTINGTIME, KSPlugin.getDword(SETTING_MAXCONNECTINGTIME, 0), FALSE);
		CheckDlgButton(hwndDlg, IDC_INCREASEEXPONENTIAL, KSPlugin.getByte(SETTING_INCREASEEXPONENTIAL, FALSE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CNCOTHERLOC, (KSPlugin.getByte(SETTING_CNCOTHERLOC, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_LOGINERR, KSPlugin.getByte(SETTING_LOGINERR, LOGINERR_NOTHING) == LOGINERR_NOTHING ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECKAPMRESUME, (KSPlugin.getByte(SETTING_CHECKAPMRESUME, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_FIRSTOFFLINE, (KSPlugin.getByte(SETTING_FIRSTOFFLINE, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_NOLOCKED, (KSPlugin.getByte(SETTING_NOLOCKED, FALSE)) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_LOGINERR_DELAY, KSPlugin.getDword(SETTING_LOGINERR_DELAY, DEFAULT_MAXDELAY), FALSE);
		SetDlgItemInt(hwndDlg, IDC_PINGCOUNT, KSPlugin.getWord(SETTING_PINGCOUNT, DEFAULT_PINGCOUNT), FALSE);
		SetDlgItemInt(hwndDlg, IDC_CNTDELAY, KSPlugin.getDword(SETTING_CNTDELAY, CHECKCONTIN_DELAY), FALSE);

		switch (KSPlugin.getByte(SETTING_LOGINERR, LOGINERR_CANCEL)) {
		case LOGINERR_SETDELAY:
			CheckRadioButton(hwndDlg, IDC_LOGINERR_CANCEL, IDC_LOGINERR_SETDELAY, IDC_LOGINERR_SETDELAY);
			break;
		default:
		case LOGINERR_CANCEL:
			CheckRadioButton(hwndDlg, IDC_LOGINERR_CANCEL, IDC_LOGINERR_SETDELAY, IDC_LOGINERR_CANCEL);
			break;
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_CANCEL), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_SETDELAY), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_DELAY), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR_SETDELAY) && IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
		break;

	case WM_COMMAND:
		if (((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)) && ((HWND)lParam == GetFocus()))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		// something changed
		switch (LOWORD(wParam)) {
		case IDC_LOGINERR:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_CANCEL), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_SETDELAY), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_DELAY), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR_SETDELAY) && IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
			break;

		case IDC_LOGINERR_CANCEL:
		case IDC_LOGINERR_SETDELAY:
			CheckRadioButton(hwndDlg, IDC_LOGINERR_CANCEL, IDC_LOGINERR_SETDELAY, LOWORD(wParam));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGINERR_DELAY), IsDlgButtonChecked(hwndDlg, IDC_LOGINERR_SETDELAY) && IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_WIZFINISH:
			KSLoadOptions();
			break;

		case PSN_APPLY:
			KSPlugin.setByte(SETTING_INCREASEEXPONENTIAL, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_INCREASEEXPONENTIAL));
			KSPlugin.setDword(SETTING_MAXDELAY, (uint32_t)GetDlgItemInt(hwndDlg, IDC_MAXDELAY, nullptr, FALSE));
			KSPlugin.setDword(SETTING_MAXCONNECTINGTIME, (uint32_t)GetDlgItemInt(hwndDlg, IDC_MAXCONNECTINGTIME, nullptr, FALSE));
			KSPlugin.setByte(SETTING_FIRSTOFFLINE, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_FIRSTOFFLINE));
			KSPlugin.setByte(SETTING_NOLOCKED, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_NOLOCKED));
			KSPlugin.setByte(SETTING_CNCOTHERLOC, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CNCOTHERLOC));
			KSPlugin.setByte(SETTING_LOGINERR, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_LOGINERR));
			if (IsDlgButtonChecked(hwndDlg, IDC_LOGINERR)) {
				if (IsDlgButtonChecked(hwndDlg, IDC_LOGINERR_SETDELAY)) {
					KSPlugin.setByte(SETTING_LOGINERR, LOGINERR_SETDELAY);
					KSPlugin.setDword(SETTING_LOGINERR_DELAY, GetDlgItemInt(hwndDlg, IDC_LOGINERR_DELAY, nullptr, FALSE));
				}
				else KSPlugin.setByte(SETTING_LOGINERR, LOGINERR_CANCEL);
			}
			else KSPlugin.setByte(SETTING_LOGINERR, LOGINERR_NOTHING);

			KSPlugin.setByte(SETTING_CHECKAPMRESUME, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_CHECKAPMRESUME));
			KSPlugin.setWord(SETTING_PINGCOUNT, (uint16_t)GetDlgItemInt(hwndDlg, IDC_PINGCOUNT, nullptr, FALSE));
			KSPlugin.setDword(SETTING_CNTDELAY, (uint32_t)GetDlgItemInt(hwndDlg, IDC_CNTDELAY, nullptr, FALSE) == 0 ? CHECKCONTIN_DELAY : GetDlgItemInt(hwndDlg, IDC_CNTDELAY, nullptr, FALSE));
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
		switch (KSPlugin.getByte(SETTING_POPUP_LEFTCLICK, POPUP_ACT_CANCEL)) {
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
		switch (KSPlugin.getByte(SETTING_POPUP_RIGHTCLICK, POPUP_ACT_CANCEL)) {
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
		switch (KSPlugin.getByte(SETTING_POPUP_DELAYTYPE, POPUP_DELAYFROMPU)) {
		case POPUP_DELAYCUSTOM:
			CheckDlgButton(hwndDlg, IDC_DELAYCUSTOM, BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), TRUE);
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
		SetDlgItemInt(hwndDlg, IDC_DELAY, KSPlugin.getDword(SETTING_POPUP_TIMEOUT, 0), FALSE);
		// back color
		SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_SETCOLOUR, 0, KSPlugin.getDword(SETTING_POPUP_BACKCOLOR, 0xAAAAAA));
		// text
		SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_SETCOLOUR, 0, KSPlugin.getDword(SETTING_POPUP_TEXTCOLOR, 0x0000CC));
		// wincolors
		CheckDlgButton(hwndDlg, IDC_WINCOLORS, KSPlugin.getByte(SETTING_POPUP_USEWINCOLORS, 0) ? BST_CHECKED : BST_UNCHECKED);
		// defaultcolors
		CheckDlgButton(hwndDlg, IDC_DEFAULTCOLORS, ((KSPlugin.getByte(SETTING_POPUP_USEDEFCOLORS, 0)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS))) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), ((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS))));
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), ((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS))));
		EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS)));
		EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS)));
		// popup types
		CheckDlgButton(hwndDlg, IDC_PUCONNLOST, KSPlugin.getByte(SETTING_PUCONNLOST, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PUOTHER, KSPlugin.getByte(SETTING_PUOTHER, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PUCONNRETRY, KSPlugin.getByte(SETTING_PUCONNRETRY, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PURESULT, KSPlugin.getByte(SETTING_PURESULT, TRUE) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PUSHOWEXTRA, KSPlugin.getByte(SETTING_PUSHOWEXTRA, TRUE) ? BST_CHECKED : BST_UNCHECKED);
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
				POPUPDATAW ppd;
				ppd.lchIcon = Skin_LoadIcon(SKINICON_STATUS_OFFLINE);
				wcsncpy(ppd.lpwzContactName, TranslateT("Keep status"), MAX_CONTACTNAME);
				wcsncpy(ppd.lpwzText, TranslateT("You broke the Internet!"), MAX_SECONDLINE);
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
				PUAddPopupW(&ppd);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			// left action
			if (IsDlgButtonChecked(hwndDlg, IDC_LNOTHING))
				KSPlugin.setByte(SETTING_POPUP_LEFTCLICK, POPUP_ACT_NOTHING);
			else if (IsDlgButtonChecked(hwndDlg, IDC_LCLOSE))
				KSPlugin.setByte(SETTING_POPUP_LEFTCLICK, POPUP_ACT_CLOSEPOPUP);
			else if (IsDlgButtonChecked(hwndDlg, IDC_LCANCEL))
				KSPlugin.setByte(SETTING_POPUP_LEFTCLICK, POPUP_ACT_CANCEL);
			// right action
			if (IsDlgButtonChecked(hwndDlg, IDC_RNOTHING))
				KSPlugin.setByte(SETTING_POPUP_RIGHTCLICK, POPUP_ACT_NOTHING);
			else if (IsDlgButtonChecked(hwndDlg, IDC_RCLOSE))
				KSPlugin.setByte(SETTING_POPUP_RIGHTCLICK, POPUP_ACT_CLOSEPOPUP);
			else if (IsDlgButtonChecked(hwndDlg, IDC_RCANCEL))
				KSPlugin.setByte(SETTING_POPUP_RIGHTCLICK, POPUP_ACT_CANCEL);
			// delay
			if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
				KSPlugin.setByte(SETTING_POPUP_DELAYTYPE, POPUP_DELAYFROMPU);
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
				KSPlugin.setByte(SETTING_POPUP_DELAYTYPE, POPUP_DELAYCUSTOM);
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
				KSPlugin.setByte(SETTING_POPUP_DELAYTYPE, POPUP_DELAYPERMANENT);
			// delay
			KSPlugin.setDword(SETTING_POPUP_TIMEOUT, GetDlgItemInt(hwndDlg, IDC_DELAY, nullptr, FALSE));
			// back color
			KSPlugin.setDword(SETTING_POPUP_BACKCOLOR, SendDlgItemMessage(hwndDlg, IDC_BGCOLOR, CPM_GETCOLOUR, 0, 0));
			// text color
			KSPlugin.setDword(SETTING_POPUP_TEXTCOLOR, SendDlgItemMessage(hwndDlg, IDC_TEXTCOLOR, CPM_GETCOLOUR, 0, 0));
			// use win
			KSPlugin.setByte(SETTING_POPUP_USEWINCOLORS, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
			// use def
			KSPlugin.setByte(SETTING_POPUP_USEDEFCOLORS, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
			// store types
			KSPlugin.setByte(SETTING_PUCONNLOST, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_PUCONNLOST));
			KSPlugin.setByte(SETTING_PUOTHER, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_PUOTHER));
			KSPlugin.setByte(SETTING_PUCONNRETRY, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_PUCONNRETRY));
			KSPlugin.setByte(SETTING_PURESULT, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_PURESULT));
			KSPlugin.setByte(SETTING_PUSHOWEXTRA, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_PUSHOWEXTRA));
		}
		break;
	}

	return 0;
}

int KeepStatusOptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Keep status");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Basic");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KS_BASIC);
	odp.pfnDlgProc = DlgProcKSBasicOpts;
	KSPlugin.addOptions(wparam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KS_ADV);
	odp.pfnDlgProc = DlgProcKSAdvOpts;
	KSPlugin.addOptions(wparam, &odp);

	memset(&odp, 0, sizeof(odp));
	odp.position = 150000000;
	odp.szGroup.a = LPGEN("Popups");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PUOPT_KEEPSTATUS);
	odp.szTitle.a = LPGEN("Keep status");
	odp.pfnDlgProc = PopupOptDlgProc;
	odp.flags = ODPF_BOLDGROUPS;
	KSPlugin.addOptions(wparam, &odp);
	return 0;
}
