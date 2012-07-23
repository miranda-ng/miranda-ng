#include "commonheaders.h"


void HistoryLog(HANDLE hContact, LPCSTR szText) {

	DBEVENTINFO dbei;
	memset(&dbei, 0, sizeof(dbei));

	dbei.cbSize = sizeof(dbei);
	dbei.szModule = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	dbei.flags = DBEF_SENT|DBEF_READ;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)strlen(szText) + 1;
	dbei.pBlob = (PBYTE)szText;

	CallService(MS_DB_EVENT_ADD, (WPARAM)0, (LPARAM)&dbei);
}


// EOF
