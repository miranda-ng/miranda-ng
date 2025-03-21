#include "stdafx.h"

CTeamsProto::CTeamsProto(const char *protoName, const wchar_t *userName) :
	PROTO<CTeamsProto>(protoName, userName),
	m_wstrCListGroup(this, "DefaultGroup", L"Teams")
{
	HookProtoEvent(ME_OPT_INITIALISE, &CTeamsProto::OnOptionsInit);

	// network
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = m_tszUserName;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
}

CTeamsProto::~CTeamsProto()
{
}
