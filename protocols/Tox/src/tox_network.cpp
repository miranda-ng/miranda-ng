#include "stdafx.h"

bool CToxProto::IsOnline()
{
	return toxThread && toxThread->IsConnected() && m_iStatus >= ID_STATUS_ONLINE;
}

void CToxProto::BootstrapUdpNode(CToxThread *toxThread, const char *address, int port, const char *hexKey)
{
	if (!toxThread)
		return;

	if (address == NULL || hexKey == NULL)
		return;

	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_bootstrap(toxThread->Tox(), address, port, binKey, &error))
		debugLogA(__FUNCTION__ ": failed to bootstrap node %s:%d \"%s\" (%d)", address, port, hexKey, error);
}

void CToxProto::BootstrapTcpRelay(CToxThread *toxThread, const char *address, int port, const char *hexKey)
{	
	if (!toxThread)
		return;

	if (address == NULL || hexKey == NULL)
		return;

	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_add_tcp_relay(toxThread->Tox(), address, port, binKey, &error))
		debugLogA(__FUNCTION__ ": failed to add tcp relay %s:%d \"%s\" (%d)", address, port, hexKey, error);
}

void CToxProto::BootstrapNodesFromDb(CToxThread *toxThread, bool isIPv6)
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
			BootstrapUdpNode(toxThread, address, port, pubKey);
			BootstrapTcpRelay(toxThread, address, port, pubKey);
			if (isIPv6)
			{
				mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, i);
				address = db_get_sa(NULL, module, setting);
				BootstrapUdpNode(toxThread, address, port, pubKey);
				BootstrapTcpRelay(toxThread, address, port, pubKey);
			}
		}
	}
}

void CToxProto::BootstrapNodesFromJson(CToxThread *toxThread, bool isUdp, bool isIPv6)
{
	ptrA json;

	VARST path(_A2W(TOX_JSON_PATH));

	if (!IsFileExists(path))
		UpdateNodes();

	if (IsFileExists(path))
	{
		FILE *hFile = _wfopen(path, L"r");
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
				JSONNode pubKey = node.at("public_key");

				if (isUdp)
				{
					int port = node.at("port").as_int();
					BootstrapUdpNode(toxThread, address.as_string().c_str(), port, pubKey.as_string().c_str());
					if (isIPv6)
					{
						address = node.at("ipv6");
						BootstrapUdpNode(toxThread, address.as_string().c_str(), port, pubKey.as_string().c_str());
					}
				}
				else
				{
					JSONNode tcpPorts = root.at("tcp_ports").as_array();
					for (size_t i = 0; i < tcpPorts.size(); i++)
					{
						int port = tcpPorts[i].as_int();
						BootstrapTcpRelay(toxThread, address.as_string().c_str(), port, pubKey.as_string().c_str());
						if (isIPv6)
						{
							address = node.at("ipv6");
							BootstrapTcpRelay(toxThread, address.as_string().c_str(), port, pubKey.as_string().c_str());
						}
					}
				}
			}
		}
	}
}

void CToxProto::BootstrapNodes(CToxThread *toxThread)
{
	debugLogA(__FUNCTION__": bootstraping DHT");
	bool isUdp = getBool("EnableUDP", 1);
	bool isIPv6 = getBool("EnableIPv6", 0);
	BootstrapNodesFromDb(toxThread, isIPv6);
	BootstrapNodesFromJson(toxThread, isUdp, isIPv6);
}

void CToxProto::UpdateNodes()
{
	HttpRequest request(REQUEST_GET, "https://nodes.tox.chat/json");
	NLHR_PTR response(request.Send(m_hNetlibUser));
	if (response->resultCode != HTTP_CODE_OK || !response->pData)
	{
		debugLogA(__FUNCTION__": failed to dowload tox.json");
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
	{
		debugLogA(__FUNCTION__": failed to dowload tox.json");
		return;
	}

	long lastUpdate = root.at("last_scan").as_int();
	if (lastUpdate <= getDword("NodesUpdate", 0))
		return;

	ptrW path(mir_wstrdup((wchar_t*)VARST(_A2W(TOX_JSON_PATH))));
	if (!IsFileExists(path))
	{
		HANDLE hProfile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hProfile == NULL)
		{
			debugLogA(__FUNCTION__": failed to create tox.json");
			return;
		}
		CloseHandle(hProfile);
	}

	FILE *hFile = _wfopen(path, L"w");
	if (!hFile)
	{
		debugLogA(__FUNCTION__": failed to open tox.json");
		return;
	}

	if (fwrite(response->pData, sizeof(char), response->dataLength, hFile) != (size_t)response->dataLength)
		debugLogA(__FUNCTION__": failed to write tox.json");

	fclose(hFile);

	setDword("NodesUpdate", lastUpdate);
}

void CToxProto::TryConnect()
{
	if (!toxThread)
	{
		return;
	}

	if (tox_self_get_connection_status(toxThread->Tox()) != TOX_CONNECTION_NONE)
	{
		toxThread->Connect();
		debugLogA(__FUNCTION__": successfuly connected to DHT");

		ForkThread(&CToxProto::LoadFriendList, NULL);

		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iStatus);
		tox_self_set_status(toxThread->Tox(), MirandaToToxStatus(m_iStatus));
		debugLogA(__FUNCTION__": changing status from %i to %i", ID_STATUS_CONNECTING, m_iDesiredStatus);
	}
	else if (m_iStatus++ > TOX_MAX_CONNECT_RETRIES)
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK);
		debugLogA(__FUNCTION__": failed to connect to DHT");
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
		/*else if (retriesCount % 50 == 0)
		{
			retriesCount--;
			BootstrapNodes();
		}*/
		else if (!(--retriesCount))
		{
			toxThread->Disconnect();
			debugLogA(__FUNCTION__": disconnected from DHT");
			SetStatus(ID_STATUS_OFFLINE);
		}
	}
}

void CToxProto::PollingThread(void*)
{
	debugLogA(__FUNCTION__": entering");
	Thread_SetName("TOX: PollingThread");

	Tox_Options *options = GetToxOptions();
	if (!options)
	{
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL);
		debugLogA(__FUNCTION__": leaving");
		return;
	}

	TOX_ERR_NEW error;
	CToxThread toxThread(options, &error);
	if (error != TOX_ERR_NEW_OK)
	{
		debugLogA(__FUNCTION__": failed to initialize tox core (%d)", error);
		ShowNotification(ToxErrorToString(error), TranslateT("Unable to initialize Tox core"), MB_ICONERROR);
		tox_options_free(options);
		debugLogA(__FUNCTION__": leaving");
		return;
	}
	tox_options_free(options);

	this->toxThread = &toxThread;

	if (!InitToxCore(&toxThread))
	{
		UninitToxCore(&toxThread);
		SetStatus(ID_STATUS_OFFLINE);
		ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_WRONGPASSWORD);
		debugLogA(__FUNCTION__": leaving");
		return;
	}

	int retriesCount = TOX_MAX_DISCONNECT_RETRIES;
	BootstrapNodes(&toxThread);

	while (!toxThread.IsTerminated())
	{
		CheckConnection(retriesCount);
		toxThread.Iterate();
	}

	toxThread.Disconnect();
	UninitToxCore(&toxThread);

	this->toxThread = NULL;

	debugLogA(__FUNCTION__": leaving");
}