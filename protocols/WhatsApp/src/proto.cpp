#include "stdafx.h"

#include "WhatsAPI++/WARegister.h"

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
	m_tszDefaultGroup(getWStringA(WHATSAPP_KEY_DEF_GROUP))
{
	update_loop_lock_ = CreateEvent(NULL, false, false, NULL);

	db_set_resident(m_szModuleName, "StatusMsg");

	CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFO, &WhatsAppProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &WhatsAppProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATAR, &WhatsAppProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATAR, &WhatsAppProto::SetMyAvatar);

	HookProtoEvent(ME_DB_CONTACT_DELETED, &WhatsAppProto::OnDeleteChat);
	HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &WhatsAppProto::OnBuildStatusMenu);

	// Create standard network connection
	wchar_t descr[512];
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
	if (m_hNetlibUser == NULL) {
		wchar_t error[200];
		mir_snwprintf(error, TranslateT("Unable to initialize Netlib for %s."), m_tszUserName);
		MessageBox(NULL, error, L"Miranda NG", MB_OK | MB_ICONERROR);
	}

	WASocketConnection::initNetwork(m_hNetlibUser);

	m_tszAvatarFolder = std::wstring(VARSW(L"%miranda_avatarcache%")) + L"\\" + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(m_tszAvatarFolder.c_str());

	if (m_tszDefaultGroup == NULL)
		m_tszDefaultGroup = mir_wstrdup(L"WhatsApp");
	Clist_GroupCreate(0, m_tszDefaultGroup);

	SetAllContactStatuses(ID_STATUS_OFFLINE, true);
}

WhatsAppProto::~WhatsAppProto()
{
	CloseHandle(update_loop_lock_);
}

int WhatsAppProto::OnEvent(PROTOEVENTTYPE evType, WPARAM, LPARAM)
{
	switch (evType) {
	case EV_PROTO_ONMENU:
		InitMenu();
		break;

	case EV_PROTO_ONLOAD:
		// Register group chat
		GCREGISTER gcr = {};
		gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
		gcr.ptszDispName = m_tszUserName;
		gcr.pszModule = m_szModuleName;
		Chat_Register(&gcr);

		HookProtoEvent(ME_GC_EVENT, &WhatsAppProto::onGroupChatEvent);
		HookProtoEvent(ME_GC_BUILDMENU, &WhatsAppProto::OnChatMenu);
		HookProtoEvent(ME_USERINFO_INITIALISE, &WhatsAppProto::OnUserInfo);
		break;
	}
	return TRUE;
}

DWORD_PTR WhatsAppProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND | PF1_CHAT | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES | PF4_SUPPORTTYPING | PF4_AVATARS;
	case PFLAGNUM_5:
		return 0;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)"WhatsApp ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"ID";
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

	case ID_STATUS_IDLE:
	default:
		m_iDesiredStatus = ID_STATUS_INVISIBLE;
		if (getByte(WHATSAPP_KEY_MAP_STATUSES, DEFAULT_MAP_STATUSES))
			break;
	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		if (m_pSocket != NULL) {
			SetEvent(update_loop_lock_);
			m_pSocket->forceShutdown();
			debugLogA("Forced shutdown");
		}

		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	}
	else if (m_pSocket == NULL && !IsStatusConnecting(m_iStatus)) {
		m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

		ResetEvent(update_loop_lock_);
		ForkThread(&WhatsAppProto::sentinelLoop, 0);
		ForkThread(&WhatsAppProto::stayConnectedLoop, 0);
	}
	else if (m_pConnection != NULL) {
		if (m_iDesiredStatus == ID_STATUS_ONLINE) {
			m_pConnection->sendAvailableForChat();
			m_iStatus = ID_STATUS_ONLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		}
		else if (m_iStatus == ID_STATUS_ONLINE && m_iDesiredStatus == ID_STATUS_INVISIBLE) {
			m_pConnection->sendClose();
			m_iStatus = ID_STATUS_INVISIBLE;
			SetAllContactStatuses(ID_STATUS_OFFLINE, true);
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
		}
	}
	else ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	return 0;
}

MCONTACT WhatsAppProto::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
	if (psr->id.w == NULL)
		return NULL;

	std::string phone(T2Utf(psr->id.w));
	std::string jid(phone + "@s.whatsapp.net");

	MCONTACT hContact = AddToContactList(jid, phone.c_str());
	if (!(flags & PALF_TEMPORARY))
		db_unset(hContact, "CList", "NotOnList");

	m_pConnection->sendPresenceSubscriptionRequest(jid.c_str());
	return hContact;
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
		return 0;

	// fake - we always accept search
	SearchParam *param = new SearchParam(id, GetSerial());
	ForkThread(&WhatsAppProto::SearchAckThread, param);
	return (HANDLE)param->id;
}

/////////////////////////////////////////////////////////////////////////////////////////

static NETLIBHTTPHEADER s_registerHeaders[] =
{
	{ "User-Agent", ACCOUNT_USER_AGENT },
	{ "Accept", "text/json" },
	{ "Content-Type", "application/x-www-form-urlencoded" }
};

bool WhatsAppProto::Register(int state, const string &cc, const string &number, const string &code, string &ret)
{
	string idx;
	DBVARIANT dbv;

	if (g_hNetlibUser == NULL) {
		NotifyEvent(m_tszUserName, TranslateT("Network connection error."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	if (!getString(WHATSAPP_KEY_IDX, &dbv)) {
		idx = dbv.pszVal;
		db_free(&dbv);
	}

	if (idx.empty()) {
		std::stringstream tm;
		tm << time(NULL);
		BYTE idxBuf[16];
		utils::md5string(tm.str(), idxBuf);
		idx = std::string((const char*)idxBuf, 16);
		setString(WHATSAPP_KEY_IDX, idx.c_str());
	}

	CMStringA url = WARegister::RequestCodeUrl(cc + number, code);
	if (url.IsEmpty())
		return false;

	NETLIBHTTPREQUEST nlhr = { sizeof(NETLIBHTTPREQUEST) };
	nlhr.requestType = REQUEST_POST;
	nlhr.szUrl = url.GetBuffer();
	nlhr.headers = s_registerHeaders;
	nlhr.headersCount = _countof(s_registerHeaders);
	nlhr.flags = NLHRF_HTTP11 | NLHRF_SSL;

	NETLIBHTTPREQUEST* pnlhr = Netlib_HttpTransaction(g_hNetlibUser, &nlhr);

	const wchar_t *ptszTitle = TranslateT("Registration");
	if (pnlhr == NULL) {
		NotifyEvent(ptszTitle, TranslateT("Registration failed. Invalid server response."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	debugLogA("Server response: %s", pnlhr->pData);

	JSONNode resp = JSONNode::parse(pnlhr->pData);
	if (!resp) {
		NotifyEvent(ptszTitle, TranslateT("Registration failed. Invalid server response."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	// Status = fail
	std::string status = resp["status"].as_string();
	if (status == "fail") {
		std::string reason = resp["reason"].as_string();
		if (reason == "stale")
			NotifyEvent(ptszTitle, TranslateT("Registration failed due to stale code. Please request a new code"), NULL, WHATSAPP_EVENT_CLIENT);
		else {
			CMStringW tmp(FORMAT, TranslateT("Registration failed. Reason: %s"), _A2T(reason.c_str()));
			NotifyEvent(ptszTitle, tmp, NULL, WHATSAPP_EVENT_CLIENT);
		}

		const JSONNode &tmpVal = resp["retry_after"];
		if (tmpVal) {
			CMStringW tmp(FORMAT, TranslateT("Please try again in %i seconds"), tmpVal.as_int());
			NotifyEvent(ptszTitle, tmp, NULL, WHATSAPP_EVENT_OTHER);
		}
		return false;
	}

	//  Request code
	if (state == REG_STATE_REQ_CODE) {
		std::string pw = resp["pw"].as_string();
		if (!pw.empty())
			ret = pw;
		else if (status == "sent")
			NotifyEvent(ptszTitle, TranslateT("Registration code has been sent to your phone."), NULL, WHATSAPP_EVENT_OTHER);
		return true;
	}

	// Register
	if (state == REG_STATE_REG_CODE) {
		std::string pw = resp["pw"].as_string();
		if (!pw.empty()) {
			ret = pw;
			return true;
		}
		NotifyEvent(ptszTitle, TranslateT("Registration failed."), NULL, WHATSAPP_EVENT_CLIENT);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

int WhatsAppProto::OnUserInfo(WPARAM, LPARAM hContact)
{
	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid && isOnline()) {
		m_pConnection->sendGetPicture((char*)jid, "image");
		m_pConnection->sendPresenceSubscriptionRequest((char*)jid);
	}
	
	return 0;
}

void WhatsAppProto::RequestFriendship(MCONTACT hContact)
{
	if (hContact == NULL || isOffline())
		return;

	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid) {
		m_pConnection->sendPresenceSubscriptionRequest((char*)jid);
	}
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

void WhatsAppProto::NotifyEvent(const string& title, const string& info, MCONTACT contact, DWORD flags, wchar_t* url)
{
	wchar_t *rawTitle = mir_a2u_cp(title.c_str(), CP_UTF8);
	wchar_t *rawInfo = mir_a2u_cp(info.c_str(), CP_UTF8);
	NotifyEvent(rawTitle, rawInfo, contact, flags, url);
	mir_free(rawTitle);
	mir_free(rawInfo);
}

void WhatsAppProto::NotifyEvent(const wchar_t *title, const wchar_t *info, MCONTACT contact, DWORD flags, wchar_t* szUrl)
{
	int ret, timeout = 0;
	COLORREF colorBack = 0, colorText = 0;

	switch (flags) {
	case WHATSAPP_EVENT_CLIENT:
		if (!getByte(WHATSAPP_KEY_EVENT_CLIENT_ENABLE, DEFAULT_EVENT_CLIENT_ENABLE))
			goto exit;
		if (!getByte(WHATSAPP_KEY_EVENT_CLIENT_DEFAULT, 0)) {
			colorBack = getDword(WHATSAPP_KEY_EVENT_CLIENT_COLBACK, DEFAULT_EVENT_COLBACK);
			colorText = getDword(WHATSAPP_KEY_EVENT_CLIENT_COLTEXT, DEFAULT_EVENT_COLTEXT);
		}
		timeout = getDword(WHATSAPP_KEY_EVENT_CLIENT_TIMEOUT, 0);
		flags |= NIIF_WARNING;
		break;

	case WHATSAPP_EVENT_OTHER:
		if (!getByte(WHATSAPP_KEY_EVENT_OTHER_ENABLE, DEFAULT_EVENT_OTHER_ENABLE))
			goto exit;
		if (!getByte(WHATSAPP_KEY_EVENT_OTHER_DEFAULT, 0)) {
			colorBack = getDword(WHATSAPP_KEY_EVENT_OTHER_COLBACK, DEFAULT_EVENT_COLBACK);
			colorText = getDword(WHATSAPP_KEY_EVENT_OTHER_COLTEXT, DEFAULT_EVENT_COLTEXT);
		}
		timeout = getDword(WHATSAPP_KEY_EVENT_OTHER_TIMEOUT, -1);
		Skin_PlaySound("OtherEvent");
		flags |= NIIF_INFO;
		break;
	}

	if (!getByte(WHATSAPP_KEY_SYSTRAY_NOTIFY, DEFAULT_SYSTRAY_NOTIFY)) {
		if (ServiceExists(MS_POPUP_ADDPOPUP)) {
			POPUPDATAT pd;
			pd.colorBack = colorBack;
			pd.colorText = colorText;
			pd.iSeconds = timeout;
			pd.lchContact = contact;
			pd.lchIcon = IcoLib_GetIconByHandle(m_hProtoIcon); // TODO: Icon test
			pd.PluginData = szUrl;
			pd.PluginWindowProc = PopupDlgProc;
			mir_wstrcpy(pd.lptzContactName, title);
			mir_wstrcpy(pd.lptzText, info);
			ret = PUAddPopupT(&pd);

			if (ret == 0)
				return;
		}
	}
	else {
		int niif_flags = flags;
		REMOVE_FLAG(niif_flags, WHATSAPP_EVENT_CLIENT | WHATSAPP_EVENT_NOTIFICATION | WHATSAPP_EVENT_OTHER);
		if (!Clist_TrayNotifyW(m_szModuleName, title, info, niif_flags, 1000 * timeout))
			goto exit;
	}

	if (FLAG_CONTAINS(flags, WHATSAPP_EVENT_CLIENT))
		MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);

exit:
	if (szUrl != NULL)
		mir_free(szUrl);
}
