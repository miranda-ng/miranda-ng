/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"

FacebookProto::FacebookProto(const char* proto_name, const TCHAR* username) :
PROTO<FacebookProto>(proto_name, username)
{
	facy.parent = this;

	signon_lock_ = CreateMutex(NULL, FALSE, NULL);
	avatar_lock_ = CreateMutex(NULL, FALSE, NULL);
	log_lock_ = CreateMutex(NULL, FALSE, NULL);
	update_loop_lock_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	facy.buddies_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.send_message_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.fcb_conn_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.notifications_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.cookies_lock_ = CreateMutex(NULL, FALSE, NULL);

	m_hMenuRoot = m_hMenuServicesRoot = m_hStatusMind = NULL;

	m_invisible = false;
	m_signingOut = false;
	m_enableChat = DEFAULT_ENABLE_CHATS;

	// Load custom locale, if set
	ptrA locale(getStringA(FACEBOOK_KEY_LOCALE));
	if (locale != NULL)
		m_locale = locale;

	CreateProtoService(PS_CREATEACCMGRUI, &FacebookProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETMYAWAYMSG, &FacebookProto::GetMyAwayMsg);
	CreateProtoService(PS_GETMYAVATART, &FacebookProto::GetMyAvatar);
	CreateProtoService(PS_GETAVATARINFOT, &FacebookProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &FacebookProto::GetAvatarCaps);
	CreateProtoService(PS_GETUNREADEMAILCOUNT, &FacebookProto::GetNotificationsCount);

	CreateProtoService(PS_JOINCHAT, &FacebookProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &FacebookProto::OnLeaveChat);

	CreateProtoService("/Mind", &FacebookProto::OnMind);
	CreateProtoService("/VisitProfile", &FacebookProto::VisitProfile);
	CreateProtoService("/VisitNotifications", &FacebookProto::VisitNotifications);

	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &FacebookProto::OnBuildStatusMenu);
	HookProtoEvent(ME_OPT_INITIALISE, &FacebookProto::OnOptionsInit);
	HookProtoEvent(ME_IDLE_CHANGED, &FacebookProto::OnIdleChanged);
	HookProtoEvent(ME_TTB_MODULELOADED, &FacebookProto::OnToolbarInit);
	HookProtoEvent(ME_GC_EVENT, &FacebookProto::OnGCEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &FacebookProto::OnGCMenuHook);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &FacebookProto::OnDbEventRead);
	HookProtoEvent(ME_MSG_WINDOWEVENT, &FacebookProto::OnProcessSrmmEvent);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &FacebookProto::OnPreCreateEvent);

	db_set_resident(m_szModuleName, "Status");
	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, FACEBOOK_KEY_MESSAGE_READ);

	InitHotkeys();
	InitPopups();
	InitSounds();

	// Create standard network connection
	TCHAR descr[512];
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = m_szModuleName;
	mir_sntprintf(descr, SIZEOF(descr), TranslateT("%s server connection"), m_tszUserName);
	nlu.ptszDescriptiveName = descr;
	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
	if (m_hNetlibUser == NULL)
		MessageBox(NULL, TranslateT("Unable to get Netlib connection for Facebook"), m_tszUserName, MB_OK);

	facy.set_handle(m_hNetlibUser);

	// Set all contacts offline -- in case we crashed
	SetAllContactStatuses(ID_STATUS_OFFLINE);

	// register special type of event
	// there's no need to declare the special service for getting text
	// because a blob contains only text
	DBEVENTTYPEDESCR evtype = { sizeof(evtype) };
	evtype.module = m_szModuleName;
	evtype.eventType = FACEBOOK_EVENTTYPE_CALL;
	evtype.descr = LPGEN("Video call");
	evtype.eventIcon = GetIconHandle("facebook");
	evtype.flags = DETF_HISTORY | DETF_MSGWINDOW;
	CallService(MS_DB_EVENT_REGISTERTYPE, 0, (LPARAM)&evtype);
}

FacebookProto::~FacebookProto()
{
	// Uninit popup classes
	for (std::vector<HANDLE>::size_type i = 0; i < popupClasses.size(); i++)
		Popup_UnregisterClass(popupClasses[i]);
	popupClasses.clear();

	Netlib_CloseHandle(m_hNetlibUser);

	WaitForSingleObject(signon_lock_, IGNORE);
	WaitForSingleObject(avatar_lock_, IGNORE);
	WaitForSingleObject(log_lock_, IGNORE);
	WaitForSingleObject(facy.buddies_lock_, IGNORE);
	WaitForSingleObject(facy.send_message_lock_, IGNORE);
	WaitForSingleObject(facy.notifications_lock_, IGNORE);
	WaitForSingleObject(facy.cookies_lock_, IGNORE);

	CloseHandle(signon_lock_);
	CloseHandle(avatar_lock_);
	CloseHandle(log_lock_);
	CloseHandle(update_loop_lock_);
	CloseHandle(facy.buddies_lock_);
	CloseHandle(facy.send_message_lock_);
	CloseHandle(facy.fcb_conn_lock_);
	CloseHandle(facy.notifications_lock_);
	CloseHandle(facy.cookies_lock_);
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR FacebookProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
	{
		DWORD_PTR flags = PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | /*PF1_ADDED |*/ PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES; // | PF1_VISLIST | PF1_INVISLIST;

		if (getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, 0))
			return flags |= PF1_MODEMSG;
		else
			return flags |= PF1_MODEMSGRECV;
	}
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE; // | PF2_SHORTAWAY;
	case PFLAGNUM_3:
		if (getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, 0))
			return PF2_ONLINE; // | PF2_SHORTAWAY;
		else
			return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_FORCEADDED | PF4_IMSENDUTF | PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE;
	case PFLAGNUM_5:
		return PF2_ONTHEPHONE;
	case PFLAG_MAXLENOFMESSAGE:
		return FACEBOOK_MESSAGE_LIMIT;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR) "Facebook ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)FACEBOOK_KEY_ID;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::SetStatus(int new_status)
{
	debugLogA("===== Beginning SetStatus process");

	// Routing statuses not supported by Facebook
	switch (new_status)
	{
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = new_status;
		break;

	case ID_STATUS_IDLE:
	default:
		m_iDesiredStatus = ID_STATUS_INVISIBLE;
		if (getByte(FACEBOOK_KEY_MAP_STATUSES, DEFAULT_MAP_STATUSES))
			break;
	case ID_STATUS_ONLINE:
	case ID_STATUS_FREECHAT:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	}

	if (new_status != ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_CONNECTING) {
		debugLogA("===== Status is already connecting, no change");
		return 0;
	}

	if (m_iStatus == m_iDesiredStatus) {
		debugLogA("===== Statuses are same, no change");
		return 0;
	}

	m_invisible = (new_status == ID_STATUS_INVISIBLE);

	ForkThread(&FacebookProto::ChangeStatus, this);
	return 0;
}

int FacebookProto::SetAwayMsg(int, const PROTOCHAR *msg)
{
	if (!msg) {
		last_status_msg_.clear();
		return 0;
	}

	char *narrow = mir_utf8encodeT(msg);
	if (last_status_msg_ != narrow)
		last_status_msg_ = narrow;
	mir_free(narrow);

	if (isOnline() && getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS, DEFAULT_SET_MIRANDA_STATUS))
		ForkThread(&FacebookProto::SetAwayMsgWorker, NULL);

	return 0;
}

void FacebookProto::SetAwayMsgWorker(void *p)
{
	if (p != NULL) {
		status_data *data = static_cast<status_data*>(p);
		facy.post_status(data);
		delete data;
	}
	else if (!last_status_msg_.empty()) {
		status_data data;
		data.text = last_status_msg_;
		data.privacy = facy.get_privacy_type();
		facy.post_status(&data);
	}
}

HANDLE FacebookProto::SearchBasic(const PROTOCHAR* id)
{
	if (isOffline())
		return 0;

	TCHAR *tid = mir_tstrdup(id);
	ForkThread(&FacebookProto::SearchIdAckThread, tid);
	return tid;
}

HANDLE FacebookProto::SearchByEmail(const PROTOCHAR* email)
{
	if (isOffline())
		return 0;

	TCHAR *temail = mir_tstrdup(email);
	ForkThread(&FacebookProto::SearchAckThread, temail);
	return temail;
}

HANDLE FacebookProto::SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName)
{
	TCHAR arg[200];
	mir_sntprintf(arg, SIZEOF(arg), _T("%s %s %s"), nick, firstName, lastName);
	return SearchByEmail(arg); // Facebook is using one search method for everything (except IDs)
}

MCONTACT FacebookProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	ptrA id(mir_t2a_cp(psr->id, CP_UTF8));
	ptrA name(mir_t2a_cp(psr->firstName, CP_UTF8));
	ptrA surname(mir_t2a_cp(psr->lastName, CP_UTF8));

	if (id == NULL)
		return NULL;

	facebook_user fbu;
	fbu.user_id = id;
	if (name != NULL)
		fbu.real_name = name;
	if (surname != NULL) {
		fbu.real_name += " ";
		fbu.real_name += surname;
	}

	if (fbu.user_id.find_first_not_of("0123456789") != std::string::npos) {
		MessageBox(0, TranslateT("Facebook ID must be numeric value."), m_tszUserName, MB_ICONERROR | MB_OK);
		return NULL;
	}

	bool add_temporarily = (flags & PALF_TEMPORARY);
	MCONTACT hContact = AddToContactList(&fbu, CONTACT_NONE, false, add_temporarily);

	// Reset NotOnList flag if present and we're adding this contact not temporarily
	if (hContact && !add_temporarily && db_get_b(hContact, "CList", "NotOnList", 0)) {
		db_unset(hContact, "CList", "Hidden");
		db_unset(hContact, "CList", "NotOnList");
	}

	return hContact;
}

int FacebookProto::AuthRequest(MCONTACT hContact, const PROTOCHAR *)
{
	return RequestFriendship(hContact, NULL);
}

int FacebookProto::Authorize(HANDLE hDbEvent)
{
	if (!hDbEvent || isOffline())
		return 1;

	MCONTACT hContact = HContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return ApproveFriendship(hContact, NULL);
}

int FacebookProto::AuthDeny(HANDLE hDbEvent, const PROTOCHAR *)
{
	if (!hDbEvent || isOffline())
		return 1;

	MCONTACT hContact = HContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return DenyFriendship(hContact, NULL);
}

int FacebookProto::GetInfo(MCONTACT hContact, int)
{
	ptrA user_id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (user_id == NULL)
		return 1;

	facebook_user fbu;
	fbu.user_id = user_id;

	LoadContactInfo(&fbu);

	// TODO: don't duplicate code this way, refactor all this userInfo loading
	// TODO: load more info about user (authorization state,...)

	std::string homepage = FACEBOOK_URL_PROFILE + fbu.user_id;
	setString(hContact, "Homepage", homepage.c_str());

	if (!fbu.real_name.empty()) {
		SaveName(hContact, &fbu);
	}

	if (fbu.gender)
		setByte(hContact, "Gender", fbu.gender);

	CheckAvatarChange(hContact, fbu.image_url);

	return 1;
}

//////////////////////////////////////////////////////////////////////////////
// SERVICES

INT_PTR FacebookProto::GetMyAwayMsg(WPARAM, LPARAM lParam)
{
	ptrT statusMsg(getTStringA("StatusMsg"));
	if (statusMsg == NULL || statusMsg[0] == '\0')
		return 0;

	return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_t2u(statusMsg) : (INT_PTR)mir_t2a(statusMsg);
}

int FacebookProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	if (m_iStatus == ID_STATUS_INVISIBLE || m_iStatus <= ID_STATUS_OFFLINE)
		return 0;

	bool bIdle = (lParam & IDF_ISIDLE) != 0;
	bool bPrivacy = (lParam & IDF_PRIVACY) != 0;

	if (facy.is_idle_ && !bIdle)
	{
		facy.is_idle_ = false;
		SetStatus(m_iDesiredStatus);
	}
	else if (!facy.is_idle_ && bIdle && !bPrivacy && m_iDesiredStatus != ID_STATUS_INVISIBLE)
	{
		facy.is_idle_ = true;
		SetStatus(ID_STATUS_IDLE);
	}

	return 0;
}

INT_PTR FacebookProto::GetNotificationsCount(WPARAM, LPARAM)
{
	if (isOffline())
		return 0;

	return facy.notifications.size();
}

//////////////////////////////////////////////////////////////////////////////

int FacebookProto::OnEvent(PROTOEVENTTYPE event, WPARAM wParam, LPARAM lParam)
{
	switch (event)
	{
	case EV_PROTO_ONLOAD:
		return OnModulesLoaded(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return OnPreShutdown(wParam, lParam);

	case EV_PROTO_ONOPTIONS:
		return OnOptionsInit(wParam, lParam);

	case EV_PROTO_ONCONTACTDELETED:
		return OnContactDeleted(wParam, lParam);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////////
// EVENTS

INT_PTR FacebookProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_FACEBOOKACCOUNT),
		(HWND)lParam, FBAccountProc, (LPARAM)this);
}

int FacebookProto::OnModulesLoaded(WPARAM, LPARAM)
{
	// Register group chat
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.dwFlags = 0; //GC_ACKMSG;
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = FACEBOOK_MESSAGE_LIMIT;
	gcr.nColors = 0;
	gcr.pColors = NULL;
	CallService(MS_GC_REGISTER, 0, reinterpret_cast<LPARAM>(&gcr));

	return 0;
}

int FacebookProto::OnPreShutdown(WPARAM, LPARAM)
{
	SetStatus(ID_STATUS_OFFLINE);
	return 0;
}

int FacebookProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position = 271828;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = FBOptionsProc;
	Options_AddPage(wParam, &odp);

	odp.position = 271829;
	odp.ptszTab = LPGENT("Events");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_EVENTS);
	odp.pfnDlgProc = FBOptionsEventsProc;
	Options_AddPage(wParam, &odp);

	odp.position = 271830;
	odp.ptszTab = LPGENT("Statuses");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_STATUSES);
	odp.pfnDlgProc = FBOptionsStatusesProc;
	Options_AddPage(wParam, &odp);

	odp.position = 271831;
	odp.ptszTab = LPGENT("Messaging");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MESSAGING);
	odp.pfnDlgProc = FBOptionsMessagingProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

int FacebookProto::OnToolbarInit(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;

	char service[100];
	mir_snprintf(service, SIZEOF(service), "%s%s", m_szModuleName, "/Mind");

	ttb.pszService = service;
	ttb.pszTooltipUp = ttb.name = LPGEN("Share status...");
	ttb.hIconHandleUp = Skin_GetIconByHandle(GetIconHandle("mind"));
	TopToolbar_AddButton(&ttb);

	return 0;
}

INT_PTR FacebookProto::OnMind(WPARAM wParam, LPARAM)
{
	if (!isOnline())
		return 1;

	MCONTACT hContact = MCONTACT(wParam);

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return 1;

	wall_data *wall = new wall_data();
	wall->user_id = id;
	wall->isPage = false;
	if (wall->user_id == facy.self_.user_id) {
		wall->title = _tcsdup(TranslateT("Own wall"));
	}
	else
		wall->title = getTStringA(hContact, FACEBOOK_KEY_NICK);

	post_status_data *data = new post_status_data(this, wall);

	if (wall->user_id == facy.self_.user_id) {
		for (std::map<std::string, std::string>::iterator iter = facy.pages.begin(); iter != facy.pages.end(); ++iter) {
			data->walls.push_back(new wall_data(iter->first, mir_utf8decodeT(iter->second.c_str()), true));
		}
	}

	HWND hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_MIND), (HWND)0, FBMindProc, reinterpret_cast<LPARAM>(data));
	ShowWindow(hDlg, SW_SHOW);

	return 0;
}

int FacebookProto::OnDbEventRead(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;

	if (isOffline() || !IsMyContact(hContact, false)) // ignore chats
		return 0;

	if (facy.ignore_read.find(hContact) != facy.ignore_read.end())
		return 0; // it's there, so we ignore this

	std::set<MCONTACT> *hContacts = new std::set<MCONTACT>();
	hContacts->insert(hContact);

	ForkThread(&FacebookProto::ReadMessageWorker, (void*)hContacts);

	return 0;
}


int FacebookProto::OnProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPENING) {
		// Set statusbar to "Message read" time (if present)
		MessageRead(event->hContact);
	}
	else if (event->uType == MSG_WINDOW_EVT_OPEN) {
		// Check if we have enabled loading messages on open window
		if (!getBool(FACEBOOK_KEY_MESSAGES_ON_OPEN, DEFAULT_MESSAGES_ON_OPEN) || isChatRoom(event->hContact))
			return 0;

		// Load last messages for this contact
		ForkThread(&FacebookProto::LoadLastMessages, new MCONTACT(event->hContact));
	}

	return 0;
}

int FacebookProto::OnPreCreateEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEvent *evt = (MessageWindowEvent *)lParam;
	if (strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;

	std::map<int, DWORD>::iterator it = facy.messages_timestamp.find(evt->seq);
	if (it != facy.messages_timestamp.end()) {
		// set correct timestamp of this message
		evt->dbei->timestamp = it->second;
		facy.messages_timestamp.erase(it);
	}
	
	return 1;
}

INT_PTR FacebookProto::CheckNewsfeeds(WPARAM, LPARAM)
{
	if (!isOffline()) {
		facy.client_notify(TranslateT("Loading newsfeeds..."));
		facy.last_feeds_update_ = 0;
		ForkThread(&FacebookProto::ProcessFeeds, NULL);
	}
	return 0;
}

INT_PTR FacebookProto::CheckFriendRequests(WPARAM, LPARAM)
{
	if (!isOffline()) {
		facy.client_notify(TranslateT("Checking friend requests..."));
		ProcessFriendRequests(NULL);
	}
	return 0;
}

INT_PTR FacebookProto::RefreshBuddyList(WPARAM, LPARAM)
{
	if (!isOffline()) {
		facy.client_notify(TranslateT("Refreshing buddy list..."));
		ForkThread(&FacebookProto::ProcessBuddyList, NULL);
	}
	return 0;
}


INT_PTR FacebookProto::VisitProfile(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = MCONTACT(wParam);

	std::string url = FACEBOOK_URL_PROFILE;

	ptrA val(getStringA(hContact, "Homepage"));
	if (val != NULL) {
		// Homepage link already present, get it
		url = val;
	}
	else {
		// No homepage link, create and save it
		val = getStringA(hContact, FACEBOOK_KEY_ID);
		if (val != NULL) {
			url += val;
			setString(hContact, "Homepage", url.c_str());
		}
	}

	OpenUrl(url);
	return 0;
}

INT_PTR FacebookProto::VisitFriendship(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = MCONTACT(wParam);

	if (wParam == 0 || !IsMyContact(hContact))
		return 1;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return 1;

	std::string url = FACEBOOK_URL_PROFILE;
	url += facy.self_.user_id;
	url += "&and=" + std::string(id);

	OpenUrl(url);
	return 0;
}

INT_PTR FacebookProto::VisitConversation(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = MCONTACT(wParam);

	if (wParam == 0 || !IsMyContact(hContact, true))
		return 1;

	bool isChat = isChatRoom(hContact);
	ptrA id(getStringA(hContact, isChat ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID));
	if (id == NULL)
		return 1;

	std::string url = FACEBOOK_URL_CONVERSATION + std::string(isChat ? "conversation-" : "") + std::string(id);

	OpenUrl(url);
	return 0;
}

INT_PTR FacebookProto::VisitNotifications(WPARAM, LPARAM)
{
	/*bool useChatRoom = getBool(FACEBOOK_KEY_NOTIFICATIONS_CHATROOM, DEFAULT_NOTIFICATIONS_CHATROOM);

	if (useChatRoom) {
	GCDEST gcd = { m_szModuleName, _T(FACEBOOK_NOTIFICATIONS_CHATROOM), GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, reinterpret_cast<LPARAM>(&gce));
	}
	else {*/
	OpenUrl(FACEBOOK_URL_NOTIFICATIONS);
	/*}*/

	return 0;
}

INT_PTR FacebookProto::Poke(WPARAM wParam, LPARAM)
{
	if (wParam == NULL || isOffline())
		return 1;

	MCONTACT hContact = MCONTACT(wParam);

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (id == NULL)
		return 1;

	ForkThread(&FacebookProto::SendPokeWorker, new std::string(id));
	return 0;
}

INT_PTR FacebookProto::CancelFriendship(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL || isOffline())
		return 1;

	bool deleting = (lParam == 1);

	MCONTACT hContact = MCONTACT(wParam);

	// Ignore groupchats and, if deleting, also not-friends
	if (isChatRoom(hContact) || (deleting && getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND))
		return 0;

	ptrT tname(getTStringA(hContact, FACEBOOK_KEY_NICK));
	if (tname == NULL)
		tname = getTStringA(hContact, FACEBOOK_KEY_ID);

	if (tname == NULL)
		return 1;

	TCHAR tstr[256];
	mir_sntprintf(tstr, SIZEOF(tstr), TranslateT("Do you want to cancel your friendship with '%s'?"), tname);
	if (MessageBox(0, tstr, m_tszUserName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {

		ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
		if (id == NULL)
			return 1;

		std::string *val = new std::string(id);

		if (deleting) {
			facebook_user *fbu = facy.buddies.find(*val);
			if (fbu != NULL)
				fbu->handle = NULL;
		}

		ForkThread(&FacebookProto::DeleteContactFromServer, val);
	}

	return 0;
}

INT_PTR FacebookProto::RequestFriendship(WPARAM wParam, LPARAM)
{
	if (wParam == NULL || isOffline())
		return 1;

	MCONTACT hContact = MCONTACT(wParam);

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (id == NULL)
		return 1;

	ForkThread(&FacebookProto::AddContactToServer, new std::string(id));
	return 0;
}

INT_PTR FacebookProto::ApproveFriendship(WPARAM wParam, LPARAM)
{
	if (wParam == NULL || isOffline())
		return 1;

	MCONTACT *hContact = new MCONTACT((MCONTACT)wParam);

	ForkThread(&FacebookProto::ApproveContactToServer, hContact);
	return 0;
}

INT_PTR FacebookProto::DenyFriendship(WPARAM wParam, LPARAM)
{
	if (wParam == NULL || isOffline())
		return 1;

	MCONTACT *hContact = new MCONTACT((MCONTACT)wParam);

	ForkThread(&FacebookProto::IgnoreFriendshipRequest, hContact);

	return 0;
}

INT_PTR FacebookProto::OnCancelFriendshipRequest(WPARAM wParam, LPARAM)
{
	if (wParam == NULL || isOffline())
		return 1;

	MCONTACT *hContact = new MCONTACT((MCONTACT)wParam);

	ForkThread(&FacebookProto::CancelFriendsRequest, hContact);
	return 0;
}

MCONTACT FacebookProto::HContactFromAuthEvent(HANDLE hEvent)
{
	DWORD body[2];
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (strcmp(dbei.szModule, m_szModuleName))
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

void FacebookProto::OpenUrlThread(void *p) {
	if (p == NULL)
		return;

	open_url *data = static_cast<open_url*>(p);

	ShellExecute(NULL, _T("open"), data->browser, data->url, NULL, SW_SHOWDEFAULT);

	delete data;
}

std::string FacebookProto::PrepareUrl(std::string url) {
	std::string::size_type pos = url.find(FACEBOOK_SERVER_DOMAIN);
	bool isFacebookUrl = (pos != std::string::npos);
	bool isRelativeUrl = (url.substr(0, 4) != "http");

	if (isFacebookUrl || isRelativeUrl) {

		// Make realtive url
		if (!isRelativeUrl) {
			url = url.substr(pos + strlen(FACEBOOK_SERVER_DOMAIN));

			// Strip eventual port
			pos = url.find("/");
			if (pos != std::string::npos && pos != 0)
				url = url.substr(pos);
		}

		// Make absolute url
		bool useHttps = getByte(FACEBOOK_KEY_FORCE_HTTPS, 1) > 0;
		url = (useHttps ? HTTP_PROTO_SECURE : HTTP_PROTO_REGULAR) + facy.get_server_type() + url;
	}

	return url;
}

void FacebookProto::OpenUrl(std::string url)
{
	url = PrepareUrl(url);
	ptrT data(mir_utf8decodeT(url.c_str()));

	// Check if there is user defined browser for opening links
	ptrT browser(getTStringA(FACEBOOK_KEY_OPEN_URL_BROWSER));
	if (browser != NULL)
		// If so, use it to open this link
		ForkThread(&FacebookProto::OpenUrlThread, new open_url(browser, data));
	else
		// Or use Miranda's service
		CallService(MS_UTILS_OPENURL, (WPARAM)OUF_TCHAR, (LPARAM)data);
}

void FacebookProto::ReadNotificationWorker(void *p)
{
	if (p == NULL)
		return;

	std::string *id = (std::string*)p;

	if (isOffline()) {
		delete id;
		return;
	}

	std::string data = "seen=0&asyncSignal=&__dyn=&__req=a&alert_ids%5B0%5D=" + utils::url::encode(*id);
	data += "&fb_dtsg=" + facy.dtsg_;
	data += "&__user=" + facy.self_.user_id;

	facy.flap(REQUEST_NOTIFICATIONS_READ, NULL, &data);

	delete id;
}

/**
 * Popup processing callback
 */
LRESULT CALLBACK PopupDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	case WM_CONTEXTMENU:
	{
		// Get the plugin data (we need the Popup service to do it)
		popup_data *data = (popup_data *)PUGetPluginData(hwnd);
		if (data != NULL) {
			if (!data->notification_id.empty())
				data->proto->ForkThread(&FacebookProto::ReadNotificationWorker, new std::string(data->notification_id));

			if (message == WM_COMMAND && !data->url.empty())
				data->proto->OpenUrl(data->url);
		}

		// After a click, destroy popup
		PUDeletePopup(hwnd);
	} break;

	case UM_FREEPLUGINDATA:
	{
		// After close, free
		popup_data *data = (popup_data *)PUGetPluginData(hwnd);
		delete data;
	} return FALSE;

	default:
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
};

/**
 * Popup classes initialization
 */
void FacebookProto::InitPopups()
{
	POPUPCLASS ppc = { sizeof(ppc) };
	ppc.flags = PCF_TCHAR;
	ppc.PluginWindowProc = PopupDlgProc;
	ppc.lParam = APF_RETURN_HWND;

	TCHAR desc[256];
	char name[256];

	// Client
	mir_sntprintf(desc, SIZEOF(desc), _T("%s/%s"), m_tszUserName, TranslateT("Client notifications"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Client");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("facebook"));
	ppc.colorBack = RGB(191, 0, 0); // red
	ppc.colorText = RGB(255, 255, 255); // white
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Newsfeeds
	mir_sntprintf(desc, SIZEOF(desc), _T("%s/%s"), m_tszUserName, TranslateT("News feeds"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Newsfeed");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("newsfeed"));
	ppc.colorBack = RGB(255, 255, 255); // white
	ppc.colorText = RGB(0, 0, 0); // black
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Notifications
	mir_sntprintf(desc, SIZEOF(desc), _T("%s/%s"), m_tszUserName, TranslateT("Notifications"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Notification");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("notification"));
	ppc.colorBack = RGB(59, 89, 152); // Facebook's blue
	ppc.colorText = RGB(255, 255, 255); // white
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Others
	mir_sntprintf(desc, SIZEOF(desc), _T("%s/%s"), m_tszUserName, TranslateT("Other events"));
	mir_snprintf(name, SIZEOF(name), "%s_%s", m_szModuleName, "Other");
	ppc.ptszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = Skin_GetIconByHandle(GetIconHandle("facebook"));
	ppc.colorBack = RGB(255, 255, 255); // white
	ppc.colorText = RGB(0, 0, 0); // black
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));
}

/**
 * Hotkeys initialiation
 */
void FacebookProto::InitHotkeys()
{
	char text[200];
	mir_strncpy(text, m_szModuleName, 100);
	char *tDest = text + strlen(text);

	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.ptszSection = m_tszUserName;
	hkd.dwFlags = HKD_TCHAR;

	strcpy(tDest, "/VisitProfile");
	hkd.ptszDescription = LPGENT("Visit profile");
	Hotkey_Register(&hkd);

	strcpy(tDest, "/VisitNotifications");
	hkd.ptszDescription = LPGENT("Visit notifications");
	Hotkey_Register(&hkd);

	strcpy(tDest, "/Mind");
	hkd.ptszDescription = LPGENT("Show 'Share status' window");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_EXT, 'F');
	Hotkey_Register(&hkd);
}

/**
 * Sounds initialization
 */
void FacebookProto::InitSounds()
{
	SkinAddNewSoundExT("Notification", m_tszUserName, LPGENT("Notification"));
	SkinAddNewSoundExT("NewsFeed", m_tszUserName, LPGENT("News Feed"));
	SkinAddNewSoundExT("OtherEvent", m_tszUserName, LPGENT("Other Event"));
}

/**
 * Sets statusbar text of hContact with last read time (from facy.readers map)
 */
void FacebookProto::MessageRead(MCONTACT hContact)
{
	/*std::map<MCONTACT, time_t>::iterator it = facy.readers.find(hContact);
	if (it == facy.readers.end())
	return;*/

	// We may use this instead of checing map as we have this info already in memory (this value is resident)
	time_t time = getDword(hContact, FACEBOOK_KEY_MESSAGE_READ, 0);
	if (!time)
		return;

	TCHAR ttime[64];
	_tcsftime(ttime, SIZEOF(ttime), _T("%X"), localtime(&time));

	StatusTextData st = { 0 };
	st.cbSize = sizeof(st);
	st.hIcon = Skin_GetIconByHandle(GetIconHandle("read"));
	mir_sntprintf(st.tszText, SIZEOF(st.tszText), TranslateT("Message read: %s"), ttime);

	CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hContact, (LPARAM)&st);
}
