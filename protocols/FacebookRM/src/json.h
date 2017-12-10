/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel

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
	int parse_friends(std::string*, std::map< std::string, facebook_user* >*, bool);
	int parse_notifications(std::string*, std::map< std::string, facebook_notification* >*);
	int parse_messages(std::string*, std::vector< facebook_message >*, std::map< std::string, facebook_notification* >*);
	int parse_unread_threads(std::string*, std::vector< std::string >*);
	int parse_thread_messages(std::string*, std::vector< facebook_message >*, bool unreadOnly);
	int parse_history(std::string*, std::vector< facebook_message >*, std::string *);
	int parse_thread_info(std::string* data, std::string* user_id);
	int parse_user_info(std::string* data, facebook_user* fbu);
	int parse_chat_info(std::string* data, facebook_chatroom* fbc);
	int parse_chat_participant_names(std::string *data, std::map<std::string, chatroom_participant>* participants);
	int parse_messages_count(std::string *data, int *messagesCount, int *unreadCount);

	facebook_json_parser(FacebookProto* proto)
	{
		this->proto = proto;
	}
};

struct PARAM
{
	LPCSTR szName;
	__forceinline PARAM(LPCSTR _name) : szName(_name)
	{}
};

struct NULL_PARAM : public PARAM
{
	__forceinline NULL_PARAM(LPCSTR _name) : PARAM(_name)
	{}
};

struct JSON_PARAM : public PARAM
{
	JSONNode node;
	__forceinline JSON_PARAM(LPCSTR _name, JSONNode _node) :
		PARAM(_name), node(_node)
	{}
};

struct BOOL_PARAM : public PARAM
{
	bool bValue;
	__forceinline BOOL_PARAM(LPCSTR _name, bool _value) :
		PARAM(_name), bValue(_value)
	{}
};

struct INT_PARAM : public PARAM
{
	int iValue;
	__forceinline INT_PARAM(LPCSTR _name, int _value) :
		PARAM(_name), iValue(_value)
	{}
};

struct CHAR_PARAM : public PARAM
{
	LPCSTR szValue;
	__forceinline CHAR_PARAM(LPCSTR _name, LPCSTR _value) :
		PARAM(_name), szValue(_value)
	{}
};

struct WCHAR_PARAM : public PARAM
{
	LPCWSTR wszValue;
	__forceinline WCHAR_PARAM(LPCSTR _name, LPCWSTR _value) :
		PARAM(_name), wszValue(_value)
	{}
};

JSONNode& operator<<(JSONNode &json, const NULL_PARAM &param);
JSONNode& operator<<(JSONNode &json, const JSON_PARAM &param);
JSONNode& operator<<(JSONNode &json, const INT_PARAM &param);
JSONNode& operator<<(JSONNode &json, const BOOL_PARAM &param);
JSONNode& operator<<(JSONNode &json, const CHAR_PARAM &param);
JSONNode& operator<<(JSONNode &json, const WCHAR_PARAM &param);
