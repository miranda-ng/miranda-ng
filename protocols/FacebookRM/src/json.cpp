/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-15 Robert P�sel

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

int facebook_json_parser::parse_buddy_list(std::string *data, List::List< facebook_user >* buddy_list)
{
	facebook_user* current = NULL;
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (payload.empty()) {
		return EXIT_FAILURE;
	}

	const JSONNode &list = payload["buddy_list"];
	if (list.empty()) {
		return EXIT_FAILURE;
	}

	// Set all contacts in map to offline
	for (List::Item< facebook_user >* i = buddy_list->begin(); i != NULL; i = i->next) {
		i->data->status_id = ID_STATUS_OFFLINE;
	}

	// Load last active times
	const JSONNode &lastActive = list["last_active_times"];
	for (auto it = lastActive.begin(); it != lastActive.end(); ++it) {
		const char *id = (*it).name();

		current = buddy_list->find(id);
		if (current == NULL) {
			buddy_list->insert(std::make_pair(id, new facebook_user()));
			current = buddy_list->find(id);
			current->user_id = id;
		}

		current->last_active = (*it).as_int();
	}

	// Find mobile friends
	const JSONNode &mobileFriends = list["mobile_friends"];
	for (auto it = mobileFriends.begin(); it != mobileFriends.end(); ++it) {
		std::string id = (*it).as_string();

		current = buddy_list->find(id);
		if (current == NULL) {
			buddy_list->insert(std::make_pair(id, new facebook_user()));
			current = buddy_list->find(id);
			current->user_id = id;
		}

		current->status_id = ID_STATUS_OFFLINE;
		current->client = CLIENT_MOBILE;
	}

	// Find now available contacts
	const JSONNode &nowAvailable = list["nowAvailableList"];
	for (auto it = nowAvailable.begin(); it != nowAvailable.end(); ++it) {
		const char *id = (*it).name();

		current = buddy_list->find(id);
		if (current == NULL) {
			buddy_list->insert(std::make_pair(id, new facebook_user()));
			current = buddy_list->find(id);
			current->user_id = id;
		}

		current->status_id = ID_STATUS_ONLINE;

		const JSONNode &p = (*it)["p"];
		if (p != NULL) {
			std::string status = p["status"].as_string(); // this seems to be "active" everytime
			std::string webStatus = p["webStatus"].as_string(); // "active", "idle" or "offline"
			std::string fbAppStatus = p["fbAppStatus"].as_string(); // "offline" or "active"
			std::string messengerStatus = p["messengerStatus"].as_string(); // "offline" or "active"
			std::string otherStatus = p["otherStatus"].as_string(); // "offline" or "active" - this seems to be "active" when webStatus is "idle" or "active" only

			// this may never happen
			if (status != "active")
				current->status_id = ID_STATUS_OFFLINE;

			bool b;

			// "webStatus" and "otherStatus" are marked as "WEB" on FB website
			if ((b = (webStatus == "active")) || otherStatus == "active") {
				current->status_id = ID_STATUS_ONLINE;
				current->client = b ? CLIENT_WEB : CLIENT_OTHER;
			}

			// "fbAppStatus" and "messengerStatus" are marked as "MOBILE" on FB website
			if ((b = (fbAppStatus == "active")) || messengerStatus == "active") {
				current->status_id = ID_STATUS_ONTHEPHONE;
				current->client = b ? CLIENT_APP : CLIENT_MESSENGER;
			}

			// this is not marked anyhow on website (yet?)
			current->idle = webStatus == "idle" || otherStatus == "idle" || fbAppStatus == "idle" || messengerStatus == "idle";
		}
	}

	// Get aditional informations about contacts (if available)
	const JSONNode &userInfos = list["userInfos"];
	for (auto it = userInfos.begin(); it != userInfos.end(); ++it) {
		const char *id = (*it).name();

		current = buddy_list->find(id);
		if (current == NULL)
			continue;

		std::string name = (*it)["name"].as_string();
		if (!name.empty())
			current->real_name = utils::text::slashu_to_utf8(name);

		std::string thumbSrc = (*it)["thumbSrc"].as_string();
		if (!thumbSrc.empty())
			current->image_url = utils::text::slashu_to_utf8(thumbSrc);
	}

	return EXIT_SUCCESS;
}

void parseUser(const JSONNode &it, facebook_user *fbu)
{
	fbu->user_id = it.name();

	std::string id = it["id"].as_string();
	if (id.empty() || id == "0") {
		// this user has deleted account or is just unavailable for us (e.g., ignore list) -> don't read dummy name and avatar and rather ignore that completely
		return;
	}

	std::string name = it["name"].as_string();
	std::string thumbSrc = it["thumbSrc"].as_string();
	std::string vanity = it["vanity"].as_string(); // username
	int gender = it["gender"].as_int();

	//const JSONNode &uri = it["uri"); // profile url
	//const JSONNode &is_friend = it["is_friend"); // e.g. "True"
	//const JSONNode &type = it["type"); // e.g. "friend" (classic contact) or "user" (disabled/deleted account)

	if (!name.empty())
		fbu->real_name = utils::text::slashu_to_utf8(name);
	if (!thumbSrc.empty())
		fbu->image_url = utils::text::slashu_to_utf8(thumbSrc);
	if (!vanity.empty())
		fbu->username = utils::text::slashu_to_utf8(vanity);

	if (gender) {
		switch (gender) {
		case 1: // female
			fbu->gender = 70;
			break;
		case 2: // male
			fbu->gender = 77;
			break;
			// case 7: not available female?
		}
	}
}

int facebook_json_parser::parse_friends(std::string *data, std::map< std::string, facebook_user* >* friends)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (payload.empty())
		return EXIT_FAILURE;

	for (auto it = payload.begin(); it != payload.end(); ++it) {
		facebook_user *fbu = new facebook_user();
		parseUser(*it, fbu);

		friends->insert(std::make_pair(fbu->user_id, fbu));
	}

	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_notifications(std::string *data, std::map< std::string, facebook_notification* > *notifications)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (payload.empty())
		return EXIT_FAILURE;

	const JSONNode &list = payload["notifications"];
	if (list.empty())
		return EXIT_FAILURE;

	// Create notifications chatroom (if it doesn't exists), because we will be writing to it new notifications here
	proto->PrepareNotificationsChatRoom();

	for (auto it = payload.begin(); it != payload.end(); ++it) {
		const char *id = (*it).name();

		const JSONNode &markup = (*it)["markup"];
		const JSONNode &unread = (*it)["unread"];
		const JSONNode &time = (*it)["time"];

		// Ignore empty and old notifications
		if (markup == NULL || unread == NULL || time == NULL || unread.as_int() == 0)
			continue;

		std::string text = utils::text::html_entities_decode(utils::text::slashu_to_utf8(markup.as_string()));

		facebook_notification* notification = new facebook_notification();

		notification->id = id;
		notification->link = utils::text::source_get_value(&text, 3, "<a ", "href=\"", "\"");
		notification->text = utils::text::remove_html(utils::text::source_get_value(&text, 1, "<abbr"));
		notification->time = utils::time::from_string(time.as_string());

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

bool ignore_duplicits(FacebookProto *proto, const std::string &mid, const std::string &)
{
	ScopedLock s(proto->facy.send_message_lock_);

	std::map<std::string, int>::iterator it = proto->facy.messages_ignore.find(mid);
	if (it != proto->facy.messages_ignore.end()) {
		proto->debugLogA("??? Ignoring duplicit/sent message ID: %s", mid.c_str());
		it->second++; // increase counter (for deleting it later)
		return true;
	}

	// remember this id to ignore duplicits
	proto->facy.messages_ignore.insert(std::make_pair(mid, 1));
	return false;
}

void parseAttachments(FacebookProto *proto, std::string *message_text, const JSONNode &it, const std::string &thread_id, std::string other_user_fbid)
{
	// Process attachements and stickers
	const JSONNode &has_attachment = it["has_attachment"];
	if (!has_attachment.empty() && has_attachment.as_bool()) {
		// Append attachements
		std::string type;
		std::string attachments_text;
		const JSONNode &attachments = it["attachments"];
		for (auto itAttachment = attachments.begin(); itAttachment != attachments.end(); ++itAttachment) {
			const JSONNode &attach_type = (*itAttachment)["attach_type"]; // "sticker", "photo", "file"
			if (attach_type != NULL) {
				// Get attachment type - "file" has priority over other types
				if (type.empty() || type != "file")
					type = attach_type.as_string();
			}
			const JSONNode &name = (*itAttachment)["name"];
			const JSONNode &url = (*itAttachment)["url"];
			if (url != NULL) {
				std::string link = url.as_string();

				if (link.find("/ajax/mercury/attachments/photo/view/") != std::string::npos)
					link = utils::url::decode(utils::text::source_get_value(&link, 2, "?uri=", "&")); // fix photo url
				else if (link.find("/") == 0)
					link = HTTP_PROTO_SECURE FACEBOOK_SERVER_REGULAR + link; // make absolute url

				if (!link.empty()) {
					std::string filename;
					if (name != NULL)
						filename = name.as_string();
					if (filename == "null")
						filename.clear();

					attachments_text += "\n" + (!filename.empty() ? "<" + filename + "> " : "") + link + "\n";

					// Stickers as smileys
					if (type == "sticker" && proto->getByte(FACEBOOK_KEY_CUSTOM_SMILEYS, DEFAULT_CUSTOM_SMILEYS)) {
						const JSONNode &metadata = (*itAttachment)["metadata"];
						if (metadata != NULL) {
							const JSONNode &stickerId_ = metadata["stickerID"];
							if (stickerId_ != NULL) {
								std::string sticker = "[[sticker:" + stickerId_.as_string() + "]]";
								attachments_text += sticker;

								if (other_user_fbid.empty() && !thread_id.empty())
									other_user_fbid = proto->ThreadIDToContactID(thread_id);

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
			/* const JSONNode &admin_snippet = it["admin_snippet");
			if (admin_snippet != NULL) {
			*message_text += admin_snippet);
			} */

			std::tstring newText;
			if (type == "sticker")
				newText = TranslateT("a sticker");
			else if (type == "file")
				newText = (attachments.size() > 1) ? TranslateT("files") : TranslateT("a file");
			else if (type == "photo")
				newText = (attachments.size() > 1) ? TranslateT("photos") : TranslateT("a photo");
			else
				newText = _A2T(type.c_str());

			TCHAR title[200];
			mir_sntprintf(title, SIZEOF(title), TranslateT("User sent %s:"), newText.c_str());

			*message_text += T2Utf(title);
			*message_text += attachments_text;
		}
		else {
			// TODO: better support for these attachments (parse it from "m_messaging" instead of "messaging"
			*message_text += T2Utf(TranslateT("User sent an unsupported attachment. Open your browser to see it."));
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

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &ms = root["ms"];
	if (ms == NULL)
		return EXIT_FAILURE;

	for (auto it = ms.begin(); it != ms.end(); ++it) {
		const JSONNode &type = (*it)["type"];
		if (type == NULL)
			continue;

		std::string t = type.as_string();
		if (t == "messaging") {
			// various messaging stuff (received and sent messages, getting seen info)

			const JSONNode &type = (*it)["event"];
			if (type == NULL)
				continue;

			std::string t = type.as_string();

			if (t == "read_receipt") {
				// user read message
				const JSONNode &reader_ = (*it)["reader"];
				const JSONNode &time_ = (*it)["time"];

				if (reader_ == NULL || time_ == NULL)
					continue;

				time_t timestamp = utils::time::from_string(time_.as_string());
				MCONTACT hContact = NULL;
				std::tstring reader;

				std::string readerId = reader_.as_string();
				const JSONNode &threadid = (*it)["tid"];
				if (threadid == NULL) {
					// classic contact
					hContact = proto->ContactIDToHContact(readerId);
				} else {
					// multi user chat
					if (!proto->m_enableChat)
						continue;

					std::string tid = threadid.as_string();

					std::map<std::string, facebook_chatroom*>::iterator it = proto->facy.chat_rooms.find(tid);
					if (it != proto->facy.chat_rooms.end()) {
						facebook_chatroom *chatroom = it->second;
						std::map<std::string, std::string> participants = chatroom->participants;

						std::map<std::string, std::string>::const_iterator participant = participants.find(readerId);
						if (participant == participants.end()) {
							// TODO: load name of this participant
							std::string name = readerId;
							participants.insert(std::make_pair(readerId, name));
							proto->AddChatContact(tid.c_str(), readerId.c_str(), name.c_str());
						}

						participant = participants.find(readerId);
						if (participant != participants.end()) {
							// TODO: remember just reader ids to avoid eventual duplication of names
							reader = _A2T(participant->second.c_str(), CP_UTF8);
							hContact = proto->ChatIDToHContact(tid);
						}
					}
				}

				if (hContact)
					proto->facy.insert_reader(hContact, timestamp, reader);
			}
			else if (t == "deliver") {
				// inbox message (multiuser or direct)

				const JSONNode &msg = (*it)["message"];
				const JSONNode &folder = (*it)["folder"];

				if (inboxOnly && folder.as_string() != "inbox")
					continue;

				const JSONNode &sender_fbid = msg["sender_fbid"];
				const JSONNode &sender_name = msg["sender_name"];
				const JSONNode &body = msg["body"];

				// looks like there is either "tid" or "other_user_fbid" - or "tid" is removed forever?
				const JSONNode &tid = msg["tid"];
				const JSONNode &other_user_id_ = msg["other_user_fbid"];

				const JSONNode &mid = msg["mid"];
				const JSONNode &timestamp = msg["timestamp"];
				const JSONNode &filtered = (*it)["is_filtered_content"];

				if (sender_fbid == NULL || sender_name == NULL || body == NULL || mid == NULL || timestamp == NULL)
					continue;

				std::string id = sender_fbid.as_string();
				std::string message_id = mid.as_string();
				std::string message_text = body.as_string();

				std::string thread_id = (tid != NULL ? tid.as_string() : "");
				std::string other_user_id = (other_user_id_ != NULL ? other_user_id_.as_string() : "");

				// Process attachements and stickers
				parseAttachments(proto, &message_text, msg, thread_id, other_user_id);

				// Ignore duplicits or messages sent from miranda
				if (body == NULL || ignore_duplicits(proto, message_id, message_text))
					continue;

				if (filtered.as_bool() && message_text.empty())
					message_text = Translate("This message is no longer available, because it was marked as abusive or spam.");

				message_text = utils::text::trim(utils::text::slashu_to_utf8(message_text), true);
				if (message_text.empty())
					continue;

				facebook_message* message = new facebook_message();
				message->isUnread = true;
				message->isIncoming = (id != proto->facy.self_.user_id);
				message->message_text = message_text;
				message->time = utils::time::from_string(timestamp.as_string());
				message->user_id = id;
				message->message_id = message_id;
				message->sender_name = utils::text::slashu_to_utf8(sender_name.as_string()); // TODO: or if not incomming use my own name from facy.self_ ?
				message->thread_id = tid.as_string(); // TODO: or if not incomming use my own id from facy.self_ ?

				const JSONNode &gthreadinfo = msg["group_thread_info"];
				message->isChat = (gthreadinfo != NULL && gthreadinfo.as_string() != "null");

				if (!message->isChat && !message->isIncoming) {
					message->sender_name.clear();
					message->user_id = !other_user_id.empty() ? other_user_id : proto->ThreadIDToContactID(message->thread_id); // TODO: Check if we have contact with this user_id in friendlist and otherwise do something different?
				}

				messages->push_back(message);
			}
		}
		else if (t == "notification_json") {
			// event notification
			const JSONNode &nodes = (*it)["nodes"];

			// Create notifications chatroom (if it doesn't exists), because we will be writing to it new notifications here
			proto->PrepareNotificationsChatRoom();

			for (auto itNodes = nodes.begin(); itNodes != nodes.end(); ++itNodes) {
				const JSONNode &text_ = (*itNodes)["unaggregatedTitle"]; // notifications one by one, not grouped
				if (text_ == NULL)
					continue;
				
				const JSONNode &text = text_["text"];
				const JSONNode &url = (*itNodes)["url"];
				const JSONNode &alert_id = (*itNodes)["alert_id"];

				const JSONNode &time_ = (*itNodes)["timestamp"];
				if (time_ == NULL)
					continue;
				const JSONNode &time = time_["time"];
				if (time == NULL || text == NULL || url == NULL || alert_id == NULL)
					continue;

				time_t timestamp = utils::time::from_string(time.as_string());
				if (timestamp > proto->facy.last_notification_time_) {
					// Only new notifications
					proto->facy.last_notification_time_ = timestamp;

					facebook_notification* notification = new facebook_notification();
					notification->text = utils::text::slashu_to_utf8(text.as_string());
					notification->link = url.as_string();
					notification->id = alert_id.as_string();
					notification->time = timestamp;

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
		else if (t == "m_notification") {
			const JSONNode &data = (*it)["data"];
			if (data == NULL)
				continue;

			const JSONNode &appId_ = data["app_id"];
			const JSONNode &type_ = data["type"];

			if (appId_.as_string() == "2356318349" || type_.as_string() == "friend_confirmed") {
				// Friendship notifications

				const JSONNode &body_ = data["body"];
				const JSONNode &html_ = body_["__html"];

				const JSONNode &href_ = data["href"];
				const JSONNode &unread_ = data["unread"];
				const JSONNode &alertId_ = data["alert_id"];

				if (html_ == NULL || href_ == NULL || unread_ == NULL || unread_.as_int() == 0)
					continue;
				
				std::string text = utils::text::remove_html(utils::text::slashu_to_utf8(html_.as_string()));
				std::string url = href_.as_string();
				std::string alert_id = alertId_.as_string();

				proto->NotifyEvent(proto->m_tszUserName, ptrT(mir_utf8decodeT(text.c_str())), NULL, FACEBOOK_EVENT_FRIENDSHIP, &url, alert_id.empty() ? NULL : &alert_id);
			}
		}
		else if (t == "jewel_requests_add") {
			// New friendship request, load them all with real names (because there is only user_id in "from" field)
			proto->ForkThread(&FacebookProto::ProcessFriendRequests, NULL);
		}
		else if (t == "typ") {
			// chat typing notification

			const JSONNode &from = (*it)["from"];
			if (from == NULL)
				continue;

			facebook_user fbu;
			fbu.user_id = from.as_string();

			MCONTACT hContact = proto->AddToContactList(&fbu, CONTACT_FRIEND); // only friends are able to send typing notifications

			const JSONNode &st = (*it)["st"];
			if (st.as_int() == 1)
				proto->StartTyping(hContact);
			else
				proto->StopTyping(hContact);
		}
		else if (t == "ttyp") {
			// multi chat typing notification
			if (!proto->m_enableChat)
				continue;

			const JSONNode &from_ = (*it)["from"];
			const JSONNode &thread_ = (*it)["thread"];
			const JSONNode &st_ = (*it)["st"];

			if (from_ == NULL || thread_ == NULL || st_ == NULL)
				continue;

			std::string tid = thread_.as_string();
			std::string from_id = from_.as_string();

			std::map<std::string, facebook_chatroom*>::iterator it = proto->facy.chat_rooms.find(thread_.as_string());
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

					if (st_.as_int() == 1) {
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

			const JSONNode &event_type = (*it)["event"];
			const JSONNode &event_data = (*it)["data"];

			if (event_type == NULL || event_data == NULL)
				continue;

			std::string t = event_type.as_string();
			if (t == "visibility_update") {
				// change of chat status
				const JSONNode &visibility = event_data["visibility"];

				bool isVisible = (visibility != NULL) && visibility.as_bool();
				proto->debugLogA("    Requested chat switch to %s", isVisible ? "Online" : "Offline");
				proto->SetStatus(isVisible ? ID_STATUS_ONLINE : ID_STATUS_INVISIBLE);
			}
		}
		else if (t == "buddylist_overlay") {
			// we opened/closed chat window - pretty useless info for us
			continue;
		} else if (t == "ticker_update:home") {
			const JSONNode &actor_ = (*it)["actor"];
			const JSONNode &story_ = (*it)["story_xhp"];

			std::string text = story_.as_string();
			text = utils::text::html_entities_decode(utils::text::slashu_to_utf8(text));

			std::string url = utils::text::source_get_value(&text, 3, "\"tickerStoryLink\"", "href=\"", "\"");
			std::string story_type = utils::text::source_get_value2(&text, "\"type\":\"", "\"");
			std::string story_class = utils::text::source_get_value2(&text, "\"entstory_class\":\"", "\"");

			text = utils::text::trim(utils::text::remove_html(text));

			std::string userId = actor_.as_string();

			MCONTACT hContact = proto->ContactIDToHContact(userId);

			proto->debugLogA("+++ Got ticker type='%s' class='%s'", story_type.c_str(), story_class.c_str());

			if (!text.empty())
				proto->NotifyEvent(proto->m_tszUserName, ptrT(mir_utf8decodeT(text.c_str())), hContact, FACEBOOK_EVENT_TICKER, &url);
		}
		else if (t == "mercury") {
			// rename multi user chat, video call, ...

			const JSONNode &actions_ = (*it)["actions"];
			if (actions_ == NULL)
				continue;

			for (unsigned int i = 0; i < actions_.size(); i++) {
				const JSONNode &action_ = actions_[i];

				const JSONNode &thread_id_ = action_["thread_id"];
				const JSONNode &log_body_ = action_["log_message_body"];
				const JSONNode &log_data_ = action_["log_message_data"];
				const JSONNode &log_type_ = action_["log_message_type"];
				if (log_data_ == NULL || log_body_ == NULL || thread_id_ == NULL || log_type_ == NULL)
					continue;

				std::string thread_id = thread_id_.as_string();
				std::string type = log_type_.as_string();
				std::string message_text = log_body_.as_string();

				if (type == "log:video-call") {
					const JSONNode &other_user_fbid_ = action_["other_user_fbid"];
					std::string id = other_user_fbid_.as_string();

					const JSONNode &timestamp = action_["timestamp"];
					const JSONNode &mid_ = action_["message_id"];

					std::string message_id = mid_.as_string();

					facebook_message* message = new facebook_message();
					message->isChat = false;
					message->isUnread = true;
					message->isIncoming = (id != proto->facy.self_.user_id);
					message->message_text = message_text;
					message->time = utils::time::from_string(timestamp.as_string());
					message->user_id = id;
					message->message_id = message_id;
					message->sender_name.clear();
					message->thread_id = thread_id;
					message->type = CALL;

					messages->push_back(message);
				}
				else {
					// TODO: check for other types, now we expect this is rename chat

					if (!proto->m_enableChat)
						continue;

					const JSONNode &name_ = log_data_["name"];
					std::string name = name_.as_string();

					// proto->RenameChat(thread_id.c_str(), name.c_str()); // this don't work, why?
					proto->setStringUtf(proto->ChatIDToHContact(thread_id), FACEBOOK_KEY_NICK, name.c_str());

					proto->UpdateChat(thread_id.c_str(), NULL, NULL, message_text.c_str());
				}
			}
		}
		else if (t == "notifications_read" || t == "notifications_seen") {
			ScopedLock s(proto->facy.notifications_lock_);

			const JSONNode &alerts = (*it)["alert_ids"];

			for (auto itAlerts = alerts.begin(); itAlerts != alerts.end(); ++itAlerts) {
				std::string id = (*itAlerts).as_string();

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

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (payload == NULL)
		return EXIT_FAILURE;

	const JSONNode &unread_threads = payload["unread_thread_ids"];
	if (unread_threads == NULL)
		return EXIT_FAILURE;

	for (auto it = unread_threads.begin(); it != unread_threads.end(); ++it) {
		const JSONNode &folder = (*it)["folder"];
		const JSONNode &thread_ids = (*it)["thread_ids"];

		if (inboxOnly && folder.as_string() != "inbox")
			continue;

		for (auto it2 = thread_ids.begin(); it2 != thread_ids.end(); ++it2)
			threads->push_back((*it2).as_string());
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_thread_messages(std::string *data, std::vector< facebook_message* >* messages, std::map< std::string, facebook_chatroom* >* chatrooms, bool unreadOnly, bool inboxOnly)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (payload == NULL) {
		return EXIT_FAILURE;
	}

	const JSONNode &actions = payload["actions"];
	const JSONNode &threads = payload["threads"];
	if (actions == NULL || threads == NULL) {
		return EXIT_FAILURE;
	}

	const JSONNode &roger = payload["roger"];
	for (auto it = roger.begin(); it != roger.end(); ++it) {
		std::string id = (*it).name();

		// Ignore "wrong" (duplicit) identifiers - these that doesn't begin with "id."
		if (id.substr(0, 3) == "id.") {
			facebook_chatroom *room = new facebook_chatroom(id);
			chatrooms->insert(std::make_pair(id, room));
		}
	}

	std::map<std::string, std::string> thread_ids;
	for (auto it = threads.begin(); it != threads.end(); ++it) {
		const JSONNode &is_canonical_user = (*it)["is_canonical_user"];
		const JSONNode &canonical = (*it)["canonical_fbid"];
		const JSONNode &thread_id = (*it)["thread_id"];
		const JSONNode &name = (*it)["name"];
		//const JSONNode &message_count = (*it)["message_count");
		//const JSONNode &unread_count = (*it)["unread_count"); // TODO: use it to check against number of loaded messages... but how?
		const JSONNode &folder = (*it)["folder"];

		if (canonical == NULL || thread_id == NULL)
			continue;

		std::string id = canonical.as_string();
		std::string tid = thread_id.as_string();

		std::map<std::string, facebook_chatroom*>::iterator iter = chatrooms->find(tid);
		if (iter != chatrooms->end()) {
			if (is_canonical_user.as_bool()) {
				chatrooms->erase(iter); // this is not chatroom
			}
			else {
				iter->second->chat_name = std::tstring(ptrT(mir_utf8decodeT(name.as_string().c_str()))); // TODO: create name from users if there is no name...

				const JSONNode &participants = (*it)["participants"];
				for (auto it2 = participants.begin(); it2 != participants.end(); ++it2) {
					std::string user_id = (*it2).name();
					iter->second->participants.insert(std::make_pair(user_id, user_id)); // TODO: get name somehow
				}
			}
		}

		iter = chatrooms->find(id);
		if (iter != chatrooms->end()) {
			chatrooms->erase(iter); // this is not chatroom
		}

		if (inboxOnly && folder.as_string() != "inbox")
			continue;

		if (id == "null")
			continue;

		thread_ids.insert(std::make_pair(tid, id));
	}

	for (auto it = actions.begin(); it != actions.end(); ++it) {
		const JSONNode &author = (*it)["author"];
		const JSONNode &author_email = (*it)["author_email"];
		const JSONNode &body = (*it)["body"];
		const JSONNode &tid = (*it)["thread_id"];
		const JSONNode &mid = (*it)["message_id"];
		const JSONNode &timestamp = (*it)["timestamp"];
		const JSONNode &filtered = (*it)["is_filtered_content"];
		const JSONNode &folder = (*it)["folder"];
		const JSONNode &is_unread = (*it)["is_unread"];

		if (author == NULL || body == NULL || mid == NULL || tid == NULL || timestamp == NULL)
			continue;

		if (inboxOnly && folder.as_string() != "inbox")
			continue;

		std::string thread_id = tid.as_string();
		std::string message_id = mid.as_string();
		std::string message_text = body.as_string();
		std::string author_id = author.as_string();
		std::string::size_type pos = author_id.find(":");
		if (pos != std::string::npos)
			author_id = author_id.substr(pos + 1);

		// Process attachements and stickers
		parseAttachments(proto, &message_text, *it, thread_id, ""); // FIXME: is here supported other_user_fbid ?

		if (filtered.as_bool() && message_text.empty())
			message_text = Translate("This message is no longer available, because it was marked as abusive or spam.");

		message_text = utils::text::trim(utils::text::slashu_to_utf8(message_text), true);
		if (message_text.empty())
			continue;

		bool isUnread = is_unread.as_bool();

		// Ignore read messages if we want only unread messages
		if (unreadOnly && !isUnread)
			continue;

		facebook_message* message = new facebook_message();
		message->message_text = message_text;
		if (author_email != NULL)
			message->sender_name = author_email.as_string();
		message->time = utils::time::from_string(timestamp.as_string());
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

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &threads = root["payload"].at("threads");
	if (threads == NULL)
		return EXIT_FAILURE;

	//std::map<std::string, std::string> thread_ids;
	for (auto it = threads.begin(); it != threads.end(); ++it) {
		const JSONNode &canonical = (*it)["canonical_fbid"];
		const JSONNode &thread_id = (*it)["thread_id"];
		//const JSONNode &message_count = (*it)["message_count"); // TODO: this could be useful for loading history from server

		if (canonical == NULL || thread_id == NULL)
			continue;

		std::string id = canonical.as_string();
		if (id == "null")
			continue;

		*user_id = id;
	}

	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_user_info(std::string *data, facebook_user* fbu)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &profiles = root["payload"].at("profiles");
	if (profiles == NULL)
		return EXIT_FAILURE;

	//std::map<std::string, std::string> user_ids;
	for (auto it = profiles.begin(); it != profiles.end(); ++it) {
		// TODO: allow more users to parse at once
		std::string id = (*it).name();

		if (fbu->user_id == id) {
			parseUser(*it, fbu);
			break;
		}
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_chat_info(std::string *data, facebook_chatroom* fbc)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root.empty())
		return EXIT_FAILURE;

	const JSONNode &threads = root["payload"].at("threads");
	if (threads == NULL)
		return EXIT_FAILURE;

	/*const JSONNode &roger = payload, "roger");
	if (roger != NULL) {
	for (unsigned int i = 0; i < json_size(roger); i++) {
	const JSONNode &it = json_at(roger, i);
	std::tstring id = _A2T(json_name(it));

	// Ignore "wrong" (duplicit) identifiers - these that doesn't begin with "id."
	if (id.substr(0, 3) == _T("id.")) {
	facebook_chatroom *room = new facebook_chatroom();
	room->thread_id = id;

	chatrooms->insert(std::make_pair((char*)_T2A(room->thread_id.c_str()), room));
	}
	}
	}*/

	for (auto it = threads.begin(); it != threads.end(); ++it) {
		const JSONNode &is_canonical_user_ = (*it)["is_canonical_user"];
		const JSONNode &thread_id_ = (*it)["thread_id"];
		const JSONNode &name_ = (*it)["name"];
		//const JSONNode &message_count_ = (*it)["message_count");
		//const JSONNode &unread_count_ = (*it)["unread_count"); // TODO: use it to check against number of loaded messages... but how?

		if (thread_id_ == NULL || is_canonical_user_ == NULL || is_canonical_user_.as_bool())
			continue;

		std::string tid = thread_id_.as_string();

		// TODO: allow more users to parse at once
		if (fbc->thread_id != tid)
			continue;

		const JSONNode &participants = (*it)["participants"];
		for (auto jt = participants.begin(); jt != participants.end(); ++jt) {
			std::string user_id = (*jt).as_string();
			fbc->participants.insert(std::make_pair(user_id.substr(5), ""));
		}

		fbc->chat_name = std::tstring(ptrT(mir_utf8decodeT(name_.as_string().c_str())));
	}

	return EXIT_SUCCESS;
}
