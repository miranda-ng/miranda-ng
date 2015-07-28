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
	if (m_pollingConnection)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_pollingConnection, 0);
	if (m_TrouterConnection)
		CallService(MS_NETLIB_SHUTDOWN, (WPARAM)m_TrouterConnection, 0);

	Netlib_CloseHandle(m_hNetlibUser); m_hNetlibUser = NULL;
}