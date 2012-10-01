#include "skype_proto.h"

void CSkypeProto::InitNetLib()
{
	wchar_t name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), this->m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = this->m_szModuleName;
	this->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->Log("Setting protocol/module name to '%s/%s'", m_szProtoName, m_szModuleName);
}

void CSkypeProto::UninitNetLib()
{
	Netlib_CloseHandle(this->hNetlibUser);
	this->hNetlibUser = NULL;
}

void CSkypeProto::Log(const char* fmt, ...)
{
	va_list va;
	char msg[1024];

	va_start(va, fmt);
	mir_vsnprintf(msg, sizeof(msg), fmt, va);
	va_end(va);

	CallService(MS_NETLIB_LOG, (WPARAM)this->hNetlibUser, (LPARAM)msg);
}