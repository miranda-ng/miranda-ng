/*
Copyright (c) 2025 Miranda NG team (https://miranda-ng.org)

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
	CCtrlEdit m_login, m_group;
	CCtrlCheck m_autosync, m_usehostname, m_usebb;
	CCtrlButton btnLogout;

public:
	COptionsMainDlg(CTeamsProto *proto, int idDialog) :
		CTeamsDlgBase(proto, idDialog),
		m_login(this, IDC_LOGIN),
		m_group(this, IDC_GROUP),
		m_autosync(this, IDC_AUTOSYNC),
		m_usehostname(this, IDC_USEHOST),
		m_usebb(this, IDC_BBCODES),
		btnLogout(this, IDC_LOGOUT)
	{
		CreateLink(m_group, proto->m_wstrCListGroup);
		CreateLink(m_autosync, proto->m_bAutoHistorySync);
		CreateLink(m_usehostname, proto->m_bUseHostnameAsPlace);
		CreateLink(m_usebb, proto->m_bUseBBCodes);

		btnLogout.OnClick = Callback(this, &COptionsMainDlg::onClick_Logout);
	}

	bool OnInitDialog() override
	{
		if (m_proto->getMStringA(DBKEY_RTOKEN).IsEmpty())
			btnLogout.Disable();

		CMStringA szLogin(m_proto->getMStringA(DBKEY_ID));
		if (szLogin.IsEmpty())
			m_login.SetText(TranslateT("<will appear after first login>"));
		else
			m_login.SetTextA(szLogin);
		m_group.SendMsg(EM_LIMITTEXT, 64, 0);
		return true;
	}

	bool OnApply() override
	{
		ptrW group(m_group.GetText());
		if (mir_wstrlen(group) > 0 && !Clist_GroupExists(group))
			Clist_GroupCreate(0, group);
		return true;
	}

	void onClick_Logout(CCtrlButton *)
	{
		m_proto->delSetting(DBKEY_RTOKEN);

		if (m_proto->IsOnline())
			m_proto->SetStatus(ID_STATUS_OFFLINE);

		btnLogout.Disable();
	}
};

/////////////////////////////////////////////////////////////////////////////////

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
