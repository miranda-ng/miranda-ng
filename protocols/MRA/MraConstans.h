#if !defined(AFX_MRA_CONSTANS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_CONSTANS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#include "proto.h"



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define MIN_MIR_VER_VERSION_DWORD			PLUGIN_MAKE_VERSION(0,9,0,0)


#define PLUGIN_VERSION_DWORD				PLUGIN_MAKE_VERSION(PROTO_VERSION_MAJOR,PROTO_VERSION_MINOR,0,1)
#define PROTOCOL_DISPLAY_NAME_ORIGA			"Mail.ru Agent"

#define MIRVER_UNKNOWN						("Mail.ru Agent unknown client")


#define PROTOCOL_NAMEA						masMraSettings.szModuleName
#define PROTOCOL_NAMEW						masMraSettings.wszModuleName
#define PROTOCOL_NAME_LEN					masMraSettings.dwModuleNameLen
#define PROTOCOL_NAME_SIZE					(PROTOCOL_NAME_LEN+1)
#define PROTOCOL_DISPLAY_NAMEA				masMraSettings.szDisplayModuleName
#define PROTOCOL_DISPLAY_NAMEW				masMraSettings.wszDisplayModuleName


#define MRA_PLUGIN_UPDATER_ID				2544 //появляется после первого выкладывания на сайт
#define MRA_SERVER_PORT_STANDART_NLB		2042
#define MRA_SERVER_PORT_STANDART			2041
#define MRA_SERVER_PORT_HTTPS				443
static const LPSTR lpcszMailRuDomains[]=
{
	("mail.ru"),
	("list.ru"),
	("bk.ru"),
	("inbox.ru"),
	("corp.mail.ru"),
	NULL
};
#define MAILRU_CHAT_CONF_DOMAIN				"chat.agent"


#define MAX_EMAIL_LEN						1024
#define MAX_FILEPATH						32768 //internal
#define BUFF_SIZE_RCV						65535 //internal
#define BUFF_SIZE_RCV_MIN_FREE				16384 //internal
#define BUFF_SIZE_BLOB						16384 //internal
#define BUFF_SIZE_URL						4096 //internal
#define NETLIB_SELECT_TIMEOUT				250 //internal // время ожидания событий на сокете
#define WAIT_FOR_THREAD_TIMEOUT				15 //internal // время ожидания завершения потока
#define THREAD_SLEEP_TIME					100 //internal
#define THREAD_MAX_PING_TIME				20 // sec, internal
#define THREAD_MAX_PING_FAIL_COUNT			3 // internal
#define SEND_QUEUE_TIMEOUT					600 //internal // время удаления недоставленных сообщений из очереди отправки
#define ALLOCATED_COUNT						32 //internal	// колличество элементов
#define EMAILS_MIN_COUNT					16 //internal	// колличество элементов обязательно проверяемых при извлечении email адреса из инфы юзера
#define PHONES_MIN_COUNT					4 //internal	// колличество элементов обязательно проверяемых при извлечении email адреса из инфы юзера



#define MRA_ALARM_MESSAGE					L"Your contact wakes you"

#define MRA_GOTO_INBOX						"/GotoInbox"
#define MRA_GOTO_INBOX_STR					L"Display &Inbox"
#define MRA_SHOW_INBOX_STATUS				"/ShowInboxStatus"
#define MRA_SHOW_INBOX_STATUS_STR			L"Display &Inbox status"
#define MRA_EDIT_PROFILE					"/EditProfile"
#define MRA_EDIT_PROFILE_STR				L"Edit &Profile"
#define MRA_MY_ALBUM						"/MyAlbum"
#define MRA_MY_ALBUM_STR					L"My Album"
#define MRA_MY_BLOG							"/MyBlog"
#define MRA_MY_BLOG_STR						L"My Blog"
#define MRA_MY_BLOGSTATUS					"/MyBlogStatus"
#define MRA_MY_BLOGSTATUS_STR				L"My Blog Status"
#define MRA_MY_VIDEO						"/MyVideo"
#define MRA_MY_VIDEO_STR					L"My Video"
#define MRA_MY_ANSWERS						"/MyAnswers"
#define MRA_MY_ANSWERS_STR					L"My Answers"
#define MRA_MY_WORLD						"/MyWorld"
#define MRA_MY_WORLD_STR					L"My World"
#define MRA_ZHUKI							"/Zhuki"
#define MRA_ZHUKI_STR						L"Zhuki"
#define MRA_CHAT							"/Chat"
#define MRA_CHAT_STR						L"Chat"
#define MRA_WEB_SEARCH						"/WebSearch"
#define MRA_WEB_SEARCH_STR					L"Web search"
#define MRA_UPD_ALL_USERS_INFO				"/UpdateAllUsersInfo"
#define MRA_UPD_ALL_USERS_INFO_STR			L"Update all users info"
#define MRA_CHK_UPDATES_USERS_AVATARS		"/CheckUpdatesUsersAvatars"
#define MRA_CHK_UPDATES_USERS_AVATARS_STR	L"Check updates users avatars"
#define MRA_REQ_AUTH_FOR_ALL				"/ReqAuthForAll"
#define MRA_REQ_AUTH_FOR_ALL_STR			L"Request authorization for all"


#define MRA_MPOP_AUTH_URL					"http://swa.mail.ru/cgi-bin/auth?Login=%s&agent=%s&page=%s"

#define MRA_WIN_INBOX_URL					"http://win.mail.ru/cgi-bin/start"
#define MRA_PDA_INBOX_URL					"http://pda.mail.ru/cgi-bin/start"
#define MRA_EDIT_PROFILE_URL				"http://win.mail.ru/cgi-bin/userinfo?mra=1"
#define MRA_CHAT_URL						"http://chat.mail.ru"
#define MRA_ZHUKI_URL						"http://zhuki.mail.ru"
#define MRA_SEARCH_URL						"http://go.mail.ru"

// used spesialy! added: /domain/user
#define MRA_BLOGS_URL						"http://blogs.mail.ru"
#define MRA_FOTO_URL						"http://foto.mail.ru"
#define MRA_VIDEO_URL						"http://video.mail.ru"
#define MRA_ANSWERS_URL						"http://otvet.mail.ru"
#define MRA_WORLD_URL						"http://my.mail.ru"


// without auth
#define MRA_REGISTER_URL					"http://win.mail.ru/cgi-bin/signup"
#define MRA_FORGOT_PASSWORD_URL				"http://win.mail.ru/cgi-bin/passremind"


// wParam=(WPARAM)hContact
#define MRA_REQ_AUTH						"/ReqAuth"
#define MRA_REQ_AUTH_STR					L"Request authorization"
#define MRA_GRANT_AUTH						"/GrantAuth"
#define MRA_GRANT_AUTH_STR					L"Grant authorization"
#define MRA_SEND_POSTCARD					"/SendPostcard"
#define MRA_SEND_POSTCARD_STR				L"&Send postcard"
#define MRA_VIEW_ALBUM						"/ViewAlbum"
#define MRA_VIEW_ALBUM_STR					L"&View Album"
#define MRA_READ_BLOG						"/ReadBlog"
#define MRA_READ_BLOG_STR					L"&Read Blog"
#define MRA_REPLY_BLOG_STATUS				"/ReplyBlogStatus"
#define MRA_REPLY_BLOG_STATUS_STR			L"Reply Blog Status"
#define MRA_VIEW_VIDEO						"/ViewVideo"
#define MRA_VIEW_VIDEO_STR					L"View Video"
#define MRA_ANSWERS							"/Answers"
#define MRA_ANSWERS_STR						L"Answers"
#define MRA_WORLD							"/World"
#define MRA_WORLD_STR						L"World"
#define MRA_SEND_NUDGE						"/SendNudge"
#define MS_NUDGE							"/Nudge"
#define MRA_SENDNUDGE_STR					L"Send &Nudge"


#define ADV_ICON_DELETED					0
#define ADV_ICON_DELETED_ID					"ADV_ICON_DELETED"
#define ADV_ICON_DELETED_STR				L"Mail box deleted"
#define ADV_ICON_NOT_ON_SERVER				1
#define ADV_ICON_NOT_ON_SERVER_ID			"ADV_ICON_NOT_ON_SERVER"
#define ADV_ICON_NOT_ON_SERVER_STR			L"Contact not on server"
#define ADV_ICON_NOT_AUTHORIZED				2
#define ADV_ICON_NOT_AUTHORIZED_ID			"ADV_ICON_NOT_AUTHORIZED"
#define ADV_ICON_NOT_AUTHORIZED_STR			L"Not authorized"
#define ADV_ICON_PHONE						3
#define ADV_ICON_PHONE_ID					"ADV_ICON_PHONE"
#define ADV_ICON_PHONE_STR					L"Phone/SMS only contact"
#define ADV_ICON_BLOGSTATUS					4
#define ADV_ICON_BLOGSTATUS_ID				"ADV_ICON_BLOGSTATUS"
#define ADV_ICON_BLOGSTATUS_STR				L"Blog status message"
#define ADV_ICON_MAX						5

static const GUI_DISPLAY_ITEM gdiExtraStatusIconsItems[] =
{
	{ ADV_ICON_DELETED_ID,			ADV_ICON_DELETED_STR,			(INT_PTR)IDI_ERROR,	NULL	},
	{ ADV_ICON_NOT_ON_SERVER_ID,	ADV_ICON_NOT_ON_SERVER_STR,		IDI_AUTHGRANT,		NULL	},
	{ ADV_ICON_NOT_AUTHORIZED_ID,	ADV_ICON_NOT_AUTHORIZED_STR,	IDI_AUTHRUGUEST,	NULL	},
	{ ADV_ICON_PHONE_ID,			ADV_ICON_PHONE_STR,				IDI_MRA_PHONE,		NULL	},
	{ ADV_ICON_BLOGSTATUS_ID,		ADV_ICON_BLOGSTATUS_STR,		IDI_BLOGSTATUS,		NULL	},
};




#define MRA_SOUND_NEW_EMAIL						"New E-mail available in Inbox"


#define MAILRU_SERVER_TIME_ZONE					-180 //internal // +0300
#define MAILRU_CONTACTISTYPING_TIMEOUT			10 //internal
#define MRA_DEFAULT_SERVER						"mrim.mail.ru"
#define MRA_DEFAULT_SERVER_PORT					MRA_SERVER_PORT_STANDART_NLB
#define MRA_MAX_MRIM_SERVER						46
#define MRA_DEFAULT_TIMEOUT_CONN_MRIM			20
#define MRA_DEFAULT_CONN_RETRY_COUNT_MRIM		2
#define MRA_DEFAULT_TIMEOUT_CONN_NLB			20
#define MRA_DEFAULT_CONN_RETRY_COUNT_NLB		3
#define MRA_DEFAULT_TIMEOUT_RECV_NLB			20
#define MRA_DEFAULT_CONN_RETRY_COUNT_FILES		2
#define MRA_DEFAULT_CONN_RETRY_COUNT_MRIMPROXY	3
#define MRA_TIMEOUT_DIRECT_CONN					30 //internal; время после которого клиент считает что к нему подключится не удалось
#define MRA_TIMEOUT_CONN_MIN					2 //internal; минимальное время ожидания пока устанавливается исходящее подключение
#define MRA_TIMEOUT_CONN_МАХ					16 //internal; максимальное время ожидания пока устанавливается исходящее подключение
#define MRA_MAXLENOFMESSAGE						32768 //internal
#define MRA_MAXCONTACTSPERPACKET				256 //internal
#define MRA_CODE_PAGE							1251 //internal
#define MRA_FEATURE_FLAGS						(FEATURE_FLAG_BASE_SMILES|FEATURE_FLAG_ADVANCED_SMILES|FEATURE_FLAG_CONTACTS_EXCH|FEATURE_FLAG_WAKEUP|FEATURE_FLAG_MULTS|FEATURE_FLAG_FILE_TRANSFER|FEATURE_FLAG_GAMES)



#define MRA_DEFAULT_POPUPS_ENABLED				TRUE
#define MRA_DEFAULT_POPUPS_EVENT_FILTER			-1
#define MRA_DEFAULT_POPUP_TIMEOUT				8
#define MRA_DEFAULT_POPUP_USE_WIN_COLORS		TRUE
#define MRA_DEFAULT_POPUP_COLOR_BACK			RGB(191,0,0)		//Red
#define MRA_DEFAULT_POPUP_COLOR_TEXT			RGB(255,245,225)	//Yellow



#define MRA_DEFAULT_NLB_FAIL_DIRECT_CONNECT		FALSE // hidden option
#define MRA_DEFAULT_AUTO_ADD_CONTACTS_TO_SERVER	TRUE
#define MRA_DEFAULT_AUTO_AUTH_REQ_ON_LOGON		FALSE
#define MRA_DEFAULT_AUTO_AUTH_GRAND_IN_CLIST	FALSE
#define MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS	FALSE
#define MRA_DEFAULT_AUTO_AUTH_GRAND_NEW_USERS_DISABLE_SPAM_CHECK	FALSE
#define MRA_DEFAULT_SLOW_SEND					TRUE
#define MRA_DEFAULT_CVT_SMILES_TO_TAGS			TRUE
#define MRA_DEFAULT_MIRVER_RAW					FALSE // hidden option
#define MRA_DEFAULT_AUTH_MESSAGE				L"Please authorize me"

#define MRA_DEFAULT_RTF_RECEIVE_ENABLE			FALSE
#define MRA_DEFAULT_RTF_SEND_ENABLE				TRUE
#define MRA_DEFAULT_RTF_SEND_SMART				TRUE
#define MRA_DEFAULT_RTF_BACKGROUND_COLOUR		RGB(255,255,255)
#define MRA_DEFAULT_RTF_FONT_COLOUR				RGB(255,0,0)
#define MRA_DEFAULT_RTF_FONT_SIZE				12
#define MRA_DEFAULT_RTF_FONT_CHARSET			RUSSIAN_CHARSET
#define MRA_DEFAULT_RTF_FONT_NAME				L"Tahoma"

#define MRA_DEFAULT_SEARCH_REMEMBER				TRUE
#define MRA_DEFAULT_LAST_SEARCH_COUTRY			0


#define MRA_DEFAULT_FILE_SEND_BLOCK_SIZE		8192 //hidden, размер блока при отправке файла

#define MRA_DEFAULT_HIDE_MENU_ITEMS_FOR_NON_MRA			FALSE

#define MRA_DEFAULT_INC_NEW_MAIL_NOTIFY					FALSE
#define MRA_DEFAULT_TRAYICON_NEW_MAIL_NOTIFY			FALSE
#define MRA_DEFAULT_TRAYICON_NEW_MAIL_CLK_TO_INBOX		FALSE



#define MRA_ANTISPAM_DEFAULT_ENABLE						TRUE
#define MRA_ANTISPAM_DEFAULT_CHK_TEMP_CONTACTS			TRUE
#define MRA_ANTISPAM_DEFAULT_DELETE_SPAMBOT_CONTACT		FALSE
#define MRA_ANTISPAM_DEFAULT_CLN_NON_ALPHNUM			TRUE
#define MRA_ANTISPAM_DEFAULT_MAX_LNG_CHANGES			5
#define MRA_ANTISPAM_DEFAULT_SHOWPOP					TRUE
#define MRA_ANTISPAM_DEFAULT_WRITETOSYSTEMHISTORY		TRUE
#define MRA_ANTISPAM_DEFAULT_SEND_SPAM_REPORT_TO_SERVER	TRUE


#define MRA_AVT_DEFAULT_ENABLE					TRUE
#define MRA_AVT_DEFAULT_WRK_THREAD_COUNTS		4 // hidden
#define MRA_AVT_DEFAULT_SERVER					"obraz.foto.mail.ru"
#define MRA_AVT_DEFAULT_SERVER_PORT				80
#define MRA_AVT_DEFAULT_TIMEOUT_CONN			10
#define MRA_AVT_DEFAULT_CONN_RETRY_COUNT		3
#define MRA_AVT_DEFAULT_TIMEOUT_RECV			30
#define MRA_AVT_DEFAULT_USE_KEEPALIVE_CONN		TRUE
#define MRA_AVT_DEFAULT_CHK_INTERVAL			60
#define MRA_AVT_DEFAULT_AVT_FILENAME			"_default.jpg"
#define MRA_AVT_DEFAULT_RET_ABC_PATH			TRUE
#define MRA_DELETE_AVT_ON_CONTACT_DELETE		TRUE
#define MRA_AVT_DEFAULT_QE_CHK_INTERVAL			1000 //internal


#define MRA_FILES_QUEUE_PROGRESS_INTERVAL		250 //internal
#define MRA_FILES_NULL_ADDRR					"192.168.0.1:26666;"
#define MRA_DEF_FS_TIMEOUT_RECV					600
#define MRA_DEF_FS_ENABLE_DIRECT_CONN			TRUE
#define MRA_DEF_FS_NO_OUT_CONN_ON_RCV			FALSE
#define MRA_DEF_FS_NO_OUT_CONN_ON_SEND			FALSE
#define MRA_DEF_FS_IGNORY_ADDITIONAL_PORTS		FALSE
#define MRA_DEF_FS_HIDE_MY_ADDRESSES			FALSE
#define MRA_DEF_FS_ADD_EXTRA_ADDRESSES			FALSE
#define MRA_DEF_FS_ENABLE_MRIM_PROXY_CONS		TRUE



#define PS_GETMYAVATAR "/GetMyAvatar"
//wParam=(char *)Buffer to file name
//lParam=(int)Buffer size
//return=0 for sucess



#define ICQACKTYPE_SMS      1001
#define ICQEVENTTYPE_SMS    2001    //database event type
#define MS_ICQ_SENDSMS      "/SendSMS" 


// Database setting names
#define DBSETTING_CAPABILITIES		"caps"
#define DBSETTING_XSTATUSID			"XStatusId"
#define DBSETTING_XSTATUSNAME		"XStatusName"
#define DBSETTING_XSTATUSMSG		"XStatusMsg"
#define DBSETTING_BLOGSTATUSTIME	"BlogStatusTime"
#define DBSETTING_BLOGSTATUSID		"BlogStatusID"
#define DBSETTING_BLOGSTATUS		"BlogStatus"
#define DBSETTING_BLOGSTATUSMUSIC	"ListeningTo"

#define PS_ICQ_SETCUSTOMSTATUS		"/SetXStatus"
#define PS_ICQ_SETCUSTOMSTATUSEX	"/SetXStatusEx"
#define PS_ICQ_GETCUSTOMSTATUS		"/GetXStatus"
#define PS_ICQ_GETCUSTOMSTATUSEX	"/GetXStatusEx"
#define PS_ICQ_GETCUSTOMSTATUSICON	"/GetXStatusIcon"


#define CSSF_MASK_STATUS    0x0001  // status member valid for set/get
#define CSSF_MASK_NAME      0x0002  // pszName member valid for set/get
#define CSSF_MASK_MESSAGE   0x0004  // pszMessage member valid for set/get
#define CSSF_DISABLE_UI     0x0040  // disable default custom status UI, wParam = bEnable
#define CSSF_DEFAULT_NAME   0x0080  // only with CSSF_MASK_NAME and get API to get default custom status name (wParam = status)
#define CSSF_STATUSES_COUNT 0x0100  // returns number of custom statuses in wParam, only get API
#define CSSF_STR_SIZES      0x0200  // returns sizes of custom status name & message (wParam & lParam members) in chars
#define CSSF_UNICODE        0x1000  // strings are in UCS-2



typedef struct {
	int cbSize;				// size of the structure
	int flags;				// combination of CSSF_*
	int *status;			// custom status id
	union {
		char *pszName;		// buffer for custom status name
		TCHAR *ptszName;
		WCHAR *pwszName;
	};
	union {
		char *pszMessage;	// buffer for custom status message
		TCHAR *ptszMessage;
		WCHAR *pwszMessage;
	};
	WPARAM *wParam;			// extra params, see flags
	LPARAM *lParam;
} ICQ_CUSTOM_STATUS;











INT_PTR			MraGetCaps				(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetName				(WPARAM wParam,LPARAM lParam);
INT_PTR			MraLoadIcon				(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSetStatus			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetStatus			(WPARAM wParam,LPARAM lParam);

INT_PTR			MraSetXStatus			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSetXStatusEx			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetXStatus			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetXStatusEx			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetXStatusIcon		(WPARAM wParam,LPARAM lParam);

INT_PTR			MraSetListeningTo		(WPARAM wParam,LPARAM lParam);

INT_PTR			MraSetAwayMsgA			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSetAwayMsg			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetAwayMsg			(WPARAM wParam,LPARAM lParam);

INT_PTR			MraAuthAllow			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraAuthDeny				(WPARAM wParam,LPARAM lParam);
INT_PTR			MraAddToList			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraAddToListByEvent		(WPARAM wParam,LPARAM lParam);

INT_PTR			MraRecvMessage			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraRecvContacts			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraRecvFile				(WPARAM wParam,LPARAM lParam);
INT_PTR			MraRecvAuth				(WPARAM wParam,LPARAM lParam);

INT_PTR			MraSendAuthRequest		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSendMessage			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSendContacts			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSendUserIsTyping		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSendNudge			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSetApparentMode		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetInfo				(WPARAM wParam,LPARAM lParam);

INT_PTR			MraBasicSearch			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSearchByEmail		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSearchByName			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraCreateAdvSearchUI	(WPARAM wParam,LPARAM lParam);
INT_PTR			MraSearchByAdvanced		(WPARAM wParam,LPARAM lParam);

INT_PTR			MraGetAvatarCaps		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetAvatarInfo		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraGetMyAvatar			(WPARAM wParam,LPARAM lParam);

INT_PTR			MraFileResume			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraFileAllow			(WPARAM wParam,LPARAM lParam);
INT_PTR			MraFileDenyCancel		(WPARAM wParam,LPARAM lParam);
INT_PTR			MraFileSend				(WPARAM wParam,LPARAM lParam);


INT_PTR			MraSendSMS				(WPARAM wParam,LPARAM lParam);


static const SERVICE_ITEM siPluginServices[] =
{
	{ PS_GETCAPS,						MraGetCaps },
	{ PS_GETNAME,						MraGetName },
	{ PS_LOADICON,						MraLoadIcon },
	{ PS_SETSTATUS,						MraSetStatus },
	{ PS_GETSTATUS,						MraGetStatus },
	
	{ PS_ICQ_SETCUSTOMSTATUS,			MraSetXStatus },
	{ PS_ICQ_SETCUSTOMSTATUSEX,			MraSetXStatusEx },
	{ PS_ICQ_GETCUSTOMSTATUS,			MraGetXStatus },
	{ PS_ICQ_GETCUSTOMSTATUSEX,			MraGetXStatusEx },
	{ PS_ICQ_GETCUSTOMSTATUSICON,		MraGetXStatusIcon },

	{ PS_SET_LISTENINGTO,				MraSetListeningTo },

	{ PS_SETAWAYMSG,					MraSetAwayMsgA },
	{ PS_SETAWAYMSGW,					MraSetAwayMsg },
	{ PSS_GETAWAYMSG,					MraGetAwayMsg },

	{ PS_AUTHALLOW,						MraAuthAllow },
	{ PS_AUTHDENYW,						MraAuthDeny },

	{ PS_ADDTOLIST,						MraAddToList },
	{ PS_ADDTOLISTBYEVENT,				MraAddToListByEvent },

	{ PSR_MESSAGE,						MraRecvMessage },
	{ PSR_CONTACTS,						MraRecvContacts },
	{ PSR_FILE,							MraRecvFile },
	{ PSR_AUTH,							MraRecvAuth },

	{ PSS_AUTHREQUESTW,					MraSendAuthRequest },
	{ PSS_MESSAGE,						MraSendMessage },
	//{ PSS_MESSAGE"W",					MraSendMessage },
	{ PSS_CONTACTS,						MraSendContacts },
	{ PSS_USERISTYPING,					MraSendUserIsTyping },
	{ PSS_SETAPPARENTMODE,				MraSetApparentMode },
	{ PSS_GETINFO,						MraGetInfo },

	{ PS_GETAVATARCAPS,					MraGetAvatarCaps },
	{ PS_GETAVATARINFO,					MraGetAvatarInfo },
	{ PS_GETMYAVATAR,					MraGetMyAvatar },

	{ PS_BASICSEARCHW,					MraBasicSearch },
	{ PS_SEARCHBYEMAILW,				MraSearchByEmail },
	{ PS_SEARCHBYNAMEW,					MraSearchByName },
	{ PS_CREATEADVSEARCHUI,				MraCreateAdvSearchUI },
	{ PS_SEARCHBYADVANCED,				MraSearchByAdvanced },

	{ PS_FILERESUMEW,					MraFileResume },
	{ PSS_FILEALLOWW,					MraFileAllow },
	{ PSS_FILEDENYW,					MraFileDenyCancel },
	{ PSS_FILECANCEL,					MraFileDenyCancel },
	{ PSS_FILEW,						MraFileSend },

	{ MS_ICQ_SENDSMS,					MraSendSMS },
	{ MRA_SEND_NUDGE,					MraSendNudge },
};




INT_PTR MraGotoInbox(WPARAM wParam,LPARAM lParam);
INT_PTR MraShowInboxStatus(WPARAM wParam,LPARAM lParam);
INT_PTR MraEditProfile(WPARAM wParam,LPARAM lParam);
INT_PTR MyAlbum(WPARAM wParam,LPARAM lParam);
INT_PTR MyBlog(WPARAM wParam,LPARAM lParam);
INT_PTR MyBlogStatus(WPARAM wParam,LPARAM lParam);
INT_PTR MyVideo(WPARAM wParam,LPARAM lParam);
INT_PTR MyAnswers(WPARAM wParam,LPARAM lParam);
INT_PTR MyWorld(WPARAM wParam,LPARAM lParam);
INT_PTR MraZhuki(WPARAM wParam,LPARAM lParam);
INT_PTR MraChat(WPARAM wParam,LPARAM lParam);
INT_PTR MraWebSearch(WPARAM wParam,LPARAM lParam);
INT_PTR MraUpdateAllUsersInfo(WPARAM wParam,LPARAM lParam);
INT_PTR MraCheckUpdatesUsersAvt(WPARAM wParam,LPARAM lParam);
INT_PTR MraRequestAuthForAll(WPARAM wParam,LPARAM lParam);

static const GUI_DISPLAY_ITEM gdiMenuItems[] =
{
	{ MRA_GOTO_INBOX,					MRA_GOTO_INBOX_STR,					IDI_INBOX,			MraGotoInbox },
	{ MRA_SHOW_INBOX_STATUS,			MRA_SHOW_INBOX_STATUS_STR,			IDI_MAIL_NOTIFY,	MraShowInboxStatus },
	{ MRA_EDIT_PROFILE,					MRA_EDIT_PROFILE_STR,				IDI_PROFILE,		MraEditProfile },
	{ MRA_MY_ALBUM,						MRA_MY_ALBUM_STR,					IDI_MRA_PHOTO,		MyAlbum },
	{ MRA_MY_BLOG,						MRA_MY_BLOG_STR,					IDI_MRA_BLOGS,		MyBlog },
	{ MRA_MY_BLOGSTATUS,				MRA_MY_BLOGSTATUS_STR,				IDI_BLOGSTATUS,		MyBlogStatus },
	{ MRA_MY_VIDEO,						MRA_MY_VIDEO_STR,					IDI_MRA_VIDEO,		MyVideo },
	{ MRA_MY_ANSWERS,					MRA_MY_ANSWERS_STR,					IDI_MRA_ANSWERS,	MyAnswers },
	{ MRA_MY_WORLD,						MRA_MY_WORLD_STR,					IDI_MRA_WORLD,		MyWorld },
	{ MRA_ZHUKI,						MRA_ZHUKI_STR,						IDI_MRA_ZHUKI,		MraZhuki },
	{ MRA_CHAT,							MRA_CHAT_STR,						IDI_MRA_CHAT,		MraChat },
	{ MRA_WEB_SEARCH,					MRA_WEB_SEARCH_STR,					IDI_MRA_WEB_SEARCH,	MraWebSearch },
	{ MRA_UPD_ALL_USERS_INFO,			MRA_UPD_ALL_USERS_INFO_STR,			IDI_PROFILE,		MraUpdateAllUsersInfo },
	{ MRA_CHK_UPDATES_USERS_AVATARS,	MRA_CHK_UPDATES_USERS_AVATARS_STR,	IDI_PROFILE,		MraCheckUpdatesUsersAvt },
	{ MRA_REQ_AUTH_FOR_ALL,				MRA_REQ_AUTH_FOR_ALL_STR,			IDI_AUTHRUGUEST,	MraRequestAuthForAll }
};



INT_PTR MraRequestAuthorization(WPARAM wParam,LPARAM lParam);
INT_PTR MraGrantAuthorization(WPARAM wParam,LPARAM lParam);
INT_PTR MraSendPostcard(WPARAM wParam,LPARAM lParam);
INT_PTR MraViewAlbum(WPARAM wParam,LPARAM lParam);
INT_PTR MraReadBlog(WPARAM wParam,LPARAM lParam);
INT_PTR MraReplyBlogStatus(WPARAM wParam,LPARAM lParam);
INT_PTR MraViewVideo(WPARAM wParam,LPARAM lParam);
INT_PTR MraAnswers(WPARAM wParam,LPARAM lParam);
INT_PTR MraWorld(WPARAM wParam,LPARAM lParam);

static const GUI_DISPLAY_ITEM gdiContactMenuItems[] =
{
	{ MRA_REQ_AUTH,			MRA_REQ_AUTH_STR,		IDI_AUTHRUGUEST,	MraRequestAuthorization },
	{ MRA_GRANT_AUTH,		MRA_GRANT_AUTH_STR,		IDI_AUTHGRANT,		MraGrantAuthorization },
	{ MRA_SEND_POSTCARD,	MRA_SEND_POSTCARD_STR,	IDI_MRA_POSTCARD,	MraSendPostcard },
	{ MRA_VIEW_ALBUM,		MRA_VIEW_ALBUM_STR,		IDI_MRA_PHOTO,		MraViewAlbum },
	{ MRA_READ_BLOG,		MRA_READ_BLOG_STR,		IDI_MRA_BLOGS,		MraReadBlog },
	{ MRA_REPLY_BLOG_STATUS,MRA_REPLY_BLOG_STATUS_STR,IDI_BLOGSTATUS,	MraReplyBlogStatus },
	{ MRA_VIEW_VIDEO,		MRA_VIEW_VIDEO_STR,		IDI_MRA_VIDEO,		MraViewVideo },
	{ MRA_ANSWERS,			MRA_ANSWERS_STR,		IDI_MRA_ANSWERS,	MraAnswers },
	{ MRA_WORLD,			MRA_WORLD_STR,			IDI_MRA_WORLD,		MraWorld },
	{ MRA_SEND_NUDGE,		MRA_SENDNUDGE_STR,		IDI_MRA_ALARM,		NULL }
};







static const LPSTR lpcszStatusUri[]=
{
	"",// offline // "status_0",
	"STATUS_ONLINE",// "status_1",
	"STATUS_AWAY",	// "status_2",
	"STATUS_INVISIBLE",// "status_3",
	"status_dnd",
	"status_chat",
	"status_4",
	"status_5",
	"status_6",
	"status_7",
	"status_8",
	"status_9",
	"status_10",
	"status_11",
	"status_12",
	"status_13",
	"status_14",
	"status_15",
	"status_16",
	"status_17",
	"status_18",
	"status_19",
	"status_20",
	"status_21",
	"status_22",
	"status_23",
	"status_24",
	//"status_25", // chat/dnd
	"status_26",
	"status_27",
	"status_28",
	"status_29",
	"status_30",
	//"status_31", // chat/dnd
	"status_32",
	"status_33",
	"status_34",
	"status_35",
	"status_36",
	"status_37",
	"status_38",
	"status_39",
	"status_40",
	"status_41",
	"status_42",
	"status_43",
	"status_44",
	"status_45",
	"status_46",
	"status_47",
	"status_48",
	"status_49",
	"status_50",
	"status_51",
	"status_52",
	"status_53",
	"status_dating",
	//"status_127",145,154
	NULL
};

static const LPWSTR lpcszXStatusNameDef[]=
{
	L"None",
	L"Sick",
	L"Home",
	L"Eating",
	L"Compass",
	L"On WC",
	L"Cooking",
	L"Walking",
	L"Alien",
	L"Shrimp",
	L"Got lost",
	L"Crazy",
	L"Duck",
	L"Playing",
	L"Smoking",
	L"Office",
	L"Meeting",
	L"Beer",
	L"Coffee",
	L"Working",
	L"Relaxing",
	L"On the phone",
	L"In institute",
	L"At school",
	L"Wrong number",
	L"Laughing",
	L"Malicious",
	L"Imp",
	L"Blind",
	L"Disappointed",
	L"Almost crying",
	L"Fearful",
	L"Angry",
	L"Vampire",
	L"Ass",
	L"Love",
	L"Sleeping",
	L"Cool!",
	L"Peace!",
	L"Cock a snook",
	L"Get out",
	L"Death",
	L"Rocket",
	L"Devil-fish",
	L"Heavy metal",
	L"Things look bad",
	L"Squirrel",
	L"Star",
	L"Music",
	L"Dating",
	NULL
};

#define MRA_XSTATUS_MENU			"/menuXStatus"
#define MRA_XSTATUS_COUNT			50	
#define MRA_MIR_XSTATUS_NONE		0
#define MRA_MIR_XSTATUS_UNKNOWN		MRA_XSTATUS_COUNT

#define MRA_XSTATUS_OFFLINE			0
#define MRA_XSTATUS_ONLINE			1
#define MRA_XSTATUS_AWAY			2
#define MRA_XSTATUS_INVISIBLE		3
#define MRA_XSTATUS_DND				4
#define MRA_XSTATUS_CHAT			5
#define MRA_XSTATUS_UNKNOWN			55

#define MRA_XSTATUS_INDEX_OFFSET	6



#endif // !defined(AFX_MRA_CONSTANS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
