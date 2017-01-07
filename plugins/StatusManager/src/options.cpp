#include "stdafx.h"

CSubPluginsOptionsDlg::CSubPluginsOptionsDlg()
	: CPluginDlgBase(hInst, IDD_OPT_SUBPLUGINS, MODULENAME),
	m_enableKeepStatus(this, IDC_ENABLE_KEEPSTATUS),
	m_enableStartupStatus(this, IDC_ENABLE_STARTUPSTATUS),
	m_enableAdvancedAutoAway(this, IDC_ENABLE_ADVANCEDAUTOAWAY)
{
}

void CSubPluginsOptionsDlg::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	wchar_t text[128];
	mir_snwprintf(text, L"Enable \"%s\" *", L"Keep status");
	m_enableKeepStatus.SetText(text);
	m_enableKeepStatus.SetState(IsSubPluginEnabled(KSMODULENAME));
	mir_snwprintf(text, L"Enable \"%s\" *", L"Startup status");
	m_enableStartupStatus.SetText(text);
	m_enableStartupStatus.SetState(IsSubPluginEnabled(SSMODULENAME));
	mir_snwprintf(text, L"Enable \"%s\" *", L"Advanced auto away");
	m_enableAdvancedAutoAway.SetText(text);
	m_enableAdvancedAutoAway.SetState(IsSubPluginEnabled(AAAMODULENAME));
}

void CSubPluginsOptionsDlg::OnApply()
{
	char setting[128];
	mir_snprintf(setting, "%s_enabled", KSMODULENAME);
	db_set_b(NULL, MODULENAME, setting, m_enableKeepStatus.GetState());
	mir_snprintf(setting, "%s_enabled", SSMODULENAME);
	db_set_b(NULL, MODULENAME, setting, m_enableStartupStatus.GetState());
	mir_snprintf(setting, "%s_enabled", AAAMODULENAME);
	db_set_b(NULL, MODULENAME, setting, m_enableAdvancedAutoAway.GetState());
}

int CSubPluginsOptionsDlg::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Status");
	odp.szTitle.w = LPGENW("Status manager");
	odp.pDialog = CSubPluginsOptionsDlg::CreateOptionsPage();
	Options_AddPage(wParam, &odp);

	return 0;
}