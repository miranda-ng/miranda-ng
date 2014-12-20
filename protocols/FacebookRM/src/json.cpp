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

int facebook_json_parser::parse_buddy_list(std::string *data, List::List< facebook_user >* buddy_list)
{
	facebook_user* current = NULL;
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *list = json_get(payload, "buddy_list");
	if (list == NULL) {
		return EXIT_FAILURE;
	}

	// Set all contacts in map to offline
	for (List::Item< facebook_user >* i = buddy_list->begin(); i != NULL; i = i->next) {
		i->data->status_id = ID_STATUS_OFFLINE;
	}

	// Load last active times
	JSONNODE *lastActive = json_get(list, "last_active_times");
	if (lastActive != NULL) {
		for (unsigned int i = 0; i < json_size(lastActive); i++) {
			JSONNODE *it = json_at(lastActive, i);
			const char *id = json_name(it);

			current = buddy_list->find(id);
			if (current == NULL) {
				buddy_list->insert(std::make_pair(id, new facebook_user()));
				current = buddy_list->find(id);
				current->user_id = id;
			}

			current->last_active = json_as_int(it);
		}
	}

	// Find mobile friends
	JSONNODE *mobileFriends = json_get(list, "mobile_friends");
	if (mobileFriends != NULL) {
		for (unsigned int i = 0; i < json_size(mobileFriends); i++) {
			JSONNODE *it = json_at(mobileFriends, i);
			std::string id = json_as_pstring(it);

			current = buddy_list->find(id);
			if (current == NULL) {
				buddy_list->insert(std::make_pair(id, new facebook_user()));
				current = buddy_list->find(id);
				current->user_id = id;
			}

			current->status_id = ID_STATUS_OFFLINE;
			current->client = CLIENT_MOBILE;
		}
	}

	// Find now available contacts
	JSONNODE *nowAvailable = json_get(list, "nowAvailableList");
	if (nowAvailable != NULL) {
		for (unsigned int i = 0; i < json_size(nowAvailable); i++) {
			JSONNODE *it = json_at(nowAvailable, i);
			const char *id = json_name(it);

			current = buddy_list->find(id);
			if (current == NULL) {
				buddy_list->insert(std::make_pair(id, new facebook_user()));
				current = buddy_list->find(id);
				current->user_id = id;
			}

			current->status_id = ID_STATUS_ONLINE;

			JSONNODE *p = json_get(it, "p");
			if (p != NULL) {
				JSONNODE *status = json_get(p, "status"); // this seems to be "active" everytime
				JSONNODE *webStatus = json_get(p, "webStatus"); // "active", "idle" or "offline"
				JSONNODE *fbAppStatus = json_get(p, "fbAppStatus"); // "offline" or "active"
				JSONNODE *messengerStatus = json_get(p, "messengerStatus"); // "offline" or "active"
				JSONNODE *otherStatus = json_get(p, "otherStatus"); // "offline" or "active" - this seems to be "active" when webStatus is "idle" or "active" only

				// this may never happen
				if (json_as_pstring(status) != "active")
					current->status_id = ID_STATUS_OFFLINE;

				bool b;

				// "webStatus" and "otherStatus" are marked as "WEB" on FB website
				if ((b = (json_as_pstring(webStatus) == "active")) || json_as_pstring(otherStatus) == "active") {
					current->status_id = ID_STATUS_ONLINE;
					current->client = b ? CLIENT_WEB : CLIENT_OTHER;
				}

				// "fbAppStatus" and "messengerStatus" are marked as "MOBILE" on FB website
				if ((b = (json_as_pstring(fbAppStatus) == "active")) || json_as_pstring(messengerStatus) == "active") {
					current->status_id = ID_STATUS_ONTHEPHONE;
					current->client = b ? CLIENT_APP : CLIENT_MESSENGER;
				}

				// this is not marked anyhow on website (yet?)
				current->idle = json_as_pstring(webStatus) == "idle"
					|| json_as_pstring(otherStatus) == "idle"
					|| json_as_pstring(fbAppStatus) == "idle"
					|| json_as_pstring(messengerStatus) == "idle";
			}
		}
	}

	// Get aditional informations about contacts (if available)
	JSONNODE *userInfos = json_get(list, "userInfos");
	if (userInfos != NULL) {
		for (unsigned int i = 0; i < json_size(userInfos); i++) {
			JSONNODE *it = json_at(userInfos, i);
			const char *id = json_name(it);

			current = buddy_list->find(id);
			if (current == NULL)
				continue;

			JSONNODE *name = json_get(it, "name");
			JSONNODE *thumbSrc = json_get(it, "thumbSrc");

			if (name != NULL)
				current->real_name = utils::text::slashu_to_utf8(json_as_pstring(name));
			if (thumbSrc != NULL)
				current->image_url = utils::text::slashu_to_utf8(json_as_pstring(thumbSrc));
		}
	}

	return EXIT_SUCCESS;
}

void parseUser(JSONNODE *it, facebook_user *fbu)
{
	fbu->user_id = json_name(it);

	JSONNODE *id = json_get(it, "id");
	if (id == NULL || json_as_pstring(id) == "0" || json_as_pstring(id).empty()) {
		// this user has deleted account or is just unavailable for us (e.g., ignore list) -> don't read dummy name and avatar and rather ignore that completely
		return;
	}

	JSONNODE *name = json_get(it, "name");
	JSONNODE *thumbSrc = json_get(it, "thumbSrc");
	JSONNODE *gender = json_get(it, "gender");
	JSONNODE *vanity = json_get(it, "vanity"); // username
	//JSONNODE *uri = json_get(it, "uri"); // profile url
	//JSONNODE *is_friend = json_get(it, "is_friend"); // e.g. "True"
	//JSONNODE *type = json_get(it, "type"); // e.g. "friend" (classic contact) or "user" (disabled/deleted account)


	if (name)
		fbu->real_name = utils::text::slashu_to_utf8(json_as_pstring(name));
	if (thumbSrc)
		fbu->image_url = utils::text::slashu_to_utf8(json_as_pstring(thumbSrc));
	if (vanity)
		fbu->username = utils::text::slashu_to_utf8(json_as_pstring(vanity));

	if (gender)
		switch (json_as_int(gender)) {
		case 1: // female
			fbu->gender = 70;
			break;
		case 2: // male
			fbu->gender = 77;
			break;
			// case 7: not available female?
	}
}

int facebook_json_parser::parse_friends(std::string *data, std::map< std::string, facebook_user* >* friends)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(payload); i++) {
		JSONNODE *it = json_at(payload, i);

		facebook_user *fbu = new facebook_user();
		parseUser(it, fbu);

		friends->insert(std::make_pair(fbu->user_id, fbu));
	}

	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_notifications(std::string *data, std::map< std::string, facebook_notification* > *notifications)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *list = json_get(payload, "notifications");
	if (list == NULL) {
		return EXIT_FAILURE;
	}

	// check if we should use use local_timestamp for unread messages and use it for notifications time too
	bool local_timestamp = proto->getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP_UNREAD, 0);

	// Create notifications chatroom (if it doesn't exists), because we will be writing to it new notifications here
	proto->PrepareNotificationsChatRoom();

	for (unsigned int i = 0; i < json_size(list); i++) {
		JSONNODE *it = json_at(list, i);
		const char *id = json_name(it);

		JSONNODE *markup = json_get(it, "markup");
		JSONNODE *unread = json_get(it, "unread");
		JSONNODE *time = json_get(it, "time");

		// Ignore empty and old notifications
		if (markup == NULL || unread == NULL || time == NULL || json_as_int(unread) == 0)
			continue;

		std::string text = utils::text::html_entities_decode(utils::text::slashu_to_utf8(json_as_pstring(markup)));

		facebook_notification* notification = new facebook_notification();

		notification->id = id;
		notification->link = utils::text::source_get_value(&text, 3, "<a ", "href=\"", "\"");
		notification->text = utils::text::remove_html(utils::text::source_get_value(&text, 1, "<abbr"));
		notification->time = local_timestamp ? ::time(NULL) : utils::time::fix_timestamp(json_as_float(time));

		// Write notification to chatroom
		proto->UpdateNotificationsChatRoom(notification);

		// If it's unseen, remember it, otherwise forget it
		if (notifications->find(notification->id) == notifications->end())
			notifications->insert(std::make_pair(notification->id, notification));
		else
			delete notification;
	}

	return EXIT_SUCCESS;
}

bool ignore_duplicits(FacebookProto *proto, const std::string &mid, const std::string &text)
{
	ScopedLock s(proto->facy.send_message_lock_);

	std::map<std::string, int>::iterator it = proto->facy.messages_ignore.find(mid);
	if (it != proto->facy.messages_ignore.end()) {
		proto->debugLogA("????? Ignoring duplicit/sent message\n%s", text.c_str());

		it->second++; // increase counter (for deleting it later)
		return true;
	}

	// remember this id to ignore duplicits
	proto->facy.messages_ignore.insert(std::make_pair(mid, 1));
	return false;
}

void parseAttachments(FacebookProto *proto, std::string *message_text, JSONNODE *it, const std::string &thread_id, std::string other_user_fbid)
{
	// Process attachements and stickers
	JSONNODE *has_attachment = json_get(it, "has_attachment");
	if (has_attachment != NULL && json_as_bool(has_attachment)) {
		// Append attachements
		std::string type;
		std::string attachments_text;
		JSONNODE *attachments = json_get(it, "attachments");
		for (unsigned int j = 0; j < json_size(attachments); j++) {
			JSONNODE *itAttachment = json_at(attachments, j);
			JSONNODE *attach_type = json_get(itAttachment, "attach_type"); // "sticker", "photo", "file"
			if (attach_type != NULL) {
				// Get attachment type - "file" has priority over other types
				if (type.empty() || type != "file")
					type = json_as_pstring(attach_type);
			}
			JSONNODE *name = json_get(itAttachment, "name");
			JSONNODE *url = json_get(itAttachment, "url");
			if (url != NULL) {
				std::string link = json_as_pstring(url);

				if (link.find("/ajax/mercury/attachments/photo/view/") != std::string::npos)
					// fix photo url
					link = utils::url::decode(utils::text::source_get_value(&link, 2, "?uri=", "&"));
				else if (link.find("/") == 0) {
					// make absolute url
					bool useHttps = proto->getByte(FACEBOOK_KEY_FORCE_HTTPS, 1) > 0;
					link = (useHttps ? HTTP_PROTO_SECURE : HTTP_PROTO_REGULAR) + std::string(FACEBOOK_SERVER_REGULAR) + link;
				}

				if (!link.empty()) {
					std::string filename;
					if (name != NULL)
						filename = json_as_pstring(name);
					if (filename == "null")
						filename.clear();

					attachments_text += "\n" + (!filename.empty() ? "<" + filename + "> " : "") + link + "\n";

					// Stickers as smileys
					if (type == "sticker" && proto->getByte(FACEBOOK_KEY_CUSTOM_SMILEYS, DEFAULT_CUSTOM_SMILEYS)) {
						JSONNODE *metadata = json_get(itAttachment, "metadata");
						if (metadata != NULL) {
							JSONNODE *stickerId_ = json_get(metadata, "stickerID");
							if (stickerId_ != NULL) {
								std::string sticker = "[[sticker:" + json_as_pstring(stickerId_) + "]]";
								attachments_text += sticker;

								if (other_user_fbid.empty() && !thread_id.empty()) {
									other_user_fbid = proto->ThreadIDToContactID(thread_id);
								}

								// FIXME: rewrite smileyadd to use custom smileys per protocol and not per contact and then remove this ugliness
								if (!other_user_fbid.empty()) {
									MCONTACT hContact = proto->ContactIDToHContact(other_user_fbid);
									proto->StickerAsSmiley(sticker, link, hContact);
								}
							}
						}
					}
				}
			}
		}

		// TODO: have this as extra event, not replace or append message content
		if (!message_text->empty())
			*message_text += "\n\n";

		if (!attachments_text.empty()) {
			// we can't use this as offline messages doesn't have it
			/* JSONNODE *admin_snippet = json_get(it, "admin_snippet");
			if (admin_snippet != NULL) {
			*message_text += json_as_pstring(admin_snippet);
			} */

			std::tstring newText;
			if (type == "sticker")
				newText = TranslateT("a sticker");
			else if (type == "file")
				newText = (json_size(attachments) > 1) ? TranslateT("files") : TranslateT("a file");
			else if (type == "photo")
				newText = (json_size(attachments) > 1) ? TranslateT("photos") : TranslateT("a photo");
			else
				newText = _A2T(type.c_str());

			TCHAR title[200];
			mir_sntprintf(title, SIZEOF(title), TranslateT("User sent %s:"), newText.c_str());

			*message_text += ptrA(mir_utf8encodeT(title));
			*message_text += attachments_text;
		}
		else {
			// TODO: better support for these attachments (parse it from "m_messaging" instead of "messaging"
			*message_text += ptrA(mir_utf8encodeT(TranslateT("User sent an unsupported attachment. Open your browser to see it.")));
		}
	}
}

int facebook_json_parser::parse_messages(std::string *data, std::vector< facebook_message* >* messages, std::map< std::string, facebook_notification* >* notifications, bool inboxOnly)
{
	// remove old received messages from map		
	for (std::map<std::string, int>::iterator it = proto->facy.messages_ignore.begin(); it != proto->facy.messages_ignore.end();) {
		if (it->second > FACEBOOK_IGNORE_COUNTER_LIMIT) {
			it = proto->facy.messages_ignore.erase(it);
		}
		else {
			it->second++; // increase counter on each request
			++it;
		}
	}

	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *ms = json_get(root, "ms");
	if (ms == NULL) {
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(ms); i++) {
		JSONNODE *it = json_at(ms, i);

		JSONNODE *type = json_get(it, "type");
		if (type == NULL)
			continue;

		std::string t = json_as_pstring(type);
		if (t == "messaging") {
			// various messaging stuff (received and sent messages, getting seen info)

			JSONNODE *type = json_get(it, "event");
			if (type == NULL)
				continue;

			std::string t = json_as_pstring(type);

			if (t == "read_receipt") {
				// user read message
				JSONNODE *reader = json_get(it, "reader");
				JSONNODE *time = json_get(it, "time");

				if (reader == NULL || time == NULL)
					continue;

				// check if we should use use local_timestamp for incoming messages and use it for read time too
				/*bool local_timestamp = proto->getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP, 0);
				time_t timestamp = local_timestamp ? ::time(NULL) : utils::time::fix_timestamp(json_as_float(time));*/

				// we can always use NOW for read time, because that's the time of receiving this event (+-)
				time_t timestamp = ::time(NULL);

				JSONNODE *threadid = json_get(it, "tid");
				if (threadid != NULL) { // multi user chat
					if (!proto->m_enableChat)
						continue;

					std::tstring tid = ptrT(json_as_string(threadid));
					std::string reader_id = json_as_pstring(reader);

					std::map<std::tstring, facebook_chatroom*>::iterator it = proto->facy.chat_rooms.find(tid);
					if (it != proto->facy.chat_rooms.end()) {
						facebook_chatroom *chatroom = it->second;
						std::map<std::string, std::string> participants = chatroom->participants;

						std::map<std::string, std::string>::const_iterator participant = participants.find(reader_id);
						if (participant == participants.end()) {
							// TODO: load name of this participant
							std::string name = reader_id;
							participants.insert(std::make_pair(reader_id, name));
							proto->AddChatContact(tid.c_str(), reader_id.c_str(), name.c_str());
						}

						participant = participants.find(reader_id);
						if (participant != participants.end()) {
							if (!chatroom->message_readers.empty())
								chatroom->message_readers += ", ";
							chatroom->message_readers += participant->second;

							MCONTACT hChatContact = proto->ChatIDToHContact(tid);
							if (!hChatContact)
								continue;

							ptrT readers(mir_utf8decodeT(chatroom->message_readers.c_str()));

							StatusTextData st = { 0 };
							st.cbSize = sizeof(st);
							st.hIcon = Skin_GetIconByHandle(GetIconHandle("read"));

							TCHAR ttime[64];
							_tcsftime(ttime, SIZEOF(ttime), _T("%X"), localtime(&timestamp));

							mir_sntprintf(st.tszText, SIZEOF(st.tszText), TranslateT("Message read: %s by %s"), ttime, readers);

							CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hChatContact, (LPARAM)&st);
						}
					}
				}
				else { // classic contact
					MCONTACT hContact = proto->ContactIDToHContact(json_as_pstring(reader));
					if (hContact) {
						proto->facy.insert_reader(hContact, timestamp);
					}
				}
			}
			else if (t == "deliver") {
				// inbox message (multiuser or direct)

				JSONNODE *msg = json_get(it, "message");
				JSONNODE *folder = json_get(it, "folder");

				if (inboxOnly && json_as_pstring(folder) != "inbox")
					continue;

				JSONNODE *sender_fbid = json_get(msg, "sender_fbid");
				JSONNODE *sender_name = json_get(msg, "sender_name");
				JSONNODE *body = json_get(msg, "body");

				// looks like there is either "tid" or "other_user_fbid" - or "tid" is removed forever?
				JSONNODE *tid = json_get(msg, "tid");
				JSONNODE *other_user_id_ = json_get(msg, "other_user_fbid");

				JSONNODE *mid = json_get(msg, "mid");
				JSONNODE *timestamp = json_get(msg, "timestamp");
				JSONNODE *filtered = json_get(it, "is_filtered_content");

				if (sender_fbid == NULL || sender_name == NULL || body == NULL || mid == NULL || timestamp == NULL)
					continue;

				std::string id = json_as_pstring(sender_fbid);
				std::string message_id = json_as_pstring(mid);
				std::string message_text = json_as_pstring(body);

				std::string thread_id = (tid != NULL ? json_as_pstring(tid) : "");
				std::string other_user_id = (other_user_id_ != NULL ? json_as_pstring(other_user_id_) : "");

				// Process attachements and stickers
				parseAttachments(proto, &message_text, msg, thread_id, other_user_id);

				// Ignore duplicits or messages sent from miranda
				if (body == NULL || ignore_duplicits(proto, message_id, message_text))
					continue;

				if (json_as_bool(filtered) && message_text.empty())
					message_text = Translate("This message is no longer available, because it was marked as abusive or spam.");

				message_text = utils::text::trim(utils::text::slashu_to_utf8(message_text), true);
				if (message_text.empty())
					continue;

				facebook_message* message = new facebook_message();
				message->isUnread = true;
				message->isIncoming = (id != proto->facy.self_.user_id);
				message->message_text = message_text;
				message->timestamp = json_as_pstring(timestamp);
				message->time = utils::time::fix_timestamp(json_as_float(timestamp));
				message->user_id = id;
				message->message_id = message_id;
				message->sender_name = utils::text::slashu_to_utf8(json_as_pstring(sender_name)); // TODO: or if not incomming use my own name from facy.self_ ?
				message->thread_id = json_as_pstring(tid); // TODO: or if not incomming use my own id from facy.self_ ?

				JSONNODE *gthreadinfo = json_get(msg, "group_thread_info");
				message->isChat = (gthreadinfo != NULL && json_as_pstring(gthreadinfo) != "null");

				if (!message->isChat && !message->isIncoming) {
					message->sender_name.clear();
					message->user_id = proto->ThreadIDToContactID(message->thread_id); // TODO: Check if we have contact with this user_id in friendlist and otherwise do something different?
				}

				messages->push_back(message);
			}
		}
		else if (t == "notification_json") {
			// event notification
			JSONNODE *nodes = json_get(it, "nodes");

			// check if we should use use local_timestamp for unread messages and use it for notifications time too
			bool local_timestamp = proto->getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP_UNREAD, 0);

			// Create notifications chatroom (if it doesn't exists), because we will be writing to it new notifications here
			proto->PrepareNotificationsChatRoom();

			for (unsigned int j = 0; j < json_size(nodes); j++) {
				JSONNODE *itNodes = json_at(nodes, j);

				//JSONNODE *text = json_get(itNodes, "title/text"); // use this when popups will be ready to allow changes of their content
				JSONNODE *text_ = json_get(itNodes, "unaggregatedTitle"); // notifications one by one, not grouped
				if (text_ == NULL)
					continue;
				JSONNODE *text = json_get(text_, "text");
				JSONNODE *url = json_get(itNodes, "url");
				JSONNODE *alert_id = json_get(itNodes, "alert_id");

				JSONNODE *time_ = json_get(itNodes, "timestamp");
				if (time_ == NULL)
					continue;
				JSONNODE *time = json_get(time_, "time");
				if (time == NULL || text == NULL || url == NULL || alert_id == NULL)
					continue;

				double timestamp = json_as_float(time);
				if (timestamp > proto->facy.last_notification_time_) {
					// Only new notifications
					proto->facy.last_notification_time_ = timestamp;

					facebook_notification* notification = new facebook_notification();
					notification->text = utils::text::slashu_to_utf8(json_as_pstring(text));
					notification->link = json_as_pstring(url);
					notification->id = json_as_pstring(alert_id);
					notification->time = local_timestamp ? ::time(NULL) : utils::time::fix_timestamp(timestamp);

					std::string::size_type pos = notification->id.find(":");
					if (pos != std::string::npos)
						notification->id = notification->id.substr(pos + 1);

					// Write notification to chatroom
					proto->UpdateNotificationsChatRoom(notification);

					// If it's unseen, remember it, otherwise forget it (here it will always be unseen)
					if (notifications->find(notification->id) == notifications->end() && !notification->seen)
						notifications->insert(std::make_pair(notification->id, notification));
					else
						delete notification;
				}
			}
		}
		else if (t == "typ") {
			// chat typing notification

			JSONNODE *from = json_get(it, "from");
			if (from == NULL)
				continue;

			facebook_user fbu;
			fbu.user_id = json_as_pstring(from);

			MCONTACT hContact = proto->AddToContactList(&fbu, CONTACT_FRIEND); // only friends are able to send typing notifications

			JSONNODE *st = json_get(it, "st");
			if (json_as_int(st) == 1)
				proto->StartTyping(hContact);
			else
				proto->StopTyping(hContact);
		}
		else if (t == "ttyp") {
			// multi chat typing notification
			if (!proto->m_enableChat)
				continue;

			JSONNODE *from_ = json_get(it, "from");
			JSONNODE *thread_ = json_get(it, "thread");
			JSONNODE *st_ = json_get(it, "st");

			if (from_ == NULL || thread_ == NULL || st_ == NULL)
				continue;

			std::tstring tid = ptrT(json_as_string(thread_));
			std::string from_id = json_as_pstring(from_);

			std::map<std::tstring, facebook_chatroom*>::iterator it = proto->facy.chat_rooms.find(tid);
			if (it != proto->facy.chat_rooms.end()) {
				facebook_chatroom *chatroom = it->second;
				std::map<std::string, std::string> participants = chatroom->participants;

				std::map<std::string, std::string>::const_iterator participant = participants.find(from_id);
				if (participant == participants.end()) {
					// TODO: load name of this participant
					std::string name = from_id;
					proto->AddChatContact(tid.c_str(), from_id.c_str(), name.c_str());
				}

				participant = participants.find(from_id);
				if (participant != participants.end()) {
					MCONTACT hChatContact = proto->ChatIDToHContact(tid);
					ptrT name(mir_utf8decodeT(participant->second.c_str()));

					if (json_as_int(st_) == 1) {
						StatusTextData st = { 0 };
						st.cbSize = sizeof(st);

						mir_sntprintf(st.tszText, SIZEOF(st.tszText), TranslateT("%s is typing a message..."), name);

						CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hChatContact, (LPARAM)&st);
					}
					else {
						CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hChatContact, NULL);
					}

					// TODO: support proper MS_PROTO_CONTACTISTYPING service for chatrooms (when it will be implemented)
				}
			}
		}
		else if (t == "privacy_changed") {
			// settings changed

			JSONNODE *event_type = json_get(it, "event");
			JSONNODE *event_data = json_get(it, "data");

			if (event_type == NULL || event_data == NULL)
				continue;

			std::string t = json_as_pstring(event_type);
			if (t == "visibility_update") {
				// change of chat status
				JSONNODE *visibility = json_get(event_data, "visibility");

				bool isVisible = (visibility != NULL) && json_as_bool(visibility);
				proto->debugLogA("      Requested chat switch to %s", isVisible ? "Online" : "Offline");
				proto->SetStatus(isVisible ? ID_STATUS_ONLINE : ID_STATUS_INVISIBLE);
			}
		}
		else if (t == "buddylist_overlay") {
			// we opened/closed chat window - pretty useless info for us
			continue;
		} else if (t == "ticker_update:home") {
			JSONNODE *actor_ = json_get(it, "actor");
			JSONNODE *time_ = json_get(it, "time");
			JSONNODE *story_ = json_get(it, "story_xhp");

			std::string text = json_as_pstring(story_);
			text = utils::text::slashu_to_utf8(text);

			text = utils::text::trim(
				utils::text::html_entities_decode(
				utils::text::source_get_value(&text, 3, "<h5", ">", "</h5>")));

			// TODO: notify ticker updates
			/* if (!text.empty()) {
				proto->NotifyEvent()
				}*/
		}
		else if (t == "mercury") {
			// rename multi user chat, video call, ...

			JSONNODE *actions_ = json_get(it, "actions");
			if (actions_ == NULL)
				continue;

			for (unsigned int i = 0; i < json_size(actions_); i++) {
				JSONNODE *action_ = json_at(actions_, i);

				JSONNODE *thread_id_ = json_get(action_, "thread_id");
				JSONNODE *log_body_ = json_get(action_, "log_message_body");
				JSONNODE *log_data_ = json_get(action_, "log_message_data");
				JSONNODE *log_type_ = json_get(action_, "log_message_type");
				if (!log_data_ || !log_body_ || !thread_id_ || !log_type_)
					continue;

				std::tstring thread_id = ptrT(json_as_string(thread_id_));
				std::string type = json_as_pstring(log_type_);
				std::string message_text = json_as_pstring(log_body_);

				if (type == "log:video-call") {
					JSONNODE *other_user_fbid_ = json_get(action_, "other_user_fbid");
					std::string id = json_as_pstring(other_user_fbid_);

					JSONNODE *timestamp = json_get(action_, "timestamp");
					JSONNODE *mid_ = json_get(action_, "message_id");

					std::string message_id = json_as_pstring(mid_);

					facebook_message* message = new facebook_message();
					message->isChat = false;
					message->isUnread = true;
					message->isIncoming = (id != proto->facy.self_.user_id);
					message->message_text = message_text;
					message->time = utils::time::fix_timestamp(json_as_float(timestamp));
					message->user_id = id;
					message->message_id = message_id;
					message->sender_name.clear();
					message->thread_id = json_as_pstring(thread_id_);
					message->type = CALL;

					messages->push_back(message);
				}
				else {
					// TODO: check for other types, now we expect this is rename chat

					if (!proto->m_enableChat)
						continue;

					JSONNODE *name_ = json_get(log_data_, "name");
					std::string name = json_as_pstring(name_);

					// proto->RenameChat(thread_id.c_str(), name.c_str()); // this don't work, why?
					proto->setStringUtf(proto->ChatIDToHContact(thread_id), FACEBOOK_KEY_NICK, name.c_str());

					proto->UpdateChat(thread_id.c_str(), NULL, NULL, message_text.c_str());
				}
			}
		}
		else if (t == "notifications_read" || t == "notifications_seen") {
			ScopedLock s(proto->facy.notifications_lock_);

			JSONNODE *alerts = json_get(it, "alert_ids");

			for (unsigned int j = 0; j < json_size(alerts); j++) {
				JSONNODE *itAlerts = json_at(alerts, j);
				std::string id = json_as_pstring(itAlerts);

				std::map<std::string, facebook_notification*>::iterator it = notifications->find(id);
				if (it != notifications->end()) {
					if (it->second->hWndPopup != NULL)
						PUDeletePopup(it->second->hWndPopup); // close popup

					delete it->second;
					notifications->erase(it);
				}
			}
		}
		else
			continue;
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_unread_threads(std::string *data, std::vector< std::string >* threads, bool inboxOnly)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *unread_threads = json_get(payload, "unread_thread_ids");
	if (unread_threads == NULL) {
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(unread_threads); i++) {
		JSONNODE *it = json_at(unread_threads, i);

		JSONNODE *folder = json_get(it, "folder");
		JSONNODE *thread_ids = json_get(it, "thread_ids");

		if (inboxOnly && json_as_pstring(folder) != "inbox")
			continue;

		for (unsigned int j = 0; j < json_size(thread_ids); j++) {
			JSONNODE *id = json_at(thread_ids, j);
			threads->push_back(json_as_pstring(id));
		}
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_thread_messages(std::string *data, std::vector< facebook_message* >* messages, std::map< std::string, facebook_chatroom* >* chatrooms, bool unreadOnly, bool inboxOnly)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *actions = json_get(payload, "actions");
	JSONNODE *threads = json_get(payload, "threads");
	if (actions == NULL || threads == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *roger = json_get(payload, "roger");
	if (roger != NULL) {
		for (unsigned int i = 0; i < json_size(roger); i++) {
			JSONNODE *it = json_at(roger, i);
			std::tstring id = _A2T(json_name(it));

			// Ignore "wrong" (duplicit) identifiers - these that doesn't begin with "id."
			if (id.substr(0, 3) == _T("id.")) {
				facebook_chatroom *room = new facebook_chatroom(id);
				chatrooms->insert(std::make_pair((char*)_T2A(room->thread_id.c_str()), room));
			}
		}
	}

	std::map<std::string, std::string> thread_ids;
	for (unsigned int i = 0; i < json_size(threads); i++) {
		JSONNODE *it = json_at(threads, i);
		JSONNODE *is_canonical_user = json_get(it, "is_canonical_user");
		JSONNODE *canonical = json_get(it, "canonical_fbid");
		JSONNODE *thread_id = json_get(it, "thread_id");
		JSONNODE *name = json_get(it, "name");
		//JSONNODE *message_count = json_get(it, "message_count");
		//JSONNODE *unread_count = json_get(it, "unread_count"); // TODO: use it to check against number of loaded messages... but how?
		JSONNODE *folder = json_get(it, "folder");

		if (canonical == NULL || thread_id == NULL)
			continue;

		std::string id = json_as_pstring(canonical);
		std::string tid = json_as_pstring(thread_id);

		std::map<std::string, facebook_chatroom*>::iterator iter = chatrooms->find(tid);
		if (iter != chatrooms->end()) {
			if (json_as_bool(is_canonical_user)) {
				chatrooms->erase(iter); // this is not chatroom
			}
			else {
				iter->second->chat_name = ptrT(json_as_string(name)); // TODO: create name from users if there is no name...

				JSONNODE *participants = json_get(it, "participants");
				for (unsigned int j = 0; j < json_size(participants); j++) {
					JSONNODE *jt = json_at(it, j);
					std::string user_id = json_name(jt);
					iter->second->participants.insert(std::make_pair(user_id, user_id)); // TODO: get name somehow
				}
			}
		}

		iter = chatrooms->find(id);
		if (iter != chatrooms->end()) {
			chatrooms->erase(iter); // this is not chatroom
		}

		if (inboxOnly && json_as_pstring(folder) != "inbox")
			continue;

		if (id == "null")
			continue;

		thread_ids.insert(std::make_pair(tid, id));
	}

	for (unsigned int i = 0; i < json_size(actions); i++) {
		JSONNODE *it = json_at(actions, i);

		JSONNODE *author = json_get(it, "author");
		JSONNODE *author_email = json_get(it, "author_email");
		JSONNODE *body = json_get(it, "body");
		JSONNODE *tid = json_get(it, "thread_id");
		JSONNODE *mid = json_get(it, "message_id");
		JSONNODE *timestamp = json_get(it, "timestamp");
		JSONNODE *filtered = json_get(it, "is_filtered_content");
		JSONNODE *folder = json_get(it, "folder");
		JSONNODE *is_unread = json_get(it, "is_unread");

		if (author == NULL || body == NULL || mid == NULL || tid == NULL || timestamp == NULL)
			continue;

		if (inboxOnly && json_as_pstring(folder) != "inbox")
			continue;

		std::string thread_id = json_as_pstring(tid);
		std::string message_id = json_as_pstring(mid);
		std::string message_text = json_as_pstring(body);
		std::string author_id = json_as_pstring(author);
		std::string::size_type pos = author_id.find(":");
		if (pos != std::string::npos)
			author_id = author_id.substr(pos + 1);

		// Process attachements and stickers
		parseAttachments(proto, &message_text, it, thread_id, ""); // FIXME: is here supported other_user_fbid ?

		if (json_as_bool(filtered) && message_text.empty())
			message_text = Translate("This message is no longer available, because it was marked as abusive or spam.");

		message_text = utils::text::trim(utils::text::slashu_to_utf8(message_text), true);
		if (message_text.empty())
			continue;

		bool isUnread = (is_unread != NULL && json_as_bool(is_unread));

		// Ignore read messages if we want only unread messages
		if (unreadOnly && !isUnread)
			continue;

		facebook_message* message = new facebook_message();
		message->message_text = message_text;
		if (author_email != NULL)
			message->sender_name = json_as_pstring(author_email);
		message->time = utils::time::fix_timestamp(json_as_float(timestamp));
		message->thread_id = thread_id;
		message->message_id = message_id;
		message->isIncoming = (author_id != proto->facy.self_.user_id);
		message->isUnread = isUnread;

		std::map<std::string, facebook_chatroom*>::iterator iter = chatrooms->find(thread_id);
		if (iter != chatrooms->end()) {
			// this is chatroom message
			message->isChat = true;
			message->user_id = author_id;
		}
		else {
			// this is standard message
			message->isChat = false;
			std::map<std::string, std::string>::iterator iter = thread_ids.find(thread_id);
			if (iter != thread_ids.end()) {
				message->user_id = iter->second; // TODO: Check if we have contact with this ID in friendlist and otherwise do something different?
			} else {
				delete message;
				continue;
			}
		}

		messages->push_back(message);
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_thread_info(std::string *data, std::string* user_id)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *threads = json_get(payload, "threads");
	if (threads == NULL) {
		return EXIT_FAILURE;
	}

	//std::map<std::string, std::string> thread_ids;
	for (unsigned int i = 0; i < json_size(threads); i++) {
		JSONNODE *it = json_at(threads, i);
		JSONNODE *canonical = json_get(it, "canonical_fbid");
		JSONNODE *thread_id = json_get(it, "thread_id");
		//JSONNODE *message_count = json_get(it, "message_count"); // TODO: this could be useful for loading history from server

		if (canonical == NULL || thread_id == NULL)
			continue;

		std::string id = json_as_pstring(canonical);
		if (id == "null")
			continue;

		*user_id = id;
	}

	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_user_info(std::string *data, facebook_user* fbu)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	JSONNODE *profiles = json_get(payload, "profiles");
	if (profiles == NULL) {
		return EXIT_FAILURE;
	}

	//std::map<std::string, std::string> user_ids;
	for (unsigned int i = 0; i < json_size(profiles); i++) {
		JSONNODE *it = json_at(profiles, i);

		// TODO: allow more users to parse at once
		std::string id = json_name(it);

		if (fbu->user_id == id) {
			parseUser(it, fbu);
			break;
		}
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_chat_info(std::string *data, facebook_chatroom* fbc)
{
	std::string jsonData = data->substr(9);

	JSONROOT root(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	//JSONNODE *actions = json_get(payload, "actions");
	JSONNODE *threads = json_get(payload, "threads");
	if (/*actions == NULL || */threads == NULL) {
		return EXIT_FAILURE;
	}

	/*JSONNODE *roger = json_get(payload, "roger");
	if (roger != NULL) {
	for (unsigned int i = 0; i < json_size(roger); i++) {
	JSONNODE *it = json_at(roger, i);
	std::tstring id = _A2T(json_name(it));

	// Ignore "wrong" (duplicit) identifiers - these that doesn't begin with "id."
	if (id.substr(0, 3) == _T("id.")) {
	facebook_chatroom *room = new facebook_chatroom();
	room->thread_id = id;

	chatrooms->insert(std::make_pair((char*)_T2A(room->thread_id.c_str()), room));
	}
	}
	}*/

	//std::map<std::string, std::string> thread_ids;
	for (unsigned int i = 0; i < json_size(threads); i++) {
		JSONNODE *it = json_at(threads, i);

		JSONNODE *is_canonical_user_ = json_get(it, "is_canonical_user");
		JSONNODE *thread_id_ = json_get(it, "thread_id");
		JSONNODE *name_ = json_get(it, "name");
		//JSONNODE *message_count_ = json_get(it, "message_count");
		//JSONNODE *unread_count_ = json_get(it, "unread_count"); // TODO: use it to check against number of loaded messages... but how?

		if (thread_id_ == NULL || is_canonical_user_ == NULL || json_as_bool(is_canonical_user_))
			continue;

		std::tstring tid = ptrT(json_as_string(thread_id_));

		// TODO: allow more users to parse at once
		if (fbc->thread_id != tid) {
			continue;
		}

		JSONNODE *participants = json_get(it, "participants");
		for (unsigned int j = 0; j < json_size(participants); j++) {
			JSONNODE *jt = json_at(participants, j);
			std::string user_id = json_as_pstring(jt);
			fbc->participants.insert(std::make_pair(user_id.substr(5), ""));
		}

		fbc->chat_name = ptrT(json_as_string(name_));
	}

	return EXIT_SUCCESS;
}
