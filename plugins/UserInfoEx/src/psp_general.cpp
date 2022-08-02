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

struct PSPGeneralDlg : public PSPBaseDlg
{
	PSPGeneralDlg() :
		PSPBaseDlg(IDD_CONTACT_GENERAL)
	{
	}

	bool OnInitDialog() override
	{
		PSPBaseDlg::OnInitDialog();

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_TITLE, SET_CONTACT_TITLE, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_FIRSTNAME, SET_CONTACT_FIRSTNAME, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_SECONDNAME, SET_CONTACT_SECONDNAME, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_LASTNAME, SET_CONTACT_LASTNAME, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_NICK, SET_CONTACT_NICK, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_DISPLAYNAME, "CList", SET_CONTACT_MYHANDLE, DBVT_WCHAR));
		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_PARTNER, SET_CONTACT_PARTNER, DBVT_WCHAR));

		UINT nList;
		LPIDSTRLIST pList;
		GetNamePrefixList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_PREFIX, SET_CONTACT_PREFIX, DBVT_BYTE, pList, nList));

		// marital groupbox
		GetMaritalList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_MARITAL, SET_CONTACT_MARITAL, DBVT_BYTE, pList, nList));

		GetLanguageList(&nList, &pList);
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_LANG1, SET_CONTACT_LANG1, DBVT_WCHAR, pList, nList));
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_LANG2, SET_CONTACT_LANG2, DBVT_WCHAR, pList, nList));
		m_ctrlList->insert(CCombo::CreateObj(m_hwnd, EDIT_LANG3, SET_CONTACT_LANG3, DBVT_WCHAR, pList, nList));
		return true;
	}

	bool OnRefresh() override
	{
		if (auto *pszProto = GetBaseProto()) {
			DBVARIANT dbv;
			CCtrlFlags Flags;
			Flags.W = DB::Setting::GetWStringCtrl(m_hContact, USERINFO, USERINFO, pszProto, SET_CONTACT_GENDER, &dbv);
			if (Flags.B.hasCustom || Flags.B.hasProto || Flags.B.hasMeta) {
				if (dbv.type == DBVT_BYTE) {
					CheckDlgButton(m_hwnd, RADIO_FEMALE, (dbv.bVal == 'F') ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton(m_hwnd, RADIO_MALE, (dbv.bVal == 'M') ? BST_CHECKED : BST_UNCHECKED);

					bool bEnable = !m_hContact || Flags.B.hasCustom || !g_plugin.bReadOnly;
					EnableWindow(GetDlgItem(m_hwnd, RADIO_FEMALE), bEnable);
					EnableWindow(GetDlgItem(m_hwnd, RADIO_MALE), bEnable);
				}
				else db_free(&dbv);
			}
		}
		
		return PSPBaseDlg::OnRefresh();
	}

	bool OnApply() override
	{
		auto *pszProto = GetBaseProto();
		if (!pszProto)
			return false;

		// gender
		uint8_t gender = IsDlgButtonChecked(m_hwnd, RADIO_FEMALE) ? 'F' :
			IsDlgButtonChecked(m_hwnd, RADIO_MALE) ? 'M' : 0;

		if (gender)
			db_set_b(m_hContact, m_hContact ? USERINFO : pszProto, SET_CONTACT_GENDER, gender);
		else
			db_unset(m_hContact, m_hContact ? USERINFO : pszProto, SET_CONTACT_GENDER);
		
		return PSPBaseDlg::OnApply();
	}

	void OnIconsChanged() override
	{
		const ICONCTRL idIcon[] = {
			{ IDI_FEMALE,  STM_SETIMAGE, ICO_FEMALE  },
			{ IDI_MALE,    STM_SETIMAGE, ICO_MALE    },
			{ IDI_MARITAL, STM_SETIMAGE, ICO_MARITAL },
		};
		IcoLib_SetCtrlIcons(m_hwnd, idIcon, _countof(idIcon));
	}
};

void InitGeneralDlg(WPARAM wParam, USERINFOPAGE &uip)
{
	uip.position = 0x8000000;
	uip.pDialog = new PSPGeneralDlg();
	uip.dwInitParam = ICONINDEX(IDI_TREE_GENERAL);
	uip.szTitle.w = LPGENW("General");
	g_plugin.addUserInfo(wParam, &uip);
}
