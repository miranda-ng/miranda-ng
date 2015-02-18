#include "common.h"

bool CToxProto::IsOnline()
{
	return isConnected && m_iStatus > ID_STATUS_OFFLINE;
}

void CToxProto::BootstrapDht()
{
	debugLogA("CToxProto::BootstrapDht: bootstraping DHT");

	bool isIPv4 = getBool("DisableIPv6", 0);
	int nodeCount = db_get_w(NULL, MODULE, TOX_SETTINGS_NODE_COUNT, 0);
	if (!nodeCount)
	{
		tox_bootstrap_from_address(
			tox, isIPv4 ? "192.254.75.102" : "2607:5600:284::2", 33445,
			ToxBinAddress("951C88B7E75C867418ACDB5D273821372BB5BD652740BCDF623A4FA293E75D2F"));
		tox_bootstrap_from_address(
			tox, "104.219.184.206", 443,
			ToxBinAddress("8CD087E31C67568103E8C2A28653337E90E6B8EDA0D765D57C6B5172B4F1F04C"));
	}
	else
	{
		char setting[MAX_PATH];
		for (int i = 0; i < nodeCount; i++)
		{
			mir_snprintf(setting, SIZEOF(setting), isIPv4 ? TOX_SETTINGS_NODE_IPV4 : TOX_SETTINGS_NODE_IPV6, i + 1);
			ptrA address(db_get_sa(NULL, MODULE, setting));
			if (address == NULL && !isIPv4)
			{
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, i + 1);
				address = db_get_sa(NULL, MODULE, setting);
			}
			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, i + 1);
			int port = db_get_w(NULL, MODULE, setting, 0);
			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, i + 1);
			ptrA pubKey(db_get_sa(NULL, MODULE, setting));
			if (pubKey && address)
			{
				tox_bootstrap_from_address(tox, address, port, ToxBinAddress(pubKey));
			}
		}
	}
}

void CToxProto::TryConnect()
{
	if (tox_isconnected(tox))
	{
		isConnected = true;
		debugLogA("CToxProto::PollingThread: successfuly connected to DHT");

		ForkThread(&CToxProto::LoadFriendList, NULL);

		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
		tox_set_user_status(tox, MirandaToToxStatus(m_iStatus));
		debugLogA("CToxProto::PollingThread: changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
	}
	else
	{
		if (m_iStatus++ > MAX_CONNECT_RETRIES)
		{
			SetStatus(ID_STATUS_OFFLINE);
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, LOGINERR_NONETWORK);
			debugLogA("CToxProto::PollingThread: failed to connect to DHT");
		}
	}
}

void CToxProto::CheckConnection(int &retriesCount)
{
	if (!isConnected)
	{
		debugLogA("CToxProto::CheckConnection: lost connection with DHT");
		TryConnect();
	}
	else
	{
		if (tox_isconnected(tox))
		{
			if (retriesCount < TOX_MAX_DISCONNECT_RETRIES)
			{
				debugLogA("CToxProto::CheckConnection: restored connection with DHT");
				retriesCount = TOX_MAX_DISCONNECT_RETRIES;
			}
		}
		else if (!(--retriesCount))
		{
			isConnected = false;
			debugLogA("CToxProto::CheckConnection: disconnected from DHT");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
}

void CToxProto::DoTox()
{
	{
		mir_cslock lock(toxLock);
		tox_do(tox);
	}
	uint32_t interval = tox_do_interval(tox);
	Sleep(interval);
}

void CToxProto::PollingThread(void*)
{
	debugLogA("CToxProto::PollingThread: entering");

	if (!InitToxCore())
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, LOGINERR_WRONGPASSWORD);
		debugLogA("CToxProto::PollingThread: leaving");
		return;
	}

	int retriesCount = TOX_MAX_DISCONNECT_RETRIES;
	isConnected = false;
	BootstrapDht();

	while (!isTerminated)
	{
		CheckConnection(retriesCount);
		DoTox();
	}

	UninitToxCore();
	isConnected = false;

	debugLogA("CToxProto::PollingThread: leaving");
}