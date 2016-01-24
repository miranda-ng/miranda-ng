/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-16 Robert P�sel

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

int FacebookProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	StopTyping(hContact);

	// Remove from "readers" list and clear statusbar
	facy.erase_reader(hContact);

	return Proto_RecvMessage(hContact, pre);
}

void FacebookProto::SendMsgWorker(void *p)
{
	if (p == NULL)
		return;

	send_direct *data = static_cast<send_direct*>(p);

	ptrA id(getStringA(data->hContact, FACEBOOK_KEY_ID));

	if (!isOnline()) {
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, (LPARAM)Translate("You cannot send messages when you are offline."));
	}
	else if (id == NULL) {
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, 0);
	}
	else {
		int tries = getByte(FACEBOOK_KEY_SEND_MESSAGE_TRIES, 1);
		tries = min(max(tries, 1), 5);

		std::string error_text;
		int result = SEND_MESSAGE_ERROR;
		while (result == SEND_MESSAGE_ERROR && tries-- > 0) {
			result = facy.send_message(data->msgid, data->hContact, data->msg, &error_text);
		}
		if (result == SEND_MESSAGE_OK) {
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)data->msgid, 0);

			// Remove from "readers" list and clear statusbar
			facy.erase_reader(data->hContact);
		}
		else {
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)data->msgid, (LPARAM)error_text.c_str());
		}
	}

	delete data;
}

void FacebookProto::SendChatMsgWorker(void *p)
{
	if (p == NULL)
		return;

	send_chat *data = static_cast<send_chat*>(p);
	std::string err_message;

	// replace %% back to %, because chat automatically does this to sent messages
	utils::text::replace_all(&data->msg, "%%", "%");

	MCONTACT hContact = ChatIDToHContact(data->chat_id);
	if (hContact) {
		ptrA tid_(getStringA(hContact, FACEBOOK_KEY_TID));
		std::string tid;
		if (tid_ != NULL && mir_strcmp(tid_, "null")) {
			tid = tid_;
		}
		else {
			std::string post_data = "client=mercury";
			post_data += "&__user=" + facy.self_.user_id;
			post_data += "&__dyn=" + facy.__dyn();
			post_data += "&__req=" + facy.__req();
			post_data += "&fb_dtsg=" + facy.dtsg_;
			post_data += "&ttstamp=" + facy.ttstamp_;
			post_data += "&__rev=" + facy.__rev();
			post_data += "&threads[thread_ids][0]=" + utils::url::encode(data->chat_id);

			http::response resp = facy.flap(REQUEST_THREAD_INFO, &post_data); // NOTE: Request revised 1.9.2015

			tid = utils::text::source_get_value(&resp.data, 2, "\"thread_id\":\"", "\"");
			if (!tid.empty() && tid.compare("null"))
				setString(hContact, FACEBOOK_KEY_TID, tid.c_str());
			debugLogA("    Got thread info: %s = %s", data->chat_id.c_str(), tid.c_str());
		}

		if (!tid.empty()) {
			if (facy.send_message(0, hContact, data->msg, &err_message) == SEND_MESSAGE_OK)
				UpdateChat(data->chat_id.c_str(), facy.self_.user_id.c_str(), facy.self_.real_name.c_str(), data->msg.c_str());
			else
				UpdateChat(data->chat_id.c_str(), NULL, NULL, err_message.c_str());
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
	if (p == NULL)
		return;

	send_typing *typing = static_cast<send_typing*>(p);

	// Dont send typing notifications to not friends - Facebook won't give them that info anyway
	if (!isChatRoom(typing->hContact) && getWord(typing->hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND) {
		delete typing;
		return;
	}

	const char *value = (isChatRoom(typing->hContact) ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID);
	ptrA id(getStringA(typing->hContact, value));
	if (id != NULL) {
		bool isChat = isChatRoom(typing->hContact);
		std::string idEncoded = utils::url::encode(std::string(id));

		std::string data = (typing->status == PROTOTYPE_SELFTYPING_ON ? "typ=1" : "typ=0");
		data += "&to=" + (isChat ? "" : idEncoded);
		data += "&source=mercury-chat";
		data += "&thread=" + idEncoded;
		data += "&__user=" + facy.self_.user_id;
		data += "&__dyn=" + facy.__dyn();
		data += "&__req=" + facy.__req();

		data += "&fb_dtsg=" + facy.dtsg_;
		data += "&ttsamp=" + facy.ttstamp_;
		data += "&__rev=" + facy.__rev();

		http::response resp = facy.flap(REQUEST_TYPING_SEND, &data); // NOTE: Request revised 1.9.2015
	}

	delete typing;
}

void FacebookProto::ReadMessageWorker(void *p)
{
	if (p == NULL)
		return;

	if (getBool(FACEBOOK_KEY_KEEP_UNREAD, 0))
		return;

	std::set<MCONTACT> *hContacts = (std::set<MCONTACT>*)p;

	if (hContacts->empty()) {
		delete hContacts;
		return;
	}

	std::string data = "fb_dtsg=" + facy.dtsg_;
	data += "&__user=" + facy.self_.user_id;
	data += "&__a=1&__dyn=&__req=&ttstamp=" + facy.ttstamp_;

	for (std::set<MCONTACT>::iterator it = hContacts->begin(); it != hContacts->end(); ++it) {
		MCONTACT hContact = *it;

		if (getBool(hContact, FACEBOOK_KEY_KEEP_UNREAD, 0))
			continue;

		// mark message read (also send seen info)
		const char *value = (isChatRoom(hContact) ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID);
		ptrA id(getStringA(hContact, value));
		if (id == NULL)
			continue;

		data += "&ids[" + utils::url::encode(std::string(id)) + "]=true";
	}
	hContacts->clear();
	delete hContacts;

	facy.flap(REQUEST_MARK_READ, &data);
}

void FacebookProto::StickerAsSmiley(std::string sticker, const std::string &url, MCONTACT hContact)
{
	std::string b64 = ptrA(mir_base64_encode((PBYTE)sticker.c_str(), (unsigned)sticker.length()));
	b64 = utils::url::encode(b64);

	std::tstring filename = GetAvatarFolder() + _T("\\stickers\\") + (TCHAR*)_A2T(b64.c_str()) + _T(".png");

	// Check if we have this sticker already and download it it not
	if (GetFileAttributes(filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
		HANDLE nlc = NULL;
		facy.save_url(url, filename, nlc);
		Netlib_CloseHandle(nlc);
	}

	SMADD_CONT cont;
	cont.cbSize = sizeof(SMADD_CONT);
	cont.hContact = hContact;
	cont.type = 1;
	cont.path = ptrT(mir_tstrdup(filename.c_str()));

	CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, (LPARAM)&cont);
}
