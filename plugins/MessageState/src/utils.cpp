#include "stdafx.h"

LONGLONG GetLastSentMessageTime(MCONTACT hContact)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent))
	{
		DBEVENTINFO dbei = { sizeof(dbei) };
		db_event_get(hDbEvent, &dbei);
		if (FLAG_CONTAINS(dbei.flags, DBEF_SENT))
			return dbei.timestamp;
	}
	return -1;
}

bool HasUnread(MCONTACT hContact)
{
	const char *szProto = GetContactProto(hContact);
	if (CheckProtoSupport(szProto))
	{
		return ((GetLastSentMessageTime(hContact) > db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0)) && db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0) != 0);
	}

	return false;
}
