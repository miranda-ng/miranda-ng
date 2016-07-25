/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-16 Robert Pösel

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

void updateStringUtf(FacebookProto *proto, MCONTACT hContact, const char *key, const std::string &value) {
	bool update_required = true;

	DBVARIANT dbv;
	if (!proto->getStringUtf(hContact, key, &dbv)) {
		update_required = mir_strcmp(dbv.pszVal, value.c_str()) != 0;
		db_free(&dbv);
	}

	if (update_required) {
		proto->setStringUtf(hContact, key, value.c_str());
	}
}

void FacebookProto::SaveName(MCONTACT hContact, const facebook_user *fbu)
{
	if (fbu->type == CONTACT_PAGE) {
		// Page has only nickname and no first/last names
		std::string nick = m_pagePrefix + " " + fbu->real_name;

		updateStringUtf(this, hContact, FACEBOOK_KEY_NICK, nick);
		delSetting(hContact, FACEBOOK_KEY_FIRST_NAME);
		delSetting(hContact, FACEBOOK_KEY_SECOND_NAME);
		delSetting(hContact, FACEBOOK_KEY_LAST_NAME);
		return;
	}

	// Save nick
	std::string nick = fbu->real_name;
	if (!getBool(FACEBOOK_KEY_NAME_AS_NICK, 1) && !fbu->nick.empty())
		nick = fbu->nick;

	updateStringUtf(this, hContact, FACEBOOK_KEY_NICK, nick);

	// Explode whole name into first, second and last name
	std::vector<std::string> names;
	utils::text::explode(fbu->real_name, " ", &names);

	updateStringUtf(this, hContact, FACEBOOK_KEY_FIRST_NAME, names.size() > 0 ? names.front().c_str() : "");
	updateStringUtf(this, hContact, FACEBOOK_KEY_LAST_NAME, names.size() > 1 ? names.back().c_str() : "");

	std::string middle;
	if (names.size() > 2) {
		for (std::string::size_type i = 1; i < names.size() - 1; i++) {
			if (!middle.empty())
				middle += " ";

			middle += names.at(i);
		}
	}
	updateStringUtf(this, hContact, FACEBOOK_KEY_SECOND_NAME, middle);
}

bool FacebookProto::IsMyContact(MCONTACT hContact, bool include_chat)
{
	const char *proto = GetContactProto(hContact);
	if (proto && !mir_strcmp(m_szModuleName, proto)) {
		if (include_chat)
			return true;
		return !isChatRoom(hContact);
	}
	return false;
}

MCONTACT FacebookProto::ChatIDToHContact(const std::string &chat_id)
{
	if (chat_id.empty()) {
		debugLogA("!!! Calling ChatIDToContactID() with empty chat_id");
		return 0;
	}

	// First check cache
	auto it = facy.chat_id_to_hcontact.find(chat_id);
	if (it != facy.chat_id_to_hcontact.end()) {
		// Check if contact is still valid
		if (CallService(MS_DB_CONTACT_IS, (WPARAM)it->second) == 1)
			return it->second;
		else
			facy.chat_id_to_hcontact.erase(it);
	}

	// Go through all local contacts
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!IsMyContact(hContact, true))
			continue;

		ptrA id(getStringA(hContact, "ChatRoomID"));
		if (id && !mir_strcmp(id, chat_id.c_str())) {
			facy.chat_id_to_hcontact.insert(std::make_pair(chat_id, hContact));
			return hContact;
		}
	}

	return 0;
}

MCONTACT FacebookProto::ContactIDToHContact(const std::string &user_id)
{
	if (user_id.empty()) {
		debugLogA("!!! Calling ContactIDToHContact() with empty user_id");
		return 0;
	}

	// First check cache
	std::map<std::string, MCONTACT>::iterator it = facy.user_id_to_hcontact.find(user_id);
	if (it != facy.user_id_to_hcontact.end()) {
		// Check if contact is still valid
		if (CallService(MS_DB_CONTACT_IS, (WPARAM)it->second) == 1)
			return it->second;
		else
			facy.user_id_to_hcontact.erase(it);
	}

	// Go through all local contacts
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
		if (id && !mir_strcmp(id, user_id.c_str())) {
			facy.user_id_to_hcontact.insert(std::make_pair(user_id, hContact));
			return hContact;
		}
	}

	return 0;
}

std::string FacebookProto::ThreadIDToContactID(const std::string &thread_id)
{
	if (thread_id.empty()) {
		debugLogA("!!! Calling ThreadIDToContactID() with empty thread_id");
		return "";
	}

	// First check cache
	std::map<std::string, std::string>::iterator it = facy.thread_id_to_user_id.find(thread_id);
	if (it != facy.thread_id_to_user_id.end()) {
		return it->second;
	}

	// Go through all local contacts
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!IsMyContact(hContact))
			continue;

		ptrA tid(getStringA(hContact, FACEBOOK_KEY_TID));
		if (tid && !mir_strcmp(tid, thread_id.c_str())) {
			ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
			std::string user_id = (id ? id : "");
			if (!user_id.empty()) {
				facy.thread_id_to_user_id.insert(std::make_pair(thread_id, user_id));
				return user_id;
			}
			break; // this shouldn't happen unless user manually deletes ID from FB contact in DB
		}
	}

	// We don't have any contact with this thread_id cached, we must ask server	
	if (isOffline())
		return "";

	std::string data = "client=mercury";
	data += "&__user=" + facy.self_.user_id;
	data += "&__dyn=" + facy.__dyn();
	data += "&__req=" + facy.__req();
	data += "&fb_dtsg=" + facy.dtsg_;
	data += "&ttstamp=" + facy.ttstamp_;
	data += "&__rev=" + facy.__rev();

	data += "&threads[thread_ids][0]=" + utils::url::encode(thread_id);

	std::string user_id;
	http::response resp = facy.flap(REQUEST_THREAD_INFO, &data); // NOTE: Request revised 1.9.2015

	if (resp.code == HTTP_CODE_OK) {
		CODE_BLOCK_TRY

			facebook_json_parser* p = new facebook_json_parser(this);
		p->parse_thread_info(&resp.data, &user_id);
		delete p;

		if (!user_id.empty())
			facy.thread_id_to_user_id.insert(std::make_pair(thread_id, user_id));

		debugLogA("*** Thread info processed");

		CODE_BLOCK_CATCH

			debugLogA("*** Error processing thread info: %s", e.what());

		CODE_BLOCK_END
	}

	return user_id;
}

void FacebookProto::LoadContactInfo(facebook_user* fbu)
{
	if (isOffline())
		return;

	// TODO: support for more friends at once
	std::string data = "ids[0]=" + utils::url::encode(fbu->user_id);

	data += "&__user=" + facy.self_.user_id;
	data += "&__dyn=" + facy.__dyn();
	data += "&__req=" + facy.__req();
	data += "&fb_dtsg=" + facy.dtsg_;
	data += "&ttstamp=" + facy.ttstamp_;
	data += "&__rev=" + facy.__rev();

	http::response resp = facy.flap(REQUEST_USER_INFO, &data); // NOTE: Request revised 11.2.2016

	if (resp.code == HTTP_CODE_OK) {
		CODE_BLOCK_TRY

			facebook_json_parser* p = new facebook_json_parser(this);
			p->parse_user_info(&resp.data, fbu);
			delete p;

			debugLogA("*** Contact thread info processed");

		CODE_BLOCK_CATCH

			debugLogA("*** Error processing contact thread info: %s", e.what());

		CODE_BLOCK_END
	}
}

void FacebookProto::LoadParticipantsNames(facebook_chatroom *fbc)
{
	std::vector<std::string> namelessIds;

	// TODO: We could load all names from server at once by skipping this for cycle and using namelessIds as all in participants list, but we would lost our local names of our contacts. But maybe that's not a problem?
	for (auto it = fbc->participants.begin(); it != fbc->participants.end(); ++it) {
		const char *id = it->first.c_str();
		chatroom_participant &user = it->second;

		if (!user.loaded) {
			if (!mir_strcmp(id, facy.self_.user_id.c_str())) {
				user.nick = facy.self_.real_name;
				user.role = ROLE_ME;
				user.loaded = true;
			}
			else {
				MCONTACT hContact = ContactIDToHContact(id);
				if (hContact != NULL) {
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

	// if (isOffline())
	//	return;

	if (!namelessIds.empty()) {
		// we have some contacts without name, let's load them all from the server

		std::string data = "&__user=" + facy.self_.user_id;
		data += "&__dyn=" + facy.__dyn();
		data += "&__req=" + facy.__req();
		data += "&fb_dtsg=" + facy.dtsg_;
		data += "&ttstamp=" + facy.ttstamp_;
		data += "&__rev=" + facy.__rev();

		for (std::string::size_type i = 0; i < namelessIds.size(); i++) {
			std::string pos = utils::conversion::to_string(&i, UTILS_CONV_UNSIGNED_NUMBER);
			std::string id = utils::url::encode(namelessIds.at(i));
			data += "&ids[" + pos + "]=" + id;
		}

		http::response resp = facy.flap(REQUEST_USER_INFO, &data); // NOTE: Request revised 11.2.2016

		if (resp.code == HTTP_CODE_OK) {
			CODE_BLOCK_TRY

			// TODO: We can cache these results and next time (e.g. for different chatroom) we can use that already cached names

			facebook_json_parser* p = new facebook_json_parser(this);
			p->parse_chat_participant_names(&resp.data, &fbc->participants);
			delete p;

			debugLogA("*** Participant names processed");

			CODE_BLOCK_CATCH

				debugLogA("*** Error processing participant names: %s", e.what());

			CODE_BLOCK_END
		}
	}
}

void FacebookProto::JoinChatrooms()
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!isChatRoom(hContact))
			continue;
		
		// Ignore archived and unsubscribed chats
		if (getBool(hContact, FACEBOOK_KEY_CHAT_IS_ARCHIVED, false) || !getBool(hContact, FACEBOOK_KEY_CHAT_IS_SUBSCRIBED, true))
			continue;

		OnJoinChat(hContact, NULL);
	}
}

void FacebookProto::LoadChatInfo(facebook_chatroom *fbc)
{
	if (isOffline())
		return;

	std::string data = "client=mercury";
	data += "&__user=" + facy.self_.user_id;
	data += "&__dyn=" + facy.__dyn();
	data += "&__req=" + facy.__req();
	data += "&fb_dtsg=" + facy.dtsg_;
	data += "&ttstamp=" + facy.ttstamp_;
	data += "&__rev=" + facy.__rev();

	std::string thread_id = utils::url::encode(fbc->thread_id);

	// request info about thread
	data += "&threads[thread_ids][0]=" + thread_id;

	// TODO: ABILITY TO DEFINE TIMESTAMP! (way to load history since specific moment? ... offset seems to does nothing at all)
	/* messages[user_ids][<<userid>>][offset]=11
	messages[user_ids][<<userid>>][timestamp]=1446369866009 // most recent message has this timestamp (included)
	messages[user_ids][<<userid>>][limit]=20 */

	http::response resp = facy.flap(REQUEST_THREAD_INFO, &data); // NOTE: Request revised 12.2.2016

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("LoadChatInfo");
		return;
	}

	CODE_BLOCK_TRY

		facebook_json_parser* p = new facebook_json_parser(this);
		p->parse_chat_info(&resp.data, fbc);
		delete p;

		// Load missing participants names
		LoadParticipantsNames(fbc);

		// If chat has no name, create name from participants list
		if (fbc->chat_name.empty()) {
			unsigned int namesUsed = 0;

			for (auto it = fbc->participants.begin(); it != fbc->participants.end(); ++it) {
				std::string participant = it->second.nick;

				// Ignore empty and numeric only participant names
				if (participant.empty() || participant.find_first_not_of("0123456789") == std::string::npos)
					continue;

				if (namesUsed > 0)
					fbc->chat_name += L", ";

				std::tstring tname = _A2T(participant.c_str(), CP_UTF8);
				fbc->chat_name += utils::text::prepare_name(tname, false);
				
				if (++namesUsed >= FACEBOOK_CHATROOM_NAMES_COUNT)
					break;
			}

			if (fbc->participants.size() > namesUsed) {
				TCHAR more[200];
				mir_sntprintf(more, TranslateT("%s and more (%d)"), fbc->chat_name.c_str(), fbc->participants.size() - namesUsed);
				fbc->chat_name = more;
			}

			// If there are no participants to create a name from, use just thread_id
			if (fbc->chat_name.empty())
				fbc->chat_name = std::tstring(_A2T(fbc->thread_id.c_str())); // TODO: is this needed? Isn't it showed automatically as id if there is no name?
		}

		debugLogA("*** Chat thread info processed");

	CODE_BLOCK_CATCH

		debugLogA("*** Error processing chat thread info: %s", e.what());

	CODE_BLOCK_END

	facy.handle_success("LoadChatInfo");
}

MCONTACT FacebookProto::AddToContactList(facebook_user* fbu, bool force_add, bool add_temporarily)
{
	// Ignore self user completely
	if (fbu->user_id == facy.self_.user_id)
		return NULL;

	// First, check if this contact exists (and if does, just return it)
	if (!force_add) {
		MCONTACT hContact = ContactIDToHContact(fbu->user_id);

		if (hContact)
			return hContact;
	}

	// Try to make a new contact
	MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD);

	if (hContact && Proto_AddToContact(hContact, m_szModuleName) != 0) {
		CallService(MS_DB_CONTACT_DELETE, hContact);
		hContact = NULL;
	}

	// If we have some contact, we'll save its data
	if (hContact) {
		// Save these values only when adding new contact, not when updating existing
		if (add_temporarily) {
			db_set_b(hContact, "CList", "Hidden", 1);
			db_set_b(hContact, "CList", "NotOnList", 1);
		}

		setString(hContact, FACEBOOK_KEY_ID, fbu->user_id.c_str());

		std::string homepage = FACEBOOK_URL_PROFILE + fbu->user_id;
		setString(hContact, "Homepage", homepage.c_str());
		setTString(hContact, "MirVer", fbu->getMirVer());

		db_unset(hContact, "CList", "MyHandle");

		if (m_tszDefaultGroup)
			db_set_ts(hContact, "CList", "Group", m_tszDefaultGroup);

		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, fbu->type);

		if (getByte(FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, 0))
			CallService(MS_IGNORE_IGNORE, hContact, (LPARAM)IGNOREEVENT_USERONLINE);

		if (!fbu->real_name.empty())
			SaveName(hContact, fbu);

		if (!fbu->username.empty())
			setString(hContact, FACEBOOK_KEY_USERNAME, fbu->username.c_str());

		if (fbu->gender)
			setByte(hContact, "Gender", fbu->gender);

		// CheckAvatarChange(hContact, fbu->image_url);
	}

	return hContact;
}

void FacebookProto::SetAllContactStatuses(int status)
{
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		if (getWord(hContact, "Status", 0) != status)
			setWord(hContact, "Status", status);
	}
}

void FacebookProto::DeleteContactFromServer(void *data)
{
	facy.handle_entry("DeleteContactFromServer");

	if (data == NULL)
		return;

	std::string id = *(std::string*)data;
	delete (std::string*)data;

	if (isOffline())
		return;

	std::string query = "norefresh=true&unref=button_dropdown&confirmed=1&__a=1";
	query += "&fb_dtsg=" + facy.dtsg_;
	query += "&uid=" + id;
	query += "&__user=" + facy.self_.user_id;
	query += "&ttstamp=" + facy.ttstamp_;

	std::string get_query = "norefresh=true&unref=button_dropdown&uid=" + id;

	// Get unread inbox threads
	http::response resp = facy.flap(REQUEST_DELETE_FRIEND, &query, &get_query);

	if (resp.data.find("\"payload\":null", 0) != std::string::npos)
	{
		// FIXME: Remember that we deleted this contact, so we won't accidentally add him at status change
		/* facebook_user* fbu = facy.buddies.find(id);
		if (fbu != NULL)
			fbu->deleted = true; */

		MCONTACT hContact = ContactIDToHContact(id);

		// If contact wasn't deleted from database
		if (hContact != NULL) {
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
			setDword(hContact, FACEBOOK_KEY_DELETED, ::time(NULL));
		}

		NotifyEvent(m_tszUserName, TranslateT("Contact was removed from your server list."), NULL, FACEBOOK_EVENT_FRIENDSHIP);
	}
	else {
		facy.client_notify(TranslateT("Error occurred when removing contact from server."));
	}

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("DeleteContactFromServer");
}

void FacebookProto::AddContactToServer(void *data)
{
	facy.handle_entry("AddContactToServer");

	if (data == NULL)
		return;

	std::string id = *(std::string*)data;
	delete (std::string*)data;

	if (isOffline())
		return;

	std::string query = "action=add_friend&how_found=profile_button&ref_param=ts&outgoing_id=&unwanted=&logging_location=&no_flyout_on_click=false&ego_log_data=&lsd=";
	query += "&fb_dtsg=" + facy.dtsg_;
	query += "&to_friend=" + id;
	query += "&__user=" + facy.self_.user_id;

	// Get unread inbox threads
	http::response resp = facy.flap(REQUEST_ADD_FRIEND, &query);

	if (resp.data.find("\"success\":true", 0) != std::string::npos) {
		MCONTACT hContact = ContactIDToHContact(id);

		// If contact wasn't deleted from database
		if (hContact != NULL)
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_REQUEST);

		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was sent."), NULL, FACEBOOK_EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when requesting friendship."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("AddContactToServer");
}

void FacebookProto::ApproveContactToServer(void *data)
{
	facy.handle_entry("ApproveContactToServer");

	if (data == NULL)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	if (isOffline())
		return;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return;

	std::string query = "action=confirm";
	query += "&id=" + std::string(id);
	query += "&__user=" + facy.self_.user_id;
	query += "&fb_dtsg=" + facy.dtsg_;

	// Ignore friendship request
	http::response resp = facy.flap(REQUEST_FRIENDSHIP, &query);

	if (resp.data.find("\"success\":true") != std::string::npos)
	{
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was accepted."), NULL, FACEBOOK_EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when accepting friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("ApproveContactToServer");
}

void FacebookProto::CancelFriendsRequest(void *data)
{
	facy.handle_entry("CancelFriendsRequest");

	if (data == NULL)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	if (isOffline())
		return;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return;

	std::string query = "confirmed=1";
	query += "&fb_dtsg=" + facy.dtsg_;
	query += "&__user=" + facy.self_.user_id;
	query += "&friend=" + std::string(id);

	// Cancel (our) friendship request
	http::response resp = facy.flap(REQUEST_CANCEL_FRIENDSHIP, &query);

	if (resp.data.find("\"payload\":null", 0) != std::string::npos)
	{
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was canceled."), NULL, FACEBOOK_EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when canceling friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("CancelFriendsRequest");
}

void FacebookProto::IgnoreFriendshipRequest(void *data)
{
	facy.handle_entry("IgnoreFriendshipRequest");

	if (data == NULL)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	if (isOffline())
		return;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return;

	std::string query = "action=reject";
	query += "&id=" + std::string(id);
	query += "&__user=" + facy.self_.user_id;
	query += "&fb_dtsg=" + facy.dtsg_;

	// Ignore friendship request
	http::response resp = facy.flap(REQUEST_FRIENDSHIP, &query);

	if (resp.data.find("\"success\":true") != std::string::npos)
	{
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was ignored."), NULL, FACEBOOK_EVENT_FRIENDSHIP);

		// Delete this contact, if he's temporary
		if (db_get_b(hContact, "CList", "NotOnList", 0))
			CallService(MS_DB_CONTACT_DELETE, hContact);
	}
	else facy.client_notify(TranslateT("Error occurred when ignoring friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("IgnoreFriendshipRequest");
}

void FacebookProto::SendPokeWorker(void *p)
{
	facy.handle_entry("SendPokeWorker");

	if (p == NULL)
		return;

	std::string *id = (std::string*)p;

	if (isOffline()) {
		delete id;
		return;
	}

	std::string data = "poke_target=" + *id;
	data += "&do_confirm=0";
	data += "&fb_dtsg=" + facy.dtsg_;
	data += "&__user=" + facy.self_.user_id;
	data += "&ttstamp=" + facy.ttstamp_;

	// Send poke
	http::response resp = facy.flap(REQUEST_POKE, &data);

	if (resp.data.find("\"payload\":null", 0) != std::string::npos) {
		resp.data = utils::text::slashu_to_utf8(
			utils::text::source_get_value(&resp.data, 2, "__html\":\"", "\"}"));

		std::string message = utils::text::source_get_value(&resp.data, 4, "<img", "<div", ">", "<\\/div>");

		if (message.empty()) // message has different format, show whole message
			message = resp.data;

		message = utils::text::html_entities_decode(
			utils::text::remove_html(message));

		ptrT tmessage(mir_utf8decodeT(message.c_str()));
		NotifyEvent(m_tszUserName, tmessage, NULL, FACEBOOK_EVENT_OTHER);
	}

	facy.handle_success("SendPokeWorker");

	delete id;
}


HANDLE FacebookProto::GetAwayMsg(MCONTACT)
{
	return 0; // Status messages are disabled
}

int FacebookProto::OnContactDeleted(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;

	// Remove this contact from caches
	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (id)
		facy.user_id_to_hcontact.erase(std::string(id));

	ptrA tid(getStringA(hContact, FACEBOOK_KEY_TID));
	if (tid)
		facy.thread_id_to_user_id.erase(std::string(tid));

	if (isChatRoom(hContact)) {
		ptrA chat_id(getStringA(hContact, "ChatRoomID"));
		if (chat_id)
			facy.chat_id_to_hcontact.erase(std::string(chat_id));
	}

	// Cancel friendship (with confirmation)
	CancelFriendship(hContact, 1);

	return 0;
}


void FacebookProto::StartTyping(MCONTACT hContact) {
	// ignore if contact is already typing
	if (facy.typers.find(hContact) != facy.typers.end())
		return;

	// show notification and insert into typing set
	CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)FACEBOOK_TYPING_TIME);
	facy.typers.insert(hContact);
}

void FacebookProto::StopTyping(MCONTACT hContact) {
	// ignore if contact is not typing
	if (facy.typers.find(hContact) == facy.typers.end())
		return;

	// show notification and remove from typing set
	CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
	facy.typers.erase(hContact);
}