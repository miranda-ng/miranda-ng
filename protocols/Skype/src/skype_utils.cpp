#include "skype_proto.h"

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