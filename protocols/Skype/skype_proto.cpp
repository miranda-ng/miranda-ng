#include "skype_proto.h"

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