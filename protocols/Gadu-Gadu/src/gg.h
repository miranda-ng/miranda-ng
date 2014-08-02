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

#if defined(__DEBUG__) || defined(_DEBUG) || defined(DEBUG)
#define DEBUGMODE // Debug Mode
#endif

#if _WIN32_WINNT < 0x0501
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

// Windows headers
// Visual C++ .NET tries to include winsock.h
// which is very ver bad
#include <winsock2.h>
#include <commctrl.h>
#include <commdlg.h>

#include <malloc.h>
#include <process.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

// Miranda IM headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_netlib.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_langpack.h>
#include <m_skin.h>
#include <m_utils.h>
#include <m_ignore.h>
#include <m_clist.h>
#include <m_clistint.h>
#include <m_options.h>
#include <m_userinfo.h>
#include <m_clui.h>
#include <m_button.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_imgsrvc.h>
#include <m_genmenu.h>
#include <m_avatars.h>
#include <m_xml.h>
#include <m_json.h>
#include <m_chat.h>
#include <m_popup.h>
#include <win2k.h>

// Custom profile folders plugin header
#include "m_folders.h"

// Visual C++ extras
#define GGINLINE

// Plugin headers
#include "resource.h"

// libgadu headers
#include "libgadu/libgadu.h"
#include "dynstuff.h"

// Search
// Extended search result structure, used for all searches
struct GGSEARCHRESULT : public PROTOSEARCHRESULT
{
	uin_t uin;
};

typedef struct
{
	HANDLE hThread;
	UINT   dwThreadId;
} GGTHREAD;

typedef struct
{
	uin_t *recipients;
	int recipients_count;
	TCHAR id[32];
	BOOL ignore;
} GGGC;

typedef struct
{
	char id[256];
	char val[256];
} GGTOKEN;

struct GGREQUESTAVATARDATA
{
	MCONTACT hContact;
	int iWaitFor;
};

struct GGGETAVATARDATA
{
	MCONTACT hContact;
	char *szAvatarURL;
};


// Wrappers of the old interface
#define GGDEF_PROTO 	 "GG"        // Default Proto
#define GGDEF_PROTONAME  "Gadu-Gadu" // Default ProtoName


// Process handles / seqs
#define GG_SEQ_INFO				100
#define GG_SEQ_SEARCH			200
#define GG_SEQ_GETNICK			300
#define GG_SEQ_CHINFO			400

// Services
#define GGS_IMPORT_SERVER		"/ImportFromServer"
#define GGS_REMOVE_SERVER		"/RemoveFromServer"
#define GGS_IMPORT_TEXT 		"/ImportFromText"
#define GGS_EXPORT_SERVER		"/ExportFromServer"
#define GGS_EXPORT_TEXT 		"/ExportFromText"

#define GGS_SENDIMAGE			"/SendImage"
#define GGS_RECVIMAGE			"/RecvImage"

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

#define GG_KEY_AVATARTS			"AvatarTs"		// Contact's avatar Timestamp

#define GG_KEY_AVATARTYPE		"AvatarType"	// Contact's avatar format
#define GG_KEY_AVATARTYPEPREV	"AvatarTypePrev"		// Contact's previous avatar format (during setting new avatar)
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


#define GG_KEY_PD_NICKNAME			"NickName"		// GG Public directory: Nick name
#define GG_KEY_PD_FIRSTNAME			"FirstName"		// GG Public directory: First name
#define GG_KEY_PD_LASTNAME			"LastName"		// GG Public directory: Last name
#define GG_KEY_PD_FAMILYNAME		"FamilyName"	// GG Public directory: Family name
#define GG_KEY_PD_CITY				"City"			// GG Public directory: City
#define GG_KEY_PD_FAMILYCITY		"CityOrigin"	// GG Public directory: Family city

#define GG_KEY_PD_AGE				"Age"			// GG Public directory: Age
#define GG_KEY_PD_BIRTHYEAR			"BirthYear"		// GG Public directory: Birth year
#define GG_KEY_PD_GANDER			"Gender"		// GG Public directory: Gander


// chpassdlgproc() multipurpose dialog proc modes
#define GG_USERUTIL_PASS	0
#define GG_USERUTIL_CREATE	1
#define GG_USERUTIL_REMOVE	2
#define GG_USERUTIL_EMAIL	3

// popup flags
#define GG_POPUP_ALLOW_MSGBOX	1
#define GG_POPUP_ONCE			2
#define GG_POPUP_ERROR			4
#define GG_POPUP_WARNING		8
#define GG_POPUP_MULTILOGON		16

// Some MSVC compatibility with gcc
#ifndef strcasecmp
#define strcasecmp _strcmpi
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

// Global variables
/////////////////////////////////////////////////

struct GGPROTO;

extern HINSTANCE hInstance;
extern CLIST_INTERFACE *pcli;
extern LIST<GGPROTO> g_Instances;
extern PLUGININFOEX pluginInfo;
extern IconItem iconList[];

/////////////////////////////////////////////////
// Methods

/* Helper functions */
const TCHAR *http_error_string(int h);
unsigned long crc_get(char *mem);
int gg_normalizestatus(int status);
char *gg_status2db(int status, const char *suffix);
TCHAR *ws_strerror(int code);
uint32_t swap32(uint32_t x);
const char *gg_version2string(int v);

/* Avatar functions */
char *gg_avatarhash(char *param);

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

#define UIN2IDA(uin,id) _itoa(uin,id,10)
#define UIN2IDT(uin,id) _itot(uin,id,10)

// Debug functions
const char *ggdebug_eventtype(gg_event *e);

#include "gg_proto.h"

#endif
