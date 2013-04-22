#include "skype_proto.h"

SettingItem CSkypeProto::setting[] = {
  {LPGENT("Full name"),		"Nick",			DBVT_WCHAR,	LI_STRING},
  {LPGENT("Mood"),			"XStatusMsg",	DBVT_WCHAR,	LI_STRING},

  {LPGENT("Mobile phone"),	"Cellular",		DBVT_WCHAR,	LI_NUMBER},
  {LPGENT("Home phone"),	"Phone",		DBVT_WCHAR,	LI_NUMBER},
  {LPGENT("Office phone"),	"CompanyPhone",	DBVT_WCHAR,	LI_NUMBER},
  {LPGENT("E-mail 1"),		"e-mail0",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("E-mail 2"),		"e-mail1",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("E-mail 3"),		"e-mail2",		DBVT_WCHAR,	LI_STRING},

  {LPGENT("Country"),		"Country",		DBVT_WCHAR,	LI_LIST},
  {LPGENT("State"),			"State",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("City"),			"City",			DBVT_WCHAR,	LI_STRING},
  {LPGENT("Time zone"),		"Timezone",		DBVT_BYTE,	LI_LIST},
  {LPGENT("Homepage"),		"Homepage",		DBVT_WCHAR,	LI_STRING},
  {LPGENT("Gender"),		"Gender",		DBVT_BYTE,	LI_LIST},
  {LPGENT("Birth day"),		"BirthDay",		DBVT_BYTE,	LI_NUMBER},
  {LPGENT("Birth month"),	"BirthMonth",	DBVT_BYTE,	LI_NUMBER},
  {LPGENT("Birth year"),	"BirthYear",	DBVT_WORD,	LI_NUMBER},
  {LPGENT("Language"),		"Language1",	DBVT_WCHAR,	LI_LIST},

  {LPGENT("About"),			"About",		DBVT_WCHAR,	LI_STRING}
};

void CSkypeProto::UpdateProfileAvatar(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* *::P_AVATAR_TIMESTAMP */ 182);
	DWORD oldTS = ::db_get_dw(hContact, this->m_szModuleName, "AvatarTS", 0);
	
	wchar_t *path = this->GetContactAvatarFilePath(hContact);
	SEBinary data = obj->GetBinProp(/* *::P_AVATAR_IMAGE */ 37);

	bool hasNewAvatar = newTS > oldTS;
	bool isAvatarEmpty = data.size() == 0;
	bool isAvatarFileExists = ::PathFileExists(path) > 0;
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

				if (hContact)
				{
					PROTO_AVATAR_INFORMATIONW pai = {0};
					pai.cbSize = sizeof(pai);
					pai.format = PA_FORMAT_JPEG;
					pai.hContact = hContact;
					::wcscpy(pai.filename, path);
						
					this->SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
				}
				else
				{
					::mir_md5_byte_t digest[16];
					::mir_md5_hash((BYTE*)data.data(), data.size(), digest);
					::db_set_blob(hContact, this->m_szModuleName, "AvatarHash", digest, 16);

					::CallService(MS_AV_SETMYAVATART, (WPARAM)m_szModuleName, (LPARAM)path);
				}
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
		::db_set_ws(hContact, this->m_szModuleName, "About", aboutText);
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
		::db_set_ws(hContact, this->m_szModuleName, "City", city);
	::mir_free(city);
}

void CSkypeProto::UpdateProfileCountry(SEObject *obj, HANDLE hContact)
{
	char* country;
	char* isocode = ::mir_strdup(obj->GetStrProp(/* *::P_COUNTRY */ 10));
	if ( !::strlen(isocode))
	{
		country = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, 0xFFFF, 0);
		::db_set_ws(hContact, this->m_szModuleName, "Country", ::mir_a2t(country));
	}
	else
	{
		country = (char*)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode, 0);
		::db_set_ws(hContact, this->m_szModuleName, "Country", ::mir_a2t(country));
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
			if (p) ::db_set_ws(hContact, this->m_szModuleName, "e-mail1", p);
			p = wcstok(NULL, L" ");
			if (p) ::db_set_ws(hContact, this->m_szModuleName, "e-mail2", p);
		}
		::mir_free(p);
	}
	::mir_free(emails);
}

void CSkypeProto::UpdateProfileFullName(SEObject *obj, HANDLE hContact)
{
	wchar_t *fullname = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_FULLNAME */ 5));
	if ( !::wcslen(fullname))
	{
		this->DeleteSetting(hContact, "FirstName");
		this->DeleteSetting(hContact, "LastName");
	}
	else
	{
		wchar_t *firstName = ::wcstok(fullname, L" ");
		wchar_t *lastName = ::wcstok(NULL, L" ");
		if (lastName == NULL)
		{
			lastName = L"";
		}
		::db_set_ws(hContact, this->m_szModuleName, "FirstName", firstName);
		::db_set_ws(hContact, this->m_szModuleName, "LastName", lastName);
	}
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
		::db_set_ws(hContact, this->m_szModuleName, "Homepage", homepage);
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
			::db_set_ws(hContact, this->m_szModuleName, "Language1", isocodes);
		}
		else
		{
			::db_set_ws(hContact, this->m_szModuleName, "Language1", p);
			p = wcstok(NULL, L" ");
			if (p) ::db_set_ws(hContact, this->m_szModuleName, "Language2", p);
			p = wcstok(NULL, L" ");
			if (p) ::db_set_ws(hContact, this->m_szModuleName, "Language3", p);
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
		::db_set_ws(hContact, this->m_szModuleName, "Cellular", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateProfilePhone(SEObject *obj, HANDLE hContact)
{
	wchar_t* phone = ::mir_a2u(obj->GetStrProp(/* *::P_PHONE_HOME */ 13));
	if ( !::wcslen(phone))
		this->DeleteSetting(hContact, "Phone");
	else
		::db_set_ws(hContact, this->m_szModuleName, "Phone", phone);
	::mir_free(phone);
}

void CSkypeProto::UpdateProfileOfficePhone(SEObject *obj, HANDLE hContact)
{
	wchar_t* phone = ::mir_a2u(obj->GetStrProp(/* *::P_PHONE_OFFICE */ 14));
	if ( !::wcslen(phone))
		this->DeleteSetting(hContact, "CompanyPhone");
	else
		::db_set_ws(hContact, this->m_szModuleName, "CompanyPhone", phone);		
	::mir_free(phone);
}

void CSkypeProto::UpdateProfileState(SEObject *obj, HANDLE hContact)
{
	wchar_t* state = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_PROVINCE */ 11));
	if ( !::wcslen(state))
		this->DeleteSetting(hContact, "State");
	else
		::db_set_ws(hContact, this->m_szModuleName, "State", state);		
	::mir_free(state);
}

void CSkypeProto::UpdateProfileStatusMessage(SEObject *obj, HANDLE hContact)
{
	wchar_t* statusMessage = ::mir_utf8decodeW(obj->GetStrProp(/* *::P_MOOD_TEXT */ 26));
	if ( !::wcslen(statusMessage))
		this->DeleteSetting(hContact, "XStatusMsg");
	else
		::db_set_ws(hContact, this->m_szModuleName, "XStatusMsg", statusMessage);
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
	this->UpdateProfileAvatar(obj, hContact);	

	uint newTS = obj->GetUintProp(/* *::P_PROFILE_TIMESTAMP */ 19);
	if (newTS > ::db_get_dw(hContact, this->m_szModuleName, "ProfileTS", 0))
	{
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
		this->UpdateProfileStatusMessage(obj, hContact);
		this->UpdateProfileTimezone(obj, hContact);

		if (hContact)
		{
			this->UpdateContactOnlineSinceTime(obj, hContact);
			this->UpdateContactLastEventDate(obj, hContact);

			::db_set_ws(hContact, this->m_szModuleName, "MirVer", L"Skype");
		}

		this->SetSettingDword("ProfileTS", newTS);
	}
}

void __cdecl CSkypeProto::LoadOwnInfo(void*)
{
	wchar_t *nick = ::db_get_wsa(NULL, this->m_szModuleName, "Nick");
	if (!nick || !::wcslen(nick))
	{
		::db_set_ws(NULL, this->m_szModuleName, "Nick", this->login);
	}
	this->UpdateProfile(this->account.fetch());
}