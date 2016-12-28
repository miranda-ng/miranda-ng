/*
Copyright © 2016 Miranda NG team

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

static int compareRequests(const AsyncHttpRequest *p1, const AsyncHttpRequest *p2)
{
	return p1->m_iReqNum - p2->m_iReqNum;
}

CDiscordProto::CDiscordProto(const char *proto_name, const wchar_t *username) :
	PROTO<CDiscordProto>(proto_name, username),
	m_arHttpQueue(10, compareRequests),
	m_evRequestsQueue(CreateEvent(NULL, FALSE, FALSE, NULL)),
	m_wszDefaultGroup(this, DB_KEY_GROUP, DB_KEYVAL_GROUP),
	m_wszEmail(this, DB_KEY_EMAIL, L"")
{
	// Services
	CreateProtoService(PS_GETNAME, &CDiscordProto::GetName);
	CreateProtoService(PS_GETSTATUS, &CDiscordProto::GetStatus);

	// Events
	HookProtoEvent(ME_OPT_INITIALISE, &CDiscordProto::OnOptionsInit);

	// Clist
	Clist_GroupCreate(NULL, m_wszDefaultGroup);

	// Network initialization
	CMStringW descr(FORMAT, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = descr.GetBuffer();
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

CDiscordProto::~CDiscordProto()
{
	debugLogA("CDiscordProto::~CDiscordProto");
	Netlib_CloseHandle(m_hNetlibUser);
	m_hNetlibUser = NULL;

	m_arHttpQueue.destroy();
	::CloseHandle(m_evRequestsQueue);
}

DWORD_PTR CDiscordProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSGRECV | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_AVATARS;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)"E-mail";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)DB_KEY_EMAIL;
	}
	return 0;
}

INT_PTR CDiscordProto::GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, m_szModuleName, (int)wParam);
	return 0;
}

INT_PTR CDiscordProto::GetStatus(WPARAM, LPARAM)
{
	return m_iStatus;
}

int CDiscordProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iStatus)
		return 0;

	m_iDesiredStatus = iNewStatus;
	int iOldStatus = m_iStatus;

	// all statuses but offline are treated as online
	if (iNewStatus >= ID_STATUS_ONLINE && iNewStatus <= ID_STATUS_OUTTOLUNCH) {
		m_iDesiredStatus = ID_STATUS_ONLINE;

		// if we're already connecting and they want to go online
		if (IsStatusConnecting(m_iStatus))
			return 0;

		// if we're already connected, don't try to reconnect
		if (m_iStatus >= ID_STATUS_ONLINE && m_iStatus <= ID_STATUS_OUTTOLUNCH)
			return 0;

		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
		m_hWorkerThread = ForkThreadEx(&CDiscordProto::ServerThread, NULL, NULL);
	}
	else if (iNewStatus == ID_STATUS_OFFLINE) {
		m_iStatus = m_iDesiredStatus;
		SetAllContactStatuses(ID_STATUS_OFFLINE);

		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)iOldStatus, m_iStatus);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnModulesLoaded(WPARAM, LPARAM)
{
	return 0;
}

int CDiscordProto::OnPreShutdown(WPARAM, LPARAM)
{
	debugLogA("CDiscordProto::OnPreShutdown");

	m_bTerminated = true;
	SetEvent(m_evRequestsQueue);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event) {
		case EV_PROTO_ONLOAD:    return OnModulesLoaded(wParam, lParam);
		case EV_PROTO_ONEXIT:    return OnPreShutdown(wParam, lParam);
		case EV_PROTO_ONOPTIONS: return OnOptionsInit(wParam, lParam);
	}

	return 1;
}
