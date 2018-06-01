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

void COptionsMainDlg::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	ptrA defaultService(db_get_sa(NULL, MODULENAME, "DefaultService"));
	int iItem = m_defaultService.AddString(TranslateT("None"));
	m_defaultService.SetCurSel(iItem);

	for (auto &service : Services) {
		iItem = m_defaultService.AddString(mir_wstrdup(service->GetUserName()), (LPARAM)service);
		if (!mir_strcmpi(service->GetAccountName(), defaultService))
			m_defaultService.SetCurSel(iItem);
	}

	BYTE strategy = db_get_b(NULL, MODULENAME, "ConflictStrategy", OnConflict::REPLACE);
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
}

void COptionsMainDlg::OnApply()
{
	int iItem = m_defaultService.GetCurSel();
	CCloudService *service = (CCloudService*)m_defaultService.GetItemData(iItem);
	if (service)
		db_set_s(NULL, MODULENAME, "DefaultService", service->GetAccountName());
	else
		db_unset(NULL, MODULENAME, "DefaultService");

	if (m_renameOnConflict.GetState())
		db_set_b(NULL, MODULENAME, "ConflictStrategy", OnConflict::RENAME);
	else if (m_repalceOnConflict.GetState())
		db_set_b(NULL, MODULENAME, "ConflictStrategy", OnConflict::REPLACE);
	else
		db_unset(NULL, MODULENAME, "ConflictStrategy");
}

/////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
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

void CAccountManagerDlg::OnInitDialog()
{
	ptrA token(m_proto->getStringA("TokenSecret"));
	m_requestAccess.Enable(!token);
	m_revokeAccess.Enable(token);
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
