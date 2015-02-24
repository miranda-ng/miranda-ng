#include "common.h"

void CToxProto::RegisterIncomingMessage(const int friendNumber, const uint8_t *message, const uint16_t messageSize)
{
	MCONTACT hContact = GetContact(friendNumber);
	if (hContact)
	{
		ptrA szMessage((char*)mir_alloc(messageSize + 1));
		mir_strncpy(szMessage, (const char*)message, messageSize + 1);

		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.timestamp = time(NULL);
		recv.szMessage = szMessage;

		ProtoChainRecvMsg(hContact, &recv);
	}
}

void CToxProto::OnFriendMessage(Tox*, const int friendNumber, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;
	proto->RegisterIncomingMessage(friendNumber, message, messageSize);
}

void CToxProto::OnFriendAction(Tox*, const int friendNumber, const uint8_t *action, const uint16_t actionSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;
	proto->RegisterIncomingMessage(friendNumber, action, actionSize);
}

int CToxProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber != TOX_ERROR)
	{
		int receipt = 0;
		if (strncmp(msg, "/me ", 4) != 0)
		{
			receipt = tox_send_message(tox, friendNumber, (uint8_t*)msg, mir_strlen(msg));
		}
		else
		{
			receipt = tox_send_action(tox, friendNumber, (uint8_t*)&msg[4], mir_strlen(msg) - 4);
		}
		if (receipt == TOX_ERROR)
		{
			debugLogA("CToxProto::SendMsg: failed to send message");
		}
		return receipt;
	}
	return 0;
}

void CToxProto::OnReadReceipt(Tox*, int32_t friendNumber, uint32_t receipt, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		proto->ProtoBroadcastAck(
			hContact,
			ACKTYPE_MESSAGE,
			ACKRESULT_SUCCESS,
			(HANDLE)receipt, 0);
	}
}

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