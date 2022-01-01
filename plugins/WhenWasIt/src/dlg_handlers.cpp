/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean
Copyright (C) 2014-22 Rozhuk Ivan

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

#define COLOR_USERINFO RGB(138, 190, 160)
#define COLOR_MBIRTHDAY RGB(222, 222, 88)
#define COLOR_BIRTHDAYREMINDER RGB(200, 120, 240)
#define COLOR_PROTOCOL RGB(255, 153, 153)
#define COLOR_MICQBIRTHDAY RGB(88, 88, 240)

#define UPCOMING_TIMER_ID 1002

const wchar_t *szShowAgeMode[] = { LPGENW("Upcoming age"), LPGENW("Current age") };
const int cShowAgeMode = sizeof(szShowAgeMode) / sizeof(szShowAgeMode[0]);

const wchar_t *szSaveModule[] = { LPGENW("UserInfo module"), LPGENW("Protocol module"), LPGENW("mBirthday module") };
const int cSaveModule = sizeof(szSaveModule) / sizeof(szSaveModule[0]);

const wchar_t *szPopupClick[] = { LPGENW("Nothing"), LPGENW("Dismiss"), LPGENW("Message window") };
const int cPopupClick = sizeof(szPopupClick) / sizeof(szPopupClick[0]);

const wchar_t *szNotifyFor[] = { LPGENW("All contacts"), LPGENW("All contacts except hidden ones"), LPGENW("All contacts except ignored ones"), LPGENW("All contacts except hidden and ignored ones") };
const int cNotifyFor = sizeof(szNotifyFor) / sizeof(szNotifyFor[0]);

#define MIN_BIRTHDAYS_WIDTH 200
#define MIN_BIRTHDAYS_HEIGHT 200

#include "commctrl.h"
void CreateToolTip(HWND target, wchar_t* tooltip, LPARAM width)
{
	HWND hwndToolTip = CreateWindow(TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, target, nullptr, nullptr, nullptr);
	if (hwndToolTip) {
		TOOLINFO ti = { 0 };
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_TRANSPARENT | TTF_SUBCLASS;
		ti.hwnd = target;
		ti.uId = 0;
		ti.hinst = nullptr;
		ti.lpszText = tooltip;
		GetClientRect(target, &ti.rect);
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
		SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, width);
		SendMessage(hwndToolTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);
	}
}

int EnablePopupsGroup(HWND hWnd, int enable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_POPUPS_STATIC), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_POPUP_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_FOREGROUND), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_BACKGROUND), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_NOBIRTHDAYS_POPUP), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_IGNORE_SUBCONTACTS), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_PREVIEW), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_LEFT_CLICK), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_RIGHT_CLICK), enable);

	return enable;
}

int EnableClistGroup(HWND hWnd, int enable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_CLIST_STATIC), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_ADVANCED_ICON), enable);
	return enable;
}

int EnableDialogGroup(HWND hWnd, int enable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_DLG_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hWnd, IDC_OPENINBACKGROUND), enable);

	return enable;
}

int AddInfoToComboBoxes(HWND hWnd)
{
	int i;

	for (i = 0; i < cShowAgeMode; i++)
		SendDlgItemMessage(hWnd, IDC_AGE_COMBOBOX, CB_ADDSTRING, 0, (LPARAM)TranslateW(szShowAgeMode[i]));

	for (i = 0; i < cSaveModule; i++)
		SendDlgItemMessage(hWnd, IDC_DEFAULT_MODULE, CB_ADDSTRING, 0, (LPARAM)TranslateW(szSaveModule[i]));

	for (i = 0; i < cPopupClick; i++) {
		SendDlgItemMessage(hWnd, IDC_LEFT_CLICK, CB_ADDSTRING, 0, (LPARAM)TranslateW(szPopupClick[i]));
		SendDlgItemMessage(hWnd, IDC_RIGHT_CLICK, CB_ADDSTRING, 0, (LPARAM)TranslateW(szPopupClick[i]));
	}

	for (i = 0; i < cNotifyFor; i++)
		SendDlgItemMessage(hWnd, IDC_NOTIFYFOR, CB_ADDSTRING, 0, (LPARAM)TranslateW(szNotifyFor[i]));

	return i;
}

SIZE GetControlTextSize(HWND hCtrl)
{
	HDC hDC = GetDC(hCtrl);
	HFONT font = (HFONT)SendMessage(hCtrl, WM_GETFONT, 0, 0);
	HFONT oldFont = (HFONT)SelectObject(hDC, font);
	const size_t maxSize = 2048;
	wchar_t buffer[maxSize];
	SIZE size;
	GetWindowText(hCtrl, buffer, _countof(buffer));
	GetTextExtentPoint32(hDC, buffer, (int)mir_wstrlen(buffer), &size);
	SelectObject(hDC, oldFont);
	ReleaseDC(hCtrl, hDC);
	return size;
}

int EnlargeControl(HWND hCtrl, HWND, SIZE oldSize)
{
	SIZE size = GetControlTextSize(hCtrl);
	RECT rect;
	GetWindowRect(hCtrl, &rect);
	int offset = (rect.right - rect.left) - oldSize.cx;
	SetWindowPos(hCtrl, HWND_TOP, 0, 0, size.cx + offset, oldSize.cy, SWP_NOMOVE);
	SetWindowPos(hCtrl, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return 0;
}

wchar_t *strtrim(wchar_t *str)
{
	size_t i = 0;
	size_t len = mir_wstrlen(str);
	while ((i < len) && (str[i] == ' ')) { i++; }
	if (i) {
		memmove(str, str + i, len - i + 1);
		len -= i;
	}

	while ((len > 0) && (str[--len] == ' '))
		str[len] = 0;

	return str;
}

INT_PTR CALLBACK DlgProcOptions(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	switch (msg) {
	case WM_INITDIALOG:
		{
			bInitializing = 1;

			SIZE oldPopupsSize = GetControlTextSize(GetDlgItem(hWnd, IDC_USE_POPUPS));
			SIZE oldClistIconSize = GetControlTextSize(GetDlgItem(hWnd, IDC_USE_CLISTICON));
			SIZE oldDialogSize = GetControlTextSize(GetDlgItem(hWnd, IDC_USE_DIALOG));

			TranslateDialogDefault(hWnd);

			EnlargeControl(GetDlgItem(hWnd, IDC_USE_POPUPS), GetDlgItem(hWnd, IDC_POPUPS_STATIC), oldPopupsSize);
			EnlargeControl(GetDlgItem(hWnd, IDC_USE_CLISTICON), GetDlgItem(hWnd, IDC_CLIST_STATIC), oldClistIconSize);
			EnlargeControl(GetDlgItem(hWnd, IDC_USE_DIALOG), GetDlgItem(hWnd, IDC_DIALOG_STATIC), oldDialogSize);

			AddInfoToComboBoxes(hWnd);

			SendDlgItemMessage(hWnd, IDC_FOREGROUND, CPM_SETDEFAULTCOLOUR, 0, FOREGROUND_COLOR);
			SendDlgItemMessage(hWnd, IDC_BACKGROUND, CPM_SETDEFAULTCOLOUR, 0, BACKGROUND_COLOR);

			SendDlgItemMessage(hWnd, IDC_FOREGROUND, CPM_SETCOLOUR, 0, commonData.foreground);
			SendDlgItemMessage(hWnd, IDC_BACKGROUND, CPM_SETCOLOUR, 0, commonData.background);

			SendDlgItemMessage(hWnd, IDC_DEFAULT_MODULE, CB_SETCURSEL, commonData.cDefaultModule, 0);
			SendDlgItemMessage(hWnd, IDC_LEFT_CLICK, CB_SETCURSEL, commonData.lPopupClick, 0);
			SendDlgItemMessage(hWnd, IDC_RIGHT_CLICK, CB_SETCURSEL, commonData.rPopupClick, 0);
			SendDlgItemMessage(hWnd, IDC_NOTIFYFOR, CB_SETCURSEL, commonData.notifyFor, 0);

			CreateToolTip(GetDlgItem(hWnd, IDC_POPUP_TIMEOUT), TranslateT("Set popup delay when notifying of upcoming birthdays.\nFormat: default delay [ | delay for birthdays occurring today]"), 400);

			wchar_t buffer[1024];
			_itow(commonData.daysInAdvance, buffer, 10);
			SetDlgItemText(hWnd, IDC_DAYS_IN_ADVANCE, buffer);
			_itow(commonData.checkInterval, buffer, 10);
			SetDlgItemText(hWnd, IDC_CHECK_INTERVAL, buffer);
			mir_snwprintf(buffer, L"%d|%d", commonData.popupTimeout, commonData.popupTimeoutToday);
			SetDlgItemText(hWnd, IDC_POPUP_TIMEOUT, buffer);
			_itow(commonData.cSoundNearDays, buffer, 10);
			SetDlgItemText(hWnd, IDC_SOUND_NEAR_DAYS_EDIT, buffer);
			_itow(commonData.cDlgTimeout, buffer, 10);
			SetDlgItemText(hWnd, IDC_DLG_TIMEOUT, buffer);
			_itow(commonData.daysAfter, buffer, 10);
			SetDlgItemText(hWnd, IDC_DAYS_AFTER, buffer);

			CheckDlgButton(hWnd, IDC_OPENINBACKGROUND, (commonData.bOpenInBackground) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hWnd, IDC_NOBIRTHDAYS_POPUP, (commonData.bNoBirthdaysPopup) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hWnd, IDC_AGE_COMBOBOX, CB_SETCURSEL, commonData.cShowAgeMode, 0);

			CheckDlgButton(hWnd, IDC_IGNORE_SUBCONTACTS, (commonData.bIgnoreSubcontacts) ? BST_CHECKED : BST_UNCHECKED);

			CheckDlgButton(hWnd, IDC_ONCE_PER_DAY, (commonData.bOncePerDay) ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_INTERVAL), !commonData.bOncePerDay);

			CheckDlgButton(hWnd, IDC_USE_DIALOG, (commonData.bUseDialog) ? BST_CHECKED : BST_UNCHECKED);
			EnableDialogGroup(hWnd, commonData.bUseDialog);

			CheckDlgButton(hWnd, IDC_USE_POPUPS, commonData.bUsePopups ? BST_CHECKED : BST_UNCHECKED);
			EnablePopupsGroup(hWnd, commonData.bUsePopups);

			CheckDlgButton(hWnd, IDC_USE_CLISTICON, BST_CHECKED);
			EnableWindow(GetDlgItem(hWnd, IDC_USE_CLISTICON), FALSE);
			EnableClistGroup(hWnd, FALSE);
		}
		bInitializing = 0;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK_INTERVAL:
		case IDC_POPUP_TIMEOUT:
		case IDC_DAYS_IN_ADVANCE:
		case IDC_DLG_TIMEOUT:
		case IDC_SOUND_NEAR_DAYS_EDIT:
		case IDC_DAYS_AFTER:
			if ((HIWORD(wParam) == EN_CHANGE) && (!bInitializing))// || (HIWORD(wParam) == CBN_SELENDOK))
				SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_USE_POPUPS:
		case IDC_USE_CLISTICON:
		case IDC_USE_DIALOG:
			EnablePopupsGroup(hWnd, IsDlgButtonChecked(hWnd, IDC_USE_POPUPS));
			EnableClistGroup(hWnd, IsDlgButtonChecked(hWnd, IDC_USE_CLISTICON));
			EnableDialogGroup(hWnd, IsDlgButtonChecked(hWnd, IDC_USE_DIALOG));
			RedrawWindow(GetDlgItem(hWnd, IDC_USE_POPUPS), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
			RedrawWindow(GetDlgItem(hWnd, IDC_USE_CLISTICON), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
			RedrawWindow(GetDlgItem(hWnd, IDC_USE_DIALOG), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE);
			//fallthrough

		case IDC_FOREGROUND:
		case IDC_BACKGROUND:
		case IDC_ADVANCED_ICON:
		case IDC_IGNORE_SUBCONTACTS:
		case IDC_AGE_COMBOBOX:
		case IDC_NOBIRTHDAYS_POPUP:
		case IDC_DEFAULT_MODULE:
		case IDC_LEFT_CLICK:
		case IDC_RIGHT_CLICK:
		case IDC_ONCE_PER_DAY:
		case IDC_NOTIFYFOR:
			SendMessage(GetParent(hWnd), PSM_CHANGED, 0, 0);
			EnableWindow(GetDlgItem(hWnd, IDC_CHECK_INTERVAL), BST_UNCHECKED == IsDlgButtonChecked(hWnd, IDC_ONCE_PER_DAY));
			break;

		case IDC_PREVIEW:
			{
				MCONTACT hContact = db_find_first();
				int dtb = rand() % 11; //0..10
				int age = rand() % 50 + 1; //1..50
				PopupNotifyBirthday(hContact, dtb, age);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				commonData.foreground = SendDlgItemMessage(hWnd, IDC_FOREGROUND, CPM_GETCOLOUR, 0, 0);
				commonData.background = SendDlgItemMessage(hWnd, IDC_BACKGROUND, CPM_GETCOLOUR, 0, 0);
				commonData.popupTimeout = POPUP_TIMEOUT;
				commonData.popupTimeoutToday = POPUP_TIMEOUT;
				commonData.bUsePopups = IsDlgButtonChecked(hWnd, IDC_USE_POPUPS);
				commonData.bUseDialog = IsDlgButtonChecked(hWnd, IDC_USE_DIALOG);
				commonData.bIgnoreSubcontacts = IsDlgButtonChecked(hWnd, IDC_IGNORE_SUBCONTACTS);
				commonData.bNoBirthdaysPopup = IsDlgButtonChecked(hWnd, IDC_NOBIRTHDAYS_POPUP);
				commonData.cShowAgeMode = SendDlgItemMessage(hWnd, IDC_AGE_COMBOBOX, CB_GETCURSEL, 0, 0);
				commonData.cDefaultModule = SendDlgItemMessage(hWnd, IDC_DEFAULT_MODULE, CB_GETCURSEL, 0, 0);
				commonData.lPopupClick = SendDlgItemMessage(hWnd, IDC_LEFT_CLICK, CB_GETCURSEL, 0, 0);
				commonData.rPopupClick = SendDlgItemMessage(hWnd, IDC_RIGHT_CLICK, CB_GETCURSEL, 0, 0);
				commonData.bOncePerDay = IsDlgButtonChecked(hWnd, IDC_ONCE_PER_DAY);
				commonData.notifyFor = SendDlgItemMessage(hWnd, IDC_NOTIFYFOR, CB_GETCURSEL, 0, 0);
				commonData.bOpenInBackground = IsDlgButtonChecked(hWnd, IDC_OPENINBACKGROUND);
				{
					const int maxSize = 1024;
					wchar_t buffer[maxSize];

					GetDlgItemText(hWnd, IDC_DAYS_IN_ADVANCE, buffer, _countof(buffer));
					wchar_t *stop = nullptr;
					commonData.daysInAdvance = wcstol(buffer, &stop, 10);

					if (*stop) { commonData.daysInAdvance = DAYS_TO_NOTIFY; }

					GetDlgItemText(hWnd, IDC_DAYS_AFTER, buffer, _countof(buffer));
					commonData.daysAfter = wcstol(buffer, &stop, 10);

					if (*stop) { commonData.daysAfter = DAYS_TO_NOTIFY_AFTER; }

					GetDlgItemText(hWnd, IDC_CHECK_INTERVAL, buffer, _countof(buffer));
					commonData.checkInterval = _wtol(buffer);
					if (!commonData.checkInterval) { commonData.checkInterval = CHECK_INTERVAL; }

					GetDlgItemText(hWnd, IDC_POPUP_TIMEOUT, buffer, _countof(buffer));
					wchar_t *pos;
					pos = wcschr(buffer, '|');
					if (pos) {
						wchar_t tmp[128];
						*pos = 0;
						mir_wstrcpy(tmp, buffer);
						strtrim(tmp);
						commonData.popupTimeout = _wtol(tmp);

						mir_wstrcpy(tmp, pos + 1);
						strtrim(tmp);
						commonData.popupTimeoutToday = _wtol(tmp);

					}
					else commonData.popupTimeout = commonData.popupTimeoutToday = _wtol(buffer);

					GetDlgItemText(hWnd, IDC_SOUND_NEAR_DAYS_EDIT, buffer, _countof(buffer));
					//cSoundNearDays = _wtol(buffer);
					commonData.cSoundNearDays = wcstol(buffer, &stop, 10);
					if (*stop) { commonData.cSoundNearDays = BIRTHDAY_NEAR_DEFAULT_DAYS; }

					GetDlgItemText(hWnd, IDC_DLG_TIMEOUT, buffer, _countof(buffer));
					commonData.cDlgTimeout = wcstol(buffer, &stop, 10);
					if (*stop) { commonData.cDlgTimeout = POPUP_TIMEOUT; }

					g_plugin.setByte("IgnoreSubcontacts", commonData.bIgnoreSubcontacts);
					g_plugin.setByte("UsePopups", commonData.bUsePopups);
					g_plugin.setByte("UseDialog", commonData.bUseDialog);
					g_plugin.setWord("CheckInterval", commonData.checkInterval);
					g_plugin.setWord("DaysInAdvance", commonData.daysInAdvance);
					g_plugin.setWord("DaysAfter", commonData.daysAfter);
					g_plugin.setWord("PopupTimeout", commonData.popupTimeout);
					g_plugin.setWord("PopupTimeoutToday", commonData.popupTimeoutToday);

					g_plugin.setByte("ShowCurrentAge", commonData.cShowAgeMode);
					g_plugin.setByte("NoBirthdaysPopup", commonData.bNoBirthdaysPopup);

					g_plugin.setByte("OpenInBackground", commonData.bOpenInBackground);

					g_plugin.setByte("SoundNearDays", commonData.cSoundNearDays);

					g_plugin.setByte("DefaultModule", commonData.cDefaultModule);

					g_plugin.setByte("PopupLeftClick", commonData.lPopupClick);
					g_plugin.setByte("PopupRightClick", commonData.rPopupClick);

					g_plugin.setDword("Foreground", commonData.foreground);
					g_plugin.setDword("Background", commonData.background);

					g_plugin.setByte("OncePerDay", commonData.bOncePerDay);

					g_plugin.setWord("DlgTimeout", commonData.cDlgTimeout);

					g_plugin.setByte("NotifyFor", commonData.notifyFor);

					RefreshAllContactListIcons();

					UpdateTimers(); //interval might get changed
				}
				bInitializing = 0;
			}
			break;
		}
		break;
	}
	return 0;
}

INT_PTR CALLBACK DlgProcAddBirthday(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

		hContact = lParam;
		WindowList_Add(hAddBirthdayWndsList, hWnd, hContact);
		Utils_RestoreWindowPositionNoSize(hWnd, hContact, MODULENAME, "BirthdayWnd");

		Window_SetIcon_IcoLib(hWnd, hAddBirthdayContact);

		for (int i = 0; i < cSaveModule; i++)
			SendDlgItemMessage(hWnd, IDC_COMPATIBILITY, CB_ADDSTRING, 0, (LPARAM)TranslateW(szSaveModule[i]));
		SendDlgItemMessage(hWnd, IDC_COMPATIBILITY, CB_SETCURSEL, commonData.cDefaultModule, 0);
		break;

	case WM_SHOWWINDOW:
		{
			wchar_t *szTooltipText = TranslateT("Please select the module where you want the date of birth to be saved.\r\n\"UserInfo\" is the default location.\r\nUse \"Protocol module\" to make the data visible in User Details.\n\"mBirthday module\" uses the same module as mBirthday plugin.");
			wchar_t *szCurrentModuleTooltip = nullptr;
			char *szProto = Proto_GetBaseAccountName(hContact);

			wchar_t buffer[2048];
			mir_snwprintf(buffer, TranslateT("Set birthday for %s:"), Clist_GetContactDisplayName(hContact));
			SetWindowText(hWnd, buffer);

			HWND hDate = GetDlgItem(hWnd, IDC_DATE);

			int year, month, day;
			int loc = GetContactDOB(hContact, year, month, day);
			if (IsDOBValid(year, month, day)) {
				SYSTEMTIME st = { 0 };
				st.wDay = day;
				st.wMonth = month;
				st.wYear = year;
				DateTime_SetSystemtime(hDate, GDT_VALID, &st);
			}
			else DateTime_SetSystemtime(hDate, GDT_NONE, NULL);

			switch (loc) {
			case DOB_MBIRTHDAY:
				DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_MBIRTHDAY);
				szCurrentModuleTooltip = L"mBirthday";
				break;

			case DOB_PROTOCOL:
				DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_PROTOCOL);
				mir_snwprintf(buffer, TranslateT("%S protocol"), szProto);
				szCurrentModuleTooltip = buffer;
				break;

			case DOB_BIRTHDAYREMINDER:
				DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_BIRTHDAYREMINDER);
				szCurrentModuleTooltip = L"Birthday Reminder";
				break;

			case DOB_USERINFO:
				DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_USERINFO);
				szCurrentModuleTooltip = L"UserInfo";
				break;

			case DOB_MICQBIRTHDAY:
				DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_MICQBIRTHDAY);
				szCurrentModuleTooltip = L"mICQBirthday";
				break;

			default:
				szCurrentModuleTooltip = nullptr;
				break;
			}

			CreateToolTip(GetDlgItem(hWnd, IDC_COMPATIBILITY), szTooltipText, 500);
			if (szCurrentModuleTooltip)
				CreateToolTip(hDate, szCurrentModuleTooltip, 400);
		}
		break;

	case WM_DESTROY:
		RefreshContactListIcons(hContact); //the birthday might be changed, refresh icon.
		Window_FreeIcon_IcoLib(hWnd);
		Utils_SaveWindowPosition(hWnd, hContact, MODULENAME, "BirthdayWnd");
		WindowList_Remove(hAddBirthdayWndsList, hWnd);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			hContact = (MCONTACT)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			HWND hDate = GetDlgItem(hWnd, IDC_DATE);
			SYSTEMTIME st;
			if (DateTime_GetSystemtime(hDate, &st) == GDT_VALID) {
				int mode = SendDlgItemMessage(hWnd, IDC_COMPATIBILITY, CB_GETCURSEL, 0, 0); //SAVE modes  in date_utils.h are synced
				SaveBirthday(hContact, st.wYear, st.wMonth, st.wDay, mode);
			}
			else SaveBirthday(hContact, 0, 0, 0, SAVE_MODE_DELETEALL);

			if (hBirthdaysDlg != nullptr)
				SendMessage(hBirthdaysDlg, WWIM_UPDATE_BIRTHDAY, hContact, NULL);

			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;
	}

	return FALSE;
}

void AddAnchorWindowToDeferList(HDWP &hdWnds, HWND window, RECT *rParent, WINDOWPOS *wndPos, int anchors)
{
	if (nullptr == window) /* Wine fix. */
		return;
	RECT rChild = AnchorCalcPos(window, rParent, wndPos, anchors);
	hdWnds = DeferWindowPos(hdWnds, window, HWND_NOTOPMOST, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, SWP_NOZORDER);
}


#define NA TranslateT("N/A")

wchar_t* GetBirthdayModule(int module, MCONTACT)
{
	switch (module) {
	case DOB_MBIRTHDAY:        return L"mBirthday";
	case DOB_PROTOCOL:         return TranslateT("Protocol module");
	case DOB_BIRTHDAYREMINDER: return L"Birthday Reminder";
	case DOB_USERINFO:         return L"UserInfo";
	case DOB_MICQBIRTHDAY:     return L"mICQBirthday";
	}
	return NA;
}

static int lastColumn = -1;

struct BirthdaysSortParams
{
	HWND hList;
	int column;
};

INT_PTR CALLBACK BirthdaysCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	BirthdaysSortParams params = *(BirthdaysSortParams *)myParam;
	const int maxSize = 1024;
	wchar_t text1[maxSize];
	wchar_t text2[maxSize];
	long value1, value2;
	ListView_GetItemText(params.hList, (int)lParam1, params.column, text1, _countof(text1));
	ListView_GetItemText(params.hList, (int)lParam2, params.column, text2, _countof(text2));

	int res = 0;

	if ((params.column == 2) || (params.column == 4)) {
		wchar_t *err1, *err2;
		value1 = wcstol(text1, &err1, 10);
		value2 = wcstol(text2, &err2, 10);

		if ((err1[0]) || (err2[0]))
			res = (err1[0]) ? 1 : -1;
		else if (value1 < value2)
			res = -1;
		else
			res = (value1 != value2);
	}
	else res = mir_wstrcmpi(text1, text2);

	res = (params.column == lastColumn) ? -res : res;
	return res;
}

//only updates the birthday part of the list view entry. Won't update the szProto and the contact name (those shouldn't change anyway :))
int UpdateBirthdayEntry(HWND hList, MCONTACT hContact, int entry, int bShowAll, int bShowCurrentAge, int bAdd)
{
	int currentMonth, currentDay;
	int res = entry;

	if (bShowCurrentAge) {
		time_t now = Today();
		struct tm *today = gmtime(&now);
		currentDay = today->tm_mday + 1;
		currentMonth = today->tm_mon + 1;
	}
	else currentMonth = currentDay = 0;

	int year, month, day;
	int module = GetContactDOB(hContact, year, month, day);
	if (bShowAll || IsDOBValid(year, month, day)) {
		lastColumn = -1; //list isn't sorted anymore
		int dtb = DaysToBirthday(Today(), year, month, day);
		int age = GetContactAge(hContact);
		if (bShowCurrentAge)
			if (month > currentMonth || (month == currentMonth) && (day > currentDay)) // birthday still to come
				age--;

		char *szProto = Proto_GetBaseAccountName(hContact);
		PROTOACCOUNT *pAcc = Proto_GetAccount(szProto);
		wchar_t *ptszAccName = (pAcc == nullptr) ? TranslateT("Unknown") : pAcc->tszAccountName;

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = entry;
		item.lParam = hContact;
		item.pszText = ptszAccName;

		if (bAdd)
			ListView_InsertItem(hList, &item);
		else
			ListView_SetItemText(hList, entry, 0, ptszAccName);

		ListView_SetItemText(hList, entry, 1, Clist_GetContactDisplayName(hContact));

		wchar_t buffer[2048];
		if ((dtb <= 366) && (dtb >= 0))
			mir_snwprintf(buffer, L"%d", dtb);
		else
			mir_snwprintf(buffer, NA);

		ListView_SetItemText(hList, entry, 2, buffer);
		if ((month != 0) && (day != 0))
			mir_snwprintf(buffer, L"%04d-%02d-%02d", year, month, day);
		else
			mir_snwprintf(buffer, NA);

		ListView_SetItemText(hList, entry, 3, buffer);

		if (age < 400 && age > 0) //hopefully noone lives longer than this :)
			mir_snwprintf(buffer, L"%d", age);
		else
			mir_snwprintf(buffer, NA);

		ListView_SetItemText(hList, entry, 4, buffer);
		ListView_SetItemText(hList, entry, 5, GetBirthdayModule(module, hContact));

		res++;
	}
	else if (!bShowAll && !bAdd)
		ListView_DeleteItem(hList, entry);

	return res;
}

static LRESULT CALLBACK BirthdaysListSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LVITEM item = { 0 };
	MCONTACT hContact;
	int i, count;

	switch (msg) {
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
		break;

	case WM_SYSKEYDOWN:
		if (wParam == 'X')
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
		break;

	case WM_LBUTTONDBLCLK:
		count = ListView_GetItemCount(hWnd);
		item.mask = LVIF_PARAM;
		for (i = 0; i < count; i++) {
			if (ListView_GetItemState(hWnd, i, LVIS_SELECTED)) {
				item.iItem = i;
				ListView_GetItem(hWnd, &item);
				hContact = (MCONTACT)item.lParam;
				CallService(MS_WWI_ADD_BIRTHDAY, hContact, 0);
				break;
			}
		}
		break;

	case WM_CONTEXTMENU:
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		count = ListView_GetItemCount(hWnd);
		item.mask = LVIF_PARAM;
		for (i = 0; i < count; i++) {
			if (ListView_GetItemState(hWnd, i, LVIS_SELECTED)) {
				item.iItem = i;
				ListView_GetItem(hWnd, &item);
				hContact = (MCONTACT)item.lParam;
				HMENU hMenu = Menu_BuildContactMenu(hContact);
				if (hMenu != nullptr) {
					Clist_MenuProcessCommand(TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, nullptr), MPCF_CONTACTMENU, hContact);
					DestroyMenu(hMenu);
				}
				break;
			}
		}
		break;
	}
	return mir_callNextSubclass(hWnd, BirthdaysListSubclassProc, msg, wParam, lParam);
}

void SetBirthdaysCount(HWND hWnd)
{
	int count = ListView_GetItemCount((GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST)));
	wchar_t title[512];
	mir_snwprintf(title, TranslateT("Birthday list (%d)"), count);
	SetWindowText(hWnd, title);
}

int LoadBirthdays(HWND hWnd, int bShowAll)
{
	HWND hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
	ListView_DeleteAllItems(hList);

	int count = 0;
	for (auto &hContact : Contacts())
		count = UpdateBirthdayEntry(hList, hContact, count, bShowAll, commonData.cShowAgeMode, 1);

	SetBirthdaysCount(hWnd);
	return 0;
}

INT_PTR CALLBACK DlgProcBirthdays(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		Window_SetIcon_IcoLib(hWnd, hListMenu);
		{
			HWND hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);

			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

			mir_subclassWindow(hList, BirthdaysListSubclassProc);

			LVCOLUMN col;
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = TranslateT("Protocol");
			col.cx = 80;
			ListView_InsertColumn(hList, 0, &col);
			col.pszText = TranslateT("Contact");
			col.cx = 180;
			ListView_InsertColumn(hList, 1, &col);
			col.pszText = TranslateT("DTB");
			col.cx = 50;
			ListView_InsertColumn(hList, 2, &col);
			col.pszText = TranslateT("Birthday");
			col.cx = 80;
			ListView_InsertColumn(hList, 3, &col);
			col.pszText = TranslateT("Age");
			col.cx = 50;
			ListView_InsertColumn(hList, 4, &col);
			col.pszText = TranslateT("Module");
			col.cx = 108;
			ListView_InsertColumn(hList, 5, &col);

			LoadBirthdays(hWnd, 0);
			int column = g_plugin.getByte("SortColumn", 0);

			BirthdaysSortParams params = {};
			params.hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
			params.column = column;
			ListView_SortItemsEx(params.hList, BirthdaysCompare, (LPARAM)&params);

			Utils_RestoreWindowPosition(hWnd, NULL, MODULENAME, "BirthdayList");
		}
		return TRUE;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WWIM_UPDATE_BIRTHDAY:
		{
			MCONTACT hContact = wParam;
			HWND hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
			LVFINDINFO fi = { 0 };

			fi.flags = LVFI_PARAM;
			fi.lParam = (LPARAM)hContact;
			int idx = ListView_FindItem(hList, -1, &fi);
			if (-1 == idx)
				UpdateBirthdayEntry(hList, hContact, ListView_GetItemCount(hList), IsDlgButtonChecked(hWnd, IDC_SHOW_ALL), commonData.cShowAgeMode, 1);
			else
				UpdateBirthdayEntry(hList, hContact, idx, IsDlgButtonChecked(hWnd, IDC_SHOW_ALL), commonData.cShowAgeMode, 0);
			SetBirthdaysCount(hWnd);
		}
		break;

	case WM_WINDOWPOSCHANGING:
		{
			HDWP hdWnds = BeginDeferWindowPos(2);
			RECT rParent;
			WINDOWPOS *wndPos = (WINDOWPOS *)lParam;
			GetWindowRect(hWnd, &rParent);

			if (wndPos->cx < MIN_BIRTHDAYS_WIDTH)
				wndPos->cx = MIN_BIRTHDAYS_WIDTH;

			if (wndPos->cy < MIN_BIRTHDAYS_HEIGHT)
				wndPos->cy = MIN_BIRTHDAYS_HEIGHT;

			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_CLOSE), &rParent, wndPos, ANCHOR_RIGHT | ANCHOR_BOTTOM);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_SHOW_ALL), &rParent, wndPos, ANCHOR_LEFT | ANCHOR_BOTTOM);
			AddAnchorWindowToDeferList(hdWnds, GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST), &rParent, wndPos, ANCHOR_ALL);

			EndDeferWindowPos(hdWnds);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDC_SHOW_ALL:
			LoadBirthdays(hWnd, IsDlgButtonChecked(hWnd, IDC_SHOW_ALL));
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_BIRTHDAYS_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case LVN_COLUMNCLICK:
				LPNMLISTVIEW lv = (LPNMLISTVIEW)lParam;
				int column = lv->iSubItem;
				g_plugin.setByte("SortColumn", column);
				BirthdaysSortParams params = {};
				params.hList = GetDlgItem(hWnd, IDC_BIRTHDAYS_LIST);
				params.column = column;
				ListView_SortItemsEx(params.hList, BirthdaysCompare, (LPARAM)&params);
				lastColumn = (params.column == lastColumn) ? -1 : params.column;
				break;
			}
		}
		break;

	case WM_DESTROY:
		hBirthdaysDlg = nullptr;
		Utils_SaveWindowPosition(hWnd, NULL, MODULENAME, "BirthdayList");
		Window_FreeIcon_IcoLib(hWnd);
		lastColumn = -1;
		break;

	}
	return 0;
}


INT_PTR CALLBACK DlgProcUpcoming(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int timeout;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWnd);
		Window_SetIcon_IcoLib(hWnd, hListMenu);
		{
			timeout = commonData.cDlgTimeout;
			HWND hList = GetDlgItem(hWnd, IDC_UPCOMING_LIST);

			mir_subclassWindow(hList, BirthdaysListSubclassProc);
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

			LVCOLUMN col;
			col.mask = LVCF_TEXT | LVCF_WIDTH;
			col.pszText = TranslateT("Contact");
			col.cx = 300;
			ListView_InsertColumn(hList, 0, &col);
			col.pszText = TranslateT("Age");
			col.cx = 45;
			ListView_InsertColumn(hList, 1, &col);
			col.pszText = TranslateT("DTB");
			col.cx = 45;
			ListView_InsertColumn(hList, 2, &col);

			ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);

			if (timeout > 0)
				SetTimer(hWnd, UPCOMING_TIMER_ID, 1000, nullptr);
			Utils_RestoreWindowPosition(hWnd, NULL, MODULENAME, "BirthdayListUpcoming");
		}
		return TRUE;

	case WM_TIMER:
		{
			const int MAX_SIZE = 512;
			wchar_t buffer[MAX_SIZE];
			timeout--;
			mir_snwprintf(buffer, (timeout != 2) ? TranslateT("Closing in %d seconds") : TranslateT("Closing in %d second"), timeout);
			SetDlgItemText(hWnd, IDC_CLOSE, buffer);

			if (timeout <= 0)
				SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WWIM_ADD_UPCOMING_BIRTHDAY:
		{
			PUpcomingBirthday data = (PUpcomingBirthday)wParam;

			HWND hList = GetDlgItem(hWnd, IDC_UPCOMING_LIST);
			LVITEM item = { 0 };
			LVFINDINFO fi = { 0 };

			fi.flags = LVFI_PARAM;
			fi.lParam = (LPARAM)data->hContact;
			if (-1 != ListView_FindItem(hList, -1, &fi))
				return 0; /* Allready in list. */

			int index = ListView_GetItemCount(hList);
			item.iItem = index;
			item.mask = LVIF_PARAM | LVIF_TEXT;
			item.lParam = (LPARAM)data->hContact;
			item.pszText = data->message;
			ListView_InsertItem(hList, &item);

			wchar_t buffer[512];
			mir_snwprintf(buffer, L"%d", data->age);
			ListView_SetItemText(hList, index, 1, buffer);
			mir_snwprintf(buffer, L"%d", data->dtb);
			ListView_SetItemText(hList, index, 2, buffer);

			BirthdaysSortParams params = {};
			params.hList = hList;
			params.column = 2;
			ListView_SortItemsEx(hList, BirthdaysCompare, (LPARAM)&params);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = 400;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = 160;
		((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = 600;
		((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = 530;
		break;

	case WM_SIZE:
		{
			HWND hList = GetDlgItem(hWnd, IDC_UPCOMING_LIST);

			RECT rcWin;
			GetWindowRect(hWnd, &rcWin);

			int cx = rcWin.right - rcWin.left;
			int cy = rcWin.bottom - rcWin.top;
			SetWindowPos(hList, nullptr, 0, 0, (cx - 30), (cy - 80), (SWP_NOZORDER | SWP_NOMOVE));
			ListView_SetColumnWidth(hList, 0, (cx - 150));
			SetWindowPos(GetDlgItem(hWnd, IDC_CLOSE), nullptr, ((cx / 2) - 95), (cy - 67), 0, 0, SWP_NOSIZE);
			RedrawWindow(hWnd, nullptr, nullptr, (RDW_FRAME | RDW_INVALIDATE));
		}
		break;

	case WM_DESTROY:
		hUpcomingDlg = nullptr;
		Utils_SaveWindowPosition(hWnd, NULL, MODULENAME, "BirthdayListUpcoming");
		Window_FreeIcon_IcoLib(hWnd);
		KillTimer(hWnd, UPCOMING_TIMER_ID);
		break;
	}

	return 0;
}

int HandlePopupClick(HWND hWnd, int action)
{
	MCONTACT hContact;

	switch (action) {
	case 2: //OPEN MESSAGE WINDOW
		hContact = (MCONTACT)PUGetContact(hWnd);
		if (hContact)
			CallServiceSync(MS_MSG_SENDMESSAGE, hContact, 0);

	case 1: //DISMISS
		PUDeletePopup(hWnd);
		break;
	}

	return 0;
}

LRESULT CALLBACK DlgProcPopup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case STN_CLICKED:
			HandlePopupClick(hWnd, commonData.lPopupClick);
			break;
		}
		break;

	case WM_CONTEXTMENU:
		HandlePopupClick(hWnd, commonData.rPopupClick);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
