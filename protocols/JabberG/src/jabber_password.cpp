/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2012-18 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_iq.h"
#include "jabber_caps.h"

class ChangePasswordDlg : public CJabberDlgBase
{
	CCtrlEdit edtOldPass, edtPass1, edtPass2;
	
public:
	ChangePasswordDlg(CJabberProto *_ppro) :
		CJabberDlgBase(_ppro, IDD_CHANGEPASSWORD),
		edtOldPass(this, IDC_OLDPASSWD),
		edtPass1(this, IDC_NEWPASSWD),
		edtPass2(this, IDC_NEWPASSWD2)
	{}

	bool OnInitDialog() override
	{
		m_proto->m_hwndJabberChangePassword = m_hwnd;
		Window_SetIcon_IcoLib(m_hwnd, g_GetIconHandle(IDI_KEYS));

		if (m_proto->m_bJabberOnline && m_proto->m_ThreadInfo != nullptr) {
			wchar_t text[1024];
			mir_snwprintf(text, TranslateT("Set New Password for %s@%S"), m_proto->m_ThreadInfo->conn.username, m_proto->m_ThreadInfo->conn.server);
			SetCaption(text);
		}
		return true;
	}

	bool OnApply() override
	{
		if (!m_proto->m_bJabberOnline || m_proto->m_ThreadInfo == nullptr)
			return false;

		ptrW newPass1(edtPass1.GetText()), newPass2(edtPass2.GetText());
		if (mir_wstrcmp(newPass1, newPass2)) {
			MessageBox(m_hwnd, TranslateT("New password does not match."), TranslateT("Change Password"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
			return false;
		}

		if (mir_wstrcmp(ptrW(edtOldPass.GetText()), m_proto->m_ThreadInfo->conn.password)) {
			MessageBox(m_hwnd, TranslateT("Current password is incorrect."), TranslateT("Change Password"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
			return false;
		}
		m_proto->m_ThreadInfo->tszNewPassword = newPass1.detach();

		XmlNodeIq iq(m_proto->AddIQ(&CJabberProto::OnIqResultSetPassword, JABBER_IQ_TYPE_SET, _A2T(m_proto->m_ThreadInfo->conn.server)));
		HXML q = iq << XQUERY(JABBER_FEAT_REGISTER);
		q << XCHILD(L"username", m_proto->m_ThreadInfo->conn.username);
		q << XCHILD(L"password", m_proto->m_ThreadInfo->tszNewPassword);
		m_proto->m_ThreadInfo->send(iq);
		return true;
	}

	void OnDestroy() override
	{
		m_proto->m_hwndJabberChangePassword = nullptr;
		Window_FreeIcon_IcoLib(m_hwnd);
	}
};

INT_PTR __cdecl CJabberProto::OnMenuHandleChangePassword(WPARAM, LPARAM)
{
	if (IsWindow(m_hwndJabberChangePassword))
		SetForegroundWindow(m_hwndJabberChangePassword);
	else
		(new ChangePasswordDlg(this))->Show();

	return 0;
}
