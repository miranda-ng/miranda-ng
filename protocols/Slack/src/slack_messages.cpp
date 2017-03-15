#include "stdafx.h"

/* MESSAGE RECEIVING */

// incoming message flow


// writing message/even into db
int CSlackProto::OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre)
{
	//return Proto_RecvMessage(hContact, pre);
	if (pre->szMessage == NULL)
		return NULL;

	DBEVENTINFO dbei = {};
	dbei.szModule = GetContactProto(hContact);
	dbei.timestamp = pre->timestamp;
	dbei.flags = 0;
	dbei.eventType = pre->lParam;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	return (INT_PTR)db_event_add(hContact, &dbei);
}

/* MESSAGE SENDING */

// outcoming message flow
void CSlackProto::OnMessageSent(JSONNode &root, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	if (!root)
	{
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)param->hMessage, (LPARAM)Translate("Unable to send message."));
		return;
	}

	bool isOk = root["ok"].as_bool();
	if (!isOk)
	{
		json_string error = root["error"].as_string();
		debugLogA(__FUNCTION__": %s", error);
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)param->hMessage, (LPARAM)error.c_str());
		return;
	}

	ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)param->hMessage, 0);
}

void CSlackProto::OnImChannelOppened(JSONNode &root, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	if (!root)
	{
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)param->hMessage, (LPARAM)Translate("Unable to open im channel."));
		mir_free(param->message);
		mir_free(param);
		return;
	}

	bool isOk = root["ok"].as_bool();
	if (!isOk)
	{
		json_string error = root["error"].as_string();
		debugLogA(__FUNCTION__": %s", error);
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)param->hMessage, (LPARAM)error.c_str());
		mir_free(param->message);
		mir_free(param);
		return;
	}

	JSONNode channel = root["channel"].as_node();
	json_string channelId = channel["id"].as_string();

	ptrA token(getStringA("TokenSecret"));

	char *message = param->message;
	HttpRequest *request;
	if (mir_strncmpi(message, "/me ", 4) == 0)
	{
		message += 4;
		request = new SendActionRequest(token, channelId.c_str(), message);
	}
	else
		request = new SendMessageRequest(token, channelId.c_str(), message);

	SendRequest(request, &CSlackProto::OnMessageSent, param);
}

void CSlackProto::SendMessageThread(void *arg)
{
	Thread_SetName("SLACK: SendMessageThread");

	SendMessageParam *param = (SendMessageParam*)arg;

	ptrA token(getStringA("TokenSecret"));
	ptrA user(getStringA(param->hContact, "UserId"));
	SendRequest(new OpenImChannelRequest(token, user), &CSlackProto::OnImChannelOppened, param);
}

void CSlackProto::SendMessageAckThread(void *arg)
{
	Sleep(100);
	ProtoBroadcastAck((MCONTACT)arg, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)1, (LPARAM)Translate("You cannot send when you are offline"));
}

// message is received by the other side

// preparing message/action to writing into db
int CSlackProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;
}

/* TYPING */

int CSlackProto::OnUserIsTyping(MCONTACT hContact, int type)
{
	return 0;
}