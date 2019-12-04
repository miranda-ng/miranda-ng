/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

void FacebookProto::SaveName(MCONTACT hContact, const facebook_user *fbu)
{
	if (fbu->type == CONTACT_PAGE) {
		// Page has only nickname and no first/last names
		std::string nick = m_pagePrefix + " " + fbu->real_name;

		setStringUtf(hContact, FACEBOOK_KEY_NICK, nick.c_str());
		delSetting(hContact, FACEBOOK_KEY_FIRST_NAME);
		delSetting(hContact, FACEBOOK_KEY_SECOND_NAME);
		delSetting(hContact, FACEBOOK_KEY_LAST_NAME);
		return;
	}

	// Save nick
	std::string nick = fbu->real_name;
	if (!getBool(FACEBOOK_KEY_NAME_AS_NICK, DEFAULT_NAME_AS_NICK) && !fbu->nick.empty())
		nick = fbu->nick;

	setStringUtf(hContact, FACEBOOK_KEY_NICK, nick.c_str());

	// Explode whole name into first, second and last name
	std::vector<std::string> names;
	utils::text::explode(fbu->real_name, " ", &names);

	setStringUtf(hContact, FACEBOOK_KEY_FIRST_NAME, names.size() > 0 ? names.front().c_str() : "");
	setStringUtf(hContact, FACEBOOK_KEY_LAST_NAME, names.size() > 1 ? names.back().c_str() : "");

	std::string middle;
	if (names.size() > 2) {
		for (std::string::size_type i = 1; i < names.size() - 1; i++) {
			if (!middle.empty())
				middle += " ";

			middle += names.at(i);
		}
	}
	setStringUtf(hContact, FACEBOOK_KEY_SECOND_NAME, middle.c_str());
}

bool FacebookProto::IsMyContact(MCONTACT hContact, bool include_chat)
{
	const char *proto = Proto_GetBaseAccountName(hContact);
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
		if (db_is_contact((WPARAM)it->second) == 1)
			return it->second;
		else
			facy.chat_id_to_hcontact.erase(it);
	}

	// Go through all local contacts
	for (auto &hContact : AccContacts()) {
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
		if (db_is_contact((WPARAM)it->second) == 1)
			return it->second;
		else
			facy.user_id_to_hcontact.erase(it);
	}

	// Go through all local contacts
	for (auto &hContact : AccContacts()) {
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
	for (auto &hContact : AccContacts()) {
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

	http::response resp = facy.sendRequest(facy.threadInfoRequest(true, thread_id.c_str()));

	std::string user_id;

	if (resp.code == HTTP_CODE_OK) {
		if (ParseThreadInfo(&resp.data, &user_id) == EXIT_SUCCESS) {
			if (!user_id.empty())
				facy.thread_id_to_user_id.insert(std::make_pair(thread_id, user_id));

			debugLogA("*** Thread info processed");
		}
		else debugLogA("*** Error processing thread info");
	}

	return user_id;
}

void FacebookProto::LoadContactInfo(facebook_user* fbu)
{
	if (isOffline())
		return;

	LIST<char> userIds(1);
	userIds.insert(mir_strdup(fbu->user_id.c_str()));

	http::response resp = facy.sendRequest(facy.userInfoRequest(userIds));

	FreeList(userIds);
	userIds.destroy();

	if (resp.code == HTTP_CODE_OK) {
		if (ParseUserInfo(&resp.data, fbu) == EXIT_SUCCESS)
			debugLogA("*** Contact thread info processed");
		else
			debugLogA("*** Error processing contact thread info");
	}
}

MCONTACT FacebookProto::AddToContactList(facebook_user* fbu, bool force_add, bool add_temporarily)
{
	// Ignore self user completely
	if (fbu->user_id == facy.self_.user_id)
		return 0;

	// First, check if this contact exists (and if does, just return it)
	if (!force_add) {
		MCONTACT hContact = ContactIDToHContact(fbu->user_id);

		if (hContact)
			return hContact;
	}

	// Try to make a new contact
	MCONTACT hContact = db_add_contact();

	if (hContact && Proto_AddToContact(hContact, m_szModuleName) != 0) {
		db_delete_contact(hContact);
		hContact = 0;
	}

	// If we have some contact, we'll save its data
	if (hContact) {
		// Save these values only when adding new contact, not when updating existing
		if (add_temporarily) {
			Contact_Hide(hContact);
			Contact_RemoveFromList(hContact);
		}

		setString(hContact, FACEBOOK_KEY_ID, fbu->user_id.c_str());

		std::string homepage = FACEBOOK_URL_PROFILE + fbu->user_id;
		setString(hContact, "Homepage", homepage.c_str());
		setWString(hContact, "MirVer", fbu->getMirVer());

		db_unset(hContact, "CList", "MyHandle");

		Clist_SetGroup(hContact, m_tszDefaultGroup);

		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, fbu->type);

		if (getByte(FACEBOOK_KEY_DISABLE_STATUS_NOTIFY, 0))
			Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);

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

void FacebookProto::DeleteContactFromServer(void *data)
{
	facy.handle_entry("DeleteContactFromServer");

	if (data == nullptr)
		return;

	std::string id = *(std::string*)data;
	delete (std::string*)data;

	if (isOffline())
		return;

	// Delete contact from server
	http::response resp = facy.sendRequest(facy.deleteFriendRequest(id.c_str()));
	if (resp.data.find("\"payload\":null", 0) != std::string::npos) {
		// If contact wasn't deleted from database
		MCONTACT hContact = ContactIDToHContact(id);
		if (hContact != 0) {
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
			setDword(hContact, FACEBOOK_KEY_DELETED, ::time(0));
		}

		NotifyEvent(m_tszUserName, TranslateT("Contact was removed from your server list."), 0, EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when removing contact from server."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("DeleteContactFromServer");
}

void FacebookProto::AddContactToServer(void *data)
{
	facy.handle_entry("AddContactToServer");

	if (data == nullptr)
		return;

	std::string id = *(std::string*)data;
	delete (std::string*)data;

	if (isOffline())
		return;

	// Request friendship
	http::response resp = facy.sendRequest(facy.addFriendRequest(id.c_str()));

	if (resp.data.find("\"success\":true", 0) != std::string::npos) {
		MCONTACT hContact = ContactIDToHContact(id);

		// If contact wasn't deleted from database
		if (hContact != 0)
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_REQUEST);

		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was sent."), 0, EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when requesting friendship."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("AddContactToServer");
}

void FacebookProto::ApproveContactToServer(void *data)
{
	facy.handle_entry("ApproveContactToServer");

	if (data == nullptr)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	if (isOffline())
		return;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return;

	// Confirm friendship request
	http::response resp = facy.sendRequest(facy.answerFriendshipRequest(id, true));

	if (resp.data.find("\"success\":true") != std::string::npos) {
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was accepted."), 0, EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when accepting friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("ApproveContactToServer");
}

void FacebookProto::CancelFriendsRequest(void *data)
{
	facy.handle_entry("CancelFriendsRequest");

	if (data == nullptr)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	if (isOffline())
		return;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return;

	// Cancel (our) friendship request
	http::response resp = facy.sendRequest(facy.cancelFriendshipRequest(id));
	if (resp.data.find("\"payload\":null", 0) != std::string::npos) {
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was canceled."), 0, EVENT_FRIENDSHIP);
	}
	else facy.client_notify(TranslateT("Error occurred when canceling friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("CancelFriendsRequest");
}

void FacebookProto::IgnoreFriendshipRequest(void *data)
{
	facy.handle_entry("IgnoreFriendshipRequest");

	if (data == nullptr)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	if (isOffline())
		return;

	ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (!id)
		return;

	// Ignore friendship request
	http::response resp = facy.sendRequest(facy.answerFriendshipRequest(id, false));

	if (resp.data.find("\"success\":true") != std::string::npos) {
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
		NotifyEvent(m_tszUserName, TranslateT("Request for friendship was ignored."), 0, EVENT_FRIENDSHIP);

		// Delete this contact, if he's temporary
		if (!Contact_OnList(hContact))
			db_delete_contact(hContact);
	}
	else facy.client_notify(TranslateT("Error occurred when ignoring friendship request."));

	if (resp.code != HTTP_CODE_OK)
		facy.handle_error("IgnoreFriendshipRequest");
}

void FacebookProto::SendPokeWorker(void *p)
{
	facy.handle_entry("SendPokeWorker");

	if (p == nullptr)
		return;

	std::string *id = (std::string*)p;

	if (isOffline()) {
		delete id;
		return;
	}

	// Send poke
	http::response resp = facy.sendRequest(facy.sendPokeRequest(id->c_str()));

	if (resp.data.find("\"payload\":null", 0) != std::string::npos) {
		resp.data = utils::text::slashu_to_utf8(
			utils::text::source_get_value(&resp.data, 2, "__html\":\"", "\"}"));

		std::string message = utils::text::source_get_value(&resp.data, 4, "<img", "<div", ">", "<\\/div>");

		if (message.empty()) // message has different format, show whole message
			message = resp.data;

		message = utils::text::html_entities_decode(
			utils::text::remove_html(message));

		ptrW tmessage(mir_utf8decodeW(message.c_str()));
		NotifyEvent(m_tszUserName, tmessage, 0, EVENT_OTHER);
	}

	facy.handle_success("SendPokeWorker");

	delete id;
}

void FacebookProto::RefreshUserInfo(void *data)
{
	if (data == nullptr)
		return;

	MCONTACT hContact = *(MCONTACT*)data;
	delete (MCONTACT*)data;

	ptrA user_id(getStringA(hContact, FACEBOOK_KEY_ID));
	if (user_id == nullptr || isOffline()) {
		ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, nullptr);
		return;
	}

	facebook_user fbu;
	fbu.user_id = user_id;

	LoadContactInfo(&fbu);

	// TODO: don't duplicate code this way, refactor all this userInfo loading
	// TODO: load more info about user (authorization state,...)

	std::string homepage = FACEBOOK_URL_PROFILE + fbu.user_id;
	setString(hContact, "Homepage", homepage.c_str());

	if (!fbu.real_name.empty())
		SaveName(hContact, &fbu);

	if (fbu.gender)
		setByte(hContact, "Gender", fbu.gender);

	int oldType = getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);
	// From server we won't get request/approve types, only none, so we don't want to overwrite and lost it in that case
	if (fbu.type != CONTACT_NONE || (oldType != CONTACT_REQUEST && oldType != CONTACT_APPROVE)) {
		setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, fbu.type);
	}

	// If this contact is page, set it as invisible (if enabled in options)
	if (getBool(FACEBOOK_KEY_PAGES_ALWAYS_ONLINE, DEFAULT_PAGES_ALWAYS_ONLINE) && fbu.type == CONTACT_PAGE)
		setWord(hContact, "Status", ID_STATUS_INVISIBLE);

	CheckAvatarChange(hContact, fbu.image_url);

	// Load additional info from profile page (e.g., birthday)
	http::response resp = facy.sendRequest(facy.profileInfoRequest(fbu.user_id.c_str()));

	if (resp.code == HTTP_CODE_OK) {
		std::string birthday = utils::text::source_get_value(&resp.data, 4, ">Birthday</", "<td", ">", "</td>");
		birthday = utils::text::remove_html(birthday);

		std::string::size_type pos = birthday.find(" ");
		std::string::size_type pos2 = birthday.find(",");
		if (pos != std::string::npos) {
			std::string month = birthday.substr(0, pos);
			std::string day = birthday.substr(pos + 1, pos2 != std::string::npos ? pos2 - pos - 1 : std::string::npos);

			setByte(hContact, "BirthDay", atoi(day.c_str()));

			const static char *months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
			for (int i = 0; i < 12; i++) {
				if (!mir_strcmp(months[i], month.c_str())) {
					setByte(hContact, "BirthMonth", i + 1);
					break;
				}
			}

			if (pos2 != std::string::npos) {
				std::string year = birthday.substr(pos2 + 2, 4);
				setWord(hContact, "BirthYear", atoi(year.c_str()));
			}
			else // We have to set ANY year, otherwise UserInfoEx shows completely wrong date
				setWord(hContact, "BirthYear", 1800);
		}
	}

	ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, nullptr);
}

void FacebookProto::OnContactDeleted(MCONTACT hContact)
{
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
}


void FacebookProto::StartTyping(MCONTACT hContact)
{
	// ignore if contact is already typing
	if (facy.typers.find(hContact) != facy.typers.end())
		return;

	// show notification and insert into typing set
	CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)FACEBOOK_TYPING_TIME);
	facy.typers.insert(hContact);
}

void FacebookProto::StopTyping(MCONTACT hContact)
{
	// ignore if contact is not typing
	if (facy.typers.find(hContact) == facy.typers.end())
		return;

	// show notification and remove from typing set
	CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
	facy.typers.erase(hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// handling friends requests

HttpRequest* facebook_client::addFriendRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/add_friend/action.php");
	
	p << INT_PARAM("__a", 1);

	p->Body
		<< CHAR_PARAM("to_friend", userId)
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("action", "add_friend")
		<< CHAR_PARAM("how_found", "profile_button")
		<< CHAR_PARAM("ref_param", "ts")
		<< CHAR_PARAM("no_flyout_on_click", "false");

	return p;
}

HttpRequest* facebook_client::deleteFriendRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/profile/removefriendconfirm.php");

	p << INT_PARAM("__a", 1)
		<< CHAR_PARAM("norefresh", "true")
		<< CHAR_PARAM("unref", "button_dropdown")
		<< CHAR_PARAM("uid", userId);

	p->Body
		<< CHAR_PARAM("uid", userId)
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("norefresh", "true")
		<< CHAR_PARAM("unref", "button_dropdown")
		<< INT_PARAM("confirmed", 1)
		<< INT_PARAM("__a", 1);

	return p;
}

HttpRequest* facebook_client::getFriendshipsRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_GET, FORMAT, "%s/friends/center/requests/", mbasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE);

	p->flags |= NLHRF_REDIRECT;

	return p;
}

HttpRequest* facebook_client::cancelFriendshipRequest(const char *userId)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/friends/requests/cancel.php");

	p << INT_PARAM("__a", 1);

	p->Body
		<< INT_PARAM("confirmed", 1)
		<< CHAR_PARAM("friend", userId)
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str());
	
	return p;
}

HttpRequest* facebook_client::answerFriendshipRequest(const char *userId, bool bConfirm)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/requests/friends/ajax/");

	p << INT_PARAM("__a", 1);

	p->Body
		<< CHAR_PARAM("action", (bConfirm) ? "confirm" : "reject")
		<< CHAR_PARAM("id", userId)
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str());
	
	return p;
}

/////////////////////////////////////////////////////////////////////////////////////////
// requesting user info

HttpRequest* facebook_client::userInfoRequest(const LIST<char> &userIds)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/chat/user_info/");

	p << INT_PARAM("dpr", 1);

	for (int i = 0; i < userIds.getCount(); i++) {
		CMStringA id(::FORMAT, "ids[%i]", i);
		p->Body << CHAR_PARAM(id, userIds[i]);
	}

	p->Body
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT");
	
	return p;
}

HttpRequest* facebook_client::userInfoAllRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/chat/user_info_all/");

	p << INT_PARAM("dpr", 1)
		<< CHAR_PARAM("viewer", self_.user_id.c_str());

	p->Body
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", -1);

	return p;
}

HttpRequest* facebook_client::buddylistUpdate()
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_MOBILE "/buddylist_update.php");
    
	p->Body
		<< CHAR_PARAM("data_fetch", "true")
		<< CHAR_PARAM("m_sess", "")
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("jazoest", "21824")
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__ajax__", "")
		<< CHAR_PARAM("__user", self_.user_id.c_str());

	// send_full_data=true sends more data

	return p;
}
