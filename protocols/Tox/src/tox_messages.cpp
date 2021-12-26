#include "stdafx.h"

void CToxProto::InitCustomDbEvents()
{
	DBEVENTTYPEDESCR dbEventType = {};
	dbEventType.module = m_szModuleName;
	dbEventType.flags = DETF_HISTORY | DETF_MSGWINDOW;

	dbEventType.eventType = DB_EVENT_ACTION;
	dbEventType.descr = Translate("Action");
	DbEvent_RegisterType(&dbEventType);
}

INT_PTR CToxProto::EventGetIcon(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO*)lParam;
	HICON icon = nullptr;

	switch (dbei->eventType) {
	case DB_EVENT_ACTION:
		icon = g_plugin.getIcon(IDI_ME);
		break;

	default:
		icon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		break;
	}

	return (INT_PTR)((wParam & LR_SHARED)
		? icon
		: CopyIcon(icon));
}

/* MESSAGE RECEIVING */

// incoming message flow
void CToxProto::OnFriendMessage(Tox *tox, uint32_t friendNumber, TOX_MESSAGE_TYPE type, const uint8_t *message, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (hContact == NULL)
		return;

	char *rawMessage = (char*)mir_alloc(length + 1);
	// old api support
	if (message[0] == 0 && length > 0) {
		length -= 3;
		mir_strncpy(rawMessage, (const char*)&message[4], length);
	}
	else mir_strncpy(rawMessage, (const char*)message, length + 1);
	rawMessage[length] = 0;

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = now();
	recv.szMessage = rawMessage;
	switch (type) {
	case TOX_MESSAGE_TYPE_NORMAL:
		recv.lParam = EVENTTYPE_MESSAGE;
		break;
	case TOX_MESSAGE_TYPE_ACTION:
		recv.lParam = DB_EVENT_ACTION;
		break;
	}
	ProtoChainRecvMsg(hContact, &recv);

	CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
}

/* MESSAGE SENDING */

// outcoming message flow
struct SendMessageParam
{
	MCONTACT hContact;
	UINT hMessage;
	char *message;
};

void CToxProto::SendMessageAsync(void *arg)
{
	Thread_SetName(MODULE ": SendMessageThread");

	SendMessageParam *param = (SendMessageParam*)arg;

	int32_t friendNumber = GetToxFriendNumber(param->hContact);
	if (friendNumber == UINT32_MAX)
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)param->hMessage, (LPARAM)ToxErrorToString(TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND));

	size_t msgLen = mir_strlen(param->message);
	uint8_t *msg = (uint8_t*)param->message;
	TOX_MESSAGE_TYPE type = TOX_MESSAGE_TYPE_NORMAL;
	if (strncmp(param->message, "/me ", 4) == 0) {
		msg += 4; msgLen -= 4;
		type = TOX_MESSAGE_TYPE_ACTION;
	}

	TOX_ERR_FRIEND_SEND_MESSAGE sendError;
	int messageNumber = tox_friend_send_message(m_tox, friendNumber, type, msg, msgLen, &sendError);
	if (sendError != TOX_ERR_FRIEND_SEND_MESSAGE_OK) {
		debugLogA(__FUNCTION__": failed to send message for %d (%d)", friendNumber, sendError);
		ProtoBroadcastAck(param->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)param->hMessage, (LPARAM)ToxErrorToString(sendError));
	}
	uint64_t messageId = (((int64_t)friendNumber) << 32) | ((int64_t)messageNumber);
	messages[messageId] = param->hMessage;

	mir_free(param->message);
	mir_free(param);
}

int CToxProto::OnSendMessage(MCONTACT hContact, const char *szMessage)
{
	if (!IsOnline()) {
		ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, nullptr, (LPARAM)TranslateT("You cannot send when you are offline."));
		return 0;
	}

	UINT hMessage = InterlockedIncrement(&hMessageProcess);

	SendMessageParam *param = (SendMessageParam*)mir_calloc(sizeof(SendMessageParam));
	param->hContact = hContact;
	param->hMessage = hMessage;
	param->message = mir_strdup(szMessage);

	ForkThread(&CToxProto::SendMessageAsync, param);

	return hMessage;
}

// message is received by the other side
void CToxProto::OnReadReceipt(Tox *tox, uint32_t friendNumber, uint32_t messageNumber, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (hContact == NULL)
		return;

	uint64_t messageId = (((int64_t)friendNumber) << 32) | ((int64_t)messageNumber);
	UINT hMessage = proto->messages[messageId];
	proto->ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)hMessage, 0);
}

// preparing message/action to writing into db
int CToxProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(Proto_GetBaseAccountName(evt->hContact), m_szModuleName))
		return 0;

	char *message = (char*)evt->dbei->pBlob;
	if (strncmp(message, "/me ", 4) != 0)
		return 0;

	evt->dbei->cbBlob = evt->dbei->cbBlob - 4;
	uint8_t *action = (uint8_t*)mir_alloc(evt->dbei->cbBlob);
	memcpy(action, &evt->dbei->pBlob[4], evt->dbei->cbBlob);
	mir_free(evt->dbei->pBlob);
	evt->dbei->pBlob = action;
	evt->dbei->eventType = DB_EVENT_ACTION;

	return 0;
}

/* STATUS MESSAGE */
void CToxProto::GetStatusMessageAsync(void* arg)
{
	Thread_SetName(MODULE ": GetStatusMessageThread");

	MCONTACT hContact = (UINT_PTR)arg;

	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX) {
		ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_FAILED, (HANDLE)hContact, 0);
		return;
	}

	TOX_ERR_FRIEND_QUERY error;
	size_t size = tox_friend_get_status_message_size(m_tox, friendNumber, &error);
	if (error != TOX_ERR_FRIEND_QUERY::TOX_ERR_FRIEND_QUERY_OK) {
		debugLogA(__FUNCTION__": failed to get status message for (%d) (%d)", friendNumber, error);
		ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_FAILED, (HANDLE)hContact, 0);
		return;
	}

	ptrA statusMessage((char*)mir_calloc(size + 1));
	if (!tox_friend_get_status_message(m_tox, friendNumber, (uint8_t*)(char*)statusMessage, &error)) {
		debugLogA(__FUNCTION__": failed to get status message for (%d) (%d)", friendNumber, error);
		ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_FAILED, (HANDLE)hContact, 0);
		return;
	}

	ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)hContact, ptrW(mir_utf8decodeW(statusMessage)));
}

/* TYPING */

int CToxProto::OnUserIsTyping(MCONTACT hContact, int type)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX)
		return 0;

	TOX_ERR_SET_TYPING error;
	if (!tox_self_set_typing(m_tox, friendNumber, type == PROTOTYPE_SELFTYPING_ON, &error))
		debugLogA(__FUNCTION__": failed to send typing (%d)", error);

	return 0;
}

void CToxProto::OnTypingChanged(Tox *tox, uint32_t friendNumber, bool isTyping, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	if (MCONTACT hContact = proto->GetContact(tox, friendNumber)) {
		int typingStatus = (isTyping ? PROTOTYPE_CONTACTTYPING_INFINITE : PROTOTYPE_CONTACTTYPING_OFF);
		CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)typingStatus);
	}
}
