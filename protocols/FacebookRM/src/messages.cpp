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

int FacebookProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	StopTyping(hContact);

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
			// request info about chat thread
			HttpRequest *request = new ThreadInfoRequest(&facy, true, data->chat_id.c_str());
			http::response resp = facy.sendRequest(request);

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
				UpdateChat(data->chat_id.c_str(), NULL, NULL, text);
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
	if (p == NULL)
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
	if (id != NULL) {
		bool isChat = isChatRoom(typing->hContact);
		HttpRequest *request = new SendTypingRequest(&facy, id, isChat, typing->status == PROTOTYPE_SELFTYPING_ON);
		http::response resp = facy.sendRequest(request);
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

	LIST<char> ids(1);
	for (std::set<MCONTACT>::iterator it = hContacts->begin(); it != hContacts->end(); ++it) {
		MCONTACT hContact = *it;

		if (getBool(hContact, FACEBOOK_KEY_KEEP_UNREAD, 0))
			continue;

		// mark message read (also send seen info)
		const char *value = (isChatRoom(hContact) ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID);
		ptrA id(getStringA(hContact, value));
		if (id == NULL)
			continue;

		ids.insert(mir_strdup(id));
	}
	
	hContacts->clear();
	delete hContacts;

	HttpRequest *request = new MarkMessageReadRequest(&facy, ids);
	facy.sendRequest(request);

	FreeList(ids);
	ids.destroy();
}

void FacebookProto::StickerAsSmiley(std::string sticker, const std::string &url, MCONTACT hContact)
{
	// Don't load stickers as smileys when we're loading history
	if (facy.loading_history)
		return;

	std::string b64 = ptrA(mir_base64_encode((PBYTE)sticker.c_str(), (unsigned)sticker.length()));
	b64 = utils::url::encode(b64);

	std::wstring filename = GetAvatarFolder() + L"\\stickers\\";
	ptrW dir(mir_wstrdup(filename.c_str()));

	filename += (wchar_t*)_A2T(b64.c_str());
	filename += L".png";

	// Check if we have this sticker already and download it if not
	if (GetFileAttributes(filename.c_str()) == INVALID_FILE_ATTRIBUTES) {
		HNETLIBCONN nlc = NULL;
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
