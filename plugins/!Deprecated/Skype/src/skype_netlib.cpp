#include "skype.h"

void CSkypeProto::InitNetLib()
{
	wchar_t name[128];
	::mir_sntprintf(name, SIZEOF(name), ::TranslateT("%s connection"), this->m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = this->m_szModuleName;
	this->m_hNetlibUser = (HANDLE)::CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->debugLogW(L"Setting protocol/module name to '%s'", (TCHAR*)_A2T(m_szModuleName));
}

void CSkypeProto::UninitNetLib()
{
	::Netlib_CloseHandle(this->m_hNetlibUser);
	this->m_hNetlibUser = NULL;
}
