// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-22 Miranda NG team
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

	IcqUserInfoDlg(CIcqProto *_ppro) :
		CUserInfoPageDlg(g_plugin, IDD_INFO_ICQ),
		ppro(_ppro)
	{
	}

	bool OnRefresh() override
	{
		SetDlgItemTextW(m_hwnd, IDC_UIN, ppro->GetUserId(m_hContact));
		SetDlgItemTextW(m_hwnd, IDC_NICK, ppro->getMStringW(m_hContact, DB_KEY_ICQNICK));

		SetDlgItemTextA(m_hwnd, IDC_IDLETIME, time2text(ppro->getDword(m_hContact, DB_KEY_IDLE)));
		SetDlgItemTextA(m_hwnd, IDC_LASTSEEN, time2text(ppro->getDword(m_hContact, DB_KEY_LASTSEEN)));
		SetDlgItemTextA(m_hwnd, IDC_MEMBERSINCE, time2text(ppro->getDword(m_hContact, DB_KEY_MEMBERSINCE)));
		SetDlgItemTextA(m_hwnd, IDC_ONLINESINCE, time2text(time(0) - ppro->getDword(m_hContact, DB_KEY_ONLINETS)));
		return false;
	}
};

int CIcqProto::OnUserInfoInit(WPARAM wParam, LPARAM hContact)
{
	if (hContact && mir_strcmp(Proto_GetBaseAccountName(hContact), m_szModuleName))
		return 0;

	if (isChatRoom(hContact))
		return 0;

	USERINFOPAGE uip = {};
	uip.flags = ODPF_UNICODE;
	if (hContact == 0) {
		uip.flags |= ODPF_DONTTRANSLATE;
		uip.szTitle.w = m_tszUserName;
	}
	else uip.szTitle.w = L"ICQ";

	uip.position = -1900000000;
	uip.pDialog = new IcqUserInfoDlg(this);
	g_plugin.addUserInfo(wParam, &uip);
	return 0;
}
