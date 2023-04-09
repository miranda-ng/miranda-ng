#include "stdafx.h"

INT_PTR IsContactPassed(WPARAM hContact, LPARAM /*lParam*/)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return CS_PASSED;

	if (!g_plugin.ProtoDisabled(szProto))
		return CS_PASSED;

	if (g_plugin.getByte(hContact, DB_KEY_ANSWERED))
		return CS_PASSED;

	if (Contact::OnList(hContact) && db_get_w(hContact, szProto, "SrvGroupId", -1) != 1)
		return CS_PASSED;

	if (IsExistMyMessage(hContact))
		return CS_PASSED;

	return CS_NOTPASSED;
}
