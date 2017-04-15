#include "stdafx.h"

COptionsMain::COptionsMain()
	: CPluginDlgBase(hInstance, IDD_OPTIONS_MAIN, MODULE),
	m_urlAutoSend(this, IDC_URL_AUTOSEND),
	m_urlPasteToMessageInputArea(this, IDC_URL_COPYTOMIA),
	m_urlCopyToClipboard(this, IDC_URL_COPYTOCB)
{
	CreateLink(m_urlAutoSend, "UrlAutoSend", DBVT_BYTE, 1);
	CreateLink(m_urlPasteToMessageInputArea, "UrlPasteToMessageInputArea", DBVT_BYTE, 0);
	CreateLink(m_urlCopyToClipboard, "UrlCopyToClipboard", DBVT_BYTE, 0);
}

void COptionsMain::OnInitDialog()
{
	CDlgBase::OnInitDialog();
}

/////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szTitle.w = _A2W(MODULE);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("General");
	odp.pDialog = new COptionsMain();
	Options_AddPage(wParam, &odp);

	return 0;
}
