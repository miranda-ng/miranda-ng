#include "stdafx.h"

void CSteamProto::SendFriendMessage(uint32_t msgId, int64_t steamId, const char *pszMessage)
{
	CFriendMessagesSendMessageRequest request;
	request.chat_entry_type = (int)EChatEntryType::ChatMsg; request.has_chat_entry_type = true;
	request.contains_bbcode = request.has_contains_bbcode = true;
	request.steamid = steamId; request.has_steamid = true;
	request.message = (char *)pszMessage;

	auto iSourceId = WSSendService(FriendSendMessage, request);
	mir_cslock lck(m_csOwnMessages);
	if (COwnMessage *pOwn = m_arOwnMessages.find((COwnMessage *)&msgId))
		pOwn->iSourceId = iSourceId;
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
	MCONTACT hContact = GetContact(AccountIdToSteamId(reply.steamid_friend));
	if (!hContact) {
		debugLogA("message from unknown account %lld ignored", reply.steamid_friend);
		return;
	}

	DB::EventInfo dbei;
	dbei.flags = DBEF_UTF;
	if (reply.has_local_echo && reply.local_echo)
		dbei.flags |= DBEF_SENT;
	dbei.cbBlob = (int)mir_strlen(reply.message);
	dbei.pBlob = reply.message;
	dbei.timestamp = reply.has_rtime32_server_timestamp ? reply.rtime32_server_timestamp : time(0);
	ProtoChainRecvMsg(hContact, dbei);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	// NOTE: Steam doesn't support sending "user stopped typing" so we're sending only positive info
	if (type == PROTOTYPE_SELFTYPING_OFF)
		return 0;

	ptrA steamId(getStringA(hContact, DBKEY_STEAM_ID));
	//SendRequest(new SendTypingRequest(m_szAccessToken, m_szUmqId, steamId));
	return 0;
}
