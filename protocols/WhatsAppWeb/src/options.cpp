/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public CProtoDlgBase<WhatsAppProto>
{
	CCtrlCheck chkHideChats;
	CCtrlEdit edtGroup;
	ptrW m_wszOldGroup;

public:
	COptionsDlg(WhatsAppProto *ppro, int iDlgID, bool bFullDlg) :
		CProtoDlgBase<WhatsAppProto>(ppro, iDlgID),
		chkHideChats(this, IDC_HIDECHATS),
		edtGroup(this, IDC_DEFGROUP),
		m_wszOldGroup(mir_wstrdup(ppro->m_wszDefaultGroup))
	{
		CreateLink(edtGroup, ppro->m_wszDefaultGroup);

		if (bFullDlg)
			CreateLink(chkHideChats, ppro->m_bHideGroupchats);
	}

	bool OnApply() override
	{
		if (mir_wstrcmp(m_proto->m_wszDefaultGroup, m_wszOldGroup))
			Clist_GroupCreate(0, m_proto->m_wszDefaultGroup);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR WhatsAppProto::SvcCreateAccMgrUI(WPARAM, LPARAM hwndParent)
{
	auto *pDlg = new COptionsDlg(this, IDD_ACCMGRUI, false);
	pDlg->SetParent((HWND)hwndParent);
	pDlg->Create();
	return (INT_PTR)pDlg->GetHwnd();
}

int WhatsAppProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.position = 1;
	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new COptionsDlg(this, IDD_OPTIONS, true);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
