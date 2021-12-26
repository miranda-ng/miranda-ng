/*
AdvancedAutoAway Plugin for Miranda-IM (www.miranda-im.org)
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

int LoadAutoAwaySetting(SMProto &autoAwaySetting, char* protoName);

bool g_bAAASettingSame;

struct AAMSGSETTING
{
	short useCustom;
	int   status;
	char* msg;
};

void DisableDialog(HWND hwndDlg)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUS), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RADUSECUSTOM), FALSE);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RADUSEMIRANDA), FALSE);
}

INT_PTR CALLBACK DlgProcAutoAwayMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static AAMSGSETTING** settings;
	static int last, count;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), ServiceExists(MS_VARS_SHOWHELP) ? SW_SHOW : SW_HIDE);
			count = 0;
			last = -1;

			PROTOACCOUNT** proto;
			int protoCount = 0;
			Proto_EnumAccounts(&protoCount, &proto);
			if (protoCount <= 0) {
				DisableDialog(hwndDlg);
				break;
			}

			uint32_t protoModeMsgFlags = 0;
			for (int i = 0; i < protoCount; i++)
				if (CallProtoService(proto[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND & ~PF1_INDIVMODEMSG)
					protoModeMsgFlags |= CallProtoService(proto[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);

			if (protoModeMsgFlags == 0) {
				DisableDialog(hwndDlg);
				break;
			}

			settings = (AAMSGSETTING**)mir_alloc(sizeof(AAMSGSETTING*));
			count = 0;
			for (auto &it : statusModes) {
				if (!(protoModeMsgFlags & Proto_Status2Flag(it.iStatus)))
					continue;

				int j = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_ADDSTRING, 0, (LPARAM)Clist_GetStatusModeDescription(it.iStatus, 0));
				SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETITEMDATA, j, it.iStatus);
				settings = (AAMSGSETTING**)mir_realloc(settings, (count + 1) * sizeof(AAMSGSETTING*));
				settings[count] = (AAMSGSETTING*)mir_alloc(sizeof(AAMSGSETTING));
				settings[count]->status = it.iStatus;

				DBVARIANT dbv;
				if (!AAAPlugin.getString(StatusModeToDbSetting(it.iStatus, SETTING_STATUSMSG), &dbv)) {
					settings[count]->msg = (char*)mir_alloc(mir_strlen(dbv.pszVal) + 1);
					mir_strcpy(settings[count]->msg, dbv.pszVal);
					db_free(&dbv);
				}
				else settings[count]->msg = nullptr;

				settings[count]->useCustom = AAAPlugin.getByte(StatusModeToDbSetting(it.iStatus, SETTING_MSGCUSTOM), FALSE);
				count += 1;
			}
			SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STATUS, CBN_SELCHANGE), 0);
		}
		break;

	case WM_COMMAND:
		if (((HIWORD(wParam) == EN_CHANGE) || (HIWORD(wParam) == BN_CLICKED)) && ((HWND)lParam == GetFocus()))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_RADUSEMIRANDA:
			CheckDlgButton(hwndDlg, IDC_RADUSECUSTOM, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_RADUSEMIRANDA) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			settings[SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCURSEL, 0, 0)]->useCustom = IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM);
			break;

		case IDC_RADUSECUSTOM:
			CheckDlgButton(hwndDlg, IDC_RADUSEMIRANDA, BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM));
			settings[SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCURSEL, 0, 0)]->useCustom = IsDlgButtonChecked(hwndDlg, IDC_RADUSECUSTOM);
			break;

		case IDC_STATUS:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int i = SendDlgItemMessage(hwndDlg, IDC_STATUS, CB_GETCURSEL, 0, 0);
				int len = SendDlgItemMessage(hwndDlg, IDC_STATUSMSG, WM_GETTEXTLENGTH, 0, 0);
				if (last != -1) {
					if (settings[last]->msg == nullptr)
						settings[last]->msg = (char*)mir_alloc(len + 1);
					else
						settings[last]->msg = (char*)mir_realloc(settings[last]->msg, len + 1);
					GetDlgItemTextA(hwndDlg, IDC_STATUSMSG, settings[last]->msg, (len + 1));
				}

				if (i != -1) {
					if (settings[i]->msg != nullptr)
						SetDlgItemTextA(hwndDlg, IDC_STATUSMSG, settings[i]->msg);
					else {
						ptrW msgw((wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, settings[i]->status, 0));
						SetDlgItemText(hwndDlg, IDC_STATUSMSG, (msgw != nullptr) ? msgw : L"");
					}

					if (settings[i]->useCustom) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), TRUE);
						CheckDlgButton(hwndDlg, IDC_RADUSECUSTOM, BST_CHECKED);
						CheckDlgButton(hwndDlg, IDC_RADUSEMIRANDA, BST_UNCHECKED);
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSMSG), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_VARIABLESHELP), FALSE);
						CheckDlgButton(hwndDlg, IDC_RADUSEMIRANDA, BST_CHECKED);
						CheckDlgButton(hwndDlg, IDC_RADUSECUSTOM, BST_UNCHECKED);
					}
				}
				last = i;
			}
			break;

		case IDC_VARIABLESHELP:
			CallService(MS_VARS_SHOWHELP, (WPARAM)GetDlgItem(hwndDlg, IDC_STATUSMSG), 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_WIZFINISH:
			AAALoadOptions();
			break;

		case PSN_APPLY:
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STATUS, CBN_SELCHANGE), 0);
			for (int i = 0; i < count; i++) {
				AAAPlugin.setByte(StatusModeToDbSetting(settings[i]->status, SETTING_MSGCUSTOM), (uint8_t)settings[i]->useCustom);
				if ((settings[i]->useCustom) && (settings[i]->msg != nullptr) && (settings[i]->msg[0] != '\0'))
					AAAPlugin.setString(StatusModeToDbSetting(settings[i]->status, SETTING_STATUSMSG), settings[i]->msg);
			}
			break;
		}
		break;

	case WM_DESTROY:
		for (int i = 0; i < count; i++) {
			mir_free(settings[i]->msg);
			mir_free(settings[i]);
		}
		mir_free(settings);
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Dialog service functions

static int WriteAutoAwaySetting(SMProto &autoAwaySetting, char *protoName)
{
	char setting[128];
	mir_snprintf(setting, "%s_OptionFlags", protoName);
	AAAPlugin.setWord(setting, (uint16_t)autoAwaySetting.optionFlags);
	mir_snprintf(setting, "%s_AwayTime", protoName);
	AAAPlugin.setWord(setting, (uint16_t)autoAwaySetting.awayTime);
	mir_snprintf(setting, "%s_NATime", protoName);
	AAAPlugin.setWord(setting, (uint16_t)autoAwaySetting.naTime);
	mir_snprintf(setting, "%s_StatusFlags", protoName);
	AAAPlugin.setWord(setting, (uint16_t)autoAwaySetting.statusFlags);
	mir_snprintf(setting, "%s_Lv1Status", protoName);
	AAAPlugin.setWord(setting, (uint16_t)autoAwaySetting.lv1Status);
	mir_snprintf(setting, "%s_Lv2Status", protoName);
	AAAPlugin.setWord(setting, (uint16_t)autoAwaySetting.lv2Status);

	return 0;
}

static void SetDialogItems(HWND hwndDlg, SMProto *setting)
{
	bool bSetNA = (setting->optionFlags & FLAG_SETNA) != 0;

	bool bIsTimed = (setting->optionFlags & FLAG_ONMOUSE) != 0;
	bool bSaver = (setting->optionFlags & FLAG_ONSAVER) != 0;
	bool bFullScr = (setting->optionFlags & FLAG_FULLSCREEN) != 0;
	bool bOnLock = (setting->optionFlags & FLAG_ONLOCK) != 0;
	bool bOnTS = (setting->optionFlags & FLAG_ONTS) != 0;
	bool bAnyOption = bIsTimed | bSaver | bFullScr | bOnLock | bOnTS;

	CheckDlgButton(hwndDlg, IDC_FULLSCREEN, bFullScr ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SCREENSAVE, bSaver ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_ONLOCK, (setting->optionFlags & FLAG_ONLOCK) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_ONTSDISCONNECT, bOnTS ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_TIMED, bIsTimed ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SETNA, bSetNA ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CONFIRM, (setting->optionFlags & FLAG_CONFIRM) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_RESETSTATUS, (setting->optionFlags & FLAG_RESET) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_ENTERIDLE, (setting->optionFlags & FLAG_ENTERIDLE) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_MONITORMIRANDA, (setting->optionFlags & FLAG_MONITORMIRANDA) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_LV2ONINACTIVE, (setting->optionFlags & FLAG_LV2ONINACTIVE) != 0 ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hwndDlg, IDC_AWAYTIME, setting->awayTime, FALSE);
	SetDlgItemInt(hwndDlg, IDC_NATIME, setting->naTime, FALSE);

	EnableWindow(GetDlgItem(hwndDlg, IDC_SETNA), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV2ONINACTIVE), bIsTimed && bSetNA);
	EnableWindow(GetDlgItem(hwndDlg, IDC_MONITORMIRANDA), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_AWAYTIME), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV1AFTERSTR), bIsTimed);

	EnableWindow(GetDlgItem(hwndDlg, IDC_LV1STATUS), bAnyOption);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSLIST), bAnyOption);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RESETSTATUS), bAnyOption && IsDlgButtonChecked(hwndDlg, IDC_LV2ONINACTIVE));
	EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM), bAnyOption && IsDlgButtonChecked(hwndDlg, IDC_LV2ONINACTIVE) && IsDlgButtonChecked(hwndDlg, IDC_RESETSTATUS));

	EnableWindow(GetDlgItem(hwndDlg, IDC_NATIME), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SETNASTR), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SETNASTR), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV2STATUS), bSetNA && bIsTimed);
	
	EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOCOL), !g_bAAASettingSame);
}

static SMProto* GetSetting(HWND hwndDlg, SMProto *sameSetting)
{
	if (g_bAAASettingSame)
		return sameSetting;

	int iItem = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_GETCURSEL, 0, 0);
	if (iItem == -1)
		return nullptr;

	INT_PTR iData = (INT_PTR)SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_GETITEMDATA, iItem, 0);
	return (iData == -1) ? nullptr : (SMProto*)iData;
}

static void SetDialogStatus(HWND hwndDlg, SMProto *sameSetting)
{
	SMProto *setting = GetSetting(hwndDlg, sameSetting);
	if (setting == nullptr)
		return;

	// create columns
	HWND hList = GetDlgItem(hwndDlg, IDC_STATUSLIST);
	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	LVCOLUMN lvCol = { 0 };
	lvCol.mask = LVCF_WIDTH | LVCF_TEXT;
	lvCol.pszText = TranslateT("Status");
	lvCol.cx = 118;
	ListView_InsertColumn(hList, 0, &lvCol);

	// get pointer to settings
	SetDialogItems(hwndDlg, setting);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rules dialog window procedure

static TProtoSettings optionSettings;

static INT_PTR CALLBACK DlgProcAutoAwayRulesOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static SMProto* sameSetting;
	SMProto *setting;
	static int init;

	switch (msg) {
	case WM_INITDIALOG:
		init = TRUE;
		TranslateDialogDefault(hwndDlg);

		// copy the settings
		optionSettings = protoList;

		sameSetting = (SMProto*)mir_alloc(sizeof(SMProto));
		LoadAutoAwaySetting(*sameSetting, SETTING_ALL);

		// fill list from currentProtoSettings
		{
			for (auto &p : optionSettings) {
				int item = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)p->m_tszAccName);
				SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_SETITEMDATA, item, (LPARAM)p);
			}
		}
		// set cursor to first protocol
		SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_SETCURSEL, 0, 0);

		// status list
		SetDialogStatus(hwndDlg, sameSetting);

		SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_PROTOCOL, (LPARAM)0x11111111);
		init = FALSE;
		break;

	case WM_COMMAND:
		if ((HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		setting = GetSetting(hwndDlg, sameSetting);
		if (!setting)
			break;

		switch (LOWORD(wParam)) {
		case IDC_PROTOCOL:
			// status listview
			init = TRUE;
			{
				HWND hList = GetDlgItem(hwndDlg, IDC_STATUSLIST);
				ListView_DeleteAllItems(hList);

				int flags = 0;
				if (!g_bAAASettingSame)
					flags = CallProtoService(setting->m_szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(setting->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);

				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_TEXT | LVIF_PARAM;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				for (auto &it : statusModes) {
					if ((flags & it.iFlag) || (it.iFlag == PF2_OFFLINE) || (g_bAAASettingSame)) {
						lvItem.pszText = Clist_GetStatusModeDescription(it.iStatus, 0);
						lvItem.lParam = (LPARAM)it.iFlag;
						ListView_InsertItem(hList, &lvItem);
						ListView_SetCheckState(hList, lvItem.iItem, setting->statusFlags & it.iFlag ? TRUE : FALSE);
						lvItem.iItem++;
					}
				}
			}
			init = FALSE;
			// status dropdown boxes
			{
				int flags = 0;
				if (!g_bAAASettingSame)
					flags = CallProtoService(setting->m_szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(setting->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);

				// clear box and add new status, loop status and check if compatible with proto
				SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_RESETCONTENT, 0, 0);
				for (auto &it : statusModes) {
					if ((flags & it.iFlag) || it.iFlag == PF2_OFFLINE || g_bAAASettingSame) {
						wchar_t *statusMode = Clist_GetStatusModeDescription(it.iStatus, 0);
						int item = SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_SETITEMDATA, item, it.iStatus);
						item = SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_SETITEMDATA, item, it.iStatus);
						if (it.iStatus == setting->lv1Status) {
							SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_SETCURSEL, (WPARAM)item, 0);
							SetDlgItemText(hwndDlg, IDC_SETNASTR, CMStringW(FORMAT, TranslateT("minutes of %s mode"), statusMode));
						}
						if (it.iStatus == setting->lv2Status)
							SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_SETCURSEL, (WPARAM)item, 0);
					}
				}
			}
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_LV1STATUS:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			setting->lv1Status = (int)SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_GETCURSEL, 0, 0), 0);
			SetDlgItemText(hwndDlg, IDC_SETNASTR, CMStringW(FORMAT, TranslateT("minutes of %s mode"), Clist_GetStatusModeDescription(setting->lv1Status, 0)));
			break;

		case IDC_LV2STATUS:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

			setting->lv2Status = (int)SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_GETCURSEL, 0, 0), 0);
			break;

		case IDC_FULLSCREEN:
			setting->optionFlags ^= FLAG_FULLSCREEN;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_SCREENSAVE:
			setting->optionFlags ^= FLAG_ONSAVER;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_ONLOCK:
			setting->optionFlags ^= FLAG_ONLOCK;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_ONTSDISCONNECT:
			setting->optionFlags ^= FLAG_ONTS;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_TIMED:
			setting->optionFlags ^= FLAG_ONMOUSE;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_SETNA:
			setting->optionFlags ^= FLAG_SETNA;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_AWAYTIME:
			setting->awayTime = GetDlgItemInt(hwndDlg, IDC_AWAYTIME, nullptr, FALSE);
			break;

		case IDC_NATIME:
			setting->naTime = GetDlgItemInt(hwndDlg, IDC_NATIME, nullptr, FALSE);
			break;

		case IDC_LV2ONINACTIVE:
			setting->optionFlags ^= FLAG_LV2ONINACTIVE;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_CONFIRM:
			setting->optionFlags ^= FLAG_CONFIRM;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_RESETSTATUS:
			setting->optionFlags ^= FLAG_RESET;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_MONITORMIRANDA:
			setting->optionFlags ^= FLAG_MONITORMIRANDA;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_ENTERIDLE:
			setting->optionFlags ^= FLAG_ENTERIDLE;
			SetDialogItems(hwndDlg, setting);
			break;
		}
		break;

	case WM_SHOWWINDOW:
		init = TRUE;

		if (setting = GetSetting(hwndDlg, sameSetting))
			SetDialogItems(hwndDlg, setting);

		SendMessage(hwndDlg, WM_COMMAND, (WPARAM)IDC_PROTOCOL, (LPARAM)0x11111111);
		init = FALSE;
		break;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_STATUSLIST:
			if (init)
				break;

			switch (((NMHDR*)lParam)->code) {
			case LVN_ITEMCHANGED:
				NMLISTVIEW *nmlv = (NMLISTVIEW*)lParam;
				if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_STATUSLIST)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK)) {
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

					if (setting = GetSetting(hwndDlg, sameSetting)) {
						HWND hList = GetDlgItem(hwndDlg, IDC_STATUSLIST);
						LVITEM lvItem = { 0 };
						lvItem.mask = LVIF_PARAM;
						lvItem.iItem = nmlv->iItem;
						ListView_GetItem(hList, &lvItem);
						int pf2Status = lvItem.lParam;
						if (ListView_GetCheckState(hList, lvItem.iItem))
							setting->statusFlags |= pf2Status;
						else
							setting->statusFlags &= ~pf2Status;
					}
				}
				break;
			}
			break;
		}

		switch (((LPNMHDR)lParam)->code) {
		case PSN_WIZFINISH:
			AAALoadOptions();
			break;

		case PSN_APPLY:
			if (g_bAAASettingSame)
				WriteAutoAwaySetting(*sameSetting, SETTING_ALL);
			else {
				for (auto &it : optionSettings)
					WriteAutoAwaySetting(*it, it->m_szName);
			}
		}
		break;

	case WM_DESTROY:
		optionSettings.destroy();
		mir_free(sameSetting);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// General options window procedure

static INT_PTR CALLBACK DlgProcAutoAwayGeneralOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_IGNLOCK, AAAPlugin.getByte(SETTING_IGNLOCK, FALSE) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IGNSYSKEYS, AAAPlugin.getByte(SETTING_IGNSYSKEYS, FALSE) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IGNALTCOMBO, AAAPlugin.getByte(SETTING_IGNALTCOMBO, FALSE) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MONITORMOUSE, AAAPlugin.getByte(SETTING_MONITORMOUSE, BST_CHECKED) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MONITORKEYBOARD, AAAPlugin.getByte(SETTING_MONITORKEYBOARD, BST_CHECKED) != 0 ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_AWAYCHECKTIMEINSECS, AAAPlugin.getWord(SETTING_AWAYCHECKTIMEINSECS, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_CONFIRMDELAY, AAAPlugin.getWord(SETTING_CONFIRMDELAY, 5), FALSE);
		CheckDlgButton(hwndDlg, g_bAAASettingSame ? IDC_SAMESETTINGS : IDC_PERPROTOCOLSETTINGS, BST_CHECKED);
		break;

	case WM_COMMAND:
		if ((HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_MONITORMOUSE:
			CheckDlgButton(hwndDlg, IDC_MONITORMOUSE, (((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD))) || (IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE)) ? BST_CHECKED : BST_UNCHECKED));
			break;

		case IDC_MONITORKEYBOARD:
			CheckDlgButton(hwndDlg, IDC_MONITORKEYBOARD, (((BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE)) && (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD))) || (IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD)) ? BST_CHECKED : BST_UNCHECKED));
			break;

		case IDC_SAMESETTINGS:
		case IDC_PERPROTOCOLSETTINGS:
			g_bAAASettingSame = IsDlgButtonChecked(hwndDlg, IDC_SAMESETTINGS);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_WIZFINISH:
				AAALoadOptions();
				break;

			case PSN_APPLY:
				AAAPlugin.setByte(SETTING_IGNLOCK, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_IGNLOCK));
				AAAPlugin.setByte(SETTING_IGNSYSKEYS, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_IGNSYSKEYS));
				AAAPlugin.setByte(SETTING_IGNALTCOMBO, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_IGNALTCOMBO));
				AAAPlugin.setByte(SETTING_SAMESETTINGS, (uint8_t)g_bAAASettingSame);
				AAAPlugin.setWord(SETTING_AWAYCHECKTIMEINSECS, (uint16_t)GetDlgItemInt(hwndDlg, IDC_AWAYCHECKTIMEINSECS, nullptr, FALSE));
				AAAPlugin.setWord(SETTING_CONFIRMDELAY, (uint16_t)GetDlgItemInt(hwndDlg, IDC_CONFIRMDELAY, nullptr, FALSE));
				AAAPlugin.setByte(SETTING_MONITORMOUSE, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE));
				AAAPlugin.setByte(SETTING_MONITORKEYBOARD, (uint8_t)IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD));
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options initialization procedure

int AutoAwayOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 1000000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Auto away");

	odp.szTab.a = LPGEN("General");
	odp.pfnDlgProc = DlgProcAutoAwayGeneralOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENAUTOAWAY);
	Options_AddPage(wParam, &odp, &AAAPlugin);

	odp.szTab.a = LPGEN("Rules");
	odp.pfnDlgProc = DlgProcAutoAwayRulesOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTOAWAY);
	Options_AddPage(wParam, &odp, &AAAPlugin);

	odp.szTab.a = LPGEN("Status messages");
	odp.pfnDlgProc = DlgProcAutoAwayMsgOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTOAWAYMSG);
	Options_AddPage(wParam, &odp, &AAAPlugin);
	return 0;
}
