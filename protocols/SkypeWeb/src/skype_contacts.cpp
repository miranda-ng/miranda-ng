#include "common.h"

WORD CSkypeProto::GetContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

void CSkypeProto::SetContactStatus(MCONTACT hContact, WORD status)
{
	WORD oldStatus = GetContactStatus(hContact);
	if (oldStatus != status)
	{
		setWord(hContact, "Status", status);
	}
}

void CSkypeProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		SetContactStatus(hContact, status);
	}
}

MCONTACT CSkypeProto::GetContactFromAuthEvent(MEVENT hEvent)
{
	DWORD body[3];
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

MCONTACT CSkypeProto::GetContact(const char *login)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA contactLogin(getStringA(hContact, SKYPE_SETTINGS_ID));
		if (mir_strcmpi(login, contactLogin) == 0)
		{
			break;
		}
	}
	return hContact;
}

MCONTACT CSkypeProto::AddContact(const char *login, bool isTemporary)
{
	MCONTACT hContact = GetContact(login);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, SKYPE_SETTINGS_ID, login);

		DBVARIANT dbv;
		if (!getTString(SKYPE_SETTINGS_GROUP, &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}

		setByte(hContact, "Auth", 1);
		setByte(hContact, "Grant", 1);

		if (isTemporary)
		{
			db_set_b(hContact, "CList", "NotOnList", 1);
		}
	}
	return hContact;
}

void CSkypeProto::LoadFriendList(void*)
{
}

INT_PTR CSkypeProto::OnRequestAuth(WPARAM hContact, LPARAM lParam)
{
	return 0;
}

INT_PTR CSkypeProto::OnGrantAuth(WPARAM hContact, LPARAM)
{
	return 0;
}

int CSkypeProto::OnContactDeleted(MCONTACT hContact, LPARAM)
{
	return 0;
}