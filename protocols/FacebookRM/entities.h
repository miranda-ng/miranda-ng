/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

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

struct facebook_user
{
	HANDLE handle;

	std::string user_id;
	std::string real_name;

	unsigned int status_id;
	unsigned int gender;

	std::string image_url;

	bool deleted;

	facebook_user( )
	{
		this->handle = NULL;
		this->user_id = this->real_name = this->image_url = "";
		this->status_id = ID_STATUS_OFFLINE;
		this->gender = 0;
		this->deleted = false;
	}

	facebook_user( facebook_user* fu )
	{
		this->handle = fu->handle;
		this->image_url = fu->image_url;
		this->real_name = fu->real_name;
		this->status_id = fu->status_id;
		this->user_id = fu->user_id;
		this->gender = fu->gender;
		this->deleted = fu->deleted;
	}
};

struct facebook_message
{
	std::string user_id;
	std::string message_text;
	std::string sender_name;
	time_t time;

	facebook_message( )
	{
		this->user_id = this->message_text = this->sender_name = "";
		this->time = 0;
	}

	facebook_message( const facebook_message& msg )
	{
		this->user_id = msg.user_id;
		this->message_text = msg.message_text;
		this->sender_name = msg.sender_name;
		this->time = msg.time;
	}
};

struct facebook_notification
{
	std::string user_id;
	std::string text;
	std::string link;

	facebook_notification( )
	{
		this->user_id = this->text = this->link = "";
	}
};

struct facebook_newsfeed
{
	std::string user_id;
	std::string title;
	std::string text;
	std::string link;

	facebook_newsfeed( )
	{
		this->user_id = this->title = this->text = this->link = "";
	}
};


struct send_chat
{
	send_chat(const std::string &chat_id,const std::string &msg) : chat_id(chat_id), msg(msg) {}
	std::string chat_id;
	std::string msg;
};


struct send_direct
{
	send_direct(HANDLE hContact,const std::string &msg, HANDLE msgid) : hContact(hContact), msg(msg), msgid(msgid) {}
	HANDLE hContact;
	std::string msg;
	HANDLE msgid;
};

struct send_typing
{
	send_typing(HANDLE hContact,const int status) : hContact(hContact), status(status) {}
	HANDLE hContact;
	int status;
};

struct send_messaging
{
	send_messaging(const std::string &user_id, const int type) : user_id(user_id), type(type) {}
	std::string user_id;
	int type;
};
