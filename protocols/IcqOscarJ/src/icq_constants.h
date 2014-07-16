// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Global constants and default settings are defined here
// -----------------------------------------------------------------------------
// Most of the protocol constants follow the naming conventions of the
// Oscar documentation at http://iserverd.khstu.ru/oscar/index.html
// BIG THANKS to Alexandr for maintaining this site and to everyone
// in the ICQ devel community who have helped to collect the data.

#ifndef __ICQ_CONSTANTS_H
#define __ICQ_CONSTANTS_H

/* Static icon indexes */
#define ISI_AUTH_REQUEST            0
#define ISI_AUTH_GRANT              1
#define ISI_AUTH_REVOKE             2
#define ISI_ADD_TO_SERVLIST         3

/* Contact menu item indexes */
#define ICMI_AUTH_REQUEST           0
#define ICMI_AUTH_GRANT             1
#define ICMI_AUTH_REVOKE            2
#define ICMI_ADD_TO_SERVLIST        3
#define ICMI_XSTATUS_DETAILS        4
#define ICMI_OPEN_PROFILE           5

/* Some default settings */
#define DEFAULT_SERVER_PORT         5190
#define DEFAULT_SERVER_PORT_SSL     443
#define DEFAULT_SERVER_HOST         "login.icq.com"
#define DEFAULT_SERVER_HOST_SSL     "slogin.icq.com"
#define DEFAULT_SS_ENABLED          1
#define DEFAULT_SS_ADDSERVER        1
#define DEFAULT_SS_LOAD             0
#define DEFAULT_SS_STORE            1
#define DEFAULT_SS_GROUP            "General"

#define DEFAULT_SECURE_LOGIN        1
#define DEFAULT_SECURE_CONNECTION   1
#define DEFAULT_LEGACY_FIX          0
#define DEFAULT_KEEPALIVE_ENABLED   1
#define DEFAULT_AIM_ENABLED         1
#define DEFAULT_UTF_ENABLED         2 // everything unicode is default
#define DEFAULT_ANSI_CODEPAGE       CP_ACP
#define DEFAULT_DCMSG_ENABLED       1 // passive dc messaging is default
#define DEFAULT_TEMPVIS_ENABLED     1 // temporary visible is enabled by default
#define DEFAULT_MTN_ENABLED         1
#define DEFAULT_AVATARS_ENABLED     1
#define DEFAULT_LOAD_AVATARS        1
#define DEFAULT_BIGGER_AVATARS      0
#define DEFAULT_AVATARS_CHECK       1
#define DEFAULT_XSTATUS_ENABLED     1
#define DEFAULT_XSTATUS_AUTO        1
#define DEFAULT_XSTATUS_RESET       0
#define DEFAULT_MOODS_ENABLED       1
#define DEFAULT_KILLSPAM_ENABLED    1

#define DEFAULT_SLOWSEND            1

#define DEFAULT_POPUPS_ENABLED      1
#define DEFAULT_SPAM_POPUPS_ENABLED 1
#define DEFAULT_LOG_POPUPS_ENABLED  1
#define DEFAULT_POPUPS_SYS_ICONS    1
#define DEFAULT_LOG0_TEXT_COLORS    RGB(0,0,0)  // LOG_NOTE
#define DEFAULT_LOG0_BACK_COLORS    RGB(255,255,255)
#define DEFAULT_LOG0_TIMEOUT        0
#define DEFAULT_LOG1_TEXT_COLORS    RGB(0,0,0)  // LOG_WARNING
#define DEFAULT_LOG1_BACK_COLORS    RGB(255,255,255)
#define DEFAULT_LOG1_TIMEOUT        0
#define DEFAULT_LOG2_TEXT_COLORS    RGB(0,0,0)  // LOG_ERROR
#define DEFAULT_LOG2_BACK_COLORS    RGB(255,255,255)
#define DEFAULT_LOG2_TIMEOUT        0
#define DEFAULT_LOG3_TEXT_COLORS    RGB(0,0,0)  // LOG_FATAL
#define DEFAULT_LOG3_BACK_COLORS    RGB(255,255,255)
#define DEFAULT_LOG3_TIMEOUT        0
#define DEFAULT_SPAM_TEXT_COLORS    RGB(193,0,38)
#define DEFAULT_SPAM_BACK_COLORS    RGB(213,209,208)
#define DEFAULT_SPAM_TIMEOUT        0
#define DEFAULT_POPUPS_WIN_COLORS   0
#define DEFAULT_POPUPS_DEF_COLORS   (BYTE)!DEFAULT_POPUPS_WIN_COLORS

/* Database setting names */
#define DBSETTING_CAPABILITIES      "caps"
// Contact's server-list items
#define DBSETTING_SERVLIST_ID       "ServerId"
#define DBSETTING_SERVLIST_GROUP    "SrvGroupId"
#define DBSETTING_SERVLIST_PERMIT   "SrvPermitId"
#define DBSETTING_SERVLIST_DENY     "SrvDenyId"
#define DBSETTING_SERVLIST_IGNORE   "SrvIgnoreId"
// Owner's server-list items
#define DBSETTING_SERVLIST_PRIVACY  "SrvVisibilityID"
#define DBSETTING_SERVLIST_PHOTO    "SrvPhotoID"
#define DBSETTING_SERVLIST_AVATAR   "SrvAvatarID"
#define DBSETTING_SERVLIST_METAINFO "SrvMetaInfoID"
#define DBSETTING_SERVLIST_UNHANDLED "SrvUnhandledIDList"
// Contact's data from server-list
#define DBSETTING_SERVLIST_DATA     "ServerData"
// User Details
#define DBSETTING_METAINFO_TOKEN    "MetaInfoToken"
#define DBSETTING_METAINFO_TIME     "MetaInfoTime"
#define DBSETTING_METAINFO_SAVED    "InfoTS"
// Status Note & Mood
#define DBSETTING_STATUS_NOTE       "StatusNote"
#define DBSETTING_STATUS_NOTE_TIME  "StatusNoteTS"
#define DBSETTING_STATUS_MOOD       "StatusMood"
// Custom Status
#define DBSETTING_XSTATUS_ID        "XStatusId"
#define DBSETTING_XSTATUS_NAME      "XStatusName"
#define DBSETTING_XSTATUS_MSG       "XStatusMsg"


// Status FLAGS (used to determine status of other users)
#define ICQ_STATUSF_ONLINE          0x0000
#define ICQ_STATUSF_AWAY            0x0001
#define ICQ_STATUSF_DND             0x0002
#define ICQ_STATUSF_NA              0x0004
#define ICQ_STATUSF_OCCUPIED        0x0010
#define ICQ_STATUSF_FFC             0x0020
#define ICQ_STATUSF_INVISIBLE       0x0100

// Status values (used to set own status)
#define ICQ_STATUS_ONLINE           0x0000
#define ICQ_STATUS_AWAY             0x0001
#define ICQ_STATUS_NA               0x0005
#define ICQ_STATUS_OCCUPIED         0x0011
#define ICQ_STATUS_DND              0x0013
#define ICQ_STATUS_FFC              0x0020
#define ICQ_STATUS_INVISIBLE        0x0100

#define STATUS_WEBAWARE             0x0001 // Status webaware flag
#define STATUS_SHOWIP               0x0002 // Status show ip flag
#define STATUS_BIRTHDAY             0x0008 // User birthday flag
#define STATUS_WEBFRONT             0x0020 // User active webfront flag
#define STATUS_DCDISABLED           0x0100 // Direct connection not supported
#define STATUS_DCAUTH               0x1000 // Direct connection upon authorization
#define STATUS_DCCONT               0x2000 // DC only with contact users



// Typing notification statuses
#define MTN_FINISHED                0x0000
#define MTN_TYPED                   0x0001
#define MTN_BEGUN                   0x0002
#define MTN_WINDOW_CLOSED           0x000F



// Ascii Capability IDs
#define CAP_RTFMSGS                 "{97B12751-243C-4334-AD22-D6ABF73F1492}"
#define CAP_UTF8MSGS                "{0946134E-4C7F-11D1-8222-444553540000}"

// Binary Capability Sizes
#define BINARY_CAP_SIZE             16
#define BINARY_SHORT_CAP_SIZE       2

// Binary Capability IDs
#define CAP_SRV_RELAY               0x09, 0x46, 0x13, 0x49, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define CAP_UTF                     0x09, 0x46, 0x13, 0x4e, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define CAP_RTF                     0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34, 0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x92
#define CAP_CONTACTS                0x09, 0x46, 0x13, 0x4b, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define CAP_TYPING                  0x56, 0x3f, 0xc8, 0x09, 0x0b, 0x6f, 0x41, 0xbd, 0x9f, 0x79, 0x42, 0x26, 0x09, 0xdf, 0xa2, 0xf3
#define CAP_ICQDIRECT               0x09, 0x46, 0x13, 0x44, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00
#define CAP_XTRAZ                   0x1A, 0x09, 0x3C, 0x6C, 0xD7, 0xFD, 0x4E, 0xC5, 0x9D, 0x51, 0xA6, 0x47, 0x4E, 0x34, 0xF5, 0xA0
#define CAP_OSCAR_FILE              0x09, 0x46, 0x13, 0x43, 0x4C, 0x7F, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00

// Miranda IM Capability bitmask
#define CAPF_SRV_RELAY              0x00000001
#define CAPF_UTF                    0x00000002
#define CAPF_RTF                    0x00000004
#define CAPF_CONTACTS               0x00000010
#define CAPF_TYPING                 0x00000020
#define CAPF_ICQDIRECT              0x00000080
#define CAPF_XTRAZ                  0x00000100
#define CAPF_OSCAR_FILE             0x00000400
#define CAPF_STATUS_MESSAGES        0x10000000
#define CAPF_STATUS_MOOD            0x40000000
#define CAPF_XSTATUS                0x80000000


// Message Capability IDs
#define MCAP_SRV_RELAY_FMT_s        0x09461349, 0x4c7f11d1, 0x82224445, 0x53540000
#define MCAP_REVERSE_DC_REQ_s       0x09461344, 0x4c7f11d1, 0x82224445, 0x53540000
#define MCAP_FILE_TRANSFER_s        0x09461343, 0x4c7f11d1, 0x82224445, 0x53540000
#define MCAP_CONTACTS_s             0x0946134b, 0x4c7f11d1, 0x82224445, 0x53540000

// Plugin Type GUIDs
#define PSIG_MESSAGE_s              0x00000000, 0x00000000, 0x00000000, 0x00000000
#define PSIG_INFO_PLUGIN_s          0xa0e93f37, 0x4fe9d311, 0xbcd20004, 0xac96dd96
#define PSIG_STATUS_PLUGIN_s        0x10cf40d1, 0x4fe9d311, 0xbcd20004, 0xac96dd96

// Plugin Message GUIDs
#define PMSG_QUERY_INFO_s           0xF002BF71, 0x4371D311, 0x8DD20010, 0x4B06462E
#define PMSG_QUERY_STATUS_s         0x10180670, 0x5471D311, 0x8DD20010, 0x4B06462E



// Message types
#define MTYPE_PLAIN                 0x01 // Plain text (simple) message
#define MTYPE_CHAT                  0x02 // Chat request message
#define MTYPE_FILEREQ               0x03 // File request / file ok message
#define MTYPE_URL                   0x04 // URL message (0xFE formatted)
#define MTYPE_AUTHREQ               0x06 // Authorization request message (0xFE formatted)
#define MTYPE_AUTHDENY              0x07 // Authorization denied message (0xFE formatted)
#define MTYPE_AUTHOK                0x08 // Authorization given message (empty)
#define MTYPE_SERVER                0x09 // Message from OSCAR server (0xFE formatted)
#define MTYPE_ADDED                 0x0C // "You-were-added" message (0xFE formatted)
#define MTYPE_WWP                   0x0D // Web pager message (0xFE formatted)
#define MTYPE_EEXPRESS              0x0E // Email express message (0xFE formatted)
#define MTYPE_CONTACTS              0x13 // Contact list message
#define MTYPE_PLUGIN                0x1A // Plugin message described by text string
#define MTYPE_AUTOONLINE            0xE7 // Auto online message (internal only)
#define MTYPE_AUTOAWAY              0xE8 // Auto away message
#define MTYPE_AUTOBUSY              0xE9 // Auto occupied message
#define MTYPE_AUTONA                0xEA // Auto not available message
#define MTYPE_AUTODND               0xEB // Auto do not disturb message
#define MTYPE_AUTOFFC               0xEC // Auto free for chat message
// Internal Message types
#define MTYPE_UNKNOWN               0x00 // Unknown message

#define MTYPE_GREETINGCARD          0x101 // Greeting Card
#define MTYPE_REQUESTCONTACTS       0x102 // Request for Contacts
#define MTYPE_MESSAGE               0x103 // Message+
#define MTYPE_STATUSMSGEXT          0x104 // StatusMsgExt (2003b)
#define MTYPE_SMS_MESSAGE           0x110 // SMS message from Mobile
#define MTYPE_SCRIPT_INVITATION     0x201 // Xtraz Invitation
#define MTYPE_SCRIPT_DATA           0x202 // Xtraz Message
#define MTYPE_SCRIPT_NOTIFY         0x208 // Xtraz Response
#define MTYPE_REVERSE_REQUEST       0x401 // Reverse DC request

// Message Plugin Type GUIDs
#define MGTYPE_MESSAGE_s            0xBE6B7305, 0x0FC2104F, 0xA6DE4DB1, 0xE3564B0E
#define MGTYPE_STATUSMSGEXT_s       0x811a18bc, 0x0e6c1847, 0xa5916f18, 0xdcc76f1a
#define MGTYPE_FILE_s               0xF02D12D9, 0x3091D311, 0x8DD70010, 0x4B06462E
#define MGTYPE_WEBURL_s             0x371C5872, 0xE987D411, 0xA4C100D0, 0xB759B1D9
#define MGTYPE_CONTACTS_s           0x2A0E7D46, 0x7676D411, 0xBCE60004, 0xAC961EA6
#define MGTYPE_GREETING_CARD_s      0x01E53B48, 0x2AE4D111, 0xB6790060, 0x97E1E294
#define MGTYPE_CHAT_s               0xBFF720B2, 0x378ED411, 0xBD280004, 0xAC96D905
#define MGTYPE_SMS_MESSAGE_s        0x0e28f600, 0x11e7d311, 0xbcf30004, 0xac969dc2
#define MGTYPE_XTRAZ_SCRIPT_s       0x3b60b3ef, 0xd82a6c45, 0xa4e09c5a, 0x5e67e865

// Message Plugin Sub-Type IDs
#define MGTYPE_STANDARD_SEND        0x0000
#define MGTYPE_CONTACTS_REQUEST     0x0002
#define MGTYPE_SCRIPT_INVITATION    0x0001
#define MGTYPE_SCRIPT_DATA          0x0002
#define MGTYPE_SCRIPT_USER_REMOVE   0x0004
#define MGTYPE_SCRIPT_NOTIFY        0x0008
#define MGTYPE_UNDEFINED            0xFFFF



/* Channels */
#define ICQ_LOGIN_CHAN              0x01
#define ICQ_DATA_CHAN               0x02
#define ICQ_ERROR_CHAN              0x03
#define ICQ_CLOSE_CHAN              0x04
#define ICQ_PING_CHAN               0x05

/* Families */
#define ICQ_SERVICE_FAMILY          0x0001
#define ICQ_LOCATION_FAMILY         0x0002
#define ICQ_BUDDY_FAMILY            0x0003
#define ICQ_MSG_FAMILY              0x0004
#define ICQ_BOS_FAMILY              0x0009
#define ICQ_LOOKUP_FAMILY           0x000a
#define ICQ_STATS_FAMILY            0x000b
#define ICQ_CHAT_NAVIGATION_FAMILY  0x000d
#define ICQ_CHAT_FAMILY             0x000e
#define ICQ_AVATAR_FAMILY           0x0010
#define ICQ_LISTS_FAMILY            0x0013
#define ICQ_EXTENSIONS_FAMILY       0x0015
#define ICQ_AUTHORIZATION_FAMILY    0x0017
#define ICQ_DIRECTORY_FAMILY        0x0025

/* Subtypes for Service Family 0x0001 */
#define ICQ_ERROR                   0x0001
#define ICQ_CLIENT_READY            0x0002
#define ICQ_SERVER_READY            0x0003
#define ICQ_CLIENT_NEW_SERVICE      0x0004
#define ICQ_SERVER_REDIRECT_SERVICE 0x0005
#define ICQ_CLIENT_REQ_RATE_INFO    0x0006
#define ICQ_SERVER_RATE_INFO        0x0007
#define ICQ_CLIENT_RATE_ACK         0x0008
#define ICQ_SERVER_RATE_CHANGE      0x000a
#define ICQ_SERVER_PAUSE            0x000b
#define ICQ_CLIENT_PAUSE_ACK        0x000c
#define ICQ_SERVER_RESUME           0x000d
#define ICQ_CLIENT_REQINFO          0x000e
#define ICQ_SERVER_NAME_INFO        0x000f
#define ICQ_SERVER_EVIL_NOTICE      0x0010
#define ICQ_CLIENT_SET_IDLE         0x0011
#define ICQ_SERVER_MIGRATIONREQ     0x0012
#define ICQ_SERVER_MOTD             0x0013
#define ICQ_CLIENT_FAMILIES         0x0017
#define ICQ_SERVER_FAMILIES2        0x0018
#define ICQ_CLIENT_SET_STATUS       0x001e
#define ICQ_SERVER_EXTSTATUS        0x0021

/* Subtypes for Location Family 0x0002 */
#define ICQ_LOCATION_CLI_REQ_RIGHTS 0x0002
#define ICQ_LOCATION_RIGHTS_REPLY   0x0003
#define ICQ_LOCATION_SET_USER_INFO  0x0004
#define ICQ_LOCATION_REQ_USER_INFO  0x0005
#define ICQ_LOCATION_USR_INFO_REPLY 0x0006
#define ICQ_LOCATION_QRY_USER_INFO  0x0015

/* Subtypes for Buddy Family 0x0003 */
#define ICQ_USER_CLI_REQBUDDY       0x0002
#define ICQ_USER_SRV_REPLYBUDDY     0x0003
#define ICQ_USER_ADDTOLIST          0x0004  /* deprecated */
#define ICQ_USER_REMOVEFROMLIST     0x0005  /* deprecated */
#define ICQ_USER_NOTIFY_REJECTED    0x000a
#define ICQ_USER_ONLINE             0x000b
#define ICQ_USER_OFFLINE            0x000c
#define ICQ_USER_ADDTOTEMPLIST      0x000f
#define ICQ_USER_REMOVEFROMTEMPLIST 0x0010

/* Subtypes for Message Family 0x0004 */
#define ICQ_MSG_SRV_ERROR           0x0001
#define ICQ_MSG_CLI_SETPARAMS       0x0002
#define ICQ_MSG_CLI_RESETPARAMS     0x0003
#define ICQ_MSG_CLI_REQICBM         0x0004
#define ICQ_MSG_SRV_REPLYICBM       0x0005
#define ICQ_MSG_SRV_SEND            0x0006
#define ICQ_MSG_SRV_RECV            0x0007
#define ICQ_MSG_SRV_MISSED_MESSAGE  0x000A
#define ICQ_MSG_RESPONSE            0x000B
#define ICQ_MSG_SRV_ACK             0x000C
#define ICQ_MSG_CLI_REQ_OFFLINE     0x0010
#define ICQ_MSG_MTN                 0x0014
#define ICQ_MSG_SRV_OFFLINE_REPLY   0x0017

/* Subtypes for Privacy Family 0x0009 */
#define ICQ_PRIVACY_REQ_RIGHTS      0x0002
#define ICQ_PRIVACY_RIGHTS_REPLY    0x0003
#define ICQ_CLI_ADDVISIBLE          0x0005
#define ICQ_CLI_REMOVEVISIBLE       0x0006
#define ICQ_CLI_ADDINVISIBLE        0x0007
#define ICQ_CLI_REMOVEINVISIBLE     0x0008
#define ICQ_PRIVACY_SERVICE_ERROR   0x0009
#define ICQ_CLI_ADDTEMPVISIBLE      0x000A
#define ICQ_CLI_REMOVETEMPVISIBLE   0x000B

/* Subtypes for Lookup Family 0x000a */
#define ICQ_LOOKUP_REQUEST          0x0002
#define ICQ_LOOKUP_EMAIL_REPLY      0x0003

/* Subtypes for Stats Family 0x000b */
#define ICQ_STATS_MINREPORTINTERVAL 0x0002

/* Subtypes for Avatar Family 0x0010 */
#define ICQ_AVATAR_ERROR            0x0001
#define ICQ_AVATAR_UPLOAD_REQUEST   0x0002
#define ICQ_AVATAR_UPLOAD_ACK       0x0003
#define ICQ_AVATAR_GET_REQUEST      0x0006
#define ICQ_AVATAR_GET_REPLY        0x0007

/* Subtypes for Server Lists Family 0x0013 */
#define ICQ_LISTS_ERROR             0x0001
#define ICQ_LISTS_CLI_REQLISTS      0x0002
#define ICQ_LISTS_SRV_REPLYLISTS    0x0003
#define ICQ_LISTS_CLI_REQUEST       0x0004
#define ICQ_LISTS_CLI_CHECK         0x0005
#define ICQ_LISTS_LIST              0x0006
#define ICQ_LISTS_GOTLIST           0x0007
#define ICQ_LISTS_ADDTOLIST         0x0008
#define ICQ_LISTS_UPDATEGROUP       0x0009
#define ICQ_LISTS_REMOVEFROMLIST    0x000A
#define ICQ_LISTS_ACK               0x000E
#define ICQ_LISTS_UPTODATE          0x000F
#define ICQ_LISTS_CLI_MODIFYSTART   0x0011
#define ICQ_LISTS_CLI_MODIFYEND     0x0012
#define ICQ_LISTS_GRANTAUTH         0x0014
#define ICQ_LISTS_AUTHGRANTED       0x0015
#define ICQ_LISTS_REVOKEAUTH        0x0016
#define ICQ_LISTS_REQUESTAUTH       0x0018
#define ICQ_LISTS_AUTHREQUEST       0x0019
#define ICQ_LISTS_CLI_AUTHRESPONSE  0x001A
#define ICQ_LISTS_SRV_AUTHRESPONSE  0x001B
#define ICQ_LISTS_YOUWEREADDED      0x001C

/* Subtypes for ICQ Extensions Family 0x0015 */
#define ICQ_META_ERROR              0x0001
#define ICQ_META_CLI_REQUEST        0x0002
#define ICQ_META_SRV_REPLY          0x0003
#define ICQ_META_SRV_UPDATE         0x0004

/* Subtypes for Authorization Family 0x0017 */
#define ICQ_SIGNON_ERROR            0x0001
#define ICQ_SIGNON_LOGIN_REQUEST    0x0002
#define ICQ_SIGNON_LOGIN_REPLY      0x0003
#define ICQ_SIGNON_REGISTRATION_REQ 0x0004
#define ICQ_SIGNON_NEW_UIN          0x0005
#define ICQ_SIGNON_AUTH_REQUEST     0x0006
#define ICQ_SIGNON_AUTH_KEY         0x0007
#define ICQ_SIGNON_REQUEST_IMAGE    0x000C
#define ICQ_SIGNON_REG_AUTH_IMAGE   0x000D

// Class constants
#define CLASS_UNCONFIRMED           0x0001
#define CLASS_ADMINISTRATOR         0x0002
#define CLASS_AOL                   0x0004
#define CLASS_COMMERCIAL            0x0008
#define CLASS_FREE                  0x0010
#define CLASS_AWAY                  0x0020
#define CLASS_ICQ                   0x0040
#define CLASS_WIRELESS              0x0080
#define CLASS_FORWARDING            0x0200
#define CLASS_BOT                   0x0400

// Reply types for SNAC 15/02 & 15/03
#define CLI_DELETE_OFFLINE_MSGS_REQ 0x003E
#define CLI_META_INFO_REQ           0x07D0
#define SRV_META_INFO_REPLY         0x07DA

// Reply subtypes for SNAC 15/02 & 15/03
#define META_PROCESSING_ERROR       0x0001 // Meta processing error server reply
#define META_SMS_DELIVERY_RECEIPT   0x0096 // Server SMS response (delivery receipt)
#define META_SET_PASSWORD_ACK       0x00AA // Set user password server ack
#define META_UNREGISTER_ACK         0x00B4 // Unregister account server ack
#define META_BASIC_USERINFO         0x00C8 // User basic info reply
#define META_WORK_USERINFO          0x00D2 // User work info reply
#define META_MORE_USERINFO          0x00DC // User more info reply
#define META_NOTES_USERINFO         0x00E6 // User notes (about) info reply
#define META_EMAIL_USERINFO         0x00EB // User extended email info reply
#define META_INTERESTS_USERINFO     0x00F0 // User interests info reply
#define META_AFFILATIONS_USERINFO   0x00FA // User past/affilations info reply
#define META_SHORT_USERINFO         0x0104 // Short user information reply
#define META_HPAGECAT_USERINFO      0x010E // User homepage category information reply
#define SRV_USER_FOUND              0x01A4 // Search: user found reply
#define SRV_LAST_USER_FOUND         0x01AE // Search: last user found reply
#define META_REGISTRATION_STATS_ACK 0x0302 // Registration stats ack
#define SRV_RANDOM_FOUND            0x0366 // Random search server reply
#define META_SET_PASSWORD_REQ       0x042E // Set user password request
#define META_REQUEST_FULL_INFO      0x04B2 // Request full user info
#define META_REQUEST_SHORT_INFO     0x04BA // Request short user info
#define META_REQUEST_SELF_INFO      0x04D0 // Request full self user info
#define META_SEARCH_GENERIC         0x055F // Search user by details (TLV)
#define META_SEARCH_UIN             0x0569 // Search user by UIN (TLV)
#define META_SEARCH_EMAIL           0x0573 // Search user by E-mail (TLV)
#define META_DIRECTORY_QUERY        0x0FA0
#define META_DIRECTORY_DATA         0x0FAA
#define META_DIRECTORY_RESPONSE     0x0FB4
#define META_DIRECTORY_UPDATE       0x0FD2
#define META_DIRECTORY_UPDATE_ACK   0x0FDC

#define META_XML_INFO               0x08A2 // Server variable requested via xml
#define META_SET_FULLINFO_REQ       0x0C3A // Set full user info request
#define META_SET_FULLINFO_ACK       0x0C3F // Server ack for set fullinfo command
#define META_SPAM_REPORT_ACK        0x2012 // Server ack for user spam report

// Subtypes for Directory meta requests (family 0x5b9)
#define DIRECTORY_QUERY_INFO        0x0002
#define DIRECTORY_SET_INFO          0x0003
#define DIRECTORY_QUERY_MULTI_INFO  0x0006
#define DIRECTORY_QUERY_INFO_ACK    0x0009
#define DIRECTORY_SET_INFO_ACK      0x000A

// TLV types

// SECURITY flags
#define TLV_AUTH        0x02F8  //   uint8      User authorization permissions
#define TLV_WEBAWARE    0x030C  //   uint8      User 'show web status' permissions


// SEARCH only TLVs
#define TLV_AGERANGE    0x0168  //   acombo     Age range to search
#define TLV_KEYWORDS    0x0226  //   sstring    Whitepages search keywords string
#define TLV_ONLINEONLY  0x0230  //   uint8      Search only online users flag
#define TLV_UIN         0x0136  //   uint32     User uin

// common
#define TLV_FIRSTNAME   0x0140  //   sstring    User firstname
#define TLV_LASTNAME    0x014A  //   sstring    User lastname
#define TLV_NICKNAME    0x0154  //   sstring    User nickname
#define TLV_EMAIL       0x015E  //   ecombo     User email
#define TLV_GENDER      0x017C  //   uint8      User gender
#define TLV_MARITAL     0x033E  //   uint8      User marital status
#define TLV_LANGUAGE    0x0186  //   uint16     User spoken language
#define TLV_CITY        0x0190  //   sstring    User home city name
#define TLV_STATE       0x019A  //   sstring    User home state abbr
#define TLV_COUNTRY     0x01A4  //   uint16     User home country code
#define TLV_COMPANY     0x01AE  //   sstring    User work company name
#define TLV_DEPARTMENT  0x01B8  //   sstring    User work department name
#define TLV_POSITION    0x01C2  //   sstring    User work position (title)
#define TLV_OCUPATION   0x01CC  //   uint16     User work ocupation code
#define TLV_PASTINFO    0x01D6  //   icombo     User affilations node
#define TLV_AFFILATIONS 0x01FE  //   icombo     User past info node
#define TLV_INTERESTS   0x01EA  //   icombo     User interests node
#define TLV_HOMEPAGE    0x0212  //   sstring    User homepage category/keywords

// changeinfo
#define TLV_AGE         0x0172  //   uint16     User age
#define TLV_URL         0x0213  //   sstring    User homepage url
#define TLV_BIRTH       0x023A  //   bcombo     User birthday info (year, month, day)
#define TLV_ABOUT       0x0258  //   sstring    User notes (about) text
#define TLV_STREET      0x0262  //   sstring    User home street address
#define TLV_ZIPCODE     0x026D  //   sstring    User home zip code
#define TLV_PHONE       0x0276  //   sstring    User home phone number
#define TLV_FAX         0x0280  //   sstring    User home fax number
#define TLV_MOBILE      0x028A  //   sstring    User home cellular phone number
#define TLV_WORKSTREET  0x0294  //   sstring    User work street address
#define TLV_WORKCITY    0x029E  //   sstring    User work city name
#define TLV_WORKSTATE   0x02A8  //   sstring    User work state name
#define TLV_WORKCOUNTRY 0x02B2  //   uint16     User work country code
#define TLV_WORKZIPCODE 0x02BD  //   sstring    User work zip code
#define TLV_WORKPHONE   0x02C6  //   sstring    User work phone number
#define TLV_WORKFAX     0x02D0  //   sstring    User work fax number
#define TLV_WORKURL     0x02DA  //   sstring    User work webpage url
#define TLV_TIMEZONE    0x0316  //   uint8      User GMT offset
#define TLV_ORGCITY     0x0320  //   sstring    User originally from city
#define TLV_ORGSTATE    0x032A  //   sstring    User originally from state
#define TLV_ORGCOUNTRY  0x0334  //   uint16     User originally from country (code)
#define TLV_ALLOWSPAM   0x0348  //   uint8
#define TLV_CODEPAGE    0x0352  //   uint16     Codepage used for details


/* Direct packet types */
#define PEER_INIT                   0xFF
#define PEER_INIT_ACK               0x01
#define PEER_MSG_INIT               0x03
#define PEER_MSG                    0x02
#define PEER_FILE_INIT              0x00
#define PEER_FILE_INIT_ACK          0x01
#define PEER_FILE_NEXTFILE          0x02
#define PEER_FILE_RESUME            0x03
#define PEER_FILE_STOP              0x04
#define PEER_FILE_SPEED             0x05
#define PEER_FILE_DATA              0x06

/* Direct command types */
#define DIRECT_CANCEL               0x07D0    /* 2000 TCP cancel previous file/chat request */
#define DIRECT_ACK                  0x07DA    /* 2010 TCP acknowledge message packet */
#define DIRECT_MESSAGE              0x07EE    /* 2030 TCP message */

// DC types
#define DC_DISABLED                 0x0000 // Direct connection disabled / auth required
#define DC_HTTPS                    0x0001 // Direct connection thru firewall or https proxy
#define DC_SOCKS                    0x0002 // Direct connection thru socks4/5 proxy server
#define DC_NORMAL                   0x0004 // Normal direct connection (without proxy/firewall)
#define DC_WEB                      0x0006 // Web client - no direct connection

// Message flags
#define MFLAG_NORMAL                0x01 // Normal message
#define MFLAG_AUTO                  0x03 // Auto-message flag
#define MFLAG_MULTI                 0x80 // This is multiple recipients message

// Some SSI constants
#define SSI_ITEM_BUDDY              0x0000  // Buddy record (name: uin for ICQ and screenname for AIM)
#define SSI_ITEM_GROUP              0x0001  // Group record
#define SSI_ITEM_PERMIT             0x0002  // Permit record ("Allow" list in AIM, and "Visible" list in ICQ)
#define SSI_ITEM_DENY               0x0003  // Deny record ("Block" list in AIM, and "Invisible" list in ICQ)
#define SSI_ITEM_VISIBILITY         0x0004  // Permit/deny settings or/and bitmask of the AIM classes
#define SSI_ITEM_PRESENCE           0x0005  // Presence info (if others can see your idle status, etc)
#define SSI_ITEM_CLIENTDATA         0x0009  // Client specific, e.g. ICQ2k shortcut bar items
#define SSI_ITEM_IGNORE             0x000e  // Ignore list record.
#define SSI_ITEM_LASTUPDATE         0x000f  // Item that contain roster update time (name: "LastUpdateDate")
#define SSI_ITEM_NONICQ             0x0010  // Non-ICQ contact (to send SMS). Name: 1#EXT, 2#EXT, etc
#define SSI_ITEM_UNKNOWN2           0x0011  // Unknown.
#define SSI_ITEM_IMPORTTIME         0x0013  // Item that contain roster import time (name: "Import time")
#define SSI_ITEM_BUDDYICON          0x0014  // Buddy icon info. (names: "1", "8", etc. according ot the icon type)
#define SSI_ITEM_SAVED              0x0019
#define SSI_ITEM_PREAUTH            0x001B
#define SSI_ITEM_METAINFO           0x0020  // Owner Details' token & last update time

#define SSI_TLV_AWAITING_AUTH       0x0066  // Contact not authorized in list
#define SSI_TLV_NOT_IN_LIST         0x006A  // Always empty
#define SSI_TLV_UNKNOWN             0x006D  // WTF ?
#define SSI_TLV_SUBITEMS            0x00C8  // List of sub-items IDs
#define SSI_TLV_VISIBILITY          0x00CA
#define SSI_TLV_SHORTCUT            0x00CD
#define SSI_TLV_TIMESTAMP           0x00D4  // Import Timestamp
#define SSI_TLV_AVATARHASH          0x00D5
#define SSI_TLV_NAME                0x0131  // Custom contact nickname
#define SSI_TLV_GROUP_OPENNED       0x0134
#define SSI_TLV_EMAIL               0x0137  // Custom contact email
#define SSI_TLV_PHONE               0x0138  // Custom contact phone number
#define SSI_TLV_PHONE_CELLULAR      0x0139  // Custom contact cellphone number
#define SSI_TLV_PHONE_SMS           0x013A  // Custom contact SMS number
#define SSI_TLV_COMMENT             0x013C  // User comment
#define SSI_TLV_METAINFO_TOKEN      0x015C  // Privacy token for Contact's details
#define SSI_TLV_METAINFO_TIME       0x015D  // Contact's details last update time

#define MAX_SSI_TLV_NAME_SIZE       0x40
#define MAX_SSI_TLV_COMMENT_SIZE    0x50

// Client ID constants (internal)
#define CLID_GENERIC                0x00  // Generic clients (eg. older official clients)
#define CLID_ALTERNATIVE            0x01  // Clients not using tick for MsgID (most third-party clients)
#define CLID_MIRANDA                0x02  // Hey, that's mate!
#define CLID_ICQ6                   0x10  // Mark ICQ6 as it has some non obvious limitations!


// Internal Constants
#define ICQ_PROTOCOL_NAME           LPGEN("ICQ")
#define ICQ_PLUG_VERSION            PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM)
#define ICQ_VERSION                 11        // Protocol version
#define DC_TYPE                     DC_NORMAL // Used for DC settings
#define MAX_CONTACTSSEND            15
#define MAX_MESSAGESNACSIZE         8000
#define CLIENTRATELIMIT             0
#define COOKIE_TIMEOUT              3600      // One hour
#define KEEPALIVE_INTERVAL          57        // Slightly less than a minute
#define WEBFRONTPORT                0x50
#define CLIENTFEATURES              0x3
#define URL_FORGOT_PASSWORD         "https://www.icq.com/password/"
#define URL_REGISTER                "https://www.icq.com/register/"
#define FLAP_MARKER                 0x2a
#define CLIENT_MD5_STRING           "AOL Instant Messenger (SM)"
#define UNIQUEIDSETTING             "UIN"
#define UINMAXLEN                   11 // DWORD string max len + 1
#define PASSWORDMAXLEN              128
#define OSCAR_PROXY_HOST            "ars.icq.com"
#define OSCAR_PROXY_VERSION         0x044A

#define CLIENT_ID_STRING            "ICQ Client"  // Client identification, mimic ICQ 6.5
#define CLIENT_ID_CODE              0x010a
#define CLIENT_VERSION_MAJOR        0x0014
#define CLIENT_VERSION_MINOR        0x0034
#define CLIENT_VERSION_LESSER       0x0000
#define CLIENT_VERSION_BUILD        0x0c18
#define CLIENT_DISTRIBUTION         0x00000611
#define CLIENT_LANGUAGE             "en"
#define CLIENT_COUNTRY              "us"

#endif /* __ICQ_CONSTANTS_H */
