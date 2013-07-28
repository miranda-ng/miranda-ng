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

// DB keys
#define FACEBOOK_KEY_LOGIN					"Email"
#define FACEBOOK_KEY_ID						"ID"
#define FACEBOOK_KEY_TID					"ThreadID"
#define FACEBOOK_KEY_NAME					"RealName"
#define FACEBOOK_KEY_NICK					"Nick"
#define FACEBOOK_KEY_PASS					"Password"
#define FACEBOOK_KEY_UPD_NAMES				"UpdateNames"
#define FACEBOOK_KEY_DEVICE_ID				"DeviceID"
#define FACEBOOK_KEY_AV_URL					"AvatarURL"
#define FACEBOOK_KEY_DELETED				"Deleted"
#define FACEBOOK_KEY_CONTACT_TYPE			"ContactType"
#define FACEBOOK_KEY_DEF_GROUP				"DefaultGroup"
#define FACEBOOK_KEY_FORCE_HTTPS			"ForceHTTPS"
#define FACEBOOK_KEY_FORCE_HTTPS_CHANNEL    "ForceHTTPSChannel"
#define FACEBOOK_KEY_SET_MIRANDA_STATUS		"SetMirandaStatus"
#define FACEBOOK_KEY_LOGGING_ENABLE			"LoggingEnable"
#define FACEBOOK_KEY_SYSTRAY_NOTIFY			"UseSystrayNotify"
#define FACEBOOK_KEY_DISABLE_STATUS_NOTIFY	"DisableStatusNotify"
#define FACEBOOK_KEY_BIG_AVATARS			"UseBigAvatars"
#define FACEBOOK_KEY_DISCONNECT_CHAT		"DisconnectChatEnable"
#define FACEBOOK_KEY_MAP_STATUSES			"MapStatuses"
#define FACEBOOK_KEY_LOAD_MOBILE			"LoadMobile"
#define FACEBOOK_KEY_CUSTOM_SMILEYS			"CustomSmileys"
#define FACEBOOK_KEY_MESSAGE_ID				"LastMessageId"
#define FACEBOOK_KEY_SERVER_TYPE			"ServerType"
#define FACEBOOK_KEY_LOCAL_TIMESTAMP		"UseLocalTimestamp"
#define FACEBOOK_KEY_PRIVACY_TYPE			"PrivacyType"
#define FACEBOOK_KEY_PLACE					"Place"
#define FACEBOOK_KEY_LAST_WALL				"LastWall"

#define FACEBOOK_KEY_POLL_RATE				"PollRate" // [HIDDEN]
#define FACEBOOK_KEY_TIMEOUTS_LIMIT			"TimeoutsLimit" // [HIDDEN]
#define FACEBOOK_KEY_DISABLE_LOGOUT			"DisableLogout" // [HIDDEN]
#define FACEBOOK_KEY_VALIDATE_RESPONSE		"ValidateResponse" // [HIDDEN] - 0 = standard, 1 = always, 2 = never
#define	FACEBOOK_KEY_LOCALE					"Locale" // [HIDDEN] - en_US, cs_CZ, etc.

#define FACEBOOK_KEY_EVENT_NOTIFICATIONS_ENABLE     "EventNotificationsEnable"
#define FACEBOOK_KEY_EVENT_FEEDS_ENABLE             "EventFeedsEnable"
#define FACEBOOK_KEY_EVENT_OTHER_ENABLE             "EventOtherEnable"
#define FACEBOOK_KEY_EVENT_CLIENT_ENABLE            "EventClientEnable"
#define FACEBOOK_KEY_FEED_TYPE                      "EventFeedsType"

#define FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLBACK    "PopupNotificationsColorBack"
#define FACEBOOK_KEY_EVENT_NOTIFICATIONS_COLTEXT    "PopupNotificationsColorText"
#define FACEBOOK_KEY_EVENT_NOTIFICATIONS_TIMEOUT    "PopupNotificationsTimeout"
#define FACEBOOK_KEY_EVENT_NOTIFICATIONS_DEFAULT    "PopupNotificationsColorDefault"

#define FACEBOOK_KEY_EVENT_FEEDS_COLBACK            "PopupFeedsColorBack"
#define FACEBOOK_KEY_EVENT_FEEDS_COLTEXT            "PopupFeedsColorText"
#define FACEBOOK_KEY_EVENT_FEEDS_TIMEOUT            "PopupFeedsTimeout"
#define FACEBOOK_KEY_EVENT_FEEDS_DEFAULT            "PopupFeedsColorDefault"

#define FACEBOOK_KEY_EVENT_OTHER_COLBACK            "PopupOtherColorBack"
#define FACEBOOK_KEY_EVENT_OTHER_COLTEXT            "PopupOtherColorText"
#define FACEBOOK_KEY_EVENT_OTHER_TIMEOUT            "PopupOtherTimeout"
#define FACEBOOK_KEY_EVENT_OTHER_DEFAULT            "PopupOtherColorDefault"

#define FACEBOOK_KEY_EVENT_CLIENT_COLBACK           "PopupClientColorBack"
#define FACEBOOK_KEY_EVENT_CLIENT_COLTEXT           "PopupClientColorText"
#define FACEBOOK_KEY_EVENT_CLIENT_TIMEOUT           "PopupClientTimeout"
#define FACEBOOK_KEY_EVENT_CLIENT_DEFAULT           "PopupClientColorDefault"
