/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// gateway worker thread

void WhatsAppProto::ServerThread(void *)
{
	do {
		m_bRespawn = false;
		ServerThreadWorker();
	}
	while (m_bRespawn);

	OnLoggedOut();
}

void WhatsAppProto::ServerThreadWorker()
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
		return;
	}

	if (pReply->resultCode != 101)
		return;

	delete m_noise;
	m_noise = new WANoise(this);
	m_noise->init();

	debugLogA("Server connection succeeded");
	m_hServerConn = pReply->nlc;
	m_lastRecvTime = time(0);
	m_iPacketId = 1;

	Utils_GetRandom(m_wMsgPrefix, sizeof(m_wMsgPrefix));

	auto &pubKey = m_noise->ephemeral.pub;
	auto *client = new proto::HandshakeMessage::ClientHello(); client->set_ephemeral(pubKey.data(), pubKey.length());
	proto::HandshakeMessage msg; msg.set_allocated_clienthello(client);
	WSSend(msg);

	MBinBuffer netbuf;

	for (m_bTerminated = false; !m_bTerminated;) {
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
		Netlib_Dump(m_hServerConn, netbuf.data(), netbuf.length(), false, 0);

		m_lastRecvTime = time(0);

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
						}
					}
				}
				break;

			case 8: // close
				debugLogA("server required to exit");
				m_bRespawn = m_bTerminated = true; // simply reconnect, don't exit
				break;

			default:
				Netlib_Dump(m_hServerConn, start, hdr.payloadSize, false, 0);
			}

			netbuf.remove(hdr.headerSize + hdr.payloadSize);
			// debugLogA("%d bytes removed from network buffer, %d bytes remain", hdr.headerSize + hdr.payloadSize, netbuf.length());
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
}

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

				auto pHandler = FindPersistentHandler(*pNode);
				if (pHandler)
					(this->*pHandler)(*pNode);
				else
					debugLogA("cannot handle incoming message");

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

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnLoggedIn()
{
	debugLogA("WhatsAppProto::OnLoggedIn");

	SetServerStatus(m_iDesiredStatus);

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;

	m_impl.m_keepAlive.Start(1000);

	// retrieve loaded prekeys
	WANode iq("iq");
	iq << CHAR_PARAM("id", generateMessageId()) << CHAR_PARAM("xmlns", "encrypt") << CHAR_PARAM("type", "get") << CHAR_PARAM("to", S_WHATSAPP_NET);
	iq.addChild("count");
	WSSendNode(iq, &WhatsAppProto::OnIqCountPrekeys);
}

void WhatsAppProto::OnLoggedOut(void)
{
	m_impl.m_keepAlive.Stop();

	debugLogA("WhatsAppProto::OnLoggedOut");
	m_bTerminated = true;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void WhatsAppProto::SendKeepAlive()
{
	time_t now = time(0);
	if (now - m_lastRecvTime > 20) {
		WANode iq("iq");
		iq << CHAR_PARAM("id", generateMessageId()) << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("type", "get") << CHAR_PARAM("xmlns", "w:p");
		iq.addChild("ping");
		WSSendNode(iq);

		m_lastRecvTime = now;
	}
}

void WhatsAppProto::SetServerStatus(int iStatus)
{
	if (mir_wstrlen(m_wszNick)) {
		WANode iq("presence");
		iq << CHAR_PARAM("name", T2Utf(m_wszNick)) << CHAR_PARAM("type", (iStatus == ID_STATUS_ONLINE) ? "available" : "unavailable");
		WSSendNode(iq);
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
