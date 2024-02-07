/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean
Copyright (C) 2014-24 Rozhuk Ivan

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

#define NA TranslateT("N/A")

wchar_t* GetBirthdayModule(int iModule, MCONTACT)
{
	switch (iModule) {
	case DOB_PROTOCOL:         return TranslateT("Protocol Module");
	case DOB_USERINFO:         return L"UserInfo";
	}
	return NA;
}

static int lastColumn = -1;

struct BirthdaysSortParams
{
	HWND hList;
	int column;
};

int CALLBACK BirthdaysCompare(LPARAM lParam1, LPARAM lParam2, LPARAM myParam)
{
	BirthdaysSortParams params = *(BirthdaysSortParams *)myParam;
	const int maxSize = 1024;
	wchar_t text1[maxSize];
	wchar_t text2[maxSize];
	long value1, value2;
	ListView_GetItemText(params.hList, (int)lParam1, params.column, text1, _countof(text1));
	ListView_GetItemText(params.hList, (int)lParam2, params.column, text2, _countof(text2));

	int res;
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

	return (params.column == lastColumn) ? -res : res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Basic list dialog

class CBasicListDlg : public CDlgBase
{
	MCONTACT SelectedItem()
	{
		int count = m_list.GetItemCount();

		LVITEM item = {};
		item.mask = LVIF_PARAM;
		for (int i = 0; i < count; i++) {
			if (m_list.GetItemState(i, LVIS_SELECTED)) {
				item.iItem = i;
				m_list.GetItem(&item);

				int res = item.lParam;
				return (res < 0) ? -res : res;
			}
		}
		return 0;
	}

protected:
	CCtrlListView m_list;

	CBasicListDlg(int dlgId) :
		CDlgBase(g_plugin, dlgId),
		m_list(this, IDC_BIRTHDAYS_LIST)
	{
		m_list.OnDoubleClick = Callback(this, &CBasicListDlg::onDblClick_List);
		m_list.OnBuildMenu = Callback(this, &CBasicListDlg::onMenu_List);
	}

	void Sort(int iCol)
	{
		BirthdaysSortParams params = {};
		params.hList = m_list.GetHwnd();
		params.column = iCol;
		m_list.SortItemsEx(BirthdaysCompare, (LPARAM)&params);
	}

	void onDblClick_List(CCtrlListView::TEventInfo *)
	{
		if (MCONTACT hContact = SelectedItem())
			AddBirthdayService(hContact, TRUE);
	}

	void onMenu_List(CContextMenuPos *pos)
	{
		if (MCONTACT hContact = SelectedItem()) {
			HMENU hMenu = Menu_BuildContactMenu(hContact);
			if (hMenu != nullptr) {
				Clist_MenuProcessCommand(TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pos->pt.x, pos->pt.y, 0, m_list.GetHwnd(), nullptr), MPCF_CONTACTMENU, hContact);
				DestroyMenu(hMenu);
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Birthday list dialog

static class CBirthdaysDlg *g_pDialog;

class CBirthdaysDlg : public CBasicListDlg
{
	void SetBirthdaysCount()
	{
		SetCaption(CMStringW(FORMAT, TranslateT("Birthday list (%d)"), m_list.GetItemCount()));
	}

	int LoadBirthdays()
	{
		m_list.DeleteAllItems();

		int count = 0;
		for (auto &hContact : Contacts()) {
			count = UpdateBirthdayEntry(hContact, count, 1, DOB_USERINFO);
			count = UpdateBirthdayEntry(hContact, count, 1, DOB_PROTOCOL);
		}

		SetBirthdaysCount();
		return 0;
	}

	// only updates the birthday part of the list view entry. Won't update the szProto and the contact name (those shouldn't change anyway :))
	int UpdateBirthdayEntry(MCONTACT hContact, int entry, int bAdd, int iModule)
	{
		int res = entry;
		bool bShowAll = chkShowAll.GetState();

		int year = 0, month = 0, day = 0;
		GetContactDOB(hContact, year, month, day, iModule);
		if (!IsDOBValid(year, month, day))
			return res;
		 
		PROTOACCOUNT *pAcc = Proto_GetContactAccount(hContact);
		if (pAcc == nullptr)
			return res;

		if (bShowAll || pAcc->IsEnabled()) {
			lastColumn = -1; // list isn't sorted anymore
			int dtb = DaysToBirthday(Today(), year, month, day);
			int age = GetContactAge(year, month, day);

			if (bAdd) {
				LVITEM item = {};
				item.mask = LVIF_TEXT | LVIF_PARAM;
				item.iItem = entry;
				item.lParam = (iModule == DOB_PROTOCOL) ? hContact : -hContact;
				item.pszText = pAcc->tszAccountName;
				m_list.InsertItem(&item);
			}
			else m_list.SetItemText(entry, 0, pAcc->tszAccountName);

			m_list.SetItemText(entry, 1, Clist_GetContactDisplayName(hContact));

			wchar_t buffer[2048];
			if ((dtb <= 366) && (dtb >= 0))
				mir_snwprintf(buffer, L"%d", dtb);
			else
				mir_snwprintf(buffer, NA);

			m_list.SetItemText(entry, 2, buffer);
			if ((month != 0) && (day != 0))
				mir_snwprintf(buffer, L"%04d-%02d-%02d", year, month, day);
			else
				mir_snwprintf(buffer, NA);
			m_list.SetItemText(entry, 3, buffer);

			if (age < 400 && age > 0) //hopefully noone lives longer than this :)
				mir_snwprintf(buffer, L"%d", age);
			else
				mir_snwprintf(buffer, NA);
			m_list.SetItemText(entry, 4, buffer);

			m_list.SetItemText(entry, 5, GetBirthdayModule(iModule, hContact));
			res++;
		}
		else if (!bShowAll && !bAdd)
			m_list.DeleteItem(entry);

		return res;
	}

	CCtrlCheck chkShowAll;

public:
	CBirthdaysDlg() :
		CBasicListDlg(IDD_BIRTHDAYS),
		chkShowAll(this, IDC_SHOW_ALL)
	{
		SetMinSize(200, 200);

		chkShowAll.OnChange = Callback(this, &CBirthdaysDlg::onChange_ShowAll);

		m_list.OnColumnClick = Callback(this, &CBirthdaysDlg::onColumnClick);
	}

	bool OnInitDialog() override
	{
		g_pDialog = this;
		Window_SetIcon_IcoLib(m_hwnd, hListMenu);

		m_list.SetExtendedListViewStyleEx(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LVCOLUMN col;
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.pszText = TranslateT("Protocol");
		col.cx = 80;
		m_list.InsertColumn(0, &col);
		
		col.pszText = TranslateT("Contact");
		col.cx = 180;
		m_list.InsertColumn(1, &col);
		
		col.pszText = TranslateT("DTB");
		col.cx = 50;
		m_list.InsertColumn(2, &col);

		col.pszText = TranslateT("Birthday");
		col.cx = 80;
		m_list.InsertColumn(3, &col);
		
		col.pszText = TranslateT("Age");
		col.cx = 50;
		m_list.InsertColumn(4, &col);
		
		col.pszText = TranslateT("Module");
		col.cx = 108;
		m_list.InsertColumn(5, &col);

		LoadBirthdays();
		int column = g_plugin.getByte("SortColumn");
		Sort(column);

		Utils_RestoreWindowPosition(m_hwnd, NULL, MODULENAME, "BirthdayList");
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_SHOW_ALL:
			return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

		case IDC_BIRTHDAYS_LIST:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		}

		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}

	void OnDestroy() override
	{
		g_pDialog = nullptr;
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "BirthdayList");
		Window_FreeIcon_IcoLib(m_hwnd);
		lastColumn = -1;
	}

	void onChange_ShowAll(CCtrlCheck*)
	{
		LoadBirthdays();
	}

	void onColumnClick(CCtrlListView::TEventInfo *ev)
	{
		int column = ev->nmlv->iSubItem;
		g_plugin.setByte("SortColumn", column);
		Sort(column);
		
		lastColumn = (column == lastColumn) ? -1 : column;
	}

	void UpdateContact(MCONTACT hContact)
	{
		LVFINDINFO fi = { 0 };
		fi.flags = LVFI_PARAM;
		fi.lParam = hContact;
		int idx = m_list.FindItem(-1, &fi);

		int iModule;
		if ((int)hContact < 0) {
			iModule = DOB_USERINFO;
			hContact = -hContact;
		}
		else iModule = DOB_PROTOCOL;

		if (-1 == idx)
			UpdateBirthdayEntry(hContact, m_list.GetItemCount(), 1, iModule);
		else
			UpdateBirthdayEntry(hContact, idx, 0, iModule);
		SetBirthdaysCount();
	}
};

INT_PTR ShowListService(WPARAM, LPARAM)
{
	if (!g_pDialog)
		(new CBirthdaysDlg())->Show();
	else
		g_pDialog->Show();
	return 0;
}

void UpdateBirthday(MCONTACT hContact)
{
	if (g_pDialog)
		g_pDialog->UpdateContact(hContact);
}

void CloseBirthdayList()
{
	if (g_pDialog)
		g_pDialog->Close();
}

/////////////////////////////////////////////////////////////////////////////////////////

static class CUpcomingDlg *g_pUpcomingDlg = nullptr;

class CUpcomingDlg : public CBasicListDlg
{
	int timeout;

	CTimer m_timer;

public:
	CUpcomingDlg() :
		CBasicListDlg(IDD_UPCOMING),
		m_timer(this, 1002)
	{
		SetMinSize(400, 160);

		m_timer.OnEvent = Callback(this, &CUpcomingDlg::onTimer);
	}

	bool OnInitDialog() override
	{
		Window_SetIcon_IcoLib(m_hwnd, hListMenu);

		g_pUpcomingDlg = this;
		timeout = g_plugin.cDlgTimeout;

		m_list.SetExtendedListViewStyleEx(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LVCOLUMN col;
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.pszText = TranslateT("Contact");
		col.cx = 300;
		m_list.InsertColumn(0, &col);

		col.pszText = TranslateT("Age");
		col.cx = 45;
		m_list.InsertColumn(1, &col);

		col.pszText = TranslateT("DTB");
		col.cx = 45;
		m_list.InsertColumn(2, &col);

		m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);

		if (timeout > 0)
			m_timer.Start(1000);

		Utils_RestoreWindowPosition(m_hwnd, NULL, MODULENAME, "BirthdayListUpcoming");
		return true;
	}

	void OnDestroy() override
	{
		g_pUpcomingDlg = nullptr;
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "BirthdayListUpcoming");
		Window_FreeIcon_IcoLib(m_hwnd);
		m_timer.Stop();
	}

	void OnResize() override
	{
		RECT rcWin;
		GetWindowRect(m_hwnd, &rcWin);

		int cx = rcWin.right - rcWin.left;
		int cy = rcWin.bottom - rcWin.top;
		SetWindowPos(m_list.GetHwnd(), nullptr, 0, 0, (cx - 30), (cy - 80), (SWP_NOZORDER | SWP_NOMOVE));

		m_list.SetColumnWidth(0, (cx - 150));
		SetWindowPos(GetDlgItem(m_hwnd, IDOK), nullptr, ((cx / 2) - 95), (cy - 67), 0, 0, SWP_NOSIZE);
		RedrawWindow(m_hwnd, nullptr, nullptr, (RDW_FRAME | RDW_INVALIDATE));
	}

	void onTimer(CTimer *)
	{
		const int MAX_SIZE = 512;
		wchar_t buffer[MAX_SIZE];
		timeout--;
		mir_snwprintf(buffer, (timeout != 2) ? TranslateT("Closing in %d seconds") : TranslateT("Closing in %d second"), timeout);
		SetDlgItemText(m_hwnd, IDOK, buffer);

		if (timeout <= 0)
			Close();
	}

	void AddBirthDay(MCONTACT hContact, wchar_t *message, int dtb, int age)
	{
		LVFINDINFO fi = { 0 };
		fi.flags = LVFI_PARAM;
		fi.lParam = (LPARAM)hContact;
		if (-1 != m_list.FindItem(-1, &fi))
			return; /* Allready in list. */

		int index = m_list.GetItemCount();
		LVITEM item = { 0 };
		item.iItem = index;
		item.mask = LVIF_PARAM | LVIF_TEXT;
		item.lParam = (LPARAM)hContact;
		item.pszText = message;
		m_list.InsertItem(&item);

		wchar_t buffer[512];
		mir_snwprintf(buffer, L"%d", age);
		m_list.SetItemText(index, 1, buffer);

		mir_snwprintf(buffer, L"%d", dtb);
		m_list.SetItemText(index, 2, buffer);

		Sort(2);
	}
};

int DialogNotifyBirthday(MCONTACT hContact, int dtb, int age)
{
	wchar_t text[1024];
	BuildDTBText(dtb, Clist_GetContactDisplayName(hContact), text, _countof(text));

	if (!g_pUpcomingDlg) {
		g_pUpcomingDlg = new CUpcomingDlg();
		g_pUpcomingDlg->Show(g_plugin.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}

	g_pUpcomingDlg->AddBirthDay(hContact, text, dtb, age);
	return 0;
}

int DialogNotifyMissedBirthday(MCONTACT hContact, int dab, int age)
{
	wchar_t text[1024];
	BuildDABText(dab, Clist_GetContactDisplayName(hContact), text, _countof(text));

	if (!g_pUpcomingDlg) {
		g_pUpcomingDlg = new CUpcomingDlg();
		g_pUpcomingDlg->Show(g_plugin.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}

	g_pUpcomingDlg->AddBirthDay(hContact, text, -dab, age);
	return 0;
}

void CloseUpcoming()
{
	if (g_pUpcomingDlg)
		g_pUpcomingDlg->Close();
}
