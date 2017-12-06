#include "stdafx.h"

COptionsMain::COptionsMain()
	: CPluginDlgBase(hInstance, IDD_OPTIONS_MAIN, MODULE),
	m_defaultService(this, IDC_DEFAULTSERVICE),
	m_doNothingOnConflict(this, IDC_DONOTHINGONCONFLICT),
	m_renameOnConflict(this, IDC_RENAMEONCONFLICT),
	m_repalceOnConflict(this, IDC_REPLACEONCONFLICT),
	m_urlAutoSend(this, IDC_URL_AUTOSEND),
	m_urlPasteToMessageInputArea(this, IDC_URL_COPYTOMIA),
	m_urlCopyToClipboard(this, IDC_URL_COPYTOCB)
{
	/*CreateLink(m_defaultService, "DefaultService", L"");

	CreateLink(m_renameOnConflict, "RenameOnConflict", DBVT_BYTE, 1);
	CreateLink(m_repalceOnConflict, "RepalceOnConflict", DBVT_BYTE, 0);*/

	CreateLink(m_urlAutoSend, "UrlAutoSend", DBVT_BYTE, 1);
	CreateLink(m_urlPasteToMessageInputArea, "UrlPasteToMessageInputArea", DBVT_BYTE, 0);
	CreateLink(m_urlCopyToClipboard, "UrlCopyToClipboard", DBVT_BYTE, 0);
}

void COptionsMain::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	ptrA defaultService(db_get_sa(NULL, MODULE, "DefaultService"));
	int iItem = m_defaultService.AddString(TranslateT("None"));
	m_defaultService.SetCurSel(iItem);

	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		iItem = m_defaultService.AddString(mir_wstrdup(service->GetText()), (LPARAM)service);
		if (!mir_strcmpi(service->GetModule(), defaultService))
			m_defaultService.SetCurSel(iItem);
	}

	BYTE strategy = db_get_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
	if (strategy == OnConflict::RENAME)
		m_renameOnConflict.SetState(TRUE);
	else if (strategy == OnConflict::REPLACE)
		m_repalceOnConflict.SetState(TRUE);
	else
		m_doNothingOnConflict.SetState(TRUE);
}

void COptionsMain::OnApply()
{
	int iItem = m_defaultService.GetCurSel();
	CCloudService *service = (CCloudService*)m_defaultService.GetItemData(iItem);
	if (service)
		db_set_s(NULL, MODULE, "DefaultService", service->GetModule());
	else
		db_unset(NULL, MODULE, "DefaultService");

	if (m_renameOnConflict.GetState())
		db_set_b(NULL, MODULE, "ConflictStrategy", OnConflict::RENAME);
	else if (m_repalceOnConflict.GetState())
		db_set_b(NULL, MODULE, "ConflictStrategy", OnConflict::REPLACE);
	else
		db_unset(NULL, MODULE, "ConflictStrategy");
}

/////////////////////////////////////////////////////////////////////////////////

int OnOptionsInitialized(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szTitle.w = _A2W(MODULE);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Services");

	//odp.szTab.w = LPGENW("General");
	odp.pDialog = new COptionsMain();
	Options_AddPage(wParam, &odp);

	return 0;
}
