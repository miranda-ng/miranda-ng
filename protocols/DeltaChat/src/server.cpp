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

void CDeltaChatProto::Logout()
{
	if (IsOnline()) {
		dc_stop_io(m_context);
		
		if (m_emitter) {
			dc_event_emitter_unref(m_emitter);
			m_emitter = nullptr;
		}
	}

	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iStatus);
}

void CDeltaChatProto::OnConnected()
{
	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	setAllContactStatuses(m_iStatus);
}

void CDeltaChatProto::ServerThread(void *)
{
	debugLogA("entering ServerThread");

	if (!dc_context_is_open(m_context))
		if (!dc_context_open(m_context, ptrA(getStringA(DB_KEY_PASSWORD)))) {
			debugLogA("failed to open context, exiting");
			Logout();
			return;
		}

	dc_set_config(m_context, "addr", m_imapUser);

	dc_set_config(m_context, "mail_server", m_imapHost);
	dc_set_config(m_context, "mail_user", m_imapUser);
	dc_set_config(m_context, "mail_pw", m_imapPass);
	dc_set_config(m_context, "mail_port", m_imapPort);
	dc_set_config(m_context, "mail_security", m_imapSsl);

	dc_set_config(m_context, "send_server", m_smtpHost);
	dc_set_config(m_context, "send_user", m_imapUser);
	dc_set_config(m_context, "send_pw", m_imapPass);
	dc_set_config(m_context, "send_port", m_smtpPort);
	dc_set_config(m_context, "send_security", m_smtpSsl);

	dc_set_config(m_context, "certificate_checks", "Automatic");
	dc_set_config(m_context, "oauth2", "false");
	dc_configure(m_context);

	if (!dc_is_configured(m_context)) {
		debugLogA("context is not properly configured, exiting");
		Logout();
		return;
	}

	dc_start_io(m_context);

	bool bImap = false;

	m_emitter = dc_get_event_emitter(m_context);

	while (m_emitter) {
		auto *event = dc_get_next_event(m_emitter);
		auto id = dc_event_get_id(event), i1 = dc_event_get_data1_int(event), i2 = dc_event_get_data2_int(event);
		auto s1 = dc_event_get_data1_str(event), s2 = dc_event_get_data2_str(event);
		debugLogA("DC event: %d (%d <%s>) (%d <%s>)", id, i1, s1, i2, s2);
		dc_event_unref(event);

		switch (id) {
		case DC_EVENT_IMAP_CONNECTED:
			bImap = true;
			OnConnected();
			break;

		case DC_EVENT_MSGS_CHANGED:
			if (i1 && i2) {
				if (auto hContact = FindContact(i1)) {
					char buf[100];
					itoa(i2, buf, 10);
					ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, HANDLE(i2), LPARAM(buf));
				}
			}
			break;

		case DC_EVENT_MSG_DELIVERED:
			if (i1 && i2) {
				if (auto hContact = FindContact(i1)) {
					char buf[100];
					itoa(i2, buf, 10);
					if (MEVENT hEvent = db_event_getById(m_szModuleName, buf))
						NS_NotifyRemoteRead(hContact, hEvent);
				}
			}
			break;

		case DC_EVENT_MSG_DELETED:
			if (i1 && i2) {
				if (auto hContact = FindContact(i1)) {
					char buf[100];
					itoa(i2, buf, 10);
					if (MEVENT hEvent = db_event_getById(m_szModuleName, buf))
						db_event_delete(hEvent, CDF_FROM_SERVER);
				}
			}
			break;

		case DC_EVENT_INCOMING_MSG:
			if (i1 && i2) {
				if (auto hContact = FindContact(i1)) {
					auto *pMsg = dc_get_msg(m_context, i2);
					char buf[100];
					itoa(i2, buf, 10);

					DB::EventInfo dbei;
					dbei.bSent = false;
					dbei.iTimestamp = dc_msg_get_timestamp(pMsg);
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.pBlob = mir_strdup(dc_msg_get_text(pMsg));
					dbei.cbBlob = (int)mir_strlen(dbei.pBlob);
					dbei.szId = buf;
					ProtoChainRecvMsg(hContact, dbei);

					dc_msg_unref(pMsg);
				}
			}
			break;
		}
	}

	if (m_emitter) {
		dc_event_emitter_unref(m_emitter);
		m_emitter = nullptr;
	}
	debugLogA("leaving ServerThread");
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDeltaChatProto::SendMarkRead()
{
	m_impl.m_markRead.Stop();

	mir_cslock lck(m_csDeleteMsg);
	dc_markseen_msgs(m_context, &*m_markIds.begin(), (int)m_markIds.size());
	m_markIds.clear();
	m_markChatId = 0;
}

void CDeltaChatProto::SendDeleteMessages()
{
	m_impl.m_deleteMsg.Stop();

	mir_cslock lck(m_csDeleteMsg);
	dc_delete_msgs(m_context, &*m_deleteIds.begin(), (int)m_deleteIds.size());
	m_deleteIds.clear();
	m_deleteChatId = 0;
}
