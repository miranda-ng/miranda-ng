#include "StdAfx.h"
#include "sametime.h"

void CloseMyConference(CSametimeProto* proto)
{
	mwConference_destroy(proto->my_conference, 0, Translate("I'm outa here."));
	proto->my_conference = nullptr;
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
  @param inviter  the identity of the user who sent the invitation
  @param invite   the invitation text
*/
void mwServiceConf_on_invited(mwConference* conf, mwLoginInfo* inviter, const char* invite)
{
	GList *members, *mem;
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLogW(L"mwServiceConf_on_invited() start");

	members = mem = mwConference_getMembers(conf);
	for (;mem;mem=mem->next) {
		if (proto->my_login_info && strcmp(proto->my_login_info->login_id, ((mwLoginInfo*)mem->data)->login_id) == 0) {
			proto->debugLogW(L"mwServiceConf_on_invited() already present");
			mwConference_reject(conf, 0, T2Utf(TranslateT("Invitation rejected - already present.")));
			return;
		}
	}
	g_list_free(members);

	wchar_t ws_username[128];
	MultiByteToWideChar(CP_UTF8, 0, (const char*)inviter->user_name, -1, ws_username, 128);

	wchar_t ws_invite[512];
	MultiByteToWideChar(CP_UTF8, 0, (const char*)invite, -1, ws_invite, 128);

	if (MessageBoxW(nullptr, ws_invite, ws_username, MB_OKCANCEL) == IDOK) {
		proto->debugLogW(L"mwServiceConf_on_invited() mwConference_accept");
		mwConference_accept(conf);
	}
	else {
		proto->debugLogW(L"mwServiceConf_on_invited() mwConference_reject");
		char* temp = mir_utf8encodeW(TranslateT("Your invitation has been rejected."));
		mwConference_reject(conf, 0, temp);
		mir_free(temp);
	}
}

void CSametimeProto::ClearInviteQueue()
{	
	debugLogW(L"CSametimeProto::ClearInviteQueue() start");
	if (!my_conference)
		return;

	mwIdBlock idb;
	idb.community = nullptr;

	while(invite_queue.size()) {
		idb.user = (char *)invite_queue.front().c_str();

		MCONTACT hContact = FindContactByUserId(idb.user);
		if (!hContact) {
			mwSametimeList* user_list = mwSametimeList_new();
			mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, T2Utf(TranslateT("None")));
			mwSametimeUser* stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &idb);

			hContact = AddContact(stuser, (options.add_contacts ? false : true));
			mwSametimeList_free(user_list);
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

		if (!found)
			mwConference_invite(my_conference, &idb, T2Utf(TranslateT("Please join this meeting.")));

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
	proto->debugLogW(L"mwServiceConf_conf_opened() start");

	ptrW tszConfId(mir_utf8decodeW(mwConference_getName(conf)));
	ptrW tszConfTitle(mir_utf8decodeW(mwConference_getTitle(conf)));

	// create new chat session
	SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, proto->m_szModuleName, tszConfId, tszConfTitle);
	if (!si)
		return;

	// add a group
	Chat_AddGroup(si, TranslateT("Normal"));

	// add users
	GCEVENT gce = { proto->m_szModuleName, 0, GC_EVENT_JOIN };
	gce.pszID.w = tszConfId;

	GList *user = members;
	for (;user; user=user->next) {
		proto->debugLogW(L"mwServiceConf_conf_opened() add user");

		ptrW tszUserName(mir_utf8decodeW(((mwLoginInfo*)user->data)->user_name));
		ptrW tszUserId(mir_utf8decodeW(((mwLoginInfo*)user->data)->login_id));
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.pszNick.w = tszUserName;
		gce.pszUID.w = tszUserId;
		gce.bIsMe = (strcmp(((mwLoginInfo*)user->data)->login_id, proto->my_login_info->login_id) == 0);
		Chat_Event( &gce);
	}

	// finalize setup (show window)
	Chat_Control(proto->m_szModuleName, tszConfId, SESSION_INITDONE);
	Chat_Control(proto->m_szModuleName, tszConfId, SESSION_ONLINE);

	if (conf == proto->my_conference)
		proto->ClearInviteQueue();
}

/** triggered when a conference is closed. This is typically when
  we've left it */
void mwServiceConf_conf_closed(mwConference* conf, guint32)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLogW(L"mwServiceConf_conf_closed() start");

	ptrW tszConfId(mir_utf8decodeW(mwConference_getName(conf)));
	Chat_Control(proto->m_szModuleName, tszConfId, SESSION_OFFLINE);
	Chat_Terminate(proto->m_szModuleName, tszConfId);
}

/** triggered when someone joins the conference */
void mwServiceConf_on_peer_joined(mwConference* conf, mwLoginInfo *user)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLogW(L"mwServiceConf_on_peer_joined() start");

	MCONTACT hContact = proto->FindContactByUserId(user->user_id);
	if (!hContact) {
		mwIdBlock idb;
		idb.user = ((mwLoginInfo *)user)->user_id;
		idb.community = nullptr;

		mwSametimeList* user_list = mwSametimeList_new();
		mwSametimeGroup* stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, T2Utf(TranslateT("None")));
		mwSametimeUser* stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &idb);

		hContact = proto->AddContact(stuser, (proto->options.add_contacts ? false : true));

		mwSametimeList_free(user_list);
	}

	ptrW tszConfId(mir_utf8decodeW(mwConference_getName(conf)));
	ptrW tszUserName(mir_utf8decodeW(user->user_name));
	ptrW tszUserId(mir_utf8decodeW(user->login_id));

	// add user
	GCEVENT gce = { proto->m_szModuleName, 0, GC_EVENT_JOIN };
	gce.pszID.w = tszConfId;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszNick.w = tszUserName;
	gce.pszUID.w = tszUserId;
	gce.pszStatus.w = L"Normal";
	gce.time = (uint32_t)time(0);

	Chat_Event( &gce);

	mir_free(tszUserName);
	mir_free(tszUserId);
	mir_free(tszConfId);
}

/** triggered when someone leaves the conference */
void mwServiceConf_on_peer_parted(mwConference* conf, mwLoginInfo* user)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLogW(L"mwServiceConf_on_peer_parted() start");

	ptrW tszConfId(mir_utf8decodeW(mwConference_getName(conf)));
	ptrW tszUserName(mir_utf8decodeW(user->user_name));
	ptrW tszUserId(mir_utf8decodeW(user->login_id));

	// remove user
	GCEVENT gce = { proto->m_szModuleName, 0, GC_EVENT_PART };
	gce.pszID.w = tszConfId;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszNick.w = tszUserName;
	gce.pszUID.w = tszUserId;
	gce.pszStatus.w = L"Normal";
	gce.time = (uint32_t)time(0);
	Chat_Event(&gce);
}

/** triggered when someone says something */
void mwServiceConf_on_text(mwConference* conf, mwLoginInfo* user, const char* what)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLogW(L"mwServiceConf_on_text() start");

	ptrW textT(mir_utf8decodeW(what));
	ptrW tszConfId(mir_utf8decodeW(mwConference_getName(conf)));
	ptrW tszUserId(mir_utf8decodeW(user->login_id));
	ptrW tszUserName(mir_utf8decodeW(user->user_name));

	GCEVENT gce = { proto->m_szModuleName, 0, GC_EVENT_MESSAGE };
	gce.pszID.w = tszConfId;
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.pszText.w = textT;
	gce.pszNick.w = tszUserName;
	gce.pszUID.w = tszUserId;
	gce.time = (uint32_t)time(0);
	Chat_Event(&gce);
}

/** typing notification */
void mwServiceConf_on_typing(mwConference* conf, mwLoginInfo*, gboolean)
{
	CSametimeProto* proto = getProtoFromMwConference(conf);
	proto->debugLogW(L"mwServiceConf_on_typing() start");
	///TODO unimplemented
}

/** optional. called from mwService_free */
void mwServiceConf_clear(mwServiceConference*)
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
	debugLogW(L"CSametimeProto::TerminateConference() start");
		
	GList *conferences, *conf;
	conferences = conf = mwServiceConference_getConferences(service_conference);
	for (;conf;conf = conf->next)
		if (strcmp(name, mwConference_getName((mwConference*)conf->data)) == 0)
			Chat_Terminate(m_szModuleName, ptrW(mir_utf8decodeW(name)));

	g_list_free(conferences);
}


int CSametimeProto::GcEventHook(WPARAM, LPARAM lParam) {

	GCHOOK* gch = (GCHOOK*)lParam;
	if (strcmp(gch->si->pszModule, m_szModuleName) != 0)
		return 0;

	GList *conferences = mwServiceConference_getConferences(service_conference);
	for (GList *conf = conferences;conf;conf = conf->next) {
		wchar_t* tszConfId = mir_utf8decodeW(mwConference_getName((mwConference*)conf->data));
		if (mir_wstrcmp(gch->si->ptszID, tszConfId) == 0) {
			switch(gch->iType) {
			case GC_USER_MESSAGE:
				{
					debugLogW(L"CSametimeProto::GcEventHook() GC_USER_MESSAGE");
					mwConference_sendText((mwConference*)conf->data, T2Utf(gch->ptszText));
				}
				break;
			case GC_SESSION_TERMINATE:
				{
					if (my_conference == conf->data){
						debugLogW(L"CSametimeProto::GcEventHook() GC_SESSION_TERMINATE CloseMyConference");
						CloseMyConference(this);
					} else {
						debugLogW(L"CSametimeProto::GcEventHook() GC_SESSION_TERMINATE mwConference_destroy");
						mwConference_destroy((mwConference*)conf->data, 0, T2Utf(TranslateT("I'm outa here.")));
					}
				}
				break;
			}
			mir_free(tszConfId);
			break;
		}
		mir_free(tszConfId);
	}

	g_list_free(conferences);
	return 1;
}

int CSametimeProto::ChatDeleted(MCONTACT hContact) {
	
	if (!Contact::IsGroupChat(hContact, m_szModuleName))
		return 0;

	debugLogW(L"CSametimeProto::ChatDeleted() hContact=[%x]", hContact);
	DBVARIANT dbv;
	if (!db_get_s(hContact, m_szModuleName, "ChatRoomID", &dbv)) {
		TerminateConference(dbv.pszVal);
		db_free(&dbv);
	}

	return 0;
}


INT_PTR CSametimeProto::onMenuLeaveChat(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	debugLogW(L"CSametimeProto::onMenuLeaveChat() hContact=[%x]", hContact);
	ChatDeleted(hContact);
	return 0;
}


INT_PTR CSametimeProto::onMenuCreateChat(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	debugLogW(L"CSametimeProto::onMenuCreateChat() hContact=[%x]", hContact);
	mwAwareIdBlock id_block;
	mwIdBlock idb;
	if (my_login_info && GetAwareIdFromContact(hContact, &id_block)) {
		wchar_t title[512];
		wchar_t* ts = mir_utf8decodeW(my_login_info->user_name);
		mir_snwprintf(title, TranslateT("%s's conference"), ts);
		mir_free(ts);

		idb.user = id_block.user;
		idb.community = id_block.community;

		invite_queue.push(idb.user);

		if (!my_conference) {
			debugLogW(L"CSametimeProto::onMenuCreateChat() mwConference_open");
			my_conference = mwConference_new(service_conference, T2Utf(title));
			mwConference_open(my_conference);
		}
		else {
			debugLogW(L"CSametimeProto::onMenuCreateChat() ClearInviteQueue");
			ClearInviteQueue();
		}

		free(id_block.user);
	}

	return 0;
}

int CSametimeProto::PrebuildContactMenu(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	debugLogW(L"CSametimeProto::PrebuildContactMenu() hContact=[%x]", hContact);

	Menu_ShowItem(hLeaveChatMenuItem, Contact::IsGroupChat(hContact, m_szModuleName));

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

	Menu_ShowItem(hCreateChatMenuItem, !Contact::IsGroupChat(hContact, m_szModuleName) && not_present);
	return 0;
}

void CSametimeProto::InitConference()
{
	debugLogW(L"CSametimeProto::InitConference()");

	my_login_info = mwSession_getLoginInfo(session);

	service_conference = mwServiceConference_new(session, &mwConference_handler);
	mwSession_addService(session, (struct mwService*)service_conference);

	HookProtoEvent(ME_GC_EVENT, &CSametimeProto::GcEventHook);
}

void CSametimeProto::DeinitConference()
{
	GList *conferences, *conf;
	debugLogW(L"CSametimeProto::DeinitConference()");

	if (service_conference){
		conferences = conf = mwServiceConference_getConferences(service_conference);
		for (;conf;conf = conf->next) {
			if (my_conference == conf->data)
				CloseMyConference(this);
			else
				mwConference_destroy((mwConference*)conf->data, 0, T2Utf(TranslateT("I'm outa here.")));
		}
		g_list_free(conferences);
	}

	my_login_info = nullptr;

	mwSession_removeService(session, mwService_CONFERENCE);
	if (service_conference){
		mwService_free((mwService*)service_conference);
		service_conference = nullptr;
	}
}

void CSametimeProto::InitConferenceMenu()
{
	debugLogW(L"CSametimeProto::InitConferenceMenu()");

	CreateProtoService(MS_SAMETIME_MENULEAVECHAT, &CSametimeProto::onMenuLeaveChat);
	CreateProtoService(MS_SAMETIME_MENUCREATECHAT, &CSametimeProto::onMenuCreateChat);

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNICODE | CMIF_NOTOFFLINE;
	
	SET_UID(mi, 0x98cf8a8c, 0x75ba, 0x46f2, 0xa3, 0x35, 0x65, 0x46, 0x4a, 0x38, 0x20, 0x7d);
	mi.name.w = LPGENW("Leave conference");
	mi.pszService = MS_SAMETIME_MENULEAVECHAT;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_LEAVE);
	hLeaveChatMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);

	SET_UID(mi, 0x45501e10, 0x2914, 0x4daa, 0xb4, 0xcf, 0x83, 0x8a, 0x6a, 0x14, 0xd, 0x7);
	mi.name.w = LPGENW("Start conference");
	mi.pszService = MS_SAMETIME_MENUCREATECHAT;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_ICON_INVITE);
	hCreateChatMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);

	HookProtoEvent(ME_CLIST_PREBUILDCONTACTMENU, &CSametimeProto::PrebuildContactMenu);
}
