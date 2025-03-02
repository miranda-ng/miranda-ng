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
		m_proto->RetrieveProfile(this);
		return true;
	}

	bool OnInitDialog() override
	{
		m_proto->WindowSubscribe(m_hwnd);

		OnRefresh();
		Display();
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

	void Display()
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
		else {
			DateTime_SetSystemtime(_hwndDate, GDT_NONE, NULL);
			DateTime_SetFormat(_hwndDate, TranslateT("Unspecified"));
		}

		edtNotes.SetText(m_proto->getMStringW(m_hContact, "Notes"));
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

void CTelegramProto::RetrieveProfile(CMyProfileDlg *pDlg)
{
	SendQuery(new TD::getUserFullInfo(m_iOwnId), &CTelegramProto::OnReceivedProfile, pDlg);
}

void CTelegramProto::OnReceivedProfile(td::ClientManager::Response &response, void *pUserData)
{
	if (!response.object)
		return;

	if (response.object->get_id() != TD::userFullInfo::ID) {
		debugLogA("Gotten class ID %d instead of %d, exiting", response.object->get_id(), TD::updateUserFullInfo::ID);
		return;
	}

	ProcessUserInfo(m_iOwnId, (TD::userFullInfo *)response.object.get());
	((CMyProfileDlg *)pUserData)->Display();
}

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

int CTelegramProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (hContact != 0)
		return 0;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE | ODPF_ICON;
	uip.szProto = m_szModuleName;
	uip.szTitle.w = m_tszUserName;
	uip.pDialog = new CMyProfileDlg(this);
	uip.dwInitParam = (INT_PTR)Skin_GetProtoIcon(m_szModuleName, ID_STATUS_ONLINE);
	return g_plugin.addUserInfo(wParam, &uip);
}
