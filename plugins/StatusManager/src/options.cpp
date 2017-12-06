#include "stdafx.h"

bool IsSubPluginEnabled(const char *name)
{
	// Check if this plugin was disabled as separate dll
	CMStringA dllName(FORMAT, "%s.dll", name);
	dllName.MakeLower();
	int dllEnabled = !db_get_b(0, "PluginDisable", dllName);

	char setting[128];
	mir_snprintf(setting, "%s_enabled", name);
	return db_get_b(0, MODULENAME, setting, dllEnabled) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CSubPluginsOptionsDlg::CSubPluginsOptionsDlg() :
	CPluginDlgBase(hInst, IDD_OPT_SUBPLUGINS, MODULENAME),
	m_enableKeepStatus(this, IDC_ENABLE_KEEPSTATUS),
	m_enableStartupStatus(this, IDC_ENABLE_STARTUPSTATUS),
	m_enableAdvancedAutoAway(this, IDC_ENABLE_ADVANCEDAUTOAWAY)
{
}

void CSubPluginsOptionsDlg::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_enableKeepStatus.SetState(IsSubPluginEnabled(KSMODULENAME));
	m_enableStartupStatus.SetState(IsSubPluginEnabled(SSMODULENAME));
	m_enableAdvancedAutoAway.SetState(IsSubPluginEnabled(AAAMODULENAME));
}

void CSubPluginsOptionsDlg::OnApply()
{
	char setting[128];
	mir_snprintf(setting, "%s_enabled", KSMODULENAME);
	db_set_b(0, MODULENAME, setting, m_enableKeepStatus.GetState());
	mir_snprintf(setting, "%s_enabled", SSMODULENAME);
	db_set_b(0, MODULENAME, setting, m_enableStartupStatus.GetState());
	mir_snprintf(setting, "%s_enabled", AAAMODULENAME);
	db_set_b(0, MODULENAME, setting, m_enableAdvancedAutoAway.GetState());
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