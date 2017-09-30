#include "stdafx.h"

void CToxProto::BootstrapUdpNode(Tox *tox, const char *address, int port, const char *hexKey)
{
	if (!toxThread)
		return;

	if (address == NULL || hexKey == NULL)
		return;

	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_bootstrap(tox, address, port, binKey, &error))
		debugLogA(__FUNCTION__ ": failed to bootstrap node %s:%d \"%s\" (%d)", address, port, hexKey, error);
}

void CToxProto::BootstrapTcpRelay(Tox *tox, const char *address, int port, const char *hexKey)
{
	if (!toxThread)
		return;

	if (address == NULL || hexKey == NULL)
		return;

	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_add_tcp_relay(tox, address, port, binKey, &error))
		debugLogA(__FUNCTION__ ": failed to add tcp relay %s:%d \"%s\" (%d)", address, port, hexKey, error);
}

void CToxProto::BootstrapNodesFromDb(Tox *tox, bool isIPv6)
{
	char module[MAX_PATH];
	mir_snprintf(module, "%s_Nodes", m_szModuleName);
	int nodeCount = db_get_w(NULL, module, TOX_SETTINGS_NODE_COUNT, 0);
	if (nodeCount > 0) {
		char setting[MAX_PATH];
		for (int i = 0; i < nodeCount; i++) {
			mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, i);
			ptrA address(db_get_sa(NULL, module, setting));
			mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, i);
			int port = db_get_w(NULL, module, setting, 33445);
			mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, i);
			ptrA pubKey(db_get_sa(NULL, module, setting));
			BootstrapUdpNode(tox, address, port, pubKey);
			BootstrapTcpRelay(tox, address, port, pubKey);
			if (isIPv6) {
				mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, i);
				address = db_get_sa(NULL, module, setting);
				BootstrapUdpNode(tox, address, port, pubKey);
				BootstrapTcpRelay(tox, address, port, pubKey);
			}
		}
	}
}

void CToxProto::BootstrapNodesFromJson(Tox *tox, bool isIPv6)
{
	ptrA json;

	VARSW path(_A2W(TOX_JSON_PATH));

	if (!IsFileExists(path))
		UpdateNodes();

	if (IsFileExists(path)) {
		FILE *hFile = _wfopen(path, L"r");
		if (hFile != NULL) {
			_fseeki64(hFile, 0, SEEK_END);
			size_t size = _ftelli64(hFile);
			json = (char*)mir_calloc(size);
			rewind(hFile);
			fread(json, sizeof(char), size, hFile);
			fclose(hFile);
		}
	}

	if (json) {
		JSONNode root = JSONNode::parse(json);
		if (!root.empty()) {
			JSONNode nodes = root.at("nodes").as_array();
			for (size_t i = 0; i < nodes.size(); i++) {
				JSONNode node = nodes[i];
				JSONNode address = node.at("ipv4");
				JSONNode pubKey = node.at("public_key");

				if (node.at("status_udp").as_bool()) {
					int port = node.at("port").as_int();
					BootstrapUdpNode(tox, address.as_string().c_str(), port, pubKey.as_string().c_str());
					if (isIPv6) {
						address = node.at("ipv6");
						BootstrapUdpNode(tox, address.as_string().c_str(), port, pubKey.as_string().c_str());
					}
				}

				if (node.at("status_tcp").as_bool()) {
					JSONNode tcpPorts = node.at("tcp_ports").as_array();
					for (size_t k = 0; k < tcpPorts.size(); k++) {
						int port = tcpPorts[k].as_int();
						BootstrapTcpRelay(tox, address.as_string().c_str(), port, pubKey.as_string().c_str());
						if (isIPv6) {
							address = node.at("ipv6");
							BootstrapTcpRelay(tox, address.as_string().c_str(), port, pubKey.as_string().c_str());
						}
					}
				}
			}
		}
	}
}

void CToxProto::BootstrapNodes(Tox *tox)
{
	UpdateNodes();
	debugLogA(__FUNCTION__": bootstraping DHT");
	// bool isUdp = getBool("EnableUDP", 1);
	bool isIPv6 = getBool("EnableIPv6", 0);
	BootstrapNodesFromDb(tox, isIPv6);
	BootstrapNodesFromJson(tox, isIPv6);
}

void CToxProto::UpdateNodes()
{
	debugLogA(__FUNCTION__": updating nodes");
	HttpRequest request(REQUEST_GET, "https://nodes.tox.chat/json");
	NLHR_PTR response(request.Send(m_hNetlibUser));
	if (!response || response->resultCode != HTTP_CODE_OK || !response->pData) {
		debugLogA(__FUNCTION__": failed to dowload tox.json");
		return;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		debugLogA(__FUNCTION__": failed to dowload tox.json");
		return;
	}

	long lastUpdate = root.at("last_scan").as_int();
	if (lastUpdate <= getDword("NodesUpdate", 0))
		return;

	ptrW path(mir_wstrdup((wchar_t*)VARSW(_A2W(TOX_JSON_PATH))));
	if (!IsFileExists(path)) {
		HANDLE hProfile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hProfile == NULL) {
			debugLogA(__FUNCTION__": failed to create tox.json");
			return;
		}
		CloseHandle(hProfile);
	}

	FILE *hFile = _wfopen(path, L"w");
	if (!hFile) {
		debugLogA(__FUNCTION__": failed to open tox.json");
		return;
	}

	if (fwrite(response->pData, sizeof(char), response->dataLength, hFile) != (size_t)response->dataLength)
		debugLogA(__FUNCTION__": failed to write tox.json");

	fclose(hFile);

	setDword("NodesUpdate", lastUpdate);
}
