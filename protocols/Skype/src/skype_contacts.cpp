#include "skype_proto.h"

void CSkypeProto::UpdateContactAuthState(HANDLE hContact, CContact::Ref contact)
{
	uint newTS = 0;
	contact->GetPropAuthreqTimestamp(newTS);
	DWORD oldTS = ::db_get_dw(NULL, this->m_szModuleName, "AuthTS", 0);
	if (newTS > oldTS)
	{
		bool result;
		if (contact->HasAuthorizedMe(result) && !result)
		{
			::db_set_b(hContact, this->m_szModuleName, "Auth", !result);
		}
		else
		{
			::db_unset(hContact, this->m_szModuleName, "Auth");
			if (contact->IsMemberOfHardwiredGroup(CContactGroup::ALL_BUDDIES, result) && !result)
				::db_set_b(hContact, this->m_szModuleName, "Grant", !result);
			else
				::db_unset(hContact, this->m_szModuleName, "Grant");
		}

		::db_set_dw(hContact, this->m_szModuleName, "AuthTS", newTS);
	}
}

void CSkypeProto::UpdateContactStatus(HANDLE hContact, CContact::Ref contact)
{
	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);
	::db_set_w(hContact, this->m_szModuleName, SKYPE_SETTINGS_STATUS, CSkypeProto::SkypeToMirandaStatus(availability));

	if (availability == CContact::SKYPEOUT)
	{
		::db_set_w(hContact, this->m_szModuleName, SKYPE_SETTINGS_STATUS, ID_STATUS_ONTHEPHONE);
	}
	else
	{
		if (availability == CContact::PENDINGAUTH)
			::db_set_b(hContact, this->m_szModuleName, "Auth", 1);
		else
			::db_unset(hContact, this->m_szModuleName, "Auth");
	}
}

void CSkypeProto::UpdateContactClient(SEObject *obj, HANDLE hContact)
{
	bool isMobile = false;
	((CContact *)obj)->HasCapability(Contact::CAPABILITY_MOBILE_DEVICE, isMobile/*, true*/);

	::db_set_ws(hContact, this->m_szModuleName, "MirVer", isMobile ? L"SkypeMobile" : L"Skype");
}

void CSkypeProto::UpdateContactNickName(SEObject *obj, HANDLE hContact)
{
	// todo: P_DISPLAYNAME = 21 is unworked
	ptrW nick( ::mir_utf8decodeW(obj->GetStrProp(/* *::P_FULLNAME */ 5)));
	if ( !::wcslen(nick))
		::db_unset(hContact, this->m_szModuleName, "Nick");
	else
		::db_set_ws(hContact, this->m_szModuleName, "Nick", nick);
}

void CSkypeProto::UpdateContactOnlineSinceTime(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* CContact::P_LASTONLINE_TIMESTAMP */35);
	DWORD oldTS = ::db_get_dw(hContact, this->m_szModuleName, "OnlineSinceTS", 0);
	if (newTS > oldTS)
		::db_set_dw(hContact, this->m_szModuleName, "OnlineSinceTS", newTS);
}

void CSkypeProto::UpdateContactLastEventDate(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* CContact::P_LASTUSED_TIMESTAMP */39);
	DWORD oldTS = ::db_get_dw(hContact, this->m_szModuleName, "LastEventDateTS", 0);
	if (newTS > oldTS)
		::db_set_dw(hContact, this->m_szModuleName, "LastEventDateTS", newTS);
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
		switch(prop) {
		case CContact::P_AUTHREQ_TIMESTAMP:
			{
				uint newTS = 0;
				contact->GetPropAuthreqTimestamp(newTS);
				DWORD oldTS = ::db_get_dw(hContact, this->m_szModuleName, "AuthTS", 0);
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

void CSkypeProto::OnContactListChanged(CContact::Ref contact)
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
	HANDLE hContact = NULL;

	::EnterCriticalSection(&this->contact_search_lock);

	for (hContact = ::db_find_first(this->m_szModuleName); hContact; hContact = ::db_find_next(hContact, this->m_szModuleName))
	{
		if ( !this->IsChatRoom(hContact))
		{
			ptrW contactSid( ::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN));
			if (::lstrcmpi(contactSid, sid) == 0)
				break;
		}
	}

	::LeaveCriticalSection(&this->contact_search_lock);

	return hContact;
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
	ptrW sid(::mir_utf8decodeW(contact->GetSid()));

	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);

	HANDLE hContact = this->GetContactBySid(sid);
	if ( !hContact)
	{
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);

		ptrW nick(::mir_utf8decodeW(contact->GetNick()));

		switch(availability) 
		{
		case CContact::SKYPEOUT:
			::db_set_b(hContact, this->m_szModuleName, "IsSkypeOut", 1);
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
	}

	return hContact;
}

void __cdecl CSkypeProto::LoadContactList(void* data)
{
	this->Log(L"Updating contacts list");

	bool isFirstLoad = data != NULL;

	this->GetHardwiredContactGroup(CContactGroup::ALL_BUDDIES, this->commonList);
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

		if ( !isFirstLoad)
		{
			// todo: move to AddContact?
			this->UpdateContactAuthState(hContact, contact);
			this->UpdateContactStatus(hContact, contact);

			ptrW nick( ::db_get_wsa(hContact, "CList", "MyHandle"));
			if ( !nick || !::wcslen(nick))
			{
				nick = ::mir_utf8decodeW(contact->GetNick());
				::db_set_ws(hContact, "CList", "MyHandle", nick);
			}

			this->UpdateProfile(contact.fetch(), hContact);
		}
	}
}

void __cdecl CSkypeProto::LoadAuthWaitList(void*)
{
	CContact::Refs authContacts;
	this->GetHardwiredContactGroup(CContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, this->authWaitList);
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
	::EnterCriticalSection(&this->contact_search_lock);

	for (HANDLE hContact = ::db_find_first(this->m_szModuleName); hContact; hContact = ::db_find_next(hContact, this->m_szModuleName))
	{
		if (::db_get_b(hContact, this->m_szModuleName, "IsSkypeOut", 0) == 0)
			::db_set_w(hContact, this->m_szModuleName, SKYPE_SETTINGS_STATUS, status);
	}

	::LeaveCriticalSection(&this->contact_search_lock);
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
		mir_ptr<wchar_t> emails( ::mir_utf8decodeW(data));

		wchar_t* main = ::wcstok(emails, L" ");
		if (main != NULL)
		{
			psr.email = main;
		}
	}

	this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_DATA, hSearch, (LPARAM)&psr);
}

void __cdecl CSkypeProto::SearchBySidAsync(void* arg)
{
	mir_ptr<wchar_t> sid((wchar_t*)arg);

	HANDLE hContact = this->GetContactBySid(sid);
	if (hContact)
	{
		this->ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
		this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)SKYPE_SEARCH_BYSID, 0);
		return;
	}

	CContactSearch::Ref search;
	this->CreateIdentitySearch(::mir_u2a(sid), search);
	search.fetch();
	search->SetProtoInfo(this, (HANDLE)SKYPE_SEARCH_BYSID);
	search->SetOnContactFindedCallback(
		(CContactSearch::OnContactFinded)&CSkypeProto::OnContactFinded);
	search->SetOnSearchCompleatedCallback(
		(CContactSearch::OnSearchCompleted)&CSkypeProto::OnSearchCompleted);

	bool valid;
	if (!search->IsValid(valid) || !valid || !search->Submit())
		return;

	search->BlockWhileSearch();
	search->Release();
}

void __cdecl CSkypeProto::SearchByEmailAsync(void* arg)
{
	mir_ptr<wchar_t> email((wchar_t *)arg);

	CContactSearch::Ref search;
	this->CreateContactSearch(search);
	search.fetch();
	search->SetProtoInfo(this, (HANDLE)SKYPE_SEARCH_BYEMAIL);
	search->SetOnContactFindedCallback(
		(CContactSearch::OnContactFinded)&CSkypeProto::OnContactFinded);
	search->SetOnSearchCompleatedCallback(
		(CContactSearch::OnSearchCompleted)&CSkypeProto::OnSearchCompleted);

	bool valid;
	if (!search->AddEmailTerm(::mir_u2a(email), valid) || !valid || !search->Submit())
		return; 

	search->BlockWhileSearch();
	search->Release();
}

void __cdecl CSkypeProto::SearchByNamesAsync(void* arg)
{
	//todo: write me
	PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT *)arg;

	std::string nick = ::mir_utf8encodeW(psr->nick);
	std::string fName = ::mir_utf8encodeW(psr->firstName);
	std::string lName = " "; lName += ::mir_utf8encodeW(psr->lastName);

	CContactSearch::Ref search;
	this->CreateContactSearch(search);
	search.fetch();
	search->SetProtoInfo(this, (HANDLE)SKYPE_SEARCH_BYNAMES);
	search->SetOnContactFindedCallback(
		(CContactSearch::OnContactFinded)&CSkypeProto::OnContactFinded);
	search->SetOnSearchCompleatedCallback(
		(CContactSearch::OnSearchCompleted)&CSkypeProto::OnSearchCompleted);

	bool valid;
	if (nick.length() != 0)
	{
		search->AddStrTerm(
			Contact::P_FULLNAME,
			CContactSearch::CONTAINS_WORD_PREFIXES,
			nick.c_str(), 
			valid,
			true);
	}
	if (fName.length() != 0)
	{
		search->AddOr();
		search->AddStrTerm(
			Contact::P_FULLNAME,
			CContactSearch::CONTAINS_WORD_PREFIXES,
			fName.c_str(), 
			valid,
			true);
	}
	if (lName.length() != 0)
	{		
		search->AddOr();
		search->AddStrTerm(
			Contact::P_FULLNAME,
			CContactSearch::CONTAINS_WORD_PREFIXES,
			lName.c_str(), 
			valid,
			true);
	}

	if (!search->Submit())
		return; 

	search->BlockWhileSearch();
	search->Release();
}

void CSkypeProto::OnContactsReceived(const ConversationRef &conversation, const MessageRef &message)
{
	CContact::Refs contacts;
	message->GetContacts(contacts);

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	SEString data;
	message->GetPropAuthor(data);			
		
	CContact::Ref author;
	this->GetContact(data, author);

	HANDLE hContact = this->AddContact(author);

	SEBinary guid;
	message->GetPropGuid(guid);
	ReadMessageParam param = { guid, messageType };

	PROTORECVEVENT pre;
	pre.flags = PREF_UTF;
	pre.lParam = (LPARAM)&param;
	pre.timestamp = timestamp;

	int msgSize = 1;
	pre.szMessage = (char *)::mir_alloc(msgSize);
	pre.szMessage[0] = 0;

	int len = 0;
	char* pCur = &pre.szMessage[0];

	for (size_t i = 0; i < contacts.size(); i ++)
	{
		contacts[i]->GetIdentity(data);
		if ( ::lstrcmpi(mir_ptr<wchar_t>(::mir_utf8decodeW(data)), this->login) != 0)
			this->AddContact(contacts[i]);
	}

	char *text = ::mir_utf8encode(::Translate("Contacts received"));

	this->AddDBEvent(
		hContact,
		SKYPE_DB_EVENT_TYPE_CONTACTS,
		timestamp,
		PREF_UTF,
		(DWORD)::strlen(text) + 1,
		(PBYTE)text);
}

void CSkypeProto::OnContactsSent(const ConversationRef &conversation, const MessageRef &message)
{
	SEString data;

	CMessage::TYPE messageType;
	message->GetPropType(messageType);

	uint timestamp;
	message->GetPropTimestamp(timestamp);

	CMessage::SENDING_STATUS status;
	message->GetPropSendingStatus(status);

	CParticipant::Refs participants;
	conversation->GetParticipants(participants, CConversation::OTHER_CONSUMERS);
	participants[0]->GetPropIdentity(data);
		
	CContact::Ref receiver;
	this->GetContact(data, receiver);

	HANDLE hContact = this->AddContact(receiver);
	this->SendBroadcast(
		hContact,
		ACKTYPE_CONTACTS,
		status == CMessage::FAILED_TO_SEND ? ACKRESULT_FAILED : ACKRESULT_SUCCESS,
		(HANDLE)message->getOID(), 0);
}

void CSkypeProto::OnContactsEvent(const ConversationRef &conversation, const MessageRef &message)
{
	SEString author;
	message->GetPropAuthor(author);
			
	if (::wcsicmp(mir_ptr<wchar_t>(::mir_utf8decodeW(author)), this->login) == 0)
		this->OnContactsSent(conversation, message);
	else
		this->OnContactsReceived(conversation, message);
}