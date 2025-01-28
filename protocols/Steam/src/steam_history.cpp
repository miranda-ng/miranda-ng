#include "stdafx.h"

INT_PTR CSteamProto::SvcCanEmptyHistory(WPARAM hContact, LPARAM)
{
	return (hContact && Contact::IsGroupChat(hContact));
}

INT_PTR CSteamProto::SvcEmptyHistory(WPARAM, LPARAM)
{
	return 1;
}

INT_PTR CSteamProto::SvcLoadServerHistory(WPARAM hContact, LPARAM)
{
	if (Contact::IsGroupChat(hContact)) {
		CChatRoomGetMessageHistoryRequest request;
		request.chat_group_id = GetId(hContact, DBKEY_STEAM_ID); request.has_chat_group_id = true;
		request.chat_id = getDword(hContact, "ChatId"); request.has_chat_id = true;
		request.max_count = 100; request.has_max_count = true;
		WSSendService(GetChatHistory, request, (void *)hContact);
	}
	else {
		CFriendMessagesGetRecentMessagesRequest request;
		request.steamid1 = m_iSteamId; request.has_steamid1 = true;
		request.steamid2 = GetId(hContact, DBKEY_STEAM_ID); request.has_steamid2 = true;
		request.count = 100; request.has_count = true;
		request.most_recent_conversation = request.has_most_recent_conversation = true;
		WSSendService(FriendGetRecentMessages, request);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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

		MEVENT hEvent;
		char szMsgId[100];
		if (pMsg->has_timestamp) {
			itoa(pMsg->timestamp, szMsgId, 10);
			hEvent = db_event_getById(m_szModuleName, szMsgId);
		}
		else hEvent = 0;

		DB::EventInfo dbei(hEvent);
		dbei.flags = DBEF_UTF;
		if (steamId == m_iSteamId)
			dbei.flags |= DBEF_SENT;
		dbei.cbBlob = (int)mir_strlen(pMsg->message);
		dbei.pBlob = mir_strdup(pMsg->message);
		if (pMsg->has_timestamp) {
			if (getDword(hContact, DBKEY_LASTMSG) < pMsg->timestamp)
				setDword(hContact, DBKEY_LASTMSG, pMsg->timestamp);

			dbei.szId = szMsgId;
			dbei.iTimestamp = pMsg->timestamp;
		}
		else dbei.iTimestamp = time(0);

		if (dbei.getEvent())
			db_event_edit(hEvent, &dbei, true);
		else
			ProtoChainRecvMsg(hContact, dbei);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

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

	uint32_t iLastMessage = getDword(hContact, DBKEY_LASTMSG);

	for (int i = 0; i < reply.n_messages; i++) {
		auto *pMsg = reply.messages[i];
		
		MEVENT hEvent;
		char szMsgId[100];

		if (pMsg->has_timestamp) {
			if (iLastMessage < pMsg->timestamp)
				iLastMessage = pMsg->timestamp;
			itoa(pMsg->timestamp, szMsgId, 10);
			hEvent = db_event_getById(m_szModuleName, szMsgId);
		}
		else hEvent = 0;

		DB::EventInfo dbei(hEvent);
		dbei.flags = DBEF_UTF;
		if (pMsg->has_unread && !pMsg->unread)
			dbei.flags |= DBEF_READ;
		if (pMsg->accountid == m_iSteamId)
			dbei.flags |= DBEF_SENT;
		dbei.cbBlob = (int)mir_strlen(pMsg->message);
		dbei.pBlob = mir_strdup(pMsg->message);
		if (pMsg->has_timestamp) {
			dbei.iTimestamp = pMsg->timestamp;
			dbei.szId = szMsgId;
		}
		else dbei.iTimestamp = time(0);

		if (dbei.getEvent())
			db_event_edit(hEvent, &dbei, true);
		else
			ProtoChainRecvMsg(hContact, dbei);
	}

	setDword(hContact, DBKEY_LASTMSG, iLastMessage);
}
