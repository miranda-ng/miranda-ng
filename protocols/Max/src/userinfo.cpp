/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"

class CMaxMyProfileDlg : public CUserInfoPageDlg
{
	CMaxProto *m_proto;
	CCtrlEdit m_edtBio;

public:
	CMaxMyProfileDlg(CMaxProto *p) :
		CUserInfoPageDlg(g_plugin, IDD_MYPROFILE),
		m_proto(p),
		m_edtBio(this, IDC_BIO)
	{
	}

	bool OnRefresh() override
	{
		m_edtBio.SetText(m_proto->getMStringW("About"));
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		if (urc->wId == IDC_BIO)
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
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

