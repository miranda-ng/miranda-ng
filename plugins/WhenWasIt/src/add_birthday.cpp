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

void UpdateBirthday(MCONTACT);

class CAddBirthdayDlg : public CDlgBase
{
	MCONTACT m_hContact;
	int m_saveMethod;

public:
	CAddBirthdayDlg(MCONTACT hContact) :
		CDlgBase(g_plugin, IDD_ADD_BIRTHDAY),
		m_hContact(hContact)
	{
		if ((int)m_hContact < 0) {
			m_saveMethod = DOB_USERINFO;
			m_hContact = -m_hContact;
		}
		else m_saveMethod = DOB_PROTOCOL;
	}

	bool OnInitDialog() override
	{
		WindowList_Add(hAddBirthdayWndsList, m_hwnd, m_hContact);
		Utils_RestoreWindowPositionNoSize(m_hwnd, m_hContact, MODULENAME, "BirthdayWnd");

		Window_SetIcon_IcoLib(m_hwnd, hAddBirthdayContact);

		CMStringW buf(FORMAT, TranslateT("Set birthday for %s:"), Clist_GetContactDisplayName(m_hContact));
		SetCaption(buf);

		HWND hDate = GetDlgItem(m_hwnd, IDC_DATE);

		int year, month, day;
		GetContactDOB(m_hContact, year, month, day, m_saveMethod);
		if (IsDOBValid(year, month, day)) {
			SYSTEMTIME st = { 0 };
			st.wDay = day;
			st.wMonth = month;
			st.wYear = year;
			DateTime_SetSystemtime(hDate, GDT_VALID, &st);
		}
		else DateTime_SetSystemtime(hDate, GDT_NONE, NULL);

		DateTime_SetMonthCalColor(hDate, MCSC_TITLEBK, COLOR_USERINFO);
		return true;
	}

	bool OnApply() override
	{
		HWND hDate = GetDlgItem(m_hwnd, IDC_DATE);
		SYSTEMTIME st;
		if (DateTime_GetSystemtime(hDate, &st) == GDT_VALID)
			SaveBirthday(m_hContact, st.wYear, st.wMonth, st.wDay, m_saveMethod);
		else
			DeleteBirthday(m_hContact);

		UpdateBirthday((m_saveMethod == DOB_PROTOCOL) ? m_hContact : -m_hContact);
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
