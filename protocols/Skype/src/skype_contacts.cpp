#include "skype_proto.h"

HANDLE CSkypeProto::GetContactBySkypeLogin(const char* skypeLogin)
{
	for (HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		//if(!IsMyContact(hContact))
		//	continue;

		DBVARIANT dbv;
		if( !DBGetContactSettingString(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN, &dbv))
		{
			if(strcmp(skypeLogin, dbv.pszVal) == 0)
			{
				DBFreeVariant(&dbv);
				return hContact;
			}
			else
				DBFreeVariant(&dbv);
		}
	}

	return 0;
}

void __cdecl CSkypeProto::LoadContactList(void*)
{
	CContactGroup::Ref contacts;
	g_skype->GetHardwiredContactGroup(CContactGroup::SKYPE_BUDDIES, contacts);

    contacts->GetContacts(contacts->ContactList);
    fetch(contacts->ContactList);

    for (unsigned int i = 0; i < contacts->ContactList.size(); i++)
    {
		SEString name;
		SEString skypeLogin;

		int status = ID_STATUS_OFFLINE;
		CContact::AVAILABILITY availability = CContact::OFFLINE;

        contacts->ContactList[i]->GetPropSkypename(skypeLogin);
        printf("%3d. %s\n", i+1, (const char*)skypeLogin);

		HANDLE hContact = this->GetContactBySkypeLogin(skypeLogin);
		if (!hContact)
		{
			hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
			DBWriteContactSettingString(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN, skypeLogin);
		}

		if (CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)this->m_szModuleName) == 0 )
		{

			contacts->ContactList[i]->GetPropDisplayname(name);
			contacts->ContactList[i]->GetPropAvailability(availability);

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

			DBDeleteContactSetting(hContact, "CList", "MyHandle");

			/*DBVARIANT dbv;
			if (!this->GetSettingString(this->m_szModuleName,FACEBOOK_KEY_DEF_GROUP,&dbv))
			{
				DBWriteContactSettingTString(hContact,"CList","Group",dbv.ptszVal);
				DBFreeVariant(&dbv);
			}*/

			DBWriteContactSettingString(hContact, this->m_szModuleName, SKYPE_SETTINGS_NAME, name);
			DBWriteContactSettingWord(hContact, this->m_szModuleName, SKYPE_SETTINGS_STATUS, ID_STATUS_ONLINE);
			DBWriteContactSettingString(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN, skypeLogin);
		}
		else
		{
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
		}

		//HANDLE hContact = AddToContactList(fbu, FACEBOOK_CONTACT_APPROVE, false, fbu->real_name.c_str());
		//DBWriteContactSettingByte(hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, FACEBOOK_CONTACT_APPROVE);
	}
}

void CSkypeProto::SetAllContactStatuses(int status)
{
	for (HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	    hContact;
	    hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0))
	{
		if (DBGetContactSettingWord(hContact, this->m_szModuleName, SKYPE_SETTINGS_STATUS, ID_STATUS_OFFLINE) == status)
			continue;

		DBWriteContactSettingWord(hContact, this->m_szModuleName, SKYPE_SETTINGS_STATUS, status);
	}
}