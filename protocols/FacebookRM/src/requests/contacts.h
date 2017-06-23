/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel

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

#ifndef _FACEBOOK_REQUEST_CONTACTS_H_
#define _FACEBOOK_REQUEST_CONTACTS_H_

// getting frienship requests (using mobile website)
class GetFriendshipsRequest : public HttpRequest
{
public:
	GetFriendshipsRequest(bool mobileBasicWorks) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/friends/center/requests/", mobileBasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE)
	{
		flags |= NLHRF_REDIRECT;
	}
};


// getting info about particular friend
// revised 17.8.2016
class UserInfoRequest : public HttpRequest
{
public:
	UserInfoRequest(facebook_client *fc, const LIST<char> &userIds) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/chat/user_info/")
	{
		Url
			<< "dpr=1";

		for (int i = 0; i < userIds.getCount(); i++) {
			CMStringA id(::FORMAT, "ids[%i]", i);
			Body << CHAR_VALUE(id, ptrA(mir_urlEncode(userIds[i])));
		}

		Body
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< "__a=1"
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1";
	}
};

// getting info about all friends
// revised 17.8.2016
class UserInfoAllRequest : public HttpRequest
{
public:
	UserInfoAllRequest(facebook_client *fc) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/chat/user_info_all/")
	{
		Url
			<< "dpr=1"
			<< CHAR_VALUE("viewer", fc->self_.user_id.c_str());

		Body
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< "__a=1"
			<< "__pc=PHASED:DEFAULT"
			<< "__be=-1";
	}
};

// requesting friendships
class AddFriendRequest : public HttpRequest
{
public:
	AddFriendRequest(facebook_client *fc, const char *userId) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/add_friend/action.php")
	{
		Url
			<< "__a=1";

		Body
			<< CHAR_VALUE("to_friend", userId)
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< "action=add_friend"
			<< "how_found=profile_button"
			<< "ref_param=ts"
			<< "outgoing_id="
			<< "unwanted="
			<< "logging_location="
			<< "no_flyout_on_click=false"
			<< "ego_log_data="
			<< "lsd=";
	}
};

// deleting friendships
class DeleteFriendRequest : public HttpRequest
{
public:
	DeleteFriendRequest(facebook_client *fc, const char *userId) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/profile/removefriendconfirm.php")
	{
		Url
			<< "__a=1"
			<< "norefresh=true"
			<< "unref=button_dropdown"
			<< CHAR_VALUE("uid", userId);

		Body
			<< CHAR_VALUE("uid", userId)
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str())
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< "norefresh=true"
			<< "unref=button_dropdown"
			<< "confirmed=1"
			<< "__a=1";
	}
};

// canceling (our) friendship request
class CancelFriendshipRequest : public HttpRequest
{
public:
	CancelFriendshipRequest(facebook_client *fc, const char *userId) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/friends/requests/cancel.php")
	{
		Url
			<< "__a=1";

		Body
			<< "confirmed=1"
			<< CHAR_VALUE("friend", userId)
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str());
	}
};

// approving or ignoring friendship requests
class AnswerFriendshipRequest : public HttpRequest
{
public:
	enum Answer { CONFIRM, REJECT };

	AnswerFriendshipRequest(facebook_client *fc, const char *userId, Answer answer) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/requests/friends/ajax/")
	{
		Url
			<< "__a=1";

		const char *action = "";
		switch (answer) {
		case CONFIRM:
			action = "confirm";
			break;
		case REJECT:
			action = "reject";
			break;
		}

		Body
			<< CHAR_VALUE("action", action)
			<< CHAR_VALUE("id", userId)
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str());
	}
};

#endif //_FACEBOOK_REQUEST_CONTACTS_H_
