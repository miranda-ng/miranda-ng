// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

class AskAuthProcDlg : public CProtoDlgBase<CIcqProto>
{
	MCONTACT m_hContact;

	CCtrlEdit m_auth;
	CCtrlButton m_btnOk;

public:
	AskAuthProcDlg(CIcqProto *_ppro, MCONTACT _hContact) :
		CProtoDlgBase<CIcqProto>(_ppro, IDD_ASKAUTH, false),
		m_hContact(_hContact),
		m_auth(this, IDC_EDITAUTH),
		m_btnOk(this, IDOK)
	{
		m_autoClose = CLOSE_ON_CANCEL; // let onOk() to close window manually
		m_btnOk.OnClick = Callback(this, &AskAuthProcDlg::onOk);
	}

	virtual void OnInitDialog()
	{
		if (!m_hContact || !m_proto->icqOnline())
			EndDialog(m_hwnd, 0);

		m_auth.SendMsg(EM_LIMITTEXT, 255, 0);
		m_auth.SetText(TranslateT("Please authorize me to add you to my contact list."));
	}

	void onOk(CCtrlButton*)
	{
		if (!m_proto->icqOnline())
			return;

		DWORD dwUin;
		uid_str szUid;
		if (m_proto->getContactUid(m_hContact, &dwUin, &szUid))
			return; // Invalid contact

		char *szReason = GetWindowTextUtf(m_auth.GetHwnd());
		m_proto->icq_sendAuthReqServ(dwUin, szUid, szReason);
		SAFE_FREE((void**)&szReason);

		// auth bug fix (thx Bio)
		if (m_proto->m_bSsiEnabled && dwUin)
			m_proto->resetServContactAuthState(m_hContact);

		EndDialog(m_hwnd, 0);
	}
};

INT_PTR CIcqProto::RequestAuthorization(WPARAM wParam, LPARAM)
{
	AskAuthProcDlg(this, wParam).DoModal();
	return 0;
}
