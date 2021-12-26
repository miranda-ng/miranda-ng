#include "stdafx.h"

void CToxProto::BootstrapUdpNode(Tox *tox, const char *address, int port, const char *hexKey)
{
	if (address == nullptr || hexKey == nullptr)
		return;

	ToxBinAddress binKey(hexKey, TOX_PUBLIC_KEY_SIZE * 2);
	TOX_ERR_BOOTSTRAP error;
	if (!tox_bootstrap(tox, address, port, binKey, &error))
		debugLogA(__FUNCTION__ ": failed to bootstrap node %s:%d \"%s\" (%d)", address, port, hexKey, error);
}

void CToxProto::BootstrapTcpRelay(Tox *tox, const char *address, int port, const char *hexKey)
{
	if (address == nullptr || hexKey == nullptr)
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
	int nodeCount = db_get_w(0, module, TOX_SETTINGS_NODE_COUNT, 0);
	if (nodeCount == 0)
		return;

	char setting[MAX_PATH];
	for (int i = 0; i < nodeCount; i++) {
		mir_snprintf(setting, TOX_SETTINGS_NODE_IPV4, i);
		ptrA address(db_get_sa(0, module, setting));
		mir_snprintf(setting, TOX_SETTINGS_NODE_PORT, i);
		int port = db_get_w(0, module, setting, 33445);
		mir_snprintf(setting, TOX_SETTINGS_NODE_PKEY, i);
		ptrA pubKey(db_get_sa(0, module, setting));
		BootstrapUdpNode(tox, address, port, pubKey);
		BootstrapTcpRelay(tox, address, port, pubKey);
		if (isIPv6) {
			mir_snprintf(setting, TOX_SETTINGS_NODE_IPV6, i);
			address = db_get_sa(0, module, setting);
			BootstrapUdpNode(tox, address, port, pubKey);
			BootstrapTcpRelay(tox, address, port, pubKey);
		}
	}
}

void CToxProto::BootstrapNodesFromJson(Tox *tox, bool isIPv6)
{
	VARSW path(TOX_JSON_PATH);
	long lastUpdate = getDword("NodesUpdate", 0);
	if (!IsFileExists(path) || lastUpdate < (now() - 86400 /* 24h */))
		UpdateNodes();

	JSONNode nodes = ParseNodes();
	auto nodeCount = nodes.size();
	if (nodeCount == 0)
		return;

	static int j = rand() % nodeCount;

	int i = 0;
	while (i < 2) {
		JSONNode node = nodes[j % nodeCount];

		bool udpStatus = node.at("status_udp").as_bool();
		bool tcpStatus = node.at("status_tcp").as_bool();
		if (!udpStatus && !tcpStatus) {
			nodeCount = j - 1;
			j = rand() % nodeCount;
			if (j == 0)
				return;
			continue;
		}

		JSONNode address = node.at("ipv4");
		JSONNode pubKey = node.at("public_key");

		if (udpStatus) {
			int port = node.at("port").as_int();
			BootstrapUdpNode(tox, address.as_string().c_str(), port, pubKey.as_string().c_str());
			if (isIPv6) {
				address = node.at("ipv6");
				BootstrapUdpNode(tox, address.as_string().c_str(), port, pubKey.as_string().c_str());
			}
		}

		if (tcpStatus) {
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

		j++;
		i++;
	}
}

void CToxProto::BootstrapNodes(Tox *tox)
{
	debugLogA(__FUNCTION__": bootstraping DHT");
	bool isIPv6 = getBool("EnableIPv6", 0);
	BootstrapNodesFromJson(tox, isIPv6);
	BootstrapNodesFromDb(tox, isIPv6);
}

void CToxProto::UpdateNodes()
{
	VARSW path(TOX_JSON_PATH);

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

	uint32_t lastUpdate = root.at("last_scan").as_int();
	if (lastUpdate <= getDword("NodesUpdate", 0))
		return;

	FILE *hFile = _wfopen(path, L"w");
	if (hFile == nullptr) {
		debugLogA(__FUNCTION__": failed to open tox.json");
		return;
	}

	if (fwrite(response->pData, sizeof(char), response->dataLength, hFile) != (size_t)response->dataLength)
		debugLogA(__FUNCTION__": failed to write tox.json");

	fclose(hFile);

	setDword("NodesUpdate", lastUpdate);
}

JSONNode CToxProto::ParseNodes()
{
	VARSW path(TOX_JSON_PATH);

	if (!IsFileExists(path)) {
		debugLogA(__FUNCTION__": could not find tox.json");
		return JSONNode(JSON_ARRAY);
	}

	FILE *hFile = _wfopen(path, L"r");
	if (hFile == nullptr) {
		debugLogA(__FUNCTION__": failed to open tox.json");
		return JSONNode(JSON_ARRAY);
	}

	_fseeki64(hFile, 0, SEEK_END);
	size_t size = _ftelli64(hFile);
	ptrA json((char*)mir_calloc(size));
	rewind(hFile);
	fread(json, sizeof(char), size, hFile);
	fclose(hFile);

	JSONNode root = JSONNode::parse(json);
	if (root.empty()) {
		debugLogA(__FUNCTION__": failed to parse tox.json");
		return JSONNode(JSON_ARRAY);
	}

	return root.at("nodes").as_array();
}
