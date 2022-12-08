/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public CProtoDlgBase<CMTProto>
{
	CCtrlCheck chkHideChats, chkUsePopups;
	CCtrlEdit edtGroup, edtPhone;
	ptrW m_wszOldGroup;

public:
	COptionsDlg(CMTProto *ppro, int iDlgID, bool bFullDlg) :
		CProtoDlgBase<CMTProto>(ppro, iDlgID),
		chkUsePopups(this, IDC_POPUPS),
		chkHideChats(this, IDC_HIDECHATS),
		edtPhone(this, IDC_PHONE),
		edtGroup(this, IDC_DEFGROUP),
		m_wszOldGroup(mir_wstrdup(ppro->m_wszDefaultGroup))
	{
		CreateLink(edtPhone, ppro->m_szOwnPhone);
		CreateLink(edtGroup, ppro->m_wszDefaultGroup);

		if (bFullDlg) {
			CreateLink(chkUsePopups, ppro->m_bUsePopups);
			CreateLink(chkHideChats, ppro->m_bHideGroupchats);
		}
	}

	bool OnApply() override
	{
		if (!mir_wstrlen(m_proto->m_szOwnPhone)) {
			SetFocus(edtPhone.GetHwnd());
			return false;
		}

		if (mir_wstrcmp(m_proto->m_wszDefaultGroup, m_wszOldGroup))
			Clist_GroupCreate(0, m_proto->m_wszDefaultGroup);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CMTProto::SvcCreateAccMgrUI(WPARAM, LPARAM hwndParent)
{
	auto *pDlg = new COptionsDlg(this, IDD_ACCMGRUI, false);
	pDlg->SetParent((HWND)hwndParent);
	pDlg->Create();
	return (INT_PTR)pDlg->GetHwnd();
}

int CMTProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.position = 1;
	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new COptionsDlg(this, IDD_OPTIONS, true);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
