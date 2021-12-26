/*

'AutoShutdown'-Plugin for Miranda IM

Copyright 2004-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Shutdown-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

/************************* Option Page ********************************/

#define M_ENABLE_SUBCTLS  (WM_APP+111)

static INT_PTR CALLBACK ShutdownOptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			uint16_t setting = g_plugin.getWord("ConfirmDlgCountdown", SETTING_CONFIRMDLGCOUNTDOWN_DEFAULT);
			if (setting < 3)
				setting = SETTING_CONFIRMDLGCOUNTDOWN_DEFAULT;
			SendDlgItemMessage(hwndDlg, IDC_SPIN_CONFIRMDLGCOUNTDOWN, UDM_SETRANGE, 0, MAKELPARAM(999, 3));
			SendDlgItemMessage(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN, EM_SETLIMITTEXT, 3, 0);
			SendDlgItemMessage(hwndDlg, IDC_SPIN_CONFIRMDLGCOUNTDOWN, UDM_SETPOS, 0, MAKELPARAM(setting, 0));
			SetDlgItemInt(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN, setting, FALSE);
		}
		CheckDlgButton(hwndDlg, IDC_CHECK_SMARTOFFLINECHECK, g_plugin.getByte("SmartOfflineCheck", SETTING_SMARTOFFLINECHECK_DEFAULT) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_REMEMBERONRESTART, g_plugin.getByte("RememberOnRestart", SETTING_REMEMBERONRESTART_DEFAULT) != 0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_SHOWCONFIRMDLG, g_plugin.getByte("ShowConfirmDlg", SETTING_SHOWCONFIRMDLG_DEFAULT) != 0 ? BST_CHECKED : BST_UNCHECKED);
		{
			BOOL enabled = ServiceIsTypeEnabled(SDSDT_SHUTDOWN, 0);
			if (enabled) {
				if (ServiceExists(MS_WEATHER_UPDATE)) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_WEATHER), TRUE);
					CheckDlgButton(hwndDlg, IDC_CHECK_WEATHER, g_plugin.getByte("WeatherShutdown", SETTING_WEATHERSHUTDOWN_DEFAULT) != 0 ? BST_CHECKED : BST_UNCHECKED);
				}
			}
		}
		SendMessage(hwndDlg, M_ENABLE_SUBCTLS, 0, 0);
		return TRUE; /* default focus */

	case M_ENABLE_SUBCTLS:
		{
			BOOL checked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHOWCONFIRMDLG) != 0;
			if (checked != IsWindowEnabled(GetDlgItem(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN))) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN), checked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SPIN_CONFIRMDLGCOUNTDOWN), checked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT_COUNTDOWNSTARTS), checked);
				EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT_SECONDS), checked);
			}
			return TRUE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDIT_CONFIRMDLGCOUNTDOWN:
			if (HIWORD(wParam) == EN_KILLFOCUS)
				if ((int)GetDlgItemInt(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN, nullptr, TRUE) < 3) {
					SendDlgItemMessage(hwndDlg, IDC_SPIN_CONFIRMDLGCOUNTDOWN, UDM_SETPOS, 0, MAKELPARAM(3, 0));
					SetDlgItemInt(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN, 3, FALSE);
				}
			if (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE; /* no apply */
			break;
		}
		PostMessage(hwndDlg, M_ENABLE_SUBCTLS, 0, 0);
		PostMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0); /* enable apply */
		return FALSE;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code) {
		case PSN_APPLY:
			g_plugin.setByte("ShowConfirmDlg", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHOWCONFIRMDLG) != 0));
			g_plugin.setWord("ConfirmDlgCountdown", (uint16_t)GetDlgItemInt(hwndDlg, IDC_EDIT_CONFIRMDLGCOUNTDOWN, nullptr, FALSE));
			g_plugin.setByte("RememberOnRestart", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_CHECK_REMEMBERONRESTART) != 0));
			g_plugin.setByte("SmartOfflineCheck", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_CHECK_SMARTOFFLINECHECK) != 0));
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHECK_WEATHER)))
				g_plugin.setByte("WeatherShutdown", (uint8_t)(IsDlgButtonChecked(hwndDlg, IDC_CHECK_WEATHER) != 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SHUTDOWN);
	odp.position = 900000002;
	odp.szGroup.w = LPGENW("Events"); /* autotranslated */
	odp.szTitle.w = LPGENW("Automatic shutdown"); /* autotranslated */
	odp.szTab.w = LPGENW("Automatic shutdown");  /* autotranslated, can be made a tab */
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pfnDlgProc = ShutdownOptDlgProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
