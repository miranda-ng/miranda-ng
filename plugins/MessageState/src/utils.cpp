#include "stdafx.h"

time_t GetLastSentMessageTime(MCONTACT hContact)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.flags & DBEF_SENT)
			return dbei.timestamp;
	}
	return -1;
}

bool HasUnread(MCONTACT hContact)
{
	if (!CheckProtoSupport(Proto_GetBaseAccountName(hContact)))
		return false;
	
	if (GetLastSentMessageTime(hContact) <= g_plugin.getDword(hContact, DBKEY_MESSAGE_READ_TIME, 0))
		return false;
	
	return g_plugin.getDword(hContact, DBKEY_MESSAGE_READ_TIME, 0) != 0;
}
