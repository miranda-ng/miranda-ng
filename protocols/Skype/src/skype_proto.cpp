#include "skype_proto.h"

CSkypeProto::CSkypeProto(const char* protoName, const TCHAR* userName)
{
	this->isOffline = true;
	this->m_iVersion = 2;
	this->m_iStatus = ID_STATUS_OFFLINE;
	this->m_tszUserName = mir_tstrdup(userName);
	this->m_szModuleName = mir_strdup(protoName);
	this->m_szProtoName = mir_strdup(protoName);
	_strlwr(m_szProtoName);
	this->m_szProtoName[0] = toupper(m_szProtoName[0]);

	TCHAR name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), this->m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof( nlu );
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	this->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->Log("Setting protocol/module name to '%s/%s'", m_szProtoName, m_szModuleName);

	this->CreateProtoService(PS_CREATEACCMGRUI, &CSkypeProto::SvcCreateAccMgrUI);
}

CSkypeProto::~CSkypeProto()
{
	Netlib_CloseHandle(this->hNetlibUser);
	this->hNetlibUser = NULL;

	mir_free(this->m_szProtoName);
	mir_free(this->m_szModuleName);
	mir_free(this->m_tszUserName);
}

INT_PTR CALLBACK SkypeAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CSkypeProto *proto;

	switch ( message )
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<CSkypeProto*>(lparam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);

		DBVARIANT dbv;
		if ( !DBGetContactSettingString(0, proto->ModuleName(), "SkypeName", &dbv))
		{
			SetDlgItemText(hwnd, IDC_SN, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		if ( !DBGetContactSettingString(0, proto->ModuleName(), "Password", &dbv))
		{
			CallService(
				MS_DB_CRYPT_DECODESTRING,
				wcslen(dbv.ptszVal) + 1,
				reinterpret_cast<LPARAM>(dbv.ptszVal));
			SetDlgItemText(hwnd, IDC_PW, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		if ( !proto->IsOffline()) 
		{
			SendMessage(GetDlgItem(hwnd, IDC_SN), EM_SETREADONLY, 1, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0); 
		}

		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus())
		{
			switch(LOWORD(wparam))
			{
			case IDC_SN:
			case IDC_PW:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			proto = reinterpret_cast<CSkypeProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			TCHAR str[128];

			GetDlgItemText(hwnd, IDC_SN, str, sizeof(str));
			DBWriteContactSettingTString(0, proto->ModuleName(), "SkypeName", str);

			GetDlgItemText(hwnd, IDC_PW, str, sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), reinterpret_cast<LPARAM>(str));
			DBWriteContactSettingTString(0, proto->ModuleName(), "Password", str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR __cdecl CSkypeProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (int)CreateDialogParam(
		g_hInstance, 
		MAKEINTRESOURCE(IDD_SKYPEACCOUNT), 
		(HWND)lParam, 
		::SkypeAccountProc, (LPARAM)this);
}

HANDLE __cdecl CSkypeProto::AddToList( int flags, PROTOSEARCHRESULT* psr ) { return 0; }
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

DWORD_PTR __cdecl CSkypeProto:: GetCaps( int type, HANDLE hContact ) { return 0; }
HICON  __cdecl CSkypeProto::GetIcon( int iconIndex ) { return 0; }
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
	switch(new_status)
	{
	case ID_STATUS_OFFLINE:	
		if ( !this->isOffline)
		{
			this->isOffline = true;
		   account->Logout(false);
		};
		break;

	case ID_STATUS_ONLINE:
		if (g_skype->GetAccount("", this->account))
		{
			this->isOffline = false;
			account->LoginWithPassword("", false, false);

			// Loop until LoggedIn or login failure
			//while ( (!account->loggedIn) && (!account->loggedOut) ) { Delay(1); };
		}
		break;
	}

	return 0;
}

HANDLE __cdecl CSkypeProto::GetAwayMsg( HANDLE hContact ) { return 0; }
int    __cdecl CSkypeProto::RecvAwayMsg( HANDLE hContact, int mode, PROTORECVEVENT* evt ) { return 0; }
int    __cdecl CSkypeProto::SendAwayMsg( HANDLE hContact, HANDLE hProcess, const char* msg ) { return 0; }
int    __cdecl CSkypeProto::SetAwayMsg( int m_iStatus, const TCHAR* msg ) { return 0; }

int    __cdecl CSkypeProto::UserIsTyping( HANDLE hContact, int type ) { return 0; }

int    __cdecl CSkypeProto::OnEvent( PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam ) { return 0; }

void CSkypeProto::Log(const char* fmt, ...)
{
	va_list va;
	char msg[1024];

	va_start(va, fmt);
	mir_vsnprintf(msg, sizeof(msg), fmt, va);
	va_end(va);

	CallService(MS_NETLIB_LOG, ( WPARAM )this->hNetlibUser, (LPARAM)msg);
}

void CSkypeProto::CreateProtoService(const char* szService, SkypeServiceFunc serviceProc)
{
	char temp[MAX_PATH*2];

	mir_snprintf(temp, sizeof(temp), "%s%s", this->m_szModuleName, szService);
	CreateServiceFunctionObj(temp, (MIRANDASERVICEOBJ)*(void**)&serviceProc, this);
}

char* CSkypeProto::ModuleName()
{
	return this->m_szProtoName;
}

bool CSkypeProto::IsOffline()
{
	return this->isOffline;
}