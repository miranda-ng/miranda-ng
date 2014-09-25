#include "common.h"

#include "WhatsAPI++\WARegister.h"

WhatsAppProto::WhatsAppProto(const char* proto_name, const TCHAR* username) :
	PROTO<WhatsAppProto>(proto_name, username)
{
	this->challenge = new std::vector < unsigned char > ;
	this->msgId = 0;
	this->msgIdHeader = time(NULL);

	update_loop_lock_ = CreateEvent(NULL, false, false, NULL);
	FMessage::generating_lock = new Mutex();

	CreateProtoService(PS_CREATEACCMGRUI, &WhatsAppProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_JOINCHAT, &WhatsAppProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &WhatsAppProto::OnLeaveChat);

	HookProtoEvent(ME_OPT_INITIALISE, &WhatsAppProto::OnOptionsInit);
	HookProtoEvent(ME_SYSTEM_MODULESLOADED, &WhatsAppProto::OnModulesLoaded);
	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &WhatsAppProto::OnBuildStatusMenu);

	this->InitContactMenus();

	// Create standard network connection
	TCHAR descr[512];
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	mir_sntprintf(descr, SIZEOF(descr), TranslateT("%s server connection"), m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	if (m_hNetlibUser == NULL)
		MessageBox(NULL, TranslateT("Unable to get Netlib connection for WhatsApp"), m_tszUserName, MB_OK);

	WASocketConnection::initNetwork(m_hNetlibUser);

	def_avatar_folder_ = std::tstring(VARST(_T("%miranda_avatarcache%"))) + _T("\\") + m_tszUserName;

	SetAllContactStatuses(ID_STATUS_OFFLINE, true);
}

WhatsAppProto::~WhatsAppProto()
{
	CloseHandle(update_loop_lock_);

	if (this->challenge != NULL)
		delete this->challenge;
}

int WhatsAppProto::OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// Register group chat
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &WhatsAppProto::OnChatOutgoing);
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
		return PF4_NOCUSTOMAUTH | PF4_IMSENDUTF | PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS;
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

	if (m_iStatus == ID_STATUS_CONNECTING) {
		debugLogA("===== Status is connecting, no change");
		return 0;
	}

	if (m_iStatus == m_iDesiredStatus) {
		debugLogA("===== Statuses are same, no change");
		return 0;
	}

	ForkThread(&WhatsAppProto::ChangeStatus, this);

	return 0;
}

MCONTACT WhatsAppProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return NULL;
}

int WhatsAppProto::AuthRequest(MCONTACT hContact, const PROTOCHAR *message)
{
	return this->RequestFriendship((WPARAM)hContact, NULL);
}

int WhatsAppProto::Authorize(HANDLE hDbEvent)
{
	return 1;
}

HANDLE WhatsAppProto::SearchBasic(const PROTOCHAR* id)
{
	if (isOffline())
		return 0;

	TCHAR* email = mir_tstrdup(id);
	ForkThread(&WhatsAppProto::SearchAckThread, email);
	return email;
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
		this->NotifyEvent(ptszTitle, TranslateT("Registration failed. Invalid server response."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	debugLogA("Server response: %s", pnlhr->pData);

	JSONROOT resp(pnlhr->pData);
	if (resp == NULL) {
		this->NotifyEvent(ptszTitle, TranslateT("Registration failed. Invalid server response."), NULL, WHATSAPP_EVENT_CLIENT);
		return false;
	}

	// Status = fail
	JSONNODE *val = json_get(resp, "status");
	if (!lstrcmp(ptrT(json_as_string(val)), _T("fail"))) {
		JSONNODE *tmpVal = json_get(resp, "reason");
		if (!lstrcmp(ptrT(json_as_string(tmpVal)), _T("stale")))
			this->NotifyEvent(ptszTitle, TranslateT("Registration failed due to stale code. Please request a new code"), NULL, WHATSAPP_EVENT_CLIENT);
		else
			this->NotifyEvent(ptszTitle, TranslateT("Registration failed."), NULL, WHATSAPP_EVENT_CLIENT);

		tmpVal = json_get(resp, "retry_after");
		if (tmpVal != NULL) {
			CMString tmp(FORMAT, TranslateT("Please try again in %i seconds"), json_as_int(tmpVal));
			this->NotifyEvent(ptszTitle, tmp, NULL, WHATSAPP_EVENT_OTHER);
		}
		return false;
	}

	//  Request code
	if (state == REG_STATE_REQ_CODE) {
		val = json_get(resp, "pw");
		if (val != NULL)
			ret = _T2A(ptrT(json_as_string(val)));
		else if (!lstrcmp(ptrT(json_as_string(val)), _T("sent")))
			this->NotifyEvent(ptszTitle, TranslateT("Registration code has been sent to your phone."), NULL, WHATSAPP_EVENT_OTHER);
		return true;
	}

	// Register
	if (state == REG_STATE_REG_CODE) {
		val = json_get(resp, "pw");
		if (val != NULL) {
			ret = _T2A(ptrT(json_as_string(val)));
			return true;
		}
		this->NotifyEvent(ptszTitle, TranslateT("Registration failed."), NULL, WHATSAPP_EVENT_CLIENT);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

INT_PTR WhatsAppProto::SvcCreateAccMgrUI(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_WHATSAPOPTIONS),
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
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_WHATSAPOPTIONS);
	odp.pfnDlgProc = WhatsAppAccountProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

int WhatsAppProto::RequestFriendship(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL || isOffline())
		return 0;

	ptrA jid(getStringA(hContact, WHATSAPP_KEY_ID));
	if (jid) {
		this->connection->sendQueryLastOnline((char*)jid);
		this->connection->sendPresenceSubscriptionRequest((char*)jid);
	}

	return 0;
}

std::tstring WhatsAppProto::GetAvatarFolder()
{
	return def_avatar_folder_;
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
	this->NotifyEvent(rawTitle, rawInfo, contact, flags, url);
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
			lstrcpy(pd.lptzContactName, title);
			lstrcpy(pd.lptzText, info);
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
