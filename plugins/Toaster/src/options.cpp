#include "stdafx.h"

COptions::COptions()
	: CDlgBase(g_hInstance, IDD_OPTIONS_MAIN),
	m_shortcut(this, IDC_SHORTCUT),
	m_preview(this, IDC_PREVIEW)
{
	m_shortcut.OnClick = Callback(this, &COptions::Shortcut_OnClick);
	m_preview.OnClick = Callback(this, &COptions::Preview_OnClick);
}

void COptions::OnInitDialog()
{
}

void COptions::Shortcut_OnClick(CCtrlBase*)
{
	if (FAILED(TryCreateShortcut()))
	{
		CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)TranslateT("Failed to create shortcut"), (LPARAM)SM_ERROR);
		return;
	}
	CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)TranslateT("Shortcut was added to the start menu"), (LPARAM)SM_NOTIFY);
}

void COptions::Preview_OnClick(CCtrlBase*)
{
	CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)TranslateT("Information"), (LPARAM)SM_NOTIFY);
	CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)TranslateT("Warning"), (LPARAM)SM_WARNING);
	CallService(MS_POPUP_SHOWMESSAGEW, (WPARAM)TranslateT("Error"), (LPARAM)SM_ERROR);
}

/////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.ptszTitle = _T(MODULE);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Popups");

	odp.ptszTab = _T("Main");
	odp.pDialog = new COptions();
	Options_AddPage(wParam, &odp);

	return 0;
}
