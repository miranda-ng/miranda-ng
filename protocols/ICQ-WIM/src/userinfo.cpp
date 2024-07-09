// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-24 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

struct IcqUserInfoDlg : public CUserInfoPageDlg
{
	CIcqProto *ppro;
	HANDLE hEvent = 0;
	CCtrlEdit edtFirstName, edtLastName;

	IcqUserInfoDlg(CIcqProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_INFO_ICQ),
		ppro(_ppro),
		edtLastName(this, IDC_LAST_NAME),
		edtFirstName(this, IDC_FIRST_NAME)
	{
	}

	bool OnInitDialog() override
	{
		hEvent = HookEventMessage(ME_DB_CONTACT_SETTINGCHANGED, m_hwnd, WM_USER);
		return true;
	}

	bool OnApply() override
	{
		if (edtFirstName.IsChanged())
			ppro->PatchProfileInfo("firstName", ptrW(edtFirstName.GetText()));
		if (edtLastName.IsChanged())
			ppro->PatchProfileInfo("lastName", ptrW(edtLastName.GetText()));
		return true;
	}

	void OnDestroy() override
	{
		UnhookEvent(hEvent);
	}

	UI_MESSAGE_MAP(IcqUserInfoDlg, CUserInfoPageDlg);
		UI_MESSAGE(WM_USER, OnSettingChanged);
	UI_MESSAGE_MAP_END();

	LRESULT OnSettingChanged(UINT, WPARAM hContact, LPARAM)
	{
		if (hContact == m_hContact)
			OnRefresh();
		return 0;
	}

	bool OnRefresh() override
	{
		SetDlgItemTextW(m_hwnd, IDC_UIN, ppro->GetUserId(m_hContact));
		SetDlgItemTextW(m_hwnd, IDC_NICK, ppro->getMStringW(m_hContact, DB_KEY_ICQNICK));
		SetDlgItemTextW(m_hwnd, IDC_PHONE, ppro->getMStringW(m_hContact, DB_KEY_PHONE));

		edtLastName.SetText(ppro->getMStringW(m_hContact, "LastName"));
		edtFirstName.SetText(ppro->getMStringW(m_hContact, "FirstName"));

		DBVARIANT dbv = {};
		if (!db_get(m_hContact, ppro->m_szModuleName, DB_KEY_LASTSEEN, &dbv)) {
			SetDlgItemTextW(m_hwnd, IDC_LASTSEEN, time2text(&dbv));
			db_free(&dbv);
		}

		if (!db_get(m_hContact, ppro->m_szModuleName, DB_KEY_ONLINETS, &dbv)) {
			SetDlgItemTextW(m_hwnd, IDC_ONLINESINCE, time2text(&dbv));
			db_free(&dbv);
		}

		return false;
	}
};

struct IcqAboutDlg : public CUserInfoPageDlg
{
	CIcqProto *ppro;
	CCtrlEdit edtAbout;

	IcqAboutDlg(CIcqProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_ABOUT),
		ppro(_ppro),
		edtAbout(this, IDC_ABOUT)
	{}

	bool OnRefresh() override
	{
		edtAbout.SetText(ppro->getMStringW(m_hContact, "About"));
		return false;
	}

	bool OnApply() override
	{
		if (edtAbout.IsChanged())
			ppro->PatchProfileInfo("about", ptrW(edtAbout.GetText()));
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		if (urc->wId == IDC_ABOUT)
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}
};

int CIcqProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (hContact && mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	if (isChatRoom(hContact))
		return 0;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
	uip.szGroup.w = m_tszUserName;
	uip.szProto = m_szModuleName;

	uip.szTitle.w = L"MRA";
	uip.position = -1900000000;
	uip.pDialog = new IcqUserInfoDlg(this);
	g_plugin.addUserInfo(wParam, &uip);

	uip.szTitle.w = TranslateT("About");
	uip.position = -1900000001;
	uip.pDialog = new IcqAboutDlg(this);
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
