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


void CSkypeProto::FakeAsync(void *param)
{
	::Sleep(100);
	::CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)param);
	::mir_free(param);
}

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

static HANDLE hAvatarsFolder = NULL;
static bool bInitDone = false;

void CSkypeProto::InitCustomFolders()
{
	if (bInitDone)
		return;

	bInitDone = true;
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		TCHAR AvatarsFolder[MAX_PATH];
		mir_sntprintf(AvatarsFolder, SIZEOF(AvatarsFolder), _T("%%miranda_avatarcache%%\\") _T(TCHAR_STR_PARAM), this->m_szModuleName);
		hAvatarsFolder = FoldersRegisterCustomPathT(this->m_szModuleName, "Avatars", AvatarsFolder);
	}
}

wchar_t* CSkypeProto::GetContactAvatarFilePath(HANDLE hContact)
{
	wchar_t* path = new wchar_t[MAX_PATH];
	
	this->InitCustomFolders();

	if (hAvatarsFolder == NULL || FoldersGetCustomPathT(hAvatarsFolder, path, MAX_PATH, _T("")))
	{
		wchar_t *tmpPath = ::Utils_ReplaceVarsT(L"%miranda_avatarcache%");
		::mir_sntprintf(path, MAX_PATH, _T("%s\\") _T(TCHAR_STR_PARAM), tmpPath, this->m_szModuleName);
		mir_free(tmpPath);
	}

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	wchar_t *sid = this->GetSettingString(hContact, "sid");
	if (hContact != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, sid);
	else if (sid != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.jpg"), path, sid);

	return path;
}

int CSkypeProto::CompareProtos(const CSkypeProto *p1, const CSkypeProto *p2)
{
	return wcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

void CSkypeProto::CreateService(const char* szService, SkypeServiceFunc serviceProc)
{
	char moduleName[MAXMODULELABELLENGTH];

	::mir_snprintf(moduleName, sizeof(moduleName), "%s%s", this->m_szModuleName, szService);
	::CreateServiceFunctionObj(moduleName, (MIRANDASERVICEOBJ)*(void**)&serviceProc, this);
}

void CSkypeProto::CreateServiceParam(const char* szService, SkypeServiceFunc serviceProc, LPARAM lParam)
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

int CSkypeProto::SendBroadcast(HANDLE hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	ACKDATA ack = { sizeof(ACKDATA) };
	ack.szModule = this->m_szModuleName;
	ack.hContact = hContact;
	ack.type = type;
	ack.result = result;
	ack.hProcess = hProcess;
	ack.lParam = lParam;

	return ::CallService(MS_PROTO_BROADCASTACK, 0, (LPARAM)&ack);
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

void CSkypeProto::ShowNotification(const char *nick, const wchar_t *message, int flags)
{
	if (::Miranda_Terminated()) return;

	if ( !ServiceExists(MS_POPUP_ADDPOPUPT) || !DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1))
		MessageBoxW(NULL, message, TranslateT("Skype Protocol"), MB_OK);
	else {
		if ( !nick)
			nick = "";

		POPUPDATAT_V2 ppd = {0};
		ppd.cbSize = sizeof(POPUPDATAT_V2);
		ppd.lchContact = NULL;
		lstrcpyn(ppd.lpwzContactName, ::mir_a2u(nick), MAX_CONTACTNAME);
		lstrcpyn(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Skype_main");
		ppd.colorBack = ppd.colorText = 0;
		ppd.iSeconds = 0;

		::CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
	}
}

char CSkypeProto::CharBase64[] = 
{
	'A','B','C','D','E','F','G','H','I','J','K','L','M  ','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c  ','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s  ','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8  ','9','+','/'
};

ULONG CSkypeProto::Base64Encode(char *inputString, char *outputBuffer, SIZE_T nMaxLength)
{
	int outpos = 0;
	char chr[3], enc[4];

	for (uint i = 0; i < ::strlen(inputString); i += 3)
	{
		if (outpos + 4 >= nMaxLength)
			break;

		chr[0] = inputString[i];
		chr[1] = inputString[i+1];
		chr[2] = inputString[i+2];

		enc[0] = chr[0] >> 2;
		enc[1] = ((chr[0] & 0x03) << 4) | (chr[1] >> 4);
		enc[2] = ((chr[1] & 0x0F) << 2) | (chr[2] >> 6);
		enc[3] = chr[2] & 0x3F;

		outputBuffer[outpos++] = CSkypeProto::CharBase64[enc[0]];
		outputBuffer[outpos++] = CSkypeProto::CharBase64[enc[1]];

		if (i + 1 >= ::strlen(inputString))
		{
			outputBuffer[outpos++] = '=';
			outputBuffer[outpos++] = '=';
		}
		else if (i + 2 >= ::strlen(inputString))
		{
			outputBuffer[outpos++] = CSkypeProto::CharBase64[enc[2]];
			outputBuffer[outpos++] = '=';
		}
		else
		{
			outputBuffer[outpos++] = CSkypeProto::CharBase64[enc[2]];
			outputBuffer[outpos++] = CSkypeProto::CharBase64[enc[3]];
		}
	}

	outputBuffer[outpos] = 0;
	return outpos;
}

char *CSkypeProto::RemoveHtml(char *text)
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
	
	::mir_free(text);
	return ::mir_strdup(new_string.c_str());
}