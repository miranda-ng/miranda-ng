#include "skype_proto.h"

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

wchar_t* CSkypeProto::GetContactAvatarFilePath(HANDLE hContact)
{
	wchar_t* path = new wchar_t[MAX_PATH * 2];
	
	FOLDERSGETDATA fgd = {0};
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = MAX_PATH * 2;
	fgd.szPathT = path;
	fgd.flags = FF_UNICODE;

	HANDLE hAvatarsFolder;
	if (::ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		wchar_t tszPath[MAX_PATH * 2];
		::mir_sntprintf(
			tszPath, 
			MAX_PATH * 2, 
			_T("%%miranda_avatarcache%%\\") _T(TCHAR_STR_PARAM) _T("\\"), 
			this->m_szModuleName);

			hAvatarsFolder = ::FoldersRegisterCustomPathT(this->m_szModuleName, "Avatars Cache", tszPath);
		}
	
	if (::CallService(MS_FOLDERS_GET_PATH, (WPARAM)hAvatarsFolder, (LPARAM)&fgd))
	{
		wchar_t *tmpPath = ::Utils_ReplaceVarsT(L"%miranda_avatarcache%");
		::mir_sntprintf(path, MAX_PATH * 2, _T("%s\\") _T(TCHAR_STR_PARAM) _T("\\"), tmpPath, this->m_szModuleName);
		mir_free(tmpPath);
	}
	else
		wcscat(path, L"\\");

	// make sure the avatar cache directory exists
	::CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	wchar_t *sid = this->GetSettingString("sid", ::mir_wstrdup(L""));
	::wcscat(path, sid);
	::wcscat(path, L".jpg");
	::mir_free(sid);

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
	ACKDATA ack = {0};
	ack.cbSize = sizeof(ACKDATA);
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
	}

	return loginError;
}

void CSkypeProto::ShowNotification(const wchar_t *sid, const wchar_t *message, int flags)
{
	if (::Miranda_Terminated()) return;

	if ( !ServiceExists(MS_POPUP_ADDPOPUPEX) || !DBGetContactSettingByte(NULL, "PopUp", "ModuleIsEnabled", 1) )
	{
		MessageBoxW(
			NULL, 
			message, 
			TranslateT("Skype Protocol"), 
			MB_OK);
	}
	else
	{
		POPUPDATAT_V2 ppd = {0};
		ppd.cbSize = sizeof(POPUPDATAT_V2);
		ppd.lchContact = NULL;
		lstrcpyn(ppd.lpwzContactName, sid, MAX_CONTACTNAME);
		lstrcpyn(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Skype_main");
		ppd.colorBack = ppd.colorText = 0;
		ppd.iSeconds = 0;

		::CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
	}
}