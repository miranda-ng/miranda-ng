#include "stdafx.h"

bool CToxProto::IsOnline()
{
	return toxThread && toxThread->isConnected && m_iStatus >= ID_STATUS_ONLINE;
}

void CToxProto::BootstrapNode(const char *address, int port, const char *hexKey)
{
	if (hexKey == NULL || toxThread == NULL)
		return;
	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_bootstrap(toxThread->tox, address, port, binKey, &error))
		logger->Log(__FUNCTION__ ": failed to bootstrap node %s:%d \"%s\" (%d)", address, port, hexKey, error);
	if (!tox_add_tcp_relay(toxThread->tox, address, port, binKey, &error))
		logger->Log(__FUNCTION__ ": failed to add tcp relay %s:%d \"%s\" (%d)", address, port, hexKey, error);
}

void CToxProto::BootstrapNodesFromDb(bool isIPv6)
{
	char module[MAX_PATH];
	mir_snprintf(module, "%s_Nodes", m_szModuleName);
	int nodeCount = db_get_w(NULL, module, TOX_SETTINGS_NODE_COUNT, 0);
	if (nodeCount > 0)
	{
		char setting[MAX_PATH];
		for (int i = 0; i < nodeCount; i++)
		{
			mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, i);
			ptrA address(db_get_sa(NULL, module, setting));
			mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, i);
			int port = db_get_w(NULL, module, setting, 33445);
			mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, i);
			ptrA pubKey(db_get_sa(NULL, module, setting));
			BootstrapNode(address, port, pubKey);
			if (isIPv6)
			{
				mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, i);
				address = db_get_sa(NULL, module, setting);
				BootstrapNode(address, port, pubKey);
			}
		}
	}
}

void CToxProto::BootstrapNodesFromIni(bool isIPv6)
{
	if (IsFileExists((TCHAR*)VARST(_T(TOX_INI_PATH))))
	{
		char fileName[MAX_PATH];
		mir_strcpy(fileName, VARS(TOX_INI_PATH));

		char *section, sections[MAX_PATH], value[MAX_PATH];
		GetPrivateProfileSectionNamesA(sections, _countof(sections), fileName);
		section = sections;
		while (*section != NULL)
		{
			if (strstr(section, TOX_SETTINGS_NODE_PREFIX) == section)
			{
				GetPrivateProfileStringA(section, "IPv4", NULL, value, _countof(value), fileName);
				ptrA address(mir_strdup(value));
				int port = GetPrivateProfileIntA(section, "Port", 33445, fileName);
				GetPrivateProfileStringA(section, "PubKey", NULL, value, _countof(value), fileName);
				ptrA pubKey(mir_strdup(value));
				BootstrapNode(address, port, pubKey);
				if (isIPv6)
				{
					GetPrivateProfileStringA(section, "IPv6", NULL, value, _countof(value), fileName);
					address = mir_strdup(value);
					BootstrapNode(address, port, pubKey);
				}
			}
			section += mir_strlen(section) + 1;
		}
	}
}

void CToxProto::BootstrapNodes()
{
	logger->Log(__FUNCTION__": bootstraping DHT");
	bool isIPv6 = getBool("EnableIPv6", 0);
	BootstrapNodesFromDb(isIPv6);
	BootstrapNodesFromIni(isIPv6);
}

void CToxProto::TryConnect()
{
	if (toxThread != NULL)
	{
		if (tox_self_get_connection_status(toxThread->tox) != TOX_CONNECTION_NONE)
		{
			toxThread->isConnected = true;
			logger->Log(__FUNCTION__": successfuly connected to DHT");

			ForkThread(&CToxProto::LoadFriendList, NULL);

			m_iStatus = m_iDesiredStatus;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
			tox_self_set_status(toxThread->tox, MirandaToToxStatus(m_iStatus));
			logger->Log(__FUNCTION__": changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
		}
		else if (m_iStatus++ > TOX_MAX_CONNECT_RETRIES)
		{
			SetStatus(ID_STATUS_OFFLINE);
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
			logger->Log(__FUNCTION__": failed to connect to DHT");
		}
	}
}

void CToxProto::CheckConnection(int &retriesCount)
{
	if (!toxThread || !toxThread->isConnected)
	{
		TryConnect();
	}
	else if (tox_self_get_connection_status(toxThread->tox) != TOX_CONNECTION_NONE)
	{
		if (retriesCount < TOX_MAX_DISCONNECT_RETRIES)
		{
			logger->Log(__FUNCTION__": restored connection with DHT");
			retriesCount = TOX_MAX_DISCONNECT_RETRIES;
		}
	}
	else
	{
		if (retriesCount == TOX_MAX_DISCONNECT_RETRIES)
		{
			retriesCount--;
			logger->Log(__FUNCTION__": lost connection with DHT");
		}
		else if (retriesCount % 50 == 0)
		{
			retriesCount--;
			BootstrapNodes();
		}
		else if (!(--retriesCount))
		{
			toxThread->isConnected = false;
			logger->Log(__FUNCTION__": disconnected from DHT");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
}

void CToxProto::PollingThread(void*)
{
	CToxThread toxThread;
	this->toxThread = &toxThread;

	logger->Log(__FUNCTION__": entering");

	if (!InitToxCore())
	{
		UninitToxCore();
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
		logger->Log(__FUNCTION__": leaving");
		return;
	}

	int retriesCount = TOX_MAX_DISCONNECT_RETRIES;
	toxThread.isConnected = false;
	BootstrapNodes();

	while (!toxThread.isTerminated)
	{
		CheckConnection(retriesCount);
		toxThread.Do();
	}

	UninitToxCore();
	toxThread.isConnected = false;

	logger->Log(__FUNCTION__": leaving");
}