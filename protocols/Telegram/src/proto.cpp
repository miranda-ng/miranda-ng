#include "stdafx.h"

static int CompareRequests(const TG_REQUEST_BASE *p1, const TG_REQUEST_BASE *p2)
{
	if (p1->requestId == p2->requestId)
		return 0;

	return (p1->requestId < p2->requestId) ? -1 : 1;
}

static int CompareUsers(const TG_USER *p1, const TG_USER *p2)
{
	if (p1->id == p2->id)
		return 0;

	return (p1->id < p2->id) ? -1 : 1;
}

CTelegramProto::CTelegramProto(const char* protoName, const wchar_t* userName) :
	PROTO<CTelegramProto>(protoName, userName),
	m_impl(*this),
	m_arFiles(1),
	m_arUsers(10, CompareUsers),
	m_arRequests(10, CompareRequests),
	m_szOwnPhone(this, "Phone"), 
	m_wszDeviceName(this, "DeviceName", L"Miranda NG"),
	m_wszDefaultGroup(this, "DefaultGroup", L"Telegram"),
	m_bUsePopups(this, "UsePopups", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	m_iOwnId = _atoi64(getMStringA(DBKEY_ID));

	CreateProtoService(PS_CREATEACCMGRUI, &CTelegramProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETAVATARCAPS, &CTelegramProto::SvcGetAvatarCaps);
	CreateProtoService(PS_GETAVATARINFO, &CTelegramProto::SvcGetAvatarInfo);
	CreateProtoService(PS_GETMYAVATAR, &CTelegramProto::SvcGetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &CTelegramProto::SvcSetMyAvatar);

	HookProtoEvent(ME_OPT_INITIALISE, &CTelegramProto::OnOptionsInit);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &CTelegramProto::OnDbMarkedRead);

	// avatar
	CreateDirectoryTreeW(GetAvatarPath());

	// default contacts group
	if (m_wszDefaultGroup == NULL)
		m_wszDefaultGroup = mir_wstrdup(L"WhatsApp");
	m_iBaseGroup = Clist_GroupCreate(0, m_wszDefaultGroup);

	// create standard network connection
	NETLIBUSER nlu = {};
	nlu.flags = NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = m_tszUserName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_DATABASE;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	// HookProtoEvent(ME_GC_EVENT, &WhatsAppProto::GcEventHook);
	// HookProtoEvent(ME_GC_BUILDMENU, &WhatsAppProto::GcMenuHook);
}

CTelegramProto::~CTelegramProto()
{
}

void CTelegramProto::OnModulesLoaded()
{
	CMStringA szId(getMStringA(DBKEY_ID));
	if (!szId.IsEmpty())
		m_arUsers.insert(new TG_USER(_atoi64(szId.c_str()), 0));

	for (auto &cc : AccContacts()) {
		ptrA szPath(getStringA(cc, "AvatarPath"));
		if (szPath) {
			delSetting(cc, "AvatarPath");
			delSetting(cc, DBKEY_AVATAR_HASH);
		}

		bool isGroupChat = isChatRoom(cc);
		szId = getMStringA(cc, isGroupChat ? "ChatRoomID" : DBKEY_ID);
		if (!szId.IsEmpty()) {
			auto *pUser = new TG_USER(_atoi64(szId.c_str()), cc, isGroupChat);
			pUser->szAvatarHash = getMStringA(cc, DBKEY_AVATAR_HASH);
			m_arUsers.insert(pUser);
		}
	}

	m_bSmileyAdd = ServiceExists(MS_SMILEYADD_LOADCONTACTSMILEYS);
	if (m_bSmileyAdd) {
		CMStringW wszStickersPath(GetAvatarPath() + L"\\Stickers\\*.*");
		SMADD_CONT cont = {2, m_szModuleName, wszStickersPath};
		CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, LPARAM(&cont));
	}
}

void CTelegramProto::OnShutdown()
{
	m_bTerminated = true;
}

void CTelegramProto::OnErase()
{
	m_bUnregister = true;
	ServerThread(0);

	DeleteDirectoryTreeW(GetProtoFolder(), false);
}

int CTelegramProto::OnDbMarkedRead(WPARAM hContact, LPARAM hDbEvent)
{
	if (!hContact)
		return 0;

	// filter out only events of my protocol
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (mir_strcmp(szProto, m_szModuleName))
		return 0;

	ptrA userId(getStringA(hContact, DBKEY_ID));
	if (userId) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.szId) {
			mir_cslock lck(m_csMarkRead);
			if (m_markContact) {
				if (m_markContact != hContact)
					SendMarkRead();

				m_impl.m_markRead.Stop();
			}

			m_markContact = hContact;
			m_markIds.push_back(_atoi64(dbei.szId));
			m_impl.m_markRead.Start(500);
		}
	}

	return 0;
}

INT_PTR CTelegramProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILE | PF1_CHAT | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_SERVERMSGID;
	case PFLAGNUM_5:
		return PF2_SHORTAWAY | PF2_LONGAWAY;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)L"Phone";
	default:
		return 0;
	}
}

int CTelegramProto::SendMsg(MCONTACT hContact, int, const char *pszMessage)
{
	ptrA szId(getStringA(hContact, DBKEY_ID));
	if (szId == nullptr)
		return 0;

	return SendTextMessage(_atoi64(szId), pszMessage);
}

int CTelegramProto::SetStatus(int iNewStatus)
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
		if (isRunning())
			SendQuery(new TD::close());

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (!isRunning() && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

		ForkThread(&CTelegramProto::ServerThread);
	}
	else if (isRunning()) {
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}
