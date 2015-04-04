#include "common.h"

/* MESSAGE RECEIVING */

// incoming message flow
int CSkypeProto::OnReceiveMessage(const char *messageId, const char *from, const char *to, time_t timestamp, char *content, int emoteOffset, bool isRead)
{
	setDword("LastMsgTime", timestamp);
	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = timestamp;
	recv.szMessage = &content[emoteOffset];
	recv.lParam = emoteOffset == 0
		? EVENTTYPE_MESSAGE
		: SKYPE_DB_EVENT_TYPE_ACTION;
	recv.pCustomData = (void*)messageId;
	recv.cbCustomDataSize = mir_strlen(messageId);
	if (isRead)
		recv.flags |= PREF_CREATEREAD;
	ptrA skypename(ContactUrlToName(from));
	debugLogA("Incoming message from %s", skypename);
	if (IsMe(skypename))
	{
		recv.flags |= PREF_SENT;
		MCONTACT hContact = GetContact(ptrA(ContactUrlToName(to)));
		return SaveMessageToDb(hContact, &recv);
	}
	MCONTACT hContact = GetContact(skypename);
	return ProtoChainRecvMsg(hContact, &recv);
}

// writing message/even into db
int CSkypeProto::SaveMessageToDb(MCONTACT hContact, PROTORECVEVENT *pre)
{
	//return Proto_RecvMessage(hContact, pre);
	if (pre->szMessage == NULL)
		return NULL;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = GetContactProto(hContact);
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF;
	if ((pre->flags & PREF_CREATEREAD) == PREF_CREATEREAD)
		dbei.flags |= DBEF_READ;
	if ((pre->flags & PREF_SENT) == PREF_SENT)
		dbei.flags |= DBEF_SENT;
	dbei.eventType = pre->lParam;
	dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	return (INT_PTR)db_event_add(hContact, &dbei);
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
	time_t timestamp = time(NULL); //InterlockedIncrement(&hMessageProcess);

	if (!IsOnline())
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send when you are offline.");
		return 0;
	}

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	param->hMessage = (HANDLE)timestamp;

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
	if (strncmp(message, "/me ", 4) == 0)
	{
		// TODO: make /me action send when it will work in skype web
	}
	PushRequest(new SendMsgRequest(token, username, timestamp, message, server), &CSkypeProto::OnMessageSent, param);

	return timestamp;
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = param->hMessage;
	delete param;

	if (response->resultCode != 200 && response->resultCode != 201)
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

// preparing message/action to writing into db
int CSkypeProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0)
	{
		evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
		memcpy(&evt->dbei->pBlob, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
		evt->dbei->eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	}
	char messageId[20];
	itoa(evt->seq, messageId, 10);
	int messageIdLength = mir_strlen(messageId);
	evt->dbei->pBlob = (PBYTE)mir_realloc(evt->dbei->pBlob, evt->dbei->cbBlob + messageIdLength);
	memcpy((char *)&evt->dbei->pBlob[evt->dbei->cbBlob], messageId, messageIdLength);
	evt->dbei->cbBlob += messageIdLength;

	return 1;
}

/* HISTORY SYNC */

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
		{
			int emoteOffset = json_as_int(json_get(message, "skypeemoteoffset"));
			OnReceiveMessage(clientMsgId, from, conversationLink, timestamp, content, emoteOffset, true);
		}
	}
}