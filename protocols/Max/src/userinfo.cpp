/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"

class CMaxMyProfileDlg : public CUserInfoPageDlg
{
	CMaxProto *m_proto;
	CCtrlEdit m_edtFirst, m_edtLast, m_edtBio;

public:
	CMaxMyProfileDlg(CMaxProto *p) :
		CUserInfoPageDlg(g_plugin, IDD_MYPROFILE),
		m_proto(p),
		m_edtFirst(this, IDC_FIRSTNAME),
		m_edtLast(this, IDC_LASTNAME),
		m_edtBio(this, IDC_BIO)
	{
	}

	bool OnRefresh() override
	{
		m_edtFirst.SetText(m_proto->getMStringW("FirstName"));
		m_edtLast.SetText(m_proto->getMStringW("LastName"));
		m_edtBio.SetText(m_proto->getMStringW("About"));
		return true;
	}

	bool OnApply() override
	{
		ptrW fn(m_edtFirst.GetText());
		ptrW ln(m_edtLast.GetText());
		ptrW bio(m_edtBio.GetText());
		return m_proto->SaveMyProfile(fn, ln, bio);
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_FIRSTNAME:
		case IDC_LASTNAME:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		case IDC_BIO:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}
};

int CMaxProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	// Own profile page only (root "Owner: user details"), same UX style as Telegram.
	if (hContact != 0)
		return 0;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE | ODPF_ICON;
	uip.szProto = m_szModuleName;
	uip.szTitle.w = m_tszUserName;
	uip.dwInitParam = (INT_PTR)Skin_GetProtoIcon(m_szModuleName, ID_STATUS_ONLINE);
	uip.pDialog = new CMaxMyProfileDlg(this);
	return g_plugin.addUserInfo(wParam, &uip);
}

