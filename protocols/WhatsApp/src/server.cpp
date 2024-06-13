/*

WhatsApp plugin for Miranda NG
Copyright © 2019-24 George Hazan

*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// gateway worker thread

void WhatsAppProto::ServerThread(void *)
{
	do {
		m_bRespawn = m_bUnregister = false;
		ServerThreadWorker();
	}
	while (m_bRespawn);

	OnLoggedOut();
}

void WhatsAppProto::ServerThreadWorker()
{
	// connect websocket
	MHttpHeaders hdrs;
	hdrs.AddHeader("Origin", "https://web.whatsapp.com");

	WebSocket<WhatsAppProto> ws(this);

	NLHR_PTR pReply(ws.connect(m_hNetlibUser, "web.whatsapp.com/ws/chat", &hdrs));
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
	m_ws = &ws;
	m_lastRecvTime = time(0);
	m_iPacketId = 1;

	Utils_GetRandom(m_wMsgPrefix, sizeof(m_wMsgPrefix));

	Wa__HandshakeMessage__ClientHello client;
	client.ephemeral = {m_noise->ephemeral.pub.length(), m_noise->ephemeral.pub.data()};
	client.has_ephemeral = true;

	Wa__HandshakeMessage msg;
	msg.clienthello = &client;
	WSSend(msg);

	ws.run();
	m_ws = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Binary data processing

void WebSocket<WhatsAppProto>::process(const uint8_t *pData, size_t cbDataLen)
{
	while (size_t payloadLen = p->m_noise->decodeFrame(pData, cbDataLen)) {
		if (p->m_noise->bInitFinished) {
			MBinBuffer buf = p->m_noise->decrypt(pData, payloadLen);

			WAReader rdr(buf.data(), buf.length());
			auto b = rdr.readInt8();
			if (b & 2) {
				buf.remove(1);
				buf = Utils_Unzip(buf);
				rdr = WAReader(buf.data(), buf.length());
			}

			if (WANode *pNode = rdr.readNode()) {
				CMStringA szText;
				pNode->print(szText);
				p->debugLogA("Got binary node:\n%s", szText.c_str());

				auto pHandler = p->FindPersistentHandler(*pNode);
				if (pHandler)
					(p->*pHandler)(*pNode);
				else
					p->debugLogA("cannot handle incoming message");

				delete pNode;
			}
			else {
				p->debugLogA("wrong or broken payload");
				Netlib_Dump(m_hConn, pData, cbDataLen, false, 0);
			}
		}
		else p->OnProcessHandshake(pData, (int)payloadLen);

		pData = (BYTE*)pData + payloadLen;
		cbDataLen -= payloadLen;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ProcessFailure(int code)
{
	switch (code) {
	case 401:
		debugLogA("Connection logged out from another device, exiting");
		Popup(0, TranslateT("This account was logged out from mobile phone, you need to link it again"), m_tszUserName);

		RemoveCachedSettings();
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

	if (m_bUnregister) {
		SendUnregister();
		m_bTerminated = true;
		return;
	}

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

	GC_RefreshMetadata();
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

void WhatsAppProto::SendUnregister()
{
	WANodeIq iq(IQ::SET, "md");
	*iq.addChild("remove-companion-device") << CHAR_PARAM("jid", WAJid(m_szJid, getDword(DBKEY_DEVICE_ID)).toString()) << CHAR_PARAM("reason", "user's decision");
	WSSendNode(iq, &WhatsAppProto::OnIqDoNothing);

	m_bTerminated = true;
}

void WhatsAppProto::SendUsync(const LIST<char> &jids, void *pUserInfo)
{
	WANodeIq iq(IQ::GET, "usync");

	auto *pNode1 = iq.addChild("usync");
	*pNode1 << CHAR_PARAM("sid", GenerateMessageId()) << CHAR_PARAM("mode", "query") << CHAR_PARAM("last", "true")
		<< CHAR_PARAM("index", "0") << CHAR_PARAM("context", "message");

	pNode1->addChild("query")->addChild("devices")->addAttr("version", "2");
	auto *pList = pNode1->addChild("list");
	for (auto &it : jids)
		pList->addChild("user")->addAttr("jid", it);

	WSSendNode(iq, &WhatsAppProto::OnIqGetUsync, pUserInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("WhatsAppProto::ShutdownSession");

	// shutdown all resources
	if (m_ws)
		m_ws->terminate();

	OnLoggedOut();
}
