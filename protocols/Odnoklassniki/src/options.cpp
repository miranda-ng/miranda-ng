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

class COkAccountDlg : public COkDlgBase
{
	CCtrlEdit edtLogin, edtPassword, edtInterval;

public:
	COkAccountDlg(COdnoklassnikiProto *ppro, int iDlgId) :
		COkDlgBase(ppro, iDlgId),
		edtLogin(this, IDC_LOGIN),
		edtPassword(this, IDC_PASSWORD),
		edtInterval(this, IDC_INTERVAL)
	{
		CreateLink(edtLogin, ppro->m_wszLogin);
		CreateLink(edtPassword, "Password", L"");
		if (iDlgId == IDD_OPTIONS)
			CreateLink(edtInterval, ppro->m_iPollInterval);
	}

	bool OnApply() override
	{
		// another login or password: the saved token belongs to the old one
		if (edtLogin.IsChanged() || edtPassword.IsChanged())
			m_proto->delSetting(DB_KEY_TOKEN);
		return true;
	}
};

MWindow COdnoklassnikiProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	auto *pDlg = new COkAccountDlg(this, IDD_ACCMGRUI);
	pDlg->SetParent(hwndParent);
	pDlg->Show();
	return pDlg->GetHwnd();
}

int COdnoklassnikiProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");
	odp.szTitle.w = m_tszUserName;
	odp.position = 1;
	odp.pDialog = new COkAccountDlg(this, IDD_OPTIONS);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
