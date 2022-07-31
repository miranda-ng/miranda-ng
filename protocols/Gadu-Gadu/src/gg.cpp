////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Białek
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

#include "gg.h"
#include "version.h"
#include <errno.h>

// Other variables
CMPlugin g_plugin;

static unsigned long crc_table[256];

//////////////////////////////////////////////////////////
// Plugin info

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {F3FF65F3-250E-416A-BEE9-58C93F85AB33}
	{0xf3ff65f3, 0x250e, 0x416a, {0xbe, 0xe9, 0x58, 0xc9, 0x3f, 0x85, 0xab, 0x33}}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<GaduProto>(GGDEF_PROTO, pluginInfoEx)
{
	crc_gentable();
	SetUniqueId(GG_KEY_UIN);
}

//////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

//////////////////////////////////////////////////////////
// Extra winsock function for error description
//
wchar_t* ws_strerror(int code)
{
	static wchar_t err_desc[160];

	// Not a windows error display WinSock
	if (code == 0)
	{
		wchar_t buff[128];
		int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(), 0, buff, _countof(buff), nullptr);
		if (len == 0)
			mir_snwprintf(err_desc, L"WinSock %u: Unknown error.", WSAGetLastError());
		else
			mir_snwprintf(err_desc, L"WinSock %d: %s", WSAGetLastError(), buff);

		return err_desc;
	}

	// Return normal error
	return _wcserror(code);
}

char* as_strerror(int code)
{
	static char err_desc[160];

	// Not a windows error display WinSock
	if (code == 0)
	{
		char buff[128];
		int len = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, WSAGetLastError(), 0, buff, _countof(buff), nullptr);
		if (len == 0)
			mir_snprintf(err_desc, "WinSock %u: Unknown error.", WSAGetLastError());
		else
			 mir_snprintf(err_desc, "WinSock %d: %s", WSAGetLastError(), buff);

		return err_desc;
	}

	// Return normal error
	return strerror(code);
}

//////////////////////////////////////////////////////////
// Build the crc table
void crc_gentable(void)
{
	unsigned long crc;
	unsigned long poly = 0xEDB88320L;
	for (int i = 0; i < 256; i++)
	{
		crc = i;
		for (int j = 8; j > 0; j--)
		{
			if (crc & 1)
				crc = (crc >> 1) ^ poly;
			else
				crc >>= 1;
		}
		crc_table[i] = crc;
	}
}

//////////////////////////////////////////////////////////
// Calculate the crc value
unsigned long crc_get(char *mem)
{
	register unsigned long crc = 0xFFFFFFFF;
	while (mem && *mem)
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_table[(crc ^ *(mem++)) & 0xFF];

	return (crc ^ 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////
// http_error_string()
//
// returns http error text
const wchar_t *http_error_string(int h)
{
	switch (h)
	{
	case 0:
		return (errno == ENOMEM) ? TranslateT("HTTP failed memory") : TranslateT("HTTP failed connecting");
	case GG_ERROR_RESOLVING:
		return TranslateT("HTTP failed resolving");
	case GG_ERROR_CONNECTING:
		return TranslateT("HTTP failed connecting");
	case GG_ERROR_READING:
		return TranslateT("HTTP failed reading");
	case GG_ERROR_WRITING:
		return TranslateT("HTTP failed writing");
	}

	return TranslateT("Unknown HTTP error");
}

//////////////////////////////////////////////////////////
// Cleanups from last plugin
//
void GaduProto::cleanuplastplugin(uint32_t version)
{
	// Store current plugin version
	setDword(GG_PLUGINVERSION, pluginInfoEx.version);

	//1. clean files: %miranda_avatarcache%\GG\*.(null)
	if (version < PLUGIN_MAKE_VERSION(0, 11, 0, 2)) {
		debugLogA("cleanuplastplugin() 1: version=%d Cleaning junk avatar files from < 0.11.0.2", version);

		wchar_t avatarsPath[MAX_PATH];
		mir_snwprintf(avatarsPath, L"%s\\%s", VARSW(L"%miranda_avatarcache%").get(), m_tszUserName);

		debugLogW(L"cleanuplastplugin() 1: miranda_avatarcache = %s", avatarsPath);

		wchar_t spec[MAX_PATH + 10];
		mir_snwprintf(spec, L"%s\\*.(null)", avatarsPath);
		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(spec, &ffd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				wchar_t filePathT[2 * MAX_PATH + 10];
				mir_snwprintf(filePathT, L"%s\\%s", avatarsPath, ffd.cFileName);
				if (!_waccess(filePathT, 0)) {
					debugLogW(L"cleanuplastplugin() 1: remove file = %s", filePathT);
					_wremove(filePathT);
				}
			} while (FindNextFile(hFind, &ffd) != 0);
			FindClose(hFind);
		}
	}

	//2. force SSL and keepalive; overwrite old server list; 
	if (version < PLUGIN_MAKE_VERSION(0, 11, 0, 4)) {
		setWString("ServerHosts", GG_KEYDEF_SERVERHOSTS);
		m_gaduOptions.useManualHosts = 1;
		m_gaduOptions.useSslConnection = 1;
		m_gaduOptions.keepConnectionAlive = 1;
	}
}

//////////////////////////////////////////////////////////
// When Miranda loaded its modules
//
static int gg_modulesloaded(WPARAM, LPARAM)
{
	// File Association Manager support
	gg_links_init();

	return 0;
}

//////////////////////////////////////////////////////////
// Handles PrebuildContactMenu event
//
static int gg_prebuildcontactmenu(WPARAM hContact, LPARAM)
{
	GaduProto* gg = CMPlugin::getInstance(hContact);
	if (gg == nullptr)
		return 0;

	if (gg->getDword(hContact, GG_KEY_UIN, 0) == gg->getByte(GG_KEY_UIN, 0) || gg->isChatRoom(hContact) || !Contact::OnList(hContact))
		Menu_ShowItem(gg->hBlockMenuItem, false);
	else
		Menu_ModifyItem(gg->hBlockMenuItem, gg->getByte(hContact, GG_KEY_BLOCK, 0) ? LPGENW("&Unblock") : LPGENW("&Block"));

	return 0;
}

//////////////////////////////////////////////////////////
// Contact block service function
//
INT_PTR GaduProto::blockuser(WPARAM hContact, LPARAM)
{
	setByte(hContact, GG_KEY_BLOCK, !getByte(hContact, GG_KEY_BLOCK, 0));
	notifyuser(hContact, 1);

	return 0;
}

#define GGS_BLOCKUSER "/BlockUser"

//////////////////////////////////////////////////////////
// Contact blocking initialization
//
void GaduProto::block_init()
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mi.position = -500050000;
	mi.hIcolibItem = iconList[8].hIcolib;
	mi.name.a = LPGEN("&Block");
	mi.pszService = GGS_BLOCKUSER;
	hBlockMenuItem = Menu_AddContactMenuItem(&mi, m_szModuleName);

	::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, gg_prebuildcontactmenu);
}

//////////////////////////////////////////////////////////
// Contact blocking uninitialization
//
void GaduProto::block_uninit()
{
	Menu_RemoveItem(hBlockMenuItem);
}

//////////////////////////////////////////////////////////
// Menus initialization
//
void GaduProto::OnBuildProtoMenu()
{
	HGENMENU hRoot = Menu_GetProtocolRoot(this);

	CMenuItem mi(&g_plugin);
	mi.root = hRoot;
	mi.flags = CMIF_UNICODE;

	mi.name.w = LPGENW("Conference");
	mi.position = 200001;
	mi.hIcolibItem = iconList[14].hIcolib;
	HGENMENU hGCRoot = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.name.w = LPGENW("Contact list");
	mi.position = 200002;
	mi.hIcolibItem = iconList[7].hIcolib;
	HGENMENU hCLRoot = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	gc_menus_init(hGCRoot);
	import_init(hCLRoot);
	sessions_menus_init(hRoot);
}

//////////////////////////////////////////////////////////
// When plugin is loaded
//
int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, gg_modulesloaded);

	gg_links_instancemenu_init();
	return 0;
}

//////////////////////////////////////////////////////////
// When plugin is unloaded
//
int CMPlugin::Unload()
{
	WSACleanup();
	return 0;
}

//////////////////////////////////////////////////////////
// DEBUGING FUNCTIONS
struct
{
	int type;
	char *text;
}

static const ggdebug_eventype2string[] =
{
   {GG_EVENT_NONE,                 "GG_EVENT_NONE"},
   {GG_EVENT_MSG,                  "GG_EVENT_MSG"},
   {GG_EVENT_NOTIFY,               "GG_EVENT_NOTIFY"},
   {GG_EVENT_NOTIFY_DESCR,         "GG_EVENT_NOTIFY_DESCR"},
   {GG_EVENT_STATUS,               "GG_EVENT_STATUS"},
   {GG_EVENT_ACK,                  "GG_EVENT_ACK"},
   {GG_EVENT_PONG,                 "GG_EVENT_PONG"},
   {GG_EVENT_CONN_FAILED,          "GG_EVENT_CONN_FAILED"},
   {GG_EVENT_CONN_SUCCESS,         "GG_EVENT_CONN_SUCCESS"},
   {GG_EVENT_DISCONNECT,           "GG_EVENT_DISCONNECT"},
   {GG_EVENT_DCC_NEW,              "GG_EVENT_DCC_NEW"},
   {GG_EVENT_DCC_ERROR,            "GG_EVENT_DCC_ERROR"},
   {GG_EVENT_DCC_DONE,             "GG_EVENT_DCC_DONE"},
   {GG_EVENT_DCC_CLIENT_ACCEPT,    "GG_EVENT_DCC_CLIENT_ACCEPT"},
   {GG_EVENT_DCC_CALLBACK,         "GG_EVENT_DCC_CALLBACK"},
   {GG_EVENT_DCC_NEED_FILE_INFO,   "GG_EVENT_DCC_NEED_FILE_INFO"},
   {GG_EVENT_DCC_NEED_FILE_ACK,    "GG_EVENT_DCC_NEED_FILE_ACK"},
   {GG_EVENT_DCC_NEED_VOICE_ACK,   "GG_EVENT_DCC_NEED_VOICE_ACK"},
   {GG_EVENT_DCC_VOICE_DATA,       "GG_EVENT_DCC_VOICE_DATA"},
   {GG_EVENT_PUBDIR50_SEARCH_REPLY,"GG_EVENT_PUBDIR50_SEARCH_REPLY"},
   {GG_EVENT_PUBDIR50_READ,        "GG_EVENT_PUBDIR50_READ"},
   {GG_EVENT_PUBDIR50_WRITE,       "GG_EVENT_PUBDIR50_WRITE"},
   {GG_EVENT_STATUS60,             "GG_EVENT_STATUS60"},
   {GG_EVENT_NOTIFY60,             "GG_EVENT_NOTIFY60"},
   {GG_EVENT_USERLIST,             "GG_EVENT_USERLIST"},
   {GG_EVENT_IMAGE_REQUEST,        "GG_EVENT_IMAGE_REQUEST"},
   {GG_EVENT_IMAGE_REPLY,          "GG_EVENT_IMAGE_REPLY"},
   {GG_EVENT_DCC_ACK,              "GG_EVENT_DCC_ACK"},
   {GG_EVENT_DCC7_NEW,             "GG_EVENT_DCC7_NEW"},
   {GG_EVENT_DCC7_ACCEPT,          "GG_EVENT_DCC7_ACCEPT"},
   {GG_EVENT_DCC7_REJECT,          "GG_EVENT_DCC7_REJECT"},
   {GG_EVENT_DCC7_CONNECTED,       "GG_EVENT_DCC7_CONNECTED"},
   {GG_EVENT_DCC7_ERROR,           "GG_EVENT_DCC7_ERROR"},
   {GG_EVENT_DCC7_DONE,            "GG_EVENT_DCC7_DONE"},
   {GG_EVENT_DCC7_PENDING,         "GG_EVENT_DCC7_PENDING"},
   {GG_EVENT_XML_EVENT,            "GG_EVENT_XML_EVENT"},
   {GG_EVENT_DISCONNECT_ACK,       "GG_EVENT_DISCONNECT_ACK"},
   {GG_EVENT_TYPING_NOTIFICATION,  "GG_EVENT_TYPING_NOTIFICATION"},
   {GG_EVENT_USER_DATA,            "GG_EVENT_USER_DATA"},
   {GG_EVENT_MULTILOGON_MSG,       "GG_EVENT_MULTILOGON_MSG"},
   {GG_EVENT_MULTILOGON_INFO,      "GG_EVENT_MULTILOGON_INFO"},
   {-1,                            "<unknown event>"}
};

const char *ggdebug_eventtype(gg_event *e)
{
	int i;
	for (i = 0; ggdebug_eventype2string[i].type != -1; i++)
		if (ggdebug_eventype2string[i].type == e->type)
			return ggdebug_eventype2string[i].text;

	return ggdebug_eventype2string[i].text;
}

//////////////////////////////////////////////////////////
// Log funcion
#define PREFIXLEN   6   //prefix present in DEBUGMODE contains GetCurrentThreadId()

#ifdef DEBUGMODE
void gg_debughandler(int level, const char *format, va_list ap)
{
	char szText[1024], *szFormat = _strdup(format);
	// Kill end line
	char *nl = strrchr(szFormat, '\n');
	if (nl)
		*nl = 0;

	strncpy(szText + PREFIXLEN, "[libgadu] \0", sizeof(szText) - PREFIXLEN);

	char prefix[6];
	mir_snprintf(prefix, "%lu", GetCurrentThreadId());
	size_t prefixLen = mir_strlen(prefix);
	if (prefixLen < PREFIXLEN)
		memset(prefix + prefixLen, ' ', PREFIXLEN - prefixLen);
	memcpy(szText, prefix, PREFIXLEN);

	mir_vsnprintf(szText + mir_strlen(szText), sizeof(szText) - mir_strlen(szText), szFormat, ap);
	Netlib_Log(nullptr, szText);
	free(szFormat);
}
#endif
