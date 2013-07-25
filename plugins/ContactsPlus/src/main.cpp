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

int hLangpack;

int g_Utf8EventsSupported = TRUE;

HANDLE ghSendWindowList;
HANDLE ghRecvWindowList;
gAckList gaAckData;

HGENMENU hContactMenuItem;

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
	// {0324785E-74CE-4600-B781-851773B3EFC5}
	{0x0324785E, 0x74CE, 0x4600, {0xB7, 0x81, 0x85, 0x17, 0x73, 0xB3, 0xEF, 0xC5}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  hInst = hinstDLL;
  return TRUE;
}

static int HookDBEventAdded(WPARAM wParam, LPARAM lParam)
{
  HANDLE hContact = (HANDLE)wParam;
  HANDLE hDbEvent = (HANDLE)lParam;
  //process the event
  DBEVENTINFO dbe = { sizeof(dbe) };
  db_event_get(hDbEvent, &dbe);
  //check if we should process the event
  if (dbe.flags & (DBEF_SENT|DBEF_READ) || dbe.eventType != EVENTTYPE_CONTACTS) return 0;
  //get event contents
  dbe.cbBlob = db_event_getBlobSize(hDbEvent);
  if (dbe.cbBlob != -1)
    dbe.pBlob = (PBYTE)_alloca(dbe.cbBlob);
  db_event_get(hDbEvent, &dbe);
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
    mir_sntprintf(caToolTip, SIZEOF(caToolTip), "%s %s", SRCTranslateT("Contacts received from", tmp), (TCHAR*)GetContactDisplayNameT(hContact));

    cle.ptszTooltip = caToolTip;
    cle.flags |= CLEF_UNICODE;
    CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
  }
  return 0; //continue processing by other hooks
}

static void ProcessUnreadEvents(void)
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hDbEvent = db_event_firstUnread(hContact);
		while (hDbEvent) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags & (DBEF_SENT|DBEF_READ)) && dbei.eventType == EVENTTYPE_CONTACTS) {
				//process the event
				HookDBEventAdded((WPARAM)hContact, (LPARAM)hDbEvent);
			}
			hDbEvent = db_event_next(hDbEvent);
		}
	}
}

static bool CheckContactsServiceSupport(const char* szProto)
{
	// there is no way to determine if the service exists (only proto_interface call is supported by 0.8+)
	if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_CONTACTSEND)
		return true;
	else
		return false;
}

static int HookPreBuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	char *szProto = GetContactProto(hContact);
	int bVisible = FALSE;

	if (szProto && CheckContactsServiceSupport(szProto)) {
		// known protocol, protocol supports contacts sending
		// check the selected contact if it supports contacts receive
		if (CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXCONTACTSPERPACKET, (LPARAM)hContact))
			bVisible = TRUE;
	}

	// update contact menu item's visibility
	Menu_ShowItem(hContactMenuItem, bVisible);
	return 0;
}

static int HookModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	char* modules[2] = {0};
	WCHAR tmp[MAX_PATH];

	modules[0] = MODULENAME;
	CallService("DBEditorpp/RegisterModule",(WPARAM)modules,(LPARAM)1);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.ptszName = SRCTranslateT("Contacts", tmp);
	mi.position = -2000009990;  //position in menu
	mi.flags = CMIF_KEEPUNTRANSLATED;
	mi.flags |= CMIF_UNICODE;
	mi.pszService = MS_CONTACTS_SEND;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS));
	hContactMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, HookPreBuildContactMenu);

	ghSendWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0); // no need to destroy this
	ghRecvWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0); // no need to destroy this

	ProcessUnreadEvents();
	return 0;
}

static int HookContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
  DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
  char *szProto =GetContactProto((HANDLE)wParam);

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

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	InitCommonControls();
	InitI18N();

	//init hooks
	HookEvent(ME_SYSTEM_MODULESLOADED, HookModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, HookDBEventAdded);
	HookEvent(ME_DB_CONTACT_DELETED, HookContactDeleted);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, HookContactSettingChanged);

	//create services
	CreateServiceFunction(MS_CONTACTS_SEND, ServiceSendCommand);
	CreateServiceFunction(MS_CONTACTS_RECEIVE, ServiceReceiveCommand);

	//define event sounds
	SkinAddNewSound("RecvContacts", LPGEN("Incoming Contacts"), "contacts.wav");
	SkinAddNewSound("SentContacts", LPGEN("Outgoing Contacts"), "ocontacts.wav");
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
