#include "common.h"

MEVENT CSkypeProto::GetMessageFromDb(MCONTACT hContact, const char *messageId, LONGLONG timestamp)
{
	mir_cslock lock(messageSyncLock);

	size_t messageIdLength = mir_strlen(messageId);
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent))
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		dbei.cbBlob = db_event_getBlobSize(hDbEvent);

		if (dbei.cbBlob < messageIdLength)
			continue;

		mir_ptr<BYTE> blob((PBYTE)mir_alloc(dbei.cbBlob));
		dbei.pBlob = blob;
		db_event_get(hDbEvent, &dbei);

		if (dbei.timestamp < timestamp)
			break;

		if (dbei.eventType != EVENTTYPE_MESSAGE && dbei.eventType != SKYPE_DB_EVENT_TYPE_ACTION)
			continue;

		if (memcmp(&dbei.pBlob[dbei.cbBlob - messageIdLength], messageId, messageIdLength) == 0)
			return hDbEvent;
	}

	return NULL;
}

MEVENT CSkypeProto::AddMessageToDb(MCONTACT hContact, DWORD timestamp, DWORD flags, const char *messageId, char *content, int emoteOffset)
{
	if (MEVENT hDbEvent = GetMessageFromDb(hContact, messageId, timestamp))
		return hDbEvent;
	size_t messageLength = mir_strlen(&content[emoteOffset]) + 1;
	size_t messageIdLength = mir_strlen(messageId);
	size_t cbBlob = messageLength + messageIdLength;
	PBYTE pBlob = (PBYTE)mir_alloc(cbBlob);
	memcpy(pBlob, &content[emoteOffset], messageLength);
	memcpy(pBlob + messageLength, messageId, messageIdLength);

	return AddEventToDb(hContact, emoteOffset == 0 ? EVENTTYPE_MESSAGE : SKYPE_DB_EVENT_TYPE_ACTION, timestamp, flags, cbBlob, pBlob);
}

/* MESSAGE RECEIVING */

// incoming message flow
int CSkypeProto::OnReceiveMessage(const char *messageId, const char *url, time_t timestamp, char *content, int emoteOffset, bool isRead)
{
	ptrA skypename(ContactUrlToName(url));
	MCONTACT hContact = GetContact(skypename);
	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = timestamp;
	recv.szMessage = content;
	recv.lParam = emoteOffset;
	recv.pCustomData = (void*)messageId;
	recv.cbCustomDataSize = mir_strlen(messageId);
	if (isRead)
		recv.flags |= PREF_CREATEREAD;
	debugLogA("Incoming message from %s", skypename);
	return ProtoChainRecvMsg(hContact, &recv);
}

// writing message/even into db
int CSkypeProto::SaveMessageToDb(MCONTACT hContact, PROTORECVEVENT *pre)
{
	//return Proto_RecvMessage(hContact, pre);
	if (pre->szMessage == NULL)
		return NULL;

	int flags = DBEF_UTF;
	if ((pre->flags & PREF_CREATEREAD) == PREF_CREATEREAD)
		flags |= DBEF_READ;
	if ((pre->flags & PREF_SENT) == PREF_SENT)
		flags |= DBEF_SENT;

	return AddMessageToDb(hContact, pre->timestamp, flags, (char*)pre->pCustomData, pre->szMessage, pre->lParam);
}

/* MESSAGE SENDING */

struct SendMessageParam
{
	MCONTACT hContact;
	LONGLONG hMessage;
};

// outcoming message flow
int CSkypeProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	if (!IsOnline())
	{
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send when you are offline.");
		return 0;
	}

	SendMessageParam *param = new SendMessageParam();
	param->hContact = hContact;
	param->hMessage = time(NULL);

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

	debugLogA(__FUNCTION__ " clientmsgid = %d", param->hMessage);

	if (strncmp(message, "/me ", 4) == 0)
		PushRequest(new SendActionRequest(token, username, param->hMessage, &message[4], server), &CSkypeProto::OnMessageSent, param);
	else
		PushRequest(new SendMessageRequest(token, username, param->hMessage, message, server), &CSkypeProto::OnMessageSent, param);

	return param->hMessage;
}

void CSkypeProto::OnMessageSent(const NETLIBHTTPREQUEST *response, void *arg)
{
	SendMessageParam *param = (SendMessageParam*)arg;
	MCONTACT hContact = param->hContact;
	HANDLE hMessage = (HANDLE)param->hMessage;
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
		memmove(evt->dbei->pBlob, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
		evt->dbei->eventType = SKYPE_DB_EVENT_TYPE_ACTION;
	}

	CMStringA messageId(FORMAT, "%d", evt->seq);
	evt->dbei->pBlob = (PBYTE)mir_realloc(evt->dbei->pBlob, evt->dbei->cbBlob + messageId.GetLength());
	memcpy(&evt->dbei->pBlob[evt->dbei->cbBlob], messageId, messageId.GetLength());
	evt->dbei->cbBlob += messageId.GetLength();

	return 0;
}

/* HISTORY SYNC */

void CSkypeProto::OnGetServerHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *conversations = json_as_array(json_get(root, "messages"));
	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *message = json_at(conversations, i);

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

			int flags = DBEF_UTF | DBEF_READ;

			ptrA skypename(ContactUrlToName(from));
			
			bool isMe = IsMe(skypename);
			if (isMe)
				flags |= DBEF_SENT;

			MCONTACT hContact = IsMe(skypename)
				? GetContact(ptrA(ContactUrlToName(conversationLink)))
				: GetContact(skypename);

			AddMessageToDb(hContact, timestamp, flags, clientMsgId, content, emoteOffset);
		}
	}
}

INT_PTR CSkypeProto::GetContactHistory(WPARAM hContact, LPARAM lParam)
{
	PushRequest(new GetHistoryRequest(ptrA(getStringA("registrationToken")), ptrA(db_get_sa(hContact, m_szModuleName, "Skypename")), ptrA(getStringA("Server"))), &CSkypeProto::OnGetServerHistory);
	return 0;
}

void CSkypeProto::SyncHistory()
{
	PushRequest(new SyncHistoryFirstRequest(ptrA(getStringA("registrationToken")), ptrA(getStringA("Server"))), &CSkypeProto::OnSyncHistory);
}

void CSkypeProto::OnSyncHistory(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;
	JSONROOT root(response->pData);
	if (root == NULL)
		return;
	JSONNODE *conversations = json_as_array(json_get(root, "conversations"));
	for (size_t i = 0; i < json_size(conversations); i++)
	{
		JSONNODE *conversation = json_at(conversations, i);
		JSONNODE *lastMessage = json_get(conversation, "lastMessage");
		if (lastMessage == NULL)
			continue;

		char *clientMsgId = _T2A(json_as_string(json_get(lastMessage, "clientmessageid")));
		char *conversationLink = _T2A(json_as_string(json_get(lastMessage, "conversationLink")));
		time_t composeTime(IsoToUnixTime(ptrT(json_as_string(json_get(lastMessage, "conversationLink")))));

		ptrA skypename(ContactUrlToName(conversationLink));
		if (skypename == NULL)
			return;
		MCONTACT hContact = GetContact(skypename);
		if (hContact == NULL && !IsMe(skypename))
			hContact = AddContact(skypename, true);
		if (GetMessageFromDb(hContact, clientMsgId, composeTime) == NULL)
			PushRequest(new GetHistoryRequest(ptrA(getStringA("registrationToken")), skypename, ptrA(getStringA("Server"))), &CSkypeProto::OnGetServerHistory);
	}
}