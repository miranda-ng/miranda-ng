//***************************************************************************
// $Id: proto.h, v 1.141 2005/10/24 15:32:33 shingrus Exp $
//***************************************************************************

#ifndef MRIM_PROTO_H
#define MRIM_PROTO_H

#include <sys/types.h>

#include "version.h"

typedef struct mrim_packet_header_t
{
    u_long      magic;		// Magic
    u_long      proto;		// Версия протокола
    u_long      seq;		// Sequence
    u_long      msg;		// Тип пакета
    u_long      dlen; 		// Длина данных
    u_char	reserved[24];	// Зарезервировано
}
mrim_packet_header_t;

#define CS_MAGIC    0xDEADBEEF	// Клиентский Magic ( C <-> S )


// UNICODE = (UTF-16LE) ( >= 1.17)

/***************************************************************************

		ПРОТОКОЛ СВЯЗИ КЛИЕНТ-СЕРВЕР

 ***************************************************************************/

#define MRIM_CS_HELLO       		0x1001 // C->S
// empty

#define MRIM_CS_HELLO_ACK   		0x1002 // S->C
// mrim_connection_params_t


#define MRIM_CS_LOGIN_ACK   		0x1004 // S->C
// empty

#define MRIM_CS_LOGIN_REJ   		0x1005 // S->C
// LPS reason ???

#define MRIM_CS_PING        		0x1006 // C->S
// empty

#define MRIM_CS_MESSAGE			0x1008 // C->S
// UL flags
	#define MESSAGE_FLAG_OFFLINE		0x00000001
	#define MESSAGE_FLAG_NORECV		0x00000004
	#define MESSAGE_FLAG_AUTHORIZE		0x00000008 // X-MRIM-Flags: 00000008
	#define MESSAGE_FLAG_SYSTEM		0x00000040
	#define MESSAGE_FLAG_RTF		0x00000080
	#define MESSAGE_FLAG_CONTACT		0x00000200
	#define MESSAGE_FLAG_NOTIFY		0x00000400
	#define MESSAGE_FLAG_SMS		0x00000800
	#define MESSAGE_FLAG_MULTICAST		0x00001000
	#define MESSAGE_SMS_DELIVERY_REPORT	0x00002000
	#define MESSAGE_FLAG_ALARM		0x00004000
	#define MESSAGE_FLAG_FLASH		0x00008000
	#define MESSAGE_FLAG_SPAMF_SPAM		0x00020000 // report spam back to the server
	#define MESSAGE_FLAG_MULTICHAT		0x00400000 //
		#define MULTICHAT_MESSAGE	0	// received message (s->c)
		#define MULTICHAT_GET_MEMBERS	1	// request members list from server (c->s)
		#define MULTICHAT_MEMBERS	2	// members list from server (s->c)
		#define MULTICHAT_ADD_MEMBERS	3	//
		#define MULTICHAT_ATTACHED	4	// user joined to chat (s->c)
		#define MULTICHAT_DETACHED	5	// user leave chat (s->c)
		#define MULTICHAT_DESTROYED	6	//
		#define MULTICHAT_INVITE	7	//
	#define MESSAGE_FLAG_v1p16		0x00100000 // для перекодировки юникода
	#define MESSAGE_FLAG_CP1251		0x00200000
// LPS to e-mail ANSI
// LPS message ANSI/UNICODE (see flags)
// LPS rtf-formatted message ( >= 1.1) ???
// LPS multichat_data ( >= 1.20) ???

#define MAX_MULTICAST_RECIPIENTS 50
	#define MESSAGE_USERFLAGS_MASK	0x000036A8 // Flags that user is allowed to set himself


#define MRIM_CS_MESSAGE_ACK		0x1009 // S->C
// UL msg_id
// UL flags
// LPS from e-mail ANSI
// LPS message UNICODE
// LPS rtf-formatted message ( >= 1.1)	- MESSAGE_FLAG_RTF
//	BASE64(				- MESSAGE_FLAG_AUTHORIZE
//		UL parts count = 2
//		LPS auth_sender_nick  UNICODE
//		LPS auth_request_text  UNICODE
//[ LPS multichat_data ] ( >=  1.20)	- MESSAGE_FLAG_MULTICHAT
//		UL type
//		LPS multichat_name
//		switch (type) {
//			MULTICHAT_MESSAGE {
//				LPS sender ANSI
//			}
//			MULTICHAT_MEMBERS {
//				CLPS members
//				[ LPS owner ]
//			}
//			MULTICHAT_ADD_MEMBERS {
//				LPS sender ANSI
//				CLPS members
//			}
//			MULTICHAT_ATTACHED {
//				LPS member ANSI
//			}
//			MULTICHAT_DETACHED {
//				LPS member ANSI
//			}
//			MULTICHAT_INVITE {
//				LPS sender ANSI
//			}
//		}
//	)





#define MRIM_CS_MESSAGE_RECV		0x1011 // C->S
// LPS from e-mail ANSI
// UL msg_id

#define MRIM_CS_MESSAGE_STATUS		0x1012 // S->C
// UL status
	#define MESSAGE_DELIVERED		0x0000	// Message delivered directly to user
	#define MESSAGE_REJECTED_NOUSER		0x8001  // Message rejected - no such user
	#define MESSAGE_REJECTED_INTERR		0x8003	// Internal server error
	#define MESSAGE_REJECTED_LIMIT_EXCEEDED	0x8004	// Offline messages limit exceeded
	#define MESSAGE_REJECTED_TOO_LARGE	0x8005	// Message is too large
	#define	MESSAGE_REJECTED_DENY_OFFMSG	0x8006	// User does not accept offline messages
	#define	MESSAGE_REJECTED_DENY_OFFFLSH	0x8007	// User does not accept offline flash animation

#define MRIM_CS_USER_STATUS		0x100F // S->C
// UL status
	#define STATUS_OFFLINE          0x00000000
	#define STATUS_ONLINE           0x00000001
	#define STATUS_AWAY             0x00000002
	#define STATUS_UNDETERMINATED   0x00000003
	#define STATUS_USER_DEFINED     0x00000004
	#define STATUS_FLAG_INVISIBLE   0x80000000
// LPS spec_status_uri ANSI ( >= 1.14)
	#define SPEC_STATUS_URI_MAX 256
// LPS status_title UNICODE ( >= 1.14)
	#define STATUS_TITLE_MAX 16
// LPS status_desc UNICODE ( >= 1.14)
	#define STATUS_DESC_MAX 64
// LPS user e-mail ANSI
// UL com_support ( >= 1.14)
	#define FEATURE_FLAG_RTF_MESSAGE       0x00000001
	#define FEATURE_FLAG_BASE_SMILES       0x00000002
	#define FEATURE_FLAG_ADVANCED_SMILES   0x00000004
	#define FEATURE_FLAG_CONTACTS_EXCH     0x00000008
	#define FEATURE_FLAG_WAKEUP            0x00000010
	#define FEATURE_FLAG_MULTS             0x00000020
	#define FEATURE_FLAG_FILE_TRANSFER     0x00000040
	#define FEATURE_FLAG_VOICE             0x00000080
	#define FEATURE_FLAG_VIDEO             0x00000100
	#define FEATURE_FLAG_GAMES             0x00000200
	#define FEATURE_FLAG_LAST              0x00000200
	#define FEATURE_UA_FLAG_MASK           ((FEATURE_FLAG_LAST << 1) - 1)
// LPS user_agent ( >= 1.14) ANSI
	#define USER_AGENT_MAX 255
	// Format:
	//  user_agent       = param *(param )
	//  param            = pname "=" pvalue
	//  pname            = token
	//  pvalue           = token / quoted-string
	//
	// Params:
	//  "client" - magent/jagent/???
	//  "name" - sys-name.
	//  "title" - display-name.
	//  "version" - product internal numeration. Examples: "1.2", "1.3 pre".
	//  "build" - product internal numeration (may be positive number or time).
	//  "protocol" - MMP protocol number by format "<major>.<minor>".


#define MRIM_CS_LOGOUT			0x1013 // S->C
// UL reason
	#define LOGOUT_NO_RELOGIN_FLAG	0x0010 // Logout due to double login

#define MRIM_CS_CONNECTION_PARAMS	0x1014 // S->C (>1.16 depricated ?)
// mrim_connection_params_t

#define MRIM_CS_USER_INFO		0x1015 // S->C
// (LPS key, LPS value)* X ???
// MESSAGES.TOTAL - num UNICODE
// MESSAGES.UNREAD - num UNICODE
// MRIM.NICKNAME - nick UNICODE
// client.endpoint - ip:port UNICODE


#define MRIM_CS_ADD_CONTACT		0x1019 // C->S
// UL flags (group(2) or usual(0)
	#define CONTACT_FLAG_REMOVED		0x00000001
	#define CONTACT_FLAG_GROUP		0x00000002
	#define CONTACT_FLAG_INVISIBLE		0x00000004
	#define CONTACT_FLAG_VISIBLE		0x00000008
	#define CONTACT_FLAG_IGNORE		0x00000010
	#define CONTACT_FLAG_SHADOW		0x00000020
	#define CONTACT_FLAG_AUTHORIZED		0x00000040 // ( >= 1.15)
	#define CONTACT_FLAG_MULTICHAT		0x00000080 // ( >= 1.20) = 128
	#define CONTACT_FLAG_UNICODE_NAME	0x00000200 // = 512
	#define CONTACT_FLAG_PHONE		0x00100000

// UL group id (unused if contact is group)
// LPS contact e-mail ANSI
// LPS name UNICODE
// LPS custom phones ANSI
// LPS BASE64(
//		UL parts count = 2
//		LPS auth_sender_nick ???
//		LPS auth_request_text ???
//	)
// UL actions ( >= 1.15)
// [LPS multichat_data]
//		CLPS members ( >=  1.20)
//		[ LPS owner ]
    #define ADD_CONTACT_FLAG_MYMAIL_INVITE		0x00000001
    #define ADD_CONTACT_FLAG_MULTICHAT_ATTACHE		0x00000002
	//used internal in win32 agent
	#define CONTACT_AWAITING_AUTHORIZATION_USER	0x00000100
	#define CONTACT_FLAG_TEMPORARY			0x00010000


#define MRIM_CS_ADD_CONTACT_ACK		0x101A // S->C
// UL status
	#define CONTACT_OPER_SUCCESS		0x0000
	#define CONTACT_OPER_ERROR		0x0001
	#define CONTACT_OPER_INTERR		0x0002
	#define CONTACT_OPER_NO_SUCH_USER	0x0003
	#define CONTACT_OPER_INVALID_INFO	0x0004
	#define CONTACT_OPER_USER_EXISTS	0x0005
	#define CONTACT_OPER_GROUP_LIMIT	0x0006
// UL contact_id or (u_long)-1 if status is not OK
// [LPS multichat_contact ( >=  1.20)]


#define MRIM_CS_MODIFY_CONTACT		0x101B // C->S
// UL id
// UL flags - same as for MRIM_CS_ADD_CONTACT
// UL group id (unused if contact is group)
// LPS contact e-mail ANSI
// LPS name UNICODE
// LPS custom phones ANSI

#define MRIM_CS_MODIFY_CONTACT_ACK	0x101C // S->C
// UL status, same as for MRIM_CS_ADD_CONTACT_ACK

#define MRIM_CS_OFFLINE_MESSAGE_ACK	0x101D // S->C
// UIDL
// LPS offline message ???

#define MRIM_CS_DELETE_OFFLINE_MESSAGE	0x101E // C->S
// UIDL


#define MRIM_CS_AUTHORIZE		0x1020 // C->S
// LPS user e-mail ANSI

#define MRIM_CS_AUTHORIZE_ACK		0x1021 // S->C
// LPS user e-mail ANSI

#define MRIM_CS_CHANGE_STATUS		0x1022 // C->S
// UL new status
// LPS spec_status_uri ANSI ( >= 1.14)
// LPS status_title UNICODE ( >= 1.14)
// LPS status_desc UNICODE ( >= 1.14)
// UL com_support ( >= 1.14) (see MRIM_CS_USER_STATUS)


#define MRIM_CS_GET_MPOP_SESSION	0x1024 // C->S


#define MRIM_CS_MPOP_SESSION		0x1025 // S->C
// UL status
	#define MRIM_GET_SESSION_FAIL		0
	#define MRIM_GET_SESSION_SUCCESS	1
// LPS mpop session ???


#define MRIM_CS_FILE_TRANSFER		0x1026 // C->S
// LPS TO/FROM e-mail ANSI
// DWORD id_request - uniq per connect
// DWORD FILESIZE
// LPS:	// LPS Files (FileName;FileSize;FileName;FileSize;) ANSI
		// LPS DESCRIPTION:
							// UL ?
							// Files (FileName;FileSize;FileName;FileSize;) UNICODE
		// LPS Conn (IP:Port;IP:Port;) ANSI

#define MRIM_CS_FILE_TRANSFER_ACK	0x1027 // S->C
// DWORD status
	#define FILE_TRANSFER_STATUS_OK                 1
	#define FILE_TRANSFER_STATUS_DECLINE            0
	#define FILE_TRANSFER_STATUS_ERROR              2
	#define FILE_TRANSFER_STATUS_INCOMPATIBLE_VERS  3
	#define FILE_TRANSFER_MIRROR                    4
// LPS TO/FROM e-mail ANSI
// DWORD id_request
// LPS DESCRIPTION [Conn (IP:Port;IP:Port;) ANSI]



//white pages!
#define MRIM_CS_WP_REQUEST		0x1029 // C->S
// DWORD field
// LPS value ???
#define PARAMS_NUMBER_LIMIT		50
#define PARAM_VALUE_LENGTH_LIMIT	64

//if last symbol in value eq '*' it will be replaced by LIKE '%'
// params define
// must be  in consecutive order (0..N) to quick check in check_anketa_info_request
	enum {
	MRIM_CS_WP_REQUEST_PARAM_USER =  0,		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_DOMAIN, 		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_NICKNAME, 		// UNICODE
	MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME, 		// UNICODE
	MRIM_CS_WP_REQUEST_PARAM_LASTNAME, 		// UNICODE
	MRIM_CS_WP_REQUEST_PARAM_SEX,	 		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY, 		// not used for search
	MRIM_CS_WP_REQUEST_PARAM_DATE1, 		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_DATE2, 		// ANSI
	//!!!!!!!!!!!!!!!!!!!online request param must be at end of request!!!!!!!!!!!!!!!
	MRIM_CS_WP_REQUEST_PARAM_ONLINE, 		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_STATUS, 		// we do not used it, yet
	MRIM_CS_WP_REQUEST_PARAM_CITY_ID, 		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_ZODIAC, 		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_MONTH,	// ANSI
	MRIM_CS_WP_REQUEST_PARAM_BIRTHDAY_DAY,		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_COUNTRY_ID,		// ANSI
	MRIM_CS_WP_REQUEST_PARAM_MAX
	};

#define MRIM_CS_ANKETA_INFO		0x1028 // S->C
// DWORD status
	#define MRIM_ANKETA_INFO_STATUS_OK		1
	#define MRIM_ANKETA_INFO_STATUS_NOUSER		0
	#define MRIM_ANKETA_INFO_STATUS_DBERR		2
	#define MRIM_ANKETA_INFO_STATUS_RATELIMERR	3
// DWORD fields_num
// DWORD max_rows
// DWORD server_time sec since 1970 (unixtime)
	// fields set 	//%fields_num == 0
	// values set 	//%fields_num == 0
// LPS value (numbers too) ???


#define MRIM_CS_MAILBOX_STATUS		0x1033
// DWORD new messages in mailbox


#define MRIM_CS_GAME                    0x1035
// LPS to/from e-mail ANSI
// DWORD session unique per game
// DWORD msg internal game message
	enum {
	GAME_BASE,
	GAME_CONNECTION_INVITE,
	GAME_CONNECTION_ACCEPT,
	GAME_DECLINE,
	GAME_INC_VERSION,
	GAME_NO_SUCH_GAME,
	GAME_JOIN,
	GAME_CLOSE,
	GAME_SPEED,
	GAME_SYNCHRONIZATION,
	GAME_USER_NOT_FOUND,
	GAME_ACCEPT_ACK,
	GAME_PING,
	GAME_RESULT,
	GAME_MESSAGES_NUMBER
	};
// DWORD msg_id id for ack
// DWORD time_send time of client
// LPS data ???



#define MRIM_CS_CONTACT_LIST2		0x1037 // S->C
// UL status
	#define GET_CONTACTS_OK		0x0000
	#define GET_CONTACTS_ERROR	0x0001
	#define GET_CONTACTS_INTERR	0x0002
// DWORD status  - if ...OK than this staff:
// DWORD groups number
	// mask symbols table:
	// 's' - lps
	// 'u' - unsigned long
	// 'z' - zero terminated string
	// LPS groups fields mask ANSI
	// LPS contacts fields mask ANSI
	// group fields
	// contacts fields
	// groups mask 'us' == flags, name UNICODE
	// contact mask 'uussuussssus' flags, group id, e-mail ANSI, nick UNICODE, server flags, status, custom phone numbers ANSI, spec_status_uri ANSI, status_title UNICODE, status_desc UNICODE, com_support (future flags), user_agent (formated string) ANSI, ul blog status id, ul blog status id, ul BlogStatusTime, blog status UNICODE, blog status music UNICODE, blog status sender, ?????
	//               uussuussssusuuusssss
	#define CONTACT_INTFLAG_NOT_AUTHORIZED	0x0001


//old packet cs_login with cs_statistic
#define MRIM_CS_LOGIN2			0x1038 // C->S
// LPS login e-mail ANSI
// LPS password ANSI
// DWORD status
// LPS spec_status_uri ANSI ( >= 1.14)
// LPS status_title UNICODE ( >= 1.14)
// LPS status_desc UNICODE ( >= 1.14)
// UL com_support ( >= 1.14) (see MRIM_CS_USER_STATUS)
// LPS user_agent ANSI ( >= 1.14) (see MRIM_CS_USER_STATUS)
	// + statistic packet data:
// LPS lang ( >= 1.16)
// LPS ua session ( >= 1.20) - шли пустой
// LPS replaced ua session ( >= 1.20) - шли пустой
// LPS client description ANSI
	#define MAX_CLIENT_DESCRIPTION 256
// unknown data
// LPS unknown id (len = 32)
/*		SetUL(&lpbDataCurrent, 0);//00 00 00 00
		SetUL(&lpbDataCurrent, 0);//02 BA 0A 00
		SetUL(&lpbDataCurrent, 0);//00 00 00 03
		SetUL(&lpbDataCurrent, 0);//00 00 00 02

		SetUL(&lpbDataCurrent, 0);//E2 FD 1E 22
		SetUL(&lpbDataCurrent, 0);//04 00 00 00
		SetUL(&lpbDataCurrent, 0);//02 FF FF FF
		SetUL(&lpbDataCurrent, 0);//FF 05 00 00

		SetUL(&lpbDataCurrent, 0);//00 02 FF FF
		SetUL(&lpbDataCurrent, 0);//FF FF 06 00
		SetUL(&lpbDataCurrent, 0);//00 00 02 FF
		SetUL(&lpbDataCurrent, 0);//FF FF FF 07

		SetUL(&lpbDataCurrent, 0);//00 00 00 02
		SetUL(&lpbDataCurrent, 0);//FF FF FF FF
		SetUL(&lpbDataCurrent, 0);//14 00 00 00
		SetUL(&lpbDataCurrent, 0);//02 00 00 00

		SetUL(&lpbDataCurrent, 0);//00 16 00 00
		SetUL(&lpbDataCurrent, 0);//00 02 00 00
		SetUL(&lpbDataCurrent, 0);//00 00 17 00
		SetUL(&lpbDataCurrent, 0);//00 00 02 00

		SetUL(&lpbDataCurrent, 0);//00 00 00 18
		SetUL(&lpbDataCurrent, 0);//00 00 00 02
		SetUL(&lpbDataCurrent, 0);//00 00 00 00
		SetUL(&lpbDataCurrent, 0);//19 00 00 00

		SetUL(&lpbDataCurrent, 0);//02 00 00 00
		SetUL(&lpbDataCurrent, 0);//00 1A 00 00
		SetUL(&lpbDataCurrent, 0);//00 02 00 00
		SetUL(&lpbDataCurrent, 0);//00 00 1C 00

		SetUL(&lpbDataCurrent, 0);//00 00 02 00
		SetUL(&lpbDataCurrent, 0);//00 00 00 1D
		SetUL(&lpbDataCurrent, 0);//00 00 00 02
		SetUL(&lpbDataCurrent, 0);//00 00 00 00

		SetUL(&lpbDataCurrent, 0);//23 00 00 00
		SetUL(&lpbDataCurrent, 0);//02 01 00 00
		SetUL(&lpbDataCurrent, 0);//00 24 00 00
		SetUL(&lpbDataCurrent, 0);//00 02 01 00

		SetUL(&lpbDataCurrent, 0);//00 00 25 00
		SetUL(&lpbDataCurrent, 0);//00 00 02 01
		SetUL(&lpbDataCurrent, 0);//00 00 00 26
		SetUL(&lpbDataCurrent, 0);//00 00 00 02

		SetUL(&lpbDataCurrent, 0);//00 00 00 00
		SetUL(&lpbDataCurrent, 0);//27 00 00 00
		SetUL(&lpbDataCurrent, 0);//02 00 00 00
		SetUL(&lpbDataCurrent, 0);//00 28 00 00

		SetUL(&lpbDataCurrent, 0);//00 02 00 00
		SetUL(&lpbDataCurrent, 0);//00 00 29 00
		SetUL(&lpbDataCurrent, 0);//00 00 02 01
		SetUL(&lpbDataCurrent, 0);//00 00 00 2A

		SetUL(&lpbDataCurrent, 0);//00 00 00 02
		SetUL(&lpbDataCurrent, 0);//00 00 00 00
		SetUL(&lpbDataCurrent, 0);//2B 00 00 00
		SetUL(&lpbDataCurrent, 0);//02 00 00 00

		SetUL(&lpbDataCurrent, 0);//00 2C 00 00
		SetUL(&lpbDataCurrent, 0);//00 01

		SetLPS(&lpbDataCurrent, "d3a4a3d0c95e5ba24f160a499ec8b4ea", 32);
*/


#define MRIM_CS_SMS       		0x1039 // C->S
// UL flags
// LPS to Phone ???
// LPS message ???

#define MRIM_CS_SMS_ACK			0x1040 // S->C
// UL status



#define MRIM_CS_PROXY			0x1044
// LPS          to e-mail ANSI
// DWORD        id_request
// DWORD        data_type
	#define MRIM_PROXY_TYPE_VOICE	1
	#define MRIM_PROXY_TYPE_FILES	2
	#define MRIM_PROXY_TYPE_CALLOUT	3
// LPS          user_data ???
// LPS          lps_ip_port ???
// DWORD        session_id[4]

#define MRIM_CS_PROXY_ACK		0x1045
//DWORD         status
	#define PROXY_STATUS_OK			1
	#define PROXY_STATUS_DECLINE		0
	#define PROXY_STATUS_ERROR		2
	#define PROXY_STATUS_INCOMPATIBLE_VERS	3
	#define PROXY_STATUS_NOHARDWARE		4
	#define PROXY_STATUS_MIRROR		5
	#define PROXY_STATUS_CLOSED		6
// LPS           to e-mail ANSI
// DWORD         id_request
// DWORD         data_type
// LPS           user_data ???
// LPS:          lps_ip_port ???
// DWORD[4]      Session_id

#define MRIM_CS_PROXY_HELLO		0x1046
// DWORD[4]      Session_id

#define MRIM_CS_PROXY_HELLO_ACK		0x1047



#define MRIM_CS_NEW_MAIL		0x1048 // S->C
// UL unread count
// LPS from e-mail ANSI
// LPS subject ???
// UL date
// UL uidl




#define MRIM_CS_USER_BLOG_STATUS	0x1063
// DWORD flags
	#define MRIM_BLOG_STATUS_UPDATE		0x00000001
	#define MRIM_BLOG_STATUS_MUSIC		0x00000002 // add music to status
	#define MRIM_BLOG_STATUS_REPLY		0x00000004
	#define MRIM_BLOG_STATUS_NOTIFY		0x00000010 // not set self status, alert only
// LPS user
// UINT64 id
// DWORD time
// LPS text (MRIM_BLOG_STATUS_MUSIC: track)
// LPS reply_user_nick

#define MRIM_CS_CHANGE_USER_BLOG_STATUS	0x1064
// DWORD flags
// LPS text (MRIM_BLOG_STATUS_MUSIC: track)
	#define MICBLOG_STATUS_MAX 500
// switch (flags) {
// MRIM_BLOG_STATUS_REPLY:
//      UINT64 orig_id
// }


#define MRIM_CS_UNKNOWN			0x1073
// DWORD ???
// DWORD ???

#define MRIM_CS_UNKNOWN2		0x1074 /* possible mail notification */



#define MRIM_CS_USER_GEO		0x1077
// LPS user
// LPS:	// DWORD flags?
// LPS: "geo-point":
	// LPS dolgota
	// LPS shirota
	// LPS "MAPOBJECT"
	// LPS some data?
	// LPS/DWORD
	// LPS/DWORD
	// LPS some data?
	// LPS some data?


#define MRIM_CS_SERVER_SETTINGS		0x1079





typedef struct mrim_connection_params_t
{
	unsigned long	ping_period;
}
mrim_connection_params_t;



#endif // MRIM_PROTO_H
