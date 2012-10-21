#include "skype_proto.h"

void CSkypeProto::UpdateContactAboutText(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropAbout(data);
	wchar_t* aboutText = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(aboutText, L"") == 0)
		this->DeleteSetting(hContact, "About");
	else
		this->SetSettingString(hContact, "About", aboutText);
	::mir_free(aboutText);
}

void CSkypeProto::UpdateContactAuthState(HANDLE hContact, CContact::Ref contact)
{
	uint newTS = 0;
	contact->GetPropAuthreqTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword(hContact, "AuthTS");
	if (newTS > oldTS)
	{
		CContact::AVAILABILITY data;
		contact->GetPropAvailability(data);
		this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(data));

		if (data == CContact::PENDINGAUTH)
			this->SetSettingWord(hContact, "Auth", 1);
		else
			this->DeleteSetting(hContact, "Auth");

		this->SetSettingDword(hContact, "AuthTS", newTS);
	}
}

void CSkypeProto::UpdateContactAvatar(HANDLE hContact, CContact::Ref contact)
{
	uint newTS = 0;
	contact->GetPropAvatarTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword(hContact, "AvatarTS");
	if (newTS > oldTS)
	{
		SEBinary data;
		contact->GetPropAvatarImage(data);
		
		if (data.size() > 0)
		{
			wchar_t* path = this->GetAvatarFilePath(this->GetSettingString(hContact, "sid"));
			FILE* fp = _wfopen(path, L"w");
			if (fp)
			{
				for (uint i = 0; i < data.size(); i++)
				{
					if (i)
						fputc(',', fp);
					fputc('\'', fp);
					switch(data[i])
					{
					case '\n':
						fputc('\\', fp);
						fputc('n', fp);
						break;

					default:
						fputc(data[i], fp);
					}
				}
				CloseHandle(fp);
			}
		}
		// todo: need to register avatar to contact
		this->SetSettingDword(hContact, "AvatarTS", newTS);
	}
}

void CSkypeProto::UpdateContactBirthday(HANDLE hContact, CContact::Ref contact)
{
	uint data;
	contact->GetPropBirthday(data);
	if (data > 0)
	{
		struct tm* ptm;
		time_t timeGMT = (time_t)data;
		ptm = gmtime(&timeGMT);
		this->SetSettingByte(hContact, "BirthDay", ptm->tm_mday);
		this->SetSettingByte(hContact, "BirthMonth", ptm->tm_mon);
		// todo: fix stupid year constant
		this->SetSettingWord(hContact, "BirthYear", ptm->tm_year + 1917);
	}
	else
	{
		this->DeleteSetting(hContact, "BirthDay");
		this->DeleteSetting(hContact, "BirthMonth");
		this->DeleteSetting(hContact, "BirthYear");
	}
}

void CSkypeProto::UpdateContactCity(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropCity(data);
	wchar_t* city = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(city, L"") == 0)
		this->DeleteSetting(hContact, "City");
	else
		this->SetSettingString(hContact, "City", city);
	::mir_free(city);
}

void CSkypeProto::UpdateContactCountry(HANDLE hContact, CContact::Ref contact)
{
	// country (en, ru, etc)
	SEString data;
	contact->GetPropCountry(data);
	// todo: write me
	//BYTE countryId = this->GetCountryIdByName((const char*)sData);
	//this->SetSettingByte(hContact, "Country", countryId);
}

void CSkypeProto::UpdateContactEmails(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropEmails(data);
	wchar_t* emails = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(emails, L"") == 0)
	{
		this->DeleteSetting(hContact, "e-mail0");
		this->DeleteSetting(hContact, "e-mail1");
		this->DeleteSetting(hContact, "e-mail2");
	}
	else
	{
		wchar_t* p = wcstok(emails, L" ");
		if (p == NULL)
		{
			this->SetSettingString(hContact, "e-mail0", emails);
		}
		else
		{
			this->SetSettingString(hContact, "e-mail0", p);
			p = wcstok(NULL, L" ");
			if (p) this->SetSettingString(hContact, "e-mail1", p);
			p = wcstok(NULL, L" ");
			if (p) this->SetSettingString(hContact, "e-mail2", p);
		}
	}
	::mir_free(emails);
}

void CSkypeProto::UpdateContactGender(HANDLE hContact, CContact::Ref contact)
{
	uint data;
	contact->GetPropGender(data);
	if (data)
		this->SetSettingByte(hContact, "Gender", (BYTE)(data == 1 ? 'M' : 'F'));
	else
		this->DeleteSetting(hContact, "Gender");
}

void CSkypeProto::UpdateContactHomepage(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropHomepage(data);
	wchar_t* homepage = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(homepage, L"") == 0)
		this->DeleteSetting(hContact, "Homepage");
	else
		this->SetSettingString(hContact, "Homepage", homepage);
	::mir_free(homepage);	
}

void CSkypeProto::UpdateContactLanguages(HANDLE hContact, CContact::Ref contact)
{
	// sanguages (en, ru, etc), space searated
	SEString data;
	contact->GetPropLanguages(data);
	// todo: write me
}

void CSkypeProto::UpdateContactMobilePhone(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropPhoneMobile(data);
	wchar_t* phone = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(phone, L"") == 0)
		this->DeleteSetting(hContact, "Cellular");
	else
		this->SetSettingString(hContact, "Cellular", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateContactPhone(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropPhoneHome(data);
	wchar_t* phone = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(phone, L"") == 0)
		this->DeleteSetting(hContact, "Phone");
	else
		this->SetSettingString(hContact, "Phone", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateContactOfficePhone(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropPhoneOffice(data);
	wchar_t* phone = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(phone, L"") == 0)
		this->DeleteSetting(hContact, "CompanyPhone");
	else
		this->SetSettingString(hContact, "CompanyPhone", phone);		
	::mir_free(phone);
}

void CSkypeProto::UpdateContactState(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropProvince(data);
	wchar_t* state = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(state, L"") == 0)
		this->DeleteSetting(hContact, "State");
	else
		this->SetSettingString(hContact, "State", state);		
	::mir_free(state);
}

void CSkypeProto::UpdateContactStatus(HANDLE hContact, CContact::Ref contact)
{
	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);
	this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(availability));
}

void CSkypeProto::UpdateContactStatusMessage(HANDLE hContact, CContact::Ref contact)
{
	uint newTS = 0;
	contact->GetPropMoodTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword(hContact, "XStatusTS");
	if (newTS > oldTS)
	{
		SEString data;
		contact->GetPropMoodText(data);
		wchar_t* status = ::mir_utf8decodeW((const char*)data);
		if (wcscmp(status, L"") == 0)
			this->DeleteSetting(hContact, "XStatusMsg");
		else
			this->SetSettingString(hContact, "XStatusMsg", status);
		::mir_free(status);
		this->SetSettingDword(hContact, "XStatusTS", newTS);
	}
}

void CSkypeProto::UpdateContactTimezone(HANDLE hContact, CContact::Ref contact)
{
	uint data;
	contact->GetPropTimezone(data);
	// todo: check me
	if (data > 0)
		this->SetSettingByte(hContact, "TimeZone", (data - 24*3600) / 3600);
	else
		this->DeleteSetting(hContact, "TimeZone");
}

void CSkypeProto::UpdateFullName(HANDLE hContact, CContact::Ref contact)
{
	SEString data;
	contact->GetPropFullname(data);
	wchar_t* fullname = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(fullname, L"") == 0)
	{
		this->DeleteSetting(hContact, "FirstName");
		this->DeleteSetting(hContact, "LastName");
	}
	else
	{
		wchar_t* last = _tcstok(fullname, L" ");
		wchar_t* first = _tcstok(NULL, L" ");
		if (first == NULL)
		{
			first = L"";
		}
		this->SetSettingString(hContact, "LastName", last);
		this->SetSettingString(hContact, "FirstName", first);
	}
	::mir_free(fullname);
}

void CSkypeProto::UpdateContactProfile(HANDLE hContact, CContact::Ref contact)
{
	uint newTS = 0;
	contact->GetPropProfileTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword(hContact, "ProfileTS");
	if (newTS > oldTS)
	{
		this->UpdateContactAboutText(hContact, contact);
		this->UpdateContactBirthday(hContact, contact);
		this->UpdateContactCity(hContact, contact);
		this->UpdateContactCountry(hContact, contact);
		this->UpdateContactEmails(hContact, contact);
		this->UpdateContactGender(hContact, contact);
		this->UpdateContactHomepage(hContact, contact);
		this->UpdateContactLanguages(hContact, contact);
		this->UpdateContactMobilePhone(hContact, contact);
		this->UpdateContactPhone(hContact, contact);
		this->UpdateContactOfficePhone(hContact, contact);
		this->UpdateContactState(hContact, contact);
		this->UpdateContactTimezone(hContact, contact);
		this->UpdateFullName(hContact, contact);

		this->SetSettingDword(hContact, "ProfileTS", newTS);
	}
}

void CSkypeProto::OnContactChanged(CContact::Ref contact, int prop)
{
	SEString data;
	contact->GetPropSkypename(data);
	wchar_t* sid = ::mir_a2u((const char*)data);
	HANDLE hContact = this->GetContactBySid(sid);

	if (hContact)
	{
		switch(prop)
		{
		case CContact::P_ABOUT:
			this->UpdateContactAboutText(hContact, contact);
			break;
		case CContact::P_AUTHREQ_TIMESTAMP:
			{
				uint newTS = 0;
				contact->GetPropAuthreqTimestamp(newTS);
				DWORD oldTS = this->GetSettingDword(hContact, "AuthTS");
				if (newTS > oldTS)
				{
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
			break;
		case CContact::P_AUTHREQUEST_COUNT:
			// todo: all authrequests after first should be catch here
			this->UpdateContactAuthState(hContact, contact);
			break;
		case CContact::P_AVAILABILITY:
			this->UpdateContactStatus(hContact, contact);
			break;
		case CContact::P_AVATAR_IMAGE:
		case CContact::P_AVATAR_TIMESTAMP:
			this->UpdateContactAvatar(hContact, contact);
			break;
		case CContact::P_BIRTHDAY:
			this->UpdateContactBirthday(hContact, contact);
			break;
		case CContact::P_CITY:
			this->UpdateContactCity(hContact, contact);
			break;
		case CContact::P_COUNTRY:
			this->UpdateContactCountry(hContact, contact);
			break;
		case CContact::P_EMAILS:
			this->UpdateContactEmails(hContact, contact);
			break;
		case CContact::P_GENDER:
			this->UpdateContactGender(hContact, contact);
			break;
		case CContact::P_HOMEPAGE:
			this->UpdateContactHomepage(hContact, contact);
			break;
		case CContact::P_LANGUAGES:
			this->UpdateContactLanguages(hContact, contact);
			break;
		case CContact::P_MOOD_TEXT:
		case CContact::P_MOOD_TIMESTAMP:
			this->UpdateContactStatusMessage(hContact, contact);
			break;
		case CContact::P_PHONE_HOME:
			this->UpdateContactPhone(hContact, contact);
			break;
		case CContact::P_PHONE_MOBILE:
			this->UpdateContactMobilePhone(hContact, contact);
			break;
		case CContact::P_PHONE_OFFICE:
			this->UpdateContactOfficePhone(hContact, contact);
			break;
		case CContact::P_PROFILE_TIMESTAMP:
			this->UpdateContactProfile(hContact, contact);
			break;
		case CContact::P_PROVINCE:
			this->UpdateContactState(hContact, contact);
			break;
		case CContact::P_TIMEZONE:
			this->UpdateContactTimezone(hContact, contact);
			break;
		case CContact::P_FULLNAME:
			this->UpdateFullName(hContact, contact);
			break;
		}
	}
	else
	{
	}
	//else
	//{
	//	switch(prop)
	//	{
	//	case CContact::P_RECEIVED_AUTHREQUEST:
	//		contact->GetPropDisplayname(data);
	//		wchar_t* displayName = ::mir_a2u((const char*)data);

	//		CCSDATA ccs = {0};
	//		ccs.szProtoService = PSR_AUTH;
	//		ccs.hContact = this->AddContactBySkypeName(skypeName, displayName, PALF_TEMPORARY);
	//		//pre.szMessage = (LPSTR)btBuff;
	//		//CreateBlobFromContact(ccs.hContact, lpwszMessage, dwMessageSize, btBuff, SIZEOF(btBuff), (size_t*)&pre.lParam);
	//		CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
	//		break;
	//	}
	//}

	//if (prop == CContact::P_AVAILABILITY)
	//{
	//	SEString data;

	//	contact->GetPropSkypename(data);
	//	wchar_t* skypeName = ::mir_a2u((const char*)data);

	//	HANDLE hContact = this->GetContactBySkypeName(skypeName);
	//	if (hContact)
	//	{
	//		CContact::AVAILABILITY availability;
	//		contact->GetPropAvailability(availability);
	//		this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(availability));

	//		if (availability == CContact::PENDINGAUTH)
	//			this->SetSettingWord(hContact, "Auth", 1);
	//		else
	//			this->DeleteSetting(hContact, "Auth");
	//	}
	//}
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

HANDLE CSkypeProto::GetContactBySid(const wchar_t* sid)
{
	HANDLE hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDFIRST, 0, (LPARAM)this->m_szModuleName);
	while (hContact)
	{
		if  (this->IsProtoContact(hContact))
		{
			if (::wcscmp(sid, this->GetSettingString(hContact, "sid", L"")) == 0)
				return hContact;
		}

		hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);
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

HANDLE CSkypeProto::AddContactBySid(const wchar_t* sid, const wchar_t* nick, DWORD flags)
{
	HANDLE hContact = this->GetContactBySid(sid);
	if ( !hContact)
	{
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);

		this->SetSettingString(hContact, "sid", sid);

		this->SetSettingString(hContact, "Nick", nick);
		//::DBWriteContactSettingWString(hContact, "CList", "MyHandle", displayName);

		if (flags & PALF_TEMPORARY)
		{
			::DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			::DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		}
	}
	else
	{
		if ( !(flags & PALF_TEMPORARY))
			::DBDeleteContactSetting(hContact, "CList", "NotOnList");
	}

	return hContact;
}

int CSkypeProto::SkypeToMirandaStatus(CContact::AVAILABILITY availability)
{
	int status = ID_STATUS_OFFLINE;

	switch (availability)
	{
	case CContact::ONLINE:
	case CContact::ONLINE_FROM_MOBILE:
		status = ID_STATUS_ONLINE;
		break;

	case CContact::AWAY:
	case CContact::AWAY_FROM_MOBILE:
		status = ID_STATUS_AWAY;
		break;

	case CContact::DO_NOT_DISTURB:
	case CContact::DO_NOT_DISTURB_FROM_MOBILE:
		status = ID_STATUS_DND;
		break;
	}

	return status;
}

void CSkypeProto::RemoveContact(HANDLE hContact)
{
	if (this->IsOnline() && hContact)
	{
		CContact::Ref contact;
		SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
		if (g_skype->GetContact(sid, contact))
		{
			contact->SetBuddyStatus(false);
			this->contactList.remove_val(contact);
		}
	}
}

CContact::AVAILABILITY CSkypeProto::MirandaToSkypeStatus(int status)
{
	CContact::AVAILABILITY availability = CContact::UNKNOWN;

	switch(this->m_iStatus)
	{
	case ID_STATUS_ONLINE:
		availability = CContact::ONLINE;
		break;

	case ID_STATUS_AWAY:
		availability = CContact::AWAY;
		break;

	case ID_STATUS_DND:
		availability = CContact::DO_NOT_DISTURB;
		break;

	case ID_STATUS_INVISIBLE:
		availability = CContact::INVISIBLE;
		break;
	}

	return availability;
}

void __cdecl CSkypeProto::LoadContactList(void*)
{
	g_skype->GetHardwiredContactGroup(CContactGroup::ALL_BUDDIES, this->commonList);
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
		wchar_t* sid = ::mir_utf8decodeW((const char*)data);

		contact->GetPropDisplayname(data);
		wchar_t* nick = ::mir_utf8decodeW((const char*)data);

		contact->GetPropFullname(data);
		wchar_t* name = ::mir_utf8decodeW((const char*)data);

		DWORD flags = 0;
		CContact::AVAILABILITY availability;
		contact->GetPropAvailability(availability);
		if (availability == CContact::PENDINGAUTH)
			flags = PALF_TEMPORARY;

		HANDLE hContact = this->AddContactBySid(sid, nick, flags);

		this->UpdateContactAuthState(hContact, contact);
		this->UpdateContactAvatar(hContact, contact);
		this->UpdateContactProfile(hContact, contact);
		this->UpdateContactStatus(hContact, contact);
		this->UpdateContactStatusMessage(hContact, contact);
	}

	// raise auth event for all non auth contacts
	CContact::Refs authContacts;
	g_skype->GetHardwiredContactGroup(CContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, this->authWaitList);
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
		char* sid = ::mir_utf8decodeA((const char*)data);
					
		contact->GetPropDisplayname(data);
		char* nick = ::mir_utf8decodeA((const char*)data);
					
		contact->GetPropReceivedAuthrequest(data);
		char* reason = ::mir_utf8decodeA((const char*)data);

		contact->GetPropFullname(data);
		char* fullname = ::mir_utf8decodeA((const char*)data);

		char* first = strtok(fullname, " ");
		char* last = strtok(NULL, " ");
		if (last == NULL) last = "";

		this->RaiseAuthRequestEvent(newTS, sid, nick, first, last, reason);
	}
}

void CSkypeProto::SetAllContactStatus(int status)
{
	HANDLE hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDFIRST, 0, (LPARAM)this->m_szModuleName);
	while (hContact)
	{
		if  (this->IsProtoContact(hContact))
			//if ( !this->GetSettingWord(hContact, SKYPE_SETTINGS_STATUS, ID_STATUS_OFFLINE) == status)
				this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, status);
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);
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

		wchar_t *first = wcstok(fullname, L" ");
		wchar_t *last = wcstok(NULL, L" ");
		if (last != NULL)
		{
			last = L"";
		}
		isr.firstName = first;
		isr.lastName = last;
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
	const wchar_t *sid = (wchar_t *)arg;

	HANDLE hContact = this->GetContactBySid(sid);
	if (hContact)
	{
		this->ShowNotification(sid, TranslateT("Contact already in your contact list"), 0);
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
}

void __cdecl CSkypeProto::SearchByEmailAsync(void* arg)
{
	const wchar_t *email = (wchar_t *)arg;

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
}

void __cdecl CSkypeProto::SearchByNamesAsync(void* arg)
{
}