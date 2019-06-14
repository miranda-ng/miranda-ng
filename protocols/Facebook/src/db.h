/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

#define MODULENAME "Facebook"

// Contact DB keys
#define DBKEY_LOGIN           "Email"
#define DBKEY_ID              "ID"
#define DBKEY_TID             "ThreadID"
#define DBKEY_FIRST_NAME      "FirstName"
#define DBKEY_SECOND_NAME     "SecondName"
#define DBKEY_LAST_NAME       "LastName"
#define DBKEY_NICK            "Nick"
#define DBKEY_USERNAME        "Username"
#define DBKEY_PASS            "Password"
#define DBKEY_DEVICE_ID       "DeviceID"
#define DBKEY_AVATAR          "Avatar"
#define DBKEY_DELETED         "DeletedTS"
#define DBKEY_CONTACT_TYPE    "ContactType"
#define DBKEY_MESSAGE_ID      "LastMessageId"
#define DBKEY_MESSAGE_READ    "LastMsgReadTime"
#define DBKEY_MESSAGE_READERS "MessageReaders"

// Thread specific DB keys 
#define DBKEY_CHAT_CAN_REPLY  "CanReply"
#define DBKEY_CHAT_READ_ONLY  "ReadOnly"
#define DBKEY_CHAT_IS_ARCHIVED "IsArchived"
#define DBKEY_CHAT_IS_SUBSCRIBED "IsSubscribed"

// Contact and account DB keys
#define DBKEY_KEEP_UNREAD			"KeepUnread"	// (byte) 1 = don't mark messages as read on server (works globally or per contact)

// Account DB keys
#define DBKEY_DEF_GROUP				"DefaultGroup"
#define DBKEY_SET_MIRANDA_STATUS		"SetMirandaStatus"
#define DBKEY_SYSTRAY_NOTIFY			"UseSystrayNotify"
#define DBKEY_DISABLE_STATUS_NOTIFY	"DisableStatusNotify"
#define DBKEY_BIG_AVATARS			"UseBigAvatars"
#define DBKEY_DISCONNECT_CHAT		"DisconnectChatEnable"
#define DBKEY_MAP_STATUSES			"MapStatuses"
#define DBKEY_CUSTOM_SMILEYS			"CustomSmileys"
#define DBKEY_SERVER_TYPE			"ServerType"
#define DBKEY_PRIVACY_TYPE			"PrivacyType"
#define DBKEY_PLACE					"Place"
#define DBKEY_LAST_WALL				"LastWall"
#define DBKEY_LOAD_PAGES				"LoadPages"
#define DBKEY_FILTER_ADS				"FilterAds"
#define DBKEY_LOGON_TS				"LogonTS"
#define DBKEY_LAST_ACTION_TS			"LastActionTS"
#define DBKEY_MESSAGES_ON_OPEN		"MessagesOnOpen"
#define DBKEY_MESSAGES_ON_OPEN_COUNT	"MessagesOnOpenCount"
#define DBKEY_HIDE_CHATS				"HideChats"
#define DBKEY_ENABLE_CHATS			"EnableChat"
#define DBKEY_JOIN_EXISTING_CHATS	"JoinExistingChats"
#define DBKEY_NOTIFICATIONS_CHATROOM	"NotificationsChatroom"
#define DBKEY_NAME_AS_NICK			"NameAsNick"
#define DBKEY_LOAD_ALL_CONTACTS		"LoadAllContacts"
#define DBKEY_PAGES_ALWAYS_ONLINE	"PagesAlwaysOnline"
#define DBKEY_TYPING_WHEN_INVISIBLE	"TypingWhenInvisible"

// Account DB keys - notifications
#define DBKEY_EVENT_NOTIFICATIONS_ENABLE	"EventNotificationsEnable"
#define DBKEY_EVENT_FEEDS_ENABLE			"EventFeedsEnable"
#define DBKEY_EVENT_FRIENDSHIP_ENABLE	"EventFriendshipEnable"
#define DBKEY_EVENT_TICKER_ENABLE		"EventTickerEnable"
#define DBKEY_EVENT_ON_THIS_DAY_ENABLE	"EventMemoriesEnable"
#define DBKEY_FEED_TYPE					"EventFeedsType"

// Hidden account DB keys (can't be changed through GUI)
#define DBKEY_POLL_RATE				"PollRate"					// [HIDDEN] - (byte)
#define DBKEY_TIMEOUTS_LIMIT			"TimeoutsLimit"				// [HIDDEN] - (byte)
#define DBKEY_LOCALE					"Locale"					// [HIDDEN] - (string) en_US, cs_CZ, etc. (requires restart to apply)
#define DBKEY_NASEEMS_SPAM_MODE		"NaseemsSpamMode"			// [HIDDEN] - (byte) 1 = don't load messages sent from other instances (e.g., browser) - known as "Naseem's spam mode"
#define DBKEY_OPEN_URL_BROWSER		"OpenUrlBrowser"			// [HIDDEN] - (unicode) = absolute path to browser to open url links with
#define DBKEY_SEND_MESSAGE_TRIES		"SendMessageTries"			// [HIDDEN] - (byte) = number of tries to send message, default=1, min=1, max=5
#define DBKEY_PAGE_PREFIX			"PagePrefix"				// [HIDDEN] - (unicode) = prefix for name of "page" contacts (requires restart to apply), default is emoji :page_facing_up: (written as unicode char)
