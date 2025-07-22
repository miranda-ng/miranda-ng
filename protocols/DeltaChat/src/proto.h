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

	class CProtoImpl
	{
		friend class CDeltaChatProto;
		CDeltaChatProto &m_proto;

		CTimer m_markRead, m_deleteMsg;

		void OnDeleteMsg(CTimer *)
		{
			// m_proto.SendDeleteMsg();
		}

		void OnMarkRead(CTimer *)
		{
			m_proto.SendMarkRead();
		}

		CProtoImpl(CDeltaChatProto &pro) :
			m_proto(pro),
			m_markRead(Miranda_GetSystemWindow(), UINT_PTR(this)),
			m_deleteMsg(Miranda_GetSystemWindow(), UINT_PTR(this) + 1)
		{
			m_markRead.OnEvent = Callback(this, &CProtoImpl::OnMarkRead);
			m_deleteMsg.OnEvent = Callback(this, &CProtoImpl::OnDeleteMsg);
		}
	} m_impl;

	dc_context_t *m_context = 0;
	dc_event_emitter_t *m_emitter = 0;

	bool IsOnline() const { return m_emitter != nullptr; }
	void Logout();
	void OnConnected();
	
	MCONTACT FindContact(uint32_t chat_id);

	void __cdecl ServerThread(void *);

	// IMAP server
	CMOption<wchar_t *> m_imapUser, m_imapPass, m_imapHost;
	CMOption<int> m_imapPort, m_imapSsl;

	// SMTP server
	CMOption<wchar_t *> m_smtpHost;
	CMOption<int> m_smtpPort, m_smtpSsl;

	// delete messages
	void SendDeleteMessages();

	mir_cs m_csDeleteMsg;
	uint32_t m_deleteChatId = 0;
	std::vector<uint32_t> m_deleteIds;

	// mark messages read
	void SendMarkRead();

	mir_cs m_csMarkRead;
	uint32_t m_markChatId = 0;
	std::vector<uint32_t> m_markIds;

	// menus
	INT_PTR __cdecl OnMenuEnterQR(WPARAM, LPARAM);
	INT_PTR __cdecl OnMenuHandleGrantAuth(WPARAM, LPARAM);

	int  OnPrebuildContactMenu(WPARAM hContact, LPARAM);
	void OnBuildProtoMenu() override;

public:
	CDeltaChatProto(const char*,const wchar_t*);
	~CDeltaChatProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;

	INT_PTR  GetCaps(int, MCONTACT) override;

	void __cdecl EmailSearchThread(void *);
	HANDLE   SearchByEmail(const wchar_t *email) override;

	int      SetStatus(int) override;

	int      SendMsg(MCONTACT hContact, MEVENT, const char *msg) override;

	bool     OnContactDeleted(MCONTACT hContact, uint32_t flags) override;
	void     OnEventDeleted(MCONTACT hContact, MEVENT hDbEvent, int flags) override;
	void     OnMarkRead(MCONTACT hContact, MEVENT hDbEvent) override;

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
