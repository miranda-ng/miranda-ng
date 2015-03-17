#include "common.h"

/* MESSAGE RECEIVING */

// writing message/even into db
int CSkypeProto::OnReceiveMessage(MCONTACT hContact, PROTORECVEVENT *pre)
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
int CSkypeProto::OnSendMessage(MCONTACT hContact, int flags, const char *szMessage)
{
	return 0;
}