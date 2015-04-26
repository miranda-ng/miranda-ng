#include "stdafx.h"

/* MESSAGE RECEIVING */

// incoming message flow
void CToxProto::OnFriendMessage(Tox*, uint32_t friendNumber, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
		return;

	char *rawMessage = (char*)mir_alloc(length + 1);
	// old api support
	if (message[0] == 0 && length > 0)
	{
		length -= 3;
		mir_strncpy(rawMessage, (const char*)&message[4], length);
	}
	else
		mir_strncpy(rawMessage, (const char*)message, length + 1);
	rawMessage[length] = 0;

	PROTORECVEVENT recv = { 0 };
	recv.flags = PREF_UTF;
	recv.timestamp = time(NULL);
	recv.szMessage = rawMessage;
	recv.lParam = type == TOX_MESSAGE_TYPE_NORMAL
		? EVENTTYPE_MESSAGE : DB_EVENT_ACTION;

	ProtoChainRecvMsg(hContact, &recv);

	CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
}

// writing message/even into db
int CToxProto::OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre)
{
	//return Proto_RecvMessage(hContact, pre);
	if (pre->szMessage == NULL)
		return NULL;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = GetContactProto(hContact);
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF;
	dbei.eventType = pre->lParam;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	return (INT_PTR)db_event_add(hContact, &dbei);
}

/* MESSAGE SENDING */

// outcoming message flow
int CToxProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX)
		return 0;

	ptrA message;
	if (flags & PREF_UNICODE)
		message = mir_utf8encodeW((wchar_t*)&szMessage[mir_strlen(szMessage) + 1]);
	else //if (flags & PREF_UTF)
		message = mir_strdup(szMessage);
	//else
		//message = mir_utf8encode(szMessage);

	size_t msgLen = mir_strlen(message);
	uint8_t *msg = (uint8_t*)(char*)message;
	TOX_MESSAGE_TYPE type = TOX_MESSAGE_TYPE_NORMAL;
	if (strncmp(message, "/me ", 4) == 0)
	{
		msg += 4; msgLen -= 4;
		type = TOX_MESSAGE_TYPE_ACTION;
	}
	TOX_ERR_FRIEND_SEND_MESSAGE error;
	int messageId = tox_friend_send_message(tox, friendNumber, type, msg, msgLen, &error);
	if (error != TOX_ERR_FRIEND_SEND_MESSAGE_OK)
	{
		debugLogA(__FUNCTION__": failed to send message (%d)", error);
		return 0;
	}

	return messageId;
}

// message is received by the other side
void CToxProto::OnReadReceipt(Tox*, uint32_t friendNumber, uint32_t messageId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
		return;

	proto->ProtoBroadcastAck(
		hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)messageId, 0);
}

// preparing message/action to writing into db
int CToxProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) != 0)
		return 0;

	evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
	PBYTE action = (PBYTE)mir_alloc(evt->dbei->cbBlob);
	memcpy(action, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
	mir_free(evt->dbei->pBlob);
	evt->dbei->pBlob = action;
	evt->dbei->eventType = DB_EVENT_ACTION;

	return 0;
}

/* TYPING */

int CToxProto::OnUserIsTyping(MCONTACT hContact, int type)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX)
		return 0;

	TOX_ERR_SET_TYPING error;
	if (!tox_self_set_typing(tox, friendNumber, type == PROTOTYPE_SELFTYPING_ON, &error))
		debugLogA(__FUNCTION__": failed to send typing (%d)", error);

	return 0;
}

void CToxProto::OnTypingChanged(Tox*, uint32_t friendNumber, bool isTyping, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	if (MCONTACT hContact = proto->GetContact(friendNumber))
	{
		int typingStatus = (isTyping ? PROTOTYPE_CONTACTTYPING_INFINITE : PROTOTYPE_CONTACTTYPING_OFF);
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)typingStatus);
	}
}