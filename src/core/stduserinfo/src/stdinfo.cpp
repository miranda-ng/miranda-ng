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

#define SVS_NORMAL        0
#define SVS_GENDER        1
#define SVS_ZEROISUNSPEC  2
#define SVS_IP            3
#define SVS_COUNTRY       4
#define SVS_MONTH         5
#define SVS_SIGNED        6
#define SVS_TIMEZONE      7
#define SVS_MARITAL    	  8

static void SetValue(HWND m_hwnd, int idCtrl, MCONTACT hContact, char *szModule, char *szSetting, int special)
{
	char *pstr = nullptr;
	wchar_t *pwstr = nullptr, wstr[80];

	DBVARIANT dbv = { DBVT_DELETED };

	int unspecified;
	if (szModule == nullptr)
		unspecified = 1;
	else
		unspecified = db_get_s(hContact, szModule, szSetting, &dbv, 0);

	if (!unspecified) {
		switch (dbv.type) {
		case DBVT_BYTE:
			if (special == SVS_GENDER) {
				if (dbv.cVal == 'M') pwstr = TranslateT("Male");
				else if (dbv.cVal == 'F') pwstr = TranslateT("Female");
				else unspecified = 1;
			}
			else if (special == SVS_MONTH) {
				if (dbv.bVal > 0 && dbv.bVal <= 12) {
					pwstr = wstr;
					GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SABBREVMONTHNAME1 - 1 + dbv.bVal, wstr, _countof(wstr));
				}
				else unspecified = 1;
			}
			else if (special == SVS_TIMEZONE) {
				if (dbv.cVal == -100) unspecified = 1;
				else {
					pwstr = wstr;
					mir_snwprintf(wstr, dbv.cVal ? L"UTC%+d:%02d" : L"UTC", -dbv.cVal / 2, (dbv.cVal & 1) * 30);
				}
			}
			else if (special == SVS_MARITAL) {
				switch (dbv.cVal) {
				case 0:
					pwstr = TranslateT("<not specified>");
					break;
				case 10:
					pwstr = TranslateT("Single");
					break;
				case 11:
					pwstr = TranslateT("Close relationships");
					break;
				case 12:
					pwstr = TranslateT("Engaged");
					break;
				case 20:
					pwstr = TranslateT("Married");
					break;
				case 30:
					pwstr = TranslateT("Divorced");
					break;
				case 31:
					pwstr = TranslateT("Separated");
					break;
				case 40:
					pwstr = TranslateT("Widowed");
					break;
				case 50:
					pwstr = TranslateT("Actively searching");
					break;
				case 60:
					pwstr = TranslateT("In love");
					break;
				case 70:
					pwstr = TranslateT("It's complicated");
					break;
				case 80:
					pwstr = TranslateT("In a civil union");
					break;
				default:
					unspecified = 1;
				}
			}
			else {
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.bVal == 0);
				pwstr = _itow(special == SVS_SIGNED ? dbv.cVal : dbv.bVal, wstr, 10);
			}
			break;

		case DBVT_WORD:
			if (special == SVS_COUNTRY) {
				uint16_t wSave = dbv.wVal;
				if (wSave == (uint16_t)-1) {
					char szSettingName[100];
					mir_snprintf(szSettingName, "%sName", szSetting);
					if (!db_get_ws(hContact, szModule, szSettingName, &dbv)) {
						pwstr = dbv.pwszVal;
						unspecified = false;
						break;
					}
				}

				pwstr = TranslateW(_A2T((char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, wSave, 0)));
				unspecified = pwstr == nullptr;
			}
			else {
				unspecified = (special == SVS_ZEROISUNSPEC && dbv.wVal == 0);
				pwstr = _itow(special == SVS_SIGNED ? dbv.sVal : dbv.wVal, wstr, 10);
			}
			break;

		case DBVT_DWORD:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.dVal == 0);
			if (special == SVS_IP) {
				struct in_addr ia;
				ia.S_un.S_addr = htonl(dbv.dVal);
				mir_wstrncpy(wstr, _A2T(inet_ntoa(ia)), _countof(wstr));
				pwstr = wstr;
				if (dbv.dVal == 0)
					unspecified = 1;
			}
			else pwstr = _itow(special == SVS_SIGNED ? dbv.lVal : dbv.dVal, wstr, 10);
			break;

		case DBVT_ASCIIZ:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			pstr = dbv.pszVal;
			break;

		case DBVT_UTF8:
			unspecified = (special == SVS_ZEROISUNSPEC && dbv.pszVal[0] == '\0');
			if (!unspecified) {
				SetDlgItemTextW(m_hwnd, idCtrl, TranslateW(ptrW(mir_utf8decodeW(dbv.pszVal))));
				goto LBL_Exit;
			}

			pstr = dbv.pszVal;
			mir_utf8decode(dbv.pszVal, nullptr);
			break;

		default:
			pwstr = wstr;
			mir_wstrcpy(wstr, L"???");
			break;
		}
	}

	if (unspecified)
		SetDlgItemText(m_hwnd, idCtrl, TranslateT("<not specified>"));
	else if (pwstr != nullptr)
		SetDlgItemText(m_hwnd, idCtrl, pwstr);
	else
		SetDlgItemTextA(m_hwnd, idCtrl, pstr);

LBL_Exit:
	EnableWindow(GetDlgItem(m_hwnd, idCtrl), !unspecified);
	db_free(&dbv);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Summary dlg page

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

	bool OnRefresh() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		SetValue(m_hwnd, IDC_NICK, m_hContact, szProto, "Nick", 0);
		SetValue(m_hwnd, IDC_FIRSTNAME, m_hContact, szProto, "FirstName", 0);
		SetValue(m_hwnd, IDC_LASTNAME, m_hContact, szProto, "LastName", 0);
		SetValue(m_hwnd, IDC_EMAIL, m_hContact, szProto, "e-mail", 0);
		SetValue(m_hwnd, IDC_AGE, m_hContact, szProto, "Age", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_GENDER, m_hContact, szProto, "Gender", SVS_GENDER);
		SetValue(m_hwnd, IDC_DOBDAY, m_hContact, szProto, "BirthDay", 0);
		SetValue(m_hwnd, IDC_DOBMONTH, m_hContact, szProto, "BirthMonth", SVS_MONTH);
		SetValue(m_hwnd, IDC_DOBYEAR, m_hContact, szProto, "BirthYear", 0);
		SetValue(m_hwnd, IDC_MARITAL, m_hContact, szProto, "MaritalStatus", SVS_MARITAL);
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

	bool OnRefresh() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		SetValue(m_hwnd, IDC_STREET, m_hContact, szProto, "Street", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_CITY, m_hContact, szProto, "City", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_STATE, m_hContact, szProto, "State", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_ZIP, m_hContact, szProto, "ZIP", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_COUNTRY, m_hContact, szProto, "Country", SVS_COUNTRY);
		SetValue(m_hwnd, IDC_LANGUAGE1, m_hContact, szProto, "Language1", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_LANGUAGE2, m_hContact, szProto, "Language2", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_LANGUAGE3, m_hContact, szProto, "Language3", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_TIMEZONE, m_hContact, szProto, "Timezone", SVS_TIMEZONE);
		return false;
	}

	void onSelChange_Timezone(CCtrlCombo *)
	{
		TimeZone_StoreListResult(m_hContact, NULL, cmbTimezone.GetHwnd(), TZF_PLF_CB);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Work dlg page

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

	bool OnRefresh() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		SetValue(m_hwnd, IDC_COMPANY, m_hContact, szProto, "Company", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_DEPARTMENT, m_hContact, szProto, "CompanyDepartment", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_POSITION, m_hContact, szProto, "CompanyPosition", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_STREET, m_hContact, szProto, "CompanyStreet", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_CITY, m_hContact, szProto, "CompanyCity", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_STATE, m_hContact, szProto, "CompanyState", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_ZIP, m_hContact, szProto, "CompanyZIP", SVS_ZEROISUNSPEC);
		SetValue(m_hwnd, IDC_COUNTRY, m_hContact, szProto, "CompanyCountry", SVS_COUNTRY);
		SetValue(m_hwnd, IDC_WEBPAGE, m_hContact, szProto, "CompanyHomepage", SVS_ZEROISUNSPEC);
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

	bool OnRefresh() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto == nullptr)
			return false;

		SetValue(m_hwnd, IDC_WEBPAGE, m_hContact, szProto, "Homepage", SVS_ZEROISUNSPEC);

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

	bool OnRefresh() override
	{
		char *szProto = Proto_GetBaseAccountName(m_hContact);
		if (szProto != NULL)
			SetValue(m_hwnd, IDC_ABOUT, m_hContact, szProto, "About", 0);
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
	uip.pDialog = new CSummaryDlg();
	uip.szGroup.a = LPGEN("General");
	uip.szTitle.a = LPGEN("Summary");
	g_plugin.addUserInfo(wParam, &uip);

	InitContactInfo(wParam, uip);

	uip.pDialog = new CLocationDlg();
	uip.szTitle.a = LPGEN("Location");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new CWorkDlg();
	uip.szTitle.a = LPGEN("Work");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new CBackgroundDlg();
	uip.szTitle.a = LPGEN("Background info");
	g_plugin.addUserInfo(wParam, &uip);

	uip.pDialog = new CNotesDlg();
	uip.szTitle.a = LPGEN("Notes");
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
