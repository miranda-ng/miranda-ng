#include "stdafx.h"

COptionsMain::COptionsMain()
	: CPluginDlgBase(hInstance, IDD_OPTIONS_MAIN, MODULE),
	m_urlAutoSend(this, IDC_URL_AUTOSEND),
	m_urlPasteToMessageInputArea(this, IDC_URL_COPYTOMIA),
	m_urlCopyToClipboard(this, IDC_URL_COPYTOCB),
	m_services(this, IDC_SERVICES), isServiceListInit(false)
{
	CreateLink(m_urlAutoSend, "UrlAutoSend", DBVT_BYTE, 1);
	CreateLink(m_urlPasteToMessageInputArea, "UrlPasteToMessageInputArea", DBVT_BYTE, 0);
	CreateLink(m_urlCopyToClipboard, "UrlCopyToClipboard", DBVT_BYTE, 0);
}

void COptionsMain::OnInitDialog()
{
	CDlgBase::OnInitDialog();

	m_services.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP);

	m_services.AddColumn(0, TranslateT("Account name"), 50);

	size_t count = Services.getCount();
	for (size_t i = 0; i < count; i++) {
		CCloudService *service = Services[i];

		int iItem = m_services.AddItem(mir_wstrdup(service->GetText()), -1, (LPARAM)service);
		//m_services.SetItem(iItem, 1, mir_a2u(service->GetModule));
		int isEnable = db_get_b(NULL, service->GetModule(), "IsEnable", TRUE);
		m_services.SetCheckState(iItem, isEnable);
	}

	m_services.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

	isServiceListInit = true;
}

void COptionsMain::OnApply()
{
	int count = m_services.GetItemCount();
	for (int iItem = 0; iItem < count; iItem++)
	{
		CCloudService *service = (CCloudService*)m_services.GetItemData(iItem);

		int isEnable = m_services.GetCheckState(iItem);
		db_set_b(NULL, service->GetModule(), "IsEnable", isEnable);
	}
}

INT_PTR COptionsMain::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr = (LPNMHDR)lParam;
		if (lpnmHdr->idFrom == (UINT_PTR)m_services.GetCtrlId() && lpnmHdr->code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uChanged & LVIF_STATE && pnmv->uNewState & LVIS_STATEIMAGEMASK)
			{
				if (isServiceListInit)
					NotifyChange();
			}
		}
	}
	break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
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
