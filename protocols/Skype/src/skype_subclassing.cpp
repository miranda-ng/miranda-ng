#include "skype.h"

#include "base64/base64.h"
extern "C" 
{ 
#include "aes\aes.h" 
}

// CSkype

CSkype::CSkype(int num_threads) : Skype(num_threads)
{
	this->proto = NULL;
	this->onMessagedCallback = NULL;
}

CAccount* CSkype::newAccount(int oid) 
{ 
	return new CAccount(oid, this); 
}

CContactGroup* CSkype::newContactGroup(int oid)
{ 
	return new CContactGroup(oid, this); 
}

CContact* CSkype::newContact(int oid) 
{ 
	return new CContact(oid, this); 
}

CConversation* CSkype::newConversation(int oid) 
{ 
	return new CConversation(oid, this); 
}

CParticipant* CSkype::newParticipant(int oid) 
{ 
	return new CParticipant(oid, this); 
}

CMessage* CSkype::newMessage(int oid) 
{ 
	return new CMessage(oid, this); 
}

CContactSearch*	CSkype::newContactSearch(int oid)
{
	return new CContactSearch(oid, this);
}

void CSkype::OnMessage (
	const MessageRef & message,
	const bool & changesInboxTimestamp,
	const MessageRef & supersedesHistoryMessage,
	const ConversationRef & conversation)
{
    /*uint now;
    skype->GetUnixTimestamp(now);
    conversation->SetConsumedHorizon(now);*/

	if (this->proto)
		(proto->*onMessagedCallback)(conversation->ref(), message->ref());
}

void CSkype::SetOnMessageCallback(OnMessaged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->onMessagedCallback = callback;
}

BOOL CSkype::IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if ( !AllocateAndInitializeSid(
		&NtAuthority, 
		2, 
		SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_ADMINS, 
		0, 0, 0, 0, 0, 0, 
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if ( !CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

char *CSkype::LoadKeyPair(HINSTANCE hInstance)
{
	HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(/*IDR_KEY*/107), L"BIN");
	if (hRes)
	{
		HGLOBAL hResource = LoadResource(hInstance, hRes);
		if (hResource)
		{
			aes_context ctx;
			unsigned char key[128];

			int basedecoded = Base64::Decode(MY_KEY, (char *)key, MAX_PATH);
			::aes_set_key(&ctx, key, 128);

			int dwResSize = ::SizeofResource(hInstance, hRes);
			char *pData = (char *)::GlobalLock(hResource);
			basedecoded = dwResSize;
			::GlobalUnlock(hResource);

			unsigned char *bufD = (unsigned char*)::malloc(basedecoded + 1);
			unsigned char *tmpD = (unsigned char*)::malloc(basedecoded + 1);
			basedecoded = Base64::Decode(pData, (char *)tmpD, basedecoded);

			for (int i = 0; i < basedecoded; i += 16)
				aes_decrypt(&ctx, tmpD+i, bufD+i);

			::free(tmpD);
			bufD[basedecoded] = 0; //cert should be null terminated
			return (char *)bufD;
		}
		return NULL;
	}
	return NULL;
}

int CSkype::StartSkypeRuntime(HINSTANCE hInstance, const wchar_t *profileName, int &port, const wchar_t *dbPath)
{
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	wchar_t param[128];

	::ZeroMemory(&cif, sizeof(STARTUPINFO));
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;

	//HRSRC 	hRes;
	//HGLOBAL	hResource;
	wchar_t	fileName[MAX_PATH];

	HRSRC hRes = ::FindResource(hInstance, MAKEINTRESOURCE(/*IDR_RUNTIME*/102), L"BIN");
	if (hRes)
	{
		HGLOBAL hResource = ::LoadResource(hInstance, hRes);
		if (hResource)
		{
			HANDLE hFile;
			char *pData = (char *)LockResource(hResource);
			DWORD dwSize = SizeofResource(hInstance, hRes), written = 0;
			::GetModuleFileName(hInstance, fileName, MAX_PATH);

			wchar_t *skypeKitPath = ::wcsrchr(fileName, '\\');
			if (skypeKitPath != NULL)
				*skypeKitPath = 0;
			::swprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"SkypeKit.exe");
			if ( !::PathFileExists(fileName))
			{
				if ((hFile = ::CreateFile(
					fileName,
					GENERIC_WRITE,
					0,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					0)) != INVALID_HANDLE_VALUE)
				{
					::WriteFile(hFile, (void *)pData, dwSize, &written, NULL);
					::CloseHandle(hFile);
				}
				else
				{
					// Check the current process's "run as administrator" status.
					// Elevate the process if it is not run as administrator.
					if (!CSkype::IsRunAsAdmin())
					{
						wchar_t path[MAX_PATH], cmdLine[100];
						::GetModuleFileName(NULL, path, ARRAYSIZE(path));
						::swprintf(
							cmdLine,
							SIZEOF(cmdLine),
							L" /restart:%d /profile=%s",
							::GetCurrentProcessId(),
							profileName);

						// Launch itself as administrator.
						SHELLEXECUTEINFO sei = { sizeof(sei) };
						sei.lpVerb = L"runas";
						sei.lpFile = path;
						sei.lpParameters = cmdLine;
						//sei.hwnd = hDlg;
						sei.nShow = SW_NORMAL;

						if ( !::ShellExecuteEx(&sei))
						{
							DWORD dwError = ::GetLastError();
							if (dwError == ERROR_CANCELLED)
							{
								// The user refused to allow privileges elevation.
								// Do nothing ...
							}
						}
						//else
						//{
						//	//DestroyWindow(hDlg);  // Quit itself
						//	::CallService("CloseAction", 0, 0);
						//}
					}
					return 0;
				}
			}
		}
	}

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (::Process32First(snapshot, &entry) == TRUE) {
		while (::Process32Next(snapshot, &entry) == TRUE) {
			if (::wcsicmp(entry.szExeFile, L"SkypeKit.exe") == 0) {
				HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				port += rand() % 8963 + 1000;
				::CloseHandle(hProcess);
				break;
			}
		}
	}
	::CloseHandle(snapshot);

	//::swprintf(param, SIZEOF(param), L"-p -P %d -f %s", port, dbPath);
	::swprintf(param, SIZEOF(param), L"-p -P %d", port);
	int startingrt = ::CreateProcess(
		fileName, param,
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &cif, &pi);
	DWORD rterr = GetLastError();

	//if (startingrt && rterr == ERROR_SUCCESS)
		//return 1;
	//else
		//return 0;
	return startingrt;
}

CSkype *CSkype::GetInstance(HINSTANCE hInstance, const wchar_t *profileName, const wchar_t *dbPath)
{
	int port = 8963;
	if (!CSkype::StartSkypeRuntime(hInstance, profileName, port, dbPath)) return NULL;

	char *keyPair = CSkype::LoadKeyPair(hInstance);

	CSkype *skype = new CSkype();
	TransportInterface::Status status = skype->init(keyPair, "127.0.0.1", port, 0, 2, 3);
	if (status != TransportInterface::OK)
		return NULL;
	skype->start();

	free(keyPair);

	//this->skype->SetOnMessageCallback((CSkype::OnMessaged)&CSkypeProto::OnMessage, this);
	return skype;
}

// CAccount

CAccount::CAccount(unsigned int oid, SERootObject* root) : Account(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CAccount::SetOnAccountChangedCallback(OnAccountChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

void CAccount::OnChange(int prop)
{
  if (this->proto)
	  (proto->*callback)(prop);
}

// CContactGroup

CContactGroup::CContactGroup(unsigned int oid, SERootObject* root) : ContactGroup(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CContactGroup::SetOnContactListChangedCallback(OnContactListChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

void CContactGroup::OnChange(const ContactRef& contact)
{
	if (this->proto)
		(proto->*callback)(contact);
}

// CContactSearch

CContactSearch::CContactSearch(unsigned int oid, SERootObject* root) : ContactSearch(oid, root)
{
	this->proto = NULL;
	this->SearchCompletedCallback == NULL;
	this->ContactFindedCallback == NULL;
}

void CContactSearch::OnChange(int prop)
{
	if (prop == P_CONTACT_SEARCH_STATUS)
	{
		CContactSearch::STATUS status;
		this->GetPropContactSearchStatus(status);
		if (status == FINISHED || status == FAILED)
		{
			this->isSeachFinished = true;
			if (this->proto)
				(proto->*SearchCompletedCallback)(this->hSearch);
		}
	}
}

void CContactSearch::OnNewResult(const ContactRef& contact, const uint& rankValue)
{
	if (this->proto)
		(proto->*ContactFindedCallback)(this->hSearch, contact->ref());
}

void CContactSearch::BlockWhileSearch()
{
	this->isSeachFinished = false;
	this->isSeachFailed = false;
	while (!this->isSeachFinished && !this->isSeachFailed) 
		Sleep(1); 
}

void CContactSearch::SetProtoInfo(CSkypeProto* proto, HANDLE hSearch)
{
	this->proto = proto;
	this->hSearch = hSearch;
}

void CContactSearch::SetOnSearchCompleatedCallback(OnSearchCompleted callback)
{
	this->SearchCompletedCallback = callback;
}

void CContactSearch::SetOnContactFindedCallback(OnContactFinded callback)
{
	this->ContactFindedCallback = callback;
}

// CParticipant

CParticipant::CParticipant(unsigned int oid, SERootObject* root) : Participant(oid, root) { }

SEString CParticipant::GetRankName(CParticipant::RANK rank)
{
	char *result = NULL;
	switch (rank)
	{
	case CParticipant::CREATOR:
		result = "Creator";
		break;
	case CParticipant::ADMIN:
		result = "Admin";
		break;
	case CParticipant::SPEAKER:
		result = "Speaker";
		break;
	case CParticipant::WRITER:
		result = "Writer";
		break;
	case CParticipant::SPECTATOR:
		result = "Spectator";
		break;
	case CParticipant::RETIRED:
		result = "Retried";
		break;
	case CParticipant::OUTLAW:
		result = "Outlaw";
		break;
	}
	return result;
}

// CContact

CContact::CContact(unsigned int oid, SERootObject* root) : Contact(oid, root) 
{
	this->proto = NULL;
	this->callback == NULL;
}

void CContact::SetOnContactChangedCallback(OnContactChanged callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->callback = callback;
}

//bool CContact::SentAuthRequest(SEString message)
//{
//	this->SetBuddyStatus(Contact::AUTHORIZED_BY_ME);
//	this->SendAuthRequest(message);
//}

void CContact::OnChange(int prop)
{
	if (this->proto)
		(proto->*callback)(this->ref(), prop);
}

// Conversation

CConversation::CConversation(unsigned int oid, SERootObject* root) : Conversation(oid, root) 
{
	this->proto = NULL;
	this->messageReceivedCallback = NULL;
}

void CConversation::OnMessage(const MessageRef & message)
{
	if (this->proto)
		(proto->*messageReceivedCallback)(message->ref());
}

CConversation::Ref CConversation::FindBySid(CSkype *skype, SEString sid)
{
	SEStringList participants;
	participants.append(sid);
	
	CConversation::Ref conversation;
	skype->GetConversationByParticipants(participants, conversation);

	return conversation;
}

void CConversation::SetOnMessageReceivedCallback(OnMessageReceived callback, CSkypeProto* proto)
{
	this->proto = proto;
	this->messageReceivedCallback = callback;
}

// CMessage

CMessage::CMessage(unsigned int oid, SERootObject* root) : Message(oid, root) { }