#include "stdafx.h"

CMTProto::CMTProto(const char* protoName, const wchar_t* userName) :
	PROTO<CMTProto>(protoName, userName),
	m_pClientMmanager(std::make_unique<td::ClientManager>()),
	m_arRequests(10, NumericKeySortT)
{
	m_iClientId = m_pClientMmanager->create_client_id();
}

CMTProto::~CMTProto()
{
}

INT_PTR CMTProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)"Phone";
	}
	return 0;
}

int CMTProto::SetStatus(int iNewStatus)
{
	if (m_iDesiredStatus == iNewStatus)
		return 0;
	
	int oldStatus = m_iStatus;

	// Routing statuses not supported by Telegram
	switch (iNewStatus) {
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = iNewStatus;
		break;

	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
	default:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		if (m_bRunning)
			SendQuery(new td::td_api::close());

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (!m_bRunning && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

		ForkThread(&CMTProto::ServerThread);
	}
	else if (m_bRunning) {
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}
