/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-13 Robert Pösel

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

http::response Omegle_client::flap( const int request_type, std::string* request_data, std::string* get_data )
{
	NETLIBHTTPREQUEST nlhr = {sizeof( NETLIBHTTPREQUEST )};
	nlhr.requestType = choose_method( request_type );
	std::string url = choose_request_url( request_type, request_data, get_data );
	nlhr.szUrl = (char*)url.c_str( );
	nlhr.flags = NLHRF_HTTP11 | NLHRF_NODUMP;
	nlhr.headers = get_request_headers( request_type, &nlhr.headersCount );
	nlhr.timeout = 1000 * (( request_type == OMEGLE_REQUEST_EVENTS ) ? 60 : 15);

	if ( request_data != NULL )
	{
		nlhr.pData = (char*)(*request_data).c_str();
		nlhr.dataLength = (int)request_data->length( );
	}

	parent->debugLogA("@@@@@ Sending request to '%s'", nlhr.szUrl);

	switch ( request_type )
	{
	case OMEGLE_REQUEST_HOME:
		nlhr.nlc = NULL;
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

	NETLIBHTTPREQUEST* pnlhr = ( NETLIBHTTPREQUEST* )CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)handle_, (LPARAM)&nlhr );

	http::response resp;

	switch ( request_type )
	{
	case OMEGLE_REQUEST_HOME:
		break;

	case OMEGLE_REQUEST_EVENTS:
		hEventsConnection = pnlhr ? pnlhr->nlc : NULL;
		break;

	default:
		ReleaseMutex(connection_lock_);
		hConnection = pnlhr ? pnlhr->nlc : NULL;
		break;
	}

	if ( pnlhr != NULL )
	{
		parent->debugLogA("@@@@@ Got response with code %d", pnlhr->resultCode);
		store_headers( &resp, pnlhr->headers, pnlhr->headersCount );
		resp.code = pnlhr->resultCode;
		resp.data = pnlhr->pData ? pnlhr->pData : "";

		parent->debugLogA("&&&&& Got response: %s", resp.data.c_str());

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pnlhr);
	} else {
		parent->debugLogA("!!!!! No response from server (time-out)");
		resp.code = HTTP_CODE_FAKE_DISCONNECTED;
		// Better to have something set explicitely as this value
	    // is compaired in all communication requests
	}

	return resp;
}

bool Omegle_client::handle_entry( std::string method )
{
	parent->debugLogA("   >> Entering %s()", method.c_str());
	return true;
}

bool Omegle_client::handle_success( std::string method )
{
	parent->debugLogA("   << Quitting %s()", method.c_str());
	reset_error();
	return true;
}

bool Omegle_client::handle_error( std::string method, bool force_disconnect )
{
	bool result;
	increment_error();
	parent->debugLogA("!!!!! %s(): Something with Omegle went wrong", method.c_str());

	if ( force_disconnect )
		result = false;
	else if ( error_count_ <= (UINT)db_get_b(NULL,parent->m_szModuleName,OMEGLE_KEY_TIMEOUTS_LIMIT,OMEGLE_TIMEOUTS_LIMIT))
		result = true;
	else
		result = false;

	if ( result == false )
	{
		reset_error();
		parent->UpdateChat(NULL, TranslateT("Connection error."));
		parent->StopChat(false);
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////////

std::string Omegle_client::get_server( bool not_last )
{
	BYTE q = not_last ? 1 : 0;	

	BYTE server = db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_SERVER, 0);
	if (server < 0 || server >= (SIZEOF(servers)-q))
		server = 0;

	if (server == 0) {
		srand(::time(NULL));
		server = (rand() % (SIZEOF(servers)-1-q))+1;
	}

	return servers[server];
}

std::string Omegle_client::get_language()
{
	BYTE language = db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_LANGUAGE, 0);
	if (language < 0 || language >= (SIZEOF(languages)))
		language = 0;

	return language > 0 ? languages[language].id : "en";
}

int Omegle_client::choose_method( int request_type )
{
	switch ( request_type )
	{
	case OMEGLE_REQUEST_HOME:
	case OMEGLE_REQUEST_COUNT:
		return REQUEST_GET;
	
/*	case OMEGLE_REQUEST_START:
	case OMEGLE_REQUEST_STOP:
	case OMEGLE_REQUEST_SEND:
	case OMEGLE_REQUEST_EVENTS:
	case OMEGLE_REQUEST_TYPING_START:
	case OMEGLE_REQUEST_TYPING_STOP:
	case OMEGLE_REQUEST_RECAPTCHA:
*/	default:
		return REQUEST_POST;
	}
}

std::string Omegle_client::choose_server( int request_type, std::string* data, std::string* get_data )
{
	switch ( request_type )
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
		utils::text::replace_first( &server, "%s", this->server_ );
		return server;
	}
}

std::string Omegle_client::choose_action( int request_type, std::string* data, std::string* get_data )
{
	switch ( request_type )
	{
	case OMEGLE_REQUEST_START:
		{
			std::string action = "/start?rcs=1&spid=&lang=";
			action += get_language();
			if (get_data != NULL)
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

std::string Omegle_client::choose_request_url( int request_type, std::string* data, std::string* get_data )
{
	std::string url = "";
	url.append( choose_server( request_type, data, get_data ));
	url.append( choose_action( request_type, data, get_data ));
	return url;
}

NETLIBHTTPHEADER* Omegle_client::get_request_headers( int request_type, int* headers_count )
{
	switch ( request_type )
	{
	case OMEGLE_REQUEST_START:
	case OMEGLE_REQUEST_STOP:
	case OMEGLE_REQUEST_SEND:
	case OMEGLE_REQUEST_EVENTS:
	case OMEGLE_REQUEST_TYPING_START:
	case OMEGLE_REQUEST_TYPING_STOP:
	case OMEGLE_REQUEST_RECAPTCHA:
		*headers_count = 4;
		break;

	case OMEGLE_REQUEST_HOME:
	case OMEGLE_REQUEST_COUNT:
	default:
		*headers_count = 3;
		break;
	}

	NETLIBHTTPHEADER* headers = ( NETLIBHTTPHEADER* )utils::mem::allocate( sizeof( NETLIBHTTPHEADER )*( *headers_count ));

	switch ( request_type )
	{
	case OMEGLE_REQUEST_START:
	case OMEGLE_REQUEST_STOP:
	case OMEGLE_REQUEST_SEND:
	case OMEGLE_REQUEST_EVENTS:
	case OMEGLE_REQUEST_TYPING_START:
	case OMEGLE_REQUEST_TYPING_STOP:
	case OMEGLE_REQUEST_RECAPTCHA:
		headers[3].szName = "Content-Type";
		headers[3].szValue = "application/x-www-form-urlencoded; charset=utf-8";

	case OMEGLE_REQUEST_HOME:
	case OMEGLE_REQUEST_COUNT:
	default:
		headers[2].szName = "User-Agent";
		headers[2].szValue = (char *)g_strUserAgent.c_str( );
		headers[1].szName = "Accept";
		headers[1].szValue = "*/*";
		headers[0].szName = "Accept-Language";
		headers[0].szValue = "en,en-US;q=0.9";
		break;
	}

	return headers;
}

void Omegle_client::store_headers( http::response* resp, NETLIBHTTPHEADER* headers, int headersCount )
{
	for ( int i = 0; i < headersCount; i++ )
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
	handle_entry( "start" );

	this->server_ = get_server();
	//parent->debugLogA("Chosing server %s", this->server_.c_str());
	//std::string log = Translate("Chosing server: ") + this->server_;
	//parent->UpdateChat(NULL, log.c_str());

	std::string data;

	if (this->spy_mode_) {
		//// get last server from list, which is for spy mode
		//this->server_ = servers[SIZEOF(servers)-1];

		if (this->question_.empty()) {
			data = "&wantsspy=1";
		} else {
			data = "&ask=" + utils::url::encode(this->question_);
			data += "&cansavequestion=";
			data += db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_REUSE_QUESTION, 0) ? "1" : "0";
		}
	}
	else if ( db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_MEET_COMMON, 0))
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

	if (db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_SERVER_INFO, 0))
	{
		std::string count = get_page( OMEGLE_REQUEST_COUNT );
		if (!count.empty()) {
			char str[255];
			mir_snprintf(str, sizeof(str), Translate("Connected to server %s. There are %s users online now."), server_.c_str(), count.c_str());

			TCHAR *msg = mir_a2t(str);
			parent->UpdateChat(NULL, msg);
			mir_free(msg);
		}
	} else {
		char str[255];
		mir_snprintf(str, sizeof(str), Translate("Connected to server %s."), server_.c_str());

		TCHAR *msg = mir_a2t(str);
		parent->UpdateChat(NULL, msg);
		mir_free(msg);
	}

	// Send validation
	http::response resp = flap( OMEGLE_REQUEST_START, NULL, &data );

	switch ( resp.code )
	{
	case HTTP_CODE_FAKE_DISCONNECTED:
	{
		// If is is only timeout error, try login once more
		if ( handle_error( "start" ))
			return start();
		else
			return false;
	}

	case HTTP_CODE_OK:
	{ 
		if (!resp.data.empty()) {
			this->chat_id_ = resp.data.substr(1,resp.data.length()-2);
			this->state_ = STATE_WAITING;

			return handle_success( "start" );
		} else {
			return handle_error( "start", FORCE_DISCONNECT );
		}
	}

	default:
		return handle_error( "start", FORCE_DISCONNECT );
	}
}

bool Omegle_client::stop( )
{
	if ( parent->isOffline())
		return true;

	handle_entry( "stop" );

	std::string data = "id=" + this->chat_id_;

	http::response resp = flap( OMEGLE_REQUEST_STOP, &data );

	if (hConnection)
		Netlib_CloseHandle(hConnection);
	hConnection = NULL;

	if (hEventsConnection)
		Netlib_CloseHandle(hEventsConnection);
	hEventsConnection = NULL;

	if (resp.data == "win") {
		return handle_success( "stop" );
	} else {
		return handle_error( "stop" );
	}

/*	switch ( resp.code )
	{
	case HTTP_CODE_OK:
	case HTTP_CODE_FOUND:

	default:
		
	}*/
}

bool Omegle_client::events( )
{
	handle_entry( "events" );

	std::string data = "id=" + this->chat_id_;

	// Get update
	http::response resp = flap( OMEGLE_REQUEST_EVENTS, &data );

	// Return
	switch ( resp.code )
	{
	case HTTP_CODE_OK:
	{
		if ( resp.data == "null" ) {
			// Everything is OK, no new message received -- OR it is a problem
			// TODO: if we are waiting for Stranger with common likes, then we should try standard Stranger if this takes too long
			return handle_error( "events" );
		} else if ( resp.data == "fail" ) {
			// Something went wrong
			return handle_error( "events" );
		}
		
		std::string::size_type pos = 0;
		bool newStranger = false;
		bool waiting = false;
		
		if ( resp.data.find( "[\"waiting\"]" ) != std::string::npos ) {
			// We are just waiting for new Stranger
			waiting = true;
		}

		/*if ( (pos = resp.data.find( "[\"count\"," )) != std::string::npos ) {
			// We got info about count of connected people there
			pos += 9;

			std::string count = utils::text::trim( resp.data.substr(pos, resp.data.find("]", pos) - pos));

			char str[255];
			mir_snprintf(str, sizeof(str), Translate("On whole Omegle are %s strangers online now."), count.c_str());
			
			TCHAR *msg = mir_a2t_cp(str,CP_UTF8);
			parent->UpdateChat(NULL, msg);
			mir_free(msg);
		}*/

		if ( (pos = resp.data.find( "[\"serverMessage\", \"" )) != std::string::npos ) {
			// We got server message
			pos += 19;

			std::string message = utils::text::trim( resp.data.substr(pos, resp.data.find("\"]", pos) - pos));
			TCHAR *tstr = Langpack_PcharToTchar(message.c_str());
			parent->UpdateChat(NULL, tstr);
			mir_free(tstr);
		}

		if ( resp.data.find( "[\"connected\"]" ) != std::string::npos ) {
			// Stranger connected
			if (this->spy_mode_ && !this->question_.empty()) {
				parent->AddChatContact(TranslateT("Stranger 1"));
				parent->AddChatContact(TranslateT("Stranger 2"));
				this->state_ = STATE_SPY;
			} else {			
				parent->AddChatContact(TranslateT("Stranger"));
				this->state_ = STATE_ACTIVE;
			}

			newStranger = true;
			waiting = false;
		}

		if ( (pos = resp.data.find( "[\"commonLikes\"," )) != std::string::npos ) {
			pos += 18;
			std::string like = resp.data.substr(pos, resp.data.find("\"]", pos) - pos);
			utils::text::replace_all(&like, "\", \"", ", ");

			parent->debugLogA("Got common likes: '%s'", like.c_str());

			like = Translate("You and the Stranger both like: ") + like;

			TCHAR *msg = mir_a2t(like.c_str());
			parent->SetTopic(msg);
			mir_free(msg);
		}

		if ( (pos = resp.data.find( "[\"question\"," )) != std::string::npos ) {
			pos += 14;

			std::string question = utils::text::trim(
				utils::text::special_expressions_decode(
					utils::text::slashu_to_utf8(
						resp.data.substr(pos, resp.data.find("\"]", pos) - pos)	)) );

			TCHAR *msg = mir_a2t_cp(question.c_str(),CP_UTF8);
			parent->SetTopic(msg);
			mir_free(msg);
		}

		if ( resp.data.find( "[\"typing\"]" ) != std::string::npos
			|| resp.data.find( "[\"spyTyping\"," ) != std::string::npos )
		{
			// Stranger is typing, not supported by chat module yet
			SkinPlaySound( "StrangerTyp" );
			
			
			StatusTextData st = { 0 };
			st.cbSize = sizeof(st);
			// st.hIcon = Skin_GetIconByHandle(GetIconHandle("typing_on")); // TODO: typing icon

			mir_sntprintf(st.tszText, SIZEOF(st.tszText), TranslateT("%s is typing."), TranslateT("Stranger"));

			CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)parent->GetChatHandle(), (LPARAM)&st);
		}

		if ( resp.data.find( "[\"stoppedTyping\"]" ) != std::string::npos
			|| resp.data.find( "[\"spyStoppedTyping\"," ) != std::string::npos )
		{
			// Stranger stopped typing, not supported by chat module yet
			SkinPlaySound( "StrangerTypStop" );
			
			StatusTextData st = { 0 };
			st.cbSize = sizeof(st);
			// st.hIcon = Skin_GetIconByHandle(GetIconHandle("typing_off")); // TODO: typing icon

			mir_sntprintf(st.tszText, SIZEOF(st.tszText), TranslateT("%s stopped typing."), TranslateT("Stranger"));

			CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)parent->GetChatHandle(), (LPARAM)&st);
		}

		pos = 0;
		while ( (pos = resp.data.find( "[\"gotMessage\",", pos )) != std::string::npos ) {
			pos += 16;

			std::string message = utils::text::trim(
				utils::text::special_expressions_decode(
					utils::text::slashu_to_utf8(
						resp.data.substr(pos, resp.data.find("\"]", pos) - pos)	)) );
			
			if (state_ == STATE_ACTIVE) {
				TCHAR *msg = mir_a2t_cp(message.c_str(),CP_UTF8);
				parent->UpdateChat(TranslateT("Stranger"), msg);
				mir_free(msg);

				CallService(MS_MSG_SETSTATUSTEXT, (WPARAM)parent->GetChatHandle(), 0);
			}
		}

		pos = 0;
		while ( (pos = resp.data.find( "[\"spyMessage\",", pos )) != std::string::npos ) {
			pos += 16;

			std::string message = resp.data.substr(pos, resp.data.find("\"]", pos) - pos);
			
			if (state_ == STATE_SPY) {
				std::string stranger = message.substr(0, message.find("\""));
				message = message.substr(stranger.length() + 4);

				message = utils::text::trim(
							utils::text::special_expressions_decode(
								utils::text::slashu_to_utf8( message )) );
				
				stranger = Translate(stranger.c_str());
				
				TCHAR *str = mir_a2t_cp(stranger.c_str(), CP_UTF8);				
				TCHAR *msg = mir_a2t_cp(message.c_str(), CP_UTF8);

				parent->UpdateChat(str, msg);

				mir_free(msg);
				mir_free(str);
			}
		}

		if ( resp.data.find( "[\"strangerDisconnected\"]" ) != std::string::npos ) {
			// Stranger disconnected
			if (db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_DONT_STOP, 0))
			{
				SkinPlaySound( "StrangerChange" );
				parent->NewChat();
			}
			else			
				parent->StopChat(false);
		}

		if ( (pos = resp.data.find( "[\"spyDisconnected\"," )) != std::string::npos ) {
			pos += 21;

			std::string stranger = utils::text::trim(
				utils::text::special_expressions_decode(
					utils::text::slashu_to_utf8(
						resp.data.substr(pos, resp.data.find("\"]", pos) - pos)	)) );

			char str[255];
			mir_snprintf(str, sizeof(str), Translate("%s disconnected."), Translate(stranger.c_str()));
			
			TCHAR *msg = mir_a2t(str);
			parent->UpdateChat(NULL, msg);
			mir_free(msg);

			// Stranger disconnected
			if (db_get_b(NULL, parent->m_szModuleName, OMEGLE_KEY_DONT_STOP, 0))
			{
				SkinPlaySound( "StrangerChange" );
				parent->NewChat();
			}
			else
				parent->StopChat(false);
		}

		if ( resp.data.find( "[\"recaptchaRequired\"" ) != std::string::npos ) {
			// Nothing to do with recaptcha
			parent->UpdateChat(NULL, TranslateT("Recaptcha is required.\nOpen http://omegle.com , solve Recaptcha and try again."));
			parent->StopChat(false);
		}

		if ( resp.data.find( "[\"recaptchaRejected\"]" ) != std::string::npos ) {
			// Nothing to do with recaptcha
			parent->StopChat(false);
		}

		if ( (pos = resp.data.find( "[\"error\"," )) != std::string::npos ) {
			pos += 11;

			std::string error = utils::text::trim(
				utils::text::special_expressions_decode(
					utils::text::slashu_to_utf8(
						resp.data.substr(pos, resp.data.find("\"]", pos) - pos)	)) );

			error = Translate("Error: ") + error;

			TCHAR *msg = mir_a2t(error.c_str());
			parent->UpdateChat(NULL, msg);
			mir_free(msg);
		}
				
		if (newStranger && state_ != STATE_SPY) {
			// We got new stranger in this event, lets say him "Hi message" if enabled			
			if ( db_get_b( NULL, parent->m_szModuleName, OMEGLE_KEY_HI_ENABLED, 0 )) {
				DBVARIANT dbv;
				if ( !db_get_utf( NULL, parent->m_szModuleName, OMEGLE_KEY_HI, &dbv )) {
					std::string *message = new std::string(dbv.pszVal);
					db_free(&dbv);
	
					parent->debugLogA("**Chat - saying Hi! message");
					parent->ForkThread(&OmegleProto::SendMsgWorker, message);
				}
				else parent->debugLogA("**Chat - Hi message is enabled but not used");
			}
		}

		if (waiting) {
			// If we are only waiting in this event...
			parent->UpdateChat(NULL, TranslateT("We are still waiting..."));
		}

		return handle_success( "events" );
	}

	case HTTP_CODE_FAKE_DISCONNECTED:
		// timeout
		return handle_success( "events" );

	case HTTP_CODE_FAKE_ERROR:
	default:
		return handle_error( "events" );
	}
}

bool Omegle_client::send_message( std::string message_text )
{
	handle_entry( "send_message" );

	std::string data = "msg=" + utils::url::encode( message_text );
	data += "&id=" + this->chat_id_;

	http::response resp = flap( OMEGLE_REQUEST_SEND, &data );

	switch ( resp.code )
	{
	case HTTP_CODE_OK:
		if (resp.data == "win") {
			return handle_success( "send_message" );
		}

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "send_message" );
	}
}

bool Omegle_client::typing_start()
{
	handle_entry( "typing_start" );

	std::string data = "id=" + this->chat_id_;

	http::response resp = flap( OMEGLE_REQUEST_TYPING_START, &data );

	switch ( resp.code )
	{
	case HTTP_CODE_OK:
		if (resp.data == "win") {
			return handle_success( "typing_start" );
		}

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "typing_start" );
	}
}

bool Omegle_client::typing_stop()
{
	handle_entry( "typing_stop" );

	std::string data = "id=" + this->chat_id_;

	http::response resp = flap( OMEGLE_REQUEST_TYPING_STOP, &data );

	switch ( resp.code )
	{
	case HTTP_CODE_OK:
		if (resp.data == "win") {
			return handle_success( "typing_stop" );
		}

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "typing_stop" );
	}
}

bool Omegle_client::recaptcha()
{
	// TODO: Implement!

	handle_entry( "recaptcha" );

	// data:{id:this.clientID,challenge:b,response:a}}
	//std::string data = "?id=...&challenge= ..., &response= ...";

	http::response resp = flap( OMEGLE_REQUEST_RECAPTCHA );

	switch ( resp.code )
	{
	case HTTP_CODE_OK:
/*		if (resp.data == "win") {
			return handle_success( "typing_start" );
		}*/

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		return handle_error( "typing_start" );
	}
}

std::string Omegle_client::get_page( const int request_type )
{
	handle_entry( "get_page" );

	http::response resp = flap( OMEGLE_REQUEST_COUNT );

	switch ( resp.code )
	{
	case HTTP_CODE_OK:
		handle_success( "get_page" );
		break;

	case HTTP_CODE_FAKE_ERROR:
	case HTTP_CODE_FAKE_DISCONNECTED:
	default:
		handle_error( "get_page" );
	}

	return resp.data;
}
