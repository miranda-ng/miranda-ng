#include "skype_proto.h"

wchar_t *CSkypeProto::ValidationReasons[] =
{
	LPGENW("NOT_VALIDATED")												/* NOT_VALIDATED				*/,
	LPGENW("Validation succeeded")										/* VALIDATED_OK					*/,
	LPGENW("Password is too short")										/* TOO_SHORT					*/,
	LPGENW("The value exceeds max size limit for the given property")	/* TOO_LONG						*/,
	LPGENW("Value contains illegal characters")							/* CONTAINS_INVALID_CHAR		*/,
	LPGENW("Value contains whitespace")									/* CONTAINS_SPACE				*/,
	LPGENW("Password cannot be the same as skypename")					/* SAME_AS_USERNAME				*/,
	LPGENW("Value has invalid format")									/* INVALID_FORMAT				*/,
	LPGENW("Value contains invalid word")								/* CONTAINS_INVALID_WORD		*/,
	LPGENW("Password is too simple")									/* TOO_SIMPLE					*/,
	LPGENW("Value starts with an invalid character")					/* STARTS_WITH_INVALID_CHAR		*/,
};

std::map<std::wstring, std::wstring> CSkypeProto::FillLanguages()
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
	return result;
}

void CSkypeProto::InitCustomFolders()
{
	if (m_bInitDone)
		return;

	m_bInitDone = true;

	TCHAR AvatarsFolder[MAX_PATH];
	::mir_sntprintf(AvatarsFolder, SIZEOF(AvatarsFolder), _T("%%miranda_avatarcache%%\\%S"), this->m_szModuleName);
	m_hAvatarsFolder = ::FoldersRegisterCustomPathT(LPGEN("Avatars"), m_szModuleName, AvatarsFolder, m_tszUserName);
}

// ---

void CSkypeProto::CreateServiceObj(const char* szService, SkypeServiceFunc serviceProc)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	::CreateServiceFunctionObj(moduleName, (MIRANDASERVICEOBJ)*(void**)&serviceProc, this);
}

void CSkypeProto::CreateServiceObjParam(const char* szService, SkypeServiceFunc serviceProc, LPARAM lParam)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	::CreateServiceFunctionObjParam(moduleName, (MIRANDASERVICEOBJPARAM)*(void**)&serviceProc, this, lParam);
}

HANDLE CSkypeProto::CreateEvent(const char* szService)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	return ::CreateHookableEvent(moduleName);
}

void CSkypeProto::HookEvent(const char* szEvent, SkypeEventFunc handler)
{
	::HookEventObj(szEvent, (MIRANDAHOOKOBJ)*( void**)&handler, this);
}

void CSkypeProto::FakeAsync(void *param)
{
	::Sleep(100);
	::CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)param);
	::mir_free(param);
}

int CSkypeProto::SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return ::ProtoBroadcastAck(this->m_szModuleName, hContact, type, result, hProcess, lParam);
}

int CSkypeProto::SendBroadcast(int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return this->SendBroadcast(NULL, type, result, hProcess, lParam);
}

DWORD CSkypeProto::SendBroadcastAsync(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam, size_t paramSize)
{
	ACKDATA *ack = (ACKDATA *)::mir_calloc(sizeof(ACKDATA) + paramSize);
	ack->cbSize = sizeof(ACKDATA);
	ack->szModule = this->m_szModuleName;
	ack->hContact = hContact;
	ack->type = type;
	ack->result = hResult;
	ack->hProcess = hProcess;
	ack->lParam = lParam;
	if (paramSize)
		::memcpy(ack+1, (void*)lParam, paramSize);
	::mir_forkthread(&CSkypeProto::FakeAsync, ack);
	return 0;
}

void CSkypeProto::ForkThread(SkypeThreadFunc pFunc, void *param)
{
	UINT threadID;
	::CloseHandle((HANDLE)::mir_forkthreadowner(
		(pThreadFuncOwner)*(void**)&pFunc,
		this,
		param,
		&threadID));
}

HANDLE CSkypeProto::ForkThreadEx(SkypeThreadFunc pFunc, void *param, UINT* threadID)
{
	UINT lthreadID;
	return (HANDLE)::mir_forkthreadowner(
		(pThreadFuncOwner)*(void**)&pFunc,
		this,
		param,
		threadID ? threadID : &lthreadID);
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

void CSkypeProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, HANDLE hContact)
{
	if (::Miranda_Terminated()) return;

	if ( !::ServiceExists(MS_POPUP_ADDPOPUPT) || !::db_get_b(NULL, "PopUp", "ModuleIsEnabled", 1))
		::MessageBoxW(NULL, message, caption, MB_OK | flags);
	else
	{
		POPUPDATAW ppd = {0};
		ppd.lchContact = hContact;
		if (!hContact)
		{
			lstrcpyn(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		}
		lstrcpyn(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = ::Skin_GetIcon("Skype_main");

		PUAddPopUpW(&ppd);
	}
}

void CSkypeProto::ShowNotification(const wchar_t *message, int flags, HANDLE hContact)
{
	CSkypeProto::ShowNotification(TranslateT("Skype Protocol"), message, flags, hContact);
}

char *CSkypeProto::RemoveHtml(const char *text)
{
	std::string new_string = "";
	std::string data = text;

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
	case CContact::ONLINE:
	case CContact::SKYPE_ME:
		status = ID_STATUS_ONLINE;
		break;

	case CContact::ONLINE_FROM_MOBILE:
	case CContact::SKYPE_ME_FROM_MOBILE:
		status = ID_STATUS_ONTHEPHONE;
		break;

	case CContact::AWAY:
	case CContact::AWAY_FROM_MOBILE:
		status = ID_STATUS_AWAY;
		break;

	case CContact::DO_NOT_DISTURB:
	case CContact::DO_NOT_DISTURB_FROM_MOBILE:
		status = ID_STATUS_DND;
		break;

	case CContact::SKYPEOUT:
		status = ID_STATUS_OUTTOLUNCH;
		break;

	case CContact::CONNECTING:
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

SEBinary CSkypeProto::GetAvatarBinary(wchar_t *path)
{
	SEBinary avatar;

	if (::PathFileExists(path))
	{
		int len;
		char *buffer;
		FILE* fp = ::_wfopen(path, L"rb");
		if (fp)
		{
			::fseek(fp, 0, SEEK_END);
			len = ::ftell(fp);
			::fseek(fp, 0, SEEK_SET);
			buffer = new char[len + 1];
			::fread(buffer, len, 1, fp);
			::fclose(fp);

			avatar.set(buffer, len);
		}
	}

	return avatar;
}