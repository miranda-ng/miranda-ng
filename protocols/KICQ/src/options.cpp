/*
Copyright (C) 2026 Miranda NG team

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

class CKicqAccountDlg : public CKicqDlgBase
{
	CCtrlEdit edtUin, edtPassword, edtServer, edtPort, edtGroup;

public:
	CKicqAccountDlg(CKicqProto *ppro, int iDlgId) :
		CKicqDlgBase(ppro, iDlgId),
		edtUin(this, IDC_UIN),
		edtPassword(this, IDC_PASSWORD),
		edtServer(this, IDC_SERVER),
		edtPort(this, IDC_PORT),
		edtGroup(this, IDC_GROUP)
	{
		CreateLink(edtUin, "UIN", DBVT_DWORD, 0);
		CreateLink(edtPassword, "Password", L"");
		CreateLink(edtServer, ppro->m_wszServer);
		CreateLink(edtPort, ppro->m_iPort);
		CreateLink(edtGroup, ppro->m_wszDefaultGroup);
	}

	bool OnInitDialog() override
	{
		// the XOR login of that ICQ era only uses the first 8 characters
		edtPassword.SendMsg(EM_LIMITTEXT, 8, 0);
		edtUin.SendMsg(EM_LIMITTEXT, 10, 0);
		return true;
	}
};

MWindow CKicqProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	auto *pDlg = new CKicqAccountDlg(this, IDD_ACCMGRUI);
	pDlg->SetParent(hwndParent);
	pDlg->Show();
	return pDlg->GetHwnd();
}

int CKicqProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.position = 1;
	odp.pDialog = new CKicqAccountDlg(this, IDD_OPTIONS);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
