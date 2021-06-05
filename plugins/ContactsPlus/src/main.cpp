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

#include "stdafx.h"

CMPlugin g_plugin;

int g_Utf8EventsSupported = TRUE;

MWindowList g_hSendWindowList;
MWindowList g_hRecvWindowList;
gAckList g_aAckData;

HGENMENU hContactMenuItem;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {0324785E-74CE-4600-B781-851773B3EFC5}
	{ 0x0324785E, 0x74CE, 0x4600, { 0xB7, 0x81, 0x85, 0x17, 0x73, 0xB3, 0xEF, 0xC5 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int HookDBEventAdded(WPARAM hContact, LPARAM hDbEvent)
{
	// process the event
	DBEVENTINFO dbe = {};
	db_event_get(hDbEvent, &dbe);
	
	// check if we should process the event
	if (dbe.flags & (DBEF_SENT | DBEF_READ) || dbe.eventType != EVENTTYPE_CONTACTS)
		return 0;
	
	// play received sound
	Skin_PlaySound("RecvContacts");

	// add event to the contact list
	wchar_t caToolTip[128];
	mir_snwprintf(caToolTip, L"%s %s", TranslateT("Contacts received from"), Clist_GetContactDisplayName(hContact));

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.hDbEvent = hDbEvent;
	cle.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_CONTACTS));
	cle.pszService = MS_CONTACTS_RECEIVE;
	cle.szTooltip.w = caToolTip;
	cle.flags |= CLEF_UNICODE;
	g_clistApi.pfnAddEvent(&cle);
	return 0;
}

static void ProcessUnreadEvents(void)
{
	for (auto &hContact : Contacts()) {
		for (MEVENT hDbEvent = db_event_firstUnread(hContact); hDbEvent; hDbEvent = db_event_next(hContact, hDbEvent)) {
			DBEVENTINFO dbei = {};
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

static int HookPreBuildContactMenu(WPARAM hContact, LPARAM)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	bool bVisible = false;

	if (szProto && CheckContactsServiceSupport(szProto)) {
		// known protocol, protocol supports contacts sending
		// check the selected contact if it supports contacts receive
		if (CallProtoService(szProto, PS_GETCAPS, PFLAG_MAXCONTACTSPERPACKET, hContact))
			bVisible = true;
	}

	// update contact menu item's visibility
	Menu_ShowItem(hContactMenuItem, bVisible);
	return 0;
}

static int HookModulesLoaded(WPARAM, LPARAM)
{
	char* modules[2] = {};

	modules[0] = MODULENAME;
	CallService("DBEditorpp/RegisterModule", (WPARAM)modules, 1);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x94a4ed39, 0xabd0, 0x4c70, 0x89, 0xeb, 0x1b, 0x2, 0xf0, 0xac, 0x6, 0x4c);
	mi.name.a = LPGEN("Contacts");
	mi.position = -2000009990;  //position in menu
	mi.pszService = MS_CONTACTS_SEND;
	mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_CONTACTS));
	hContactMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, HookPreBuildContactMenu);

	ProcessUnreadEvents();
	return 0;
}

static int HookContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (strcmpnull(cws->szModule, "CList") && strcmpnull(cws->szModule, szProto))
		return 0;

	WindowList_Broadcast(g_hSendWindowList, DM_UPDATETITLE, 0, 0);
	WindowList_Broadcast(g_hRecvWindowList, DM_UPDATETITLE, 0, 0);
	return 0;
}

static int HookContactDeleted(WPARAM wParam, LPARAM)
{  // if our contact gets deleted close his window
	HWND h = WindowList_Find(g_hSendWindowList, wParam);

	if (h)
		SendMessage(h, WM_CLOSE, 0, 0);

	// since we hack the window list - more windows for one contact, we need to close them all
	while (h = WindowList_Find(g_hRecvWindowList, wParam))
		SendMessage(h, WM_CLOSE, 0, 0);
	return 0;
}

static INT_PTR ServiceSendCommand(WPARAM wParam, LPARAM)
{
	//find window for hContact
	HWND hWnd = WindowList_Find(g_hSendWindowList, wParam);
	if (!hWnd)
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SEND), nullptr, SendDlgProc, wParam);
	else {
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	return 0;
}

static INT_PTR ServiceReceiveCommand(WPARAM, LPARAM lParam)
{
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_RECEIVE), nullptr, RecvDlgProc, lParam);
	return 0;
}

int CMPlugin::Load()
{
	InitCommonControls();

	g_hSendWindowList = WindowList_Create();
	g_hRecvWindowList = WindowList_Create();

	//init hooks
	HookEvent(ME_SYSTEM_MODULESLOADED, HookModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, HookDBEventAdded);
	HookEvent(ME_DB_CONTACT_DELETED, HookContactDeleted);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, HookContactSettingChanged);

	//create services
	CreateServiceFunction(MS_CONTACTS_SEND, ServiceSendCommand);
	CreateServiceFunction(MS_CONTACTS_RECEIVE, ServiceReceiveCommand);

	//define event sounds
	g_plugin.addSound("RecvContacts", LPGENW("Events"), LPGENW("Incoming Contacts"), L"contacts.wav");
	g_plugin.addSound("SentContacts", LPGENW("Events"), LPGENW("Outgoing Contacts"), L"ocontacts.wav");
	return 0;
}

int CMPlugin::Unload()
{
	WindowList_Destroy(g_hSendWindowList);
	WindowList_Destroy(g_hRecvWindowList);
	return 0;
}
