/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel

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
#include <m_history.h>
#include <m_userinfo.h>

void FacebookProto::UpdateChat(const char *chat_id, const char *id, const char *name, const char *message, DWORD timestamp, bool is_old)
{
	// replace % to %% to not interfere with chat color codes
	std::string smessage = message;
	utils::text::replace_all(&smessage, "%", "%%");

	ptrW tid(mir_a2u(id));
	ptrW tnick(mir_a2u_cp(name, CP_UTF8));
	ptrW ttext(mir_a2u_cp(smessage.c_str(), CP_UTF8));
	ptrW tchat_id(mir_a2u(chat_id));

	GCEVENT gce = { m_szModuleName, tchat_id, GC_EVENT_MESSAGE };
	gce.ptszText = ttext;
	gce.time = timestamp ? timestamp : ::time(NULL);
	if (id != NULL)
		gce.bIsMe = !mir_strcmp(id, facy.self_.user_id.c_str());
	gce.dwFlags |= GCEF_ADDTOLOG;
	if (is_old) {
		gce.dwFlags |= GCEF_NOTNOTIFY;
		gce.dwFlags &= ~GCEF_ADDTOLOG;
	}
	gce.ptszNick = tnick;
	gce.ptszUID = tid;
	Chat_Event(&gce);

	facy.erase_reader(ChatIDToHContact(chat_id));
}

void FacebookProto::RenameChat(const char *chat_id, const char *name)
{
	ptrW tchat_id(mir_a2u(chat_id));
	ptrW tname(mir_a2u_cp(name, CP_UTF8));
	Chat_ChangeSessionName(m_szModuleName, tchat_id, tname);
}

int FacebookProto::OnGCEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);

	if (mir_strcmp(hook->pszModule, m_szModuleName))
		return 0;

	// Ignore for special chatrooms
	if (!mir_wstrcmp(hook->ptszID, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM)))
		return 0;

	switch (hook->iType)
	{
	case GC_USER_MESSAGE:
	{
		std::string msg = _T2A(hook->ptszText, CP_UTF8);
		std::string chat_id = _T2A(hook->ptszID, CP_UTF8);

		if (isOnline()) {
			debugLogA("  > Chat - Outgoing message");
			ForkThread(&FacebookProto::SendChatMsgWorker, new send_chat(chat_id, msg));
		}

		break;
	}

	case GC_USER_PRIVMESS:
	{
		facebook_user fbu;
		fbu.user_id = _T2A(hook->ptszUID, CP_UTF8);

		// Find this contact in list or add new temporary contact
		MCONTACT hContact = AddToContactList(&fbu, false, true);

		if (!hContact)
			break;

		CallService(MS_MSG_SENDMESSAGEW, hContact);
		break;
	}

	/*
	case GC_USER_LOGMENU:
	{
	switch(hook->dwData)
	{
	case 10:
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, invite_to_chat_dialog,
	LPARAM(new invite_chat_param(item->id, this)));
	break;

	case 20:
	//chat_leave(id);
	break;
	}
	break;
	}
	*/

	case GC_USER_NICKLISTMENU:
	{
		MCONTACT hContact = NULL;
		if (hook->dwData == 10 || hook->dwData == 20) {
			facebook_user fbu;
			fbu.user_id = _T2A(hook->ptszUID, CP_UTF8);

			// Find this contact in list or add new temporary contact
			hContact = AddToContactList(&fbu, false, true);

			if (!hContact)
				break;
		}

		switch (hook->dwData)
		{
		case 10:
			CallService(MS_USERINFO_SHOWDIALOG, hContact);
			break;

		case 20:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact);
			break;

		case 110:
			//chat_leave(id);
			break;
		}

		break;
	}

	case GC_USER_LEAVE:
	case GC_SESSION_TERMINATE:
		break;
	}

	return 0;
}

void FacebookProto::AddChatContact(const char *chat_id, const chatroom_participant &user, bool addToLog)
{
	// Don't add user if it's already there
	if (IsChatContact(chat_id, user.user_id.c_str()))
		return;

	ptrW tchat_id(mir_a2u(chat_id));
	ptrW tnick(mir_a2u_cp(user.nick.c_str(), CP_UTF8));
	ptrW tid(mir_a2u(user.user_id.c_str()));

	GCEVENT gce = { m_szModuleName, tchat_id, GC_EVENT_JOIN };
	gce.dwFlags = addToLog ? GCEF_ADDTOLOG : 0;
	gce.ptszNick = tnick;
	gce.ptszUID = tid;
	gce.time = ::time(NULL);
	gce.bIsMe = (user.role == ROLE_ME);

	if (user.is_former) {
		gce.ptszStatus = TranslateT("Former");
	}
	else {
		switch (user.role) {
		case ROLE_ME:
			gce.ptszStatus = TranslateT("Myself");
			break;
		case ROLE_FRIEND:
			gce.ptszStatus = TranslateT("Friend");
			break;
		case ROLE_NONE:
			gce.ptszStatus = TranslateT("User");
			break;
		}
	}

	Chat_Event(&gce);
}

void FacebookProto::RemoveChatContact(const char *chat_id, const char *id, const char *name)
{
	// We dont want to remove our self-contact from chat. Ever.
	if (!mir_strcmp(id, facy.self_.user_id.c_str()))
		return;

	ptrW tchat_id(mir_a2u(chat_id));
	ptrW tnick(mir_a2u_cp(name, CP_UTF8));
	ptrW tid(mir_a2u(id));

	GCEVENT gce = { m_szModuleName, tchat_id, GC_EVENT_PART };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = tnick;
	gce.ptszUID = tid;
	gce.time = ::time(NULL);
	gce.bIsMe = false;

	Chat_Event(&gce);
}

/** Caller must free result */
char *FacebookProto::GetChatUsers(const char *chat_id)
{
	ptrW ptszChatID(mir_a2u(chat_id));

	GC_INFO gci = { 0 };
	gci.Flags = GCF_USERS;
	gci.pszModule = m_szModuleName;
	gci.pszID = ptszChatID;
	Chat_GetInfo(&gci);

	// mir_free(gci.pszUsers);
	return gci.pszUsers;
}

bool FacebookProto::IsChatContact(const char *chat_id, const char *id)
{
	ptrA users(GetChatUsers(chat_id));
	return (users != NULL && strstr(users, id) != NULL);
}

void FacebookProto::AddChat(const char *id, const wchar_t *tname)
{
	ptrW tid(mir_a2u(id));

	// Create the group chat session
	Chat_NewSession(GCW_PRIVMESS, m_szModuleName, tid, tname);

	// Send setting events
	Chat_AddGroup(m_szModuleName, tid, TranslateT("Myself"));
	Chat_AddGroup(m_szModuleName, tid, TranslateT("Friend"));
	Chat_AddGroup(m_szModuleName, tid, TranslateT("User"));
	Chat_AddGroup(m_szModuleName, tid, TranslateT("Former"));

	// Finish initialization
	bool hideChats = getBool(FACEBOOK_KEY_HIDE_CHATS, DEFAULT_HIDE_CHATS);
	Chat_Control(m_szModuleName, tid, (hideChats ? WINDOW_HIDDEN : SESSION_INITDONE));
	Chat_Control(m_szModuleName, tid, SESSION_ONLINE);
}

INT_PTR FacebookProto::OnJoinChat(WPARAM hContact, LPARAM)
{
	if (isOffline() || facy.dtsg_.empty() || !m_enableChat || IsSpecialChatRoom(hContact))
		return 0;

	ptrW idT(getWStringA(hContact, "ChatRoomID"));
	ptrA threadId(getStringA(hContact, FACEBOOK_KEY_TID));

	if (!idT || !threadId)
		return 0;

	facebook_chatroom *fbc;
	std::string thread_id = threadId;

	auto it = facy.chat_rooms.find(thread_id);
	if (it != facy.chat_rooms.end()) {
		fbc = it->second;
	}
	else {
		// We don't have this chat loaded in memory yet, lets load some info (name, list of users)
		fbc = new facebook_chatroom(thread_id);
		LoadChatInfo(fbc);
		facy.chat_rooms.insert(std::make_pair(thread_id, fbc));

		// Update loaded info about this chat
		setByte(hContact, FACEBOOK_KEY_CHAT_CAN_REPLY, fbc->can_reply);
		setByte(hContact, FACEBOOK_KEY_CHAT_READ_ONLY, fbc->read_only);
		setByte(hContact, FACEBOOK_KEY_CHAT_IS_ARCHIVED, fbc->is_archived);
		setByte(hContact, FACEBOOK_KEY_CHAT_IS_SUBSCRIBED, fbc->is_subscribed);
	}

	// RM TODO: better use check if chatroom exists/is in db/is online... no?
	// like: if (ChatIDToHContact(thread_id) == NULL) {
	ptrA users(GetChatUsers(thread_id.c_str()));
	if (users == NULL) {
		// Add chatroom
		AddChat(fbc->thread_id.c_str(), fbc->chat_name.c_str());

		// Add chat contacts
		for (std::map<std::string, chatroom_participant>::iterator jt = fbc->participants.begin(); jt != fbc->participants.end(); ++jt) {
			AddChatContact(fbc->thread_id.c_str(), jt->second, false);
		}

		// Load last messages
		delSetting(hContact, FACEBOOK_KEY_MESSAGE_ID); // We're creating new chatroom so we want load all recent messages
		ForkThread(&FacebookProto::LoadLastMessages, new MCONTACT(hContact));
	}

	return 0;
}

INT_PTR FacebookProto::OnLeaveChat(WPARAM wParam, LPARAM)
{
	ptrW idT(wParam ? getWStringA(wParam, "ChatRoomID") : NULL);

	Chat_Control(m_szModuleName, idT, SESSION_OFFLINE);
	Chat_Terminate(m_szModuleName, idT);

	if (!wParam) {
		facy.clear_chatrooms();
	}
	else if (!IsSpecialChatRoom(wParam)) {
		ptrA threadId(getStringA(wParam, FACEBOOK_KEY_TID));
		if (!threadId)
			return 0;

		auto it = facy.chat_rooms.find(std::string(threadId));
		if (it != facy.chat_rooms.end()) {
			delete it->second;
			facy.chat_rooms.erase(it);
		}
	}

	return 0;
}

int FacebookProto::OnGCMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;

	if (gcmi == NULL || _stricmp(gcmi->pszModule, m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG)
	{
		static const struct gc_item Items[] =
		{
			{ LPGENW("&Invite user..."), 10, MENU_ITEM, FALSE },
			{ LPGENW("&Leave chat session"), 20, MENU_ITEM, FALSE }
		};
		Chat_AddMenuItems(gcmi->hMenu, _countof(Items), Items);
	}
	else if (gcmi->Type == MENU_ON_NICKLIST)
	{
		if (!_stricmp(facy.self_.user_id.c_str(), _T2A(gcmi->pszUID)))
		{
			/*static const struct gc_item Items[] =
			{
			{ LPGENW("User &details"), 10, MENU_ITEM, FALSE },
			{ LPGENW("User &history"), 20, MENU_ITEM, FALSE },
			{ L"", 100, MENU_SEPARATOR, FALSE },
			{ LPGENW("&Leave chat session"), 110, MENU_ITEM, FALSE }
			};
			gcmi->nItems = _countof(Items);
			gcmi->Item = (gc_item*)Items;*/
		}
		else
		{
			static const struct gc_item Items[] =
			{
				{ LPGENW("User &details"), 10, MENU_ITEM, FALSE },
				{ LPGENW("User &history"), 20, MENU_ITEM, FALSE }
			};
			Chat_AddMenuItems(gcmi->hMenu, _countof(Items), Items);
		}
	}

	return 0;
}

bool FacebookProto::IsSpecialChatRoom(MCONTACT hContact) {
	if (!isChatRoom(hContact))
		return false;

	ptrA id(getStringA(hContact, "ChatRoomID"));
	return id && !mir_strcmp(id, FACEBOOK_NOTIFICATIONS_CHATROOM);
}

void FacebookProto::PrepareNotificationsChatRoom() {
	if (!getBool(FACEBOOK_KEY_NOTIFICATIONS_CHATROOM, DEFAULT_NOTIFICATIONS_CHATROOM))
		return;

	// Prepare notifications chatroom if not exists
	MCONTACT hNotificationsChatRoom = ChatIDToHContact(FACEBOOK_NOTIFICATIONS_CHATROOM);
	if (hNotificationsChatRoom == NULL || getDword(hNotificationsChatRoom, "Status", ID_STATUS_OFFLINE) != ID_STATUS_ONLINE) {
		wchar_t nameT[200];
		mir_snwprintf(nameT, L"%s: %s", m_tszUserName, TranslateT("Notifications"));

		// Create the group chat session
		Chat_NewSession(GCW_PRIVMESS, m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), nameT);

		// Send setting events
		Chat_Control(m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), WINDOW_HIDDEN);
		Chat_Control(m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), SESSION_ONLINE);
	}
}

void FacebookProto::UpdateNotificationsChatRoom(facebook_notification *notification) {
	if (!getBool(FACEBOOK_KEY_NOTIFICATIONS_CHATROOM, DEFAULT_NOTIFICATIONS_CHATROOM))
		return;

	std::stringstream text;
	text << notification->text << "\n\n" << PrepareUrl(notification->link);

	std::string message = text.str();
	utils::text::replace_all(&message, "%", "%%");

	ptrW idT(mir_wstrdup(_A2W(FACEBOOK_NOTIFICATIONS_CHATROOM)));
	ptrW messageT(mir_a2u_cp(message.c_str(), CP_UTF8));

	GCEVENT gce = { m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), GC_EVENT_MESSAGE };
	gce.ptszText = messageT;
	gce.time = notification->time ? notification->time : ::time(NULL);
	gce.bIsMe = false;
	gce.dwFlags |= GCEF_ADDTOLOG;
	gce.ptszNick = TranslateT("Notifications");
	gce.ptszUID = idT;

	Chat_Event(&gce);
}

std::string FacebookProto::GenerateChatName(facebook_chatroom *fbc)
{
	std::string name = "";
	unsigned int namesUsed = 0;

	for (auto it = fbc->participants.begin(); it != fbc->participants.end(); ++it) {
		std::string participant = it->second.nick;

		// Ignore self contact, empty and numeric only participant names
		if (it->second.role == ROLE_ME || participant.empty() || participant.find_first_not_of("0123456789") == std::string::npos)
			continue;

		if (namesUsed > 0)
			name += ", ";

		name += utils::text::prepare_name(participant, false);

		if (++namesUsed >= FACEBOOK_CHATROOM_NAMES_COUNT)
			break;
	}

	// Participants.size()-1 because we ignore self contact
	if (fbc->participants.size() - 1 > namesUsed) {
		wchar_t more[200];
		mir_snwprintf(more, TranslateT("%s and more (%d)"), fbc->chat_name.c_str(), fbc->participants.size() - 1 - namesUsed); // -1 because we ignore self contact
		fbc->chat_name = more;
	}

	// If there are no participants to create a name from, use just thread_id
	if (name.empty())
		name = fbc->thread_id.c_str();

	return name;
}
