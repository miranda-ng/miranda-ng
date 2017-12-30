#include "stdafx.h"

struct SendMessageParam
{
	MCONTACT hContact;
	HANDLE hMessage;
	char *message;
};

int CSteamProto::OnSendMessage(MCONTACT hContact, const char *message)
{
	UINT hMessage = InterlockedIncrement(&hMessageProcess);

	SendMessageParam *param = (SendMessageParam*)mir_calloc(sizeof(SendMessageParam));
	param->hContact = hContact;
	param->hMessage = (HANDLE)hMessage;
	param->message = mir_strdup(message);

	ptrA token(getStringA("TokenSecret"));
	ptrA umqid(getStringA("UMQID"));
	ptrA steamId(getStringA(hContact, "SteamID"));
	PushRequest(
		new SendMessageRequest(token, umqid, steamId, message),
		&CSteamProto::OnMessageSent,
		param);

	return hMessage;
}

void CSteamProto::OnMessageSent(const HttpResponse &response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	std::string error = Translate("Unknown error");
	ptrW steamId(getWStringA(param->hContact, "SteamID"));
	time_t timestamp = NULL;

	if (response)
	{
		JSONNode root = JSONNode::parse(response.Content);
		JSONNode node = root["error"];
		if (!node.isnull())
			error = node.as_string();

		node = root["utc_timestamp"];
		if (!node.isnull())
		{
			timestamp = atol(node.as_string().c_str());
			if (timestamp > getDword("LastMessageTS", 0))
				setDword("LastMessageTS", timestamp);
		}
	}

	if (mir_strcmpi(error.c_str(), "OK") != 0)
	{
		debugLogA(__FUNCTION__ ": failed to send message for %s (%s)", steamId, error.c_str());
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hMessage, (LPARAM)error.c_str());
	}
	else
	{
		// remember server time of this message
		auto it = m_mpOutMessages.find(param->hMessage);
		if (it == m_mpOutMessages.end() && timestamp != NULL)
			m_mpOutMessages[param->hMessage] = timestamp;

		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hMessage, 0);
	}

	mir_free(param->message);
	mir_free(param);
}

int CSteamProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;

	auto it = m_mpOutMessages.find((HANDLE)evt->seq);
	if (it != m_mpOutMessages.end())
	{
		evt->dbei->timestamp = it->second;
		m_mpOutMessages.erase(it);
	}

	return 0;
}

int CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	// NOTE: Steam doesn't support sending "user stopped typing" so we're sending only positive info
	if (hContact && IsOnline() && type == PROTOTYPE_SELFTYPING_ON)
	{
		ptrA token(getStringA("TokenSecret"));
		ptrA umqid(getStringA("UMQID"));
		ptrA steamId(getStringA(hContact, "SteamID"));
		PushRequest(new SendTypingRequest(token, umqid, steamId));
	}

	return 0;
}
