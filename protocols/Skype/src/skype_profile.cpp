#include "skype_proto.h"

void CSkypeProto::UpdateOwnAvatar()
{
	uint newTS = 0;
	this->account->GetPropAvatarTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword("AvatarTS");
	if (newTS > oldTS)
	{
		SEBinary data;
		//this->account->GetPropAvatarImage(data);
		
		//if (data.size() > 0)
		{
			// todo: add own avatar loading'n'registration
		}		
	}
}

void CSkypeProto::UpdateOwnBirthday()
{
	uint data;
	this->account->GetPropBirthday(data);
	TCHAR date[9];
	_itot_s(data, date, 10);
	if (data > 0)
	{
		INT day, month, year;
		_stscanf(date, _T("%04d%02d%02d"), &year, &month, &day);
		this->SetSettingByte("BirthDay", day);
		this->SetSettingByte("BirthMonth", month);
		this->SetSettingWord("BirthYear", year);

		SYSTEMTIME sToday = {0};
		GetLocalTime(&sToday);
		int nAge = sToday.wYear - year;
		if (sToday.wMonth < month || (sToday.wMonth == month && sToday.wDay < day))
			nAge--;
		if (nAge)
			this->SetSettingWord("Age", ( WORD )nAge );
	}
	else
	{
		this->DeleteSetting("BirthDay");
		this->DeleteSetting("BirthMonth");
		this->DeleteSetting("BirthYear");
		this->DeleteSetting("Age");
	}
}

void CSkypeProto::UpdateOwnCity()
{
	SEString data;
	this->account->GetPropCity(data);
	wchar_t* city = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(city, L"") == 0)
		this->DeleteSetting("City");
	else
		this->SetSettingString("City", city);
	::mir_free(city);
}

void CSkypeProto::UpdateOwnCountry()
{
	// country (en, ru, etc)
	SEString data;
	char* country;
	this->account->GetPropCountry(data);
	char* isocode = ::mir_utf8decodeA((const char*)data);
	if (strcmp(isocode, "") == 0)
	{
		country = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, 0xFFFF, 0);
		this->SetSettingString("Country", _A2T(country));
	}
	else
	{
		country = (char*)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode, 0);
		this->SetSettingString("Country", _A2T(country));
	}
	::mir_free(isocode);
}

void CSkypeProto::UpdateOwnEmails()
{
	SEString data;
	this->account->GetPropEmails(data);
	wchar_t* emails = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(emails, L"") == 0)
	{
		this->DeleteSetting("e-mail0");
		this->DeleteSetting("e-mail1");
		this->DeleteSetting("e-mail2");
	}
	else
	{
		wchar_t* p = wcstok(emails, L" ");
		if (p == NULL)
		{
			this->SetSettingString("e-mail0", emails);
		}
		else
		{
			this->SetSettingString("e-mail0", p);
			p = wcstok(NULL, L" ");
			if (p) this->SetSettingString("e-mail1", p);
			p = wcstok(NULL, L" ");
			if (p) this->SetSettingString("e-mail2", p);
		}
	}
	::mir_free(emails);
}

void CSkypeProto::UpdateOwnGender()
{
	uint data;
	this->account->GetPropGender(data);
	if (data)
		this->SetSettingByte("Gender", (BYTE)(data == 1 ? 'M' : 'F'));
	else
		this->DeleteSetting("Gender");
}

void CSkypeProto::UpdateOwnHomepage()
{
	SEString data;
	this->account->GetPropHomepage(data);
	wchar_t* homepage = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(homepage, L"") == 0)
		this->DeleteSetting("Homepage");
	else
		this->SetSettingString("Homepage", homepage);
	::mir_free(homepage);	
}

void CSkypeProto::UpdateOwnLanguages()
{
	// languages (en, ru, etc), space searated
	SEString data;
	this->account->GetPropLanguages(data);
	char* isocode = ::mir_utf8decodeA((const char*)data);
	if (strcmp(isocode, "") == 0)
	{
		this->DeleteSetting("Language1");	
	}
	else
	{
		for (int i = 0; i < SIZEOF(CSkypeProto::languages); i++)
			if ( lstrcmpiA((char*)isocode, CSkypeProto::languages[i].ISOcode) == 0)
			{
				this->SetSettingString("Language1", ::mir_a2u(CSkypeProto::languages[i].szName));
				break;
			}
	}
	::mir_free(isocode);
}

void CSkypeProto::UpdateOwnMobilePhone()
{
	SEString data;
	this->account->GetPropPhoneMobile(data);
	wchar_t* phone = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(phone, L"") == 0)
		this->DeleteSetting("Cellular");
	else
		this->SetSettingString("Cellular", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateOwnNickName()
{
	SEString data;
	this->account->GetPropFullname(data);
	wchar_t* nick = ::mir_utf8decodeW((const char*)data);
	this->SetSettingString("Nick", nick);
	::mir_free(nick);
}

void CSkypeProto::UpdateOwnPhone()
{
	SEString data;
	this->account->GetPropPhoneHome(data);
	wchar_t* phone = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(phone, L"") == 0)
		this->DeleteSetting("Phone");
	else
		this->SetSettingString("Phone", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateOwnOfficePhone()
{
	SEString data;
	this->account->GetPropPhoneOffice(data);
	wchar_t* phone = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(phone, L"") == 0)
		this->DeleteSetting("CompanyPhone");
	else
		this->SetSettingString("CompanyPhone", phone);		
	::mir_free(phone);
}

void CSkypeProto::UpdateOwnState()
{
	SEString data;
	this->account->GetPropProvince(data);
	wchar_t* state = ::mir_utf8decodeW((const char*)data);
	if (wcscmp(state, L"") == 0)
		this->DeleteSetting("State");
	else
		this->SetSettingString("State", state);		
	::mir_free(state);
}

void CSkypeProto::UpdateOwnStatusMessage()
{
	uint newTS = 0;
	this->account->GetPropMoodTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword("XStatusTS");
	if (newTS > oldTS)
	{
		SEString data;
		this->account->GetPropMoodText(data);
		wchar_t* status = ::mir_utf8decodeW((const char*)data);
		if (wcscmp(status, L"") == 0)
			this->DeleteSetting("XStatusMsg");
		else
			this->SetSettingString("XStatusMsg", status);
		::mir_free(status);
		this->SetSettingDword("XStatusTS", newTS);
	}
}

void CSkypeProto::UpdateOwnTimezone()
{
	uint data;
	this->account->GetPropTimezone(data);
	if (data > 0)
	{
		uint diffmin = (data - 24*3600) / 60;
		wchar_t sign[2];
		if (diffmin < 0)
			::wcscpy(sign, L"-");
		else
			::wcscpy(sign, L"+");
		uint hours = ::abs((int)(diffmin / 60));
		uint mins = ::abs((int)(diffmin % 60));
		wchar_t timeshift[7];
		::mir_sntprintf(timeshift, SIZEOF(timeshift), _T("%s%d:%02d"), sign, hours, mins);
			
		wchar_t *szMin = wcschr(timeshift, ':');
		int nTz = ::_wtoi(timeshift) * -2;
		nTz += (nTz < 0 ? -1 : 1) * (szMin ? _ttoi( szMin + 1 ) / 30 : 0);

		TIME_ZONE_INFORMATION tzinfo;
		if (::GetTimeZoneInformation(&tzinfo) == TIME_ZONE_ID_DAYLIGHT)
			nTz -= tzinfo.DaylightBias / 30;

		this->SetSettingByte("Timezone", (signed char)nTz);
	}
	else
		this->DeleteSetting("TimeZone");
}

void CSkypeProto::UpdateOwnProfile()
{
	uint newTS = 0;
	this->account->GetPropProfileTimestamp(newTS);
	DWORD oldTS = this->GetSettingDword("ProfileTS");
	if (newTS > oldTS)
	{
		this->UpdateOwnAvatar();
		this->UpdateOwnBirthday();
		this->UpdateOwnCity();
		this->UpdateOwnCountry();
		this->UpdateOwnEmails();
		this->UpdateOwnGender();
		this->UpdateOwnHomepage();
		this->UpdateOwnLanguages();
		this->UpdateOwnMobilePhone();
		this->UpdateOwnNickName();
		this->UpdateOwnPhone();
		this->UpdateOwnOfficePhone();
		this->UpdateOwnState();
		this->UpdateOwnStatusMessage();
		this->UpdateOwnTimezone();

		this->SetSettingDword("ProfileTS", newTS);
	}
}

void CSkypeProto::OnAccountChanged(int prop)
{
	switch(prop)
	{
	case CAccount::P_AVATAR_IMAGE:
	case CAccount::P_AVATAR_TIMESTAMP:
		this->UpdateOwnAvatar();
		break;
	case CAccount::P_BIRTHDAY:
		this->UpdateOwnBirthday();
		break;
	case CAccount::P_CITY:
		this->UpdateOwnCity();
		break;
	case CAccount::P_COUNTRY:
		this->UpdateOwnCountry();
		break;
	case CAccount::P_EMAILS:
		this->UpdateOwnEmails();
		break;
	case CAccount::P_GENDER:
		this->UpdateOwnGender();
		break;
	case CAccount::P_HOMEPAGE:
		this->UpdateOwnHomepage();
		break;
	case CAccount::P_LANGUAGES:
		this->UpdateOwnLanguages();
		break;
	case CAccount::P_MOOD_TEXT:
	case CAccount::P_MOOD_TIMESTAMP:
		this->UpdateOwnStatusMessage();
		break;
	case CAccount::P_PHONE_HOME:
		this->UpdateOwnPhone();
		break;
	case CAccount::P_PHONE_MOBILE:
		this->UpdateOwnMobilePhone();
		break;
	case CAccount::P_PHONE_OFFICE:
		this->UpdateOwnOfficePhone();
		break;
	case CAccount::P_PROFILE_TIMESTAMP:
		this->UpdateOwnProfile();
		break;
	case CAccount::P_PROVINCE:
		this->UpdateOwnState();
		break;
	case CAccount::P_TIMEZONE:
		this->UpdateOwnTimezone();
		break;
	case CAccount::P_FULLNAME:
		this->UpdateOwnNickName();
		break;
	}
}

void __cdecl CSkypeProto::LoadOwnInfo(void*)
{
	this->UpdateOwnProfile();	
}