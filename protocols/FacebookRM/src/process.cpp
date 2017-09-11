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

/**
 * Helper function for loading name from database (or use default one specified as parameter), used for title of few notifications.
 */
std::string getContactName(FacebookProto *proto, MCONTACT hContact, const char *defaultName)
{
	std::string name = defaultName;

	DBVARIANT dbv;
	if (!proto->getStringUtf(hContact, FACEBOOK_KEY_NICK, &dbv)) {
		name = dbv.pszVal;
		db_free(&dbv);
	}

	return name;
}

void FacebookProto::ProcessFriendList(void*)
{
	if (isOffline())
		return;

	facy.handle_entry("load_friends");

	// Get friends list
	HttpRequest *request = new UserInfoAllRequest(&facy);
	http::response resp = facy.sendRequest(request);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("load_friends");
		return;
	}

	debugLogA("*** Starting processing friend list");

	try {
		std::map<std::string, facebook_user*> friends;

		bool loadAllContacts = getBool(FACEBOOK_KEY_LOAD_ALL_CONTACTS, DEFAULT_LOAD_ALL_CONTACTS);
		bool pagesAlwaysOnline = getBool(FACEBOOK_KEY_PAGES_ALWAYS_ONLINE, DEFAULT_PAGES_ALWAYS_ONLINE);

		facebook_json_parser(this).parse_friends(&resp.data, &friends, loadAllContacts);

		// Check and update old contacts
		for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
			if (isChatRoom(hContact))
				continue;

			// TODO RM: change name of "Deleted" key to "DeletedTS", remove this code in some next version
			int deletedTS = getDword(hContact, "Deleted", 0);
			if (deletedTS != 0) {
				delSetting(hContact, "Deleted");
				setDword(hContact, FACEBOOK_KEY_DELETED, deletedTS);
			}

			// If this contact is page, set it as invisible (if enabled in options)
			if (pagesAlwaysOnline && getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE) == CONTACT_PAGE) {
				setWord(hContact, "Status", ID_STATUS_INVISIBLE);
			}

			facebook_user *fbu;
			ptrA id(getStringA(hContact, FACEBOOK_KEY_ID));
			if (id != NULL) {
				std::map< std::string, facebook_user* >::iterator iter;

				if ((iter = friends.find(std::string(id))) != friends.end()) {
					// Found contact, update it and remove from map
					fbu = iter->second;

					// TODO RM: remove, because contacts cant change it, so its only for "first run"
					// - but what with contacts, that was added after logon?
					// Update gender
					setByte(hContact, "Gender", (int)fbu->gender);

					// TODO: remove this in some future version?
					// Remove old useless "RealName" field
					delSetting(hContact, "RealName");

					// Update real name and nick
					if (!fbu->real_name.empty()) {
						SaveName(hContact, fbu);
					}

					// Update username
					if (!fbu->username.empty())
						setString(hContact, FACEBOOK_KEY_USERNAME, fbu->username.c_str());
					else
						delSetting(hContact, FACEBOOK_KEY_USERNAME);

					// Update contact type
					setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, fbu->type);
					// TODO: remove that popup and use "Contact added you" event?

					// Wasn't contact removed from "server-list" someday? And is it friend now? (as we can get also non-friends from this request now)?
					if (fbu->type == CONTACT_FRIEND && getDword(hContact, FACEBOOK_KEY_DELETED, 0)) {
						delSetting(hContact, FACEBOOK_KEY_DELETED);

						// Notify it, if user wants to be notified
						if (getByte(FACEBOOK_KEY_EVENT_FRIENDSHIP_ENABLE, DEFAULT_EVENT_FRIENDSHIP_ENABLE)) {
							std::string url = FACEBOOK_URL_PROFILE + fbu->user_id;
							std::string contactname = getContactName(this, hContact, !fbu->real_name.empty() ? fbu->real_name.c_str() : fbu->user_id.c_str());

							ptrW szTitle(mir_utf8decodeW(contactname.c_str()));
							NotifyEvent(szTitle, TranslateT("Contact is back on server-list."), hContact, EVENT_FRIENDSHIP, &url);
						}
					}

					// Check avatar change
					CheckAvatarChange(hContact, fbu->image_url);

					// Mark this contact as deleted ("processed") and delete them later (as there may be some duplicit contacts to use)
					fbu->deleted = true;
				}
				else {
					// Contact is not on "server-list", notify it was removed (if it was real friend before)

					// Was this real friend before?
					if (getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE) == CONTACT_FRIEND) {
						setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_NONE);

						// Wasn't we already been notified about this contact?
						if (!getDword(hContact, FACEBOOK_KEY_DELETED, 0)) {
							setDword(hContact, FACEBOOK_KEY_DELETED, ::time(NULL));

							// Notify it, if user wants to be notified
							if (getByte(FACEBOOK_KEY_EVENT_FRIENDSHIP_ENABLE, DEFAULT_EVENT_FRIENDSHIP_ENABLE)) {
								std::string url = FACEBOOK_URL_PROFILE + std::string(id);
								std::string contactname = getContactName(this, hContact, id);

								ptrW szTitle(mir_utf8decodeW(contactname.c_str()));
								NotifyEvent(szTitle, TranslateT("Contact is no longer on server-list."), hContact, EVENT_FRIENDSHIP, &url);
							}
						}
					}
				}
			}
		}

		// Check remaining contacts in map and add them to contact list
		for (std::map< std::string, facebook_user* >::iterator it = friends.begin(); it != friends.end();) {
			if (!it->second->deleted)
				AddToContactList(it->second, true); // we know this contact doesn't exists, so we force add it

			delete it->second;
			it = friends.erase(it);
		}
		friends.clear();

		debugLogA("*** Friend list processed");
	}
	catch (const std::exception &e) {
		debugLogA("*** Error processing friend list: %s", e.what());
	}
}

void FacebookProto::ProcessUnreadMessages(void*)
{
	if (isOffline())
		return;

	facy.handle_entry("ProcessUnreadMessages");

	HttpRequest *request = new UnreadThreadsRequest(&facy);
	http::response resp = facy.sendRequest(request);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("ProcessUnreadMessages");
		return;
	}

	try {
		std::vector<std::string> threads;
		facebook_json_parser(this).parse_unread_threads(&resp.data, &threads);

		ForkThread(&FacebookProto::ProcessUnreadMessage, new std::vector<std::string>(threads));

		debugLogA("*** Unread threads list processed");
	}
	catch (const std::exception &e) {
		debugLogA("*** Error processing unread threads list: %s", e.what());
	}

	facy.handle_success("ProcessUnreadMessages");
}

void FacebookProto::ProcessUnreadMessage(void *pParam)
{
	if (pParam == NULL)
		return;

	std::vector<std::string> *threads = (std::vector<std::string>*)pParam;

	if (isOffline()) {
		delete threads;
		return;
	}

	facy.handle_entry("ProcessUnreadMessage");

	int offset = 0;
	int limit = 21;

	// FIXME: Rework this whole request as offset doesn't work anyway, and allow to load all the unread messages for each thread (IMHO could be done in 2 single requests = 1) get number of messages for all threads 2) load the counts of messages for all threads)

	// TODO: First load info about amount of unread messages, then load exactly this amount for each thread

	while (!threads->empty()) {		
		
		LIST<char> ids(1);
		for (std::vector<std::string>::size_type i = 0; i < threads->size(); i++) {
			ids.insert(mir_strdup(threads->at(i).c_str()));
		}

		HttpRequest *request = new ThreadInfoRequest(&facy, ids, offset, limit);
		http::response resp = facy.sendRequest(request);

		FreeList(ids);
		ids.destroy();

		if (resp.code == HTTP_CODE_OK) {
			try {
				std::vector<facebook_message> messages;
				facebook_json_parser(this).parse_thread_messages(&resp.data, &messages, false);

				ReceiveMessages(messages, true);
				debugLogA("*** Unread messages processed");
			}
			catch (const std::exception &e) {
				debugLogA("*** Error processing unread messages: %s", e.what());
			}

			facy.handle_success("ProcessUnreadMessage");
		}
		else {
			facy.handle_error("ProcessUnreadMessage");
		}

		offset += limit;
		limit = 20; // TODO: use better limits?

		threads->clear(); // TODO: if we have limit messages from one user, there may be more unread messages... continue with it... otherwise remove that threadd from threads list -- or do it in json parser? hm			 = allow more than "limit" unread messages to be parsed
	}

	delete threads;
}

void FacebookProto::LoadLastMessages(void *pParam)
{
	if (pParam == NULL)
		return;

	MCONTACT hContact = *(MCONTACT*)pParam;
	delete (MCONTACT*)pParam;

	if (isOffline())
		return;

	facy.handle_entry("LoadLastMessages");
	if (!isOnline())
		return;

	bool isChat = isChatRoom(hContact);

	if (isChat && (!m_enableChat || IsSpecialChatRoom(hContact))) // disabled chats or special chatroom (e.g. nofitications)
		return;

	ptrA item_id(getStringA(hContact, isChat ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID));
	if (item_id == NULL) {
		debugLogA("!!! LoadLastMessages(): Contact has no TID/ID");
		return;
	}

	int count = min(FACEBOOK_MESSAGES_ON_OPEN_LIMIT, getByte(FACEBOOK_KEY_MESSAGES_ON_OPEN_COUNT, DEFAULT_MESSAGES_ON_OPEN_COUNT));

	HttpRequest *request = new ThreadInfoRequest(&facy, isChat, (const char*) item_id, count);
	http::response resp = facy.sendRequest(request);

	if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
		facy.handle_error("LoadLastMessages");
		return;
	}

	// Temporarily disable marking messages as read for this contact
	facy.ignore_read.insert(hContact);

	try {
		std::vector<facebook_message> messages;
		facebook_json_parser(this).parse_thread_messages(&resp.data, &messages, false);

		ReceiveMessages(messages, true);
		debugLogA("*** Thread messages processed");
	}
	catch (const std::exception &e) {
		debugLogA("*** Error processing thread messages: %s", e.what());
	}

	facy.handle_success("LoadLastMessages");

	// Enable marking messages as read for this contact
	facy.ignore_read.erase(hContact);

	// And force mark read
	OnDbEventRead(hContact, NULL);
}

void FacebookProto::LoadHistory(void *pParam)
{
	if (pParam == NULL)
		return;

	MCONTACT hContact = *(MCONTACT*)pParam;
	delete (MCONTACT*)pParam;

	ScopedLock s(facy.loading_history_lock_);

	// Allow loading history only from one contact at a time
	if (!isOnline() || facy.loading_history)
		return;

	facy.handle_entry("LoadHistory");

	bool isChat = isChatRoom(hContact);
	if (isChat) // TODO: Support chats?
		return;
	/*if (isChat && (!m_enableChat || IsSpecialChatRoom(hContact))) // disabled chats or special chatroom (e.g. nofitications)
		return;*/

	ptrA item_id(getStringA(hContact, isChat ? FACEBOOK_KEY_TID : FACEBOOK_KEY_ID));
	if (item_id == NULL) {
		debugLogA("!!! LoadHistory(): Contact has no TID/ID");
		return;
	}

	// first get info about this thread and how many messages is there
	http::response resp = facy.sendRequest(new ThreadInfoRequest(&facy, isChat, item_id));

	if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
		facy.handle_error("LoadHistory");
		return;
	}

	int messagesCount = -1;
	int unreadCount = -1;

	facebook_json_parser* p = new facebook_json_parser(this);
	if (p->parse_messages_count(&resp.data, &messagesCount, &unreadCount) == EXIT_FAILURE) {
		delete p;
		facy.handle_error("LoadHistory");
		return;
	}

	// Temporarily disable marking messages as read for this contact
	facy.ignore_read.insert(hContact);
	// Mark we're loading history, so we can behave differently (e.g., stickers won't be refreshed as it slows the whole process down drastically)
	facy.loading_history = true;

	POPUPDATAW pd = { sizeof(pd) };
	pd.iSeconds = 5;
	pd.lchContact = hContact;
	pd.lchIcon = IcoLib_GetIconByHandle(GetIconHandle("conversation")); // TODO: Use better icon
	wcsncpy(pd.lptzContactName, m_tszUserName, MAX_CONTACTNAME);
	wcsncpy(pd.lptzText, TranslateT("Loading history started."), MAX_SECONDLINE);

	HWND popupHwnd = NULL;
	if (ServiceExists(MS_POPUP_ADDPOPUPW)) {
		popupHwnd = (HWND)CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&pd, (LPARAM)APF_RETURN_HWND);
	}

	std::vector<facebook_message> messages;
	std::string firstTimestamp = "";
	std::string firstMessageId = "";
	std::string lastMessageId = "";
	int loadedMessages = 0;
	int messagesPerBatch = messagesCount > 10000 ? 500 : 100;
	for (int batch = 0; batch < messagesCount; batch += messagesPerBatch) {
		if (!isOnline())
			break;

		// Load batch of messages
		resp = facy.sendRequest(new ThreadInfoRequest(&facy, isChat, item_id, batch, firstTimestamp.c_str(), messagesPerBatch));

		if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
			facy.handle_error("LoadHistory");
			break;
		}

		// Parse the result
		try {
			messages.clear();

			p->parse_history(&resp.data, &messages, &firstTimestamp);

			// Receive messages
			std::string previousFirstMessageId = firstMessageId;
			for (std::vector<facebook_message*>::size_type i = 0; i < messages.size(); i++) {
				facebook_message &msg = messages[i];

				// First message might overlap (as we are using it's timestamp for the next loading), so we need to check for it
				if (i == 0) {
					firstMessageId = msg.message_id;
				}
				if (previousFirstMessageId == msg.message_id) {
					continue;
				}
				lastMessageId = msg.message_id;

				// We don't use ProtoChainRecvMsg here as this is just loading of old messages, which we just add to log
				DBEVENTINFO dbei = {};
				if (msg.type == MESSAGE)
					dbei.eventType = EVENTTYPE_MESSAGE;
				else if (msg.type == VIDEO_CALL || msg.type == PHONE_CALL)
					dbei.eventType = FACEBOOK_EVENTTYPE_CALL;
				else
					dbei.eventType = EVENTTYPE_URL; // FIXME: Use better and specific type for our other event types.

				dbei.flags = DBEF_UTF;

				if (!msg.isIncoming)
					dbei.flags |= DBEF_SENT;

				if (!msg.isUnread)
					dbei.flags |= DBEF_READ;

				dbei.szModule = m_szModuleName;
				dbei.timestamp = msg.time;
				dbei.cbBlob = (DWORD)msg.message_text.length() + 1;
				dbei.pBlob = (PBYTE)msg.message_text.c_str();
				db_event_add(hContact, &dbei);

				loadedMessages++;
			}

			// Save last message id of first batch which is latest message completely, because we're going backwards
			if (batch == 0 && !lastMessageId.empty()) {
				setString(hContact, FACEBOOK_KEY_MESSAGE_ID, lastMessageId.c_str());
			}

			debugLogA("*** Load history messages processed");
		}
		catch (const std::exception &e) {
			debugLogA("*** Error processing load history messages: %s", e.what());
			break;
		}

		// Update progress popup
		CMStringW text;
		text.AppendFormat(TranslateT("Loading messages: %d/%d"), loadedMessages, messagesCount);

		if (ServiceExists(MS_POPUP_CHANGETEXTW) && popupHwnd) {
			PUChangeTextW(popupHwnd, text);
		}
		else if (ServiceExists(MS_POPUP_ADDPOPUPW)) {
			wcsncpy(pd.lptzText, text, MAX_SECONDLINE);
			pd.iSeconds = 1;
			popupHwnd = (HWND)CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&pd, (LPARAM)0);
		}

		// There is no more messages
		if (messages.empty() || loadedMessages > messagesCount) {
			break;
		}
	}

	delete p;

	facy.handle_success("LoadHistory");

	// Enable marking messages as read for this contact
	facy.ignore_read.erase(hContact);
	// Reset loading history flag
	facy.loading_history = false;

	if (ServiceExists(MS_POPUP_CHANGETEXTW) && popupHwnd) {
		PUChangeTextW(popupHwnd, TranslateT("Loading history completed."));
	} else if (ServiceExists(MS_POPUP_ADDPOPUPW)) {
		pd.iSeconds = 5;
		wcsncpy(pd.lptzText, TranslateT("Loading history completed."), MAX_SECONDLINE);
		popupHwnd = (HWND)CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&pd, (LPARAM)0);
	}
	// PUDeletePopup(popupHwnd);
}

void parseFeeds(const std::string &text, std::vector<facebook_newsfeed *> &news, DWORD &last_post_time, bool filterAds = true) {
	std::string::size_type pos = 0;
	UINT limit = 0;

	DWORD new_time = last_post_time;

	while ((pos = text.find("fbUserPost\"", pos)) != std::string::npos && limit <= 25)
	{
		std::string post = text.substr(pos, text.find("</form>", pos) - pos);
		pos += 5;

		std::string post_header = utils::text::source_get_value(&post, 3, "<h5", ">", "</h5>");
		std::string post_message = utils::text::source_get_value(&post, 3, " userContent\"", ">", "<form");
		std::string post_link = utils::text::source_get_value(&post, 4, "</h5>", "<a", "href=\"", "\"");
		if (post_link == "#") {
			post_link = utils::text::source_get_value(&post, 5, "</h5>", "<a", "<a", "href=\"", "\"");
		}
		std::string post_time = utils::text::source_get_value(&post, 3, "</h5>", "<abbr", "</a>");

		std::string time = utils::text::source_get_value(&post_time, 2, "data-utime=\"", "\"");
		std::string time_text = utils::text::source_get_value(&post_time, 2, ">", "</abbr>");

		if (time.empty()) {
			// alternative parsing (probably page like or advertisement)
			time = utils::text::source_get_value(&post, 2, "content_timestamp&quot;:&quot;", "&quot;");
		}

		DWORD ttime;
		if (!utils::conversion::from_string<DWORD>(ttime, time, std::dec)) {
			//debugLogA("!!! - Newsfeed with wrong/empty time (probably wrong parsing)\n%s", post.c_str());
			continue;
		}

		if (ttime > new_time) {
			new_time = ttime; // remember newest time from all these posts
			//debugLogA("    - Newsfeed time: %d (new)", ttime);
		}
		else if (ttime <= last_post_time) {
			//debugLogA("    - Newsfeed time: %d (ignored)", ttime);
			continue; // ignore posts older than newest post of previous check
		}
		else {
			//debugLogA("    - Newsfeed time: %d (normal)", ttime);
		}

		std::string timeandloc = utils::text::trim(utils::text::html_entities_decode(utils::text::remove_html(time_text)));
		std::string post_place = utils::text::source_get_value(&post, 4, "</abbr>", "<a", ">", "</a>");
		post_place = utils::text::trim(utils::text::remove_html(post_place));
		if (!post_place.empty()) {
			timeandloc += " · " + post_place;
		}

		// in title keep only name, end of events like "X shared link" put into message
		std::string::size_type pos2 = post_header.find("</a>");
		std::string header_author = utils::text::trim(
			utils::text::html_entities_decode(
			utils::text::remove_html(
			post_header.substr(0, pos2))));
		std::string header_rest = utils::text::trim(
			utils::text::html_entities_decode(
			utils::text::remove_html(
			post_header.substr(pos2, post_header.length() - pos2))));

		// Strip "Translate" and other buttons
		do {
			pos2 = post_message.find("role=\"button\"");
			if (pos2 != std::string::npos) {
				pos2 = post_message.find(">", pos2);
				if (pos2 != std::string::npos) {
					std::string::size_type pos3 = post_message.find("</a>", pos2);
					std::string tmp = post_message.substr(0, pos2);
					if (pos3 != std::string::npos) {
						tmp += post_message.substr(pos3, post_message.length() - pos3);
					}
					post_message = tmp;
				}
			}
		} while (pos2 != std::string::npos);

		// Strip "See more" link
		pos2 = post_message.find("<span class=\"see_more_link_inner\">");
		if (pos2 != std::string::npos) {
			post_message = post_message.substr(0, pos2);
		}

		// Process attachment (if present)
		std::string post_attachment = "";
		pos2 = post_message.find("class=\"mtm\">");
		if (pos2 != std::string::npos) {
			pos2 += 12;
			post_attachment = post_message.substr(pos2, post_message.length() - pos2);
			post_message = post_message.substr(0, pos2);

			// Add new lines between some elements to improve formatting
			utils::text::replace_all(&post_attachment, "</a>", "</a>\n");
			utils::text::replace_all(&post_attachment, "ellipsis\">", "ellipsis\">\n");

			post_attachment = utils::text::trim(
				utils::text::html_entities_decode(
				utils::text::remove_html(post_attachment)));

			post_attachment = utils::text::truncate_utf8(post_attachment, MAX_LINK_DESCRIPTION_LEN);

			if (post_attachment.empty()) {
				// This is some textless attachment, so mention it
				post_attachment = ptrA(mir_utf8encode(Translate("<attachment without text>")));
			}
		}

		post_message = utils::text::trim(
			utils::text::html_entities_decode(
			utils::text::remove_html(post_message)));

		// Truncate text of newsfeed when it's too long
		post_message = utils::text::truncate_utf8(post_message, MAX_NEWSFEED_LEN);

		std::string content = "";
		if (header_rest.length() > 2)
			content += TEXT_ELLIPSIS + header_rest + "\n";
		if (!post_message.empty())
			content += post_message + "\n";
		if (!post_attachment.empty())
			content += TEXT_EMOJI_LINK" " + post_attachment + "\n";
		if (!timeandloc.empty())
			content += TEXT_EMOJI_CLOCK" " + timeandloc;

		facebook_newsfeed* nf = new facebook_newsfeed;

		nf->title = header_author;

		nf->user_id = utils::text::source_get_value(&post_header, 2, "user.php?id=", "&amp;");

		nf->link = utils::text::html_entities_decode(post_link);

		// Check if we don't want to show ads posts
		bool filtered = filterAds && (nf->link.find("/about/ads") != std::string::npos
			|| post.find("class=\"uiStreamSponsoredLink\"") != std::string::npos
			|| post.find("href=\"/about/ads\"") != std::string::npos);

		nf->text = utils::text::trim(content);

		if (filtered || nf->title.empty() || nf->text.empty()) {
			//debugLogA("    \\ Newsfeed (time: %d) is filtered: %s", ttime, filtered ? "advertisement" : (nf->title.empty() ? "title empty" : "text empty"));
			delete nf;
			continue;
		}
		else {
			//debugLogA("    Got newsfeed (time: %d)", ttime);
		}

		news.push_back(nf);
		pos++;
		limit++;
	}

	last_post_time = new_time;
}

void FacebookProto::ProcessMemories(void *p)
{
	if (isOffline())
		return;

	bool manuallyTriggered = (p == MANUALLY_TRIGGERED);
	if (manuallyTriggered) {
		facy.info_notify(TranslateT("Loading memories..."));
	}

	size_t numMemories = 0;

	facy.handle_entry(__FUNCTION__);

	HttpRequest *request = new MemoriesRequest(&facy);
	http::response resp = facy.sendRequest(request);

	if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
		facy.handle_error(__FUNCTION__);
		return;
	}

	std::string jsonData = resp.data.substr(9);
	JSONNode root = JSONNode::parse(jsonData.c_str());
	if (root) {
		const JSONNode &html_ = root["domops"].at((json_index_t)0).at((json_index_t)3).at("__html");
		if (html_) {
			std::string html = utils::text::html_entities_decode(utils::text::slashu_to_utf8(html_.as_string()));

			std::vector<facebook_newsfeed *> news;
			DWORD new_time = 0;
			parseFeeds(html, news, new_time, true);

			if (!news.empty()) {
				Skin_PlaySound("Memories");
			}

			numMemories = news.size();

			for (std::vector<facebook_newsfeed*>::size_type i = 0; i < news.size(); i++)
			{
				ptrW tszTitle(mir_utf8decodeW(news[i]->title.c_str()));
				ptrW tszText(mir_utf8decodeW(news[i]->text.c_str()));

				NotifyEvent(TranslateT("On this day"), tszText, NULL, EVENT_ON_THIS_DAY, &news[i]->link);
				delete news[i];
			}
			news.clear();
		}
	}

	if (manuallyTriggered) {
		CMStringW text(FORMAT, TranslateT("Found %d memories."), numMemories);
		facy.info_notify(text.GetBuffer());
	}

	facy.handle_success(__FUNCTION__);
}

void FacebookProto::ReceiveMessages(std::vector<facebook_message> &messages, bool check_duplicates)
{
	bool naseemsSpamMode = getBool(FACEBOOK_KEY_NASEEMS_SPAM_MODE, false);

	// TODO: make this checking more lightweight as now it is not effective at all...
	if (check_duplicates) {
		// 1. check if there are some message that we already have (compare FACEBOOK_KEY_MESSAGE_ID = last received message ID)
		for (size_t i = 0; i < messages.size(); i++) {
			facebook_message &msg = messages[i];

			MCONTACT hContact = msg.isChat ? ChatIDToHContact(msg.thread_id) : ContactIDToHContact(msg.user_id);
			if (hContact == NULL)
				continue;

			ptrA lastId(getStringA(hContact, FACEBOOK_KEY_MESSAGE_ID));
			if (lastId == NULL)
				continue;

			if (!msg.message_id.compare(lastId)) {
				// Equal, ignore all older messages (including this) from same contact
				for (std::vector<facebook_message*>::size_type j = 0; j < messages.size(); j++) {
					bool equalsId = msg.isChat
						? (messages[j].thread_id == msg.thread_id)
						: (messages[j].user_id == msg.user_id);

					if (equalsId && messages[j].time <= msg.time)
						messages[j].flag_ = 1;
				}
			}
		}

		// 2. remove all marked messages from list
		for (std::vector<facebook_message>::iterator it = messages.begin(); it != messages.end();) {
			if ((*it).flag_ == 1)
				it = messages.erase(it);
			else
				++it;
		}
	}

	std::set<MCONTACT> *hChatContacts = new std::set<MCONTACT>();

	for (std::vector<facebook_message*>::size_type i = 0; i < messages.size(); i++) {
		facebook_message &msg = messages[i];
		if (msg.isChat) {
			if (!m_enableChat)
				continue;

			// Multi-user message
			debugLogA("  < Got chat message ID: %s", msg.message_id.c_str());

			facebook_chatroom *fbc;
			std::string thread_id = msg.thread_id.c_str();

			auto it = facy.chat_rooms.find(thread_id);
			if (it != facy.chat_rooms.end()) {
				fbc = it->second;
			}
			else {
				// In Naseem's spam mode we ignore outgoing messages sent from other instances
				if (naseemsSpamMode && !msg.isIncoming)
					continue;

				// We don't have this chat loaded in memory yet, lets load some info (name, list of users)
				fbc = new facebook_chatroom(thread_id);
				LoadChatInfo(fbc);
				facy.chat_rooms.insert(std::make_pair(thread_id, fbc));
			}

			MCONTACT hChatContact = NULL;
			// RM TODO: better use check if chatroom exists/is in db/is online... no?
			// like: if (ChatIDToHContact(thread_id) == NULL) {
			ptrA users(GetChatUsers(fbc->thread_id.c_str()));
			if (users == NULL) {
				AddChat(fbc->thread_id.c_str(), fbc->chat_name.c_str());
				hChatContact = ChatIDToHContact(fbc->thread_id);
				// Set thread id (TID) for later
				setString(hChatContact, FACEBOOK_KEY_TID, fbc->thread_id.c_str());

				for (auto jt = fbc->participants.begin(); jt != fbc->participants.end(); ++jt) {
					AddChatContact(fbc->thread_id.c_str(), jt->second, false);
				}
			}

			if (!hChatContact)
				hChatContact = ChatIDToHContact(fbc->thread_id);

			if (!hChatContact) {
				// hopefully shouldn't happen, but who knows?
				debugLogW(L"!!! No hChatContact for %s", fbc->thread_id.c_str());
				continue;
			}

			// We don't want to save (this) message ID for chatrooms
			// setString(hChatContact, FACEBOOK_KEY_MESSAGE_ID, msg.message_id.c_str());
			setDword(FACEBOOK_KEY_LAST_ACTION_TS, msg.time);

			// Save TID
			setString(hChatContact, FACEBOOK_KEY_TID, msg.thread_id.c_str());

			// Get name of this chat participant
			std::string name = msg.user_id; // fallback to numeric id
			{
				auto jt = fbc->participants.find(msg.user_id);
				if (jt != fbc->participants.end()) {
					name = jt->second.nick;
				}
			}

			switch (msg.type) {
			default:
			case MESSAGE:
				UpdateChat(fbc->thread_id.c_str(), msg.user_id.c_str(), name.c_str(), msg.message_text.c_str(), msg.time);
				break;
			case ADMIN_TEXT:
				UpdateChat(thread_id.c_str(), NULL, NULL, msg.message_text.c_str());
				break;
			case SUBSCRIBE:
			case UNSUBSCRIBE:
				UpdateChat(thread_id.c_str(), NULL, NULL, msg.message_text.c_str());
				{
					std::vector<std::string> ids;
					utils::text::explode(msg.data, ";", &ids);
					for (std::vector<std::string>::size_type k = 0; k < ids.size(); k++) {
						auto jt = fbc->participants.find(ids[k]);
						if (jt == fbc->participants.end()) {
							// We don't have this user there yet, so load info about him and then process event
							chatroom_participant participant;
							participant.is_former = (msg.type == UNSUBSCRIBE);
							participant.user_id = ids[k];

							// FIXME: Load info about all participants at once
							fbc->participants.insert(std::make_pair(participant.user_id, participant));
							LoadParticipantsNames(fbc);
							jt = fbc->participants.find(ids[k]);
						}
						if (jt != fbc->participants.end()) {
							if (msg.type == SUBSCRIBE)
								AddChatContact(thread_id.c_str(), jt->second, msg.isUnread);
							else
								RemoveChatContact(thread_id.c_str(), jt->second.user_id.c_str(), jt->second.nick.c_str());
						}
					}
				}
				break;
			case THREAD_NAME: {
				UpdateChat(thread_id.c_str(), NULL, NULL, msg.message_text.c_str());

				std::string chatName = (!msg.data.empty() ? msg.data : GenerateChatName(fbc));
				// proto->RenameChat(thread_id.c_str(), chatName.c_str()); // this don't work, why?
				setStringUtf(hChatContact, FACEBOOK_KEY_NICK, chatName.c_str());
				break;
			}
			case THREAD_IMAGE:
				UpdateChat(thread_id.c_str(), NULL, NULL, msg.message_text.c_str());
				break;
			}

			// Automatically mark message as read because chatroom doesn't support onRead event (yet)
			hChatContacts->insert(hChatContact); // std::set checks duplicates at insert automatically
		}
		else {
			// Single-user message
			debugLogA("  < Got message ID: %s", msg.message_id.c_str());

			facebook_user fbu;
			fbu.user_id = msg.user_id;

			MCONTACT hContact = ContactIDToHContact(fbu.user_id);
			if (hContact == NULL) {
				// In Naseem's spam mode we ignore outgoing messages sent from other instances
				if (naseemsSpamMode && !msg.isIncoming)
					continue;

				// We don't have this contact, lets load info about him
				LoadContactInfo(&fbu);

				hContact = AddToContactList(&fbu);
			}

			if (!hContact) {
				// hopefully shouldn't happen, but who knows?
				debugLogA("!!! No hContact for %s", msg.user_id.c_str());
				continue;
			}

			// Save last (this) message ID
			setString(hContact, FACEBOOK_KEY_MESSAGE_ID, msg.message_id.c_str());

			// Save TID
			setString(hContact, FACEBOOK_KEY_TID, msg.thread_id.c_str());

			if (msg.isIncoming && msg.isUnread && msg.type == MESSAGE) {
				PROTORECVEVENT recv = { 0 };
				recv.szMessage = const_cast<char*>(msg.message_text.c_str());
				recv.timestamp = msg.time;
				ProtoChainRecvMsg(hContact, &recv);
			}
			else {
				DBEVENTINFO dbei = {};
				if (msg.type == MESSAGE)
					dbei.eventType = EVENTTYPE_MESSAGE;
				else if (msg.type == VIDEO_CALL || msg.type == PHONE_CALL)
					dbei.eventType = FACEBOOK_EVENTTYPE_CALL;
				else
					dbei.eventType = EVENTTYPE_URL; // FIXME: Use better and specific type for our other event types.

				dbei.flags = DBEF_UTF;

				if (!msg.isIncoming)
					dbei.flags |= DBEF_SENT;

				if (!msg.isUnread)
					dbei.flags |= DBEF_READ;

				dbei.szModule = m_szModuleName;
				dbei.timestamp = msg.time;
				dbei.cbBlob = (DWORD)msg.message_text.length() + 1;
				dbei.pBlob = (PBYTE)msg.message_text.c_str();
				db_event_add(hContact, &dbei);
			}

			// Reset the "message seen" info when we get any new message (doesn't matter if sent from other instance or received)
			if (msg.isUnread || !msg.isIncoming)
				facy.erase_reader(hContact);
		}
	}

	if (!hChatContacts->empty()) {
		ForkThread(&FacebookProto::ReadMessageWorker, (void*)hChatContacts);
	}
	else {
		delete hChatContacts;
	}
}

void FacebookProto::ProcessMessages(void* data)
{
	if (data == NULL)
		return;

	std::string* resp = (std::string*)data;

	if (isOffline()) {
		delete resp;
		return;
	}

	debugLogA("*** Starting processing messages");

	try {
		std::vector<facebook_message> messages;
		facebook_json_parser(this).parse_messages(resp, &messages, &facy.notifications);

		ReceiveMessages(messages);

		if (getBool(FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE, DEFAULT_EVENT_NOTIFICATIONS_ENABLE))
			ShowNotifications();

		debugLogA("*** Messages processed");
	}
	catch (const std::exception &e) {
		debugLogA("*** Error processing messages: %s", e.what());
	}

	delete resp;
}

void FacebookProto::ShowNotifications() {
	ScopedLock s(facy.notifications_lock_);

	// Show popups for unseen notifications and/or write them to chatroom
	for (std::map<std::string, facebook_notification*>::iterator it = facy.notifications.begin(); it != facy.notifications.end(); ++it) {
		facebook_notification *notification = it->second;
		if (notification != NULL && !notification->seen) {
			debugLogA("    Showing popup for notification ID: %s", notification->id.c_str());
			ptrW szText(mir_utf8decodeW(notification->text.c_str()));
			MCONTACT hContact = (notification->user_id.empty() ? NULL : ContactIDToHContact(notification->user_id));
			notification->hWndPopup = NotifyEvent(m_tszUserName, szText, hContact, EVENT_NOTIFICATION, &notification->link, &notification->id, notification->icon);
			notification->seen = true;
		}
	}
}

void FacebookProto::ProcessNotifications(void *p)
{
	if (isOffline())
		return;

	bool manuallyTriggered = (p == MANUALLY_TRIGGERED);
	if (manuallyTriggered) {
		facy.info_notify(TranslateT("Loading notifications..."));
	}

	facy.handle_entry("notifications");

	// Get notifications
	HttpRequest *request = new GetNotificationsRequest(&facy, FACEBOOK_NOTIFICATIONS_LOAD_COUNT);
	http::response resp = facy.sendRequest(request);

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("notifications");
		return;
	}

	// Process notifications
	debugLogA("*** Starting processing notifications");

	try {
		size_t numNotifications = facy.notifications.size();

		facebook_json_parser(this).parse_notifications(&resp.data, &facy.notifications);

		if (manuallyTriggered) {
			numNotifications = facy.notifications.size() - numNotifications;
			CMStringW text(FORMAT, TranslateT("Found %d notifications."), numNotifications);
			facy.info_notify(text.GetBuffer());
		}

		ShowNotifications();

		debugLogA("*** Notifications processed");
	}
	catch (const std::exception &e) {
		debugLogA("*** Error processing notifications: %s", e.what());
	}
}

void FacebookProto::ProcessFriendRequests(void *p)
{
	if (isOffline())
		return;

	bool manuallyTriggered = (p == MANUALLY_TRIGGERED);
	if (manuallyTriggered) {
		facy.info_notify(TranslateT("Loading friendship requests..."));
	}

	facy.handle_entry("friendRequests");

	// Get load friendships
	http::response resp = facy.sendRequest(new GetFriendshipsRequest(facy.mbasicWorks));

	// Workaround not working "mbasic." website for some people
	if (!resp.isValid()) {
		// Remember it didn't worked and try it again (internally it will use "m." this time)
		facy.mbasicWorks = false;
		resp = facy.sendRequest(new GetFriendshipsRequest(facy.mbasicWorks));
	}

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("friendRequests");
		return;
	}

	int numRequestsNew = 0;
	int numRequestsOld = 0;

	// Parse it
	std::string reqs = utils::text::source_get_value(&resp.data, 3, "id=\"friends_center_main\"", "</h3>", "/friends/center/suggestions/");

	std::string::size_type pos = 0;
	std::string::size_type pos2 = 0;
	bool last = (reqs.find("seenrequesttime=") == std::string::npos); // false when there are some requests

	while (!last && !reqs.empty()) {
		std::string req;
		if ((pos2 = reqs.find("</table>", pos)) != std::string::npos) {
			req = reqs.substr(pos, pos2 - pos);
			pos = pos2 + 8;
		} else {
			req = reqs.substr(pos);
			last = true;
		}
		
		std::string get = utils::text::source_get_value(&req, 2, "notifications.php?", "\"");
		std::string time = utils::text::source_get_value2(&get, "seenrequesttime=", "&\"");
		std::string reason = utils::text::remove_html(utils::text::source_get_value(&req, 4, "</a>", "<div", ">", "</div>"));

		facebook_user fbu;
		fbu.real_name = utils::text::remove_html(utils::text::source_get_value(&req, 3, "<a", ">", "</a>"));
		fbu.user_id = utils::text::source_get_value2(&get, "confirm=", "&\"");
		fbu.type = CONTACT_APPROVE;

		if (!fbu.user_id.empty() && !fbu.real_name.empty()) {
			MCONTACT hContact = AddToContactList(&fbu);
			setByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, CONTACT_APPROVE);

			bool isNew = false;
			ptrA oldTime(getStringA(hContact, "RequestTime"));
			if (oldTime == NULL || mir_strcmp(oldTime, time.c_str())) {
				// This is new request
				isNew = true;
				setString(hContact, "RequestTime", time.c_str());

				DB_AUTH_BLOB blob(hContact, fbu.real_name.c_str(), 0, 0, fbu.user_id.c_str(), reason.c_str());

				DBEVENTINFO dbei = {};
				dbei.szModule = m_szModuleName;
				dbei.timestamp = ::time(NULL);
				dbei.flags = DBEF_UTF;
				dbei.eventType = EVENTTYPE_AUTHREQUEST;
				dbei.cbBlob = blob.size();
				dbei.pBlob = blob;
				db_event_add(0, &dbei);
			}
			debugLogA("  < (%s) Friendship request [%s]", (isNew ? "New" : "Old"), time.c_str());

			if (isNew)
				numRequestsNew++;
			else
				numRequestsOld++;
		}
		else debugLogA("!!! Wrong friendship request:\n%s", req.c_str());
	}

	if (manuallyTriggered) {
		CMStringW text;
		if (numRequestsOld > 0)
			text.AppendFormat(TranslateT("Found %d friendship requests (%d seen)."), numRequestsNew, numRequestsOld);
		else
			text.AppendFormat(TranslateT("Found %d friendship requests."), numRequestsNew);
		facy.info_notify(text.GetBuffer());
	}

	facy.handle_success("friendRequests");
}

void FacebookProto::ProcessFeeds(void *p)
{
	if (!isOnline())
		return;

	bool manuallyTriggered = (p == MANUALLY_TRIGGERED);
	if (manuallyTriggered) {
		facy.info_notify(TranslateT("Loading wall posts..."));
	}

	facy.handle_entry("feeds");

	// Get feeds
	HttpRequest *request = new NewsfeedRequest(&facy);
	http::response resp = facy.sendRequest(request);

	if (resp.code != HTTP_CODE_OK || resp.data.empty()) {
		facy.handle_error("feeds");
		return;
	}

	std::vector<facebook_newsfeed *> news;
	DWORD new_time = facy.last_feeds_update_;
	bool filterAds = getBool(FACEBOOK_KEY_FILTER_ADS, DEFAULT_FILTER_ADS);

	parseFeeds(resp.data, news, new_time, filterAds);

	if (!news.empty()) {
		Skin_PlaySound("NewsFeed");
	}

	if (manuallyTriggered) {
		CMStringW text(FORMAT, TranslateT("Found %d wall posts."), news.size());
		facy.info_notify(text.GetBuffer());
	}

	for (std::vector<facebook_newsfeed*>::size_type i = 0; i < news.size(); i++)
	{
		ptrW tszTitle(mir_utf8decodeW(news[i]->title.c_str()));
		ptrW tszText(mir_utf8decodeW(news[i]->text.c_str()));
		MCONTACT hContact = ContactIDToHContact(news[i]->user_id);

		NotifyEvent(tszTitle, tszText, hContact, EVENT_NEWSFEED, &news[i]->link);
		delete news[i];
	}
	news.clear();

	// Set time of last update to time of newest post
	this->facy.last_feeds_update_ = new_time;

	facy.handle_success("feeds");
}

void FacebookProto::ProcessPages(void*)
{
	if (isOffline() || !getByte(FACEBOOK_KEY_LOAD_PAGES, DEFAULT_LOAD_PAGES))
		return;

	facy.handle_entry("load_pages");

	// Get feeds
	http::response resp = facy.sendRequest(new GetPagesRequest());

	if (resp.code != HTTP_CODE_OK) {
		facy.handle_error("load_pages");
		return;
	}

	std::string content = utils::text::source_get_value(&resp.data, 2, "id=\"bookmarksSeeAllSection\"", "</code>");

	std::string::size_type start, end;
	start = content.find("<li", 0);
	while (start != std::string::npos) {
		end = content.find("<li", start + 1);
		if (end == std::string::npos)
			end = content.length();

		std::string item = content.substr(start, end - start);
		//item = utils::text::source_get_value(&item, 2, "data-gt=", ">");

		start = content.find("<li", start + 1);

		std::string id = utils::text::source_get_value(&item, 3, "data-gt=", "bmid&quot;:&quot;", "&quot;");
		std::string title = utils::text::slashu_to_utf8(utils::text::source_get_value(&item, 3, "data-gt=", "title=\"", "\""));
		std::string href = utils::text::source_get_value(&item, 3, "data-gt=", "href=\"", "\"");

		// Ignore pages channel
		if (href.find("/pages/feed") != std::string::npos)
			continue;

		if (id.empty() || title.empty())
			continue;

		debugLogA("    Got page ID: %s", id.c_str());
		facy.pages[id] = title;
	}

	facy.handle_success("load_pages");
}

void FacebookProto::SearchAckThread(void *targ)
{
	facy.handle_entry("searchAckThread");

	int count = 0;
	std::string search = utils::url::encode(T2Utf((wchar_t *)targ).str());
	std::string ssid;
	int pn = 1;

	while (count < 50 && !isOffline())
	{
		SearchRequest *request = new SearchRequest(facy.mbasicWorks, search.c_str(), count, pn, ssid.c_str());
		http::response resp = facy.sendRequest(request);

		if (resp.code == HTTP_CODE_OK)
		{
			std::string items = utils::text::source_get_value(&resp.data, 4, "<body", "</form", "<table", "</table>");

			std::string::size_type pos = 0;
			std::string::size_type pos2 = 0;

			while ((pos = items.find("<tr", pos)) != std::string::npos) {
				std::string item = items.substr(pos, items.find("</tr>", pos) - pos);
				pos++; count++;

				std::string id;
				std::string type; // Type of search result: 69=group, 274=page, 844=event, 2048=contact
				std::string name = utils::text::source_get_value(&item, 3, "<a", ">", "</");
				std::string surname;
				std::string nick;
				std::string common = utils::text::source_get_value(&item, 4, "</a>", "<span", ">", "</span>");

				std::string url = utils::text::source_get_value(&item, 3, "<a", "href=\"", "\"");
				std::string sld = utils::text::source_get_value2(&url, "sld=", "&\"", true);
				// sld is Base64 encoded and then URL encoded string. So replace potential "%3D" with "="
				utils::text::replace_all(&sld, "%3D", "=");
				// decode Base64 string
				ptrA data_((char*)mir_base64_decode(sld.c_str(), 0));
				if (data_) {
					std::string data = data_;
					id = utils::text::source_get_value2(&data, "\"ent_id\":", ",}");
					type = utils::text::source_get_value2(&data, "\"result_type\":", ",}");
				}

				if (type == "274") { // page
					// When searching pages we use whole name as nick and use prefix
					nick = m_pagePrefix + " " + name;
					name = "";
					if (common.empty()) {
						// Pages has additional data in <div>, not in <span> as people
						common = utils::text::source_get_value(&item, 4, "</a>", "<div", ">", "</div>");
					}
				}
				else if (type == "2048") { // people
					// When searching for people we try to parse nick and split first/last name
					if ((pos2 = name.find("<span class=\"alternate_name\">")) != std::string::npos) {
						nick = name.substr(pos2 + 30, name.length() - pos2 - 31); // also remove brackets around nickname
						name = name.substr(0, pos2 - 1);
					}

					if ((pos2 = name.find(" ")) != std::string::npos) {
						surname = name.substr(pos2 + 1, name.length() - pos2 - 1);
						name = name.substr(0, pos2);
					}
				}
				else {
					// This is group or event, let's ignore that
					continue;
				}

				// ignore self contact and empty ids
				if (id.empty() || id == facy.self_.user_id)
					continue;

				ptrW tid(mir_utf8decodeW(id.c_str()));
				ptrW tname(mir_utf8decodeW(utils::text::html_entities_decode(name).c_str()));
				ptrW tsurname(mir_utf8decodeW(utils::text::html_entities_decode(surname).c_str()));
				ptrW tnick(mir_utf8decodeW(utils::text::html_entities_decode(nick).c_str()));
				ptrW tcommon(mir_utf8decodeW(utils::text::html_entities_decode(common).c_str()));

				PROTOSEARCHRESULT psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.id.w = tid;
				psr.nick.w = tnick;
				psr.firstName.w = tname;
				psr.lastName.w = tsurname;
				psr.email.w = tcommon;
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&psr);
			}

			ssid = utils::text::source_get_value(&resp.data, 3, "id=\"more_objects\"", "ssid=", "&");
			pn++; // increment page number
			if (ssid.empty())
				break; // No more results
		}
		else {
			break;
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);

	facy.handle_success("searchAckThread");

	mir_free(targ);
}

void FacebookProto::SearchIdAckThread(void *targ)
{
	facy.handle_entry("searchIdAckThread");

	std::string search = T2Utf((wchar_t*)targ).str();
	if (search.find(FACEBOOK_SERVER_DOMAIN "/") != std::string::npos) {
		// User entered URL, let's extract id/username from it
		std::string id = utils::text::source_get_value2(&search, "/profile.php?id=", "&#", true);
		if (id.empty()) {
			// This link probably contains username (if user entered proper profile url)
			id = utils::text::source_get_value2(&search, FACEBOOK_SERVER_DOMAIN "/", "?&#", true);
		}
		search = id;
	}
	search = utils::url::encode(search);

	if (!isOffline() && !search.empty())
	{
		http::response resp = facy.sendRequest(new ProfileRequest(facy.mbasicWorks, search.c_str()));

		if (resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end()) {
			search = utils::text::source_get_value(&resp.headers["Location"], 2, FACEBOOK_SERVER_MBASIC"/", "_rdr");

			// Use only valid username redirects
			if (search.find("home.php") == std::string::npos)
				resp = facy.sendRequest(new ProfileRequest(facy.mbasicWorks, search.c_str()));
		}

		if (resp.code == HTTP_CODE_OK)
		{
			std::string about = utils::text::source_get_value(&resp.data, 2, "id=\"root\"", "</body>");

			std::string id = utils::text::source_get_value2(&about, ";id=", "&\"");
			if (id.empty())
				id = utils::text::source_get_value2(&about, "?bid=", "&\"");
			std::string name = utils::text::source_get_value(&about, 3, "<strong", ">", "</strong");
			if (name.empty()) {
				name = utils::text::source_get_value(&resp.data, 2, "<title>", "</title>");
			}
			std::string surname;

			std::string::size_type pos;
			if ((pos = name.find(" ")) != std::string::npos) {
				surname = name.substr(pos + 1, name.length() - pos - 1);
				name = name.substr(0, pos);
			}

			// ignore self contact and empty ids
			if (!id.empty() && id != facy.self_.user_id){
				ptrW tid(mir_utf8decodeW(id.c_str()));
				ptrW tname(mir_utf8decodeW(name.c_str()));
				ptrW tsurname(mir_utf8decodeW(surname.c_str()));

				PROTOSEARCHRESULT psr = { 0 };
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.id.w = tid;
				psr.firstName.w = tname;
				psr.lastName.w = tsurname;
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, targ, (LPARAM)&psr);
			}
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, targ, 0);

	facy.handle_success("searchIdAckThread");

	mir_free(targ);
}
