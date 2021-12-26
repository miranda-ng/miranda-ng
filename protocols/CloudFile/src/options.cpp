#include "stdafx.h"

COptionsMainDlg::COptionsMainDlg()
	: CDlgBase(g_plugin, IDD_OPTIONS_MAIN),
	m_defaultService(this, IDC_DEFAULTSERVICE),
	m_doNothingOnConflict(this, IDC_DONOTHINGONCONFLICT),
	m_renameOnConflict(this, IDC_RENAMEONCONFLICT),
	m_repalceOnConflict(this, IDC_REPLACEONCONFLICT),
	m_urlAutoSend(this, IDC_URL_AUTOSEND),
	m_urlPasteToMessageInputArea(this, IDC_URL_COPYTOMIA),
	m_urlCopyToClipboard(this, IDC_URL_COPYTOCB)
{
	CreateLink(m_defaultService, "DefaultService", L"");

	CreateLink(m_urlAutoSend, "UrlAutoSend", DBVT_BYTE, 1);
	CreateLink(m_urlPasteToMessageInputArea, "UrlPasteToMessageInputArea", DBVT_BYTE, 0);
	CreateLink(m_urlCopyToClipboard, "UrlCopyToClipboard", DBVT_BYTE, 0);
}

bool COptionsMainDlg::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	ptrA defaultService(g_plugin.getStringA("DefaultService"));
	int iItem = m_defaultService.AddString(TranslateT("None"));
	m_defaultService.SetCurSel(iItem);

	for (auto &service : Services) {
		iItem = m_defaultService.AddString(mir_wstrdup(service->GetUserName()), (LPARAM)service);
		if (!mir_strcmpi(service->GetAccountName(), defaultService))
			m_defaultService.SetCurSel(iItem);
	}

	uint8_t strategy = g_plugin.getByte("ConflictStrategy", OnConflict::REPLACE);
	switch (strategy)
	{
	case OnConflict::RENAME:
		m_renameOnConflict.SetState(TRUE);
		m_repalceOnConflict.SetState(FALSE);
		m_doNothingOnConflict.SetState(FALSE);
		break;
	case OnConflict::REPLACE:
		m_renameOnConflict.SetState(FALSE);
		m_repalceOnConflict.SetState(TRUE);
		m_doNothingOnConflict.SetState(FALSE);
		break;
	default:
		m_renameOnConflict.SetState(FALSE);
		m_repalceOnConflict.SetState(FALSE);
		m_doNothingOnConflict.SetState(TRUE);
		break;
	}
	return true;
}

bool COptionsMainDlg::OnApply()
{
	CCloudService *service = (CCloudService*)m_defaultService.GetCurData();
	if (service)
		g_plugin.setString("DefaultService", service->GetAccountName());
	else
		g_plugin.delSetting("DefaultService");

	if (m_renameOnConflict.GetState())
		g_plugin.setByte("ConflictStrategy", OnConflict::RENAME);
	else if (m_repalceOnConflict.GetState())
		g_plugin.setByte("ConflictStrategy", OnConflict::REPLACE);
	else
		g_plugin.delSetting("ConflictStrategy");
	return true;
}

/////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = _A2W(MODULENAME);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Services");

	//odp.szTab.w = LPGENW("General");
	odp.pDialog = new COptionsMainDlg();
	g_plugin.addOptions(wParam, &odp);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////

CAccountManagerDlg::CAccountManagerDlg(CCloudService *service)
	: CProtoDlgBase(service, IDD_ACCMGR),
	m_requestAccess(this, IDC_REQUESTACCESS),
	m_revokeAccess(this, IDC_REVOKEACCESS)
{
	m_requestAccess.OnClick = Callback(this, &CAccountManagerDlg::RequestAccess_OnClick);
	m_revokeAccess.OnClick = Callback(this, &CAccountManagerDlg::RevokeAccess_OnClick);
}

bool CAccountManagerDlg::OnInitDialog()
{
	ptrA token(m_proto->getStringA("TokenSecret"));
	m_requestAccess.Enable(!token);
	m_revokeAccess.Enable(token);
	return true;
}

void CAccountManagerDlg::RequestAccess_OnClick(CCtrlButton*)
{
	m_proto->Login(m_hwnd);
	ptrA token(m_proto->getStringA("TokenSecret"));
	m_requestAccess.Enable(!token);
	m_revokeAccess.Enable(token);
}

void CAccountManagerDlg::RevokeAccess_OnClick(CCtrlButton*)
{
	m_proto->Logout();
	m_requestAccess.Enable();
	m_revokeAccess.Disable();
}

/////////////////////////////////////////////////////////////////////////////////
