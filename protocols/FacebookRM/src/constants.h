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

// Product management
#define FACEBOOK_NAME							"Facebook"
#define FACEBOOK_URL_HOMEPAGE					"https://www.facebook.com"
#define FACEBOOK_URL_REQUESTS					"https://www.facebook.com/n/?reqs.php"
#define FACEBOOK_URL_MESSAGES					"https://www.facebook.com/n/?inbox"
#define FACEBOOK_URL_NOTIFICATIONS				"https://www.facebook.com/n/?notifications"
#define FACEBOOK_URL_PROFILE					"https://www.facebook.com/profile.php?id="
#define FACEBOOK_URL_GROUP						"https://www.facebook.com/n/?home.php&sk=group_"
#define FACEBOOK_URL_PICTURE					"https://graph.facebook.com/%s/picture"
#define FACEBOOK_URL_CONVERSATION				"https://www.facebook.com/messages/t/"
//#define FACEBOOK_URL_STICKER					"https://www.facebook.com/stickers/asset/?sticker_id=%s&image_type=BestEffortImage"

// Connection
#define FACEBOOK_SERVER_REGULAR              "www.facebook.com"
#define FACEBOOK_SERVER_MBASIC               "mbasic.facebook.com"
#define FACEBOOK_SERVER_MOBILE               "m.facebook.com"
#define FACEBOOK_SERVER_CHAT                 "%s-%s.facebook.com"
#define FACEBOOK_SERVER_LOGIN                "www.facebook.com"
#define FACEBOOK_SERVER_APPS                 "apps.facebook.com"
#define FACEBOOK_SERVER_DOMAIN               "facebook.com"

// Facebook clients
#define FACEBOOK_CLIENT_WEB                  L"Facebook (website)"
#define FACEBOOK_CLIENT_MOBILE               L"Facebook (mobile)"
#define FACEBOOK_CLIENT_OTHER                L"Facebook (other)"
#define FACEBOOK_CLIENT_APP                  L"Facebook App"
#define FACEBOOK_CLIENT_MESSENGER            L"Facebook Messenger"

// Various constants
#define FACEBOOK_NOTIFICATIONS_CHATROOM      "_notifications"
#define FACEBOOK_CHATROOM_NAMES_COUNT        3 // number of participant names to use for chatrooms without specific name (on website it's 2)
#define FACEBOOK_NOTIFICATIONS_LOAD_COUNT    20 // number of last notifications to load on login to notify
#define MANUALLY_TRIGGERED					 (void*)1 // to use as thread parameter to notify this action was activated by user, so we should show info popups

// Limits
#define FACEBOOK_MESSAGE_LIMIT					200000 // this is guessed limit, in reality it is bigger
#define FACEBOOK_MESSAGE_LIMIT_TEXT				"200000"
#define FACEBOOK_MIND_LIMIT						63206 // this should be correct maximal limit
#define FACEBOOK_MIND_LIMIT_TEXT				"63206"
#define FACEBOOK_TIMEOUTS_LIMIT					3
#define FACEBOOK_GROUP_NAME_LIMIT				100
#define FACEBOOK_MESSAGES_ON_OPEN_LIMIT			99
#define FACEBOOK_TYPING_TIME					60
#define FACEBOOK_IGNORE_COUNTER_LIMIT			100 // how many consequent requests it should keep info about duplicit message ids
#define FACEBOOK_PING_TIME						600 // every 10 minutes send activity_ping (it is just random/guessed value)

#define MAX_NEWSFEED_LEN						500
#define MAX_LINK_DESCRIPTION_LEN				200

#define TEXT_ELLIPSIS							"\xE2\x80\xA6" // unicode ellipsis
#define TEXT_EMOJI_LINK							"\xF0\x9F\x94\x97" // emoji :link:
#define TEXT_EMOJI_CLOCK						"\xF0\x9F\x95\x92" // emoji :clock3:
#define TEXT_EMOJI_PAGE							"\xF0\x9F\x93\x84" // emoji :page_facing_up:

// Defaults
#define FACEBOOK_MINIMAL_POLL_RATE				10
#define FACEBOOK_DEFAULT_POLL_RATE				24 // in seconds
#define FACEBOOK_MAXIMAL_POLL_RATE				60

#define DEFAULT_SET_MIRANDA_STATUS				0
#define DEFAULT_LOGGING_ENABLE					0
#define DEFAULT_SYSTRAY_NOTIFY					0
#define DEFAULT_DISABLE_STATUS_NOTIFY			0
#define DEFAULT_BIG_AVATARS						0
#define DEFAULT_DISCONNECT_CHAT					0
#define DEFAULT_MAP_STATUSES					0
#define DEFAULT_CUSTOM_SMILEYS					0
#define DEFAULT_LOAD_PAGES						0
#define DEFAULT_KEEP_UNREAD						0
#define DEFAULT_FILTER_ADS						0
#define DEFAULT_MESSAGES_ON_OPEN				0
#define DEFAULT_MESSAGES_ON_OPEN_COUNT			10
#define DEFAULT_HIDE_CHATS						0
#define DEFAULT_ENABLE_CHATS					1
#define DEFAULT_JOIN_EXISTING_CHATS				1
#define DEFAULT_NOTIFICATIONS_CHATROOM			0
#define DEFAULT_NAME_AS_NICK					1
#define DEFAULT_LOAD_ALL_CONTACTS				0
#define DEFAULT_PAGES_ALWAYS_ONLINE				1
#define DEFAULT_TYPING_WHEN_INVISIBLE			1

#define DEFAULT_EVENT_NOTIFICATIONS_ENABLE		1
#define DEFAULT_EVENT_FEEDS_ENABLE				0
#define DEFAULT_EVENT_FRIENDSHIP_ENABLE			1
#define DEFAULT_EVENT_TICKER_ENABLE				0
#define DEFAULT_EVENT_ON_THIS_DAY_ENABLE		0

// Send message return values
#define SEND_MESSAGE_OK							0
#define SEND_MESSAGE_ERROR						1
#define SEND_MESSAGE_CANCEL						-1

// Event types
#define FACEBOOK_EVENTTYPE_CALL					10010

enum ContactType {
	CONTACT_FRIEND	= 1,	// contact that IS on our server list
	CONTACT_NONE	= 2,	// contact that ISN'T on our server list
	CONTACT_REQUEST	= 3,	// contact that we asked for friendship
	CONTACT_APPROVE	= 4,	// contact that is asking us for approval of friendship
	CONTACT_PAGE	= 5		// contact is Facebook page
};

enum ClientType {
	CLIENT_WEB		 = 1,	// Facebook website
	CLIENT_APP		 = 2,	// Facebook mobile application
	CLIENT_MESSENGER = 3,	// Facebook Messenger application
	CLIENT_OTHER	 = 4,	// Facebook over XMPP
	CLIENT_MOBILE	 = 5	// Facebook on unknown mobile client (can't be determined for offline contacts)
};

// TODO: Rework to "Events" and add also typing, seen, contact status, chat switch, etc. then have JSON parser only for parsing and a single place for processing updates
// Each type then could have also separate class representing it. And when giving it in some list, make it map of arrays with key=thread_id to group it by contact/chat, to have it more effective
// Use also for Poke and such events...
enum MessageType {
	MESSAGE			= 1,
	PHONE_CALL		= 2,
	VIDEO_CALL		= 3,
	ADMIN_TEXT		= 4,
	SUBSCRIBE		= 5,
	UNSUBSCRIBE		= 6,
	THREAD_NAME		= 7,
	THREAD_IMAGE	= 8,
};

enum EventType {
	EVENT_CLIENT		= 1, // Client error
	EVENT_NEWSFEED		= 2, // Newsfeed (wall) message
	EVENT_NOTIFICATION	= 3, // New notification
	EVENT_OTHER			= 4, // Other event (poke sent, status update, ...)
	EVENT_FRIENDSHIP	= 5, // Friendship event
	EVENT_TICKER		= 6, // Ticker message
	EVENT_ON_THIS_DAY	= 7, // On this day post (memories)
};

enum ParticipantRole {
	ROLE_ME = 0,
	ROLE_FRIEND = 1,
	ROLE_NONE = 2
};

typedef struct {
	const char *name;
	const char *id;
} ttype;

// News Feed types
static const ttype feed_types[] = {
	{ LPGEN("Top News"), "h_nor" },
	{ LPGEN("Most Recent"), "h_chr" },
	{ LPGEN("Pages"), "pages" },
	//{ LPGEN("Apps and Games"), "FEED_FILTER_KEY_APPS_AND_GAMES" },

	//{ LPGEN("Wall Posts"), "app_2915120374" },	
	//{ LPGEN("Photos"), "app_2305272732_2392950137" },
	//{ LPGEN("Links"), "app_2309869772" },
};

// Server types
static const ttype server_types[] = {
	{ LPGEN("Classic website"), "www.facebook.com" },
	{ LPGEN("Mobile website"), "m.facebook.com" },
	{ LPGEN("Smartphone website"), "touch.facebook.com" },
};

// Status privacy types
static const ttype privacy_types[] = {
	{ LPGEN("Public"), "80" },
	{ LPGEN("Friends of friends"), "111&audience[0][custom_value]=50" },
	{ LPGEN("Friends"), "40" },
	{ LPGEN("Friends except acquaintances"), "127" },	
	{ LPGEN("Only me"), "10" },
};