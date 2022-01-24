#include "stdafx.h"

void CToxProto::InitNetlib()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = m_tszUserName;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	debugLogA(__FUNCTION__":Setting protocol / module name to '%s'", m_szModuleName);
}
