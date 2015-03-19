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

MCONTACT CSkypeProto::GetContact(const char *skypename)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA cSkypename(getStringA(hContact, SKYPE_SETTINGS_ID));
		if (mir_strcmpi(skypename, cSkypename) == 0)
		{
			break;
		}
	}
	return hContact;
}

MCONTACT CSkypeProto::AddContact(const char *skypename, bool isTemporary)
{
	MCONTACT hContact = GetContact(skypename);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		setString(hContact, SKYPE_SETTINGS_ID, skypename);

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

//[{ "username":"echo123", "firstname" : "Echo \/ Sound Test Service", "lastname" : null, "avatarUrl" : null, "mood" : null, "richMood" : null, "displayname" : null, "country" : null, "city" : null },...]
void CSkypeProto::LoadProfiles(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	JSONNODE *items = json_as_array(root), *item, *node;
	for (size_t i = 0; i < json_size(items); i++)
	{
		item = json_at(items, i);
		if (item == NULL)
			break;

		node = json_get(item, "username");
		ptrA skypename(mir_t2a(ptrT(json_as_string(node))));
		MCONTACT hContact = AddContact(skypename);
		if (hContact)
		{
			node = json_get(item, "fullname");
			CMString realname = ptrT(json_as_string(node));
			if (!realname.IsEmpty() && realname != "null")
			{
				size_t pos = realname.Find(' ', 1);
				if (mir_strcmpi(skypename, "echo123") != 0 && pos != -1)
				{
					setTString(hContact, "FirstName", realname.Mid(0, pos));
					setTString(hContact, "LastName", realname.Mid(pos + 1));
				}
				else
				{
					setTString(hContact, "FirstName", realname);
					delSetting(hContact, "LastName");
				}
			}
			else
			{
				delSetting(hContact, "FirstName");
				delSetting(hContact, "LastName");
			}

			node = json_get(item, "display_name");
			CMString nick = ptrT(json_as_string(node));
			if (!nick.IsEmpty() && nick != "null")
				setTString(hContact, "Nick", nick);
			else
				delSetting(hContact, "Nick");

			node = json_get(item, "avatarUrl");
			CMStringA avatarUrl = mir_t2a(ptrT(json_as_string(node)));
			if (avatarUrl && avatarUrl != "null")
				; // TODO: load avatar

			node = json_get(item, "mood");
			CMString mood = ptrT(json_as_string(node));
			if (!mood.IsEmpty() && mood != "null")
				db_set_ts(hContact, "CList", "StatusMsg", mood);
			else
				db_unset(hContact, "CList", "StatusMsg");

			node = json_get(item, "richMood");
			ptrT richMood(json_as_string(node));

			node = json_get(item, "country");
			ptrA country(mir_t2a(ptrT(json_as_string(node))));

			node = json_get(item, "city");
			CMString city = ptrT(json_as_string(node));
			if (!city.IsEmpty() && city != "null")
				setTString(hContact, "City", city);
			else
				delSetting(hContact, "City");
		}

	}
	json_delete(items);
}

//[{"skypename":"echo123", "authorized" : true, "blocked" : false, ...},...]
// other properties is exists but empty
void CSkypeProto::LoadContacts(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONROOT root(response->pData);
	if (root == NULL)
		return;

	LIST<char> skypenames(1);

	JSONNODE *items = json_as_array(root), *item, *node;
	for (size_t i = 0; i < json_size(items); i++)
	{
		item = json_at(items, i);
		if (item == NULL)
			break;

		node = json_get(item, "skypename");
		ptrA skypename(mir_t2a(ptrT(json_as_string(node))));
		MCONTACT hContact = AddContact(skypename);
		if (hContact)
		{
			node = json_get(item, "authorized");
			if (json_as_bool(node))
			{
				delSetting(hContact, "Auth");
				delSetting(hContact, "Grant");
			}
			else
			{
				setByte(hContact, "Grant", 1);
			}
			
			node = json_get(item, "blocked");
			setByte(hContact, "IsBlocked", json_as_bool(node));

			skypenames.insert(mir_strdup(skypename));
		}
	}
	json_delete(items);

	if (skypenames.getCount() > 0)
	{
		ptrA token(getStringA("TokenSecret"));
		PushRequest(new GetProfilesRequest(token, skypenames), &CSkypeProto::LoadProfiles);

		for (size_t i = 0; i < skypenames.getCount(); i++)
		{
			mir_free(skypenames[i]);
		}
		skypenames.destroy();
	}
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