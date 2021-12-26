#include "commonheaders.h"

void HistoryLog(MCONTACT hContact, LPCSTR szText)
{
	DBEVENTINFO dbei = {};
	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.flags = DBEF_SENT | DBEF_READ;
	dbei.timestamp = time(0);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)mir_strlen(szText) + 1;
	dbei.pBlob = (uint8_t*)szText;
	db_event_add(0, &dbei);
}
