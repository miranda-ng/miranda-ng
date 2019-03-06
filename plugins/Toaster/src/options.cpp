#include "stdafx.h"

COptions::COptions()
	: CDlgBase(g_plugin, IDD_OPTIONS_MAIN),
	m_shortcut(this, IDC_SHORTCUT),
	m_preview(this, IDC_PREVIEW),
	m_enabled(this, IDC_CHECK_ENABLED)
{
	m_shortcut.OnClick = Callback(this, &COptions::Shortcut_OnClick);
	m_preview.OnClick = Callback(this, &COptions::Preview_OnClick);
	m_enabled.OnChange = Callback(this, &COptions::Enabled_OnChange);
}

bool COptions::OnInitDialog()
{
	m_enabled.SetState(Popup_Enabled());
	return true;
}

void COptions::Shortcut_OnClick(CCtrlBase*)
{
	if (FAILED(TryCreateShortcut()))
		PUShowMessageW(TranslateT("Failed to create shortcut"), SM_ERROR);
	else
		PUShowMessageW(TranslateT("Shortcut was added to the start menu"), SM_NOTIFY);
}

void COptions::Preview_OnClick(CCtrlBase*)
{
	PUShowMessageW(TranslateT("Information"), SM_NOTIFY);
	PUShowMessageW(TranslateT("Warning"), SM_WARNING);
	PUShowMessageW(TranslateT("Error"), SM_ERROR);
}

void COptions::Enabled_OnChange(CCtrlCheck* chk)
{
	Popup_Enable(chk->GetState());
}

/////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = L"Popups";
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.position = -790000000;
	odp.szTab.w = _T(MODULENAME);
	odp.pDialog = new COptions();
	g_plugin.addOptions(wParam, &odp);

	return 0;
}

