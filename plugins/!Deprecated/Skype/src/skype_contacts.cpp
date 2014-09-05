#include "skype.h"

void CSkypeProto::UpdateContactAuthState(MCONTACT hContact, const ContactRef &contact)
{
	uint newTS = 0;
	contact->GetPropAuthreqTimestamp(newTS);
	DWORD oldTS = this->getDword("AuthTS", 0);
	if (newTS > oldTS)
	{
		bool result;
		if (contact->HasAuthorizedMe(result) && !result)
			this->setByte(hContact, "Auth", !result);
		else
		{
			this->delSetting(hContact, "Auth");
			if (contact->IsMemberOfHardwiredGroup(CContactGroup::ALL_BUDDIES, result) && !result)
				this->setByte(hContact, "Grant", !result);
			else
				this->delSetting(hContact, "Grant");
		}

		this->setDword(hContact, "AuthTS", newTS);
	}
}

void CSkypeProto::UpdateContactStatus(MCONTACT hContact, const ContactRef &contact)
{
	Contact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);
	this->setWord(hContact, SKYPE_SETTINGS_STATUS, CSkypeProto::SkypeToMirandaStatus(availability));

	if (availability == Contact::SKYPEOUT)
		this->setWord(hContact, SKYPE_SETTINGS_STATUS, ID_STATUS_ONTHEPHONE);
	else
	{
		if (availability == Contact::PENDINGAUTH)
			this->setByte(hContact, "Auth", 1);
		else
			this->delSetting(hContact, "Auth");
	}
}

void CSkypeProto::UpdateContactClient(MCONTACT hContact, const ContactRef &contact)
{
	bool isMobile = false;
	contact->HasCapability(Contact::CAPABILITY_MOBILE_DEVICE, isMobile/*, true*/);

	this->setTString(hContact, "MirVer", isMobile ? L"SkypeMobile" : L"Skype");
}

void CSkypeProto::UpdateContactOnlineSinceTime(MCONTACT hContact, const ContactRef &contact)
{
	uint newTS = 0;
	contact->GetPropLastonlineTimestamp(newTS);
	DWORD oldTS = ::db_get_dw(hContact, this->m_szModuleName, "OnlineSinceTS", 0);
	if (newTS > oldTS)
		this->setDword(hContact, "OnlineSinceTS", newTS);
}

void CSkypeProto::UpdateContactLastEventDate(MCONTACT hContact, const ContactRef &contact)
{
	uint newTS = 0;
	contact->GetPropLastusedTimestamp(newTS);
	DWORD oldTS = this->getDword(hContact, "LastEventDateTS", 0);
	if (newTS > oldTS)
		this->setDword(hContact, "LastEventDateTS", newTS);
}

void CSkypeProto::OnContactChanged(const ContactRef &contact, int prop)
{
	SEString data;
	contact->GetPropSkypename(data);
	wchar_t *sid = ::mir_utf8decodeW(data);
	MCONTACT hContact = this->GetContactBySid(sid);
	::mir_free(sid);

	SEObject *contactObj = contact.fetch();

	if (hContact)
	{
		switch(prop)
		{
		case Contact::P_AUTHREQ_TIMESTAMP:
			{
				uint newTS = 0;
				contact->GetPropAuthreqTimestamp(newTS);
				DWORD oldTS = this->getDword(hContact, "AuthTS", 0);
				if (newTS > oldTS)
					this->RaiseAuthRequestEvent(newTS, contact);
			}
			break;

		case Contact::P_AUTHREQUEST_COUNT:
			// todo: all authrequests after first should be catch here
			this->UpdateContactAuthState(hContact, contact);
			break;

		case Contact::P_AVAILABILITY:
			this->UpdateContactStatus(hContact, contact);
			this->UpdateChatUserStatus(contact);
			break;

		//case CContact::P_AVATAR_IMAGE:
		case Contact::P_AVATAR_TIMESTAMP:
			this->UpdateProfileAvatar(contactObj, hContact);
			break;

		//case CContact::P_MOOD_TEXT:
		case Contact::P_MOOD_TIMESTAMP:
			this->UpdateProfileStatusMessage(contactObj, hContact);
			break;

		case Contact::P_FULLNAME:
			this->UpdateChatUserNick(contact);
			break;

		case Contact::P_PROFILE_TIMESTAMP:
			this->UpdateProfile(contactObj, hContact);
			break;
		}
	}
}

void CSkypeProto::OnContactListChanged(const ContactRef &contact)
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

bool CSkypeProto::IsProtoContact(MCONTACT hContact)
{
	return ::lstrcmpiA(::GetContactProto(hContact), this->m_szModuleName) == 0;
}

MCONTACT CSkypeProto::GetContactBySid(const wchar_t *sid)
{
	MCONTACT hContact = NULL;

	::EnterCriticalSection(&this->contact_search_lock);

	for (hContact = ::db_find_first(this->m_szModuleName); hContact; hContact = ::db_find_next(hContact, this->m_szModuleName))
	{
		ptrW contactSid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
		if (::lstrcmpi(contactSid, sid) == 0)
			break;
	}

	::LeaveCriticalSection(&this->contact_search_lock);

	return hContact;
}

MCONTACT CSkypeProto::GetContactFromAuthEvent(HANDLE hEvent)
{
	// db_event_getContact
	DWORD body[3];
	DBEVENTINFO dbei = { sizeof(DBEVENTINFO) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (::db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (strcmp(dbei.szModule, this->m_szModuleName) != 0)
		return INVALID_CONTACT_ID;

	return ::DbGetAuthEventContact(&dbei);
}

MCONTACT CSkypeProto::AddContact(CContact::Ref contact, bool isTemporary)
{
	ptrW sid(::mir_utf8decodeW(contact->GetSid()));

	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);

	MCONTACT hContact = this->GetContactBySid(sid);
	if ( !hContact)
	{
		hContact = (MCONTACT)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)this->m_szModuleName);

		ptrW nick(::mir_utf8decodeW(contact->GetNick()));

		switch(availability) {
		case CContact::SKYPEOUT:
			this->setByte(hContact, "IsSkypeOut", 1);
			break;

		case CContact::PENDINGAUTH:
			::db_set_b(hContact, "CList", "NotOnList", 1);
			break;

		case CContact::BLOCKED:
		case CContact::BLOCKED_SKYPEOUT:
			::db_set_b(hContact, "CList", "Hidden", 1);
			break;

		default:
			this->delSetting(hContact, "IsSkypeOut");
			::db_unset(hContact, "CList", "Hidden");
			::db_unset(hContact, "CList", "NotOnList");
		}

		this->setTString(hContact, SKYPE_SETTINGS_SID, sid);
		this->setTString(hContact, "Nick", nick);

		DBVARIANT dbv;
		if ( !this->getTString(SKYPE_SETTINGS_DEF_GROUP, &dbv))
		{
			::db_set_ts(hContact, "CList", "Group", dbv.ptszVal);
			::db_free(&dbv);
		}
	}

	return hContact;
}

void __cdecl CSkypeProto::LoadContactList(void* data)
{
	this->debugLogW(L"Updating contacts list");

	bool isFirstLoad = data != NULL;

	this->GetHardwiredContactGroup(CContactGroup::ALL_BUDDIES, this->commonList);
	this->commonList.fetch();

	this->commonList->GetContacts(this->contactList);
	fetch(this->contactList);
	for (uint i = 0; i < this->contactList.size(); i++)
	{
		CContact::Ref contact = this->contactList[i];

		MCONTACT hContact = this->AddContact(contact);

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

	this->authWaitList->GetContacts(authContacts);
	for (uint i = 0; i < authContacts.size(); i++)
	{
		CContact::Ref contact = authContacts[i];

		uint newTS = 0;
		contact->GetPropAuthreqTimestamp(newTS);

		this->RaiseAuthRequestEvent(newTS, contact);
	}
}

bool CSkypeProto::IsContactOnline(MCONTACT hContact)
{
	return this->getWord(hContact, SKYPE_SETTINGS_STATUS, ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE;
}

void CSkypeProto::SetAllContactStatus(int status)
{
	::EnterCriticalSection(&this->contact_search_lock);

	for (MCONTACT hContact = ::db_find_first(this->m_szModuleName); hContact; hContact = ::db_find_next(hContact, this->m_szModuleName))
	{
		if (this->getByte(hContact, "IsSkypeOut", 0) != 0)
			continue;
		if (this->isChatRoom(hContact))
			continue;
		if (this->IsContactOnline(hContact))
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

	MCONTACT hContact = this->GetContactBySid(sid);
	if (hContact)
	{
		this->ShowNotification(TranslateT("Contact already in your contact list"), 0, hContact);
		this->SendBroadcast(ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)SKYPE_SEARCH_BYSID, 0);
		return;
	}

	CContactSearch::Ref search;
	this->CreateIdentitySearch(::mir_u2a(sid), search);
	search.fetch();
	search->SetProtoInfo((HANDLE)SKYPE_SEARCH_BYSID);

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
	search->SetProtoInfo((HANDLE)SKYPE_SEARCH_BYEMAIL);

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
	search->SetProtoInfo((HANDLE)SKYPE_SEARCH_BYNAMES);

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

	MCONTACT hContact = this->AddContact(author);

	SEBinary guid;
	message->GetPropGuid(guid);
	ReadMessageParam param = { guid, messageType };

	PROTORECVEVENT pre = { 0 };
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

	MCONTACT hContact = this->AddContact(receiver);
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