#include "StdAfx.h"
#include "sametime.h"

void CloseMyConference(CSametimeProto* proto)
{
	mwConference_destroy(proto->my_conference, 0, Translate("I'm outa here."));
	proto->my_conference = 0;
}

CSametimeProto* getProtoFromMwConference(mwConference* conf)
{
	mwServiceConference* servConference = mwConference_getServiceConference(conf);
	mwService* service = mwServiceConference_getService(servConference);
	mwSession* session = mwService_getSession(service);
	return (CSametimeProto*)mwSession_getProperty(session, "PROTO_STRUCT_PTR");
}

/** triggered when we receive a conference invitation. Call
  mwConference_accept to accept the invitation and join the
  conference, or mwConference_close to reject the invitation.

  @param conf     the newly created conference
  @param inviter  the indentity of the user who sent the invitation
  @param invite   the invitation text
*/

void mwServiceConf_on_invited(mwConference* conf, mwLoginInfo* inviter, const char* invite)
{
	GList *members, *mem;
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_on_invited() start"));

	members = mem = mwConference_getMembers(conf);
	for (;mem;mem=mem->next) {
		if (proto->my_login_info && strcmp(proto->my_login_info->login_id, ((mwLoginInfo*)mem->data)->login_id) == 0) {
			proto->debugLog(_T("mwServiceConf_on_invited() already present"));
			char* utfs = mir_utf8encodeT(TranslateT("Invitation rejected - already present."));
			mwConference_reject(conf, 0, utfs);
			mir_free(utfs);
			return;
		}
	}
	g_list_free(members);

	wchar_t ws_username[128];
	MultiByteToWideChar(CP_UTF8, 0, (const char*)inviter->user_name, -1, ws_username, 128);

	wchar_t ws_invite[512];
	MultiByteToWideChar(CP_UTF8, 0, (const char*)invite, -1, ws_invite, 128);

	if (MessageBoxW(0, ws_invite, ws_username, MB_OKCANCEL) == IDOK) {
		proto->debugLog(_T("mwServiceConf_on_invited() mwConference_accept"));
		mwConference_accept(conf);
	}
	else {
		proto->debugLog(_T("mwServiceConf_on_invited() mwConference_reject"));
		char* temp = mir_utf8encodeT(TranslateT("Your invitation has been rejected."));
		mwConference_reject(conf, 0, temp);
		mir_free(temp);
	}
}

void CSametimeProto::ClearInviteQueue()
{	
	debugLog(_T("CSametimeProto::ClearInviteQueue() start"));
	if (!my_conference)
		return;

	mwIdBlock idb;
	idb.community = 0;

	while(invite_queue.size()) {
		idb.user = (char *)invite_queue.front().c_str();

		MCONTACT hContact = FindContactByUserId(idb.user);
		if (!hContact) {
			mwSametimeList* user_list = mwSametimeList_new();
			char* utfs = mir_utf8encodeT(TranslateT("None"));
			mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, utfs);
			mwSametimeUser* stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &idb);

			hContact = AddContact(stuser, (options.add_contacts ? false : true));
			mwSametimeList_free(user_list);
			mir_free(utfs);

		}

		bool found = false;
		GList *members, *mem;
		members = mem = mwConference_getMembers(my_conference);
		for (;mem;mem=mem->next) {
			if (my_login_info && strcmp(idb.user, ((mwLoginInfo *)mem->data)->user_id) == 0) {
				found = true;
				break;
			}
		}
		g_list_free(members);

		if (!found) {
			char* temp = mir_utf8encodeT(TranslateT("Please join this meeting."));
			mwConference_invite(my_conference, &idb, temp);
			mir_free(temp);
		}

		invite_queue.pop();
	}
}

/** triggered when we enter the conference. Provides the initial
  conference membership list as a GList of mwLoginInfo structures

  @param conf     the conference just joined
  @param members  mwLoginInfo list of existing conference members
*/
void mwServiceConf_conf_opened(mwConference* conf, GList* members) 
{	
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_conf_opened() start"));

	TCHAR* tszConfId = mir_utf8decodeT(mwConference_getName(conf));
	TCHAR* tszConfTitle = mir_utf8decodeT(mwConference_getTitle(conf));

	// create new chat session
	GCSESSION gcs = { sizeof(gcs) };
	gcs.dwFlags = 0;
	gcs.iType = GCW_CHATROOM;
	gcs.pszModule = proto->m_szModuleName;
	gcs.ptszID = tszConfId;
	gcs.ptszName = tszConfTitle;
	gcs.dwItemData = 0;

	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcs);
	mir_free(tszConfTitle);

	//add a group
	GCDEST gcd = { proto->m_szModuleName, 0 };
	gcd.iType = GC_EVENT_ADDGROUP;
	gcd.ptszID = tszConfId;

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszStatus = TranslateT("Normal");

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	// add users
	gcd.iType = GC_EVENT_JOIN;

	GList *user = members;
	for (;user; user=user->next) {
		proto->debugLog(_T("mwServiceConf_conf_opened() add user"));

		TCHAR* tszUserName = mir_utf8decodeT(((mwLoginInfo*)user->data)->user_name);
		TCHAR* tszUserId = mir_utf8decodeT(((mwLoginInfo*)user->data)->login_id);
		gce.ptszNick = tszUserName;
		gce.ptszUID = tszUserId;
		gce.bIsMe = (strcmp(((mwLoginInfo*)user->data)->login_id, proto->my_login_info->login_id) == 0);

		CallServiceSync(MS_GC_EVENT, 0, (LPARAM) &gce);

		mir_free(tszUserName);
		mir_free(tszUserId);
	}

	// finalize setup (show window)
	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

	if (conf == proto->my_conference)
		proto->ClearInviteQueue();

	mir_free(tszConfId);
}

/** triggered when a conference is closed. This is typically when
  we've left it */
void mwServiceConf_conf_closed(mwConference* conf, guint32 reason)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_conf_closed() start"));

	TCHAR* tszConfId = mir_utf8decodeT(mwConference_getName(conf));

	GCDEST gcd = { proto->m_szModuleName };
	gcd.ptszID = tszConfId;
	gcd.iType = GC_EVENT_CONTROL;

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;

	CallService(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	CallService(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
	mir_free(tszConfId);
}

/** triggered when someone joins the conference */
void mwServiceConf_on_peer_joined(mwConference* conf, mwLoginInfo *user)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_on_peer_joined() start"));

	MCONTACT hContact = proto->FindContactByUserId(user->user_id);
	if (!hContact) {
		mwIdBlock idb;
		idb.user = ((mwLoginInfo *)user)->user_id;
		idb.community = 0;

		mwSametimeList* user_list = mwSametimeList_new();
		char* utfs = mir_utf8encodeT(TranslateT("None"));
		mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, utfs);
		mwSametimeUser* stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &idb);

		hContact = proto->AddContact(stuser, (proto->options.add_contacts ? false : true));

		mwSametimeList_free(user_list);
		mir_free(utfs);
	}

	ptrT tszConfId(mir_utf8decodeT(mwConference_getName(conf)));
	ptrT tszUserName(mir_utf8decodeT(user->user_name));
	ptrT tszUserId(mir_utf8decodeT(user->login_id));

	// add user
	GCDEST gcd = { proto->m_szModuleName };
	gcd.ptszID = tszConfId;
	gcd.iType = GC_EVENT_JOIN;

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = tszUserName;
	gce.ptszUID = tszUserId;
	gce.ptszStatus = _T("Normal");
	gce.time = (DWORD)time(0);

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM) &gce);

	mir_free(tszUserName);
	mir_free(tszUserId);
	mir_free(tszConfId);
}

/** triggered when someone leaves the conference */
void mwServiceConf_on_peer_parted(mwConference* conf, mwLoginInfo* user)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_on_peer_parted() start"));

	ptrT tszConfId(mir_utf8decodeT(mwConference_getName(conf)));
	ptrT tszUserName(mir_utf8decodeT(user->user_name));
	ptrT tszUserId(mir_utf8decodeT(user->login_id));

	// remove user
	GCDEST gcd = { proto->m_szModuleName };
	gcd.ptszID = tszConfId;
	gcd.iType = GC_EVENT_PART;

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = tszUserName;
	gce.ptszUID = tszUserId;
	gce.ptszStatus = _T("Normal");
	gce.time = (DWORD)time(0);
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
}

/** triggered when someone says something */
void mwServiceConf_on_text(mwConference* conf, mwLoginInfo* user, const char* what)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_on_text() start"));

	TCHAR* tszConfId = mir_utf8decodeT(mwConference_getName(conf));

	GCDEST gcd = { proto->m_szModuleName };
	gcd.ptszID = tszConfId;
	gcd.iType = GC_EVENT_MESSAGE;

	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;

	TCHAR* textT = mir_utf8decodeT(what);
	TCHAR* tszUserName = mir_utf8decodeT(user->user_name);
	TCHAR* tszUserId = mir_utf8decodeT(user->login_id);
	gce.ptszText = textT;
	gce.ptszNick = tszUserName;
	gce.ptszUID = tszUserId;
	gce.time = (DWORD)time(0);

	CallService(MS_GC_EVENT, 0, (LPARAM)(GCEVENT *) &gce);

	mir_free(textT);
	mir_free(tszUserName);
	mir_free(tszUserId);
	mir_free(tszConfId);
}

/** typing notification */
void mwServiceConf_on_typing(mwConference* conf, mwLoginInfo* who, gboolean typing)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLog(_T("mwServiceConf_on_typing() start"));
	///TODO unimplemented
}

/** optional. called from mwService_free */
void mwServiceConf_clear(mwServiceConference* srvc)
{
}

mwConferenceHandler mwConference_handler = {
	mwServiceConf_on_invited,
	mwServiceConf_conf_opened,
	mwServiceConf_conf_closed,
	mwServiceConf_on_peer_joined,
	mwServiceConf_on_peer_parted,
	mwServiceConf_on_text,
	mwServiceConf_on_typing,
	mwServiceConf_clear
};

void CSametimeProto::TerminateConference(char* name)
{
	debugLog(_T("CSametimeProto::TerminateConference() start"));
		
	GList *conferences, *conf;
	conferences = conf = mwServiceConference_getConferences(service_conference);
	for (;conf;conf = conf->next) {
		if (strcmp(name, mwConference_getName((mwConference*)conf->data)) == 0) {

			TCHAR* idt = mir_utf8decodeT(name);
			GCDEST gcd = {m_szModuleName, idt, GC_EVENT_CONTROL};

			GCEVENT gce = { sizeof(gce), &gcd };
			gce.dwFlags = GCEF_ADDTOLOG;
			CallService(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
			
			mir_free(idt);
		}
	}
	g_list_free(conferences);
}


int CSametimeProto::GcEventHook(WPARAM wParam, LPARAM lParam) {

	GCHOOK* gch = (GCHOOK*)lParam;

	if (strcmp(gch->pDest->pszModule, m_szModuleName) != 0) return 0;

	GList *conferences, *conf;
	conferences = conf = mwServiceConference_getConferences(service_conference);
	for (;conf;conf = conf->next) {
		TCHAR* tszConfId = mir_utf8decodeT(mwConference_getName((mwConference*)conf->data));
		if (_tcscmp(gch->pDest->ptszID, tszConfId) == 0) {
			
			switch(gch->pDest->iType) {
			case GC_USER_MESSAGE:
				{
					debugLog(_T("CSametimeProto::GcEventHook() GC_USER_MESSAGE"));
					char* utf_msg;
					utf_msg = mir_utf8encodeT(gch->ptszText);
					mwConference_sendText((mwConference*)conf->data, utf_msg);
					mir_free(utf_msg);
				}
				break;
			case GC_SESSION_TERMINATE:
				{
					if (my_conference == conf->data){
						debugLog(_T("CSametimeProto::GcEventHook() GC_SESSION_TERMINATE CloseMyConference"));
						CloseMyConference(this);
					} else {
						debugLog(_T("CSametimeProto::GcEventHook() GC_SESSION_TERMINATE mwConference_destroy"));
						char* utfs = mir_utf8encodeT(TranslateT("I'm outa here."));
						mwConference_destroy((mwConference*)conf->data, 0, utfs);
						mir_free(utfs);
					}
				}
				break;
			}

			break;
		}
		mir_free(tszConfId);
	}

	g_list_free(conferences);

	return 0;
}

int CSametimeProto::ChatDeleted(MCONTACT hContact) {
	
	if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0) == 0)
		return 0;

	debugLog(_T("CSametimeProto::ChatDeleted() hContact=[%x]"), hContact);
	DBVARIANT dbv;
	if (!db_get_s(hContact, m_szModuleName, "ChatRoomID", &dbv)) {
		TerminateConference(dbv.pszVal);
		db_free(&dbv);
	}

	return 0;
}


INT_PTR CSametimeProto::onMenuLeaveChat(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	debugLog(_T("CSametimeProto::onMenuLeaveChat() hContact=[%x]"), hContact);
	ChatDeleted(hContact);
	return 0;
}


INT_PTR CSametimeProto::onMenuCreateChat(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	debugLog(_T("CSametimeProto::onMenuCreateChat() hContact=[%x]"), hContact);
	mwAwareIdBlock id_block;
	mwIdBlock idb;
	if (my_login_info && GetAwareIdFromContact(hContact, &id_block)) {
		TCHAR title[512];
		TCHAR* ts = mir_utf8decodeT(my_login_info->user_name);
		mir_sntprintf(title, SIZEOF(title), TranslateT("%s's conference"), ts);
		mir_free(ts);

		idb.user = id_block.user;
		idb.community = id_block.community;

		invite_queue.push(idb.user);

		if (!my_conference) {
			debugLog(_T("CSametimeProto::onMenuCreateChat() mwConference_open"));
			char* utfs;
			my_conference = mwConference_new(service_conference, utfs = mir_utf8encodeT(title));
			mwConference_open(my_conference);
			mir_free(utfs);
		} else {
			debugLog(_T("CSametimeProto::onMenuCreateChat() ClearInviteQueue"));
			ClearInviteQueue();
		}

		free(id_block.user);
	}

	return 0;
}

int CSametimeProto::PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;
	debugLog(_T("CSametimeProto::PrebuildContactMenu() hContact=[%x]"), hContact);
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (db_get_b(hContact, m_szModuleName, "ChatRoom", 0) == 1 ? 0 : CMIF_HIDDEN);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hLeaveChatMenuItem, (LPARAM)&mi);

	// if user is already in our meeting, 
	bool not_present = true;
	DBVARIANT dbv;
	if (my_conference && !db_get_utf(hContact, m_szModuleName, "stid", &dbv)) {
		char* user_id = dbv.pszVal;

		GList *members, *mem;
		members = mem = mwConference_getMembers(my_conference);
		for (;mem;mem=mem->next) {
			if (my_login_info && strcmp(user_id, ((mwLoginInfo *)mem->data)->user_id) == 0) {
				not_present = false;
				break;
			}
		}
		g_list_free(members);

		db_free(&dbv);
	}
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | (db_get_b(hContact, m_szModuleName, "ChatRoom", 0) == 0 && not_present ? 0 : CMIF_HIDDEN);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hCreateChatMenuItem, (LPARAM)&mi);

	return 0;
}

void CSametimeProto::InitConference()
{
	debugLog(_T("CSametimeProto::InitConference()"));

	my_login_info = mwSession_getLoginInfo(session);

	service_conference = mwServiceConference_new(session, &mwConference_handler);
	mwSession_addService(session, (struct mwService*)service_conference);

	HookProtoEvent(ME_GC_EVENT, &CSametimeProto::GcEventHook);
}

void CSametimeProto::DeinitConference()
{
	GList *conferences, *conf;
	debugLog(_T("CSametimeProto::DeinitConference()"));

	if (service_conference){
		conferences = conf = mwServiceConference_getConferences(service_conference);
		for (;conf;conf = conf->next) {
			if (my_conference == conf->data) CloseMyConference(this);
			else {
				char* utfs = mir_utf8encodeT(TranslateT("I'm outa here."));
				mwConference_destroy((mwConference*)conf->data, 0, utfs);
				mir_free(utfs);
			}
		}
		g_list_free(conferences);
	}

	my_login_info = 0;

	mwSession_removeService(session, mwService_CONFERENCE);
	if (service_conference){
		mwService_free((mwService*)service_conference);
		service_conference = 0;
	}
}

void CSametimeProto::InitConferenceMenu()
{
	debugLog(_T("CSametimeProto::InitConferenceMenu()"));

	CreateProtoService(MS_SAMETIME_MENULEAVECHAT, &CSametimeProto::onMenuLeaveChat);
	CreateProtoService(MS_SAMETIME_MENUCREATECHAT, &CSametimeProto::onMenuCreateChat);

	char service[128];

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR | CMIF_NOTOFFLINE;
	mi.pszContactOwner = m_szModuleName;
	
	mi.ptszName = LPGENT("Leave conference");
	mir_snprintf(service, SIZEOF(service), "%s%s", m_szModuleName, MS_SAMETIME_MENULEAVECHAT);
	mi.pszService = service;
	mi.icolibItem = GetIconHandle(IDI_ICON_LEAVE);
	hLeaveChatMenuItem = Menu_AddContactMenuItem(&mi);

	mi.ptszName = LPGENT("Start conference");
	mir_snprintf(service, SIZEOF(service), "%s%s", m_szModuleName, MS_SAMETIME_MENUCREATECHAT);
	mi.pszService = service;
	mi.icolibItem = GetIconHandle(IDI_ICON_INVITE);
	hCreateChatMenuItem = Menu_AddContactMenuItem(&mi);

	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSametimeProto::PrebuildContactMenu);
}

void CSametimeProto::DeinitConferenceMenu()
{
	debugLog(_T("CSametimeProto::DeinitConferenceMenu()"));
	CallService(MO_REMOVEMENUITEM, (WPARAM)hLeaveChatMenuItem, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)hCreateChatMenuItem, 0);
}

