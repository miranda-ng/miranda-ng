/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-17 Robert P�sel

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

void parseUser(const JSONNode &it, facebook_user *fbu)
{
	fbu->user_id = it.name();

	std::string id = it["id"].as_string(); // same as element's name, but doesn't exists for "page" type
	std::string alternateName = it["alternateName"].as_string(); // nickname
	std::string name = it["name"].as_string();
	std::string thumbSrc = it["thumbSrc"].as_string();
	std::string vanity = it["vanity"].as_string(); // username
	std::string type = it["type"].as_string(); // "friend", "page", "user" (friend with disabled account or not friend)
	bool isFriend = it["is_friend"].as_bool(); // e.g. "True" or "False" for type="friend" (I don't know why), "False" for type="user", doesn't exist for type="page"
	int gender = it["gender"].as_int();
	// bool isMessengerContact = it["is_nonfriend_messenger_contact"].as_bool(); // "True" or "False", but we don't care as "is_friend" and "type" are enough for us

	//const JSONNode &uri = it["uri"); // profile url	

	if (type == "user" && (id.empty() || id == "0")) {
		// this user has deleted account or is just unavailable for us (e.g., ignore list) -> don't read dummy name and avatar and rather ignore that completely
		return;
	}

	if (type == "friend" && isFriend)
		fbu->type = CONTACT_FRIEND;
	else if (type == "user" || (type == "friend" && !isFriend))
		fbu->type = CONTACT_NONE;
	else if (type == "page")
		fbu->type = CONTACT_PAGE;

	if (!name.empty())
		fbu->real_name = utils::text::slashu_to_utf8(name);
	if (!thumbSrc.empty())
		fbu->image_url = utils::text::slashu_to_utf8(thumbSrc);
	if (!vanity.empty())
		fbu->username = utils::text::slashu_to_utf8(vanity);
	if (!alternateName.empty())
		fbu->nick = alternateName;

	if (gender) {
		switch (gender) {
		case 1: // female
			fbu->gender = 70;
			break;
		case 2: // male
			fbu->gender = 77;
			break;
		// case 7: // not available female?
		// case 11: // page
		}
	}
}

void parseMessageType(FacebookProto *proto, facebook_message &message, const JSONNode &log_type_, const JSONNode &log_body_, const JSONNode &log_data_) {
	if (!log_type_ || !log_body_ || !log_data_)
		return;

	std::string logType = log_type_.as_string();

	if (logType == "log:phone-call") {
		message.type = PHONE_CALL;
	}
	else if (logType == "log:video-call") {
		message.type = VIDEO_CALL;
	}
	else if (logType == "log:generic-admin-text") {
		message.type = ADMIN_TEXT;
	}
	else if (logType == "log:subscribe") {
		message.type = SUBSCRIBE;

		const JSONNode &fbids_ = log_data_["added_participants"];
		for (auto it2 = fbids_.begin(); it2 != fbids_.end(); ++it2) {
			std::string id = (*it2).as_string().substr(5); // strip "fbid:" prefix
			if (!message.data.empty())
				message.data += ";";
			message.data += id;
		}
	}
	else if (logType == "log:unsubscribe") {
		message.type = UNSUBSCRIBE;

		const JSONNode &fbids_ = log_data_["removed_participants"];
		for (auto it2 = fbids_.begin(); it2 != fbids_.end(); ++it2) {
			std::string id = (*it2).as_string().substr(5); // strip "fbid:" prefix
			if (!message.data.empty())
				message.data += ";";
			message.data += id;
		}
	}
	else if (logType == "log:thread-name") {
		message.type = THREAD_NAME;
		message.data = log_data_["name"].as_string();
	}
	else if (logType == "log:thread-image") {
		message.type = THREAD_IMAGE;
	}
	else {
		proto->debugLogA("!!! Unknown log type - %s", logType.c_str());
	}
}

int facebook_json_parser::parse_chat_participant_names(std::string *data, std::map<std::string, chatroom_participant>* participants)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &profiles = root["payload"].at("profiles");
	if (!profiles)
		return EXIT_FAILURE;

	for (auto it = profiles.begin(); it != profiles.end(); ++it) {
		std::string userId = (*it).name();
		std::string userName = (*it)["name"].as_string();
		std::string type = (*it)["type"].as_string();

		if (userId.empty() || userName.empty())
			continue;

		auto participant = participants->find(userId);
		if (participant != participants->end()) {
			chatroom_participant &user = participant->second;
			user.nick = userName;

			if (type == "friend")
				user.role = ROLE_FRIEND;
			else if (type == "user")
				user.role = ROLE_NONE;
			else if (type == "page") {
				user.role = ROLE_NONE;
				// Use prefix for "page" users
				user.nick = proto->m_pagePrefix + " " + userName;
			}

			user.loaded = true;
		}
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_friends(std::string *data, std::map< std::string, facebook_user* >* friends, bool loadAllContacts)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (!payload)
		return EXIT_FAILURE;

	for (auto it = payload.begin(); it != payload.end(); ++it) {
		facebook_user *fbu = new facebook_user();
		parseUser(*it, fbu);

		// Facebook now sends also other types of contacts, which we do not want here
		if (!loadAllContacts && fbu->type != CONTACT_FRIEND) {
			delete fbu;
			continue;
		}

		friends->insert(std::make_pair(fbu->user_id, fbu));
	}

	return EXIT_SUCCESS;
}


int facebook_json_parser::parse_notifications(std::string *data, std::map< std::string, facebook_notification* > *notifications)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &list = root["payload"].at("nodes");
	if (!list)
		return EXIT_FAILURE;

	// Create notifications chatroom (if it doesn't exists), because we will be writing to it new notifications here
	proto->PrepareNotificationsChatRoom();

	for (auto it = list.begin(); it != list.end(); ++it) {
		const JSONNode &id_ = (*it)["alert_id"];
		const JSONNode &state_ = (*it)["seen_state"];
		const JSONNode &time_ = (*it)["timestamp"]["time"];
		const JSONNode &text_ = (*it)["title"]["text"];
		const JSONNode &url_ = (*it)["url"];
		const JSONNode &icon_ = (*it)["icon"]["uri"];

		// Ignore empty and old notifications
		if (!text_ || !state_ || state_.as_string() == "SEEN_AND_READ" || !time_)
			continue;

		facebook_notification* notification = new facebook_notification();

		notification->id = id_.as_string();

		// Fix notification ID
		std::string::size_type pos = notification->id.find(":");
		if (pos != std::string::npos)
			notification->id = notification->id.substr(pos + 1);

		notification->link = url_.as_string();
		notification->text = utils::text::html_entities_decode(utils::text::slashu_to_utf8(text_.as_string()));
		notification->time = utils::time::from_string(time_.as_string());
		notification->setIcon(icon_.as_string());

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

bool ignore_duplicits(FacebookProto *proto, const std::string &mid)
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

std::string absolutizeUrl(std::string &url) {
	if (url.find("/") == 0) {
		url = HTTP_PROTO_SECURE FACEBOOK_SERVER_REGULAR + url;
	}
	return url;
}

void parseAttachments(FacebookProto *proto, std::string *message_text, const JSONNode &delta_, std::string other_user_fbid, bool legacy)
{
	std::string attachments_text;
	std::string type;

	const JSONNode &attachments_ = delta_["attachments"];
	if (!attachments_ || attachments_.empty())
		return;
		
	for (auto itAttachment = attachments_.begin(); itAttachment != attachments_.end(); ++itAttachment) {
		const JSONNode &attach_ = legacy ? (*itAttachment) : (*itAttachment)["mercury"];

		type = attach_["attach_type"].as_string();  // "sticker", "photo", "file", "share", "animated_image", "video"

		if (type == "photo") {
			std::string filename = attach_["name"].as_string();
			std::string link = attach_["hires_url"].as_string();
			if (link.empty()) {
				link = attach_["large_preview_url"].as_string();
			}
			if (link.empty()) {
				link = attach_["preview_url"].as_string();
			}

			if (!link.empty()) {
				attachments_text += "\n" + (!filename.empty() ? "<" + filename + "> " : "") + absolutizeUrl(link) + "\n";
			}
		}
		else if (type == "file" || type == "video") {
			std::string filename = attach_["name"].as_string();
			std::string link = attach_["url"].as_string();

			if (!link.empty()) {
				attachments_text += "\n" + (!filename.empty() ? "<" + filename + "> " : "") + absolutizeUrl(link) + "\n";
			}
		}
		else if (type == "share") {
			const JSONNode &share = attach_["share"];
			if (share) {
				std::string title = share["title"] ? share["title"].as_string() : "";
				std::string description = share["description"] ? share["description"].as_string() : "";
				std::string link = share["uri"].as_string();

				// shorten long descriptions
				description = utils::text::truncate_utf8(description, MAX_LINK_DESCRIPTION_LEN);

				if (link.find("//www." FACEBOOK_SERVER_DOMAIN "/l.php") != std::string::npos || link.find("//l." FACEBOOK_SERVER_DOMAIN) != std::string::npos) {
					// de-facebook this link
					link = utils::url::decode(utils::text::source_get_value2(&link, "l.php?u=", "&", true));
				}

				if (!link.empty()) {
					attachments_text += "\n";
					if (!title.empty())
						attachments_text += title + "\n";
					if (!description.empty())
						attachments_text += description + "\n";
					attachments_text += absolutizeUrl(link) + "\n";
				}
			}
		}
		else if (type == "sticker") {
			std::string link = attach_["url"].as_string();
			if (!link.empty()) {
				attachments_text += "\n" + absolutizeUrl(link) + "\n";
			}

			const JSONNode &metadata = attach_["metadata"];
			if (metadata) {
				const JSONNode &stickerId_ = metadata["stickerID"];
				if (stickerId_) {
					std::string sticker = "[[sticker:" + stickerId_.as_string() + "]]\n";
					attachments_text += sticker;

					// Stickers as smileys
					if (proto->getByte(FACEBOOK_KEY_CUSTOM_SMILEYS, DEFAULT_CUSTOM_SMILEYS) && !proto->facy.loading_history) {
						// FIXME: rewrite smileyadd to use custom smileys per protocol and not per contact and then remove this ugliness
						if (!other_user_fbid.empty()) {
							MCONTACT hContact = proto->ContactIDToHContact(other_user_fbid);
							proto->StickerAsSmiley(sticker, link, hContact);
						}
					}
				}
			}
		}
		else if (type == "animated_image") {
			std::string link = attach_["hires_url"].as_string();
			if (link.empty()) {
				link = attach_["large_preview_url"].as_string();
			}
			if (link.empty()) {
				link = attach_["preview_url"].as_string();
			}

			if (!link.empty()) {
				attachments_text += "\n" + absolutizeUrl(link) + "\n";
			}
		}
		else {
			proto->debugLogA("json::parseAttachments (%s) - Unknown attachment type '%s'", legacy ? "legacy" : "not legacy", type.c_str());
		}
	}

	// TODO: have this as extra event, not replace or append message content
	if (!message_text->empty())
		*message_text += "\n\n";

	if (!attachments_text.empty()) {
		std::wstring newText;
		if (type == "sticker")
			newText = TranslateT("a sticker");
		else if (type == "share")
			newText = TranslateT("a link");
		else if (type == "file")
			newText = (attachments_.size() > 1) ? TranslateT("files") : TranslateT("a file");
		else if (type == "photo")
			newText = (attachments_.size() > 1) ? TranslateT("photos") : TranslateT("a photo");
		else if (type == "video")
			newText = TranslateT("a video");
		else if (type == "animated_image")
			newText = TranslateT("a GIF");
		else
			newText = _A2T(type.c_str());

		wchar_t title[200];
		mir_snwprintf(title, TranslateT("User sent %s:"), newText.c_str());

		*message_text += T2Utf(title);
		*message_text += attachments_text;
	}
	else {
		*message_text += T2Utf(TranslateT("User sent an unsupported attachment. Open your browser to see it."));
		proto->debugLogA("json::parseAttachments (%s) - Unsupported attachment:\n%s", legacy ? "legacy" : "not legacy", attachments_.as_string().c_str());
	}
}

bool parseMessageMetadata(FacebookProto *proto, facebook_message &message, const JSONNode &meta_)
{
	if (!meta_)
		return false;

	const JSONNode &actorFbId_ = meta_["actorFbId"]; // who did the action
	const JSONNode &adminText_ = meta_["adminText"]; // description of the event (only for special events, not for user messages)
	const JSONNode &messageId_ = meta_["messageId"];
	const JSONNode &otherUserFbId_ = meta_["threadKey"]["otherUserFbId"]; // for whom the event is (only for single conversations)
	const JSONNode &threadFbId_ = meta_["threadKey"]["threadFbId"]; // thread of the event (only for multi chat conversations)
	const JSONNode &timestamp_ = meta_["timestamp"];

	if (!actorFbId_ || !messageId_ || !timestamp_)
		return false;

	std::string actorFbId = (actorFbId_ ? actorFbId_.as_string() : "");
	std::string otherUserFbId = (otherUserFbId_ ? otherUserFbId_.as_string() : "");
	std::string threadFbId = (!otherUserFbId_ && threadFbId_ ? "id." + threadFbId_.as_string() : ""); // NOTE: we must add "id." prefix as this is threadFbId and we want threadId (but only for multi chats)

	message.isChat = otherUserFbId.empty();
	message.isIncoming = (actorFbId_.as_string() != proto->facy.self_.user_id);
	message.isUnread = message.isIncoming;
	message.message_text = (adminText_ ? adminText_.as_string() : "");
	message.time = utils::time::from_string(timestamp_.as_string());
	message.user_id = message.isChat ? actorFbId : otherUserFbId;
	message.message_id = messageId_.as_string();
	message.thread_id = threadFbId;
	return true;
}

bool processSpecialMessage(FacebookProto *proto, std::vector<facebook_message>* messages, const JSONNode &meta_, MessageType messageType, const std::string &messageData = "")
{
	facebook_message message;
	message.type = messageType;
	message.data = messageData;

	// Parse message metadata
	if (!parseMessageMetadata(proto, message, meta_)) {
		proto->debugLogA("json::processSpecialMessage - given empty messageMetadata");
		return false;
	}

	// Ignore duplicits or messages sent from miranda
	if (ignore_duplicits(proto, message.message_id)) {
		return false;
	}

	messages->push_back(message);
	return true;
}


int facebook_json_parser::parse_messages(std::string *pData, std::vector<facebook_message>* messages, std::map< std::string, facebook_notification* >* notifications)
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

	JSONNode root = JSONNode::parse(pData->substr(9).c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &ms = root["ms"];
	if (!ms)
		return EXIT_FAILURE;

	for (auto it = ms.begin(); it != ms.end(); ++it) {
		const JSONNode &type = (*it)["type"];
		if (!type)
			continue;

		std::string t = type.as_string();
		if (t == "delta") {
			// new messaging stuff

			const JSONNode &delta_ = (*it)["delta"];
			if (!delta_)
				continue;

			const JSONNode &cls_ = delta_["class"];
			std::string cls = cls_.as_string();
			if (cls == "NewMessage") { // revised 5.3.2017		
				facebook_message message;

				// Parse message metadata				
				const JSONNode &meta_ = delta_["messageMetadata"];
				if (!parseMessageMetadata(proto, message, meta_)) {
					proto->debugLogA("json::parse_messages - No messageMetadata element");
					continue;
				}

				// Ignore duplicits or messages sent from miranda
				if (ignore_duplicits(proto, message.message_id))
					continue;

				const JSONNode &body_ = delta_["body"];
				std::string messageText = body_.as_string();

				// Process attachements and stickers
				parseAttachments(proto, &messageText, delta_, (message.isChat ? "" : message.user_id), false);

				message.message_text = utils::text::trim(messageText, true);
				messages->push_back(message);
			}
			else if (cls == "ReplaceMessage") { // revised 5.3.2017
				//const JSONNode &newMessage_ = delta_["newMessage"];
				// In newMessage object: "attachments", "body", "data"["meta_ranges"], "messageMetadata"["actorFbId", "messageId", "threadKey"["otherUserFbId", "threadFbId"], "timestamp"], "ttl"
				// ttl is usually "TTL_OFF"
				// meta_ranges is e.g. meta_ranges=[{"offset":11,"length":3,"type":1,"data":{"name":"timestamp","value":1488715200}}]
			}
			else if (cls == "ReadReceipt") {
				// user read message

				// when read
				const JSONNode &time_ = delta_["actionTimestampMs"];
				if (!time_)
					continue;
				
				time_t timestamp = utils::time::from_string(time_.as_string());

				// for multi chats (not available for single)
				const JSONNode &actor_ = delta_["actorFbId"]; // who read the message
				const JSONNode &thread_ = delta_["threadKey"]["threadFbId"]; // chat thread

				// for single chats (not available for multi)
				const JSONNode &reader_ = delta_["threadKey"]["otherUserFbId"]; // who read the message

				if (actor_ && thread_) {
					// multi chat
					
					// ignore if disabled
					if (!proto->m_enableChat)
						continue;

					std::string readerId = actor_.as_string();
					std::string tid = "id." + thread_.as_string(); // NOTE: threadFbId means just numeric id of thread, without "id." prefix. We add it here to have it consistent with other methods (where threadId is used)

					MCONTACT hContact = proto->ChatIDToHContact(tid);
					proto->facy.insert_reader(hContact, timestamp, readerId);
				}
				else if (reader_) {
					// single chat
					std::string userId = reader_.as_string();

					MCONTACT hContact = proto->ContactIDToHContact(userId);
					proto->facy.insert_reader(hContact, timestamp);
				}
			}
			else if (cls == "MarkRead") { // revised 5.3.2017
				// Messages (thread) was marked as read on server
				//const JSONNode &other_user_id_ = delta_["threadKeys"]["otherUserFbId"]; // for whom the message is (only for single conversations)
				//const JSONNode &thread_fbid_ = delta_["threadKeys"]["threadFbId"]; // thread of the message (only for multi chat conversations)
				//const JSONNode &actionTimestamp_ = delta_["actionTimestamp"]; // timestamp; more recent than watermarkTimestamp
				//const JSONNode &watermarkTimestamp_ = delta_["watermarkTimestamp"]; // timestamp
			}
			else if (cls == "NoOp") { // revised 5.3.2017
				//const JSONNode &numNoOps_ = delta_["numNoOps"]; // number, usually 1 but I don't know what is it for
				continue;
			}
			else if (cls == "ForcedFetch") { // revised 5.3.2017
				// when something related to thread changes (e.g. change of nickname of other user, thread image, etc.)
			}
			else if (cls == "AdminTextMessage") { // revised 5.3.2017
				// various system messages - approving friendship, changing thread nickname, etc.
				const JSONNode &meta_ = delta_["messageMetadata"];
				MessageType messageType = ADMIN_TEXT;

				// TODO: Do something special with some delta types
				const JSONNode &type_ = delta_["type"];
				//const JSONNode &untyped_ = delta_["untypedData"];

				std::string deltaType = type_.as_string();
				if (deltaType == "confirm_friend_request") {
					//const JSONNode &connection_type_ = untyped_["connection_type"]; // e.g. "friend_request"
					//const JSONNode &friend_request_recipient_ = untyped_["friend_request_recipient"]; // userid
					//const JSONNode &friend_request_sender_ = untyped_["friend_request_sender"]; // userid
				}
				else if (deltaType == "change_thread_nickname") {
					//const JSONNode &nickname_ = untyped_["nickname"]; // new nickname
					//const JSONNode &participant_id_ = untyped_["participant_id"]; // user fbid of that participant
				}
				else if (deltaType == "change_thread_theme") {
					//const JSONNode &theme_color_ = untyped_["theme_color"]; // hexa format aarrggbb
				}
				else if (deltaType == "type=change_thread_icon") {
					//const JSONNode &thread_icon_ = untyped_["thread_icon"]; // emoji symbol
				}
				else {
					proto->debugLogA("json::parse_messages - Unknown AdminTextMessage type '%s'", deltaType.c_str());
				}

				if (!processSpecialMessage(proto, messages, meta_, messageType)) {
					// Message wasn't added - either it is duplicate or there was some error
					continue;
				}
			}
			else if (cls == "RTCEventLog") { // revised 5.3.2017
				// various voice/video calls events
				const JSONNode &meta_ = delta_["messageMetadata"];
				MessageType messageType = ADMIN_TEXT;

				// TODO: Do something special with some types
				//const JSONNode &duration_ = delta_["duration"]; // numeric, probably length of call, e.g. 0
				const JSONNode &eventType_ = delta_["eventType"]; // e.g. "VOICE_EVENT", "VIDEO_EVENT"

				std::string eventType = eventType_.as_string();
				if (eventType == "VOICE_EVENT") {
					messageType = PHONE_CALL;
				}
				else if (eventType == "VIDEO_EVENT") {
					messageType = VIDEO_CALL;
				}
				else {
					proto->debugLogA("json::parse_messages - Unknown RTCEventLog type '%s'", eventType.c_str());
				}

				if (!processSpecialMessage(proto, messages, meta_, messageType)) {
					// Message wasn't added - either it is duplicate or there was some error
					continue;
				}
			}
			else if (cls == "ThreadName") {
				// changed thread name (multi user chat)
				const JSONNode &meta_ = delta_["messageMetadata"];
				
				const JSONNode &name_ = delta_["name"]; // new name of the chat (could be empty)
				std::string data = (name_ ? name_.as_string() : "");

				processSpecialMessage(proto, messages, meta_, THREAD_NAME, data);
			}
			else if (cls == "ThreadMuteSettings") {
				//const JSONNode &expireTime_ = delta_["expireTime"]; // timestamp until which this thread will be muted; could be 0 = unmuted
				// _delta["threadKey"] contains "threadFbId" or "otherUserId" identifier
			}
			else if (cls == "ParticipantLeftGroupThread") {
				// user was removed from multi user chat
				const JSONNode &meta_ = delta_["messageMetadata"];
				
				const JSONNode &leftParticipantFbId_ = delta_["leftParticipantFbId"];
				std::string data = (leftParticipantFbId_ ? leftParticipantFbId_.as_string() : "");

				processSpecialMessage(proto, messages, meta_, UNSUBSCRIBE, data);
			}
			else if (cls == "ParticipantsAddedToGroupThread") {
				// user was added to multi user chat
				const JSONNode &meta_ = delta_["messageMetadata"];
				std::string data = "";

				const JSONNode &addedParticipants_ = delta_["addedParticipants"]; // array of added participants
				for (auto it2 = addedParticipants_.begin(); it2 != addedParticipants_.end(); ++it2) {
					//const JSONNode &firstName_ = (*it2)["firstName"];
					//const JSONNode &fullName_ = (*it2)["fullName"];
					//const JSONNode &isMessengerUser_ = (*it2)["isMessengerUser"]; // boolean
					const JSONNode &userFbId_ = (*it2)["userFbId"]; // userid

					// TODO: Take advantage of given fullName so we don't need to load it manually afterwards
					if (userFbId_) {
						if (!data.empty())
							data += ";";
						data += userFbId_.as_string();
					}
				}

				processSpecialMessage(proto, messages, meta_, SUBSCRIBE, data);
			}
			else {
				// DeliveryReceipt, MarkRead, ThreadDelete
				proto->debugLogA("json::parse_messages - Unknown delta class '%s'", cls.c_str());
			}
		}
		else if (t == "notification_json") {
			// event notification
			const JSONNode &nodes = (*it)["nodes"];

			// Create notifications chatroom (if it doesn't exists), because we will be writing to it new notifications here
			proto->PrepareNotificationsChatRoom();

			for (auto itNodes = nodes.begin(); itNodes != nodes.end(); ++itNodes) {
				const JSONNode &text_ = (*itNodes)["unaggregatedTitle"]; // notifications one by one, not grouped
				if (!text_)
					continue;
				
				const JSONNode &text = text_["text"];
				const JSONNode &url = (*itNodes)["url"];
				const JSONNode &alert_id = (*itNodes)["alert_id"];
				const JSONNode &icon_ = (*itNodes)["icon"]["uri"];

				const JSONNode &time_ = (*itNodes)["timestamp"];
				if (!time_)
					continue;
				const JSONNode &time = time_["time"];
				if (!time || !text || !url || !alert_id)
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
					notification->setIcon(icon_.as_string());

					// Fix notification ID
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
			if (!data)
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

				if (!html_ || !href_ || !unread_ || unread_.as_int() == 0)
					continue;
				
				std::string text = utils::text::remove_html(utils::text::slashu_to_utf8(html_.as_string()));
				std::string url = href_.as_string();
				std::string alert_id = alertId_.as_string();

				// Notify it, if user wants to be notified
				if (proto->getByte(FACEBOOK_KEY_EVENT_FRIENDSHIP_ENABLE, DEFAULT_EVENT_FRIENDSHIP_ENABLE)) {
					proto->NotifyEvent(proto->m_tszUserName, ptrW(mir_utf8decodeW(text.c_str())), NULL, EVENT_FRIENDSHIP, &url, alert_id.empty() ? NULL : &alert_id);
				}
			}
		}
		else if (t == "jewel_requests_add") {
			// New friendship request, load them all with real names (because there is only user_id in "from" field)
			proto->ForkThread(&FacebookProto::ProcessFriendRequests, NULL);
		}
		/*else if (t == "jewel_requests_handled") { // revised 5.3.2017
			// When some request is approved (or perhaps even ignored/removed)
			const JSONNode &item_id_ = (*it)["item_id"]; // "<other_userid>_1_req"
			const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
		}
		else if (t == "type=jewel_requests_remove_old") { // revised 5.3.2017
			// Probably same as above? Happened in same situation. Could happen few times in a row.
			const JSONNode &from_ = (*it)["from"]; // other_userid
			const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
		}*/
		else if (t == "typ") { // revised 5.3.2017
			// chat typing notification
			const JSONNode &from_ = (*it)["from"]; // user fbid
			const JSONNode &to_ = (*it)["to"]; // user fbid (should be our own, but could be from our page too)
			//const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
			//const JSONNode &from_mobile_ = (*it)["from_mobile"]; // boolean // TODO: perhaps we should update user client based on this?
			const JSONNode &st_ = (*it)["st"]; // typing status - 1 = started typing, 0 = stopped typing

			// Ignore wrong (without "from") typing notifications or that are not meant for us (but e.g. for our page)
			if (!from_ || to_.as_string() != proto->facy.self_.user_id)
				continue;

			facebook_user fbu;
			fbu.user_id = from_.as_string();
			fbu.type = CONTACT_FRIEND; // only friends are able to send typing notifications
			MCONTACT hContact = proto->AddToContactList(&fbu);

			if (st_.as_int() == 1)
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
			if (!from_ || !thread_ || !st_)
				continue;

			std::string tid = thread_.as_string();
			std::string from_id = from_.as_string();

			auto itRoom = proto->facy.chat_rooms.find(thread_.as_string());
			if (itRoom != proto->facy.chat_rooms.end()) {
				facebook_chatroom *chatroom = itRoom->second;
				std::map<std::string, chatroom_participant> participants = chatroom->participants;

				auto participant = participants.find(from_id);
				if (participant == participants.end()) {
					// TODO: load name of this participant
					chatroom_participant new_participant;
					new_participant.user_id = from_id;
					new_participant.nick = from_id;
					proto->AddChatContact(tid.c_str(), new_participant, true);
				}

				participant = participants.find(from_id);
				if (participant != participants.end()) {
					MCONTACT hChatContact = proto->ChatIDToHContact(tid);
					ptrW name(mir_utf8decodeW(participant->second.nick.c_str()));

					if (st_.as_int() == 1)
						Srmm_SetStatusText(hChatContact, CMStringW(FORMAT, TranslateT("%s is typing a message..."), name));
					else
						Srmm_SetStatusText(hChatContact, nullptr);

					// TODO: support proper MS_PROTO_CONTACTISTYPING service for chatrooms (when it will be implemented)
				}
			}
		}
		else if (t == "privacy_changed") {
			// settings changed

			const JSONNode &event_type = (*it)["event"];
			const JSONNode &event_data = (*it)["data"];
			if (!event_type || !event_data)
				continue;

			std::string et = event_type.as_string();
			if (et == "visibility_update") {
				// change of chat status
				const JSONNode &visibility = event_data["visibility"];

				bool isVisible = visibility && visibility.as_bool();
				proto->debugLogA("    Requested chat switch to %s", isVisible ? "Online" : "Offline");

				// If actual status is not what server says, change it (handle also local away status, which means online)
				if (isVisible != (proto->m_iStatus != ID_STATUS_INVISIBLE))
					proto->SetStatus(isVisible ? ID_STATUS_ONLINE : ID_STATUS_INVISIBLE);
			}
		}
		else if (t == "chatproxy-presence") {
			const JSONNode &buddyList = (*it)["buddyList"];
			if (!buddyList)
				continue;

			time_t offlineThreshold = time(NULL) - 15 * 60; // contacts last active more than 15 minutes will be marked offline

			for (auto itNodes = buddyList.begin(); itNodes != buddyList.end(); ++itNodes) {
				std::string id = (*itNodes).name();

				MCONTACT hContact = proto->ContactIDToHContact(id);
				if (!hContact) {
					// Facebook now sends info also about some nonfriends, so we just ignore status change of contacts we don't have in list
					continue;
				}

				// TODO: Check for friends existence/inexistence? Here we should get all friends (but we're already doing friendslist request, so we should have fresh data already)

				const JSONNode &p_ = (*itNodes)["p"]; // possible values: 0, 2 (something more?) (might not be present)
				const JSONNode &lat_ = (*itNodes)["lat"]; // timestamp of last activity (could be 0) (is always present)
				const JSONNode &vc_ = (*itNodes)["vc"]; // possible values: 0, 8, 10 (something more?) (might not be present)

				int status = ID_STATUS_DND; // DND to easily spot some problem, as we expect it will always be p==0 or p==2 below

				// Probably means presence: 0 = away, 2 = online, when not present then that probably means don't change that status
				if (p_) {
					int p = p_.as_int();

					if (p == 0)
						status = ID_STATUS_AWAY;
					else if (p == 2)
						status = ID_STATUS_ONLINE;

					proto->setWord(hContact, "Status", status);
				}

				// Last active time
				if (lat_) {
					time_t last_active = utils::time::from_string(lat_.as_string());

					if (last_active > 0)
						proto->setDword(hContact, "LastActiveTS", last_active);
					else
						proto->delSetting(hContact, "LastActiveTS");

					// Set users inactive for too long as offline
					if (last_active > 0 && last_active < offlineThreshold) {
						proto->setWord(hContact, "Status", ID_STATUS_OFFLINE);
					}
				}

				// Probably means client: guess 0 = web, 8 = messenger, 10 = something else?
				if (vc_) {
					int vc = vc_.as_int();
					wchar_t *client;

					if (vc == 0) {
						client = FACEBOOK_CLIENT_WEB;
					}
					else if (vc == 8) {
						client = FACEBOOK_CLIENT_MESSENGER; // I was online on Miranda, but when looked at myself at messenger.com I had icon of Messenger.
					}
					else if (vc == 10) {
						client = FACEBOOK_CLIENT_MOBILE;
					}
					else {
						client = FACEBOOK_CLIENT_OTHER;
					}
					proto->setWString(hContact, "MirVer", client);
				}
			}
		}
		else if (t == "buddylist_overlay") {
			// TODO: This is now supported also via /ajax/mercury/tabs_presence.php request (probably)
			// additional info about user status (status, used client)
			const JSONNode &overlay_ = (*it)["overlay"];
			if (!overlay_)
				continue;

			time_t offlineThreshold = time(NULL) - 15 * 60; // contacts last active more than 15 minutes will be marked offline

			for (auto itNodes = overlay_.begin(); itNodes != overlay_.end(); ++itNodes) {
				std::string id = (*itNodes).name();

				MCONTACT hContact = proto->ContactIDToHContact(id);
				if (!hContact) {
					// Facebook now sends info also about some nonfriends, so we just ignore status change of contacts we don't have in list
					continue;
				}

				/* ptrW client(getWStringA(fbu->handle, "MirVer"));
				if (!client || mir_wstrcmp(client, fbu->getMirVer()))
					setWString(fbu->handle, "MirVer", fbu->getMirVer());
				*/

				const JSONNode &a_ = (*itNodes)["a"]; // possible values: 0, 2 (something more?)
				const JSONNode &la_ = (*itNodes)["la"]; // timestamp of last activity (could be 0)
				const JSONNode &s_ = (*itNodes)["s"]; // possible values: push (something more?)
				const JSONNode &vc_ = (*itNodes)["vc"]; // possible values: 0, 8, 10 (something more?)

				// Friller account has also these:
				// const JSONNode &ol_ = (*itNodes)["ol"]; // possible values: -1 (when goes to offline), 0 (when goes back online) (something more?)
				// const JSONNode &p_ = (*itNodes)["p"]; // class with fbAppStatus, messengerStatus, otherStatus, status, webStatus

				int status = ID_STATUS_FREECHAT; // FREECHAT to easily spot some problem, as we expect it will always be p==0 or p==2 below

				if (a_) {
					int a = a_.as_int();

					if (a == 0)
						status = ID_STATUS_OFFLINE;
					else if (a == 2)
						status = ID_STATUS_ONLINE;
				}
				else {
					status = ID_STATUS_OFFLINE;
				}

				if (la_ /*&& status != ID_STATUS_ONLINE*/) {
					time_t last_active = utils::time::from_string(la_.as_string());
					
					// we should set IdleTS only when contact is IDLE, or OFFLINE
					//if (proto->getDword(hContact, "IdleTS", 0) != last_active) {
					//	if (/*(fbu->idle || status == ID_STATUS_OFFLINE) &&*/ last_active > 0)
					//		proto->setDword(hContact, "IdleTS", last_active);
					//	else
					//		proto->delSetting(hContact, "IdleTS");
					//}

					/*if (last_active > 0)
						proto->setDword(hContact, "LastActiveTS", last_active);
					else
						proto->delSetting(hContact, "LastActiveTS");
					*/

					// Set users inactive for too long as offline
					if (last_active > 0 && last_active < offlineThreshold)
						status = ID_STATUS_OFFLINE;
				}
				else {
					proto->delSetting(hContact, "IdleTS");
				}

				proto->setWord(hContact, "Status", status);

				if (s_) {
					// what to do with this?
				}
				// Probably means client: guess 0 = web, 8 = messenger, 10 = something else?
				if (vc_) {
					wchar_t *client = FACEBOOK_CLIENT_WEB;

					/*if (vc == 0) {
						// means active some time ago? (on messenger or also on web)
						client = FACEBOOK_CLIENT_WEB;
					}
					else if (vc == 8) {
						client = FACEBOOK_CLIENT_MESSENGER; // I was online on Miranda, but when looked at myself at messenger.com I had icon of Messenger.
					}
					else if (vc == 10) {
						// means actually active on messenger
						client = FACEBOOK_CLIENT_MOBILE;
					}
					else {
						client = FACEBOOK_CLIENT_OTHER;
					}*/

					proto->setWString(hContact, "MirVer", client);
				}
			}
		} else if (t == "ticker_update:home") {
			if (!proto->getByte(FACEBOOK_KEY_EVENT_TICKER_ENABLE, DEFAULT_EVENT_TICKER_ENABLE))
				continue;

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
				proto->NotifyEvent(proto->m_tszUserName, ptrW(mir_utf8decodeW(text.c_str())), hContact, EVENT_TICKER, &url);
		}
		else if (t == "notifications_read" || t == "notifications_seen") { // revised 5.3.2017
			ScopedLock s(proto->facy.notifications_lock_);

			const JSONNode &alerts = (*it)["alert_ids"];

			for (auto itAlerts = alerts.begin(); itAlerts != alerts.end(); ++itAlerts) {
				std::string id = (*itAlerts).as_string();

				auto itAlert = notifications->find(id);
				if (itAlert != notifications->end()) {
					if (itAlert->second->hWndPopup != NULL)
						PUDeletePopup(itAlert->second->hWndPopup); // close popup

					delete itAlert->second;
					notifications->erase(itAlert);
				}
			}
		}
		/*else if (t == "mobile_requests_count") { // revised 5.3.2017
			// Notifies about remaining friendship requests (happens e.g. after approving friendship)
			const JSONNode &num_friend_confirmed_unseen_ = (*it)["num_friend_confirmed_unseen"]; // number, e.g. "0"
			const JSONNode &num_unread_ = (*it)["num_unread"]; // number, e.g. "0"
			const JSONNode &num_unseen_ = (*it)["num_unseen"]; // number, e.g. "0"
			const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
		}
		else if (t == "friending_state_change") { // revised 5.3.2017
			const JSONNode &userid_ = (*it)["userid"]; // fbid of user this event is about
			const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
			const JSONNode &action_ = (*it)["action"]; // "confirm" = when we approved friendship

			if (action_.as_string() == "confirm") {
				// ...
			}
		}
		else if (t == "inbox") { // revised 5.3.2017
			const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
			const JSONNode &recent_unread_ = (*it)["recent_unread"]; // number
			const JSONNode &seen_timestamp_ = (*it)["seen_timestamp"]; // number
			const JSONNode &unread_ = (*it)["unread"]; // number
			const JSONNode &unseen_ = (*it)["unseen"]; // number
		}
		else if (t == "webrtc") { // revised 5.3.2017
			const JSONNode &realtime_viewer_fbid_ = (*it)["realtime_viewer_fbid"]; // our user fbid
			const JSONNode &source_ = (*it)["source"]; // e.g. "www"
			const JSONNode &msg_type_ = (*it)["msg_type"]; // e.g. "hang_up", "other_dismiss", "ice_candidate", "offer", "offer_ack", ...?
			const JSONNode &id_ = (*it)["id"]; // some numeric id
			const JSONNode &call_id_ = (*it)["call_id"]; // some numeric id
			const JSONNode &from_ = (*it)["from"]; // user fbid that started this event
			const JSONNode &payload_ = (*it)["payload"]; // string with some metadata (usually same as above)
		}*/
		else
			continue;
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_unread_threads(std::string *data, std::vector< std::string >* threads)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &unread_threads = root["payload"].at("unread_thread_fbids");
	if (!unread_threads)
		return EXIT_FAILURE;

	for (auto it = unread_threads.begin(); it != unread_threads.end(); ++it) {
		// For multi user chats
		const JSONNode &thread_fbids = (*it)["thread_fbids"];
		for (auto jt = thread_fbids.begin(); jt != thread_fbids.end(); ++jt)
			threads->push_back((*jt).as_string());

		// For classic conversations
		const JSONNode &other_user_fbids = (*it)["other_user_fbids"];
		for (auto jt = other_user_fbids.begin(); jt != other_user_fbids.end(); ++jt)
			threads->push_back((*jt).as_string());
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_thread_messages(std::string *data, std::vector< facebook_message >* messages, bool unreadOnly)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (!payload)
		return EXIT_FAILURE;

	const JSONNode &actions = payload["actions"];
	const JSONNode &threads = payload["threads"];
	if (!actions || !threads)
		return EXIT_FAILURE;

	for (auto it = actions.begin(); it != actions.end(); ++it) {
		const JSONNode &author_ = (*it)["author"];
		const JSONNode &other_user_fbid_ = (*it)["other_user_fbid"];
		const JSONNode &body_ = (*it)["body"];
		const JSONNode &thread_id_ = (*it)["thread_id"];
		const JSONNode &thread_fbid_ = (*it)["thread_fbid"];
		const JSONNode &mid_ = (*it)["message_id"];
		const JSONNode &timestamp_ = (*it)["timestamp"];
		const JSONNode &filtered_ = (*it)["is_filtered_content"];
		const JSONNode &is_unread_ = (*it)["is_unread"];

		// Either there is "body" (for classic messages), or "log_message_type" and "log_message_body" (for log messages)
		const JSONNode &log_type_ = (*it)["log_message_type"];
		const JSONNode &log_body_ = (*it)["log_message_body"];
		const JSONNode &log_data_ = (*it)["log_message_data"]; // additional data for this log message

		if (!author_ || (!body_ && !log_body_) || !mid_ || (!thread_fbid_ && !thread_id_) || !timestamp_) {
			proto->debugLogA("parse_thread_messages: ignoring message (%s) - missing attribute", mid_.as_string().c_str());
			continue;
		}

		std::string thread_id = thread_id_.as_string();
		std::string thread_fbid = thread_fbid_.as_string();
		std::string message_id = mid_.as_string();
		std::string message_text = body_ ? body_.as_string() : log_body_.as_string();
		std::string author_id = author_.as_string();
		std::string other_user_fbid = other_user_fbid_ ? other_user_fbid_.as_string() : "";
		std::string::size_type pos = author_id.find(":"); // strip "fbid:" prefix
		if (pos != std::string::npos)
			author_id = author_id.substr(pos + 1);

		// Process attachements and stickers
		parseAttachments(proto, &message_text, *it, other_user_fbid, true);

		if (filtered_.as_bool() && message_text.empty())
			message_text = Translate("This message is no longer available, because it was marked as abusive or spam.");

		message_text = utils::text::trim(utils::text::slashu_to_utf8(message_text), true);
		if (message_text.empty()) {
			proto->debugLogA("parse_thread_messages: ignoring message (%s) - empty message text", mid_.as_string().c_str());
			continue;
		}

		bool isUnread = is_unread_.as_bool();

		// Ignore read messages if we want only unread messages
		if (unreadOnly && !isUnread)
			continue;

		facebook_message message;
		message.message_text = message_text;
		message.time = utils::time::from_string(timestamp_.as_string());
		message.message_id = message_id;
		message.isIncoming = (author_id != proto->facy.self_.user_id);
		message.isUnread = isUnread;

		message.isChat = other_user_fbid.empty();
		if (message.isChat) {
			message.user_id = author_id;
			message.thread_id = "id." + thread_fbid;
		}
		else {
			message.user_id = other_user_fbid;
			message.thread_id = thread_id;
		}

		parseMessageType(proto, message, log_type_, log_body_, log_data_);

		messages->push_back(message);
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_history(std::string *data, std::vector< facebook_message >* messages, std::string *firstTimestamp)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &payload = root["payload"];
	if (!payload)
		return EXIT_FAILURE;

	const JSONNode &actions = payload["actions"];
	if (!actions)
		return EXIT_FAILURE;

	bool first = true;

	for (auto it = actions.begin(); it != actions.end(); ++it) {
		const JSONNode &author = (*it)["author"];
		const JSONNode &other_user_fbid = (*it)["other_user_fbid"];
		const JSONNode &body = (*it)["body"];
		const JSONNode &tid = (*it)["thread_id"];
		const JSONNode &mid = (*it)["message_id"];
		const JSONNode &timestamp = (*it)["timestamp"];
		const JSONNode &filtered = (*it)["is_filtered_content"];
		const JSONNode &is_unread = (*it)["is_unread"];

		// Either there is "body" (for classic messages), or "log_message_type" and "log_message_body" (for log messages)
		const JSONNode &log_type_ = (*it)["log_message_type"];
		const JSONNode &log_body_ = (*it)["log_message_body"];
		const JSONNode &log_data_ = (*it)["log_message_data"];

		if (!author || (!body && !log_body_) || !mid || !tid || !timestamp) {
			proto->debugLogA("parse_history: ignoring message (%s) - missing attribute", mid.as_string().c_str());
			continue;
		}

		if (first) {
			*firstTimestamp = timestamp.as_string();
			first = false;
		}

		std::string thread_id = tid.as_string();
		std::string message_id = mid.as_string();
		std::string message_text = body ? body.as_string() : log_body_.as_string();
		std::string author_id = author.as_string();
		std::string other_user_id = other_user_fbid ? other_user_fbid.as_string() : "";
		std::string::size_type pos = author_id.find(":"); // strip "fbid:" prefix
		if (pos != std::string::npos)
			author_id = author_id.substr(pos + 1);

		// Process attachements and stickers
		parseAttachments(proto, &message_text, *it, other_user_id, true);

		if (filtered.as_bool() && message_text.empty())
			message_text = Translate("This message is no longer available, because it was marked as abusive or spam.");

		message_text = utils::text::trim(utils::text::slashu_to_utf8(message_text), true);
		if (message_text.empty()) {
			proto->debugLogA("parse_history: ignoring message (%s) - empty message text", mid.as_string().c_str());
			continue;
		}

		facebook_message message;
		message.message_text = message_text;
		message.time = utils::time::from_string(timestamp.as_string());
		message.thread_id = thread_id;
		message.message_id = message_id;
		message.isIncoming = (author_id != proto->facy.self_.user_id);
		message.isUnread = is_unread.as_bool();
		message.isChat = false;
		message.user_id = other_user_id;

		parseMessageType(proto, message, log_type_, log_body_, log_data_);

		messages->push_back(message);
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_thread_info(std::string *data, std::string* user_id)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &threads = root["payload"].at("threads");
	if (!threads)
		return EXIT_FAILURE;

	//std::map<std::string, std::string> thread_ids;
	for (auto it = threads.begin(); it != threads.end(); ++it) {
		const JSONNode &canonical = (*it)["canonical_fbid"];
		const JSONNode &thread_id = (*it)["thread_id"];
		//const JSONNode &message_count = (*it)["message_count"); // TODO: this could be useful for loading history from server
		if (!canonical || !thread_id)
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
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &profiles = root["payload"].at("profiles");
	if (!profiles)
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
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &threads = root["payload"].at("threads");
	if (!threads)
		return EXIT_FAILURE;

	for (auto it = threads.begin(); it != threads.end(); ++it) {
		const JSONNode &is_canonical_user_ = (*it)["is_canonical_user"];
		const JSONNode &thread_fbid_ = (*it)["thread_fbid"];
		const JSONNode &name_ = (*it)["name"];
		//const JSONNode &message_count_ = (*it)["message_count");
		//const JSONNode &unread_count_ = (*it)["unread_count"); // TODO: use it to check against number of loaded messages... but how?

		if (!thread_fbid_ || !is_canonical_user_ || is_canonical_user_.as_bool())
			continue;

		std::string tid = "id." + thread_fbid_.as_string();

		// TODO: allow more chats to parse at once
		if (fbc->thread_id != tid)
			continue;

		chatroom_participant user;

		user.is_former = true;
		const JSONNode &former_participants = (*it)["former_participants"];
		for (auto jt = former_participants.begin(); jt != former_participants.end(); ++jt) {
			user.role = (*jt)["is_friend"].as_bool() ? ROLE_FRIEND : ROLE_NONE;
			user.user_id = (*jt)["id"].as_string().substr(5); // strip "fbid:" prefix
			fbc->participants.insert(std::make_pair(user.user_id, user));
		}

		user.is_former = false;
		user.role = ROLE_NONE;
		const JSONNode &participants = (*it)["participants"];
		for (auto jt = participants.begin(); jt != participants.end(); ++jt) {
			user.user_id = (*jt).as_string().substr(5); // strip "fbid:" prefix
			fbc->participants.insert(std::make_pair(user.user_id, user));
		}

		// TODO: don't automatically join unsubscribed or archived chatrooms

		fbc->can_reply = (*it)["can_reply"].as_bool();
		fbc->is_archived = (*it)["is_archived"].as_bool();
		fbc->is_subscribed = (*it)["is_subscribed"].as_bool();
		fbc->read_only = (*it)["read_only"].as_bool();
		fbc->chat_name = std::wstring(ptrW(mir_utf8decodeW(name_.as_string().c_str())));
	}

	return EXIT_SUCCESS;
}

int facebook_json_parser::parse_messages_count(std::string *data, int *messagesCount, int *unreadCount)
{
	std::string jsonData = data->substr(9);

	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (!root)
		return EXIT_FAILURE;

	const JSONNode &threads = root["payload"].at("threads");
	if (!threads)
		return EXIT_FAILURE;

	for (auto it = threads.begin(); it != threads.end(); ++it) {
		const JSONNode &message_count_ = (*it)["message_count"];
		const JSONNode &unread_count_ = (*it)["unread_count"];

		if (!message_count_|| !unread_count_)
			return EXIT_FAILURE;

		*messagesCount = message_count_.as_int();
		*unreadCount = unread_count_.as_int();
	}

	return EXIT_SUCCESS;
}
