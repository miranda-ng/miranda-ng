#include "skype.h"

void __cdecl CSkypeProto::LoadOwnInfo(void *)
{
	/*ptrW nick( ::db_get_wsa(NULL, this->m_szModuleName, "Nick"));
	if ( !nick)
	{
		SEString data;
		this->account->GetPropFullname(data);

		if (data.length() == 0)
		{
			this->account->GetPropSkypename(data);
		}

		nick = ::mir_utf8decodeW(data);
		::db_set_ws(NULL, this->m_szModuleName, "Nick", nick);
	}*/

	this->UpdateProfile(this->account.fetch());
}

INT_PTR __cdecl CSkypeProto::SetMyNickName(WPARAM wParam, LPARAM lParam)
{
	wchar_t *nick = (wParam & SMNN_UNICODE) ? ::mir_wstrdup((wchar_t *)lParam) : ::mir_a2t((char*) lParam);

	//::db_set_ws(NULL, this->m_szModuleName, "Nick", nick);
	
	if ( !this->IsOnline())
		return 0;

	this->account->SetStrProperty(Account::P_FULLNAME, (char *)ptrA(::mir_utf8encodeW(nick)));
	
	return 0;
}

void CSkypeProto::SaveOwnInfoToServer(HWND hwndPage, int iPage)
{
	wchar_t text[2048];

	switch (iPage) 
	{
	// Page 0: Personal
	case 0:
		{
			::GetDlgItemText(hwndPage, IDC_FULLNAME, text, SIZEOF(text));
			if (this->account->SetStrProperty(Account::P_FULLNAME, (char *)ptrA(::mir_utf8encodeW(text))))
				this->setTString("Nick", text);

			::GetDlgItemText(hwndPage, IDC_MOOD, text, SIZEOF(text));
			this->account->SetStrProperty(Account::P_MOOD_TEXT, (char *)ptrA(::mir_utf8encodeW(text)));

			::GetDlgItemText(hwndPage, IDC_ABOUT, text, SIZEOF(text));
			this->account->SetStrProperty(Account::P_ABOUT, (char *)ptrA(::mir_utf8encodeW(text)));

			::GetDlgItemText(hwndPage, IDC_HOMEPAGE, text, SIZEOF(text));
			this->account->SetStrProperty(Account::P_HOMEPAGE, (char *)ptrA(::mir_utf8encodeW(text)));

			this->account->SetIntProperty(
				Account::P_GENDER, 
				::SendMessage(::GetDlgItem(hwndPage, IDC_GENDER), CB_GETCURSEL, 0, 0));

			char day[3], month[3], year[5], date[9];
			::GetDlgItemTextA(hwndPage, IDC_BIRTH_DAY, day, 3);
			::GetDlgItemTextA(hwndPage, IDC_BIRTH_MONTH, month, 3);
			::GetDlgItemTextA(hwndPage, IDC_BIRTH_YEAR, year, 5);
			::mir_snprintf(date, 9, "%s%s%s", year, month, day);
			int value = atoi(date);
			this->account->SetIntProperty(Account::P_BIRTHDAY, value);

			int lang = ::SendMessage(GetDlgItem(hwndPage, IDC_LANGUAGE), CB_GETCURSEL, 0, 0);
			if (lang != -1) {
				std::wstring key = *(std::wstring *)SendMessage(GetDlgItem(hwndPage, IDC_LANGUAGE), CB_GETITEMDATA, lang, 0);
				this->account->SetStrProperty(
					Account::P_LANGUAGES, 
					(char *)ptrA(::mir_utf8encodeW(key.c_str())));
			}
		}
		break;

	// Page 1: Contacts
	case 1:
		wchar_t emails[2048];
		::GetDlgItemText(hwndPage, IDC_EMAIL1, emails, SIZEOF(emails));
		::GetDlgItemText(hwndPage, IDC_EMAIL2, text, SIZEOF(text));
		if (::wcslen(text) > 0)
		{
			::wcscat(emails, L" ");
			::wcscat(emails, text);
		}
		::GetDlgItemText(hwndPage, IDC_EMAIL3, text, SIZEOF(text));
		if (::wcslen(text) > 0)
		{
			::wcscat(emails, L" ");
			::wcscat(emails, text);
		}
		this->account->SetStrProperty(Account::P_EMAILS, (char *)ptrA(::mir_utf8encodeW(emails)));

		::GetDlgItemText(hwndPage, IDC_MOBPHONE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PHONE_MOBILE, (char *)ptrA(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_HOMEPHONE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PHONE_HOME, (char *)ptrA(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_OFFICEPHONE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PHONE_OFFICE, (char *)ptrA(::mir_utf8encodeW(text)));

		break;

	// Page 2: Home
	case 2:
		::GetDlgItemText(hwndPage, IDC_CITY, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_CITY, (char *)ptrA(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_STATE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PROVINCE, (char *)ptrA(::mir_utf8encodeW(text)));

		int i = ::SendMessage(::GetDlgItem(hwndPage, IDC_COUNTRY), CB_GETCURSEL, 0, 0);
		char *iso = (char *)::SendMessage(::GetDlgItem(hwndPage, IDC_COUNTRY), CB_GETITEMDATA, i, 0);
		this->account->SetStrProperty(Account::P_COUNTRY, iso);

		HWND ctrl = ::GetDlgItem(hwndPage, IDC_TIMEZONE);
		i = ::SendMessage(ctrl, CB_GETCURSEL, 0, 0);
		HANDLE hTimeZone = (HANDLE)::SendMessage(ctrl, CB_GETITEMDATA, i, 0);

		SYSTEMTIME my_st, utc_ts;
		tmi.getTimeZoneTime(hTimeZone, &my_st);
		tmi.getTimeZoneTime(UTC_TIME_HANDLE, &utc_ts);

		uint diff_to_UTC_in_seconds = (my_st.wHour - utc_ts.wHour) * 3600 + (my_st.wMinute - utc_ts.wMinute) * 60;
		uint timezone = 24*3600 + diff_to_UTC_in_seconds;
		this->account->SetIntProperty(Account::P_TIMEZONE, timezone);

		break;
	}
}