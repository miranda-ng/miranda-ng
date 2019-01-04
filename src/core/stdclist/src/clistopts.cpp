/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

static INT_PTR CALLBACK DlgProcGenOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_USER + 1:
		{
			MCONTACT hContact = wParam;
			DBCONTACTWRITESETTING *ws = (DBCONTACTWRITESETTING *)lParam;
			if (hContact == NULL && ws != nullptr && ws->szModule != nullptr && ws->szSetting != nullptr &&
				strcmp(ws->szModule, "CList") == 0 && strcmp(ws->szSetting, "UseGroups") == 0 && IsWindowVisible(hwndDlg))
			{
				CheckDlgButton(hwndDlg, IDC_DISABLEGROUPS, ws->value.bVal == 0 ? BST_CHECKED : BST_UNCHECKED);
			}
		}
		break;

	case WM_DESTROY:
		UnhookEvent((HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
		break;

	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)HookEventMessage(ME_DB_CONTACT_SETTINGCHANGED, hwndDlg, WM_USER + 1));
		CheckDlgButton(hwndDlg, IDC_HIDEOFFLINE,
			g_plugin.getByte("HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HIDEEMPTYGROUPS,
			g_plugin.getByte("HideEmptyGroups", SETTING_HIDEEMPTYGROUPS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISABLEGROUPS,
			g_plugin.getByte("UseGroups", SETTING_USEGROUPS_DEFAULT) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_SORTBYNAME, !g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT)
			&& !g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SORTBYSTATUS,
			g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SORTBYPROTO,
			g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CONFIRMDELETE,
			g_plugin.getByte("ConfirmDelete", SETTING_CONFIRMDELETE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_ONECLK,
			g_plugin.getByte("Tray1Click", SETTING_TRAY1CLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYSSTATUS,
			g_plugin.getByte("AlwaysStatus", SETTING_ALWAYSSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ALWAYSMULTI,
			!g_plugin.getByte("AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DONTCYCLE,
			g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_SINGLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CYCLE,
			g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_CYCLE ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_MULTITRAY,
			g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DISABLEBLINK,
			g_plugin.getByte("DisableTrayFlash", 0) == 1 ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKTIME), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		EnableWindow(GetDlgItem(hwndDlg, IDC_STMSDELAY), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		CheckDlgButton(hwndDlg, IDC_ICONBLINK, g_plugin.getByte("NoIconBlink", 0) == 1 ? BST_CHECKED : BST_UNCHECKED);
		if (IsDlgButtonChecked(hwndDlg, IDC_DONTCYCLE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIMESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSMULTI), FALSE);
		}
		if (IsDlgButtonChecked(hwndDlg, IDC_CYCLE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSMULTI), FALSE);
		}
		if (IsDlgButtonChecked(hwndDlg, IDC_MULTITRAY)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIMESPIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS), FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETRANGE, 0, MAKELONG(120, 1));
		SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("CycleTime", SETTING_CYCLETIME_DEFAULT), 0));
		{
			ptrA szPrimaryStatus(g_plugin.getStringA("PrimaryStatus"));

			int item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Global"));
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM)0);

			for (auto &pa : Accounts()) {
				if (!pa->IsEnabled() || CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
					continue;

				item = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM)pa->tszAccountName);
				SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM)pa);
				if (!mir_strcmp(szPrimaryStatus, pa->szModuleName))
					SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETCURSEL, item, 0);
			}
		}
		if (CB_ERR == SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0))
			SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(hwndDlg, IDC_BLINKTIME), 0);   // set buddy
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETRANGE, 0, MAKELONG(0x3FFF, 250));
		SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("IconFlashTime", 550), 0));
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_DONTCYCLE || LOWORD(wParam) == IDC_CYCLE || LOWORD(wParam) == IDC_MULTITRAY) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_PRIMARYSTATUS), IsDlgButtonChecked(hwndDlg, IDC_DONTCYCLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIME), IsDlgButtonChecked(hwndDlg, IDC_CYCLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLETIMESPIN), IsDlgButtonChecked(hwndDlg, IDC_CYCLE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_ALWAYSMULTI), IsDlgButtonChecked(hwndDlg, IDC_MULTITRAY));
		}
		if (LOWORD(wParam) == IDC_DISABLEBLINK) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKTIME), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
			EnableWindow(GetDlgItem(hwndDlg, IDC_BLINKSPIN), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
			EnableWindow(GetDlgItem(hwndDlg, IDC_STMSDELAY), BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
		}
		if (LOWORD(wParam) == IDC_CYCLETIME && HIWORD(wParam) != EN_CHANGE)
			break;
		if (LOWORD(wParam) == IDC_PRIMARYSTATUS && HIWORD(wParam) != CBN_SELCHANGE)
			break;
		if (LOWORD(wParam) == IDC_CYCLETIME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		if (LOWORD(wParam) == IDC_BLINKTIME && HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
			return 0;       // dont make apply enabled during buddy set crap
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				g_plugin.setByte("HideOffline", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDEOFFLINE));
				g_plugin.setByte("HideEmptyGroups", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_HIDEEMPTYGROUPS));
				g_plugin.setByte("UseGroups", (BYTE)!IsDlgButtonChecked(hwndDlg, IDC_DISABLEGROUPS));

				g_bSortByStatus = IsDlgButtonChecked(hwndDlg, IDC_SORTBYSTATUS);
				g_bSortByProto = IsDlgButtonChecked(hwndDlg, IDC_SORTBYPROTO);

				g_plugin.setByte("SortByStatus", (BYTE)g_bSortByStatus);
				g_plugin.setByte("SortByProto", (BYTE)g_bSortByProto);
				g_plugin.setByte("ConfirmDelete", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CONFIRMDELETE));
				g_plugin.setByte("Tray1Click", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ONECLK));
				g_plugin.setByte("AlwaysStatus", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ALWAYSSTATUS));
				g_plugin.setByte("AlwaysMulti", (BYTE)!IsDlgButtonChecked(hwndDlg, IDC_ALWAYSMULTI));
				g_plugin.setByte("TrayIcon",
					(BYTE)(IsDlgButtonChecked(hwndDlg, IDC_DONTCYCLE) ? SETTING_TRAYICON_SINGLE
						: (IsDlgButtonChecked(hwndDlg, IDC_CYCLE) ? SETTING_TRAYICON_CYCLE : SETTING_TRAYICON_MULTI)));
				g_plugin.setWord("CycleTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_CYCLETIMESPIN, UDM_GETPOS, 0, 0));
				g_plugin.setWord("IconFlashTime", (WORD)SendDlgItemMessage(hwndDlg, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
				g_plugin.setByte("DisableTrayFlash", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DISABLEBLINK));
				g_plugin.setByte("NoIconBlink", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ICONBLINK));
				{
					int cur = SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0);
					PROTOACCOUNT *pa = (PROTOACCOUNT*)SendDlgItemMessage(hwndDlg, IDC_PRIMARYSTATUS, CB_GETITEMDATA, cur, 0);
					if (pa == nullptr)
						g_plugin.delSetting("PrimaryStatus");
					else
						g_plugin.setString("PrimaryStatus", pa->szModuleName);
				}

				Clist_TrayIconIconsChanged();
				Clist_LoadContactTree();  /* this won't do job properly since it only really works when changes happen */
				g_clistApi.pfnInvalidateDisplayNameCacheEntry(INVALID_CONTACT_ID);        /* force reshuffle */
				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;
}

/****************************************************************************************/

int CListOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST);
	odp.szTitle.a = LPGEN("Contact list");
	odp.pfnDlgProc = DlgProcGenOpts;
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
