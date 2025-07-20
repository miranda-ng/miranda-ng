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

CDeltaChatProto::CDeltaChatProto(const char *szModuleName, const wchar_t *wszUserName) :
	PROTO<CDeltaChatProto>(szModuleName, wszUserName),
	m_imapUser(szModuleName, "ImapUser", L""),
	m_imapPass(szModuleName, "ImapPassword", L""),
	m_imapHost(szModuleName, "ImapHost", L""),
	m_imapPort(szModuleName, "ImapPort", 0)
{
	CMStringW wszFilename(FORMAT, L"%s\\%S\\data", VARSW(L"%miranda_userdata%").get(), szModuleName);
	CreatePathToFileW(wszFilename);
	m_context = dc_context_new_closed(T2Utf(wszFilename));

	// hooks
	HookProtoEvent(ME_OPT_INITIALISE, &CDeltaChatProto::OnOptionsInit);

	// netlib support
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = m_tszUserName;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

CDeltaChatProto::~CDeltaChatProto()
{
	dc_context_unref(m_context);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDeltaChatProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_AUTHREQ | PF1_SERVERCLIST | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_AVATARS | PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_FORCEAUTH | PF4_SUPPORTIDLE | PF4_SUPPORTTYPING | PF4_SERVERMSGID;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT(DB_KEY_ID);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDeltaChatProto::SetStatus(int new_status)
{
	if (new_status != ID_STATUS_OFFLINE)
		new_status = ID_STATUS_ONLINE;

	int old_status = m_iStatus;
	m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE) {
		if (!Miranda_IsTerminated())
			setAllContactStatuses(ID_STATUS_OFFLINE, false);

		if (m_bRunning)
			SendLogout();

		Logout();
	}
	else if (!m_bRunning && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		ForkThread(&CDeltaChatProto::ServerThread);
	}
	else if (m_bRunning) {
		m_iStatus = new_status;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	}

	return 0;
}
