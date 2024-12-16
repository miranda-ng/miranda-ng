#include "stdafx.h"

void CSteamProto::OnGotConversations(const CFriendsMessagesGetActiveMessageSessionsResponse &reply, const CMsgProtoBufHeader &hdr)
{
	if (hdr.failed())
		return;

	for (int i=0; i < reply.n_message_sessions; i++) {
		auto *session = reply.message_sessions[i];

		const char *who = AccountIdToSteamId(session->accountid_friend);
		MCONTACT hContact = GetContact(who);
		if (!hContact)
			continue;

		// Don't load any messages when we don't have lastMessageTS, as it may cause duplicates
		time_t storedMessageTS = getDword(hContact, DB_KEY_LASTMSGTS);
		if (storedMessageTS == 0)
			continue;

		time_t lastMessageTS = session->last_message;
		if (lastMessageTS > storedMessageTS)
			SendRequest(new GetHistoryMessagesRequest(m_szAccessToken, m_iSteamId, who, storedMessageTS), &CSteamProto::OnGotHistoryMessages, (void*)hContact);
	}
}

void CSteamProto::OnGotHistoryMessages(const JSONNode &root, void *arg)
{
	if (root.isnull())
		return;

	MCONTACT hContact = UINT_PTR(arg);
	time_t storedMessageTS = getDword(hContact, DB_KEY_LASTMSGTS);
	time_t newTS = storedMessageTS;

	const JSONNode &response = root["response"];
	const JSONNode &messages = response["messages"];
	for (size_t i = messages.size(); i > 0; i--) {
		const JSONNode &message = messages[i - 1];

		long long accountId = _wtoi64(message["accountid"].as_mstring());
		const char *steamId = AccountIdToSteamId(accountId);

		json_string text = message["message"].as_string();

		time_t timestamp = _wtoi64(message["timestamp"].as_mstring());

		// Ignore already existing messages
		if (timestamp <= storedMessageTS)
			continue;

		DB::EventInfo dbei;
		dbei.timestamp = timestamp;
		dbei.pBlob = (char *)text.c_str();

		if (IsMe(steamId))
			dbei.flags = DBEF_SENT;

		RecvMsg(hContact, dbei);

		if (timestamp > newTS)
			newTS = timestamp;
	}

	setDword(hContact, DB_KEY_LASTMSGTS, newTS);		
}
