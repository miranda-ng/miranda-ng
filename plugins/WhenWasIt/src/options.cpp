/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

const wchar_t* szShowAgeMode[] = { LPGENW("Upcoming age"), LPGENW("Current age") };

const wchar_t* szPopupClick[] = { LPGENW("Nothing"), LPGENW("Dismiss"), LPGENW("Message window") };

const wchar_t* szNotifyFor[] = { LPGENW("All contacts"), LPGENW("All contacts except hidden ones"), LPGENW("All contacts except ignored ones"), LPGENW("All contacts except hidden and ignored ones") };

static SIZE GetControlTextSize(HWND hCtrl)
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

static int EnlargeControl(HWND hCtrl, HWND, SIZE oldSize)
{
	SIZE size = GetControlTextSize(hCtrl);
	RECT rect;
	GetWindowRect(hCtrl, &rect);
	int offset = (rect.right - rect.left) - oldSize.cx;
	SetWindowPos(hCtrl, HWND_TOP, 0, 0, size.cx + offset, oldSize.cy, SWP_NOMOVE);
	SetWindowPos(hCtrl, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return 0;
}

static wchar_t* strtrim(wchar_t *str)
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

class COptionsDlg : public CDlgBase
{
	CCtrlButton btnPreview;
	CCtrlCheck chkPopups, chkDialog, chkOncePerDay;
	CCtrlCombo cmbAge, cmbLClick, cmbRClick, cmbNotify;
	CCtrlColor clrFore, clrBack;

	SIZE oldPopupsSize, oldDialogSize;

	UI_MESSAGE_MAP(COptionsDlg, CDlgBase);
		UI_MESSAGE(WM_INITDIALOG, OnInitWindow);
	UI_MESSAGE_MAP_END();

	INT_PTR OnInitWindow(UINT, WPARAM, LPARAM)
	{
		oldPopupsSize = GetControlTextSize(GetDlgItem(m_hwnd, IDC_USE_POPUPS));
		oldDialogSize = GetControlTextSize(GetDlgItem(m_hwnd, IDC_USE_DIALOG));
		return FALSE;
	}

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_WWI),
		btnPreview(this, IDC_PREVIEW),
		clrFore(this, IDC_FOREGROUND),
		clrBack(this, IDC_BACKGROUND),
		cmbAge(this, IDC_AGE_COMBOBOX),
		cmbNotify(this, IDC_NOTIFYFOR),
		cmbLClick(this, IDC_LEFT_CLICK),
		cmbRClick(this, IDC_RIGHT_CLICK),
		chkPopups(this, IDC_USE_POPUPS),
		chkDialog(this, IDC_USE_DIALOG),
		chkOncePerDay(this, IDC_ONCE_PER_DAY)
	{
		CreateLink(clrFore, g_plugin.foreground);
		CreateLink(clrBack, g_plugin.background);

		btnPreview.OnClick = Callback(this, &COptionsDlg::onClick_Preview);

		chkPopups.OnChange = Callback(this, &COptionsDlg::onChange_Popups);
		chkDialog.OnChange = Callback(this, &COptionsDlg::onChange_Dialog);
		chkOncePerDay.OnChange = Callback(this, &COptionsDlg::onChange_OncePerDay);
	}

	bool OnInitDialog() override
	{
		EnlargeControl(chkPopups.GetHwnd(), GetDlgItem(m_hwnd, IDC_POPUPS_STATIC), oldPopupsSize);
		EnlargeControl(GetDlgItem(m_hwnd, IDC_USE_DIALOG), GetDlgItem(m_hwnd, IDC_DIALOG_STATIC), oldDialogSize);

		for (auto &it : szShowAgeMode)
			cmbAge.AddString(TranslateW(it));

		for (auto &it : szPopupClick) {
			cmbLClick.AddString(TranslateW(it));
			cmbRClick.AddString(TranslateW(it));
		}

		for (auto &it : szNotifyFor)
			cmbNotify.AddString(TranslateW(it));

		cmbLClick.SetCurSel(g_plugin.lPopupClick);
		cmbRClick.SetCurSel(g_plugin.rPopupClick);
		cmbNotify.SetCurSel(g_plugin.notifyFor);

		CreateToolTip(GetDlgItem(m_hwnd, IDC_POPUP_TIMEOUT), TranslateT("Set popup delay when notifying of upcoming birthdays.\nFormat: default delay [ | delay for birthdays occurring today]"), 400);

		wchar_t buffer[1024];
		_itow(g_plugin.daysInAdvance, buffer, 10);
		SetDlgItemText(m_hwnd, IDC_DAYS_IN_ADVANCE, buffer);
		_itow(g_plugin.checkInterval, buffer, 10);
		SetDlgItemText(m_hwnd, IDC_CHECK_INTERVAL, buffer);
		mir_snwprintf(buffer, L"%d|%d", (int)g_plugin.popupTimeout, (int)g_plugin.popupTimeoutToday);
		SetDlgItemText(m_hwnd, IDC_POPUP_TIMEOUT, buffer);
		_itow(g_plugin.cSoundNearDays, buffer, 10);
		SetDlgItemText(m_hwnd, IDC_SOUND_NEAR_DAYS_EDIT, buffer);
		_itow(g_plugin.cDlgTimeout, buffer, 10);
		SetDlgItemText(m_hwnd, IDC_DLG_TIMEOUT, buffer);
		_itow(g_plugin.daysAfter, buffer, 10);
		SetDlgItemText(m_hwnd, IDC_DAYS_AFTER, buffer);

		CheckDlgButton(m_hwnd, IDC_OPENINBACKGROUND, (g_plugin.bOpenInBackground) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(m_hwnd, IDC_NOBIRTHDAYS_POPUP, (g_plugin.bNoBirthdaysPopup) ? BST_CHECKED : BST_UNCHECKED);
		cmbAge.SetCurSel(g_plugin.cShowAgeMode);

		CheckDlgButton(m_hwnd, IDC_IGNORE_SUBCONTACTS, (g_plugin.bIgnoreSubcontacts) ? BST_CHECKED : BST_UNCHECKED);

		chkOncePerDay.SetState(g_plugin.bOncePerDay);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECK_INTERVAL), !g_plugin.bOncePerDay);

		CheckDlgButton(m_hwnd, IDC_USE_DIALOG, (g_plugin.bUseDialog) ? BST_CHECKED : BST_UNCHECKED);

		chkPopups.SetState(g_plugin.bUsePopups);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.popupTimeout = POPUP_TIMEOUT;
		g_plugin.popupTimeoutToday = POPUP_TIMEOUT;
		g_plugin.bUsePopups = chkPopups.GetState();
		g_plugin.bUseDialog = IsDlgButtonChecked(m_hwnd, IDC_USE_DIALOG);
		g_plugin.bIgnoreSubcontacts = IsDlgButtonChecked(m_hwnd, IDC_IGNORE_SUBCONTACTS);
		g_plugin.bNoBirthdaysPopup = IsDlgButtonChecked(m_hwnd, IDC_NOBIRTHDAYS_POPUP);
		g_plugin.cShowAgeMode = cmbAge.GetCurSel();
		g_plugin.lPopupClick = cmbLClick.GetCurSel();
		g_plugin.rPopupClick = cmbRClick.GetCurSel();
		g_plugin.notifyFor = cmbNotify.GetCurSel();
		g_plugin.bOncePerDay = chkOncePerDay.GetState();
		g_plugin.bOpenInBackground = IsDlgButtonChecked(m_hwnd, IDC_OPENINBACKGROUND);

		g_plugin.daysInAdvance = GetDlgItemInt(m_hwnd, IDC_DAYS_IN_ADVANCE, 0, 0);
		g_plugin.daysAfter = GetDlgItemInt(m_hwnd, IDC_DAYS_AFTER, 0, 0);
		g_plugin.checkInterval = GetDlgItemInt(m_hwnd, IDC_CHECK_INTERVAL, 0, 0);
		g_plugin.cSoundNearDays = GetDlgItemInt(m_hwnd, IDC_SOUND_NEAR_DAYS_EDIT, 0, 0);
		g_plugin.cDlgTimeout = GetDlgItemInt(m_hwnd, IDC_DLG_TIMEOUT, 0, 0);

		const int maxSize = 1024;
		wchar_t buffer[maxSize];
		GetDlgItemText(m_hwnd, IDC_POPUP_TIMEOUT, buffer, _countof(buffer));
		wchar_t *pos = wcschr(buffer, '|');
		if (pos) {
			wchar_t tmp[128];
			*pos = 0;
			mir_wstrcpy(tmp, buffer);
			strtrim(tmp);
			g_plugin.popupTimeout = _wtol(tmp);

			mir_wstrcpy(tmp, pos + 1);
			strtrim(tmp);
			g_plugin.popupTimeoutToday = _wtol(tmp);

		}
		else g_plugin.popupTimeout = g_plugin.popupTimeoutToday = _wtol(buffer);

		RefreshAllContactListIcons();

		UpdateTimers(); //interval might get changed
		return true;
	}

	void onChange_Popups(CCtrlCheck *pCheck)
	{
		bool bEnable = pCheck->GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_POPUPS_STATIC), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_POPUP_TIMEOUT), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_NOBIRTHDAYS_POPUP), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_IGNORE_SUBCONTACTS), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_PREVIEW), bEnable);
		clrFore.Enable(bEnable);
		clrBack.Enable(bEnable);
		cmbLClick.Enable(bEnable);
		cmbRClick.Enable(bEnable);
	}

	void onChange_Dialog(CCtrlCheck *pCheck)
	{
		bool bEnable = pCheck->GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_DLG_TIMEOUT), bEnable);
		EnableWindow(GetDlgItem(m_hwnd, IDC_OPENINBACKGROUND), bEnable);
	}

	void onChange_OncePerDay(CCtrlCheck *pCheck)
	{
		bool bEnable = pCheck->GetState();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHECK_INTERVAL), !bEnable);
	}

	void onClick_Preview(CCtrlButton *)
	{
		MCONTACT hContact = db_find_first();
		int dtb = rand() % 11; //0..10
		int age = rand() % 50 + 1; //1..50
		PopupNotifyBirthday(hContact, dtb, age);
	}
};

int OnOptionsInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 100000000;
	odp.pDialog = new COptionsDlg();
	odp.szTitle.a = LPGEN("Birthdays");
	odp.szGroup.a = LPGEN("Contacts");
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
