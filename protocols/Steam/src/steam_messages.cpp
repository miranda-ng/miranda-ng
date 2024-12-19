#include "stdafx.h"

int64_t CSteamProto::SendFriendMessage(EChatEntryType entry_type, int64_t steamId, const char *pszMessage)
{
	CFriendMessagesSendMessageRequest request;
	request.chat_entry_type = (int)entry_type; request.has_chat_entry_type = true;
	request.contains_bbcode = request.has_contains_bbcode = true;
	request.steamid = steamId; request.has_steamid = true;
	request.message = (char *)pszMessage;
	return WSSendService(FriendSendMessage, request);
}

void CSteamProto::OnMessageSent(const CFriendMessagesSendMessageResponse &reply, const CMsgProtoBufHeader &hdr)
{
	COwnMessage tmp(0, 0);
	{
		mir_cslock lck(m_csOwnMessages);
		for (auto &it : m_arOwnMessages)
			if (it->iSourceId == hdr.jobid_target) {
				tmp = *it;
				m_arOwnMessages.remove(m_arOwnMessages.indexOf(&it));
				break;
			}
	}

	if (!tmp.hContact)
		return;

	if (hdr.failed()) {
		CMStringW wszMessage(FORMAT, TranslateT("Message sending has failed with error %d"), hdr.eresult);
		ProtoBroadcastAck(tmp.hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tmp.iMessageId, (LPARAM)wszMessage.c_str());
	}
	else {
		uint32_t timestamp = (reply.has_server_timestamp) ? reply.server_timestamp : 0;
		if (timestamp > getDword(tmp.hContact, DB_KEY_LASTMSGTS))
			setDword(tmp.hContact, DB_KEY_LASTMSGTS, timestamp);

		tmp.timestamp = timestamp;
		ProtoBroadcastAck(tmp.hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tmp.iMessageId, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSteamProto::OnGotIncomingMessage(const CFriendMessagesIncomingMessageNotification &reply, const CMsgProtoBufHeader &)
{
	MCONTACT hContact = GetContact(reply.steamid_friend);
	if (!hContact) {
		debugLogA("message from unknown account %lld ignored", reply.steamid_friend);
		return;
	}

	switch (EChatEntryType(reply.chat_entry_type)) {
	case EChatEntryType::ChatMsg:
		{
			DB::EventInfo dbei;
			dbei.flags = DBEF_UTF;
			if (reply.has_local_echo && reply.local_echo)
				dbei.flags |= DBEF_SENT;
			dbei.cbBlob = (int)mir_strlen(reply.message);
			dbei.pBlob = reply.message;
			dbei.timestamp = reply.has_rtime32_server_timestamp ? reply.rtime32_server_timestamp : time(0);
			ProtoChainRecvMsg(hContact, dbei);
		}
		break;

	case EChatEntryType::Typing:
		CallService(MS_PROTO_CONTACTISTYPING, hContact, 10);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	// NOTE: Steam doesn't support sending "user stopped typing" so we're sending only positive info
	if (type == PROTOTYPE_SELFTYPING_ON)
		SendFriendMessage(EChatEntryType::Typing, GetId(hContact, DBKEY_STEAM_ID), "");
	return 0;
}
