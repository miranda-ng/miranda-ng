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

struct PSPCompanyDlg : public PSPBaseDlg
{
	PSPCompanyDlg() :
		PSPBaseDlg(IDD_CONTACT_COMPANY)
	{}

	bool OnInitDialog() override
	{
		PSPBaseDlg::OnInitDialog();

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_COMPANY, SET_CONTACT_COMPANY, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_DEPARTMENT, SET_CONTACT_COMPANY_DEPARTMENT, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_OFFICE, SET_CONTACT_COMPANY_OFFICE, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_POSITION, SET_CONTACT_COMPANY_POSITION, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_SUPERIOR, SET_CONTACT_COMPANY_SUPERIOR, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_ASSISTENT, SET_CONTACT_COMPANY_ASSISTENT, DBVT_WCHAR));

		UINT nList;
		LPIDSTRLIST pList;
		GetOccupationList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_OCCUPATION, SET_CONTACT_COMPANY_OCCUPATION, DBVT_WORD, pList, nList));
		return true;
	}
};

void InitCompanyDlg(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.position = 0x8000004;
	uip.pDialog = new PSPCompanyDlg();
	uip.dwInitParam = ICONINDEX(IDI_TREE_COMPANY);
	uip.szTitle.w = LPGENW("Work");
	g_plugin.addUserInfo(wParam, &uip);
}
