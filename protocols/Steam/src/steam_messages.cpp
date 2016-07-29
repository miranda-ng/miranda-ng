#include "stdafx.h"

struct SendMessageParam
{
	MCONTACT hContact;
	HANDLE hMessage;
	char *message;
};

void MessageParamFree(void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	mir_free(param->message);
	mir_free(param);
}

int CSteamProto::OnSendMessage(MCONTACT hContact, const char* message)
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
		param, MessageParamFree);

	return hMessage;
}

void CSteamProto::OnMessageSent(const HttpResponse *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	ptrW error(mir_wstrdup(TranslateT("Unknown error")));
	ptrW steamId(getWStringA(param->hContact, "SteamID"));
	time_t timestamp = NULL;

	if (ResponseHttpOk(response))
	{
		JSONROOT root(response->pData);
		JSONNode *node = json_get(root, "error");
		if (node)
			error = json_as_string(node);

		node = json_get(root, "utc_timestamp");
		if (node)
		{
			timestamp = atol(ptrA(mir_u2a(ptrW(json_as_string(node)))));
			if (timestamp > getDword("LastMessageTS", 0))
				setDword("LastMessageTS", timestamp);
		}
	}

	if (mir_wstrcmpi(error, L"OK") != 0)
	{
		ptrA errorA(mir_u2a(error));
		debugLogA("CSteamProto::OnMessageSent: failed to send message for %s (%s)", steamId, errorA);
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hMessage, (LPARAM)errorA);
	}
	else
	{
		// remember server time of this message
		auto it = m_mpOutMessages.find(param->hMessage);
		if (it == m_mpOutMessages.end() && timestamp != NULL)
			m_mpOutMessages[param->hMessage] = timestamp;

		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hMessage, 0);
	}
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
