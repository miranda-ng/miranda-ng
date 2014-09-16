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
   __PLUGIN_NAME,
   PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
   __DESCRIPTION,
   __AUTHOR,
   __AUTHOREMAIL,
   __COPYRIGHT,
   __AUTHORWEB,
   UNICODE_AWARE,
   // {F3FF65F3-250E-416A-BEE9-58C93F85AB33}
   { 0xf3ff65f3, 0x250e, 0x416a, { 0xbe, 0xe9, 0x58, 0xc9, 0x3f, 0x85, 0xab, 0x33 } }
};

// Other variables
HINSTANCE hInstance;

XML_API xi;
SSL_API si;
CLIST_INTERFACE *pcli;
int hLangpack;
LIST<GGPROTO> g_Instances(1, PtrKeySortT);

// Event hooks
static HANDLE hHookModulesLoaded = NULL;
static HANDLE hHookPreShutdown = NULL;

static unsigned long crc_table[256];

//////////////////////////////////////////////////////////
// Extra winsock function for error description

TCHAR* ws_strerror(int code)
{
   static TCHAR err_desc[160];

   // Not a windows error display WinSock
   if (code == 0)
   {
      TCHAR buff[128];
      int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), 0, buff, SIZEOF(buff), NULL);
      if (len == 0)
         mir_sntprintf(err_desc, SIZEOF(err_desc), _T("WinSock %u: Unknown error."), WSAGetLastError());
      else
         mir_sntprintf(err_desc, SIZEOF(err_desc), _T("WinSock %d: %s"), WSAGetLastError(), buff);
      return err_desc;
   }

   // Return normal error
   return _tcserror(code);
}

//////////////////////////////////////////////////////////
// Build the crc table
void crc_gentable(void)
{
   unsigned long crc, poly;
   int   i, j;

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
const TCHAR *http_error_string(int h)
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
// Gets plugin info

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
   return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

//////////////////////////////////////////////////////////
// Cleanups from last plugin

void GGPROTO::cleanuplastplugin(DWORD version)
{
   // Store current plugin version
   setDword(GG_PLUGINVERSION, pluginInfo.version);

   //1. clean files: %miranda_avatarcache%\GG\*.(null)
   if (version < PLUGIN_MAKE_VERSION(0, 11, 0, 2)){
      debugLogA("cleanuplastplugin() 1: version=%d Cleaning junk avatar files from < 0.11.0.2", version);

      TCHAR avatarsPath[MAX_PATH];
      mir_sntprintf(avatarsPath, MAX_PATH, _T("%s\\%s"), VARST( _T("%miranda_avatarcache%")), m_tszUserName);

      debugLog(_T("cleanuplastplugin() 1: miranda_avatarcache = %s"), avatarsPath);

      if (avatarsPath !=  NULL){
         HANDLE hFind = INVALID_HANDLE_VALUE;
         TCHAR spec[MAX_PATH + 10];
         mir_sntprintf(spec, MAX_PATH + 10, _T("%s\\*.(null)"), avatarsPath);
         WIN32_FIND_DATA ffd;
         hFind = FindFirstFile(spec, &ffd);
         if (hFind != INVALID_HANDLE_VALUE) {
            do {
               TCHAR filePathT [2*MAX_PATH + 10];
               mir_sntprintf(filePathT, 2*MAX_PATH + 10, _T("%s\\%s"), avatarsPath, ffd.cFileName);
               if (!_taccess(filePathT, 0)){
                  debugLog(_T("cleanuplastplugin() 1: remove file = %s"), filePathT);
                  _tremove(filePathT);
               }
            } while (FindNextFile(hFind, &ffd) != 0);
            FindClose(hFind);
         }
      }
   }

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
// Gets protocol instance associated with a contact
static GGPROTO* gg_getprotoinstance(MCONTACT hContact)
{
   char* szProto = GetContactProto(hContact);
   if (szProto == NULL)
      return NULL;

   for (int i=0; i < g_Instances.getCount(); i++)
      if (strcmp(szProto, g_Instances[i]->m_szModuleName) == 0)
         return g_Instances[i];

   return NULL;
}

//////////////////////////////////////////////////////////
// Handles PrebuildContactMenu event
static int gg_prebuildcontactmenu(WPARAM hContact, LPARAM lParam)
{
   GGPROTO* gg = gg_getprotoinstance(hContact);
   if (gg == NULL)
      return 0;

   CLISTMENUITEM mi = { sizeof(mi) };
   mi.flags = CMIM_NAME | CMIM_FLAGS | CMIF_TCHAR;
   if ( gg->getDword(hContact, GG_KEY_UIN, 0) == gg->getByte(GG_KEY_UIN, 0) ||
        gg->isChatRoom(hContact) || db_get_b(hContact, "CList", "NotOnList", 0))
      mi.flags |= CMIF_HIDDEN;
   mi.ptszName = gg->getByte(hContact, GG_KEY_BLOCK, 0) ? LPGENT("&Unblock") : LPGENT("&Block");
   Menu_ModifyItem(gg->hBlockMenuItem, &mi);
   return 0;
}

//////////////////////////////////////////////////////////
// Contact block service function
INT_PTR GGPROTO::blockuser(WPARAM hContact, LPARAM lParam)
{
   setByte(hContact, GG_KEY_BLOCK, !getByte(hContact, GG_KEY_BLOCK, 0));
   notifyuser(hContact, 1);
   return 0;
}


//////////////////////////////////////////////////////////
// Contact blocking initialization

#define GGS_BLOCKUSER "/BlockUser"
void GGPROTO::block_init()
{
   char service[64];
   mir_snprintf(service, sizeof(service), "%s%s", m_szModuleName, GGS_BLOCKUSER);
   CreateProtoService(GGS_BLOCKUSER, &GGPROTO::blockuser);

   CLISTMENUITEM mi = { sizeof(mi) };
   mi.flags = CMIF_TCHAR;
   mi.position = -500050000;
   mi.icolibItem = iconList[8].hIcolib;
   mi.ptszName = LPGENT("&Block");
   mi.pszService = service;
   mi.pszContactOwner = m_szModuleName;
   hBlockMenuItem = Menu_AddContactMenuItem(&mi);

   ::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, gg_prebuildcontactmenu);
}

//////////////////////////////////////////////////////////
// Contact blocking uninitialization

void GGPROTO::block_uninit()
{
   CallService(MO_REMOVEMENUITEM, (WPARAM)hBlockMenuItem, 0);
}

//////////////////////////////////////////////////////////
// Menus initialization
void GGPROTO::menus_init()
{
   CLISTMENUITEM mi = { sizeof(mi) };

   HGENMENU hGCRoot, hCLRoot, hRoot = MO_GetProtoRootMenu(m_szModuleName);
   if (hRoot == NULL) {
      mi.ptszName = m_tszUserName;
      mi.position = 500090000;
      mi.hParentMenu = HGENMENU_ROOT;
      mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
      mi.icolibItem = iconList[0].hIcolib;
      hGCRoot = hCLRoot = hRoot = hMenuRoot = Menu_AddProtoMenuItem(&mi);
   }
   else {
      mi.hParentMenu = hRoot;
      mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;

      mi.ptszName = LPGENT("Conference");
      mi.position = 200001;
      mi.icolibItem = iconList[14].hIcolib;
      hGCRoot = Menu_AddProtoMenuItem(&mi);

      mi.ptszName = LPGENT("Contact list");
      mi.position = 200002;
      mi.icolibItem = iconList[7].hIcolib;
      hCLRoot = Menu_AddProtoMenuItem(&mi);

      if (hMenuRoot)
         CallService(MO_REMOVEMENUITEM, (WPARAM)hMenuRoot, 0);
      hMenuRoot = NULL;
   }

   gc_menus_init(hGCRoot);
   import_init(hCLRoot);
   sessions_menus_init(hRoot);
}

//////////////////////////////////////////////////////////
// Module instance initialization

static GGPROTO *gg_proto_init(const char* pszProtoName, const TCHAR* tszUserName)
{
   GGPROTO *gg = new GGPROTO(pszProtoName, tszUserName);
   g_Instances.insert(gg);
   return gg;
}

//////////////////////////////////////////////////////////
// Module instance uninitialization

static int gg_proto_uninit(PROTO_INTERFACE *proto)
{
   GGPROTO *gg = (GGPROTO *)proto;
   g_Instances.remove(gg);
   delete gg;
   return 0;
}

//////////////////////////////////////////////////////////
// When plugin is loaded

extern "C" int __declspec(dllexport) Load(void)
{
   mir_getXI(&xi);
   mir_getLP(&pluginInfo);
   mir_getCLI();

   // Hook system events
   hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, gg_modulesloaded);

   // Prepare protocol name
   PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
   pd.szName = GGDEF_PROTO;
   pd.fnInit = (pfnInitProto)gg_proto_init;
   pd.fnUninit = (pfnUninitProto)gg_proto_uninit;
   pd.type = PROTOTYPE_PROTOCOL;

   // Register module
   CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) &pd);
   gg_links_instancemenu_init();
   return 0;
}

//////////////////////////////////////////////////////////
// When plugin is unloaded

extern "C" int __declspec(dllexport) Unload()
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
   {GG_EVENT_XML_ACTION,           "GG_EVENT_XML_ACTION"},
   {GG_EVENT_TYPING_NOTIFICATION,  "GG_EVENT_TYPING_NOTIFICATION"},
   {GG_EVENT_USER_DATA,            "GG_EVENT_USER_DATA"},
   {GG_EVENT_MULTILOGON_MSG,       "GG_EVENT_MULTILOGON_MSG"},
   {GG_EVENT_MULTILOGON_INFO,      "GG_EVENT_MULTILOGON_INFO"},
   {-1,                            "<unknown event>"}
};

const char *ggdebug_eventtype(gg_event *e)
{
   int i;
   for(i = 0; ggdebug_eventype2string[i].type != -1; i++)
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
   if (nl) *nl = 0;

   strncpy(szText + PREFIXLEN, "[libgadu] \0", sizeof(szText) - PREFIXLEN);

   char prefix[6];
   mir_snprintf(prefix, PREFIXLEN, "%lu", GetCurrentThreadId());
   size_t prefixLen = strlen(prefix);
   if (prefixLen < PREFIXLEN) memset(prefix + prefixLen, ' ', PREFIXLEN - prefixLen);
   memcpy(szText, prefix, PREFIXLEN);

   mir_vsnprintf(szText + strlen(szText), sizeof(szText) - strlen(szText), szFormat, ap);
   CallService(MS_NETLIB_LOG, NULL, (LPARAM)szText);
   free(szFormat);
}
#endif

//////////////////////////////////////////////////////////
// main DLL function

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
   crc_gentable();
   hInstance = hInst;
#ifdef DEBUGMODE
   gg_debug_level = GG_DEBUG_FUNCTION;
   gg_debug_handler = gg_debughandler;
#endif
   return TRUE;
}
