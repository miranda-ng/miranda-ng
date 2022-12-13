#include "stdafx.h"

static int CompareUsers(const TG_USER *p1, const TG_USER *p2)
{
	if (p1->id == p2->id)
		return 0;

	return (p1->id < p2->id) ? -1 : 1;
}

CMTProto::CMTProto(const char* protoName, const wchar_t* userName) :
	PROTO<CMTProto>(protoName, userName),
	m_pClientMmanager(std::make_unique<td::ClientManager>()),
	m_arUsers(10, CompareUsers),
	m_arRequests(10, NumericKeySortT),
	m_szOwnPhone(this, "Phone"), 
	m_wszDefaultGroup(this, "DefaultGroup", L"Telegram"),
	m_bUsePopups(this, "UsePopups", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	m_iClientId = m_pClientMmanager->create_client_id();

	CreateProtoService(PS_CREATEACCMGRUI, &CMTProto::SvcCreateAccMgrUI);

	HookProtoEvent(ME_OPT_INITIALISE, &CMTProto::OnOptionsInit);
}

CMTProto::~CMTProto()
{
}

void CMTProto::OnModulesLoaded()
{
	CMStringA szId(getMStringA(DBKEY_ID));
	if (!szId.IsEmpty())
		m_arUsers.insert(new TG_USER(_atoi64(szId.c_str()), 0));

	for (auto &cc : AccContacts()) {
		bool isGroupChat = isChatRoom(cc);
		szId = getMStringA(cc, isGroupChat ? "ChatRoomID" : DBKEY_ID);
		if (!szId.IsEmpty())
			m_arUsers.insert(new TG_USER(_atoi64(szId.c_str()), cc, isGroupChat));
	}
}

void CMTProto::OnErase()
{
	DeleteDirectoryTreeW(GetProtoFolder(), false);
}

INT_PTR CMTProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_CHAT | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SERVERMSGID;
	case PFLAGNUM_5:
		return 0;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)L"Phone";
	}
	return 0;
}

int CMTProto::SetStatus(int iNewStatus)
{
	if (m_iDesiredStatus == iNewStatus)
		return 0;
	
	int oldStatus = m_iStatus;

	// Routing statuses not supported by Telegram
	switch (iNewStatus) {
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = iNewStatus;
		break;

	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
	default:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		if (m_bRunning)
			SendQuery(new td::td_api::close());

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (!m_bRunning && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

		ForkThread(&CMTProto::ServerThread);
	}
	else if (m_bRunning) {
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}
