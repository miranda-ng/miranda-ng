/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

int WhatsAppProto::WSSend(const CMStringA &str, WA_PKT_HANDLER pHandler)
{
	if (m_hServerConn == nullptr)
		return -1;

	int pktId = ++m_iPktNumber;

	CMStringA buf;
	buf.Format("%d.--%d,", (int)m_iLoginTime, pktId);
	if (!str.IsEmpty()) {
		buf.AppendChar(',');
		buf += str;
	}

	if (pHandler != nullptr) {
		auto *pReq = new WARequest;
		pReq->issued = time(0);
		pReq->pHandler = pHandler;
		pReq->pktId = pktId;

		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(pReq);
	}

	debugLogA("Sending packet #%d: %s", pktId, buf.c_str());
	WebSocket_Send(m_hServerConn, buf.c_str(), buf.GetLength());
	return pktId;
}

void WhatsAppProto::OnLoggedIn()
{
	debugLogA("CDiscordProto::OnLoggedIn");
	m_bOnline = true;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	SendKeepAlive();
	m_impl.m_keepAlive.Start(60000);
}

void WhatsAppProto::OnLoggedOut(void)
{
	debugLogA("CDiscordProto::OnLoggedOut");
	m_bOnline = false;
	m_bTerminated = true;
	m_iPktNumber = 0;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void WhatsAppProto::SendKeepAlive()
{
	WebSocket_Send(m_hServerConn, "?,,", 3);

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

void WhatsAppProto::OnRestoreSession1(const JSONNode&)
{
	ptrA szClient(getStringA(DBKEY_CLIENT_TOKEN)), szServer(getStringA(DBKEY_SERVER_TOKEN));
	if (szClient == nullptr || szServer == nullptr) {
		ShutdownSession();
		return;
	}

	CMStringA payload(FORMAT, "[\"admin\",\"login\",\"%s\",\"%s\",\"%s\",\"takeover\"]", szClient.get(), szServer.get(), m_szClientId.c_str());
	WSSend(payload, &WhatsAppProto::OnRestoreSession2);
}

void WhatsAppProto::OnRestoreSession2(const JSONNode &root)
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

void WhatsAppProto::OnStartSession(const JSONNode &root)
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

bool WhatsAppProto::WSReadPacket(int nBytes, const WSHeader &hdr, MBinBuffer &res)
{
	size_t currPacketSize = nBytes - hdr.headerSize;

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
	int offset = 0;
	MBinBuffer netbuf;

	while (!bExit && !m_bTerminated) {
		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hServerConn, (char *)buf + offset, _countof(buf) - offset, MSG_NODUMP);
		if (bufSize == 0) {
			debugLogA("Gateway connection gracefully closed");
			break;
		}
		if (bufSize < 0) {
			debugLogA("Gateway connection error, exiting");
			break;
		}

		WSHeader hdr;
		if (!WebSocket_InitHeader(hdr, buf, bufSize)) {
			offset += bufSize;
			continue;
		}
		
		// we have some additional data, not only opcode
		if ((size_t)bufSize > hdr.headerSize) {
			netbuf.append(buf, bufSize);
			if (!WSReadPacket(bufSize, hdr, netbuf))
				break;
		}

		offset = 0;
		debugLogA("Got packet: buffer = %d, opcode = %d, payloadSize = %d, final = %d, masked = %d", netbuf.length(), hdr.opCode, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);

		// read all payloads from the current buffer, one by one
		size_t prevSize = 0;
		while (true) {
			MBinBuffer currPacket;
			currPacket.assign(netbuf.data() + hdr.headerSize, hdr.payloadSize);
			currPacket.append("", 1);
			
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
					if (hdr.opCode == 2 && hdr.payloadSize > 32) {
						MBinBuffer dest;
						if (!decryptBinaryMessage(dataSize, pos, dest)) {
							Netlib_Dump(m_hServerConn, currPacket.data(), currPacket.length(), false, 0);
							debugLogA("cannot decrypt incoming message");
							break;
						}

						// Netlib_Dump(m_hServerConn, dest.data(), dest.length(), false, 0);
						ProcessBinaryPacket(dest);
					}
					else {
						CMStringA szJson(pos, (int)dataSize);

						JSONNode root = JSONNode::parse(szJson);
						if (root) {
							debugLogA("JSON received:\n%s", szJson.c_str());

							int sessId, pktId;
							if (sscanf(start, "%d.--%d,", &sessId, &pktId) == 2) {
								auto *pReq = m_arPacketQueue.find((WARequest *)&pktId);
								if (pReq != nullptr) {
									(this->*pReq->pHandler)(root);
								}
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

			case 9: // ping
				debugLogA("ping received");
				Netlib_Send(m_hServerConn, "?,,", 3, 0);
				break;

			default:
				Netlib_Dump(m_hServerConn, start, hdr.payloadSize, false, 0);
			}

			if (hdr.bIsFinal)
				netbuf.remove(hdr.headerSize + hdr.payloadSize);

			if (netbuf.length() == 0)
				break;

			// if we have not enough data for header, continue reading
			if (!WebSocket_InitHeader(hdr, netbuf.data(), netbuf.length()))
				break;

			// if we have not enough data for data, continue reading
			if (hdr.headerSize + hdr.payloadSize > netbuf.length())
				break;

			debugLogA("Got inner packet: buffer = %d, opcode = %d, headerSize = %d, payloadSize = %d, final = %d, masked = %d", netbuf.length(), hdr.opCode, hdr.headerSize, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);
			if (prevSize == netbuf.length()) {
				netbuf.remove(prevSize);
				debugLogA("dropping current packet, exiting");
				break;
			}

			prevSize = netbuf.length();
		}
	}

	debugLogA("Server connection dropped");
	Netlib_CloseHandle(m_hServerConn);
	m_hServerConn = nullptr;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Binary data processing

void WhatsAppProto::ProcessBinaryPacket(const MBinBuffer &buf)
{
	WAReader reader(buf.data(), buf.length());

	JSONNode root;
	if (!reader.readNode(root))
		return;

	debugLogA("packed JSON: %s", root.write().c_str());
	
	CMStringA szType = root["type"].as_mstring();
	if (szType == "contacts")
		ProcessContacts(root["$list$"]);
	else if (szType == "chat")
		ProcessChats(root["$list$"]);
	else {
		CMStringA szAdd = root["add"].as_mstring();
		if (!szAdd.IsEmpty())
			ProcessAdd(szAdd, root["$list$"]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ProcessAdd(const CMStringA &type, const JSONNode &list)
{
	for (auto &it : list) {
		std::string buf = it["$bin$"].as_string();

		size_t resLen;
		ptrA pRes((char*)mir_base64_decode(buf.c_str(), &resLen));
		if (pRes == nullptr)
			continue;

		proto::WebMessageInfo payLoad;
		if (!payLoad.ParseFromArray(pRes, (int)resLen)) {
			debugLogA("Error: message failed to decode by protobuf!");
			Netlib_Dump(m_hServerConn, pRes, resLen, false, 0);
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

				GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
				gce.pszID.a = jid;
				gce.dwFlags = GCEF_ADDTOLOG | GCEF_UTF8;
				gce.pszUID.a = payLoad.participant().c_str();
				gce.pszText.a = szText;
				gce.time = dwTimestamp;
				gce.bIsMe = true;
				Chat_Event(&gce);
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

void WhatsAppProto::ProcessChats(const JSONNode &list)
{
	for (auto &it : list) {
		CMStringW jid(it["jid"].as_mstring());
		auto *pUser = AddUser(T2Utf(jid), false);

		DWORD dwLastId = it["t"].as_int();
		setDword(pUser->hContact, "LastWriteTime", dwLastId);

		pUser->dwModifyTag = it["modify_tag"].as_int();

		if (pUser->si) {
			DWORD dwMute = _wtoi(it["mute"].as_mstring());
			Chat_Mute(pUser->si, dwMute ? CHATMODE_MUTE : CHATMODE_NORMAL);
		}
	}
}

void WhatsAppProto::ProcessContacts(const JSONNode &list)
{
	for (auto &it : list) {
		CMStringW jid(it["jid"].as_mstring());
		auto *pUser = AddUser(T2Utf(jid), false);

		if (strstr(pUser->szId, "@g.us")) {
			InitChat(pUser, it);
			continue;
		}
		
		CMStringW wszNick(it["notify"].as_mstring());
		if (wszNick.IsEmpty()) {
			int idx = jid.Find('@');
			wszNick = (idx == -1) ? jid : jid.Left(idx);
		}
		setWString(pUser->hContact, "Nick", wszNick);

		CMStringW wszFullName(it["name"].as_mstring());
		wszFullName.TrimRight();
		if (!wszFullName.IsEmpty()) {
			setWString(pUser->hContact, "FullName", wszFullName);

			CMStringW wszShort(it["short"].as_mstring());
			wszShort.TrimRight();
			if (!wszShort.IsEmpty()) {
				setWString(pUser->hContact, "FirstName", wszShort);
				if (wszShort.GetLength()+1 < wszFullName.GetLength())
					setWString(pUser->hContact, "LastName", wszFullName.c_str() + 1 + wszShort.GetLength());
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

	m_szJid = root["wid"].as_mstring();
	setString(DBKEY_ID, m_szJid);

	CMStringA szSecret(root["secret"].as_mstring());
	if (!szSecret.IsEmpty())
		if (!ProcessSecret(szSecret)) {
			ShutdownSession();
			return;
		}

	setWString(DBKEY_NICK, root["pushname"].as_mstring());
	setWString(DBKEY_CLIENT_TOKEN, root["clientToken"].as_mstring());
	setWString(DBKEY_SERVER_TOKEN, root["serverToken"].as_mstring());
	setWString(DBKEY_BROWSER_TOKEN, root["browserToken"].as_mstring());

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
