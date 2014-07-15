/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-13 Robert P�sel

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

void updateStringUtf(FacebookProto *proto, MCONTACT hContact, const char *key, std::string value) {
	bool update_required = true;
	
	DBVARIANT dbv;
	if (!proto->getStringUtf(hContact, key, &dbv)) {
		update_required = strcmp(dbv.pszVal, value.c_str()) != 0;
		db_free(&dbv);
	}

	if (update_required) {
		proto->setStringUtf(hContact, key, value.c_str());
	}
}

void FacebookProto::SaveName(MCONTACT hContact, const facebook_user *fbu)
{
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

	std::string middle = "";
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
	if (proto && !strcmp(m_szModuleName, proto)) {
		if (include_chat)
			return true;
		return !isChatRoom(hContact);
	}
	return false;
}

MCONTACT FacebookProto::ChatIDToHContact(std::tstring chat_id)
{
	// TODO: use some cache to optimize this

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!IsMyContact(hContact, true))
			continue;

		ptrT id(getTStringA(hContact, "ChatRoomID"));
		if (id && !_tcscmp(id, chat_id.c_str()))
			return hContact;
	}

	return 0;
}

MCONTACT FacebookProto::ContactIDToHContact(std::string user_id)
{
	// TODO: use some cache to optimize this

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
		if (id && !strcmp(id, user_id.c_str()))
			return hContact;
	}

	return 0;
}

std::string FacebookProto::ThreadIDToContactID(std::string thread_id)
{
	std::string user_id;

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (!IsMyContact(hContact))
			continue;

		ptrA tid(getStringA(hContact, FACEBOOK_KEY_TID));
		if (tid && !strcmp(tid, thread_id.c_str())) {
			user_id = ptrA( getStringA(hContact, FACEBOOK_KEY_ID));
			return user_id;
		}
	}
	
	// We don't have any contact with thish thread_id cached, we must ask server	

	std::string data = "client=mercury";
	data += "&__user=" + facy.self_.user_id;
	data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
	data += "&__a=1&__dyn=&__req=&ttstamp=0";
	data += "&threads[thread_ids][0]=" + utils::url::encode(thread_id);

	http::response resp = facy.flap(REQUEST_THREAD_INFO, &data);

	if (resp.code == HTTP_CODE_OK) {
		CODE_BLOCK_TRY

		facebook_json_parser* p = new facebook_json_parser(this);
		p->parse_thread_info(&resp.data, &user_id);
		delete p;

		debugLogA("***** Thread info processed");

		CODE_BLOCK_CATCH

		debugLogA("***** Error processing thread info: %s", e.what());

		CODE_BLOCK_END
	}

	return user_id;
}

void FacebookProto::LoadContactInfo(facebook_user* fbu)
{
	// TODO: support for more friends at once
	std::string get_query = "&ids[0]=" + utils::url::encode(fbu->user_id);
	
	http::response resp = facy.flap(REQUEST_LOAD_FRIEND, NULL, &get_query);

	if (resp.code == HTTP_CODE_OK) {
		CODE_BLOCK_TRY

		facebook_json_parser* p = new facebook_json_parser(this);
		p->parse_user_info(&resp.data, fbu);
		delete p;

		debugLogA("***** Contact thread info processed");

		CODE_BLOCK_CATCH

			debugLogA("***** Error processing contact thread info: %s", e.what());

		CODE_BLOCK_END
	}
}

void FacebookProto::LoadParticipantsNames(facebook_chatroom *fbc)
{	
	for (std::map<std::string, std::string>::iterator it = fbc->participants.begin(); it != fbc->participants.end(); ++it) {
		if (it->second.empty()) {
			if (!strcmp(it->first.c_str(), facy.self_.user_id.c_str()))
				it->second = facy.self_.real_name;
			else {
				MCONTACT hContact = ContactIDToHContact(it->first.c_str());
				if (hContact != NULL) {
					DBVARIANT dbv;
					if (!getStringUtf(hContact, FACEBOOK_KEY_NICK, &dbv)) {
						it->second = dbv.pszVal;
						db_free(&dbv);
					}
					// TODO: set correct role (friend/user) for this contact here - need rework participants map to <id, participant>
				}

				// TODO: load unknown contact's names from server
				if (it->second.empty())
					it->second = it->first;
			}
		}
	}
}

void FacebookProto::LoadChatInfo(facebook_chatroom *fbc)
{
	std::string data = "client=mercury";
	data += "&__user=" + facy.self_.user_id;
	data += "&fb_dtsg=" + (!facy.dtsg_.empty() ? facy.dtsg_ : "0");
	data += "&__a=1&__dyn=&__req=&ttstamp=0";

	std::string thread_id = utils::url::encode(std::string(_T2A(fbc->thread_id.c_str())));

	// request info about thread
	data += "&threads[thread_ids][0]=" + thread_id;

	http::response resp = facy.flap(REQUEST_THREAD_INFO, &data);

	if (resp.code == HTTP_CODE_OK) {

		CODE_BLOCK_TRY

		facebook_json_parser* p = new facebook_json_parser(this);
		p->parse_chat_info(&resp.data, fbc);
		delete p;

		// Load missing participants names
		LoadParticipantsNames(fbc);

		// If chat has no name, create name from participants list
		if (fbc->chat_name.empty()) {
			unsigned int namesCount = 3; // how many names should be in room name; max. 5

			for (std::map<std::string, std::string>::iterator it = fbc->participants.begin(); it != fbc->participants.end(); ++it) {
				if (it->second.empty())
					continue;

				if (!fbc->chat_name.empty())
					fbc->chat_name += _T(", ");

				std::string name;
				std::string::size_type pos;
				if ((pos = it->second.find(" ")) != std::string::npos) {
					name = it->second.substr(0, pos);
				}
				else {
					name = it->second;
				}

				fbc->chat_name += _A2T(name.c_str());
			}

			if (fbc->participants.size() > namesCount) {
				TCHAR more[200];
				mir_sntprintf(more, SIZEOF(more), TranslateT("%s and more (%d)"), fbc->chat_name.c_str(), fbc->participants.size() - namesCount);
				fbc->chat_name = more;
			}

			// If there are no participants to create a name from, use just thread_id
			if (fbc->chat_name.empty())
				fbc->chat_name = fbc->thread_id; // TODO: is this needed? Isn't it showed automatically as id if there is no name?
		}

		//ReceiveMessages(messages, local_timestamp, true); // don't let it fall into infinite cycle, solve it somehow...

		debugLogA("***** Chat thread info processed");

		CODE_BLOCK_CATCH

			debugLogA("***** Error processing chat thread info: %s", e.what());

		CODE_BLOCK_END

			facy.handle_success("LoadChatInfo");
	}
	else {
		facy.handle_error("LoadChatInfo");
	}

}

MCONTACT FacebookProto::AddToContactList(facebook_user* fbu, ContactType type, bool force_add)
{
	// First, check if this contact exists (and if does, just return it)
	if (!force_add) {
		MCONTACT hContact = ContactIDToHContact(fbu->user_id);

		if (hContact)
			return hContact;
	}

	// Try to make a new contact
	MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);

	if (hContact && CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName) != 0) {
		CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		hContact = NULL;
	}

	// If we have some contact, we'll save its data
	if (hContact) {
		// Save these values only when adding new contact, not when updating existing
		setString(hContact, FACEBOOK_KEY_ID, fbu->user_id.c_str());

		std::string homepage = FACEBOOK_URL_PROFILE + fbu->user_id;
		setString(hContact, "Homepage", homepage.c_str());
		setTString(hContact, "MirVer", fbu->getMirVer());

		db_unset(hContact, "CList", "MyHandle");

		ptrT group(getTStringA(NULL, FACEBOOK_KEY_DEF_GROUP));
		if (group)
			db_set_ts(hContact, "CList", "Group", group);

		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, type);

		if (getByte(FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, 0))
			CallService(MS_IGNORE_IGNORE, hContact, (LPARAM)IGNOREEVENT_USERONLINE);

		if (!fbu->real_name.empty())
			SaveName(hContact, fbu);

		if (!fbu->username.empty())
			setString(hContact, FACEBOOK_KEY_USERNAME, fbu->username.c_str());

		if (fbu->gender)
			setByte(hContact, "Gender", fbu->gender);

		CheckAvatarChange(hContact, fbu->image_url);
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

	std::string id = (*(std::string*)data);
	delete data;

	std::string query = "norefresh=true&unref=button_dropdown&confirmed=1&phstamp=0&__a=1";
	query += "&fb_dtsg=" + facy.dtsg_;
	query += "&uid=" + id;
	query += "&__user=" + facy.self_.user_id;

	std::string get_query = "norefresh=true&unref=button_dropdown&uid=" + id;

	// Get unread inbox threads
	http::response resp = facy.flap(REQUEST_DELETE_FRIEND, &query, &get_query);

	if (resp.data.find("\"payload\":null", 0) != std::string::npos)
	{
		facebook_user* fbu = facy.buddies.find(id);
		if (fbu != NULL)
			fbu->deleted = true;

		MCONTACT hContact = ContactIDToHContact(id);

		// If contact wasn't deleted from database
		if (hContact != NULL) {
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
			setDword(hContact, FACEBOOK_KEY_DELETED, ::time(NULL));
		}

		NotifyEvent(m_tszUserName, TranslateT("Contact was removed from your server list."), NULL, FACEBOOK_EVENT_OTHER);
	} else {
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

	std::string id = (*(std::string*)data);
	delete data;

	std::string query = "action=add_friend&how_found=profile_button&ref_param=ts&outgoing_id=&unwanted=&logging_location=&no_flyout_on_click=false&ego_log_data=&lsd=";
	query += "&fb_dtsg=" + facy.dtsg_;
	query += "&to_friend=" + id;
	query += "&__user=" + facy.self_.user_id;

	// Get unread inbox threads
	http::response resp = facy.flap(REQUEST_REQUEST_FRIEND, &query);

	if (resp.data.find("\"success\":true", 0) != std::string::npos) {
		MCONTACT hContact = ContactIDToHContact(id);

		// If contact wasn't deleted from database
		if (hContact != NULL)
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_REQUEST);

		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was sent."), NULL, FACEBOOK_EVENT_OTHER);
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
	delete data;

	std::string post_data = "fb_dtsg=" + facy.dtsg_;
	post_data += "&charset_test=%e2%82%ac%2c%c2%b4%2c%e2%82%ac%2c%c2%b4%2c%e6%b0%b4%2c%d0%94%2c%d0%84&confirm_button=";

	std::string get_data = "id=";

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	get_data += id;

	http::response resp = facy.flap(REQUEST_APPROVE_FRIEND, &post_data, &get_data);

	setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
}

void FacebookProto::CancelFriendsRequest(void *data)
{
	facy.handle_entry("CancelFriendsRequest");

	if (data == NULL)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete data;

	std::string query = "phstamp=0&confirmed=1";
	query += "&fb_dtsg=" + facy.dtsg_;
	query += "&__user=" + facy.self_.user_id;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	query += "&friend=" + std::string(id);

	// Get unread inbox threads
	http::response resp = facy.flap(REQUEST_CANCEL_REQUEST, &query);

	if (resp.data.find("\"payload\":null", 0) != std::string::npos)
	{
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was canceled."), NULL, FACEBOOK_EVENT_OTHER);
	}
	else facy.client_notify(TranslateT("Error occurred when canceling friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("CancelFriendsRequest");
}

void FacebookProto::SendPokeWorker(void *p)
{
	facy.handle_entry("SendPokeWorker");

	if (p == NULL)
		return;

	std::string id = (*(std::string*)p);
	delete p;

	std::string data = "poke_target=" + id;
	data += "&do_confirm=0&phstamp=0";
	data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
	data += "&__user=" + facy.self_.user_id;

	// Send poke
	http::response resp = facy.flap(REQUEST_POKE, &data);

	if (resp.data.find("\"payload\":null", 0) != std::string::npos) {
		resp.data = utils::text::slashu_to_utf8(
				utils::text::source_get_value(&resp.data, 2, "__html\":\"", "\"}"));

		std::string message = utils::text::source_get_value(&resp.data, 4, "<img", "<div", ">", "<\\/div>");

		if (message.empty()) // message has different format, show whole message
			message = resp.data;

		message = utils::text::special_expressions_decode(
			utils::text::remove_html(message));

		ptrT tmessage( mir_utf8decodeT(message.c_str()));
		NotifyEvent(m_tszUserName, tmessage, NULL, FACEBOOK_EVENT_OTHER);
	}

	facy.handle_success("SendPokeWorker");
}


HANDLE FacebookProto::GetAwayMsg(MCONTACT hContact)
{
	return 0; // Status messages are disabled
}

int FacebookProto::OnContactDeleted(WPARAM wParam,LPARAM)
{
	CancelFriendship(wParam, 1);

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