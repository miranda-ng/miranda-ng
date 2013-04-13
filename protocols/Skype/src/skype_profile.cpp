#include "skype_proto.h"

void CSkypeProto::UpdateProfileAvatar(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* *::P_AVATAR_TIMESTAMP */ 182);
	DWORD oldTS = this->GetSettingDword(hContact, "AvatarTS");
	
	wchar_t *path = this->GetContactAvatarFilePath(hContact);
	SEBinary data = obj->GetBinProp(/* *::P_AVATAR_IMAGE */ 37);

	//if ((newTS > oldTS) || (!newTS && data.size() > 0 && _waccess(path, 0) == -1) || (newTS && _waccess(path, 0) == -1)) //hack for avatars without timestamp
	bool hasNewAvatar = newTS > oldTS;
	bool isAvatarEmpty = data.size() == 0;
	bool isAvatarFileExists = ::PathFileExists(path);
	if ( !isAvatarEmpty)
	{
		if (hasNewAvatar || !isAvatarFileExists)
		{
			FILE* fp = ::_wfopen(path, L"wb");
			if (fp)
			{
				::fwrite(data.data(), sizeof(char), data.size(), fp);
				::fclose(fp);

				this->SetSettingDword("AvatarTS", newTS);

				PROTO_AVATAR_INFORMATIONW pai = {0};
				pai.cbSize = sizeof(pai);
				pai.format = PA_FORMAT_JPEG;
				pai.hContact = hContact;
				::wcscpy(pai.filename, path);
		
				this->SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
			}
		}
	}
	else if (isAvatarFileExists)
	{
		::_wremove(path);
		this->SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, 0);
	}

	delete [] path;
}

void CSkypeProto::UpdateProfileAboutText(SEObject *obj, HANDLE hContact)
{
	wchar_t* aboutText = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_ABOUT */ 18));
	if ( !::wcslen(aboutText))
		this->DeleteSetting(hContact, "About");
	else
		this->SetSettingString(hContact, "About", aboutText);
	::mir_free(aboutText);
}

void CSkypeProto::UpdateProfileBirthday(SEObject *obj, HANDLE hContact)
{
	uint data = obj->GetUintProp(/* *::P_BIRTHDAY */ 7);
	if (data > 0)
	{
		TCHAR date[9];
		_itot_s(data, date, 10);

		INT day, month, year;
		_stscanf(date, _T("%04d%02d%02d"), &year, &month, &day);

		this->SetSettingByte(hContact, "BirthDay", day);
		this->SetSettingByte(hContact, "BirthMonth", month);
		this->SetSettingWord(hContact, "BirthYear", year);

		SYSTEMTIME sToday = {0};
		GetLocalTime(&sToday);
		int nAge = sToday.wYear - year;
		if (sToday.wMonth < month || (sToday.wMonth == month && sToday.wDay < day))
			nAge--;
		if (nAge)
			this->SetSettingWord( hContact, "Age", ( WORD )nAge );
	}
	else
	{
		this->DeleteSetting(hContact, "BirthDay");
		this->DeleteSetting(hContact, "BirthMonth");
		this->DeleteSetting(hContact, "BirthYear");
		this->DeleteSetting(hContact, "Age");
	}
}

void CSkypeProto::UpdateProfileCity(SEObject *obj, HANDLE hContact)
{
	wchar_t* city = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_CITY */ 12));
	if ( !::wcslen(city))
		this->DeleteSetting(hContact, "City");
	else
		this->SetSettingString(hContact, "City", city);
	::mir_free(city);
}

void CSkypeProto::UpdateProfileCountry(SEObject *obj, HANDLE hContact)
{
	char* country;
	char* isocode = ::mir_strdup(obj->GetStrProp(/* *::P_COUNTRY */ 10));
	if ( !::strlen(isocode))
	{
		country = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, 0xFFFF, 0);
		this->SetSettingString(hContact, "Country", ::mir_a2t(country));
	}
	else
	{
		country = (char*)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode, 0);
		this->SetSettingString(hContact, "Country", ::mir_a2t(country));
	}
	::mir_free(isocode);
}

void CSkypeProto::UpdateProfileEmails(SEObject *obj, HANDLE hContact)
{
	wchar_t* emails = ::mir_a2u(obj->GetStrProp(/* *::P_EMAILS */ 16));
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
		::mir_free(p);
	}
	::mir_free(emails);
}

void CSkypeProto::UpdateProfileFullName(SEObject *obj, HANDLE hContact)
{
	wchar_t* fullname = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_FULLNAME */ 5));
	if ( !::wcslen(fullname))
		this->DeleteSetting(hContact, "LastName");
	else
		this->SetSettingString(hContact, "LastName", fullname);
	::mir_free(fullname);
}

void CSkypeProto::UpdateProfileGender(SEObject *obj, HANDLE hContact)
{
	uint data = obj->GetUintProp(/* *::P_GENDER */ 8);
	if (data)
		this->SetSettingByte(hContact, "Gender", (BYTE)(data == 1 ? 'M' : 'F'));
	else
		this->DeleteSetting(hContact, "Gender");
}

void CSkypeProto::UpdateProfileHomepage(SEObject *obj, HANDLE hContact)
{
	wchar_t* homepage = ::mir_a2u(obj->GetStrProp(/* *::P_HOMEPAGE */ 17));
	if (::wcscmp(homepage, L"") == 0)
		this->DeleteSetting(hContact, "Homepage");
	else
		this->SetSettingString(hContact, "Homepage", homepage);
	::mir_free(homepage);	
}

void CSkypeProto::UpdateProfileLanguages(SEObject *obj, HANDLE hContact)
{
	wchar_t *isocodes = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_LANGUAGES */ 9));
	if ( !::wcslen(isocodes))
	{
		this->DeleteSetting(hContact, "Language1");
		this->DeleteSetting(hContact, "Language2");
		this->DeleteSetting(hContact, "Language3");
	}
	else
	{
		wchar_t* p = wcstok(isocodes, L" ");
		if (p == NULL)
		{
			this->SetSettingString(hContact, "e-Language1", isocodes);
		}
		else
		{
			this->SetSettingString(hContact, "e-Language1", p);
			p = wcstok(NULL, L" ");
			if (p) this->SetSettingString(hContact, "e-Language2", p);
			p = wcstok(NULL, L" ");
			if (p) this->SetSettingString(hContact, "e-Language3", p);
		}

		// todo: fix
		/*for (int i = 0; i < SIZEOF(CSkypeProto::languages); i++)
			if ( ::stricmp((char*)isocode, CSkypeProto::languages[i].ISOcode) == 0)
			{
				this->SetSettingString(hContact, "Language1", ::mir_a2u(CSkypeProto::languages[i].szName));
				break;
			}*/
	}
	::mir_free(isocodes);
}

void CSkypeProto::UpdateProfileMobilePhone(SEObject *obj, HANDLE hContact)
{
	wchar_t* phone = ::mir_a2u(obj->GetStrProp(/* *::P_PHONE_MOBILE */ 15));
	if ( !::wcslen(phone))
		this->DeleteSetting(hContact, "Cellular");
	else
		this->SetSettingString(hContact, "Cellular", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateProfilePhone(SEObject *obj, HANDLE hContact)
{
	wchar_t* phone = ::mir_a2u(obj->GetStrProp(/* *::P_PHONE_HOME */ 13));
	if ( !::wcslen(phone))
		this->DeleteSetting(hContact, "Phone");
	else
		this->SetSettingString(hContact, "Phone", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateProfileOfficePhone(SEObject *obj, HANDLE hContact)
{
	wchar_t* phone = ::mir_a2u(obj->GetStrProp(/* *::P_PHONE_OFFICE */ 14));
	if ( !::wcslen(phone))
		this->DeleteSetting(hContact, "CompanyPhone");
	else
		this->SetSettingString(hContact, "CompanyPhone", phone);		
	::mir_free(phone);
}

void CSkypeProto::UpdateProfileState(SEObject *obj, HANDLE hContact)
{
	wchar_t* state = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_PROVINCE */ 11));
	if ( !::wcslen(state))
		this->DeleteSetting(hContact, "State");
	else
		this->SetSettingString(hContact, "State", state);		
	::mir_free(state);
}

void CSkypeProto::UpdateProfileStatusMessage(SEObject *obj, HANDLE hContact)
{
	wchar_t* statusMessage = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_MOOD_TEXT */ 26));
	if ( !::wcslen(statusMessage))
		this->DeleteSetting(hContact, "XStatusMsg");
	else
		this->SetSettingString(hContact, "XStatusMsg", statusMessage);
	::mir_free(statusMessage);
}

void CSkypeProto::UpdateProfileTimezone(SEObject *obj, HANDLE hContact)
{
	uint data = obj->GetUintProp(/* *::P_TIMEZONE */ 27);
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

		this->SetSettingByte(hContact, "Timezone", (signed char)nTz);
	}
	else
		this->DeleteSetting(hContact, "TimeZone");
}

void CSkypeProto::UpdateProfile(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* *::P_PROFILE_TIMESTAMP */ 19);
	if (newTS > this->GetSettingDword("ProfileTS"))
	{
		this->UpdateProfileAvatar(obj, hContact);
		this->UpdateProfileAboutText(obj, hContact);
		this->UpdateProfileBirthday(obj, hContact);
		this->UpdateProfileCity(obj, hContact);
		this->UpdateProfileCountry(obj, hContact);
		this->UpdateProfileEmails(obj, hContact);
		this->UpdateProfileFullName(obj, hContact);
		this->UpdateProfileGender(obj, hContact);
		this->UpdateProfileHomepage(obj, hContact);
		this->UpdateProfileLanguages(obj, hContact);
		this->UpdateProfileMobilePhone(obj, hContact);
		this->UpdateProfilePhone(obj, hContact);
		this->UpdateProfileOfficePhone(obj, hContact);
		this->UpdateProfileState(obj, hContact);
		this->UpdateProfileTimezone(obj, hContact);

		if (hContact)
		{
			this->UpdateContactOnlineSinceTime(obj, hContact);
			this->UpdateContactLastEventDate(obj, hContact);

			this->SetSettingString(hContact, "MirVer", L"Skype");
		}

		this->SetSettingDword("ProfileTS", newTS);
	}
}

void __cdecl CSkypeProto::LoadOwnInfo(void*)
{
	this->UpdateProfile(this->account.fetch());
}