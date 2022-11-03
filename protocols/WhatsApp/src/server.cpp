/*

WhatsApp plugin for Miranda NG
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

	Wa__HandshakeMessage__ClientHello client;
	client.ephemeral = {m_noise->ephemeral.pub.length(), m_noise->ephemeral.pub.data()};
	client.has_ephemeral = true;

	Wa__HandshakeMessage msg;
	msg.clienthello = &client;
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

		// debugLogA("Got packet: buffer = %d, opcode = %d, headerSize = %d, payloadSize = %d, final = %d, masked = %d", 
		//		netbuf.length(), hdr.opCode, hdr.headerSize, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);
		// Netlib_Dump(m_hServerConn, netbuf.data(), netbuf.length(), false, 0);

		m_lastRecvTime = time(0);

		// read all payloads from the current buffer, one by one
		while (true) {
			MBinBuffer currPacket;
			currPacket.assign(netbuf.data() + hdr.headerSize, hdr.payloadSize);
			
			switch (hdr.opCode) {
			case 1: // json packet
				debugLogA("Text packet, skipping");
				/*
					currPacket.append("", 1); // add 0 to use strchr safely
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
				*/
				break;

			case 2: // binary packet
				if (hdr.payloadSize > 32)
					ProcessBinaryPacket(currPacket.data(), hdr.payloadSize);
				break;

			case 8: // close
				debugLogA("server required to exit");
				m_bRespawn = m_bTerminated = true; // simply reconnect, don't exit
				break;

			default:
				Netlib_Dump(m_hServerConn, currPacket.data(), hdr.payloadSize, false, 0);
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

void WhatsAppProto::ProcessBinaryPacket(const uint8_t *pData, size_t cbDataLen)
{
	while (size_t payloadLen = m_noise->decodeFrame(pData, cbDataLen)) {
		if (m_noise->bInitFinished) {
			MBinBuffer buf = m_noise->decrypt(pData, payloadLen);

			WAReader rdr(buf.data(), buf.length());
			auto b = rdr.readInt8();
			if (b & 2) {
				buf.remove(1);
				buf = unzip(buf);
				rdr = WAReader(buf.data(), buf.length());
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

const char *pszNeededItems[] = {
	"AM_BaseProto", "DefaultGroup", "DeviceName", "HideChats", "NLlog", "Nick"
};

static int sttEnumFunc(const char *szSetting, void *param)
{
	for (auto &it : pszNeededItems)
		if (!mir_strcmp(it, szSetting))
			return 0;

	auto *pList = (LIST<char>*)param;
	pList->insert(mir_strdup(szSetting));
	return 0;
}

void WhatsAppProto::ProcessFailure(int code)
{
	switch (code) {
	case 401:
		debugLogA("Connection logged out from another device, exiting");
		Popup(0, TranslateT("This account was logged out from another device, you need to register it again"), m_tszUserName);

		// remove all temporary data from database & disk folder
		{
			LIST<char> arSettings(50);
			db_enum_settings(0, sttEnumFunc, m_szModuleName, &arSettings);
			for (auto &it : arSettings) {
				delSetting(it);
				mir_free(it);
			}
		}
		m_szJid.Empty();
		OnErase();
		break;

	case 408:
		debugLogA("Connection lost, exiting");
		break;

	case 411:
		debugLogA("Conflict between two devices, exiting");
		break;

	case 428:
		debugLogA("Connection forcibly closed by the server, exiting");
		break;

	case 440:
		debugLogA("Connection replaced from another device, exiting");
		break;

	case 515:
		debugLogA("Server required to restart immediately, leaving thread");
		m_bRespawn = true;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnLoggedIn()
{
	debugLogA("WhatsAppProto::OnLoggedIn");

	SetServerStatus(m_iDesiredStatus);

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, m_iDesiredStatus);
	m_iStatus = m_iDesiredStatus;
	m_bUpdatedPrekeys = false;

	m_impl.m_keepAlive.Start(1000);

	// retrieve initial info
	WANodeIq abt(IQ::GET, "abt");
	abt.addChild("props")->addAttr("protocol", "1");
	WSSendNode(abt, &WhatsAppProto::OnIqDoNothing);

	WSSendNode(
		WANodeIq(IQ::GET, "w") << XCHILD("props"),
		&WhatsAppProto::OnIqDoNothing);

	WSSendNode(
		WANodeIq(IQ::GET, "blocklist"),
		&WhatsAppProto::OnIqBlockList);

	WSSendNode(
		WANodeIq(IQ::GET, "privacy") << XCHILD("privacy"),
		&WhatsAppProto::OnIqDoNothing);

	for (auto &it : m_arUsers)
		if (it->bIsGroupChat)
			GC_Init(it);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Service packets sending

void WhatsAppProto::SendAck(const WANode &node)
{
	WANode ack("ack");
	ack << CHAR_PARAM("to", node.getAttr("from")) << CHAR_PARAM("id", node.getAttr("id")) << CHAR_PARAM("class", node.title);
	if (node.title != "message")
		if (auto *param = node.getAttr("type"))
			ack << CHAR_PARAM("type", param);
	if (auto *param = node.getAttr("participant"))
		ack << CHAR_PARAM("participant", param);
	if (auto *param = node.getAttr("recipient"))
		ack << CHAR_PARAM("recipient", param);
	WSSendNode(ack);
}

void WhatsAppProto::SendKeepAlive()
{
	time_t now = time(0);
	if (now - m_lastRecvTime > 20) {
		WSSendNode(WANodeIq(IQ::GET, "w:p") << XCHILD("ping"), &WhatsAppProto::OnIqDoNothing);

		m_lastRecvTime = now;
	}

	for (auto &it : m_arUsers) {
		if (it->m_timer1 && now - it->m_timer1 > 600) {
			it->m_timer1 = 0;
			it->m_timer2 = now;
			setWord(it->hContact, "Status", ID_STATUS_AWAY);
		}
		else if (it->m_timer2 && now - it->m_timer2 > 600) {
			it->m_timer2 = 0;
			setWord(it->hContact, "Status", ID_STATUS_OFFLINE);
		}
	}
}

void WhatsAppProto::SendReceipt(const char *pszTo, const char *pszParticipant, const char *pszId, const char *pszType)
{
	WANode receipt("receipt");
	receipt << CHAR_PARAM("id", pszId);

	if (!mir_strcmp(pszType, "read") || !mir_strcmp(pszType, "read-self"))
		receipt << INT_PARAM("t", time(0));

	if (!mir_strcmp(pszType, "sender") && WAJid(pszTo).isUser())
		receipt << CHAR_PARAM("to", pszParticipant) << CHAR_PARAM("recipient", pszTo);
	else {
		receipt << CHAR_PARAM("to", pszTo);
		if (pszParticipant)
			receipt << CHAR_PARAM("participant", pszParticipant);
	}

	if (pszType)
		receipt << CHAR_PARAM("type", pszType);
	WSSendNode(receipt);
}

void WhatsAppProto::SetServerStatus(int iStatus)
{
	if (mir_wstrlen(m_wszNick))
		WSSendNode(
			WANode("presence") << CHAR_PARAM("name", T2Utf(m_wszNick)) << CHAR_PARAM("type", (iStatus == ID_STATUS_ONLINE) ? "available" : "unavailable"),
			&WhatsAppProto::OnIqDoNothing);
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
