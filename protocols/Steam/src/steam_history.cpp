#include "stdafx.h"

void CSteamProto::OnGotConversations(const HttpResponse *response)
{
	if (!ResponseHttpOk(response))
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNode *node = json_get(root, "response");

	if (m_lastMessageTS <= 0)
	{
		// Remember and save actual timestamp (as it is first we've got)
		JSONNode *timestampNode = json_get(node, "timestamp");
		m_lastMessageTS = _ttoi64(ptrT(json_as_string(timestampNode)));
		setDword("LastMessageTS", m_lastMessageTS);

		// And don't load any messages as it may cause duplicates
		return;
	}

	JSONNode *sessions = json_get(node, "message_sessions");
	JSONNode *nsessions = json_as_array(sessions);

	if (nsessions != NULL)
	{
		ptrA token(getStringA("TokenSecret"));
		ptrA steamId(getStringA("SteamID"));
		

		for (size_t i = 0; i < json_size(nsessions); i++)
		{
			JSONNode *session = json_at(nsessions, i);

			node = json_get(session, "accountid_friend");
			const char *who = AccountIdToSteamId(_ttoi64(ptrT(json_as_string(node))));

			node = json_get(session, "last_message");
			time_t lastMessageTS = _ttoi64(ptrT(json_as_string(node)));

			/*node = json_get(session, "last_view");
			time_t last_view = _ttoi64(ptrT(json_as_string(node)));

			node = json_get(session, "unread_message_count");
			long unread_count = json_as_int(node);*/

			if (lastMessageTS > m_lastMessageTS)
			{
				PushRequest(
					new GetHistoryMessagesRequest(token, steamId, who, m_lastMessageTS),
					&CSteamProto::OnGotHistoryMessages,
					mir_strdup(who),
					MirFreeArg);
			}
		}

		json_delete(nsessions);
	}
}

void CSteamProto::OnGotHistoryMessages(const HttpResponse *response, void *arg)
{
	MCONTACT hContact = FindContact((char*)arg);
	if (!hContact)
		return;

	if (!ResponseHttpOk(response))
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNode *node = json_get(root, "response");

	JSONNode *messages = json_get(node, "messages");
	JSONNode *nmessages = json_as_array(messages);

	// Self SteamID
	ptrA steamId(getStringA("SteamID"));

	for (size_t i = json_size(nmessages) - 1; i >= 0; i--)
	{
		JSONNode *message = json_at(nmessages, i);

		node = json_get(message, "accountid");
		const char *authorSteamId = AccountIdToSteamId(_ttoi64(ptrT(json_as_string(node))));

		node = json_get(message, "message");
		ptrT text(json_as_string(node));
		T2Utf szMessage(text);

		node = json_get(message, "timestamp");
		time_t timestamp = _ttoi64(ptrT(json_as_string(node)));

		PROTORECVEVENT recv = { 0 };
		recv.timestamp = timestamp;
		recv.szMessage = szMessage;

		if (strcmp(steamId, authorSteamId))
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

		// Update last message timestamp
		if (timestamp > getDword("LastMessageTS", 0))
			setDword("LastMessageTS", timestamp);
	}

	json_delete(nmessages);
}
