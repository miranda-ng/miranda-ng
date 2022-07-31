#include "stdafx.h"

INT_PTR IsContactPassed(WPARAM hContact, LPARAM /*lParam*/)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return CS_PASSED;

	if (!g_sets.ProtoDisabled(szProto))
		return CS_PASSED;

	if (g_plugin.getByte(hContact, DB_KEY_ANSWERED))
		return CS_PASSED;

	if (Contact::OnList(hContact) && db_get_w(hContact, szProto, "SrvGroupId", -1) != 1)
		return CS_PASSED;

	if (IsExistMyMessage(hContact))
		return CS_PASSED;

	return CS_NOTPASSED;
}

INT_PTR RemoveTempContacts(WPARAM, LPARAM lParam)
{
	for (MCONTACT hContact = db_find_first(); hContact;) {
		MCONTACT hNext = db_find_next(hContact);
		ptrW szGroup(Clist_GetGroup(hContact));

		if (!Contact::OnList(hContact) || (szGroup != NULL && (wcsstr(szGroup, L"Not In List") || wcsstr(szGroup, TranslateT("Not In List"))))) {
			char *szProto = Proto_GetBaseAccountName(hContact);
			if (szProto != nullptr) {
				// Check if protocol uses server side lists
				uint32_t caps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
				if (caps & PF1_SERVERCLIST) {
					int status = Proto_GetStatus(szProto);
					if (status == ID_STATUS_OFFLINE || IsStatusConnecting(status))
						// Set a flag so we remember to delete the contact when the protocol goes online the next time
						db_set_b(hContact, "CList", "Delete", 1);
					else
						db_delete_contact(hContact);
				}
			}
		}

		hContact = hNext;
	}

	int hGroup = 1;
	wchar_t *group_name;
	do {
		group_name = Clist_GroupGetName(hGroup, nullptr);
		if (group_name != nullptr && wcsstr(group_name, TranslateT("Not In List"))) {
			uint8_t ConfirmDelete = Clist::ConfirmDelete;
			if (ConfirmDelete)
				db_set_b(0, "CList", "ConfirmDelete", 0);

			Clist_GroupDelete(hGroup);
			if (ConfirmDelete)
				db_set_b(0, "CList", "ConfirmDelete", ConfirmDelete);
			break;
		}
		hGroup++;
	} while (group_name);
	if (!lParam)
		MessageBox(nullptr, TranslateT("Complete"), TranslateT(MODULENAME), MB_ICONINFORMATION);

	return 0;
}

int OnSystemModulesLoaded(WPARAM, LPARAM)
{
	if (g_sets.RemTmpAll)
		RemoveTempContacts(0, 1);
	return 0;
}
