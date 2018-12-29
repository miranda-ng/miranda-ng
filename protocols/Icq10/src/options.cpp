// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

class CIcqOptionsDlg : public CProtoDlgBase<CIcqProto>
{
	CCtrlEdit edtUin, edtPassword;
	CCtrlCheck chkSlowSend;

public:
	CIcqOptionsDlg(CIcqProto *ppro, int iDlgID, bool bFullDlg) :
		CProtoDlgBase<CIcqProto>(ppro, iDlgID),
		edtUin(this, IDC_UIN),
		edtPassword(this, IDC_PASSWORD),
		chkSlowSend(this, IDC_SLOWSEND)
	{
		CreateLink(edtUin, ppro->m_dwUin);
		CreateLink(edtPassword, ppro->m_szPassword);

		if (bFullDlg)
			CreateLink(chkSlowSend, ppro->m_bSlowSend);
	}
};

INT_PTR CIcqProto::CreateAccMgrUI(WPARAM, LPARAM hwndParent)
{
	CIcqOptionsDlg *pDlg = new CIcqOptionsDlg(this, IDD_OPTIONS_ACCMGR, false);
	pDlg->SetParent((HWND)hwndParent);
	pDlg->Create();
	return (INT_PTR)pDlg->GetHwnd();
}

int CIcqProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.position = 1;
//	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CIcqOptionsDlg(this, IDD_OPTIONS_FULL, true);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
