/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-21 Miranda NG team

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

http::response Omegle_client::flap(const int request_type, std::string *post_data, std::string *get_data)
{
	http::response resp;

	// Prepare the request
	NETLIBHTTPREQUEST nlhr = { sizeof(NETLIBHTTPREQUEST) };

	// Set request URL
	std::string url = choose_server(request_type) + choose_action(request_type, get_data);
	nlhr.szUrl = (char*)url.c_str();

	// Set timeout (bigger for channel request)
	nlhr.timeout = 1000 * ((request_type == OMEGLE_REQUEST_EVENTS) ? 65 : 20);

	// Set request type (GET/POST) and eventually also POST data
	if (post_data != nullptr) {
		nlhr.requestType = REQUEST_POST;
		nlhr.pData = (char*)(*post_data).c_str();
		nlhr.dataLength = (int)post_data->length();
	}
	else {
		nlhr.requestType = REQUEST_GET;
	}

	// Set headers - it depends on requestType so it must be after setting that
	nlhr.headers = get_request_headers(nlhr.requestType, &nlhr.headersCount);

	// Set flags
	nlhr.flags = NLHRF_HTTP11;

#ifdef _DEBUG 
	nlhr.flags |= NLHRF_DUMPASTEXT;
#else
	nlhr.flags |= NLHRF_NODUMP;
#endif	

	// Set persistent connection (or not)
	switch (request_type)
	{
	case OMEGLE_REQUEST_HOME:
		nlhr.nlc = nullptr;
		break;

	case OMEGLE_REQUEST_EVENTS:
		nlhr.nlc = hEventsConnection;
		nlhr.flags |= NLHRF_PERSISTENT;
		break;

	default:
		WaitForSingleObject(connection_lock_, INFINITE);
		nlhr.nlc = hConnection;
		nlhr.flags |= NLHRF_PERSISTENT;
		break;
	}

	parent->debugLogA("@@@@@ Sending request to '%s'", nlhr.szUrl);

	// Send the request	
	NLHR_PTR pnlhr(Netlib_HttpTransaction(handle_, &nlhr));

	mir_free(nlhr.headers);

	// Remember the persistent connection handle (or not)
	switch (request_type)
	{
	case OMEGLE_REQUEST_HOME:
		break;

	case OMEGLE_REQUEST_EVENTS:
		hEventsConnection = pnlhr ? pnlhr->nlc : nullptr;
		break;

	default:
		ReleaseMutex(connection_lock_);
		hConnection = pnlhr ? pnlhr->nlc : nullptr;
		break;
	}

	// Check and copy response data
	if (pnlhr != nullptr)
	{
		parent->debugLogA("@@@@@ Got response with code %d", pnlhr->resultCode);
		store_headers(&resp, pnlhr->headers, pnlhr->headersCount);
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		// This error code gives us some error page, but we don't need that, so clear the response
		if (pnlhr->resultCode == HTTP_CODE_WEB_SERVER_IS_DOWN) {
			resp.data = "";
		}

		parent->debugLogA("&&&&& Got response: %s", resp.data.c_str());
	}
	else {
		parent->debugLogA("!!!!! No response from server (time-out)");
		resp.code = HTTP_CODE_FAKE_DISCONNECTED;
		// Better to have something set explicitely as this value is compaired in all communication requests
	}

	return resp;
}

bool Omegle_client::handle_entry(const std::string &method)
{
	parent->debugLogA("   >> Entering %s()", method.c_str());
	return true;
}

bool Omegle_client::handle_success(const std::string &method)
{
	parent->debugLogA("   << Quitting %s()", method.c_str());
	reset_error();
	return true;
}

bool Omegle_client::handle_error(const std::string &method, bool force_disconnect)
{
	bool result;
	increment_error();
	parent->debugLogA("!!!!! %s(): Something with Omegle went wrong", method.c_str());

	if (force_disconnect)
		result = false;
	else if (error_count_ <= (UINT)db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_TIMEOUTS_LIMIT, OMEGLE_TIMEOUTS_LIMIT))
		result = true;
	else
		result = false;

	if (result == false)
	{
		reset_error();
		parent->UpdateChat(nullptr, TranslateT("Connection error."));
		parent->StopChat(false);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

std::string Omegle_client::get_server(bool not_last)
{
	int q = not_last ? 1 : 0;

	int server = db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_SERVER, 0);
	if (server < 0 || server >= (int)(_countof(servers) - q))
		server = 0;

	if (server == 0) {
		srand(::time(0));
		server = (rand() % (_countof(servers) - 1 - q)) + 1;
	}

	return servers[server];
}

std::string Omegle_client::get_language()
{
	int language = db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_LANGUAGE, 0);
	if (language < 0 || language >= (_countof(languages)))
		language = 0;

	return language > 0 ? languages[language].id : "en";
}

std::string Omegle_client::choose_server(int request_type)
{
	switch (request_type)
	{
	case OMEGLE_REQUEST_HOME:
		return OMEGLE_SERVER_REGULAR;

		/*	case OMEGLE_REQUEST_START:
			case OMEGLE_REQUEST_STOP:
			case OMEGLE_REQUEST_SEND:
			case OMEGLE_REQUEST_EVENTS:
			case OMEGLE_REQUEST_TYPING_START:
			case OMEGLE_REQUEST_TYPING_STOP:
			case OMEGLE_REQUEST_RECAPTCHA:
			case OMEGLE_REQUEST_COUNT:
			*/	default:
				std::string server = OMEGLE_SERVER_CHAT;
				utils::text::replace_first(&server, "%s", this->server_);
				return server;
	}
}

std::string Omegle_client::choose_action(int request_type, std::string* get_data)
{
	switch (request_type)
	{
	case OMEGLE_REQUEST_START:
	{
		std::string action = "/start?caps=recaptcha2,t&rcs=1&spid=&lang=";
		action += get_language();
		if (get_data != nullptr)
			action += (*get_data);

		return action;
	}

	case OMEGLE_REQUEST_STOP:
		return "/disconnect";

	case OMEGLE_REQUEST_SEND:
		return "/send";

	case OMEGLE_REQUEST_EVENTS:
		return "/events";

	case OMEGLE_REQUEST_TYPING_START:
		return "/typing";

	case OMEGLE_REQUEST_TYPING_STOP:
		return "/stoppedtyping";

	case OMEGLE_REQUEST_RECAPTCHA:
		return "/recaptcha";

	case OMEGLE_REQUEST_COUNT:
		return "/count";

		// "/stoplookingforcommonlikes"

		/*	case OMEGLE_REQUEST_HOME:
		*/	default:
			return "/";
	}
}


NETLIBHTTPHEADER* Omegle_client::get_request_headers(int request_type, int* headers_count)
{
	if (request_type == REQUEST_POST)
		*headers_count = 4;
	else
		*headers_count = 3;

	NETLIBHTTPHEADER *headers = (NETLIBHTTPHEADER*)mir_calloc(sizeof(NETLIBHTTPHEADER)*(*headers_count));

	if (request_type == REQUEST_POST) {
		headers[3].szName = "Content-Type";
		headers[3].szValue = "application/x-www-form-urlencoded; charset=utf-8";
	}

	headers[2].szName = "User-Agent";
	headers[2].szValue = Netlib_GetUserAgent();
	headers[1].szName = "Accept";
	headers[1].szValue = "*/*";
	headers[0].szName = "Accept-Language";
	headers[0].szValue = "en,en-US;q=0.9";

	return headers;
}

void Omegle_client::store_headers(http::response* resp, NETLIBHTTPHEADER* headers, int headersCount)
{
	for (size_t i = 0; i < (size_t)headersCount; i++)
	{
		std::string header_name = headers[i].szName;
		std::string header_value = headers[i].szValue;

		// TODO RM: (un)comment
		//parent->debugLogA("----- Got header '%s': %s", header_name.c_str(), header_value.c_str());
		resp->headers[header_name] = header_value;
	}
}

//////////////////////////////////////////////////////////////////////////////

bool Omegle_client::start()
{
	HANDLE_ENTRY;

	this->server_ = get_server();
	//parent->debugLogA("Chosing server %s", this->server_.c_str());
	//std::string log = Translate("Chosing server: ") + this->server_;
	//parent->UpdateChat(NULL, log.c_str());

	std::string data;

	if (this->spy_mode_) {
		//// get last server from list, which is for spy mode
		//this->server_ = servers[_countof(servers)-1];

		if (this->question_.empty()) {
			data = "&wantsspy=1";
		}
		else {
			data = "&ask=" + utils::url::encode(this->question_);
			data += "&cansavequestion=";
			data += db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_REUSE_QUESTION, 0) ? "1" : "0";
		}
	}
	else if (db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_MEET_COMMON, 0))
	{
		DBVARIANT dbv;
		if (!db_get_utf(NULL, parent->m_szModuleName, OMEGLE_KEY_INTERESTS, &dbv))
		{
			std::string topics = dbv.pszVal;
			std::string topic;

			db_free(&dbv);

			std::string::size_type pos = 0;
			std::string::size_type pos2 = 0;
			while ((pos2 = topics.find(",", pos)) != std::string::npos) {
				topic = topics.substr(pos, pos2 - pos);
				topic = utils::text::trim(topic);

				if (!topic.empty()) {
					if (pos > 0)
						data += ",";

					data += "\"" + topic + "\"";
				}

				pos = pos2 + 1;
			}

			topic = topics.substr(pos);
			topic = utils::text::trim(topic);
			if (!topic.empty()) {
				if (pos > 0)
					data += ",";
				data += "\"" + topic + "\"";
			}

			parent->debugLogA("TOPICS: %s", data.c_str());

			if (!data.empty()) {
				data = "[" + data + "]";
				data = "&topics=" + utils::url::encode(data);
			}

			//// get any server but last, which is for spy mode
			//this->server_ = get_server(true);
		}
	}

	if (db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_SERVER_INFO, 0))
	{
		std::string count = get_page(OMEGLE_REQUEST_COUNT);
		if (!count.empty()) {
			char str[255];
			mir_snprintf(str, Translate("Connected to server %s. There are %s users online now."), server_.c_str(), count.c_str());

			wchar_t *msg = mir_a2u(str);
			parent->UpdateChat(nullptr, msg);
			mir_free(msg);
		}
	}
	else {
		char str[255];
		mir_snprintf(str, Translate("Connected to server %s."), server_.c_str());

		wchar_t *msg = mir_a2u(str);
		parent->UpdateChat(nullptr, msg);
		mir_free(msg);
	}

	// Send validation
	http::response resp = flap(OMEGLE_REQUEST_START, nullptr, &data);

	switch (resp.code)
	{
	case HTTP_CODE_FAKE_DISCONNECTED:
	{
		// If is is only timeout error, try login once more
		if (HANDLE_ERROR(false))
			return start();
		else
			return false;
	}

	case HTTP_CODE_OK:
	{
		if (!resp.data.empty()) {
			this->chat_id_ = resp.data.substr(1, resp.data.length() - 2);
			this->state_ = STATE_WAITING;

			return HANDLE_SUCCESS;
		}
		else {
			return HANDLE_ERROR(FORCE_DISCONNECT);
		}
	}

	default:
		return HANDLE_ERROR(FORCE_DISCONNECT);
	}
}

bool Omegle_client::stop()
{
	if (parent->isOffline())
		return true;

	HANDLE_ENTRY;

	std::string data = "id=" + this->chat_id_;

	http::response resp = flap(OMEGLE_REQUEST_STOP, &data);

	if (hConnection)
		Netlib_CloseHandle(hConnection);
	hConnection = nullptr;

	if (hEventsConnection)
		Netlib_CloseHandle(hEventsConnection);
	hEventsConnection = nullptr;

	if (resp.data == "win") {
		return HANDLE_SUCCESS;
	}
	else {
		return HANDLE_ERROR(false);
	}

	/*	switch ( resp.code )
		{
		case HTTP_CODE_OK:
		case HTTP_CODE_FOUND:

		default:

		}*/
}

bool Omegle_client::events()
{
	HANDLE_ENTRY;

	std::string post_data = "id=" + this->chat_id_;

	// Get update
	http::response resp = flap(OMEGLE_REQUEST_EVENTS, &post_data);

	// Return
	switch (resp.code)
	{
	case HTTP_CODE_OK:
	{
		if (resp.data == "null") {
			// Everything is OK, no new message received -- OR it is a problem
			// TODO: if we are waiting for Stranger with common likes, then we should try standard Stranger if this takes too long
			return HANDLE_ERROR(false);
		}
		else if (resp.data == "fail") {
			// Something went wrong
			return HANDLE_ERROR(false);
		}

		JSONROOT root(resp.data.c_str());
		if (root == nullptr)
			return HANDLE_ERROR(false);

		bool newStranger = false;
		bool waiting = false;

		for (size_t i = 0; i < json_size(root); i++) {
			JSONNode *item = json_at(root, i);
			if (item == nullptr)
				continue;

			std::string name = _T2A(json_as_string(json_at(item, 0)));
		
			if (name == "waiting") {
				// We are just waiting for new Stranger
				waiting = true;
			}
			else if (name == "identDigests") {
				// We get some comma separated hashes, I'm not sure what for
			}
			else if (name == "statusInfo") {
				JSONNode *data = json_at(item, 1);

				// We got some object as second parameter
				//data["antinudepercent"]; // probably 1 by default
				//data["antinudeservers"]; // array of server names, like "waw3.omegle.com"
				//data["rtmfp"]; // some rtmfp protocol address
				//data["servers"]; // array of server names, like "front5.omegle.com"
				//data["spyeeQueueTime"]; // some float number, e.g. 0.0701999903
				//data["spyQueueTime"]; // some float number, e.g. 4.7505000114
				//data["timestamp"]; // e.g. 1445336566.0196209

				// We got info about count of connected people there
				ptrW count(json_as_string(json_get(data, "count")));
				wchar_t strT[255];
				mir_snwprintf(strT, TranslateT("On whole Omegle are %s strangers online now."), count.get());

				parent->UpdateChat(nullptr, strT);
			}
			else if (name == "serverMessage") {
				ptrW message(json_as_string(json_at(item, 1)));
				parent->UpdateChat(nullptr, TranslateW(message));
			}
			else if (name == "connected") {
				// Stranger connected
				if (this->spy_mode_ && !this->question_.empty()) {
					parent->AddChatContact(TranslateT("Stranger 1"));
					parent->AddChatContact(TranslateT("Stranger 2"));
					this->state_ = STATE_SPY;
				}
				else {
					parent->AddChatContact(TranslateT("Stranger"));
					this->state_ = STATE_ACTIVE;
				}

				newStranger = true;
				waiting = false;
			}
			else if (name == "commonLikes") {
				std::wstring likes = TranslateT("You and the Stranger both like: ");

				JSONNode *items = json_at(item, 1);
				size_t size = json_size(items);
				for (size_t j = 0; j < size; j++) {
					likes += ptrW(json_as_string(json_at(items, j)));
					if (j < size - 1)
						likes += L", ";
				}
				
				parent->debugLogW(L"Got common likes: '%s'", likes.c_str());
				parent->SetTopic(likes.c_str());
			}
			else if (name == "question") {
				ptrW question(json_as_string(json_at(item, 1)));
				parent->SetTopic(question);
			}
			else if (name == "typing" || name == "spyTyping") {
				// Stranger is typing, not supported by chat module yet
				Skin_PlaySound("StrangerTyp");

				ptrW who(name == "spyTyping" ? json_as_string(json_at(item, 1)) : mir_wstrdup(L"Stranger"));
				Srmm_SetStatusText(parent->GetChatHandle(), CMStringW(FORMAT, TranslateT("%s is typing."), TranslateW(who)), g_plugin.getIcon(IDI_TYPING_ON));
			}
			else if (name == "stoppedTyping" || name == "spyStoppedTyping") {
				// Stranger stopped typing, not supported by chat module yet
				Skin_PlaySound("StrangerTypStop");

				ptrW who(name == "spyTyping" ? json_as_string(json_at(item, 1)) : mir_wstrdup(L"Stranger"));
				Srmm_SetStatusText(parent->GetChatHandle(), CMStringW(FORMAT, TranslateT("%s stopped typing."), TranslateW(who)), g_plugin.getIcon(IDI_TYPING_OFF));
			}
			else if (name == "gotMessage") {
				Srmm_SetStatusText(parent->GetChatHandle(), nullptr);

				// Play sound as we received message
				Skin_PlaySound("StrangerMessage");

				if (state_ == STATE_ACTIVE) {
					ptrW msg(json_as_string(json_at(item, 1)));
					parent->UpdateChat(TranslateT("Stranger"), msg);
				}
			}
			else if (name == "spyMessage") {
				Srmm_SetStatusText(parent->GetChatHandle(), nullptr);

				// Play sound as we received message
				Skin_PlaySound("StrangerMessage");

				if (state_ == STATE_SPY) {
					ptrW stranger(json_as_string(json_at(item, 1)));
					ptrW msg(json_as_string(json_at(item, 2)));
					parent->UpdateChat(stranger, msg);
				}
			}
			else if (name == "strangerDisconnected") {
				Srmm_SetStatusText(parent->GetChatHandle(), nullptr);

				// Stranger disconnected
				if (db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_DONT_STOP, 0))
				{
					Skin_PlaySound("StrangerChange");
					parent->NewChat();
				}
				else
					parent->StopChat(false);
			}
			else if (name == "spyDisconnected") {
				Srmm_SetStatusText(parent->GetChatHandle(), nullptr);

				ptrW stranger(json_as_string(json_at(item, 1)));

				wchar_t strT[255];
				mir_snwprintf(strT, TranslateT("%s disconnected."), TranslateW(stranger));
				parent->UpdateChat(nullptr, strT);

				// Stranger disconnected
				if (db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_DONT_STOP, 0))
				{
					Skin_PlaySound("StrangerChange");
					parent->NewChat();
				}
				else
					parent->StopChat(false);
			}
			else if (name == "recaptchaRequired") {
				// Nothing to do with recaptcha
				parent->UpdateChat(nullptr, TranslateT("Recaptcha is required.\nOpen http://omegle.com , solve Recaptcha and try again."));
				parent->StopChat(false);
			}
			else if (name == "recaptchaRejected") {
				// Nothing to do with recaptcha
				parent->StopChat(false);
			}
			else if (name == "error") {
				ptrW error(json_as_string(json_at(item, 1)));

				wchar_t strT[255];
				mir_snwprintf(strT, TranslateT("Error: %s"), TranslateW(error));
				parent->UpdateChat(nullptr, strT);
			}
		}
		
		if (newStranger && !spy_mode_) {
			// We got new stranger in this event, lets say him "Hi message" if enabled			
			if (db_get_b(0, parent->m_szModuleName, OMEGLE_KEY_HI_ENABLED, 0)) {
				DBVARIANT dbv;
				if (!db_get_utf(NULL, parent->m_szModuleName, OMEGLE_KEY_HI, &dbv)) {
					std::vector<std::string> messages;
					utils::text::explode(std::string(dbv.pszVal), "\r\n", &messages);
					db_free(&dbv);

					int pos = rand() % messages.size();
					std::string *message = new std::string(messages.at(pos));

					parent->debugLogA("**Chat - saying Hi! message");
					parent->ForkThread(&OmegleProto::SendMsgWorker, message);
				}
				else parent->debugLogA("**Chat - Hi message is enabled but not used");
			}
		}

		if (waiting) {
			// If we are only waiting in this event...
			parent->UpdateChat(nullptr, TranslateT("We are still waiting..."));
		}

		return HANDLE_SUCCESS;
	}

	case HTTP_CODE_FAKE_DISCONNECTED:
		// timeout
		return HANDLE_SUCCESS;

	default:
		return HANDLE_ERROR(false);
	}
}

bool Omegle_client::send_message(const std::string &message_text)
{
	HANDLE_ENTRY;

	std::string data = "msg=" + utils::url::encode(message_text);
	data += "&id=" + this->chat_id_;

	http::response resp = flap(OMEGLE_REQUEST_SEND, &data);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
		if (resp.data == "win") {
			return HANDLE_SUCCESS;
		}

	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return HANDLE_ERROR(false);
	}
}

bool Omegle_client::typing_start()
{
	HANDLE_ENTRY;

	std::string data = "id=" + this->chat_id_;

	http::response resp = flap(OMEGLE_REQUEST_TYPING_START, &data);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
		if (resp.data == "win") {
			return HANDLE_SUCCESS;
		}

	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return HANDLE_ERROR(false);
	}
}

bool Omegle_client::typing_stop()
{
	HANDLE_ENTRY;

	std::string data = "id=" + this->chat_id_;

	http::response resp = flap(OMEGLE_REQUEST_TYPING_STOP, &data);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
		if (resp.data == "win") {
			return HANDLE_SUCCESS;
		}

	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return HANDLE_ERROR(false);
	}
}

bool Omegle_client::recaptcha()
{
	// TODO: Implement!

	HANDLE_ENTRY;

	// data:{id:this.clientID,challenge:b,response:a}}
	//std::string data = "?id=...&challenge= ..., &response= ...";

	http::response resp = flap(OMEGLE_REQUEST_RECAPTCHA);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
		/*		if (resp.data == "win") {
					return handle_success( "typing_start" );
					}*/

	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return HANDLE_ERROR(false);
	}
}

std::string Omegle_client::get_page(const int request_type)
{
	HANDLE_ENTRY;

	http::response resp = flap(request_type);

	switch (resp.code)
	{
	case HTTP_CODE_OK:
		HANDLE_SUCCESS;
		break;

	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		HANDLE_ERROR(false);
	}

	return resp.data;
}
