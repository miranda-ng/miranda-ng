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

#pragma once

struct facebook_user
{
	MCONTACT handle;

	std::string user_id;
	std::string real_name;
	std::string nick;
	std::string username;

	unsigned int status_id;
	unsigned int gender;
	time_t last_active;

	std::string image_url;

	bool deleted;
	bool idle;

	bool updated;

	ClientType client;
	ContactType type;

	facebook_user()
	{
		this->handle = NULL;
		this->status_id = ID_STATUS_OFFLINE;
		this->gender = this->last_active = 0;
		this->deleted = this->idle = this->updated = false;
		this->client = CLIENT_WEB;
		this->type = CONTACT_NONE;
	}

	wchar_t *getMirVer()
	{
		switch (this->client) {
		case CLIENT_APP:
			return FACEBOOK_CLIENT_APP;
		case CLIENT_MESSENGER:
			return FACEBOOK_CLIENT_MESSENGER;
		case CLIENT_OTHER:
			return FACEBOOK_CLIENT_OTHER;
		case CLIENT_MOBILE:
			return FACEBOOK_CLIENT_MOBILE;
		case CLIENT_WEB:
		default:
			return FACEBOOK_CLIENT_WEB;
		}
	}
};

struct chatroom_participant
{
	std::string user_id;
	std::string nick;	
	ParticipantRole role;	
	bool is_former;
	bool loaded;

	chatroom_participant() : loaded(false), is_former(false), role(ROLE_NONE) {}
};

struct facebook_chatroom
{	
	std::string thread_id;
	std::wstring chat_name;
	std::map<std::string, chatroom_participant> participants;
	bool can_reply;
	bool is_archived;
	bool is_subscribed;
	bool read_only;

	facebook_chatroom(std::string thread_id) : thread_id(thread_id) {
		this->can_reply = true;
		this->is_archived = false;
		this->is_subscribed = true;
		this->read_only = false;
	}
};

struct facebook_message
{
	std::string user_id;
	std::string message_text;	
	std::string message_id;
	std::string thread_id;
	std::string data;
	time_t time;
	bool isIncoming;
	bool isUnread;
	bool isChat;
	MessageType type;

	int flag_;

	facebook_message()
	{
		this->time = 0;
		this->isUnread = true;
		this->isIncoming = true;
		this->isChat = false;
		this->type = MESSAGE;
		this->flag_ = 0;
	}
};

struct facebook_notification
{
	std::string user_id;
	std::string text;
	std::string link;
	std::string id;
	const char *icon;
	time_t time;
	bool seen;
	HWND hWndPopup;

	facebook_notification()
	{
		this->time = 0;
		this->seen = false;
		this->hWndPopup = NULL;
		this->icon = NULL;
	}

	void setIcon(const std::string &iconUrl)
	{
		if (iconUrl == "https://www.facebook.com/rsrc.php/v3/yj/r/6WffvhOaXGY.png")
			icon = "like";
		else if (iconUrl == "https://www.facebook.com/rsrc.php/v3/y1/r/RvGKklgAefT.png")
			icon = "love";
		else if (iconUrl == "https://www.facebook.com/rsrc.php/v3/yV/r/McJA2ZjdJmf.png")
			icon = "haha";
		else if (iconUrl == "https://www.facebook.com/rsrc.php/v3/yL/r/IfsimazVjj4.png")
			icon = "wow";
		else if (iconUrl == "https://www.facebook.com/rsrc.php/v3/yH/r/jOeSrGlcPLG.png")
			icon = "sad";
		else if (iconUrl == "https://www.facebook.com/rsrc.php/v3/y9/r/6K8v8Ju8kL2.png")
			icon = "angry";
		else
			icon = NULL;
	}
};

struct facebook_newsfeed
{
	std::string user_id;
	std::string title;
	std::string text;
	std::string link;

	facebook_newsfeed() {}
};

struct send_chat
{
	send_chat(const std::string &chat_id,const std::string &msg) : chat_id(chat_id), msg(msg) {}
	std::string chat_id;
	std::string msg;
};

struct send_direct
{
	send_direct(MCONTACT hContact,const std::string &msg, int msgid) : hContact(hContact), msg(msg), msgid(msgid) {}
	MCONTACT hContact;
	std::string msg;
	int msgid;
};

struct send_typing
{
	send_typing(MCONTACT hContact,const int status) : hContact(hContact), status(status) {}
	MCONTACT hContact;
	int status;
};

struct popup_data
{
	popup_data(FacebookProto *proto) : proto(proto) {}
	popup_data(FacebookProto *proto, std::string url) : proto(proto), url(url) {}
	FacebookProto *proto;
	std::string url;
	std::string notification_id;
};

struct status_data
{
	status_data() {
		this->isPage = false;
	}
	std::string user_id;
	std::string text;
	std::string url;	
	std::string place;	
	std::string privacy;
	bool isPage;
	std::vector<facebook_user*> users;
};

struct wall_data
{
	wall_data() {
		this->title = NULL;
		this->isPage = false;
	}
	wall_data(std::string user_id, wchar_t *title, bool isPage = false) : user_id(user_id), title(title), isPage(isPage) {}
	std::string user_id;
	wchar_t *title;
	bool isPage;
};

struct post_status_data {
	post_status_data(FacebookProto *proto) : proto(proto) {}
	post_status_data(FacebookProto *proto, wall_data *wall) : proto(proto) {
		this->walls.push_back(wall);
	}
	FacebookProto *proto;
	std::vector<wall_data*> walls;
};

struct open_url
{
	open_url(wchar_t *browser, wchar_t *url) : browser(browser), url(url) {}
	wchar_t *browser;
	wchar_t *url;	
};
