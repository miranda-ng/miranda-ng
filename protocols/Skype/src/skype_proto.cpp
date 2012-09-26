#include "skype.h"

CSkypeProto::CSkypeProto(const char* protoName, const TCHAR* userName)
{
	m_iVersion = 2;
	m_iStatus = ID_STATUS_OFFLINE;
	m_tszUserName = mir_tstrdup(userName);
	m_szModuleName = mir_strdup(protoName);
	m_szProtoName = mir_strdup(protoName);
	_strlwr(m_szProtoName);
	m_szProtoName[0] = toupper(m_szProtoName[0]);

	TCHAR name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof( nlu );
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->Log("Setting protocol/module name to '%s/%s'", m_szProtoName, m_szModuleName);
}

CSkypeProto::~CSkypeProto()
{
	Netlib_CloseHandle(m_hNetlibUser);
	m_hNetlibUser = NULL;

	mir_free(m_szProtoName);
	mir_free(m_szModuleName);
	mir_free(m_tszUserName);
}

void CSkypeProto::Log( const char* fmt, ... )
{
	va_list va;
	char msg[1024];

	va_start(va, fmt);
	mir_vsnprintf(msg, sizeof(msg), fmt, va);
	va_end(va);

	CallService(MS_NETLIB_LOG, ( WPARAM )m_hNetlibUser, (LPARAM)msg);
}

void CSkypeProto::CreateProtoService(const char *szService, ServiceFunc serviceProc)
{
	char str[ MAXMODULELABELLENGTH ];
	strcpy( str, m_szModuleName );
	strcat( str, szService );
	::CreateServiceFunctionObj( str, ( MIRANDASERVICEOBJ )*( void** )&serviceProc, this );
}

HANDLE CSkypeProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return NULL/*hContact*/;
}

HANDLE CSkypeProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent)
{
	return NULL;
}

int CSkypeProto::Authorize(HANDLE hDbEvent)
{
	return NULL;
}

int CSkypeProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR *reason)
{
	return NULL;
}

int CSkypeProto::AuthRecv(HANDLE hContact, PROTORECVEVENT* pre)
{
	return NULL;
}

int CSkypeProto::AuthRequest(HANDLE hContact, const TCHAR *lptszMessage)
{
	return NULL;
}

HANDLE CSkypeProto::ChangeInfo(int type, void *info_data)
{
	return NULL;
}

HANDLE CSkypeProto::FileAllow(HANDLE hContact, HANDLE hTransfer, const TCHAR *szPath)
{
	return NULL;
}

int CSkypeProto::FileCancel(HANDLE hContact, HANDLE hTransfer)
{
	return NULL;
}

int CSkypeProto::FileDeny(HANDLE hContact, HANDLE hTransfer, const TCHAR*)
{
	return NULL;
}

int CSkypeProto::FileResume(HANDLE, int*, const TCHAR**)
{
	return 1;
}

DWORD_PTR CSkypeProto::GetCaps(int type, HANDLE hContact)
{
	return NULL;
}

HICON CSkypeProto::GetIcon(int iconIndex)
{
	return NULL;
}

int CSkypeProto::GetInfo(HANDLE hContact, int infoType)
{
	return NULL;
}

HANDLE CSkypeProto::SearchBasic(const TCHAR *id)
{
	return NULL;
}

HANDLE CSkypeProto::SearchByEmail(const TCHAR *email)
{
	return NULL;
}

HANDLE CSkypeProto::SearchByName(const TCHAR *pszNick, const TCHAR *pszFirstName, const TCHAR *pszLastName)
{
	return NULL;
}

HWND CSkypeProto::SearchAdvanced(HWND hWndDlg)
{
	return NULL;
}

HWND CSkypeProto::CreateExtendedSearchUI(HWND owner)
{
	return NULL;
}

int CSkypeProto::RecvContacts(HANDLE hContact, PROTORECVEVENT* pre)
{
	return NULL;
}

int CSkypeProto::RecvFile(HANDLE hContact, PROTORECVFILET *pre)
{
	return NULL;
}

int CSkypeProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	return NULL;
}

int CSkypeProto::RecvUrl(HANDLE, PROTORECVEVENT*)
{
	return 1;
}

int CSkypeProto::SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList)
{
	return NULL;
}

HANDLE CSkypeProto::SendFile(HANDLE hContact, const TCHAR* szDescription, TCHAR** ppszFiles)
{
	return NULL;
}

int CSkypeProto::SendMsg(HANDLE hContact, int flags, const char *lpszMessage)
{
	return NULL;
}

int CSkypeProto::SendUrl(HANDLE, int, const char*)
{
	return 1;
}

int CSkypeProto::SetApparentMode(HANDLE hContact, int mode)
{
	return NULL;
}

int CSkypeProto::SetStatus(int iNewStatus)
{
	return NULL;
}

HANDLE CSkypeProto::GetAwayMsg(HANDLE hContact)
{
	return NULL;
}

int CSkypeProto::RecvAwayMsg(HANDLE, int, PROTORECVEVENT*)
{
	return 1;
}

int CSkypeProto::SendAwayMsg(HANDLE, HANDLE, const char* )
{
	return 1;
}

int CSkypeProto::SetAwayMsg(int m_iStatus, const TCHAR* msg)
{
	return NULL;
}

int CSkypeProto::UserIsTyping(HANDLE hContact, int type)
{
	return NULL;
}

int CSkypeProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	return NULL;
}