////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Bia³ek
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#ifndef GG_H
#define GG_H

#define MIRANDA_VER 0x0A00

#if defined(__DEBUG__) || defined(_DEBUG) || defined(DEBUG)
#define DEBUGMODE // Debug Mode
#endif

#if _WIN32_WINNT < 0x0501
#define _WIN32_WINNT 0x0501
#endif

#include <m_stdhdr.h>

#ifdef __cplusplus
extern "C" {
#endif

// Windows headers
// Visual C++ .NET tries to include winsock.h
// which is very ver bad
#if (_MSC_VER >= 1300)
#include <winsock2.h>
#else
#include <windows.h>
#endif
#include <commctrl.h>
#include <commdlg.h>
#include <process.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

// Miranda IM headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_langpack.h>
#include <m_plugins.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_clui.h>
#include <m_button.h>
#include <m_clc.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_imgsrvc.h>
#include <m_genmenu.h>
#include <m_file.h>
#include <m_avatars.h>
#include <m_xml.h>
#include <m_chat.h>
#include <m_popup.h>
#include <win2k.h>

// Custom profile folders plugin header
#include "m_folders.h"

// Visual C++ extras
#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define GGINLINE
#else
#define GGINLINE inline
#endif

// Plugin headers
#include "resource.h"

// libgadu headers
#include "libgadu/libgadu.h"
#include "dynstuff.h"

// Search
// Extended search result structure, used for all searches
typedef struct
{
	PROTOSEARCHRESULT hdr;
	uin_t uin;
} GGSEARCHRESULT;

typedef struct
{
	HANDLE hThread;
	DWORD dwThreadId;
} GGTHREAD;

typedef struct
{
	PROTO_INTERFACE proto;
	LPTSTR name;
	CRITICAL_SECTION ft_mutex, sess_mutex, img_mutex, modemsg_mutex, avatar_mutex, sessions_mutex;
	list_t watches, transfers, requests, chats, imagedlgs, avatar_requests, avatar_transfers, sessions;
	int gc_enabled, gc_id, list_remove, unicode_core, check_first_conn;
	uin_t next_uin;
	unsigned long last_crc;
	GGTHREAD pth_dcc;
	GGTHREAD pth_sess;
	GGTHREAD pth_avatar;
	struct gg_session *sess;
	struct gg_dcc *dcc;
	HANDLE event;
	HANDLE hConnStopEvent;
	SOCKET sock;
	UINT_PTR timer;
	struct
	{
		char *online;
		char *away;
		char *dnd;
		char *freechat;
		char *invisible;
		char *offline;
	} modemsg;
	HANDLE netlib,
		hookOptsInit,
		hookUserInfoInit,
		hookGCUserEvent,
		hookGCMenuBuild;
	HGENMENU hMenuRoot;
	HGENMENU hMainMenu[7];
	HANDLE hPrebuildMenuHook;
	HANDLE hBlockMenuItem;
	HANDLE hImageMenuItem;
	HANDLE hInstanceMenuItem;
	HANDLE hAvatarsFolder;
	HANDLE hImagesFolder;
	HANDLE hwndSessionsDlg;
} GGPROTO;

typedef struct
{
	int mode;
	uin_t uin;
	char *pass;
	char *email;
	GGPROTO *gg;
} GGUSERUTILDLGDATA;

typedef struct
{
	uin_t *recipients;
	int recipients_count;
	char id[32];
	BOOL ignore;
} GGGC;

typedef struct
{
	char id[256];
	char val[256];
} GGTOKEN;

// GG Thread Function
typedef void (__cdecl GGThreadFunc)(void*, void*);

#if 0 /* #ifdef DEBUGMODE */
#define EnterCriticalSection(lpCS)	{gg_netlog(gg,"EnterCriticalSection @ %s:%d", __FILE__, __LINE__); EnterCriticalSection(lpCS);}
#define LeaveCriticalSection(lpCS)	{gg_netlog(gg,"LeaveCriticalSection @ %s:%d", __FILE__, __LINE__); LeaveCriticalSection(lpCS);}
#endif


// Wrappers of the old interface
#define GG_PROTO		(gg->proto.m_szModuleName)
#define GG_PROTONAME	(gg->name)
#define GGDEF_PROTO 	 "GG"        // Default Proto
#define GGDEF_PROTONAME  "Gadu-Gadu" // Default ProtoName


// Process handles / seqs
#define GG_SEQ_INFO				100
#define GG_SEQ_SEARCH			200
#define GG_SEQ_GETNICK			300
#define GG_SEQ_CHINFO			400

// Services
#define GGS_IMPORT_SERVER		"%s/ImportFromServer"
#define GGS_REMOVE_SERVER		"%s/RemoveFromServer"
#define GGS_IMPORT_TEXT 		"%s/ImportFromText"
#define GGS_EXPORT_SERVER		"%s/ExportFromServer"
#define GGS_EXPORT_TEXT 		"%s/ExportFromText"

#define GGS_SENDIMAGE			"%s/SendImage"
#define GGS_RECVIMAGE			"%s/RecvImage"

// Keys
#define GG_PLUGINVERSION		"Version"		// Plugin version.. user for cleanup from previous versions

#define GG_KEY_UIN				"UIN"			// Uin - unique number
#define GG_KEY_PASSWORD			"Password"		// Password
#define GG_KEY_EMAIL			"e-mail"		// E-mail
#define GG_KEY_STATUS			"Status"		// Status
#define GG_KEY_NICK				"Nick"			// Nick
#define GG_KEY_STATUSDESCR		"StatusMsg" 	// Users status description, to be compatible with MWClist
												// should be stored in "CList" group
#define GG_KEY_TOKEN			"Token"			// OAuth Access Token
#define GG_KEY_TOKENSECRET		"TokenSecret"	// OAuth Access Token Secret

#define GG_KEY_KEEPALIVE		"KeepAlive" 	// Keep-alive support
#define GG_KEYDEF_KEEPALIVE		1

#define GG_KEY_SHOWCERRORS		"ShowCErrors"	// Show connection errors
#define GG_KEYDEF_SHOWCERRORS	1

#define GG_KEY_ARECONNECT		"AReconnect"	// Automatically reconnect
#define GG_KEYDEF_ARECONNECT	0

#define GG_KEY_LEAVESTATUSMSG	"LeaveStatusMsg"// Leave status msg when disconnected
#define GG_KEYDEF_LEAVESTATUSMSG 0
#define GG_KEY_LEAVESTATUS		"LeaveStatus"
#define GG_KEYDEF_LEAVESTATUS	0

#define GG_KEY_FRIENDSONLY		"FriendsOnly"	// Friend only visibility
#define GG_KEYDEF_FRIENDSONLY	0

#define GG_KEY_SHOWLINKS		"ShowLinks"		// Show links from unknown contacts
#define GG_KEYDEF_SHOWLINKS		0

#define GG_KEY_ENABLEAVATARS	"EnableAvatars"	// Enable avatars support
#define GG_KEYDEF_ENABLEAVATARS	1

#define GG_KEY_AVATARHASH		"AvatarHash"	// Contact's avatar hash

#define GG_KEY_AVATARURL		"AvatarURL"		// Contact's avatar URL

#define GG_KEY_AVATARTYPE		"AvatarType"	// Contact's avatar format
#define GG_KEYDEF_AVATARTYPE	PA_FORMAT_UNKNOWN

#define GG_KEY_AVATARREQUESTED		"AvatarRequested"	// When contact's avatar is requested
#define GG_KEYDEF_AVATARREQUESTED	0

#define GG_KEY_SHOWINVISIBLE	"ShowInvisible" // Show invisible users when described
#define GG_KEYDEF_SHOWINVISIBLE	0

#define GG_KEY_IGNORECONF		"IgnoreConf"	// Ignore incoming conference messages
#define GG_KEYDEF_IGNORECONF	0

#define GG_KEY_IMGRECEIVE		"ReceiveImg"	// Popup image window automatically
#define GG_KEYDEF_IMGRECEIVE	1

#define GG_KEY_IMGMETHOD		"PopupImg"		// Popup image window automatically
#define GG_KEYDEF_IMGMETHOD		1

#define GG_KEY_MSGACK			"MessageAck"	// Acknowledge when sending msg
#define GG_KEYDEF_MSGACK		1

#define GG_KEY_MANUALHOST		"ManualHost"	// Specify by hand server host/port
#define GG_KEYDEF_MANUALHOST	0
#define GG_KEY_SSLCONN			"SSLConnection" // Use SSL/TLS for connections
#define GG_KEYDEF_SSLCONN		0
#define GG_KEY_SERVERHOSTS		"ServerHosts"	// NL separated list of hosts for server connection
#define GG_KEYDEF_SERVERHOSTS	"91.197.13.54\r\n91.197.13.66\r\n91.197.13.69\r\n91.197.13.72\r\n91.197.13.75\r\n91.197.13.81"

#define GG_KEY_CLIENTIP 		"IP"			// Contact IP (by notify)
#define GG_KEY_CLIENTPORT		"ClientPort"	// Contact port
#define GG_KEY_CLIENTVERSION	"ClientVersion"	// Contact app version

#define GG_KEY_DIRECTCONNS		"DirectConns"	// Use direct connections
#define GG_KEYDEF_DIRECTCONNS	1
#define GG_KEY_DIRECTPORT		"DirectPort"	// Direct connections port
#define GG_KEYDEF_DIRECTPORT	1550

#define GG_KEY_FORWARDING		"Forwarding"	// Use forwarding
#define GG_KEYDEF_FORWARDING	0
#define GG_KEY_FORWARDHOST		"ForwardHost"	// Forwarding host (firewall)
#define GG_KEY_FORWARDPORT		"ForwardPort"	// Forwarding port (firewall port)
#define GG_KEYDEF_FORWARDPORT	1550			// Forwarding port (firewall port)

#define GG_KEY_GC_POLICY_UNKNOWN		"GCPolicyUnknown"
#define GG_KEYDEF_GC_POLICY_UNKNOWN 	1

#define GG_KEY_GC_COUNT_UNKNOWN 		"GCCountUnknown"
#define GG_KEYDEF_GC_COUNT_UNKNOWN		5

#define GG_KEY_GC_POLICY_TOTAL			"GCPolicyTotal"
#define GG_KEYDEF_GC_POLICY_TOTAL		1

#define GG_KEY_GC_COUNT_TOTAL			"GCCountTotal"
#define GG_KEYDEF_GC_COUNT_TOTAL		10

#define GG_KEY_GC_POLICY_DEFAULT		"GCPolicyDefault"
#define GG_KEYDEF_GC_POLICY_DEFAULT 	0

#define GG_KEY_BLOCK			"Block"			// Contact is blocked
#define GG_KEY_APPARENT 		"ApparentMode"	// Visible list

#define GG_KEY_TIMEDEVIATION	"TimeDeviation" // Max time deviation for connections (seconds)
#define GG_KEYDEF_TIMEDEVIATION	300

#define GG_KEY_LOGONTIME		"LogonTS"

#define GG_KEY_RECONNINTERVAL		"ReconnectInterval"
#define GG_KEYDEF_RECONNINTERVAL	3000

// chpassdlgproc() multipurpose dialog proc modes
#define GG_USERUTIL_PASS	0
#define GG_USERUTIL_CREATE	1
#define GG_USERUTIL_REMOVE	2
#define GG_USERUTIL_EMAIL	3

// popup flags
#define	GG_POPUP_ALLOW_MSGBOX	1
#define GG_POPUP_ONCE			2
#define GG_POPUP_ERROR			4
#define GG_POPUP_WARNING		8
#define GG_POPUP_MULTILOGON		16

#define LocalEventUnhook(hook)	if(hook) UnhookEvent(hook)

// Some MSVC compatibility with gcc
#ifdef _MSC_VER
#ifndef strcasecmp
#define strcasecmp _strcmpi
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif
#endif

// Global variables
/////////////////////////////////////////////////

extern HINSTANCE hInstance;
extern PLUGINLINK *pluginLink;
extern CLIST_INTERFACE *pcli;
extern struct LIST_INTERFACE li;
extern list_t g_Instances;

// Screen saver
#ifndef SPI_GETSCREENSAVERRUNNING
#define SPI_GETSCREENSAVERRUNNING 114
#endif

/////////////////////////////////////////////////
// Methods

/* Helper functions */
const char *http_error_string(int h);
unsigned long crc_get(char *mem);
int status_m2gg(GGPROTO *gg, int status, int descr);
int status_gg2m(GGPROTO *gg, int status);
char *gg_status2db(int status, const char *suffix);
char *ws_strerror(int code);
uint32_t swap32(uint32_t x);
const char *gg_version2string(int v);
void gg_checknewuser(GGPROTO* gg, uin_t uin, const char* passwd);

/* Thread functions */
void gg_forkthread(GGPROTO *gg, GGThreadFunc pFunc, void *param);
HANDLE gg_forkthreadex(GGPROTO *gg, GGThreadFunc pFunc, void *param, UINT *threadId);
void gg_threadwait(GGPROTO *gg, GGTHREAD *thread);

/* Global GG functions */
void gg_notifyuser(GGPROTO *gg, HANDLE hContact, int refresh);
void gg_setalloffline(GGPROTO *gg);
void gg_disconnect(GGPROTO *gg);
HANDLE gg_getcontact(GGPROTO *gg, uin_t uin, int create, int inlist, char *nick);
void gg_registerservices(GGPROTO *gg);
void __cdecl gg_mainthread(GGPROTO *gg, void *empty);
int gg_isonline(GGPROTO *gg);
int gg_refreshstatus(GGPROTO *gg, int status);

void gg_broadcastnewstatus(GGPROTO *gg, int newStatus);
int gg_contactdeleted(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
int gg_dbsettingchanged(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
void gg_notifyall(GGPROTO *gg);
void gg_changecontactstatus(GGPROTO *gg, uin_t uin, int status, const char *idescr, int time, uint32_t remote_ip, uint16_t remote_port, uint32_t version);
char *gg_getstatusmsg(GGPROTO *gg, int status);
void gg_dccstart(GGPROTO *gg);
void gg_dccconnect(GGPROTO *gg, uin_t uin);
int gg_gettoken(GGPROTO *gg, GGTOKEN *token);
void gg_parsecontacts(GGPROTO *gg, char *contacts);
int gg_getinfo(PROTO_INTERFACE *proto, HANDLE hContact, int infoType);
void gg_remindpassword(GGPROTO *gg, uin_t uin, const char *email);
void *gg_img_loadpicture(GGPROTO *gg, struct gg_event* e, char *szFileName);
int gg_img_releasepicture(void *img);
int gg_img_display(GGPROTO *gg, HANDLE hContact, void *img);
int gg_img_displayasmsg(GGPROTO *gg, HANDLE hContact, void *img);
int gg_event(PROTO_INTERFACE *proto, PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

/* Avatar functions */
void gg_getavatarfilename(GGPROTO *gg, HANDLE hContact, char *pszDest, int cbLen);
char *gg_avatarhash(char *param);
void gg_getavatar(GGPROTO *gg, HANDLE hContact, char *szAvatarURL);
void gg_requestavatar(GGPROTO *gg, HANDLE hContact, int iWaitFor);
void gg_initavatarrequestthread(GGPROTO *gg);
void gg_uninitavatarrequestthread(GGPROTO *gg);
void gg_getuseravatar(GGPROTO *gg);
void gg_setavatar(GGPROTO *gg, const char *szFilename);
INT_PTR gg_getavatarinfo(GGPROTO *gg, WPARAM wParam, LPARAM lParam);

/* File transfer functions */
HANDLE gg_fileallow(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath);
int gg_filecancel(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hTransfer);
int gg_filedeny(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason);
int gg_recvfile(PROTO_INTERFACE *proto, HANDLE hContact, PROTOFILEEVENT* pre);
HANDLE gg_sendfile(PROTO_INTERFACE *proto, HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles);

/* Import module */
void gg_import_init(GGPROTO *gg, HGENMENU hRoot);

/* Keep-alive module */
void gg_keepalive_init(GGPROTO *gg);
void gg_keepalive_destroy(GGPROTO *gg);

/* Image reception functions */
int gg_img_init(GGPROTO *gg);
int gg_img_destroy(GGPROTO *gg);
int gg_img_shutdown(GGPROTO *gg);
INT_PTR gg_img_recvimage(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
INT_PTR gg_img_sendimage(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
int gg_img_sendonrequest(GGPROTO *gg, struct gg_event* e);
BOOL gg_img_opened(GGPROTO *gg, uin_t uin);

/* IcoLib functions */
void gg_icolib_init();
HICON LoadIconEx(const char* name, BOOL big);
HANDLE GetIconHandle(int iconId);
void ReleaseIconEx(const char* name, BOOL big);
void WindowSetIcon(HWND hWnd, const char* name);
void WindowFreeIcon(HWND hWnd);

/* URI parser functions */
void gg_links_instancemenu_init();
void gg_links_init();
void gg_links_destroy();
void gg_links_instance_init(GGPROTO* gg);

/* OAuth functions */
char *gg_oauth_header(GGPROTO *gg, const char *httpmethod, const char *url);
int gg_oauth_checktoken(GGPROTO *gg, int force);

/* UI page initializers */
int gg_options_init(GGPROTO *gg, WPARAM wParam, LPARAM lParam);
int gg_details_init(GGPROTO *gg, WPARAM wParam, LPARAM lParam);

/* Groupchat functions */
int gg_gc_init(GGPROTO *gg);
void gg_gc_menus_init(GGPROTO *gg, HGENMENU hRoot);
int gg_gc_destroy(GGPROTO *gg);
char * gg_gc_getchat(GGPROTO *gg, uin_t sender, uin_t *recipients, int recipients_count);
GGGC *gg_gc_lookup(GGPROTO *gg, char *id);
int gg_gc_changenick(GGPROTO *gg, HANDLE hContact, char *pszNick);
#define UIN2ID(uin,id) _itoa(uin,id,10)

/* Popups functions */
void gg_initpopups(GGPROTO* gg);
void gg_showpopup(GGPROTO* gg, const char* nickname, const char* msg, int flags);

/* Sessions functions */
INT_PTR gg_sessions_view(GGPROTO* gg, WPARAM wParam, LPARAM lParam);
void gg_sessions_updatedlg(GGPROTO* gg);
BOOL gg_sessions_closedlg(GGPROTO* gg);
void gg_sessions_menus_init(GGPROTO* gg, HGENMENU hRoot);

/* Event helpers */
#define HookProtoEvent(name, func, proto)             HookEventObj(name, (MIRANDAHOOKOBJ)func, proto)
#define CreateProtoServiceFunction(name, func, proto) CreateServiceFunctionObj(name, (MIRANDASERVICEOBJ)func, proto)
typedef INT_PTR (*GGPROTOFUNC)(GGPROTO*,WPARAM,LPARAM);
void CreateProtoService(const char* szService, GGPROTOFUNC serviceProc, GGPROTO *gg);

/* ANSI <-> Unicode conversion helpers */
#define gg_a2t(s) gg->unicode_core ? (TCHAR *)mir_a2u(s) : (TCHAR *)mir_strdup(s)
#define gg_t2a(s) gg->unicode_core ? mir_u2a((wchar_t *)s) : mir_strdup(s)

// Debug functions
int gg_netlog(const GGPROTO *gg, const char *fmt, ...);
const char *ggdebug_eventtype(struct gg_event *e);

#ifdef __cplusplus
}
#endif
#endif
