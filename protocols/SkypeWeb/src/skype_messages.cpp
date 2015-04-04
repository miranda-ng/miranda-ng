#include "common.h"

/* MESSAGE RECEIVING */

// writing message/even into db
int CSkypeProto::OnReceiveMessage(const char *from, const char *convLink, time_t timestamp, char *content)
{
	setDword("LastMsgTime", timestamp);
	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = timestamp;
	recv.szMessage = content;

	ptrA skypename(ContactUrlToName(from));
	debugLogA("Incoming message from %s", skypename);
	if (IsMe(skypename))
	{
		recv.flags |= PREF_SENT;
		MCONTACT hContact = GetContact(ContactUrlToName(convLink));
		return ProtoChainRecvMsg(hContact, &recv);
	}
	MCONTACT hContact = GetContact(skypename);
	return ProtoChainRecvMsg(hContact, &recv);
}

/* MESSAGE SENDING */

struct SendMessageParam
{
	MCONTACT hContact;
	HANDLE hMessage;
};

// outcoming message flow
int CSkypeProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	ULONG hMessage = InterlockedIncrement(&hMessageProcess);

	if (!IsOnline())
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send when you are offline.");
		return 0;
	}

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	param->hMessage = (HANDLE)hMessage;

	ptrA message;
	if (flags & PREF_UNICODE)
		message = mir_utf8encodeW((wchar_t*)&szMessage[mir_strlen(szMessage) + 1]);
	else if (flags & PREF_UTF)
		message = mir_strdup(szMessage);
	else
		message = mir_utf8encode(szMessage);

	ptrA server(getStringA("Server"));
	ptrA token(getStringA("registrationToken"));
	ptrA username(getStringA(hContact, "Skypename"));
	PushRequest(new SendMsgRequest(token, username, message, server), &CSkypeProto::OnMessageSent, param);

	return hMessage;
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = param->hMessage;
	delete param;

	if (response->resultCode != 200 || response->resultCode != 201)
	{
		CMStringA error = "Unknown error";
		if (response)
		{
			JSONROOT root(response->pData);
			JSONNODE *node = json_get(root, "errorCode");
			error = _T2A(json_as_string(node));
		}
		ptrT username(getTStringA(hContact, "Skypename"));
		debugLogA(__FUNCTION__": failed to send message for %s (%s)", username, error);
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, hMessage, (LPARAM)error.GetBuffer());
		return;
	}

	ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, hMessage, 0);
}

void CSkypeProto::OnGetServerHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *conversations = json_as_array(json_get(root, "conversations"));
	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *message = json_get(json_at(conversations, i), "lastMessage");

		ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(message, "clientmessageid")))));
		ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(message, "skypeeditedid")))));
		ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(message, "messagetype")))));
		ptrA from(mir_t2a(ptrT(json_as_string(json_get(message, "from")))));
		ptrA content(mir_t2a(ptrT(json_as_string(json_get(message, "content")))));
		ptrT composeTime(json_as_string(json_get(message, "composetime")));
		ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(message, "conversationLink")))));
		time_t timestamp = IsoToUnixTime(composeTime);
		if (conversationLink != NULL && strstr(conversationLink, "/8:"))
			OnReceiveMessage(from, conversationLink, timestamp, content);
	}
}