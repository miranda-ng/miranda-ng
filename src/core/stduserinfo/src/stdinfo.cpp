/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

/////////////////////////////////////////////////////////////////////////////////////////
// Summary dlg page

static DataItem itemsSummary1[] = {
	{ "Nick",          IDC_NICK,      0 },
	{ "FirstName",     IDC_FIRSTNAME, 0 },
	{ "LastName",      IDC_LASTNAME,  0 },
	{ "e-mail",        IDC_EMAIL,     0 },
	{ "Age",           IDC_AGE,       SVS_ZEROISUNSPEC },
	{ "Gender",        IDC_GENDER,    SVS_GENDER },
	{ "MaritalStatus", IDC_MARITAL,   SVS_MARITAL },
	{ "Nick",          IDC_NICK,      0 },
};

static DataItem itemsSummary2[] = {
	{ "BirthDay",      IDC_DOBDAY,    0 },
	{ "BirthMonth",    IDC_DOBMONTH,  SVS_MONTH },
	{ "BirthYear",     IDC_DOBYEAR,   0 },
};

class CSummaryDlg : public CUserInfoPageDlg
{
	CCtrlHyperlink m_email;

public:
	CSummaryDlg() :
		CUserInfoPageDlg(g_plugin, IDD_INFO_SUMMARY),
		m_email(this, IDC_EMAIL)
	{
		m_email.OnClick = Callback(this, &CSummaryDlg::onClick_Email);
	}

	bool IsEmpty() const override
	{
		if (!IsEmptyValue(m_hContact, itemsSummary1))
			return false;
		
		const char *szProto = Proto_GetBaseAccountName(m_hContact);
		const char *szModule = (-1 == db_get_dw(m_hContact, "UserInfo", "BirthDay", -1)) ? szProto : "UserInfo";
		return IsEmptyValue(m_hContact, itemsSummary2, szModule);
	}

	bool OnRefresh() override
	{
		const char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		SetValue(m_hwnd, m_hContact, itemsSummary1);

		const char *szModule = (-1 == db_get_dw(m_hContact, "UserInfo", "BirthDay", -1)) ? szProto : "UserInfo";
		SetValue(m_hwnd, m_hContact, itemsSummary2, szModule);
		return false;
	}

	void onClick_Email(CCtrlHyperlink *pLink)
	{
		if (IsWindowEnabled(pLink->GetHwnd())) {
			wchar_t szExec[264];
			mir_snwprintf(szExec, L"mailto:%s", ptrW(m_email.GetText()).get());
			ShellExecute(m_hwnd, L"open", szExec, NULL, NULL, SW_SHOW);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Location dlg page

static DataItem itemsLocation[] = {
	{ "Street",    IDC_STREET,    SVS_ZEROISUNSPEC },
	{ "City",      IDC_CITY,      SVS_ZEROISUNSPEC },
	{ "State",     IDC_STATE,     SVS_ZEROISUNSPEC },
	{ "ZIP",       IDC_ZIP,       SVS_ZEROISUNSPEC },
	{ "Country",   IDC_COUNTRY,   SVS_COUNTRY      },
	{ "Language1", IDC_LANGUAGE1, SVS_ZEROISUNSPEC },
	{ "Language2", IDC_LANGUAGE2, SVS_ZEROISUNSPEC },
	{ "Language3", IDC_LANGUAGE3, SVS_ZEROISUNSPEC },
	{ "Timezone",  IDC_TIMEZONE,  SVS_TIMEZONE     },
};

class CLocationDlg : public CUserInfoPageDlg
{
	CTimer m_timer;
	CCtrlCombo cmbTimezone;

public:
	CLocationDlg() :
		CUserInfoPageDlg(g_plugin, IDD_INFO_LOCATION),
		m_timer(this, 1),
		cmbTimezone(this, IDC_TIMEZONESELECT)
	{
		m_timer.OnEvent = Callback(this, &CLocationDlg::onTimer);

		cmbTimezone.OnSelChanged = Callback(this, &CLocationDlg::onSelChange_Timezone);
	}

	bool OnInitDialog() override
	{
		m_timer.Start(1000);

		TimeZone_PrepareList(m_hContact, NULL, cmbTimezone.GetHwnd(), TZF_PLF_CB);
		onTimer(0);
		return true;
	}

	void onTimer(CTimer *)
	{
		if (m_hContact == 0)
			return;

		wchar_t szTime[80];
		if (printDateTimeByContact(m_hContact, L"s", szTime, _countof(szTime), TZF_KNOWNONLY)) {
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOCALTIME), FALSE);
			SetDlgItemText(m_hwnd, IDC_LOCALTIME, TranslateT("<not specified>"));
		}
		else {
			EnableWindow(GetDlgItem(m_hwnd, IDC_LOCALTIME), TRUE);
			SetDlgItemText(m_hwnd, IDC_LOCALTIME, szTime);
		}
	}

	bool IsEmpty() const override
	{
		return IsEmptyValue(m_hContact, itemsLocation);
	}
	
	bool OnRefresh() override
	{
		SetValue(m_hwnd, m_hContact, itemsLocation);
		return false;
	}

	void onSelChange_Timezone(CCtrlCombo *)
	{
		TimeZone_StoreListResult(m_hContact, NULL, cmbTimezone.GetHwnd(), TZF_PLF_CB);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Work dlg page

static DataItem itemsWork[] = {
	{ "Company",           IDC_COMPANY,    SVS_ZEROISUNSPEC },
	{ "CompanyDepartment", IDC_DEPARTMENT, SVS_ZEROISUNSPEC },
	{ "CompanyPosition",   IDC_POSITION,   SVS_ZEROISUNSPEC },
	{ "CompanyStreet",     IDC_STREET,     SVS_ZEROISUNSPEC },
	{ "CompanyCity",       IDC_CITY,       SVS_ZEROISUNSPEC },
	{ "CompanyState",      IDC_STATE,      SVS_ZEROISUNSPEC },
	{ "CompanyZIP",        IDC_ZIP,        SVS_ZEROISUNSPEC },
	{ "CompanyCountry",    IDC_COUNTRY,    SVS_COUNTRY      },
	{ "CompanyHomepage",   IDC_WEBPAGE,    SVS_ZEROISUNSPEC },
};

class CWorkDlg : public CUserInfoPageDlg
{
	CCtrlHyperlink m_url;

public:
	CWorkDlg() :
		CUserInfoPageDlg(g_plugin, IDD_INFO_WORK),
		m_url(this, IDC_WEBPAGE)
	{
		m_url.OnClick = Callback(this, &CWorkDlg::onClick_Page);
	}

	bool IsEmpty() const override
	{
		return IsEmptyValue(m_hContact, itemsWork);
	}

	bool OnRefresh() override
	{
		SetValue(m_hwnd, m_hContact, itemsWork);
		return false;
	}

	void onClick_Page(CCtrlHyperlink *pLink)
	{
		if (IsWindowEnabled(pLink->GetHwnd()))
			Utils_OpenUrl(ptrA(pLink->GetTextA()));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Work dlg page

static DataItem itemsBackground1[] = {
	{ "Homepage", IDC_WEBPAGE, SVS_ZEROISUNSPEC },
};

static DataItem itemsBackground2[] = {
	{ "Past0", 0, 0 },
	{ "Past0Text", 0, 0 },
	{ "Affiliation0", 0, 0 },
	{ "Affiliation0Text", 0, 0 },
	{ "Interest0Cat", 0, 0 },
	{ "Interest%dText", 0, 0 },
};

// Resizes all columns in a listview (report style)
// to make all text visible
void ResizeColumns(CCtrlListView &ctrl)
{
	ctrl.SetColumnWidth(0, LVSCW_AUTOSIZE);
	ctrl.SetColumnWidth(1, LVSCW_AUTOSIZE);
}

class CBackgroundDlg : public CUserInfoPageDlg
{
	CCtrlHyperlink m_link;
	CCtrlListView m_past, m_interest;

public:
	CBackgroundDlg() :
		CUserInfoPageDlg(g_plugin, IDD_INFO_BACKGROUND),
		m_link(this, IDC_WEBPAGE),
		m_past(this, IDC_PAST),
		m_interest(this, IDC_INTERESTS)
	{
		m_link.OnClick = Callback(this, &CBackgroundDlg::onClick_Url);
	}

	bool OnInitDialog() override
	{
		RECT rc;
		GetClientRect(m_past.GetHwnd(), &rc);
		rc.right -= GetSystemMetrics(SM_CXVSCROLL);

		LVCOLUMN lvc;
		lvc.mask = LVCF_WIDTH;
		lvc.cx = rc.right / 3;
		m_past.InsertColumn(0, &lvc);
		m_interest.InsertColumn(0, &lvc);

		lvc.cx = rc.right - rc.right / 3;
		m_past.InsertColumn(1, &lvc);
		m_interest.InsertColumn(1, &lvc);

		m_past.SetExtendedListViewStyleEx(LVS_EX_LABELTIP, LVS_EX_LABELTIP);
		m_interest.SetExtendedListViewStyleEx(LVS_EX_LABELTIP, LVS_EX_LABELTIP);
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_PAST:
		case IDC_INTERESTS:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		}

		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	bool IsEmpty() const override
	{
		if (!IsEmptyValue(m_hContact, itemsBackground1))
			return false;

		return IsEmptyValue(m_hContact, itemsBackground2);
	}

	bool OnRefresh() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		SetValue(m_hwnd, m_hContact, itemsBackground1, szProto);

		// past
		m_past.DeleteAllItems();

		char idstr[33];
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;
		lvi.iItem = 0;
		for (int i = 0;; i++) {
			mir_snprintf(idstr, "Past%d", i);
			ptrW tszColText(db_get_wsa(m_hContact, szProto, idstr));
			if (tszColText == NULL)
				break;
			mir_snprintf(idstr, "Past%dText", i);
			ptrW tszText(db_get_wsa(m_hContact, szProto, idstr));
			if (tszText == NULL)
				break;

			lvi.pszText = tszColText;
			m_past.InsertItem(&lvi);
			m_past.SetItemText(lvi.iItem, 1, tszText);
			lvi.iItem++;
		}

		// affiliation
		for (int i = 0;; i++) {
			mir_snprintf(idstr, "Affiliation%d", i);
			ptrW tszColText(db_get_wsa(m_hContact, szProto, idstr));
			if (tszColText == NULL)
				break;
			mir_snprintf(idstr, "Affiliation%dText", i);
			ptrW tszText(db_get_wsa(m_hContact, szProto, idstr));
			if (tszText == NULL)
				break;

			lvi.pszText = tszColText;
			m_past.InsertItem(&lvi);
			m_past.SetItemText(lvi.iItem, 1, tszText);
			lvi.iItem++;
		}

		ResizeColumns(m_past);

		// interests
		m_interest.DeleteAllItems();
		
		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 0;
		lvi.iItem = 0;
		for (int i = 0;; i++) {
			mir_snprintf(idstr, "Interest%dCat", i);
			ptrW tszColText(db_get_wsa(m_hContact, szProto, idstr));
			if (tszColText == NULL)
				break;
			mir_snprintf(idstr, "Interest%dText", i);
			ptrW tszText(db_get_wsa(m_hContact, szProto, idstr));
			if (tszText == NULL)
				break;

			lvi.pszText = tszColText;
			m_interest.InsertItem(&lvi);
			m_interest.SetItemText(lvi.iItem, 1, tszText);
			lvi.iItem++;
		}
		ResizeColumns(m_interest);
		return false;
	}

	void onClick_Url(CCtrlHyperlink *pLink)
	{
		if (IsWindowEnabled(pLink->GetHwnd()))
			Utils_OpenUrl(ptrA(pLink->GetTextA()));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Notes dlg page

static DataItem itemsNotes[] = {
	{ "About", IDC_ABOUT, 0 },
};

class CNotesDlg : public CUserInfoPageDlg
{
	CCtrlEdit mynotes;

public:
	CNotesDlg() :
		CUserInfoPageDlg(g_plugin, IDD_INFO_NOTES),
		mynotes(this, IDC_MYNOTES)
	{
	}

	bool OnInitDialog() override
	{
		HDC hDC = GetDC(m_hwnd);
		LOGFONT lf = {};
		lf.lfHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		ReleaseDC(m_hwnd, hDC);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfWeight = FW_NORMAL;
		mir_wstrcpy(lf.lfFaceName, L"Courier New");
		HFONT hFont = CreateFontIndirect(&lf);
		SendDlgItemMessage(m_hwnd, IDC_ABOUT, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

		ptrW szNotes(g_plugin.getWStringA(m_hContact, "MyNotes"));
		if (szNotes != nullptr)
			mynotes.SetText(szNotes);

		mynotes.SetMaxLength(2048);
		return true;
	}

	bool OnApply() override
	{
		ptrW wszText(mynotes.GetText());
		if (mir_wstrlen(wszText))
			g_plugin.setWString(m_hContact, "MyNotes", wszText);
		else
			g_plugin.delSetting(m_hContact, "MyNotes");

		return true;
	}

	void OnDestroy() override
	{
		DeleteObject((HFONT)SendDlgItemMessage(m_hwnd, IDC_ABOUT, WM_GETFONT, 0, 0));
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_ABOUT:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

		case IDC_MYNOTES:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		}

		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	bool IsEmpty() const override
	{
		return IsEmptyValue(m_hContact, itemsNotes) && db_mc_isSub(m_hContact);
	}

	bool OnRefresh() override
	{
		SetValue(m_hwnd, m_hContact, itemsNotes);
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitContactInfo(WPARAM, USERINFOPAGE &uip);

int DetailsInit(WPARAM wParam, LPARAM lParam)
{
	if (lParam == NULL)
		return 0;

	if (Proto_GetBaseAccountName(lParam) == nullptr)
		return 0;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_ICON;

	uip.pDialog = new CSummaryDlg();
	uip.szTitle.a = LPGEN("Summary");
	uip.dwInitParam = (INT_PTR)g_plugin.getIconHandle(IDI_SUMMARY);
	g_plugin.addUserInfo(wParam, &uip);

	InitContactInfo(wParam, uip);

	uip.pDialog = new CLocationDlg();
	uip.szTitle.a = LPGEN("Location");
	uip.dwInitParam = (INT_PTR)g_plugin.getIconHandle(IDI_LOCATION);
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new CWorkDlg();
	uip.szTitle.a = LPGEN("Work");
	uip.dwInitParam = (INT_PTR)g_plugin.getIconHandle(IDI_WORK);
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new CBackgroundDlg();
	uip.szTitle.a = LPGEN("Background info");
	uip.dwInitParam = 0;
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new CNotesDlg();
	uip.szTitle.a = LPGEN("Notes");
	uip.dwInitParam = (INT_PTR)g_plugin.getIconHandle(IDI_NOTES);
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
