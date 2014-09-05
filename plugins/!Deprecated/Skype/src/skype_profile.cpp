#include "skype.h"
#include <sstream>

void CSkypeProto::UpdateProfileAvatar(SEObject *obj, MCONTACT hContact)
{
	uint newTS = obj->GetUintProp(/* *::P_AVATAR_TIMESTAMP */ 182);
	//if (!newTS) return; //uncomment when skypekit will be work correctly

	DWORD oldTS = this->getDword(hContact, "AvatarTS", 0);

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

				this->setDword(hContact, "AvatarTS", newTS);

				if (hContact)
				{
					PROTO_AVATAR_INFORMATIONW pai = { sizeof(pai) };
					pai.format = PA_FORMAT_JPEG;
					pai.hContact = hContact;
					::wcscpy(pai.filename, path);
						
					this->SendBroadcast(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
				}
				else
				{
					BYTE digest[16];
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

void CSkypeProto::UpdateProfileAboutText(SEObject *obj, MCONTACT hContact)
{
	ptrW aboutText(::mir_utf8decodeW(obj->GetStrProp(/* *::P_ABOUT */ 18)));
	if ( !::wcslen(aboutText))
		this->delSetting(hContact, "About");
	else
		this->setTString(hContact, "About", aboutText);
}

void CSkypeProto::UpdateProfileBirthday(SEObject *obj, MCONTACT hContact)
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

		this->setByte(hContact, "BirthDay", day);
		this->setByte(hContact, "BirthMonth", month);
		this->setWord(hContact, "BirthYear", year);

		int nAge = sToday.wYear - year;
		if (sToday.wMonth < month || (sToday.wMonth == month && sToday.wDay < day))
			nAge--;
		if (nAge)
			this->setWord(hContact, "Age", (WORD)nAge );
	}
	else
	{
		this->delSetting(hContact, "BirthDay");
		this->delSetting(hContact, "BirthMonth");
		this->delSetting(hContact, "BirthYear");
		this->delSetting(hContact, "Age");
	}
}

void CSkypeProto::UpdateProfileCity(SEObject *obj, MCONTACT hContact)
{
	ptrW city(::mir_utf8decodeW(obj->GetStrProp(/* *::P_CITY */ 12)));
	if ( !::wcslen(city))
		this->delSetting(hContact, "City");
	else
		this->setTString(hContact, "City", city);
}

void CSkypeProto::UpdateProfileCountry(SEObject *obj, MCONTACT hContact)
{
	char *country;
	ptrA isocode(::mir_strdup(obj->GetStrProp(/* *::P_COUNTRY */ 10)));
	if ( !::strlen(isocode))
		this->delSetting(hContact, "Country");
	else
	{
		country = (char *)CallService(MS_UTILS_GETCOUNTRYBYISOCODE, (WPARAM)isocode, 0);
		this->setTString(hContact, "Country", _A2T(country));
	}
}

void CSkypeProto::UpdateProfileEmails(SEObject *obj, MCONTACT hContact)
{
	ptrW emails(::mir_utf8decodeW(obj->GetStrProp(/* *::P_EMAILS */ 16)));
	if (::wcscmp(emails, L"") == 0)
	{
		this->delSetting(hContact, "e-mail0");
		this->delSetting(hContact, "e-mail1");
		this->delSetting(hContact, "e-mail2");
	}
	else
	{
		StringList emls = emails;
		for (size_t i = 0; i < emls.size(); i++)
		{
			std::stringstream ss;
			ss << "e-mail" << i;
			std::string key = ss.str();
		
			this->setTString(hContact, key.c_str(), emls[i]);
		}
	}
}

void CSkypeProto::UpdateProfileFullName(SEObject *obj, MCONTACT hContact)
{
	ptrW fullname(::mir_utf8decodeW(obj->GetStrProp(/* *::P_FULLNAME */ 5)));
	if ( !::wcslen(fullname))
	{
		this->delSetting(hContact, "FirstName");
		this->delSetting(hContact, "LastName");
	}
	else
	{
		StringList names = fullname;

		this->setTString(hContact, "FirstName", names[0]);
		if (names.size() > 1)
			this->setTString(hContact, "LastName", names[1]);
	}
}

void CSkypeProto::UpdateProfileGender(SEObject *obj, MCONTACT hContact)
{
	uint data = obj->GetUintProp(/* *::P_GENDER */ 8);
	if (data)
		this->setByte(hContact, "Gender", (BYTE)(data == 1 ? 'M' : 'F'));
	else
		this->delSetting(hContact, "Gender");
}

void CSkypeProto::UpdateProfileHomepage(SEObject *obj, MCONTACT hContact)
{
	ptrW homepage(::mir_utf8decodeW(obj->GetStrProp(/* *::P_HOMEPAGE */ 17)));
	if (::wcscmp(homepage, L"") == 0)
		this->delSetting(hContact, "Homepage");
	else
		this->setTString(hContact, "Homepage", homepage);
}

void CSkypeProto::UpdateProfileLanguages(SEObject *obj, MCONTACT hContact)
{
	ptrW isocodes(::mir_utf8decodeW(obj->GetStrProp(/* *::P_LANGUAGES */ 9)));

	this->delSetting(hContact, "Language1");
	this->delSetting(hContact, "Language2");
	this->delSetting(hContact, "Language3");

	StringList langs = isocodes;
	for (size_t i = 0; i < langs.size(); i++)
	{
		if (CSkypeProto::languages.count(langs[i]))
		{
			std::stringstream ss;
			ss << "Language" << i + 1;
			std::string key = ss.str();
			std::wstring val = CSkypeProto::languages[langs[i]];
			this->setTString(hContact, key.c_str(), val.c_str());
		}
	}
}

void CSkypeProto::UpdateProfileMobilePhone(SEObject *obj, MCONTACT hContact)
{
	ptrW phone(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PHONE_MOBILE */ 15)));
	if ( !::wcslen(phone))
		this->delSetting(hContact, "Cellular");
	else
		this->setTString(hContact, "Cellular", phone);
}

void CSkypeProto::UpdateProfileNick(SEObject *obj, MCONTACT hContact)
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
		this->delSetting(hContact, "Nick");
	else
		this->setTString(hContact, "Nick", nick);
}

void CSkypeProto::UpdateProfilePhone(SEObject *obj, MCONTACT hContact)
{
	ptrW phone(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PHONE_MOBILE */ 13)));
	if ( !::wcslen(phone))
		this->delSetting(hContact, "Phone");
	else
		this->setTString(hContact, "Phone", phone);
}

void CSkypeProto::UpdateProfileOfficePhone(SEObject *obj, MCONTACT hContact)
{
	ptrW phone(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PHONE_OFFICE */ 14)));
	if ( !::wcslen(phone))
		this->delSetting(hContact, "CompanyPhone");
	else
		this->setTString(hContact, "CompanyPhone", phone);		
}

void CSkypeProto::UpdateProfileState(SEObject *obj, MCONTACT hContact)
{
	ptrW state(::mir_utf8decodeW(obj->GetStrProp(/* *::P_PROVINCE */ 11)));
	if ( !::wcslen(state))
		this->delSetting(hContact, "State");
	else
		this->setTString(hContact, "State", state);		
}

void CSkypeProto::UpdateProfileStatusMessage(SEObject *obj, MCONTACT hContact)
{
	ptrW statusMessage(::mir_utf8decodeW(obj->GetStrProp(/* *::P_MOOD_TEXT */ 26)));
	if ( !::wcslen(statusMessage))
		this->delSetting(hContact, "XStatusMsg");
	else
		this->setTString(hContact, "XStatusMsg", statusMessage);
}

void CSkypeProto::UpdateProfileTimezone(SEObject *obj, MCONTACT hContact)
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

		this->setByte(hContact, "Timezone", (signed char)nTz);
	}
	else this->delSetting(hContact, "Timezone");
}

void CSkypeProto::UpdateProfile(SEObject *obj, MCONTACT hContact)
{
	this->debugLogW(L"Updating profile for %p", hContact);
	this->UpdateProfileAvatar(obj, hContact);

	uint newTS = hContact ? obj->GetUintProp(Contact::P_PROFILE_TIMESTAMP) : obj->GetUintProp(Account::P_PROFILE_TIMESTAMP);
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

	this->setDword(hContact, "ProfileTS", newTS);
}