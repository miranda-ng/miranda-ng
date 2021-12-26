#include "stdafx.h"

bool CToxProto::IsOnline()
{
	return m_tox && m_iStatus >= ID_STATUS_ONLINE;
}

void CToxProto::TryConnect()
{
	TOX_CONNECTION connectionStatus = tox_self_get_connection_status(m_tox);
	if (connectionStatus != TOX_CONNECTION_NONE) {
		debugLogA(__FUNCTION__": successfuly connected to DHT");

		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
		tox_self_set_status(m_tox, MirandaToToxStatus(m_iStatus));

		debugLogA(__FUNCTION__": changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);

		UpdateStatusMenu(NULL, NULL);

		LoadFriendList(m_tox);
		return;
	}

	int maxConnectRetries = getByte("MaxConnectRetries", TOX_MAX_CONNECT_RETRIES);
	if (m_iStatus++ > maxConnectRetries) {
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
		debugLogA(__FUNCTION__": failed to connect to DHT");
		return;
	}
}

void CToxProto::CheckConnection()
{
	int maxReconnectRetries = getByte("MaxReconnectRetries", TOX_MAX_RECONNECT_RETRIES);

	TOX_CONNECTION connectionStatus = tox_self_get_connection_status(m_tox);
	if (connectionStatus != TOX_CONNECTION_NONE) {
		if (m_retriesCount < maxReconnectRetries) {
			debugLogA(__FUNCTION__": restored connection with DHT");
			m_retriesCount = maxReconnectRetries;
		}
		return;
	}

	if (m_retriesCount == maxReconnectRetries) {
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

void CToxProto::OnToxCheck(void *arg, uint8_t)
{
	CToxProto *proto = (CToxProto*)arg;
	if (proto->m_tox == nullptr)
		return;

	// int retriesCount = proto->getByte("MaxReconnectRetries", TOX_MAX_RECONNECT_RETRIES);
	if (proto->m_iStatus < ID_STATUS_ONLINE)
		proto->TryConnect();
	else
		proto->CheckConnection();
}

void CToxProto::OnToxPoll(void *arg, uint8_t)
{
	CToxProto *proto = (CToxProto*)arg;
	if (proto->m_tox)
		tox_iterate(proto->m_tox, arg);

	/*uint32_t interval = tox_iteration_interval(proto->m_tox);
	interval = interval
		? interval
		: TOX_DEFAULT_INTERVAL;*/
}
