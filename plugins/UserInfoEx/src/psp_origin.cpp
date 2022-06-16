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

class PSPOriginDlg : public PSPBaseDlg
{
	CTimer timer;
	CCtrlCombo cmbCountry;

public:
	PSPOriginDlg() :
		PSPBaseDlg(IDD_CONTACT_ORIGIN),
		timer(this, 1),
		cmbCountry(this, EDIT_COUNTRY)
	{
		timer.OnEvent = Callback(this, &PSPOriginDlg::onTimer);
		cmbCountry.OnSelChanged = Callback(this, &PSPOriginDlg::onSelChanged_Country);
	}

	bool OnInitDialog() override
	{
		PSPBaseDlg::OnInitDialog();

		timer.Start(5000);

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_STREET, SET_CONTACT_ORIGIN_STREET, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_ZIP, SET_CONTACT_ORIGIN_ZIP, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_CITY, SET_CONTACT_ORIGIN_CITY, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_STATE, SET_CONTACT_ORIGIN_STATE, DBVT_WCHAR));

		UINT nList;
		LPIDSTRLIST pList;
		GetCountryList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_COUNTRY, SET_CONTACT_ORIGIN_COUNTRY, DBVT_WORD, pList, nList));

		m_ctrlList->insert(CTzCombo::CreateObj(m_hwnd, EDIT_TIMEZONE, nullptr));
		return true;
	}

	bool OnRefresh() override
	{
		auto *pszProto = GetBaseProto();
		if (!pszProto || !m_hContact)
			return false;

		MTime mt;
		if (mt.DBGetStamp(m_hContact, USERINFO, SET_CONTACT_ADDEDTIME) && strstr(pszProto, "ICQ")) {
			uint32_t dwStamp;

			dwStamp = DB::Contact::WhenAdded(db_get_dw(m_hContact, pszProto, "UIN", 0), pszProto);
			if (dwStamp > 0)
				mt.FromStampAsUTC(dwStamp);
		}
		if (mt.IsValid()) {
			wchar_t szTime[MAX_PATH];
			LPTSTR ptr;

			mt.UTCToLocal();
			mt.DateFormatLong(szTime, _countof(szTime));

			mir_wstrcat(szTime, L" - ");
			ptr = szTime + mir_wstrlen(szTime);
			mt.TimeFormat(ptr, _countof(szTime) - (ptr - szTime));
			SetDlgItemText(m_hwnd, TXT_DATEADDED, szTime);
		}
		
		return PSPBaseDlg::OnRefresh();
	}

	void OnIconsChanged() override
	{
		const ICONCTRL idIcon[] = {
			{ IDI_TREE_ADDRESS, STM_SETIMAGE, ICO_ADDRESS },
			{ IDI_CLOCK,        STM_SETIMAGE, ICO_CLOCK },
		};

		IcoLib_SetCtrlIcons(m_hwnd, idIcon, _countof(idIcon));
	}

	void onTimer(CTimer *)
	{
		wchar_t szTime[32];
		CTzCombo::GetObj(m_hwnd, EDIT_TIMEZONE)->GetTime(szTime, _countof(szTime));
		SetDlgItemText(m_hwnd, TXT_TIME, szTime);
	}

	void onSelChanged_Country(CCtrlCombo *)
	{
		UpdateCountryIcon(cmbCountry);
	}
};

void InitOriginDlg(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.position = 0x8000002;
	uip.pDialog = new PSPOriginDlg();
	uip.dwInitParam = ICONINDEX(IDI_TREE_ADVANCED);
	uip.szTitle.w = LPGENW("General") L"\\" LPGENW("Origin");
	g_plugin.addUserInfo(wParam, &uip);
}
