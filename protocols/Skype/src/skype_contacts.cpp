#include "skype_proto.h"

void CSkypeProto::OnContactChanged(CContact* contact, int prop)
{
	if (prop == CContact::P_AVAILABILITY)
	{
		SEString data;

		contact->GetPropSkypename(data);
		wchar_t* skypeName = ::mir_a2u((const char*)data);

		HANDLE hContact = this->GetContactBySkypeName(skypeName);
		if (hContact)
		{
			CContact::AVAILABILITY availability;
			contact->GetPropAvailability(availability);
			this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(availability));

			if (availability == CContact::PENDINGAUTH)
				this->SetSettingWord(hContact, "Auth", 1);
			else
				this->DeleteSetting(hContact, "Auth");
		}
	}
}

bool CSkypeProto::IsProtoContact(HANDLE hContact)
{
	return ::CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName) < 0;
}

HANDLE CSkypeProto::GetContactBySkypeName(wchar_t* skypeName)
{
	HANDLE hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if  (this->IsProtoContact(hContact))
		{
			wchar_t* data = this->GetSettingString(hContact, "SkypeName", L"");
			bool result = ::wcscmp(skypeName, data) == 0;
			mir_free(data);
			if (result)
				return hContact;
		}

		hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	return 0;
}

HANDLE CSkypeProto::AddContactBySkypeName(wchar_t* skypeName, wchar_t* displayName, DWORD flags)
{
	HANDLE hContact = this->GetContactBySkypeName(skypeName);
	if ( !hContact)
	{
		hContact = (HANDLE)::CallService(MS_DB_CONTACT_ADD, 0, 0);
		::CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName);

		this->SetSettingString(hContact, "SkypeName", skypeName);
		this->SetSettingString(hContact, "Nick", displayName);
		//::DBWriteContactSettingWString(hContact, "CList", "MyHandle", displayName);

		if (flags & PALF_TEMPORARY)
		{
			::DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
			::DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		}
	}
	else
	{
		if (!(flags & PALF_TEMPORARY))
			::DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
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

void CSkypeProto::LoadContactInfo(HANDLE hContact, CContact::Ref contact)
{
	CContact::AVAILABILITY availability;
	contact->GetPropAvailability(availability);
	this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(availability));

	if (availability == CContact::PENDINGAUTH)
		this->SetSettingWord(hContact, "Auth", 1);
	else
		this->DeleteSetting(hContact, "Auth");

	uint newTS = 0;
	DWORD oldTS = 0;

	// profile info
	contact->GetPropProfileTimestamp(newTS);
	oldTS = this->GetSettingDword(hContact, "ProfileUpdateTS");
	//if (newTS > oldTS)
	{
		uint uData;
		SEString sData;
		// birth date
		contact->GetPropBirthday(uData);
		if (uData > 0)
		{
			struct tm* ptm;
			time_t timeGMT = (time_t)uData;
			ptm = gmtime(&timeGMT);
			this->SetSettingByte(hContact, "BirthDay", ptm->tm_mday);
			this->SetSettingByte(hContact, "BirthMonth", ptm->tm_mon);
			this->SetSettingWord(hContact, "BirthYear", ptm->tm_year + 1917);
		}
		// gender
		contact->GetPropGender(uData);
		this->SetSettingByte(hContact, "Gender", (BYTE)(uData ? 'M' : 'F'));
		// timezone
		contact->GetPropTimezone(uData);
		if (uData > 0)
			this->SetSettingByte(hContact, "TimeZone", uData);
		else
			this->DeleteSetting(hContact, "TimeZone");
		// language
        contact->GetPropLanguages(sData);
		// country (en, ru, etc)
		contact->GetPropCountry(sData);
		BYTE countryId = this->GetCountryIdByName((const char*)sData);
		this->SetSettingByte(hContact, "Country", countryId);
		// state
		contact->GetPropProvince(sData);
		this->SetSettingString(hContact, "State", ::mir_a2u((const char*)sData));
		// city
		contact->GetPropCity(sData);
		this->SetSettingString(hContact, "City", ::mir_a2u((const char*)sData));
		// home phone
		contact->GetPropPhoneHome(sData);
		this->SetSettingString(hContact, "Phone", ::mir_a2u((const char*)sData));
        // office phone
		contact->GetPropPhoneOffice(sData);
		this->SetSettingString(hContact, "CompanyPhone", ::mir_a2u((const char*)sData));
        // mobile phone
		contact->GetPropPhoneMobile(sData);
		this->SetSettingString(hContact, "Cellular", ::mir_a2u((const char*)sData));
		// e-mail
		contact->GetPropEmails(sData);
		this->SetSettingString(hContact, "e-mail", ::mir_a2u((const char*)sData));
		// homepage
		contact->GetPropHomepage(sData);
		this->SetSettingString(hContact, "Homepage", ::mir_a2u((const char*)sData));
		// about
		contact->GetPropAbout(sData);
		this->SetSettingString(hContact, "About", ::mir_a2u((const char*)sData));
		// profile update ts
		this->SetSettingDword(hContact, "ProfileUpdateTS", newTS);
	}

	// mood text
	contact->GetPropProfileTimestamp(newTS);
	oldTS = this->GetSettingDword(hContact, "XStatusTS");
	if (newTS > oldTS)
	{
		SEString status;
		contact->GetPropAbout(status);
		this->SetSettingString(hContact, "XStatusMsg", ::mir_a2u((const char*)status));
		// mood text update ts
		this->SetSettingDword(hContact, "XStatusTS", newTS);
	}

	// avatar
	contact->GetPropProfileTimestamp(newTS);
	oldTS = this->GetSettingDword(hContact, "AvatarTS");
	if (newTS > oldTS)
	{
		SEBinary avatar;
		contact->GetPropAvatarImage(avatar);
		
		if (avatar.size() > 0)
		{
			FILE* fp = _wfopen(this->GetAvatarFilePath(this->GetSettingString(hContact, "SkypeName")), L"w");
			for (int i = 0; i < avatar.size(); i++)
			{
				if (i)
					fputc(',', fp);
				fputc('\'', fp);
				switch(avatar[i])
				{
				case '\n':
					fputc('\\', fp);
					fputc('n', fp);
					break;

				default:
					fputc(avatar[i], fp);
				}
			}
			CloseHandle(fp);
		}
		// todo: need to register avatar to contact
		//avatar update ts
		this->SetSettingDword(hContact, "AvatarTS", newTS);
	}
}

void __cdecl CSkypeProto::LoadContactList(void*)
{
	g_skype->GetHardwiredContactGroup(CContactGroup::ALL_KNOWN_CONTACTS, this->contactGroup);

    this->contactGroup->GetContacts(this->contactGroup->ContactList);
    Sid::fetch(this->contactGroup->ContactList);

    for (unsigned int i = 0; i < this->contactGroup->ContactList.size(); i++)
    {
		CContact::Ref contact = this->contactGroup->ContactList[i];
		contact->SetOnContactChangeCallback((OnContactChangeFunc)&CSkypeProto::OnContactChanged, this);

		SEString data;

		contact->GetPropSkypename(data);
		wchar_t* skypeName = ::mir_a2u((const char*)data);

		contact->GetPropDisplayname(data);
		wchar_t* displayName = :: mir_utf8decodeW((const char*)data);

		HANDLE hContact = this->AddContactBySkypeName(skypeName, displayName, 0);

		this->LoadContactInfo(hContact, contact);
	}
}

void CSkypeProto::SetAllContactStatus(int status)
{
	HANDLE hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if  (this->IsProtoContact(hContact))
			if ( !this->GetSettingWord(hContact, SKYPE_SETTINGS_STATUS, ID_STATUS_OFFLINE) == status)
				this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, status);

		hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}