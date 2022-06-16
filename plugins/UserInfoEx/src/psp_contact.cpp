/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

class PSPContactDlgBase : public PSPBaseDlg
{
	CCtrlEdit homePage;
	CCtrlCombo cmbCountry;
	CCtrlButton btnGoto;

public:
	PSPContactDlgBase() :
		PSPBaseDlg(IDD_CONTACT_ADDRESS),
		btnGoto(this, BTN_GOTO),
		homePage(this, EDIT_HOMEPAGE),
		cmbCountry(this, EDIT_COUNTRY)
	{
		btnGoto.OnClick = Callback(this, &PSPContactDlgBase::onClick_Goto);
		homePage.OnChange = Callback(this, &PSPContactDlgBase::onChange_HomePage);
		cmbCountry.OnSelChanged = Callback(this, &PSPContactDlgBase::onSelChanged_Country);
	}

	bool OnInitDialog() override
	{
		btnGoto.SendMsg(BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open in browser"), MBBF_TCHAR);
		return PSPBaseDlg::OnInitDialog();
	}

	void onClick_Goto(CCtrlButton *)
	{
		Utils_OpenUrlW(ptrW(homePage.GetText()));
	}

	void onChange_HomePage(CCtrlEdit *)
	{
		btnGoto.Enable(mir_wstrlen(ptrW(homePage.GetText())) > 0);
	}

	void onSelChanged_Country(CCtrlCombo *)
	{
		UpdateCountryIcon(cmbCountry);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Home contact info

struct PSPContactHomeDlg : public PSPContactDlgBase
{
	bool OnInitDialog() override
	{
		PSPContactDlgBase::OnInitDialog();

		wchar_t szAddr[MAX_PATH];
		mir_snwprintf(szAddr, L"%s (%s)", TranslateT("Address"), TranslateT("home"));
		SetDlgItemText(m_hwnd, IDC_PAGETITLE, szAddr);

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_STREET, SET_CONTACT_STREET, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_CITY, SET_CONTACT_CITY, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_ZIP, SET_CONTACT_ZIP, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_STATE, SET_CONTACT_STATE, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_HOMEPAGE, SET_CONTACT_HOMEPAGE, DBVT_WCHAR));

		UINT nList;
		LPIDSTRLIST pList;
		GetCountryList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_COUNTRY, SET_CONTACT_COUNTRY, DBVT_WORD, pList, nList));
		return true;
	}

	bool OnRefresh() override
	{
		auto *pszProto = GetBaseProto();
		if (!pszProto)
			return false;

		uint8_t bChanged = 0;

		// phone numbers
		HWND hCtrl = GetDlgItem(m_hwnd, EDIT_PHONE);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_PHONE, TranslateT(SET_CONTACT_PHONE), m_hContact, USERINFO, pszProto, SET_CONTACT_PHONE);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_FAX, TranslateT(SET_CONTACT_FAX), m_hContact, USERINFO, pszProto, SET_CONTACT_FAX);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_CELLULAR, TranslateT(SET_CONTACT_CELLULAR), m_hContact, USERINFO, pszProto, SET_CONTACT_CELLULAR);
		bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, IDI_BTN_CUSTOMPHONE, 0, m_hContact, USERINFO, pszProto, SET_CONTACT_MYPHONE_CAT, SET_CONTACT_MYPHONE_VAL);
		SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);

		// emails
		hCtrl = GetDlgItem(m_hwnd, EDIT_EMAIL);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_EMAIL, TranslateT("Primary e-mail"), m_hContact, USERINFO, pszProto, SET_CONTACT_EMAIL);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_EMAIL, TranslateT("Secondary e-mail"), m_hContact, USERINFO, pszProto, SET_CONTACT_EMAIL0);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_EMAIL, TranslateT("Tertiary e-mail"), m_hContact, USERINFO, pszProto, SET_CONTACT_EMAIL1);
		bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, IDI_BTN_EMAIL, 0, m_hContact, USERINFO, pszProto, SET_CONTACT_MYEMAIL_CAT, SET_CONTACT_MYEMAIL_VAL);
		SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);

		PSPBaseDlg::OnRefresh();
		return bChanged != 0;
	}

	bool OnApply() override
	{
		auto *pszProto = GetBaseProto();
		if (!pszProto)
			return false;

		HWND hCtrl = GetDlgItem(m_hwnd, EDIT_PHONE);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_PHONE);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_FAX);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_CELLULAR);
		CtrlContactWriteMyItemsToDB(hCtrl, 3, m_hContact, USERINFO, pszProto, SET_CONTACT_MYPHONE_CAT, SET_CONTACT_MYPHONE_VAL);
		SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);

		hCtrl = GetDlgItem(m_hwnd, EDIT_EMAIL);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_EMAIL);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_EMAIL0);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_EMAIL1);
		CtrlContactWriteMyItemsToDB(hCtrl, 3, m_hContact, USERINFO, pszProto, SET_CONTACT_MYEMAIL_CAT, SET_CONTACT_MYEMAIL_VAL);
		SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);
		
		return PSPBaseDlg::OnApply();
	}

	void OnIconsChanged() override
	{
		HICON hIcon = g_plugin.getIcon(IDI_BTN_GOTO);
		SendDlgItemMessage(m_hwnd, BTN_GOTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetDlgItemText(m_hwnd, BTN_GOTO, hIcon ? L"" : L"->");

		hIcon = g_plugin.getIcon(IDI_TREE_ADDRESS);
		SendDlgItemMessage(m_hwnd, ICO_ADDRESS, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		ShowWindow(GetDlgItem(m_hwnd, ICO_ADDRESS), hIcon ? SW_SHOW : SW_HIDE);

		SendDlgItemMessage(m_hwnd, EDIT_PHONE, WM_SETICON, NULL, NULL);
		SendDlgItemMessage(m_hwnd, EDIT_EMAIL, WM_SETICON, NULL, NULL);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Work contact info

struct PSPContactWorkDlg : public PSPContactDlgBase
{
	bool OnInitDialog() override
	{
		PSPContactDlgBase::OnInitDialog();

		wchar_t szAddr[MAX_PATH];
		mir_snwprintf(szAddr, L"%s (%s)", TranslateT("Address and contact"), TranslateT("company"));
		SetDlgItemText(m_hwnd, IDC_PAGETITLE, szAddr);

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_STREET, SET_CONTACT_COMPANY_STREET, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_CITY, SET_CONTACT_COMPANY_CITY, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_ZIP, SET_CONTACT_COMPANY_ZIP, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_STATE, SET_CONTACT_COMPANY_STATE, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_HOMEPAGE, SET_CONTACT_COMPANY_HOMEPAGE, DBVT_WCHAR));

		UINT nList;
		LPIDSTRLIST pList;
		GetCountryList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_COUNTRY, SET_CONTACT_COMPANY_COUNTRY, DBVT_WORD, pList, nList));
		return true;
	}

	bool OnApply() override
	{
		auto *pszProto = GetBaseProto();
		if (!pszProto)
			return false;

		HWND hCtrl = GetDlgItem(m_hwnd, EDIT_PHONE);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_PHONE);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_FAX);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_CELLULAR);
		CtrlContactWriteMyItemsToDB(hCtrl, 3, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYPHONE_CAT, SET_CONTACT_COMPANY_MYPHONE_VAL);
		SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);

		hCtrl = GetDlgItem(m_hwnd, EDIT_EMAIL);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL0);
		CtrlContactWriteItemToDB(hCtrl, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL1);
		CtrlContactWriteMyItemsToDB(hCtrl, 3, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYEMAIL_CAT, SET_CONTACT_COMPANY_MYEMAIL_VAL);
		SendMessage(hCtrl, CBEXM_RESETCHANGED, NULL, NULL);
		
		return PSPBaseDlg::OnApply();
	}

	bool OnRefresh() override
	{
		auto *pszProto = GetBaseProto();
		if (!pszProto)
			return false;

		uint8_t bChanged = 0;

		// phone numbers
		HWND hCtrl = GetDlgItem(m_hwnd, EDIT_PHONE);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_PHONE, TranslateT(SET_CONTACT_PHONE), m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_PHONE);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_FAX, TranslateT(SET_CONTACT_FAX), m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_FAX);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_CELLULAR, TranslateT(SET_CONTACT_CELLULAR), m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_CELLULAR);
		bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, IDI_BTN_CUSTOMPHONE, 0, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYPHONE_CAT, SET_CONTACT_COMPANY_MYPHONE_VAL);
		SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);

		// emails
		hCtrl = GetDlgItem(m_hwnd, EDIT_EMAIL);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_EMAIL, TranslateT("Primary e-mail"), m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_EMAIL, TranslateT("Secondary e-mail"), m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL0);
		bChanged |= CtrlContactAddItemFromDB(hCtrl, IDI_BTN_EMAIL, TranslateT("Tertiary e-mail"), m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_EMAIL1);
		bChanged |= CtrlContactAddMyItemsFromDB(hCtrl, IDI_BTN_EMAIL, 0, m_hContact, USERINFO, pszProto, SET_CONTACT_COMPANY_MYEMAIL_CAT, SET_CONTACT_COMPANY_MYEMAIL_VAL);
		SendMessage(hCtrl, CBEXM_SETCURSEL, (WPARAM)-1, TRUE);

		PSPBaseDlg::OnRefresh();
		return bChanged != 0;
	}

	void OnIconsChanged() override
	{
		HICON hIcon = g_plugin.getIcon(IDI_BTN_GOTO);
		SendDlgItemMessage(m_hwnd, BTN_GOTO, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetDlgItemText(m_hwnd, BTN_GOTO, hIcon ? L"" : L"->");

		hIcon = g_plugin.getIcon(IDI_TREE_ADDRESS);
		SendDlgItemMessage(m_hwnd, ICO_ADDRESS, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		ShowWindow(GetDlgItem(m_hwnd, ICO_ADDRESS), hIcon ? SW_SHOW : SW_HIDE);

		SendDlgItemMessage(m_hwnd, EDIT_PHONE, WM_SETICON, NULL, NULL);
		SendDlgItemMessage(m_hwnd, EDIT_EMAIL, WM_SETICON, NULL, NULL);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

void InitContactDlg(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.position = 0x8000001;
	uip.pDialog = new PSPContactHomeDlg();
	uip.dwInitParam = ICONINDEX(IDI_TREE_ADDRESS);
	uip.szTitle.w = LPGENW("General") L"\\" LPGENW("Contact (private)");
	g_plugin.addUserInfo(wParam, &uip);

	uip.position = 0x8000005;
	uip.pDialog = new PSPContactWorkDlg();
	uip.dwInitParam = ICONINDEX(IDI_TREE_ADDRESS);
	uip.szTitle.w = LPGENW("Work") L"\\" LPGENW("Contact (work)");
	g_plugin.addUserInfo(wParam, &uip);
}
