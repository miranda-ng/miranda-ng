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

CLIST_INTERFACE *pcli;

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
	{ 0x0324785E, 0x74CE, 0x4600, { 0xB7, 0x81, 0x85, 0x17, 0x73, 0xB3, 0xEF, 0xC5 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static int HookDBEventAdded(WPARAM hContact, LPARAM lParam)
{
	HANDLE hDbEvent = (HANDLE)lParam;
	//process the event
	DBEVENTINFO dbe = { sizeof(dbe) };
	db_event_get(hDbEvent, &dbe);
	//check if we should process the event
	if (dbe.flags & (DBEF_SENT | DBEF_READ) || dbe.eventType != EVENTTYPE_CONTACTS) return 0;
	//get event contents
	dbe.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbe.cbBlob != -1)
		dbe.pBlob = (PBYTE)_alloca(dbe.cbBlob);
	db_event_get(hDbEvent, &dbe);
	//play received sound
	SkinPlaySound("RecvContacts");
	{
		//add event to the contact list
		TCHAR caToolTip[128];
		mir_sntprintf(caToolTip, SIZEOF(caToolTip), _T("%s %s"), TranslateT("Contacts received from"), pcli->pfnGetContactDisplayName(hContact, 0));

		CLISTEVENT cle = { sizeof(cle) };
		cle.hContact = hContact;
		cle.hDbEvent = hDbEvent;
		cle.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS));
		cle.pszService = MS_CONTACTS_RECEIVE;
		cle.ptszTooltip = caToolTip;
		cle.flags |= CLEF_UNICODE;
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
	}
	return 0; //continue processing by other hooks
}

static void ProcessUnreadEvents(void)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		for (HANDLE hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && dbei.eventType == EVENTTYPE_CONTACTS) {
				//process the event
				HookDBEventAdded(hContact, (LPARAM)hDbEvent);
			}
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

static int HookPreBuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	char *szProto = GetContactProto(hContact);
	int bVisible = FALSE;

	if (szProto && CheckContactsServiceSupport(szProto)) {
		// known protocol, protocol supports contacts sending
		// check the selected contact if it supports contacts receive
		if (CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXCONTACTSPERPACKET, hContact))
			bVisible = TRUE;
	}

	// update contact menu item's visibility
	Menu_ShowItem(hContactMenuItem, bVisible);
	return 0;
}

static int HookModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	char* modules[2] = { 0 };

	modules[0] = MODULENAME;
	CallService("DBEditorpp/RegisterModule", (WPARAM)modules, 1);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszName = LPGEN("Contacts");
	mi.position = -2000009990;  //position in menu
	mi.pszService = MS_CONTACTS_SEND;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS));
	hContactMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, HookPreBuildContactMenu);

	ProcessUnreadEvents();
	return 0;
}

static int HookContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *szProto = GetContactProto(hContact);
	if (strcmpnull(cws->szModule, "CList") && strcmpnull(cws->szModule, szProto))
		return 0;

	WindowList_Broadcast(ghSendWindowList, DM_UPDATETITLE, 0, 0);
	WindowList_Broadcast(ghRecvWindowList, DM_UPDATETITLE, 0, 0);
	return 0;
}

static int HookContactDeleted(WPARAM wParam, LPARAM lParam)
{  // if our contact gets deleted close his window
	HWND h = WindowList_Find(ghSendWindowList, wParam);

	if (h)
		SendMessage(h, WM_CLOSE, 0, 0);

	// since we hack the window list - more windows for one contact, we need to close them all
	while (h = WindowList_Find(ghRecvWindowList, wParam))
		SendMessage(h, WM_CLOSE, 0, 0);
	return 0;
}

static INT_PTR ServiceSendCommand(WPARAM wParam, LPARAM lParam)
{
	//find window for hContact
	HWND hWnd = WindowList_Find(ghSendWindowList, wParam);
	if (!hWnd)
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SEND), NULL, SendDlgProc, wParam);
	else {
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	return 0;
}

static INT_PTR ServiceReceiveCommand(WPARAM wParam, LPARAM lParam)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_RECEIVE), NULL, RecvDlgProc, lParam);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	InitCommonControls();

	ghSendWindowList = WindowList_Create();
	ghRecvWindowList = WindowList_Create();

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
	WindowList_Destroy(ghSendWindowList);
	WindowList_Destroy(ghRecvWindowList);
	return 0;
}
