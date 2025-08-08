/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Dialog for my own profile in Telegram

class CMyProfileDlg : public CUserInfoPageDlg
{
	CTelegramProto *m_proto;
	CCtrlEdit edtNotes;

public:
	CMyProfileDlg(CTelegramProto *ppro) :
		CUserInfoPageDlg(g_plugin, IDD_MYPROFILE),
		m_proto(ppro),
		edtNotes(this, IDC_BIO)
	{}

	bool OnRefresh() override
	{
		HWND _hwndDate = GetDlgItem(m_hwnd, IDC_BIRTHDAY);
		SYSTEMTIME st = {};
		st.wDay = m_proto->getWord(m_hContact, "BirthDay");
		st.wMonth = m_proto->getWord(m_hContact, "BirthMonth");
		st.wYear = m_proto->getWord(m_hContact, "BirthYear");

		if (st.wDay && st.wMonth && st.wYear) {
			DateTime_SetSystemtime(_hwndDate, GDT_VALID, &st);
			DateTime_SetFormat(_hwndDate, NULL);
		}
		else DateTime_SetFormat(_hwndDate, TranslateT("Unspecified"));

		edtNotes.SetText(m_proto->getMStringW(m_hContact, "About"));
		return true;
	}

	bool OnInitDialog() override
	{
		m_proto->WindowSubscribe(m_hwnd);

		m_proto->GetInfo(0, 0);
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_BIO:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
	}

	void OnDestroy() override
	{
		m_proto->WindowUnsubscribe(m_hwnd);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Channel info dialog

class CChannelInfoDlg : public CUserInfoPageDlg
{
	CMStringA m_szLink;
	CTelegramProto *m_proto;

	CCtrlEdit edtUserCount;
	CCtrlHyperlink m_link;

public:
	CChannelInfoDlg(CTelegramProto *ppro) :
		CUserInfoPageDlg(g_plugin, IDD_CHANNEL_INFO),
		m_proto(ppro),
		m_link(this, IDC_LINK),
		edtUserCount(this, IDC_USER_COUNT)
	{}

	bool OnInitDialog() override
	{
		m_proto->WindowSubscribe(m_hwnd);
		return true;
	}

	int Resizer(UTILRESIZECONTROL *urc) override
	{
		switch (urc->wId) {
		case IDC_LINK:
			return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;
		}
		return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;
	}

	void OnDestroy() override
	{
		m_proto->WindowUnsubscribe(m_hwnd);
	}

	bool OnRefresh() override
	{
		edtUserCount.SetInt(m_proto->getDword(m_hContact, "MemberCount"));

		m_szLink = m_proto->getMStringA(m_hContact, "Link");
		m_link.SetUrl(m_szLink);
		m_link.SetTextA(m_szLink);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

int CTelegramProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE | ODPF_ICON;
	uip.szProto = m_szModuleName;
	uip.szTitle.w = m_tszUserName;
	uip.dwInitParam = (INT_PTR)Skin_GetProtoIcon(m_szModuleName, ID_STATUS_ONLINE);

	if (hContact != 0) {
		// we display this tab only for our contacts
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (!szProto || mir_strcmp(szProto, m_szModuleName))
			return 0;

		auto *pUser = FindUser(GetId(hContact));
		if (pUser && pUser->isChannel)
			uip.pDialog = new CChannelInfoDlg(this);
		else
			return 0;
	}
	else uip.pDialog = new CMyProfileDlg(this);
	return g_plugin.addUserInfo(wParam, &uip);
}
