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

// ---

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