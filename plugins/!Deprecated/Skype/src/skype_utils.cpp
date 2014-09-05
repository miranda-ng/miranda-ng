#include "skype.h"

wchar_t *CSkypeProto::ValidationReasons[] =
{
	LPGENW("NOT_VALIDATED")												/* NOT_VALIDATED				*/,
	LPGENW("Validation succeeded")										/* VALIDATED_OK					*/,
	LPGENW("Password is too short")										/* TOO_SHORT					*/,
	LPGENW("The value exceeds max size limit for the given property")	/* TOO_LONG						*/,
	LPGENW("Value contains illegal characters")							/* CONTAINS_INVALID_CHAR		*/,
	LPGENW("Value contains whitespace")									/* CONTAINS_SPACE				*/,
	LPGENW("Password cannot be the same as Skype name")					/* SAME_AS_USERNAME				*/,
	LPGENW("Value has invalid format")									/* INVALID_FORMAT				*/,
	LPGENW("Value contains invalid word")								/* CONTAINS_INVALID_WORD		*/,
	LPGENW("Password is too simple")									/* TOO_SIMPLE					*/,
	LPGENW("Value starts with an invalid character")					/* STARTS_WITH_INVALID_CHAR		*/,
};

std::map<std::wstring, std::wstring> CSkypeProto::languages;

void CSkypeProto::InitLanguages()
{
	std::map<std::wstring, std::wstring> result;
	result[L"ab"] = L"Abkhazian";
	result[L"aa"] = L"Afar";
	result[L"af"] = L"Afrikaans";
	result[L"ak"] = L"Akan";
	result[L"sq"] = L"Albanian";
	result[L"am"] = L"Amharic";
	result[L"ar"] = L"Arabic";
	result[L"an"] = L"Aragonese";
	result[L"hy"] = L"Armenian";
	result[L"as"] = L"Assamese";
	result[L"av"] = L"Avaric";
	result[L"ae"] = L"Avestan";
	result[L"ay"] = L"Aymara";
	result[L"az"] = L"Azerbaijani";
	result[L"bm"] = L"Bambara";
	result[L"ba"] = L"Bashkir";
	result[L"eu"] = L"Basque";
	result[L"be"] = L"Belarusian";
	result[L"bn"] = L"Bengali";
	result[L"bh"] = L"Bihari languages";
	result[L"bi"] = L"Bislama";
	result[L"nb"] = L"Bokmal, Norwegian";
	result[L"bs"] = L"Bosnian";
	result[L"br"] = L"Breton";
	result[L"bg"] = L"Bulgarian";
	result[L"my"] = L"Burmese";
	result[L"es"] = L"Castilian";
	result[L"ca"] = L"Catalan";
	result[L"km"] = L"Central Khmer";
	result[L"ch"] = L"Chamorro";
	result[L"ce"] = L"Chechen";
	result[L"ny"] = L"Chewa";
	result[L"ny"] = L"Chichewa";
	result[L"zh"] = L"Chinese";
	result[L"za"] = L"Chuang";
	result[L"cu"] = L"Church Slavic";
	result[L"cu"] = L"Church Slavonic";
	result[L"cv"] = L"Chuvash";
	result[L"kw"] = L"Cornish";
	result[L"co"] = L"Corsican";
	result[L"cr"] = L"Cree";
	result[L"hr"] = L"Croatian";
	result[L"cs"] = L"Czech";
	result[L"da"] = L"Danish";
	result[L"dv"] = L"Dhivehi";
	result[L"dv"] = L"Divehi";
	result[L"nl"] = L"Dutch";
	result[L"dz"] = L"Dzongkha";
	result[L"en"] = L"English";
	result[L"eo"] = L"Esperanto";
	result[L"et"] = L"Estonian";
	result[L"ee"] = L"Ewe";
	result[L"fo"] = L"Faroese";
	result[L"fj"] = L"Fijian";
	result[L"fi"] = L"Finnish";
	result[L"nl"] = L"Flemish";
	result[L"fr"] = L"French";
	result[L"ff"] = L"Fulah";
	result[L"gd"] = L"Gaelic";
	result[L"gl"] = L"Galician";
	result[L"lg"] = L"Ganda";
	result[L"ka"] = L"Georgian";
	result[L"de"] = L"German";
	result[L"ki"] = L"Gikuyu";
	result[L"el"] = L"Greek, Modern (1453-)";
	result[L"kl"] = L"Greenlandic";
	result[L"gn"] = L"Guarani";
	result[L"gu"] = L"Gujarati";
	result[L"ht"] = L"Haitian";
	result[L"ht"] = L"Haitian Creole";
	result[L"ha"] = L"Hausa";
	result[L"he"] = L"Hebrew";
	result[L"hz"] = L"Herero";
	result[L"hi"] = L"Hindi";
	result[L"ho"] = L"Hiri Motu";
	result[L"hu"] = L"Hungarian";
	result[L"is"] = L"Icelandic";
	result[L"io"] = L"Ido";
	result[L"ig"] = L"Igbo";
	result[L"id"] = L"Indonesian";
	result[L"ia"] = L"Interlingua (International Auxiliary Language Association)";
	result[L"ie"] = L"Interlingue";
	result[L"iu"] = L"Inuktitut";
	result[L"ik"] = L"Inupiaq";
	result[L"ga"] = L"Irish";
	result[L"it"] = L"Italian";
	result[L"ja"] = L"Japanese";
	result[L"jv"] = L"Javanese";
	result[L"kl"] = L"Kalaallisut";
	result[L"kn"] = L"Kannada";
	result[L"kr"] = L"Kanuri";
	result[L"ks"] = L"Kashmiri";
	result[L"kk"] = L"Kazakh";
	result[L"ki"] = L"Kikuyu";
	result[L"rw"] = L"Kinyarwanda";
	result[L"ky"] = L"Kirghiz";
	result[L"kv"] = L"Komi";
	result[L"kg"] = L"Kongo";
	result[L"ko"] = L"Korean";
	result[L"kj"] = L"Kuanyama";
	result[L"ku"] = L"Kurdish";
	result[L"kj"] = L"Kwanyama";
	result[L"ky"] = L"Kyrgyz";
	result[L"lo"] = L"Lao";
	result[L"la"] = L"Latin";
	result[L"lv"] = L"Latvian";
	result[L"lb"] = L"Letzeburgesch";
	result[L"li"] = L"Limburgan";
	result[L"li"] = L"Limburger";
	result[L"li"] = L"Limburgish";
	result[L"ln"] = L"Lingala";
	result[L"lt"] = L"Lithuanian";
	result[L"lu"] = L"Luba-Katanga";
	result[L"lb"] = L"Luxembourgish";
	result[L"mk"] = L"Macedonian";
	result[L"mg"] = L"Malagasy";
	result[L"ms"] = L"Malay";
	result[L"ml"] = L"Malayalam";
	result[L"dv"] = L"Maldivian";
	result[L"mt"] = L"Maltese";
	result[L"gv"] = L"Manx";
	result[L"mi"] = L"Maori";
	result[L"mr"] = L"Marathi";
	result[L"mh"] = L"Marshallese";
	result[L"ro"] = L"Moldavian";
	result[L"ro"] = L"Moldovan";
	result[L"mn"] = L"Mongolian";
	result[L"na"] = L"Nauru";
	result[L"nv"] = L"Navaho";
	result[L"nv"] = L"Navajo";
	result[L"nd"] = L"Ndebele, North";
	result[L"nr"] = L"Ndebele, South";
	result[L"ng"] = L"Ndonga";
	result[L"ne"] = L"Nepali";
	result[L"nd"] = L"North Ndebele";
	result[L"se"] = L"Northern Sami";
	result[L"no"] = L"Norwegian";
	result[L"nb"] = L"Norwegian Bokmal";
	result[L"nn"] = L"Norwegian Nynorsk";
	result[L"ii"] = L"Nuosu";
	result[L"ny"] = L"Nyanja";
	result[L"nn"] = L"Nynorsk, Norwegian";
	result[L"ie"] = L"Occidental";
	result[L"oc"] = L"Occitan (post 1500)";
	result[L"oj"] = L"Ojibwa";
	result[L"cu"] = L"Old Bulgarian";
	result[L"cu"] = L"Old Church Slavonic";
	result[L"cu"] = L"Old Slavonic";
	result[L"or"] = L"Oriya";
	result[L"om"] = L"Oromo";
	result[L"os"] = L"Ossetian";
	result[L"os"] = L"Ossetic";
	result[L"pi"] = L"Pali";
	result[L"pa"] = L"Panjabi";
	result[L"ps"] = L"Pashto";
	result[L"fa"] = L"Persian";
	result[L"pl"] = L"Polish";
	result[L"pt"] = L"Portuguese";
	result[L"pa"] = L"Punjabi";
	result[L"ps"] = L"Pushto";
	result[L"qu"] = L"Quechua";
	result[L"ro"] = L"Romanian";
	result[L"rm"] = L"Romansh";
	result[L"rn"] = L"Rundi";
	result[L"ru"] = L"Russian";
	result[L"sm"] = L"Samoan";
	result[L"sg"] = L"Sango";
	result[L"sa"] = L"Sanskrit";
	result[L"sc"] = L"Sardinian";
	result[L"gd"] = L"Scottish Gaelic";
	result[L"sr"] = L"Serbian";
	result[L"sn"] = L"Shona";
	result[L"ii"] = L"Sichuan Yi";
	result[L"sd"] = L"Sindhi";
	result[L"si"] = L"Sinhala";
	result[L"si"] = L"Sinhalese";
	result[L"sk"] = L"Slovak";
	result[L"sl"] = L"Slovenian";
	result[L"so"] = L"Somali";
	result[L"st"] = L"Sotho, Southern";
	result[L"nr"] = L"South Ndebele";
	result[L"es"] = L"Spanish";
	result[L"su"] = L"Sundanese";
	result[L"sw"] = L"Swahili";
	result[L"ss"] = L"Swati";
	result[L"sv"] = L"Swedish";
	result[L"tl"] = L"Tagalog";
	result[L"ty"] = L"Tahitian";
	result[L"tg"] = L"Tajik";
	result[L"ta"] = L"Tamil";
	result[L"tt"] = L"Tatar";
	result[L"te"] = L"Telugu";
	result[L"th"] = L"Thai";
	result[L"bo"] = L"Tibetan";
	result[L"ti"] = L"Tigrinya";
	result[L"to"] = L"Tonga (Tonga Islands)";
	result[L"ts"] = L"Tsonga";
	result[L"tn"] = L"Tswana";
	result[L"tr"] = L"Turkish";
	result[L"tk"] = L"Turkmen";
	result[L"tw"] = L"Twi";
	result[L"ug"] = L"Uighur";
	result[L"uk"] = L"Ukrainian";
	result[L"ur"] = L"Urdu";
	result[L"ug"] = L"Uyghur";
	result[L"uz"] = L"Uzbek";
	result[L"ca"] = L"Valencian";
	result[L"ve"] = L"Venda";
	result[L"vi"] = L"Vietnamese";
	result[L"vo"] = L"Volapuk";
	result[L"wa"] = L"Walloon";
	result[L"cy"] = L"Welsh";
	result[L"fy"] = L"Western Frisian";
	result[L"wo"] = L"Wolof";
	result[L"xh"] = L"Xhosa";
	result[L"yi"] = L"Yiddish";
	result[L"yo"] = L"Yoruba";
	result[L"za"] = L"Zhuang";
	result[L"zu"] = L"Zulu";
}

// ---

int CSkypeProto::SendBroadcast(MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return ::ProtoBroadcastAck(this->m_szModuleName, hContact, type, result, hProcess, lParam);
}

int CSkypeProto::SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return this->SendBroadcast(NULL, type, result, hProcess, lParam);
}

//

int CSkypeProto::SkypeToMirandaLoginError(CAccount::LOGOUTREASON logoutReason)
{
	int loginError = 0;

	// todo: rewrite!!

	switch (logoutReason)
	{
	case CAccount::SERVER_OVERLOADED:
	case CAccount::P2P_CONNECT_FAILED:
	case CAccount::SERVER_CONNECT_FAILED:
		loginError = LOGINERR_NOSERVER;
		break;
	case CAccount::HTTPS_PROXY_AUTH_FAILED:
	case CAccount::SOCKS_PROXY_AUTH_FAILED:
		loginError = LOGINERR_PROXYFAILURE;
		break;
	case CAccount::INCORRECT_PASSWORD:
	case CAccount::UNACCEPTABLE_PASSWORD:
		loginError = LOGINERR_WRONGPASSWORD;
		break;

	case CAccount::INVALID_APP_ID:
		loginError = 1001;
		break;
	}

	return loginError;
}

void CSkypeProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (::Miranda_Terminated()) return;

	if ( ::ServiceExists(MS_POPUP_ADDPOPUPT) && ::db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAW ppd = {0};
		ppd.lchContact = hContact;
		::wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		::wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = ::Skin_GetIcon("Skype_main");

		if ( !::PUAddPopupW(&ppd))
			return;
			
	}
	
	::MessageBoxW(NULL, message, caption, MB_OK | flags);
}

void CSkypeProto::ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	CSkypeProto::ShowNotification(::TranslateT(MODULE), message, flags, hContact);
}

struct HtmlEntity
{
	const char *entity;
	char symbol;
};

const HtmlEntity htmlEntities[]={
	{"nbsp",	' '},
	{"amp",		'&'},
	{"quot",	'"'},
	{"lt",		'<'},
	{"gt",		'>'},
	{"apos",	'\''},
	{"copy",	'©'},
	// TODO: add more
};

char *CSkypeProto::RemoveHtml(const char *text)
{
	std::string new_string = "";
	std::string data = text;

	if (data.find("\x1b\xe3\xac\x8d\x1d") != -1)
		data = "CONVERSATION MEMBERS:" + data.substr(5, data.length() - 5);

	for (std::string::size_type i = 0; i < data.length(); i++)
	{
		if (data.at(i) == '<' && data.at(i+1) != ' ')
		{
			i = data.find(">", i);
			if (i == std::string::npos)
				break;

			continue;
		}

		if (data.at(i) == '&') {
			std::string::size_type begin = i;
			i = data.find(";", i);
			if (i == std::string::npos) {
				i = begin;
			} else {
				std::string entity = data.substr(begin+1, i-begin-1);

				bool found = false;
				for (int j=0; j<SIZEOF(htmlEntities); j++)
				{
					if (!stricmp(entity.c_str(), htmlEntities[j].entity)) {
						new_string += htmlEntities[j].symbol;
						found = true;
						break;
					}
				}

				if (found)
					continue;
				else
					i = begin;
			}
		}

		new_string += data.at(i);
	}

	return ::mir_strdup(new_string.c_str());
}

int CSkypeProto::SkypeToMirandaStatus(CContact::AVAILABILITY availability)
{
	int status = ID_STATUS_OFFLINE;

	switch (availability)
	{
	case Contact::ONLINE:
	case Contact::SKYPE_ME:
		status = ID_STATUS_ONLINE;
		break;

	case Contact::ONLINE_FROM_MOBILE:
	case Contact::SKYPE_ME_FROM_MOBILE:
		status = ID_STATUS_ONTHEPHONE;
		break;

	case Contact::AWAY:
	case Contact::AWAY_FROM_MOBILE:
		status = ID_STATUS_AWAY;
		break;

	case Contact::DO_NOT_DISTURB:
	case Contact::DO_NOT_DISTURB_FROM_MOBILE:
		status = ID_STATUS_DND;
		break;

	case Contact::INVISIBLE:
		status = ID_STATUS_INVISIBLE;
		break;

	case Contact::SKYPEOUT:
		status = ID_STATUS_ONTHEPHONE;
		break;

	case Contact::CONNECTING:
		status = ID_STATUS_CONNECTING;
		break;
	}

	return status;
}

CContact::AVAILABILITY CSkypeProto::MirandaToSkypeStatus(int status)
{
	CContact::AVAILABILITY availability = CContact::UNKNOWN;

	switch(status)
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

bool CSkypeProto::FileExists(wchar_t *path)
{
	//return ::GetFileAttributes(fileName) != DWORD(-1)
	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(path, &wfd);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		::FindClose(hFind);
		return true;
	}
	return false;
}

void CSkypeProto::CopyToClipboard(const wchar_t *text)
{
	HWND hwnd = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	::OpenClipboard(hwnd);
	::EmptyClipboard();
	HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(::lstrlen(text)+1));
	TCHAR *s = (TCHAR *)::GlobalLock(hMem);
	::lstrcpy(s, text);
	::GlobalUnlock(hMem);
	::SetClipboardData(CF_UNICODETEXT, hMem);
	::CloseClipboard();
}

void CSkypeProto::ReplaceSpecialChars(wchar_t *text, wchar_t replaceWith)
{
	if (text == NULL)
		return;

	wchar_t *special = L"\\/:*?\"<>|";
	for (size_t i = 0; i < ::wcslen(text); i++)
	if (::wcschr(special, text[i]) != NULL)
		text[i] = replaceWith;
}

INT_PTR CSkypeProto::ParseSkypeUri(WPARAM wParam, LPARAM lParam)
{
	if (CSkypeProto::instanceList.getCount() == 0 || !CSkypeProto::instanceList[0]->IsOnline())
		return 1;

	CSkypeProto *ppro = CSkypeProto::instanceList[0];

	wchar_t *args = ::mir_wstrdup((wchar_t *)lParam);
	if (args == NULL)
		return 1;
		// set focus on clist

	wchar_t * q = ::wcschr(args, L'?');
	if (q == NULL)
		return 1;
		
	wchar_t *c = q + 1; *q = 0;
	StringList commands = StringList(c, L"&");
	StringList participants = StringList(args, L";");
	ptrW command, arg, commandAndParam;

	if ( !::lstrcmpiW(commands[0], L"chat"))
		ppro->ChatRoomParseUriComands(c);
	else
	{
		wchar_t message[1024];
		::mir_sntprintf(message, SIZEOF(message), ::TranslateT("Command \"%s\" is unsupported"), args);
		CSkypeProto::ShowNotification(message);
		return 1;
	}

	//for (size_t i = 1; i < commands.size(); i++)
	//{
	//	command = ::mir_wstrdup(commands[i]);
	//	wchar_t * p = ::wcschr(command, L'=');
	//	if (p != NULL)
	//	{
	//		arg = p + 1;
	//		*p = 0;
	//	}

	//	if ( !::lstrcmpiW(command, L"blob"))
	//	{
	//		ppro->JoinToChat(arg);
	//		break;
	//	}
	//	else if ( !::lstrcmpiW(command, L"topic") && !participants.empty())
	//	{
	//		ChatRoomParam param(NULL, participants, ppro);
	//		::wcscpy(param.topic, arg);
	//		ppro->CreateChatRoom(&param);
	//		break;
	//		
	//	}
	//}
	
	return 0;
}
