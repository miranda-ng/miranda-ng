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

bool WhatsAppProto::ProcessChallenge(const CMStringA &szChallenge)
{
	if (szChallenge.IsEmpty() || mac_key.isEmpty())
		return false;

	size_t cbLen;
	void *pChallenge = mir_base64_decode(szChallenge, &cbLen);

	BYTE digest[32];
	unsigned cbResult = sizeof(digest);
	HMAC(EVP_sha256(), mac_key.data(), (int)mac_key.length(), (BYTE*)pChallenge, (int)cbLen, digest, &cbResult);

	ptrA szServer(getStringA(DBKEY_SERVER_TOKEN));
	//CMStringA payload(FORMAT, "[\"admin\",\"challenge\",\"%s\",\"%s\",\"%s\"]",
//		ptrA(mir_base64_encode(digest, cbResult)).get(), szServer.get(), m_szClientId.c_str());
	//WSSend(payload);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool WhatsAppProto::ProcessSecret(const CMStringA &szSecret)
{
	if (szSecret.IsEmpty())
		return false;

	size_t secretLen = 0;
	mir_ptr<BYTE> pSecret((BYTE *)mir_base64_decode(szSecret, &secretLen));
	if (pSecret == nullptr || secretLen != 144) {
		debugLogA("Invalid secret key, dropping session (secret len = %u", (unsigned)secretLen);
		return false;
	}

	ec_public_key pPeerPublic;
	memcpy(pPeerPublic.data, pSecret, 32);

	MBinBuffer privKey;
	if (!getBlob(DBKEY_PRIVATE_KEY, privKey))
		return false;

	ec_private_key pMyPrivate;
	memcpy(pMyPrivate.data, privKey.data(), 32);

	uint8_t *pSharedKey, *pSharedExpanded;
	int sharedLen = curve_calculate_agreement(&pSharedKey, &pPeerPublic, &pMyPrivate);
	{
		BYTE salt[32], md[32];
		unsigned int md_len = 32;
		memset(salt, 0, sizeof(salt));
		HMAC(EVP_sha256(), salt, sizeof(salt), pSharedKey, sharedLen, md, &md_len);

		hkdf_context *pHKDF;
		hkdf_create(&pHKDF, 3, g_plugin.pCtx);
		hkdf_expand(pHKDF, &pSharedExpanded, md, sizeof(md), 0, 0, 80);
		hkdf_destroy(pHKDF);
	}

	// validation
	{
		unsigned int md_len = 32;
		BYTE sum[32], md[32], enc[112], *key = pSharedExpanded + 32;
		memcpy(enc, pSecret, 32);
		memcpy(enc + 32, (BYTE *)pSecret + 64, 80);
		memcpy(sum, (BYTE *)pSecret + 32, 32);
		HMAC(EVP_sha256(), key, 32, enc, sizeof(enc), md, &md_len);
		if (memcmp(md, sum, 32)) {
			debugLogA("Secret key validation failed, exiting");
			return false;
		}
	}

	// woohoo, everything is ok, decrypt keys
	{
		BYTE enc[80], dec[112], key[32], iv[16];
		memcpy(key, pSharedExpanded, sizeof(key));
		memcpy(iv, pSharedExpanded+64, sizeof(iv));
		memcpy(enc, pSecret.get() + 64, sizeof(enc));

		int dec_len = 0, final_len = 0;
		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
		EVP_DecryptUpdate(ctx, dec, &dec_len, enc, sizeof(enc));
		EVP_DecryptFinal_ex(ctx, dec + dec_len, &final_len);
		dec_len += final_len;
		EVP_CIPHER_CTX_free(ctx);

		enc_key.assign(dec, 32);
		mac_key.assign(dec + 32, 32);

		db_set_blob(0, m_szModuleName, DBKEY_ENC_KEY, enc_key.data(), (int)enc_key.length());
		db_set_blob(0, m_szModuleName, DBKEY_MAC_KEY, mac_key.data(), (int)mac_key.length());
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnRestoreSession1(const JSONNode&, void*)
{
	ptrA szClient(getStringA(DBKEY_CLIENT_TOKEN)), szServer(getStringA(DBKEY_SERVER_TOKEN));
	if (szClient == nullptr || szServer == nullptr) {
		ShutdownSession();
		return;
	}

	// CMStringA payload(FORMAT, "[\"admin\",\"login\",\"%s\",\"%s\",\"%s\",\"takeover\"]", szClient.get(), szServer.get(), m_szClientId.c_str());
	// WSSend(payload, &WhatsAppProto::OnRestoreSession2);
}

void WhatsAppProto::OnRestoreSession2(const JSONNode &root, void*)
{
	int status = root["status"].as_int();
	if (status != 200) {
		debugLogA("Attempt to restore session failed with error %d", status);

		if (status == 401 || status == 419) {
			POPUPDATAW Popup = {};
			Popup.lchIcon = IcoLib_GetIconByHandle(Skin_GetIconHandle(SKINICON_ERROR));
			wcsncpy_s(Popup.lpwzText, TranslateT("You need to launch WhatsApp on your phone"), _TRUNCATE);
			wcsncpy_s(Popup.lpwzContactName, m_tszUserName, _TRUNCATE);
			Popup.iSeconds = 10;
			PUAddPopupW(&Popup);
		}

		ShutdownSession();
		return;
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
	m_noise = new WANoise();

	debugLogA("Server connection succeeded");
	m_hServerConn = pReply->nlc;
	m_iLoginTime = time(0);
	m_iPktNumber = 0;
	m_szClientToken = getMStringA(DBKEY_CLIENT_TOKEN);

	MFileVersion v;
	Miranda_GetFileVersion(&v);

	auto &pubKey = m_noise->getPub();
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
		CMStringA szChallenge(root["challenge"].as_mstring());
		if (!ProcessChallenge(szChallenge)) {
			ShutdownSession();
			return;
		}
	}
}

void WhatsAppProto::ProcessConn(const JSONNode &root)
{
	CloseQrDialog();

	writeStr(DBKEY_ID, root["wid"]);
	m_szJid = getMStringA(DBKEY_ID);

	CMStringA szSecret(root["secret"].as_mstring());
	if (!szSecret.IsEmpty())
		if (!ProcessSecret(szSecret)) {
			ShutdownSession();
			return;
		}

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
