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

#define MAX_NEWSFEED_LEN 500

void FacebookProto::ProcessBuddyList(void*)
{
	ScopedLock s(facy.buddies_lock_);

	if (isOffline())
		return;

	facy.handle_entry("ProcessBuddyList");

	// Prepare update data
	std::string post_data = "user=" + facy.self_.user_id + "&fetch_mobile=true&phstamp=0&fb_dtsg=" + facy.dtsg_ + "&__user=" + facy.self_.user_id + "&cached_user_info_ids=";
	
	int counter = 0;
	for (List::Item< facebook_user >* i = facy.buddies.begin(); i != NULL; i = i->next, counter++)
	{
		post_data += i->data->user_id + "%2C";
	}

	// Get buddy list
	http::response resp = facy.flap(REQUEST_BUDDY_LIST, &post_data);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("buddy_list");
		return;
	}

	debugLogA("***** Starting processing buddy list");

	CODE_BLOCK_TRY

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_buddy_list(&resp.data, &facy.buddies);
	delete p;

	for (List::Item< facebook_user >* i = facy.buddies.begin(); i != NULL;)
	{		
		facebook_user* fbu = i->data;

		std::string status;
		switch (fbu->status_id) {
			case ID_STATUS_OFFLINE:
				status = "Offline"; break;
			case ID_STATUS_ONLINE:
				status = "Online"; break;
			case ID_STATUS_ONTHEPHONE:
				status = "Mobile"; break;
		}
		
		if (fbu->idle)
			status += " (idle)";

		debugLogA("      Now %s: %s", status.c_str(), fbu->user_id.c_str());

		if (!fbu->deleted)
		{
			if (!fbu->handle) // just been added
				fbu->handle = AddToContactList(fbu, CONTACT_FRIEND);
			
			ptrT client( getTStringA(fbu->handle, "MirVer"));
			if (!client || _tcscmp(client, fbu->getMirVer()))
				setTString(fbu->handle, "MirVer", fbu->getMirVer());

			if (getDword(fbu->handle, "IdleTS", 0) != fbu->last_active) {
				if ((fbu->idle || fbu->status_id == ID_STATUS_OFFLINE) && fbu->last_active > 0)
					setDword(fbu->handle, "IdleTS", fbu->last_active);
				else
					delSetting(fbu->handle, "IdleTS");
			}
		}

		if (fbu->status_id == ID_STATUS_OFFLINE || fbu->deleted)
		{
			if (fbu->handle)
				setWord(fbu->handle, "Status", ID_STATUS_OFFLINE);

			std::string to_delete(i->key);
			i = i->next;
			facy.buddies.erase(to_delete);
		} else {
			i = i->next;

			if (!fbu->handle) // just been added
				fbu->handle = AddToContactList(fbu, CONTACT_FRIEND);

			if (getWord(fbu->handle, "Status", 0) != fbu->status_id)
				setWord(fbu->handle, "Status", fbu->status_id);

			if (getDword(fbu->handle, "LastActiveTS", 0) != fbu->last_active) {
				if (fbu->last_active > 0)
					setDword(fbu->handle, "LastActiveTS", fbu->last_active);
				else
					delSetting(fbu->handle, "LastActiveTS");
			}

			if (getByte(fbu->handle, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND) {
				setByte(fbu->handle, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
				// TODO: remove that popup and use "Contact added you" event?
			}

			// Wasn't contact removed from "server-list" someday?
			if (getDword(fbu->handle, FACEBOOK_KEY_DELETED, 0)) {
				delSetting(fbu->handle, FACEBOOK_KEY_DELETED);

				std::string url = FACEBOOK_URL_PROFILE + fbu->user_id;

				ptrT szTitle( mir_utf8decodeT(fbu->real_name.c_str()));
				NotifyEvent(szTitle, TranslateT("Contact is back on server-list."), fbu->handle, FACEBOOK_EVENT_OTHER, &url);
			}

			// Check avatar change
			CheckAvatarChange(fbu->handle, fbu->image_url);
		}
	}

	debugLogA("***** Buddy list processed");

	CODE_BLOCK_CATCH

	debugLogA("***** Error processing buddy list: %s", e.what());

	CODE_BLOCK_END
}

void FacebookProto::ProcessFriendList(void*)
{
	ScopedLock s(facy.buddies_lock_);

	if (isOffline())
		return;

	facy.handle_entry("load_friends");

	// Get buddy list
	http::response resp = facy.flap(REQUEST_LOAD_FRIENDS);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("load_friends");
		return;
	}	

	debugLogA("***** Starting processing friend list");

	CODE_BLOCK_TRY

	std::map<std::string, facebook_user*> friends;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_friends(&resp.data, &friends);
	delete p;


	// Check and update old contacts
	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if ( isChatRoom(hContact))
			continue;

		DBVARIANT dbv;
		facebook_user *fbu;
		if (!getString(hContact, FACEBOOK_KEY_ID, &dbv)) {
			std::string id = dbv.pszVal;
			db_free(&dbv);
			
			std::map< std::string, facebook_user* >::iterator iter;
			
			if ((iter = friends.find(id)) != friends.end()) {
				// Found contact, update it and remove from map
				fbu = iter->second;

				// TODO RM: remove, because contacts cant change it, so its only for "first run"
					// - but what with contacts, that was added after logon?
				// Update gender
				if (getByte(hContact, "Gender", 0) != fbu->gender)
					setByte(hContact, "Gender", fbu->gender);
				
				// TODO: remove this in some future version?
				// Remove old useless "RealName" field
				ptrA realname(getStringA(hContact, "RealName"));
				if (realname != NULL) {
					delSetting(hContact, "RealName");
				}

				// Update real name and nick
				if (!fbu->real_name.empty()) {
					SaveName(hContact, fbu);
				}

				if (getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND) {
					setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
					// TODO: remove that popup and use "Contact added you" event?
				}

				// Wasn't contact removed from "server-list" someday?
				if (getDword(hContact, FACEBOOK_KEY_DELETED, 0)) {
					delSetting(hContact, FACEBOOK_KEY_DELETED);

					std::string url = FACEBOOK_URL_PROFILE + fbu->user_id;					

					ptrT szTitle( mir_utf8decodeT(fbu->real_name.c_str()));
					NotifyEvent(szTitle, TranslateT("Contact is back on server-list."), hContact, FACEBOOK_EVENT_OTHER, &url);
				}

				// Check avatar change
				CheckAvatarChange(hContact, fbu->image_url);

				// Mark this contact as deleted ("processed") and delete them later (as there may be some duplicit contacts to use)
				fbu->deleted = true;
			} else {
				// Contact was removed from "server-list", notify it

				// Wasnt we already been notified about this contact? And was this real friend?
				if (!getDword(hContact, FACEBOOK_KEY_DELETED, 0) && getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) == CONTACT_FRIEND) {
					setDword(hContact, FACEBOOK_KEY_DELETED, ::time(NULL));
					setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);

					std::string contactname = id;
					if (!getStringUtf(hContact, FACEBOOK_KEY_NICK, &dbv)) {
						contactname = dbv.pszVal;
						db_free(&dbv);
					}

					std::string url = FACEBOOK_URL_PROFILE + id;

					ptrT szTitle( mir_utf8decodeT(contactname.c_str()));
					NotifyEvent(szTitle, TranslateT("Contact is no longer on server-list."), hContact, FACEBOOK_EVENT_OTHER, &url);
				}
			}
		}
	}

	// Check remaining contacts in map and add them to contact list
	for (std::map< std::string, facebook_user* >::iterator it = friends.begin(); it != friends.end(); ) {
		if (!it->second->deleted)
			AddToContactList(it->second, CONTACT_FRIEND, true); // we know this contact doesn't exists, so we force add it

		delete it->second;
		it = friends.erase(it);
	}
	friends.clear();

	debugLogA("***** Friend list processed");

	CODE_BLOCK_CATCH

	debugLogA("***** Error processing friend list: %s", e.what());

	CODE_BLOCK_END
}

void FacebookProto::ProcessUnreadMessages(void*)
{
	facy.handle_entry("ProcessUnreadMessages");

	// receive messages from all folders by default, use hidden setting to receive only inbox messages
	bool inboxOnly = getBool(FACEBOOK_KEY_INBOX_ONLY, 0);

	std::string data = "folders[0]=inbox";
	if (!inboxOnly)
		data += "&folders[1]=other";
	data += "&client=mercury";
	data += "__user=" + facy.self_.user_id;
	data += "&fb_dtsg=" + (!facy.dtsg_.empty() ? facy.dtsg_ : "0");
	data += "&__a=1&__dyn=&__req=&ttstamp=0";

	http::response resp = facy.flap(REQUEST_UNREAD_THREADS, &data);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("ProcessUnreadMessages");
		return;
	}
	
	CODE_BLOCK_TRY
		
	std::vector<std::string> threads;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_unread_threads(&resp.data, &threads, inboxOnly);
	delete p;

	ForkThread(&FacebookProto::ProcessUnreadMessage, new std::vector<std::string>(threads));

	debugLogA("***** Unread threads list processed");

	CODE_BLOCK_CATCH

	debugLogA("***** Error processing unread threads list: %s", e.what());

	CODE_BLOCK_END
	
	facy.handle_success("ProcessUnreadMessages");
}

void FacebookProto::ProcessUnreadMessage(void *p)
{
	if (p == NULL)
		return;

	facy.handle_entry("ProcessUnreadMessage");

	std::vector<std::string> threads = *(std::vector<std::string>*)p;
	delete (std::vector<std::string>*)p;

	int offset = 0;
	int limit = 21;

	bool local_timestamp = getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP_UNREAD, 0);

	// receive messages from all folders by default, use hidden setting to receive only inbox messages
	bool inboxOnly = getBool(FACEBOOK_KEY_INBOX_ONLY, 0);

	http::response resp;

	while (!threads.empty()) {
		std::string data = "client=mercury";
		data += "&__user=" + facy.self_.user_id;
		data += "&fb_dtsg=" + (!facy.dtsg_.empty() ? facy.dtsg_ : "0");
		data += "&__a=1&__dyn=&__req=&ttstamp=0";
	
		for (std::vector<std::string>::size_type i = 0; i < threads.size(); i++) {
			std::string thread_id = utils::url::encode(threads[i]);

			// request messages from thread
			data += "&messages[thread_ids][" + thread_id;
			data += "][offset]=" + utils::conversion::to_string(&offset, UTILS_CONV_SIGNED_NUMBER);
			data += "&messages[thread_ids][" + thread_id;
			data += "][limit]=" + utils::conversion::to_string(&limit, UTILS_CONV_SIGNED_NUMBER);
			
			// request info about thread
			data += "&threads[thread_ids][" + utils::conversion::to_string(&i, UTILS_CONV_UNSIGNED_NUMBER);
			data += "]=" + thread_id;
		}

		resp = facy.flap(REQUEST_THREAD_INFO, &data);

		if (resp.code == HTTP_CODE_OK) {
		
			CODE_BLOCK_TRY
		
			std::vector<facebook_message*> messages;
			std::map<std::string, facebook_chatroom*> chatrooms;

			facebook_json_parser* p = new facebook_json_parser(this);
			p->parse_thread_messages(&resp.data, &messages, &chatrooms, false, inboxOnly, limit);
			delete p;	

			for (std::map<std::string, facebook_chatroom*>::iterator it = chatrooms.begin(); it != chatrooms.end(); ) {

				facebook_chatroom *room = it->second;
				MCONTACT hChatContact = NULL;
				if (GetChatUsers(room->thread_id.c_str()) == NULL) {
					AddChat(room->thread_id.c_str(), room->chat_name.c_str());
					hChatContact = ChatIDToHContact(room->thread_id);
					// Set thread id (TID) for later
					setTString(hChatContact, FACEBOOK_KEY_TID, room->thread_id.c_str());

					for (std::map<std::string, std::string>::iterator jt = room->participants.begin(); jt != room->participants.end(); ) {
						AddChatContact(room->thread_id.c_str(), jt->first.c_str(), jt->second.c_str());
						++jt;
					}
				}

				if (!hChatContact)
					hChatContact = ChatIDToHContact(room->thread_id);

				ForkThread(&FacebookProto::ReadMessageWorker, (void*)hChatContact);

				delete it->second;
				it = chatrooms.erase(it);
			}
			chatrooms.clear();

			ReceiveMessages(messages, local_timestamp, true);

			debugLogA("***** Unread messages processed");

			CODE_BLOCK_CATCH

			debugLogA("***** Error processing unread messages: %s", e.what());

			CODE_BLOCK_END
	
			facy.handle_success("ProcessUnreadMessage");
		} else {
			facy.handle_error("ProcessUnreadMessage");
		}

		offset += limit;
		limit = 20; // TODO: use better limits?
		
		threads.clear(); // TODO: if we have limit messages from one user, there may be more unread messages... continue with it... otherwise remove that threadd from threads list -- or do it in json parser? hm			 = allow more than "limit" unread messages to be parsed
	}
}

void FacebookProto::LoadLastMessages(void *p)
{
	if (p == NULL)
		return;

	facy.handle_entry("LoadLastMessages");

	MCONTACT hContact = *(MCONTACT*)p;
	delete (MCONTACT*)p;

	if (!isOnline())
		return;

	std::string data = "client=mercury";
	data += "&__user=" + facy.self_.user_id;
	data += "&fb_dtsg=" + (!facy.dtsg_.empty() ? facy.dtsg_ : "0");
	data += "&__a=1&__dyn=&__req=&ttstamp=0";

	bool isChat = isChatRoom(hContact);

	ptrA item_id(getStringA(hContact, isChat ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID));
	if (item_id == NULL) {
		debugLogA("!!!!! LoadLastMessages: Contact has no TID/ID");
		return;
	}

	std::string id = utils::url::encode(std::string(item_id));
	std::string type = isChat ? "thread_ids" : "user_ids";
	int count = getByte(FACEBOOK_KEY_MESSAGES_ON_OPEN_COUNT, DEFAULT_MESSAGES_ON_OPEN_COUNT);
	count = min(count, FACEBOOK_MESSAGES_ON_OPEN_LIMIT);

	// request messages from thread
	data += "&messages[" + type + "][" + id;
	data += "][offset]=0";
	data += "&messages[" + type + "][" + id;
	data += "][limit]=" + utils::conversion::to_string(&count, UTILS_CONV_UNSIGNED_NUMBER);

	// request info about thread
	data += "&threads[" + type + "][0]=" + id;

	http::response resp = facy.flap(REQUEST_THREAD_INFO, &data);

	if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
		facy.handle_error("LoadLastMessages");
		return;
	}

	// Temporarily disable marking messages as read for this contact
	facy.ignore_read.insert(std::make_pair(hContact, true));

CODE_BLOCK_TRY

	std::vector<facebook_message*> messages;
	std::map<std::string, facebook_chatroom*> chatrooms;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_thread_messages(&resp.data, &messages, &chatrooms, false, false, 20);
	delete p;

	for (std::map<std::string, facebook_chatroom*>::iterator it = chatrooms.begin(); it != chatrooms.end();) {

		facebook_chatroom *room = it->second;
		MCONTACT hChatContact = NULL;
		if (GetChatUsers(room->thread_id.c_str()) == NULL) {
			AddChat(room->thread_id.c_str(), room->chat_name.c_str());
			hChatContact = ChatIDToHContact(room->thread_id);
			// Set thread id (TID) for later
			setTString(hChatContact, FACEBOOK_KEY_TID, room->thread_id.c_str());

			for (std::map<std::string, std::string>::iterator jt = room->participants.begin(); jt != room->participants.end();) {
				AddChatContact(room->thread_id.c_str(), jt->first.c_str(), jt->second.c_str());
				++jt;
			}
		}

		if (!hChatContact)
			hChatContact = ChatIDToHContact(room->thread_id);

		ForkThread(&FacebookProto::ReadMessageWorker, (void*)hChatContact);

		delete it->second;
		it = chatrooms.erase(it);
	}
	chatrooms.clear();

	bool local_timestamp = getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP_UNREAD, 0);
	ReceiveMessages(messages, local_timestamp, true);

	debugLogA("***** Thread messages processed");

CODE_BLOCK_CATCH

	debugLogA("***** Error processing thread messages: %s", e.what());

CODE_BLOCK_END

	facy.handle_success("LoadLastMessages");

	// Enable marking messages as read for this contact
	std::map<MCONTACT, bool>::iterator it = facy.ignore_read.find(hContact);
	if (it != facy.ignore_read.end()) {
		it->second = false;
	}
	
	// And force mark read
	OnDbEventRead(hContact, NULL);
}


void FacebookProto::ReceiveMessages(std::vector<facebook_message*> messages, bool local_timestamp, bool check_duplicates)
{
	bool naseemsSpamMode = getBool(FACEBOOK_KEY_NASEEMS_SPAM_MODE, false);

	// TODO: make this checking more lightweight as now it is not effective at all...
	if (check_duplicates) {
		// 1. check if there are some message that we already have (compare FACEBOOK_KEY_MESSAGE_ID = last received message ID)
		for (std::vector<facebook_message*>::size_type i = 0; i < messages.size(); i++) {

			MCONTACT hContact = messages[i]->isChat
				? ChatIDToHContact(std::tstring(_A2T(messages[i]->thread_id.c_str())))
				: ContactIDToHContact(messages[i]->user_id);

			if (hContact == NULL)
				continue;

			ptrA lastId(getStringA(hContact, FACEBOOK_KEY_MESSAGE_ID));
			if (lastId == NULL)
				continue;

			if (!messages[i]->message_id.compare(lastId)) {
				// Equal, ignore all older messages (including this) from same contact
				for (std::vector<facebook_message*>::size_type j = 0; j < messages.size(); j++) {
					bool equalsId = messages[i]->isChat
						? (messages[j]->thread_id == messages[i]->thread_id)
						: (messages[j]->user_id == messages[i]->user_id);

					if (equalsId && messages[j]->time <= messages[i]->time)
						messages[j]->flag_ = 1;
				}
			}
		}

		// 2. remove all marked messages from list
		for (std::vector<facebook_message*>::iterator it = messages.begin(); it != messages.end();) {
			if ((*it)->flag_ == 1)
				it = messages.erase(it);
			else
				++it;
		}
	}

	for(std::vector<facebook_message*>::size_type i = 0; i < messages.size(); i++) {
		DWORD timestamp = local_timestamp || !messages[i]->time ? ::time(NULL) : messages[i]->time;

		if (messages[i]->isChat) {
			// Multi-user message
			debugLogA("      Got chat message: %s", messages[i]->message_text.c_str());

			std::tstring tthread_id = _A2T(messages[i]->thread_id.c_str());

			// RM TODO: better use check if chatroom exists/is in db/is online... no?
			// like: if (ChatIDToHContact(tthread_id) == NULL) {
			if (GetChatUsers(tthread_id.c_str()) == NULL) {
				// In Naseem's spam mode we ignore outgoing messages sent from other instances
				if (naseemsSpamMode && !messages[i]->isIncoming) {
					delete messages[i];
					continue;
				}

				// TODO: We don't have this chat, lets load info about it (name, list of users, last messages, etc.)
				//LoadChatInfo(&fbu); // In this method use json's parse_chat_info
				std::tstring tthread_name = tthread_id; // TODO: use correct name for chat, not thread_id

				AddChat(tthread_id.c_str(), tthread_name.c_str());
			}

			MCONTACT hChatContact = ChatIDToHContact(tthread_id);

			// Save last (this) message ID
			setString(hChatContact, FACEBOOK_KEY_MESSAGE_ID, messages[i]->message_id.c_str());

			// Save TID if not exists already
			ptrA tid(getStringA(hChatContact, FACEBOOK_KEY_TID));
			if (!tid || strcmp(tid, messages[i]->thread_id.c_str()))
				setString(hChatContact, FACEBOOK_KEY_TID, messages[i]->thread_id.c_str());

			ParseSmileys(messages[i]->message_text, hChatContact);

			// TODO: support also system messages (rename chat, user quit, etc.)! (here? or it is somewhere else?
			// ... we must add some new "type" field into facebook_message structure and use it also for Pokes and similar)
			UpdateChat(tthread_id.c_str(), messages[i]->user_id.c_str(), messages[i]->sender_name.c_str(), messages[i]->message_text.c_str(), timestamp);

			// Automatically mark message as read because chatroom doesn't support onRead event (yet)
			ForkThread(&FacebookProto::ReadMessageWorker, (void*)hChatContact);
		} else {
			// Single-user message
			debugLogA("      Got message: %s", messages[i]->message_text.c_str());

			facebook_user fbu;
			fbu.user_id = messages[i]->user_id;
			fbu.real_name = messages[i]->sender_name;

			MCONTACT hContact = ContactIDToHContact(fbu.user_id);
			if (hContact == NULL) {
				// In Naseem's spam mode we ignore outgoing messages sent from other instances
				if (naseemsSpamMode && !messages[i]->isIncoming) {
					delete messages[i];
					continue;
				}

				// We don't have this contact, lets load info about him
				LoadContactInfo(&fbu);

				hContact = AddToContactList(&fbu, CONTACT_NONE);
			}

			// Save last (this) message ID
			setString(hContact, FACEBOOK_KEY_MESSAGE_ID, messages[i]->message_id.c_str());

			// Save TID if not exists already
			ptrA tid(getStringA(hContact, FACEBOOK_KEY_TID));
			if (!tid || strcmp(tid, messages[i]->thread_id.c_str()))
				setString(hContact, FACEBOOK_KEY_TID, messages[i]->thread_id.c_str());

			ParseSmileys(messages[i]->message_text, hContact);

			if (messages[i]->isIncoming && messages[i]->isUnread) {
				PROTORECVEVENT recv = { 0 };
				recv.flags = PREF_UTF;
				recv.szMessage = const_cast<char*>(messages[i]->message_text.c_str());
				recv.timestamp = timestamp;
				ProtoChainRecvMsg(hContact, &recv);
			} else {
				DBEVENTINFO dbei = { 0 };
				dbei.cbSize = sizeof(dbei);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.flags = DBEF_UTF;

				if (!messages[i]->isIncoming)
					dbei.flags |= DBEF_SENT;

				if (!messages[i]->isUnread)
					dbei.flags |= DBEF_READ;

				dbei.szModule = m_szModuleName;
				dbei.timestamp = timestamp;
				dbei.cbBlob = (DWORD)messages[i]->message_text.length() + 1;
				dbei.pBlob = (PBYTE)messages[i]->message_text.c_str();
				db_event_add(hContact, &dbei);
			}
		}
		delete messages[i];
	}
	messages.clear();
}

void FacebookProto::ProcessMessages(void* data)
{
	if (data == NULL)
		return;

	std::string* resp = (std::string*)data;

	// receive messages from all folders by default, use hidden setting to receive only inbox messages
	bool inboxOnly = getBool(FACEBOOK_KEY_INBOX_ONLY, 0);

	if (isOffline())
		goto exit;

	debugLogA("***** Starting processing messages");

	CODE_BLOCK_TRY

	std::vector< facebook_message* > messages;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_messages(data, &messages, &facy.notifications, inboxOnly);
	delete p;

	bool local_timestamp = getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP, 0);

	ReceiveMessages(messages, local_timestamp);

	ShowNotifications();

	debugLogA("***** Messages processed");

	CODE_BLOCK_CATCH

	debugLogA("***** Error processing messages: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::ShowNotifications() {
	if (!getByte(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
		return;
	
	for (std::map<std::string, facebook_notification*>::iterator it = facy.notifications.begin(); it != facy.notifications.end(); ++it) {
		if (it->second != NULL && !it->second->seen) {
			debugLogA("      Got notification: %s", it->second->text.c_str());
			ptrT szText(mir_utf8decodeT(it->second->text.c_str()));
			it->second->hWndPopup = NotifyEvent(m_tszUserName, szText, ContactIDToHContact(it->second->user_id), FACEBOOK_EVENT_NOTIFICATION, &it->second->link, &it->second->id);
			it->second->seen = true;
		}
	}
}

void FacebookProto::ProcessNotifications(void*)
{
	if (isOffline())
		return;

	facy.handle_entry("notifications");

	// Get notifications
	http::response resp = facy.flap(REQUEST_NOTIFICATIONS);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("notifications");
		return;
	}


	// Process notifications
	debugLogA("***** Starting processing notifications");

	CODE_BLOCK_TRY

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_notifications(&(resp.data), &facy.notifications);
	delete p;

	ShowNotifications();

	debugLogA("***** Notifications processed");

	CODE_BLOCK_CATCH

	debugLogA("***** Error processing notifications: %s", e.what());

	CODE_BLOCK_END
}

void FacebookProto::ProcessFriendRequests(void*)
{
	facy.handle_entry("friendRequests");

	// Get notifications
	http::response resp = facy.flap(REQUEST_LOAD_REQUESTS);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("friendRequests");
		return;
	}
	
	// Parse it
	std::string reqs = utils::text::source_get_value(&resp.data, 2, "class=\"mRequestItem", "al aps\">");

	std::string::size_type pos = 0;
	std::string::size_type pos2 = 0;
	bool last = false;

	while (!last && !reqs.empty()) {
		std::string req;
		if ((pos2 = reqs.find("class=\"mRequestItem", pos)) != std::string::npos) {
			req = reqs.substr(pos, pos2 - pos);
			pos = pos2 + 19;
		} else {
			req = reqs.substr(pos);
			last = true;
		}
				
		std::string get = utils::text::source_get_value(&req, 3, "<form", "action=\"", "\">");
		std::string time = utils::text::source_get_value2(&get, "seenrequesttime=", "&\"");

		facebook_user *fbu = new facebook_user();
		fbu->real_name = utils::text::source_get_value(&req, 2, "class=\"actor\">", "</");
		fbu->user_id = utils::text::source_get_value(&get, 2, "id=", "&");

		if (!fbu->user_id.empty() && !fbu->real_name.empty())
		{
			MCONTACT hContact = AddToContactList(fbu, CONTACT_APPROVE);
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_APPROVE);

			bool seen = false;

			DBVARIANT dbv;
			if (!getString(hContact, "RequestTime", &dbv)) {
				seen = !strcmp(dbv.pszVal, time.c_str());
				db_free(&dbv);
			}

			if (!seen) {
				// This is new request
				setString(hContact, "RequestTime", time.c_str());

				//blob is: uin(DWORD), hContact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
				//blob is: 0(DWORD), hContact(HANDLE), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ)
				DBEVENTINFO dbei = {0};
				dbei.cbSize = sizeof(DBEVENTINFO);
				dbei.szModule = m_szModuleName;
				dbei.timestamp = ::time(NULL);
				dbei.flags = DBEF_UTF;
				dbei.eventType = EVENTTYPE_AUTHREQUEST;
				dbei.cbBlob = (DWORD)(sizeof(DWORD)*2 + fbu->real_name.length() + 5);
					
				PBYTE pCurBlob = dbei.pBlob = (PBYTE) mir_alloc(dbei.cbBlob);					
				*(PDWORD)pCurBlob = 0; pCurBlob += sizeof(DWORD);                    // UID
				*(PDWORD)pCurBlob = (DWORD)hContact; pCurBlob += sizeof(DWORD);      // Contact Handle
				strcpy((char*)pCurBlob, fbu->real_name.data()); pCurBlob += fbu->real_name.length()+1;	// Nickname
				*pCurBlob = '\0'; pCurBlob++;                                        // First Name
				*pCurBlob = '\0'; pCurBlob++;                                        // Last Name
				*pCurBlob = '\0'; pCurBlob++;                                        // E-mail
				*pCurBlob = '\0';                                                    // Reason

				db_event_add(0, &dbei);				

				debugLogA("      (New) Friendship request from: %s (%s) [%s]", fbu->real_name.c_str(), fbu->user_id.c_str(), time.c_str());
			} else {
				debugLogA("      (Old) Friendship request from: %s (%s) [%s]", fbu->real_name.c_str(), fbu->user_id.c_str(), time.c_str());
			}
		} else {
			debugLogA(" !!!  Wrong friendship request");
			debugLogA("%s", req.c_str());
		}
	}

	facy.handle_success("friendRequests");
}

void FacebookProto::ProcessFeeds(void* data)
{
	if (!isOnline())
		return;

	facy.handle_entry("feeds");

	// Get feeds
	http::response resp = facy.flap(REQUEST_FEEDS);

	if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
		facy.handle_error("feeds");
		return;
	}

	CODE_BLOCK_TRY

	debugLogA("***** Starting processing feeds");

	std::vector< facebook_newsfeed* > news;

	std::string::size_type pos = 0;
	UINT limit = 0;

	DWORD new_time = facy.last_feeds_update_;
	bool filterAds = getBool(FACEBOOK_KEY_FILTER_ADS, DEFAULT_FILTER_ADS);

	debugLogA("      Last feeds update (old): %d", facy.last_feeds_update_);

	while ((pos = resp.data.find("<div class=\"userContentWrapper", pos)) != std::string::npos && limit <= 25)
	{		
		/*std::string::size_type pos2 = resp.data.find("<div class=\"userContentWrapper", pos+5);
		if (pos2 == std::string::npos)
			pos2 = resp.data.length();
		
		std::string post = resp.data.substr(pos, pos2 - pos);*/
		std::string post = resp.data.substr(pos, resp.data.find("</form>", pos) - pos);
		pos += 5;

		std::string post_header = utils::text::source_get_value(&post, 3, "<h5", ">", "</h5>");
		std::string post_message = utils::text::source_get_value(&post, 3, " userContent\"", ">", "<form");
		std::string post_link = utils::text::source_get_value(&post, 4, "</h5>", "<a", "href=\"", "\"");
		//std::string post_attach = utils::text::source_get_value(&post, 4, "<div class=", "uiStreamAttachments", ">", "<form");

		std::string post_time = utils::text::source_get_value(&post, 3, "</h5>", "<abbr", "</a>");

		std::string time = utils::text::source_get_value(&post_time, 2, "data-utime=\"", "\"");
		std::string time_text = utils::text::source_get_value(&post_time, 2, ">", "</abbr>");

		if (time.empty()) {
			// alternative parsing (probably page like or advertisement)
			time = utils::text::source_get_value(&post_time, 2, "content_timestamp&quot;:&quot;", "&quot;");
		}

		DWORD ttime;
		if (!utils::conversion::from_string<DWORD>(ttime, time, std::dec)) {
			debugLogA("! ! ! - Newsfeed with wrong/empty time (probably wrong parsing)\n%s", post.c_str());
			continue;
		}

		if (ttime > new_time) {
			new_time = ttime; // remember newest time from all these posts
			debugLogA("      - Newsfeed time: %d (new)", ttime);
		} 
		else if (ttime <= facy.last_feeds_update_) {
			debugLogA("      - Newsfeed time: %d (ignored)", ttime);
			continue; // ignore posts older than newest post of previous check
		}
		else {
			debugLogA("      - Newsfeed time: %d (normal)", ttime);
		}

		std::string post_place = utils::text::source_get_value(&post, 4, "</abbr>", "<a", ">", "</a>");

		std::string premsg = "\n" + time_text;

		post_place = utils::text::trim(
			utils::text::remove_html(post_place));
		if (!post_place.empty()) {
			premsg += " - " + post_place;
		}
		premsg += "\n";

		// in title keep only name, end of events like "X shared link" put into message
		std::string::size_type pos2 = post_header.find("‎");

		if (pos2 != std::string::npos) {
			utils::text::replace_first(&post_header, "‎", " → ");
		} else {
			pos2 = post_header.find("</a></");
			if (pos2 != std::string::npos) {
				pos2 += 4;
				std::string a = utils::text::trim(utils::text::remove_html(post_header.substr(pos2, post_header.length() - pos2)));
				if (a.length() > 2)
					premsg += a;
				post_header = post_header.substr(0, pos2);
			}
		}

		// Strip "Translate" link
		pos2 = post_message.find("role=\"button\">");
		if (pos2 != std::string::npos) {
			post_message = post_message.substr(0, pos2 + 14);
		}

		post_message = premsg + post_message;

		// append attachement to message (if any)
		//post_message += utils::text::trim(post_attach);

		facebook_newsfeed* nf = new facebook_newsfeed;

		nf->title = utils::text::trim(
			utils::text::special_expressions_decode(
				utils::text::remove_html(post_header)));

		nf->user_id = utils::text::source_get_value(&post_header, 2, "user.php?id=", "&amp;");
		
		nf->link = utils::text::special_expressions_decode(post_link);
		
		// Check if we don't want to show ads posts
		bool filtered = filterAds && (nf->link.find("/about/ads") != std::string::npos
			|| post.find("class=\"uiStreamSponsoredLink\"") != std::string::npos
			|| post.find("href=\"/about/ads\"") != std::string::npos);

		nf->text = utils::text::trim(
			utils::text::special_expressions_decode(
				utils::text::remove_html(
					utils::text::edit_html(post_message))));		

		if (filtered || nf->title.empty() || nf->text.empty()) {
			debugLogA("      \\ Newsfeed (time: %d) is filtered: %s", ttime, filtered ? "advertisement" : (nf->title.empty() ? "title empty" : "text empty"));
			delete nf;
			continue;
		}
		else {
			debugLogA("      Got newsfeed (time: %d): %s %s", ttime, nf->title.c_str(), nf->text.c_str());
		}

		news.push_back(nf);
		pos++;
		limit++;
	}

	debugLogA("      Last feeds update (new): %d", new_time);

	for(std::vector<facebook_newsfeed*>::size_type i=0; i<news.size(); i++)
	{
		ptrT tszTitle( mir_utf8decodeT(news[i]->title.c_str()));
		ptrT tszText( mir_utf8decodeT(news[i]->text.c_str()));

		// Truncate text of newsfeed when it's too long
		if (_tcslen(tszText) > MAX_NEWSFEED_LEN) {
			TCHAR buf[MAX_NEWSFEED_LEN + 3 + 1] = { 0 };
			_tcsncpy(buf, tszText, MAX_NEWSFEED_LEN);
			_tcsncpy(&buf[MAX_NEWSFEED_LEN], _T("..."), 3);
			tszText = buf;
		}

		NotifyEvent(tszTitle, tszText, this->ContactIDToHContact(news[i]->user_id), FACEBOOK_EVENT_NEWSFEED, &news[i]->link);
		delete news[i];
	}
	news.clear();

	// Set time of last update to time of newest post
	this->facy.last_feeds_update_ = new_time;

	debugLogA("***** Feeds processed");

	CODE_BLOCK_CATCH

	debugLogA("***** Error processing feeds: %s", e.what());

	CODE_BLOCK_END

	facy.handle_success("feeds");
}

void FacebookProto::ProcessPages(void *data)
{
	if (!getByte(FACEBOOK_KEY_LOAD_PAGES, DEFAULT_LOAD_PAGES))
		return;

	facy.handle_entry("load_pages");

	// Get feeds
	http::response resp = facy.flap(REQUEST_PAGES);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("load_pages");
		return;
	}

	std::string content = utils::text::source_get_value(&resp.data, 2, "id=\"bookmarksSeeAllSection\"", "</code>");

	std::string::size_type start, end;
	start = content.find("<li", 0);
	while (start != std::string::npos) {
		end = content.find("<li", start + 1);
		if (end == std::string::npos)
			end = content.length();

		std::string item = content.substr(start, end - start);
		//item = utils::text::source_get_value(&item, 2, "data-gt=", ">");

		start = content.find("<li", start + 1);

		std::string id = utils::text::source_get_value(&item, 3, "data-gt=", "bmid&quot;:&quot;", "&quot;");
		std::string title = utils::text::special_expressions_decode(utils::text::slashu_to_utf8(utils::text::source_get_value(&item, 3, "data-gt=", "title=\"", "\"")));
		std::string href = utils::text::source_get_value(&item, 3, "data-gt=", "href=\"", "\"");

		// Ignore pages channel
		if (href.find("/pages/feed") != std::string::npos)
			continue;

		if (id.empty() || title.empty())
			continue;

		debugLogA("      Got page: %s (%s)", title.c_str(), id.c_str());
		facy.pages[id] = title;
	}

	facy.handle_success("load_pages");
}

void FacebookProto::SearchAckThread(void *targ)
{
	facy.handle_entry("searchAckThread");

	int count = 0;

	char *arg = mir_utf8encodeT((TCHAR*)targ);
	std::string search = utils::url::encode(arg);
	std::string ssid;

	while (count < 50 && !isOffline())
	{
		std::string get_data = search + "&s=" + utils::conversion::to_string(&count, UTILS_CONV_UNSIGNED_NUMBER);
		if (!ssid.empty())
			get_data += "&ssid=" + ssid;

		http::response resp = facy.flap(REQUEST_SEARCH, NULL, &get_data);

		if (resp.code == HTTP_CODE_OK)
		{
			std::string items = utils::text::source_get_value(&resp.data, 2, "<body", "</body>");

			std::string::size_type pos = 0;
			std::string::size_type pos2 = 0;

			while ((pos = items.find("<td class=\"pic\">", pos)) != std::string::npos) {
				std::string item = items.substr(pos, items.find("</tr>", pos) - pos);
				pos++; count++;

				std::string id = utils::text::source_get_value2(&item, "?id=", "&\"");
				if (id.empty())
					id = utils::text::source_get_value2(&item, "?ids=", "&\"");

				std::string name = utils::text::source_get_value(&item, 4, "<td class=\"name\">", "<a", ">", "</");
				std::string surname;
				std::string nick;
				std::string common = utils::text::source_get_value(&item, 2, "<span class=\"mfss fcg\">", "</span>");

				if ((pos2 = name.find("<span class=\"alternate_name\">")) != std::string::npos) {
					nick = name.substr(pos2 + 30, name.length() - pos2 - 31); // also remove brackets around nickname
					name = name.substr(0, pos2 - 1);
				}

				if ((pos2 = name.find(" ")) != std::string::npos) {
					surname = name.substr(pos2 + 1, name.length() - pos2 - 1);
					name = name.substr(0, pos2);
				}

				// ignore self contact and empty ids
				if (id.empty() || id == facy.self_.user_id)
					continue;

				ptrT tid( mir_utf8decodeT(id.c_str()));
				ptrT tname( mir_utf8decodeT(utils::text::special_expressions_decode(name).c_str()));
				ptrT tsurname( mir_utf8decodeT(utils::text::special_expressions_decode(surname).c_str()));
				ptrT tnick( mir_utf8decodeT(utils::text::special_expressions_decode(nick).c_str()));
				ptrT tcommon( mir_utf8decodeT(utils::text::special_expressions_decode(common).c_str()));

				PROTOSEARCHRESULT isr = {0};
				isr.cbSize = sizeof(isr);
				isr.flags = PSR_TCHAR;
				isr.id  = tid;
				isr.nick  = tnick;
				isr.firstName = tname;
				isr.lastName = tsurname;
				isr.email = tcommon;

				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&isr);
			}

			ssid = utils::text::source_get_value(&items, 3, "id=\"more_objects\"", "ssid=", "&");
			if (ssid.empty())
				break; // No more results
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);

	facy.handle_success("searchAckThread");

	mir_free(targ);
	mir_free(arg);
}

void FacebookProto::SearchIdAckThread(void *targ)
{
	facy.handle_entry("searchIdAckThread");

	char *arg = mir_utf8encodeT((TCHAR*)targ);
	std::string search = utils::url::encode(arg) + "?";

	if (!isOffline())
	{
		http::response resp = facy.flap(REQUEST_USER_INFO, NULL, &search);

		if (resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end()) {
			search = utils::text::source_get_value(&resp.headers["Location"], 2, FACEBOOK_SERVER_MOBILE"/", "_rdr", true);
			resp = facy.flap(REQUEST_USER_INFO, NULL, &search);
		}

		if (resp.code == HTTP_CODE_OK)
		{
			std::string about = utils::text::source_get_value(&resp.data, 2, "<div class=\"timeline", "id=\"footer");
		
			std::string id = utils::text::source_get_value2(&about, ";id=", "&\"");
			if (id.empty())
				id = utils::text::source_get_value2(&about, "?bid=", "&\"");
			std::string name = utils::text::source_get_value(&about, 3, "class=\"profileName", ">", "</");
			std::string surname;

			std::string::size_type pos;
			if ((pos = name.find(" ")) != std::string::npos) {
				surname = name.substr(pos + 1, name.length() - pos - 1);
				name = name.substr(0, pos);
			}

			// ignore self contact and empty ids
			if (!id.empty() && id != facy.self_.user_id){
				ptrT tid( mir_utf8decodeT(id.c_str()));
				ptrT tname( mir_utf8decodeT(name.c_str()));
				ptrT tsurname( mir_utf8decodeT(surname.c_str()));

				PROTOSEARCHRESULT isr = {0};
				isr.cbSize = sizeof(isr);
				isr.flags = PSR_TCHAR;
				isr.id = tid;
				isr.firstName = tname;
				isr.lastName = tsurname;

				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&isr);
			}
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);	

	facy.handle_success("searchIdAckThread");	

	mir_free(targ);
	mir_free(arg);
}
