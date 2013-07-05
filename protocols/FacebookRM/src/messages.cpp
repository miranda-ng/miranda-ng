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

int FacebookProto::RecvMsg(HANDLE hContact, PROTORECVEVENT *pre)
{
	ForkThread(&FacebookProto::ReadMessageWorker, this, hContact);
	CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact, (LPARAM)PROTOTYPE_CONTACTTYPING_OFF);

	return Proto_RecvMessage(hContact, pre);
}

void FacebookProto::SendMsgWorker(void *p)
{
	if(p == NULL)
		return;

//	ScopedLock s(facy.send_message_lock_, 500);

	send_direct *data = static_cast<send_direct*>(p);

	DBVARIANT dbv;

	if (!isOnline())
	{
		ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, data->msgid, (LPARAM)Translate("You cannot send messages when you are offline."));
	}
	else if (!db_get_s(data->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv))
	{
		//ParseSmileys(data->msg, data->hContact);

		int retries = 5;
		std::string error_text = "";
		bool result = false;
		while (!result && retries > 0) {
			result = facy.send_message(dbv.pszVal, data->msg, &error_text, retries % 2 == 0 ? MESSAGE_INBOX : MESSAGE_MERCURY);
			retries--;
		}
		if (result) {
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, data->msgid, 0);
			CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)data->hContact, NULL);
		} else {
			char *err = mir_utf8decodeA(error_text.c_str());
			ProtoBroadcastAck(data->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, data->msgid, (LPARAM)err);
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

	HANDLE hContact = ChatIDToHContact(data->chat_id);
	if (hContact) {		
		std::string tid;
		DBVARIANT dbv;
		if (!db_get_s(hContact, m_szModuleName, FACEBOOK_KEY_TID, &dbv)) {
			tid = dbv.pszVal;
			db_free(&dbv);
		} else {
			std::string post_data = "threads[group_ids][0]=" + data->chat_id;
			post_data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
			post_data += "&__user=" + facy.self_.user_id;
			post_data += "&phstamp=0";

			http::response resp = facy.flap(REQUEST_THREAD_INFO, &post_data);
			facy.validate_response(&resp);

			tid = utils::text::source_get_value(&resp.data, 2, "\"thread_id\":\"", "\"");
			db_set_s(hContact, m_szModuleName, FACEBOOK_KEY_TID, tid.c_str());
			Log("      Got thread info: %s = %s", data->chat_id.c_str(), tid.c_str());
		}		
		
		if (!tid.empty())
			facy.send_message(tid, data->msg, &err_message, MESSAGE_TID);
	}

	delete data;
}

int FacebookProto::SendMsg(HANDLE hContact, int flags, const char *msg)
{
	// TODO: msg comes as Unicode (retyped wchar_t*), why should we convert it as ANSI to UTF-8? o_O
	if (flags & PREF_UNICODE)
		msg = mir_utf8encode(msg);
  
	facy.msgid_ = (facy.msgid_ % 1024)+1;
	ForkThread(&FacebookProto::SendMsgWorker, this, new send_direct(hContact, msg, (HANDLE)facy.msgid_));
	return facy.msgid_;
}

int FacebookProto::UserIsTyping(HANDLE hContact,int type)
{ 
	if (hContact && isOnline())
		ForkThread(&FacebookProto::SendTypingWorker, this, new send_typing(hContact, type));

	return 0;
}

void FacebookProto::SendTypingWorker(void *p)
{
	if(p == NULL)
		return;

	send_typing *typing = static_cast<send_typing*>(p);

	// Dont send typing notifications to contacts, that are offline or not friends
	if (db_get_w(typing->hContact,m_szModuleName,"Status", 0) == ID_STATUS_OFFLINE
		|| db_get_b(typing->hContact, m_szModuleName, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND)
		return;

	// TODO RM: maybe better send typing optimalization
	facy.is_typing_ = (typing->status == PROTOTYPE_SELFTYPING_ON);
	SleepEx(2000, true);

	if (!facy.is_typing_ == (typing->status == PROTOTYPE_SELFTYPING_ON))
	{
		delete typing;
		return;
	}
		
	DBVARIANT dbv;
	if (!db_get_s(typing->hContact,m_szModuleName,FACEBOOK_KEY_ID,&dbv))
	{
		std::string data = "&source=mercury-chat";
		data += (typing->status == PROTOTYPE_SELFTYPING_ON ? "&typ=1" : "&typ=0"); // PROTOTYPE_SELFTYPING_OFF
		data += "&to=" + std::string(dbv.pszVal);
		data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
		data += "&lsd=&phstamp=0&__user=" + facy.self_.user_id;
		
		http::response resp = facy.flap(REQUEST_TYPING_SEND, &data);

		db_free(&dbv);
	}		

	delete typing;
}

void FacebookProto::ReadMessageWorker(void *p)
{
	if (p == NULL)
		return;

	HANDLE hContact = static_cast<HANDLE>(p);

	if (!db_get_b(NULL, m_szModuleName, FACEBOOK_KEY_MARK_READ, 1)) {
		// old variant - no seen info updated
		ptrA id( db_get_sa(hContact, m_szModuleName, FACEBOOK_KEY_ID));
		if (id == NULL) return;

		std::string data = "action=chatMarkRead";
		data += "&other_user=" + std::string(id);
		data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
		data += "&lsd=&__user=" + facy.self_.user_id;
	
		facy.flap(REQUEST_ASYNC, &data);
	} else {
		// new variant - with seen info
		ptrA mid( db_get_sa(hContact, m_szModuleName, FACEBOOK_KEY_MESSAGE_ID));
		if (mid == NULL) return;

		std::string data = "ids[" + std::string(mid) + "]=true";
		data += "&fb_dtsg=" + (facy.dtsg_.length() ? facy.dtsg_ : "0");
		data += "&__user=" + facy.self_.user_id;
		data += "&__a=1&__dyn=&__req=j&phstamp=0";

		facy.flap(REQUEST_MARK_READ, &data);
	}
}

void FacebookProto::ParseSmileys(std::string message, HANDLE hContact)
{
	if (!db_get_b(NULL,m_szModuleName,FACEBOOK_KEY_CUSTOM_SMILEYS, DEFAULT_CUSTOM_SMILEYS))
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
		
		std::string url = "http://graph.facebook.com/%s/picture";
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
		TCHAR *path = _tcsdup(filename.c_str());

		SMADD_CONT cont;
		cont.cbSize = sizeof(SMADD_CONT);
		cont.hContact = hContact;
		cont.type = 1;
		cont.path = path;

		CallService(MS_SMILEYADD_LOADCONTACTSMILEYS, 0, (LPARAM)&cont);
		mir_free(path);
	}
	Netlib_CloseHandle(nlc);
}
