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

#include "..\stdafx.h"

int LoadAutoAwaySetting(TAAAProtoSetting &autoAwaySetting, char* protoName);

INT_PTR CALLBACK DlgProcAutoAwayMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static BOOL bSettingSame = FALSE;

/////////////////////////////////////////////////////////////////////////////////////////
// Dialog service functions

static int WriteAutoAwaySetting(TAAAProtoSetting &autoAwaySetting, char *protoName)
{
	char setting[128];
	mir_snprintf(setting, "%s_OptionFlags", protoName);
	db_set_w(0, AAAMODULENAME, setting, (WORD)autoAwaySetting.optionFlags);
	mir_snprintf(setting, "%s_AwayTime", protoName);
	db_set_w(0, AAAMODULENAME, setting, (WORD)autoAwaySetting.awayTime);
	mir_snprintf(setting, "%s_NATime", protoName);
	db_set_w(0, AAAMODULENAME, setting, (WORD)autoAwaySetting.naTime);
	mir_snprintf(setting, "%s_StatusFlags", protoName);
	db_set_w(0, AAAMODULENAME, setting, (WORD)autoAwaySetting.statusFlags);
	mir_snprintf(setting, "%s_Lv1Status", protoName);
	db_set_w(0, AAAMODULENAME, setting, (WORD)autoAwaySetting.lv1Status);
	mir_snprintf(setting, "%s_Lv2Status", protoName);
	db_set_w(0, AAAMODULENAME, setting, (WORD)autoAwaySetting.lv2Status);

	return 0;
}

static void SetDialogItems(HWND hwndDlg, TAAAProtoSetting *setting)
{
	bool bIsTimed = (setting->optionFlags & FLAG_ONMOUSE) != 0;
	bool bSetNA = (setting->optionFlags & FLAG_SETNA) != 0;
	bool bSaver = (setting->optionFlags & FLAG_ONSAVER) != 0;
	bool bFullScr = (setting->optionFlags & FLAG_FULLSCREEN) != 0;

	CheckDlgButton(hwndDlg, IDC_FULLSCREEN, bFullScr ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SCREENSAVE, bSaver ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_ONLOCK, (setting->optionFlags & FLAG_ONLOCK) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_TIMED, bIsTimed ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SETNA, bSetNA ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CONFIRM, (setting->optionFlags & FLAG_CONFIRM) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_RESETSTATUS, (setting->optionFlags & FLAG_RESET) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_MONITORMIRANDA, (setting->optionFlags & FLAG_MONITORMIRANDA) != 0 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_LV2ONINACTIVE, (setting->optionFlags & FLAG_LV2ONINACTIVE) != 0 ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemInt(hwndDlg, IDC_AWAYTIME, setting->awayTime, FALSE);
	SetDlgItemInt(hwndDlg, IDC_NATIME, setting->naTime, FALSE);

	EnableWindow(GetDlgItem(hwndDlg, IDC_SETNA), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV2ONINACTIVE), bIsTimed && bSetNA);
	EnableWindow(GetDlgItem(hwndDlg, IDC_MONITORMIRANDA), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_AWAYTIME), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV1AFTERSTR), bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV1STATUS), bIsTimed || bSaver || bFullScr);
	EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSLIST), bIsTimed || bSaver || bFullScr);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RESETSTATUS), (bIsTimed || bSaver || bFullScr) && IsDlgButtonChecked(hwndDlg, IDC_LV2ONINACTIVE));
	EnableWindow(GetDlgItem(hwndDlg, IDC_CONFIRM), (bIsTimed || bSaver || bFullScr) && IsDlgButtonChecked(hwndDlg, IDC_LV2ONINACTIVE) && IsDlgButtonChecked(hwndDlg, IDC_RESETSTATUS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_NATIME), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SETNASTR), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SETNASTR), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LV2STATUS), bSetNA && bIsTimed);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOCOL), !bSettingSame);
}

static TAAAProtoSetting* GetSetting(HWND hwndDlg, TAAAProtoSetting *sameSetting)
{
	if (bSettingSame)
		return sameSetting;

	int iItem = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_GETCURSEL, 0, 0);
	if (iItem == -1)
		return nullptr;

	INT_PTR iData = (INT_PTR)SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_GETITEMDATA, iItem, 0);
	return (iData == -1) ? nullptr : (TAAAProtoSetting*)iData;
}

static void SetDialogStatus(HWND hwndDlg, TAAAProtoSetting *sameSetting)
{
	TAAAProtoSetting *setting = GetSetting(hwndDlg, sameSetting);
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

int AAACompareSettings(const TAAAProtoSetting *p1, const TAAAProtoSetting *p2)
{
	return mir_strcmp(p1->m_szName, p2->m_szName);
}

static TAAAProtoSettingList optionSettings(10, AAACompareSettings);

static INT_PTR CALLBACK DlgProcAutoAwayRulesOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TAAAProtoSetting* sameSetting;
	TAAAProtoSetting *setting;
	static int init;

	switch (msg) {
	case WM_INITDIALOG:
		init = TRUE;
		TranslateDialogDefault(hwndDlg);

		// copy the settings
		optionSettings = autoAwaySettings;

		sameSetting = (TAAAProtoSetting*)malloc(sizeof(TAAAProtoSetting));
		LoadAutoAwaySetting(*sameSetting, SETTING_ALL);

		// fill list from currentProtoSettings
		{
			for (int i = 0; i < optionSettings.getCount(); i++) {
				TAAAProtoSetting &p = optionSettings[i];
				int item = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)p.m_tszAccName);
				SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_SETITEMDATA, item, (LPARAM)&p);
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
				if (!bSettingSame)
					flags = CallProtoService(setting->m_szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(setting->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);

				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_TEXT | LVIF_PARAM;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				for (int i = 0; i < _countof(statusModeList); i++) {
					if ((flags & statusModePf2List[i]) || (statusModePf2List[i] == PF2_OFFLINE) || (bSettingSame)) {
						lvItem.pszText = pcli->pfnGetStatusModeDescription(statusModeList[i], 0);
						lvItem.lParam = (LPARAM)statusModePf2List[i];
						ListView_InsertItem(hList, &lvItem);
						ListView_SetCheckState(hList, lvItem.iItem, setting->statusFlags & statusModePf2List[i] ? TRUE : FALSE);
						lvItem.iItem++;
					}
				}
			}
			init = FALSE;
			// status dropdown boxes
			{
				int flags = 0;
				if (!bSettingSame)
					flags = CallProtoService(setting->m_szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(setting->m_szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);

				// clear box and add new status, loop status and check if compatible with proto
				SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_RESETCONTENT, 0, 0);
				for (int i = 0; i < _countof(statusModeList); i++) {
					if ((flags & statusModePf2List[i]) || statusModePf2List[i] == PF2_OFFLINE || bSettingSame) {
						wchar_t *statusMode = pcli->pfnGetStatusModeDescription(statusModeList[i], 0);
						int item = SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_SETITEMDATA, item, (LPARAM)statusModeList[i]);
						item = SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_SETITEMDATA, item, (LPARAM)statusModeList[i]);
						if (statusModeList[i] == setting->lv1Status) {
							SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_SETCURSEL, (WPARAM)item, 0);
							SetDlgItemText(hwndDlg, IDC_SETNASTR, CMStringW(FORMAT, TranslateT("minutes of %s mode"), statusMode));
						}
						if (statusModeList[i] == setting->lv2Status)
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
			SetDlgItemText(hwndDlg, IDC_SETNASTR, CMStringW(FORMAT, TranslateT("minutes of %s mode"), pcli->pfnGetStatusModeDescription(setting->lv1Status, 0)));
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
		case PSN_APPLY:
			if (bSettingSame)
				WriteAutoAwaySetting(*sameSetting, SETTING_ALL);
			else {
				for (int i = 0; i < optionSettings.getCount(); i++)
					WriteAutoAwaySetting(optionSettings[i], optionSettings[i].m_szName);
			}
			AAALoadOptions(autoAwaySettings, FALSE);
		}
		break;

	case WM_DESTROY:
		optionSettings.destroy();
		free(sameSetting);
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
		CheckDlgButton(hwndDlg, IDC_IGNLOCK, db_get_b(0, AAAMODULENAME, SETTING_IGNLOCK, FALSE) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IGNSYSKEYS, db_get_b(0, AAAMODULENAME, SETTING_IGNSYSKEYS, FALSE) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_IGNALTCOMBO, db_get_b(0, AAAMODULENAME, SETTING_IGNALTCOMBO, FALSE) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MONITORMOUSE, db_get_b(0, AAAMODULENAME, SETTING_MONITORMOUSE, BST_CHECKED) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MONITORKEYBOARD, db_get_b(0, AAAMODULENAME, SETTING_MONITORKEYBOARD, BST_CHECKED) != 0 ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_AWAYCHECKTIMEINSECS, db_get_w(0, AAAMODULENAME, SETTING_AWAYCHECKTIMEINSECS, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_CONFIRMDELAY, db_get_w(0, AAAMODULENAME, SETTING_CONFIRMDELAY, 5), FALSE);
		CheckDlgButton(hwndDlg, bSettingSame ? IDC_SAMESETTINGS : IDC_PERPROTOCOLSETTINGS, BST_CHECKED);
		ShowWindow(GetDlgItem(hwndDlg, IDC_IDLEWARNING), db_get_b(0, "Idle", "AAEnable", 0));
		break;

	case WM_SHOWWINDOW:
		ShowWindow(GetDlgItem(hwndDlg, IDC_IDLEWARNING), (db_get_b(0, "Idle", "AAEnable", 0)));
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
			bSettingSame = IsDlgButtonChecked(hwndDlg, IDC_SAMESETTINGS);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			db_set_b(0, AAAMODULENAME, SETTING_IGNLOCK, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNLOCK));
			db_set_b(0, AAAMODULENAME, SETTING_IGNSYSKEYS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNSYSKEYS));
			db_set_b(0, AAAMODULENAME, SETTING_IGNALTCOMBO, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNALTCOMBO));
			db_set_b(0, AAAMODULENAME, SETTING_SAMESETTINGS, (BYTE)bSettingSame);
			db_set_w(0, AAAMODULENAME, SETTING_AWAYCHECKTIMEINSECS, (WORD)GetDlgItemInt(hwndDlg, IDC_AWAYCHECKTIMEINSECS, nullptr, FALSE));
			db_set_w(0, AAAMODULENAME, SETTING_CONFIRMDELAY, (WORD)GetDlgItemInt(hwndDlg, IDC_CONFIRMDELAY, nullptr, FALSE));
			db_set_b(0, AAAMODULENAME, SETTING_MONITORMOUSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE));
			db_set_b(0, AAAMODULENAME, SETTING_MONITORKEYBOARD, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD));
			AAALoadOptions(autoAwaySettings, FALSE);
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options initialization procedure

int AutoAwayOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 1000000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Status");
	odp.szTitle.a = LPGEN("Auto away");

	odp.szTab.a = LPGEN("General");
	odp.pfnDlgProc = DlgProcAutoAwayGeneralOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENAUTOAWAY);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Rules");
	odp.pfnDlgProc = DlgProcAutoAwayRulesOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTOAWAY);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Status messages");
	odp.pfnDlgProc = DlgProcAutoAwayMsgOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_AUTOAWAYMSG);
	Options_AddPage(wParam, &odp);
	return 0;
}
