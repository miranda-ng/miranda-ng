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

struct PSPEditDlg : public PSPBaseDlg
{
	const char *m_szSetting;

	PSPEditDlg(const char *szSetting) :
		PSPBaseDlg(IDD_CONTACT_ABOUT),
		m_szSetting(szSetting)
	{}

	bool OnInitDialog() override
	{
		PSPBaseDlg::OnInitDialog();

		if (!mir_strcmp(m_szSetting, SET_CONTACT_MYNOTES))
			SetDlgItemText(m_hwnd, IDC_PAGETITLE, TranslateT("My notes:"));
		else
			SetDlgItemText(m_hwnd, IDC_PAGETITLE, TranslateT("About:"));

		m_ctrlList->insert(CEditCtrl::CreateObj(m_hwnd, EDIT_ABOUT, m_szSetting, DBVT_WCHAR));

		// remove static edge in aero mode
		if (IsAeroMode())
			SetWindowLongPtr(GetDlgItem(m_hwnd, EDIT_ABOUT), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, EDIT_ABOUT), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

		SendDlgItemMessage(m_hwnd, EDIT_ABOUT, EM_SETEVENTMASK, 0, /*ENM_KEYEVENTS | */ENM_LINK | ENM_CHANGE);
		SendDlgItemMessage(m_hwnd, EDIT_ABOUT, EM_AUTOURLDETECT, TRUE, NULL);
		if (!m_hContact)
			SendDlgItemMessage(m_hwnd, EDIT_ABOUT, EM_LIMITTEXT, 1024, NULL);
		return true;
	}

	INT_PTR DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg) {
		case WM_NOTIFY:
			// notification handler for richedit control
			switch (((LPNMHDR)lParam)->idFrom) {
			case EDIT_ABOUT:
				// notification handler for a link within the richedit control
				switch (((LPNMHDR)lParam)->code) {
				case EN_LINK:
					return CEditCtrl::GetObj(((LPNMHDR)lParam)->hwndFrom)->LinkNotificationHandler((ENLINK *)lParam);
				}
				return FALSE;
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case EDIT_ABOUT:
				if (HIWORD(wParam) == EN_CHANGE) {
					CBaseCtrl *pResult = CBaseCtrl::GetObj((HWND)lParam);
					if (PtrIsValid(pResult) && (pResult->_cbSize == sizeof(CBaseCtrl)))
						pResult->OnChangedByUser(HIWORD(wParam));
				}
			}
			return FALSE;
		}

		return PSPBaseDlg::DlgProc(uMsg, wParam, lParam);
	}
};

void InitOriginDlg(WPARAM wParam, USERINFOPAGE &uip, bool bReadOnly)
{
	if (!bReadOnly) {
		uip.position = 0x8000006;
		uip.pDialog = new PSPEditDlg(SET_CONTACT_ABOUT);
		uip.dwInitParam = ICONINDEX(IDI_TREE_ABOUT);
		uip.szTitle.w = LPGENW("About");
		g_plugin.addUserInfo(wParam, &uip);

		uip.szTitle.w = LPGENW("About") L"\\" LPGENW("Notes");
	}
	else uip.szTitle.w = LPGENW("Notes");

	uip.position = 0x8000008;
	uip.pDialog = new PSPEditDlg(SET_CONTACT_MYNOTES);
	uip.dwInitParam = ICONINDEX(IDI_TREE_NOTES);
	g_plugin.addUserInfo(wParam, &uip);
}
