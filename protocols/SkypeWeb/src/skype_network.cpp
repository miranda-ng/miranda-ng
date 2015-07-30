#include "stdafx.h"

void CSkypeProto::InitNetwork()
{
	wchar_t name[128];
	mir_sntprintf(name, _countof(name), TranslateT("%s connection"), m_tszUserName);
	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

void CSkypeProto::UnInitNetwork()
{
	ShutdownConnections();
	Netlib_CloseHandle(m_hNetlibUser); m_hNetlibUser = NULL;
}

void CSkypeProto::ShutdownConnections()
{
	Netlib_Shutdown(m_hNetlibUser);
}