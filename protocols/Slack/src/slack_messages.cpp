#include "stdafx.h"

/* MESSAGE RECEIVING */

// incoming message flow


// writing message/even into db
int CSlackProto::OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre)
{
	//return Proto_RecvMessage(hContact, pre);
	if (pre->szMessage == NULL)
		return NULL;

	DBEVENTINFO dbei = {};
	dbei.szModule = GetContactProto(hContact);
	dbei.timestamp = pre->timestamp;
	dbei.flags = 0;
	dbei.eventType = pre->lParam;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	return (INT_PTR)db_event_add(hContact, &dbei);
}

/* MESSAGE SENDING */

// outcoming message flow
int CSlackProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	return 0;
}

// message is received by the other side

// preparing message/action to writing into db
int CSlackProto::OnPreCreateMessage(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent*)lParam;
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;
}

/* TYPING */

int CSlackProto::OnUserIsTyping(MCONTACT hContact, int type)
{
	return 0;
}