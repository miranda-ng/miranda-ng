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

int facebook_json_parser::parse_buddy_list(void* data, List::List< facebook_user >* buddy_list)
{
	facebook_user* current = NULL;
	std::string jsonData = static_cast< std::string* >(data)->substr(9);

	JSONNODE *root = json_parse(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	JSONNODE *list = json_get(payload, "buddy_list");
	if (list == NULL) {
		json_delete(root);
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
			char *id = json_name(it);
			
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
			char *id = json_as_string(it);
			
			current = buddy_list->find(id);
			if (current == NULL) {
				buddy_list->insert(std::make_pair(id, new facebook_user()));
				current = buddy_list->find(id);
				current->user_id = id;
			}

			current->status_id = ID_STATUS_ONTHEPHONE;
		}
	}

	// Find now awailable contacts
	JSONNODE *nowAvailable = json_get(list, "nowAvailableList");
	if (nowAvailable != NULL) {		
		for (unsigned int i = 0; i < json_size(nowAvailable); i++) {
			JSONNODE *it = json_at(nowAvailable, i);
			char *id = json_name(it);
			
			current = buddy_list->find(id);
			if (current == NULL) {
				buddy_list->insert(std::make_pair(id, new facebook_user()));
				current = buddy_list->find(id);
				current->user_id = id;
			}

			current->status_id = ID_STATUS_ONLINE;
			
			// In new version of Facebook "i" means "offline"
			JSONNODE *idle = json_get(it, "i");
			if (idle != NULL && json_as_bool(idle))
				current->status_id = ID_STATUS_OFFLINE;
		}
	}

	// Get aditional informations about contacts (if available)
	JSONNODE *userInfos = json_get(list, "userInfos");
	if (userInfos != NULL) {		
		for (unsigned int i = 0; i < json_size(userInfos); i++) {
			JSONNODE *it = json_at(userInfos, i);
			char *id = json_name(it);
			
			current = buddy_list->find(id);
			if (current == NULL)
				continue;

			JSONNODE *name = json_get(it, "name");
			JSONNODE *thumbSrc = json_get(it, "thumbSrc");

			if (name != NULL)
				current->real_name = utils::text::slashu_to_utf8(utils::text::special_expressions_decode(json_as_string(name)));
			if (thumbSrc != NULL)			
				current->image_url = utils::text::slashu_to_utf8(utils::text::special_expressions_decode(json_as_string(thumbSrc)));
		}
	}

	json_delete(root);
	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_friends(void* data, std::map< std::string, facebook_user* >* friends)
{
	std::string jsonData = static_cast< std::string* >(data)->substr(9);
	
	JSONNODE *root = json_parse(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(payload); i++) {
		JSONNODE *it = json_at(payload, i);
		char *id = json_name(it);

		JSONNODE *name = json_get(it, "name");
		JSONNODE *thumbSrc = json_get(it, "thumbSrc");
		JSONNODE *gender = json_get(it, "gender");
		//JSONNODE *vanity = json_get(it, "vanity"); // username
		//JSONNODE *uri = json_get(it, "uri"); // profile url
		//JSONNODE *is_friend = json_get(it, "is_friend"); // e.g. "True"
		//JSONNODE *type = json_get(it, "type"); // e.g. "friend" (classic contact) or "user" (disabled/deleted account)

		facebook_user *fbu = new facebook_user();

		fbu->user_id = id;
		if (name)
			fbu->real_name = utils::text::slashu_to_utf8(utils::text::special_expressions_decode(json_as_string(name)));
		if (thumbSrc)
			fbu->image_url = utils::text::slashu_to_utf8(utils::text::special_expressions_decode(json_as_string(thumbSrc)));

		if (gender)
			switch (json_as_int(gender)) {
			case 1: // female
				fbu->gender = 70;
				break;
			case 2: // male
				fbu-> gender = 77;
				break;
			}

		friends->insert(std::make_pair(id, fbu));
	}

	json_delete(root);
	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_notifications(void *data, std::vector< facebook_notification* > *notifications) 
{
	std::string jsonData = static_cast< std::string* >(data)->substr(9);
	
	JSONNODE *root = json_parse(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	JSONNODE *list = json_get(payload, "notifications");
	if (list == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(list); i++) {
		JSONNODE *it = json_at(list, i);
		char *id = json_name(it);

		JSONNODE *markup = json_get(it, "markup");
		JSONNODE *unread = json_get(it, "unread");
		//JSONNODE *time = json_get(it, "time");

		// Ignore empty and old notifications
		if (markup == NULL || unread == NULL || json_as_int(unread) == 0)
			continue;

		std::string text = utils::text::slashu_to_utf8(utils::text::special_expressions_decode(json_as_string(markup)));

		facebook_notification* notification = new facebook_notification();

		notification->id = id;
		notification->link = utils::text::source_get_value(&text, 3, "<a ", "href=\"", "\"");
		notification->text = utils::text::remove_html(utils::text::source_get_value(&text, 1, "<abbr"));		

		notifications->push_back(notification);
	}

	json_delete(root);
	return EXIT_SUCCESS;
}

bool ignore_duplicits(FacebookProto *proto, std::string mid, std::string text)
{
	std::map<std::string, bool>::iterator it = proto->facy.messages_ignore.find(mid);
	if (it != proto->facy.messages_ignore.end()) {
		std::string msg = "????? Ignoring duplicit/sent message\n" + text;
		proto->Log(msg.c_str());

		it->second = true; // mark to delete it at the end
		return true;
	}
					
	// remember this id to ignore duplicits
	proto->facy.messages_ignore.insert(std::make_pair(mid, true));
	return false;
}

void parseAttachments(FacebookProto *proto, std::string *message_text, JSONNODE *it)
{
	// Process attachements and stickers
	JSONNODE *has_attachment = json_get(it, "has_attachment");
	if (has_attachment != NULL && json_as_bool(has_attachment)) {
		// Append attachements
		std::string type = "";
		std::string attachments_text = "";
		JSONNODE *attachments = json_get(it, "attachments");
		for (unsigned int j = 0; j < json_size(attachments); j++) {
			JSONNODE *itAttachment = json_at(attachments, j);
			JSONNODE *attach_type = json_get(itAttachment, "attach_type"); // "sticker", "photo", "file"
			if (attach_type != NULL) {
				// Get attachment type - "file" has priority over other types
				if (type.empty() || type != "file")
					type = json_as_string(attach_type);
			}
			JSONNODE *name = json_get(itAttachment, "name");
			JSONNODE *url = json_get(itAttachment, "url");
			if (url != NULL) {
				std::string link = json_as_string(url);
							
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
						filename = json_as_string(name);
					if (filename == "null")
						filename.clear();

					attachments_text += "\n" + (!filename.empty() ? "<" + filename + "> " : "") + link + "\n";
				}
			}
		}

		if (!attachments_text.empty()) {
			// TODO: have this as extra event, not replace or append message content
			if (!message_text->empty())
				*message_text += "\n\n";
			
			// we can use this as offline messages doesn't have it
			/* JSONNODE *admin_snippet = json_get(it, "admin_snippet");
			if (admin_snippet != NULL) {
				*message_text += json_as_string(admin_snippet);
			} */

			if (type == "sticker")
				type = Translate("a sticker");
			else if (type == "file")
				type = (json_size(attachments) > 1) ? Translate("files") : Translate("a file");
			else if (type == "photo")
				type = (json_size(attachments) > 1) ? Translate("photos") : Translate("a photo");

			char title[200];
			mir_snprintf(title, SIZEOF(title), Translate("User sent you %s:"), type.c_str()); 

			*message_text += title;
			*message_text += attachments_text;
		}
	}
}

int facebook_json_parser::parse_messages(void* data, std::vector< facebook_message* >* messages, std::vector< facebook_notification* >* notifications)
{
	std::string jsonData = static_cast< std::string* >(data)->substr(9);

	JSONNODE *root = json_parse(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *ms = json_get(root, "ms");
	if (ms == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(ms); i++) {
		JSONNODE *it = json_at(ms, i);
		
		JSONNODE *type = json_get(it, "type");
		if (type == NULL)
			continue;

		std::string t = json_as_string(type);
		if (t == "msg" || t == "offline_msg") {
			// we use this only for outgoing messages
			
			// TODO: load recipient id here, but use data from "messaging" section, problem is that "messaging" is usually before this

			JSONNODE *msg = json_get(it, "msg");
			if (msg == NULL)
				continue;

			JSONNODE *from = json_get(it, "from");
			JSONNODE *from_name = json_get(it, "from_name");
			JSONNODE *text = json_get(msg, "text");
			JSONNODE *messageId = json_get(msg, "messageId");
			JSONNODE *time = json_get(msg, "time");
			// JSONNODE *tab_type = json_get(it, "tab_type"); // e.g. "friend"

			if (from == NULL || from_name == NULL || text == NULL || messageId == NULL || time == NULL)
				continue;

			char *from_id = json_as_string(from);

			// ignore incomming messages
			if (from_id != proto->facy.self_.user_id)
				continue;

			std::string message_id = json_as_string(messageId);
			std::string message_text = json_as_string(text);

			message_text = utils::text::trim(utils::text::special_expressions_decode(utils::text::slashu_to_utf8(message_text)), true);
			if (message_text.empty())
				continue;

			JSONNODE *truncated = json_get(msg, "truncated");
			if (truncated != NULL && json_as_int(truncated) == 1) {
				std::string msg = "????? We got truncated message\n";
				msg += utils::text::special_expressions_decode(utils::text::slashu_to_utf8(message_text));
				proto->Log(msg.c_str());
			}

			// ignore duplicits or messages sent from miranda
			if (ignore_duplicits(proto, message_id, message_text))
				continue;

			// Outgoing message
			JSONNODE *to = json_get(it, "to");
			if (to == NULL)
				continue;

			JSONNODE *to_name = json_get(it, "to_name");

			facebook_message* message = new facebook_message();
			message->message_text = message_text;
			message->sender_name = utils::text::special_expressions_decode(utils::text::slashu_to_utf8(json_as_string(to_name)));
			message->time = utils::time::fix_timestamp(json_as_float(time));
			message->user_id = json_as_string(to); // TODO: Check if we have contact with this ID in friendlist and otherwise do something different?
			message->message_id = message_id;
			message->isIncoming = false;

			messages->push_back(message);
		} else if (t == "messaging") {
			// we use this only for incomming messages (and getting seen info)

			JSONNODE *type = json_get(it, "event");
			if (type == NULL)
				continue;

			std::string t = json_as_string(type);

			if (t == "read_receipt") {
				// user read message
				JSONNODE *reader = json_get(it, "reader");
				JSONNODE *time = json_get(it, "time");

				if (reader == NULL || time == NULL)
					continue;

				// TODO: add check for chat contacts
				HANDLE hContact = proto->ContactIDToHContact(json_as_string(reader));
				if (hContact) {
					TCHAR ttime[64], tstr[100];
					_tcsftime(ttime, SIZEOF(ttime), _T("%X"), utils::conversion::fbtime_to_timeinfo(json_as_float(time)));
					mir_sntprintf(tstr, SIZEOF(tstr), TranslateT("Message read: %s"), ttime);

					CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hContact, (LPARAM)tstr);
				}
			} else if (t == "deliver") {
				// inbox message (multiuser or direct)

				JSONNODE *msg = json_get(it, "message");

				JSONNODE *sender_fbid = json_get(msg, "sender_fbid");
				JSONNODE *sender_name = json_get(msg, "sender_name");
				JSONNODE *body = json_get(msg, "body");
				JSONNODE *tid = json_get(msg, "tid");
				JSONNODE *mid = json_get(msg, "mid");
				JSONNODE *timestamp = json_get(msg, "timestamp");

				if (sender_fbid == NULL || sender_name == NULL || body == NULL || mid == NULL || timestamp == NULL)
					continue;

				std::string id = json_as_string(sender_fbid);
				std::string message_id = json_as_string(mid);
				std::string message_text = json_as_string(body);

				// Process attachements and stickers
				parseAttachments(proto, &message_text, msg);

				// Ignore messages from myself, as there is no id of recipient
				if (id == proto->facy.self_.user_id)
					continue;

				// Ignore duplicits or messages sent from miranda
				if (body == NULL || ignore_duplicits(proto, message_id, message_text))
					continue;

				message_text = utils::text::trim(utils::text::special_expressions_decode(utils::text::slashu_to_utf8(message_text)), true);
				if (message_text.empty())
					continue;

				facebook_message* message = new facebook_message();
				message->message_text = message_text;
				message->sender_name = utils::text::special_expressions_decode(utils::text::slashu_to_utf8(json_as_string(sender_name)));
				message->time = utils::time::fix_timestamp(json_as_float(timestamp));
				message->user_id = id; // TODO: Check if we have contact with this ID in friendlist and otherwise do something different?
				message->message_id = message_id;

				messages->push_back(message);
			}
		} else if (t == "thread_msg") {
			// multiuser message

			JSONNODE *from_name = json_get(it, "from_name");
			JSONNODE *to_name_ = json_get(it, "to_name");
			if (to_name_ == NULL)
				continue;
			JSONNODE *to_name = json_get(to_name_, "__html");
			JSONNODE *to_id = json_get(it, "to");
			JSONNODE *from_id = json_get(it, "from");

			JSONNODE *msg = json_get(it, "msg");
			JSONNODE *text = json_get(msg, "text");
			JSONNODE *messageId = json_get(msg, "messageId");

			if (from_id == NULL || text == NULL || messageId == NULL)
				continue;

			std::string id = json_as_string(from_id);
			std::string message_id = json_as_string(messageId);
			std::string message_text = json_as_string(text);

			// Ignore messages from myself
			if (id == proto->facy.self_.user_id)
				continue;
				
			// Ignore duplicits or messages sent from miranda
			if (ignore_duplicits(proto, message_id, message_text))
				continue;

			message_text = utils::text::trim(utils::text::special_expressions_decode(utils::text::slashu_to_utf8(message_text)), true);
			if (message_text.empty())
				continue;

			std::string title = utils::text::special_expressions_decode(utils::text::slashu_to_utf8(json_as_string(to_name)));
			std::string url = "/?action=read&sk=inbox&page&query&tid=" + id;
			std::string popup_text = utils::text::special_expressions_decode(utils::text::slashu_to_utf8(json_as_string(from_name)));
			popup_text += ": " + message_text;

			proto->Log("      Got multichat message");
		    
			TCHAR* szTitle = mir_utf8decodeT(title.c_str());
			TCHAR* szText = mir_utf8decodeT(popup_text.c_str());
			proto->NotifyEvent(szTitle, szText, NULL, FACEBOOK_EVENT_OTHER, &url);
			mir_free(szTitle);
			mir_free(szText);
		} else if (t == "notification_json") {
			// event notification

			if (!proto->getByte(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
				continue;

			JSONNODE *nodes = json_get(it, "nodes");
			for (unsigned int j = 0; j < json_size(nodes); j++) {
				JSONNODE *itNodes = json_at(nodes, j);

				//JSONNODE *text = json_get(itNodes, "title/text");
				JSONNODE *text_ = json_get(itNodes, "unaggregatedTitle");
				if (text_ == NULL)
					continue;
				JSONNODE *text = json_get(text_, "text");
				JSONNODE *url = json_get(itNodes, "url");
				JSONNODE *alert_id = json_get(itNodes, "alert_id");
				
				JSONNODE *time_ = json_get(itNodes, "timestamp");
				if (time_ == NULL)
					continue;
				JSONNODE *time = json_get(time_, "time");
				if (time == NULL || text == NULL || url == NULL || alert_id == NULL || time == NULL)
					continue;

				unsigned __int64 timestamp = json_as_float(time);
				if (timestamp > proto->facy.last_notification_time_) {
					// Only new notifications
					proto->facy.last_notification_time_ = timestamp;

					facebook_notification* notification = new facebook_notification();
					notification->text = utils::text::slashu_to_utf8(json_as_string(text));
  					notification->link = utils::text::special_expressions_decode(json_as_string(url));					
					notification->id = json_as_string(alert_id);

					std::string::size_type pos = notification->id.find(":");
					if (pos != std::string::npos)
						notification->id = notification->id.substr(pos+1);

					notifications->push_back(notification);
				}
			}
		} else if (t == "typ") {
			// chat typing notification

			JSONNODE *from = json_get(it, "from");
			if (from == NULL)
				continue;

			facebook_user fbu;
			fbu.user_id = json_as_string(from);

			HANDLE hContact = proto->AddToContactList(&fbu, CONTACT_FRIEND);
				
			if (proto->getWord(hContact, "Status", 0) == ID_STATUS_OFFLINE)
				proto->setWord(hContact, "Status", ID_STATUS_ONLINE);

			JSONNODE *st = json_get(it, "st");
			if (json_as_int(st) == 1)
				CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)60);
			else
				CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
		} else if (t == "privacy_changed") {
			// settings changed

			JSONNODE *event_type = json_get(it, "event");
			JSONNODE *event_data = json_get(it, "data");

			if (event_type == NULL || event_data == NULL)
				continue;

			std::string t = json_as_string(event_type);
			if (t == "visibility_update") {
				// change of chat status
				JSONNODE *visibility = json_get(event_data, "visibility");
				
				bool isVisible = (visibility != NULL) && json_as_bool(visibility);
				proto->Log("      Requested chat switch to %s", isVisible ? "Online" : "Offline");
				proto->SetStatus(isVisible ? ID_STATUS_ONLINE : ID_STATUS_INVISIBLE);
			}
		}
		else
			continue;
	}

	// remove received messages from map		
	for (std::map<std::string, bool>::iterator it = proto->facy.messages_ignore.begin(); it != proto->facy.messages_ignore.end(); ) {
		if (it->second)
			it = proto->facy.messages_ignore.erase(it);
		else
			++it;
	}

	json_delete(root);
	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_unread_threads(void* data, std::vector< std::string >* threads)
{
	std::string jsonData = static_cast< std::string* >(data)->substr(9);
	
	JSONNODE *root = json_parse(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	JSONNODE *unread_threads = json_get(payload, "unread_thread_ids");
	if (unread_threads == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	for (unsigned int i = 0; i < json_size(unread_threads); i++) {
		JSONNODE *it = json_at(unread_threads, i);

		JSONNODE *folder = json_get(it, "folder");
		JSONNODE *thread_ids = json_get(it, "thread_ids");

		for (unsigned int j = 0; j < json_size(thread_ids); j++) {
			JSONNODE *id = json_at(thread_ids, j);
			threads->push_back(json_as_string(id));
		}
	}

	json_delete(root);
	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_thread_messages(void* data, std::vector< facebook_message* >* messages, bool unreadOnly, int limit)
{
	std::string jsonData = static_cast< std::string* >(data)->substr(9);

	JSONNODE *root = json_parse(jsonData.c_str());
	if (root == NULL)
		return EXIT_FAILURE;

	JSONNODE *payload = json_get(root, "payload");
	if (payload == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	JSONNODE *actions = json_get(payload, "actions");
	JSONNODE *threads = json_get(payload, "threads");
	if (actions == NULL || threads == NULL) {
		json_delete(root);
		return EXIT_FAILURE;
	}

	std::map<std::string, std::string> thread_ids;
	for (unsigned int i = 0; i < json_size(threads); i++) {
		JSONNODE *it = json_at(threads, i);
		JSONNODE *canonical = json_get(it, "canonical_fbid");
		JSONNODE *thread_id = json_get(it, "thread_id");
		JSONNODE *unread_count = json_get(it, "unread_count"); // TODO: use it to check against number of loaded messages... but how?
		
		if (canonical == NULL || thread_id == NULL)
			continue;

		std::string id = json_as_string(canonical);
		if (id == "null")
			continue;
		
		std::string tid = json_as_string(thread_id);
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

		if (author == NULL || body == NULL || mid == NULL || tid == NULL || timestamp == NULL)
			continue;

		// Ignore read messages if we want only unread messages
		JSONNODE *is_unread = json_get(it, "is_unread");
		if (unreadOnly && (is_unread == NULL || !json_as_bool(is_unread)))
			continue;

		// Try to get user id from "threads" array (and simultaneously ignore multi user threads)		
		std::map<std::string, std::string>::iterator iter = thread_ids.find(json_as_string(tid));
		if (iter == thread_ids.end())
			continue; // not found or ignored multi user thread

		std::string user_id = iter->second;
		std::string message_id = json_as_string(mid);
		std::string message_text = json_as_string(body);
		std::string author_id = json_as_string(author);
		std::string::size_type pos = author_id.find(":");
		if (pos != std::string::npos)
			author_id = author_id.substr(pos+1);

		// Process attachements and stickers
		parseAttachments(proto, &message_text, it);

		message_text = utils::text::trim(utils::text::special_expressions_decode(utils::text::slashu_to_utf8(message_text)), true);
		if (message_text.empty())
			continue;

		facebook_message* message = new facebook_message();
		message->message_text = message_text;
		if (author_email != NULL)
			message->sender_name = json_as_string(author_email);
		message->time = utils::time::fix_timestamp(json_as_float(timestamp));
		message->user_id = user_id; // TODO: Check if we have contact with this ID in friendlist and otherwise do something different?
		message->message_id = message_id;
		message->isIncoming = (author_id != proto->facy.self_.user_id);

		messages->push_back(message);
	}

	json_delete(root);
	return EXIT_SUCCESS;
}
