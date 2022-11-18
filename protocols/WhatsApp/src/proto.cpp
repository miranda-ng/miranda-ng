/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

struct SearchParam
{
	SearchParam(const wchar_t *_jid, LONG _id) :
		jid(_jid), id(_id) 
		{}

	std::wstring jid;
	LONG id;
};

static int CompareOwnMsgs(const WAOwnMessage *p1, const WAOwnMessage *p2)
{
	return strcmp(p1->szMessageId, p2->szMessageId);
}

static int CompareUsers(const WAUser *p1, const WAUser *p2)
{
	return strcmp(p1->szId, p2->szId);
}

static int CompareCollections(const WACollection *p1, const WACollection *p2)
{
	return strcmp(p1->szName, p2->szName);
}

static int CompareRequests(const WARequestBase *p1, const WARequestBase *p2)
{
	return strcmp(p1->szPacketId, p2->szPacketId);
}

WhatsAppProto::WhatsAppProto(const char *proto_name, const wchar_t *username) :
	PROTO<WhatsAppProto>(proto_name, username),
	m_impl(*this),
	m_signalStore(this, ""),
	m_szJid(getMStringA(DBKEY_ID)),
	m_tszDefaultGroup(getWStringA(DBKEY_DEF_GROUP)),
	m_arUsers(10, CompareUsers),
	m_arDevices(1),
	m_arOwnMsgs(1, CompareOwnMsgs),
	m_arPersistent(1),
	m_arPacketQueue(10, CompareRequests),
	m_arCollections(10, CompareCollections),

	m_wszNick(this, "Nick"),
	m_wszDeviceName(this, "DeviceName", L"Miranda NG"),
	m_wszDefaultGroup(this, "DefaultGroup", L"WhatsApp"),
	m_bUsePopups(this, "UsePopups", true),
	m_bUseBbcodes(this, "UseBbcodes", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	db_set_resident(m_szModuleName, "StatusMsg");

	CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &WhatsAppProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &WhatsAppProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &WhatsAppProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &WhatsAppProto::SetMyAvatar);

	HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);

	InitSync();
	InitPopups();
	InitPersistentHandlers();

	// Create standard network connection
	wchar_t descr[512];
	mir_snwprintf(descr, TranslateT("%s (server)"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

	// Temporary folder
	CreateDirectoryTreeW(CMStringW(VARSW(L"%miranda_userdata%")) + L"\\" + _A2T(m_szModuleName));

	// Avatars folder
	m_tszAvatarFolder = CMStringW(VARSW(L"%miranda_avatarcache%")) + L"\\" + _A2T(m_szModuleName);
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(m_tszAvatarFolder.c_str());

	// default contacts group
	if (m_tszDefaultGroup == NULL)
		m_tszDefaultGroup = mir_wstrdup(L"WhatsApp");
	Clist_GroupCreate(0, m_tszDefaultGroup);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF | GC_DATABASE;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);

	HookProtoEvent(ME_GC_EVENT, &WhatsAppProto::GcEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &WhatsAppProto::GcMenuHook);
}

WhatsAppProto::~WhatsAppProto()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnErase - remove temporary folder for account

void WhatsAppProto::OnErase()
{
	DeleteDirectoryTreeW(CMStringW(VARSW(L"%miranda_userdata%")) + L"\\" + _A2T(m_szModuleName), false);
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded emulator for an account

void WhatsAppProto::OnModulesLoaded()
{
	// initialize contacts cache
	if (!m_szJid.IsEmpty())
		m_arUsers.insert(new WAUser(0, m_szJid, false));

	for (auto &cc : AccContacts()) {
		bool bIsChat = isChatRoom(cc);
		CMStringA szId(getMStringA(cc, bIsChat ? "ChatRoomID" : DBKEY_ID));
		if (!szId.IsEmpty())
			m_arUsers.insert(new WAUser(cc, szId, bIsChat));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// PROTO_INTERFACE implementation

MCONTACT WhatsAppProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr->id.w == nullptr)
		return NULL;

	auto *pUser = AddUser(T2Utf(psr->id.w), (flags & PALF_TEMPORARY) != 0);
	db_unset(pUser->hContact, "CList", "NotOnList");
	
	return pUser->hContact;
}

INT_PTR WhatsAppProto::GetCaps(int type, MCONTACT)
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
		return (DWORD_PTR)L"WhatsApp ID";
	}
	return 0;
}

int WhatsAppProto::SetStatus(int new_status)
{
	if (m_iDesiredStatus == new_status)
		return 0;

	if (!mir_wstrlen(m_wszNick)) {
		Popup(0, LPGENW("Connection cannot be established! You have not completed all necessary fields (Nick)."), LPGENW("Error"));
		return 0;
	}

	int oldStatus = m_iStatus;

	// Routing statuses not supported by WhatsApp
	switch (new_status) {
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = new_status;
		break;

	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
	default:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		SetServerStatus(m_iDesiredStatus);

		if (m_hServerConn != nullptr)
			Netlib_Shutdown(m_hServerConn);

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (m_hServerConn == nullptr && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

		ForkThread(&WhatsAppProto::ServerThread);
	}
	else if (m_hServerConn != nullptr) {
		SetServerStatus(m_iDesiredStatus);

		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int WhatsAppProto::SendMsg(MCONTACT hContact, int, const char *pszMsg)
{
	ptrA jid(getStringA(hContact, DBKEY_ID));
	if (jid == nullptr || pszMsg == nullptr)
		return 0;

	if (!isOnline()) {
		debugLogA("No connection");
		return 0;
	}

	return SendTextMessage(jid, pszMsg);
}

int WhatsAppProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && isOnline()) {
		ptrA jid(getStringA(hContact, DBKEY_ID));
		if (jid && isOnline()) {
			WSSendNode(
				WANode("chatstates") << CHAR_PARAM("to", jid) << XCHILD((type == PROTOTYPE_SELFTYPING_ON) ? "composing" : "paused"));
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// contacts search

void WhatsAppProto::SearchAckThread(void *targ)
{
	Sleep(100);

	SearchParam *param = (SearchParam*)targ;
	PROTOSEARCHRESULT psr = {};
	psr.cbSize = sizeof(psr);
	psr.flags = PSR_UNICODE;
	psr.nick.w = psr.firstName.w = psr.lastName.w = L"";
	psr.id.w = (wchar_t*)param->jid.c_str();

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)param->id, (LPARAM)&psr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)param->id, 0);

	delete param;
}

HANDLE WhatsAppProto::SearchBasic(const wchar_t* id)
{
	if (!isOnline())
		return nullptr;

	// fake - we always accept search
	SearchParam *param = new SearchParam(id, -1);
	ForkThread(&WhatsAppProto::SearchAckThread, param);
	return (HANDLE)param->id;
}
