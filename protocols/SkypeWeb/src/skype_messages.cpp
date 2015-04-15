/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

MEVENT CSkypeProto::GetMessageFromDb(MCONTACT hContact, const char *messageId, LONGLONG timestamp)
{
	if(messageId == NULL)
		return NULL;
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
	debugLogA("Incoming message from %s", skypename);

	MCONTACT hContact = AddContact(skypename, true);
	if (hContact == NULL)
		return 0;

	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = timestamp;
	recv.szMessage = content;
	recv.lParam = emoteOffset;
	recv.pCustomData = (void*)messageId;
	recv.cbCustomDataSize = mir_strlen(messageId);

	if (isRead)
		recv.flags |= PREF_CREATEREAD;

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

	if (response == NULL || (response->resultCode != 200 && response->resultCode != 201))
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

/* MESSAGE EVENT */

void CSkypeProto::OnPrivateMessageEvent(JSONNODE *node)
{
	ptrA clientMsgId(mir_t2a(ptrT(json_as_string(json_get(node, "clientmessageid")))));
	ptrA skypeEditedId(mir_t2a(ptrT(json_as_string(json_get(node, "skypeeditedid")))));	

	bool isEdited = (json_get(node, "skypeeditedid") != NULL);

	ptrT composeTime(json_as_string(json_get(node, "composetime")));
	time_t timestamp = getByte("UseLocalTime", 0) ? time(NULL) : IsoToUnixTime(composeTime);

	ptrA from(mir_t2a(ptrT(json_as_string(json_get(node, "from")))));
	ptrA skypename(ContactUrlToName(from));

	ptrA content(mir_t2a(ptrT(json_as_string(json_get(node, "content")))));
	int emoteOffset = json_as_int(json_get(node, "skypeemoteoffset"));

	ptrA message(RemoveHtml(content));

	if (IsMe(skypename))
	{
		ptrA conversationLink(mir_t2a(ptrT(json_as_string(json_get(node, "conversationLink")))));
		ptrA cSkypename(ContactUrlToName(conversationLink));
		MCONTACT hContact = AddContact(cSkypename, true);
		int hMessage = atoi(clientMsgId);
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)hMessage, 0);
		debugLogA(__FUNCTION__" timestamp = %d clientmsgid = %s", timestamp, clientMsgId);
		AddMessageToDb(hContact, timestamp, DBEF_UTF | DBEF_SENT, clientMsgId, message, emoteOffset);
	}
	else
	{
		ptrA messageType(mir_t2a(ptrT(json_as_string(json_get(node, "messagetype")))));

		MCONTACT hContact = AddContact(skypename, true);

		if (!mir_strcmpi(messageType, "Control/Typing"))
			CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_INFINITE);

		else if (!mir_strcmpi(messageType, "Control/ClearTyping"))
			CallService(MS_PROTO_CONTACTISTYPING, hContact, PROTOTYPE_CONTACTTYPING_OFF);

		else if (!mir_strcmpi(messageType, "Text") || !mir_strcmpi(messageType, "RichText"))
		{
			debugLogA(__FUNCTION__" timestamp = %d clientmsgid = %s", timestamp, clientMsgId);
			MEVENT dbevent =  GetMessageFromDb(hContact, skypeEditedId);
			if (isEdited && dbevent != NULL)
			{
				DBEVENTINFO dbei = { sizeof(dbei) };
				db_event_get(dbevent, &dbei);
				time_t dbEventTimestamp = dbei.timestamp;
				if (!getByte("SaveEditedMessage", 0))
				{
					db_event_delete(hContact, dbevent);
				}
				OnReceiveMessage(clientMsgId, from, dbEventTimestamp + 1, message, emoteOffset);
			}
			else
				OnReceiveMessage(clientMsgId, from, timestamp, message, emoteOffset);
		}
		else if (!mir_strcmpi(messageType, "Event/SkypeVideoMessage"))
			return; //not supported
	}
}

int CSkypeProto::OnDbEventRead(WPARAM, LPARAM hDbEvent)
{
	debugLogA(__FUNCTION__);
	if (IsOnline())
		MarkMessagesRead(hDbEvent);
	return 0;
}

void CSkypeProto::MarkMessagesRead(MEVENT hDbEvent)
{
	debugLogA(__FUNCTION__);
	DBEVENTINFO dbei = { sizeof(dbei) };
	db_event_get(hDbEvent, &dbei);
	time_t timestamp = dbei.timestamp;
	PushRequest(new MarkMessageReadRequest(ptrA(getStringA("registrationToken")), timestamp, ptrA(getStringA("Server"))));
}