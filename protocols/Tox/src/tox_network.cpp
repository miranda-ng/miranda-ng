#include "stdafx.h"

bool CToxProto::IsOnline()
{
	return toxThread && toxThread->IsConnected() && m_iStatus >= ID_STATUS_ONLINE;
}

void CToxProto::BootstrapNode(const char *address, int port, const char *hexKey)
{
	if (hexKey == NULL || toxThread == NULL)
		return;

	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_bootstrap(toxThread->Tox(), address, port, binKey, &error))
		logger->Log(__FUNCTION__ ": failed to bootstrap node %s:%d \"%s\" (%d)", address, port, hexKey, error);
	if (!tox_add_tcp_relay(toxThread->Tox(), address, port, binKey, &error))
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

void CToxProto::BootstrapNodesFromJson(bool isIPv6)
{
	char *json = NULL;

	ptrT path(mir_tstrdup((TCHAR*)VARST(_T(TOX_JSON_PATH))));

	if (!IsFileExists(path))
		UpdateNodes();

	if (IsFileExists(path))
	{
		FILE *hFile = _tfopen(path, L"r");
		if (hFile != NULL)
		{
			_fseeki64(hFile, 0, SEEK_END);
			size_t size = _ftelli64(hFile);
			json = (char*)mir_calloc(size);
			rewind(hFile);
			fread(json, sizeof(char), size, hFile);
			fclose(hFile);
		}
	}

	if (json)
	{
		JSONNode root = JSONNode::parse(json);
		if (!root.empty())
		{
			JSONNode nodes = root.at("nodes").as_array();
			for (size_t i = 0; i < nodes.size(); i++)
			{
				JSONNode node = nodes[i];

				JSONNode address = node.at("ipv4");
				int port = node.at("port").as_int();
				JSONNode pubKey = node.at("public_key");
				BootstrapNode(address.as_string().c_str(), port, pubKey.as_string().c_str());
				if (isIPv6)
				{
					address = node.at("ipv6");
					BootstrapNode(address.as_string().c_str(), port, pubKey.as_string().c_str());
				}
			}
		}
	}
}

void CToxProto::BootstrapNodes()
{
	logger->Log(__FUNCTION__": bootstraping DHT");
	bool isIPv6 = getBool("EnableIPv6", 0);
	BootstrapNodesFromDb(isIPv6);
	BootstrapNodesFromJson(isIPv6);
}

void CToxProto::UpdateNodes()
{
	ptrT path(mir_tstrdup((TCHAR*)VARST(_T(TOX_JSON_PATH))));

	if (!IsFileExists(path))
	{
		HANDLE hProfile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hProfile == NULL)
		{
			logger->Log(__FUNCTION__": failed to create tox.json");
			return;
		}
		CloseHandle(hProfile);
	}

	HttpRequest request(REQUEST_GET, "https://nodes.tox.chat/json");
	NLHR_PTR response(request.Send(hNetlib));

	if (response->resultCode == 200 && response->pData)
	{
		FILE *hFile = _tfopen(path, _T("w"));
		if (hFile)
		{
			fwrite(response->pData, sizeof(char), response->dataLength, hFile);
			fclose(hFile);
		}
	}
}

void CToxProto::TryConnect()
{
	if (toxThread != NULL)
	{
		if (tox_self_get_connection_status(toxThread->Tox()) != TOX_CONNECTION_NONE)
		{
			toxThread->Connect();
			logger->Log(__FUNCTION__": successfuly connected to DHT");

			ForkThread(&CToxProto::LoadFriendList, NULL);

			m_iStatus = m_iDesiredStatus;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
			tox_self_set_status(toxThread->Tox(), MirandaToToxStatus(m_iStatus));
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
	if (!toxThread || !toxThread->IsConnected())
	{
		TryConnect();
	}
	else if (tox_self_get_connection_status(toxThread->Tox()) != TOX_CONNECTION_NONE)
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
			toxThread->Disconnect();
			logger->Log(__FUNCTION__": disconnected from DHT");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
}

void CToxProto::PollingThread(void*)
{
	Tox_Options *options = GetToxOptions();
	if (!options)
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL);
		logger->Log(__FUNCTION__": leaving");
		return;
	}

	TOX_ERR_NEW error;
	CToxThread toxThread(options, &error);
	if (error != TOX_ERR_NEW_OK)
	{
		logger->Log(__FUNCTION__": failed to initialize tox core (%d)", error);
		ShowNotification(ToxErrorToString(error), TranslateT("Unable to initialize Tox core"), MB_ICONERROR);
		tox_options_free(options);
	}
	tox_options_free(options);

	this->toxThread = &toxThread;

	logger->Log(__FUNCTION__": entering");

	if (!InitToxCore(&toxThread))
	{
		UninitToxCore(&toxThread);
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
		logger->Log(__FUNCTION__": leaving");
		return;
	}

	int retriesCount = TOX_MAX_DISCONNECT_RETRIES;
	BootstrapNodes();

	while (!toxThread.IsTerminated())
	{
		CheckConnection(retriesCount);
		toxThread.Iterate();
	}

	this->toxThread = NULL;

	toxThread.Disconnect();
	UninitToxCore(&toxThread);

	logger->Log(__FUNCTION__": leaving");
}