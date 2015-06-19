/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

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
		if (status == ID_STATUS_OFFLINE)
			db_unset(hContact, m_szModuleName, "MirVer");
	}
}

void CSkypeProto::SetAllContactsStatus(WORD status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (!isChatRoom(hContact))
			SetContactStatus(hContact, status);
	}
	if (status == ID_STATUS_OFFLINE)
		CloseAllChatChatSessions();
}

void CSkypeProto::SetChatStatus(MCONTACT hContact, int iStatus)
{
	ptrT tszChatID(getTStringA(hContact, "ChatRoomID"));
	if (tszChatID == NULL)
		return;
	GCDEST gcd = { m_szModuleName, tszChatID, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, (iStatus == ID_STATUS_OFFLINE) ? SESSION_OFFLINE : SESSION_ONLINE, (LPARAM)&gce);
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

	if (mir_strcmp(dbei.szModule, m_szModuleName) != 0)
		return INVALID_CONTACT_ID;
	return DbGetAuthEventContact(&dbei);
}

MCONTACT CSkypeProto::FindContact(const char *skypename)
{
	MCONTACT hContact = NULL;
	for (hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		ptrA cSkypename(getStringA(hContact, SKYPE_SETTINGS_ID));
		if (!mir_strcmpi(skypename, cSkypename))
			break;
	}
	return hContact;
}

MCONTACT CSkypeProto::AddContact(const char *skypename, bool isTemporary)
{
	MCONTACT hContact = FindContact(skypename);
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		Proto_AddToContact(hContact, m_szModuleName);

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
			db_set_b(hContact, "CList", "NotOnList", 1);
	}
	return hContact;
}

void CSkypeProto::LoadContactsAuth(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	const JSONNode &items = root.as_array();
	for (size_t i = 0; i < items.size(); i++)
	{
		const JSONNode &item = items.at(i);
		if (!item)
			break;

		std::string skypename = item["sender"].as_string();
		std::string reason = root["greeting"].as_string();
		time_t eventTime = IsoToUnixTime(root["event_time_iso"].as_string().c_str());

		MCONTACT hContact = AddContact(skypename.c_str());
		if (hContact)
		{
			time_t lastEventTime = db_get_dw(hContact, m_szModuleName, "LastAuthRequestTime", 0);

			if (lastEventTime == 0 || lastEventTime < eventTime)
			{
				db_set_dw(hContact, m_szModuleName, "LastAuthRequestTime", eventTime);
				delSetting(hContact, "Auth");

				PROTORECVEVENT pre = { 0 };
				pre.timestamp = time(NULL);
				pre.lParam = (DWORD)(sizeof(DWORD) * 2 + skypename.size() + reason.size() + 5);

				/*blob is: 0(DWORD), hContact(DWORD), nick(ASCIIZ), firstName(ASCIIZ), lastName(ASCIIZ), id(ASCIIZ), reason(ASCIIZ)*/
				PBYTE pBlob, pCurBlob;
				pCurBlob = pBlob = (PBYTE)mir_calloc(pre.lParam);

				*((PDWORD)pCurBlob) = 0;
				pCurBlob += sizeof(DWORD);
				*((PDWORD)pCurBlob) = (DWORD)hContact;
				pCurBlob += sizeof(DWORD);
				pCurBlob += 3;
				mir_strcpy((char*)pCurBlob, skypename.c_str());
				pCurBlob += skypename.size() + 1;
				mir_strcpy((char*)pCurBlob, reason.c_str());
				pre.szMessage = (char*)pBlob;

				ProtoChainRecv(hContact, PSR_AUTH, 0, (LPARAM)&pre);
			}
		}
	}
}

//[{"username":"echo123", "firstname" : "Echo \/ Sound Test Service", "lastname" : null, "avatarUrl" : null, "mood" : null, "richMood" : null, "displayname" : null, "country" : null, "city" : null},...]
void CSkypeProto::LoadContactsInfo(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	const JSONNode &items = root.as_array();
	for (size_t i = 0; i < items.size(); i++)
	{
		const JSONNode &item = items.at(i);
		if (!item)
			break;

		std::string skypename = item["username"].as_string();
		MCONTACT hContact = AddContact(skypename.c_str());
		if (hContact)
		{
			UpdateProfileCountry(item, hContact);
			UpdateProfileCity(item, hContact);
			UpdateProfileStatusMessage(item, hContact);
		}
	}
}

//[{"skypename":"echo123", "authorized" : true, "blocked" : false, ...},...]
// other properties is exists but empty

void CSkypeProto::LoadContactList(const NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		return;

	JSONNode root = JSONNode::parse(response->pData);
	if (!root)
		return;

	LIST<char> skypenames(1);
	bool loadAll = getBool("LoadAllContacts", false);
	const JSONNode &items = root["contacts"].as_array();
	for (size_t i = 0; i < items.size(); i++)
	{
		const JSONNode &item = items.at(i);
		if (!item)
			break;

		const JSONNode &name = item["name"];
		const JSONNode &phones = item["phones"];

		std::string skypename = item["id"].as_string();
		CMString display_name = item["display_name"].as_mstring();
		CMString first_name = name["first"].as_mstring();
		CMString last_name = name["surname"].as_mstring();
		CMString avatar_url = item["avatar_url"].as_mstring();
		std::string type = item["type"].as_string();
		
		if (type == "skype" || loadAll)
		{
			MCONTACT hContact = AddContact(skypename.c_str());
			if (hContact)
			{
				if (item["authorized"].as_bool())
				{
					delSetting(hContact, "Auth");
					delSetting(hContact, "Grant");
				}
				else setByte(hContact, "Grant", 1);

				if (item["blocked"].as_bool())
				{
					db_set_dw(hContact, "Ignore", "Mask1", 127);
					db_set_b(hContact, "CList", "Hidden", 1);
					setByte(hContact, "IsBlocked", 1);
				}
				else
				{
					if (db_get_b(hContact, m_szModuleName, "IsBlocked", 0))
					{
						db_set_dw(hContact, "Ignore", "Mask1", 0);
						db_set_b(hContact, "CList", "Hidden", 0);
						setByte(hContact, "IsBlocked", 0);
					}
				}

				setString(hContact, "Type", type.c_str());

				if (display_name) 
					setTString(hContact, "Nick", display_name); 
				if (first_name) 
					setTString(hContact, "FirstName", first_name); 
				if (last_name)
					setTString(hContact, "LastName", last_name); 

				SetAvatarUrl(hContact, avatar_url);
				ReloadAvatarInfo(hContact);

				for (size_t j = 0; j < phones.size(); j++)
				{
					const JSONNode &phone = phones.at(j);
					if (!phone)
						break;

					CMString number = phone["number"].as_mstring();

					switch (phone["type"].as_int())
					{
					case 0:
						setTString(hContact, "Phone", number);
						break;
					case 2:
						setTString(hContact, "Cellular", number);
						break;
					}
				}

				if (type == "skype" || type == "msn") skypenames.insert(mir_strdup(skypename.c_str()));
			}
		}
	}

	ptrA token(getStringA("TokenSecret"));
	if (skypenames.getCount() > 0)
	{
		int i = 0;
		do
		{
			LIST<char> users(1);
			for (; i < skypenames.getCount() && users.getCount() < 25; i++)
			{
				users.insert(skypenames[i]);
			}

			PushRequest(new GetContactsInfoRequest(token, users), &CSkypeProto::LoadContactsInfo);

			for (int j = 0; i < users.getCount(); j++)
				mir_free(users[j]);
			users.destroy();
		}
		while(i < skypenames.getCount());

		for (int i = 0; i < skypenames.getCount(); i++)
			mir_free(skypenames[i]);
		skypenames.destroy();
	}
	PushRequest(new GetContactsAuthRequest(token), &CSkypeProto::LoadContactsAuth);
}

INT_PTR CSkypeProto::OnRequestAuth(WPARAM hContact, LPARAM)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AddContactRequest(m_szTokenSecret, skypename));
	return 0;
}

INT_PTR CSkypeProto::OnGrantAuth(WPARAM hContact, LPARAM)
{
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	ptrA skypename(getStringA(hContact, SKYPE_SETTINGS_ID));
	PushRequest(new AuthAcceptRequest(m_szTokenSecret, skypename));
	return 0;
}

int CSkypeProto::OnContactDeleted(MCONTACT hContact, LPARAM)
{
	if (!IsOnline()) return 1;

	if (hContact && !isChatRoom(hContact))
		PushRequest(new DeleteContactRequest(m_szTokenSecret, db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID)));
	return 0;
}

INT_PTR CSkypeProto::BlockContact(WPARAM hContact, LPARAM)
{
	if (!IsOnline()) return 1;

	if (IDYES == MessageBox(NULL, TranslateT("Are you sure?"), TranslateT("Warning"), MB_YESNO | MB_ICONQUESTION))
		SendRequest(new BlockContactRequest(m_szTokenSecret, ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID))), &CSkypeProto::OnBlockContact, (void *)hContact);
	return 0;
}

void CSkypeProto::OnBlockContact(const NETLIBHTTPREQUEST *response, void *p)
{
	MCONTACT hContact = (MCONTACT)p;
	if (response == NULL)
		return;
	db_set_dw(hContact, "Ignore", "Mask1", 127);
	db_set_b(hContact, "CList", "Hidden", 1);
}

INT_PTR CSkypeProto::UnblockContact(WPARAM hContact, LPARAM)
{
	SendRequest(new UnblockContactRequest(m_szTokenSecret, ptrA(db_get_sa(hContact, m_szModuleName, SKYPE_SETTINGS_ID))), &CSkypeProto::OnUnblockContact, (void *)hContact);
	return 0;
}

void CSkypeProto::OnUnblockContact(const NETLIBHTTPREQUEST *response, void *p)
{
	MCONTACT hContact = (MCONTACT)p;
	if (response == NULL)
		return;
	db_set_dw(hContact, "Ignore", "Mask1", 0);
	db_set_b(hContact, "CList", "Hidden", 0);
	db_set_b(hContact, m_szModuleName, "IsBlocked", 0);
}