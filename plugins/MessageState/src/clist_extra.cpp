#include "stdafx.h"

HANDLE hExtraIcon = NULL;

bool HasUnread(MCONTACT hContact)
{
	const char *szProto = GetContactProto(hContact);
	if (CheckProtoSupport(szProto))
	{
		return ((GetLastSentMessageTime(hContact) > db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0)) && db_get_dw(hContact, MODULENAME, DBKEY_MESSAGE_READ_TIME, 0) != 0);
	}

	return false;
}

int ExtraIconsApply(WPARAM hContact, LPARAM)
{
	if (hContact == NULL) return 0;

	if (HasUnread(hContact))
		ExtraIcon_SetIconByName(hExtraIcon, hContact, "clist_unread_icon");
	else
		ExtraIcon_Clear(hExtraIcon, hContact);

	return 0;
}

void InitClistExtraIcon()
{
	hExtraIcon = ExtraIcon_RegisterIcolib("messagestate_unread", LPGEN("MessageState unread extra icon"), "clist_unread_icon");
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ExtraIconsApply);
	
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		ExtraIconsApply(hContact, 0);
}