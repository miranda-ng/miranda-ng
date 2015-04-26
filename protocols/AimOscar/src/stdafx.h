/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef AIM_H
#define AIM_H

//System includes
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <richedit.h>
#include <sys/stat.h>
#include <time.h>
#include <malloc.h>
#include <Uxtheme.h>

//Miranda NG includes
#include <msapi\vssym32.h>
#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_button.h>
#include <m_chat.h>
#include <m_clistint.h>
#include <m_database.h>
#include <m_history.h>
#include <m_idle.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protomod.h>
#include <m_userinfo.h>
#include <m_icolib.h>
#include <m_imgsrvc.h>
#include <win2k.h>
#include <m_extraicons.h>
#include <m_protoint.h>

#include <m_folders.h>
#include <m_assocmgr.h>

//rest of includes
#include "avatars.h"
#include "utility.h"
#include "chat.h"
#include "direct_connect.h"
#include "conv.h"
#include "file.h"
#include "flap.h"
#include "links.h"
#include "snac.h"
#include "tlv.h"
#include "packets.h"
#include "proxy.h"
#include "resource.h"
#include "proto.h"
#include "theme.h"
#include "ui.h"
#include "version.h"

// Protocol limits
#define MAX_SCREEN_NAME_LENGTH      97
#define MAX_GROUP_NAME_LENGTH       48
#define MAX_NICKNAME_LENGTH         64
#define MAX_MESSAGE_LENGTH          3978
#define MAX_STATUS_MESSAGE_LENGTH   251
#define MAX_AWAY_MESSAGE_LENGTH     4096
#define MAX_ICON_SIZE               7168

// ICBM parameter flags
#define ICBM_CHANNEL_MSGS_ALLOWED   0x00000001
#define ICBM_MISSED_CALLS_ENABLED   0x00000002
#define ICBM_EVENTS_ALLOWED         0x00000008
#define ICBM_SMS_SUPPORTED          0x00000010
#define ICBM_OFFLINE_MSGS_ALLOWED   0x00000100

// SSI preferences
#define SHOW_IDLE                   0x00000400
#define SHOW_RECENT_BUDDIES         0x00020000
#define SHOW_TYPING                 0x00400000

//Extended Status Icon Numbers
#define ACCOUNT_TYPE_UNCONFIRMED    1
#define ACCOUNT_TYPE_CONFIRMED      2
#define ACCOUNT_TYPE_ICQ            3
#define ACCOUNT_TYPE_AOL            4
#define ACCOUNT_TYPE_ADMIN          5
#define EXTENDED_STATUS_BOT         1
#define EXTENDED_STATUS_HIPTOP      2

//Popup flags
#define MAIL_POPUP                  0x04
#define ERROR_POPUP                 0x08
#define TCHAR_POPUP                 0x10

//Main Option Window Keys
#define AIM_KEY_SN                  "SN"
#define AIM_KEY_NK                  "Nick"
#define AIM_KEY_PW                  "Password"
#define AIM_KEY_HN                  "loginhost"
#define AIM_KEY_PN                  "loginport"
#define AIM_KEY_DC                  "DelConf"//delivery confirmation
#define AIM_KEY_FP                  "ForceProxyTransfer"
#define AIM_KEY_HF                  "HiptopFake"
#define AIM_KEY_AT                  "DisableATIcons"
#define AIM_KEY_ES                  "DisableESIcons"
#define AIM_KEY_DM                  "DisableModeMsg"
#define AIM_KEY_FI                  "FormatIncoming"//html->bbcodes
#define AIM_KEY_FO                  "FormatOutgoing"//bbcodes->html
#define AIM_KEY_II                  "InstantIdle"
#define AIM_KEY_IIT                 "InstantIdleTS"
#define AIM_KEY_CM                  "CheckMail"
#define AIM_KEY_MG                  "ManageGroups"
#define AIM_KEY_DA                  "DisableAvatars"
#define AIM_KEY_DSSL                "DisableSSL"
#define AIM_KEY_FSC                 "ForceSingleClient"

#define OTH_KEY_SM                  "StatusMsg"
#define OTH_KEY_GP                  "Group"
//Module Name Key
#define MOD_KEY_CL                  "CList"
//Settings Keys
#define AIM_KEY_PR                  "Profile"
#define AIM_KEY_LA                  "LastAwayChange"
//Contact Keys
#define AIM_KEY_BI                  "BuddyId"
#define AIM_KEY_GI                  "GroupId"
#define AIM_KEY_ST                  "Status"
#define AIM_KEY_IT                  "IdleTS"
#define AIM_KEY_OT                  "LogonTS"
#define AIM_KEY_MS                  "MemberTS"
#define AIM_KEY_AC                  "AccType"//account type
#define AIM_KEY_ET                  "ESType"//Extended Status type
#define AIM_KEY_MV                  "MirVer"
#define AIM_KEY_US                  "Utf8Support"
#define AIM_KEY_NL                  "NotOnList"
#define AIM_KEY_LM                  "LastMessage"
#define AIM_KEY_AH                  "AvatarHash"
#define AIM_KEY_AHT                 "AvatarType"
#define AIM_KEY_EM                  "e-mail"
#define AIM_KEY_LV                  "LastVer"
#define AIM_KEY_TIS		            "TotalIMsSent"
#define AIM_KEY_TIR                 "TotalIMsReceived"
#define AIM_KEY_TAM                 "TotalAwayMessages"
#define AIM_KEY_TTO                 "TotalTimeOnline"
#define AIM_KEY_BLS                 "IsBlast"
#define AIM_KEY_NIL                 "IsNotInList"

#define AIM_DEFAULT_SERVER          "slogin.oscar.aol.com"
#define AIM_DEFAULT_SERVER_NS       "login.oscar.aol.com"
#define AIM_PROXY_SERVER            "ars.oscar.aol.com"
#define AIM_DEFAULT_PORT            5190
#define AIM_DEFAULT_SSL_PORT        443

//Some Defaults for various things
#define DEFAULT_KEEPALIVE_TIMER     39 // secs
#define DEFAULT_GRACE_PERIOD        60
#define AIM_DEFAULT_GROUP           "miranda merged"
#define SYSTEM_BUDDY                "aolsystemmsg"
#define DEFAULT_AWAY_MSG            "I am away from my computer right now."
//Md5 Roasting stuff
#define AIM_MD5_STRING              "AOL Instant Messenger (SM)"
#define MD5_HASH_LENGTH             16

//Aim Version Stuff
#define AIM_CLIENT_MAJOR_VERSION    5
#define AIM_CLIENT_MINOR_VERSION    9
#define AIM_CLIENT_LESSER_VERSION   0
#define AIM_CLIENT_BUILD_NUMBER     0x1772
#define AIM_CLIENT_ID_NUMBER        0x0109
#define AIM_CLIENT_DISTRIBUTION_NUMBER 0x0150

#define AIM_LANGUAGE "en"
#define AIM_COUNTRY "us"
#define AIM_MSG_TYPE "text/x-aolrtf; charset=\"us-ascii\""
#define AIM_MSG_TYPE_UNICODE "text/x-aolrtf; charset=\"unicode-2-0\""
#define AIM_TOOL_VERSION "\x01\x10\x18\xf1"

//Supported Clients
#define CLIENT_UNKNOWN "?"
#define CLIENT_AIM5 "AIM 5.x"
#define CLIENT_AIM4 "AIM 4.x"
#define CLIENT_AIMEXPRESS5 "AIM Express 5"
#define CLIENT_AIMEXPRESS6 "AIM Express 6"
#define CLIENT_AIMEXPRESS7 "AIM Express 7"
#define CLIENT_ICQ "ICQ"
#define CLIENT_AIM_TRITON "AIM Triton"
#define CLIENT_AIM6_1 "AIM 6.1"
#define CLIENT_AIM6_5 "AIM 6.5"
#define CLIENT_AIM6_8 "AIM 6.8"
#define CLIENT_AIM6_9 "AIM 6.9"
#define CLIENT_AIM7_0 "AIM 7.0"
#define CLIENT_AIMTOC "AIM TOC"
#define CLIENT_BOT "AIM Bot"
#define CLIENT_GAIM "Gaim"
#define CLIENT_PURPLE "Purple"
#define CLIENT_ADIUM "Adium X"
#define CLIENT_GPRS "GPRS"
#define CLIENT_ICHAT "iChat"
#define CLIENT_IM2 "IM2"
#define CLIENT_KOPETE "Kopete"
#define CLIENT_MEEBO "Meebo"
#define CLIENT_DIGSBY "Digsby"
#define CLIENT_BEEJIVE "beejive"
#define CLIENT_MICQ "mICQ"
#define CLIENT_AIMOSCAR "Miranda IM %d.%d.%d.%d (AIM v%d.%d.%d.%d)"
#define CLIENT_OSCARJ "Miranda IM %d.%d.%d.%d%s (ICQ v%d.%d.%d.%d%s)"
#define CLIENT_OSCARSN "Miranda IM %d.%d.%d.%d%s (ICQ S!N v%d.%d.%d.%d%s)%s"
#define CLIENT_OSCARPL "Miranda IM %d.%d.%d.%d%s (ICQ Plus v%d.%d.%d.%d%s)%s"
#define CLIENT_NAIM "naim"
#define CLIENT_QIP "qip"
#define CLIENT_SIM "SIM"
#define CLIENT_SMS "SMS"
#define CLIENT_TERRAIM "TerraIM"
#define CLIENT_TRILLIAN_PRO "Trillian Pro"
#define CLIENT_TRILLIAN "Trillian"
#define CLIENT_TRILLIAN_ASTRA "Trillian Astra"
#define CLIENT_BLAST "Blast Group"

//Aim Caps
#define AIM_CAPS_LENGTH 16

// Official
#define AIM_CAP_SHORT_CAPS "\x09\x46\x00\x00\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_SECURE_IM "\x09\x46\x00\x01\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_XHTML_IM "\x09\x46\x00\x02\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_RTCVIDEO "\x09\x46\x01\x01\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_HAS_MICROPHONE "\x09\x46\x01\x02\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_HAS_CAMERA "\x09\x46\x01\x03\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_RTCAUDIO "\x09\x46\x01\x04\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_HOST_STATUS_TEXT_AWARE "\x09\x46\x01\x0a\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_RT_IM "\x09\x46\x01\x0b\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_SMART_CAPS "\x09\x46\x01\xff\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_FILE_TRANSFER "\x09\x46\x13\x43\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_DIRECT_IM "\x09\x46\x13\x45\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_FILE_SHARING "\x09\x46\x13\x48\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_SUPPORT_ICQ "\x09\x46\x13\x4D\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"

#define AIM_CAP_AVAILABLE_FOR_CALL "\x09\x46\x01\x05\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_ACA "\x09\x46\x01\x06\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_MULTI_AUDIO "\x09\x46\x01\x07\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_MULTI_VIDEO "\x09\x46\x01\x08\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_VICEROY "\x09\x46\xf0\x04\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_BUDDY_ICON "\x09\x46\x13\x46\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_VOICE_CHAT "\x09\x46\x13\x41\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_DIRECT_PLAY "\x09\x46\x13\x42\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_ICQ_DIRECT_CONNECT "\x09\x46\x13\x44\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_GAMES "\x09\x46\x13\x47\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_ICQ_SERVER_RELAY "\x09\x46\x13\x49\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"//icq? channel 2 extended, TLV(0x2711) based messages
#define AIM_CAP_CHAT_ROBOTS "\x09\x46\x13\x4A\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_SHARE_BUDDIES "\x09\x46\x13\x4B\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_CHAT "\x74\x8F\x24\x20\x62\x87\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_AMO "\x09\x46\x01\x0c\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"

// Extensions
#define AIM_CAP_HIPTOP "\x09\x46\x13\x23\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_UTF8 "\x09\x46\x13\x4E\x4C\x7F\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_UNKNOWN4 "\x09\x46\xf0\x03\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_UNKNOWNC "\x09\x46\xf0\x05\x4c\x7f\x11\xD1\x82\x22\x44\x45\x53\x54\0\0"
#define AIM_CAP_IM2 "\x74\xed\xc3\x36\x44\xdf\x48\x5b\x8b\x1c\x67\x1a\x1f\x86\x09\x9f"
#define AIM_CAP_TRILLIAN "\xF2\xE7\xC7\xF4\xFE\xAD\x4D\xFB\xB2\x35\x36\x79\x8B\xDF\0\0"
extern char AIM_CAP_MIRANDA[]; //Miranda cap EXTERN

//Aim Services
#define AIM_SERVICE_GENERIC "\0\x01\0\x04"//version 4
#define AIM_SERVICE_SSI "\0\x13\0\x03"//version 3
#define AIM_SERVICE_LOCATION "\0\x02\0\x01"//version 1
#define AIM_SERVICE_BUDDYLIST "\0\x03\0\x01"//version 1
#define AIM_SERVICE_MESSAGING "\0\x04\0\x01"//version 1
#define AIM_SERVICE_INVITATION "\0\x06\0\x01"//version 1
#define AIM_SERVICE_ADMIN "\0\x07\0\x01"//version 1
#define AIM_SERVICE_POPUP "\0\x08\0\x01"//version 1
#define AIM_SERVICE_BOS "\0\x09\0\x01"//version 1
#define AIM_SERVICE_AVATAR "\0\x10\0\x01"//version 1
#define AIM_SERVICE_USERLOOKUP "\0\x0A\0\x01"//version 1
#define AIM_SERVICE_STATS "\0\x0B\0\x01"//version 1
#define AIM_SERVICE_CHATNAV "\0\x0D\0\x01"//version 1
#define AIM_SERVICE_DIRSEARCH "\0\x0F\0\x01"//version 1
#define AIM_SERVICE_CHAT "\0\x0E\0\x01"//version 1
#define AIM_SERVICE_ICQ "\0\x15\0\x01"//version 1
#define AIM_SERVICE_MAIL "\0\x18\0\x01"//version 1
#define AIM_SERVICE_UNKNOWN "\0\x22\0\x01"//version 1
#define AIM_SERVICE_RATES "\0\x01\0\x02\0\x03\0\x04\0\x05"

//Aim Statuses
#define AIM_STATUS_WEBAWARE "\0\x01"
#define AIM_STATUS_SHOWIP "\0\x02"
#define AIM_STATUS_BIRTHDAY "\0\x08"
#define AIM_STATUS_WEBFRONT "\0\x20"
#define AIM_STATUS_DCAUTH "\x10\0"
#define AIM_STATUS_DCCONT "\x20\0"
#define AIM_STATUS_NULL "\0\0"

#define AIM_STATUS_ONLINE		0x00000000
#define AIM_STATUS_AWAY         0x00000001
#define AIM_STATUS_DND          0x00000002
#define AIM_STATUS_OUT          0x00000004
#define AIM_STATUS_BUSY         0x00000010
#define AIM_STATUS_CHAT         0x00000020	// Broken. If set, you cannot unset.
#define AIM_STATUS_INVISIBLE    0x00000100

extern HINSTANCE hInstance; //plugin dll instance

#endif
