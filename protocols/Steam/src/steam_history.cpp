#include "stdafx.h"

void CSteamProto::SendHistoryRequest(uint64_t accountId, uint32_t startTime)
{
	CFriendMessagesGetRecentMessagesRequest request;
	request.steamid1 = m_iSteamId; request.has_steamid1 = true;
	request.steamid2 = AccountIdToSteamId(accountId); request.has_steamid2 = true;
	request.rtime32_start_time = startTime; request.has_rtime32_start_time = true;
	WSSendService(FriendGetRecentMessages, request);
}

void CSteamProto::OnGotRecentMessages(const CFriendMessagesGetRecentMessagesResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	for (int i = 0; i < reply.n_messages; i++) {
		auto *pMsg = reply.messages[i];
		auto steamId = AccountIdToSteamId(pMsg->accountid);

		MCONTACT hContact = GetContact(steamId);
		if (!hContact)
			continue;

		char szMsgId[100];
		itoa(pMsg->timestamp, szMsgId, 10);

		DB::EventInfo dbei(pMsg->has_timestamp ? db_event_getById(m_szModuleName, szMsgId) : 0);
		dbei.flags = DBEF_UTF;
		if (steamId == m_iSteamId)
			dbei.flags |= DBEF_SENT;
		dbei.cbBlob = (int)mir_strlen(pMsg->message);
		dbei.pBlob = mir_strdup(pMsg->message);
		dbei.timestamp = pMsg->has_timestamp ? pMsg->timestamp : time(0);
		dbei.szId = szMsgId;

		if (dbei.getEvent())
			db_event_edit(dbei.getEvent(), &dbei, true);
		else
			ProtoChainRecvMsg(hContact, dbei);
	}
}

void CSteamProto::OnGotConversations(const CFriendsMessagesGetActiveMessageSessionsResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	for (int i=0; i < reply.n_message_sessions; i++) {
		auto *session = reply.message_sessions[i];

		uint64_t steamId = AccountIdToSteamId(session->accountid_friend);
		MCONTACT hContact = GetContact(steamId);
		if (!hContact)
			continue;

		time_t storedMessageTS = getDword(hContact, DBKEY_LASTMSG);
		if (session->last_message > storedMessageTS)
			SendHistoryRequest(steamId, storedMessageTS);
	}
}

void CSteamProto::OnGotHistoryMessages(const CMsgClientChatGetFriendMessageHistoryResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	MCONTACT hContact = GetContact(reply.steamid);
	if (!hContact)
		return;

	for (int i = 0; i < reply.n_messages; i++) {
		auto *pMsg = reply.messages[i];
		
		char szMsgId[100];
		itoa(pMsg->timestamp, szMsgId, 10);

		DB::EventInfo dbei(pMsg->has_timestamp ? db_event_getById(m_szModuleName, szMsgId) : 0);
		dbei.flags = DBEF_UTF;
		if (pMsg->has_unread && !pMsg->unread)
			dbei.flags |= DBEF_READ;
		if (pMsg->accountid == m_iSteamId)
			dbei.flags |= DBEF_SENT;
		dbei.cbBlob = (int)mir_strlen(pMsg->message);
		dbei.pBlob = mir_strdup(pMsg->message);
		dbei.timestamp = pMsg->has_timestamp ? pMsg->timestamp : time(0);
		dbei.szId = szMsgId;

		if (dbei.getEvent())
			db_event_edit(dbei.getEvent(), &dbei, true);
		else
			ProtoChainRecvMsg(hContact, dbei);
	}
}
