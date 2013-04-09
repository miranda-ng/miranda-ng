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
	if (hContact)
	{
		uint newTS = obj->GetUintProp(/* CContact::P_LASTUSED_TIMESTAMP */39);
		DWORD oldTS = this->GetSettingDword(hContact, "LastEventDateTS");
		if (newTS > oldTS)
			this->SetSettingDword(hContact, "LastEventDateTS", newTS);
	}
}

void CSkypeProto::OnContactChanged(CContact::Ref contact, int prop)
{
	SEString data;
	contact->GetPropSkypename(data);
	const char *sid = (const char*)data;
	HANDLE hContact = this->GetContactBySid(sid);

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
					contact->GetPropDisplayname(data);
					char* nick = ::mir_utf8decodeA((const char*)data);
					
					contact->GetPropReceivedAuthrequest(data);
					char* reason = ::mir_utf8decodeA((const char*)data);

					contact->GetPropFullname(data);
					char* fullname = ::mir_utf8decodeA((const char*)data);

					char* first = strtok(fullname, " ");
					char* last = strtok(NULL, " ");
					if (last == NULL)
					{
						last = "";
					}
					
					this->RaiseAuthRequestEvent(newTS, sid, nick, first, last, reason);
				}
			}
			break;
		case CContact::P_AUTHREQUEST_COUNT:
			// todo: all authrequests after first should be catch here
			this->UpdateContactAuthState(hContact, contact);
			break;

		case CContact::P_AVAILABILITY:
			this->UpdateContactStatus(hContact, contact);
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

		contact->GetPropSkypename(data);
		char* sid = ::mir_utf8decodeA((const char*)data);
					
		contact->GetPropDisplayname(data);
		char* nick = ::mir_utf8decodeA((const char*)data);
					
		contact->GetPropReceivedAuthrequest(data);
		char* reason = ::mir_utf8decodeA((const char*)data);

		contact->GetPropFullname(data);
		char* fullname = ::mir_utf8decodeA((const char*)data);

		char* first = strtok(fullname, " ");
		char* last = strtok(NULL, " ");
		if (last == NULL)
		{
			last = "";
		}
					
		this->RaiseAuthRequestEvent(newTS, sid, nick, first, last, reason);
	}
}

bool CSkypeProto::IsProtoContact(HANDLE hContact)
{
	return ::CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName) < 0;
}

HANDLE CSkypeProto::GetContactBySid(const char *sid)
{
	HANDLE hContact = ::db_find_first();
	while (hContact)
	{
		if  (this->IsProtoContact(hContact) && !this->IsChatRoom(hContact))
		{
			char *contactSid = ::db_get_sa(hContact, this->m_szModuleName, "sid");
			if (contactSid && ::strcmp(sid, contactSid) == 0)
				return hContact;
		}

		hContact = ::db_find_next(hContact);
	}

	return 0;
}

HANDLE CSkypeProto::GetContactFromAuthEvent(HANDLE hEvent)
{
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

HANDLE CSkypeProto::AddContactBySid(const char* sid, const char* nick, DWORD flags)
{
	HANDLE hContact = this->GetContactBySid(sid);
	if ( !hContact)
	{
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);

		::db_set_s(hContact, this->m_szModuleName, "sid", sid);
		::db_set_s(hContact, this->m_szModuleName, "Nick", nick);
		
		CContact::Ref contact;
		if (this->skype->GetContact(sid, contact))
		{
			contact.fetch();
			bool result;
			if (contact->IsMemberOfHardwiredGroup(CContactGroup::ALL_BUDDIES, result))
			{
				CContactGroup::Ref group;
				if (this->skype->GetHardwiredContactGroup(CContactGroup::ALL_BUDDIES, group))
				{
					group.fetch();
					group->AddContact(contact);
				}
			}
		}

		if (flags & 256)
		{
			this->SetSettingByte(hContact, "IsSkypeOut", 1);
		}
		else
		{
			this->SetSettingByte(hContact, "Auth", 1);
			::db_unset(hContact, this->m_szModuleName, "IsSkypeOut");

			this->UpdateProfile(contact.fetch(), hContact);

			if (flags & PALF_TEMPORARY)
			{
				::db_set_b(hContact, "CList", "NotOnList", 1);
				::db_set_b(hContact, "CList", "Hidden", 1);
			}
		}
	}
	else
	{
		if ( !(flags & PALF_TEMPORARY))
			::db_unset(hContact, "CList", "NotOnList");
	}

	return hContact;
}

void __cdecl CSkypeProto::LoadContactList(void*)
{
	this->skype->GetHardwiredContactGroup(CContactGroup::ALL_BUDDIES, this->commonList);
	this->commonList.fetch();
	this->commonList->SetOnContactListChangedCallback(
		(CContactGroup::OnContactListChanged)&CSkypeProto::OnContactListChanged, 
		this);
	this->commonList->GetContacts(this->contactList);
	Sid::fetch(this->contactList);	

	for (unsigned int i = 0; i < this->contactList.size(); i++)
	{
		CContact::Ref contact = this->contactList[i];
		contact->SetOnContactChangedCallback(
			(CContact::OnContactChanged)&CSkypeProto::OnContactChanged, 
			this);

		SEString data;

		contact->GetPropSkypename(data);
		char *sid = ::mir_strdup(data);

		contact->GetPropDisplayname(data);
		char *nick = ::mir_utf8decodeA(data);

		contact->GetPropFullname(data);
		char *name = ::mir_utf8decodeA(data);

		DWORD flags = 0;
		CContact::AVAILABILITY availability;
		contact->GetPropAvailability(availability);

		if (availability == CContact::SKYPEOUT)
		{
			flags |= 256;
			contact->GetPropPstnnumber(data);
			::mir_free(sid);
			sid = ::mir_strdup((const char *)data);
		}
		else if (availability == CContact::PENDINGAUTH)
			flags = PALF_TEMPORARY;

		HANDLE hContact = this->AddContactBySid(sid, nick, flags);

		SEObject *obj = contact.fetch();
		this->UpdateContactAuthState(hContact, contact);
		this->UpdateContactStatus(hContact, contact);

		this->UpdateProfile(obj, hContact);
		this->UpdateProfileAvatar(obj, hContact);
		this->UpdateProfileStatusMessage(obj, hContact);
	}

	//CConversation::Refs conversations;
	//this->skype->GetConversationList(conversations);
	//for (uint i = 0; i < conversations.size(); i++)
	//{
	//	CConversation::TYPE type;
	//	conversations[i]->GetPropType(type);

	//	CConversation::MY_STATUS status;
	//	conversations[i]->GetPropMyStatus(status);
	//	if (type == CConversation::CONFERENCE)
	//	{
	//		SEString data;

	//		conversations[i]->GetPropIdentity(data);
	//		char *cid = ::mir_strdup(data);

	//		conversations[i]->GetPropDisplayname(data);
	//		char *name = ::mir_utf8decodeA(data);

	//		HANDLE hContact = this->AddChatRoomByID(cid, name);
	//		//::DBWriteContactSettingString(hContact, this->m_szModuleName, "Nick", name);

	//		CConversation::LOCAL_LIVESTATUS live;
	//		conversations[i]->GetPropLocalLivestatus(live);

	//		if (status == CConversation::CONSUMER)// && live != CConversation::NONE)
	//		{
	//			this->JoinToChat(cid, false);
	//		}
	//	}
	//}

	/*CConversation::Refs conversations;
	this->skype->GetConversationList(conversations);
	for (uint i = 0; i < conversations.size(); i++)
	{
		conversations[i]->Delete();
	}*/

	// raise auth event for all non auth contacts
	CContact::Refs authContacts;
	this->skype->GetHardwiredContactGroup(CContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, this->authWaitList);
	this->authWaitList.fetch();
	this->authWaitList->SetOnContactListChangedCallback(
		(CContactGroup::OnContactListChanged)&CSkypeProto::OnContactListChanged, 
		this);
	this->authWaitList->GetContacts(authContacts);
    //Sid::fetch(this->contactList);	

    for (unsigned int i = 0; i < authContacts.size(); i++)
    {
		CContact::Ref contact = authContacts[i];
		/*contact->SetOnContactChangedCallback(
			(CContact::OnContactChanged)&CSkypeProto::OnContactChanged, 
			this);*/

		SEString data;

		uint newTS = 0;
		contact->GetPropAuthreqTimestamp(newTS);

		contact->GetPropSkypename(data);
		const char* sid = (const char *)data;
		
		CContact::AVAILABILITY availability;
		contact->GetPropAvailability(availability);

		if (availability == CContact::SKYPEOUT)
		{
			contact->GetPropPstnnumber(data);
			sid = (const char*)data;
		}
					
		contact->GetPropDisplayname(data);
		char* nick = ::mir_utf8decodeA((const char *)data);
					
		contact->GetPropReceivedAuthrequest(data);
		char* reason = ::mir_utf8decodeA((const char *)data);

		contact->GetPropFullname(data);
		char* fullname = ::mir_utf8decodeA((const char *)data);

		char* first = strtok(fullname, " ");
		char* last = strtok(NULL, " ");
		if (last == NULL) last = "";

		this->RaiseAuthRequestEvent(newTS, sid, nick, first, last, reason);
	}
}

void CSkypeProto::SetAllContactStatus(int status)
{
	HANDLE hContact = db_find_first();
	while (hContact)
	{
		if  (this->IsProtoContact(hContact))
		{
			//if ( !this->GetSettingWord(hContact, SKYPE_SETTINGS_STATUS, ID_STATUS_OFFLINE) == status)
			if( this->GetSettingByte(hContact, "IsSkypeOut", 0) == 0)
				this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, status);
		}
		hContact = db_find_next(hContact);
	}
}

void CSkypeProto::OnSearchCompleted(HANDLE hSearch)
{
	this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_SUCCESS, hSearch, 0);
}

void CSkypeProto::OnContactFinded(HANDLE hSearch, CContact::Ref contact)
{
	PROTOSEARCHRESULT isr = {0};
	isr.cbSize = sizeof(isr);
	isr.flags = PSR_TCHAR;
		
	SEString data;
	contact->GetPropSkypename(data);
	isr.id = ::mir_utf8decodeW((const char *)data);
	contact->GetPropDisplayname(data);
	isr.nick  = ::mir_utf8decodeW((const char *)data);
	{
		contact->GetPropFullname(data);
		wchar_t *fullname = ::mir_utf8decodeW((const char*)data);

		isr.lastName = fullname;
	}
	{
		contact->GetPropEmails(data);
		wchar_t *emails = ::mir_utf8decodeW((const char*)data);

		wchar_t* main = wcstok(emails, L" ");
		if (main != NULL)
		{
			isr.email = main;
		}
	}
	this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_DATA, hSearch, (LPARAM)&isr);
}

void __cdecl CSkypeProto::SearchBySidAsync(void* arg)
{
	const char *sid = (char *)arg;

	HANDLE hContact = this->GetContactBySid(sid);
	if (hContact)
	{
		this->ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
		this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)SKYPE_SEARCH_BYSID, 0);
		return;
	}

	CContactSearch::Ref search;
	this->skype->CreateIdentitySearch(sid, search);
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
}

void __cdecl CSkypeProto::SearchByEmailAsync(void* arg)
{
	const wchar_t *email = (wchar_t *)arg;

	CContactSearch::Ref search;
	this->skype->CreateContactSearch(search);
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
}

void __cdecl CSkypeProto::SearchByNamesAsync(void* arg)
{
}