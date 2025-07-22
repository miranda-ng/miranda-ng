/*
Copyright © 2025 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

class CAccountOptionsDlg : public CDeltaChatDlgBase
{
	CCtrlEdit edtImapHost, edtImapUser, edtImapPass, edtSmtpHost;
	CCtrlSpin spinImapPort, spinSmtpPort;
	CCtrlCombo cmbImapSsl, cmbSmtpSsl;

	void InitSslCombo(CCtrlCombo &ctrl)
	{
		ctrl.AddString(TranslateT("Automatic"), DC_SOCKET_AUTO);
		ctrl.AddString(TranslateT("STARTTLS"), DC_SOCKET_STARTTLS);
		ctrl.AddString(TranslateT("SSL (old)"), DC_SOCKET_SSL);
		ctrl.AddString(TranslateT("Plain"), DC_SOCKET_PLAIN);
	}

public:
	CAccountOptionsDlg(CDeltaChatProto *ppro, int iDlgID) :
		CDeltaChatDlgBase(ppro, iDlgID),
		cmbImapSsl(this, IDC_IMAP_SSL),
		cmbSmtpSsl(this, IDC_SMTP_SSL),
		edtImapHost(this, IDC_IMAPHOST),
		edtImapUser(this, IDC_IMAPUSER),
		edtImapPass(this, IDC_IMAPPASS),
		edtSmtpHost(this, IDC_SMTP_HOST),
		spinImapPort(this, IDC_IMAPPORT_SPIN, 10000),
		spinSmtpPort(this, IDC_SMTPPORT_SPIN, 10000)
	{
		CreateLink(cmbImapSsl, ppro->m_imapSsl);
		CreateLink(edtImapHost, ppro->m_imapHost);
		CreateLink(edtImapUser, ppro->m_imapUser);
		CreateLink(edtImapPass, ppro->m_imapPass);
		CreateLink(spinImapPort, ppro->m_imapPort);

		CreateLink(cmbSmtpSsl, ppro->m_smtpSsl);
		CreateLink(edtSmtpHost, ppro->m_smtpHost);
		CreateLink(spinSmtpPort, ppro->m_smtpPort);
	}

	bool OnInitDialog() override
	{
		InitSslCombo(cmbImapSsl);
		InitSslCombo(cmbSmtpSsl);

		cmbImapSsl.SetCurSel(m_proto->m_imapSsl);
		cmbSmtpSsl.SetCurSel(m_proto->m_smtpSsl);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////

int CDeltaChatProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CAccountOptionsDlg(this, IDD_OPTIONS_ACCOUNT);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
