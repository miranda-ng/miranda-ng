#include "stdafx.h"

void CSlackProto::InitNetlib()
{
	wchar_t name[128];
	mir_snwprintf(name, TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	debugLogA(__FUNCTION__":Setting protocol / module name to '%s'", m_szModuleName);
}

void CSlackProto::UninitNetlib()
{
	Netlib_CloseHandle(hNetlib);
	hNetlib = NULL;
}
