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

void facebook_client::client_notify(wchar_t* message)
{
	parent->NotifyEvent(parent->m_tszUserName, message, NULL, EVENT_CLIENT);
}

void facebook_client::info_notify(wchar_t* message)
{
	parent->NotifyEvent(parent->m_tszUserName, message, NULL, EVENT_OTHER);
}

http::response facebook_client::sendRequest(HttpRequest *request)
{
	http::response resp;

	if (parent->isOffline()) {
		resp.code = HTTP_CODE_FAKE_OFFLINE;
		return resp;
	}

	// Check and append user defined locale if request doesn't have it forced already
	if (!parent->m_locale.empty() && strstr(request->szUrl, "&locale=") == NULL)
		request->Url << CHAR_VALUE("locale", parent->m_locale.c_str());

	request->Headers
		<< CHAR_VALUE("Accept-Language", "en,en-US;q=0.9")
		<< CHAR_VALUE("Accept", "*/*")
		<< CHAR_VALUE("User-Agent", g_strUserAgent.c_str())
		<< CHAR_VALUE("Cookie", ptrA(load_cookies())); // FIXME: Rework load_cookies to not do strdup

	if (request->requestType == REQUEST_POST) {
		request->Headers
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
	}

	// TODO: rather change http_request than doing this ifdef magic here?
/*#ifdef _DEBUG 
	request->flags &= ~NLHRF_NODUMP;
	request->flags |= NLHRF_DUMPASTEXT;
#else
	request->flags &= ~NLHRF_DUMPASTEXT;
	request->flags |= NLHRF_NODUMP;
#endif*/

	// Set persistent connection (or not)
	switch (request->Persistent) {
	case ChannelRequest::NONE:
		request->nlc = NULL;
		request->flags &= ~NLHRF_PERSISTENT;
		break;
	case ChannelRequest::CHANNEL:
		request->nlc = hChannelCon;
		request->flags |= NLHRF_PERSISTENT;
		break;
	case ChannelRequest::MESSAGES:
		request->nlc = hMessagesCon;
		request->flags |= NLHRF_PERSISTENT;
		break;
	case ChannelRequest::DEFAULT:
		WaitForSingleObject(fcb_conn_lock_, INFINITE);
		request->nlc = hFcbCon;
		request->flags |= NLHRF_PERSISTENT;
		break;
	}

	parent->debugLogA("@@@ Sending request to '%s'", request->szUrl);

	// Send the request	
	NETLIBHTTPREQUEST *pnlhr = request->Send(handle_);

	// Remember the persistent connection handle (or not)
	switch (request->Persistent) {
	case ChannelRequest::NONE:
		break;
	case ChannelRequest::CHANNEL:
		hChannelCon = pnlhr ? pnlhr->nlc : NULL;
		break;
	case ChannelRequest::MESSAGES:
		hMessagesCon = pnlhr ? pnlhr->nlc : NULL;
		break;
	case ChannelRequest::DEFAULT:
		ReleaseMutex(fcb_conn_lock_);
		hFcbCon = pnlhr ? pnlhr->nlc : NULL;
		break;
	}

	// Check and copy response data
	if (pnlhr != NULL) {
		parent->debugLogA("@@@ Got response with code %d", pnlhr->resultCode);
		store_headers(&resp, pnlhr->headers, pnlhr->headersCount);
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		Netlib_FreeHttpRequest(pnlhr);
	}
	else {
		parent->debugLogA("!!! No response from server (time-out)");
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
				std::string error;

				pos = resp.data.find("\"errorDescription\":\"", pos);
				if (pos != std::string::npos) {
					pos += 20;

					std::string::size_type pos2 = resp.data.find("\",\"", pos);
					if (pos2 == std::string::npos) {
						pos2 = resp.data.find("\"", pos);
					}

					error = resp.data.substr(pos, pos2 - pos);
					error = utils::text::trim(utils::text::html_entities_decode(utils::text::remove_html(utils::text::slashu_to_utf8(error))));
					error = ptrA(mir_utf8decodeA(error.c_str()));
				}

				std::string title;
				pos = resp.data.find("\"errorSummary\":\"", pos);
				if (pos != std::string::npos) {
					pos += 16;
					title = resp.data.substr(pos, resp.data.find("\"", pos) - pos);
					title = utils::text::trim(utils::text::html_entities_decode(utils::text::remove_html(utils::text::slashu_to_utf8(title))));
					title = ptrA(mir_utf8decodeA(title.c_str()));
				}

				bool silent = resp.data.find("\"silentError\":1") != std::string::npos;

				resp.error_number = error_num;
				resp.error_text = error;
				resp.error_title = title;
				resp.code = HTTP_CODE_FAKE_ERROR;

				parent->debugLogA("!!! Received Facebook error: %d -- %s", error_num, error.c_str());
				if (request->NotifyErrors && !silent)
					client_notify(_A2T(error.c_str()));
			}
		}
	}

	// Delete the request object
	delete request;

	return resp;
}

bool facebook_client::handle_entry(const std::string &method)
{
	parent->debugLogA(" >> Entering %s()", method.c_str());
	return true;
}

bool facebook_client::handle_success(const std::string &method)
{
	parent->debugLogA(" << Quitting %s()", method.c_str());
	reset_error();
	return true;
}

bool facebook_client::handle_error(const std::string &method, int action)
{
	increment_error();
	parent->debugLogA("!!! %s(): Something with Facebook went wrong", method.c_str());

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

std::string facebook_client::get_newsfeed_type()
{
	BYTE feed_type = parent->getByte(FACEBOOK_KEY_FEED_TYPE, 0);
	if (feed_type >= _countof(feed_types))
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
	if (server_type >= _countof(server_types))
		server_type = 0;
	return server_types[server_type].id;
}

std::string facebook_client::get_privacy_type()
{
	BYTE privacy_type = parent->getByte(FACEBOOK_KEY_PRIVACY_TYPE, 0);
	if (privacy_type >= _countof(privacy_types))
		privacy_type = 0;
	return privacy_types[privacy_type].id;
}


char* facebook_client::load_cookies()
{
	ScopedLock s(cookies_lock_);

	std::string cookieString;

	if (!cookies.empty()) {
		for (std::map< std::string, std::string >::iterator iter = cookies.begin(); iter != cookies.end(); ++iter)
		{
			cookieString.append(iter->first);
			cookieString.append(1, '=');
			cookieString.append(iter->second);
			cookieString.append(1, ';');
		}
	}

	return mir_strdup(cookieString.c_str());
}

void facebook_client::store_headers(http::response* resp, NETLIBHTTPHEADER* headers, int headersCount)
{
	ScopedLock c(cookies_lock_);

	for (int i = 0; i < headersCount; i++) {
		std::string header_name = headers[i].szName;
		std::string header_value = headers[i].szValue;

		if (header_name == "Set-Cookie") {
			std::string::size_type pos = header_value.find("=");
			std::string cookie_name = header_value.substr(0, pos);
			std::string cookie_value = header_value.substr(pos + 1, header_value.find(";", pos) - pos - 1);

			if (cookie_value == "deleted")
				cookies.erase(cookie_name);
			else
				cookies[cookie_name] = cookie_value;
		} else {
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
	ScopedLock s(notifications_lock_);

	for (auto it = notifications.begin(); it != notifications.end();) {
		if (it->second->hWndPopup != NULL)
			PUDeletePopup(it->second->hWndPopup); // close popup

		delete it->second;
		it = notifications.erase(it);
	}

	notifications.clear();
}

void facebook_client::clear_chatrooms()
{
	for (auto it = chat_rooms.begin(); it != chat_rooms.end();) {
		delete it->second;
		it = chat_rooms.erase(it);
	}
	chat_rooms.clear();
}

/**
 * Clears readers info for all contacts from readers list and db
 */
void facebook_client::clear_readers()
{
	for (std::map<MCONTACT, time_t>::iterator it = readers.begin(); it != readers.end();) {
		if (parent->isChatRoom(it->first))
			parent->delSetting(it->first, FACEBOOK_KEY_MESSAGE_READERS);

		parent->delSetting(it->first, FACEBOOK_KEY_MESSAGE_READ);
		it = readers.erase(it);
	}
	readers.clear();
}

/**
 * Inserts info to readers list, db and writes to statusbar
 */
void facebook_client::insert_reader(MCONTACT hContact, time_t timestamp, const std::string &readerId)
{
	if (!hContact)
		return;

	if (parent->isChatRoom(hContact)) {
		std::string tid = ptrA(parent->getStringA(hContact, "ChatRoomID"));

		std::string name = readerId;

		// get name of this participant from chatroom's participants list
		auto itRoom = chat_rooms.find(tid);
		if (itRoom != chat_rooms.end()) {
			facebook_chatroom *chatroom = itRoom->second;
			std::map<std::string, chatroom_participant> participants = chatroom->participants;

			// try to get name of this participant
			auto participant = participants.find(readerId);
			if (participant != participants.end()) {
				name = participant->second.nick;
			}
		}
		
		std::wstring treaders;

		// Load old readers
		ptrW told(parent->getWStringA(hContact, FACEBOOK_KEY_MESSAGE_READERS));
		if (told)
			treaders = std::wstring(told) + L", ";

		// Append new reader name and remember them
		std::string reader = utils::text::prepare_name(name, true);
		treaders += _A2T(reader.c_str(), CP_UTF8);
		parent->setWString(hContact, FACEBOOK_KEY_MESSAGE_READERS, treaders.c_str());
	}

	parent->setDword(hContact, FACEBOOK_KEY_MESSAGE_READ, timestamp);
	readers.insert(std::make_pair(hContact, timestamp));
	parent->MessageRead(hContact);
	if (ServiceExists(MS_MESSAGESTATE_UPDATE)) 
	{
		MessageReadData data(timestamp, MRD_TYPE_READTIME); 
		CallService(MS_MESSAGESTATE_UPDATE, hContact, (LPARAM)&data);
	}
}

/**
 * Removes info from readers list, db and clears statusbar
 */
void facebook_client::erase_reader(MCONTACT hContact)
{
	if (parent->isChatRoom(hContact)) {
		parent->delSetting(hContact, FACEBOOK_KEY_MESSAGE_READERS);
	}
	
	parent->delSetting(hContact, FACEBOOK_KEY_MESSAGE_READ);

	readers.erase(hContact);
	Srmm_SetStatusText(hContact, nullptr);
}

void loginError(FacebookProto *proto, std::string error_str) {
	utils::text::replace_all(&error_str, "<br \\/>", "\n");
	utils::text::replace_all(&error_str, "\n\n\n", "\n\n");

	error_str = utils::text::trim(
		utils::text::html_entities_decode(
		utils::text::remove_html(error_str)));

	proto->debugLogA("!!! Login error: %s", !error_str.empty() ? error_str.c_str() : "Unknown error");

	wchar_t buf[200];
	mir_snwprintf(buf, TranslateT("Login error: %s"),
		(error_str.empty()) ? TranslateT("Unknown error") : ptrW(mir_utf8decodeW(error_str.c_str())));
	proto->facy.client_notify(buf);
}

void parseJsCookies(const std::string &search, const std::string &data, std::map<std::string, std::string> &cookies) {
	std::string::size_type pos = 0;
	while ((pos = data.find(search, pos)) != std::string::npos) {
		pos += search.length();

		std::string::size_type pos2 = data.find("\",\"", pos);
		if (pos2 == std::string::npos)
			continue;

		std::string name = utils::url::encode(data.substr(pos, pos2 - pos));

		pos = pos2 + 3;
		pos2 = data.find("\"", pos);
		if (pos2 == std::string::npos)
			continue;

		std::string value = data.substr(pos, pos2 - pos);
		cookies[name] = utils::url::encode(utils::text::html_entities_decode(value));
	}
}

bool facebook_client::login(const char *username, const char *password)
{
	handle_entry("login");

	username_ = username;
	password_ = password;

	std::string postData;
	std::string getData;

	if (cookies.empty()) {
		// Set device ID
		ptrA device(parent->getStringA(FACEBOOK_KEY_DEVICE_ID));
		if (device != NULL)
			cookies["datr"] = device;

		// Get initial cookies
		LoginRequest *request = new LoginRequest();
		http::response resp = sendRequest(request);

		// Also parse cookies set by JavaScript
		parseJsCookies("[\"CookieCore\",\"setWithoutChecksIfFirstPartyContext\",[],[\"", resp.data, cookies);

		// Parse hidden inputs and other data
		std::string form = utils::text::source_get_value(&resp.data, 2, "<form", "</form>");
		utils::text::replace_all(&form, "\\\"", "\"");

		postData = utils::text::source_get_form_data(&form, true);
		getData = utils::text::source_get_value(&form, 2, "login.php?login_attempt=1&amp;", "\"");
	}

	// Send validation
	HttpRequest *request = new LoginRequest(username, password, getData.c_str(), postData.c_str());
	http::response resp = sendRequest(request);

	// Save Device ID
	if (!cookies["datr"].empty())
		parent->setString(FACEBOOK_KEY_DEVICE_ID, cookies["datr"].c_str());

	if (resp.code == HTTP_CODE_FOUND && resp.headers.find("Location") != resp.headers.end())
	{
		std::string location = resp.headers["Location"];

		// Check for invalid requests
		if (location.find("invalid_request.php") != std::string::npos) {
			client_notify(TranslateT("Login error: Invalid request."));
			parent->debugLogA("!!! Login error: Invalid request.");
			return handle_error("login", FORCE_QUIT);
		}

		// Check whether login checks are required
		if (location.find("/checkpoint/") != std::string::npos) {
			request = new SetupMachineRequest();
			resp = sendRequest(request);

			if (resp.data.find("login_approvals_no_phones") != std::string::npos) {
				// Code approval - but no phones in account
				loginError(parent, utils::text::source_get_value(&resp.data, 4, "login_approvals_no_phones", "<div", ">", "</div>"));
				return handle_error("login", FORCE_QUIT);
			}

			if (resp.data.find("name=\"submit[Continue]\"") != std::string::npos) {
				int attempt = 0;
				// Check if we need to put approval code (aka "two-factor auth")
				while (resp.data.find("id=\"approvals_code\"") != std::string::npos) {
					parent->debugLogA("    Login info: Approval code required.");

					std::string fb_dtsg = utils::url::encode(utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\""));
					std::string nh = utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");

					CFacebookGuardDialog guardDialog(parent, fb_dtsg.c_str());
					if (guardDialog.DoModal() != DIALOG_RESULT_OK) {
						parent->SetStatus(ID_STATUS_OFFLINE);
						return false;
					}

					// We need verification code from user (he can get it via Facebook application on phone or by requesting code via SMS)
					const char *givenCode = guardDialog.GetCode();

					request = new SetupMachineRequest(fb_dtsg.c_str(), nh.c_str(), "Continue");
					request->Body << CHAR_VALUE("approvals_code", givenCode);
					resp = sendRequest(request);

					if (resp.data.find("id=\"approvals_code\"") != std::string::npos) {
						// We get no error message if we put wrong code. Facebook just shows same form again.
						if (++attempt >= 3) {
							client_notify(TranslateT("You entered too many invalid verification codes. Plugin will disconnect."));
							parent->debugLogA("!!! Login error: Too many invalid attempts to verification code.");
							return handle_error("login", FORCE_QUIT);
						}
						else {
							client_notify(TranslateT("You entered wrong verification code. Try it again."));
						}
					}
					else {
						// After successful verification is showed different page - classic form to save device (as handled at the bottom)
						break;
					}
				}

				// Save this actual device
				if (resp.data.find("name=\"submit[Continue]\"") != std::string::npos
					&& resp.data.find("name=\"name_action_selected\"") != std::string::npos) {
					std::string fb_dtsg = utils::url::encode(utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\""));
					std::string nh = utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");

					request = new SetupMachineRequest(fb_dtsg.c_str(), nh.c_str(), "Continue");
					request->Body << "&name_action_selected=save_device"; // Save device - or "dont_save"
					resp = sendRequest(request);
				}

				// Check if we need to approve also last unapproved device

				// 1) Continue to check last unknown login
				if (resp.data.find("name=\"submit[Continue]\"") != std::string::npos) {
					std::string fb_dtsg = utils::url::encode(utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\""));
					std::string nh = utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");

					request = new SetupMachineRequest(fb_dtsg.c_str(), nh.c_str(), "Continue");
					resp = sendRequest(request);
				}

				// In this step might be needed identity confirmation
				if (resp.data.find("name=\"birthday_captcha_") != std::string::npos) {
					// Account is locked and needs identity confirmation
					client_notify(TranslateT("Login error: Your account is temporarily locked. You need to confirm this device from web browser."));
					parent->debugLogA("!!! Login error: Birthday confirmation.");
					return handle_error("login", FORCE_QUIT);
				}

				// 2) Approve last unknown login
				if (resp.data.find("name=\"submit[This was me]\"") != std::string::npos) {
					CMStringW tszTitle;
					tszTitle.AppendFormat(L"%s - %s", parent->m_tszUserName, TranslateT("Check last login"));
					CMStringW tszMessage(TranslateT("Do you recognize this activity?"));

					std::string activity = utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 3, "<body", "</strong></div>", "</div>"));
					activity = utils::text::trim(utils::text::html_entities_decode(utils::text::remove_html(activity)));
					if (!activity.empty()) {
						tszMessage.AppendFormat(L"\n\n%s", ptrW(mir_utf8decodeW(activity.c_str())));
					}

					if (MessageBox(0, tszMessage, tszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1) != IDYES) {
						// We will cancel connecting right away, because we don't want to handle password changing via Miranda
						client_notify(TranslateT("Login error: You need to confirm last unknown login or revoke it from web browser."));
						return handle_error("login", FORCE_QUIT);
					}

					std::string fb_dtsg = utils::url::encode(utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\""));
					std::string nh = utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");

					request = new SetupMachineRequest(fb_dtsg.c_str(), nh.c_str(), "This was me"); // Recognize device (or "This wasn't me" - this will force to change account password)
					resp = sendRequest(request);

					// 3) Save last device
					fb_dtsg = utils::url::encode(utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\""));
					nh = utils::text::source_get_value(&resp.data, 3, "name=\"nh\"", "value=\"", "\"");
					
					request = new SetupMachineRequest(fb_dtsg.c_str(), nh.c_str(), "Continue");
					request->Body << "&name_action_selected=save_device"; // Save device - or "dont_save"
					resp = sendRequest(request);
				}
			}
			else if (resp.data.find("name=\"submit[Get Started]\"") != std::string::npos) {
				if (!parent->getBool(FACEBOOK_KEY_TRIED_DELETING_DEVICE_ID)) {
					// Try to remove DeviceID and login again
					cookies["datr"] = "";
					parent->delSetting(FACEBOOK_KEY_DEVICE_ID);
					parent->setByte(FACEBOOK_KEY_TRIED_DELETING_DEVICE_ID, 1);
					return login(username, password);
				} else {
					// Reset flag
					parent->delSetting(FACEBOOK_KEY_TRIED_DELETING_DEVICE_ID);
					// Facebook things that computer was infected by malware and needs cleaning
					client_notify(TranslateT("Login error: Facebook thinks your computer is infected. Solve it by logging in via 'private browsing' mode of your web browser and run their antivirus check."));
					parent->debugLogA("!!! Login error: Facebook requires computer scan.");
					return handle_error("login", FORCE_QUIT);
				}
			}
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
		if (resp.data.find("id=\"captcha\"") != std::string::npos) {
			client_notify(TranslateT("Login error: Captcha code is required. You need to confirm this device from web browser."));
			parent->debugLogA("!!! Login error: Captcha code is required.");
			return handle_error("login", FORCE_QUIT);
		}

		// Get and notify error message
		std::string error = utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 3, "[\"LoginFormError\"", "\"__html\":\"", "\"}"));
		if (error.empty())
			error = utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 3, "role=\"alert\"", ">", "</div"));
		if (error.empty())
			error = utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 3, "id=\"globalContainer\"", ">", "</div"));
		if (error.empty())
			error = utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 2, "<strong>", "</strong"));
		loginError(parent, error);
	}
	case HTTP_CODE_FORBIDDEN: // Forbidden
	case HTTP_CODE_NOT_FOUND: // Not Found
	default:
		return handle_error("login", FORCE_QUIT);

	case HTTP_CODE_FOUND: // Found and redirected somewhere
	{
		if (resp.headers.find("Location") != resp.headers.end()) {
			std::string redirectUrl = resp.headers["Location"];
			std::string expectedUrl = HTTP_PROTO_SECURE FACEBOOK_SERVER_REGULAR "/";

			// Remove eventual parameters
			std::string::size_type pos = redirectUrl.rfind("?");
			if (pos != std::string::npos)
				redirectUrl = redirectUrl.substr(0, pos);
			
			if (redirectUrl != expectedUrl) {
				// Unexpected redirect, but we try to ignore it - maybe we were logged in anyway
				parent->debugLogA("!!! Login error: Unexpected redirect: %s (Original: %s) (Expected: %s)", redirectUrl.c_str(), resp.headers["Location"].c_str(), expectedUrl.c_str());
			}
		}

		if (cookies.find("c_user") != cookies.end()) {
			// Probably logged in, everything seems OK
			this->self_.user_id = cookies.find("c_user")->second;
			parent->setString(FACEBOOK_KEY_ID, this->self_.user_id.c_str());
			parent->debugLogA("    Got self user id: %s", this->self_.user_id.c_str());
			return handle_success("login");
		}
		else {
			client_notify(TranslateT("Login error, probably bad login credentials."));
			parent->debugLogA("!!! Login error, probably bad login credentials.");
			return handle_error("login", FORCE_QUIT);
		}
	}
	}
}

bool facebook_client::logout()
{
	handle_entry("logout");

	LogoutRequest *request = new LogoutRequest(this->dtsg_.c_str(), this->logout_hash_.c_str());
	http::response resp = sendRequest(request);

	this->username_.clear();
	this->password_.clear();
	this->self_.user_id.clear();

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
	http::response resp = sendRequest(new DtsgRequest());

	this->dtsg_ = utils::url::encode(utils::text::source_get_value(&resp.data, 3, "name=\"fb_dtsg\"", "value=\"", "\""));
	{
		// Compute ttstamp from dtsg_
		std::stringstream csrf;
		for (unsigned int i = 0; i < this->dtsg_.length(); i++) {
			csrf << (int)this->dtsg_.at(i);
		}
		this->ttstamp_ = "2" + csrf.str();
	}	

	if (this->dtsg_.empty()) {
		parent->debugLogA("!!! Empty dtsg. Source code:\n%s", resp.data.c_str());
		client_notify(TranslateT("Could not load communication token. You should report this and wait for plugin update."));
		return handle_error("home", FORCE_QUIT);
	} else {
		parent->debugLogA("    Got self dtsg");
	}

	resp = sendRequest(new HomeRequest());

	switch (resp.code)
	{
	case HTTP_CODE_OK:
	{
		std::string touchSearch = "{\"id\":" + this->self_.user_id;
		std::string touchData = utils::text::source_get_value(&resp.data, 2, touchSearch.c_str(), "}");

		// Get real name (from touch version)
		if (!touchData.empty())
			this->self_.real_name = utils::text::html_entities_decode(utils::text::slashu_to_utf8(utils::text::source_get_value(&touchData, 2, "\"name\":\"", "\"")));

		// Another attempt to get real name (from mbasic version)
		if (this->self_.real_name.empty())
			this->self_.real_name = utils::text::source_get_value(&resp.data, 4, "id=\"root", "<strong", ">", "</strong>");

		// Try to get name again, if we've got some some weird version of Facebook
		if (this->self_.real_name.empty())
			this->self_.real_name = utils::text::source_get_value(&resp.data, 5, "id=\"root", "</a>", "<div", ">", "</div>");

		// Another attempt to get name
		if (this->self_.real_name.empty())
			this->self_.real_name = utils::text::source_get_value(&resp.data, 5, "id=\"root", "</td>", "<div", ">", "</td>");

		// Get and strip optional nickname
		std::string::size_type pos = this->self_.real_name.find("<span class=\"alternate_name\">");
		if (pos != std::string::npos) {
			this->self_.nick = utils::text::source_get_value(&this->self_.real_name, 2, "<span class=\"alternate_name\">(", ")</span>");
			parent->debugLogA("    Got self nick name: %s", this->self_.nick.c_str());

			this->self_.real_name = this->self_.real_name.substr(0, pos - 1);
		}
		
		// Another attempt to get optional nickname
		if (this->self_.nick.empty())
			this->self_.nick = utils::text::html_entities_decode(utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 3, "class=\\\"alternate_name\\\"", ">(", ")\\u003C\\/")));

		this->self_.real_name = utils::text::remove_html(this->self_.real_name);
		parent->debugLogA("    Got self real name (nickname): %s (%s)", this->self_.real_name.c_str(), this->self_.nick.c_str());
		parent->SaveName(NULL, &this->self_);

		// Get avatar (from touch version)
		if (!touchData.empty())
			this->self_.image_url = utils::text::html_entities_decode(utils::text::slashu_to_utf8(utils::text::source_get_value(&touchData, 2, "\"pic\":\"", "\"")));

		// Another attempt to get avatar(from mbasic version)
		if (this->self_.image_url.empty())
			this->self_.image_url = utils::text::source_get_value(&resp.data, 3, "id=\"root", "<img src=\"", "\"");
		
		// Another attempt to get avatar
		if (this->self_.image_url.empty()) {
			this->self_.image_url = utils::text::source_get_value(&resp.data, 3, "id=\"root", "/photo.php?", "\"");
			
			// Prepare this special url (not direct image url) to be handled correctly in CheckAvatarChange()
			// It must contain "/" at the beginning and also shouldn't contain "?" as parameters after that are stripped
			if (!this->self_.image_url.empty())
				this->self_.image_url = "/" + this->self_.image_url;
		}
		
		// Final attempt to get avatar as on some pages is only link to photo page and not link to image itself
		if (this->self_.image_url.empty()) {
			HttpRequest *request = new ProfilePictureRequest(this->mbasicWorks, self_.user_id.c_str());
			http::response resp2 = sendRequest(request);
				
			// Get avatar (from mbasic version of photo page)
			this->self_.image_url = utils::text::html_entities_decode(utils::text::source_get_value(&resp2.data, 3, "id=\"root", "<img src=\"", "\""));

			// Get avatar (from touch version)
			if (this->self_.image_url.empty())
				this->self_.image_url = utils::text::html_entities_decode(utils::text::source_get_value(&resp2.data, 3, "id=\"root", "background-image: url(&quot;", "&quot;)"));

			// Sometimes even Facebook doesn't show any picture at all! So just ignore this error in that case...
			if (this->self_.image_url.empty()) {
				parent->debugLogA("!!! Empty avatar even from avatar page. Source code:\n%s", resp2.data.c_str());
				
				// Set dumb avatar "url" (see how it works in CheckAvatarChange()) so we can't tell if/when the avatar changed, but it will be loaded at least once
				this->self_.image_url = "/NO_AVATAR/";
			}
		}

		parent->debugLogA("    Got self avatar: %s", this->self_.image_url.c_str());
		parent->CheckAvatarChange(NULL, this->self_.image_url);

		// Get logout hash
		this->logout_hash_ = utils::text::source_get_value2(&resp.data, "/logout.php?h=", "&\"");
		parent->debugLogA("    Got self logout hash: %s", this->logout_hash_.c_str());

		if (this->self_.real_name.empty() || this->self_.image_url.empty() || this->logout_hash_.empty()) {
			parent->debugLogA("!!! Empty nick/avatar/hash. Source code:\n%s", resp.data.c_str());
			client_notify(TranslateT("Could not load all required data. Plugin may still work correctly, but you should report this and wait for plugin update."));
		}

		return handle_success("home");
	}
	case HTTP_CODE_FOUND:
		// Work-around for replica_down, f**king hell what's that?
		parent->debugLogA("!!! REPLICA_DOWN is back in force!");
		return this->home();

	default:
		return handle_error("home", FORCE_QUIT);
	}
}

bool facebook_client::chat_state(bool online)
{
	handle_entry("chat_state");

	HttpRequest *request = new SetVisibilityRequest(this, online);
	http::response resp = sendRequest(request);

	if (!resp.error_title.empty())
		return handle_error("chat_state");

	return handle_success("chat_state");
}

bool facebook_client::reconnect()
{
	handle_entry("reconnect");

	// Request reconnect
	http::response resp = sendRequest(new ReconnectRequest(this));

	switch (resp.code)
	{
	case HTTP_CODE_OK:
	{
		this->chat_channel_ = utils::text::source_get_value(&resp.data, 2, "\"user_channel\":\"", "\"");
		parent->debugLogA("    Got self channel: %s", this->chat_channel_.c_str());

		this->chat_channel_partition_ = utils::text::source_get_value2(&resp.data, "\"partition\":", ",}");
		parent->debugLogA("    Got self channel partition: %s", this->chat_channel_partition_.c_str());

		this->chat_channel_host_ = utils::text::source_get_value(&resp.data, 2, "\"host\":\"", "\"");
		parent->debugLogA("    Got self channel host: %s", this->chat_channel_host_.c_str());

		this->chat_sequence_num_ = utils::text::source_get_value2(&resp.data, "\"seq\":", ",}");
		parent->debugLogA("    Got self sequence number: %s", this->chat_sequence_num_.c_str());

		this->chat_conn_num_ = utils::text::source_get_value2(&resp.data, "\"max_conn\":", ",}");
		parent->debugLogA("    Got self max_conn: %s", this->chat_conn_num_.c_str());

		this->chat_sticky_num_ = utils::text::source_get_value(&resp.data, 2, "\"sticky_token\":\"", "\"");
		parent->debugLogA("    Got self sticky_token: %s", this->chat_sticky_num_.c_str());

		//std::string retry_interval = utils::text::source_get_value2(&resp.data, "\"retry_interval\":", ",}");
		//parent->debugLogA("    Got self retry_interval: %s", retry_interval.c_str());

		//std::string visibility = utils::text::source_get_value2(&resp.data, "\"visibility\":", ",}");
		//parent->debugLogA("    Got self visibility: %s", visibility.c_str());

		// Send activity_ping after each reconnect
		activity_ping();

		return handle_success("reconnect");
	}

	default:
		return handle_error("reconnect", FORCE_DISCONNECT);
	}
}

bool facebook_client::channel()
{
	handle_entry("channel");

	// Get updates
	ChannelRequest *request = new ChannelRequest(this, ChannelRequest::PULL);
	http::response resp = sendRequest(request);

	if (resp.data.empty()) {
		// Something went wrong
		return handle_error("channel");
	}

	// Load traceId, if present
	std::string traceId = utils::text::source_get_value(&resp.data, 2, "\"tr\":\"", "\"");
	if (!traceId.empty()) {
		this->chat_traceid_ = traceId;
	}

	std::string type = utils::text::source_get_value(&resp.data, 2, "\"t\":\"", "\"");
	parent->debugLogA("Pull response type = %s", type.c_str());

	if (type == "continue" || type == "heartbeat") {
		// Everything is OK, no new message received
	}
	else if (type == "lb") {
		// Some new stuff (idk how does it work yet)
		this->chat_sticky_pool_ = utils::text::source_get_value(&resp.data, 2, "\"pool\":\"", "\"");
		parent->debugLogA("    Got self sticky pool: %s", this->chat_sticky_pool_.c_str());

		this->chat_sticky_num_ = utils::text::source_get_value2(&resp.data, "\"sticky\":\"", "\"");
		parent->debugLogA("    Got self sticky number: %s", this->chat_sticky_num_.c_str());
	}
	else if (type == "refresh") {
		// Requested relogin (due to some settings change, removing this session, etc.)
		parent->debugLogA("!!! Requested refresh");

		this->chat_sequence_num_ = utils::text::source_get_value2(&resp.data, "\"seq\":", ",}");
		parent->debugLogA("    Got self sequence number: %s", this->chat_sequence_num_.c_str());

		this->chat_reconnect_reason_ = utils::text::source_get_value2(&resp.data, "\"reason\":", ",}");
		parent->debugLogA("    Got reconnect reason: %s", this->chat_reconnect_reason_.c_str());

		return this->reconnect();
	}
	else if (!type.empty()) { // for "msg", "fullReload" and maybe also other types
		// Something has been received, throw to new thread to process
		std::string* response_data = new std::string(resp.data);
		parent->ForkThread(&FacebookProto::ProcessMessages, response_data);

		// Get new sequence number
		std::string seq = utils::text::source_get_value2(&resp.data, "\"seq\":", ",}");
		parent->debugLogA("    Got self sequence number: %s", seq.c_str());

		if (type == "msg") {
			// Update msgs_recv number for every "msg" type we receive (during fullRefresh/reload responses it stays the same)
			this->chat_msgs_recv_++;
		}
		else if (type == "fullReload") {
			// At fullReload we force our seq number to received value (there may have been some error or something)
			this->chat_sequence_num_ = seq;
		}

		// Check if it's different from our old one (which means we should increment our old one)
		if (seq != this->chat_sequence_num_) {
			// Facebook now often return much bigger number which results in skipping few data requests, so we increment it manually
			// Bigger skips (when there is some problem or something) are handled when fullreload/refresh response type
			int iseq = 0;
			if (utils::conversion::from_string<int>(iseq, this->chat_sequence_num_, std::dec)) {
				// Increment and convert it back to string
				iseq++;
				std::string newSeq = utils::conversion::to_string(&iseq, UTILS_CONV_SIGNED_NUMBER);

				// Check if we have different seq than the one from Facebook
				if (newSeq != seq) {
					parent->debugLogA("!!! Use self incremented sequence number: %s (instead of: %s)", newSeq.c_str(), seq.c_str());
					seq = newSeq;
				}
			}
		}

		this->chat_sequence_num_ = seq;
	}
	else {
		// No type? This shouldn't happen unless there is a big API change.
		return handle_error("channel");
	}

	// Return
	switch (resp.code)
	{
	case HTTP_CODE_OK:
		return handle_success("channel");

	case HTTP_CODE_GATEWAY_TIMEOUT:
		// Maybe we have same clientid as other connected client, try to generate different one
		this->chat_clientid_ = utils::text::rand_string(8, "0123456789abcdef", &this->random_);

		// Intentionally fall to handle_error() below
	case HTTP_CODE_FAKE_DISCONNECTED:
	case HTTP_CODE_FAKE_ERROR:
	default:
		return handle_error("channel");
	}
}

bool facebook_client::activity_ping()
{
	// Don't send ping when we are not online
	if (parent->m_iStatus != ID_STATUS_ONLINE)
		return true;

	handle_entry("activity_ping");

	ChannelRequest *request = new ChannelRequest(this, ChannelRequest::PING);
	http::response resp = sendRequest(request);

	// Remember this last ping time
	parent->m_pingTS = ::time(NULL);

	if (resp.data.empty() || resp.data.find("\"t\":\"pong\"") == resp.data.npos) {
		// Something went wrong
		return handle_error("activity_ping");
	}

	return handle_success("activity_ping");
}

int facebook_client::send_message(int seqid, MCONTACT hContact, const std::string &message_text, std::string *error_text, const std::string &captcha_persist_data, const std::string &captcha)
{
	handle_entry("send_message");

	bool isChatRoom = parent->isChatRoom(hContact);

	ptrA userId( parent->getStringA(hContact, FACEBOOK_KEY_ID));
	ptrA threadId( parent->getStringA(hContact, FACEBOOK_KEY_TID));
	
	// Check if we have userId/threadId to be able to send message
	if ((isChatRoom && (threadId == NULL || !mir_strcmp(threadId, "null")))
		|| (!isChatRoom && (userId == NULL || !mir_strcmp(userId, "null")))) {
		// This shouldn't happen unless user manually deletes some data via Database Editor++
		*error_text = Translate("Contact doesn't have required data in database.");

		handle_error("send_message");
		return SEND_MESSAGE_ERROR;
	}

	// Probably we can generate any random messageID, it just have to be numeric and don't start with "0". We will receive it in response as "client_message_id".
	std::string messageId = utils::text::rand_string(10, "123456789", &this->random_);

	http::response resp;

	{
		HttpRequest *request = new SendMessageRequest(this, userId, threadId, messageId.c_str(), message_text.c_str(), isChatRoom, captcha.c_str(), captcha_persist_data.c_str());
		ScopedLock s(send_message_lock_);		
		resp = sendRequest(request);

		*error_text = resp.error_text;

		if (resp.error_number == 0) {
			// Everything is OK
			// Remember this message id
			std::string mid = utils::text::source_get_value(&resp.data, 2, "\"message_id\":\"", "\"");
			if (mid.empty())
				mid = utils::text::source_get_value(&resp.data, 2, "\"mid\":\"", "\""); // TODO: This is probably not used anymore

			// For classic contacts remember last message id
			if (!parent->isChatRoom(hContact))
				parent->setString(hContact, FACEBOOK_KEY_MESSAGE_ID, mid.c_str());

			// Get timestamp
			std::string timestamp = utils::text::source_get_value(&resp.data, 2, "\"timestamp\":", ",");
			time_t time = utils::time::from_string(timestamp);

			// Remember last action timestamp for history sync
			parent->setDword(FACEBOOK_KEY_LAST_ACTION_TS, time);

			// For classic conversation we try to replace timestamp of added event in OnPreCreateEvent()
			if (seqid > 0)
				messages_timestamp.insert(std::make_pair(seqid, time));

			// We have this message in database, so ignore further tries (in channel) to add it again
			messages_ignore.insert(std::make_pair(mid, 0));
		}
	}

	switch (resp.error_number) {
	case 0: 
		// Everything is OK
		break;

	// case 1356002: // You are offline (probably you can't use mercury or some other request when chat is offline)

	case 1356003: // Contact is offline
		parent->setWord(hContact, "Status", ID_STATUS_OFFLINE);
		return SEND_MESSAGE_ERROR;

	case 1356026: // Contact has alternative client
		client_notify(TranslateT("Need confirmation for sending messages to other clients.\nOpen Facebook website and try to send message to this contact again!"));
		return SEND_MESSAGE_ERROR;

	case 1357007: // Security check (captcha) is required
		{
			std::string imageUrl = utils::text::html_entities_decode(utils::text::slashu_to_utf8(utils::text::source_get_value(&resp.data, 3, "img class=\\\"img\\\"", "src=\\\"", "\\\"")));
			std::string captchaPersistData = utils::text::source_get_value(&resp.data, 3, "\\\"captcha_persist_data\\\"", "value=\\\"", "\\\"");

			parent->debugLogA("    Got imageUrl (first): %s", imageUrl.c_str());
			parent->debugLogA("    Got captchaPersistData (first): %s", captchaPersistData.c_str());

			HttpRequest *request = new RefreshCaptchaRequest(this, captchaPersistData.c_str());
			http::response capResp = sendRequest(request);

			if (capResp.code == HTTP_CODE_OK) {
				imageUrl = utils::text::html_entities_decode(utils::text::slashu_to_utf8(utils::text::source_get_value(&capResp.data, 3, "img class=\\\"img\\\"", "src=\\\"", "\\\"")));
				captchaPersistData = utils::text::source_get_value(&capResp.data, 3, "\\\"captcha_persist_data\\\"", "value=\\\"", "\\\"");

				parent->debugLogA("    Got imageUrl (second): %s", imageUrl.c_str());
				parent->debugLogA("    Got captchaPersistData (second): %s", captchaPersistData.c_str());

				std::string result;
				if (!parent->RunCaptchaForm(imageUrl, result)) {
					*error_text = Translate("User cancel captcha challenge.");
					return SEND_MESSAGE_CANCEL;
				}

				return send_message(seqid, hContact, message_text, error_text, captchaPersistData, result);
			}
		}
		return SEND_MESSAGE_CANCEL; // Cancel because we failed to load captcha image so we can't continue only with error

	//case 1404123: // Blocked sending messages (with URLs) because Facebook think our computer is infected with malware

	default: // Other error
		parent->debugLogA("!!! Send message error #%d: %s", resp.error_number, resp.error_text.c_str());
		return SEND_MESSAGE_ERROR;
	}

	switch (resp.code) {
	case HTTP_CODE_OK:
		handle_success("send_message");
		return SEND_MESSAGE_OK;

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		*error_text = Translate("Timeout when sending message.");

		handle_error("send_message");
		return SEND_MESSAGE_ERROR;
	}
}

bool facebook_client::post_status(status_data *status)
{
	if (status == NULL || (status->text.empty() && status->url.empty()))
		return false;

	handle_entry("post_status");

	if (status->isPage) {
		// Switch to page identity by which name we will share this post
		sendRequest(new SwitchIdentityRequest(this->dtsg_.c_str(), status->user_id.c_str()));
	}

	std::string linkData;
	if (!status->url.empty()) {
		HttpRequest *request = new LinkScraperRequest(this, status);
		http::response resp = sendRequest(request);

		std::string temp = utils::text::html_entities_decode(utils::text::slashu_to_utf8(resp.data));
		std::string form = utils::text::source_get_value(&temp, 2, "<form", "</form>");
		utils::text::replace_all(&form, "\\\"", "\"");
		linkData += utils::text::source_get_form_data(&form);		
		// FIXME: Rework to some "scraped_link" structure to simplify working with it?
	}

	HttpRequest *request = new SharePostRequest(this, status, linkData.c_str());
	http::response resp = sendRequest(request);

	if (status->isPage) {
		// Switch back to our identity
		sendRequest(new SwitchIdentityRequest(this->dtsg_.c_str(), this->self_.user_id.c_str()));
	}

	// cleanup status elements (delete users)
	for (std::vector<facebook_user*>::size_type i = 0; i < status->users.size(); i++) {
		delete status->users[i];
	}
	status->users.clear();

	if (resp.isValid()) {
		parent->NotifyEvent(parent->m_tszUserName, TranslateT("Status update was successful."), NULL, EVENT_OTHER);
		return handle_success("post_status");
	}

	return handle_error("post_status");
}

//////////////////////////////////////////////////////////////////////////////

bool facebook_client::save_url(const std::string &url, const std::wstring &filename, HNETLIBCONN &nlc)
{
	NETLIBHTTPREQUEST req = { sizeof(req) };
	NETLIBHTTPREQUEST *resp;
	req.requestType = REQUEST_GET;
	req.szUrl = const_cast<char*>(url.c_str());
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_PERSISTENT | NLHRF_NODUMP;
	req.nlc = nlc;

	resp = Netlib_HttpTransaction(handle_, &req);

	bool ret = false;

	if (resp) {
		nlc = resp->nlc;
		parent->debugLogA("@@@ Saving URL %s to file %s", url.c_str(), _T2A(filename.c_str()));

		// Create folder if necessary
		std::wstring dir = filename.substr(0, filename.rfind('\\'));
		if (_waccess(dir.c_str(), 0))
			CreateDirectoryTreeW(dir.c_str());

		// Write to file
		FILE *f = _wfopen(filename.c_str(), L"wb");
		if (f != NULL) {
			fwrite(resp->pData, 1, resp->dataLength, f);
			fclose(f);

			ret = _waccess(filename.c_str(), 0) == 0;
		}

		Netlib_FreeHttpRequest(resp);
	}
	else {
		nlc = NULL;
	}

	return ret;
}

bool facebook_client::sms_code(const char *fb_dtsg)
{
	http::response resp = sendRequest(new LoginSmsRequest(this, fb_dtsg));

	if (resp.data.find("\"is_valid\":true", 0) == std::string::npos) {
		// Code wasn't send
		client_notify(TranslateT("Error occurred when requesting verification SMS code."));
		return false;
	}

	parent->NotifyEvent(parent->m_tszUserName, TranslateT("Verification SMS code was sent to your mobile phone."), NULL, EVENT_OTHER);
	return true;
}