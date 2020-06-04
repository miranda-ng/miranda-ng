/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

void CreateAuthString(char *auth, MCONTACT hContact, CFeedEditor *pDlg)
{
	wchar_t *tlogin = nullptr, *tpass = nullptr;
	if (hContact && g_plugin.getByte(hContact, "UseAuth")) {
		tlogin = g_plugin.getWStringA(hContact, "Login");
		tpass = g_plugin.getWStringA(hContact, "Password");
	}
	else if (pDlg && pDlg->m_useauth.IsChecked()) {
		tlogin = pDlg->m_login.GetText();
		tpass = pDlg->m_password.GetText();
	}
	char *user = mir_u2a(tlogin), *pass = mir_u2a(tpass);

	char str[MAX_PATH];
	int len = mir_snprintf(str, "%s:%s", user, pass);
	mir_free(user);
	mir_free(pass);
	mir_free(tlogin);
	mir_free(tpass);

	mir_snprintf(auth, 250, "Basic %s", ptrA(mir_base64_encode(str, (size_t)len)).get());
}

CAuthRequest::CAuthRequest(CFeedEditor *pDlg, MCONTACT hContact) :
	CDlgBase(g_plugin, IDD_AUTHENTICATION),
	m_feedname(this, IDC_FEEDNAME),
	m_username(this, IDC_FEEDUSERNAME),
	m_password(this, IDC_FEEDPASSWORD)
{
	m_pDlg = pDlg;
	m_hContact = hContact;
}

bool CAuthRequest::OnInitDialog()
{
	if (m_pDlg) {
		ptrW strfeedtitle(m_pDlg->m_feedtitle.GetText());
		
		if (strfeedtitle)
			m_feedname.SetText(strfeedtitle);
		else {
			ptrW strfeedurl(m_pDlg->m_feedurl.GetText());
			m_feedname.SetText(strfeedurl);
		}
	}
	else if (m_hContact) {
		ptrW ptszNick(g_plugin.getWStringA(m_hContact, "Nick"));
		if (!ptszNick)
			ptszNick = g_plugin.getWStringA(m_hContact, "URL");
		if (ptszNick)
			m_feedname.SetText(ptszNick);
	}
	return true;
}

bool CAuthRequest::OnApply()
{
	ptrW strfeedusername(m_username.GetText());
	if (!strfeedusername || mir_wstrcmp(strfeedusername, L"") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter your username"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}
	ptrA strfeedpassword(m_password.GetTextA());
	if (!strfeedpassword || mir_strcmp(strfeedpassword, "") == 0) {
		MessageBox(m_hwnd, TranslateT("Enter your password"), TranslateT("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (m_pDlg) {
		m_pDlg->m_useauth.SetState(1);
		m_pDlg->m_login.Enable(1);
		m_pDlg->m_password.Enable(1);
		m_pDlg->m_login.SetText(strfeedusername);
		m_pDlg->m_password.SetTextA(strfeedpassword);
	}
	else if (m_hContact) {
		g_plugin.setByte(m_hContact, "UseAuth", 1);
		g_plugin.setWString(m_hContact, "Login", strfeedusername);
		g_plugin.setString(m_hContact, "Password", strfeedpassword);
	}
	return true;
}
