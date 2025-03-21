/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

class COptionsMainDlg : public CTeamsDlgBase
{
	CCtrlEdit m_login, m_password, m_group;

public:
	COptionsMainDlg(CTeamsProto *proto, int idDialog) :
		CTeamsDlgBase(proto, idDialog),
		m_login(this, IDC_LOGIN),
		m_password(this, IDC_PASSWORD),
		m_group(this, IDC_GROUP)
	{
		CreateLink(m_group, proto->m_wstrCListGroup);
	}

	bool OnInitDialog() override
	{
		m_login.SetTextA(ptrA(m_proto->getStringA("Login")));
		m_password.SetTextA(pass_ptrA(m_proto->getStringA("Password")));
		m_login.SendMsg(EM_LIMITTEXT, 128, 0);
		m_password.SendMsg(EM_LIMITTEXT, 128, 0);
		m_group.SendMsg(EM_LIMITTEXT, 64, 0);
		return true;
	}

	bool OnApply() override
	{
		m_proto->setString("Login", ptrA(m_login.GetTextA()));
		m_proto->setString("Password", ptrA(m_password.GetTextA()));
		ptrW group(m_group.GetText());
		if (mir_wstrlen(group) > 0 && !Clist_GroupExists(group))
			Clist_GroupCreate(0, group);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////
// module entry points

MWindow CTeamsProto::OnCreateAccMgrUI(MWindow hwndParent)
{
	auto *pDlg = new COptionsMainDlg(this, IDD_ACCOUNT_MANAGER);
	pDlg->SetParent(hwndParent);
	pDlg->Show();
	return pDlg->GetHwnd();
}

int CTeamsProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new COptionsMainDlg(this, IDD_OPTIONS_MAIN);
	g_plugin.addOptions(wParam, &odp);

	return 0;
}


