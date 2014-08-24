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

#include "constants.h"

#pragma once

#define FORCE_DISCONNECT 1
#define FORCE_QUIT 2

class facebook_client
{
public:

	////////////////////////////////////////////////////////////

	// Client definition

	facebook_client()
	{
		username_ = password_ = \
		chat_sequence_num_ = chat_channel_host_ = chat_channel_partition_ = \
		dtsg_ = logout_hash_ = chat_sticky_num_ = chat_conn_num_ = chat_clientid_ = chat_traceid_ = "";

		msgid_ = error_count_ = last_feeds_update_ = last_notification_time_ = 0;

		https_ = is_idle_ = is_typing_ = false;

		buddies_lock_ = send_message_lock_ = NULL;
		hMsgCon = NULL;
		hFcbCon = NULL;
		fcb_conn_lock_ = NULL;
	}

	HANDLE hMsgCon;
	HANDLE hFcbCon;
	HANDLE fcb_conn_lock_;

	// Parent handle

	FacebookProto*  parent;

	// User data

	facebook_user   self_;

	std::string username_;
	std::string password_;

	std::string dtsg_;
	std::string logout_hash_;
	std::string chat_channel_;
	std::string chat_channel_host_;
	std::string chat_channel_partition_;
	std::string chat_sequence_num_;
	std::string chat_reconnect_reason_;
	std::string chat_sticky_num_;
	std::string chat_conn_num_;
	std::string chat_clientid_;
	std::string chat_traceid_;
	bool    is_typing_;
	bool	is_idle_;
	bool	https_;
	time_t  last_feeds_update_;
	unsigned __int64 last_notification_time_;
	int     msgid_;

	////////////////////////////////////////////////////////////

	// Client vs protocol communication

	void    client_notify(TCHAR* message);

	////////////////////////////////////////////////////////////

	// Cookies, Data storage

	HANDLE cookies_lock_;

	std::map<std::string, std::string> cookies;
	std::map<std::string, std::string> pages;
	std::map<std::tstring, facebook_chatroom*> chat_rooms;
	std::map<std::string, facebook_notification*> notifications;

	std::string get_newsfeed_type();
	std::string get_server_type();
	std::string get_privacy_type();

	std::map<MCONTACT, bool> ignore_read;
	std::set<MCONTACT> typers;		// store info about typing contacts, because Facebook doesn't send "stopped typing" event when there is actual message being sent
	std::map<MCONTACT, time_t> readers;

	char*   load_cookies();
	void    store_headers(http::response* resp, NETLIBHTTPHEADER* headers, int headers_count);
	void    clear_cookies();
	void	clear_notifications();
	void	clear_chatrooms();

	////////////////////////////////////////////////////////////

	// Connection handling

	unsigned int error_count_;

	bool    handle_entry(std::string method);
	bool    handle_success(std::string method);
	bool    handle_error(std::string method, int force_disconnect = 0);

	void __inline increment_error() { this->error_count_++; }
	void __inline decrement_error() { if (error_count_ > 0) error_count_--; }
	void __inline reset_error() { error_count_ = 0; }

	////////////////////////////////////////////////////////////

	// Login handling

	bool    login(const char *username, const char *password);
	bool    logout();

	const std::string & get_username() const;

	////////////////////////////////////////////////////////////

	// Session handling

	bool    home();
	bool    reconnect();
	bool    chat_state(bool online = true);

	////////////////////////////////////////////////////////////

	// Updates handling

	List::List<facebook_user> buddies;
	HANDLE  buddies_lock_;
	HANDLE  send_message_lock_;

	////////////////////////////////////////////////////////////

	// Messages handling

	std::map<std::string, int> messages_ignore;

	bool    channel();
	int		send_message(MCONTACT, std::string message_recipient, std::string message_text, std::string *error_text, MessageMethod method, std::string captchaPersistData = "", std::string captcha = "");
	////////////////////////////////////////////////////////////

	// Status handling

	bool    post_status(status_data *data);

	////////////////////////////////////////////////////////////

	// HTTP communication

	http::response flap(RequestType request_type, std::string* request_data = NULL, std::string* request_get_data = NULL, int method = 0);
	bool    save_url(const std::string &url,const std::tstring &filename, HANDLE &nlc);

	DWORD   choose_security_level(RequestType);
	int     choose_method(RequestType);
	bool	notify_errors(RequestType);
	std::string choose_proto(RequestType);
	std::string choose_server(RequestType, std::string* data = NULL, std::string* get_data = NULL);
	std::string choose_action(RequestType, std::string* data = NULL, std::string* get_data = NULL);

	NETLIBHTTPHEADER*   get_request_headers(int request_type, int* headers_count);

	////////////////////////////////////////////////////////////

	// Netlib handle

	HANDLE handle_;

	void set_handle(HANDLE h)
	{
		handle_ = h;
	}
};
