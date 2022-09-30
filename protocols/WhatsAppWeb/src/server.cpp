/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnLoggedIn()
{
	debugLogA("WhatsAppProto::OnLoggedIn");

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	SendKeepAlive();
	m_impl.m_keepAlive.Start(60000);
}

void WhatsAppProto::OnLoggedOut(void)
{
	debugLogA("WhatsAppProto::OnLoggedOut");
	m_bTerminated = true;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void WhatsAppProto::SendKeepAlive()
{
	WebSocket_SendText(m_hServerConn, "?,,");

	time_t now = time(0);

	for (auto &it : m_arUsers) {
		if (it->m_time1 && now - it->m_time1 >= 1200) { // 20 minutes
			setWord(it->hContact, "Status", ID_STATUS_NA);
			it->m_time1 = 0;
			it->m_time2 = now;
		}
		else if (it->m_time2 && now - it->m_time2 >= 1200) { // 20 minutes
			setWord(it->hContact, "Status", ID_STATUS_OFFLINE);
			it->m_time2 = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("WhatsAppProto::ShutdownSession");

	// shutdown all resources
	if (m_hServerConn)
		Netlib_Shutdown(m_hServerConn);

	OnLoggedOut();
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnIqPairDevice(const WANode &node)
{
	WANode reply("iq");
	reply << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("type", "result") << CHAR_PARAM("id", node.getAttr("id"));
	WSSendNode(reply);

	if (auto *pRef = node.getChild("pair-device")->getChild("ref")) {
		ShowQrCode(pRef->getBody());
	}
	else {
		debugLogA("OnIqPairDevice: got reply without ref, exiting");
		ShutdownSession();
	}
}

void WhatsAppProto::OnIqPairSuccess(const WANode &node)
{
	CloseQrDialog();

	auto *pRoot = node.getChild("pair-success");

	try {
		if (auto *pPlatform = pRoot->getChild("platform"))
			debugLogA("Got response from platform: %s", pPlatform->getBody().c_str());

		if (auto *pBiz = pRoot->getChild("biz"))
			if (auto *pszName = pBiz->getAttr("name"))
				setUString("Nick", pszName);

		if (auto *pDevice = pRoot->getChild("device")) {
			if (auto *pszJid = pDevice->getAttr("jid"))
				setUString("jid", pszJid);
		}
		else throw "OnIqPairSuccess: got reply without device info, exiting";

		if (auto *pIdentity = pRoot->getChild("device-identity")) {
			proto::ADVSignedDeviceIdentityHMAC payload;
			if (!payload.ParseFromArray(pIdentity->content.data(), (int)pIdentity->content.length()))
				throw "OnIqPairSuccess: got reply with invalid identity, exiting";

			auto &hmac = payload.hmac();
			debugLogA("Received HMAC signature: %s", hmac.c_str());

			auto &details = payload.details();
			Netlib_Dump(nullptr, details.c_str(), details.size(), false, 0);

			// check details signature using HMAC
			{
				uint8_t signature[32];
				unsigned int out_len = sizeof(signature);
				MBinBuffer secret(getBlob(DBKEY_SECRET_KEY));
				HMAC(EVP_sha256(), secret.data(), (int)secret.length(), (BYTE *)details.c_str(), (int)details.size(), signature, &out_len);
				if (memcmp(hmac.c_str(), signature, sizeof(signature)))
					throw "OnIqPairSuccess: got reply with invalid details signature, exiting";
			}

			proto::ADVSignedDeviceIdentity account;
			if (!account.ParseFromString(details))
				throw "OnIqPairSuccess: got reply with invalid account, exiting";

			auto &deviceDetails = account.details();
			auto &accountSignature = account.accountsignature();
			auto &accountSignatureKey = account.accountsignaturekey();
			{
				MBinBuffer buf;
				buf.append("\x06\x00", 2);
				buf.append(deviceDetails.c_str(), deviceDetails.size());
				buf.append(m_noise->signedIdentity.pub.data(), m_noise->signedIdentity.pub.length());

				ec_public_key key = {};
				memcpy(key.data, accountSignatureKey.c_str(), sizeof(key.data));
				if (1 != curve_verify_signature(&key, (BYTE *)buf.data(), buf.length(), (BYTE *)accountSignature.c_str(), accountSignature.size()))
					throw "OnIqPairSuccess: got reply with invalid account signature, exiting";
			}
			debugLogA("Received valid account signature");
			{
				MBinBuffer buf;
				buf.append("\x06\x01", 2);
				buf.append(deviceDetails.c_str(), deviceDetails.size());
				buf.append(m_noise->signedIdentity.pub.data(), m_noise->signedIdentity.pub.length());
				buf.append(accountSignatureKey.c_str(), accountSignatureKey.size());
				
				signal_buffer *result;
				ec_private_key key = {};
				memcpy(key.data, m_noise->signedIdentity.priv.data(), m_noise->signedIdentity.priv.length());
				if (curve_calculate_signature(g_plugin.pCtx, &result, &key, (BYTE *)buf.data(), buf.length()) != 0)
					throw "OnIqPairSuccess: cannot calculate account signature, exiting";

				account.set_devicesignature(result->data, result->len);
				signal_buffer_free(result);
			}

			setDword("SignalDeviceId", 0);
			{
				MBinBuffer key;
				if (accountSignatureKey.size() == 32)
					key.append(KEY_BUNDLE_TYPE, 1);
				key.append(accountSignatureKey.c_str(), accountSignatureKey.size());
				db_set_blob(0, m_szModuleName, "SignalIdentifierKey", key.data(), (int)key.length());
			}

			account.clear_accountsignaturekey();

			MBinBuffer accountEnc(account.ByteSize());
			account.SerializeToArray(accountEnc.data(), (int)accountEnc.length());
			db_set_blob(0, m_szModuleName, "WAAccount", accountEnc.data(), (int)accountEnc.length());

			proto::ADVDeviceIdentity deviceIdentity;
			deviceIdentity.ParseFromString(deviceDetails);

			WANode reply("iq");
			reply << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("type", "result") << CHAR_PARAM("id", node.getAttr("id"));

			WANode *nodePair = reply.addChild("pair-device-sign");

			WANode *nodeDeviceIdentity = nodePair->addChild("device-identity");
			nodeDeviceIdentity->addAttr("key-index", deviceIdentity.keyindex());
			nodeDeviceIdentity->content.append(accountEnc.data(), accountEnc.length());
			WSSendNode(reply);
		}
		else throw "OnIqPairSuccess: got reply without identity, exiting";
	}
	catch (const char *pErrMsg) {
		debugLogA(pErrMsg);
		ShutdownSession();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnProcessHandshake(const void *pData, int cbLen)
{
	proto::HandshakeMessage msg;
	if (!msg.ParseFromArray(pData, cbLen)) {
		debugLogA("Error parsing data, exiting");

LBL_Error:
		ShutdownSession();
		return;
	}

	auto &static_ = msg.serverhello().static_();
	auto &payload_ = msg.serverhello().payload();
	auto &ephemeral_ = msg.serverhello().ephemeral();

	// std::string ephemeral_("\xe5\x3d\xff\xc6\x59\xef\xa7\x64\xf6\x48\xf3\x46\x15\xb4\x4f\x13\xfa\xc6\x29\x18\x34\xd4\xa4\xa9\xad\xa0\xa3\x05\xaa\x4d\xda\x36", 32);
	// std::string static_("\xa9\x70\xc0\xfe\xe0\x72\x3b\x0a\x6e\x0a\xd4\xe1\xb4\x5f\xcd\xb8\x06\x68\xdd\x6b\x45\x36\xe0\x7d\x0d\xe9\x00\x4d\xb6\xaf\xfa\xa3\xb7\x54\x82\x24\xa9\xe4\x2c\xd4\xe5\xd0\x2f\xd6\x31\x2b\xca\xec", 48);
	// std::string payload_("\x11\xcc\xb8\x74\xe6\x27\x29\x37\x65\xb2\x9e\x47\x53\x89\xf7\xce\x23\x03\x9c\xd4\x9e\x4b\x12\xdc\x3f\x10\xe4\x68\xfe\xfd\x31\x80\x1d\x48\x01\x9c\x31\xef\x54\xdb\xa1\x8f\xdb\x6b\x53\x84\xbb\x6d\xb4\x0c\x61\x1f\xcd\xe7\x3c\x0e\xe2\x18\xe4\x95\xf7\xbc\x5b\xbf\x80\x93\x21\x98\x80\x20\x9b\x71\x27\x47\x39\xe9\x04\x08\x5d\xd2\x62\x48\xf6\x23\xba\xa0\x31\xfc\x7c\xeb\xa0\xaa\x56\x04\x71\x71\x84\x9b\x08\xa4\xc9\x33\xd5\x07\x04\x5f\x1c\xd2\x6f\x7d\x5d\x83\x29\x5f\x80\x4a\xbf\x7c\xd9\x7c\xd0\x2b\x9a\x1e\xe0\x28\x33\x89\xb5\x3b\xd2\xe7\x7f\xfc\xd6\xa8\x55\xe2\x9c\x6e\x5f\x6f\x08\xa1\xf3\xfd\x5e\xff\x56\xee\x6f\x31\x47\xeb\xd4\x07\x92\x81\x72\x68\x91\x9d\xe9\xb3\x5f\x1d\x61\x8e\xce\x55\x4b\xbe\x74\x5d\xef\xea\xe4\x23\x63\x0d\x7c\xd4\xa3\xf8\xa1\x29\x60\xd7\x2c\xe8\xfc\xb5\x89\x99\x32\x95\xfc\xec\x6f\x7b\x2a\x23\xf4\x75\xbe\xe3\x21\x6a\x71\x3f\xc4\x1b\x99\x9f\x42\xd5\x19\xd8\xcc\xe7\xab\x90\xdd\xe5\xd8\xa5\xe3\xb5\x5c\xa2\x54\xf3\x4b\x0c\xa1\xe2\xa2\x91\xa3\xd0\x92\x6d\xfa\xab\x5a\xf6\x80\xee\x84\xbe\xaa\x75\x5e\xee\x6b\x91\x49", 257);

	m_noise->updateHash(ephemeral_.c_str(), ephemeral_.size());
	m_noise->mixIntoKey(m_noise->ephemeral.priv.data(), ephemeral_.c_str());

	MBinBuffer decryptedStatic = m_noise->decrypt(static_.c_str(), static_.size());
	m_noise->mixIntoKey(m_noise->ephemeral.priv.data(), decryptedStatic.data());

	MBinBuffer decryptedCert = m_noise->decrypt(payload_.c_str(), payload_.size());

	proto::CertChain cert; cert.ParseFromArray(decryptedCert.data(), (int)decryptedCert.length());
	proto::CertChain::NoiseCertificate::Details details; details.ParseFromString(cert.intermediate().details());
	if (details.issuerserial() != 0) {
		debugLogA("Invalid certificate serial number, exiting");
		goto LBL_Error;
	}

	MBinBuffer encryptedPub = m_noise->encrypt(m_noise->noiseKeys.pub.data(), m_noise->noiseKeys.pub.length());
	m_noise->mixIntoKey(m_noise->noiseKeys.priv.data(), ephemeral_.c_str());

	// create reply
	proto::ClientPayload node;

	MFileVersion v;
	Miranda_GetFileVersion(&v);

	// generate registration packet
	if (m_szClientToken.IsEmpty()) {
		uint8_t appVersion[16];
		mir_md5_hash((BYTE*)APP_VERSION, sizeof(APP_VERSION) - 1, appVersion);

		auto *pAppVersion = new proto::DeviceProps_AppVersion();
		pAppVersion->set_primary(v[0]);
		pAppVersion->set_secondary(v[1]);
		pAppVersion->set_tertiary(v[2]);
		pAppVersion->set_quaternary(v[3]);

		proto::DeviceProps pCompanion;
		pCompanion.set_os("Miranda");
		pCompanion.set_allocated_version(pAppVersion);
		pCompanion.set_platformtype(proto::DeviceProps_PlatformType_DESKTOP);
		pCompanion.set_requirefullsync(false);

		MBinBuffer buf(pCompanion.ByteSize());
		pCompanion.SerializeToArray(buf.data(), (int)buf.length());

		auto *pPairingData = new proto::ClientPayload_DevicePairingRegistrationData();
		pPairingData->set_deviceprops(buf.data(), buf.length());
		pPairingData->set_buildhash(appVersion, sizeof(appVersion));
		pPairingData->set_eregid(encodeBigEndian(getDword(DBKEY_REG_ID)));
		pPairingData->set_ekeytype(KEY_BUNDLE_TYPE);
		pPairingData->set_eident(m_noise->signedIdentity.pub.data(), m_noise->signedIdentity.pub.length());
		pPairingData->set_eskeyid(encodeBigEndian(m_noise->preKey.keyid));
		pPairingData->set_eskeyval(m_noise->preKey.pub.data(), m_noise->preKey.pub.length());
		pPairingData->set_eskeysig(m_noise->preKey.signature.data(), m_noise->preKey.signature.length());
		node.set_allocated_devicepairingdata(pPairingData);

		node.set_passive(false);
	}
	// generate login packet
	else {
		node.set_passive(true);
	}

	auto *pUserVersion = new proto::ClientPayload_UserAgent_AppVersion();
	pUserVersion->set_primary(2);
	pUserVersion->set_secondary(2230);
	pUserVersion->set_tertiary(15);

	auto *pUserAgent = new proto::ClientPayload_UserAgent();
	pUserAgent->set_allocated_appversion(pUserVersion);
	pUserAgent->set_platform(proto::ClientPayload_UserAgent_Platform_WEB);
	pUserAgent->set_releasechannel(proto::ClientPayload_UserAgent_ReleaseChannel_RELEASE);
	pUserAgent->set_mcc("000");
	pUserAgent->set_mnc("000");
	pUserAgent->set_osversion("0.1");
	pUserAgent->set_osbuildnumber("0.1");
	pUserAgent->set_manufacturer("");
	pUserAgent->set_device("Desktop");
	pUserAgent->set_localelanguageiso6391("en");
	pUserAgent->set_localecountryiso31661alpha2("US");

	auto *pWebInfo = new proto::ClientPayload_WebInfo();
	pWebInfo->set_websubplatform(proto::ClientPayload_WebInfo_WebSubPlatform_WEB_BROWSER);

	node.set_connecttype(proto::ClientPayload_ConnectType_WIFI_UNKNOWN);
	node.set_connectreason(proto::ClientPayload_ConnectReason_USER_ACTIVATED);
	node.set_allocated_useragent(pUserAgent);
	node.set_allocated_webinfo(pWebInfo);

	MBinBuffer payload(node.ByteSize());
	node.SerializeToArray(payload.data(), (int)payload.length());

	MBinBuffer payloadEnc = m_noise->encrypt(payload.data(), payload.length());

	auto *pFinish = new proto::HandshakeMessage_ClientFinish();
	pFinish->set_payload(payloadEnc.data(), payloadEnc.length());
	pFinish->set_static_(encryptedPub.data(), encryptedPub.length());

	proto::HandshakeMessage handshake;
	handshake.set_allocated_clientfinish(pFinish);
	WSSend(handshake);

	m_noise->finish();
}

/////////////////////////////////////////////////////////////////////////////////////////
// gateway worker thread

bool WhatsAppProto::WSReadPacket(const WSHeader &hdr, MBinBuffer &res)
{
	size_t currPacketSize = res.length() - hdr.headerSize;

	char buf[1024];
	while (currPacketSize < hdr.payloadSize) {
		int result = Netlib_Recv(m_hServerConn, buf, _countof(buf), MSG_NODUMP);
		if (result == 0) {
			debugLogA("Gateway connection gracefully closed");
			return false;
		}
		if (result < 0) {
			debugLogA("Gateway connection error, exiting");
			return false;
		}
		
		currPacketSize += result;
		res.append(buf, result);
	}
	return true;
}

void WhatsAppProto::ServerThread(void *)
{
	m_bTerminated = false;

	while (ServerThreadWorker())
		;
	ShutdownSession();
}

bool WhatsAppProto::ServerThreadWorker()
{
	// connect websocket
	NETLIBHTTPHEADER hdrs[] =
	{
		{ "Origin", "https://web.whatsapp.com" },
		{ 0, 0 }
	};

	NLHR_PTR pReply(WebSocket_Connect(m_hNetlibUser, "web.whatsapp.com/ws/chat", hdrs));
	if (pReply == nullptr) {
		debugLogA("Server connection failed, exiting");
		return false;
	}

	if (pReply->resultCode != 101)
		return false;

	delete m_noise;
	m_noise = new WANoise(this);
	m_noise->init();

	debugLogA("Server connection succeeded");
	m_hServerConn = pReply->nlc;
	m_iLoginTime = time(0);
	m_szClientToken = getMStringA(DBKEY_CLIENT_TOKEN);

	auto &pubKey = m_noise->ephemeral.pub;
	auto *client = new proto::HandshakeMessage::ClientHello(); client->set_ephemeral(pubKey.data(), pubKey.length());
	proto::HandshakeMessage msg; msg.set_allocated_clienthello(client);
	WSSend(msg);

	bool bExit = false;
	MBinBuffer netbuf;

	while (!bExit && !m_bTerminated) {
		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hServerConn, (char *)buf, _countof(buf), MSG_NODUMP);
		if (bufSize == 0) {
			debugLogA("Gateway connection gracefully closed");
			break;
		}
		if (bufSize < 0) {
			debugLogA("Gateway connection error, exiting");
			break;
		}

		netbuf.append(buf, bufSize);

		WSHeader hdr;
		if (!WebSocket_InitHeader(hdr, netbuf.data(), netbuf.length()))
			continue;
		
		// we lack some data, let's read them
		if (netbuf.length() < hdr.headerSize + hdr.payloadSize)
			if (!WSReadPacket(hdr, netbuf))
				break;

		debugLogA("Got packet: buffer = %d, opcode = %d, headerSize = %d, payloadSize = %d, final = %d, masked = %d", 
			netbuf.length(), hdr.opCode, hdr.headerSize, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);
		// Netlib_Dump(m_hServerConn, netbuf.data(), netbuf.length(), false, 0);

		// read all payloads from the current buffer, one by one
		while (true) {
			MBinBuffer currPacket;
			currPacket.assign(netbuf.data() + hdr.headerSize, hdr.payloadSize);
			currPacket.append("", 1); // add 0 to use strchr safely
			
			const char *start = currPacket.data();

			switch (hdr.opCode) {
			case 1: // json packet
			case 2: // binary packet
				// process a packet here
				{
					const char *pos = strchr(start, ',');
					if (pos != nullptr)
						pos++;
					else
						pos = start;
					size_t dataSize = hdr.payloadSize - size_t(pos - start);

					// try to decode
					if (hdr.opCode == 2 && hdr.payloadSize > 32)
						ProcessBinaryPacket(pos, dataSize);
					else {
						CMStringA szJson(pos, (int)dataSize);

						JSONNode root = JSONNode::parse(szJson);
						if (root) {
							debugLogA("JSON received:\n%s", start);

							CMStringA szPrefix(start, int(pos - start - 1));
							auto *pReq = m_arPacketQueue.find((WARequest *)&szPrefix);
							if (pReq != nullptr) {
								root << CHAR_PARAM("$id$", szPrefix);
							}
							else ProcessPacket(root);
						}
					}
				}
				break;

			case 8: // close
				debugLogA("server required to exit");
				bExit = true; // simply reconnect, don't exit
				break;

			default:
				Netlib_Dump(m_hServerConn, start, hdr.payloadSize, false, 0);
			}

			netbuf.remove(hdr.headerSize + hdr.payloadSize);
			debugLogA("%d bytes removed from network buffer, %d bytes remain", hdr.headerSize + hdr.payloadSize, netbuf.length());
			if (netbuf.length() == 0)
				break;

			// if we have not enough data for header, continue reading
			if (!WebSocket_InitHeader(hdr, netbuf.data(), netbuf.length())) {
				debugLogA("not enough data for header, continue reading");
				break;
			}

			// if we have not enough data for data, continue reading
			if (hdr.headerSize + hdr.payloadSize > netbuf.length()) {
				debugLogA("not enough place for data (%d+%d > %d), continue reading", hdr.headerSize, hdr.payloadSize, netbuf.length());
				break;
			}

			debugLogA("Got inner packet: buffer = %d, opcode = %d, headerSize = %d, payloadSize = %d, final = %d, masked = %d", 
				netbuf.length(), hdr.opCode, hdr.headerSize, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);
		}
	}

	debugLogA("Server connection dropped");
	Netlib_CloseHandle(m_hServerConn);
	m_hServerConn = nullptr;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Binary data processing

void WhatsAppProto::ProcessBinaryPacket(const void *pData, size_t cbDataLen)
{
	while (size_t payloadLen = m_noise->decodeFrame(pData, cbDataLen)) {
		if (m_noise->bInitFinished) {
			MBinBuffer buf = m_noise->decrypt(pData, payloadLen);

			WAReader rdr(buf.data(), buf.length());
			auto b = rdr.readInt8();
			if (b & 2) {
				debugLogA("zipped nodes are not supported");
				return;
			}

			if (WANode *pNode = rdr.readNode()) {
				CMStringA szText;
				pNode->print(szText);
				debugLogA("Got binary node:\n%s", szText.c_str());

				ProcessBinaryNode(*pNode);
				delete pNode;
			}
			else {
				debugLogA("wrong or broken payload");
				Netlib_Dump(m_hServerConn, pData, cbDataLen, false, 0);
			}
		}
		else OnProcessHandshake(pData, (int)payloadLen);

		pData = (BYTE*)pData + payloadLen;
		cbDataLen -= payloadLen;
	}
}

void WhatsAppProto::ProcessBinaryNode(const WANode &node)
{
	if (m_arPacketQueue.getCount()) {
		WARequest req = m_arPacketQueue[0];
		m_arPacketQueue.remove(0);

		(this->*req.pHandler)(node);
		return;
	}

	auto pHandler = FindPersistentHandler(node);
	if (pHandler)
		(this->*pHandler)(node);
	else
		debugLogA("cannot handle incoming message");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Json data processing

void WhatsAppProto::ProcessPacket(const JSONNode &root)
{
	CMStringA szType = root[(size_t)0].as_mstring();
	const JSONNode &content = root[1];

	if (szType == "Conn")
		ProcessConn(content);
	else if (szType == "Cmd")
		ProcessCmd(content);
	else if (szType == "Presence")
		ProcessPresence(content);
	else if (szType == "Blocklist")
		ProcessBlocked(content);
}

void WhatsAppProto::ProcessBlocked(const JSONNode &node)
{
	for (auto &it : node["blocklist"]) {
		auto *pUser = AddUser(it.as_string().c_str(), false);
		Ignore_Ignore(pUser->hContact, IGNOREEVENT_ALL);
		Contact::Hide(pUser->hContact);
	}
}

void WhatsAppProto::ProcessCmd(const JSONNode &root)
{
	CMStringW wszType(root["type"].as_mstring());
	if (wszType == L"challenge") {
	}
}

void WhatsAppProto::ProcessConn(const JSONNode &root)
{
	CloseQrDialog();

	writeStr(DBKEY_ID, root["wid"]);
	m_szJid = getMStringA(DBKEY_ID);

	writeStr(DBKEY_NICK, root["pushname"]);
	writeStr(DBKEY_CLIENT_TOKEN, root["clientToken"]);
	writeStr(DBKEY_SERVER_TOKEN, root["serverToken"]);
	writeStr(DBKEY_BROWSER_TOKEN, root["browserToken"]);

	OnLoggedIn();
}

void WhatsAppProto::ProcessPresence(const JSONNode &root)
{
	CMStringA jid = root["id"].as_mstring();
	if (auto *pUser = FindUser(jid)) {
		CMStringA state(root["type"].as_mstring());
		DWORD timestamp(_wtoi(root["t"].as_mstring()));
		if (state == "available") {
			setWord(pUser->hContact, "Status", ID_STATUS_ONLINE);
		}
		else if (state == "unavailable") {
			setWord(pUser->hContact, "Status", ID_STATUS_AWAY);
			pUser->m_time1 = timestamp;
		}
	}
	else debugLogA("Presence from unknown contact %s ignored", jid.c_str());
}
