/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

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

WhatsAppProto::WhatsAppProto(const char *proto_name, const wchar_t *username)
	: PROTO<WhatsAppProto>(proto_name, username),
	m_tszDefaultGroup(getWStringA(DBKEY_DEF_GROUP)),
	m_arPacketQueue(10, NumericKeySortT)
{
	db_set_resident(m_szModuleName, "StatusMsg");

	// CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &WhatsAppProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &WhatsAppProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &WhatsAppProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &WhatsAppProto::SetMyAvatar);

	// HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);
	// HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &WhatsAppProto::OnBuildStatusMenu);

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
	if (m_hNetlibUser == nullptr) {
		wchar_t error[200];
		mir_snwprintf(error, TranslateT("Unable to initialize Netlib for %s."), m_tszUserName);
		MessageBox(nullptr, error, L"Miranda NG", MB_OK | MB_ICONERROR);
	}

	m_tszAvatarFolder = CMStringW(VARSW(L"%miranda_avatarcache%")) + L"\\" + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(m_tszAvatarFolder.c_str());

	if (m_tszDefaultGroup == NULL)
		m_tszDefaultGroup = mir_wstrdup(L"WhatsApp");
	Clist_GroupCreate(0, m_tszDefaultGroup);
}

WhatsAppProto::~WhatsAppProto()
{
}

MCONTACT WhatsAppProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr->id.w == nullptr)
		return NULL;

	std::string phone(T2Utf(psr->id.w));
	std::string jid(phone + "@s.whatsapp.net");

	/*	MCONTACT hContact = AddToContactList(jid, phone.c_str());
		if (!(flags & PALF_TEMPORARY))
			db_unset(hContact, "CList", "NotOnList");

		return hContact;*/
}

INT_PTR WhatsAppProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND | PF1_CHAT | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SUPPORTTYPING | PF4_AVATARS;
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

int WhatsAppProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	ptrA jid(getStringA(hContact, "ID"));
	if (jid == NULL)
		return 0;

	if (!isOnline()) {
		debugLogA("No connection");
		return 0;
	}

	return 0;
}

int WhatsAppProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && isOnline()) {
		ptrA jid(getStringA(hContact, DBKEY_ID));
		if (jid && isOnline()) {
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::SearchAckThread(void *targ)
{
	Sleep(100);

	SearchParam *param = (SearchParam*)targ;
	PROTOSEARCHRESULT psr = { 0 };
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
	if (isOffline())
		return nullptr;

	// fake - we always accept search
	SearchParam *param = new SearchParam(id, -1);
	ForkThread(&WhatsAppProto::SearchAckThread, param);
	return (HANDLE)param->id;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

int WhatsAppProto::OnUserInfo(WPARAM, LPARAM hContact)
{
/*	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid && isOnline()) {
		m_pConnection->sendGetPicture((char*)jid, "image");
		m_pConnection->sendPresenceSubscriptionRequest((char*)jid);
	}
*/	
	return 0;
}

void WhatsAppProto::RequestFriendship(MCONTACT hContact)
{
	if (hContact == NULL || isOffline())
		return;

/*	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid)
		m_pConnection->sendPresenceSubscriptionRequest((char*)jid); */
}

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
};
