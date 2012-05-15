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

#include "gg.h"
#include "version.h"
#include <errno.h>

// Plugin info
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Gadu-Gadu Protocol",
	__VERSION_DWORD,
	"Provides support for Gadu-Gadu protocol",
	"Bartosz Bia³ek, Adam Strzelecki",
	"dezred"/*antispam*/"@"/*antispam*/"gmail"/*antispam*/"."/*antispam*/"com",
	"© 2009-2012 Bartosz Bia³ek, 2003-2009 Adam Strzelecki",
	"http://www.miranda-im.pl/",
	0,
	0,
	// {F3FF65F3-250E-416A-BEE9-58C93F85AB33}
	{ 0xf3ff65f3, 0x250e, 0x416a, { 0xbe, 0xe9, 0x58, 0xc9, 0x3f, 0x85, 0xab, 0x33 } }
};
static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};

// Other variables
HINSTANCE hInstance;
PLUGINLINK *pluginLink;
struct MM_INTERFACE mmi;
struct SHA1_INTERFACE sha1i;
struct MD5_INTERFACE md5i;
struct LIST_INTERFACE li;
XML_API xi;
SSL_API si;
CLIST_INTERFACE *pcli;
int hLangpack;
list_t g_Instances;

// Event hooks
static HANDLE hHookModulesLoaded = NULL;
static HANDLE hHookPreShutdown = NULL;

static unsigned long crc_table[256];

//////////////////////////////////////////////////////////
// Extra winsock function for error description
char *ws_strerror(int code)
{
	static char err_desc[160];

	// Not a windows error display WinSock
	if(code == 0)
	{
		char buff[128];
		int len;
		len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
				  NULL, WSAGetLastError(), 0, buff,
				  sizeof(buff), NULL);
		if(len == 0)
			mir_snprintf(err_desc, sizeof(err_desc), "WinSock %u: Unknown error.", WSAGetLastError());
		else
			mir_snprintf(err_desc, sizeof(err_desc), "WinSock %d: %s", WSAGetLastError(), buff);
		return err_desc;
	}

	// Return normal error
	return strerror(code);
}

//////////////////////////////////////////////////////////
// Build the crc table
void crc_gentable(void)
{
	unsigned long crc, poly;
	int	i, j;

	poly = 0xEDB88320L;
	for (i = 0; i < 256; i++)
	{
		crc = i;
		for (j = 8; j > 0; j--)
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
	while(mem && *mem)
		crc = ((crc>>8) & 0x00FFFFFF) ^ crc_table[(crc ^ *(mem++)) & 0xFF];

	return (crc ^ 0xFFFFFFFF);
}

//////////////////////////////////////////////////////////
// http_error_string()
//
// returns http error text
const char *http_error_string(int h)
{
	switch (h)
	{
		case 0:
			return Translate((errno == ENOMEM) ? "HTTP failed memory" : "HTTP failed connecting");
		case GG_ERROR_RESOLVING:
			return Translate("HTTP failed resolving");
		case GG_ERROR_CONNECTING:
			return Translate("HTTP failed connecting");
		case GG_ERROR_READING:
			return Translate("HTTP failed reading");
		case GG_ERROR_WRITING:
			return Translate("HTTP failed writing");
	}

	return Translate("Unknown HTTP error");
}

//////////////////////////////////////////////////////////
// Gets plugin info
__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < MIRANDA_VERSION_CORE)
	{
		MessageBox(
			NULL,
			"The Gadu-Gadu protocol plugin cannot be loaded. It requires Miranda IM " MIRANDA_VERSION_CORE_STRING " or later.",
			"Gadu-Gadu Protocol Plugin",
			MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST
		);
		return NULL;
	}
	return &pluginInfo;
}
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

//////////////////////////////////////////////////////////
// Cleanups from last plugin
void gg_cleanuplastplugin(GGPROTO *gg, DWORD version)
{
	HANDLE hContact;
	char *szProto;

	// Remove bad e-mail and phones from
	if(version < PLUGIN_MAKE_VERSION(0, 0, 1, 4))
	{
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_cleanuplastplugin(%d): Cleaning junk Phone settings from < 0.0.1.4 ...", version);
#endif
		// Look for contact in DB
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
			if(szProto != NULL && !strcmp(szProto, GG_PROTO))
			{
				// Do contact cleanup
				DBDeleteContactSetting(hContact, GG_PROTO, GG_KEY_EMAIL);
				DBDeleteContactSetting(hContact, GG_PROTO, "Phone");
			}
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	}

	// Remove GG entries for non GG contacts
	if(version < PLUGIN_MAKE_VERSION(0, 0, 3, 5))
	{
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_cleanuplastplugin(%d): Cleaning junk Nick settings from < 0.0.3.5 ...", version);
#endif
		// Look for contact in DB
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact)
		{
			szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
			if(szProto != NULL && strcmp(szProto, GG_PROTO))
			{
				// Do nick entry cleanup
				DBDeleteContactSetting(hContact, GG_PROTO, GG_KEY_NICK);
			}
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	}

	// Remove old unneeded entry
	if(version < PLUGIN_MAKE_VERSION(0, 0, 5, 3))
		DBDeleteContactSetting(NULL, GG_PROTO, "ShowNotOnMyList");

	// Store this plugin version
	DBWriteContactSettingDword(NULL, GG_PROTO, GG_PLUGINVERSION, pluginInfo.version);
}

//////////////////////////////////////////////////////////
// Custom folders initialization
void gg_initcustomfolders(GGPROTO *gg)
{
	char szPath[MAX_PATH];
	char *tmpPath = Utils_ReplaceVars("%miranda_avatarcache%");
	mir_snprintf(szPath, MAX_PATH, "%s\\%s", tmpPath, GG_PROTO);
	mir_free(tmpPath);
	gg->hAvatarsFolder = FoldersRegisterCustomPath(GG_PROTO, "Avatars", szPath);

	tmpPath = Utils_ReplaceVars("%miranda_userdata%");
	mir_snprintf(szPath, MAX_PATH, "%s\\%s\\ImageCache", tmpPath, GG_PROTO);
	mir_free(tmpPath);
	gg->hImagesFolder = FoldersRegisterCustomPath(GG_PROTO, "Images", szPath);
}

//////////////////////////////////////////////////////////
// When Miranda loaded its modules
static int gg_modulesloaded(WPARAM wParam, LPARAM lParam)
{
	// Get SSL API
	mir_getSI(&si);

	// File Association Manager support
	gg_links_init();

	return 0;
}

//////////////////////////////////////////////////////////
// When Miranda starting shutdown sequence
static int gg_preshutdown(WPARAM wParam, LPARAM lParam)
{
	gg_links_destroy();

	return 0;
}

//////////////////////////////////////////////////////////
// Gets protocol instance associated with a contact
static GGPROTO* gg_getprotoinstance(HANDLE hContact)
{
	char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	list_t l = g_Instances;

	if (szProto == NULL)
		return NULL;

	for (; l; l = l->next)
	{
		GGPROTO* gg = l->data;
		if (strcmp(szProto, GG_PROTO) == 0)
			return gg;
	}

	return NULL;
}

//////////////////////////////////////////////////////////
// Handles PrebuildContactMenu event
static int gg_prebuildcontactmenu(WPARAM wParam, LPARAM lParam)
{
	const HANDLE hContact = (HANDLE)wParam;
	CLISTMENUITEM mi = {0};
	GGPROTO* gg = gg_getprotoinstance(hContact);

	if (gg == NULL)
		return 0;

	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_NAME | CMIM_FLAGS | CMIF_ICONFROMICOLIB;
	if (DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0) == DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0) ||
		DBGetContactSettingByte(hContact, GG_PROTO, "ChatRoom", 0) ||
		DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
		mi.flags |= CMIF_HIDDEN;
	mi.pszName = DBGetContactSettingByte(hContact, GG_PROTO, GG_KEY_BLOCK, 0) ? LPGEN("&Unblock") : LPGEN("&Block");
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)gg->hBlockMenuItem, (LPARAM)&mi);

	return 0;
}

//////////////////////////////////////////////////////////
// Contact block service function
INT_PTR gg_blockuser(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	const HANDLE hContact = (HANDLE)wParam;
	DBWriteContactSettingByte(hContact, GG_PROTO, GG_KEY_BLOCK, !DBGetContactSettingByte(hContact, GG_PROTO, GG_KEY_BLOCK, 0));
	gg_notifyuser(gg, hContact, 1);
	return 0;
}


//////////////////////////////////////////////////////////
// Contact blocking initialization
#define GGS_BLOCKUSER "%s/BlockUser"
static void gg_block_init(GGPROTO *gg)
{
	CLISTMENUITEM mi = {0};
	char service[64];

	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB;

	mir_snprintf(service, sizeof(service), GGS_BLOCKUSER, GG_PROTO);
	CreateProtoServiceFunction(service, gg_blockuser, gg);
	mi.position = -500050000;
	mi.icolibItem = GetIconHandle(IDI_BLOCK);
	mi.pszName = LPGEN("&Block");
	mi.pszService = service;
	mi.pszContactOwner = GG_PROTO;
	gg->hBlockMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	gg->hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, gg_prebuildcontactmenu);
}

//////////////////////////////////////////////////////////
// Contact blocking uninitialization
static void gg_block_uninit(GGPROTO *gg)
{
	UnhookEvent(gg->hPrebuildMenuHook);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)gg->hBlockMenuItem, 0);
}

//////////////////////////////////////////////////////////
// Menus initialization
void gg_menus_init(GGPROTO *gg)
{
	HGENMENU hGCRoot, hCLRoot, hRoot = MO_GetProtoRootMenu(gg->proto.m_szModuleName);
	CLISTMENUITEM mi = {0};

	mi.cbSize = sizeof(mi);
	if (hRoot == NULL)
	{
		mi.ptszName = GG_PROTONAME;
		mi.position = 500090000;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = GetIconHandle(IDI_GG);
		hGCRoot = hCLRoot = hRoot = gg->hMenuRoot = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);
	}
	else
	{
		mi.hParentMenu = hRoot;
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTHANDLE | CMIF_TCHAR;

		mi.ptszName = LPGENT("Conference");
		mi.position = 200001;
		mi.icolibItem = GetIconHandle(IDI_CONFERENCE);
		hGCRoot = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

		mi.ptszName = LPGENT("Contact list");
		mi.position = 200002;
		mi.icolibItem = GetIconHandle(IDI_LIST);
		hCLRoot = (HANDLE)CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM) &mi);

		if (gg->hMenuRoot)
			CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)gg->hMenuRoot, 0);
		gg->hMenuRoot = NULL;
	}

	gg_gc_menus_init(gg, hGCRoot);
	gg_import_init(gg, hCLRoot);
	gg_sessions_menus_init(gg, hRoot);
}

//////////////////////////////////////////////////////////
// Custom protocol event
int gg_event(PROTO_INTERFACE *proto, PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	GGPROTO *gg = (GGPROTO *)proto;
	switch( eventType )
	{
		case EV_PROTO_ONLOAD:
		{
			gg->hookOptsInit = HookProtoEvent(ME_OPT_INITIALISE, gg_options_init, gg);
			gg->hookUserInfoInit = HookProtoEvent(ME_USERINFO_INITIALISE, gg_details_init, gg);
#ifdef DEBUGMODE
			gg_netlog(gg, "gg_event(EV_PROTO_ONLOAD): loading modules...");
#endif
			// Init misc stuff
			gg_icolib_init();
			gg_initpopups(gg);
			gg_gc_init(gg);
			gg_keepalive_init(gg);
			gg_img_init(gg);
			gg_block_init(gg);

			// Try to fetch user avatar
			gg_getuseravatar(gg);
			break;
		}
		case EV_PROTO_ONEXIT:
#ifdef DEBUGMODE
			gg_netlog(gg, "gg_event(EV_PROTO_ONEXIT)/gg_preshutdown(): signalling shutdown...");
#endif
			// Stop avatar request thread
			gg_uninitavatarrequestthread(gg);
			// Stop main connection session thread
#ifdef DEBUGMODE
			gg_netlog(gg, "gg_event(EV_PROTO_ONEXIT): Waiting until Server Thread finished, if needed.");
#endif
			gg_threadwait(gg, &gg->pth_sess);
			gg_img_shutdown(gg);
			gg_sessions_closedlg(gg);
			break;

		case EV_PROTO_ONOPTIONS:
			return gg_options_init(gg, wParam, lParam);

		case EV_PROTO_ONMENU:
			gg_menus_init(gg);
			break;

		case EV_PROTO_ONRENAME:
#ifdef DEBUGMODE
			gg_netlog(gg, "gg_event(EV_PROTO_ONRENAME): renaming account...");
#endif
			mir_free(gg->name);
			gg->name = gg_t2a(gg->proto.m_tszUserName);
			if (gg->hMenuRoot)
			{
				CLISTMENUITEM mi = {0};
				mi.cbSize = sizeof(mi);
				mi.flags = CMIM_NAME | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
				mi.ptszName = GG_PROTONAME;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)gg->hMenuRoot, (LPARAM)&mi);
			}
			break;

		case EV_PROTO_ONCONTACTDELETED:
			return gg_contactdeleted(gg, wParam, lParam);

		case EV_PROTO_DBSETTINGSCHANGED:
			return gg_dbsettingchanged(gg, wParam, lParam);
	}
	return TRUE;
}

//////////////////////////////////////////////////////////
// Module instance initialization
static GGPROTO *gg_proto_init(const char* pszProtoName, const TCHAR* tszUserName)
{
	DWORD dwVersion;
	GGPROTO *gg = (GGPROTO *)mir_alloc(sizeof(GGPROTO));
	char szVer[MAX_PATH];
	NETLIBUSER nlu = { 0 };

	ZeroMemory(gg, sizeof(GGPROTO));
	gg->proto.vtbl = (PROTO_INTERFACE_VTBL*)mir_alloc(sizeof(PROTO_INTERFACE_VTBL));
	// Are we running under unicode Miranda core ?
	CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
	_strlwr(szVer); // make sure it is lowercase
	gg->unicode_core = (strstr(szVer, "unicode") != NULL);

	// Init mutexes
	InitializeCriticalSection(&gg->sess_mutex);
	InitializeCriticalSection(&gg->ft_mutex);
	InitializeCriticalSection(&gg->img_mutex);
	InitializeCriticalSection(&gg->modemsg_mutex);
	InitializeCriticalSection(&gg->avatar_mutex);
	InitializeCriticalSection(&gg->sessions_mutex);

	// Init instance names
	gg->proto.m_szModuleName = mir_strdup(pszProtoName);
	gg->proto.m_szProtoName = GGDEF_PROTONAME;
	gg->proto.m_iVersion = 2;

/* Anyway we won't get Unicode in GG yet */
#ifdef _UNICODE
	gg->name = gg->proto.m_tszUserName = mir_tstrdup(tszUserName);
#else
	gg->proto.m_tszUserName = gg->unicode_core ? (TCHAR *)mir_wstrdup((wchar_t *)tszUserName) : (TCHAR *)mir_strdup((char *)tszUserName);
	gg->name = gg_t2a(tszUserName);
#endif

	// Register netlib user
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szSettingsModule = gg->proto.m_szModuleName;
	if (gg->unicode_core) {
		WCHAR name[128];
		_snwprintf(name, SIZEOF(name), TranslateW(L"%s connection"), gg->proto.m_tszUserName);
		nlu.ptszDescriptiveName = (TCHAR *)name;
		nlu.flags |= NUF_UNICODE;
	} else {
		char name[128];
		mir_snprintf(name, SIZEOF(name), Translate("%s connection"), gg->proto.m_tszUserName);
		nlu.ptszDescriptiveName = name;
	}
	gg->netlib = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	// Register services
	gg_registerservices(gg);

	// Offline contacts and clear logon time
	gg_setalloffline(gg);
	DBWriteContactSettingDword(NULL, GG_PROTO, GG_KEY_LOGONTIME, 0);

	if ((dwVersion = DBGetContactSettingDword(NULL, GG_PROTO, GG_PLUGINVERSION, 0)) < pluginInfo.version)
		gg_cleanuplastplugin(gg, dwVersion);

	// Add to the instance list
	list_add(&g_Instances, gg, 0);

	gg_links_instance_init(gg);
	gg_initcustomfolders(gg);
	gg_initavatarrequestthread(gg);

	return gg;
}

//////////////////////////////////////////////////////////
// Module instance uninitialization
static int gg_proto_uninit(PROTO_INTERFACE *proto)
{
	GGPROTO *gg = (GGPROTO *)proto;

#ifdef DEBUGMODE
	gg_netlog(gg, "gg_proto_uninit(): destroying protocol interface");
#endif

	// Destroy modules
	gg_block_uninit(gg);
	gg_img_destroy(gg);
	gg_keepalive_destroy(gg);
	gg_gc_destroy(gg);

	// Remove from the instance list
	list_remove(&g_Instances, gg, 0);

	if (gg->hMenuRoot)
		CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)gg->hMenuRoot, 0);

	// Close handles
	LocalEventUnhook(gg->hookOptsInit);
	LocalEventUnhook(gg->hookUserInfoInit);
	Netlib_CloseHandle(gg->netlib);

	// Destroy mutexes
	DeleteCriticalSection(&gg->sess_mutex);
	DeleteCriticalSection(&gg->ft_mutex);
	DeleteCriticalSection(&gg->img_mutex);
	DeleteCriticalSection(&gg->modemsg_mutex);
	DeleteCriticalSection(&gg->avatar_mutex);
	DeleteCriticalSection(&gg->sessions_mutex);

	// Free status messages
	if (gg->modemsg.online)    mir_free(gg->modemsg.online);
	if (gg->modemsg.away)      mir_free(gg->modemsg.away);
	if (gg->modemsg.dnd)       mir_free(gg->modemsg.dnd);
	if (gg->modemsg.freechat)  mir_free(gg->modemsg.freechat);
	if (gg->modemsg.invisible) mir_free(gg->modemsg.invisible);
	if (gg->modemsg.offline)   mir_free(gg->modemsg.offline);

	mir_free(gg->proto.m_szModuleName);
	mir_free(gg->proto.m_tszUserName);
	mir_free(gg->name);
	mir_free(gg->proto.vtbl);
	mir_free(gg);

	return 0;
}

//////////////////////////////////////////////////////////
// When plugin is loaded
int __declspec(dllexport) Load(PLUGINLINK * link)
{
	WSADATA wsaData;
	PROTOCOLDESCRIPTOR pd;

	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getSHA1I(&sha1i);
	mir_getMD5I(&md5i);
	mir_getLI(&li);
	mir_getXI(&xi);
	mir_getLP(&pluginInfo);

	// Init winsock
	if (WSAStartup(MAKEWORD( 1, 1 ), &wsaData))
		return 1;

	pcli = (CLIST_INTERFACE*)CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)hInstance);

	// Hook system events
	hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, gg_modulesloaded);
	hHookPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, gg_preshutdown);

	// Prepare protocol name
	ZeroMemory(&pd, sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = GGDEF_PROTO;
	pd.fnInit = (pfnInitProto)gg_proto_init;
	pd.fnUninit = (pfnUninitProto)gg_proto_uninit;
	pd.type = PROTOTYPE_PROTOCOL;

	// Register module
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) &pd);
	gg_links_instancemenu_init();

	// Instance list
	g_Instances = NULL;

	return 0;
}

//////////////////////////////////////////////////////////
// When plugin is unloaded
int __declspec(dllexport) Unload()
{
	LocalEventUnhook(hHookModulesLoaded);
	LocalEventUnhook(hHookPreShutdown);

	// Cleanup WinSock
	WSACleanup();

	return 0;
}

//////////////////////////////////////////////////////////
// Adds a new protocol specific service function
void CreateProtoService(const char* szService, GGPROTOFUNC serviceProc, GGPROTO *gg)
{
	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, sizeof(str), "%s%s", gg->proto.m_szModuleName, szService);
	CreateServiceFunctionObj(str, (MIRANDASERVICEOBJ)serviceProc, gg);
}

//////////////////////////////////////////////////////////
// Forks a thread
void gg_forkthread(GGPROTO *gg, GGThreadFunc pFunc, void *param)
{
	CloseHandle((HANDLE)mir_forkthreadowner((pThreadFuncOwner)&pFunc, gg, param, NULL));
}

//////////////////////////////////////////////////////////
// Forks a thread and returns a pseudo handle for it
HANDLE gg_forkthreadex(GGPROTO *gg, GGThreadFunc pFunc, void *param, UINT *threadId)
{
	return (HANDLE)mir_forkthreadowner((pThreadFuncOwner)&pFunc, gg, param, threadId);
}

//////////////////////////////////////////////////////////
// Wait for thread to stop
void gg_threadwait(GGPROTO *gg, GGTHREAD *thread)
{
	if (!thread->hThread) return;
	while (WaitForSingleObjectEx(thread->hThread, INFINITE, TRUE) != WAIT_OBJECT_0);
	CloseHandle(thread->hThread);
	ZeroMemory(thread, sizeof(GGTHREAD));
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
	{GG_EVENT_NONE,					"GG_EVENT_NONE"},
	{GG_EVENT_MSG,					"GG_EVENT_MSG"},
	{GG_EVENT_NOTIFY,				"GG_EVENT_NOTIFY"},
	{GG_EVENT_NOTIFY_DESCR,			"GG_EVENT_NOTIFY_DESCR"},
	{GG_EVENT_STATUS,				"GG_EVENT_STATUS"},
	{GG_EVENT_ACK,					"GG_EVENT_ACK"},
	{GG_EVENT_PONG,					"GG_EVENT_PONG"},
	{GG_EVENT_CONN_FAILED,			"GG_EVENT_CONN_FAILED"},
	{GG_EVENT_CONN_SUCCESS,			"GG_EVENT_CONN_SUCCESS"},
	{GG_EVENT_DISCONNECT,			"GG_EVENT_DISCONNECT"},
	{GG_EVENT_DCC_NEW,				"GG_EVENT_DCC_NEW"},
	{GG_EVENT_DCC_ERROR,			"GG_EVENT_DCC_ERROR"},
	{GG_EVENT_DCC_DONE,				"GG_EVENT_DCC_DONE"},
	{GG_EVENT_DCC_CLIENT_ACCEPT,	"GG_EVENT_DCC_CLIENT_ACCEPT"},
	{GG_EVENT_DCC_CALLBACK,			"GG_EVENT_DCC_CALLBACK"},
	{GG_EVENT_DCC_NEED_FILE_INFO,	"GG_EVENT_DCC_NEED_FILE_INFO"},
	{GG_EVENT_DCC_NEED_FILE_ACK,	"GG_EVENT_DCC_NEED_FILE_ACK"},
	{GG_EVENT_DCC_NEED_VOICE_ACK,	"GG_EVENT_DCC_NEED_VOICE_ACK"},
	{GG_EVENT_DCC_VOICE_DATA,		"GG_EVENT_DCC_VOICE_DATA"},
	{GG_EVENT_PUBDIR50_SEARCH_REPLY,"GG_EVENT_PUBDIR50_SEARCH_REPLY"},
	{GG_EVENT_PUBDIR50_READ,		"GG_EVENT_PUBDIR50_READ"},
	{GG_EVENT_PUBDIR50_WRITE,		"GG_EVENT_PUBDIR50_WRITE"},
	{GG_EVENT_STATUS60,				"GG_EVENT_STATUS60"},
	{GG_EVENT_NOTIFY60,				"GG_EVENT_NOTIFY60"},
	{GG_EVENT_USERLIST,				"GG_EVENT_USERLIST"},
	{GG_EVENT_IMAGE_REQUEST,		"GG_EVENT_IMAGE_REQUEST"},
	{GG_EVENT_IMAGE_REPLY,			"GG_EVENT_IMAGE_REPLY"},
	{GG_EVENT_DCC_ACK,				"GG_EVENT_DCC_ACK"},
	{GG_EVENT_DCC7_NEW,				"GG_EVENT_DCC7_NEW"},
	{GG_EVENT_DCC7_ACCEPT,			"GG_EVENT_DCC7_ACCEPT"},
	{GG_EVENT_DCC7_REJECT,			"GG_EVENT_DCC7_REJECT"},
	{GG_EVENT_DCC7_CONNECTED,		"GG_EVENT_DCC7_CONNECTED"},
	{GG_EVENT_DCC7_ERROR,			"GG_EVENT_DCC7_ERROR"},
	{GG_EVENT_DCC7_DONE,			"GG_EVENT_DCC7_DONE"},
	{GG_EVENT_DCC7_PENDING,			"GG_EVENT_DCC7_PENDING"},
	{GG_EVENT_XML_EVENT,			"GG_EVENT_XML_EVENT"},
	{GG_EVENT_DISCONNECT_ACK,		"GG_EVENT_DISCONNECT_ACK"},
	{GG_EVENT_XML_ACTION,			"GG_EVENT_XML_ACTION"},
	{GG_EVENT_TYPING_NOTIFICATION,	"GG_EVENT_TYPING_NOTIFICATION"},
	{GG_EVENT_USER_DATA,			"GG_EVENT_USER_DATA"},
	{GG_EVENT_MULTILOGON_MSG,		"GG_EVENT_MULTILOGON_MSG"},
	{GG_EVENT_MULTILOGON_INFO,		"GG_EVENT_MULTILOGON_INFO"},
	{-1,							"<unknown event>"}
};

const char *ggdebug_eventtype(struct gg_event *e)
{
	int i;
	for(i = 0; ggdebug_eventype2string[i].type != -1; i++)
		if(ggdebug_eventype2string[i].type == e->type)
			return ggdebug_eventype2string[i].text;
	return ggdebug_eventype2string[i].text;
}

#ifdef DEBUGMODE
void gg_debughandler(int level, const char *format, va_list ap)
{
	char szText[1024], *szFormat = _strdup(format);
	// Kill end line
	char *nl = strrchr(szFormat, '\n');
	if(nl) *nl = 0;

	strncpy(szText, "[libgadu] \0", sizeof(szText));

	mir_vsnprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), szFormat, ap);
	CallService(MS_NETLIB_LOG, (WPARAM) NULL, (LPARAM) szText);
	free(szFormat);
}
#endif

//////////////////////////////////////////////////////////
// Log funcion
int gg_netlog(const GGPROTO *gg, const char *fmt, ...)
{
	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
	return CallService(MS_NETLIB_LOG, (WPARAM) gg->netlib, (LPARAM) szText);
}

//////////////////////////////////////////////////////////
// main DLL function
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
	crc_gentable();
	hInstance = hInst;
#ifdef DEBUGMODE
	gg_debug_handler = gg_debughandler;
#endif
	return TRUE;
}
