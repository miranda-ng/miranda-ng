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

bool WhatsAppProto::ProcessHandshake(const MBinBuffer &keyEnc)
{
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
		pCompanion.set_requirefullsync(true);

		MBinBuffer buf(pCompanion.ByteSize());
		pCompanion.SerializeToArray(buf.data(), (int)buf.length());

		auto *pPairingData = new proto::ClientPayload_DevicePairingRegistrationData();
		pPairingData->set_deviceprops(buf.data(), buf.length());
		pPairingData->set_buildhash(appVersion, sizeof(appVersion));

		MBinBuffer tmp = encodeBigEndian(getDword(DBKEY_REG_ID));
		pPairingData->set_eregid(tmp.data(), tmp.length());

		node.set_allocated_devicepairingdata(pPairingData);
	}
	// generate login packet
	else {
		node.set_passive(true);
	}

	auto *pUserVersion = new proto::ClientPayload_UserAgent_AppVersion();
	pUserVersion->set_primary(v[0]);
	pUserVersion->set_secondary(v[1]);
	pUserVersion->set_tertiary(v[2]);
	pUserVersion->set_quaternary(v[3]);

	auto *pUserAgent = new proto::ClientPayload_UserAgent();
	pUserAgent->set_allocated_appversion(pUserVersion);
	pUserAgent->set_platform(proto::ClientPayload_UserAgent_Platform_WINDOWS);
	pUserAgent->set_releasechannel(proto::ClientPayload_UserAgent_ReleaseChannel_RELEASE);
	pUserAgent->set_mcc("000");
	pUserAgent->set_mnc("000");
	pUserAgent->set_osversion("10.0");
	pUserAgent->set_osbuildnumber("10.0");
	pUserAgent->set_manufacturer("");
	pUserAgent->set_device("Desktop");
	pUserAgent->set_localelanguageiso6391("en");
	pUserAgent->set_localecountryiso31661alpha2("US");

	auto *pWebInfo = new proto::ClientPayload_WebInfo();
	pWebInfo->set_websubplatform(proto::ClientPayload_WebInfo_WebSubPlatform_WINDA);

	node.set_connecttype(proto::ClientPayload_ConnectType_WIFI_UNKNOWN);
	node.set_connectreason(proto::ClientPayload_ConnectReason_USER_ACTIVATED);
	node.set_allocated_useragent(pUserAgent);
	node.set_allocated_webinfo(pWebInfo);
	return true;
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

void WhatsAppProto::OnStartSession(const JSONNode &root, void*)
{
	int status = root["status"].as_int();
	if (status != 200) {
		debugLogA("Session start failed with error %d", status);
		ShutdownSession();
		return;
	}

	CMStringA ref = root["ref"].as_mstring();
	ShowQrCode(ref);
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
	m_iPktNumber = 0;
	m_szClientToken = getMStringA(DBKEY_CLIENT_TOKEN);

	auto &pubKey = m_noise->noiseKeys.pub;
	ptrA szPubKey(mir_base64_encode(pubKey.data(), pubKey.length()));
	auto *client = new proto::HandshakeMessage::ClientHello(); client->set_ephemeral(pubKey.data(), pubKey.length());
	proto::HandshakeMessage msg; msg.set_allocated_clienthello(client);
	WSSend(msg, &WhatsAppProto::OnStartSession);

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
						ProcessBinaryPacket((const uint8_t*)pos, dataSize);
					else {
						CMStringA szJson(pos, (int)dataSize);

						JSONNode root = JSONNode::parse(szJson);
						if (root) {
							debugLogA("JSON received:\n%s", start);

							CMStringA szPrefix(start, int(pos - start - 1));
							auto *pReq = m_arPacketQueue.find((WARequest *)&szPrefix);
							if (pReq != nullptr) {
								root << CHAR_PARAM("$id$", szPrefix);
								(this->*pReq->pHandler)(root, pReq->pUserInfo);
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

void WhatsAppProto::ProcessBinaryPacket(const uint8_t *pData, size_t cbDataLen)
{
	Netlib_Dump(m_hServerConn, pData, cbDataLen, false, 0);

	while (cbDataLen > 3) {
		size_t payloadLen = 0;
		for (int i = 0; i < 3; i++) {
			payloadLen <<= 8;
			payloadLen += pData[i];
		}

		pData += 3;
		cbDataLen -= 3;
		debugLogA("got payload of size %d", payloadLen);

		if (payloadLen > cbDataLen) {
			debugLogA("payload length exceeds capacity %d", cbDataLen);
			return;
		}

		if (!m_noise->decodeFrame(pData, payloadLen))
			debugLogA("cannot decrypt incoming message");

		pData += payloadLen;
		cbDataLen -= payloadLen;
	}
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
