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

void CSteamProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	ptrT error(mir_tstrdup(TranslateT("Unknown error")));
	ptrT steamId(getTStringA(param->hContact, "SteamID"));

	if (response != NULL && response->resultCode == HTTP_CODE_OK)
	{
		JSONROOT root(response->pData);
		JSONNode *node = json_get(root, "error");
		if (node)
			error = json_as_string(node);
	}

	if (mir_tstrcmpi(error, _T("OK")) != 0)
	{
		ptrA errorA(mir_t2a(error));
		debugLogA("CSteamProto::OnMessageSent: failed to send message for %s (%s)", steamId, errorA);
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hMessage, (LPARAM)errorA);
		return;
	}

	ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, param->hMessage, 0);
}