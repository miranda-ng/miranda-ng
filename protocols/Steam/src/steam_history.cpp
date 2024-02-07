#include "stdafx.h"

void CSteamProto::OnGotConversations(const JSONNode &root, void *)
{
	if (root.isnull())
		return;

	const JSONNode &response = root["response"];
	for (auto &session : response["message_sessions"]) {
		long long accountId = _wtoi64(session["accountid_friend"].as_mstring());

		const char *who = AccountIdToSteamId(accountId);
		MCONTACT hContact = GetContact(who);
		if (!hContact)
			continue;

		// Don't load any messages when we don't have lastMessageTS, as it may cause duplicates
		time_t storedMessageTS = getDword(hContact, DB_KEY_LASTMSGTS);
		if (storedMessageTS == 0)
			continue;

		time_t lastMessageTS = _wtoi64(session["last_message"].as_mstring());
		if (lastMessageTS > storedMessageTS) {
			ptrA token(getStringA("TokenSecret"));
			ptrA steamId(getStringA(DBKEY_STEAM_ID));
			SendRequest(new GetHistoryMessagesRequest(token, steamId, who, storedMessageTS), &CSteamProto::OnGotHistoryMessages, (void*)hContact);
		}
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
