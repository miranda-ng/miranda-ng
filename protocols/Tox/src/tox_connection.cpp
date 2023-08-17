#include "stdafx.h"

bool CToxProto::IsOnline()
{
	return m_tox && m_iStatus >= ID_STATUS_ONLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CToxProto::OnLoggedIn()
{
	debugLogA(__FUNCTION__": successfuly connected to DHT");

	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
	tox_self_set_status(m_tox, MirandaToToxStatus(m_iStatus));

	debugLogA(__FUNCTION__": changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);

	UpdateStatusMenu(NULL, NULL);

	m_impl.timerPoll.Start(TOX_DEFAULT_INTERVAL);

	LoadFriendList(m_tox);
}

void CToxProto::OnLoggedFail()
{
	int maxConnectRetries = getByte("MaxConnectRetries", TOX_MAX_CONNECT_RETRIES);

	if (m_iStatus > ID_STATUS_OFFLINE) {
		if (m_retriesCount == maxConnectRetries) {
			m_retriesCount--;
			debugLogA(__FUNCTION__": lost connection with DHT");
			return;
		}

		if (!(--m_retriesCount)) {
			debugLogA(__FUNCTION__": disconnected from DHT");
			SetStatus(ID_STATUS_OFFLINE);
			return;
		}
	}
	else {
		if (m_iStatus++ > maxConnectRetries) {
			SetStatus(ID_STATUS_OFFLINE);
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
			debugLogA(__FUNCTION__": failed to connect to DHT");
		}
	}
}

void CToxProto::OnToxCheck()
{
	if (m_tox == nullptr)
		return;

	int iStatus = tox_self_get_connection_status(m_tox);
	if (iStatus == TOX_CONNECTION_NONE)
		OnLoggedFail();
	else if 	(iStatus != m_prevToxStatus)
		OnLoggedIn();

	m_prevToxStatus = iStatus;
}

void CToxProto::OnToxPoll()
{
	if (m_tox)
		tox_iterate(m_tox, this);
}
