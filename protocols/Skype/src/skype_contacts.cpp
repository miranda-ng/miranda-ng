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
				DBDeleteContactSetting(hContact, this->m_szModuleName, "Auth");
		}
	}
}

bool CSkypeProto::IsProtoContact(HANDLE hContact)
{
	return (::CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName));
}

HANDLE CSkypeProto::GetContactBySkypeName(wchar_t* skypeName)
{
	HANDLE hContact = (HANDLE)::CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		if  (this->IsProtoContact(hContact))
		{
			if (::wcscmp(skypeName, this->GetSettingString(hContact, "SkypeName", L"")) == 0)
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

		CContact::AVAILABILITY availability;
		contact->GetPropAvailability(availability);
		this->SetSettingWord(hContact, SKYPE_SETTINGS_STATUS, this->SkypeToMirandaStatus(availability));

		if (availability == CContact::PENDINGAUTH)
			this->SetSettingWord(hContact, "Auth", 1);
		else
			DBDeleteContactSetting(hContact, this->m_szModuleName, "Auth");
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