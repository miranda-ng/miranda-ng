/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

enum ChatMenuItems
{
	IDM_INVITE = 10, IDM_EXIT, IDM_DESTROY,

	IDM_DETAILS = 20, IDM_HISTORY
};

static const struct gc_item LogMenuItems[] =
{
	{ LPGENW("&Invite user..."), IDM_INVITE, MENU_ITEM, FALSE },
	{ LPGENW("E&xit chat session"), IDM_EXIT, MENU_ITEM, FALSE },
	{ LPGENW("&Destroy chat session"), IDM_DESTROY, MENU_ITEM, FALSE },
};

static const struct gc_item NickMenuItems[] =
{
	{ LPGENW("User &details"), IDM_DETAILS, MENU_ITEM, FALSE },
	{ LPGENW("User &history"), IDM_HISTORY, MENU_ITEM, FALSE },
};

void FacebookProto::UpdateChat(const char *chat_id, const char *id, const char *name, const char *message, DWORD timestamp, bool is_old)
{
	// replace % to %% to not interfere with chat color codes
	std::string smessage = message;
	utils::text::replace_all(&smessage, "%", "%%");

	GCEVENT gce = { m_szModuleName, chat_id, GC_EVENT_MESSAGE };
	gce.pszText.a = smessage.c_str();
	gce.time = timestamp ? timestamp : ::time(0);
	if (id != nullptr)
		gce.bIsMe = !mir_strcmp(id, facy.self_.user_id.c_str());
	gce.dwFlags = GCEF_ADDTOLOG | GCEF_UTF8;
	if (is_old) {
		gce.dwFlags |= GCEF_NOTNOTIFY;
		gce.dwFlags &= ~GCEF_ADDTOLOG;
	}
	gce.pszNick.a = name;
	gce.pszUID.a = id;
	Chat_Event(&gce);

	facy.erase_reader(ChatIDToHContact(chat_id));
}

void FacebookProto::RenameChat(const char *chat_id, const char *name)
{
	Chat_ChangeSessionName(m_szModuleName, _A2T(chat_id), Utf2T(name));
}

int FacebookProto::OnGCEvent(WPARAM, LPARAM lParam)
{
	GCHOOK *hook = reinterpret_cast<GCHOOK*>(lParam);
	if (mir_strcmp(hook->pszModule, m_szModuleName))
		return 0;

	// Ignore for special chatrooms
	if (!mir_wstrcmp(hook->ptszID, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM)))
		return 0;

	switch (hook->iType) {
	case GC_USER_MESSAGE:
		if (isOnline()) {
			debugLogA("  > Chat - Outgoing message");
			std::string msg = _T2A(hook->ptszText, CP_UTF8);
			std::string chat_id = _T2A(hook->ptszID, CP_UTF8);
			ForkThread(&FacebookProto::SendChatMsgWorker, new send_chat(chat_id, msg));
		}
		break;

	case GC_USER_PRIVMESS:
		{
			facebook_user fbu;
			fbu.user_id = _T2A(hook->ptszUID, CP_UTF8);

			// Find this contact in list or add new temporary contact
			MCONTACT hContact = AddToContactList(&fbu, false, true);
			if (!hContact)
				break;

			CallService(MS_MSG_SENDMESSAGEW, hContact);
		}
		break;

	case GC_USER_LOGMENU:
		switch (hook->dwData) {
		case IDM_INVITE:
			break;

		case IDM_EXIT:
			{
				std::string thread_id = _T2A(hook->ptszID, CP_UTF8);
				auto it = facy.chat_rooms.find(thread_id);
				if (it != facy.chat_rooms.end())
					facy.sendRequest(facy.exitThreadRequest(it->second));
			}
			break;

		case IDM_DESTROY:
			{
				std::string thread_id = _T2A(hook->ptszID, CP_UTF8);
				auto it = facy.chat_rooms.find(thread_id);
				if (it != facy.chat_rooms.end())
					if (IDOK == MessageBoxW(nullptr, TranslateT("Delete conversation"), TranslateT("This will permanently delete the conversation history"), MB_OKCANCEL))
						facy.sendRequest(facy.destroyThreadRequest(it->second));
			}
		}
		break;

	case GC_USER_NICKLISTMENU:
		MCONTACT hContact = 0;
		if (hook->dwData == 10 || hook->dwData == 20) {
			facebook_user fbu;
			fbu.user_id = _T2A(hook->ptszUID, CP_UTF8);

			// Find this contact in list or add new temporary contact
			hContact = AddToContactList(&fbu, false, true);
			if (!hContact)
				break;
		}

		switch (hook->dwData) {
		case IDM_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, hContact);
			break;

		case IDM_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact);
			break;
		}

		break;
	}

	return 0;
}

void FacebookProto::AddChatContact(const char *chat_id, const chatroom_participant &user, bool addToLog)
{
	// Don't add user if it's already there
	if (IsChatContact(chat_id, user.user_id.c_str()))
		return;

	GCEVENT gce = { m_szModuleName, chat_id, GC_EVENT_JOIN };
	gce.dwFlags = GCEF_UTF8 + (addToLog ? GCEF_ADDTOLOG : 0);
	gce.pszNick.a = user.nick.c_str();
	gce.pszUID.a = user.user_id.c_str();
	gce.time = ::time(0);
	gce.bIsMe = (user.role == ROLE_ME);

	if (user.is_former) {
		gce.pszStatus.a = TranslateU("Former");
	}
	else {
		switch (user.role) {
		case ROLE_ME:
			gce.pszStatus.a = TranslateU("Myself");
			break;
		case ROLE_FRIEND:
			gce.pszStatus.a = TranslateU("Friend");
			break;
		case ROLE_NONE:
			gce.pszStatus.a = TranslateU("User");
			break;
		}
	}

	Chat_Event(&gce);
}

void FacebookProto::RemoveChatContact(const char *chat_id, const char *id, const char *name)
{
	GCEVENT gce = { m_szModuleName, chat_id, GC_EVENT_PART };
	gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
	gce.pszNick.a = name;
	gce.pszUID.a = id;
	gce.time = ::time(0);
	gce.bIsMe = false;

	Chat_Event(&gce);
}

/** Caller must free result */
char *FacebookProto::GetChatUsers(const char *chat_id)
{
	ptrW ptszChatID(mir_a2u(chat_id));

	GC_INFO gci = {};
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
	return (users != nullptr && strstr(users, id) != nullptr);
}

void FacebookProto::AddChat(const char *id, const wchar_t *tname)
{
	ptrW tid(mir_a2u(id));

	// Create the group chat session
	SESSION_INFO *si = Chat_NewSession(GCW_PRIVMESS, m_szModuleName, tid, tname);
	if (!si)
		return;

	// Send setting events
	Chat_AddGroup(si, TranslateT("Myself"));
	Chat_AddGroup(si, TranslateT("Friend"));
	Chat_AddGroup(si, TranslateT("User"));
	Chat_AddGroup(si, TranslateT("Former"));

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
	if (it != facy.chat_rooms.end())
		fbc = it->second;
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
	// like: if (ChatIDToHContact(thread_id) == nullptr) {
	ptrA users(GetChatUsers(thread_id.c_str()));
	if (users == nullptr) {
		// Add chatroom
		AddChat(fbc->thread_id.c_str(), fbc->chat_name.c_str());

		// Add chat contacts
		for (auto &jt : fbc->participants)
			AddChatContact(fbc->thread_id.c_str(), jt.second, false);

		// Load last messages
		delSetting(hContact, FACEBOOK_KEY_MESSAGE_ID); // We're creating new chatroom so we want load all recent messages
		ForkThread(&FacebookProto::LoadLastMessages, new MCONTACT(hContact));
	}

	return 0;
}

INT_PTR FacebookProto::OnLeaveChat(WPARAM wParam, LPARAM)
{
	ptrW idT(wParam ? getWStringA(wParam, "ChatRoomID") : nullptr);

	Chat_Control(m_szModuleName, idT, SESSION_OFFLINE);
	Chat_Terminate(m_szModuleName, idT);

	if (!wParam)
		facy.clear_chatrooms();
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
	if (mir_strcmp(gcmi->pszModule, m_szModuleName))
		return 0;

	if (gcmi->Type == MENU_ON_LOG)
		Chat_AddMenuItems(gcmi->hMenu, _countof(LogMenuItems), LogMenuItems, &g_plugin);
	else if (gcmi->Type == MENU_ON_NICKLIST)
		Chat_AddMenuItems(gcmi->hMenu, _countof(NickMenuItems), NickMenuItems, &g_plugin);
	return 0;
}

bool FacebookProto::IsSpecialChatRoom(MCONTACT hContact)
{
	if (!isChatRoom(hContact))
		return false;

	ptrA id(getStringA(hContact, "ChatRoomID"));
	return id && !mir_strcmp(id, FACEBOOK_NOTIFICATIONS_CHATROOM);
}

void FacebookProto::PrepareNotificationsChatRoom()
{
	if (!getBool(FACEBOOK_KEY_NOTIFICATIONS_CHATROOM, DEFAULT_NOTIFICATIONS_CHATROOM))
		return;

	// Prepare notifications chatroom if not exists
	MCONTACT hNotificationsChatRoom = ChatIDToHContact(FACEBOOK_NOTIFICATIONS_CHATROOM);
	if (hNotificationsChatRoom == 0 || getDword(hNotificationsChatRoom, "Status", ID_STATUS_OFFLINE) != ID_STATUS_ONLINE) {
		wchar_t nameT[200];
		mir_snwprintf(nameT, L"%s: %s", m_tszUserName, TranslateT("Notifications"));

		// Create the group chat session
		Chat_NewSession(GCW_PRIVMESS, m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), nameT);

		// Send setting events
		Chat_Control(m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), WINDOW_HIDDEN);
		Chat_Control(m_szModuleName, _A2W(FACEBOOK_NOTIFICATIONS_CHATROOM), SESSION_ONLINE);
	}
}

void FacebookProto::UpdateNotificationsChatRoom(facebook_notification *notification)
{
	if (!getBool(FACEBOOK_KEY_NOTIFICATIONS_CHATROOM, DEFAULT_NOTIFICATIONS_CHATROOM))
		return;

	std::stringstream text;
	text << notification->text << "\n\n" << PrepareUrl(notification->link);

	std::string message = text.str();
	utils::text::replace_all(&message, "%", "%%");

	GCEVENT gce = { m_szModuleName, FACEBOOK_NOTIFICATIONS_CHATROOM, GC_EVENT_MESSAGE };
	gce.pszText.a = message.c_str();
	gce.time = notification->time ? notification->time : ::time(0);
	gce.bIsMe = false;
	gce.dwFlags = GCEF_UTF8 + GCEF_ADDTOLOG;
	gce.pszNick.a = TranslateU("Notifications");
	gce.pszUID.a = FACEBOOK_NOTIFICATIONS_CHATROOM;
	Chat_Event(&gce);
}

std::string FacebookProto::GenerateChatName(facebook_chatroom *fbc)
{
	std::string name = "";
	unsigned int namesUsed = 0;

	for (auto &it : fbc->participants) {
		std::string participant = it.second.nick;

		// Ignore self contact, empty and numeric only participant names
		if (it.second.role == ROLE_ME || participant.empty() || participant.find_first_not_of("0123456789") == std::string::npos)
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

void FacebookProto::LoadParticipantsNames(facebook_chatroom *fbc)
{
	std::vector<std::string> namelessIds;

	// TODO: We could load all names from server at once by skipping this for cycle and using namelessIds as all in participants list, but we would lost our local names of our contacts. But maybe that's not a problem?
	for (auto &it : fbc->participants) {
		const char *id = it.first.c_str();
		chatroom_participant &user = it.second;

		if (!user.loaded) {
			if (!mir_strcmp(id, facy.self_.user_id.c_str())) {
				user.nick = facy.self_.real_name;
				user.role = ROLE_ME;
				user.loaded = true;
			}
			else {
				MCONTACT hContact = ContactIDToHContact(id);
				if (hContact != 0) {
					DBVARIANT dbv;
					if (!getStringUtf(hContact, FACEBOOK_KEY_NICK, &dbv)) {
						user.nick = dbv.pszVal;
						db_free(&dbv);
					}
					if (user.role == ROLE_NONE) {
						int type = getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE);
						if (type == CONTACT_FRIEND)
							user.role = ROLE_FRIEND;
						else
							user.role = ROLE_NONE;
					}
					user.loaded = true;
				}

				if (!user.loaded)
					namelessIds.push_back(id);
			}
		}
	}

	if (!namelessIds.empty()) {
		// we have some contacts without name, let's load them all from the server

		LIST<char> userIds(1);
		for (std::string::size_type i = 0; i < namelessIds.size(); i++)
			userIds.insert(mir_strdup(namelessIds.at(i).c_str()));

		http::response resp = facy.sendRequest(facy.userInfoRequest(userIds));

		FreeList(userIds);
		userIds.destroy();

		if (resp.code == HTTP_CODE_OK) {
			// TODO: We can cache these results and next time (e.g. for different chatroom) we can use that already cached names
			if (EXIT_SUCCESS == ParseChatParticipants(&resp.data, &fbc->participants))
				debugLogA("*** Participant names processed");
			else
				debugLogA("*** Error processing participant names");
		}
	}
}

void FacebookProto::JoinChatrooms()
{
	for (auto &hContact : AccContacts()) {
		if (!isChatRoom(hContact))
			continue;

		// Ignore archived and unsubscribed chats
		if (getBool(hContact, FACEBOOK_KEY_CHAT_IS_ARCHIVED, false) || !getBool(hContact, FACEBOOK_KEY_CHAT_IS_SUBSCRIBED, true))
			continue;

		OnJoinChat(hContact, 0);
	}
}

void FacebookProto::LoadChatInfo(facebook_chatroom *fbc)
{
	if (isOffline())
		return;

	// request info about chat thread
	http::response resp = facy.sendRequest(facy.threadInfoRequest(true, fbc->thread_id.c_str()));
	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("LoadChatInfo");
		return;
	}

	if (ParseChatInfo(&resp.data, fbc) == EXIT_SUCCESS) {
		// Load missing participants names
		LoadParticipantsNames(fbc);

		// If chat has no name, create name from participants list
		if (fbc->chat_name.empty()) {
			std::string newName = GenerateChatName(fbc);
			fbc->chat_name = _A2T(newName.c_str(), CP_UTF8);
		}

		debugLogA("*** Chat thread info processed");
	}
	else debugLogA("*** Error processing chat thread info");

	facy.handle_success("LoadChatInfo");
}

int FacebookProto::ParseChatInfo(std::string *data, facebook_chatroom* fbc)
{
	size_t len = data->find("\r\n");
	if (len != data->npos)
		data->erase(len);

	JSONNode root = JSONNode::parse(data->c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &thread = root["o0"]["data"]["message_thread"];
	if (!thread)
		return EXIT_FAILURE;

	const JSONNode &thread_fbid_ = thread["thread_key"]["thread_fbid"];
	const JSONNode &name_ = thread["name"];
	if (!thread_fbid_)
		return EXIT_FAILURE;

	std::string tid = "id." + thread_fbid_.as_string();
	if (fbc->thread_id != tid)
		return EXIT_FAILURE;

	chatroom_participant user;
	user.is_former = false;
	user.role = ROLE_NONE;
	const JSONNode &participants = thread["all_participants"]["nodes"];
	for (auto &jt : participants) {
		user.user_id = jt["messaging_actor"]["id"].as_string();
		fbc->participants.insert(std::make_pair(user.user_id, user));
	}

	fbc->can_reply = thread["can_reply"].as_bool();
	fbc->is_archived = thread["has_viewer_archived"].as_bool();
	fbc->is_subscribed = thread["is_viewer_subscribed"].as_bool();
	fbc->read_only = thread["read_only"].as_bool();
	fbc->chat_name = std::wstring(ptrW(mir_utf8decodeW(name_.as_string().c_str())));
	return EXIT_SUCCESS;
}
