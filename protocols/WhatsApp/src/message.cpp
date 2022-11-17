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

	Wa__MessageKey key;
	key.remotejid = szChatId.GetBuffer();
	key.id = (char*)msgId;
	key.fromme = bFromMe; key.has_fromme = true;
	if (participant)
		key.participant = (char*)participant;

	Wa__WebMessageInfo msg;
	msg.key = &key;
	msg.messagetimestamp = _atoi64(node.getAttr("t")); msg.has_messagetimestamp = true;
	msg.pushname = (char*)node.getAttr("notify");
	if (bFromMe)
		msg.status = WA__WEB_MESSAGE_INFO__STATUS__SERVER_ACK, msg.has_status = true;

	int iDecryptable = 0;

	for (auto &it : node.getChildren()) {
		if (it->title != "enc" || it->content.length() == 0)
			continue;

		MBinBuffer msgBody;
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

			if (msgBody.isEmpty())
				throw "Invalid e2e message";

			iDecryptable++;

			proto::Message encMsg(unpadBuffer16(msgBody));
			if (!encMsg)
				throw "Invalid decoded message";

			if (encMsg->devicesentmessage)
				msg.message = encMsg->devicesentmessage->message;
			else
				msg.message = encMsg;

			if (encMsg->senderkeydistributionmessage)
				m_signalStore.processSenderKeyMessage(szAuthor, encMsg->senderkeydistributionmessage);

			ProcessMessage(type, msg);

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
		catch (const char *pszError) {
			debugLogA("Message decryption failed with error: %s", pszError);
		}

		if (!iDecryptable) {
			debugLogA("Nothing to decrypt");
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static const Wa__Message* getBody(const Wa__Message *message)
{
	if (message->ephemeralmessage) {
		auto *pMsg = message->ephemeralmessage->message;
		return (pMsg->viewoncemessage) ? pMsg->viewoncemessage->message : pMsg;
	}

	if (message->viewoncemessage)
		return message->viewoncemessage->message;

	return message;
}

void WhatsAppProto::ProcessMessage(WAMSG type, const Wa__WebMessageInfo &msg)
{
	auto *key = msg.key;
	auto *body = getBody(msg.message);

	debugLogA("Got a message: %s", protobuf_c_text_to_string(&msg).c_str());

	uint32_t timestamp = msg.messagetimestamp;
	char *participant = key->participant, *chatId;
	auto *msgId = key->id;

	if (type.bPrivateChat || type.bGroupChat)
		chatId = key->remotejid;
	else
		chatId = (participant) ? participant : key->remotejid;

	WAUser *pUser = AddUser(chatId, false);

	if (!key->fromme && msg.pushname && pUser && !pUser->bIsGroupChat)
		setUString(pUser->hContact, "Nick", msg.pushname);

	// try to extract some text
	if (pUser) {
		CMStringA szMessageText(GetMessageText(body));
		if (!szMessageText.IsEmpty()) {
			PROTORECVEVENT pre = {};
			pre.timestamp = timestamp;
			pre.szMessage = szMessageText.GetBuffer();
			pre.szMsgId = msgId;
			if (type.bOffline)
				pre.flags |= PREF_CREATEREAD;
			if (key->fromme)
				pre.flags |= PREF_SENT;
			ProtoChainRecvMsg(pUser->hContact, &pre);

			if (pUser->bIsGroupChat) {
				GCEVENT gce = {m_szModuleName, 0, GC_EVENT_MESSAGE};
				gce.dwFlags = GCEF_UTF8;
				gce.pszID.a = pUser->szId;
				gce.pszUID.a = participant;
				gce.bIsMe = key->fromme;
				gce.pszText.a = szMessageText.GetBuffer();
				gce.time = timestamp;
				Chat_Event(&gce);
			}
		}
	}

	if (body->protocolmessage) {
		auto *protoMsg = body->protocolmessage;
		switch (protoMsg->type) {
		case WA__MESSAGE__PROTOCOL_MESSAGE__TYPE__APP_STATE_SYNC_KEY_SHARE:
			for (int i = 0; i < protoMsg->appstatesynckeyshare->n_keys; i++) {
				auto *it = protoMsg->appstatesynckeyshare->keys[i];
				auto &keyid = it->keyid->keyid;
				auto &keydata = it->keydata->keydata;

				CMStringA szSetting(FORMAT, "AppSyncKey%d", decodeBigEndian(keyid));
				db_set_blob(0, m_szModuleName, szSetting, keydata.data, (unsigned)keydata.len);
			}
			break;

		case WA__MESSAGE__PROTOCOL_MESSAGE__TYPE__APP_STATE_FATAL_EXCEPTION_NOTIFICATION:
			m_impl.m_resyncApp.Stop();
			m_impl.m_resyncApp.Start(10000);
			break;

		case WA__MESSAGE__PROTOCOL_MESSAGE__TYPE__HISTORY_SYNC_NOTIFICATION:
			debugLogA("History sync notification");
			if (auto *pHist = protoMsg->historysyncnotification) {
				MBinBuffer buf(DownloadEncryptedFile(directPath2url(pHist->directpath), pHist->mediakey, "History"));
				if (!buf.isEmpty()) {
					MBinBuffer inflate(unzip(unpadBuffer16(buf)));

					proto::HistorySync sync(inflate);
					if (sync)
						ProcessHistorySync(sync);
				}
			}
			break;

		case WA__MESSAGE__PROTOCOL_MESSAGE__TYPE__REVOKE:
			break;

		case WA__MESSAGE__PROTOCOL_MESSAGE__TYPE__EPHEMERAL_SETTING:
			if (pUser) {
				setDword(pUser->hContact, DBKEY_EPHEMERAL_TS, timestamp);
				setDword(pUser->hContact, DBKEY_EPHEMERAL_EXPIRE, protoMsg->ephemeralexpiration);
			}
			break;
		}
	}
	else if (body->reactionmessage) {
		debugLogA("Got a reaction to a message");
	}
	else if (msg.has_messagestubtype) {
		switch (msg.messagestubtype) {
		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_LEAVE:
		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_REMOVE:
			debugLogA("Participant %s removed from chat", participant);
			break;

		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_ADD:
		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_INVITE:
		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_ADD_REQUEST_JOIN:
			debugLogA("Participant %s added to chat", participant);
			break;

		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_DEMOTE:
			debugLogA("Participant %s demoted", participant);
			break;

		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_PARTICIPANT_PROMOTE:
			debugLogA("Participant %s promoted", participant);
			break;

		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_CHANGE_ANNOUNCE:
			debugLogA("Groupchat announce", participant);
			break;

		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_CHANGE_RESTRICT:
			debugLogA("Groupchat restriction", participant);
			break;

		case WA__WEB_MESSAGE_INFO__STUB_TYPE__GROUP_CHANGE_SUBJECT:
			debugLogA("Groupchat subject was changed", participant);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnReceiveAck(const WANode &node)
{
	auto *pUser = FindUser(node.getAttr("from"));
	if (pUser == nullptr)
		return;

	if (!mir_strcmp(node.getAttr("class"), "message")) {
		WAOwnMessage tmp(0, 0, node.getAttr("id"));
		{
			mir_cslock lck(m_csOwnMessages);
			if (auto *pOwn = m_arOwnMsgs.find(&tmp)) {
				tmp.pktId = pOwn->pktId;
				m_arOwnMsgs.remove(pOwn);
			}
			else return;
		}
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tmp.pktId, (LPARAM)tmp.szMessageId.c_str());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

bool WhatsAppProto::CreateMsgParticipant(WANode *pParticipants, const WAJid &jid, const MBinBuffer &orig)
{
	int type = 0;

	try {
		MBinBuffer pBuffer(m_signalStore.encryptSignalProto(jid, orig, type));

		auto *pNode = pParticipants->addChild("to");
		pNode->addAttr("jid", jid.toString());

		auto *pEnc = pNode->addChild("enc");
		*pEnc << CHAR_PARAM("v", "2") << CHAR_PARAM("type", (type == 3) ? "pkmsg" : "msg");
		pEnc->content.assign(pBuffer.data(), pBuffer.length());
	}
	catch (const char *) {
	}
	
	return type == 3;
}

int WhatsAppProto::SendTextMessage(const char *jid, const char *pszMsg)
{
	WAJid toJid(jid);

	// send task creation
	auto *pTask = new WASendTask(jid);

	// basic message 
	Wa__Message body;
	body.conversation = (char*)pszMsg;

	if (toJid.isGroup()) {
		MBinBuffer encodedMsg(proto::Serialize(&body));
		padBuffer16(encodedMsg);

		MBinBuffer skmsgKey;
		MBinBuffer cipherText(m_signalStore.encryptSenderKey(toJid, m_szJid, encodedMsg, skmsgKey));

		auto *pEnc = pTask->payLoad.addChild("enc");
		*pEnc << CHAR_PARAM("v", "2") << CHAR_PARAM("type", "skmsg");
		pEnc->content.append(cipherText.data(), cipherText.length());

		Wa__Message__SenderKeyDistributionMessage sentBody;
		sentBody.axolotlsenderkeydistributionmessage.data = skmsgKey.data();
		sentBody.axolotlsenderkeydistributionmessage.len = skmsgKey.length();
		sentBody.has_axolotlsenderkeydistributionmessage = true;
		sentBody.groupid = (char*)jid;

		Wa__Message msg;
		msg.senderkeydistributionmessage = &sentBody;

		pTask->content.append(proto::Serialize(&msg));

		if (auto *pUser = FindUser(jid))
			if (pUser->si)
				for (auto &it : pUser->si->arUsers)
					pTask->arDest.insert(new WAJid(T2Utf(it->pszUID)));
	}
	else {
		Wa__Message__DeviceSentMessage sentBody;
		sentBody.message = &body;
		sentBody.destinationjid = (char*)jid;

		Wa__Message msg;
		msg.devicesentmessage = &sentBody;

		pTask->content.append(proto::Serialize(&msg));

		pTask->arDest.insert(new WAJid(toJid));
	}

	padBuffer16(pTask->content);

	for (auto &it : m_arDevices)
		if (it->device != (int)getDword(DBKEY_DEVICE_ID))
			pTask->arDest.insert(new WAJid(*it));

	// check session presence first
	LIST<WAJid> missingKeys(1);
	for (auto &it : pTask->arDest) {
		MSignalSession tmp(it->user, it->device);
		auto blob = getBlob(tmp.getSetting());
		if (blob.isEmpty())
			missingKeys.insert(it);
	}

	// generate & reserve packet id
	int pktId;
	{
		mir_cslock lck(m_csOwnMessages);
		pktId = m_iPacketId++;
		m_arOwnMsgs.insert(new WAOwnMessage(pktId, jid, pTask->szMsgId));
	}

	// if some keys are missing, schedule task for execution & retrieve keys
	if (missingKeys.getCount()) {
		WANodeIq iq(IQ::GET, "encrypt");
		auto *pKey = iq.addChild("key");
		for (auto &it: missingKeys)
			pKey->addChild("user")->addAttr("jid", it->toString());
		WSSendNode(iq, &WhatsAppProto::OnIqGetKeys, pTask);
	}
	// otherwise simply execute the task
	else SendTask(pTask);

	return pktId;
}

void WhatsAppProto::SendTask(WASendTask *pTask)
{
	// pack all data and send the whole payload
	bool shouldIncludeIdentity = false;
	auto *pParticipants = pTask->payLoad.addChild("participants");

	for (auto &it : pTask->arDest)
		shouldIncludeIdentity |= CreateMsgParticipant(pParticipants, *it, pTask->content);

	if (shouldIncludeIdentity) {
		MBinBuffer encIdentity(m_signalStore.encodeSignedIdentity(true));
		auto *pNode = pTask->payLoad.addChild("device-identity");
		pNode->content.assign(encIdentity.data(), encIdentity.length());
	}

	WSSendNode(pTask->payLoad);
	delete pTask;
}
