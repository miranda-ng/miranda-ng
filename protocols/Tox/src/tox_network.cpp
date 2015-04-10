#include "common.h"

bool CToxProto::IsOnline()
{
	return isConnected && m_iStatus > ID_STATUS_OFFLINE;
}

void CToxProto::BootstrapNode(const char *address, int port, const uint8_t *pubKey)
{
	TOX_ERR_BOOTSTRAP error;
	if (!tox_bootstrap(tox, address, port, pubKey, &error))
	{
		debugLogA("CToxProto::BootstrapNode: failed to bootstrap node %s:%d \"%s\" (%d)", address, port, (const char*)ToxHexAddress(pubKey), error);
	}
}

void CToxProto::BootstrapNodesFromDb(bool isIPv6)
{
	char module[MAX_PATH];
	mir_snprintf(module, SIZEOF(module), "%s_Nodes", m_szModuleName);
	int nodeCount = db_get_w(NULL, module, TOX_SETTINGS_NODE_COUNT, 0);
	if (nodeCount > 0)
	{
		char setting[MAX_PATH];
		for (int i = 0; i < nodeCount; i++)
		{
			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, i);
			ptrA address(db_get_sa(NULL, module, setting));
			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, i);
			int port = db_get_w(NULL, module, setting, 33445);
			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, i);
			ToxBinAddress pubKey(ptrA(db_get_sa(NULL, module, setting)));
			BootstrapNode(address, port, pubKey);
			if (isIPv6)
			{
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, i);
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
		GetPrivateProfileSectionNamesA(sections, SIZEOF(sections), fileName);
		section = sections;
		while (*section != NULL)
		{
			if (strstr(section, TOX_SETTINGS_NODE_PREFIX) == section)
			{
				GetPrivateProfileStringA(section, "IPv4", NULL, value, SIZEOF(value), fileName);
				ptrA address(mir_strdup(value));
				int port = GetPrivateProfileIntA(section, "Port", 33445, fileName);
				GetPrivateProfileStringA(section, "PubKey", NULL, value, SIZEOF(value), fileName);
				ToxBinAddress pubKey(value);
				BootstrapNode(address, port, pubKey);
				if (isIPv6)
				{
					GetPrivateProfileStringA(section, "IPv6", NULL, value, SIZEOF(value), fileName);
					address = mir_strdup(value);
					BootstrapNode(address, port, pubKey);
				}
			}
			section += strlen(section) + 1;
		}
	}
}

void CToxProto::BootstrapNodes()
{
	debugLogA(__FUNCTION__": bootstraping DHT");
	bool isIPv6 = getBool("EnableIPv6", 0);
	BootstrapNodesFromDb(isIPv6);
	BootstrapNodesFromIni(isIPv6);
}

void CToxProto::TryConnect()
{
	if (tox_self_get_connection_status(tox) != TOX_CONNECTION_NONE)
	{
		isConnected = true;
		debugLogA(__FUNCTION__": successfuly connected to DHT");

		ForkThread(&CToxProto::LoadFriendList, NULL);

		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
		tox_self_set_status(tox, MirandaToToxStatus(m_iStatus));
		debugLogA(__FUNCTION__": changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
	}
	else if (m_iStatus++ > TOX_MAX_CONNECT_RETRIES)
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, LOGINERR_NONETWORK);
		debugLogA(__FUNCTION__": failed to connect to DHT");
	}
}

void CToxProto::CheckConnection(int &retriesCount)
{
	if (!isConnected)
	{
		TryConnect();
	}
	else if (tox_self_get_connection_status(tox) != TOX_CONNECTION_NONE)
	{
		if (retriesCount < TOX_MAX_DISCONNECT_RETRIES)
		{
			debugLogA(__FUNCTION__": restored connection with DHT");
			retriesCount = TOX_MAX_DISCONNECT_RETRIES;
		}
	}
	else
	{
		if (retriesCount == TOX_MAX_DISCONNECT_RETRIES)
		{
			retriesCount--;
			debugLogA(__FUNCTION__": lost connection with DHT");
		}
		else if (retriesCount % 50 == 0)
		{
			retriesCount--;
			BootstrapNodes();
		}
		else if (!(--retriesCount))
		{
			isConnected = false;
			debugLogA(__FUNCTION__": disconnected from DHT");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
}

void CToxProto::DoTox()
{
	{
		mir_cslock lock(toxLock);
		tox_iterate(tox);
	}
	uint32_t interval = tox_iteration_interval(tox);
	Sleep(interval);
}

void CToxProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__": entering");

	if (!InitToxCore())
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, (HANDLE)NULL, LOGINERR_WRONGPASSWORD);
		debugLogA(__FUNCTION__": leaving");
		return;
	}

	int retriesCount = TOX_MAX_DISCONNECT_RETRIES;
	isConnected = false;
	BootstrapNodes();

	while (!isTerminated)
	{
		CheckConnection(retriesCount);
		DoTox();
	}

	UninitToxCore();
	isConnected = false;

	debugLogA(__FUNCTION__": leaving");
}