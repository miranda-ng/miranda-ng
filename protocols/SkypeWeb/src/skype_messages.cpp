#include "common.h"

/* MESSAGE RECEIVING */

// writing message/even into db
int CSkypeProto::OnReceiveMessage(const char *from, const char *convLink, time_t timeStamp, char *content)
{
	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = timeStamp;
	recv.szMessage = content;
	debugLogA("Incoming message from %s", ContactUrlToName(from));
	if (IsMe(ContactUrlToName(from)))
	{
		recv.flags |= PREF_SENT;
		MCONTACT hContact = GetContact(ContactUrlToName(convLink));
		return ProtoChainRecvMsg(hContact, &recv);
	}
	MCONTACT hContact = GetContact(ContactUrlToName(from));
	return ProtoChainRecvMsg(hContact, &recv);
}

/* MESSAGE SENDING */

// outcoming message flow
int CSkypeProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	UINT hMessage = InterlockedIncrement(&hMessageProcess);

	SendMessageParam *param = (SendMessageParam*)mir_calloc(sizeof(SendMessageParam));
	param->hContact = hContact;
	param->hMessage = (HANDLE)hMessage;
	param->msg = szMessage;
	param->flags = flags;

	ForkThread(&CSkypeProto::SendMsgThread, (void*)param);

	return hMessage;
}

void CSkypeProto::SendMsgThread(void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	if (!IsOnline())
	{
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, param->hMessage, (LPARAM)Translate("You cannot send messages when you are offline."));
		mir_free(param);
		return;
	}

	CMStringA message = (param->flags & PREF_UNICODE) ? ptrA(mir_utf8encode(param->msg)) : param->msg; // TODO: mir_utf8encode check taken from FacebookRM, is it needed? Usually we get PREF_UTF8 flag instead.

	ptrA token(getStringA("registrationToken"));
	ptrA username(getStringA(param->hContact, "Skypename"));
	PushRequest(
		new SendMsgRequest(token, username, message, getStringA("Server"))/*,
																		  &CSkypeProto::OnMessageSent*/);
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;

	ptrT error(mir_tstrdup(TranslateT("Unknown error")));
	ptrT username(getTStringA(param->hContact, "Skypename"));

	if (response != NULL && (response->resultCode == 201 || response->resultCode == 200))
	{
		JSONROOT root(response->pData);
		JSONNODE *node = json_get(root, "errorCode");
		if (node)
			error = json_as_string(node);
	}

	int status = ACKRESULT_FAILED;

	if (error == NULL)
	{
		status = ACKRESULT_SUCCESS;
	}
	else
		debugLog(_T("CSkypeProto::OnMessageSent: failed to send message for %s (%s)"), username, error);

	ProtoBroadcastAck(
		param->hContact,
		ACKTYPE_MESSAGE,
		status,
		param->hMessage,
		error);
}