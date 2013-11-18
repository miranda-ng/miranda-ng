#include "conference.h"

typedef std::queue<std::string> InviteQueue;

InviteQueue invite_queue;

mwServiceConference *service_conference = 0;

HANDLE hGcEvent = 0; 

mwLoginInfo *my_login_info = 0; 

mwConference *my_conference = 0;

HANDLE hChatDeletedEvent = 0, hMenuBuildEvent = 0;

HANDLE hLeaveChatMenuItem = 0, hCreateChatMenuItem = 0;

HICON hIconLeave = 0, hIconCreate = 0;

#define MS_SAMETIME_MENULEAVECHAT		"/LeaveChat"
#define MS_SAMETIME_MENUCREATECHAT		"/CreateChat"

void CloseMyConference() {
	mwConference_destroy(my_conference, 0, Translate("I'm outa here."));
	my_conference = 0;
}

/** triggered when we receive a conference invitation. Call
  mwConference_accept to accept the invitation and join the
  conference, or mwConference_close to reject the invitation.

  @param conf     the newly created conference
  @param inviter  the indentity of the user who sent the invitation
  @param invite   the invitation text
*/
void mwServiceConf_on_invited(mwConference *conf, mwLoginInfo *inviter, const char *invite) {
	GList *members, *mem;
	members = mem = mwConference_getMembers(conf);
	for(;mem;mem=mem->next) {
		if(my_login_info && strcmp(my_login_info->login_id, ((mwLoginInfo *)mem->data)->login_id) == 0) {
			char *utfs = t2u(TranslateT("Invitation rejected - already present."));
			mwConference_reject(conf, 0, utfs);
			free(utfs);
			return;
		}
	}
	g_list_free(members);

	wchar_t ws_username[128];
	MultiByteToWideChar(CP_UTF8, 0, (const char *)inviter->user_name, -1, ws_username, 128);

	wchar_t ws_invite[512];
	MultiByteToWideChar(CP_UTF8, 0, (const char *)invite, -1, ws_invite, 128);

	if(MessageBoxW(0, ws_invite, ws_username, MB_OKCANCEL) == IDOK) {
		mwConference_accept(conf);
	} else {
		char *temp = t2u(TranslateT("Your invitation has been rejected."));
		mwConference_reject(conf, 0, temp);
		free(temp);
	}

}

void ClearInviteQueue() {
	if(!my_conference) return;

	mwIdBlock idb;
	idb.community = 0;

	while(invite_queue.size()) {
		idb.user = (char *)invite_queue.front().c_str();

		HANDLE hContact = FindContactByUserId(idb.user);
		if(!hContact) {
			mwSametimeList *user_list = mwSametimeList_new();
			char *utfs = t2u(TranslateT("None"));
			mwSametimeGroup *stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, utfs);
			mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &idb);

			hContact = AddContact(stuser, (options.add_contacts ? false : true));
			mwSametimeList_free(user_list);
			free(utfs);

		}

		bool found = false;
		GList *members, *mem;
		members = mem = mwConference_getMembers(my_conference);
		for(;mem;mem=mem->next) {
			if(my_login_info && strcmp(idb.user, ((mwLoginInfo *)mem->data)->user_id) == 0) {
				found = true;
				break;
			}
		}
		g_list_free(members);

		if(!found) {
			char *temp = t2u(TranslateT("Please join this meeting."));
			mwConference_invite(my_conference, &idb, temp);
			free(temp);
		}

		invite_queue.pop();
	}
}
/** triggered when we enter the conference. Provides the initial
  conference membership list as a GList of mwLoginInfo structures

  @param conf     the conference just joined
  @param members  mwLoginInfo list of existing conference members
*/
void mwServiceConf_conf_opened(mwConference *conf, GList *members) {
	//MessageBox(0, "Conference Opened", "msg", MB_OK);
	
	TCHAR *szId = u2t(mwConference_getName(conf));

	// create new chat session
	GCSESSION gcs = {0};
	gcs.dwFlags = GC_TCHAR;
	gcs.cbSize = sizeof(gcs);
	gcs.iType = GCW_CHATROOM;
	gcs.pszModule = PROTO;
	gcs.ptszID = szId;
	gcs.ptszName = u2t(mwConference_getTitle(conf));
	gcs.dwItemData = 0;

	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)(GCSESSION *) &gcs);
	free((char *)gcs.pszName);

	//add a group
	GCDEST gcd = {0};
	gcd.pszModule = PROTO;
	gcd.ptszID = szId;
	gcd.iType = GC_EVENT_ADDGROUP;

	GCEVENT gce = {0};	
	gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
	gce.cbSize = sizeof(gce);
	gce.pDest = &gcd;
	gce.ptszStatus = TranslateT("Normal");

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	// add users
	gcd.iType = GC_EVENT_JOIN;

	GList *user = members;
	for(;user; user=user->next) {

		gce.ptszNick = u2t(((mwLoginInfo *)user->data)->user_name);
		gce.ptszUID = u2t(((mwLoginInfo *)user->data)->login_id);
		gce.bIsMe = (strcmp(((mwLoginInfo *)user->data)->login_id, my_login_info->login_id) == 0);

		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)(GCEVENT *) &gce);

		free((TCHAR *)gce.ptszNick);
		free((TCHAR *)gce.ptszUID);
	}

	// finalize setup (show window)
	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);

	if(conf == my_conference) {
		ClearInviteQueue();
	}

	free(szId);
}

/** triggered when a conference is closed. This is typically when
  we've left it */
void mwServiceConf_conf_closed(mwConference *conf, guint32 reason) {
	//MessageBox(0, "Conference closed", "msg", MB_OK);
	
	TCHAR *szId = u2t(mwConference_getName(conf));

	GCDEST gcd = {0};
	gcd.pszModule = PROTO;
	gcd.ptszID = szId;

	GCEVENT gce = {0};
	gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
	gce.cbSize = sizeof(gce);
	gce.pDest = &gcd;
	gcd.iType = GC_EVENT_CONTROL;

	CallService(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
	free(szId);
}

/** triggered when someone joins the conference */
void mwServiceConf_on_peer_joined(mwConference *conf, mwLoginInfo *user) {
	TCHAR *szId = u2t(mwConference_getName(conf));

	HANDLE hContact = FindContactByUserId(((mwLoginInfo *)user)->user_id);
	if(!hContact) {
		mwIdBlock idb;
		idb.user = ((mwLoginInfo *)user)->user_id;
		idb.community = 0;

		mwSametimeList *user_list = mwSametimeList_new();
		char *utfs = t2u(TranslateT("None"));
		mwSametimeGroup *stgroup = mwSametimeGroup_new(user_list, mwSametimeGroup_NORMAL, utfs);
		mwSametimeUser *stuser = mwSametimeUser_new(stgroup, mwSametimeUser_NORMAL, &idb);

		hContact = AddContact(stuser, (options.add_contacts ? false : true));

		mwSametimeList_free(user_list);
		free(utfs);
	}

	// add user
	GCDEST gcd;
	gcd.pszModule = PROTO;
	gcd.ptszID = szId;
	gcd.iType = GC_EVENT_JOIN;

	GCEVENT gce = {0};
	gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
	gce.cbSize = sizeof(gce);
	gce.pDest = &gcd;
	gce.ptszNick = u2t(((mwLoginInfo *)user)->user_name);
	gce.ptszUID = u2t(((mwLoginInfo *)user)->login_id);
	gce.pszStatus = "Normal";
	gce.time = (DWORD)time(0);

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)(GCEVENT *) &gce);

	free((TCHAR *)gce.ptszNick);
	free((TCHAR *)gce.ptszUID);
	free(szId);
}

/** triggered when someone leaves the conference */
void mwServiceConf_on_peer_parted(mwConference *conf, mwLoginInfo *user) {
	TCHAR *szId = u2t(mwConference_getName(conf));

	// remove user
	GCDEST gcd;
	gcd.pszModule = PROTO;
	gcd.pszID = (char *)szId;
	gcd.iType = GC_EVENT_PART;

	GCEVENT gce = {0};
	gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
	gce.cbSize = sizeof(gce);
	gce.pDest = &gcd;
	gce.ptszNick = u2t(((mwLoginInfo *)user)->user_name);
	gce.ptszUID = u2t(((mwLoginInfo *)user)->login_id);
	gce.pszStatus = "Normal";
	gce.time = (DWORD)time(0);

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)(GCEVENT *) &gce);

	free((TCHAR *)gce.ptszNick);
	free((TCHAR *)gce.ptszUID);
	free(szId);
}

/** triggered when someone says something */
void mwServiceConf_on_text(mwConference *conf, mwLoginInfo *user, const char *what) {
	TCHAR *szId = u2t(mwConference_getName(conf));

	GCDEST gcd;
	gcd.pszModule = PROTO;
	gcd.pszID = (char *)szId;
	gcd.iType = GC_EVENT_MESSAGE;

	GCEVENT gce = {0};
	gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
	gce.cbSize = sizeof(gce);
	gce.pDest = &gcd;


	//char msg[MAX_MESSAGE_SIZE];
	//wchar_t ws_msg[MAX_MESSAGE_SIZE];

	//MultiByteToWideChar(CP_UTF8, 0, what, -1, ws_msg, MAX_MESSAGE_SIZE);
	//WideCharToMultiByte(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), WC_COMPOSITECHECK, ws_msg, -1, msg, MAX_MESSAGE_SIZE * sizeof(char), 0, 0);

	if(unicode_chat)
		gce.ptszText = u2w(what);
	else
		gce.pszText = u2a(what);
	
	gce.ptszNick = u2t(((mwLoginInfo *)user)->user_name);
	gce.ptszUID = u2t(((mwLoginInfo *)user)->login_id);
	gce.time = (DWORD)time(0);

	CallService(MS_GC_EVENT, 0, (LPARAM)(GCEVENT *) &gce);

	free((char *)gce.pszText);
	free((TCHAR *)gce.ptszNick);
	free((TCHAR *)gce.ptszUID);
	free(szId);
}

/** typing notification */
void mwServiceConf_on_typing(mwConference *conf, mwLoginInfo *who, gboolean typing) {
}

/** optional. called from mwService_free */
void mwServiceConf_clear(mwServiceConference *srvc) {
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

void TerminateConference(char *name) {
	//MessageBox(0, name, "Terminating Conference", MB_OK);
		
	GList *conferences, *conf;
	conferences = conf = mwServiceConference_getConferences(service_conference);
	for(;conf;conf = conf->next) {
		if(strcmp(name, mwConference_getName((mwConference *)conf->data)) == 0) {
			GCDEST gcd = {0};
			gcd.pszModule = PROTO;
			gcd.ptszID = u2t(name);

			GCEVENT gce = {0};
			gce.dwFlags = GC_TCHAR | GCEF_ADDTOLOG;
			gce.cbSize = sizeof(gce);
			gce.pDest = &gcd;
			gcd.iType = GC_EVENT_CONTROL;

			CallService(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
			
			free((TCHAR *)gcd.ptszID);
		}
	}
	g_list_free(conferences);
}


int GcEventHook(WPARAM wParam, LPARAM lParam) {
	GCHOOK *gch = (GCHOOK *)lParam;

	if(strcmp(gch->pDest->pszModule, PROTO) != 0) return 0;

	GList *conferences, *conf;
	conferences = conf = mwServiceConference_getConferences(service_conference);
	for(;conf;conf = conf->next) {
		TCHAR *szId = u2t(mwConference_getName((mwConference *)conf->data));
		if(_tcscmp(gch->pDest->ptszID, szId) == 0) {
			
			switch(gch->pDest->iType) {
			case GC_USER_MESSAGE:
				{
					//MessageBox(0, "Sending message", "Sending", MB_OK);
					//wchar_t msg[MAX_MESSAGE_SIZE];
					//char utf_msg[MAX_MESSAGE_SIZE];
					//MultiByteToWideChar(CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), MB_PRECOMPOSED, gch->pszText, -1, msg, MAX_MESSAGE_SIZE);
					//WideCharToMultiByte(CP_UTF8, 0, msg, -1, utf_msg, MAX_MESSAGE_SIZE, 0, 0);

					char *utf_msg;
					if(unicode_chat)
						utf_msg = w2u(gch->ptszText);
					else
						utf_msg = a2u(gch->pszText);
					
					mwConference_sendText((mwConference *)conf->data, utf_msg);

					free(utf_msg);
				}
				break;
			case GC_SESSION_TERMINATE:
				{
					//MessageBox(0, "Terminate chat event", "msg", MB_OK);
					if(my_conference == conf->data) CloseMyConference();
					else {
						char *utfs = t2u(TranslateT("I'm outa here."));
						mwConference_destroy((mwConference *)conf->data, 0, utfs);
						free(utfs);
					}
				}
				break;
			}

			break;
		}
		free(szId);
	}

	g_list_free(conferences);

	return 0;
}

int ChatDeleted(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	if(DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0) == 0)
		return 0;

	DBVARIANT dbv;
	if(!DBGetContactSetting(hContact, PROTO, "ChatRoomID", &dbv)) {
		TerminateConference(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	return 0;
}

int CreateChat(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	mwAwareIdBlock id_block;
	mwIdBlock idb;
	if(my_login_info && GetAwareIdFromContact(hContact, &id_block)) {
		TCHAR title[512];
		TCHAR *ts = u2t(my_login_info->user_name);

#ifdef _UNICODE
		swprintf(title, TranslateT("%s's Conference"), ts); // TODO: FIX potential buffer overflow
#else
		snprintf(title, 256, Translate("%s's Conference"), ts);
#endif
		free(ts);

		idb.user = id_block.user;
		idb.community = id_block.community;

		invite_queue.push(idb.user);

		if(!my_conference) {
			char *utfs;
			my_conference = mwConference_new(service_conference, utfs = t2u(title));
			mwConference_open(my_conference);
			free(utfs);
		} else {
			ClearInviteQueue();
		}

		free(id_block.user);
	}

	return 0;
}

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;


	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | (DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0) == 1 ? 0 : CMIF_HIDDEN);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hLeaveChatMenuItem, (LPARAM)&mi);

	// if user is already in our meeting, 
	bool not_present = true;
	DBVARIANT dbv;
	if(my_conference && !DBGetContactSettingUtf(hContact, PROTO, "stid", &dbv)) {
		char *user_id = dbv.pszVal;

		GList *members, *mem;
		members = mem = mwConference_getMembers(my_conference);
		for(;mem;mem=mem->next) {
			if(my_login_info && strcmp(user_id, ((mwLoginInfo *)mem->data)->user_id) == 0) {
				not_present = false;
				break;
			}
		}
		g_list_free(members);

		DBFreeVariant(&dbv);
	}
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | (DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0) == 0 && not_present ? 0 : CMIF_HIDDEN);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hCreateChatMenuItem, (LPARAM)&mi);

	return 0;
}

void InitConference(mwSession *session) {
	my_login_info = mwSession_getLoginInfo(session);

	mwSession_addService(session, (mwService *)(service_conference = mwServiceConference_new(session, &mwConference_handler)));

	hGcEvent = HookEvent(ME_GC_EVENT, GcEventHook);

	hChatDeletedEvent = HookEvent(ME_DB_CONTACT_DELETED, ChatDeleted);

}

void DeinitConference(mwSession *session) {
	GList *conferences, *conf;
	conferences = conf = mwServiceConference_getConferences(service_conference);
	for(;conf;conf = conf->next) {
		if(my_conference == conf->data) CloseMyConference();
		else {
			char *utfs = t2u(TranslateT("I'm outa here."));
			mwConference_destroy((mwConference *)conf->data, 0, utfs);
			free(utfs);
		}
	
	}
	g_list_free(conferences);

	
	UnhookEvent(hMenuBuildEvent);
	hMenuBuildEvent = 0;
	UnhookEvent(hChatDeletedEvent);
	hChatDeletedEvent = 0;

	my_login_info = 0;

	UnhookEvent(hGcEvent);
	hGcEvent = 0;

	mwSession_removeService(session, mwService_CONFERENCE);
	mwService_free((mwService *)service_conference);
	service_conference = 0;
}

void InitConferenceMenu() {
	CreateProtoServiceFunction(PROTO, MS_SAMETIME_MENULEAVECHAT, ChatDeleted);
	CreateProtoServiceFunction(PROTO, MS_SAMETIME_MENUCREATECHAT, CreateChat);

	hIconLeave = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_LEAVE));
	hIconCreate = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_INVITE));

	char service_function[128];
	strcpy(service_function, PROTO);
	char *d = service_function + strlen(service_function);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.pszContactOwner = PROTO;

	mi.pszName = Translate("Leave Conference");
	strcpy(d, MS_SAMETIME_MENULEAVECHAT);
	mi.pszService = service_function;
	mi.hIcon = hIconLeave;

	hLeaveChatMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mi.flags = CMIF_NOTOFFLINE;
	mi.pszName = Translate("Start Conference");
	strcpy(d, MS_SAMETIME_MENUCREATECHAT);
	mi.pszService = service_function;
	mi.hIcon = hIconCreate;

	hCreateChatMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	hMenuBuildEvent = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

}

void DeinitConferenceMenu() {
	DestroyIcon(hIconLeave);
	DestroyIcon(hIconCreate);
}

