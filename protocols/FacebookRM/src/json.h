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

#pragma once

// Parser front-end

#define lltoa _i64toa

class facebook_json_parser
{
public:
	FacebookProto* proto;
	int parse_buddy_list(std::string*, List::List< facebook_user >*);
	int parse_friends(std::string*, std::map< std::string, facebook_user* >*);
	int parse_notifications(std::string*, std::map< std::string, facebook_notification* >*);
	int parse_messages(std::string*, std::vector< facebook_message* >*, std::map< std::string, facebook_notification* >*, bool inboxOnly);
	int parse_unread_threads(std::string*, std::vector< std::string >*, bool inboxOnly);
	int parse_thread_messages(std::string*, std::vector< facebook_message* >*, std::map< std::string, facebook_chatroom* >*, bool unreadOnly, bool inboxOnly);
	int parse_thread_info(std::string* data, std::string* user_id);
	int parse_user_info(std::string* data, facebook_user* fbu);
	int parse_chat_info(std::string* data, facebook_chatroom* fbc);

	facebook_json_parser(FacebookProto* proto)
	{
		this->proto = proto;
	}
};
