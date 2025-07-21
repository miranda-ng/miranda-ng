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

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////

class CDeltaChatProto : public PROTO<CDeltaChatProto>
{
	friend class CAccountOptionsDlg;

	dc_context_t *m_context = 0;
	dc_event_emitter_t *m_emitter = 0;

	bool IsOnline() const { return m_emitter != nullptr; }
	void Logout();
	void OnConnected();

	void __cdecl ServerThread(void *);

	// IMAP server
	CMOption<wchar_t *> m_imapUser, m_imapPass, m_imapHost;
	CMOption<int> m_imapPort, m_imapSsl;

	// SMTP server
	CMOption<wchar_t *> m_smtpHost;
	CMOption<int> m_smtpPort, m_smtpSsl;

public:
	CDeltaChatProto(const char*,const wchar_t*);
	~CDeltaChatProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	INT_PTR  GetCaps(int, MCONTACT) override;

	int      SetStatus(int) override;

	//////////////////////////////////////////////////////////////////////////////////////

	int  OnOptionsInit(WPARAM, LPARAM);
};

typedef CProtoDlgBase<CDeltaChatProto> CDeltaChatDlgBase;

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public ACCPROTOPLUGIN<CDeltaChatProto>
{
	CMPlugin();

	int Load() override;
};
