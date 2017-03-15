#include "stdafx.h"

WORD CSlackProto::GetContactStatus(MCONTACT hContact)
{
	return getWord(hContact, "Status", ID_STATUS_OFFLINE);
}

void CSlackProto::SetContactStatus(MCONTACT hContact, WORD status)
{
	WORD oldStatus = GetContactStatus(hContact);
	if (oldStatus != status)
		setWord(hContact, "Status", status);
}

void CSlackProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
		SetContactStatus(hContact, status);
}

MCONTACT CSlackProto::GetContact(const char *userId)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA contactPhone(getStringA(hContact, "UserId"));
		if (mir_strcmp(userId, contactPhone) == 0)
			break;
	}
	return hContact;
}

MCONTACT CSlackProto::AddContact(const char *userId, const char *nick, bool isTemporary)
{
	MCONTACT hContact = GetContact(userId);
	if (!hContact)
	{
		hContact = db_add_contact();
		Proto_AddToContact(hContact, m_szModuleName);

		setString(hContact, "UserId", userId);

		if (mir_strlen(nick))
			setWString(hContact, "Nick", ptrW(mir_utf8decodeW(nick)));

		DBVARIANT dbv;
		if (!getWString("TeamName", &dbv) && Clist_GroupExists(dbv.ptszVal))
		{
			db_set_ws(hContact, "CList", "Group", dbv.ptszVal);
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

void CSlackProto::OnGotUserProfile(JSONNode &root)
{
	if (!root)
	{
		debugLogA(__FUNCTION__": failed to load user profile");
		return;
	}

	bool isOk = root["ok"].as_bool();
	if (!isOk)
	{
		debugLogA(__FUNCTION__": failed to load users profile");
		return;
	}

	JSONNode profile = root["profile"].as_node();
}

void CSlackProto::OnGotUserProfile(MCONTACT hContact, JSONNode &root)
{
	if (!root)
	{
		debugLogA(__FUNCTION__": failed to read users profile");
		return;
	}

	CMStringW firstName = root["first_name"].as_mstring();
	setWString(hContact, "FirstName", firstName);

	CMStringW lastName = root["last_name"].as_mstring();
	setWString(hContact, "LastName", lastName);
}

void CSlackProto::OnGotUserList(JSONNode &root, void*)
{
	if (!root)
	{
		debugLogA(__FUNCTION__": failed to load users list");
		return;
	}

	bool isOk = root["ok"].as_bool();
	if (!isOk)
	{
		debugLogA(__FUNCTION__": failed to load users list");
		return;
	}

	JSONNode users = root["members"].as_array();
	for (size_t i = 0; i < users.size(); i++)
	{
		JSONNode user = users[i];

		json_string userId = user["id"].as_string();
		json_string nick = user["name"].as_string();
		bool isDeleted = user["deleted"].as_bool();
		MCONTACT hContact = AddContact(userId.c_str(), nick.c_str(), isDeleted);
		if (hContact)
		{
			json_string teamId = user["team_id"].as_string();
			setString(hContact, "TeamId", teamId.c_str());

			CMStringW status = root["status"].as_mstring();
			if (!status.IsEmpty())
				setWString(hContact, "StatusMsg", status);

			json_string presence = user["presence"].as_string();
			SetContactStatus(hContact, SlackToMirandaStatus(presence.c_str()));

			JSONNode profile = root["profile"].as_node();
			OnGotUserProfile(hContact, profile);
		}
	}
}

INT_PTR CSlackProto::OnRequestAuth(WPARAM hContact, LPARAM lParam)
{
	if (!IsOnline())
	{
		return -1; // ???
	}
	return 0;
}

INT_PTR CSlackProto::OnGrantAuth(WPARAM hContact, LPARAM)
{
	if (!IsOnline())
	{
		// TODO: warn
		return 0;
	}

	return 0;
}

int CSlackProto::OnContactDeleted(MCONTACT hContact, LPARAM)
{
	if (!IsOnline())
	{
		// TODO: warn
		return 0;
	}

	return 0;
}