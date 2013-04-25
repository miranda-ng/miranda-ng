#include "skype_proto.h"

void CSkypeProto::UpdateContactAuthState(HANDLE hContact, CContact::Ref contact)
{
	uint newTS = 0;
	contact->GetPropAuthreqTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword(hContact, "AuthTS");
	if (newTS > oldTS)
	{
		bool result;
		if (contact->HasAuthorizedMe(result) && !result)
		{
			this->SetSettingByte(hContact, "Auth", !result);
		}
		else
		{
			this->DeleteSetting(hContact, "Auth");
			if (contact->IsMemberOfHardwiredGroup(CContactGroup::ALL_BUDDIES, result) && !result)
				this->SetSettingByte(hContact, "Grant", !result);
			else
				this->DeleteSetting(hContact, "Grant");
		}

		this->SetSettingDword(hContact, "AuthTS", newTS);
	}
}

void CSkypeProto::UpdateContactStatus(HANDLE hContact, CContact::Ref contact)
{
	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);
	this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(availability));

	if (availability == CContact::SKYPEOUT)
	{
		this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, ID_STATUS_OUTTOLUNCH);
	}
	else
	{
		if (availability == CContact::PENDINGAUTH)
			this->SetSettingByte(hContact, "Auth", 1);
		else
			this->DeleteSetting(hContact, "Auth");
	}
}

void CSkypeProto::UpdateContactOnlineSinceTime(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* CContact::P_LASTONLINE_TIMESTAMP */35);
	DWORD oldTS = this->GetSettingDword(hContact, "OnlineSinceTS");
	if (newTS > oldTS)
		this->SetSettingDword(hContact, "OnlineSinceTS", newTS);
}

void CSkypeProto::UpdateContactLastEventDate(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* CContact::P_LASTUSED_TIMESTAMP */39);
	DWORD oldTS = this->GetSettingDword(hContact, "LastEventDateTS");
	if (newTS > oldTS)
		this->SetSettingDword(hContact, "LastEventDateTS", newTS);
}

void CSkypeProto::OnContactChanged(CContact::Ref contact, int prop)
{
	SEString data;
	contact->GetPropSkypename(data);
	wchar_t *sid = ::mir_utf8decodeW(data);
	HANDLE hContact = this->GetContactBySid(sid);
	::mir_free(sid);

	SEObject *contactObj = contact.fetch();

	if (hContact)
	{
		switch(prop)
		{
		case CContact::P_AUTHREQ_TIMESTAMP:
			{
				uint newTS = 0;
				contact->GetPropAuthreqTimestamp(newTS);
				DWORD oldTS = this->GetSettingDword(hContact, "AuthTS");
				if (newTS > oldTS)
				{
					this->RaiseAuthRequestEvent(newTS, contact);
				}
			}
			break;

		case CContact::P_AUTHREQUEST_COUNT:
			// todo: all authrequests after first should be catch here
			this->UpdateContactAuthState(hContact, contact);
			break;

		case CContact::P_AVAILABILITY:
			this->UpdateContactStatus(hContact, contact);
			this->UpdateChatUserStatus(contact);
			break;

		//case CContact::P_AVATAR_IMAGE:
		case CContact::P_AVATAR_TIMESTAMP:
			this->UpdateProfileAvatar(contactObj, hContact);
			break;

		//case CContact::P_MOOD_TEXT:
		case CContact::P_MOOD_TIMESTAMP:
			this->UpdateProfileStatusMessage(contactObj, hContact);
			break;

		case CContact::P_PROFILE_TIMESTAMP:
			this->UpdateProfile(contactObj, hContact);
			break;
		}
	}
}

void CSkypeProto::OnContactListChanged(const ContactRef& contact)
{
	bool result;

	contact->IsMemberOfHardwiredGroup(CContactGroup::ALL_BUDDIES, result);
	if (result)
	{
		if ( !this->contactList.contains(contact))
		{
			CContact::Ref newContact(contact);
			this->contactList.append(newContact);
			newContact.fetch();
			newContact->SetOnContactChangedCallback(
				(CContact::OnContactChanged)&CSkypeProto::OnContactChanged, 
				this);
		}
	}

	contact->IsMemberOfHardwiredGroup(CContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, result);
	if (result)
	{
		SEString data;

		uint newTS = 0;
		contact->GetPropAuthreqTimestamp(newTS);

		this->RaiseAuthRequestEvent(newTS, contact);
	}
}

bool CSkypeProto::IsProtoContact(HANDLE hContact)
{
	return ::CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName) < 0;
}

HANDLE CSkypeProto::GetContactBySid(const wchar_t *sid)
{
	HANDLE hContact = ::db_find_first();
	while (hContact)
	{
		if (this->IsProtoContact(hContact) && !this->IsChatRoom(hContact))
		{
			mir_ptr<wchar_t> contactSid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN));
			if (contactSid && ::wcsicmp(sid, contactSid) == 0)
				return hContact;
		}

		hContact = ::db_find_next(hContact);
	}

	return 0;
}

HANDLE CSkypeProto::GetContactFromAuthEvent(HANDLE hEvent)
{
	// db_event_getContact
	DWORD body[3];
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (::CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbei))
		return INVALID_HANDLE_VALUE;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_HANDLE_VALUE;

	if (strcmp(dbei.szModule, this->m_szModuleName) != 0)
		return INVALID_HANDLE_VALUE;

	return ::DbGetAuthEventContact(&dbei);
}

HANDLE CSkypeProto::AddContact(CContact::Ref contact)
{
	wchar_t *sid = ::mir_utf8decodeW(contact->GetSid());

	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);

	HANDLE hContact = this->GetContactBySid(sid);
	if ( !hContact)
	{
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);

		wchar_t *nick = ::mir_utf8decodeW(contact->GetNick());

		switch(availability)
		{
		case CContact::SKYPEOUT:
			this->SetSettingByte(hContact, "IsSkypeOut", 1);
			break;

		case CContact::PENDINGAUTH:
			::db_set_b(hContact, "CList", "NotOnList", 1);
			break;

		case CContact::BLOCKED:
		case CContact::BLOCKED_SKYPEOUT:
			::db_set_b(hContact, "CList", "Hidden", 1);
			break;

		default:
			::db_unset(hContact, this->m_szModuleName, "IsSkypeOut");
			//::db_unset(hContact, "CList", "Hidden");
			::db_unset(hContact, "CList", "NotOnList");
		}

		::db_set_ws(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN, sid);
		::db_set_ws(hContact, this->m_szModuleName, "Nick", nick);
		DBVARIANT dbv;
		if(!db_get_ts(NULL, m_szModuleName, SKYPE_SETTINGS_DEF_GROUP, &dbv))
		{
			db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			db_free(&dbv);
		}

		::mir_free(nick);
	}

	::mir_free(sid);

	return hContact;
}

void __cdecl CSkypeProto::LoadContactList(void*)
{
	g_skype->GetHardwiredContactGroup(CContactGroup::ALL_BUDDIES, this->commonList);
	this->commonList.fetch();
	this->commonList->SetOnContactListChangedCallback(
		(CContactGroup::OnContactListChanged)&CSkypeProto::OnContactListChanged, 
		this);

	this->commonList->GetContacts(this->contactList);
	fetch(this->contactList);
	for (uint i = 0; i < this->contactList.size(); i++)
	{
		CContact::Ref contact = this->contactList[i];
		contact->SetOnContactChangedCallback(
			(CContact::OnContactChanged)&CSkypeProto::OnContactChanged, 
			this);

		HANDLE hContact = this->AddContact(contact);

		// todo: move to AddContact?
		this->UpdateContactAuthState(hContact, contact);
		this->UpdateContactStatus(hContact, contact);

		this->UpdateProfile(contact.fetch(), hContact);
	}
}

void __cdecl CSkypeProto::LoadChatList(void*)
{
	CConversation::Refs conversations;
	g_skype->GetConversationList(conversations);
	for (uint i = 0; i < conversations.size(); i++)
	{
		CConversation::TYPE type;
		conversations[i]->GetPropType(type);

		CConversation::MY_STATUS status;
		conversations[i]->GetPropMyStatus(status);
		if (type == CConversation::CONFERENCE && status == CConversation::CONSUMER)
		{
			auto conversation = conversations[i];

			this->AddChatRoom(conversation);
			this->JoinToChat(conversation, false);
		}
	}

	/*CConversation::Refs conversations;
	g_skype->GetConversationList(conversations);
	for (uint i = 0; i < conversations.size(); i++)
	{
		conversations[i]->Delete();
	}*/
}

void __cdecl CSkypeProto::LoadAuthWaitList(void*)
{
	CContact::Refs authContacts;
	g_skype->GetHardwiredContactGroup(CContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, this->authWaitList);
	this->authWaitList.fetch();
	this->authWaitList->SetOnContactListChangedCallback(
		(CContactGroup::OnContactListChanged)&CSkypeProto::OnContactListChanged, 
		this);

	this->authWaitList->GetContacts(authContacts);
	for (uint i = 0; i < authContacts.size(); i++)
	{
		CContact::Ref contact = authContacts[i];

		uint newTS = 0;
		contact->GetPropAuthreqTimestamp(newTS);

		this->RaiseAuthRequestEvent(newTS, contact);
	}
}

bool CSkypeProto::IsContactOnline(HANDLE hContact)
{
	return ::db_get_w(
		hContact,
		this->m_szModuleName,
		SKYPE_SETTINGS_STATUS,
		ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE;
}

void CSkypeProto::SetAllContactStatus(int status)
{
	HANDLE hContact = ::db_find_first();
	while (hContact)
	{
		if (this->IsProtoContact(hContact) && this->GetSettingByte(hContact, "IsSkypeOut", 0) == 0)
			this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, status);

		hContact = ::db_find_next(hContact);
	}
}

void CSkypeProto::OnSearchCompleted(HANDLE hSearch)
{
	this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch, 0);
}

void CSkypeProto::OnContactFinded(CContact::Ref contact, HANDLE hSearch)
{
	PROTOSEARCHRESULT psr = {0};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_TCHAR;

	SEString data;
	contact->GetPropSkypename(data);
	psr.id = ::mir_utf8decodeW(data);
	contact->GetPropDisplayname(data);
	psr.nick  = ::mir_utf8decodeW(data);

	SEString firstName, lastName;
	contact->GetFullname(firstName, lastName);
	psr.firstName = ::mir_utf8decodeW(firstName);
	psr.lastName = ::mir_utf8decodeW(lastName);

	{
		contact->GetPropEmails(data);
		wchar_t *emails = ::mir_utf8decodeW(data);

		wchar_t* main = ::wcstok(emails, L" ");
		if (main != NULL)
		{
			psr.email = main;
		}

		::mir_free(emails);
	}

	this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_DATA, hSearch, (LPARAM)&psr);
}

void __cdecl CSkypeProto::SearchBySidAsync(void* arg)
{
	wchar_t *sid = (wchar_t*)arg;

	HANDLE hContact = this->GetContactBySid(sid);
	if (hContact)
	{
		this->ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
		this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)SKYPE_SEARCH_BYSID, 0);
		return;
	}

	CContactSearch::Ref search;
	g_skype->CreateIdentitySearch(::mir_u2a(sid), search);
	search.fetch();
	search->SetProtoInfo(this, (HANDLE)SKYPE_SEARCH_BYSID);
	search->SetOnContactFindedCallback(
		(CContactSearch::OnContactFinded)&CSkypeProto::OnContactFinded);
	search->SetOnSearchCompleatedCallback(
		(CContactSearch::OnSearchCompleted)&CSkypeProto::OnSearchCompleted);

	bool valid;
	if (!search->IsValid(valid) || !valid || !search->Submit())
	{
		return;
	}
	search->BlockWhileSearch();
	search->Release();

	::mir_free(sid);
}

void __cdecl CSkypeProto::SearchByEmailAsync(void* arg)
{
	wchar_t *email = (wchar_t *)arg;

	CContactSearch::Ref search;
	g_skype->CreateContactSearch(search);
	search.fetch();
	search->SetProtoInfo(this, (HANDLE)SKYPE_SEARCH_BYEMAIL);
	search->SetOnContactFindedCallback(
		(CContactSearch::OnContactFinded)&CSkypeProto::OnContactFinded);
	search->SetOnSearchCompleatedCallback(
		(CContactSearch::OnSearchCompleted)&CSkypeProto::OnSearchCompleted);

	bool valid;
	if (!search->AddEmailTerm(::mir_u2a(email), valid) || !valid || !search->Submit())
	{
		return; 
	}
	search->BlockWhileSearch();
	search->Release();

	::mir_free(email);
}

void __cdecl CSkypeProto::SearchByNamesAsync(void* arg)
{
	//todo: write me
	PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT *)arg;
}