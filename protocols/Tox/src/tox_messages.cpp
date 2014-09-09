#include "common.h"

void CToxProto::OnFriendMessage(Tox *tox, const int number, const uint8_t *message, const uint16_t messageSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		PROTORECVEVENT recv = { 0 };
		recv.flags = PREF_UTF;
		recv.timestamp = time(NULL);
		recv.szMessage = (char*)message;

		ProtoChainRecvMsg(hContact, &recv);
	}
}

void CToxProto::OnFriendAction(Tox *tox, const int number, const uint8_t *action, const uint16_t actionSize, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		proto->AddDbEvent(
			hContact,
			TOX_DB_EVENT_TYPE_ACTION,
			time(NULL),
			DBEF_UTF,
			actionSize,
			(BYTE*)action);
	}
}

int __cdecl CToxProto::SendMsg(MCONTACT hContact, int flags, const char* msg)
{
	DBVARIANT dbv;
	std::vector<uint8_t> id(TOX_CLIENT_ID_SIZE);
	if (!db_get(hContact, m_szModuleName, TOX_SETTINGS_ID, &dbv))
	{
		memcpy(&id[0], dbv.pbVal, TOX_CLIENT_ID_SIZE);
		db_free(&dbv);
	}

	uint32_t number = tox_get_friend_number(tox, id.data());
	if (number == TOX_ERROR)
	{
		debugLogA("CToxProto::SendMsg: failed to get friend number");
		return 0;
	}

	int result = 0;
	if (strncmp(msg, "/me ", 4) != 0)
	{
		result = tox_send_message(tox, number, (uint8_t*)msg, (uint16_t)strlen(msg));
	}
	else
	{
		result = tox_send_action(tox, number, (uint8_t*)&msg[4], (uint16_t)strlen(msg) - 4);
	}
	if (result == 0)
	{
		debugLogA("CToxProto::SendMsg: could not to send message");
	}

	return result;
}

void CToxProto::OnReadReceipt(Tox *tox, int32_t number, uint32_t receipt, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		proto->ProtoBroadcastAck(
			hContact,
			ACKTYPE_MESSAGE,
			ACKRESULT_SUCCESS,
			(HANDLE)receipt, 0);
	}
}

int CToxProto::OnPreCreateMessage(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;
	if (strcmp(GetContactProto(evt->hContact), m_szModuleName))
	{
		return 0;
	}

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) == 0)
	{
		BYTE *action = (BYTE*)mir_alloc(sizeof(BYTE)* (evt->dbei->cbBlob - 4));
		memcpy(action, (char*)&evt->dbei->pBlob[4], evt->dbei->cbBlob - 4);
		mir_free(evt->dbei->pBlob);
		evt->dbei->pBlob = action;
		evt->dbei->cbBlob -= 4;

		evt->dbei->eventType = TOX_DB_EVENT_TYPE_ACTION;
	}

	return 1;
}

void CToxProto::OnTypingChanged(Tox *tox, const int number, uint8_t isTyping, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)isTyping);
	}
}

int __cdecl CToxProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && IsOnline())
	{
		DBVARIANT dbv;
		std::vector<uint8_t> id(TOX_CLIENT_ID_SIZE);
		if (!db_get(hContact, m_szModuleName, TOX_SETTINGS_ID, &dbv))
		{
			memcpy(&id[0], dbv.pbVal, TOX_CLIENT_ID_SIZE);
			db_free(&dbv);
		}
		uint32_t number = tox_get_friend_number(tox, id.data());
		if (number >= 0)
		{
			tox_set_user_is_typing(tox, number, type);
			return 0;
		}
	}

	return 1;
}