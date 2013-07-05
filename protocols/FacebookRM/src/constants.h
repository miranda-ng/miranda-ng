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

#pragma once

// Product management
#define FACEBOOK_NAME							"Facebook"
#define FACEBOOK_MOBILE							"Facebook (Mobile)"
#define FACEBOOK_URL_HOMEPAGE					"http://www.facebook.com"
#define FACEBOOK_URL_REQUESTS					"http://www.facebook.com/n/?reqs.php"
#define FACEBOOK_URL_MESSAGES					"http://www.facebook.com/n/?inbox"
#define FACEBOOK_URL_NOTIFICATIONS				"http://www.facebook.com/n/?notifications.php"
#define FACEBOOK_URL_PROFILE					"http://www.facebook.com/profile.php?id="
#define FACEBOOK_URL_GROUP						"http://www.facebook.com/n/?home.php&sk=group_"

// Connection
#define FACEBOOK_SERVER_REGULAR					"www.facebook.com"
#define FACEBOOK_SERVER_MOBILE					"m.facebook.com"
#define FACEBOOK_SERVER_CHAT					"%s-%s.facebook.com"
#define FACEBOOK_SERVER_LOGIN					"login.facebook.com"
#define FACEBOOK_SERVER_APPS					"apps.facebook.com"

// Limits
#define FACEBOOK_MESSAGE_LIMIT					1024
#define FACEBOOK_MESSAGE_LIMIT_TEXT				"1024"
#define FACEBOOK_MIND_LIMIT						420
#define FACEBOOK_MIND_LIMIT_TEXT				"420"
#define FACEBOOK_TIMEOUTS_LIMIT					5
#define FACEBOOK_GROUP_NAME_LIMIT				100

// Defaults
#define FACEBOOK_MINIMAL_POLL_RATE				10
#define FACEBOOK_DEFAULT_POLL_RATE				24 // in seconds
#define FACEBOOK_MAXIMAL_POLL_RATE				60

#define DEFAULT_FORCE_HTTPS						0
#define DEFAULT_FORCE_HTTPS_CHANNEL				0
#define DEFAULT_SET_MIRANDA_STATUS				0
#define DEFAULT_LOGGING_ENABLE					0
#define DEFAULT_SYSTRAY_NOTIFY					0
#define DEFAULT_DISABLE_STATUS_NOTIFY			0
#define DEFAULT_BIG_AVATARS						0
#define DEFAULT_DISCONNECT_CHAT					0
#define DEFAULT_MAP_STATUSES					0
#define DEFAULT_LOAD_MOBILE						0
#define DEFAULT_CUSTOM_SMILEYS					0

#define DEFAULT_EVENT_NOTIFICATIONS_ENABLE		1
#define DEFAULT_EVENT_FEEDS_ENABLE				1
#define DEFAULT_EVENT_OTHER_ENABLE				1
#define DEFAULT_EVENT_CLIENT_ENABLE				1
#define DEFAULT_EVENT_COLBACK					0x00ffffff
#define DEFAULT_EVENT_COLTEXT					0x00000000
#define DEFAULT_EVENT_TIMEOUT_TYPE				0
#define DEFAULT_EVENT_TIMEOUT					20

// Event flags
#define FACEBOOK_EVENT_CLIENT					0x10000000 // Facebook error or info message
#define FACEBOOK_EVENT_NEWSFEED					0x20000000 // Facebook newsfeed (wall) message
#define FACEBOOK_EVENT_NOTIFICATION				0x40000000 // Facebook new notification
#define FACEBOOK_EVENT_OTHER					0x80000000 // Facebook other event - friend requests/new messages

// Facebook request types // TODO: Provide MS_ and release in FB plugin API?
#define FACEBOOK_REQUEST_LOGIN					100 // connecting physically
#define FACEBOOK_REQUEST_LOGOUT					101 // disconnecting physically
#define FACEBOOK_REQUEST_SETUP_MACHINE			102 // setting machine name
#define FACEBOOK_REQUEST_HOME					103 // getting own name, avatar, ...
#define FACEBOOK_REQUEST_DTSG					104 // getting __fb_dtsg__
#define FACEBOOK_REQUEST_RECONNECT				105 // getting __sequence_num__ and __channel_id__
#define FACEBOOK_REQUEST_VISIBILITY				305 // setting chat visibility

#define FACEBOOK_REQUEST_FEEDS					120 // getting feeds
#define FACEBOOK_REQUEST_NOTIFICATIONS			121 // getting notifications
#define FACEBOOK_REQUEST_LOAD_REQUESTS			122 // getting friend requests

#define FACEBOOK_REQUEST_STATUS_SET				130 // setting my "What's on my mind?"
#define FACEBOOK_REQUEST_SEARCH					140 // searching 
#define FACEBOOK_REQUEST_POKE					150 // sending pokes
#define FACEBOOK_REQUEST_NOTIFICATIONS_READ		151 // marking notifications read

#define FACEBOOK_REQUEST_BUDDY_LIST				200 // getting regular updates (friends online, ...)
#define FACEBOOK_REQUEST_LOAD_FRIENDS			201 // getting list of all friends
#define FACEBOOK_REQUEST_REQUEST_FRIEND			210 // requesting friends
#define FACEBOOK_REQUEST_APPROVE_FRIEND			211 // approving friends
#define FACEBOOK_REQUEST_DELETE_FRIEND			212 // deleting friends
#define FACEBOOK_REQUEST_CANCEL_REQUEST			213 // canceling friends request

#define FACEBOOK_REQUEST_MESSAGE_SEND			300 // sending message
#define FACEBOOK_REQUEST_MESSAGE_SEND2			301	// sending message through inbox
#define FACEBOOK_REQUEST_MESSAGES_RECEIVE		302 // receiving messages
#define FACEBOOK_REQUEST_TYPING_SEND			303 // sending typing notification

#define FACEBOOK_REQUEST_THREAD_INFO			400 // getting thread info
#define FACEBOOK_REQUEST_UNREAD_THREADS			401	// getting unread threads
#define FACEBOOK_REQUEST_UNREAD_MESSAGES		402	// getting unread messages
#define	FACEBOOK_REQUEST_ASYNC					403 // marking messages read and getting other things
#define FACEBOOK_REQUEST_MARK_READ				404 // marking messages read (new)

// Send message types
#define MESSAGE_INBOX							0
#define MESSAGE_MERCURY							1
#define MESSAGE_TID								2
#define MESSAGE_ASYNC							3

// Contact types
#define FACEBOOK_CONTACT_FRIEND					1 // contact that IS on our server list
#define FACEBOOK_CONTACT_NONE					2 // contact that ISN'T on our server list
#define FACEBOOK_CONTACT_REQUEST				3 // contact that we asked for friendship
#define FACEBOOK_CONTACT_APPROVE				4 // contact that is asking us for approval of friendship

typedef struct {
	const char *name;
	const char *id;
} ttype;

// News Feed types
static const ttype feed_types[] = {
	{ LPGEN("Most Recent"), "lf_" }, //h_chr?
	{ LPGEN("Wall Posts"), "app_2915120374" },
	{ LPGEN("Top News"), "h_nor" }, //h
	{ LPGEN("Photos"), "app_2305272732_2392950137" },
	{ LPGEN("Links"), "app_2309869772" },
	{ LPGEN("Apps and Games"), "appsandgames" },
};

// Server types
static const ttype server_types[] = {
	{ LPGEN("Classic website"), "www.facebook.com" },
	{ LPGEN("Mobile website"), "m.facebook.com" },
	{ LPGEN("Smartphone website"), "touch.facebook.com" },
};

// Status privacy types
static const ttype privacy_types[] = {
	{ LPGEN("For everyone"), "80" },
	{ LPGEN("For friends of friends"), "111&audience[0][custom_value]=50" },
	{ LPGEN("For friends"), "40" },
	{ LPGEN("Only for me"), "10" },
};