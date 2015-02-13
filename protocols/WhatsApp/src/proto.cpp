#include "common.h"

#include "WhatsAPI++\WARegister.h"

struct SearchParam
{
	SearchParam(const TCHAR *_jid, LONG _id) :
		jid(_jid), id(_id) 
		{}

	std::tstring jid;
	LONG id;
};

WhatsAppProto::WhatsAppProto(const char* proto_name, const TCHAR* username) :
	PROTO<WhatsAppProto>(proto_name, username),
	m_tszDefaultGroup(getTStringA(WHATSAPP_KEY_DEF_GROUP))
{
	update_loop_lock_ = CreateEvent(NULL, false, false, NULL);

	db_set_resident(m_szModuleName, "Status");
	db_set_resident(m_szModuleName, "StatusMsg");

	CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);

	CreateProtoService(PS_GETAVATARINFOT, &WhatsAppProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &WhatsAppProto::GetAvatarCaps);
	CreateProtoService(PS_GETMYAVATART, &WhatsAppProto::GetMyAvatar);
	CreateProtoService(PS_SETMYAVATART, &WhatsAppProto::SetMyAvatar);

	HookProtoEvent(ME_DB_CONTACT_DELETED, &WhatsAppProto::OnDeleteChat);
	HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &WhatsAppProto::OnBuildStatusMenu);

	// Create standard network connection
	TCHAR descr[512];
	mir_sntprintf(descr, SIZEOF(descr), TranslateT("%s server connection"), m_tszUserName);

	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	if (m_hNetlibUser == NULL)
		MessageBox(NULL, TranslateT("Unable to get Netlib connection for WhatsApp"), m_tszUserName, MB_OK);

	WASocketConnection::initNetwork(m_hNetlibUser);

	m_tszAvatarFolder = std::tstring(VARST(_T("%miranda_avatarcache%"))) + _T("\\") + m_tszUserName;
	DWORD dwAttributes = GetFileAttributes(m_tszAvatarFolder.c_str());
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(m_tszAvatarFolder.c_str());

	if (m_tszDefaultGroup == NULL)
		m_tszDefaultGroup = mir_tstrdup(_T("WhatsApp"));
	Clist_CreateGroup(0, m_tszDefaultGroup);

	SetAllContactStatuses(ID_STATUS_OFFLINE, true);
}

WhatsAppProto::~WhatsAppProto()
{
	CloseHandle(update_loop_lock_);
}

int WhatsAppProto::OnEvent(PROTOEVENTTYPE evType, WPARAM wParam, LPARAM lParam)
{
	if (evType == EV_PROTO_ONLOAD) {
		// Register group chat
		GCREGISTER gcr = { sizeof(gcr) };
		gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
		gcr.ptszDispName = m_tszUserName;
		gcr.pszModule = m_szModuleName;
		CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

		HookProtoEvent(ME_GC_EVENT, &WhatsAppProto::onGroupChatEvent);
		HookProtoEvent(ME_GC_BUILDMENU, &WhatsAppProto::OnChatMenu);
	}
	return 0;
}

DWORD_PTR WhatsAppProto::GetCaps(int type, MCONTACT hContact)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_CHAT | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_MODEMSGRECV;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE;
	case PFLAGNUM_3:
		return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_IMSENDUTF | PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_SUPPORTTYPING | PF4_AVATARS;
	case PFLAGNUM_5:
		return 0;
	case PFLAG_MAXLENOFMESSAGE:
		return 500; // #TODO
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
	else if (m_pSocket == NULL && !(m_iStatus >= ID_STATUS_CONNECTING && m_iStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)) {
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

MCONTACT WhatsAppProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	if (psr->id == NULL)
		return NULL;

	std::string phone(ptrA(mir_utf8encodeT(psr->id)));
	std::string jid(phone + "@s.whatsapp.net");

	MCONTACT hContact = AddToContactList(jid, phone.c_str());
	if (!(flags & PALF_TEMPORARY))
		db_unset(hContact, "CList", "NotOnList");

	m_pConnection->sendQueryLastOnline(jid.c_str());
	m_pConnection->sendPresenceSubscriptionRequest(jid.c_str());
	return hContact;
}

int WhatsAppProto::AuthRequest(MCONTACT hContact, const PROTOCHAR *message)
{
	RequestFriendship(hContact);
	return 0;
}

int WhatsAppProto::Authorize(MEVENT hDbEvent)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::SearchAckThread(void *targ)
{
	Sleep(100);

	SearchParam *param = (SearchParam*)targ;
	PROTOSEARCHRESULT sr = { 0 };
	sr.cbSize = sizeof(sr);
	sr.flags = PSR_TCHAR;
	sr.nick = _T("");
	sr.firstName = _T("");
	sr.lastName = _T("");
	sr.id = (TCHAR*)param->jid.c_str();

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)param->id, (LPARAM)&sr);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)param->id, 0);

	delete param;
}

HANDLE WhatsAppProto::SearchBasic(const PROTOCHAR* id)
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

	if (WASocketConnection::hNetlibUser == NULL) {
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
	nlhr.headersCount = SIZEOF(s_registerHeaders);
	nlhr.flags = NLHRF_HTTP11 | NLHRF_GENERATEHOST | NLHRF_REMOVEHOST | NLHRF_SSL;

	NETLIBHTTPREQUEST* pnlhr = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,
		(WPARAM)WASocketConnection::hNetlibUser, (LPARAM)&nlhr);

	const TCHAR *ptszTitle = TranslateT("Registration");
	if (pnlhr == NULL) {
		NotifyEvent(ptszTitle, TranslateT("Registration failed. Invalid server response."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	debugLogA("Server response: %s", pnlhr->pData);

	JSONROOT resp(pnlhr->pData);
	if (resp == NULL) {
		NotifyEvent(ptszTitle, TranslateT("Registration failed. Invalid server response."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	// Status = fail
	JSONNODE *val = json_get(resp, "status");
	if (!mir_tstrcmp(ptrT(json_as_string(val)), _T("fail"))) {
		JSONNODE *tmpVal = json_get(resp, "reason");
		if (!mir_tstrcmp(ptrT(json_as_string(tmpVal)), _T("stale")))
			NotifyEvent(ptszTitle, TranslateT("Registration failed due to stale code. Please request a new code"), NULL, WHATSAPP_EVENT_CLIENT);
		else
			NotifyEvent(ptszTitle, TranslateT("Registration failed."), NULL, WHATSAPP_EVENT_CLIENT);

		tmpVal = json_get(resp, "retry_after");
		if (tmpVal != NULL) {
			CMString tmp(FORMAT, TranslateT("Please try again in %i seconds"), json_as_int(tmpVal));
			NotifyEvent(ptszTitle, tmp, NULL, WHATSAPP_EVENT_OTHER);
		}
		return false;
	}

	//  Request code
	if (state == REG_STATE_REQ_CODE) {
		val = json_get(resp, "pw");
		if (val != NULL)
			ret = _T2A(ptrT(json_as_string(val)));
		else if (!mir_tstrcmp(ptrT(json_as_string(val)), _T("sent")))
			NotifyEvent(ptszTitle, TranslateT("Registration code has been sent to your phone."), NULL, WHATSAPP_EVENT_OTHER);
		return true;
	}

	// Register
	if (state == REG_STATE_REG_CODE) {
		val = json_get(resp, "pw");
		if (val != NULL) {
			ret = _T2A(ptrT(json_as_string(val)));
			return true;
		}
		NotifyEvent(ptszTitle, TranslateT("Registration failed."), NULL, WHATSAPP_EVENT_CLIENT);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

INT_PTR WhatsAppProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_ACCMGRUI),
		(HWND)lParam, WhatsAppAccountProc, (LPARAM)this);
}

int WhatsAppProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position = 1;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = WhatsAppAccountProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

void WhatsAppProto::RequestFriendship(MCONTACT hContact)
{
	if (hContact == NULL || isOffline())
		return;

	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid) {
		m_pConnection->sendQueryLastOnline((char*)jid);
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

void WhatsAppProto::NotifyEvent(const string& title, const string& info, MCONTACT contact, DWORD flags, TCHAR* url)
{
	TCHAR *rawTitle = mir_a2t_cp(title.c_str(), CP_UTF8);
	TCHAR *rawInfo = mir_a2t_cp(info.c_str(), CP_UTF8);
	NotifyEvent(rawTitle, rawInfo, contact, flags, url);
	mir_free(rawTitle);
	mir_free(rawInfo);
}

void WhatsAppProto::NotifyEvent(const TCHAR *title, const TCHAR *info, MCONTACT contact, DWORD flags, TCHAR* szUrl)
{
	int ret; int timeout; COLORREF colorBack = 0; COLORREF colorText = 0;

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
		SkinPlaySound("OtherEvent");
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
			pd.lchIcon = Skin_GetIconByHandle(m_hProtoIcon); // TODO: Icon test
			pd.PluginData = szUrl;
			pd.PluginWindowProc = (WNDPROC)PopupDlgProc;
			mir_tstrcpy(pd.lptzContactName, title);
			mir_tstrcpy(pd.lptzText, info);
			ret = PUAddPopupT(&pd);

			if (ret == 0)
				return;
		}
	}
	else {
		if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
			MIRANDASYSTRAYNOTIFY err;
			int niif_flags = flags;
			REMOVE_FLAG(niif_flags, WHATSAPP_EVENT_CLIENT | WHATSAPP_EVENT_NOTIFICATION | WHATSAPP_EVENT_OTHER);
			err.szProto = m_szModuleName;
			err.cbSize = sizeof(err);
			err.dwInfoFlags = NIIF_INTERN_TCHAR | niif_flags;
			err.tszInfoTitle = (TCHAR*)title;
			err.tszInfo = (TCHAR*)info;
			err.uTimeout = 1000 * timeout;
			ret = CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)& err);

			if (ret == 0)
				goto exit;
		}
	}

	if (FLAG_CONTAINS(flags, WHATSAPP_EVENT_CLIENT))
		MessageBox(NULL, info, title, MB_OK | MB_ICONINFORMATION);

exit:
	if (szUrl != NULL)
		mir_free(szUrl);
}
