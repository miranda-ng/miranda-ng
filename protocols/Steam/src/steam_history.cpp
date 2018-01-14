#include "stdafx.h"

void CSteamProto::OnGotConversations(const JSONNode &root, void*)
{
	// Don't load any messages when we don't have lastMessageTS, as it may cause duplicates
	if (m_lastMessageTS <= 0)
		return;

	if (root.isnull())
		return;

	JSONNode response = root["response"];
	JSONNode sessions = response["message_sessions"].as_array();
	if (sessions.isnull())
		return;

	for (const JSONNode &session : sessions)
	{
		long long accountId = _wtoi64(session["accountid_friend"].as_mstring());
		const char *who = AccountIdToSteamId(accountId);

		time_t lastMessageTS = _wtoi64(session["last_message"].as_mstring());

		/*node = json_get(session, "last_view");
		time_t last_view = _wtoi64(ptrW(json_as_string(node)));

		node = json_get(session, "unread_message_count");
		long unread_count = json_as_int(node);*/

		if (lastMessageTS > m_lastMessageTS)
		{
			ptrA token(getStringA("TokenSecret"));
			ptrA steamId(getStringA("SteamID"));

			PushRequest(
				new GetHistoryMessagesRequest(token, steamId, who, m_lastMessageTS),
				&CSteamProto::OnGotHistoryMessages,
				mir_strdup(who));
		}
	}
}

void CSteamProto::OnGotHistoryMessages(const JSONNode &root, void *arg)
{
	ptrA cSteamId((char*)arg);

	if (root.isnull())
		return;

	JSONNode response = root["response"];
	JSONNode messages = response["messages"].as_array();
	for (size_t i = messages.size(); i > 0; i--)
	{
		JSONNode message = messages[i - 1];

		long long accountId = _wtoi64(message["accountid"].as_mstring());
		const char *steamId = AccountIdToSteamId(accountId);

		json_string text = message["message"].as_string();

		time_t timestamp = _wtoi64(message["timestamp"].as_mstring());

		// Ignore already existing messages
		if (timestamp <= m_lastMessageTS)
			continue;

		PROTORECVEVENT recv = { 0 };
		recv.timestamp = timestamp;
		recv.szMessage = (char*)text.c_str();

		MCONTACT hContact = GetContact(cSteamId);
		if (!hContact)
			return;

		if (IsMe(steamId))
		{
			// Received message
			ProtoChainRecvMsg(hContact, &recv);
		}
		else
		{
			// Sent message
			recv.flags = PREF_SENT;
			Proto_RecvMessage(hContact, &recv);
		}
	}
}
