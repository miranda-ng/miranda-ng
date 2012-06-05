/*
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation,  either version 2 of the License,  or
(at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not,  see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "proto.h"

void CALLBACK TwitterProto::APC_callback(ULONG_PTR p)
{
	reinterpret_cast<TwitterProto*>(p)->LOG("***** Executing APC");
}

template<typename T>
inline static T db_pod_get(HANDLE hContact, const char *module, const char *setting, 
	T errorValue)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING cgs;

	cgs.szModule  = module;
	cgs.szSetting = setting;
	cgs.pValue    = &dbv;
	if (CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&cgs))
		return errorValue;

	// TODO: remove this,  it's just a temporary workaround
	if (dbv.type == DBVT_DWORD)
		return dbv.dVal;

	if (dbv.cpbVal != sizeof(T))
		return errorValue;
	return *reinterpret_cast<T*>(dbv.pbVal);
}

template<typename T>
inline static INT_PTR db_pod_set(HANDLE hContact, const char *module, const char *setting, 
	T val)
{
	DBCONTACTWRITESETTING cws;

	cws.szModule     = module;
	cws.szSetting    = setting;
	cws.value.type   = DBVT_BLOB;
	cws.value.cpbVal = sizeof(T);
	cws.value.pbVal  = reinterpret_cast<BYTE*>(&val);
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
}

void TwitterProto::SignOn(void*)
{
	LOG("***** Beginning SignOn process");
	WaitForSingleObject(&signon_lock_, INFINITE);

	// Kill the old thread if it's still around
	if (hMsgLoop_)
	{
		LOG("***** Requesting MessageLoop to exit");
		QueueUserAPC(APC_callback, hMsgLoop_, (ULONG_PTR)this);
		LOG("***** Waiting for old MessageLoop to exit");
		WaitForSingleObject(hMsgLoop_, INFINITE);
		CloseHandle(hMsgLoop_);
	}
	if (NegotiateConnection()) // Could this be? The legendary Go Time??
	{
		if (!in_chat_ && db_byte_get(0, m_szModuleName, TWITTER_KEY_CHATFEED, 0))
			OnJoinChat(0, true);
		
		SetAllContactStatuses(ID_STATUS_ONLINE);
		hMsgLoop_ = ForkThreadEx(&TwitterProto::MessageLoop, this);
	}

	ReleaseMutex(signon_lock_);
	LOG("***** SignOn complete");
}

bool TwitterProto::NegotiateConnection()
{
	LOG("***** Negotiating connection with Twitter");

	int old_status = m_iStatus;
	std::string user, pass;
	DBVARIANT dbv;

	if ( !DBGetContactSettingString(0, m_szModuleName, TWITTER_KEY_UN, &dbv)) {
		user = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else {
		ShowPopup(TranslateT("Please enter a username."));
		return false;
	}

	if ( !DBGetContactSettingString(0, m_szModuleName, TWITTER_KEY_PASS, &dbv)) {
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal)+1, 
			reinterpret_cast<LPARAM>(dbv.pszVal));
		pass = dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	else {
		ShowPopup(TranslateT("Please enter a password."));
		return false;
	}

	if ( !DBGetContactSettingString(0, m_szModuleName, TWITTER_KEY_BASEURL, &dbv)) {
		ScopedLock s(twitter_lock_);
		twit_.set_base_url(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	bool success;
	{
		ScopedLock s(twitter_lock_);
		success = twit_.set_credentials(user, pass);
	}

	if (!success) {
		ShowPopup(TranslateT("Your username/password combination was incorrect."));
		ProtoBroadcastAck(m_szModuleName, 0, ACKTYPE_STATUS, ACKRESULT_FAILED, 
			(HANDLE)old_status, m_iStatus);

		// Set to offline
		old_status = m_iStatus;
		m_iDesiredStatus = m_iStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(m_szModuleName, 0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, 
			(HANDLE)old_status, m_iStatus);

		return false;
	}
	
	m_iStatus = m_iDesiredStatus;
	ProtoBroadcastAck(m_szModuleName, 0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);
	return true;
}


void TwitterProto::MessageLoop(void*)
{
	LOG("***** Entering Twitter::MessageLoop");

	since_id_    = db_pod_get<twitter_id>(0, m_szModuleName, TWITTER_KEY_SINCEID, 0);
	dm_since_id_ = db_pod_get<twitter_id>(0, m_szModuleName, TWITTER_KEY_DMSINCEID, 0);

	bool new_account = db_byte_get(0, m_szModuleName, TWITTER_KEY_NEW, 1) != 0;
	bool popups      = db_byte_get(0, m_szModuleName, TWITTER_KEY_POPUP_SIGNON, 1) != 0;

	int poll_rate = db_dword_get(0, m_szModuleName, TWITTER_KEY_POLLRATE, 80);

	for(unsigned int i=0;;i++)
	{
		if (m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		if (i%4 == 0)
			UpdateFriends();

		if (m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		UpdateStatuses(new_account, popups);

		if (m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		UpdateMessages(new_account);

		if (new_account) // Not anymore!
		{
			new_account = false;
			DBWriteContactSettingByte(0, m_szModuleName, TWITTER_KEY_NEW, 0);
		}

		if (m_iStatus != ID_STATUS_ONLINE)
			goto exit;
		LOG("***** TwitterProto::MessageLoop going to sleep...");
		if (SleepEx(poll_rate*1000, true) == WAIT_IO_COMPLETION)
			goto exit;
		LOG("***** TwitterProto::MessageLoop waking up...");

		popups = true;
	}

exit:
	{
		ScopedLock s(twitter_lock_);
		twit_.set_credentials("", "", false);
	}
	LOG("***** Exiting TwitterProto::MessageLoop");
}

struct update_avatar
{
	update_avatar(HANDLE hContact, const std::string &url) : hContact(hContact), url(url) {}
	HANDLE hContact;
	std::string url;
};

void TwitterProto::UpdateAvatarWorker(void *p)
{
	if (p == 0)
		return;
	std::auto_ptr<update_avatar> data( static_cast<update_avatar*>(p));
	DBVARIANT dbv;

	if (DBGetContactSettingTString(data->hContact, m_szModuleName, TWITTER_KEY_UN, &dbv))
		return;

	std::string ext = data->url.substr(data->url.rfind('.'));
	std::string filename = GetAvatarFolder() + '\\' + dbv.pszVal + ext;
	DBFreeVariant(&dbv);

	PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
	ai.hContact = data->hContact;
	ai.format = ext_to_format(ext);
	strncpy(ai.filename, filename.c_str(), MAX_PATH);

	LOG("***** Updating avatar: %s", data->url.c_str());
	WaitForSingleObjectEx(avatar_lock_, INFINITE, true);
	if (CallService(MS_SYSTEM_TERMINATED, 0, 0))
	{
		LOG("***** Terminating avatar update early: %s", data->url.c_str());
		return;
	}

	if (save_url(hAvatarNetlib_, data->url, filename))
	{
		DBWriteContactSettingString(data->hContact, m_szModuleName, TWITTER_KEY_AV_URL, 
			data->url.c_str());
		ProtoBroadcastAck(m_szModuleName, data->hContact, ACKTYPE_AVATAR, 
			ACKRESULT_SUCCESS, &ai, 0);
	}
	else
		ProtoBroadcastAck(m_szModuleName, data->hContact, ACKTYPE_AVATAR, 
			ACKRESULT_FAILED,  &ai, 0);
	ReleaseMutex(avatar_lock_);
	LOG("***** Done avatar: %s", data->url.c_str());
}

void TwitterProto::UpdateAvatar(HANDLE hContact, const std::string &url, bool force)
{
	DBVARIANT dbv;

	if ( !force &&
	  ( !DBGetContactSettingString(hContact, m_szModuleName, TWITTER_KEY_AV_URL, &dbv) &&
	    url == dbv.pszVal))
	{
		LOG("***** Avatar already up-to-date: %s", url.c_str());
	}
	else
	{
		// TODO: more defaults (configurable?)
		if (url == "http://static.twitter.com/images/default_profile_normal.png")
		{
			PROTO_AVATAR_INFORMATION ai = {sizeof(ai), hContact};
			
			db_string_set(hContact, m_szModuleName, TWITTER_KEY_AV_URL, url.c_str());
			ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AVATAR, 
				ACKRESULT_SUCCESS, &ai, 0);
		}
		else
		{
			ForkThread(&TwitterProto::UpdateAvatarWorker,  this, 
				new update_avatar(hContact, url));
		}
	}

	DBFreeVariant(&dbv);
}

void TwitterProto::UpdateFriends()
{
	try
	{
		ScopedLock s(twitter_lock_);
		std::vector<twitter_user> friends = twit_.get_friends();
		s.Unlock();

		for(std::vector<twitter_user>::iterator i=friends.begin(); i!=friends.end(); ++i) {
			if (i->username == twit_.get_username())
				continue;

			HANDLE hContact = AddToClientList( _A2T(i->username.c_str()), i->status.text.c_str());
			UpdateAvatar(hContact, i->profile_image_url);
		}
		LOG("***** Friends list updated");
	}
	catch(const bad_response &)
	{
		LOG("***** Bad response from server,  signing off");
		SetStatus(ID_STATUS_OFFLINE);
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating friends list,  an error occurred: ")
			+e.what()).c_str());
		LOG("***** Error updating friends list: %s", e.what());
	}

}

void TwitterProto::ShowContactPopup(HANDLE hContact, const std::wstring &text)
{
	if (!ServiceExists(MS_POPUP_ADDPOPUPT) || DBGetContactSettingByte(0, 
		m_szModuleName, TWITTER_KEY_POPUP_SHOW, 0) == 0)
	{
		return;
	}

	POPUPDATAT popup = {};
	popup.lchContact = hContact;
	popup.iSeconds = db_dword_get(0, m_szModuleName, TWITTER_KEY_POPUP_TIMEOUT, 0);
	
	popup.colorBack = db_dword_get(0, m_szModuleName, TWITTER_KEY_POPUP_COLBACK, 0);
	if (popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOW);
	popup.colorText = db_dword_get(0, m_szModuleName, TWITTER_KEY_POPUP_COLTEXT, 0);
	if (popup.colorBack == 0xFFFFFFFF)
		popup.colorBack = GetSysColor(COLOR_WINDOWTEXT);

	DBVARIANT dbv;
	if ( !DBGetContactSettingString(hContact, "CList", "MyHandle", &dbv) ||
		!DBGetContactSettingString(hContact, m_szModuleName, TWITTER_KEY_UN, &dbv))
	{
		mbcs_to_tcs(CP_UTF8, dbv.pszVal, popup.lptzContactName, MAX_CONTACTNAME);
		DBFreeVariant(&dbv);
	}

	CallService(MS_POPUP_ADDPOPUPT, reinterpret_cast<WPARAM>(text.c_str()), 0);
}

void TwitterProto::UpdateStatuses(bool pre_read, bool popups)
{
	try
	{
		ScopedLock s(twitter_lock_);
		twitter::status_list updates = twit_.get_statuses(200, since_id_);
		s.Unlock();

		if (!updates.empty())
			since_id_ = std::max(since_id_,  updates[0].status.id);

		for(twitter::status_list::reverse_iterator i=updates.rbegin(); i!=updates.rend(); ++i)
		{
			if (!pre_read && in_chat_)
				UpdateChat(*i);

			if (i->username == twit_.get_username())
				continue;

			HANDLE hContact = AddToClientList( _A2T(i->username.c_str()), _T(""));

			DBEVENTINFO dbei = {sizeof(dbei)};
			
			dbei.pBlob = (BYTE*)(i->status.text.c_str());
			dbei.cbBlob = i->status.text.size()+1;
			dbei.eventType = TWITTER_DB_EVENT_TYPE_TWEET;
			dbei.flags = DBEF_UTF;
			//dbei.flags = DBEF_READ;
			dbei.timestamp = static_cast<DWORD>(i->status.time);
			dbei.szModule = m_szModuleName;
			CallService(MS_DB_EVENT_ADD,  (WPARAM)hContact,  (LPARAM)&dbei);

			DBWriteContactSettingTString(hContact, "CList", "StatusMsg", i->status.text.c_str());

			if ( !pre_read && popups )
				ShowContactPopup(hContact, i->status.text);
		}

		db_pod_set(0, m_szModuleName, TWITTER_KEY_SINCEID, since_id_);
		LOG("***** Status messages updated");
	}
	catch(const bad_response &)
	{
		LOG("***** Bad response from server,  signing off");
		SetStatus(ID_STATUS_OFFLINE);
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating status messages,  an error occurred: ")
			+e.what()).c_str());
		LOG("***** Error updating status messages: %s", e.what());
	}
}

void TwitterProto::UpdateMessages(bool pre_read)
{
	try
	{
		ScopedLock s(twitter_lock_);
		twitter::status_list messages = twit_.get_direct(dm_since_id_);
		s.Unlock();

		if (messages.size())
			dm_since_id_ = std::max(dm_since_id_,  messages[0].status.id);

		for(twitter::status_list::reverse_iterator i=messages.rbegin(); i!=messages.rend(); ++i) {
			HANDLE hContact = AddToClientList( _A2T(i->username.c_str()), _T(""));

			PROTORECVEVENT recv = {};
			CCSDATA ccs = {};

			recv.flags = PREF_TCHAR;
			if (pre_read)
				recv.flags |= PREF_CREATEREAD;
			recv.szMessage = ( char* )i->status.text.c_str();
			recv.timestamp = static_cast<DWORD>(i->status.time);

			ccs.hContact = hContact;
			ccs.szProtoService = PSR_MESSAGE;
			ccs.wParam = ID_STATUS_ONLINE;
			ccs.lParam = reinterpret_cast<LPARAM>(&recv);
			CallService(MS_PROTO_CHAINRECV, 0, reinterpret_cast<LPARAM>(&ccs));
		}

		db_pod_set(0, m_szModuleName, TWITTER_KEY_DMSINCEID, dm_since_id_);
		LOG("***** Direct messages updated");
	}
	catch(const bad_response &)
	{
		LOG("***** Bad response from server,  signing off");
		SetStatus(ID_STATUS_OFFLINE);
	}
	catch(const std::exception &e)
	{
		ShowPopup( (std::string("While updating direct messages,  an error occurred: ")
			+e.what()).c_str());
		LOG("***** Error updating direct messages: %s", e.what());
	}
}