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

void CToxProto::OnTypingChanged(Tox *tox, const int number, uint8_t isTyping, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)isTyping);
	}
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