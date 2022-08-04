/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

static LRESULT CALLBACK EditProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_SETCURSOR:
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		return 1;
	}
	return mir_callNextSubclass(hdlg, EditProc, msg, wparam, lparam);
}

struct UserinfoDlg : public CUserInfoPageDlg
{
	UserinfoDlg() :
		CUserInfoPageDlg(g_plugin, IDD_USERINFO)
	{}

	bool OnInitDialog() override
	{
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_INFOTEXT), EditProc);
		WindowList_Add(g_pUserInfo, m_hwnd, m_hContact);
		return true;
	}

	bool OnRefresh() override
	{
		ptrW szout(g_plugin.getWStringA("UserStamp"));
		CMStringW str = ParseString((szout != NULL) ? szout : DEFAULT_USERSTAMP, m_hContact);
		SetDlgItemText(m_hwnd, IDC_INFOTEXT, str);

		if (!mir_wstrcmp(str, TranslateT("<unknown>")))
			EnableWindow(GetDlgItem(m_hwnd, IDC_INFOTEXT), FALSE);
		return false;
	}

	void OnDestroy() override
	{
		WindowList_Remove(g_pUserInfo, m_hwnd);
	}
};

int UserinfoInit(WPARAM wparam, LPARAM hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (IsWatchedProtocol(szProto) && !Contact::IsGroupChat(hContact, szProto)) {
		USERINFOPAGE uip = {};
		uip.flags = ODPF_ICON;
		uip.szTitle.a = LPGEN("Last seen");
		uip.pDialog = new UserinfoDlg();
		uip.dwInitParam = (LPARAM)g_plugin.getIconHandle(IDI_CLOCK);
		g_plugin.addUserInfo(wparam, &uip);
	}
	return 0;
}
