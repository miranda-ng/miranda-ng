/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015 Robert Pösel

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

http::response MinecraftDynmapProto::sendRequest(const int request_type, std::string *post_data, std::string *get_data)
{
	http::response resp;

	// Prepare the request
	NETLIBHTTPREQUEST nlhr = { sizeof(NETLIBHTTPREQUEST) };

	// FIXME: get server

	// Set request URL
	std::string url = m_server + chooseAction(request_type, get_data);
	nlhr.szUrl = (char*)url.c_str();

	// Set timeout (bigger for channel request)
	nlhr.timeout = 1000 * ((request_type == MINECRAFTDYNMAP_REQUEST_EVENTS) ? 65 : 20);

	// Set request type (GET/POST) and eventually also POST data
	if (post_data != NULL) {
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
	case MINECRAFTDYNMAP_REQUEST_HOME:
		nlhr.nlc = NULL;
		break;

	case MINECRAFTDYNMAP_REQUEST_EVENTS:
		nlhr.nlc = hEventsConnection;
		nlhr.flags |= NLHRF_PERSISTENT;
		break;

	default:
		WaitForSingleObject(connection_lock_, INFINITE);
		nlhr.nlc = hConnection;
		nlhr.flags |= NLHRF_PERSISTENT;
		break;
	}

	debugLogA("@@@@@ Sending request to '%s'", nlhr.szUrl);

	// Send the request	
	NETLIBHTTPREQUEST *pnlhr = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&nlhr);

	mir_free(nlhr.headers);

	// Remember the persistent connection handle (or not)
	switch (request_type)
	{
	case MINECRAFTDYNMAP_REQUEST_HOME:
		break;

	case MINECRAFTDYNMAP_REQUEST_EVENTS:
		hEventsConnection = pnlhr ? pnlhr->nlc : NULL;
		break;

	default:
		ReleaseMutex(connection_lock_);
		hConnection = pnlhr ? pnlhr->nlc : NULL;
		break;
	}

	// Check and copy response data
	if (pnlhr != NULL)
	{
		debugLogA("@@@@@ Got response with code %d", pnlhr->resultCode);
		store_headers(&resp, pnlhr->headers, pnlhr->headersCount);
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		// debugLogA("&&&&& Got response: %s", resp.data.c_str());

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	} else {
		debugLogA("!!!!! No response from server (time-out)");
		resp.code = HTTP_CODE_FAKE_DISCONNECTED;
		// Better to have something set explicitely as this value is compaired in all communication requests
	}

	return resp;
}

//////////////////////////////////////////////////////////////////////////////

std::string MinecraftDynmapProto::chooseAction(int request_type, std::string *get_data)
{
	switch (request_type) {
		case MINECRAFTDYNMAP_REQUEST_MESSAGE: {
			return "/up/sendmessage";
		}

		case MINECRAFTDYNMAP_REQUEST_CONFIGURATION: {
			return "/up/configuration";
		}

		case MINECRAFTDYNMAP_REQUEST_EVENTS: {
			std::string request = "/up/world/%s/%s";

			// Set world
			std::string world = "world"; // TODO: configurable world?
			utils::text::replace_first(&request, "%s", world);

			// Set timestamp
			utils::text::replace_first(&request, "%s", !m_timestamp.empty() ? m_timestamp : "0");

			return request;
		}

		//case MINECRAFTDYNMAP_REQUEST_HOME:
		default: {
			return "/" + *get_data;
		}
	}
}


NETLIBHTTPHEADER* MinecraftDynmapProto::get_request_headers(int request_type, int* headers_count)
{
	if (request_type == REQUEST_POST)
		*headers_count = 5;
	else
		*headers_count = 4;

	NETLIBHTTPHEADER *headers = (NETLIBHTTPHEADER*)mir_calloc(sizeof(NETLIBHTTPHEADER)*(*headers_count));

	if (request_type == REQUEST_POST) {
		headers[4].szName = "Content-Type";
		headers[4].szValue = "application/json; charset=utf-8";
	}

	headers[3].szName = "Cookie";
	headers[3].szValue = (char *)m_cookie.c_str();
	headers[2].szName = "User-Agent";
	headers[2].szValue = (char *)g_strUserAgent.c_str();
	headers[1].szName = "Accept";
	headers[1].szValue = "*/*";
	headers[0].szName = "Accept-Language";
	headers[0].szValue = "en,en-US;q=0.9";

	return headers;
}

void MinecraftDynmapProto::store_headers(http::response* resp, NETLIBHTTPHEADER* headers, int headersCount)
{
	for (size_t i = 0; i < (size_t)headersCount; i++) {
		std::string header_name = headers[i].szName;
		std::string header_value = headers[i].szValue;
		
		resp->headers[header_name] = header_value;
	}
}

//////////////////////////////////////////////////////////////////////////////

bool MinecraftDynmapProto::doSignOn()
{
	handleEntry(__FUNCTION__);

	http::response resp = sendRequest(MINECRAFTDYNMAP_REQUEST_CONFIGURATION);

	if (resp.code != HTTP_CODE_OK) {
		return handleError(__FUNCTION__, "Can't load configuration", true);
	}

	JSONROOT root(resp.data.c_str());
	if (root == NULL)
		return false;

	/*
	JSONNODE *allowchat_ = json_get(root, "allowchat"); // boolean
	JSONNODE *allowwebchat_ = json_get(root, "allowwebchat"); // boolean
	JSONNODE *loggedin_ = json_get(root, "loggedin"); // boolean
	JSONNODE *loginEnabled_ = json_get(root, "login-enabled"); // boolean
	JSONNODE *loginRequired_ = json_get(root, "webchat-requires-login"); // boolean
	*/

	JSONNODE *title_ = json_get(root, "title"); // name of server
	JSONNODE *interval_ = json_get(root, "webchat-interval"); // limit in seconds for sending messages
	JSONNODE *rate_ = json_get(root, "updaterate"); // probably update rate for events request

	if (title_ == NULL || interval_ == NULL || rate_ == NULL) {
		return handleError(__FUNCTION__, "No title, interval or rate in configuration", true);
	}

	m_title = json_as_pstring(title_);
	m_interval = json_as_int(interval_);
	m_updateRate = json_as_int(rate_);
	m_cookie.clear();

	if (resp.headers.find("Set-Cookie") != resp.headers.end()) {
		// Load Session identifier
		std::string cookies = resp.headers["Set-Cookie"];

		const char *findStr = "JSESSIONID=";
		std::string::size_type start = cookies.find(findStr);
		
		if (start != std::string::npos) {
			m_cookie = cookies.substr(start, cookies.find(";") - start);
		}
	}

	if (m_cookie.empty()) {
		return handleError(__FUNCTION__, "Empty session id", true);
	}

	return handleSuccess(__FUNCTION__);
}

bool MinecraftDynmapProto::doEvents()
{
	handleEntry(__FUNCTION__);

	// Get update
	http::response resp = sendRequest(MINECRAFTDYNMAP_REQUEST_EVENTS);

	if (resp.code != HTTP_CODE_OK)
		return handleError(__FUNCTION__, "Response is not code 200");

	JSONROOT root(resp.data.c_str());
	if (root == NULL)
		return handleError(__FUNCTION__, "Invalid JSON response");

	JSONNODE *timestamp_ = json_get(root, "timestamp");
	if (timestamp_ == NULL)
		return handleError(__FUNCTION__, "Received no timestamp node");

	m_timestamp = json_as_pstring(timestamp_);

	JSONNODE *updates_ = json_get(root, "updates");
	if (updates_ == NULL)
		return handleError(__FUNCTION__, "Received no updates node");

	for (unsigned int i = 0; i < json_size(updates_); i++) {
		JSONNODE *it = json_at(updates_, i);

		JSONNODE *type_ = json_get(it, "type");
		if (type_ != NULL && json_as_pstring(type_) == "chat") {
			JSONNODE *time_ = json_get(it, "timestamp");
			// JSONNODE *source_ = json_get(it, "source"); // e.g. "web"
			JSONNODE *playerName_ = json_get(it, "playerName");
			JSONNODE *message_ = json_get(it, "message");
			// TODO: there are also "channel" and "account" elements

			if (time_ == NULL || playerName_ == NULL || message_ == NULL) {
				debugLog(_T("Error: No player name, time or text for message"));
				continue;
			}

			time_t timestamp = utils::time::from_string(json_as_pstring(time_));
			ptrT name(json_as_string(playerName_));
			ptrT message(json_as_string(message_));

			debugLog(_T("Received message: [%d] %s -> %s"), timestamp, name, message);
			UpdateChat(name, message, timestamp);
		}
	}

	return handleSuccess(__FUNCTION__);
}

bool MinecraftDynmapProto::doSendMessage(const std::string &message_text)
{
	handleEntry(__FUNCTION__);

	std::string data = "{\"name\":\"";
	data += this->nick_;
	data += "\", \"message\" : \"";
	data += message_text;
	data += "\"}";

	http::response resp = sendRequest(MINECRAFTDYNMAP_REQUEST_MESSAGE, &data);

	if (resp.code == HTTP_CODE_OK) {
		JSONROOT root(resp.data.c_str());
		if (root != NULL) {
			JSONNODE *error_ = json_get(root, "error");
			if (error_ != NULL) {
				std::string error = json_as_pstring(error_);
				if (error == "none") {
					return handleSuccess(__FUNCTION__);
				}
				else if (error == "not-allowed") {
					UpdateChat(NULL, TranslateT("Message was not sent. Probably you are sending them too fast or chat is disabled completely."));
				}
			}
		}
	}

	return handleError(__FUNCTION__);
}

std::string MinecraftDynmapProto::doGetPage(const int request_type)
{
	handleEntry(__FUNCTION__);

	http::response resp = sendRequest(request_type);

	if (resp.code == HTTP_CODE_OK) {
		handleSuccess(__FUNCTION__);
	} else {
		handleError(__FUNCTION__);
	}

	return resp.data;
}

void MinecraftDynmapProto::SignOnWorker(void*)
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	debugLogA("[%d.%d.%d] Using Omegle Protocol %s", t.wDay, t.wMonth, t.wYear, __VERSION_STRING_DOTS);

	ScopedLock s(signon_lock_);

	int old_status = m_iStatus;

	// Load server from database
	m_server = ptrA(db_get_sa(NULL, m_szModuleName, MINECRAFTDYNMAP_KEY_SERVER));
	
	if (m_server.empty()) {
		MessageBox(NULL, TranslateT("Set server address to connect."), m_tszUserName, MB_OK);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}

	// Fix format of given server
	if (m_server.substr(0, 7) != "http://" && m_server.substr(0, 8) != "https://")
		m_server = "http://" + m_server;
	if (m_server.substr(m_server.length() - 1, 1) == "/")
		m_server = m_server.substr(0, m_server.length() -1);

	if (doSignOn()) {
		// Signed in, switch to online, create chatroom and start events loop
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		setDword("LogonTS", (DWORD)time(NULL));
		ClearChat();
		OnJoinChat(0, false);

		ResetEvent(events_loop_event_);

		ForkThread(&MinecraftDynmapProto::EventsLoop, this);
	}
	else {
		// Some error
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_FAILED, (HANDLE)old_status, m_iStatus);
	}

}

void MinecraftDynmapProto::SignOffWorker(void*)
{
	ScopedLock s(signon_lock_);

	SetEvent(events_loop_event_);

	m_cookie.clear();
	m_title.clear();
	m_server.clear();
	m_timestamp.clear();

	int old_status = m_iStatus;
	m_iStatus = ID_STATUS_OFFLINE;

	Netlib_Shutdown(hEventsConnection);

	OnLeaveChat(NULL, NULL);

	delSetting("LogonTS");

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	//SetAllContactStatuses(ID_STATUS_OFFLINE);
	//ToggleStatusMenuItems(false);

	if (hConnection)
		Netlib_CloseHandle(hConnection);
	hConnection = NULL;

	if (hEventsConnection)
		Netlib_CloseHandle(hEventsConnection);
	hEventsConnection = NULL;
}

void MinecraftDynmapProto::EventsLoop(void *)
{
	ScopedLock s(events_loop_lock_);

	time_t tim = ::time(NULL);
	debugLogA(">>>>> Entering %s[%d]", __FUNCTION__, tim);

	while (doEvents())
	{
		if (!isOnline())
			break;

		if (WaitForSingleObjectEx(events_loop_event_, m_updateRate, true) != WAIT_TIMEOUT) // FIXME: correct timeout
			break;

		debugLogA("***** %s[%d] refreshing...", __FUNCTION__, tim);
	}

	ResetEvent(events_loop_event_);
	ResetEvent(events_loop_lock_);	
	debugLogA("<<<<< Exiting %s[%d]", __FUNCTION__, tim);
}

void MinecraftDynmapProto::SendMsgWorker(void *p)
{
	if (p == NULL)
		return;

	ScopedLock s(send_message_lock_);

	std::string data = *(std::string*)p;
	delete (std::string*)p;

	data = utils::text::trim(data);

	if (isOnline() && data.length()) {
		doSendMessage(data);
	}
}
