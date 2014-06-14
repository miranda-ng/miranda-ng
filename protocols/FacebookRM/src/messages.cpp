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

int FacebookProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT *pre)
{
	//CallService(MS_PROTO_CONTACTISTYPING, hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);
	CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)hContact, NULL);

	return Proto_RecvMessage(hContact, pre);
}

void FacebookProto::SendMsgWorker(void *p)
{
	if(p == NULL)
		return;

	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;

	if (!isOnline())
	{
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, data->msgid, (LPARAM)Translate("You cannot send messages when you are offline."));
	}
	else if (!getString(data->hContact, FACEBOOK_KEY_ID, &dbv))
	{
		//ParseSmileys(data->msg, data->hContact);

		int retries = 5;
		std::string error_text = "";
		bool result = false;
		while (!result && retries > 0) {
			result = facy.send_message(data->hContact, dbv.pszVal, data->msg, &error_text, retries % 2 == 0 ? MESSAGE_INBOX : MESSAGE_MERCURY);
			retries--;
		}
		if (result) {
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, data->msgid, 0);
			CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)data->hContact, NULL);
		} else {
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, data->msgid, (LPARAM)error_text.c_str());
		}
		db_free(&dbv);
	}

	delete data;
}

void FacebookProto::SendChatMsgWorker(void *p)
{
	if(p == NULL)
		return;

	send_chat *data = static_cast<send_chat*>(p);
	std::string err_message = "";

	MCONTACT hContact = ChatIDToHContact(std::tstring(_A2T(data->chat_id.c_str())));
	if (hContact) {		
		std::string tid;
		DBVARIANT dbv;
		if (!getString(hContact, FACEBOOK_KEY_TID, &dbv)) {
			tid = dbv.pszVal;
			db_free(&dbv);
		} else {
			std::string post_data = "threads[group_ids][0]=" + utils::url::encode(data->chat_id);
			post_data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
			post_data += "&__user=" + facy.self_.user_id;
			post_data += "&phstamp=0";

			http::response resp = facy.flap(REQUEST_THREAD_INFO, &post_data);

			tid = utils::text::source_get_value(&resp.data, 2, "\"thread_id\":\"", "\"");
			setString(hContact, FACEBOOK_KEY_TID, tid.c_str());
			debugLogA("      Got thread info: %s = %s", data->chat_id.c_str(), tid.c_str());
		}		
		
		if (!tid.empty()) {
			if (facy.send_message(hContact, tid, data->msg, &err_message, MESSAGE_TID))
				UpdateChat(_A2T(data->chat_id.c_str()), facy.self_.user_id.c_str(), facy.self_.real_name.c_str(), data->msg.c_str());
			else
				UpdateChat(_A2T(data->chat_id.c_str()), NULL, NULL, err_message.c_str());
		}
	}

	delete data;
}

int FacebookProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	// TODO: msg comes as Unicode (retyped wchar_t*), why should we convert it as ANSI to UTF-8? o_O
	if (flags & PREF_UNICODE)
		msg = mir_utf8encode(msg);
  
	facy.msgid_ = (facy.msgid_ % 1024) + 1;
	ForkThread(&FacebookProto::SendMsgWorker, new send_direct(hContact, msg, (HANDLE)facy.msgid_));
	return facy.msgid_;
}

int FacebookProto::UserIsTyping(MCONTACT hContact,int type)
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

	// TODO RM: maybe better send typing optimalization
	facy.is_typing_ = (typing->status == PROTOTYPE_SELFTYPING_ON);
	SleepEx(2000, true);

	if (!facy.is_typing_ == (typing->status == PROTOTYPE_SELFTYPING_ON)) {
		delete typing;
		return;
	}
	
	const char *value = (isChatRoom(typing->hContact) ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID);
	ptrA id( getStringA(typing->hContact, value));
	if (id != NULL) {
		std::string data = "&source=mercury-chat";
		data += (typing->status == PROTOTYPE_SELFTYPING_ON ? "&typ=1" : "&typ=0");
		
		data += "&to=";
		if (isChatRoom(typing->hContact))
			data += "&thread=";
		data += utils::url::encode(std::string(id));

		data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
		data += "&lsd=&phstamp=0&__user=" + facy.self_.user_id;
		
		http::response resp = facy.flap(REQUEST_TYPING_SEND, &data);
	}

	delete typing;
}

void FacebookProto::ReadMessageWorker(void *p)
{
	if (p == NULL)
		return;
	
	MCONTACT hContact = (MCONTACT)p;
	
	if (getBool(FACEBOOK_KEY_KEEP_UNREAD, 0) || getBool(hContact, FACEBOOK_KEY_KEEP_UNREAD, 0))
		return;

	// mark message read (also send seen info)
	ptrA tid( getStringA(hContact, FACEBOOK_KEY_TID));
	if (tid == NULL)
		return;

	std::string data = "ids[" + utils::url::encode(std::string(tid)) + "]=true";
	data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
	data += "&__user=" + facy.self_.user_id;
	data += "&__a=1&__dyn=&__req=&ttstamp=0";

	facy.flap(REQUEST_MARK_READ, &data);
}

void FacebookProto::ParseSmileys(std::string message, MCONTACT hContact)
{
	if (!getByte(FACEBOOK_KEY_CUSTOM_SMILEYS, DEFAULT_CUSTOM_SMILEYS))
		return;

	HANDLE nlc = NULL;
	std::string::size_type pos = 0;
	bool anything = false;
	while ((pos = message.find("[[", pos)) != std::string::npos) {
		std::string::size_type pos2 = message.find("]]", pos);
		if (pos2 == std::string::npos)
			break;

		std::string smiley = message.substr(pos, pos2+2-pos);
		pos = pos2;
		
		std::string url = FACEBOOK_URL_PICTURE;
		utils::text::replace_first(&url, "%s", smiley.substr(2, smiley.length()-4));

		std::string b64 = ptrA( mir_base64_encode((PBYTE)smiley.c_str(), (unsigned)smiley.length()));
		b64 = utils::url::encode(b64);

		std::tstring filename = GetAvatarFolder() + L"\\smileys\\" + (TCHAR*)_A2T(b64.c_str()) + _T(".jpg");
		FILE *f = _tfopen(filename.c_str(), _T("r"));
		if (f) {
			fclose(f);
		} else {
			facy.save_url(url, filename, nlc);
		}
		ptrT path( _tcsdup(filename.c_str()));

		SMADD_CONT cont;
		cont.cbSize = sizeof(SMADD_CONT);
		cont.hContact = hContact;
		cont.type = 1;
		cont.path = path;

		CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, (LPARAM)&cont);
	}
	Netlib_CloseHandle(nlc);
}
