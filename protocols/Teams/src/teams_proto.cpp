#include "stdafx.h"

CTeamsProto::CTeamsProto(const char *protoName, const wchar_t *userName) :
	PROTO<CTeamsProto>(protoName, userName),
	m_impl(*this),
	m_requests(10),
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

INT_PTR CTeamsProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH | PF1_MODEMSG | PF1_FILE | PF1_SERVERCLIST;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_3:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_SHORTAWAY | PF2_HEAVYDND;
	case PFLAGNUM_4:
		return PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SERVERMSGID | PF4_SERVERFORMATTING;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("Teams ID");
	}
	return 0;
}

int CTeamsProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == m_iDesiredStatus)
		return 0;

	switch (iNewStatus) {
	case ID_STATUS_FREECHAT: iNewStatus = ID_STATUS_ONLINE; break;
	case ID_STATUS_NA:       iNewStatus = ID_STATUS_AWAY;   break;
	case ID_STATUS_OCCUPIED: iNewStatus = ID_STATUS_DND;    break;
	}

	debugLogA(__FUNCTION__ ": changing status from %i to %i", m_iStatus, iNewStatus);

	int old_status = m_iStatus;
	m_iDesiredStatus = iNewStatus;

	if (iNewStatus == ID_STATUS_OFFLINE) {
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		StopQueue();

		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, ID_STATUS_OFFLINE);

		if (!Miranda_IsTerminated())
			setAllContactStatuses(ID_STATUS_OFFLINE, false);
		return 0;
	}

	if (m_iStatus == ID_STATUS_OFFLINE)
		Login();
	// else
	//	PushRequest(new SetStatusRequest(MirandaToSkypeStatus(m_iDesiredStatus)));
	return 0;
}
