#include "common.h"

/* MESSAGE RECEIVING */

// incoming message flow
void CToxProto::OnFriendMessage(Tox*, const int friendNumber, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		ptrA szMessage((char*)mir_alloc(messageSize + 1));
		mir_strncpy(szMessage, (const char*)message, messageSize + 1);

		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.timestamp = time(NULL);
		recv.szMessage = szMessage;
		recv.lParam = EVENTTYPE_MESSAGE;

		ProtoChainRecvMsg(hContact, &recv);
	}
}

// incoming action flow
void CToxProto::OnFriendAction(Tox*, const int friendNumber, const uint8_t *action, const uint16_t actionSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		ptrA szMessage((char*)mir_alloc(actionSize + 1));
		mir_strncpy(szMessage, (const char*)action, actionSize + 1);

		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.timestamp = time(NULL);
		recv.szMessage = szMessage;
		recv.lParam = TOX_DB_EVENT_TYPE_ACTION;

		ProtoChainRecvMsg(hContact, &recv);
	}
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
	dbei.cbBlob = (DWORD)strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	return (INT_PTR)db_event_add(hContact, &dbei);
}

/* MESSAGE SENDING */

// outcoming message flow
int CToxProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == TOX_ERROR)
	{
		return 0;
	}

	ptrA message;
	if (flags & PREF_UNICODE)
	{
		message = mir_utf8encodeW((wchar_t*)&szMessage[mir_strlen(szMessage) + 1]);
	}
	else if (flags & PREF_UTF)
	{
		message = mir_strdup(szMessage);
	}
	else
	{
		message = mir_utf8encode(szMessage);
	}

	int receipt = 0;
	if (strncmp(message, "/me ", 4) != 0)
	{
		receipt = tox_send_message(tox, friendNumber, (uint8_t*)(char*)message, mir_strlen(message));
	}
	else
	{
		receipt = tox_send_action(tox, friendNumber, (uint8_t*)&message[4], mir_strlen(message) - 4);
	}
	if (receipt == TOX_ERROR)
	{
		debugLogA("CToxProto::OnSendMessage: failed to send message");
	}
	return receipt;
}

// message is received by the other side
void CToxProto::OnReadReceipt(Tox*, int32_t friendNumber, uint32_t receipt, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		proto->ProtoBroadcastAck(
			hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)receipt, 0);
	}
}

// preparing message/action to writing into db
int CToxProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (strcmp(GetContactProto(evt->hContact), m_szModuleName))
	{
		return 0;
	}

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0)
	{
		evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
		PBYTE action = (PBYTE)mir_alloc(evt->dbei->cbBlob);
		memcpy(action, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
		mir_free(evt->dbei->pBlob);
		evt->dbei->pBlob = action;
		evt->dbei->eventType = TOX_DB_EVENT_TYPE_ACTION;
	}

	return 1;
}

/* TYPING */

void CToxProto::OnTypingChanged(Tox*, const int number, uint8_t isTyping, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(number);
	if (hContact)
	{
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)isTyping);
	}
}

int CToxProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && IsOnline())
	{
		int32_t friendNumber = GetToxFriendNumber(hContact);
		if (friendNumber >= 0)
		{
			tox_set_user_is_typing(tox, friendNumber, type);
			return 0;
		}
	}

	return 1;
}