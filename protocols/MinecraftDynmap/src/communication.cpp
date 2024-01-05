/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015-17 Robert Pösel, 2017-23 Miranda NG team

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

MHttpResponse* MinecraftDynmapProto::sendRequest(const int request_type, std::string *post_data, std::string *get_data)
{
	// Prepare the request
	MHttpRequest nlhr;

	// FIXME: get server

	// Set request URL
	std::string url = m_server + chooseAction(request_type, get_data);
	nlhr.m_szUrl = url.c_str();

	// Set timeout (bigger for channel request)
	nlhr.timeout = 1000 * ((request_type == MINECRAFTDYNMAP_REQUEST_EVENTS) ? 65 : 20);

	// Set request type (GET/POST) and eventually also POST data
	if (post_data != nullptr) {
		nlhr.requestType = REQUEST_POST;
		nlhr.SetData(post_data->c_str(), post_data->length());
	}
	else nlhr.requestType = REQUEST_GET;

	// Set headers - it depends on requestType so it must be after setting that
	if (request_type == REQUEST_POST)
		nlhr.AddHeader("Content-Type", "application/json; charset=utf-8");
	
	nlhr.AddHeader("Cookie", m_cookie.c_str());
	nlhr.AddHeader("User-Agent", g_strUserAgent.c_str());
	nlhr.AddHeader("Accept", "*/*");
	nlhr.AddHeader("Accept-Language", "en,en-US;q=0.9");

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
		nlhr.nlc = nullptr;
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

	debugLogA("@@@@@ Sending request to '%s'", nlhr.m_szUrl.c_str());

	// Send the request	
	auto *pnlhr = Netlib_HttpTransaction(m_hNetlibUser, &nlhr);

	// Remember the persistent connection handle (or not)
	switch (request_type) {
	case MINECRAFTDYNMAP_REQUEST_HOME:
		break;

	case MINECRAFTDYNMAP_REQUEST_EVENTS:
		hEventsConnection = pnlhr ? pnlhr->nlc : nullptr;
		break;

	default:
		ReleaseMutex(connection_lock_);
		hConnection = pnlhr ? pnlhr->nlc : nullptr;
		break;
	}

	return pnlhr;
}

//////////////////////////////////////////////////////////////////////////////

std::string MinecraftDynmapProto::chooseAction(int request_type, std::string *get_data)
{
	switch (request_type) {
		case MINECRAFTDYNMAP_REQUEST_MESSAGE:
			return "/up/sendmessage";

		case MINECRAFTDYNMAP_REQUEST_CONFIGURATION:
			return "/up/configuration";

		case MINECRAFTDYNMAP_REQUEST_EVENTS:
			return std::string("/up/world/world/") + (!m_timestamp.empty() ? m_timestamp : "0");

		//case MINECRAFTDYNMAP_REQUEST_HOME:
		default:
			return "/" + *get_data;
	}
}

//////////////////////////////////////////////////////////////////////////////

bool MinecraftDynmapProto::doSignOn()
{
	handleEntry(__FUNCTION__);

	NLHR_PTR resp(sendRequest(MINECRAFTDYNMAP_REQUEST_CONFIGURATION));
	if (!resp || resp->resultCode != HTTP_CODE_OK)
		return handleError(__FUNCTION__, "Can't load configuration", true);

	JSONNode root = JSONNode::parse(resp->body);
	if (!root)
		return false;

	/*
	const JSONNode &allowchat_ = root["allowchat"]; // boolean
	const JSONNode &allowwebchat_ = root["allowwebchat"]; // boolean
	const JSONNode &loggedin_ = root["loggedin"]; // boolean
	const JSONNode &loginEnabled_ = root["login-enabled"]; // boolean
	const JSONNode &loginRequired_ = root["webchat-requires-login"]; // boolean
	*/

	const JSONNode &title_ = root["title"]; // name of server
	const JSONNode &interval_ = root["webchat-interval"]; // limit in seconds for sending messages
	const JSONNode &rate_ = root["updaterate"]; // probably update rate for events request

	if (!title_ || !interval_ || !rate_) {
		return handleError(__FUNCTION__, "No title, interval or rate in configuration", true);
	}

	m_title = title_.as_string();
	m_interval = interval_.as_int();
	m_updateRate = rate_.as_int();
	m_cookie.clear();

	if (auto *pszCookie = resp->FindHeader("Set-Cookie")) {
		m_cookie = pszCookie;

		// Load Session identifier
		const char *findStr = "JSESSIONID=";
		std::string::size_type start = m_cookie.find(findStr);
		if (start != std::string::npos)
			m_cookie = m_cookie.substr(start, m_cookie.find(";") - start);
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
	NLHR_PTR resp(sendRequest(MINECRAFTDYNMAP_REQUEST_EVENTS));
	if (!resp || resp->resultCode != HTTP_CODE_OK)
		return handleError(__FUNCTION__, "Response is not code 200");

	JSONNode root = JSONNode::parse(resp->body);
	if (!root)
		return handleError(__FUNCTION__, "Invalid JSON response");

	const JSONNode &timestamp_ = root["timestamp"];
	if (!timestamp_)
		return handleError(__FUNCTION__, "Received no timestamp node");

	m_timestamp = timestamp_.as_string();

	const JSONNode &updates_ = root["updates"];
	if (!updates_)
		return handleError(__FUNCTION__, "Received no updates node");

	for (auto it = updates_.begin(); it != updates_.end(); ++it) {
		const JSONNode &type_ = (*it)["type"];
		if (type_ && type_.as_string() == "chat") {
			const JSONNode &time_ = (*it)["timestamp"];
			// const JSONNode &source_ = (*it)["source"]; // e.g. "web"
			const JSONNode &playerName_ = (*it)["playerName"];
			const JSONNode &message_ = (*it)["message"];
			// TODO: there are also "channel" and "account" elements

			if (!time_ || !playerName_ || !message_) {
				debugLogW(L"Error: No player name, time or text for message");
				continue;
			}

			time_t timestamp = atoi(time_.as_string().c_str());
			std::string name = playerName_.as_string();
			std::string message = message_.as_string();

			debugLogW(L"Received message: [%d] %s -> %s", timestamp, name.c_str(), message.c_str());
			UpdateChat(name.c_str(), message.c_str(), timestamp);
		}
	}

	return handleSuccess(__FUNCTION__);
}

bool MinecraftDynmapProto::doSendMessage(const std::string &message_text)
{
	handleEntry(__FUNCTION__);

	JSONNode json(JSON_NODE);
	json.push_back(JSONNode("name", m_nick.c_str()));
	json.push_back(JSONNode("message", message_text.c_str()));
	std::string data = json.write();

	NLHR_PTR resp(sendRequest(MINECRAFTDYNMAP_REQUEST_MESSAGE, &data));
	if (resp && resp->resultCode == HTTP_CODE_OK) {
		JSONNode root = JSONNode::parse(resp->body);
		if (root) {
			const JSONNode &error_ = root["error"];
			if (error_) {
				std::string error = error_.as_string();
				if (error == "none") {
					return handleSuccess(__FUNCTION__);
				}
				else if (error == "not-allowed") {
					UpdateChat(nullptr, Translate("Message was not sent. Probably you are sending them too fast or chat is disabled completely."));
				}
			}
		}
	}

	return handleError(__FUNCTION__);
}

std::string MinecraftDynmapProto::doGetPage(const int request_type)
{
	handleEntry(__FUNCTION__);

	NLHR_PTR resp(sendRequest(request_type));
	if (resp && resp->resultCode == HTTP_CODE_OK) {
		handleSuccess(__FUNCTION__);
		return resp->body.c_str();
	}

	handleError(__FUNCTION__);
	return "";
}

void MinecraftDynmapProto::SignOnWorker(void*)
{
	SYSTEMTIME t;
	GetLocalTime(&t);
	debugLogA("[%d.%d.%d] Using Omegle Protocol %s", t.wDay, t.wMonth, t.wYear, __VERSION_STRING_DOTS);

	ScopedLock s(signon_lock_);

	int old_status = m_iStatus;

	// Load server from database
	ptrA str(db_get_sa(0, m_szModuleName, MINECRAFTDYNMAP_KEY_SERVER));
	if (!str || !str[0]) {
		MessageBox(nullptr, TranslateT("Set server address to connect."), m_tszUserName, MB_OK);
		SetStatus(ID_STATUS_OFFLINE);
		return;
	}
	m_server = str;

	// Fix format of given server
	if (m_server.substr(0, 7) != "http://" && m_server.substr(0, 8) != "https://")
		m_server = "http://" + m_server;
	if (m_server.substr(m_server.length() - 1, 1) == "/")
		m_server = m_server.substr(0, m_server.length() -1);

	if (doSignOn()) {
		// Signed in, switch to online, create chatroom and start events loop
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		setDword("LogonTS", (uint32_t)time(0));
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

	if (hConnection)
		Netlib_CloseHandle(hConnection);
	hConnection = nullptr;

	if (hEventsConnection)
		Netlib_CloseHandle(hEventsConnection);
	hEventsConnection = nullptr;
}

void MinecraftDynmapProto::EventsLoop(void *)
{
	ScopedLock s(events_loop_lock_);

	time_t tim = ::time(0);
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
	if (p == nullptr)
		return;

	ScopedLock s(send_message_lock_);

	CMStringA data = ((std::string*)p)->c_str();
	delete (std::string*)p;

	data.TrimRight();

	if (isOnline() && data.GetLength())
		doSendMessage(data.c_str());
}
