/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

//CSkypeInvideDlg

CSkypeInviteDlg::CSkypeInviteDlg(CSkypeProto *proto) :
	CSkypeDlgBase(proto, IDD_GC_INVITE, false), m_ok(this, IDOK), m_cancel(this, IDCANCEL), m_combo(this, IDC_CONTACT)
{
	m_ok.OnClick = Callback(this, &CSkypeInviteDlg::btnOk_OnOk);
}

void CSkypeInviteDlg::OnInitDialog()
{
	for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) 
	{
		if (!m_proto->isChatRoom(hContact))
		{
			TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
			m_combo.AddString(ptszNick, hContact);
		}
	}
}
void CSkypeInviteDlg::btnOk_OnOk(CCtrlButton*)
{
	m_hContact = m_combo.GetItemData(m_combo.GetCurSel());
	EndDialog(m_hwnd, 1);
}
