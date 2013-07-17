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

void FacebookProto::ProcessBuddyList(void* data)
{
	if (data == NULL)
		return;

	ScopedLock s(facy.buddies_lock_);

	std::string* resp = (std::string*)data;

	if (isOffline())
		goto exit;

	LOG("***** Starting processing buddy list");

	CODE_BLOCK_TRY

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_buddy_list(data, &facy.buddies);
	delete p;

	bool use_mobile_status = getByte(FACEBOOK_KEY_LOAD_MOBILE, DEFAULT_LOAD_MOBILE) != 0;

	for (List::Item< facebook_user >* i = facy.buddies.begin(); i != NULL;)
	{		
		facebook_user* fbu = i->data;
		bool on_mobile = false;

		char *status;
		switch (fbu->status_id) {
			case ID_STATUS_OFFLINE:
				status = "offline"; break;				
			case ID_STATUS_ONLINE:
				status = "online"; break;				
			case ID_STATUS_ONTHEPHONE:
				on_mobile = true;				
				status = "onthephone";

				if (!use_mobile_status)
					fbu->status_id = ID_STATUS_OFFLINE;
				break;
		}
		LOG("      Now %s: %s", status, i->data->real_name.c_str());

		if (!fbu->deleted)
		{
			HANDLE hContact = fbu->handle;
			if (!hContact)
				hContact = AddToContactList(fbu, CONTACT_FRIEND);
			
			DBVARIANT dbv;
			TCHAR* client = on_mobile ? _T(FACEBOOK_MOBILE) : _T(FACEBOOK_NAME);
			if (!getTString(hContact, "MirVer", &dbv)) {
				if (_tcscmp(dbv.ptszVal, client))
					setTString(hContact, "MirVer", client);
				db_free(&dbv);
			}
			else setTString(hContact, "MirVer", client);
		}

		if (fbu->status_id == ID_STATUS_OFFLINE || fbu->deleted)
		{
			if (fbu->handle)
				setWord(fbu->handle, "Status", ID_STATUS_OFFLINE);

			std::string to_delete(i->key);
			i = i->next;
			facy.buddies.erase(to_delete);
		} else {
			i = i->next;

			if (!fbu->handle) // just been added
				fbu->handle = AddToContactList(fbu, CONTACT_FRIEND);

			if (getWord(fbu->handle, "Status", 0) != fbu->status_id)
				setWord(fbu->handle, "Status", fbu->status_id);

			if (getDword(fbu->handle, "LastActiveTS", 0) != fbu->last_active) {
				if (fbu->last_active > 0)
					setDword(fbu->handle, "LastActiveTS", fbu->last_active);
				else
					delSetting(fbu->handle, "LastActiveTS");
			}

			if (getByte(fbu->handle, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND) {
				setByte(fbu->handle, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
				// TODO: remove that popup and use "Contact added you" event?
			}

			// Wasn't contact removed from "server-list" someday?
			if (getDword(fbu->handle, FACEBOOK_KEY_DELETED, 0)) {
				delSetting(fbu->handle, FACEBOOK_KEY_DELETED);

				std::string url = FACEBOOK_URL_PROFILE + fbu->user_id;					

				TCHAR* szTitle = mir_utf8decodeT(fbu->real_name.c_str());
				NotifyEvent(szTitle, TranslateT("Contact is back on server-list."), fbu->handle, FACEBOOK_EVENT_OTHER, &url);
				mir_free(szTitle);
			}

			// Check avatar change
			CheckAvatarChange(fbu->handle, fbu->image_url);
		}
	}

	LOG("***** Buddy list processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing buddy list: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::ProcessFriendList(void* data)
{
	if (data == NULL)
		return;

	std::string* resp = (std::string*)data;

	LOG("***** Starting processing friend list");

	CODE_BLOCK_TRY

	std::map<std::string, facebook_user*> friends;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_friends(data, &friends);
	delete p;


	// Check and update old contacts
	for (HANDLE hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (getByte(hContact, "ChatRoom", 0))
			continue;

		DBVARIANT dbv;
		facebook_user *fbu;
		if (!getString(hContact, FACEBOOK_KEY_ID, &dbv)) {
			std::string id = dbv.pszVal;
			db_free(&dbv);
			
			std::map< std::string, facebook_user* >::iterator iter;
			
			if ((iter = friends.find(id)) != friends.end()) {
				// Found contact, update it and remove from map
				fbu = iter->second;

				DBVARIANT dbv;
				bool update_required = true;

				// TODO RM: remove, because contacts cant change it, so its only for "first run"
					// - but what with contacts, that was added after logon?
				// Update gender
				if (getByte(hContact, "Gender", 0) != fbu->gender)
					setByte(hContact, "Gender", fbu->gender);

				// Update real name
				if (!db_get_utf(hContact, m_szModuleName, FACEBOOK_KEY_NAME, &dbv))
				{
					update_required = strcmp(dbv.pszVal, fbu->real_name.c_str()) != 0;
					db_free(&dbv);
				}
				if (update_required)
				{
					db_set_utf(hContact, m_szModuleName, FACEBOOK_KEY_NAME, fbu->real_name.c_str());
					db_set_utf(hContact, m_szModuleName, FACEBOOK_KEY_NICK, fbu->real_name.c_str());
				}

				if (getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) != CONTACT_FRIEND) {
					setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_FRIEND);
					// TODO: remove that popup and use "Contact added you" event?
				}

				// Wasn't contact removed from "server-list" someday?
				if (getDword(hContact, FACEBOOK_KEY_DELETED, 0)) {
					delSetting(hContact, FACEBOOK_KEY_DELETED);

					std::string url = FACEBOOK_URL_PROFILE + fbu->user_id;					

					TCHAR* szTitle = mir_utf8decodeT(fbu->real_name.c_str());
					NotifyEvent(szTitle, TranslateT("Contact is back on server-list."), hContact, FACEBOOK_EVENT_OTHER, &url);
					mir_free(szTitle);
				}

				// Check avatar change
				CheckAvatarChange(hContact, fbu->image_url);
			
				delete fbu;
				friends.erase(iter);
			} else {
				// Contact was removed from "server-list", notify it

				// Wasnt we already been notified about this contact? And was this real friend?
				if (!getDword(hContact, FACEBOOK_KEY_DELETED, 0) 
					&& getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0) == CONTACT_FRIEND)
				{

					setDword(hContact, FACEBOOK_KEY_DELETED, ::time(NULL));
					setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);

					std::string contactname = id;
					if (!db_get_utf(hContact, m_szModuleName, FACEBOOK_KEY_NAME, &dbv)) {
						contactname = dbv.pszVal;
						db_free(&dbv);
					}

					std::string url = FACEBOOK_URL_PROFILE + id;

					TCHAR* szTitle = mir_utf8decodeT(contactname.c_str());
					NotifyEvent(szTitle, TranslateT("Contact is no longer on server-list."), hContact, FACEBOOK_EVENT_OTHER, &url);
					mir_free(szTitle);
				}
			}
		}
	}

	// Check remain contacts in map and add it to contact list
	for (std::map< std::string, facebook_user* >::iterator iter = friends.begin(); iter != friends.end(); ++iter) {
		facebook_user *fbu = iter->second;
		
		HANDLE hContact = AddToContactList(fbu, CONTACT_FRIEND, true); // This contact is surely new ...are you sure?
	}

	LOG("***** Friend list processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing friend list: %s", e.what());

	CODE_BLOCK_END

	delete resp;
}

void FacebookProto::ProcessUnreadMessages(void*)
{
	facy.handle_entry("messages");

	int count = 0;
	std::string page;

	while (count < 30) // allow max 30 unread threads to be parsed
	{
		std::string get_data = "&page=" + page;

		http::response resp = facy.flap(REQUEST_UNREAD_THREADS, NULL, &get_data);

		// Process result data
		facy.validate_response(&resp);

		if (resp.code == HTTP_CODE_OK)
		{
			std::string items = utils::text::source_get_value(&resp.data, 2, "<div id=\"threadlist_rows", "</body>");

			std::string::size_type pos = 0;
			std::string::size_type pos2 = 0;

			while ((pos = items.find("id=\"threadlist_row_", pos)) != std::string::npos) {
				std::string item = items.substr(pos, items.find("</div>", pos) - pos);
				pos++; count++;

				std::string tid = utils::text::source_get_value2(&item, "?tid=", "&\"");
				if (tid.empty())
					continue;

				ForkThread(&FacebookProto::ProcessUnreadMessage, new std::string(tid));
			}

			page = utils::text::source_get_value(&items, 3, "id=\"see_older_threads\"", "page=", "&");
			if (page.empty())
				break; // No more results
		}
	}

}

void FacebookProto::ProcessUnreadMessage(void *tid_data)
{
	if (tid_data == NULL)
		return;

	// TODO: get them from /ajax/mercury/thread_info.php

	std::string get_data = "tid=" + *(std::string*)tid_data;
	delete (std::string*)tid_data;

	http::response resp = facy.flap(REQUEST_UNREAD_MESSAGES, NULL, &get_data);
	facy.validate_response(&resp);

	if (resp.code != HTTP_CODE_OK) {
		LOG(" !! !! Error when getting messages list");
		return;
	}

	if (resp.data.find("<option value=\"leave_conversation\">") != std::string::npos) {
		LOG(" !! !! This is multi user chat");
		return;
	}

	std::string messageslist = utils::text::source_get_value(&resp.data, 2, "id=\"messageGroup\">", "</form>");

	facebook_user fbu;
	HANDLE hContact = NULL;

	std::string::size_type pos = 0, pos2 = 0;
	while ((pos2 = messageslist.find("class=\"acw apl abt", pos2)) != std::string::npos) {
		pos2 += 19;
		std::string group = messageslist.substr(pos2, messageslist.find("class=\"actions ", pos2) - pos2);

		if (group.find("has_attachment&quot;:true") != std::string::npos) {
			// TODO: do something different for attachements? (inspiration in revision <5236)
		}

		while ((pos = group.find("<div data-store=", pos)) != std::string::npos) {
			std::string message = group.substr(pos, group.find("</div>", pos) - pos);
			pos++;

			std::string author = utils::text::source_get_value2(&message, "author&quot;:", ",}");

			// Ignore our messages
			if (author == facy.self_.user_id)
				continue;

			// Get/create contact
			if (hContact == NULL) {
				fbu.user_id = author;
				fbu.real_name = utils::text::slashu_to_utf8(utils::text::source_get_value(&group, 2, "name&quot;:&quot;", "&quot;"));
				hContact = AddToContactList(&fbu, CONTACT_NONE);
				// TODO: if contact is newly added, get his user info
				// TODO: maybe create new "receiveMsg" function and use it for offline and channel messages?
			}

			DWORD timestamp = utils::conversion::to_timestamp(utils::text::source_get_value2(&message, "timestamp&quot;:", ",}"));
			std::string text = utils::text::source_get_value(&message, 2, "<span>", "</span>");

			text = utils::text::trim(utils::text::special_expressions_decode(utils::text::remove_html(text)));
			// TODO: fix smileys in text
			/*
			<img src="https://fbstatic-a.akamaihd.net/rsrc.php/v2/yH/r/SOe5wIZyutW.png" width="16" height="16" class="img"/>

			"yH/r/viyyiQhRqLr.png" -> :-P
			"yo/r/X8YPpi6kcyo.png" -> :-)
			"yH/r/SOe5wIZyutW.png" -> :-D
			...
			*/

			if (text.empty() || hContact == NULL)
				continue;

			LOG("Got unread message: \"%s\"", text.c_str());

			ParseSmileys(text, hContact);

			PROTORECVEVENT recv = {0};
			recv.flags = PREF_UTF;
			recv.szMessage = const_cast<char*>(text.c_str());
			recv.timestamp = timestamp;
			ProtoChainRecvMsg(hContact, &recv);
		}
	}
}

void FacebookProto::ProcessMessages(void* data)
{
	if (data == NULL)
		return;

	std::string* resp = (std::string*)data;

	if (isOffline())
		goto exit;

	LOG("***** Starting processing messages");

	CODE_BLOCK_TRY

	std::vector< facebook_message* > messages;
	std::vector< facebook_notification* > notifications;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_messages(data, &messages, &notifications);
	delete p;

	bool local_timestamp = getBool(FACEBOOK_KEY_LOCAL_TIMESTAMP, 0);

	for(std::vector<facebook_message*>::size_type i=0; i<messages.size(); i++)
	{
		if (messages[i]->user_id != facy.self_.user_id)
		{
			LOG("      Got message: %s", messages[i]->message_text.c_str());
			facebook_user fbu;
			fbu.user_id = messages[i]->user_id;
			fbu.real_name = messages[i]->sender_name;

			HANDLE hContact = AddToContactList(&fbu, CONTACT_NONE);
			setString(hContact, FACEBOOK_KEY_MESSAGE_ID, messages[i]->message_id.c_str());

			// TODO: if contact is newly added, get his user info
			// TODO: maybe create new "receiveMsg" function and use it for offline and channel messages?

			ParseSmileys(messages[i]->message_text, hContact);

			PROTORECVEVENT recv = {0};
			recv.flags = PREF_UTF;
			recv.szMessage = const_cast<char*>(messages[i]->message_text.c_str());
			recv.timestamp = local_timestamp ? ::time(NULL) : messages[i]->time;
			ProtoChainRecvMsg(hContact, &recv);
		}
		delete messages[i];
	}
	messages.clear();

	for(std::vector<facebook_notification*>::size_type i=0; i<notifications.size(); i++)
	{
		LOG("      Got notification: %s", notifications[i]->text.c_str());
		TCHAR* szTitle = mir_utf8decodeT(this->m_szModuleName);
		TCHAR* szText = mir_utf8decodeT(notifications[i]->text.c_str());
		NotifyEvent(szTitle, szText, ContactIDToHContact(notifications[i]->user_id), FACEBOOK_EVENT_NOTIFICATION, &notifications[i]->link, &notifications[i]->id);
		mir_free(szTitle);
		mir_free(szText);

		delete notifications[i];
	}
	notifications.clear();

	LOG("***** Messages processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing messages: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::ProcessNotifications(void*)
{
	if (isOffline())
		return;

	if (!getByte(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
		return;

	facy.handle_entry("notifications");

	// Get notifications
	http::response resp = facy.flap(REQUEST_NOTIFICATIONS);

	// Process result data
	facy.validate_response(&resp);
  
	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("notifications");
		return;
	}


	// Process notifications
	LOG("***** Starting processing notifications");

	CODE_BLOCK_TRY

	std::vector< facebook_notification* > notifications;

	facebook_json_parser* p = new facebook_json_parser(this);
	p->parse_notifications(&(resp.data), &notifications);
	delete p;

	for(std::vector<facebook_notification*>::size_type i=0; i<notifications.size(); i++)
	{
		LOG("      Got notification: %s", notifications[i]->text.c_str());
		TCHAR* szTitle = mir_utf8decodeT(this->m_szModuleName);
		TCHAR* szText = mir_utf8decodeT(notifications[i]->text.c_str());
		NotifyEvent(szTitle, szText, ContactIDToHContact(notifications[i]->user_id), FACEBOOK_EVENT_NOTIFICATION, &notifications[i]->link, &notifications[i]->id);
		mir_free(szTitle);
		mir_free(szText);

		delete notifications[i];
	}
	notifications.clear();

	LOG("***** Notifications processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing notifications: %s", e.what());

	CODE_BLOCK_END
}

void FacebookProto::ProcessFriendRequests(void*)
{
	facy.handle_entry("friendRequests");

	// Get notifications
	http::response resp = facy.flap(REQUEST_LOAD_REQUESTS);

	// Process result data
	facy.validate_response(&resp);
  
	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("friendRequests");
		return;
	}
	
	// Parse it
	std::string reqs = utils::text::source_get_value(&resp.data, 2, "class=\"mRequestItem", "al aps\">");

	std::string::size_type pos = 0;
	std::string::size_type pos2 = 0;
	bool last = false;

	while (!last && !reqs.empty()) {
		std::string req;
		if ((pos2 = reqs.find("class=\"mRequestItem", pos)) != std::string::npos) {
			req = reqs.substr(pos, pos2 - pos);
			pos = pos2 + 19;
		} else {
			req = reqs.substr(pos);
			last = true;
		}
				
		std::string get = utils::text::source_get_value(&req, 3, "<form", "action=\"", "\">");
		std::string time = utils::text::source_get_value2(&get, "seenrequesttime=", "&\"");

		facebook_user *fbu = new facebook_user();
		fbu->real_name = utils::text::source_get_value(&req, 2, "class=\"actor\">", "</");
		fbu->user_id = utils::text::source_get_value(&get, 2, "id=", "&");

		if (fbu->user_id.length() && fbu->real_name.length())
		{
			HANDLE hContact = AddToContactList(fbu, CONTACT_APPROVE);
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_APPROVE);

			bool seen = false;

			DBVARIANT dbv;
			if (!getString(hContact, "RequestTime", &dbv)) {
				seen = !strcmp(dbv.pszVal, time.c_str());
				db_free(&dbv);
			}

			if (!seen) {
				// This is new request
				setString(hContact, "RequestTime", time.c_str());

				//blob is: uin(DWORD), hContact(HANDLE), nick(ASCIIZ), first(ASCIIZ), last(ASCIIZ), email(ASCIIZ), reason(ASCIIZ)
				//blob is: 0(DWORD), hContact(HANDLE), nick(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ), ""(ASCIIZ)
				DBEVENTINFO dbei = {0};
				dbei.cbSize = sizeof(DBEVENTINFO);
				dbei.szModule = m_szModuleName;
				dbei.timestamp = ::time(NULL);
				dbei.flags = DBEF_UTF;
				dbei.eventType = EVENTTYPE_AUTHREQUEST;
				dbei.cbBlob = (DWORD)(sizeof(DWORD)*2 + fbu->real_name.length() + 5);
					
				PBYTE pCurBlob = dbei.pBlob = (PBYTE) mir_alloc(dbei.cbBlob);					
				*(PDWORD)pCurBlob = 0; pCurBlob += sizeof(DWORD);                    // UID
				*(PDWORD)pCurBlob = (DWORD)hContact; pCurBlob += sizeof(DWORD);      // Contact Handle
				strcpy((char*)pCurBlob, fbu->real_name.data()); pCurBlob += fbu->real_name.length()+1;	// Nickname
				*pCurBlob = '\0'; pCurBlob++;                                        // First Name
				*pCurBlob = '\0'; pCurBlob++;                                        // Last Name
				*pCurBlob = '\0'; pCurBlob++;                                        // E-mail
				*pCurBlob = '\0';                                                    // Reason

				db_event_add(0, &dbei);				

				LOG("      (New) Friendship request from: %s (%s) [%s]", fbu->real_name.c_str(), fbu->user_id.c_str(), time.c_str());
			} else {
				LOG("      (Old) Friendship request from: %s (%s) [%s]", fbu->real_name.c_str(), fbu->user_id.c_str(), time.c_str());
			}
		} else {
			LOG(" !!!  Wrong friendship request");
			LOG(req.c_str());
		}
	}

	facy.handle_success("friendRequests");
}

void FacebookProto::ProcessFeeds(void* data)
{
	if (data == NULL)
		return;

	std::string* resp = (std::string*)data;

	if (!isOnline())
		goto exit;

	CODE_BLOCK_TRY

	LOG("***** Starting processing feeds");

	std::vector< facebook_newsfeed* > news;

	std::string::size_type pos = 0;
	UINT limit = 0;

	*resp = utils::text::slashu_to_utf8(*resp);	
	*resp = utils::text::source_get_value(resp, 2, "\"html\":\"", ">\"");

	while ((pos = resp->find("<div class=\\\"mainWrapper\\\"", pos)) != std::string::npos && limit <= 25)
	{		
		std::string::size_type pos2 = resp->find("<div class=\\\"mainWrapper\\\"", pos+5);
		if (pos2 == std::string::npos)
			pos2 = resp->length();
		
		std::string post = resp->substr(pos, pos2 - pos);
		pos += 5;

		std::string post_header = utils::text::source_get_value(&post, 4, "<h5 class=", "uiStreamHeadline", ">", "<\\/h5>");
		std::string post_message = utils::text::source_get_value(&post, 3, "<h5 class=\\\"uiStreamMessage userContentWrapper", ">", "<\\/h5>");
		std::string post_link = utils::text::source_get_value(&post, 3, "<span class=\\\"uiStreamSource\\\"", ">", "<\\/span>");
		std::string post_attach = utils::text::source_get_value(&post, 4, "<div class=", "uiStreamAttachments", ">", "<form");

		//std::string post_time = utils::text::source_get_value(&post_link, 2, "data-utime=\\\"", "\\\"");
		//std::string post_time_text = utils::text::source_get_value(&post_link, 3, "class=\\\"timestamp livetimestamp", ">", "<");

		// in title keep only name, end of events like "X shared link" put into message
		pos2 = post_header.find("<\\/a>") + 5;
		post_message = post_header.substr(pos2, post_header.length() - pos2) + post_message;
		post_header = post_header.substr(0, pos2);

		// append attachement to message (if any)
		post_message += utils::text::trim(post_attach);

		facebook_newsfeed* nf = new facebook_newsfeed;

		nf->title = utils::text::trim(
			utils::text::special_expressions_decode(
				utils::text::remove_html(post_header)));

		nf->user_id = utils::text::source_get_value(&post_header, 2, "user.php?id=", "\\\"");
		
		nf->link = utils::text::special_expressions_decode(
				utils::text::source_get_value(&post_link, 2, "href=\\\"", "\\\">"));

		nf->text = utils::text::trim(
			utils::text::special_expressions_decode(
				utils::text::remove_html(
					utils::text::edit_html(post_message))));

		//nf->text += "\n" + post_time_text;

		if (!nf->title.length() || !nf->text.length())
		{
			delete nf;
			continue;
		}

		if (nf->text.length() > 500)
		{
			nf->text = nf->text.substr(0, 500);
			nf->text += "...";
		}

		news.push_back(nf);
		pos++;
		limit++;
	}

	for(std::vector<facebook_newsfeed*>::size_type i=0; i<news.size(); i++)
	{
		LOG("      Got newsfeed: %s %s", news[i]->title.c_str(), news[i]->text.c_str());
		TCHAR* szTitle = mir_utf8decodeT(news[i]->title.c_str());
		TCHAR* szText = mir_utf8decodeT(news[i]->text.c_str());
		NotifyEvent(szTitle,szText,this->ContactIDToHContact(news[i]->user_id),FACEBOOK_EVENT_NEWSFEED, &news[i]->link);
		mir_free(szTitle);
		mir_free(szText);
		delete news[i];
	}
	news.clear();

	this->facy.last_feeds_update_ = ::time(NULL);

	LOG("***** Feeds processed");

	CODE_BLOCK_CATCH

	LOG("***** Error processing feeds: %s", e.what());

	CODE_BLOCK_END

exit:
	delete resp;
}

void FacebookProto::SearchAckThread(void *targ)
{
	facy.handle_entry("searchAckThread");

	int count = 0;

	char *arg = mir_utf8encodeT((TCHAR*)targ);
	std::string search = utils::url::encode(arg);
	std::string ssid;

	while (count < 50 && !isOffline())
	{
		std::string get_data = search + "&s=" + utils::conversion::to_string(&count, UTILS_CONV_UNSIGNED_NUMBER);
		if (!ssid.empty())
			get_data += "&ssid=" + ssid;

		http::response resp = facy.flap(REQUEST_SEARCH, NULL, &get_data);

		// Process result data
		facy.validate_response(&resp);

		if (resp.code == HTTP_CODE_OK)
		{
			std::string items = utils::text::source_get_value(&resp.data, 2, "<body", "</body>");

			std::string::size_type pos = 0;
			std::string::size_type pos2 = 0;

			while ((pos = items.find("<td class=\"pic\">", pos)) != std::string::npos) {
				std::string item = items.substr(pos, items.find("</tr>", pos) - pos);
				pos++; count++;

				std::string id = utils::text::source_get_value2(&item, "?id=", "&\"");
				if (id.empty())
					id = utils::text::source_get_value2(&item, "?ids=", "&\"");

				std::string name = utils::text::source_get_value(&item, 4, "<td class=\"name\">", "<a", ">", "</");
				std::string surname;
				std::string nick;
				std::string common = utils::text::source_get_value(&item, 2, "<span class=\"mfss fcg\">", "</span>");

				if ((pos2 = name.find("<span class=\"alternate_name\">")) != std::string::npos) {
					nick = name.substr(pos2 + 30, name.length() - pos2 - 31); // also remove brackets around nickname
					name = name.substr(0, pos2 - 1);
				}

				if ((pos2 = name.find(" ")) != std::string::npos) {
					surname = name.substr(pos2 + 1, name.length() - pos2 - 1);
					name = name.substr(0, pos2);
				}

				// ignore self contact and empty ids
				if (id.empty() || id == facy.self_.user_id)
					continue;

				ptrT tid = mir_utf8decodeT(id.c_str());
				ptrT tname = mir_utf8decodeT(utils::text::special_expressions_decode(name).c_str());
				ptrT tsurname = mir_utf8decodeT(utils::text::special_expressions_decode(surname).c_str());
				ptrT tnick = mir_utf8decodeT(utils::text::special_expressions_decode(nick).c_str());
				ptrT tcommon = mir_utf8decodeT(utils::text::special_expressions_decode(common).c_str());

				PROTOSEARCHRESULT isr = {0};
				isr.cbSize = sizeof(isr);
				isr.flags = PSR_TCHAR;
				isr.id  = tid;
				isr.nick  = tnick;
				isr.firstName = tname;
				isr.lastName = tsurname;
				isr.email = tcommon;

				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&isr);
			}

			ssid = utils::text::source_get_value(&items, 3, "id=\"more_objects\"", "ssid=", "&");
			if (ssid.empty())
				break; // No more results
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);

	facy.handle_success("searchAckThread");

	mir_free(targ);
	mir_free(arg);
}

void FacebookProto::SearchIdAckThread(void *targ)
{
	facy.handle_entry("searchIdAckThread");

	char *arg = mir_utf8encodeT((TCHAR*)targ);
	std::string search = utils::url::encode(arg) + "?";

	if (!isOffline())
	{
		http::response resp = facy.flap(REQUEST_USER_INFO, NULL, &search);

		if (resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end()) {
			search = utils::text::source_get_value(&resp.headers["Location"], 2, FACEBOOK_SERVER_MOBILE"/", "_rdr", true);
			resp = facy.flap(REQUEST_USER_INFO, NULL, &search);
		}

		facy.validate_response(&resp);

		if (resp.code == HTTP_CODE_OK)
		{
			std::string about = utils::text::source_get_value(&resp.data, 2, "<div class=\"timeline", "<div id=\"footer");
		
			std::string id = utils::text::source_get_value2(&about, ";id=", "&\"");
			if (id.empty())
				id = utils::text::source_get_value2(&about, "?bid=", "&\"");
			std::string name = utils::text::source_get_value(&about, 3, "class=\"profileName", ">", "</");
			std::string surname;

			std::string::size_type pos;
			if ((pos = name.find(" ")) != std::string::npos) {
				surname = name.substr(pos + 1, name.length() - pos - 1);
				name = name.substr(0, pos);
			}

			// ignore self contact and empty ids
			if (!id.empty() && id != facy.self_.user_id){
				ptrT tid = mir_utf8decodeT(id.c_str());
				ptrT tname = mir_utf8decodeT(name.c_str());
				ptrT tsurname = mir_utf8decodeT(surname.c_str());

				PROTOSEARCHRESULT isr = {0};
				isr.cbSize = sizeof(isr);
				isr.flags = PSR_TCHAR;
				isr.id  = tid;
				isr.firstName = tname;
				isr.lastName = tsurname;

				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&isr);
			}
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);	

	facy.handle_success("searchIdAckThread");	

	mir_free(targ);
	mir_free(arg);
}