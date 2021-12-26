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

/* Services */
static HWND hwndSettingsDlg;

const uint32_t unitValues[] = { 1,60,60 * 60,60 * 60 * 24,60 * 60 * 24 * 7,60 * 60 * 24 * 31 };
const wchar_t *unitNames[] = { LPGENW("Second(s)"), LPGENW("Minute(s)"), LPGENW("Hour(s)"), LPGENW("Day(s)"), LPGENW("Week(s)"), LPGENW("Month(s)") };

/************************* Dialog *************************************/

static void EnableDlgItem(HWND hwndDlg, int idCtrl, BOOL fEnable)
{
	hwndDlg = GetDlgItem(hwndDlg, idCtrl);
	if (hwndDlg != nullptr && IsWindowEnabled(hwndDlg) != fEnable)
		EnableWindow(hwndDlg, fEnable);
}

static BOOL CALLBACK DisplayCpuUsageProc(uint8_t nCpuUsage, LPARAM lParam)
{
	/* dialog closed? */
	if (!IsWindow((HWND)lParam))
		return FALSE; /* stop poll thread */

	wchar_t str[64];
	mir_snwprintf(str, TranslateT("(current: %u%%)"), nCpuUsage);
	SetWindowText((HWND)lParam, str);
	return TRUE;
}

static bool AnyProtoHasCaps(uint32_t caps1)
{
	for (auto &pa : Accounts())
		if (CallProtoService(pa->szModuleName, PS_GETCAPS, (WPARAM)PFLAGNUM_1, 0) & caps1)
			return true; /* CALLSERVICE_NOTFOUND also handled gracefully */

	return false;
}

#define M_ENABLE_SUBCTLS       (WM_APP+111)
#define M_UPDATE_SHUTDOWNDESC  (WM_APP+112)
#define M_CHECK_DATETIME       (WM_APP+113)
static INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			hwndSettingsDlg = hwndDlg;
			LCID locale = Langpack_GetDefaultLocale();
			SendDlgItemMessage(hwndDlg, IDC_ICON_HEADER, STM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIcon("AutoShutdown_Header"));
			{
				HFONT hBoldFont;
				LOGFONT lf;
				if (GetObject((HFONT)SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_GETFONT, 0, 0), sizeof(lf), &lf)) {
					lf.lfWeight = FW_BOLD;
					hBoldFont = CreateFontIndirect(&lf);
				}
				else hBoldFont = nullptr;
				SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_SETFONT, (WPARAM)hBoldFont, FALSE);
			}
			/* read-in watcher flags */
			{
				uint16_t watcherType = g_plugin.getWord("WatcherFlags", SETTING_WATCHERFLAGS_DEFAULT);
				CheckRadioButton(hwndDlg, IDC_RADIO_STTIME, IDC_RADIO_STCOUNTDOWN, (watcherType&SDWTF_ST_TIME) ? IDC_RADIO_STTIME : IDC_RADIO_STCOUNTDOWN);
				CheckDlgButton(hwndDlg, IDC_CHECK_SPECIFICTIME, (watcherType&SDWTF_SPECIFICTIME) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHECK_MESSAGE, (watcherType&SDWTF_MESSAGE) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHECK_FILETRANSFER, (watcherType&SDWTF_FILETRANSFER) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHECK_IDLE, (watcherType&SDWTF_IDLE) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHECK_STATUS, (watcherType&SDWTF_STATUS) != 0 ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_CHECK_CPUUSAGE, (watcherType&SDWTF_CPUUSAGE) != 0 ? BST_CHECKED : BST_UNCHECKED);
			}
			/* read-in countdown val */
			{
				SYSTEMTIME st;
				if (!TimeStampToSystemTime((time_t)g_plugin.getDword("TimeStamp", SETTING_TIMESTAMP_DEFAULT), &st))
					GetLocalTime(&st);
				DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_TIME_TIMESTAMP), GDT_VALID, &st);
				DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATE_TIMESTAMP), GDT_VALID, &st);
				SendMessage(hwndDlg, M_CHECK_DATETIME, 0, 0);
			}
			{
				uint32_t setting = g_plugin.getDword("Countdown", SETTING_COUNTDOWN_DEFAULT);
				if (setting < 1) setting = SETTING_COUNTDOWN_DEFAULT;
				SendDlgItemMessage(hwndDlg, IDC_SPIN_COUNTDOWN, UDM_SETRANGE, 0, MAKELPARAM(UD_MAXVAL, 1));
				SendDlgItemMessage(hwndDlg, IDC_EDIT_COUNTDOWN, EM_SETLIMITTEXT, (WPARAM)10, 0);
				SendDlgItemMessage(hwndDlg, IDC_SPIN_COUNTDOWN, UDM_SETPOS, 0, MAKELPARAM(setting, 0));
				SetDlgItemInt(hwndDlg, IDC_EDIT_COUNTDOWN, setting, FALSE);
			}
			{
				HWND hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO_COUNTDOWNUNIT);
				uint32_t lastUnit = g_plugin.getDword("CountdownUnit", SETTING_COUNTDOWNUNIT_DEFAULT);
				SendMessage(hwndCombo, CB_SETLOCALE, (WPARAM)locale, 0); /* sort order */
				SendMessage(hwndCombo, CB_INITSTORAGE, _countof(unitNames), _countof(unitNames) * 16); /* approx. */
				for (int i = 0; i < _countof(unitNames); ++i) {
					int index = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateW(unitNames[i]));
					if (index != LB_ERR) {
						SendMessage(hwndCombo, CB_SETITEMDATA, index, (LPARAM)unitValues[i]);
						if (i == 0 || unitValues[i] == lastUnit) SendMessage(hwndCombo, CB_SETCURSEL, index, 0);
					}
				}
			}
			{
				DBVARIANT dbv;
				if (!g_plugin.getWString("Message", &dbv)) {
					SetDlgItemText(hwndDlg, IDC_EDIT_MESSAGE, dbv.pwszVal);
					mir_free(dbv.pwszVal);
				}
			}
			/* cpuusage threshold */
			{
				uint8_t setting = DBGetContactSettingRangedByte(0, MODULENAME, "CpuUsageThreshold", SETTING_CPUUSAGETHRESHOLD_DEFAULT, 1, 100);
				SendDlgItemMessage(hwndDlg, IDC_SPIN_CPUUSAGE, UDM_SETRANGE, 0, MAKELPARAM(100, 1));
				SendDlgItemMessage(hwndDlg, IDC_EDIT_CPUUSAGE, EM_SETLIMITTEXT, (WPARAM)3, 0);
				SendDlgItemMessage(hwndDlg, IDC_SPIN_CPUUSAGE, UDM_SETPOS, 0, MAKELPARAM(setting, 0));
				SetDlgItemInt(hwndDlg, IDC_EDIT_CPUUSAGE, setting, FALSE);
			}
			/* shutdown types */
			{
				HWND hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO_SHUTDOWNTYPE);
				uint8_t lastShutdownType = g_plugin.getByte("ShutdownType", SETTING_SHUTDOWNTYPE_DEFAULT);
				SendMessage(hwndCombo, CB_SETLOCALE, (WPARAM)locale, 0); /* sort order */
				SendMessage(hwndCombo, CB_SETEXTENDEDUI, TRUE, 0);
				SendMessage(hwndCombo, CB_INITSTORAGE, SDSDT_MAX, SDSDT_MAX * 32);
				for (uint8_t shutdownType = 1; shutdownType <= SDSDT_MAX; ++shutdownType)
					if (ServiceIsTypeEnabled(shutdownType, 0)) {
						wchar_t *pszText = (wchar_t*)ServiceGetTypeDescription(shutdownType, GSTDF_TCHAR); /* never fails */
						int index = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)pszText);
						if (index != LB_ERR) {
							SendMessage(hwndCombo, CB_SETITEMDATA, index, (LPARAM)shutdownType);
							if (shutdownType == 1 || shutdownType == lastShutdownType) SendMessage(hwndCombo, CB_SETCURSEL, (WPARAM)index, 0);
						}
					}
				SendMessage(hwndDlg, M_UPDATE_SHUTDOWNDESC, 0, (LPARAM)hwndCombo);
			}
			/* check if proto is installed that supports instant messages and check if a message dialog plugin is installed */
			if (!AnyProtoHasCaps(PF1_IMRECV) || !ServiceExists(MS_MSG_SENDMESSAGE)) { /* no srmessage present? */
				CheckDlgButton(hwndDlg, IDC_CHECK_MESSAGE, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_MESSAGE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_MESSAGE), FALSE);
			}
			/* check if proto is installed that supports file transfers and check if a file transfer dialog is available */
			if (!AnyProtoHasCaps(PF1_FILESEND) && !AnyProtoHasCaps(PF1_FILERECV)) {  /* no srfile present? */
				CheckDlgButton(hwndDlg, IDC_CHECK_FILETRANSFER, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_FILETRANSFER), FALSE);
			}
			/* check if cpu usage can be detected */
			if (!PollCpuUsage(DisplayCpuUsageProc, (LPARAM)GetDlgItem(hwndDlg, IDC_TEXT_CURRENTCPU), 1800)) {
				CheckDlgButton(hwndDlg, IDC_CHECK_CPUUSAGE, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_CPUUSAGE), FALSE);
			}
		}
		SendMessage(hwndDlg, M_ENABLE_SUBCTLS, 0, 0);
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULENAME, "SettingsDlg_");
		return TRUE; /* default focus */

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULENAME, "SettingsDlg_");
		{
			HFONT hFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_GETFONT, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_TEXT_HEADER, WM_SETFONT, 0, FALSE); /* no return value */
			if (hFont != nullptr)
				DeleteObject(hFont);
			hwndSettingsDlg = nullptr;
		}
		return TRUE;

	case WM_CTLCOLORSTATIC:
		switch (GetDlgCtrlID((HWND)lParam)) {
		case IDC_ICON_HEADER:
			SetBkMode((HDC)wParam, TRANSPARENT);

		case IDC_RECT_HEADER:
			/* need to set COLOR_WINDOW manually for Win9x */
			SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
			return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

		case IDC_TEXT_HEADER:
		case IDC_TEXT_HEADERDESC:
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (INT_PTR)GetStockObject(NULL_BRUSH);
		}
		break;

	case M_ENABLE_SUBCTLS:
		{
			BOOL checked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_MESSAGE) != 0;
			EnableDlgItem(hwndDlg, IDC_EDIT_MESSAGE, checked);
			checked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SPECIFICTIME) != 0;
			EnableDlgItem(hwndDlg, IDC_RADIO_STTIME, checked);
			EnableDlgItem(hwndDlg, IDC_RADIO_STCOUNTDOWN, checked);
			checked = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SPECIFICTIME) && IsDlgButtonChecked(hwndDlg, IDC_RADIO_STTIME));
			EnableDlgItem(hwndDlg, IDC_TIME_TIMESTAMP, checked);
			EnableDlgItem(hwndDlg, IDC_DATE_TIMESTAMP, checked);
			checked = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SPECIFICTIME) && IsDlgButtonChecked(hwndDlg, IDC_RADIO_STCOUNTDOWN));
			EnableDlgItem(hwndDlg, IDC_EDIT_COUNTDOWN, checked);
			EnableDlgItem(hwndDlg, IDC_SPIN_COUNTDOWN, checked);
			EnableDlgItem(hwndDlg, IDC_COMBO_COUNTDOWNUNIT, checked);
			checked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_IDLE) != 0;
			EnableDlgItem(hwndDlg, IDC_URL_IDLE, checked);
			checked = IsDlgButtonChecked(hwndDlg, IDC_CHECK_CPUUSAGE) != 0;
			EnableDlgItem(hwndDlg, IDC_EDIT_CPUUSAGE, checked);
			EnableDlgItem(hwndDlg, IDC_SPIN_CPUUSAGE, checked);
			EnableDlgItem(hwndDlg, IDC_TEXT_PERCENT, checked);
			EnableDlgItem(hwndDlg, IDC_TEXT_CURRENTCPU, checked);
			checked = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SPECIFICTIME) || IsDlgButtonChecked(hwndDlg, IDC_CHECK_MESSAGE) ||
				IsDlgButtonChecked(hwndDlg, IDC_CHECK_IDLE) || IsDlgButtonChecked(hwndDlg, IDC_CHECK_STATUS) ||
				IsDlgButtonChecked(hwndDlg, IDC_CHECK_FILETRANSFER) || IsDlgButtonChecked(hwndDlg, IDC_CHECK_CPUUSAGE));
			EnableDlgItem(hwndDlg, IDOK, checked);
		}
		return TRUE;

	case M_UPDATE_SHUTDOWNDESC: /* lParam=(LPARAM)(HWND)hwndCombo */
		{
			uint8_t shutdownType = (uint8_t)SendMessage((HWND)lParam, CB_GETITEMDATA, SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0), 0);
			SetDlgItemText(hwndDlg, IDC_TEXT_SHUTDOWNTYPE, (wchar_t*)ServiceGetTypeDescription(shutdownType, GSTDF_LONGDESC | GSTDF_TCHAR));
		}
		return TRUE;

	case WM_TIMECHANGE: /* system time changed */
		SendMessage(hwndDlg, M_CHECK_DATETIME, 0, 0);
		return TRUE;

	case M_CHECK_DATETIME:
		{
			SYSTEMTIME st, stBuf;
			time_t timestamp;
			DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATE_TIMESTAMP), &stBuf);
			DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_TIME_TIMESTAMP), &st);
			st.wDay = stBuf.wDay;
			st.wDayOfWeek = stBuf.wDayOfWeek;
			st.wMonth = stBuf.wMonth;
			st.wYear = stBuf.wYear;
			GetLocalTime(&stBuf);
			if (SystemTimeToTimeStamp(&st, &timestamp)) {
				/* set to current date if earlier */
				if (timestamp < time(0)) {
					st.wDay = stBuf.wDay;
					st.wDayOfWeek = stBuf.wDayOfWeek;
					st.wMonth = stBuf.wMonth;
					st.wYear = stBuf.wYear;
					if (SystemTimeToTimeStamp(&st, &timestamp)) {
						/* step one day up if still earlier */
						if (timestamp < time(0)) {
							timestamp += 24 * 60 * 60;
							TimeStampToSystemTime(timestamp, &st);
						}
					}
				}
			}
			DateTime_SetRange(GetDlgItem(hwndDlg, IDC_DATE_TIMESTAMP), GDTR_MIN, &stBuf);
			DateTime_SetRange(GetDlgItem(hwndDlg, IDC_TIME_TIMESTAMP), GDTR_MIN, &stBuf);
			DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATE_TIMESTAMP), GDT_VALID, &st);
			DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_TIME_TIMESTAMP), GDT_VALID, &st);
		}
		return TRUE;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_TIME_TIMESTAMP:
		case IDC_DATE_TIMESTAMP:
			switch (((NMHDR*)lParam)->code) {
			case DTN_CLOSEUP:
			case NM_KILLFOCUS:
				PostMessage(hwndDlg, M_CHECK_DATETIME, 0, 0);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK_MESSAGE:
		case IDC_CHECK_FILETRANSFER:
		case IDC_CHECK_IDLE:
		case IDC_CHECK_CPUUSAGE:
		case IDC_CHECK_STATUS:
		case IDC_CHECK_SPECIFICTIME:
		case IDC_RADIO_STTIME:
		case IDC_RADIO_STCOUNTDOWN:
			SendMessage(hwndDlg, M_ENABLE_SUBCTLS, 0, 0);
			return TRUE;

		case IDC_EDIT_COUNTDOWN:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
				if ((int)GetDlgItemInt(hwndDlg, IDC_EDIT_COUNTDOWN, nullptr, TRUE) < 1) {
					SendDlgItemMessage(hwndDlg, IDC_SPIN_COUNTDOWN, UDM_SETPOS, 0, MAKELPARAM(1, 0));
					SetDlgItemInt(hwndDlg, IDC_EDIT_COUNTDOWN, 1, FALSE);
				}
				return TRUE;
			}
			break;

		case IDC_EDIT_CPUUSAGE:
			if (HIWORD(wParam) == EN_KILLFOCUS) {
				uint16_t val = (uint16_t)GetDlgItemInt(hwndDlg, IDC_EDIT_CPUUSAGE, nullptr, FALSE);
				if (val < 1) val = 1;
				else if (val>100) val = 100;
				SendDlgItemMessage(hwndDlg, IDC_SPIN_CPUUSAGE, UDM_SETPOS, 0, MAKELPARAM(val, 0));
				SetDlgItemInt(hwndDlg, IDC_EDIT_CPUUSAGE, val, FALSE);
				return TRUE;
			}
			break;

		case IDC_URL_IDLE:
			g_plugin.openOptions(L"Status", L"Idle");
			return TRUE;

		case IDC_COMBO_SHUTDOWNTYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				SendMessage(hwndDlg, M_UPDATE_SHUTDOWNDESC, 0, lParam);
			return TRUE;

		case IDOK: /* save settings and start watcher */
			ShowWindow(hwndDlg, SW_HIDE);
			/* message text */
			{
				HWND hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_MESSAGE);
				int len = GetWindowTextLength(hwndEdit) + 1;
				wchar_t *pszText = (wchar_t*)mir_alloc(len*sizeof(wchar_t));
				if (pszText != nullptr && GetWindowText(hwndEdit, pszText, len + 1)) {
					ltrimw(rtrimw(pszText));
					g_plugin.setWString("Message", pszText);
				}
				mir_free(pszText); /* does NULL check */
			}
			/* timestamp */
			{
				SYSTEMTIME st;
				time_t timestamp;
				DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_TIME_TIMESTAMP), &st); /* time gets synchronized */
				if (!SystemTimeToTimeStamp(&st, &timestamp))
					timestamp = time(0);
				g_plugin.setDword("TimeStamp", (uint32_t)timestamp);
			}
			/* shutdown type */
			{
				int index = SendDlgItemMessage(hwndDlg, IDC_COMBO_SHUTDOWNTYPE, CB_GETCURSEL, 0, 0);
				if (index != LB_ERR)
					g_plugin.setByte("ShutdownType", (uint8_t)SendDlgItemMessage(hwndDlg, IDC_COMBO_SHUTDOWNTYPE, CB_GETITEMDATA, (WPARAM)index, 0));
				index = SendDlgItemMessage(hwndDlg, IDC_COMBO_COUNTDOWNUNIT, CB_GETCURSEL, 0, 0);
				if (index != LB_ERR)
					g_plugin.setDword("CountdownUnit", (uint32_t)SendDlgItemMessage(hwndDlg, IDC_COMBO_COUNTDOWNUNIT, CB_GETITEMDATA, (WPARAM)index, 0));
				g_plugin.setDword("Countdown", (uint32_t)GetDlgItemInt(hwndDlg, IDC_EDIT_COUNTDOWN, nullptr, FALSE));
				g_plugin.setByte("CpuUsageThreshold", (uint8_t)GetDlgItemInt(hwndDlg, IDC_EDIT_CPUUSAGE, nullptr, FALSE));
			}
			/* watcher type */
			{
				uint16_t watcherType = (uint16_t)(IsDlgButtonChecked(hwndDlg, IDC_RADIO_STTIME) ? SDWTF_ST_TIME : SDWTF_ST_COUNTDOWN);
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_SPECIFICTIME)) watcherType |= SDWTF_SPECIFICTIME;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MESSAGE)) watcherType |= SDWTF_MESSAGE;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_FILETRANSFER)) watcherType |= SDWTF_FILETRANSFER;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_IDLE)) watcherType |= SDWTF_IDLE;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_STATUS)) watcherType |= SDWTF_STATUS;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_CPUUSAGE)) watcherType |= SDWTF_CPUUSAGE;
				g_plugin.setWord("WatcherFlags", watcherType);
				ServiceStartWatcher(0, watcherType);
			}
			DestroyWindow(hwndDlg);
			return TRUE;
			// fall through 

		case IDCANCEL: /* WM_CLOSE */
			DestroyWindow(hwndDlg);
			SetShutdownToolbarButton(false);
			SetShutdownMenuItem(false);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/************************* Services ***********************************/

static INT_PTR ServiceShowSettingsDialog(WPARAM, LPARAM)
{
	if (hwndSettingsDlg != nullptr) { /* already opened, bring to front */
		SetForegroundWindow(hwndSettingsDlg);
		return 0;
	}
	return CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SETTINGS), nullptr, SettingsDlgProc) == nullptr;
}

/************************* Toolbar ************************************/

static HANDLE hToolbarButton;

int ToolbarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.hIconHandleUp = iconList[2].hIcolib;
	ttb.hIconHandleDn = iconList[1].hIcolib;
	ttb.pszService = "AutoShutdown/MenuCommand";
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	ttb.name = LPGEN("Start/Stop automatic shutdown");
	ttb.pszTooltipUp = LPGEN("Start automatic shutdown");
	ttb.pszTooltipDn = LPGEN("Stop automatic shutdown");
	hToolbarButton = g_plugin.addTTB(&ttb);
	return 0;
}

void SetShutdownToolbarButton(bool fActive)
{
	if (hToolbarButton)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hToolbarButton, fActive ? TTBST_PUSHED : 0);
}

/************************* Menu Item **********************************/

static HGENMENU hMainMenuItem, hTrayMenuItem;

void SetShutdownMenuItem(bool fActive)
{
	/* main menu */
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x61e2a38f, 0xcd94, 0x4f72, 0x84, 0x8c, 0x72, 0x92, 0xde, 0x1d, 0x6d, 0x5);
	mi.position = 2001090000;
	if (fActive) {
		mi.hIcolibItem = iconList[1].hIcolib;
		mi.name.w = LPGENW("Stop automatic &shutdown");
	}
	else {
		mi.hIcolibItem = iconList[2].hIcolib;
		mi.name.w = LPGENW("Automatic &shutdown...");
	}
	mi.pszService = "AutoShutdown/MenuCommand";
	mi.flags = CMIF_UNICODE;
	if (hMainMenuItem != nullptr)
		Menu_ModifyItem(hMainMenuItem, mi.name.w, mi.hIcolibItem);
	else
		hMainMenuItem = Menu_AddMainMenuItem(&mi);

	/* tray menu */
	mi.position = 899999;
	if (hTrayMenuItem != nullptr)
		Menu_ModifyItem(hTrayMenuItem, mi.name.w, mi.hIcolibItem);
	else
		hTrayMenuItem = Menu_AddTrayMenuItem(&mi);
}

static INT_PTR MenuItemCommand(WPARAM, LPARAM)
{
	/* toggle between StopWatcher and ShowSettingsDdialog */
	if (ServiceIsWatcherEnabled(0, 0))
		ServiceStopWatcher(0, 0);
	else
		ServiceShowSettingsDialog(0, 0);
	return 0;
}

/************************* Misc ***************************************/

void InitSettingsDlg(void)
{
	/* Menu Item */
	CreateServiceFunction("AutoShutdown/MenuCommand", MenuItemCommand);
	hMainMenuItem = hTrayMenuItem = nullptr;
	SetShutdownMenuItem(false);
	/* Hotkey */
	AddHotkey();
	/* Services */
	hwndSettingsDlg = nullptr;
	CreateServiceFunction(MS_AUTOSHUTDOWN_SHOWSETTINGSDIALOG, ServiceShowSettingsDialog);
}
