// --------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
// 
// Copyright © 2002 Dominus Procellarum 
// Copyright © 2004-2008 Joe Kucera
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
//
// -----------------------------------------------------------------------------

#include "contacts.h"


HINSTANCE hInst;

PLUGINLINK *pluginLink;

int g_NewProtoAPI = FALSE;

int g_SendAckSupported = FALSE;
int g_Utf8EventsSupported = FALSE;

HANDLE ghSendWindowList;
HANDLE ghRecvWindowList;
gAckList gaAckData;

HANDLE hServiceSend;
HANDLE hServiceReceive;

HANDLE hHookModulesLoaded = NULL;
HANDLE hHookDBEventAdded = NULL;
HANDLE hHookContactDeleted = NULL;
HANDLE hHookContactSettingChanged = NULL;
HANDLE hHookPreBuildContactMenu = NULL;

HANDLE hContactMenuItem = NULL;

int g_UnicodeCore;

PLUGININFOEX pluginInfo = {
  sizeof(PLUGININFOEX),
#ifdef WIN64
  "Send/Receive Contacts+ (x64)",
#else
  "Send/Receive Contacts+",
#endif
  PLUGIN_MAKE_VERSION(1,5,2,0),
  "Allows you to send and receive contacts",
  "Joe Kucera, Todor Totev",
  "jokusoftware@miranda-im.org",
  "(C) 2004-2008 Joe Kucera, Original Code (C) 2002 Dominus Procellarum",
  "http://addons.miranda-im.org/details.php?action=viewfile&id=1253",
  0, //no flags by default
  0, //doesn't replace anything built-in
  {0x0324785E, 0x74CE, 0x4600,  {0xB7, 0x81, 0x85, 0x17, 0x73, 0xB3, 0xEF, 0xC5 } } // {0324785E-74CE-4600-B781-851773B3EFC5}
};


static int HookDBEventAdded(WPARAM wParam, LPARAM lParam)
{
  HANDLE hContact = (HANDLE)wParam;
  HANDLE hDbEvent = (HANDLE)lParam;
  //process the event
  DBEVENTINFO dbe = {0};

  dbe.cbSize = sizeof(DBEVENTINFO);
  //get event details
  CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe);
  //check if we should process the event
  if (dbe.flags & (DBEF_SENT|DBEF_READ) || dbe.eventType != EVENTTYPE_CONTACTS) return 0;
  //get event contents
  dbe.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0);
  if (dbe.cbBlob != -1)
    dbe.pBlob = (PBYTE)_alloca(dbe.cbBlob);
  CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbe);
  //play received sound
  SkinPlaySound("RecvContacts");
  { //add event to the contact list
    CLISTEVENT cle = {0};
    TCHAR caToolTip[128];

    cle.cbSize = sizeof(cle);
    cle.hContact = hContact;
    cle.hDbEvent = hDbEvent;
    cle.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS));
    cle.pszService = MS_CONTACTS_RECEIVE;

    WCHAR tmp[MAX_PATH];
    _snprintfT(caToolTip, 64, "%s %s", SRCTranslateT("Contacts received from", tmp), (TCHAR*)GetContactDisplayNameT(hContact));

    cle.ptszTooltip = caToolTip;
    if (g_UnicodeCore)
      cle.flags |= CLEF_UNICODE;
    CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
  }
  return 0; //continue processing by other hooks
}


static void ProcessUnreadEvents(void)
{
  DBEVENTINFO dbei = {0};
  HANDLE hDbEvent,hContact;

  dbei.cbSize = sizeof(dbei);

  hContact = SRCFindFirstContact();
  while (hContact) 
  {
    hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0);

    while (hDbEvent) 
    {
      dbei.cbBlob=0;
      CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei);
      if (!(dbei.flags&(DBEF_SENT|DBEF_READ)) && dbei.eventType==EVENTTYPE_CONTACTS) 
      { //process the event
        HookDBEventAdded((WPARAM)hContact, (LPARAM)hDbEvent);
      }
      hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0);
    }
    hContact = SRCFindNextContact(hContact);
  }
}


static bool CheckContactsServiceSupport(const char* szProto)
{
  if (g_NewProtoAPI)
  { // there is no way to determine if the service exists (only proto_interface call is supported by 0.8+)
    if (SRCCallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_CONTACTSEND)
      return true;
  }
  else
  { // check the real send service (only 0.7.x and older)
    char serstr[MAX_PATH+30];

    strcpy(serstr, szProto);
    strcat(serstr, PSS_CONTACTS);
    if (ServiceExists(serstr))
      return true;
  }
  return false;
}


static int HookPreBuildContactMenu(WPARAM wParam, LPARAM lParam)
{
  HANDLE hContact = (HANDLE)wParam;
  char* szProto = GetContactProto(hContact);
  int bVisible = FALSE;

  if (szProto && CheckContactsServiceSupport(szProto))
  { // known protocol, protocol supports contacts sending
    // check the selected contact if it supports contacts receive
    if (SRCCallProtoService(szProto, PS_GETCAPS, PFLAG_MAXCONTACTSPERPACKET, (LPARAM)hContact))
      bVisible = TRUE;
  }

  { // update contact menu item's visibility
    CLISTMENUITEM mi = {0};

    mi.cbSize = sizeof(mi);
    if (bVisible)
      mi.flags = CMIM_FLAGS;
    else
      mi.flags = CMIM_FLAGS | CMIF_HIDDEN;

    CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactMenuItem, (LPARAM)&mi);
  }

  return 0;
}


static int HookModulesLoaded(WPARAM wParam, LPARAM lParam)
{
  char* modules[2] = {0};
  WCHAR tmp[MAX_PATH];

  modules[0] = MODULENAME;
  CallService("DBEditorpp/RegisterModule",(WPARAM)modules,(LPARAM)1);

  CLISTMENUITEM mi = {0};
  mi.cbSize = sizeof(mi);
  mi.ptszName = SRCTranslateT("Contacts", tmp);
  mi.position = -2000009990;  //position in menu
  mi.flags = CMIF_KEEPUNTRANSLATED;
  if (g_UnicodeCore)
    mi.flags |= CMIF_UNICODE;
  mi.pszService = MS_CONTACTS_SEND; 
  mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS));

  hContactMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
  hHookPreBuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, HookPreBuildContactMenu);

  ghSendWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0); // no need to destroy this
  ghRecvWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0); // no need to destroy this

#ifndef WIN64
  //register for Update (the FL name is different...)
  CallService(MS_UPDATE_REGISTERFL, 1253, (WPARAM)&pluginInfo);
#endif

  ProcessUnreadEvents();
  return 0;
}


static int HookContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
  DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
  char *szProto = GetContactProto((HANDLE)wParam);

  if (strcmpnull(cws->szModule,"CList") && strcmpnull(cws->szModule, szProto)) return 0;

  WindowList_Broadcast(ghSendWindowList,DM_UPDATETITLE,0,0);
  WindowList_Broadcast(ghRecvWindowList,DM_UPDATETITLE,0,0);

  return 0;
}


static int HookContactDeleted(WPARAM wParam, LPARAM lParam)
{  // if our contact gets deleted close his window
  HWND h = WindowList_Find(ghSendWindowList,(HANDLE)wParam);

  if (h) 
  {
    SendMessageT(h,WM_CLOSE,0,0);
  }

  while (h = WindowList_Find(ghRecvWindowList, (HANDLE)wParam))
  { // since we hack the window list - more windows for one contact, we need to close them all
    SendMessageT(h, WM_CLOSE,0,0);
  }
  return 0;
}


static INT_PTR ServiceSendCommand(WPARAM wParam, LPARAM lParam)
{
  HWND hWnd;
  //find window for hContact
  hWnd = WindowList_Find(ghSendWindowList, (HANDLE)wParam);

  if (!hWnd)
    CreateDialogParamT(hInst, MAKEINTRESOURCE(IDD_SEND), NULL, SendDlgProc, (LPARAM)(HANDLE)wParam);
  else
  {
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
  }
  return 0;
}

static INT_PTR ServiceReceiveCommand(WPARAM wParam, LPARAM lParam)
{
  CLISTEVENT* pcle = (CLISTEVENT*)lParam;

  CreateDialogParamT(hInst, MAKEINTRESOURCE(IDD_RECEIVE), NULL, RecvDlgProc, (LPARAM)pcle);

  return 0;
}


static void* PrepareMirandaPluginInfo(DWORD mirandaVersion)
{ 
  if (!(mirandaVersion >= PLUGIN_MAKE_VERSION(0,4,0,0)))
    pluginInfo.description = "Allows you to send and receive contacts; Please upgrade your Miranda IM to version 0.4 for better functionality.";
  else
    g_SendAckSupported = TRUE;

  if (mirandaVersion >= PLUGIN_MAKE_VERSION(0,7,0,0))
    g_Utf8EventsSupported = TRUE;

  if (mirandaVersion >= PLUGIN_MAKE_VERSION(0,8,0,8))
    g_NewProtoAPI = TRUE;

  if (mirandaVersion >= PLUGIN_MAKE_VERSION(0,3,3,0))
  {
    // Are we running under Unicode Windows version ?
    if ((GetVersion() & 0x80000000) == 0)
    {
      pluginInfo.flags = 1; // UNICODE_AWARE
    }
    return &pluginInfo; 
  }
  return NULL;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
  pluginInfo.cbSize = sizeof(PLUGININFOEX);
  return (PLUGININFOEX*)PrepareMirandaPluginInfo(mirandaVersion);
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
  pluginInfo.cbSize = sizeof(PLUGININFO);
  return (PLUGININFO*)PrepareMirandaPluginInfo(mirandaVersion);
}

static const MUUID interfaces[] = {MIID_SRCONTACTS, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
  pluginLink = link;
  InitCommonControls();
  InitI18N();

  { // Are we running under unicode Miranda core ?
    char szVer[MAX_PATH];

    CallService(MS_SYSTEM_GETVERSIONTEXT, MAX_PATH, (LPARAM)szVer);
    _strlwr(szVer);
    g_UnicodeCore = (strstr(szVer, "unicode") != NULL);
  }
  //init hooks
  hHookModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, HookModulesLoaded);
  hHookDBEventAdded = HookEvent(ME_DB_EVENT_ADDED, HookDBEventAdded);
  hHookContactDeleted = HookEvent(ME_DB_CONTACT_DELETED, HookContactDeleted);
  hHookContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, HookContactSettingChanged);
  //create services
  hServiceSend = CreateServiceFunction(MS_CONTACTS_SEND, ServiceSendCommand);
  hServiceReceive = CreateServiceFunction(MS_CONTACTS_RECEIVE, ServiceReceiveCommand);
  //define event sounds
  SkinAddNewSound("RecvContacts", LPGEN("Incoming Contacts"), "contacts.wav");
  SkinAddNewSound("SentContacts", LPGEN("Outgoing Contacts"), "ocontacts.wav");

  return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
  UnhookEvent(hHookModulesLoaded);
  UnhookEvent(hHookDBEventAdded);
  UnhookEvent(hHookContactDeleted);
  UnhookEvent(hHookContactSettingChanged);
  UnhookEvent(hHookPreBuildContactMenu);

  DestroyServiceFunction(hServiceSend);
  DestroyServiceFunction(hServiceReceive);

  return 0;
}
