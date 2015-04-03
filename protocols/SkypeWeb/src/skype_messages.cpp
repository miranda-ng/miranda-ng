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
	return 0;
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