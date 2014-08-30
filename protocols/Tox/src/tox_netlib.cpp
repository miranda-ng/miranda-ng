#include "common.h"

void CToxProto::InitNetlib()
{
	wchar_t name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = m_szModuleName;
	hNetlib = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	debugLogA("Setting protocol/module name to '%s'", m_szModuleName);
}

void CToxProto::UninitNetlib()
{
	Netlib_CloseHandle(hNetlib);
	hNetlib = NULL;
}
