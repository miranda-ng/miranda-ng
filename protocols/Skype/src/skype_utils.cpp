#include "skype_proto.h"

wchar_t* CSkypeProto::LogoutReasons[] =
{
	LPGENW("LOGOUT_CALLED")												/* LOGOUT_CALLED				*/,
	LPGENW("HTTPS_PROXY_AUTH_FAILED")									/* HTTPS_PROXY_AUTH_FAILED		*/,
	LPGENW("SOCKS_PROXY_AUTH_FAILED")									/* SOCKS_PROXY_AUTH_FAILED		*/,
	LPGENW("P2P_CONNECT_FAILED")										/* P2P_CONNECT_FAILED			*/,
	LPGENW("SERVER_CONNECT_FAILED")										/* SERVER_CONNECT_FAILED		*/,
	LPGENW("SERVER_OVERLOADED")											/* SERVER_OVERLOADED			*/,
	LPGENW("DB_IN_USE")													/* DB_IN_USE					*/,
	LPGENW("Invalid skypename")											/* INVALID_SKYPENAME			*/,
	LPGENW("Invalid email")												/* INVALID_EMAIL				*/,
	LPGENW("Unacceptable password")										/* UNACCEPTABLE_PASSWORD		*/,
	LPGENW("SKYPENAME_TAKEN")											/* SKYPENAME_TAKEN				*/,
	LPGENW("REJECTED_AS_UNDERAGE")										/* REJECTED_AS_UNDERAGE			*/,
	LPGENW("NO_SUCH_IDENTITY")											/* NO_SUCH_IDENTITY				*/,
	LPGENW("Incorrect password")										/* INCORRECT_PASSWORD			*/,
	LPGENW("Too many login attempts")									/* TOO_MANY_LOGIN_ATTEMPTS		*/,
	LPGENW("PASSWORD_HAS_CHANGED")										/* PASSWORD_HAS_CHANGED			*/,
	LPGENW("PERIODIC_UIC_UPDATE_FAILED")								/* PERIODIC_UIC_UPDATE_FAILED	*/,
	LPGENW("DB_DISK_FULL")												/* DB_DISK_FULL					*/,
	LPGENW("DB_IO_ERROR")												/* DB_IO_ERROR					*/,
	LPGENW("DB_CORRUPT")												/* DB_CORRUPT					*/,
	LPGENW("DB_FAILURE")												/* DB_FAILURE					*/,
	LPGENW("INVALID_APP_ID")											/* INVALID_APP_ID				*/,
	LPGENW("APP_ID_FAILURE")											/* APP_ID_FAILURE				*/,
	LPGENW("UNSUPPORTED_VERSION")										/* UNSUPPORTED_VERSION			*/,
	LPGENW("ATO (Account TakeOver) detected, account blocked")			/* ATO_BLOCKED					*/,
	LPGENW("Logout from another instance")								/* REMOTE_LOGOUT				*/,
	LPGENW("")															/* ACCESS_TOKEN_RENEWAL_FAILED 	*/
};

wchar_t* CSkypeProto::ValidationReasons[] =
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

wchar_t* CSkypeProto::PasswordChangeReasons[] =
{
	LPGENW("Password change succeeded")									/* PWD_OK						*/,
	LPGENW("")															/* PWD_CHANGING					*/,
	LPGENW("Old password was incorrect")								/* PWD_INVALID_OLD_PASSWORD		*/,
	LPGENW("Failed to verify password. No connection to server")		/* PWD_SERVER_CONNECT_FAILED	*/,
	LPGENW("Password was set but server didn't like it much")			/* PWD_OK_BUT_CHANGE_SUGGESTED	*/,
	LPGENW("New password was exactly the same as old one")				/* PWD_MUST_DIFFER_FROM_OLD		*/,
	LPGENW("The new password was unacceptable")							/* PWD_INVALID_NEW_PWD			*/,
	LPGENW("Account was currently not logged in")						/* PWD_MUST_LOG_IN_TO_CHANGE	*/,
};

LanguagesListEntry CSkypeProto::languages[] =
{
	{"Abkhazian", "ab"},
	{"Afar", "aa"},
	{"Afrikaans", "af"},
	{"Akan", "ak"},
	{"Albanian", "sq"},
	{"Amharic", "am"},
	{"Arabic", "ar"},
	{"Aragonese", "an"},
	{"Armenian", "hy"},
	{"Assamese", "as"},
	{"Avaric", "av"},
	{"Avestan", "ae"},
	{"Aymara", "ay"},
	{"Azerbaijani", "az"},
	{"Bambara", "bm"},
	{"Bashkir", "ba"},
	{"Basque", "eu"},
	{"Belarusian", "be"},
	{"Bengali", "bn"},
	{"Bihari languages", "bh"},
	{"Bislama", "bi"},
	{"Bokmal, Norwegian", "nb"},
	{"Bosnian", "bs"},
	{"Breton", "br"},
	{"Bulgarian", "bg"},
	{"Burmese", "my"},
	{"Castilian", "es"},
	{"Catalan", "ca"},
	{"Central Khmer", "km"},
	{"Chamorro", "ch"},
	{"Chechen", "ce"},
	{"Chewa", "ny"},
	{"Chichewa", "ny"},
	{"Chinese", "zh"},
	{"Chuang", "za"},
	{"Church Slavic", "cu"},
	{"Church Slavonic", "cu"},
	{"Chuvash", "cv"},
	{"Cornish", "kw"},
	{"Corsican", "co"},
	{"Cree", "cr"},
	{"Croatian", "hr"},
	{"Czech", "cs"},
	{"Danish", "da"},
	{"Dhivehi", "dv"},
	{"Divehi", "dv"},
	{"Dutch", "nl"},
	{"Dzongkha", "dz"},
	{"English", "en"},
	{"Esperanto", "eo"},
	{"Estonian", "et"},
	{"Ewe", "ee"},
	{"Faroese", "fo"},
	{"Fijian", "fj"},
	{"Finnish", "fi"},
	{"Flemish", "nl"},
	{"French", "fr"},
	{"Fulah", "ff"},
	{"Gaelic", "gd"},
	{"Galician", "gl"},
	{"Ganda", "lg"},
	{"Georgian", "ka"},
	{"German", "de"},
	{"Gikuyu", "ki"},
	{"Greek, Modern (1453-)", "el"},
	{"Greenlandic", "kl"},
	{"Guarani", "gn"},
	{"Gujarati", "gu"},
	{"Haitian", "ht"},
	{"Haitian Creole", "ht"},
	{"Hausa", "ha"},
	{"Hebrew", "he"},
	{"Herero", "hz"},
	{"Hindi", "hi"},
	{"Hiri Motu", "ho"},
	{"Hungarian", "hu"},
	{"Icelandic", "is"},
	{"Ido", "io"},
	{"Igbo", "ig"},
	{"Indonesian", "id"},
	{"Interlingua (International Auxiliary Language Association)", "ia"},
	{"Interlingue", "ie"},
	{"Inuktitut", "iu"},
	{"Inupiaq", "ik"},
	{"Irish", "ga"},
	{"Italian", "it"},
	{"Japanese", "ja"},
	{"Javanese", "jv"},
	{"Kalaallisut", "kl"},
	{"Kannada", "kn"},
	{"Kanuri", "kr"},
	{"Kashmiri", "ks"},
	{"Kazakh", "kk"},
	{"Kikuyu", "ki"},
	{"Kinyarwanda", "rw"},
	{"Kirghiz", "ky"},
	{"Komi", "kv"},
	{"Kongo", "kg"},
	{"Korean", "ko"},
	{"Kuanyama", "kj"},
	{"Kurdish", "ku"},
	{"Kwanyama", "kj"},
	{"Kyrgyz", "ky"},
	{"Lao", "lo"},
	{"Latin", "la"},
	{"Latvian", "lv"},
	{"Letzeburgesch", "lb"},
	{"Limburgan", "li"},
	{"Limburger", "li"},
	{"Limburgish", "li"},
	{"Lingala", "ln"},
	{"Lithuanian", "lt"},
	{"Luba-Katanga", "lu"},
	{"Luxembourgish", "lb"},
	{"Macedonian", "mk"},
	{"Malagasy", "mg"},
	{"Malay", "ms"},
	{"Malayalam", "ml"},
	{"Maldivian", "dv"},
	{"Maltese", "mt"},
	{"Manx", "gv"},
	{"Maori", "mi"},
	{"Marathi", "mr"},
	{"Marshallese", "mh"},
	{"Moldavian", "ro"},
	{"Moldovan", "ro"},
	{"Mongolian", "mn"},
	{"Nauru", "na"},
	{"Navaho", "nv"},
	{"Navajo", "nv"},
	{"Ndebele, North", "nd"},
	{"Ndebele, South", "nr"},
	{"Ndonga", "ng"},
	{"Nepali", "ne"},
	{"North Ndebele", "nd"},
	{"Northern Sami", "se"},
	{"Norwegian", "no"},
	{"Norwegian Bokmal", "nb"},
	{"Norwegian Nynorsk", "nn"},
	{"Nuosu", "ii"},
	{"Nyanja", "ny"},
	{"Nynorsk, Norwegian", "nn"},
	{"Occidental", "ie"},
	{"Occitan (post 1500)", "oc"},
	{"Ojibwa", "oj"},
	{"Old Bulgarian", "cu"},
	{"Old Church Slavonic", "cu"},
	{"Old Slavonic", "cu"},
	{"Oriya", "or"},
	{"Oromo", "om"},
	{"Ossetian", "os"},
	{"Ossetic", "os"},
	{"Pali", "pi"},
	{"Panjabi", "pa"},
	{"Pashto", "ps"},
	{"Persian", "fa"},
	{"Polish", "pl"},
	{"Portuguese", "pt"},
	{"Punjabi", "pa"},
	{"Pushto", "ps"},
	{"Quechua", "qu"},
	{"Romanian", "ro"},
	{"Romansh", "rm"},
	{"Rundi", "rn"},
	{"Russian", "ru"},
	{"Samoan", "sm"},
	{"Sango", "sg"},
	{"Sanskrit", "sa"},
	{"Sardinian", "sc"},
	{"Scottish Gaelic", "gd"},
	{"Serbian", "sr"},
	{"Shona", "sn"},
	{"Sichuan Yi", "ii"},
	{"Sindhi", "sd"},
	{"Sinhala", "si"},
	{"Sinhalese", "si"},
	{"Slovak", "sk"},
	{"Slovenian", "sl"},
	{"Somali", "so"},
	{"Sotho, Southern", "st"},
	{"South Ndebele", "nr"},
	{"Spanish", "es"},
	{"Sundanese", "su"},
	{"Swahili", "sw"},
	{"Swati", "ss"},
	{"Swedish", "sv"},
	{"Tagalog", "tl"},
	{"Tahitian", "ty"},
	{"Tajik", "tg"},
	{"Tamil", "ta"},
	{"Tatar", "tt"},
	{"Telugu", "te"},
	{"Thai", "th"},
	{"Tibetan", "bo"},
	{"Tigrinya", "ti"},
	{"Tonga (Tonga Islands)", "to"},
	{"Tsonga", "ts"},
	{"Tswana", "tn"},
	{"Turkish", "tr"},
	{"Turkmen", "tk"},
	{"Twi", "tw"},
	{"Uighur", "ug"},
	{"Ukrainian", "uk"},
	{"Urdu", "ur"},
	{"Uyghur", "ug"},
	{"Uzbek", "uz"},
	{"Valencian", "ca"},
	{"Venda", "ve"},
	{"Vietnamese", "vi"},
	{"Volapuk", "vo"},
	{"Walloon", "wa"},
	{"Welsh", "cy"},
	{"Western Frisian", "fy"},
	{"Wolof", "wo"},
	{"Xhosa", "xh"},
	{"Yiddish", "yi"},
	{"Yoruba", "yo"},
	{"Zhuang", "za"},
	{"Zulu", "zu"}
};

int CSkypeProto::DetectAvatarFormatBuffer(const char *pBuffer)
{
	if (!strncmp(pBuffer, "%PNG", 4))
		return PA_FORMAT_PNG;

	if (!strncmp(pBuffer, "GIF8", 4))
		return PA_FORMAT_GIF;

	if (!_strnicmp(pBuffer, "<?xml", 5))
		return PA_FORMAT_XML;

	if ((((DWORD*)pBuffer)[0] == 0xE0FFD8FFul) || (((DWORD*)pBuffer)[0] == 0xE1FFD8FFul))
		return PA_FORMAT_JPEG;

	if (!strncmp(pBuffer, "BM", 2))
		return PA_FORMAT_BMP;

	return PA_FORMAT_UNKNOWN;
}

int CSkypeProto::DetectAvatarFormat(const wchar_t *path)
{
	int src = _wopen(path, _O_BINARY | _O_RDONLY, 0);
	if (src == -1)
		return PA_FORMAT_UNKNOWN;

	char pBuf[32];
	_read(src, pBuf, 32);
	_close(src);

	return CSkypeProto::DetectAvatarFormatBuffer(pBuf);
}

void CSkypeProto::InitCustomFolders()
{
	if (m_bInitDone)
		return;

	m_bInitDone = true;

	TCHAR AvatarsFolder[MAX_PATH];
	mir_sntprintf(AvatarsFolder, SIZEOF(AvatarsFolder), _T("%%miranda_avatarcache%%\\%S"), this->m_szModuleName);
	m_hAvatarsFolder = ::FoldersRegisterCustomPathT(LPGEN("Avatars"), m_szModuleName, AvatarsFolder, m_tszUserName);
}

wchar_t* CSkypeProto::GetContactAvatarFilePath(HANDLE hContact)
{
	wchar_t* path = new wchar_t[MAX_PATH];

	this->InitCustomFolders();

	if (m_hAvatarsFolder == NULL || FoldersGetCustomPathT(m_hAvatarsFolder, path, MAX_PATH, _T("")))
	{
		wchar_t *tmpPath = ::Utils_ReplaceVarsT(L"%miranda_avatarcache%");
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%S"), tmpPath, this->m_szModuleName);
		::mir_free(tmpPath);
	}

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	::mir_ptr<wchar_t> sid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN));
	if (hContact != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, sid);
	else if (sid != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.jpg"), path, sid);
	else
	{
		delete [] path;
		return NULL;
	}

	return path;
}

int CSkypeProto::CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2)
{
	return wcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

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