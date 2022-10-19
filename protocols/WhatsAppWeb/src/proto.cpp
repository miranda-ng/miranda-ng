/*

WhatsAppWeb plugin for Miranda NG
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
	return strcmp(p1->szPrefix, p2->szPrefix);
}

static int CompareUsers(const WAUser *p1, const WAUser *p2)
{
	return strcmp(p1->szId, p2->szId);
}

static int CompareCollections(const WACollection *p1, const WACollection *p2)
{
	return strcmp(p1->szName, p2->szName);
}

WhatsAppProto::WhatsAppProto(const char *proto_name, const wchar_t *username) :
	PROTO<WhatsAppProto>(proto_name, username),
	m_impl(*this),
	m_signalStore(this, ""),
	m_szJid(getMStringA(DBKEY_JID)),
	m_tszDefaultGroup(getWStringA(DBKEY_DEF_GROUP)),
	m_arUsers(10, CompareUsers),
	m_arDevices(1),
	m_arOwnMsgs(1, CompareOwnMsgs),
	m_arPersistent(1),
	m_arPacketQueue(10),
	m_arCollections(10, CompareCollections),

	m_wszNick(this, "Nick"),
	m_wszDefaultGroup(this, "DefaultGroup", L"WhatsApp"),
	m_bUsePopups(this, "UsePopups", true),
	m_bHideGroupchats(this, "HideChats", true)
{
	db_set_resident(m_szModuleName, "StatusMsg");

	CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &WhatsAppProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &WhatsAppProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &WhatsAppProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &WhatsAppProto::SetMyAvatar);

	HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);

	InitCollections();
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
	m_tszAvatarFolder = CMStringW(VARSW(L"%miranda_avatarcache%")) + L"\\" + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(m_tszAvatarFolder.c_str());

	// default contacts group
	if (m_tszDefaultGroup == NULL)
		m_tszDefaultGroup = mir_wstrdup(L"WhatsApp");
	Clist_GroupCreate(0, m_tszDefaultGroup);

	// groupchat initialization
	GCREGISTER gcr = {};
	gcr.dwFlags = GC_TYPNOTIF;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	Chat_Register(&gcr);
}

WhatsAppProto::~WhatsAppProto()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded emulator for an account

void WhatsAppProto::OnModulesLoaded()
{
	// initialize contacts cache
	m_arUsers.insert(new WAUser(0, m_szJid, false));

	for (auto &cc : AccContacts()) {
		bool bIsChat = isChatRoom(cc);
		CMStringA szId(getMStringA(cc, bIsChat ? "ChatRoomID" : DBKEY_JID));
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
		return (DWORD_PTR)"WhatsApp ID";
	}
	return 0;
}

int WhatsAppProto::SetStatus(int new_status)
{
	if (m_iDesiredStatus == new_status)
		return 0;

	if (!mir_wstrlen(m_wszNick)) {
		Popup(0, LPGENW("You need to specify nick name in the Options dialog"), LPGENW("Error"));
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

void WhatsAppProto::OnSendMessage(const JSONNode &node, void*)
{
	CMStringA szPrefix= node["$id$"].as_mstring();

	WAOwnMessage tmp(0, 0, szPrefix);
	{
		mir_cslock lck(m_csOwnMessages);
		auto *pOwn = m_arOwnMsgs.find(&tmp);
		if (pOwn == nullptr)
			return;
		
		tmp.pktId = pOwn->pktId;
		tmp.hContact = pOwn->hContact;
		m_arOwnMsgs.remove(pOwn);
	}

	int status = node["status"].as_int();
	if (status == 200)
		ProtoBroadcastAck(tmp.hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tmp.pktId);
	else {
		CMStringW wszError(FORMAT, TranslateT("Operation failed with server error status %d"), status);
		ProtoBroadcastAck(tmp.hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tmp.pktId, LPARAM(wszError.c_str()));
	}
}

int WhatsAppProto::SendMsg(MCONTACT hContact, int, const char *pszMsg)
{
	ptrA jid(getStringA(hContact, DBKEY_JID));
	if (jid == nullptr || pszMsg == nullptr)
		return 0;

	if (!isOnline()) {
		debugLogA("No connection");
		return 0;
	}

	char msgId[16], szMsgId[33];
	Utils_GetRandom(msgId, sizeof(msgId));
	bin2hex(msgId, sizeof(msgId), szMsgId);

	auto *key = new proto::MessageKey();
	key->set_remotejid(jid);
	key->set_fromme(true);
	key->set_id(szMsgId);

	proto::WebMessageInfo msg;
	msg.set_allocated_key(key);
	msg.mutable_message()->set_conversation(pszMsg);
	msg.set_messagetimestamp(_time64(0));

	size_t cbBinaryLen = msg.ByteSizeLong();
	mir_ptr<BYTE> pBuf((BYTE *)mir_alloc(cbBinaryLen));
	msg.SerializeToArray(pBuf, (int)cbBinaryLen);

	WANode payLoad;
	payLoad.title = "action";
	payLoad.addAttr("type", "relay");
	payLoad.content.assign(pBuf, cbBinaryLen);
	
	int pktId = WSSendNode(payLoad);

	mir_cslock lck(m_csOwnMessages);
	m_arOwnMsgs.insert(new WAOwnMessage(pktId, hContact, szMsgId));
	return pktId;
}

int WhatsAppProto::UserIsTyping(MCONTACT hContact, int)
{
	if (hContact && isOnline()) {
		ptrA jid(getStringA(hContact, DBKEY_JID));
		if (jid && isOnline()) {
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

//////////////////////////////////////////////////////////////////////////////

static int enumCollections(const char *szSetting, void *param)
{
	auto *pList = (LIST<char> *)param;
	if (!memcmp(szSetting, "Collection_", 11))
		pList->insert(mir_strdup(szSetting));
	return 0;
}

void WhatsAppProto::InitCollections()
{
	LIST<char> settings(10);
	db_enum_settings(0, enumCollections, m_szModuleName, &settings);

	for (auto &it : settings) {
		m_arCollections.insert(new WACollection(it + 11, getDword(it)));
		mir_free(it);
	}
}
