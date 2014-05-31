/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-13 Robert P�sel

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

void facebook_client::client_notify(TCHAR* message)
{
	parent->NotifyEvent(parent->m_tszUserName, message, NULL, FACEBOOK_EVENT_CLIENT);
}

http::response facebook_client::flap(RequestType request_type, std::string* request_data, std::string* request_get_data, int method)
{
	NETLIBHTTPREQUEST nlhr = {sizeof(NETLIBHTTPREQUEST)};
	nlhr.requestType = !method ? choose_method(request_type) : method;
	
	std::string url = choose_proto(request_type);
	url.append(choose_server(request_type, request_data, request_get_data));
	url.append(choose_action(request_type, request_data, request_get_data));

	nlhr.szUrl = (char*)url.c_str();
	nlhr.flags = NLHRF_HTTP11 | choose_security_level(request_type);
	nlhr.headers = get_request_headers(nlhr.requestType, &nlhr.headersCount);

	#ifdef _DEBUG 
		nlhr.flags |= NLHRF_DUMPASTEXT;
	#else
		nlhr.flags |= NLHRF_NODUMP;
	#endif
	
	switch (request_type)
	{
	case REQUEST_MESSAGES_RECEIVE:
		nlhr.timeout = 1000 * 65; break;
	default:
		nlhr.timeout = 1000 * 20; break;
	}

	if (request_data != NULL)
	{
		nlhr.pData = (char*)(*request_data).c_str();
		nlhr.dataLength = (int)request_data->length();
	}

	parent->debugLogA("@@@@@ Sending request to '%s'", nlhr.szUrl);

	switch (request_type)
	{
	case REQUEST_LOGIN:
		nlhr.nlc = NULL;
		break;

	case REQUEST_MESSAGES_RECEIVE:
		nlhr.nlc = hMsgCon;
		nlhr.flags |= NLHRF_PERSISTENT;
		break;

	default:
		WaitForSingleObject(fcb_conn_lock_, INFINITE);
		nlhr.nlc = hFcbCon;
		nlhr.flags |= NLHRF_PERSISTENT;
		break;
	}

	NETLIBHTTPREQUEST* pnlhr = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)handle_, (LPARAM)&nlhr);

	mir_free(nlhr.headers[3].szValue);
	mir_free(nlhr.headers);

	http::response resp;

	switch (request_type)
	{
	case REQUEST_LOGIN:
	case REQUEST_SETUP_MACHINE:
		break;

	case REQUEST_MESSAGES_RECEIVE:
		hMsgCon = pnlhr ? pnlhr->nlc : NULL;
		break;

	default:
		ReleaseMutex(fcb_conn_lock_);
		hFcbCon = pnlhr ? pnlhr->nlc : NULL;
		break;
	}

	if (pnlhr != NULL)
	{
		parent->debugLogA("@@@@@ Got response with code %d", pnlhr->resultCode);
		store_headers(&resp, pnlhr->headers, pnlhr->headersCount);
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	} else {
		parent->debugLogA("!!!!! No response from server (time-out)");
		resp.code = HTTP_CODE_FAKE_DISCONNECTED;
		// Better to have something set explicitely as this value is compaired in all communication requests
	}

	// Get Facebook's error message
	if (resp.code == HTTP_CODE_OK) {
		std::string::size_type pos = resp.data.find("\"error\":");
		if (pos != std::string::npos) {
			pos += 8;
			int error_num = atoi(resp.data.substr(pos, resp.data.find(",", pos) - pos).c_str());
			if (error_num != 0) {
				std::string error = "";

				pos = resp.data.find("\"errorDescription\":\"", pos);
				if (pos != std::string::npos) {
					pos += 20;
					error = resp.data.substr(pos, resp.data.find("\"", pos) - pos);
					error = utils::text::trim(utils::text::special_expressions_decode(utils::text::slashu_to_utf8(error)));
					error = ptrA( mir_utf8decodeA(error.c_str()));	
				}

				std::string title = "";
				pos = resp.data.find("\"errorSummary\":\"", pos);
				if (pos != std::string::npos) {
					pos += 16;
					title = resp.data.substr(pos, resp.data.find("\"", pos) - pos);
					title = utils::text::trim(utils::text::special_expressions_decode(utils::text::slashu_to_utf8(title)));
					title = ptrA( mir_utf8decodeA(title.c_str()));	
				}

				bool silent = resp.data.find("\"silentError\":1") != std::string::npos;

				resp.error_number = error_num;
				resp.error_text = error;
				resp.error_title = title;
				resp.code = HTTP_CODE_FAKE_ERROR;

				parent->debugLogA(" ! !  Received Facebook error: %d -- %s", error_num, error.c_str());
				if (notify_errors(request_type) && !silent)
					client_notify(_A2T(error.c_str()));
			}
		}
	}

	return resp;
}

bool facebook_client::handle_entry(std::string method)
{
	parent->debugLogA("   >> Entering %s()", method.c_str());
	return true;
}

bool facebook_client::handle_success(std::string method)
{
	parent->debugLogA("   << Quitting %s()", method.c_str());
	reset_error();
	return true;
}

bool facebook_client::handle_error(std::string method, int action)
{
	increment_error();
	parent->debugLogA("!!!!! %s(): Something with Facebook went wrong", method.c_str());

	bool result = (error_count_ <= (UINT)parent->getByte(FACEBOOK_KEY_TIMEOUTS_LIMIT, FACEBOOK_TIMEOUTS_LIMIT));
	if (action == FORCE_DISCONNECT || action == FORCE_QUIT)
		result = false;

	if (!result) {
		reset_error();
		
		if (action != FORCE_QUIT)
			parent->SetStatus(ID_STATUS_OFFLINE);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

DWORD facebook_client::choose_security_level(RequestType request_type)
{
	if (this->https_)
		if (request_type != REQUEST_MESSAGES_RECEIVE || parent->getByte(FACEBOOK_KEY_FORCE_HTTPS_CHANNEL, DEFAULT_FORCE_HTTPS_CHANNEL))
			return NLHRF_SSL;

	switch (request_type) {
	case REQUEST_LOGIN:
	case REQUEST_SETUP_MACHINE:
		return NLHRF_SSL;

//	case REQUEST_LOGOUT:
//	case REQUEST_HOME:
//	case REQUEST_DTSG:
//	case REQUEST_BUDDY_LIST:
//	case REQUEST_LOAD_FRIEND:
//	case REQUEST_LOAD_FRIENDS:
//	case REQUEST_USER_INFO:
//	case REQUEST_LOAD_REQUESTS:
//	case REQUEST_SEARCH:
//  case REQUEST_DELETE_FRIEND:
//	case REQUEST_REQUEST_FRIEND:
//	case REQUEST_APPROVE_FRIEND:
//	case REQUEST_CANCEL_REQUEST:
//	case REQUEST_FEEDS:
//	case REQUEST_PAGES:
//	case REQUEST_NOTIFICATIONS:
//	case REQUEST_RECONNECT:
//	case REQUEST_POST_STATUS:
//	case REQUEST_IDENTITY_SWITCH:
//	case REQUEST_STATUS_COMPOSER:
//	case REQUEST_LINK_SCRAPER:
//	case REQUEST_MESSAGE_SEND:
//	case REQUEST_MESSAGE_SEND2:
//	case REQUEST_THREAD_INFO:
//	case REQUEST_MESSAGES_RECEIVE:
//	case REQUEST_VISIBILITY:
//	case REQUEST_POKE:
//	case REQUEST_ASYNC:
//	case REQUEST_MARK_READ:
//	case REQUEST_NOTIFICATIONS_READ:
//	case REQUEST_UNREAD_THREADS:
//	case REQUEST_TYPING_SEND:
	default:
		return (DWORD)0;
	}
}

int facebook_client::choose_method(RequestType request_type)
{
	switch (request_type)
	{
	case REQUEST_LOGIN:
	case REQUEST_SETUP_MACHINE:
	case REQUEST_BUDDY_LIST:
	case REQUEST_POST_STATUS:
	case REQUEST_IDENTITY_SWITCH:
	case REQUEST_STATUS_COMPOSER:
	case REQUEST_LINK_SCRAPER:
	case REQUEST_MESSAGE_SEND:
	case REQUEST_MESSAGE_SEND2:
	case REQUEST_THREAD_INFO:
	case REQUEST_VISIBILITY:
	case REQUEST_POKE:
	case REQUEST_ASYNC:
	case REQUEST_MARK_READ:
	case REQUEST_NOTIFICATIONS_READ:
	case REQUEST_TYPING_SEND:
	case REQUEST_LOGOUT:
	case REQUEST_DELETE_FRIEND:
	case REQUEST_REQUEST_FRIEND:
	case REQUEST_APPROVE_FRIEND:
	case REQUEST_CANCEL_REQUEST:
	case REQUEST_UNREAD_THREADS:
		return REQUEST_POST;

//	case REQUEST_HOME:
//	case REQUEST_DTSG:
//	case REQUEST_MESSAGES_RECEIVE:
//	case REQUEST_FEEDS:
//	case REQUEST_PAGES:
//	case REQUEST_NOTIFICATIONS:
//	case REQUEST_RECONNECT:
//	case REQUEST_LOAD_FRIEND:
//	case REQUEST_LOAD_FRIENDS:
//	case REQUEST_USER_INFO:
//	case REQUEST_LOAD_REQUESTS:
//	case REQUEST_SEARCH:
	default:
		return REQUEST_GET;
	}
}

std::string facebook_client::choose_proto(RequestType request_type)
{
	if (choose_security_level(request_type) == NLHRF_SSL)
		return HTTP_PROTO_SECURE;
	else
		return HTTP_PROTO_REGULAR;	
}

std::string facebook_client::choose_server(RequestType request_type, std::string* data, std::string* get_data)
{
	switch (request_type)
	{
	case REQUEST_LOGIN:
		return FACEBOOK_SERVER_LOGIN;

	case REQUEST_MESSAGES_RECEIVE:
	{
		std::string server = FACEBOOK_SERVER_CHAT;
		utils::text::replace_first(&server, "%s", this->chat_conn_num_.empty() ? "0" : this->chat_conn_num_);
		utils::text::replace_first(&server, "%s", this->chat_channel_host_);
		return server;
	}

	case REQUEST_HOME:
	case REQUEST_DTSG:
	case REQUEST_APPROVE_FRIEND:
	case REQUEST_LOAD_REQUESTS:
	case REQUEST_SEARCH:
	case REQUEST_USER_INFO:
		return FACEBOOK_SERVER_MOBILE;

//	case REQUEST_LOGOUT:
//	case REQUEST_BUDDY_LIST:
//	case REQUEST_LOAD_FRIEND:
//	case REQUEST_LOAD_FRIENDS:
//	case REQUEST_FEEDS:
//	case REQUEST_PAGES:
//	case REQUEST_NOTIFICATIONS:
//	case REQUEST_RECONNECT:
//	case REQUEST_POST_STATUS:
//	case REQUEST_IDENTITY_SWITCH:
//	case REQUEST_STATUS_COMPOSER:
//	case REQUEST_LINK_SCRAPER:
//	case REQUEST_MESSAGE_SEND:
//	case REQUEST_MESSAGE_SEND2:
//	case REQUEST_THREAD_INFO:
//	case REQUEST_VISIBILITY:
//	case REQUEST_POKE:
//	case REQUEST_ASYNC:
//	case REQUEST_MARK_READ:
//	case REQUEST_NOTIFICATIONS_READ:
//	case REQUEST_TYPING_SEND:
//	case REQUEST_SETUP_MACHINE:
//  case REQUEST_DELETE_FRIEND:
//	case REQUEST_REQUEST_FRIEND:
//	case REQUEST_CANCEL_REQUEST:
//	case REQUEST_UNREAD_THREADS:
	default:
		return FACEBOOK_SERVER_REGULAR;
	}
}

std::string facebook_client::choose_action(RequestType request_type, std::string* data, std::string* get_data)
{
	switch (request_type)
	{
	case REQUEST_LOGIN:
		return "/login.php?login_attempt=1";

	case REQUEST_SETUP_MACHINE:
		return "/checkpoint/?next";

	case REQUEST_LOGOUT:
		return "/logout.php";

	case REQUEST_HOME:
		return "/profile.php?v=info";

	case REQUEST_DTSG:
		return "/editprofile.php?edit=current_city&type=basic";

	case REQUEST_BUDDY_LIST:
		return "/ajax/chat/buddy_list.php?__a=1";

	case REQUEST_LOAD_FRIEND:
	{
		std::string action = "/ajax/chat/user_info.php?__a=1&viewer=%s&__user=%s";
		utils::text::replace_all(&action, "%s", self_.user_id);
		if (get_data != NULL) {
			action += "&" + (*get_data);
		}
		return action;
	}

	case REQUEST_LOAD_FRIENDS:
	{
		std::string action = "/ajax/chat/user_info_all.php?__a=1&viewer=%s&__user=%s";
		utils::text::replace_all(&action, "%s", self_.user_id);
		return action;
	}

	case REQUEST_USER_INFO:
	{		
		std::string action = "/%sv=info";
		if (get_data != NULL) {
			utils::text::replace_all(&action, "%s", *get_data);
		}
		return action;
	}

	case REQUEST_LOAD_REQUESTS:
	{
		return "/friends/";
	}

	case REQUEST_SEARCH:
	{
		std::string action = "/search/?search=people&query=";
		if (get_data != NULL) {
			action += *get_data;
		}
		return action;
	}

	case REQUEST_UNREAD_THREADS:
	{
		return "/ajax/mercury/unread_threads.php?__a=1";
	}

	case REQUEST_DELETE_FRIEND:
	{
		std::string action = "/ajax/profile/removefriendconfirm.php?__a=1";
		if (get_data != NULL) {
			action += *get_data;
		}
		return action;
	}

	case REQUEST_REQUEST_FRIEND:
	{
		return "/ajax/add_friend/action.php?__a=1";
	}

	case REQUEST_APPROVE_FRIEND:
	{
		std::string action = "/a/notifications.php?__a=1";
		if (get_data != NULL) {
			action += "&" + (*get_data);
		}
		return action;
	}

	case REQUEST_CANCEL_REQUEST:
	{
		return "/ajax/friends/requests/cancel.php?__a=1";
	}

	case REQUEST_FEEDS:
	{
		std::string action = "/ajax/home/generic.php?" + get_newsfeed_type();
		action += "&__user=" + self_.user_id + "&__a=1";

		/*std::string newest = utils::conversion::to_string((void*)&this->last_feeds_update_, UTILS_CONV_TIME_T);
		utils::text::replace_first(&action, "%s", newest);
		utils::text::replace_first(&action, "%s", self_.user_id);*/
		return action;
	}

	case REQUEST_PAGES:
	{
		return "/bookmarks/pages";
	}

	case REQUEST_NOTIFICATIONS:
	{
		std::string action = "/ajax/notifications/get.php?__a=1&user=%s&time=0&version=2&__user=%s";
		// TODO: use better format notifications request
		// std::string action = "/ajax/notifications/client/get.php?__a=1&user=%s&time=0&version=2&__user=%s";
		utils::text::replace_all(&action, "%s", self_.user_id);
		return action;
	}
	
	case REQUEST_RECONNECT:
	{
		std::string action = "/ajax/presence/reconnect.php?__a=1&reason=%s&fb_dtsg=%s&__user=%s";
		
		if (this->chat_reconnect_reason_.empty())
			this->chat_reconnect_reason_ = "6";

		utils::text::replace_first(&action, "%s", this->chat_reconnect_reason_);
		utils::text::replace_first(&action, "%s", this->dtsg_);
		utils::text::replace_first(&action, "%s", this->self_.user_id);
		return action;
	}

	case REQUEST_POST_STATUS:
		return "/ajax/updatestatus.php?__a=1";

	case REQUEST_IDENTITY_SWITCH:
		return "/identity_switch.php?__a=1";

	case REQUEST_STATUS_COMPOSER:
		return "/ajax/profile/composer.php?__a=1";

	case REQUEST_LINK_SCRAPER:
	{
		std::string action = "/ajax/composerx/attachment/link/scraper/?__a=1&composerurihash=2&scrape_url=";
		if (get_data != NULL) {
			action += utils::url::encode(*get_data);
		}
		return action;
	}

	case REQUEST_MESSAGE_SEND:
		return "/ajax/mercury/send_messages.php?__a=1";

	case REQUEST_MESSAGE_SEND2:
		return "/ajax/messaging/send.php";

	case REQUEST_THREAD_INFO:
		return "/ajax/mercury/thread_info.php?__a=1";

	case REQUEST_MESSAGES_RECEIVE:
	{
		std::string action = "/pull?channel=" + (this->chat_channel_.empty() ? "p_" + self_.user_id : this->chat_channel_);
		action += "&seq=" + (this->chat_sequence_num_.empty() ? "0" : this->chat_sequence_num_);
		action += "&partition=" + (this->chat_channel_partition_.empty() ? "0" : this->chat_channel_partition_);
		action += "&clientid=" + this->chat_clientid_;
		action += "&cb=" + utils::text::rand_string(4, "0123456789abcdefghijklmnopqrstuvwxyz");
		action += "&idle=0&state=active";
		if (!this->chat_sticky_num_.empty())
			action += "&sticky_token=" + this->chat_sticky_num_;

		return action;
	}

	case REQUEST_VISIBILITY:
		return "/ajax/chat/privacy/visibility.php?__a=1";

	case REQUEST_POKE:
		return "/pokes/dialog/?__a=1";

	case REQUEST_ASYNC:
	{
		std::string action = "/ajax/messaging/async.php?__a=1";
		if (get_data != NULL) {
			action += "&" + (*get_data);
		}
		return action;
	}

	case REQUEST_MARK_READ:
		return "/ajax/mercury/change_read_status.php?__a=1";

	case REQUEST_NOTIFICATIONS_READ:
	{
		std::string action = "/ajax/notifications/mark_read.php?__a=1";
		if (get_data != NULL) {
			action += "&" + (*get_data);
		}
		return action;
	}

	case REQUEST_TYPING_SEND:
		return "/ajax/messaging/typ.php?__a=1";

	default:
		return "/?_fb_noscript=1";
	}
}

bool facebook_client::notify_errors(RequestType request_type)
{
	switch (request_type)
	{
	case REQUEST_BUDDY_LIST:
	case REQUEST_MESSAGE_SEND2:
	case REQUEST_MESSAGE_SEND:
	case REQUEST_ASYNC:
		return false;

	default:
		return true;
	}
}

NETLIBHTTPHEADER* facebook_client::get_request_headers(int request_type, int* headers_count)
{
	if (request_type == REQUEST_POST)
		*headers_count = 5;
	else
		*headers_count = 4;

	NETLIBHTTPHEADER* headers = (NETLIBHTTPHEADER*)mir_calloc(sizeof(NETLIBHTTPHEADER)*(*headers_count));

	if (request_type == REQUEST_POST)
	{
		headers[4].szName = "Content-Type";
		headers[4].szValue = "application/x-www-form-urlencoded; charset=utf-8";
	}

	headers[3].szName = "Cookie";
	headers[3].szValue = load_cookies();
	headers[2].szName = "User-Agent";
	headers[2].szValue = (char *)g_strUserAgent.c_str();
	headers[1].szName = "Accept";
	headers[1].szValue = "*/*";
	headers[0].szName = "Accept-Language";
	headers[0].szValue = "en,en-US;q=0.9";

	return headers;
}

std::string facebook_client::get_newsfeed_type()
{
	BYTE feed_type = parent->getByte(FACEBOOK_KEY_FEED_TYPE, 0);
	if (feed_type >= SIZEOF(feed_types))
		feed_type = 0;

	std::string ret = "sk=";
	ret += feed_types[feed_type].id;
	ret += "&key=";
	ret += (feed_type < 2 ? "nf" : feed_types[feed_type].id);
	return ret;
}

std::string facebook_client::get_server_type()
{
	BYTE server_type = parent->getByte(FACEBOOK_KEY_SERVER_TYPE, 0);
	if (server_type >= SIZEOF(server_types))
		server_type = 0;
	return server_types[server_type].id;
}

std::string facebook_client::get_privacy_type()
{
	BYTE privacy_type = parent->getByte(FACEBOOK_KEY_PRIVACY_TYPE, 0);
	if (privacy_type >= SIZEOF(privacy_types))
		privacy_type = 0;
	return privacy_types[privacy_type].id;
}


char* facebook_client::load_cookies()
{
	ScopedLock s(cookies_lock_);

	std::string cookieString = "isfbe=false;";

	if (!cookies.empty())
		for (std::map< std::string, std::string >::iterator iter = cookies.begin(); iter != cookies.end(); ++iter)
		{
			cookieString.append(iter->first);
			cookieString.append(1, '=');
			cookieString.append(iter->second);
			cookieString.append(1, ';');
		}
	
	return mir_strdup(cookieString.c_str());
}

void facebook_client::store_headers(http::response* resp, NETLIBHTTPHEADER* headers, int headersCount)
{
	ScopedLock c(cookies_lock_);

	for (int i = 0; i < headersCount; i++)
	{
		std::string header_name = headers[i].szName; // TODO: Casting?
		std::string header_value = headers[i].szValue; // TODO: Casting?

		if (header_name == "Set-Cookie")
		{
			std::string cookie_name = header_value.substr(0, header_value.find("="));
			std::string cookie_value = header_value.substr(header_value.find("=") + 1, header_value.find(";") - header_value.find("=") - 1);
			if (cookie_value == "deleted")
			{
				parent->debugLogA("      Deleted cookie '%s'", cookie_name.c_str());
				cookies.erase(cookie_name);
			} else {
				parent->debugLogA("      New cookie '%s'", cookie_name.c_str());
				cookies[cookie_name] = cookie_value;
			}
		}
		else
		{ // TODO RM: (un)comment
			//parent->debugLogA("----- Got header '%s': %s", header_name.c_str(), header_value.c_str());
			resp->headers[header_name] = header_value;
		}
	}
}

void facebook_client::clear_cookies()
{
	ScopedLock s(cookies_lock_);

	if (!cookies.empty())
		cookies.clear();
}

void facebook_client::clear_notifications()
{	
	for (std::map<std::string, facebook_notification*>::iterator it = notifications.begin(); it != notifications.end(); ) {
		if (it->second->hWndPopup != NULL)
			PUDeletePopup(it->second->hWndPopup); // close popup

		delete it->second;
		it = notifications.erase(it);
	}

	notifications.clear();
}

void loginError(FacebookProto *proto, std::string error_str) {
	error_str = utils::text::trim(
			utils::text::special_expressions_decode(
				utils::text::remove_html(
					utils::text::edit_html(error_str))));
	
	proto->debugLogA(" ! !  Login error: %s", !error_str.empty() ? error_str.c_str() : "Unknown error");

	TCHAR buf[200];
	mir_sntprintf(buf, SIZEOF(buf), TranslateT("Login error: %s"), 
		(error_str.empty()) ? TranslateT("Unknown error") : ptrT(mir_utf8decodeT(error_str.c_str())));
	proto->facy.client_notify(buf);
}

bool facebook_client::login(const char *username, const char *password)
{
	handle_entry("login");

	username_ = username;
	password_ = password;
	
	if (cookies.empty()) {
		// Set device ID
		ptrA device( parent->getStringA(FACEBOOK_KEY_DEVICE_ID));
		if (device != NULL)
			cookies["datr"] = device;

		// Get initial cookies
		flap(REQUEST_HOME);
	}

	// Prepare login data
	std::string data = "persistent=1";
	data += "&email=" + utils::url::encode(username);
	data += "&pass=" + utils::url::encode(password);

	ptrA locale(parent->getStringA(FACEBOOK_KEY_LOCALE));
	if (locale != NULL)
		data += "&locale=" + std::string(locale);

	// Send validation
	http::response resp = flap(REQUEST_LOGIN, &data);

	// Save Device ID
	if (!cookies["datr"].empty())
		parent->setString(FACEBOOK_KEY_DEVICE_ID, cookies["datr"].c_str());

	if (resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end())
	{
		// Check for invalid requests
		if (resp.headers["Location"].find("invalid_request.php") != std::string::npos) {
			client_notify(TranslateT("Login error: Invalid request."));
			parent->debugLogA(" ! !  Login error: Invalid request.");
			return handle_error("login", FORCE_QUIT);
		}

		// Check whether some Facebook things are required
		if (resp.headers["Location"].find("help.php") != std::string::npos)
		{
			client_notify(TranslateT("Login error: Some Facebook things are required."));
			parent->debugLogA(" ! !  Login error: Some Facebook things are required.");
			// return handle_error("login", FORCE_QUIT);
		}
		
		// Check whether setting Machine name is required
		if (resp.headers["Location"].find("/checkpoint/") != std::string::npos)
		{
			resp = flap(REQUEST_SETUP_MACHINE, NULL, NULL, REQUEST_GET);

			if (resp.data.find("login_approvals_no_phones") != std::string::npos) {
				// Code approval - but no phones in account
				loginError(parent, utils::text::source_get_value(&resp.data, 4, "login_approvals_no_phones", "<div", ">", "</div>"));
				return handle_error("login", FORCE_QUIT);
			}

			std::string inner_data;
			if (resp.data.find("name=\"submit[Continue]\"") != std::string::npos) {
				
				// Check if we need to approve also last unapproved device
				if (resp.data.find("name=\"name_action_selected\"") == std::string::npos) {
					// 1) Continue
					inner_data = "submit[Continue]=Continue";
					inner_data += "&nh=" + utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");
					inner_data += "&fb_dtsg=" + utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\"");
					resp = flap(REQUEST_SETUP_MACHINE, &inner_data);

					// 2) Approve last unknown login
					// inner_data = "submit[I%20don't%20recognize]=I%20don't%20recognize"; // Don't recognize - this will force to change account password
					inner_data = "submit[This%20is%20Okay]=This%20is%20Okay"; // Recognize
					inner_data += "&nh=" + utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");
					inner_data += "&fb_dtsg=" + utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\"");
					resp = flap(REQUEST_SETUP_MACHINE, &inner_data);

					// 3) Save last device
					inner_data = "submit[Continue]=Continue";
					inner_data += "&nh=" + utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");
					inner_data += "&fb_dtsg=" + utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\"");
					inner_data += "&name_action_selected=save_device"; // Save device - or "dont_save"
					resp = flap(REQUEST_SETUP_MACHINE, &inner_data);
				}

				// Save this actual device
				inner_data = "submit[Continue]=Continue";
				inner_data += "&nh=" + utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");
				inner_data += "&fb_dtsg=" + utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\"");
				inner_data += "&name_action_selected=save_device"; // Save device - or "dont_save"
				resp = flap(REQUEST_SETUP_MACHINE, &inner_data);
			}

			// Save actual machine name
			// inner_data = "machine_name=Miranda%20NG&submit[Don't%20Save]=Don't%20Save"; // Don't save
			inner_data = "machine_name=Miranda%20NG&submit[Save%20Device]=Save%20Device"; // Save
			inner_data += "&lsd=" + utils::text::source_get_value(&resp.data, 3, "name=\"lsd\"", "value=\"", "\"");
			inner_data += "&nh=" + utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");
			inner_data += "&fb_dtsg=" + utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\"");

			resp = flap(REQUEST_SETUP_MACHINE, &inner_data);
		}
	}

	switch (resp.code)
	{
	case HTTP_CODE_FAKE_DISCONNECTED:
	{
		// When is error only because timeout, try login once more
		if (handle_error("login"))
			return login(username, password);
		else
			return handle_error("login", FORCE_QUIT);
	}

	case HTTP_CODE_OK: // OK page returned, but that is regular login page we don't want in fact
	{ 
		// Check whether captcha code is required
		if (resp.data.find("id=\"captcha\"") != std::string::npos)
		{
			client_notify(TranslateT("Login error: Captcha code is required. Bad login credentials?"));
			parent->debugLogA(" ! !  Login error: Captcha code is required.");
			return handle_error("login", FORCE_QUIT);
		}

		// Get and notify error message
		loginError(parent, utils::text::source_get_value(&resp.data, 4, "login_error_box", "<div", ">", "</div>")); 
	}
	case HTTP_CODE_FORBIDDEN: // Forbidden
	case HTTP_CODE_NOT_FOUND: // Not Found
	default:
		return handle_error("login", FORCE_QUIT);

	case HTTP_CODE_FOUND: // Found and redirected to Home, Logged in, everything is OK
	{
		if (cookies.find("c_user") != cookies.end()) {
			this->self_.user_id = cookies.find("c_user")->second;
			parent->setString(FACEBOOK_KEY_ID, this->self_.user_id.c_str());
			parent->debugLogA("      Got self user id: %s", this->self_.user_id.c_str());
			return handle_success("login");
		} else {
			client_notify(TranslateT("Login error, probably bad login credentials."));
			parent->debugLogA(" ! !  Login error, probably bad login credentials.");
			return handle_error("login", FORCE_QUIT);
		}
	}
	}
}

bool facebook_client::logout()
{
	if (parent->getByte(FACEBOOK_KEY_DISABLE_LOGOUT, 0))
		return true;

	handle_entry("logout");

	std::string data = "fb_dtsg=" + (!this->dtsg_.empty() ? this->dtsg_ : "0");
	data += "&ref=mb&h=" + this->logout_hash_;

	http::response resp = flap(REQUEST_LOGOUT, &data);

	if (hFcbCon)
		Netlib_CloseHandle(hFcbCon);
	hFcbCon = NULL;

	// Process result
	username_ = password_ = self_.user_id = "";

	switch (resp.code)
	{
	case HTTP_CODE_OK:
	case HTTP_CODE_FOUND:
		return handle_success("logout");

	default:
		return false; // Logout not finished properly, but..okay, who cares :P
	}
}

bool facebook_client::home()
{
	handle_entry("home");

	// get fb_dtsg
	http::response resp = flap(REQUEST_DTSG);

	// Check whether HTTPS connection is required and we don't have it enabled
	if (!this->https_ && resp.headers["Location"].find("https://") != std::string::npos) {
		client_notify(TranslateT("Your account requires HTTPS connection. Activating."));
		parent->setByte(FACEBOOK_KEY_FORCE_HTTPS, 1);
		this->https_ = true;
		return home();
	}

	this->dtsg_ = utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\"");
	parent->debugLogA("      Got self dtsg: %s", this->dtsg_.c_str());

	if (this->dtsg_.empty())
		return false;
		
	resp = flap(REQUEST_HOME);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
	{		
		// Get real name
		this->self_.real_name = utils::text::source_get_value(&resp.data, 2, "<strong class=\"profileName\">", "</strong>");
		
		// Try to get name again, if we've got some some weird version of Facebook
		if (this->self_.real_name.empty())
			this->self_.real_name = utils::text::source_get_value(&resp.data, 4, "id=\"root", "<strong", ">", "</strong>");

		// Get and strip optional nickname
		std::string::size_type pos = this->self_.real_name.find("<span class=\"alternate_name\">");
		if (pos != std::string::npos) {
			this->self_.nick = utils::text::source_get_value(&this->self_.real_name, 2, "<span class=\"alternate_name\">(", ")</span>");
			parent->debugLogA("      Got self nick name: %s", this->self_.nick.c_str());

			this->self_.real_name = this->self_.real_name.substr(0, pos - 1);
		}
		parent->debugLogA("      Got self real name: %s", this->self_.real_name.c_str());

		if (this->self_.real_name.empty()) {
			client_notify(TranslateT("Something happened to Facebook. Maybe there was some major update so you should wait for an update."));
			return handle_error("home", FORCE_QUIT);
		}

		// Save name and nickname
		parent->SaveName(NULL, &this->self_);

		// Get avatar
		this->self_.image_url = utils::text::source_get_value(&resp.data, 3, "class=\"l\"", "<img src=\"", "\"");
		parent->debugLogA("      Got self avatar: %s", this->self_.image_url.c_str());
		parent->CheckAvatarChange(NULL, this->self_.image_url);

		// Get logout hash
		this->logout_hash_ = utils::text::source_get_value2(&resp.data, "/logout.php?h=", "&\"");
		parent->debugLogA("      Got self logout hash: %s", this->logout_hash_.c_str());

		return handle_success("home");
	}
	case HTTP_CODE_FOUND:
		// Work-around for replica_down, f**king hell what's that?
		parent->debugLogA("      REPLICA_DOWN is back in force!");
		return this->home();
	
	default:
		return handle_error("home", FORCE_QUIT);
	}
}

bool facebook_client::chat_state(bool online)
{
	handle_entry("chat_state");
  
	std::string data = (online ? "visibility=1" : "visibility=0");
	data += "&window_id=0";
	data += "&fb_dtsg=" + (!dtsg_.empty() ? dtsg_ : "0");
	data += "&phstamp=0&__user=" + self_.user_id;
	http::response resp = flap(REQUEST_VISIBILITY, &data);
  
	return handle_success("chat_state");
}

bool facebook_client::reconnect()
{
	handle_entry("reconnect");

	// Request reconnect
	http::response resp = flap(REQUEST_RECONNECT);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
	{
		this->chat_channel_ = utils::text::source_get_value(&resp.data, 2, "\"user_channel\":\"", "\"");
		parent->debugLogA("      Got self channel: %s", this->chat_channel_.c_str());
				
		this->chat_channel_partition_ = utils::text::source_get_value2(&resp.data, "\"partition\":", ",}");
		parent->debugLogA("      Got self channel partition: %s", this->chat_channel_partition_.c_str());
		
		this->chat_channel_host_ = utils::text::source_get_value(&resp.data, 2, "\"host\":\"", "\"");
		parent->debugLogA("      Got self channel host: %s", this->chat_channel_host_.c_str());

		this->chat_sequence_num_ = utils::text::source_get_value2(&resp.data, "\"seq\":", ",}");
		parent->debugLogA("      Got self sequence number: %s", this->chat_sequence_num_.c_str());

		this->chat_conn_num_ = utils::text::source_get_value2(&resp.data, "\"max_conn\":", ",}");
		parent->debugLogA("      Got self max_conn: %s", this->chat_conn_num_.c_str());

		return handle_success("reconnect");
	}
	 
	default:
		return handle_error("reconnect", FORCE_DISCONNECT);
	}
}

bool facebook_client::channel()
{
	handle_entry("channel");

	// Get update
	http::response resp = flap(REQUEST_MESSAGES_RECEIVE);

	if (resp.data.empty()) {
		// Something went wrong
		return handle_error("channel");
	}

	std::string type = utils::text::source_get_value(&resp.data, 2, "\"t\":\"", "\"");
	if (type == "continue" || type == "heartbeat") {
		// Everything is OK, no new message received
	}
	else if (type == "lb") {
		// Some new stuff (idk how does it work yet)
		this->chat_channel_host_ = utils::text::source_get_value(&resp.data, 2, "\"vip\":\"", "\"");
		parent->debugLogA("      Got self channel host: %s", this->chat_channel_host_.c_str());

		this->chat_sticky_num_ = utils::text::source_get_value2(&resp.data, "\"sticky\":\"", "\"");
		parent->debugLogA("      Got self sticky number: %s", this->chat_sticky_num_.c_str());
	}
	else if (type == "fullReload" || type == "refresh") {
		// Something went wrong (server flooding?)
		parent->debugLogA("! ! ! Requested %s", type.c_str());

		this->chat_sequence_num_ = utils::text::source_get_value2(&resp.data, "\"seq\":", ",}");
		parent->debugLogA("      Got self sequence number: %s", this->chat_sequence_num_.c_str());

		this->chat_reconnect_reason_ = utils::text::source_get_value2(&resp.data, "\"reason\":", ",}");
		parent->debugLogA("      Reconnect reason: %s", this->chat_reconnect_reason_.c_str());

		if (type == "refresh")
			return this->reconnect();
	}
	else {
		// Something has been received, throw to new thread to process
		std::string* response_data = new std::string(resp.data);
		parent->ForkThread(&FacebookProto::ProcessMessages, response_data);

		// Increment sequence number
		this->chat_sequence_num_ = utils::text::source_get_value2(&resp.data, "\"seq\":", ",}");
		parent->debugLogA("      Got self sequence number: %s", this->chat_sequence_num_.c_str());
	}

	// Return
	switch (resp.code)
	{
	case HTTP_CODE_OK:
		return handle_success("channel");

	case HTTP_CODE_GATEWAY_TIMEOUT:
		// Maybe we have same clientid as other connected client, try to generate different one
		this->chat_clientid_ = utils::text::rand_string(8, "0123456789abcdef");

		// Intentionally fall to handle_error() below
	case HTTP_CODE_FAKE_DISCONNECTED:
	case HTTP_CODE_FAKE_ERROR:
	default:
		return handle_error("channel");
	}
}

bool facebook_client::send_message(MCONTACT hContact, std::string message_recipient, std::string message_text, std::string *error_text, MessageMethod method)
{
	ScopedLock s(send_message_lock_);

	handle_entry("send_message");

	http::response resp;

	switch (method) {
		case MESSAGE_INBOX:
		{
			parent->debugLogA("    > Sending message through INBOX");
			std::string data = "action=send";
			data += "&body=" + utils::url::encode(message_text);
			data += "&recipients[0]=" + message_recipient;
			data += "&__user=" + this->self_.user_id;
			data += "&__a=1";
			data += "&fb_dtsg=" + (!dtsg_.empty() ? dtsg_ : "0");
			data += "&phstamp=0";

			resp = flap(REQUEST_MESSAGE_SEND2, &data);
			break;
		}
		case MESSAGE_MERCURY:
		{
			parent->debugLogA("    > Sending message through CHAT");
			std::string data = "message_batch[0][action_type]=ma-type:user-generated-message";
			data += "&message_batch[0][thread_id]";
			data += "&message_batch[0][author]=fbid:" + this->self_.user_id;
			data += "&message_batch[0][author_email]";
			data += "&message_batch[0][coordinates]";
			data += "&message_batch[0][timestamp]=" + utils::time::mili_timestamp();
			data += "&message_batch[0][timestamp_absolute]";
			data += "&message_batch[0][timestamp_relative]";
			data += "&message_batch[0][is_unread]=false";
			data += "&message_batch[0][is_cleared]=false";
			data += "&message_batch[0][is_forward]=false";
			data += "&message_batch[0][spoof_warning]=false";
			data += "&message_batch[0][source]=source:chat:web";
			data += "&message_batch[0][source_tags][0]=source:chat";
			data += "&message_batch[0][body]=" + utils::url::encode(message_text);
			data += "&message_batch[0][has_attachment]=false";
			data += "&message_batch[0][html_body]=false";
			data += "&message_batch[0][specific_to_list][0]=fbid:" + message_recipient;
			data += "&message_batch[0][specific_to_list][1]=fbid:" + this->self_.user_id;
			data += "&message_batch[0][status]=0";
			data += "&message_batch[0][message_id]";
			data += "&message_batch[0][client_thread_id]=user:" + message_recipient;
			data += "&client=mercury";
			data += "&fb_dtsg=" + (!dtsg_.empty() ? dtsg_ : "0");
			data += "&__user=" + this->self_.user_id;
			data += "&__a=1";
			data += "&phstamp=0";

			resp = flap(REQUEST_MESSAGE_SEND, &data);
			break;
		}
		case MESSAGE_TID:
		{
			parent->debugLogA("    > Sending message through MERCURY (TID)");
			std::string data = "message_batch[0][action_type]=ma-type:user-generated-message";
			data += "&message_batch[0][thread_id]=" + message_recipient;
			data += "&message_batch[0][author]=fbid:" + this->self_.user_id;
			data += "&message_batch[0][timestamp]=" + utils::time::mili_timestamp();
			data += "&message_batch[0][timestamp_absolute]=";
			data += "&message_batch[0][timestamp_relative]=";
			data += "&message_batch[0][is_unread]=false";
			data += "&message_batch[0][is_cleared]=false";
			data += "&message_batch[0][is_forward]=false";
			data += "&message_batch[0][source]=source:chat:web";
			data += "&message_batch[0][body]=" + utils::url::encode(message_text);
			data += "&message_batch[0][has_attachment]=false";
			data += "&message_batch[0][is_html]=false";
			data += "&message_batch[0][message_id]=";
			data += "&fb_dtsg=" + (!dtsg_.empty() ? dtsg_ : "0");
			data += "&__user=" + this->self_.user_id;
			data += "&phstamp=0";

			resp = flap(REQUEST_MESSAGE_SEND, &data);
			break;
		}
		case MESSAGE_ASYNC:
		{
			parent->debugLogA("    > Sending message through ASYNC");
			std::string data = "action=send";
			data += "&body=" + utils::url::encode(message_text);
			data += "&recipients[0]=" + message_recipient;
			data += "&lsd=";
			data += "&fb_dtsg=" + (!dtsg_.empty() ? dtsg_ : "0");

			resp = flap(REQUEST_ASYNC, &data);
			break;
		}
	}
	
	*error_text = resp.error_text;

	switch (resp.error_number)
	{
  	case 0: // Everything is OK
	{
		// Remember this message id
		std::string mid = utils::text::source_get_value(&resp.data, 2, "\"message_id\":\"", "\"");
		parent->setString(hContact, FACEBOOK_KEY_MESSAGE_ID, mid.c_str());
		messages_ignore.insert(std::make_pair(mid, false));
	} break;

    //case 1356002: // You are offline - wtf??

	case 1356003: // Contact is offline
	{
		MCONTACT hContact = parent->ContactIDToHContact(message_recipient);
		if (hContact != NULL)
  			parent->setWord(hContact, "Status", ID_STATUS_OFFLINE);
		return false;
	} break;

  	case 1356026: // Contact has alternative client
	{
		client_notify(TranslateT("Need confirmation for sending messages to other clients.\nOpen Facebook website and try to send message to this contact again!"));
		return false;
	} break;
 
    default: // Other error
		parent->debugLogA(" !!!  Send message error #%d: %s", resp.error_number, resp.error_text);
		return false;
 	}

	switch (resp.code)
	{
	case HTTP_CODE_OK:
		return handle_success("send_message");

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		*error_text = Translate("Timeout when sending message.");

		handle_error("send_message");
		return false;
	}
}

bool facebook_client::post_status(status_data *status)
{
	if (status == NULL || (status->text.empty() && status->url.empty()))
		return false;

	handle_entry("post_status");

	if (status->isPage) {
		std::string data = "fb_dtsg=" + (!this->dtsg_.empty() ? this->dtsg_ : "0");
		data += "&user_id=" + status->user_id;
		data += "&url=" + std::string(FACEBOOK_URL_HOMEPAGE);
		flap(REQUEST_IDENTITY_SWITCH, &data);
	}

	std::string data;
	RequestType request = REQUEST_POST_STATUS;

	if (!status->url.empty()) {
		data = "fb_dtsg=" + (!this->dtsg_.empty() ? this->dtsg_ : "0");
		data += "&composerid=u_jsonp_2_b";
		data += "&targetid=" + (status->user_id.empty() ? this->self_.user_id : status->user_id);
		data += "&istimeline=1&composercontext=composer&onecolumn=1&nctr[_mod]=pagelet_timeline_recent&__a=1&ttstamp=0";
		data += "&__user=" + (status->isPage && !status->user_id.empty() ? status->user_id : this->self_.user_id);
		data += "&loaded_components[0]=maininput&loaded_components[1]=backdateicon&loaded_components[2]=withtaggericon&loaded_components[3]=cameraicon&loaded_components[4]=placetaggericon&loaded_components[5]=mainprivacywidget&loaded_components[6]=withtaggericon&loaded_components[7]=backdateicon&loaded_components[8]=placetaggericon&loaded_components[9]=cameraicon&loaded_components[10]=mainprivacywidget&loaded_components[11]=maininput&loaded_components[12]=explicitplaceinput&loaded_components[13]=hiddenplaceinput&loaded_components[14]=placenameinput&loaded_components[15]=hiddensessionid&loaded_components[16]=withtagger&loaded_components[17]=backdatepicker&loaded_components[18]=placetagger&loaded_components[19]=citysharericon";
		http::response resp = flap(REQUEST_LINK_SCRAPER, &data, &status->url);
		resp.data = utils::text::special_expressions_decode(utils::text::slashu_to_utf8(resp.data));

		data = "&xhpc_context=profile&xhpc_ismeta=1&xhpc_timeline=1&xhpc_composerid=u_jsonp_2_0&is_explicit_place=&composertags_place=&composer_session_id=&composertags_city=&disable_location_sharing=false&composer_predicted_city=&nctr[_mod]=pagelet_composer&__a=1&__dyn=&__req=1f&ttstamp=0";
		std::string form = utils::text::source_get_value(&resp.data, 2, "<form", "</form>");
		utils::text::replace_all(&form, "\\\"", "\"");
		data += "&" + utils::text::source_get_form_data(&form) + "&";
		//data += "&no_picture=0";

		request = REQUEST_STATUS_COMPOSER;
	}

	std::string text = utils::url::encode(status->text);

	data += "fb_dtsg=" + (!this->dtsg_.empty() ? this->dtsg_ : "0");
	data += "&xhpc_targetid=" + (status->user_id.empty() ? this->self_.user_id : status->user_id);
	data += "&__user=" + (status->isPage && !status->user_id.empty() ? status->user_id : this->self_.user_id);
	data += "&xhpc_message=" + text;
	data += "&xhpc_message_text=" + text;
	if (!status->isPage)
		data += "&audience[0][value]=" + get_privacy_type();
	if (!status->place.empty()) {
		data += "&composertags_place_name=";
		data += utils::url::encode(status->place);
	}
	for(std::vector<facebook_user*>::size_type i = 0; i < status->users.size(); i++) {
		data += "&composertags_with[" + utils::conversion::to_string(&i, UTILS_CONV_UNSIGNED_NUMBER);
		data += "]=" + status->users[i]->user_id;
		data += "&text_composertags_with[" + utils::conversion::to_string(&i, UTILS_CONV_UNSIGNED_NUMBER);
		data += "]=" + status->users[i]->real_name;
		delete status->users[i];
	}
	status->users.clear();

	data += "&xhpc_context=profile&xhpc_ismeta=1&xhpc_timeline=1&xhpc_composerid=u_0_2y&is_explicit_place=&composertags_place=&composertags_city=";

	http::response resp = flap(request, &data);

	if (status->isPage) {
		std::string data = "fb_dtsg=" + (!this->dtsg_.empty() ? this->dtsg_ : "0");
		data += "&user_id=" + this->self_.user_id;
		data += "&url=" + std::string(FACEBOOK_URL_HOMEPAGE);
		flap(REQUEST_IDENTITY_SWITCH, &data);
	}

	if (resp.isValid()) {
		parent->NotifyEvent(parent->m_tszUserName, TranslateT("Status update was successful."), NULL, FACEBOOK_EVENT_OTHER);
		return handle_success("post_status");
	}

	return handle_error("post_status");
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_client::save_url(const std::string &url,const std::tstring &filename, HANDLE &nlc)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	NETLIBHTTPREQUEST *resp;
	req.requestType = REQUEST_GET;
	req.szUrl = const_cast<char*>(url.c_str());
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_PERSISTENT | NLHRF_NODUMP;
	req.nlc = nlc;

	resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService(MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(this->parent->m_hNetlibUser), reinterpret_cast<LPARAM>(&req)));

	bool ret = false;

	if (resp) {
		nlc = resp->nlc;
		parent->debugLogA("@@@@@ Saving avatar URL %s to path %s", url.c_str(), _T2A(filename.c_str()));

		// Create folder if necessary
		std::tstring dir = filename.substr(0,filename.rfind('\\'));
		if (_taccess(dir.c_str(), 0))
			CreateDirectoryTreeT(dir.c_str());

		// Write to file
		FILE *f = _tfopen(filename.c_str(), _T("wb"));
		if (f != NULL) {
			fwrite(resp->pData,1,resp->dataLength,f);
			fclose(f);

			ret = _taccess(filename.c_str(), 0) == 0;
		}

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	} else {
		nlc = NULL;
	}

	return ret;
}
