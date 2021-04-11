#include "stdafx.h"

CMOption<bool> g_AAAEnabled(MODULENAME, AAAMODULENAME "_enabled", true);
CMOption<bool> g_KSEnabled(MODULENAME, KSMODULENAME "_enabled", true);
CMOption<bool> g_SSEnabled(MODULENAME, SSMODULENAME "_enabled", true);

/////////////////////////////////////////////////////////////////////////////////////////

class CSubPluginsOptionsDlg : public CDlgBase
{
	CCtrlCheck m_enableKeepStatus;
	CCtrlCheck m_enableStartupStatus;
	CCtrlCheck m_enableAdvancedAutoAway;

public:
	CSubPluginsOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_SUBPLUGINS),
		m_enableKeepStatus(this, IDC_ENABLE_KEEPSTATUS),
		m_enableStartupStatus(this, IDC_ENABLE_STARTUPSTATUS),
		m_enableAdvancedAutoAway(this, IDC_ENABLE_ADVANCEDAUTOAWAY)
	{
		CreateLink(m_enableKeepStatus, g_KSEnabled);
		CreateLink(m_enableStartupStatus, g_SSEnabled);
		CreateLink(m_enableAdvancedAutoAway, g_AAAEnabled);
	}

	bool OnApply() override
	{
		bool bEnabled = m_enableKeepStatus.GetState();
		if (bEnabled != g_KSEnabled) {
			if (bEnabled)
				KeepStatusLoad();
			else
				KeepStatusUnload();
		}

		bEnabled = m_enableStartupStatus.GetState();
		if (bEnabled != g_SSEnabled) {
			if (bEnabled)
				StartupStatusLoad();
			else
				StartupStatusUnload();
		}

		bEnabled = m_enableAdvancedAutoAway.GetState();
		if (bEnabled != g_AAAEnabled) {
			if (bEnabled)
				AdvancedAutoAwayLoad();
			else
				AdvancedAutoAwayUnload();
		}
		return true;
	}
};

int OnCommonOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Status");
	odp.szTitle.w = LPGENW("Status manager");
	odp.pDialog = new CSubPluginsOptionsDlg();
	g_plugin.addOptions(wParam, &odp);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool IsSubPluginEnabled(const char *name)
{
	bool res = IsPluginOnWhiteList(name);
	SetPluginOnWhiteList(name, true);
	return res;
}

void InitCommonOptions()
{
	// if this options dialog was never filled, apply default options
	if (g_plugin.getByte(AAAMODULENAME "_enabled", -1) == -1) {
		g_AAAEnabled = IsSubPluginEnabled(AAAMODULENAME);
		g_KSEnabled = IsSubPluginEnabled(KSMODULENAME);
		g_SSEnabled = IsSubPluginEnabled(SSMODULENAME);
	}
}

