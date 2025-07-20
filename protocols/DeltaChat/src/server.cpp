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
	dc_stop_io(m_context);

	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iStatus);
}

void CDeltaChatProto::SendLogout()
{}

void CDeltaChatProto::ServerThread(void *)
{
	if (!dc_context_is_open(m_context))
		if (!dc_context_open(m_context, ptrA(getStringA(DB_KEY_ID)))) {
			debugLogA("failed to open context, exiting");
			Logout();
			return;
		}

	dc_set_config(m_context, "addr", T2Utf(m_imapUser));
	dc_set_config(m_context, "mail_server", T2Utf(m_imapHost));
	dc_set_config(m_context, "mail_user", T2Utf(m_imapUser));
	dc_set_config(m_context, "mail_pw", T2Utf(m_imapPass));

	dc_set_config(m_context, "send_server", "");
	dc_set_config(m_context, "send_user", T2Utf(m_imapUser));
	dc_set_config(m_context, "send_pw", T2Utf(m_imapPass));

	dc_set_config(m_context, "certificate_checks", "Automatic");
	dc_set_config(m_context, "oauth2", "false");

	char buf[100];
	itoa(m_imapPort, buf, 10);
	dc_set_config(m_context, "mail_port", buf);
	dc_configure(m_context);
	HandleEvents();

	if (!dc_is_configured(m_context)) {
		debugLogA("context is not properly configured, exiting");
		Logout();
		return;
	}

	int oldStatus = m_iStatus;
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	dc_start_io(m_context);
}
