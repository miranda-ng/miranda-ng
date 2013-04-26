#include "skype_proto.h"

void CSkypeProto::InitNetLib()
{
	wchar_t name[128];
	::mir_sntprintf(name, SIZEOF(name), ::TranslateT("%s connection"), this->m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = this->m_szModuleName;
	this->hNetLibUser = (HANDLE)::CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->Log(L"Setting protocol/module name to '%s'", (TCHAR*)_A2T(m_szModuleName));
}

void CSkypeProto::UninitNetLib()
{
	::Netlib_CloseHandle(this->hNetLibUser);
	this->hNetLibUser = NULL;
}

void CSkypeProto::Log(const wchar_t *fmt, ...)
{
	va_list va;
	wchar_t msg[1024];

	va_start(va, fmt);
	::mir_vsntprintf(msg, SIZEOF(msg), fmt, va);
	va_end(va);

	::CallService(MS_NETLIB_LOGW, (WPARAM)this->hNetLibUser, (LPARAM)msg);
}