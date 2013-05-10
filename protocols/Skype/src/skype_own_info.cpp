#include "skype_proto.h"

void __cdecl CSkypeProto::LoadOwnInfo(void *)
{
	mir_ptr<wchar_t> nick( ::db_get_wsa(NULL, this->m_szModuleName, "Nick"));
	if (nick == NULL)
	{
		SEString data;
		this->account->GetPropFullname(data);

		nick = ::mir_utf8decodeW(data);
		::db_set_ws(NULL, this->m_szModuleName, "Nick", nick);
	}
	//this->UpdateProfileAvatar(this->account.fetch());
	this->UpdateProfile(this->account.fetch());
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
			this->account->SetStrProperty(Account::P_FULLNAME, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

			::GetDlgItemText(hwndPage, IDC_MOOD, text, SIZEOF(text));
			this->account->SetStrProperty(Account::P_MOOD_TEXT, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

			::GetDlgItemText(hwndPage, IDC_ABOUT, text, SIZEOF(text));
			this->account->SetStrProperty(Account::P_ABOUT, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

			::GetDlgItemText(hwndPage, IDC_HOMEPAGE, text, SIZEOF(text));
			this->account->SetStrProperty(Account::P_HOMEPAGE, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

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
					(char*)mir_ptr<char>(::mir_utf8encodeW(CSkypeProto::languages[key].c_str())));
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
		this->account->SetStrProperty(Account::P_EMAILS, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_MOBPHONE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PHONE_MOBILE, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_HOMEPHONE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PHONE_HOME, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_OFFICEPHONE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PHONE_OFFICE, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

		break;

	// Page 2: Home
	case 2:
		::GetDlgItemText(hwndPage, IDC_CITY, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_CITY, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

		::GetDlgItemText(hwndPage, IDC_STATE, text, SIZEOF(text));
		this->account->SetStrProperty(Account::P_PROVINCE, (char*)mir_ptr<char>(::mir_utf8encodeW(text)));

		int i = ::SendMessage(GetDlgItem(hwndPage, IDC_COUNTRY), CB_GETCURSEL, 0, 0);
		int id = ::SendMessage(GetDlgItem(hwndPage, IDC_COUNTRY), CB_GETITEMDATA, i, 0);
		char *countrystr = (char *)::CallService(MS_UTILS_GETCOUNTRYBYNUMBER, (WPARAM)id, 0);
		this->account->SetStrProperty(Account::P_COUNTRY, countrystr);

		break;
	}
}