/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

int WhatsAppProto::WSSend(const CMStringA &str, WA_PKT_HANDLER pHandler, void *pUserInfo)
{
	if (m_hServerConn == nullptr)
		return -1;

	int pktId = ++m_iPktNumber;

	CMStringA buf;
	buf.Format("%d.--%d", (int)m_iLoginTime, pktId);

	if (pHandler != nullptr) {
		auto *pReq = new WARequest;
		pReq->pHandler = pHandler;
		pReq->szPrefix = buf;
		pReq->pUserInfo = pUserInfo;

		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(pReq);
	}

	buf.AppendChar(',');
	if (!str.IsEmpty()) {
		buf.AppendChar(',');
		buf += str;
	}


	debugLogA("Sending packet #%d: %s", pktId, buf.c_str());
	WebSocket_SendText(m_hServerConn, buf.c_str());
	return pktId;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char zeroData[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int WhatsAppProto::WSSendNode(const char *pszPrefix, WAMetric metric, int flags, WANode &node, WA_PKT_HANDLER pHandler)
{
	if (m_hServerConn == nullptr)
		return 0;

	{
		char str[100];
		_i64toa(_time64(0), str, 10);
		node.addAttr("epoch", str);

		CMStringA szText;
		node.print(szText);
		debugLogA("Sending binary node: %s", szText.c_str());
	}

	WAWriter writer;
	writer.writeNode(&node);

	// AES block size = 16 bytes, let's expand data to block size boundary
	size_t rest = writer.body.length() % 16;
	if (rest != 0)
		writer.body.append(zeroData, 16 - rest);

	BYTE iv[16];
	Utils_GetRandom(iv, sizeof(iv));

	// allocate the buffer of the same size + 32 bytes for temporary operations
	MBinBuffer enc;
	enc.assign(writer.body.data(), writer.body.length()); 
	enc.append(mac_key.data(), mac_key.length()); 

	int enc_len = 0, final_len = 0;
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (BYTE*)enc_key.data(), iv);
	EVP_EncryptUpdate(ctx, (BYTE*)enc.data(), &enc_len, (BYTE*)writer.body.data(), (int)writer.body.length());
	EVP_EncryptFinal_ex(ctx, (BYTE*)enc.data() + enc_len, &final_len);
	EVP_CIPHER_CTX_free(ctx);

	// build the resulting buffer of the following structure:
	// - packet prefix
	// - 32 bytes of HMAC
	// - 16 bytes of iv
	// - rest of encoded data
	
	BYTE hmac[32];
	unsigned int hmac_len = 32;
	HMAC(EVP_sha256(), mac_key.data(), (int)mac_key.length(), (BYTE*)enc.data(), enc_len, hmac, &hmac_len);

	int pktId = ++m_iPktNumber;

	if (pHandler != nullptr) {
		auto *pReq = new WARequest;
		pReq->pHandler = pHandler;
		pReq->szPrefix = pszPrefix;

		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(pReq);
	}

	char postPrefix[3] = { ',', (char)metric, (char)flags };

	MBinBuffer ret;
	ret.append(pszPrefix, strlen(pszPrefix));
	ret.append(postPrefix, sizeof(postPrefix));
	ret.append(hmac, sizeof(hmac));
	ret.append(iv, sizeof(iv));
	ret.append(enc.data(), enc_len);
	WebSocket_SendBinary(m_hServerConn, ret.data(), ret.length());

	return pktId;
}

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
	m_iPktNumber = 0;

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
	CMStringA payload(FORMAT, "[\"admin\",\"challenge\",\"%s\",\"%s\",\"%s\"]",
		ptrA(mir_base64_encode(digest, cbResult)).get(), szServer.get(), m_szClientId.c_str());
	WSSend(payload);
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

	CMStringA payload(FORMAT, "[\"admin\",\"login\",\"%s\",\"%s\",\"%s\",\"takeover\"]", szClient.get(), szServer.get(), m_szClientId.c_str());
	WSSend(payload, &WhatsAppProto::OnRestoreSession2);
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
	NETLIBHTTPHEADER hdrs[] =
	{
		{ "Origin", "https://web.whatsapp.com" },
		{ 0, 0 }
	};

	NLHR_PTR pReply(WebSocket_Connect(m_hNetlibUser, "web.whatsapp.com/ws", hdrs));
	if (pReply == nullptr) {
		debugLogA("Server connection failed, exiting");
		return false;
	}

	if (pReply->resultCode != 101)
		return false;

	debugLogA("Server connection succeeded");
	m_hServerConn = pReply->nlc;
	m_iLoginTime = time(0);
	m_szClientToken = getMStringA(DBKEY_CLIENT_TOKEN);
	getBlob(DBKEY_ENC_KEY, enc_key);
	getBlob(DBKEY_MAC_KEY, mac_key);

	MFileVersion v;
	Miranda_GetFileVersion(&v);

	CMStringA payload(FORMAT, "[\"admin\",\"init\",[0,3,4940],[\"Windows\",\"Chrome\",\"10\"],\"%s\",true]", m_szClientId.c_str());
	if (m_szClientToken.IsEmpty() || mac_key.isEmpty() || enc_key.isEmpty())
		WSSend(payload, &WhatsAppProto::OnStartSession);
	else
		WSSend(payload, &WhatsAppProto::OnRestoreSession1);

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

void WhatsAppProto::ProcessBinaryPacket(const void *pData, size_t cbDataLen)
{
	MBinBuffer dest;
	if (!decryptBinaryMessage(cbDataLen, pData, dest)) {
		debugLogA("cannot decrypt incoming message");
		Netlib_Dump(m_hServerConn, pData, cbDataLen, false, 0);
		return;
	}

	WAReader reader(dest.data(), dest.length());
	WANode *pRoot = reader.readNode();
	if (pRoot == nullptr) { // smth went wrong
		debugLogA("cannot read binary message");
		Netlib_Dump(m_hServerConn, dest.data(), dest.length(), false, 0);
		return;
	}

	CMStringA szText;
	pRoot->print(szText);
	debugLogA("packed content: %s", szText.c_str());
	
	CMStringA szType = pRoot->getAttr("type");
	if (szType == "contacts")
		ProcessContacts(pRoot);
	else if (szType == "chat")
		ProcessChats(pRoot);
	else {
		CMStringA szAdd = pRoot->getAttr("add");
		if (!szAdd.IsEmpty())
			ProcessAdd(szAdd, pRoot);
	}

	delete pRoot;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ProcessAdd(const CMStringA &type, const WANode *root)
{
	for (auto &p: root->children) {
		proto::WebMessageInfo payLoad;
		if (!payLoad.ParseFromArray(p->content.data(), (int)p->content.length())) {
			debugLogA("Error: message failed to decode by protobuf!");
			Netlib_Dump(m_hServerConn, p->content.data(), p->content.length(), false, 0);
			continue;
		}

		// this part is common for all types of messages
		auto &key= payLoad.key();

		// if this event already exists in the database - skip it
		if (db_event_getById(m_szModuleName, key.id().c_str()))
			continue;

		CMStringA jid(key.remotejid().c_str());
		if (!jid.Replace("@s.whatsapp.net", "@c.us"))
			jid.Replace("@g.whatsapp.net", "@g.us");

		auto *pUser = AddUser(jid, false);
		DWORD dwTimestamp = DWORD(payLoad.messagetimestamp());
		CMStringA szMsgText;

		// regular messages
		if (payLoad.has_message()) {
			auto &msg = payLoad.message();
			szMsgText = msg.conversation().c_str();
		}

		if (!szMsgText.IsEmpty()) {
			if (pUser->si) {
				CMStringA szText(szMsgText);
				szText.Replace("%", "%%");

				CMStringA szUserJid(payLoad.participant().c_str());
				if (!szUserJid.Replace("@s.whatsapp.net", "@c.us"))
					szUserJid.Replace("@g.whatsapp.net", "@g.us");

				if (pUser->bInited) {
					GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
					gce.pszID.a = jid;
					gce.dwFlags = GCEF_ADDTOLOG | GCEF_UTF8;
					gce.pszUID.a = szUserJid;
					gce.pszText.a = szText;
					gce.time = dwTimestamp;
					gce.bIsMe = (jid == m_szJid);
					Chat_Event(&gce);
				}
				else {
					auto *pMsg = new WAHistoryMessage;
					pMsg->jid = jid;
					pMsg->text = szText;
					pMsg->timestamp = dwTimestamp;
					pUser->arHistory.insert(pMsg);
				}
			}
			else {
				PROTORECVEVENT pre = { 0 };
				pre.timestamp = dwTimestamp;
				pre.szMessage = szMsgText.GetBuffer();
				pre.flags = (type == "relay" ? 0 : PREF_CREATEREAD);
				pre.szMsgId = key.id().c_str();
				if (key.fromme())
					pre.flags |= PREF_SENT;
				ProtoChainRecvMsg(pUser->hContact, &pre);
			}
		}
	}
}

void WhatsAppProto::ProcessChats(const WANode *root)
{
	for (auto &p: root->children) {
		CMStringA jid = p->getAttr("jid");
		auto *pUser = AddUser(jid, false);

		DWORD dwLastId = atoi(p->getAttr("t"));
		setDword(pUser->hContact, "LastWriteTime", dwLastId);

		pUser->dwModifyTag = atoi(p->getAttr("modify_tag"));

		if (pUser->si) {
			DWORD dwMute = atoi(p->getAttr("mute"));
			Chat_Mute(pUser->si, dwMute ? CHATMODE_MUTE : CHATMODE_NORMAL);
		}
	}
}

void WhatsAppProto::ProcessContacts(const WANode *root)
{
	for (auto &p: root->children) {
		CMStringA jid(p->getAttr("jid"));
		auto *pUser = AddUser(jid, false);

		if (strstr(pUser->szId, "@g.us")) {
			InitChat(pUser, p);
			continue;
		}
		
		CMStringA wszNick(p->getAttr("notify"));
		if (wszNick.IsEmpty()) {
			int idx = jid.Find('@');
			wszNick = (idx == -1) ? jid : jid.Left(idx);
		}
		setUString(pUser->hContact, "Nick", wszNick);

		CMStringA wszFullName(p->getAttr("name"));
		wszFullName.TrimRight();
		if (!wszFullName.IsEmpty()) {
			setUString(pUser->hContact, "FullName", wszFullName);

			CMStringA wszShort(p->getAttr("short"));
			wszShort.TrimRight();
			if (!wszShort.IsEmpty()) {
				setUString(pUser->hContact, "FirstName", wszShort);
				if (wszShort.GetLength()+1 < wszFullName.GetLength())
					setUString(pUser->hContact, "LastName", wszFullName.c_str() + 1 + wszShort.GetLength());
			}
		}
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
		Contact_Hide(pUser->hContact);
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
