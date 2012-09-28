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

	this->signin_lock = CreateMutex(0, false, 0);

	TCHAR name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), this->m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof( nlu );
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	this->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->Log("Setting protocol/module name to '%s/%s'", m_szProtoName, m_szModuleName);

	this->CreateService(PS_CREATEACCMGRUI, &CSkypeProto::OnAccountManagerInit);
}

CSkypeProto::~CSkypeProto()
{
	Netlib_CloseHandle(this->hNetlibUser);
	this->hNetlibUser = NULL;

	CloseHandle(this->signin_lock);

	mir_free(this->m_szProtoName);
	mir_free(this->m_szModuleName);
	mir_free(this->m_tszUserName);
}

HANDLE __cdecl CSkypeProto::AddToList(int flags, PROTOSEARCHRESULT* psr) 
{
	if (psr->cbSize != sizeof(PROTOSEARCHRESULT))
		return 0;

	return this->AddToListBySkypeLogin(psr->id, psr->nick, psr->firstName, psr->lastName, flags);
}

HANDLE __cdecl CSkypeProto::AddToListByEvent( int flags, int iContact, HANDLE hDbEvent ) { return 0; }
int    __cdecl CSkypeProto::Authorize( HANDLE hDbEvent ) { return 0; }
int    __cdecl CSkypeProto::AuthDeny( HANDLE hDbEvent, const TCHAR* szReason ) { return 0; }
int    __cdecl CSkypeProto::AuthRecv( HANDLE hContact, PROTORECVEVENT* ) { return 0; }
int    __cdecl CSkypeProto::AuthRequest( HANDLE hContact, const TCHAR* szMessage ) { return 0; }

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
		return PF1_IM | PF1_PEER2PEER;
	case PFLAGNUM_2:
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_INVISIBLE;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH | PF4_FORCEADDED | PF4_SUPPORTTYPING | PF4_AVATARS | 
			PF4_OFFLINEFILES | PF4_IMSENDUTF | PF4_IMSENDOFFLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)Translate("Skype login");
	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR) "SL";
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

HANDLE __cdecl CSkypeProto::SearchBasic( const TCHAR* id ) { return 0; }
HANDLE __cdecl CSkypeProto::SearchByEmail( const TCHAR* email ) { return 0; }
HANDLE __cdecl CSkypeProto::SearchByName( const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName ) { return 0; }
HWND   __cdecl CSkypeProto::SearchAdvanced( HWND owner ) { return 0; }
HWND   __cdecl CSkypeProto::CreateExtendedSearchUI( HWND owner ) { return 0; }

int    __cdecl CSkypeProto::RecvContacts( HANDLE hContact, PROTORECVEVENT* ) { return 0; }
int    __cdecl CSkypeProto::RecvFile( HANDLE hContact, PROTORECVFILET* ) { return 0; }
int    __cdecl CSkypeProto::RecvMsg( HANDLE hContact, PROTORECVEVENT* ) { return 0; }
int    __cdecl CSkypeProto::RecvUrl( HANDLE hContact, PROTORECVEVENT* ) { return 0; }

int    __cdecl CSkypeProto::SendContacts( HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList ) { return 0; }
HANDLE __cdecl CSkypeProto::SendFile( HANDLE hContact, const TCHAR* szDescription, TCHAR** ppszFiles ) { return 0; }
int    __cdecl CSkypeProto::SendMsg( HANDLE hContact, int flags, const char* msg ) { return 0; }
int    __cdecl CSkypeProto::SendUrl( HANDLE hContact, int flags, const char* url ) { return 0; }

int    __cdecl CSkypeProto::SetApparentMode( HANDLE hContact, int mode ) { return 0; }

int CSkypeProto::SetStatus(int new_status)
{
	if (new_status == this->m_iStatus)
		return 0;

	int old_status = this->m_iStatus;
	this->m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE && old_status != ID_STATUS_OFFLINE)
	{
		this->m_iStatus = new_status;
		//todo: set all status to offline
		this->account->Logout(true);
		this->account->BlockWhileLoggingOut();
		this->account->SetAvailability(CContact::OFFLINE);
	}
	else 
	{
		this->m_iStatus = new_status;
		if (old_status == ID_STATUS_OFFLINE)
		{
			this->login = this->GetSettingString(SKYPE_SETTINGS_LOGIN);
			if (g_skype->GetAccount(mir_u2a(login), this->account))
			{
				this->m_iStatus = ID_STATUS_CONNECTING;
				this->password = this->GetDecodeSettingString(SKYPE_SETTINGS_PASSWORD);
			
				this->ForkThread(&CSkypeProto::SignIn, this);
			}
		}

		CContact::AVAILABILITY availability = CContact::UNKNOWN;
		switch(this->m_iStatus)
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

		if(availability != CContact::UNKNOWN)
			this->account->SetAvailability(availability);	
	}

	this->SendBroadcast(ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, this->m_iStatus); 

	return 0;
}

HANDLE __cdecl CSkypeProto::GetAwayMsg(HANDLE hContact) { return 0; }
int    __cdecl CSkypeProto::RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt ) { return 0; }
int    __cdecl CSkypeProto::SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg ) { return 0; }
int    __cdecl CSkypeProto::SetAwayMsg( int m_iStatus, const TCHAR* msg ) { return 0; }

int    __cdecl CSkypeProto::UserIsTyping( HANDLE hContact, int type ) { return 0; }

int    __cdecl CSkypeProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType) 
	{
	case EV_PROTO_ONLOAD:
		return this->OnModulesLoaded(0, 0);
	
	case EV_PROTO_ONEXIT: 
		return this->OnPreShutdown(0, 0);
	}

	return 1;
}


char* CSkypeProto::ModuleName()
{
	return this->m_szModuleName;
}

bool CSkypeProto::IsOffline()
{
	return this->m_iStatus == ID_STATUS_OFFLINE;
}

void __cdecl CSkypeProto::SignIn(void*)
{
	WaitForSingleObject(&this->signin_lock, INFINITE);

	this->account->LoginWithPassword(mir_u2a(this->password), false, false);
	this->account->BlockWhileLoggingIn();
	this->SetStatus(this->m_iDesiredStatus);
	this->ForkThread(&CSkypeProto::LoadContactList, this);

	ReleaseMutex(this->signin_lock);
}

void __cdecl CSkypeProto::LoadContactList(void*)
{
	CContactGroup::Ref contacts;
	g_skype->GetHardwiredContactGroup(CContactGroup::ALL_KNOWN_CONTACTS, contacts);

    contacts->GetContacts(contacts->ContactList);
    fetch(contacts->ContactList);

    for (unsigned int i = 0; i < contacts->ContactList.size(); i++)
    {
		SEString contactName;
        contacts->ContactList[i]->GetPropDisplayname(contactName);
        printf("%3d. %s\n", i+1, (const char*)contactName);

		//HANDLE hContact = AddToContactList(fbu, FACEBOOK_CONTACT_APPROVE, false, fbu->real_name.c_str());
		//DBWriteContactSettingByte(hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, FACEBOOK_CONTACT_APPROVE);
	}
}

HANDLE CSkypeProto::AddToListBySkypeLogin(TCHAR* skypeName, TCHAR* nickName, TCHAR* firstName, TCHAR* lastName, DWORD flags)
{
	//if (!skypeName)
		return NULL;

	/*BOOL bAdded;
	HANDLE hContact = MraHContactFromEmail( _T2A(plpsEMail), lstrlen(plpsEMail), TRUE, TRUE, &bAdded);
	if (hContact) {
		if (nickName)
			mraSetStringW(hContact, "Nick", nickName);
		if (firstName)
			mraSetStringW(hContact, "FirstName", firstName);
		if (lastName)
			mraSetStringW(hContact, "LastName", lastName);

		if (flags & PALF_TEMPORARY)
			DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
		else
			DBDeleteContactSetting(hContact, "CList", "NotOnList");

		if (bAdded)
			MraUpdateContactInfo(hContact);
	}

	return hContact;*/
}