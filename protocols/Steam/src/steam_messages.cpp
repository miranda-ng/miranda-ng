#include "stdafx.h"

void CSteamProto::SendFriendMessage(EChatEntryType entry_type, int64_t steamId, const char *pszMessage, void *pInfo)
{
	CFriendMessagesSendMessageRequest request;
	request.chat_entry_type = (int)entry_type; request.has_chat_entry_type = true;
	request.contains_bbcode = request.has_contains_bbcode = true;
	request.steamid = steamId; request.has_steamid = true;
	request.message = (char *)pszMessage;
	WSSendService(FriendSendMessage, request, pInfo);
}

void CSteamProto::OnMessageSent(const CFriendMessagesSendMessageResponse &reply, const CMsgProtoBufHeader &hdr)
{
	auto *pOwn = (COwnMessage*)GetRequestInfo(hdr.jobid_target);
	if (pOwn == nullptr)
		return;

	if (hdr.failed()) {
		CMStringW wszMessage(FORMAT, TranslateT("Message sending has failed with error %d"), hdr.eresult);
		ProtoBroadcastAck(pOwn->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)pOwn->iMessageId, (LPARAM)wszMessage.c_str());
	}
	else {
		uint32_t timestamp = (reply.has_server_timestamp) ? reply.server_timestamp : 0;
		if (timestamp > getDword(pOwn->hContact, DBKEY_LASTMSG))
			setDword(pOwn->hContact, DBKEY_LASTMSG, timestamp);

		pOwn->timestamp = timestamp;
		ProtoBroadcastAck(pOwn->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pOwn->iMessageId, 0);
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
			dbei.iTimestamp = reply.has_rtime32_server_timestamp ? reply.rtime32_server_timestamp : time(0);
			ProtoChainRecvMsg(hContact, dbei);
		}
		break;

	case EChatEntryType::Typing:
		CallService(MS_PROTO_CONTACTISTYPING, hContact, 10);
		break;
	}
}

void CSteamProto::OnGotMarkRead(const CFriendMessagesAckMessageNotification &reply, const CMsgProtoBufHeader &)
{
	MCONTACT hContact = GetContact(reply.steamid_partner);
	if (!hContact) {
		debugLogA("notification from unknown account %lld ignored", reply.steamid_partner);
		return;
	}

	DB::ECPTR pCursor(DB::Events(hContact, db_event_firstUnread(hContact)));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DB::EventInfo dbei(hDbEvent, false);
		if (reply.timestamp > dbei.iTimestamp)
			break;

		if (!dbei.bRead)
			db_event_markRead(hContact, hDbEvent, true);
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
