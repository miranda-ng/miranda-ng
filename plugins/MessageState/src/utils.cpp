#include "stdafx.h"

static mir_cs csContacts;
OBJLIST<ContactData> g_arContacts(50, NumericKeySortT);

// always returns an object, creates an empty one if missing
ContactData* FindContact(MCONTACT hContact)
{
	mir_cslock lck(csContacts);

	auto *p = g_arContacts.find((ContactData *)&hContact);
	if (p == nullptr)
		g_arContacts.insert(p = new ContactData(hContact));

	return p;
}

bool HasUnread(MCONTACT hContact)
{
	if (!CheckProtoSupport(Proto_GetBaseAccountName(hContact)))
		return false;
	
	auto *p = FindContact(hContact);
	if (p->dwLastSentTime <= p->dwLastReadTime)
		return false;
	
	return p->dwLastReadTime != 0;
}
