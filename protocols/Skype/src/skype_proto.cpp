#include "skype_proto.h"

CSkypeProto::CSkypeProto(const char* protoName, const TCHAR* userName)
{
	this->m_iVersion = 2;
	this->m_iStatus = ID_STATUS_OFFLINE;
	this->m_tszUserName = mir_tstrdup(userName);
	this->m_szModuleName = mir_strdup(protoName);
	this->m_szProtoName = mir_strdup(protoName);
	//_strlwr(m_szProtoName);
	//this->m_szProtoName[0] = toupper(m_szProtoName[0]);

	//this->login = NULL;
	this->password = NULL;
	this->rememberPassword = false;

	this->signin_lock = CreateMutex(0, false, 0);
	this->SetAllContactStatus(ID_STATUS_OFFLINE);

	this->CreateService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);

	this->InitNetLib();
}

CSkypeProto::~CSkypeProto()
{
	this->UninitNetLib();

	CloseHandle(this->signin_lock);

	::mir_free(this->login);
	::mir_free(this->password);

	::mir_free(this->m_szProtoName);
	::mir_free(this->m_szModuleName);
	::mir_free(this->m_tszUserName);
}

HANDLE __cdecl CSkypeProto::AddToList(int flags, PROTOSEARCHRESULT* psr) 
{
	return this->AddContactBySid(psr->id, psr->nick, 0);
}

HANDLE __cdecl CSkypeProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent) 
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);

	/*if ((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) != -1) 
	{
		dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
		if (CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei) == 0 &&
				!strcmp(dbei.szModule, m_szModuleName) &&
				(dbei.eventType == EVENTTYPE_AUTHREQUEST || dbei.eventType == EVENTTYPE_CONTACTS)) 
		{
			char *nick = (char*)(dbei.pBlob + sizeof(DWORD) * 2);
			char *firstName = nick + strlen(nick) + 1;
			char *lastName = firstName + strlen(firstName) + 1;
			char *skypeName = lastName + strlen(lastName) + 1;
			return AddContactBySkypeName(::mir_a2u(skypeName), ::mir_a2u(nick), 0);
		}
	}*/
	return 0;
}

int __cdecl CSkypeProto::Authorize(HANDLE hDbEvent) 
{
	if (this->IsOnline() && hDbEvent)
	{
		HANDLE hContact = this->GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_HANDLE_VALUE)
			return 1;

		return CSkypeProto::GrantAuth((WPARAM)hContact, NULL);
	}

	return 1;
}

int __cdecl CSkypeProto::AuthDeny(HANDLE hDbEvent, const TCHAR* szReason) 
{
	if (this->IsOnline())
	{
		HANDLE hContact = this->GetContactFromAuthEvent(hDbEvent);
		if (hContact == INVALID_HANDLE_VALUE)
			return 1;

		return CSkypeProto::RevokeAuth((WPARAM)hContact, NULL);
	}

	return 1; 
}

int __cdecl CSkypeProto::AuthRecv(HANDLE hContact, PROTORECVEVENT* pre) 
{
	DWORD flags = 0;

	if (pre->flags & PREF_CREATEREAD) 
		flags |= DBEF_READ;

	if (pre->flags & PREF_UTF) 
		flags |= DBEF_UTF;

	this->AddDataBaseEvent(
		hContact, 
		EVENTTYPE_AUTHREQUEST, 
		pre->timestamp, 
		flags, 
		pre->lParam, 
		(PBYTE)pre->szMessage);

	return 0;
}

int __cdecl CSkypeProto::AuthRequest(HANDLE hContact, const TCHAR* szMessage) 
{
	if (this->IsOnline() && hContact)
	{
		CContact::Ref contact;
		SEString sid(::mir_u2a(this->GetSettingString(hContact, "sid")));
		if (g_skype->GetContact(sid, contact))
			contact->SendAuthRequest(::mir_u2a(szMessage));
		
		return 0;
	}

	return 1;
}

HANDLE __cdecl CSkypeProto::ChangeInfo( int iInfoType, void* pInfoData ) { return 0; }

HANDLE __cdecl CSkypeProto::FileAllow( HANDLE hContact, HANDLE hTransfer, const TCHAR* szPath ) { return 0; }
int    __cdecl CSkypeProto::FileCancel( HANDLE hContact, HANDLE hTransfer ) { return 0; }
int    __cdecl CSkypeProto::FileDeny( HANDLE hContact, HANDLE hTransfer, const TCHAR* szReason ) { return 0; }
int    __cdecl CSkypeProto::FileResume( HANDLE hTransfer, int* action, const TCHAR** szFilename ) { return 0; }

DWORD_PTR __cdecl CSkypeProto:: GetCaps(int type, HANDLE hContact) 
{ 
	switch(type)
	{        
	case PFLAGNUM_1:
		return PF1_IM  | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_SEARCHBYEMAIL/* | PF1_SEARCHBYNAME*/;
	case PFLAGNUM_2:
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_AVATARS | 
			PF4_OFFLINEFILES | PF4_IMSENDUTF | PF4_IMSENDOFFLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("Skype Name");
	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR) "sid";
	default:
		return 0;
	}
}

HICON  __cdecl CSkypeProto::GetIcon( int iconIndex )
{
	if (LOWORD(iconIndex) == PLI_PROTOCOL)
	{
		HICON ico = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"Skype_main");
		return CopyIcon(ico);
	} else
		return 0;
}

int    __cdecl CSkypeProto::GetInfo( HANDLE hContact, int infoType ) { return 0; }

HANDLE __cdecl CSkypeProto::SearchBasic(const TCHAR* id) 
{ 
	if ( !this->IsOnline())
		return 0;

	wchar_t *data = ::mir_tstrdup(id);
	this->ForkThread(&CSkypeProto::SearchBySidAsync, data);

	return (HANDLE)SKYPE_SEARCH_BYSID;
}

HANDLE __cdecl CSkypeProto::SearchByEmail(const TCHAR* email) 
{ 
	if ( !this->IsOnline())
		return 0;

	wchar_t *data = ::mir_tstrdup(email);
	this->ForkThread(&CSkypeProto::SearchByEmailAsync, data);

	return (HANDLE)SKYPE_SEARCH_BYEMAIL;
}
HANDLE __cdecl CSkypeProto::SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName) 
{ 
	PROTOSEARCHRESULT isr = {0};
	isr.cbSize = sizeof(isr);
	isr.flags = PSR_TCHAR;
	isr.nick = ::mir_wstrdup(nick);
	isr.firstName = ::mir_wstrdup(firstName);
	isr.lastName = ::mir_wstrdup(lastName);

	this->ForkThread(&CSkypeProto::SearchByNamesAsync, &isr);

	return (HANDLE)SKYPE_SEARCH_BYNAMES;
}
HWND   __cdecl CSkypeProto::SearchAdvanced( HWND owner ) { return 0; }

HWND   __cdecl CSkypeProto::CreateExtendedSearchUI( HWND owner ){ return 0; }

int    __cdecl CSkypeProto::RecvContacts( HANDLE hContact, PROTORECVEVENT* ) { return 0; }
int    __cdecl CSkypeProto::RecvFile( HANDLE hContact, PROTORECVFILET* ) { return 0; }

int    __cdecl CSkypeProto::RecvMsg( HANDLE hContact, PROTORECVEVENT* pre) 
{ 
	this->UserIsTyping(hContact, PROTOTYPE_SELFTYPING_OFF);
	return ::Proto_RecvMessage(hContact, pre);
}

int    __cdecl CSkypeProto::RecvUrl( HANDLE hContact, PROTORECVEVENT* ) { return 0; }

int    __cdecl CSkypeProto::SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList ) { return 0; }
HANDLE __cdecl CSkypeProto::SendFile( HANDLE hContact, const TCHAR* szDescription, TCHAR** ppszFiles ) { return 0; }

int    __cdecl CSkypeProto::SendMsg(HANDLE hContact, int flags, const char* msg) 
{ 
	int result = ::InterlockedIncrement((LONG volatile*)&dwCMDNum);

	CConversation::Ref conversation;
	g_skype->GetConversationByIdentity(::mir_u2a(this->GetSettingString(hContact, "sid")), conversation);
	if (conversation) 
	{
		Message::Ref message;
		conversation->PostText(msg, message);
	}

	this->SendBroadcastAsync(
		hContact,
		ACKTYPE_MESSAGE,
		ACKRESULT_SUCCESS,
		(HANDLE)result,
		0);
	
	return result; 
}

int    __cdecl CSkypeProto::SendUrl( HANDLE hContact, int flags, const char* url ) { return 0; }

int    __cdecl CSkypeProto::SetApparentMode( HANDLE hContact, int mode ) { return 0; }

int CSkypeProto::SetStatus(int new_status)
{
	if (new_status == this->m_iStatus)
		return 0;

	int old_status = this->m_iStatus;
	this->m_iDesiredStatus = new_status;

	switch (new_status)
	{
	case ID_STATUS_OFFLINE:
		if ( this->account->IsOnline())
		{
			this->account->SetAvailability(CContact::OFFLINE);
			this->account->Logout(true);
			this->account->BlockWhileLoggingOut();

			this->m_iStatus = new_status;
			this->SetAllContactStatus(ID_STATUS_OFFLINE);
		}
		break;

	default:
		if (old_status == ID_STATUS_OFFLINE && !this->IsOnline())
		{
			this->m_iStatus = ID_STATUS_CONNECTING;
			if ( !this->SignIn()) return 0;

		}
		else
		{
			CContact::AVAILABILITY availability = this->MirandaToSkypeStatus(new_status);
			if (availability != CContact::UNKNOWN)
				this->account->SetAvailability(availability);

			this->m_iStatus = new_status;
		}
		
		break;
	}

	this->SetSettingWord("Status", this->m_iStatus);
	this->SendBroadcastAsync(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, this->m_iStatus); 

	return 0;
}

HANDLE __cdecl CSkypeProto::GetAwayMsg(HANDLE hContact) { return 0; }
int    __cdecl CSkypeProto::RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt ) { return 0; }
int    __cdecl CSkypeProto::SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg ) { return 0; }
int    __cdecl CSkypeProto::SetAwayMsg( int m_iStatus, const TCHAR* msg ) { return 0; }

int    __cdecl CSkypeProto::UserIsTyping( HANDLE hContact, int type ) 
{ 
	if (hContact && this->IsOnline() && this->m_iStatus != ID_STATUS_INVISIBLE)
	{
		CConversation::Ref conversation;
		g_skype->GetConversationByIdentity(::mir_u2a(this->GetSettingString(hContact, "sid")), conversation);
		if (conversation) 
		{
			switch (type) 
			{
				case PROTOTYPE_SELFTYPING_ON:
					conversation->SetMyTextStatusTo(Participant::WRITING);
					return 0;

				case PROTOTYPE_SELFTYPING_OFF:
					conversation->SetMyTextStatusTo(Participant::READING); // mb TEXT_UNKNOWN?
					return 0;
			}
		}
	}
	return 1; 
}

int    __cdecl CSkypeProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) 
	{
	case EV_PROTO_ONLOAD:
		return this->OnModulesLoaded(wParam, lParam);
	
	case EV_PROTO_ONEXIT: 
		return this->OnPreShutdown(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return this->OnContactDeleted(wParam, lParam);
	}

	return 1;
}

void __cdecl CSkypeProto::SignInAsync(void*)
{
	//WaitForSingleObject(&this->signin_lock, INFINITE);

	this->account->LoginWithPassword(::mir_u2a(this->password), false, false);
	this->account->BlockWhileLoggingIn();
	if ( !this->rememberPassword)
	{
		for (int i = ::wcslen(this->password); i >= 0; i--)
			this->password[i] = L'\0';
	}

	if (this->account->isLoggedOut)
	{
		this->m_iStatus = ID_STATUS_OFFLINE;
		this->SendBroadcast(
			ACKTYPE_LOGIN,
			ACKRESULT_FAILED,
			NULL, 
			this->SkypeToMirandaLoginError(this->account->logoutReason));
		this->ShowNotification(
			NULL, 
			::mir_a2u(this->account->logoutReasonString));
	}
	else
	{
		g_skype->GetConversationList(g_skype->inbox, CConversation::INBOX_CONVERSATIONS);
		fetch(g_skype->inbox);
		g_skype->SetOnConversationAddedCallback(
			(CSkype::OnConversationAdded)&CSkypeProto::OnConversationAdded, this);
		for (uint i = 0 ; i < g_skype->inbox.size(); i++)
		{
			g_skype->inbox[i]->SetOnMessageReceivedCallback(
				(CConversation::OnMessageReceived)&CSkypeProto::OnOnMessageReceived, this);
		}

		this->SetStatus(this->m_iDesiredStatus);
		this->ForkThread(&CSkypeProto::LoadContactList, this);
		//this->LoadContactList(this);
		
		this->account.fetch();
		this->account->SetOnAccountChangedCallback(
			(CAccount::OnAccountChanged)&CSkypeProto::OnAccountChanged, this);

		//this->ForkThread(&CSkypeProto::LoadOwnInfo, this);
		this->LoadOwnInfo(this);
	}

	//ReleaseMutex(this->signin_lock);
}

bool CSkypeProto::SignIn(bool isReadPassword)
{
	if (::wcscmp(this->login, L"") == 0)
	{
		this->m_iStatus = ID_STATUS_OFFLINE;
		this->SendBroadcast(ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
		::MessageBox(
			NULL, 
			TranslateT("You have not entered a Skype name.\nConfigure this in Options->Network->Skype and try again."),
			_T("Skype"),
			MB_OK);
	}
	else if (g_skype->GetAccount(::mir_u2a(this->login), this->account))
	{
		/*this->m_iStatus = ID_STATUS_CONNECTING;
		this->SendBroadcast(ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, this->m_iStatus); */
		if (isReadPassword)
			this->password = this->GetDecodeSettingString(SKYPE_SETTINGS_PASSWORD);				
		if (::wcscmp(this->password, L"") == 0)
			this->RequestPassword();
		else
		{
			this->ForkThread(&CSkypeProto::SignInAsync, this);
			//this->SignInAsync(this);
			return true;
		}
	}

	return false;
}

bool  CSkypeProto::IsOnline()
{
	return this->account && this->account->IsOnline();
	//return this->m_iStatus != ID_STATUS_OFFLINE;
}

void CSkypeProto::RequestPassword()
{
	::DialogBoxParam(
		g_hInstance, 
		MAKEINTRESOURCE(IDD_PASSWORDREQUEST), 
		NULL, 
		CSkypeProto::SkypePasswordProc, 
		LPARAM(this));
}

void CSkypeProto::OnOnMessageReceived(const char *sid, const char *text)
{
	this->RaiseMessageReceivedEvent(time(NULL), sid, sid, text);
}

void CSkypeProto::OnConversationAdded(CConversation::Ref conversation)
{
	conversation->SetOnMessageReceivedCallback(
		(CConversation::OnMessageReceived)&CSkypeProto::OnOnMessageReceived, this);
}