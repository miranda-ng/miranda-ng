#include "stdafx.h"

int CSteamProto::SendMsg(MCONTACT hContact, MEVENT, const char *message)
{
	if (!IsOnline())
		return -1;

	UINT hMessage = InterlockedIncrement(&hMessageProcess);
	CMStringA szId(FORMAT, "%d", hMessage);
	{
		mir_cslock lck(m_csOwnMessages);
		m_arOwnMessages.insert(new COwnMessage(hContact, hMessage));
	}

	CFriendMessagesSendMessageRequest request;
	request.chat_entry_type = (int)EChatEntryType::ChatMsg; request.has_chat_entry_type = true;
	request.client_message_id = szId.GetBuffer();
	request.contains_bbcode = request.has_contains_bbcode = true;
	request.steamid = GetId(hContact, DBKEY_STEAM_ID); request.has_steamid = true;
	request.message = (char *)message;
	WSSendClient("FriendMessages.SendMessage#1", request, &CSteamProto::OnMessageSent);

	return hMessage;
}

void CSteamProto::OnMessageSent(const uint8_t *buf, size_t cbLen)
{
	proto::FriendMessagesSendMessageResponse reply(buf, cbLen);
	if (!reply)
		return;

	COwnMessage *pOwn;
	{
		mir_cslock lck(m_csOwnMessages);
		pOwn = m_arOwnMessages.find((COwnMessage *)&reply->ordinal);
	}
	
	if (pOwn) {
		uint32_t timestamp = (reply->has_server_timestamp) ? reply->server_timestamp : 0;
		if (timestamp > getDword(pOwn->hContact, DB_KEY_LASTMSGTS))
			setDword(pOwn->hContact, DB_KEY_LASTMSGTS, timestamp);

		pOwn->timestamp = timestamp;
		ProtoBroadcastAck(pOwn->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)pOwn->iMessageId, 0);
	}
}

int CSteamProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;
	if (mir_strcmp(Proto_GetBaseAccountName(evt->hContact), m_szModuleName))
		return 0;

	mir_cslock lck(m_csOwnMessages);
	if (auto *pOwn = m_arOwnMessages.find((COwnMessage *)&evt->seq)) {
		evt->dbei->timestamp = pOwn->timestamp;
		m_arOwnMessages.remove(pOwn);
	}

	return 0;
}

int CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	// NOTE: Steam doesn't support sending "user stopped typing" so we're sending only positive info
	if (type == PROTOTYPE_SELFTYPING_OFF)
		return 0;

	ptrA steamId(getStringA(hContact, DBKEY_STEAM_ID));
	//SendRequest(new SendTypingRequest(m_szAccessToken, m_szUmqId, steamId));
	return 0;
}
