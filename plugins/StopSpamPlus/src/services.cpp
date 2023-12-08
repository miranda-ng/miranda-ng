#include "stdafx.h"

INT_PTR IsContactPassed(WPARAM hContact, LPARAM /*lParam*/)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return CS_PASSED;

	if (!g_plugin.ProtoDisabled(szProto))
		return CS_PASSED;

	if (g_plugin.getByte(hContact, DB_KEY_ANSWERED) || g_plugin.getByte(hContact, DB_KEY_HASSENT))
		return CS_PASSED;

	if (Contact::OnList(hContact))
		return CS_PASSED;

	return CS_NOTPASSED;
}
