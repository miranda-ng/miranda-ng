/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

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
