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

MEVENT FacebookProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	StopTyping(hContact);

	return CSuper::RecvMsg(hContact, pre);
}

void FacebookProto::SendMsgWorker(void *p)
{
	if (p == nullptr)
		return;

	send_direct *data = static_cast<send_direct*>(p);

	ptrA id(getStringA(data->hContact, FACEBOOK_KEY_ID));

	if (!isOnline())
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, (LPARAM)TranslateT("You cannot send messages when you are offline."));
	else if (id == nullptr)
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid);
	else {
		int tries = getByte(FACEBOOK_KEY_SEND_MESSAGE_TRIES, 1);
		tries = min(max(tries, 1), 5);

		std::string error_text;
		int result = SEND_MESSAGE_ERROR;
		while (result == SEND_MESSAGE_ERROR && tries-- > 0)
			result = facy.send_message(data->msgid, data->hContact, data->msg, &error_text);

		if (result == SEND_MESSAGE_OK) {
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)data->msgid);

			// Remove from "readers" list and clear statusbar
			facy.erase_reader(data->hContact);
		}
		else ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, (LPARAM)_A2T(error_text.c_str()));
	}

	delete data;
}

void FacebookProto::SendChatMsgWorker(void *p)
{
	if (p == nullptr)
		return;

	send_chat *data = static_cast<send_chat*>(p);
	std::string err_message;

	// replace %% back to %, because chat automatically does this to sent messages
	utils::text::replace_all(&data->msg, "%%", "%");

	MCONTACT hContact = ChatIDToHContact(data->chat_id);
	if (hContact) {
		ptrA tid_(getStringA(hContact, FACEBOOK_KEY_TID));
		std::string tid;
		if (tid_ != nullptr && mir_strcmp(tid_, "null"))
			tid = tid_;
		else {
			// request info about chat thread
			http::response resp = facy.sendRequest(facy.threadInfoRequest(true, data->chat_id.c_str()));

			tid = utils::text::source_get_value(&resp.data, 2, "\"thread_id\":\"", "\"");
			if (!tid.empty() && tid.compare("null"))
				setString(hContact, FACEBOOK_KEY_TID, tid.c_str());
			debugLogA("    Got thread info: %s = %s", data->chat_id.c_str(), tid.c_str());
		}

		if (!tid.empty()) {
			if (facy.send_message(0, hContact, data->msg, &err_message) == SEND_MESSAGE_OK)
				UpdateChat(data->chat_id.c_str(), facy.self_.user_id.c_str(), facy.self_.real_name.c_str(), data->msg.c_str());
			else {
				ptrA text(mir_utf8encode(err_message.c_str()));
				UpdateChat(data->chat_id.c_str(), nullptr, nullptr, text);
			}
		}
	}

	delete data;
}

int FacebookProto::SendMsg(MCONTACT hContact, int, const char *msg)
{
	std::string message = msg;
	unsigned int msgId = InterlockedIncrement(&facy.msgid_);

	ForkThread(&FacebookProto::SendMsgWorker, new send_direct(hContact, message, msgId));
	return msgId;
}

int FacebookProto::UserIsTyping(MCONTACT hContact, int type)
{
	if (hContact && isOnline())
		ForkThread(&FacebookProto::SendTypingWorker, new send_typing(hContact, type));

	return 0;
}

void FacebookProto::SendTypingWorker(void *p)
{
	if (p == nullptr)
		return;

	send_typing *typing = static_cast<send_typing*>(p);

	// Don't send typing notifications when we are invisible and user don't want that
	bool typingWhenInvisible = getBool(FACEBOOK_KEY_TYPING_WHEN_INVISIBLE, DEFAULT_TYPING_WHEN_INVISIBLE);
	if (isInvisible() && !typingWhenInvisible) {
		delete typing;
		return;
	}

	// Dont send typing notifications to not friends - Facebook won't give them that info anyway
	if (!isChatRoom(typing->hContact) && getWord(typing->hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND) {
		delete typing;
		return;
	}

	const char *value = (isChatRoom(typing->hContact) ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID);
	ptrA id(getStringA(typing->hContact, value));
	if (id != nullptr)
		http::response resp = facy.sendRequest(facy.sendTypingRequest(id, isChatRoom(typing->hContact), typing->status == PROTOTYPE_SELFTYPING_ON));

	delete typing;
}

void FacebookProto::ReadMessageWorker(void *p)
{
	if (p == nullptr)
		return;

	if (getBool(FACEBOOK_KEY_KEEP_UNREAD, 0))
		return;

	std::set<MCONTACT> *hContacts = (std::set<MCONTACT>*)p;

	if (hContacts->empty()) {
		delete hContacts;
		return;
	}

	LIST<char> ids(1);
	for (auto &hContact : *hContacts) {
		if (getBool(hContact, FACEBOOK_KEY_KEEP_UNREAD, 0))
			continue;

		// mark message read (also send seen info)
		const char *value = (isChatRoom(hContact) ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID);
		ptrA id(getStringA(hContact, value));
		if (id == nullptr)
			continue;

		ids.insert(mir_strdup(id));
	}

	hContacts->clear();
	delete hContacts;

	facy.sendRequest(facy.markMessageReadRequest(ids));

	FreeList(ids);
	ids.destroy();
}

void FacebookProto::StickerAsSmiley(std::string sticker, const std::string &url, MCONTACT hContact)
{
	// Don't load stickers as smileys when we're loading history
	if (facy.loading_history)
		return;

	std::string b64 = ptrA(mir_base64_encode(sticker.c_str(), sticker.length()));
	b64 = utils::url::encode(b64);

	std::wstring filename = GetAvatarFolder() + L"\\stickers\\";
	ptrW dir(mir_wstrdup(filename.c_str()));

	filename += (wchar_t*)_A2T(b64.c_str());
	filename += L".png";

	// Check if we have this sticker already and download it if not
	if (GetFileAttributes(filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
		HNETLIBCONN nlc = nullptr;
		facy.save_url(url, filename, nlc);
		Netlib_CloseHandle(nlc);
	}

	SMADD_CONT cont;
	cont.cbSize = sizeof(SMADD_CONT);
	cont.hContact = hContact;
	cont.type = 0;
	cont.path = dir;
	CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, (LPARAM)&cont);
}

//////////////////////////////////////////////////////////////////////////////////////////

HttpRequest* facebook_client::sendMessageRequest(
	const char *userId,
	const char *threadId,
	const char *messageId,
	const char *messageText,
	bool isChat,
	const char *captcha,
	const char *captchaPersistData)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/messaging/send/");

	// Don't notify errors for this request, because we're getting them inline in messaging window
	p->NotifyErrors = false;

	// Use own persistent connection for sending messages
	p->Persistent = p->MESSAGES;

	p << INT_PARAM("dpr", 1);

	if (mir_strlen(captcha) > 0)
		p->Body << CHAR_PARAM("captcha_persist_data", captchaPersistData) << CHAR_PARAM("captcha_response", captcha);

	p->Body << CHAR_PARAM("client", "mercury") << CHAR_PARAM("action_type", "ma-type:user-generated-message");

	// Experimental sticker sending support
	std::string message_text = messageText; // FIXME: Rewrite this without std::string...
	if (message_text.substr(0, 10) == "[[sticker:" && message_text.substr(message_text.length() - 2) == "]]")
		// TODO: For sending GIF images instead of "sticker_id=" there is "image_ids[0]=", otherwise it's same
		p->Body
			<< CHAR_PARAM("body", "")
			<< CHAR_PARAM("sticker_id", message_text.substr(10, message_text.length() - 10 - 2).c_str())
			<< BOOL_PARAM("has_attachment", true);
	else 
		p->Body << CHAR_PARAM("body", messageText) << BOOL_PARAM("has_attachment", false);

	p->Body
		<< INT_PARAM("ephemeral_ttl_mode", 0)
		<< CHAR_PARAM("message_id", messageId)
		<< CHAR_PARAM("offline_threading_id", messageId); // Same as message ID

	if (isChat) {
		// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
		std::string threadFbid = threadId; // FIXME: Rewrite this without std::string...
		if (threadFbid.substr(0, 3) == "id.")
			threadFbid = threadFbid.substr(3);

		p->Body << CHAR_PARAM("thread_fbid", threadFbid.c_str());
	}
	else 
		p->Body
			<< CHAR_PARAM("other_user_fbid", userId)
			<< CHAR_PARAM("specific_to_list[0]", CMStringA(::FORMAT, "fbid:%s", userId))
			<< CHAR_PARAM("specific_to_list[1]", CMStringA(::FORMAT, "fbid:%s", self_.user_id.c_str()));

	p->Body
		// << "signature_id=" // TODO: How to generate signature ID? It is present only when sending via "mercury"
		<< CHAR_PARAM("source", "source:chat:web") // or "source:titan:web" for web_messenger
		<< CHAR_PARAM("timestamp", utils::time::mili_timestamp().c_str())
		<< CHAR_PARAM("ui_push_phase", "V3")
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< INT_PARAM("__a", 1)
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__be", -1);
	
	return p;
}

HttpRequest* facebook_client::sendTypingRequest(const char *userId, bool isChat, bool isTyping)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/messaging/typ.php");

	p << INT_PARAM("dpr", 1);

	p->Body
		<< INT_PARAM("typ", isTyping ? 1 : 0)
		<< CHAR_PARAM("to", isChat ? "" : userId)
		<< CHAR_PARAM("thread", userId)
		<< CHAR_PARAM("source", "mercury-chat")
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", -1);

	return p;
}

HttpRequest* facebook_client::markMessageReadRequest(const LIST<char> &ids)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/change_read_status.php");

	p << INT_PARAM("__a", 1);

	for (auto &it : ids) {
		std::string id_ = it;
		// NOTE: Remove "id." prefix as here we need to give threadFbId and not threadId
		if (id_.substr(0, 3) == "id.")
			id_ = id_.substr(3);

		CMStringA id(::FORMAT, "ids[%s]", mir_urlEncode(id_.c_str()).c_str());
		p->Body << BOOL_PARAM(id, true);
	}

	p->Body
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< INT_PARAM("__a", 1);

	return p;
}

/////////////////////////////////////////////////////////////////////////////////////////

HttpRequest* facebook_client::destroyThreadRequest(facebook_chatroom *fbc)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/mercury/delete_thread.php");

	p << INT_PARAM("dpr", 1);

	p->Body
		<< CHAR_PARAM("ids[0]", fbc->thread_id.substr(3).c_str())
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", 1);

	return p;
}

HttpRequest* facebook_client::exitThreadRequest(facebook_chatroom *fbc)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/messaging/send/");

	p << INT_PARAM("dpr", 1);

	std::string msgid = utils::text::rand_string(15);

	p->Body
		<< CHAR_PARAM("client", "mercury")
		<< CHAR_PARAM("action_type", "ma-type:log-message")
		<< CHAR_PARAM("log_message_data[removed_participants][0]", ("fbid:" + self_.user_id).c_str())
		<< CHAR_PARAM("log_message_type", "log:unsubscribe")
		<<	CHAR_PARAM("message_id", msgid.c_str())
		<< CHAR_PARAM("offline_threading_id", msgid.c_str())
		<< CHAR_PARAM("source", "source:chat:web")
		<<	CHAR_PARAM("thread_fbid", fbc->thread_id.substr(3).c_str())
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< INT64_PARAM("timestamp", ::time(0) * 1000)
		<< CHAR_PARAM("__user", self_.user_id.c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("__rev", __rev())
		<< CHAR_PARAM("__pc", "PHASED:DEFAULT")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__be", 1);

	return p;
}
