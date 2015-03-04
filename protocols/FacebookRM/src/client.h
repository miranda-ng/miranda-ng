/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-15 Robert P�sel

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
		msgid_ = error_count_ = last_feeds_update_ = last_notification_time_ = random_ = 0;

		is_typing_ = false;

		buddies_lock_ = send_message_lock_ = notifications_lock_ = cookies_lock_ = NULL;
		hMsgCon = NULL;
		hFcbCon = NULL;
		fcb_conn_lock_ = NULL;
		handle_ = NULL;
		parent = NULL;
	}

	HANDLE hMsgCon;
	HANDLE hFcbCon;
	HANDLE fcb_conn_lock_;

	// Random generator value for this client

	unsigned int random_;

	// Parent handle

	FacebookProto*  parent;

	// User data

	facebook_user   self_;

	std::string username_;
	std::string password_;

	std::string dtsg_;
	std::string csrf_;
	std::string logout_hash_;
	std::string chat_channel_;
	std::string chat_channel_host_;
	std::string chat_channel_partition_;
	std::string chat_sequence_num_;
	std::string chat_reconnect_reason_;
	std::string chat_sticky_num_;
	std::string chat_sticky_pool_;
	std::string chat_conn_num_;
	std::string chat_clientid_;
	std::string chat_traceid_;
	bool is_typing_;
	time_t last_feeds_update_;
	time_t last_notification_time_;
	int msgid_;

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

	// Contact/thread id caches
	std::map<std::string, std::string> thread_id_to_user_id;
	std::map<std::tstring, MCONTACT> chat_id_to_hcontact;
	std::map<std::string, MCONTACT> user_id_to_hcontact;

	std::string get_newsfeed_type();
	std::string get_server_type();
	std::string get_privacy_type();

	std::set<MCONTACT> ignore_read;
	std::set<MCONTACT> typers;		// store info about typing contacts, because Facebook doesn't send "stopped typing" event when there is actual message being sent
	std::map<MCONTACT, time_t> readers;

	char*   load_cookies();
	void    store_headers(http::response* resp, NETLIBHTTPHEADER* headers, int headers_count);
	void    clear_cookies();
	void	clear_notifications();
	void	clear_chatrooms();
	void	clear_readers();
	void	insert_reader(MCONTACT, time_t, const std::tstring &reader = _T(""));
	void	erase_reader(MCONTACT);

	////////////////////////////////////////////////////////////

	// Connection handling

	unsigned int error_count_;

	bool    handle_entry(const std::string &method);
	bool    handle_success(const std::string &method);
	bool    handle_error(const std::string &method, int force_disconnect = 0);

	void __inline increment_error() { this->error_count_++; }
	void __inline decrement_error() { if (error_count_ > 0) error_count_--; }
	void __inline reset_error() { error_count_ = 0; }

	////////////////////////////////////////////////////////////

	// Helpers for data

	std::string __inline phstamp(const std::string &data) { 
		std::stringstream out;
		out << '2' << this->csrf_ << (int)data.length();
		return out.str();
	}

	std::string __inline ttstamp() {
		std::stringstream out;
		out << '1' << this->csrf_;
		return out.str();
	}

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
	HANDLE  notifications_lock_;

	////////////////////////////////////////////////////////////

	// Messages handling

	std::map<std::string, int> messages_ignore;
	std::map<int, time_t> messages_timestamp;

	bool    channel();
	bool	activity_ping();
	int		send_message(int seqid, MCONTACT, const std::string &message_recipient, const std::string &message_text, std::string *error_text, MessageMethod method, const std::string &captchaPersistData = "", const std::string &captcha = "");

	////////////////////////////////////////////////////////////

	// Status handling

	bool    post_status(status_data *data);

	////////////////////////////////////////////////////////////

	// HTTP communication

	http::response flap(RequestType request_type, std::string *post_data = NULL, std::string *get_data = NULL, int method = 0);
	bool save_url(const std::string &url,const std::tstring &filename, HANDLE &nlc);

	bool notify_errors(RequestType);
	std::string choose_server(RequestType);
	std::string choose_action(RequestType, std::string *get_data = NULL);

	NETLIBHTTPHEADER *get_request_headers(int request_type, int *headers_count);

	////////////////////////////////////////////////////////////

	// Netlib handle

	HANDLE handle_;

	void set_handle(HANDLE h)
	{
		handle_ = h;
	}
};
