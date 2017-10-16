/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-17 Robert P�sel

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

#include "stdafx.h"

FacebookProto::FacebookProto(const char* proto_name, const wchar_t* username) :
	PROTO<FacebookProto>(proto_name, username),
	m_tszDefaultGroup(getWStringA(FACEBOOK_KEY_DEF_GROUP))
{
	facy.parent = this;

	signon_lock_ = CreateMutex(NULL, FALSE, NULL);
	avatar_lock_ = CreateMutex(NULL, FALSE, NULL);
	log_lock_ = CreateMutex(NULL, FALSE, NULL);
	update_loop_lock_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	facy.send_message_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.fcb_conn_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.notifications_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.cookies_lock_ = CreateMutex(NULL, FALSE, NULL);
	facy.loading_history_lock_ = CreateMutex(NULL, FALSE, NULL);

	// Initialize random seed for this client
	facy.random_ = ::time(NULL) + PtrToUint(&facy);

	m_enableChat = DEFAULT_ENABLE_CHATS;

	// Load custom locale, if set
	ptrA locale(getStringA(FACEBOOK_KEY_LOCALE));
	if (locale != NULL)
		m_locale = locale;

	// Load custom page prefix, if set
	ptrW pagePrefix(getWStringA(FACEBOOK_KEY_PAGE_PREFIX));
	m_pagePrefix = (pagePrefix != NULL) ? _T2A(pagePrefix, CP_UTF8) : TEXT_EMOJI_PAGE;

	if (m_tszDefaultGroup == NULL)
		m_tszDefaultGroup = mir_wstrdup(L"Facebook");

	CreateProtoService(PS_CREATEACCMGRUI, &FacebookProto::SvcCreateAccMgrUI);
	CreateProtoService(PS_GETMYAWAYMSG, &FacebookProto::GetMyAwayMsg);
	CreateProtoService(PS_GETMYAVATAR, &FacebookProto::GetMyAvatar);
	CreateProtoService(PS_GETAVATARINFO, &FacebookProto::GetAvatarInfo);
	CreateProtoService(PS_GETAVATARCAPS, &FacebookProto::GetAvatarCaps);
	CreateProtoService(PS_GETUNREADEMAILCOUNT, &FacebookProto::GetNotificationsCount);

	CreateProtoService(PS_JOINCHAT, &FacebookProto::OnJoinChat);
	CreateProtoService(PS_LEAVECHAT, &FacebookProto::OnLeaveChat);

	HookProtoEvent(ME_CLIST_PREBUILDSTATUSMENU, &FacebookProto::OnBuildStatusMenu);
	HookProtoEvent(ME_OPT_INITIALISE, &FacebookProto::OnOptionsInit);
	HookProtoEvent(ME_IDLE_CHANGED, &FacebookProto::OnIdleChanged);
	HookProtoEvent(ME_TTB_MODULELOADED, &FacebookProto::OnToolbarInit);
	HookProtoEvent(ME_GC_EVENT, &FacebookProto::OnGCEvent);
	HookProtoEvent(ME_GC_BUILDMENU, &FacebookProto::OnGCMenuHook);
	HookProtoEvent(ME_DB_EVENT_MARKED_READ, &FacebookProto::OnDbEventRead);

	db_set_resident(m_szModuleName, "IdleTS");
	db_set_resident(m_szModuleName, FACEBOOK_KEY_MESSAGE_READ);
	db_set_resident(m_szModuleName, FACEBOOK_KEY_MESSAGE_READERS);
	db_set_resident(m_szModuleName, FACEBOOK_KEY_TRIED_DELETING_DEVICE_ID);

	InitHotkeys();
	InitPopups();
	InitSounds();

	// Create standard network connection
	wchar_t descr[512];
	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = m_szModuleName;
	mir_snwprintf(descr, TranslateT("%s server connection"), m_tszUserName);
	nlu.szDescriptiveName.w = descr;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
	if (m_hNetlibUser == NULL) {
		wchar_t error[200];
		mir_snwprintf(error, TranslateT("Unable to initialize Netlib for %s."), m_tszUserName);
		MessageBox(NULL, error, L"Miranda NG", MB_OK | MB_ICONERROR);
	}

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
	DbEvent_RegisterType(&evtype);
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
	WaitForSingleObject(facy.send_message_lock_, IGNORE);
	WaitForSingleObject(facy.notifications_lock_, IGNORE);
	WaitForSingleObject(facy.cookies_lock_, IGNORE);
	WaitForSingleObject(facy.loading_history_lock_, IGNORE);

	CloseHandle(signon_lock_);
	CloseHandle(avatar_lock_);
	CloseHandle(log_lock_);
	CloseHandle(update_loop_lock_);
	CloseHandle(facy.send_message_lock_);
	CloseHandle(facy.fcb_conn_lock_);
	CloseHandle(facy.notifications_lock_);
	CloseHandle(facy.cookies_lock_);
	CloseHandle(facy.loading_history_lock_);
}

//////////////////////////////////////////////////////////////////////////////

DWORD_PTR FacebookProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
	{
		DWORD_PTR flags = PF1_IM | PF1_CHAT | PF1_SERVERCLIST | PF1_AUTHREQ | /*PF1_ADDED |*/ PF1_BASICSEARCH | PF1_SEARCHBYEMAIL | PF1_SEARCHBYNAME | PF1_ADDSEARCHRES; // | PF1_VISLIST | PF1_INVISLIST;

		if (getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS))
			return flags |= PF1_MODEMSG;
		else
			return flags |= PF1_MODEMSGRECV;
	}
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_INVISIBLE | PF2_ONTHEPHONE | PF2_IDLE;
	case PFLAGNUM_3:
		if (getByte(FACEBOOK_KEY_SET_MIRANDA_STATUS))
			return PF2_ONLINE; // | PF2_SHORTAWAY;
		else
			return 0;
	case PFLAGNUM_4:
		return PF4_NOCUSTOMAUTH | PF4_FORCEADDED | PF4_AVATARS | PF4_SUPPORTTYPING | PF4_NOAUTHDENYREASON | PF4_IMSENDOFFLINE | PF4_READNOTIFY;
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
	debugLogA("=== Beginning SetStatus process");

	if (new_status != ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_CONNECTING) {
		debugLogA("=== Status is already connecting, no change");
		return 0;
	}

	// Routing statuses not supported by Facebook
	switch (new_status)
	{
	case ID_STATUS_ONLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_INVISIBLE:
	case ID_STATUS_OFFLINE:
		m_iDesiredStatus = new_status;
		break;
	case ID_STATUS_NA:
		m_iDesiredStatus = ID_STATUS_AWAY;
		break;
	case ID_STATUS_FREECHAT:
		m_iDesiredStatus = ID_STATUS_ONLINE;
		break;
	default:
		m_iDesiredStatus = getByte(FACEBOOK_KEY_MAP_STATUSES, DEFAULT_MAP_STATUSES) ? ID_STATUS_INVISIBLE : ID_STATUS_AWAY;
		break;
	}

	if (m_iStatus == m_iDesiredStatus) {
		debugLogA("=== Statuses are same, no change");
		return 0;
	}

	m_invisible = (new_status == ID_STATUS_INVISIBLE);

	ForkThread(&FacebookProto::ChangeStatus, this);
	return 0;
}

int FacebookProto::SetAwayMsg(int, const wchar_t *msg)
{
	if (!msg) {
		last_status_msg_.clear();
		return 0;
	}

	T2Utf narrow(msg);
	if (last_status_msg_ != (char*)narrow)
		last_status_msg_ = narrow;

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

HANDLE FacebookProto::SearchBasic(const wchar_t* id)
{
	if (isOffline())
		return 0;

	wchar_t *tid = mir_wstrdup(id);
	ForkThread(&FacebookProto::SearchIdAckThread, tid);
	return tid;
}

HANDLE FacebookProto::SearchByEmail(const wchar_t* email)
{
	if (isOffline())
		return 0;

	wchar_t *temail = mir_wstrdup(email);
	ForkThread(&FacebookProto::SearchAckThread, temail);
	return temail;
}

HANDLE FacebookProto::SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName)
{
	wchar_t arg[200];
	mir_snwprintf(arg, L"%s %s %s", nick, firstName, lastName);
	return SearchByEmail(arg); // Facebook is using one search method for everything (except IDs)
}

MCONTACT FacebookProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	ptrA id(mir_u2a_cp(psr->id.w, CP_UTF8));
	ptrA name(mir_u2a_cp(psr->firstName.w, CP_UTF8));
	ptrA surname(mir_u2a_cp(psr->lastName.w, CP_UTF8));

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
	MCONTACT hContact = AddToContactList(&fbu, false, add_temporarily);

	// Reset NotOnList flag if present and we're adding this contact not temporarily
	if (hContact && !add_temporarily && db_get_b(hContact, "CList", "NotOnList", 0)) {
		db_unset(hContact, "CList", "Hidden");
		db_unset(hContact, "CList", "NotOnList");
	}

	return hContact;
}

int FacebookProto::AuthRequest(MCONTACT hContact, const wchar_t *)
{
	return RequestFriendship(hContact, NULL);
}

int FacebookProto::Authorize(MEVENT hDbEvent)
{
	if (!hDbEvent || isOffline())
		return 1;

	MCONTACT hContact = HContactFromAuthEvent(hDbEvent);
	if (hContact == INVALID_CONTACT_ID)
		return 1;

	return ApproveFriendship(hContact, NULL);
}

int FacebookProto::AuthDeny(MEVENT hDbEvent, const wchar_t *)
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
	ForkThread(&FacebookProto::RefreshUserInfo, (void*)new MCONTACT(hContact));
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// SERVICES

INT_PTR FacebookProto::GetMyAwayMsg(WPARAM, LPARAM lParam)
{
	ptrW statusMsg(getWStringA("StatusMsg"));
	if (statusMsg == NULL || statusMsg[0] == '\0')
		return 0;

	return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_wstrdup(statusMsg) : (INT_PTR)mir_u2a(statusMsg);
}

int FacebookProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	bool idle = (lParam & IDF_ISIDLE) != 0;
	bool privacy = (lParam & IDF_PRIVACY) != 0;

	// Respect user choice about (not) notifying idle to protocols
	if (privacy) {
		// Reset it to 0 if there is some time already
		if (m_idleTS)
		{
			m_idleTS = 0;
			delSetting("IdleTS");
		}

		return 0;
	}

	// We don't want to reset idle time when we're already in idle state
	if (idle && m_idleTS > 0)
		return 0;

	if (idle) {
		// User started being idle
		MIRANDA_IDLE_INFO mii = { sizeof(mii) };
		CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii);

		// Compute time when user really became idle
		m_idleTS = time(0) - mii.idleTime * 60;
		setDword("IdleTS", m_idleTS);
	} else {
		// User stopped being idle
		m_idleTS = 0;
		delSetting("IdleTS");

		// Set sending activity_ping at next channel request (because I don't want to create new thread here for such small thing)
		m_pingTS = 0;
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

	case EV_PROTO_ONMENU:
		InitMenu();
		break;
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
	HookProtoEvent(ME_MSG_WINDOWEVENT, &FacebookProto::OnProcessSrmmEvent);
	HookProtoEvent(ME_MSG_PRECREATEEVENT, &FacebookProto::OnPreCreateEvent);

	// Register group chat
	GCREGISTER gcr = {};
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = FACEBOOK_MESSAGE_LIMIT;
	Chat_Register(&gcr);
	return 0;
}

int FacebookProto::OnPreShutdown(WPARAM, LPARAM)
{
	SetStatus(ID_STATUS_OFFLINE);
	return 0;
}

int FacebookProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInstance;
	odp.szTitle.w = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;

	odp.position = 271828;
	odp.szGroup.w = LPGENW("Network");
	odp.szTab.w = LPGENW("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = FBOptionsProc;
	Options_AddPage(wParam, &odp);

	odp.position = 271829;
	odp.szTab.w = LPGENW("Events");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_EVENTS);
	odp.pfnDlgProc = FBOptionsEventsProc;
	Options_AddPage(wParam, &odp);

	odp.position = 271830;
	odp.szTab.w = LPGENW("Statuses");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_STATUSES);
	odp.pfnDlgProc = FBOptionsStatusesProc;
	Options_AddPage(wParam, &odp);

	odp.position = 271831;
	odp.szTab.w = LPGENW("Messaging");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MESSAGING);
	odp.pfnDlgProc = FBOptionsMessagingProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

int FacebookProto::OnToolbarInit(WPARAM, LPARAM)
{
	TTBButton ttb = { 0 };
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;

	char service[100];
	mir_snprintf(service, "%s%s", m_szModuleName, "/Mind");

	ttb.pszService = service;
	ttb.pszTooltipUp = ttb.name = LPGEN("Share status...");
	ttb.hIconHandleUp = IcoLib_GetIconByHandle(GetIconHandle("mind"));
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
		wall->title = wcsdup(TranslateT("Own wall"));
	}
	else
		wall->title = getWStringA(hContact, FACEBOOK_KEY_NICK);

	post_status_data *data = new post_status_data(this, wall);

	if (wall->user_id == facy.self_.user_id) {
		for (std::map<std::string, std::string>::iterator iter = facy.pages.begin(); iter != facy.pages.end(); ++iter) {
			data->walls.push_back(new wall_data(iter->first, mir_utf8decodeW(iter->second.c_str()), true));
		}
	}

	HWND hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_MIND), (HWND)0, FBMindProc, reinterpret_cast<LPARAM>(data));
	ShowWindow(hDlg, SW_SHOW);

	return 0;
}

int FacebookProto::OnDbEventRead(WPARAM, LPARAM lParam)
{
	MCONTACT hContact = db_event_getContact((MEVENT)lParam);

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
	if (mir_strcmp(GetContactProto(evt->hContact), m_szModuleName))
		return 0;

	std::map<int, time_t>::iterator it = facy.messages_timestamp.find(evt->seq);
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
		// If holding control, load all newsfeeds (not only newer since last check)
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		if (ctrlPressed) {
			facy.last_feeds_update_ = 0;
		}
		ForkThread(&FacebookProto::ProcessFeeds, MANUALLY_TRIGGERED);
	}
	return 0;
}

INT_PTR FacebookProto::CheckFriendRequests(WPARAM, LPARAM)
{
	if (!isOffline()) {
		ForkThread(&FacebookProto::ProcessFriendRequests, MANUALLY_TRIGGERED);
	}
	return 0;
}

INT_PTR FacebookProto::CheckNotifications(WPARAM, LPARAM)
{
	if (!isOffline()) {
		ForkThread(&FacebookProto::ProcessNotifications, MANUALLY_TRIGGERED);
	}
	return 0;
}

INT_PTR FacebookProto::CheckMemories(WPARAM, LPARAM)
{
	if (!isOffline()) {
		ForkThread(&FacebookProto::ProcessMemories, MANUALLY_TRIGGERED);
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

	std::string url = FACEBOOK_URL_CONVERSATION + std::string(id);

	OpenUrl(url);
	return 0;
}

INT_PTR FacebookProto::VisitNotifications(WPARAM, LPARAM)
{
	/*bool useChatRoom = getBool(FACEBOOK_KEY_NOTIFICATIONS_CHATROOM, DEFAULT_NOTIFICATIONS_CHATROOM);

	if (useChatRoom) {
	GCEVENT gce = { m_szModuleName, _T(FACEBOOK_NOTIFICATIONS_CHATROOM), GC_EVENT_CONTROL };
	Chat_Control(WINDOW_VISIBLE);
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

INT_PTR FacebookProto::LoadHistory(WPARAM wParam, LPARAM)
{
	if (wParam == NULL || isOffline())
		return 1;

	MCONTACT hContact = MCONTACT(wParam);

	// Ignore groupchats // TODO: Support for groupchats?
	if (isChatRoom(hContact))
		return 0;

	// Allow loading history only from one contact at a time
	if (facy.loading_history) {
		const wchar_t *message = TranslateT("Loading history is already in progress. It can't run for more contacts at once so please wait until it finishes.");
		MessageBox(0, message, m_tszUserName, MB_ICONWARNING | MB_OK);
		return 0;
	}

	ptrW name(getWStringA(hContact, FACEBOOK_KEY_NICK));
	if (name == NULL)
		name = getWStringA(hContact, FACEBOOK_KEY_ID);
	if (name == NULL)
		return 1;

	CMStringW title;
	title.AppendFormat(L"%s - %s", m_tszUserName, name);
	const wchar_t *message = TranslateT("This will load all messages from the server. To avoid having duplicate messages in your history, delete existing messages manually before continuing.\nLoading process might take a while, so be patient.\n\nDo you want to continue?");

	if (MessageBox(0, message, title, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		ForkThread(&FacebookProto::LoadHistory, new MCONTACT(hContact));
	}

	return 0;
}

INT_PTR FacebookProto::CancelFriendship(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL || isOffline())
		return 1;

	bool deleting = (lParam == 1);

	MCONTACT hContact = MCONTACT(wParam);

	// Ignore groupchats and, if deleting, also not-friends
	if (isChatRoom(hContact) || (deleting && getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE) != CONTACT_FRIEND))
		return 0;

	ptrW tname(getWStringA(hContact, FACEBOOK_KEY_NICK));
	if (tname == NULL)
		tname = getWStringA(hContact, FACEBOOK_KEY_ID);

	if (tname == NULL)
		return 1;

	wchar_t tstr[256];
	mir_snwprintf(tstr, TranslateT("Do you want to cancel your friendship with '%s'?"), tname);
	if (MessageBox(0, tstr, m_tszUserName, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {

		ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
		if (id == NULL)
			return 1;

		std::string *val = new std::string(id);

		// FIXME: Remember that we deleted this contact, so we won't accidentally add him at status change
		/*if (deleting) {
			facebook_user *fbu = facy.buddies.find(*val);
			if (fbu != NULL)
				fbu->handle = NULL;
		}*/

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

MCONTACT FacebookProto::HContactFromAuthEvent(MEVENT hEvent)
{
	DWORD body[2];
	DBEVENTINFO dbei = {};
	dbei.cbBlob = sizeof(DWORD) * 2;
	dbei.pBlob = (PBYTE)&body;

	if (db_event_get(hEvent, &dbei))
		return INVALID_CONTACT_ID;

	if (dbei.eventType != EVENTTYPE_AUTHREQUEST)
		return INVALID_CONTACT_ID;

	if (mir_strcmp(dbei.szModule, m_szModuleName))
		return INVALID_CONTACT_ID;

	return DbGetAuthEventContact(&dbei);
}

void FacebookProto::OpenUrlThread(void *p) {
	if (p == NULL)
		return;

	open_url *data = static_cast<open_url*>(p);

	ShellExecute(NULL, L"open", data->browser, data->url, NULL, SW_SHOWDEFAULT);

	delete data;
}

std::string FacebookProto::PrepareUrl(std::string url) {
	std::string::size_type pos = url.find(FACEBOOK_SERVER_DOMAIN);
	bool isFacebookUrl = (pos != std::string::npos);
	bool isAbsoluteUrl = (url.find("://") != std::string::npos);

	// Do nothing with absolute non-Facebook URLs
	if (isAbsoluteUrl && !isFacebookUrl)
		return url;
	
	// Transform absolute URL to relative
	if (isAbsoluteUrl) {
		// Check and ignore special subdomains
		std::string subdomain = utils::text::source_get_value(&url, 2, "://", "." FACEBOOK_SERVER_DOMAIN);
		if (subdomain == "developers")
			return url;
		
		// Make relative url
		url = url.substr(pos + mir_strlen(FACEBOOK_SERVER_DOMAIN));

		// Strip eventual port
		pos = url.find("/");
		if (pos != std::string::npos && pos != 0)
			url = url.substr(pos);
	}

	// URL is relative now, make and return absolute
	return HTTP_PROTO_SECURE + facy.get_server_type() + url;
}

void FacebookProto::OpenUrl(std::string url)
{
	url = PrepareUrl(url);
	ptrW data(mir_utf8decodeW(url.c_str()));

	// Check if there is user defined browser for opening links
	ptrW browser(getWStringA(FACEBOOK_KEY_OPEN_URL_BROWSER));
	if (browser != NULL)
		// If so, use it to open this link
		ForkThread(&FacebookProto::OpenUrlThread, new open_url(browser, data));
	else
		// Or use Miranda's service
		Utils_OpenUrlW(data);
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

	HttpRequest *request = new MarkNotificationReadRequest(&facy, id->c_str());
	facy.sendRequest(request);

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

	wchar_t desc[256];
	char name[256];

	// Client
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Client errors"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Client");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("facebook"));
	ppc.colorBack = RGB(191, 0, 0); // red
	ppc.colorText = RGB(255, 255, 255); // white
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Newsfeeds
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Wall posts"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Newsfeed");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("newsfeed"));
	ppc.colorBack = RGB(255, 255, 255); // white
	ppc.colorText = RGB(0, 0, 0); // black
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Notifications
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Notifications"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Notification");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("notification"));
	ppc.colorBack = RGB(59, 89, 152); // Facebook's blue
	ppc.colorText = RGB(255, 255, 255); // white
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Others
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Other events"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Other");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("facebook"));
	ppc.colorBack = RGB(255, 255, 255); // white
	ppc.colorText = RGB(0, 0, 0); // black
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Friendship changes
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Friendship events"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Friendship");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("friendship"));
	ppc.colorBack = RGB(47, 71, 122); // Facebook's darker blue
	ppc.colorText = RGB(255, 255, 255); // white
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// Ticker
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Real-time friends activity"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Ticker");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("newsfeed"));
	ppc.colorBack = RGB(255, 255, 255); // white
	ppc.colorText = RGB(0, 0, 0); // black
	ppc.iSeconds = 0;
	popupClasses.push_back(Popup_RegisterClass(&ppc));

	// On this day (memories)
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Memories"));
	mir_snprintf(name, "%s_%s", m_szModuleName, "Memories");
	ppc.pwszDescription = desc;
	ppc.pszName = name;
	ppc.hIcon = IcoLib_GetIconByHandle(GetIconHandle("memories"));
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
	char *tDest = text + mir_strlen(text);

	HOTKEYDESC hkd = {};
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.szSection.w = m_tszUserName;
	hkd.dwFlags = HKD_UNICODE;

	mir_strcpy(tDest, "/VisitProfile");
	hkd.szDescription.w = LPGENW("Visit profile");
	Hotkey_Register(&hkd);

	mir_strcpy(tDest, "/VisitNotifications");
	hkd.szDescription.w = LPGENW("Visit notifications");
	Hotkey_Register(&hkd);

	mir_strcpy(tDest, "/Mind");
	hkd.szDescription.w = LPGENW("Show 'Share status' window");
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_EXT, 'F');
	Hotkey_Register(&hkd);
}

/**
 * Sounds initialization
 */
void FacebookProto::InitSounds()
{
	Skin_AddSound("Notification", m_tszUserName, LPGENW("Notification"));
	Skin_AddSound("NewsFeed", m_tszUserName, LPGENW("Newsfeed event"));
	Skin_AddSound("OtherEvent", m_tszUserName, LPGENW("Other event"));
	Skin_AddSound("Friendship", m_tszUserName, LPGENW("Friendship event"));
	Skin_AddSound("Ticker", m_tszUserName, LPGENW("Ticker event"));
	Skin_AddSound("Memories", m_tszUserName, LPGENW("Memories"));
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

	wchar_t ttime[64];
	wcsftime(ttime, _countof(ttime), L"%X", localtime(&time));

	HICON hIcon = IcoLib_GetIconByHandle(GetIconHandle("read"));

	if (isChatRoom(hContact)) {
		// Load readers names
		ptrW treaders(getWStringA(hContact, FACEBOOK_KEY_MESSAGE_READERS));
		Srmm_SetStatusText(hContact, CMStringW(FORMAT, TranslateT("Message read: %s by %s"), ttime, treaders ? treaders : L"???"), hIcon);
	}
	else if (!ServiceExists(MS_MESSAGESTATE_UPDATE))
		Srmm_SetStatusText(hContact, CMStringW(FORMAT, TranslateT("Message read: %s"), ttime), hIcon);
}
