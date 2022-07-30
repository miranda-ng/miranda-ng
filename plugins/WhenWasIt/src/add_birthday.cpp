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

#define COLOR_USERINFO RGB(138, 190, 160)
#define COLOR_MBIRTHDAY RGB(222, 222, 88)
#define COLOR_BIRTHDAYREMINDER RGB(200, 120, 240)
#define COLOR_MICQBIRTHDAY RGB(88, 88, 240)
#define COLOR_PROTOCOL RGB(255, 153, 153)

extern const wchar_t* szSaveModule[2];

class CAddBirthdayDlg : public CDlgBase
{
	MCONTACT m_hContact;

	CCtrlCombo cmbCompat;

public:
	CAddBirthdayDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_ADD_BIRTHDAY),
		m_hContact(hContact),
		cmbCompat(this, IDC_COMPATIBILITY)
	{
	}

	bool OnInitDialog() override
	{
		WindowList_Add(hAddBirthdayWndsList, m_hwnd, m_hContact);
		Utils_RestoreWindowPositionNoSize(m_hwnd, m_hContact, MODULENAME, "BirthdayWnd");

		Window_SetIcon_IcoLib(m_hwnd, hAddBirthdayContact);

		for (auto &it : szSaveModule)
			cmbCompat.AddString(TranslateW(it));
		cmbCompat.SetCurSel(g_plugin.cDefaultModule);

		wchar_t *szTooltipText = TranslateT("Please select the module where you want the date of birth to be saved.\r\n\"UserInfo\" is the default location.\r\nUse \"Protocol module\" to make the data visible in User Details.\n\"mBirthday module\" uses the same module as mBirthday plugin.");

		CMStringW buf(FORMAT, TranslateT("Set birthday for %s:"), Clist_GetContactDisplayName(m_hContact));
		SetCaption(buf);

		HWND hDate = GetDlgItem(m_hwnd, IDC_DATE);

		int year, month, day;
		int loc = GetContactDOB(m_hContact, year, month, day);
		if (IsDOBValid(year, month, day)) {
			SYSTEMTIME st = { 0 };
			st.wDay = day;
			st.wMonth = month;
			st.wYear = year;
			DateTime_SetSystemtime(hDate, GDT_VALID, &st);
		}
		else DateTime_SetSystemtime(hDate, GDT_NONE, NULL);

		const wchar_t *szCurrentModuleTooltip;
		switch (loc) {
		case DOB_PROTOCOL:
			DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_PROTOCOL);
			buf.Format(TranslateT("%S protocol"), Proto_GetBaseAccountName(m_hContact));
			szCurrentModuleTooltip = buf;
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

		CreateToolTip(cmbCompat.GetHwnd(), szTooltipText, 500);
		if (szCurrentModuleTooltip)
			CreateToolTip(hDate, szCurrentModuleTooltip, 400);
		return true;
	}

	bool OnApply() override
	{
		HWND hDate = GetDlgItem(m_hwnd, IDC_DATE);
		SYSTEMTIME st;
		if (DateTime_GetSystemtime(hDate, &st) == GDT_VALID)
			SaveBirthday(m_hContact, st.wYear, st.wMonth, st.wDay, cmbCompat.GetCurSel());
		else
			SaveBirthday(m_hContact, 0, 0, 0, SAVE_MODE_DELETEALL);

		if (hBirthdaysDlg != nullptr)
			SendMessage(hBirthdaysDlg, WWIM_UPDATE_BIRTHDAY, m_hContact, NULL);
		return true;
	}

	void OnDestroy() override
	{
		RefreshContactListIcons(m_hContact); //the birthday might be changed, refresh icon.
		Window_FreeIcon_IcoLib(m_hwnd);
		Utils_SaveWindowPosition(m_hwnd, m_hContact, MODULENAME, "BirthdayWnd");
		WindowList_Remove(hAddBirthdayWndsList, m_hwnd);
	}
};

INT_PTR AddBirthdayService(WPARAM hContact, LPARAM)
{
	HWND hWnd = WindowList_Find(hAddBirthdayWndsList, hContact);
	if (!hWnd)
		(new CAddBirthdayDlg(hContact))->Show();
	else
		ShowWindow(hWnd, SW_SHOW);

	return 0;
}
