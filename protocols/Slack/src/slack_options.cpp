#include "stdafx.h"

CSlackOptionsMain::CSlackOptionsMain(CSlackProto *proto, int idDialog)
	: CSuper(proto, idDialog, false),
	m_team(this, IDC_TEAM), m_email(this, IDC_EMAIL),
	m_password(this, IDC_PASSWORD), m_group(this, IDC_GROUP)
{

	CreateLink(m_team, "Team", L"");
	CreateLink(m_email, "Email", L"");
	CreateLink(m_password, "Password", L"");
	CreateLink(m_group, "DefaultGroup", _A2W(MODULE));
}

void CSlackOptionsMain::OnInitDialog()
{
	CSuper::OnInitDialog();

	SendMessage(m_team.GetHwnd(), EM_LIMITTEXT, 21, 0);
	SendMessage(m_email.GetHwnd(), EM_LIMITTEXT, 40, 0);
	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 40, 0);
	SendMessage(m_group.GetHwnd(), EM_LIMITTEXT, 64, 0);
}

void CSlackOptionsMain::OnApply()
{
}

/////////////////////////////////////////////////////////////////////////////////

int CSlackProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = CSlackOptionsMain::CreateOptionsPage(this);
	//Options_AddPage(wParam, &odp);

	return 0;
}
