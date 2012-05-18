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

// Version management
#define __VERSION_DWORD             PLUGIN_MAKE_VERSION(0, 0, 8, 1)
#define __VERSION_STRING            "0.0.8.1"

// Product management
#define FACEBOOK_NAME               "Facebook"
#define FACEBOOK_URL_HOMEPAGE       "http://www.facebook.com"
#define FACEBOOK_URL_REQUESTS       "http://www.facebook.com/n/?reqs.php"
#define FACEBOOK_URL_MESSAGES       "http://www.facebook.com/n/?inbox"
#define FACEBOOK_URL_NOTIFICATIONS  "http://www.facebook.com/n/?notifications.php"
#define FACEBOOK_URL_PROFILE        "http://www.facebook.com/profile.php?id="
#define FACEBOOK_URL_GROUP          "http://www.facebook.com/n/?home.php&sk=group_"

// Connection
#define FACEBOOK_SERVER_REGULAR     "www.facebook.com"
#define FACEBOOK_SERVER_CHAT        "%s.%s.facebook.com"
#define FACEBOOK_SERVER_CHAT2       "%s-%s.facebook.com"
#define FACEBOOK_SERVER_LOGIN       "login.facebook.com"
#define FACEBOOK_SERVER_APPS        "apps.facebook.com"

// Limits
#define FACEBOOK_MESSAGE_LIMIT      1024
#define FACEBOOK_MESSAGE_LIMIT_TEXT "1024"
#define FACEBOOK_MIND_LIMIT         420
#define FACEBOOK_MIND_LIMIT_TEXT    "420"
#define FACEBOOK_TIMEOUTS_LIMIT     5
#define FACEBOOK_GROUP_NAME_LIMIT   100

// Defaults
#define FACEBOOK_MINIMAL_POLL_RATE              10
#define FACEBOOK_DEFAULT_POLL_RATE              24 // in seconds
#define FACEBOOK_MAXIMAL_POLL_RATE              60

#define DEFAULT_FORCE_HTTPS             0
#define DEFAULT_FORCE_HTTPS_CHANNEL     0
#define DEFAULT_CLOSE_WINDOWS_ENABLE    0
#define DEFAULT_SET_MIRANDA_STATUS      0
#define DEFAULT_LOGGING_ENABLE          0
#define DEFAULT_SYSTRAY_NOTIFY          0
#define DEFAULT_DISABLE_STATUS_NOTIFY	0
#define DEFAULT_BIG_AVATARS				0
#define DEFAULT_DISCONNECT_CHAT			0
#define DEFAULT_PARSE_MESSAGES			0
#define DEFAULT_MAP_STATUSES			0
#define DEFAULT_LOAD_MOBILE				0
#define DEFAULT_ENABLE_GROUPCHATS		0

#define DEFAULT_EVENT_NOTIFICATIONS_ENABLE  1
#define DEFAULT_EVENT_FEEDS_ENABLE          1
#define DEFAULT_EVENT_OTHER_ENABLE          1
#define DEFAULT_EVENT_CLIENT_ENABLE         1
#define DEFAULT_EVENT_COLBACK           0x00ffffff
#define DEFAULT_EVENT_COLTEXT           0x00000000
#define DEFAULT_EVENT_TIMEOUT_TYPE      0
#define DEFAULT_EVENT_TIMEOUT           20

// Event flags
#define FACEBOOK_EVENT_CLIENT          0x10000000 // Facebook error or info message
#define FACEBOOK_EVENT_NEWSFEED        0x20000000 // Facebook newsfeed (wall) message
#define FACEBOOK_EVENT_NOTIFICATION    0x40000000 // Facebook new notification
#define FACEBOOK_EVENT_OTHER           0x80000000 // Facebook other event - friend requests/new messages

// Facebook request types // TODO: Provide MS_ and release in FB plugin API?
#define FACEBOOK_REQUEST_LOGIN                  100 // connecting physically
#define FACEBOOK_REQUEST_SETUP_MACHINE          102 // setting machine name
#define FACEBOOK_REQUEST_LOGOUT                 106 // disconnecting physically
#define FACEBOOK_REQUEST_HOME                   110 // getting __post_form_id__ + __fb_dtsg__ + ...
#define FACEBOOK_REQUEST_BUDDY_LIST             120 // getting regular updates (friends online, ...)
#define FACEBOOK_REQUEST_LOAD_FRIENDS			121 // getting list of all friends
#define FACEBOOK_REQUEST_DELETE_FRIEND			122 // deleting friends
#define FACEBOOK_REQUEST_ADD_FRIEND				123 // adding friends
#define FACEBOOK_REQUEST_FEEDS                  125 // getting feeds
#define FACEBOOK_REQUEST_NOTIFICATIONS			126 // getting notifications
#define FACEBOOK_REQUEST_RECONNECT              130 // getting __sequence_num__ and __channel_id__
#define FACEBOOK_REQUEST_STATUS_SET             251 // setting my "What's on my mind?"
#define FACEBOOK_REQUEST_MESSAGE_SEND           300 // sending message
#define FACEBOOK_REQUEST_MESSAGES_RECEIVE       301 // receiving messages
#define FACEBOOK_REQUEST_TYPING_SEND            304 // sending typing notification
#define FACEBOOK_REQUEST_VISIBILITY             305 // setting chat visibility
#define FACEBOOK_REQUEST_TABS					306 // closing message window
#define	FACEBOOK_REQUEST_ASYNC					307 // marking messages read and getting other things

#define FACEBOOK_RECV_MESSAGE	1
#define FACEBOOK_SEND_MESSAGE	2

// News Feed types
static const struct
{
	const char *name;
	const char *id;
} feed_types[] = {
	{ "Most Recent", "lf_" }, //h_chr?
	{ "Wall Posts", "app_2915120374" },
	{ "Top News", "h_nor" }, //h
	{ "Photos", "app_2305272732_2392950137" },
	{ "Links", "app_2309869772" },
	{ "Apps and Games", "appsandgames" },
};