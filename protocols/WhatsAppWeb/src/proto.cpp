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

static int CompareRequests(const WARequest *p1, const WARequest *p2)
{
	return strcmp(p1->szPrefix, p2->szPrefix);
}

static int CompareUsers(const WAUser *p1, const WAUser *p2)
{
	return strcmp(p1->szId, p2->szId);
}

WhatsAppProto::WhatsAppProto(const char *proto_name, const wchar_t *username) :
	PROTO<WhatsAppProto>(proto_name, username),
	m_impl(*this),
	m_tszDefaultGroup(getWStringA(DBKEY_DEF_GROUP)),
	m_arUsers(10, CompareUsers),
	m_arOwnMsgs(1, CompareOwnMsgs),
	m_arPacketQueue(10, CompareRequests),
	m_wszDefaultGroup(this, "DefaultGroup", L"WhatsApp"),
	m_bHideGroupchats(this, "HideChats", true)
{
	db_set_resident(m_szModuleName, "StatusMsg");

	CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &WhatsAppProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &WhatsAppProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &WhatsAppProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &WhatsAppProto::SetMyAvatar);

	HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);

	// Client id generation
	m_szClientId = getMStringA(DBKEY_CLIENT_ID);
	if (m_szClientId.IsEmpty()) {
		int8_t randBytes[16];
		Utils_GetRandom(randBytes, sizeof(randBytes));

		m_szClientId = ptrA(mir_base64_encode(randBytes, sizeof(randBytes)));
		setString(DBKEY_CLIENT_ID, m_szClientId);
	}

	// Create standard network connection
	wchar_t descr[512];
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);

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
	for (auto &cc : AccContacts()) {
		CMStringA szId(getMStringA(cc, isChatRoom(cc) ? "ChatRoomID" : DBKEY_ID));
		if (!szId.IsEmpty())
			m_arUsers.insert(new WAUser(cc, szId));
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
		return PF2_ONLINE | PF2_INVISIBLE;
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

	int oldStatus = m_iStatus;
	debugLogA("===== Beginning SetStatus process");

	// Routing statuses not supported by WhatsApp
	switch (new_status) {
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = new_status;
		break;

	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;

	default:
		m_iDesiredStatus = ID_STATUS_INVISIBLE;
		break;
	}

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
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
		if (m_iDesiredStatus == ID_STATUS_ONLINE) {
			// m_pConn->sendAvailableForChat();
			m_iStatus = ID_STATUS_ONLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		}
		else if (m_iStatus == ID_STATUS_ONLINE && m_iDesiredStatus == ID_STATUS_INVISIBLE) {
			// m_pConn->sendClose();
			m_iStatus = ID_STATUS_INVISIBLE;
			setAllContactStatuses(ID_STATUS_OFFLINE);
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		}
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
	ptrA jid(getStringA(hContact, DBKEY_ID));
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
	
	int pktId = WSSendNode(szMsgId, WAMetric::message, int(WAFlag::ignore), payLoad, &WhatsAppProto::OnSendMessage);

	mir_cslock lck(m_csOwnMessages);
	m_arOwnMsgs.insert(new WAOwnMessage(pktId, hContact, szMsgId));
	return pktId;
}

int WhatsAppProto::UserIsTyping(MCONTACT hContact, int)
{
	if (hContact && isOnline()) {
		ptrA jid(getStringA(hContact, DBKEY_ID));
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
// EVENTS

LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		// After a click, destroy popup
		PUDeletePopup(hwnd);
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hwnd);
		break;

	case UM_FREEPLUGINDATA:
		// After close, free
		mir_free(PUGetPluginData(hwnd));
		return FALSE;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
