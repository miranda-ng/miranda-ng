#include "skype.h"
#include <sstream>

void CSkypeProto::UpdateProfileAvatar(SEObject *obj, HANDLE hContact)
{
	uint newTS = obj->GetUintProp(/* *::P_AVATAR_TIMESTAMP */ 182);
	//if (!newTS) return; //uncomment when skypekit will be work correctly

	DWORD oldTS = ::db_get_dw(hContact, this->m_szModuleName, "AvatarTS", 0);

	ptrW path( this->GetContactAvatarFilePath(hContact));
	bool isAvatarFileExists = CSkypeProto::FileExists(path);
	if (newTS > oldTS || !isAvatarFileExists)
	{
		SEBinary data = obj->GetBinProp(/* *::P_AVATAR_IMAGE */ 37);
		if (data.size() > 0)
		{
			FILE *fp = ::_wfopen(path, L"wb");
			if (fp)
			{
				::fwrite(data.data(), sizeof(char), data.size(), fp);
				::fclose(fp);

				::db_set_dw(hContact, this->m_szModuleName, "AvatarTS", newTS);

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
					::mir_md5_hash((BYTE*)data.data(), (int)data.size(), digest);
					::db_set_blob(hContact, this->m_szModuleName, "AvatarHash", digest, 16);

					::CallService(MS_AV_SETMYAVATART, (WPARAM)m_szModuleName, (LPARAM)path);
				}
			}
		}
		else if (isAvatarFileExists)
		{
			::_wremove(path);
			this->SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, 0);
		}
	}
}

void CSkypeProto::UpdateProfileAboutText(SEObject *obj, HANDLE hContact)
{
	ptrW aboutText(::mir_utf8decodeW(obj->GetStrProp(/* *::P_ABOUT */ 18)));
	if ( !::wcslen(aboutText))
		::db_unset(hContact, this->m_szModuleName, "About");
	else
		::db_set_ws(hContact, this->m_szModuleName, "About", aboutText);
}

void CSkypeProto::UpdateProfileBirthday(SEObject *obj, HANDLE hContact)
{
	uint data = obj->GetUintProp(/* *::P_BIRTHDAY */ 7);
	if (data > 0)
	{
		TCHAR date[20];
		_itot_s(data, date, 10);

		INT day, month, year;
		_stscanf(date, _T("%04d%02d%02d"), &year, &month, &day);

		SYSTEMTIME sToday = {0};
		GetLocalTime(&sToday);

		if (sToday.wYear > year) return;
		else if(sToday.wYear == year && sToday.wMonth > month) return;
		else if(sToday.wYear == year && sToday.wMonth == month && sToday.wDay >= day) return;

		::db_set_b(hContact, this->m_szModuleName, "BirthDay", day);
		::db_set_b(hContact, this->m_szModuleName, "BirthMonth", month);
		::db_set_w(hContact, this->m_szModuleName, "BirthYear", year);

		int nAge = sToday.wYear - year;
		if (sToday.wMonth < month || (sToday.wMonth == month && sToday.wDay < day))
			nAge--;
		if (nAge)
			::db_set_w(hContact, this->m_szModuleName, "Age", ( WORD )nAge );
	}
	else
	{
		::db_unset(hContact, this->m_szModuleName, "BirthDay");
		::db_unset(hContact, this->m_szModuleName, "BirthMonth");
		::db_unset(hContact, this->m_szModuleName, "BirthYear");
		::db_unset(hContact, this->m_szModuleName, "Age");
	}
}

void CSkypeProto::UpdateProfileCity(SEObject *obj, HANDLE hContact)
{
	ptrW city(::mir_utf8decodeW(obj->GetStrProp(/* *::P_CITY */ 12)));
	if ( !::wcslen(city))
		::db_unset(hContact, this->m_szModuleName, "City");
	else
		::db_set_ws(hContact, this->m_szModuleName, "City", city);
}

void CSkypeProto::UpdateProfileCountry(SEObject *obj, HANDLE hContact)
{
	char *country;
	ptrA isocode(::mir_strdup(obj->GetStrProp(/* *::P_COUNTRY */ 10)));
	if ( !::strlen(isocode))
	{
		::db_unset(hContact, this->m_szModuleName, "Country");
	}
	else
	{
		country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode, 0);
		::db_set_ws(hContact, this->m_szModuleName, "Country", _A2T(country));
	}
}

void CSkypeProto::UpdateProfileEmails(SEObject *obj, HANDLE hContact)
{
	ptrW emails(::mir_utf8decodeW(obj->GetStrProp(/* *::P_EMAILS */ 16)));
	if (::wcscmp(emails, L"") == 0)
	{
		::db_unset(hContact, this->m_szModuleName, "e-mail0");
		::db_unset(hContact, this->m_szModuleName, "e-mail1");
		::db_unset(hContact, this->m_szModuleName, "e-mail2");
	}
	else
	{
		StringList emls = emails;
		for (size_t i = 0; i < emls.size(); i++)
		{
			std::stringstream ss;
			ss << "e-mail" << i;
			std::string key = ss.str();
		
			::db_set_ws(hContact, this->m_szModuleName, key.c_str(), emls[i]);
		}
	}
}

void CSkypeProto::UpdateProfileFullName(SEObject *obj, HANDLE hContact)
{
	ptrW fullname(::mir_utf8decodeW(obj->GetStrProp(/* *::P_FULLNAME */ 5)));
	if ( !::wcslen(fullname))
	{
		::db_unset(hContact, this->m_szModuleName, "FirstName");
		::db_unset(hContact, this->m_szModuleName, "LastName");
	}
	else
	{
		StringList names = fullname;

		::db_set_ws(hContact, this->m_szModuleName, "FirstName", names[0]);
		if (names.size() > 1)
			::db_set_ws(hContact, this->m_szModuleName, "LastName", names[1]);
	}
}

void CSkypeProto::UpdateProfileGender(SEObject *obj, HANDLE hContact)
{
	uint data = obj->GetUintProp(/* *::P_GENDER */ 8);
	if (data)
		::db_set_b(hContact, this->m_szModuleName, "Gender", (BYTE)(data == 1 ? 'M' : 'F'));
	else
		::db_unset(hContact, this->m_szModuleName, "Gender");
}

void CSkypeProto::UpdateProfileHomepage(SEObject *obj, HANDLE hContact)
{
	ptrW homepage(::mir_utf8decodeW(obj->GetStrProp(/* *::P_HOMEPAGE */ 17)));
	if (::wcscmp(homepage, L"") == 0)
		::db_unset(hContact, this->m_szModuleName, "Homepage");
	else
		::db_set_ws(hContact, this->m_szModuleName, "Homepage", homepage);
}

void CSkypeProto::UpdateProfileLanguages(SEObject *obj, HANDLE hContact)
{
	ptrW isocodes(::mir_utf8decodeW(obj->GetStrProp(/* *::P_LANGUAGES */ 9)));

	::db_unset(hContact, this->m_szModuleName, "Language1");
	::db_unset(hContact, this->m_szModuleName, "Language2");
	::db_unset(hContact, this->m_szModuleName, "Language3");

	StringList langs = isocodes;
	for (size_t i = 0; i < langs.size(); i++)
	{
		if (CSkypeProto::languages.count(langs[i]))
		{
			std::stringstream ss;
			ss << "Language" << i + 1;
			std::string key = ss.str();
			std::wstring val = CSkypeProto::languages[langs[i]];
			::db_set_ws(hContact, this->m_szModuleName, key.c_str(), val.c_str());
		}
	}
}

void CSkypeProto::UpdateProfileMobilePhone(SEObject *obj, HANDLE hContact)
{
	ptrW phone(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PHONE_MOBILE */ 15)));
	if ( !::wcslen(phone))
		::db_unset(hContact, this->m_szModuleName, "Cellular");
	else
		::db_set_ws(hContact, this->m_szModuleName, "Cellular", phone);
}

void CSkypeProto::UpdateProfileNick(SEObject *obj, HANDLE hContact)
{
	ptrW nick;
	if (hContact)
	{
		CContact *contact = (CContact *)obj;
		nick = ::mir_utf8decodeW(contact->GetNick());
	}
	else
		nick = ::mir_utf8decodeW(obj->GetStrProp(Account::P_FULLNAME));

	if ( !::wcslen(nick))
		::db_unset(hContact, this->m_szModuleName, "Nick");
	else
		::db_set_ws(hContact, this->m_szModuleName, "Nick", nick);
}

void CSkypeProto::UpdateProfilePhone(SEObject *obj, HANDLE hContact)
{
	ptrW phone(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PHONE_MOBILE */ 13)));
	if ( !::wcslen(phone))
		::db_unset(hContact, this->m_szModuleName, "Phone");
	else
		::db_set_ws(hContact, this->m_szModuleName, "Phone", phone);
}

void CSkypeProto::UpdateProfileOfficePhone(SEObject *obj, HANDLE hContact)
{
	ptrW phone(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PHONE_OFFICE */ 14)));
	if ( !::wcslen(phone))
		::db_unset(hContact, this->m_szModuleName, "CompanyPhone");
	else
		::db_set_ws(hContact, this->m_szModuleName, "CompanyPhone", phone);		
}

void CSkypeProto::UpdateProfileState(SEObject *obj, HANDLE hContact)
{
	ptrW state(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PROVINCE */ 11)));
	if ( !::wcslen(state))
		::db_unset(hContact, this->m_szModuleName, "State");
	else
		::db_set_ws(hContact, this->m_szModuleName, "State", state);		
}

void CSkypeProto::UpdateProfileStatusMessage(SEObject *obj, HANDLE hContact)
{
	ptrW statusMessage(::mir_utf8decodeW(obj->GetStrProp(/* *::P_MOOD_TEXT */ 26)));
	if ( !::wcslen(statusMessage))
		::db_unset(hContact, this->m_szModuleName, "XStatusMsg");
	else
		::db_set_ws(hContact, this->m_szModuleName, "XStatusMsg", statusMessage);
}

void CSkypeProto::UpdateProfileTimezone(SEObject *obj, HANDLE hContact)
{
	LONG data = obj->GetUintProp(/* *::P_TIMEZONE */ 27);
	if (data > 0)
	{
		LONG diffmin = (data - 24*3600) / 60;
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

		::db_set_b(hContact, this->m_szModuleName, "Timezone", (signed char)nTz);
	}
	else
		::db_unset(hContact, this->m_szModuleName, "Timezone");
}

void CSkypeProto::UpdateProfile(SEObject *obj, HANDLE hContact)
{
	this->Log(L"Updating profile for %p", hContact);
	this->UpdateProfileAvatar(obj, hContact);

	uint newTS = hContact ? obj->GetUintProp(Contact::P_PROFILE_TIMESTAMP) : obj->GetUintProp(Account::P_PROFILE_TIMESTAMP);
	//if (newTS > ::db_get_dw(hContact, this->m_szModuleName, "ProfileTS", 0))
	//{
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
		this->UpdateProfileNick(obj, hContact);
		this->UpdateProfilePhone(obj, hContact);
		this->UpdateProfileOfficePhone(obj, hContact);
		this->UpdateProfileState(obj, hContact);
		this->UpdateProfileStatusMessage(obj, hContact);
		this->UpdateProfileTimezone(obj, hContact);

		if (hContact)
		{
			ContactRef ref(obj->getOID());
			this->UpdateContactClient(hContact, ref);
			this->UpdateContactLastEventDate(hContact, ref);
			this->UpdateContactOnlineSinceTime(hContact, ref);
		}

		::db_set_dw(hContact, this->m_szModuleName, "ProfileTS", newTS);
	//}
}