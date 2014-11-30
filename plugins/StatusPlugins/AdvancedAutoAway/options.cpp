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

#include "../commonstatus.h"
#include "advancedautoaway.h"
#include "../resource.h"
#include <commctrl.h>

int LoadAutoAwaySetting(TAAAProtoSetting &autoAwaySetting, char* protoName);

INT_PTR CALLBACK DlgProcAutoAwayMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static BOOL bSettingSame = FALSE;

/////////////////////////////////////////////////////////////////////////////////////////
// Dialog service functions

static int WriteAutoAwaySetting(TAAAProtoSetting &autoAwaySetting, char *protoName)
{
	char setting[128];
	mir_snprintf(setting, sizeof(setting), "%s_OptionFlags", protoName);
	db_set_w(NULL, MODULENAME, setting, (WORD)autoAwaySetting.optionFlags);
	mir_snprintf(setting, sizeof(setting), "%s_AwayTime", protoName);
	db_set_w(NULL, MODULENAME, setting, (WORD)autoAwaySetting.awayTime);
	mir_snprintf(setting, sizeof(setting), "%s_NATime", protoName);
	db_set_w(NULL, MODULENAME, setting, (WORD)autoAwaySetting.naTime);
	mir_snprintf(setting, sizeof(setting), "%s_StatusFlags", protoName);
	db_set_w(NULL, MODULENAME, setting, (WORD)autoAwaySetting.statusFlags);
	mir_snprintf(setting, sizeof(setting), "%s_Lv1Status", protoName);
	db_set_w(NULL, MODULENAME, setting, (WORD)autoAwaySetting.lv1Status);
	mir_snprintf(setting, sizeof(setting), "%s_Lv2Status", protoName);
	db_set_w(NULL, MODULENAME, setting, (WORD)autoAwaySetting.lv2Status);

	return 0;
}

static void SetDialogItems(HWND hwndDlg, TAAAProtoSetting *setting)
{
	bool bIsTimed = (setting->optionFlags & FLAG_ONMOUSE) != 0;
	bool bSetNA = (setting->optionFlags & FLAG_SETNA) != 0;
	bool bSaver = (setting->optionFlags & FLAG_ONSAVER) != 0;
	bool bFullScr = (setting->optionFlags & FLAG_FULLSCREEN) != 0;

	CheckDlgButton(hwndDlg, IDC_FULLSCREEN, bFullScr);
	CheckDlgButton(hwndDlg, IDC_SCREENSAVE, bSaver);
	CheckDlgButton(hwndDlg, IDC_ONLOCK, (setting->optionFlags & FLAG_ONLOCK) != 0);
	CheckDlgButton(hwndDlg, IDC_TIMED, bIsTimed);
	CheckDlgButton(hwndDlg, IDC_SETNA, bSetNA);
	CheckDlgButton(hwndDlg, IDC_CONFIRM, (setting->optionFlags & FLAG_CONFIRM) != 0);
	CheckDlgButton(hwndDlg, IDC_RESETSTATUS, (setting->optionFlags & FLAG_RESET) != 0);
	CheckDlgButton(hwndDlg, IDC_MONITORMIRANDA, (setting->optionFlags & FLAG_MONITORMIRANDA) != 0);
	CheckDlgButton(hwndDlg, IDC_LV2ONINACTIVE, (setting->optionFlags & FLAG_LV2ONINACTIVE) != 0);

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
		return NULL;

	INT_PTR iData = (INT_PTR)SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_GETITEMDATA, iItem, 0);
	return (iData == -1) ? NULL : (TAAAProtoSetting*)iData;
}

static void SetDialogStatus(HWND hwndDlg, TAAAProtoSetting *sameSetting)
{
	TAAAProtoSetting *setting = GetSetting(hwndDlg, sameSetting);
	if (setting == NULL)
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

static TAAAProtoSettingList optionSettings(10, CompareSettings);

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
				int item = SendDlgItemMessage(hwndDlg, IDC_PROTOCOL, CB_ADDSTRING, 0, (LPARAM)p.tszAccName);
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
					flags = CallProtoService(setting->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(setting->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);

				LVITEM lvItem = { 0 };
				lvItem.mask = LVIF_TEXT | LVIF_PARAM;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				for (int i = 0; i < SIZEOF(statusModeList); i++) {
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
					flags = CallProtoService(setting->szName, PS_GETCAPS, PFLAGNUM_2, 0)&~CallProtoService(setting->szName, PS_GETCAPS, (WPARAM)PFLAGNUM_5, 0);

				// clear box and add new status, loop status and check if compatible with proto
				SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_RESETCONTENT, 0, 0);
				for (int i=0; i < SIZEOF(statusModeList); i++) {
					if ((flags & statusModePf2List[i]) || statusModePf2List[i] == PF2_OFFLINE || bSettingSame) {
						TCHAR *statusMode = pcli->pfnGetStatusModeDescription(statusModeList[i], 0);
						int item = SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_SETITEMDATA, item, (LPARAM)statusModeList[i]);
						item = SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_ADDSTRING, 0, (LPARAM)statusMode);
						SendDlgItemMessage(hwndDlg, IDC_LV2STATUS, CB_SETITEMDATA, item, (LPARAM)statusModeList[i]);
						if (statusModeList[i] == setting->lv1Status) {
							SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_SETCURSEL, (WPARAM)item, 0);
							SetDlgItemText(hwndDlg, IDC_SETNASTR, CMString(FORMAT, TranslateT("minutes of %s mode"), statusMode));
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
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,0,0);

			setting->lv1Status = (int)SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_LV1STATUS, CB_GETCURSEL, 0, 0), 0);
			SetDlgItemText(hwndDlg, IDC_SETNASTR, CMString(FORMAT, TranslateT("minutes of %s mode"), pcli->pfnGetStatusModeDescription(setting->lv1Status, 0)));
			break;

		case IDC_LV2STATUS:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(GetParent(hwndDlg),PSM_CHANGED,0,0);

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
			setting->awayTime = GetDlgItemInt(hwndDlg,IDC_AWAYTIME, NULL, FALSE);
			break;

		case IDC_NATIME:
			setting->naTime = GetDlgItemInt(hwndDlg,IDC_NATIME, NULL, FALSE);
			break;

		case IDC_LV2ONINACTIVE:
			setting->optionFlags^=FLAG_LV2ONINACTIVE;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_CONFIRM:
			setting->optionFlags^=FLAG_CONFIRM;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_RESETSTATUS:
			setting->optionFlags^=FLAG_RESET;
			SetDialogItems(hwndDlg, setting);
			break;

		case IDC_MONITORMIRANDA:
			setting->optionFlags^=FLAG_MONITORMIRANDA;
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
		switch(((NMHDR*)lParam)->idFrom) {
		case IDC_STATUSLIST:
			if (init)
				break;

			switch(((NMHDR*)lParam)->code) {
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
				for (int i=0; i < optionSettings.getCount(); i++ )
					WriteAutoAwaySetting(optionSettings[i], optionSettings[i].szName);
			}
			LoadOptions(autoAwaySettings, FALSE);
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
		CheckDlgButton(hwndDlg, IDC_IGNLOCK, db_get_b(NULL, MODULENAME, SETTING_IGNLOCK, FALSE) != 0);
		CheckDlgButton(hwndDlg, IDC_IGNSYSKEYS, db_get_b(NULL, MODULENAME, SETTING_IGNSYSKEYS, FALSE) != 0);
		CheckDlgButton(hwndDlg, IDC_IGNALTCOMBO, db_get_b(NULL, MODULENAME, SETTING_IGNALTCOMBO, FALSE) != 0);
		CheckDlgButton(hwndDlg, IDC_MONITORMOUSE, db_get_b(NULL, MODULENAME, SETTING_MONITORMOUSE, TRUE) != 0);
		CheckDlgButton(hwndDlg, IDC_MONITORKEYBOARD, db_get_b(NULL, MODULENAME, SETTING_MONITORKEYBOARD, TRUE) != 0);
		SetDlgItemInt(hwndDlg, IDC_AWAYCHECKTIMEINSECS, db_get_w(NULL, MODULENAME, SETTING_AWAYCHECKTIMEINSECS, 5), FALSE);
		SetDlgItemInt(hwndDlg, IDC_CONFIRMDELAY, db_get_w(NULL, MODULENAME, SETTING_CONFIRMDELAY, 5), FALSE);
		CheckDlgButton(hwndDlg, bSettingSame ? IDC_SAMESETTINGS : IDC_PERPROTOCOLSETTINGS, BST_CHECKED);
		ShowWindow(GetDlgItem(hwndDlg, IDC_IDLEWARNING), db_get_b(NULL, "Idle", "AAEnable", 0));
		break;

	case WM_SHOWWINDOW:
		ShowWindow(GetDlgItem(hwndDlg, IDC_IDLEWARNING), (db_get_b(NULL, "Idle", "AAEnable", 0)));
		break;
	
	case WM_COMMAND:
		if (( HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED ) && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch(LOWORD(wParam)) {
		case IDC_MONITORMOUSE:
			CheckDlgButton(hwndDlg, IDC_MONITORMOUSE, (((!IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE))&&(!IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD)))||(IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE))?BST_CHECKED:BST_UNCHECKED));
			break;

		case IDC_MONITORKEYBOARD:
			CheckDlgButton(hwndDlg, IDC_MONITORKEYBOARD, (((!IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE))&&(!IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD)))||(IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD))?BST_CHECKED:BST_UNCHECKED));
			break;

		case IDC_SAMESETTINGS:
		case IDC_PERPROTOCOLSETTINGS:
			bSettingSame = IsDlgButtonChecked(hwndDlg, IDC_SAMESETTINGS);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			db_set_b(NULL, MODULENAME, SETTING_IGNLOCK, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNLOCK));
			db_set_b(NULL, MODULENAME, SETTING_IGNSYSKEYS, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNSYSKEYS));
			db_set_b(NULL, MODULENAME, SETTING_IGNALTCOMBO, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_IGNALTCOMBO));
			db_set_b(NULL, MODULENAME, SETTING_SAMESETTINGS, (BYTE)bSettingSame);
			db_set_w(NULL, MODULENAME, SETTING_AWAYCHECKTIMEINSECS, (WORD)GetDlgItemInt(hwndDlg, IDC_AWAYCHECKTIMEINSECS, NULL, FALSE));
			db_set_w(NULL, MODULENAME, SETTING_CONFIRMDELAY, (WORD)GetDlgItemInt(hwndDlg, IDC_CONFIRMDELAY, NULL, FALSE));
			db_set_b(NULL, MODULENAME, SETTING_MONITORMOUSE, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MONITORMOUSE));
			db_set_b(NULL, MODULENAME, SETTING_MONITORKEYBOARD, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_MONITORKEYBOARD));
			LoadOptions(autoAwaySettings, FALSE);
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tab window procedure

static INT_PTR CALLBACK DlgProcAutoAwayTabs(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			RECT rcTabs, rcOptions, rcPage;
			bSettingSame = db_get_b(NULL, MODULENAME, SETTING_SAMESETTINGS, FALSE);

			// set tabs
			int tabCount = 0;
			HWND hTab = GetDlgItem(hwndDlg, IDC_TABS);
			GetWindowRect(hTab, &rcTabs);
			GetWindowRect(hwndDlg, &rcOptions);

			// general tab
			TCITEM tci = { 0 };
			tci.mask = TCIF_TEXT | TCIF_PARAM;
			tci.pszText = TranslateT("General");
			HWND hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OPT_GENAUTOAWAY), hwndDlg, DlgProcAutoAwayGeneralOpts, (LPARAM)GetParent(hwndDlg));
			EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)hPage;
			GetClientRect(hPage, &rcPage);
			MoveWindow(hPage, (rcTabs.left - rcOptions.left) + ((rcTabs.right-rcTabs.left)-(rcPage.right-rcPage.left))/2, 10 + (rcTabs.top - rcOptions.top) + ((rcTabs.bottom-rcTabs.top)-(rcPage.bottom-rcPage.top))/2, rcPage.right-rcPage.left, rcPage.bottom-rcPage.top, TRUE);
			ShowWindow(hPage, SW_HIDE);
			TabCtrl_InsertItem(hTab, tabCount++, &tci);
			HWND hShow = hPage;

			// rules tab
			tci.mask = TCIF_TEXT|TCIF_PARAM;
			tci.pszText = TranslateT("Rules");
			hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OPT_AUTOAWAY), hwndDlg, DlgProcAutoAwayRulesOpts, (LPARAM)GetParent(hwndDlg));
			EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)hPage;
			GetClientRect(hPage, &rcPage);
			MoveWindow(hPage, (rcTabs.left - rcOptions.left) + ((rcTabs.right-rcTabs.left)-(rcPage.right-rcPage.left))/2, 10 + (rcTabs.top - rcOptions.top) + ((rcTabs.bottom-rcTabs.top)-(rcPage.bottom-rcPage.top))/2, rcPage.right-rcPage.left, rcPage.bottom-rcPage.top, TRUE);
			ShowWindow(hPage, SW_HIDE);
			TabCtrl_InsertItem(hTab, tabCount++, &tci);

			// messages tab
			tci.mask = TCIF_TEXT|TCIF_PARAM;
			tci.pszText = TranslateT("Status messages");
			hPage = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_OPT_AUTOAWAYMSG), hwndDlg, DlgProcAutoAwayMsgOpts, (LPARAM)GetParent(hwndDlg));
			EnableThemeDialogTexture(hPage, ETDT_ENABLETAB);

			tci.lParam = (LPARAM)hPage;
			GetClientRect(hPage, &rcPage);
			MoveWindow(hPage, (rcTabs.left - rcOptions.left) + ((rcTabs.right-rcTabs.left)-(rcPage.right-rcPage.left))/2, 10 + (rcTabs.top - rcOptions.top) + ((rcTabs.bottom-rcTabs.top)-(rcPage.bottom-rcPage.top))/2, rcPage.right-rcPage.left, rcPage.bottom-rcPage.top, TRUE);
			ShowWindow(hPage, SW_HIDE);
			TabCtrl_InsertItem(hTab, tabCount++, &tci);

			ShowWindow(hShow, SW_SHOW);			
		}
		break;

	case PSM_CHANGED:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if ((((NMHDR*)lParam)->idFrom == IDC_TABS)) {
			if (((NMHDR*)lParam)->code == TCN_SELCHANGING) {
				TCITEM tci;

				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TABS), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS)), &tci);
				ShowWindow((HWND)tci.lParam, SW_HIDE);
			}
			else if (((NMHDR*)lParam)->code == TCN_SELCHANGE) {
				TCITEM tci;

				tci.mask = TCIF_PARAM;
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TABS), TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_TABS)), &tci);
				ShowWindow((HWND)tci.lParam, SW_SHOW);
			}
		}
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			TCITEM tci;
			tci.mask = TCIF_PARAM;
			int count = TabCtrl_GetItemCount(GetDlgItem(hwndDlg, IDC_TABS));
			for (int i = 0; i < count; i++) {
				TabCtrl_GetItem(GetDlgItem(hwndDlg, IDC_TABS), i, &tci);
				SendMessage((HWND)tci.lParam, WM_NOTIFY, 0, lParam);
			}
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Options initialization procedure

int AutoAwayOptInitialise(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 1000000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TABS);
	odp.pszTitle = LPGEN("Auto Away");
	odp.pszGroup = LPGEN("Status");
	odp.pfnDlgProc = DlgProcAutoAwayTabs;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
