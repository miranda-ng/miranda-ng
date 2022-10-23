/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::OnReceiveMessage(const WANode &node)
{
	auto *msgId = node.getAttr("id");
	auto *msgType = node.getAttr("type");
	auto *msgFrom = node.getAttr("from");
	auto *category = node.getAttr("category");
	auto *recipient = node.getAttr("recipient");
	auto *participant = node.getAttr("participant");

	if (msgType == nullptr || msgFrom == nullptr || msgId == nullptr) {
		debugLogA("bad message received: <%s> <%s> <%s>", msgType, msgFrom, msgId);
		return;
	}

	MEVENT hEvent = db_event_getById(m_szModuleName, msgId);
	if (hEvent) {
		debugLogA("this message is already processed: %s", msgId);
		return;
	}

	SendAck(node);

	WAMSG type;
	WAJid jid(msgFrom);
	CMStringA szAuthor, szChatId;

	if (node.getAttr("offline"))
		type.bOffline = true;

	// message from one user to another
	if (jid.isUser()) {
		if (recipient) {
			if (m_szJid != msgFrom) {
				debugLogA("strange message: with recipient, but not from me");
				return;
			}
			szChatId = recipient;
		}
		else szChatId = msgFrom;

		type.bPrivateChat = true;
		szAuthor = msgFrom;
	}
	else if (jid.isGroup()) {
		if (!participant) {
			debugLogA("strange message: from group, but without participant");
			return;
		}

		type.bGroupChat = true;
		szAuthor = participant;
		szChatId = msgFrom;
	}
	else if (jid.isBroadcast()) {
		if (!participant) {
			debugLogA("strange message: from group, but without participant");
			return;
		}

		bool bIsMe = m_szJid == participant;
		if (jid.isStatusBroadcast()) {
			if (bIsMe)
				type.bDirectStatus = true;
			else
				type.bOtherStatus = true;
		}
		else {
			if (bIsMe)
				type.bPeerBroadcast = true;
			else
				type.bOtherBroadcast = true;
		}
		szChatId = msgFrom;
		szAuthor = participant;
	}
	else {
		debugLogA("invalid message type");
		return;
	}

	CMStringA szSender = (type.bPrivateChat) ? szAuthor : szChatId;
	bool bFromMe = (m_szJid == msgFrom);
	if (!bFromMe && participant)
		bFromMe = m_szJid == participant;

	auto *pKey = new proto::MessageKey();
	pKey->set_remotejid(szChatId);
	pKey->set_id(msgId);
	pKey->set_fromme(bFromMe);
	if (participant)
		pKey->set_participant(participant);

	proto::WebMessageInfo msg;
	msg.set_allocated_key(pKey);
	msg.set_messagetimestamp(_atoi64(node.getAttr("t")));
	msg.set_pushname(node.getAttr("notify"));
	if (bFromMe)
		msg.set_status(proto::WebMessageInfo_Status_SERVER_ACK);

	int iDecryptable = 0;

	for (auto &it : node.getChildren()) {
		if (it->title == "verified_name") {
			proto::VerifiedNameCertificate cert;
			cert << it->content;

			proto::VerifiedNameCertificate::Details details;
			details.ParseFromString(cert.details());

			msg.set_verifiedbizname(details.verifiedname());
			continue;
		}

		if (it->title != "enc" || it->content.length() == 0)
			continue;

		SignalBuffer msgBody;
		auto *pszType = it->getAttr("type");
		try {
			if (!mir_strcmp(pszType, "pkmsg") || !mir_strcmp(pszType, "msg")) {
				CMStringA szUser = (WAJid(szSender).isUser()) ? szSender : szAuthor;
				msgBody = m_signalStore.decryptSignalProto(szUser, pszType, it->content);
			}
			else if (!mir_strcmp(pszType, "skmsg")) {
				msgBody = m_signalStore.decryptGroupSignalProto(szSender, szAuthor, it->content);
			}
			else throw "Invalid e2e type";

			if (!msgBody)
				throw "Invalid e2e message";

			iDecryptable++;

			proto::Message encMsg;
			encMsg.ParseFromArray(msgBody.data(), msgBody.len());
			if (encMsg.devicesentmessage().has_message())
				msg.set_allocated_message(new proto::Message(encMsg.devicesentmessage().message()));
			else
				msg.set_allocated_message(new proto::Message(encMsg));

			if (encMsg.has_senderkeydistributionmessage())
				m_signalStore.processSenderKeyMessage(encMsg.senderkeydistributionmessage());

			ProcessMessage(type, msg);
			msg.clear_message();

			// send receipt
			const char *pszReceiptType = nullptr, *pszReceiptTo = participant;
			if (!mir_strcmp(category, "peer"))
				pszReceiptType = "peer_msg";
			else if (bFromMe) {
				// message was sent by me from a different device
				pszReceiptType = "sender";
				if (WAJid(szChatId).isUser())
					pszReceiptTo = szAuthor;
			}
			else if (!m_hServerConn)
				pszReceiptType = "inactive";

			SendReceipt(szChatId, pszReceiptTo, msgId, pszReceiptType);
		}
		catch (const char *) {
		}

		if (!iDecryptable) {
			debugLogA("Nothing to decrypt");
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static const proto::Message& getBody(const proto::Message &message)
{
	if (message.has_ephemeralmessage()) {
		auto &pMsg = message.ephemeralmessage().message();
		return (pMsg.has_viewoncemessage()) ? pMsg.viewoncemessage().message() : pMsg;
	}

	if (message.has_viewoncemessage())
		return message.viewoncemessage().message();

	return message;
}

void WhatsAppProto::ProcessMessage(WAMSG type, const proto::WebMessageInfo &msg)
{
	auto &key = msg.key();
	auto &body = getBody(msg.message());
	bool bFromMe = key.fromme();

	debugLogA("Got a message: %s", msg.Utf8DebugString().c_str());

	uint32_t timestamp = msg.messagetimestamp();
	auto &participant = key.participant();
	auto &chatId = key.remotejid();
	auto &msgId = key.id();
	
	WAUser *pUser = FindUser(chatId.c_str());
	if (pUser == nullptr) {
		if (type.bPrivateChat)
			pUser = AddUser(chatId.c_str(), false, false);
		else if (type.bGroupChat)
			pUser = AddUser(chatId.c_str(), false, true);
	}

	if (!bFromMe && msg.has_pushname() && pUser && !pUser->bIsGroupChat)
		setUString(pUser->hContact, "Nick", msg.pushname().c_str());

	// try to extract some text 
	if (body.has_conversation()) {
		auto &conversation = body.conversation();
		if (!conversation.empty()) {
			PROTORECVEVENT pre = {};
			pre.timestamp = timestamp;
			pre.szMessage = (char*)conversation.c_str();
			pre.szMsgId = msgId.c_str();
			if (type.bOffline)
				pre.flags |= PREF_CREATEREAD;
			if (bFromMe)
				pre.flags |= PREF_SENT;
			ProtoChainRecvMsg(pUser->hContact, &pre);
		}
	}

	if (body.has_protocolmessage()) {
		auto &protoMsg = body.protocolmessage();
		switch (protoMsg.type()) {
		case proto::Message_ProtocolMessage_Type_APP_STATE_SYNC_KEY_SHARE:
			if (protoMsg.appstatesynckeyshare().keys_size()) {
				for (auto &it : protoMsg.appstatesynckeyshare().keys()) {
					auto &keyid = it.keyid().keyid();
					auto &keydata = it.keydata().keydata();

					CMStringA szSetting(FORMAT, "AppSyncKey%d", decodeBigEndian(keyid));
					db_set_blob(0, m_szModuleName, szSetting, keydata.c_str(), (unsigned)keydata.size());
				}
			}
			break;

		case proto::Message_ProtocolMessage_Type_HISTORY_SYNC_NOTIFICATION:
			debugLogA("History sync notification");
			m_impl.m_resyncApp.Stop();
			m_impl.m_resyncApp.Start(10000);
			break;

		case proto::Message_ProtocolMessage_Type_REVOKE:
			break;

		case proto::Message_ProtocolMessage_Type_EPHEMERAL_SETTING:
			if (pUser) {
				setDword(pUser->hContact, DBKEY_EPHEMERAL_TS, timestamp);
				setDword(pUser->hContact, DBKEY_EPHEMERAL_EXPIRE, protoMsg.ephemeralexpiration());
			}
			break;
		}
	}
	else if (body.has_reactionmessage()) {
		debugLogA("Got a reaction to a message");
	}
	else if (msg.has_messagestubtype()) {
		switch (msg.messagestubtype()) {
		case proto::WebMessageInfo::GROUP_PARTICIPANT_LEAVE:
		case proto::WebMessageInfo::GROUP_PARTICIPANT_REMOVE:
			debugLogA("Participant %s removed from chat", participant.c_str());
			break;

		case proto::WebMessageInfo::GROUP_PARTICIPANT_ADD:
		case proto::WebMessageInfo::GROUP_PARTICIPANT_INVITE:
		case proto::WebMessageInfo::GROUP_PARTICIPANT_ADD_REQUEST_JOIN:
			debugLogA("Participant %s added to chat", participant.c_str());
			break;

		case proto::WebMessageInfo::GROUP_PARTICIPANT_DEMOTE:
			debugLogA("Participant %s demoted", participant.c_str());
			break;

		case proto::WebMessageInfo::GROUP_PARTICIPANT_PROMOTE:
			debugLogA("Participant %s promoted", participant.c_str());
			break;

		case proto::WebMessageInfo::GROUP_CHANGE_ANNOUNCE:
			debugLogA("Groupchat announce", participant.c_str());
			break;

		case proto::WebMessageInfo::GROUP_CHANGE_RESTRICT:
			debugLogA("Groupchat restriction", participant.c_str());
			break;

		case proto::WebMessageInfo::GROUP_CHANGE_SUBJECT:
			debugLogA("Groupchat subject was changed", participant.c_str());
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

bool WhatsAppProto::CreateMsgParticipant(WANode *pParticipants, const WAJid &jid, const MBinBuffer &orig)
{
	int type = 0;

	try {
		SignalBuffer pBuffer(m_signalStore.encryptSignalProto(jid, orig, type));

		auto *pNode = pParticipants->addChild("to");
		pNode->addAttr("jid", jid.toString());

		auto *pEnc = pNode->addChild("enc");
		*pEnc << CHAR_PARAM("v", "2") << CHAR_PARAM("type", (type == 3) ? "pkmsg" : "msg");
		pEnc->content.assign(pBuffer.data(), pBuffer.len());
	}
	catch (const char *) {
	}
	
	return type == 3;
}

int WhatsAppProto::SendTextMessage(const char *jid, const char *pszMsg)
{
	char msgId[16], szMsgId[33];
	Utils_GetRandom(msgId, sizeof(msgId));
	bin2hex(msgId, sizeof(msgId), szMsgId);
	strupr(szMsgId);

	auto *pBody = new proto::Message();
	pBody->set_conversation(pszMsg);

	auto *pSentBody = new proto::Message_DeviceSentMessage();
	pSentBody->set_allocated_message(pBody);
	pSentBody->set_destinationjid(jid);

	proto::Message msg;
	msg.set_allocated_devicesentmessage(pSentBody);

	MBinBuffer encMsg(msg.ByteSizeLong());
	msg.SerializeToArray(encMsg.data(), (int)encMsg.length());

	WANode payLoad("message");
	payLoad << CHAR_PARAM("id", szMsgId) << CHAR_PARAM("type", "text") << CHAR_PARAM("to", jid);
	
	auto *pParticipants = payLoad.addChild("participants");
	bool shouldIncludeIdentity = CreateMsgParticipant(pParticipants, WAJid(jid), false);
	for (auto &it : m_arDevices)
		shouldIncludeIdentity |= CreateMsgParticipant(pParticipants, it->jid, true);

	if (shouldIncludeIdentity) {
		MBinBuffer encIdentity(m_signalStore.encodeSignedIdentity(true));
		auto *pNode = payLoad.addChild("device-identity");
		pNode->content.assign(encIdentity.data(), encIdentity.length());
	}

	WSSendNode(payLoad);
	
	mir_cslock lck(m_csOwnMessages);
	int pktId = m_iPacketId++;
	m_arOwnMsgs.insert(new WAOwnMessage(pktId, jid, szMsgId));
	return pktId;
}
