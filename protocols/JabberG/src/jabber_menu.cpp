/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"
#include "jabber_list.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_disco.h"

#define MENUITEM_LASTSEEN	1
#define MENUITEM_SERVER		2
#define MENUITEM_RESOURCES	10

static MWindowList hDialogsList = nullptr;
static HANDLE hStatusMenuInit;

static int hChooserMenu;
static int iChooserMenuPos = 30000;

static HGENMENU g_hMenuAddBookmark;
static HGENMENU g_hMenuLogin;
static HGENMENU g_hMenuRefresh;
static HGENMENU g_hMenuCommands;
static HGENMENU g_hMenuSendNote;
static HGENMENU g_hMenuResourcesRoot;
static HGENMENU g_hMenuResourcesActive;
static HGENMENU g_hMenuResourcesServer;

struct
{
	int icon;
	int mode;
}
static PresenceModeArray[] =
{
	{ SKINICON_STATUS_ONLINE, ID_STATUS_ONLINE },
	{ SKINICON_STATUS_AWAY, ID_STATUS_AWAY },
	{ SKINICON_STATUS_NA, ID_STATUS_NA },
	{ SKINICON_STATUS_DND, ID_STATUS_DND },
	{ SKINICON_STATUS_FREE4CHAT, ID_STATUS_FREECHAT },
};
static HGENMENU g_hMenuDirectPresence[_countof(PresenceModeArray) + 1];

static INT_PTR JabberMenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;
	return 0;
}

static INT_PTR JabberMenuBookmarkAdd(WPARAM hContact, LPARAM lParam)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnMenuBookmarkAdd(hContact, lParam) : 0;
}

static INT_PTR JabberMenuTransportLogin(WPARAM hContact, LPARAM lParam)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnMenuTransportLogin(hContact, lParam) : 0;
}

static INT_PTR JabberMenuTransportResolve(WPARAM hContact, LPARAM lParam)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnMenuTransportResolve(hContact, lParam) : 0;
}

static INT_PTR JabberContactMenuRunCommands(WPARAM hContact, LPARAM lParam)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->ContactMenuRunCommands(hContact, lParam) : 0;
}

static INT_PTR JabberMenuSendNote(WPARAM hContact, LPARAM lParam)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnMenuSendNote(hContact, lParam) : 0;
}

static INT_PTR JabberMenuHandleResource(WPARAM hContact, LPARAM lParam, LPARAM lRes)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnMenuHandleResource(hContact, lParam, lRes) : 0;
}

static INT_PTR JabberMenuHandleDirectPresence(WPARAM hContact, LPARAM lParam, LPARAM lRes)
{
	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnMenuHandleDirectPresence(hContact, lParam, lRes) : 0;
}

static int JabberPrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	Menu_ShowItem(g_hMenuCommands, false);
	Menu_ShowItem(g_hMenuSendNote, false);
	Menu_ShowItem(g_hMenuLogin, false);
	Menu_ShowItem(g_hMenuRefresh, false);
	Menu_ShowItem(g_hMenuAddBookmark, false);
	Menu_ShowItem(g_hMenuResourcesRoot, false);
	Menu_ShowItem(g_hMenuDirectPresence[0], false);

	CJabberProto *ppro = CMPlugin::getInstance(hContact);
	return(ppro) ? ppro->OnPrebuildContactMenu(hContact, lParam) : 0;
}

void g_MenuInit(void)
{
	hStatusMenuInit = CreateHookableEvent(ME_JABBER_MENUINIT);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, JabberPrebuildContactMenu);

	CreateServiceFunction("Jabber/MenuChoose", JabberMenuChooseService);

	hChooserMenu = Menu_AddObject("JabberAccountChooser", LPGEN("Jabber account chooser"), nullptr, "Jabber/MenuChoose");
	{
		CMenuItem mi(&g_plugin);
		mi.name.a = "Cancel";
		mi.position = 9999999;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
		Menu_AddItem(hChooserMenu, &mi, nullptr);
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization

	CMenuItem mi(&g_plugin);

	// "Add to Bookmarks"
	SET_UID(mi, 0x7d06d00b, 0x3a3e, 0x4d65, 0xac, 0xc5, 0x63, 0xe2, 0x60, 0xbe, 0xc6, 0x6);
	mi.pszService = "Jabber/AddToBookmarks";
	mi.name.a = LPGEN("Add to Bookmarks");
	mi.position = -1999901006;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BOOKMARKS);
	g_hMenuAddBookmark = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuBookmarkAdd);

	// Login/logout
	SET_UID(mi, 0x7674d540, 0x2638, 0x4958, 0x99, 0xda, 0x8, 0x3f, 0xad, 0x66, 0x8f, 0xed);
	mi.pszService = "Jabber/TransportLogin";
	mi.name.a = LPGEN("Login/logout");
	mi.position = -1999901007;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_LOGIN);
	g_hMenuLogin = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuTransportLogin);

	// Retrieve nicks
	SET_UID(mi, 0x6adf70d9, 0x6e92, 0x4a4f, 0x90, 0x71, 0x67, 0xa7, 0xaa, 0x1a, 0x19, 0x7a);
	mi.pszService = "Jabber/TransportGetNicks";
	mi.name.a = LPGEN("Resolve nicks");
	mi.position = -1999901008;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_REFRESH);
	g_hMenuRefresh = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuTransportResolve);

	// Run Commands
	SET_UID(mi, 0x25546e26, 0xc82, 0x4715, 0xb8, 0xca, 0xe5, 0xf7, 0x2a, 0x58, 0x9, 0x2);
	mi.pszService = "Jabber/RunCommands";
	mi.name.a = LPGEN("Commands");
	mi.position = -1999901009;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_COMMAND);
	g_hMenuCommands = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberContactMenuRunCommands);

	// Send Note
	SET_UID(mi, 0xf4b0cc51, 0xab9, 0x4cf0, 0x96, 0xaa, 0x22, 0xa0, 0x33, 0x9b, 0x56, 0xc5);
	mi.pszService = "Jabber/SendNote";
	mi.name.a = LPGEN("Send Note");
	mi.position = -1999901010;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SEND_NOTE);
	g_hMenuSendNote = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuSendNote);

	//////////////////////////////////////////////////////////////////////////////////////
	// Direct Presence

	SET_UID(mi, 0x89803943, 0xa87e, 0x4ae9, 0xbf, 0x79, 0xe3, 0xf3, 0xd6, 0x86, 0xf8, 0x3d);
	mi.pszService = "Jabber/DirectPresenceDummySvc";
	mi.name.a = LPGEN("Send Presence");
	mi.position = -1999901011;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_NOTES);
	g_hMenuDirectPresence[0] = Menu_AddContactMenuItem(&mi);
	UNSET_UID(mi);

	mi.flags |= CMIF_UNICODE | CMIF_SYSTEM;
	mi.root = g_hMenuDirectPresence[0];
	for (int i = 0; i < _countof(PresenceModeArray); i++) {
		char buf[] = "Jabber/DirectPresenceX";
		buf[_countof(buf) - 2] = '0' + i;
		mi.pszService = buf;
		mi.name.w = Clist_GetStatusModeDescription(PresenceModeArray[i].mode, 0);
		mi.position = -1999901000;
		mi.hIcolibItem = Skin_LoadIcon(PresenceModeArray[i].icon);
		g_hMenuDirectPresence[i + 1] = Menu_AddContactMenuItem(&mi);
		CreateServiceFunctionParam(mi.pszService, JabberMenuHandleDirectPresence, PresenceModeArray[i].mode);
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Resource selector

	SET_UID(mi, 0x32a7bb9d, 0x4d9, 0x49b3, 0xac, 0x8f, 0x83, 0xb5, 0x6b, 0xff, 0x4f, 0x5);
	mi.flags = 0;
	mi.root = nullptr;
	mi.pszService = "Jabber/ResourceSelectorDummySvc";
	mi.name.a = LPGEN("Jabber Resource");
	mi.position = -1999901011;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_JABBER);
	g_hMenuResourcesRoot = Menu_AddContactMenuItem(&mi);
	UNSET_UID(mi);

	mi.flags |= CMIF_SYSTEM; // these menu items aren't tunable
	mi.root = g_hMenuResourcesRoot;
	mi.pszService = "Jabber/UseResource_last";
	mi.name.a = LPGEN("Last Active");
	mi.position = -1999901000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_JABBER);
	g_hMenuResourcesActive = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionParam(mi.pszService, JabberMenuHandleResource, MENUITEM_LASTSEEN);

	mi.pszService = "Jabber/UseResource_server";
	mi.name.a = LPGEN("Server's Choice");
	mi.position = -1999901000;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_NODE_SERVER);
	g_hMenuResourcesServer = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionParam(mi.pszService, JabberMenuHandleResource, MENUITEM_SERVER);
}

void g_MenuUninit(void)
{
	DestroyHookableEvent(hStatusMenuInit);

	Menu_RemoveItem(g_hMenuLogin);
	Menu_RemoveItem(g_hMenuRefresh);
	Menu_RemoveItem(g_hMenuAddBookmark);

	WindowList_Destroy(hDialogsList);
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact menu services

int CJabberProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		return 0;

	Menu_ShowItem(GetMenuItem(PROTO_MENU_LOAD_HISTORY), false);
	bool bIsChatRoom = isChatRoom(hContact);
	bool bIsTransport = getBool(hContact, "IsTransport", false);

	if (!m_bJabberOnline)
		return 0;

	Menu_ShowItem(g_hMenuDirectPresence[0], TRUE);
	for (int i = 0; i < _countof(PresenceModeArray); i++)
		Menu_ModifyItem(g_hMenuDirectPresence[i + 1], nullptr, Skin_GetProtoIcon(m_szModuleName, PresenceModeArray[i].mode));

	if (m_ThreadInfo && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_MAM))
		Menu_ShowItem(GetMenuItem(PROTO_MENU_LOAD_HISTORY), true);

	if (bIsChatRoom) {
		ptrA roomid(getUStringA(hContact, "ChatRoomID"));
		if (ListGetItemPtr(LIST_BOOKMARK, roomid) == nullptr)
			if (m_ThreadInfo && m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)
				Menu_ShowItem(g_hMenuAddBookmark, TRUE);
	}

	if (bIsChatRoom == GCW_CHATROOM)
		return 0;

	if (bIsTransport) {
		Menu_ShowItem(g_hMenuLogin, TRUE);
		Menu_ShowItem(g_hMenuRefresh, TRUE);
	}

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid == nullptr)
		return 0;

	JabberCapsBits jcb = GetTotalJidCapabilities(jid);
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return 0;

	bool bCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	Menu_ShowItem(GetMenuItem(PROTO_MENU_REQ_AUTH), item->subscription == SUB_FROM || item->subscription == SUB_NONE || bCtrlPressed);
	Menu_ShowItem(GetMenuItem(PROTO_MENU_GRANT_AUTH), bCtrlPressed);
	Menu_ShowItem(GetMenuItem(PROTO_MENU_REVOKE_AUTH), item->subscription == SUB_FROM || item->subscription == SUB_BOTH || bCtrlPressed);
	Menu_ShowItem(g_hMenuCommands, ((jcb & JABBER_CAPS_COMMANDS) != 0) || bCtrlPressed);
	Menu_ShowItem(g_hMenuSendNote, TRUE);

	if (item->arResources.getCount() == 0)
		return 0;

	Menu_ShowItem(g_hMenuResourcesRoot, TRUE);
	Menu_ModifyItem(g_hMenuResourcesRoot, nullptr, m_hProtoIcon);
	Menu_ModifyItem(g_hMenuResourcesActive, nullptr, m_hProtoIcon, (item->resourceMode == RSMODE_LASTSEEN) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(g_hMenuResourcesServer, nullptr, m_hProtoIcon, (item->resourceMode == RSMODE_SERVER) ? CMIF_CHECKED : 0);

	int nMenuResourceItemsNew = m_nMenuResourceItems;
	if (m_nMenuResourceItems < item->arResources.getCount()) {
		m_phMenuResourceItems = (HGENMENU*)mir_realloc(m_phMenuResourceItems, item->arResources.getCount() * sizeof(HGENMENU));
		nMenuResourceItemsNew = item->arResources.getCount();
	}

	char text[256];
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_SYSTEM;
	mi.pszService = text;

	for (int i = 0; i < nMenuResourceItemsNew; i++) {
		mir_snprintf(text, "/UseResource_%d", i);
		if (i >= m_nMenuResourceItems) {
			CreateProtoServiceParam(text, &CJabberProto::OnMenuHandleResource, MENUITEM_RESOURCES + i);
			mi.name.a = "";
			mi.position = i;
			mi.root = g_hMenuResourcesRoot;
			m_phMenuResourceItems[i] = Menu_AddContactMenuItem(&mi, m_szModuleName);
		}
		if (i < item->arResources.getCount()) {
			pResourceStatus r(item->arResources[i]);

			HICON hIcon = (HICON)INVALID_HANDLE_VALUE;
			Menu_SetChecked(m_phMenuResourceItems[i], item->resourceMode == RSMODE_MANUAL && item->m_pManualResource == r);

			if (ServiceExists(MS_FP_GETCLIENTICONT)) {
				CMStringA szTmp;
				FormatMirVer(r, szTmp);
				hIcon = Finger_GetClientIcon(Utf2T(szTmp), 0);
			}

			CMStringW szTmp;
			szTmp.Format(L"%s [%s, %d]", Utf2T(r->m_szResourceName).get(), Clist_GetStatusModeDescription(r->m_iStatus, 0), r->m_iPriority);
			Menu_ModifyItem(m_phMenuResourceItems[i], szTmp, hIcon);
			DestroyIcon(hIcon);
		}
		else Menu_ShowItem(m_phMenuResourceItems[i], FALSE);
	}

	m_nMenuResourceItems = nMenuResourceItemsNew;
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleRequestAuth(WPARAM hContact, LPARAM)
{
	if (hContact != 0 && m_bJabberOnline) {
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid != nullptr)
			m_ThreadInfo->send(XmlNode("presence") << XATTR("to", jid) << XATTR("type", "subscribe"));
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleGrantAuth(WPARAM hContact, LPARAM)
{
	if (hContact != 0 && m_bJabberOnline) {
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid != nullptr)
			m_ThreadInfo->send(XmlNode("presence") << XATTR("to", jid) << XATTR("type", "subscribed"));
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleRevokeAuth(WPARAM hContact, LPARAM)
{
	if (hContact != 0 && m_bJabberOnline) {
		ptrA jid(getUStringA(hContact, "jid"));
		if (jid != nullptr)
			m_ThreadInfo->send(XmlNode("presence") << XATTR("to", jid) << XATTR("type", "unsubscribed"));
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuTransportLogin(WPARAM hContact, LPARAM)
{
	if (!getByte(hContact, "IsTransport", 0))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrA(getUStringA(hContact, "jid")));
	if (item != nullptr) {
		XmlNode p("presence"); XmlAddAttr(p, "to", item->jid);
		if (item->getTemp()->m_iStatus == ID_STATUS_ONLINE)
			XmlAddAttr(p, "type", "unavailable");
		m_ThreadInfo->send(p);
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuTransportResolve(WPARAM hContact, LPARAM)
{
	if (!getByte(hContact, "IsTransport", 0))
		return 0;

	ptrA jid(getUStringA(hContact, "jid"));
	if (jid != nullptr)
		ResolveTransportNicks(jid);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuBookmarkAdd(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0; // we do not add ourself to the roster. (buggy situation - should not happen)

	ptrA roomID(getUStringA(hContact, "ChatRoomID"));
	if (roomID == nullptr)
		return 0;

	if (ListGetItemPtr(LIST_BOOKMARK, roomID) == nullptr) {
		JABBER_LIST_ITEM *item = new JABBER_LIST_ITEM();
		item->jid = mir_strdup(roomID);
		item->name = Clist_GetContactDisplayName(hContact);
		item->type = mir_strdup("conference");
		item->nick = getUStringA(hContact, "MyNick");
		AddEditBookmark(item);
		delete item;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// status menu

void CJabberProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = m_hMenuRoot = Menu_GetProtocolRoot(this);
	mi.flags = CMIF_UNMOVABLE;

	// "Bookmarks..."
	mi.pszService = "/Bookmarks";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleBookmarks);
	mi.name.a = LPGEN("Bookmarks");
	mi.position = 200001;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BOOKMARKS);
	m_hMenuBookmarks = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// "Services..."
	mi.pszService = nullptr;
	mi.name.a = LPGEN("Services...");
	mi.position = 200003;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SERVICE_DISCOVERY);
	HGENMENU hMenuServicesRoot = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// "Service Discovery..."
	mi.root = hMenuServicesRoot;
	mi.pszService = "/ServiceDiscovery";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleServiceDiscovery);
	mi.name.a = LPGEN("Service Discovery");
	mi.position = 2000050001;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SERVICE_DISCOVERY);
	m_hMenuServiceDiscovery = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/SD/MyTransports";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleServiceDiscoveryMyTransports);
	mi.name.a = LPGEN("Registered Transports");
	mi.position = 2000050003;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_TRANSPORTL);
	m_hMenuSDMyTransports = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/SD/Transports";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleServiceDiscoveryTransports);
	mi.name.a = LPGEN("Local Server Transports");
	mi.position = 2000050004;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_TRANSPORT);
	m_hMenuSDTransports = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/SD/Conferences";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleServiceDiscoveryConferences);
	mi.name.a = LPGEN("Browse chatrooms");
	mi.position = 2000050005;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_GROUP);
	m_hMenuSDConferences = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/Groupchat";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleJoinGroupchat);
	mi.name.a = LPGEN("Create/Join group chat");
	mi.position = 2000050006;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_GROUP);
	m_hMenuGroupchat = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// "Change Password..."
	mi.pszService = "/ChangePassword";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleChangePassword);
	mi.name.a = LPGEN("Change Password");
	mi.position = 2000050007;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_KEYS);
	m_hMenuChangePassword = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// "Roster editor"
	mi.pszService = "/RosterEditor";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleRosterControl);
	mi.name.a = LPGEN("Roster editor");
	mi.position = 2000050009;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_AGENTS);
	m_hMenuRosterControl = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// "XML Console"
	mi.pszService = "/XMLConsole";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleConsole);
	mi.name.a = LPGEN("XML Console");
	mi.position = 2000050010;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_CONSOLE);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/Notes";
	CreateProtoService(mi.pszService, &CJabberProto::OnMenuHandleNotes);
	mi.name.a = LPGEN("Notes");
	mi.position = 2000050011;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_NOTES);
	m_hMenuNotes = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	BuildPrivacyMenu();
	if (m_menuItemsStatus)
		BuildPrivacyListsMenu(false);

	//////////////////////////////////////////////////////////////////////////////////////
	// build priority menu

	BuildPriorityMenu();

	//////////////////////////////////////////////////////////////////////////////////////
	// finalize status menu

	m_pepServices.RebuildMenu();
	CheckMenuItems();

	NotifyFastHook(hStatusMenuInit, (WPARAM)m_hMenuRoot, (LPARAM)(IJabberInterface*)this);
}

//////////////////////////////////////////////////////////////////////////
// priority popup in status menu

INT_PTR CJabberProto::OnMenuSetPriority(WPARAM, LPARAM, LPARAM dwDelta)
{
	int iDelta = (int)dwDelta;
	int priority = getDword("Priority", 0) + iDelta;
	if (priority > 127)
		priority = 127;
	else if (priority < -128)
		priority = -128;
	setDword("Priority", priority);
	SendPresence(m_iStatus, true);
	return 0;
}

void CJabberProto::BuildPriorityMenu()
{
	m_priorityMenuVal = 0;
	m_priorityMenuValSet = false;

	if (m_hMenuPriorityRoot)
		Menu_RemoveItem(m_hMenuPriorityRoot);

	CMenuItem mi(&g_plugin);
	mi.pszService = nullptr;
	mi.position = 200006;
	mi.root = m_hMenuRoot;
	mi.name.a = LPGEN("Resource priority");
	mi.flags = CMIF_UNMOVABLE | CMIF_HIDDEN;
	m_hMenuPriorityRoot = Menu_AddProtoMenuItem(&mi);
	Menu_ShowItem(m_hMenuPriorityRoot, m_menuItemsStatus != 0);

	wchar_t szName[128];
	char srvFce[MAX_PATH + 64];

	mi.pszService = srvFce;
	mi.name.w = szName;
	mi.position = 2000040000;
	mi.flags = CMIF_UNMOVABLE | CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
	mi.root = m_hMenuPriorityRoot;

	mir_snprintf(srvFce, "/menuSetPriority/%d", 0);
	bool needServices = !ProtoServiceExists(m_szModuleName, srvFce);
	if (needServices)
		CreateProtoServiceParam(srvFce, &CJabberProto::OnMenuSetPriority, 0);

	int steps[] = { 10, 5, 1, 0, -1, -5, -10 };
	for (int i = 0; i < _countof(steps); i++) {
		if (!steps[i]) {
			mi.position += 100000;
			continue;
		}

		mir_snprintf(srvFce, "/menuSetPriority/%d", steps[i]);
		if (steps[i] > 0) {
			mir_snwprintf(szName, TranslateT("Increase priority by %d"), steps[i]);
			mi.hIcolibItem = g_plugin.getIconHandle(IDI_ARROW_UP);
		}
		else {
			mir_snwprintf(szName, TranslateT("Decrease priority by %d"), -steps[i]);
			mi.hIcolibItem = g_plugin.getIconHandle(IDI_ARROW_DOWN);
		}

		if (needServices)
			CreateProtoServiceParam(srvFce, &CJabberProto::OnMenuSetPriority, (LPARAM)steps[i]);

		mi.position++;
		Menu_AddProtoMenuItem(&mi, m_szModuleName);
	}

	UpdatePriorityMenu(getDword("Priority", 0));
}

void CJabberProto::UpdatePriorityMenu(int priority)
{
	if (!m_hMenuPriorityRoot || (m_priorityMenuValSet && priority == m_priorityMenuVal))
		return;

	wchar_t szName[128];
	mir_snwprintf(szName, TranslateT("Resource priority [%d]"), (int)priority);
	Menu_ModifyItem(m_hMenuPriorityRoot, szName);

	m_priorityMenuVal = priority;
	m_priorityMenuValSet = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::GlobalMenuInit()
{
	//////////////////////////////////////////////////////////////////////////////////////
	// Account chooser menu

	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_UNMOVABLE | CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
	mi.position = iChooserMenuPos++;
	mi.name.w = m_tszUserName;
	m_hChooseMenuItem = Menu_AddItem(hChooserMenu, &mi, this);

	//////////////////////////////////////////////////////////////////////////////////////
	// Hotkeys

	char text[200];
	strncpy_s(text, m_szModuleName, _TRUNCATE);
	char* tDest = text + mir_strlen(text);

	HOTKEYDESC hkd = {};
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.szSection.w = m_tszUserName;
	hkd.dwFlags = HKD_UNICODE;

	mir_strcpy(tDest, "/Groupchat");
	hkd.szDescription.w = LPGENW("Join conference");
	g_plugin.addHotkey(&hkd);

	mir_strcpy(tDest, "/Bookmarks");
	hkd.szDescription.w = LPGENW("Open bookmarks");
	g_plugin.addHotkey(&hkd);

	mir_strcpy(tDest, "/PrivacyLists");
	hkd.szDescription.w = LPGENW("Privacy lists");
	g_plugin.addHotkey(&hkd);

	mir_strcpy(tDest, "/ServiceDiscovery");
	hkd.szDescription.w = LPGENW("Service discovery");
	g_plugin.addHotkey(&hkd);
}

static INT_PTR g_ToolbarHandleJoinGroupchat(WPARAM w, LPARAM l)
{
	if (CJabberProto *ppro = JabberChooseInstance())
		return ppro->OnMenuHandleJoinGroupchat(w, l);
	return 0;
}

static INT_PTR g_ToolbarHandleBookmarks(WPARAM w, LPARAM l)
{
	if (CJabberProto *ppro = JabberChooseInstance())
		return ppro->OnMenuHandleBookmarks(w, l);
	return 0;
}

static INT_PTR g_ToolbarHandleServiceDiscovery(WPARAM w, LPARAM l)
{
	if (CJabberProto *ppro = JabberChooseInstance())
		return ppro->OnMenuHandleServiceDiscovery(w, l);
	return 0;
}

int g_OnToolbarInit(WPARAM, LPARAM)
{
	if (CMPlugin::g_arInstances.getCount() == 0)
		return 0;

	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;

	CreateServiceFunction("JABBER/*/Groupchat", g_ToolbarHandleJoinGroupchat);
	ttb.pszService = "JABBER/*/Groupchat";
	ttb.pszTooltipUp = ttb.name = LPGEN("Join conference");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_GROUP);
	g_plugin.addTTB(&ttb);

	CreateServiceFunction("JABBER/*/Bookmarks", g_ToolbarHandleBookmarks);
	ttb.pszService = "JABBER/*/Bookmarks";
	ttb.pszTooltipUp = ttb.name = LPGEN("Open bookmarks");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_BOOKMARKS);
	g_plugin.addTTB(&ttb);

	CreateServiceFunction("JABBER/*/ServiceDiscovery", g_ToolbarHandleServiceDiscovery);
	ttb.pszService = "JABBER/*/ServiceDiscovery";
	ttb.pszTooltipUp = ttb.name = LPGEN("Service discovery");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_SERVICE_DISCOVERY);
	g_plugin.addTTB(&ttb);
	return 0;
}

void CJabberProto::GlobalMenuUninit()
{
	if (m_phMenuResourceItems) {
		for (int i = 0; i < m_nMenuResourceItems; i++)
			Menu_RemoveItem(m_phMenuResourceItems[i]);
		mir_free(m_phMenuResourceItems);
		m_phMenuResourceItems = nullptr;
	}
	m_nMenuResourceItems = 0;
}

void CJabberProto::EnableMenuItems(bool bEnable)
{
	m_menuItemsStatus = bEnable;
	CheckMenuItems();
}

int CJabberProto::OnLangChanged(WPARAM, LPARAM)
{
	BuildPriorityMenu();

	for (auto &it : m_pepServices)
		it->UpdateMenuView();
	return 0;
}

void CJabberProto::CheckMenuItems()
{
	bool bEnabled = (m_menuItemsStatus != 0);
	Menu_EnableItem(m_hMenuChangePassword, bEnabled);
	Menu_EnableItem(m_hMenuGroupchat, bEnabled);

	Menu_EnableItem(m_hMenuPrivacyLists, bEnabled);
	Menu_EnableItem(m_hMenuRosterControl, bEnabled);
	Menu_EnableItem(m_hMenuServiceDiscovery, bEnabled);
	Menu_EnableItem(m_hMenuSDMyTransports, bEnabled);
	Menu_EnableItem(m_hMenuSDTransports, bEnabled);
	Menu_EnableItem(m_hMenuSDConferences, bEnabled);

	bEnabled = m_ThreadInfo && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE);
	Menu_ShowItem(m_hMenuNotes, bEnabled);
	Menu_ShowItem(m_hMenuBookmarks, bEnabled);

	Menu_ShowItem(m_hPrivacyMenuRoot, m_ThreadInfo && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVACY_LISTS));

	Menu_ShowItem(m_hMenuPriorityRoot, m_menuItemsStatus != 0);

	for (auto &it : m_pepServices)
		Menu_ShowItem(it->GetMenu(), m_bPepSupported);

	JabberUpdateDialogs(m_menuItemsStatus);
}

//////////////////////////////////////////////////////////////////////////
// resource menu

void CJabberProto::MenuUpdateSrmmIcon(JABBER_LIST_ITEM *item)
{
	if (item->list != LIST_ROSTER)
		return;

	MCONTACT hContact = HContactFromJID(item->jid);
	if (hContact)
		Srmm_SetIconFlags(hContact, m_szModuleName, 0, item->arResources.getCount() ? 0 : MBF_DISABLED);
}

int CJabberProto::OnProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		if (!hDialogsList)
			hDialogsList = WindowList_Create();
		WindowList_Add(hDialogsList, event->hwndWindow, event->hContact);

		ptrA jid(getUStringA(event->hContact, "jid"));
		if (jid != nullptr) {
			JABBER_LIST_ITEM *pItem = ListGetItemPtr(LIST_ROSTER, jid);
			if (pItem && m_ThreadInfo && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_ARCHIVE_AUTO) && m_bEnableMsgArchive)
				RetrieveMessageArchive(event->hContact, pItem);
		}
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSING) {
		if (hDialogsList)
			WindowList_Remove(hDialogsList, event->hwndWindow);

		if (m_bJabberOnline) {
			char jid[JABBER_MAX_JID_LEN];
			if (GetClientJID(event->hContact, jid, _countof(jid))) {
				pResourceStatus r(ResourceInfoFromJID(jid));
				if (m_bEnableChatStates && (GetResourceCapabilities(jid, r) & JABBER_CAPS_CHATSTATES))
					m_ThreadInfo->send(XmlNode("message") << XATTR("to", jid) << XATTR("type", "chat") << XATTRID(SerialNext()) << XCHILDNS("gone", JABBER_FEAT_CHATSTATES));
			}
		}
	}

	return 0;
}

int CJabberProto::OnProcessSrmmIconClick(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (mir_strcmp(sicd->szModule, m_szModuleName))
		return 0;

	if (!hContact)
		return 0;

	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, ptrA(getUStringA(hContact, "jid")));
	if (LI == nullptr)
		return 0;

	HMENU hMenu = CreatePopupMenu();
	wchar_t buf[256];

	mir_snwprintf(buf, TranslateT("Last active (%s)"),
		LI->m_pLastSeenResource ? Utf2T(LI->m_pLastSeenResource->m_szResourceName).get() : TranslateT("No activity yet, use server's choice"));
	AppendMenu(hMenu, MF_STRING, MENUITEM_LASTSEEN, buf);

	AppendMenu(hMenu, MF_STRING, MENUITEM_SERVER, TranslateT("Highest priority (server's choice)"));

	AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
	for (int i = 0; i < LI->arResources.getCount(); i++)
		AppendMenu(hMenu, MF_STRING, MENUITEM_RESOURCES + i, Utf2T(LI->arResources[i]->m_szResourceName));

	if (LI->resourceMode == RSMODE_LASTSEEN)
		CheckMenuItem(hMenu, MENUITEM_LASTSEEN, MF_BYCOMMAND | MF_CHECKED);
	else if (LI->resourceMode == RSMODE_SERVER)
		CheckMenuItem(hMenu, MENUITEM_SERVER, MF_BYCOMMAND | MF_CHECKED);
	else if (LI->m_pManualResource)
		CheckMenuItem(hMenu, MENUITEM_RESOURCES + LI->arResources.indexOf(LI->m_pManualResource), MF_BYCOMMAND | MF_CHECKED);

	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, WindowList_Find(hDialogsList, hContact), nullptr);

	if (res == MENUITEM_LASTSEEN) {
		LI->m_pManualResource = nullptr;
		LI->resourceMode = RSMODE_LASTSEEN;
	}
	else if (res == MENUITEM_SERVER) {
		LI->m_pManualResource = nullptr;
		LI->resourceMode = RSMODE_SERVER;
	}
	else if (res >= MENUITEM_RESOURCES) {
		LI->m_pManualResource = LI->arResources[res - MENUITEM_RESOURCES];
		LI->resourceMode = RSMODE_MANUAL;
	}

	UpdateMirVer(LI);
	MenuUpdateSrmmIcon(LI);

	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleResource(WPARAM hContact, LPARAM, LPARAM res)
{
	if (!m_bJabberOnline || !hContact)
		return 0;

	ptrA tszJid(getUStringA(hContact, "jid"));
	if (tszJid == nullptr)
		return 0;

	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, tszJid);
	if (LI == nullptr)
		return 0;

	if (res == MENUITEM_LASTSEEN) {
		LI->m_pManualResource = nullptr;
		LI->resourceMode = RSMODE_LASTSEEN;
	}
	else if (res == MENUITEM_SERVER) {
		LI->m_pManualResource = nullptr;
		LI->resourceMode = RSMODE_SERVER;
	}
	else if (res >= MENUITEM_RESOURCES) {
		LI->m_pManualResource = LI->arResources[res - MENUITEM_RESOURCES];
		LI->resourceMode = RSMODE_MANUAL;
	}

	UpdateMirVer(LI);
	MenuUpdateSrmmIcon(LI);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleDirectPresence(WPARAM hContact, LPARAM, LPARAM res)
{
	if (!m_bJabberOnline || !hContact)
		return 0;

	CMStringA jid;
	ptrA szJid(getUStringA(hContact, "jid"));
	if (szJid == nullptr) {
		ptrA roomid(getUStringA(hContact, "ChatRoomID"));
		if (roomid == nullptr)
			return 0;

		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, roomid);
		if (item == nullptr)
			return 0;

		jid = MakeJid(item->jid, item->nick);
	}
	else jid = szJid;

	CMStringW szValue;
	if (EnterString(szValue, TranslateT("Status Message"), ESF_MULTILINE))
		SendPresenceTo(res, jid, nullptr, T2Utf(szValue));
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Choose protocol instance

CJabberProto* JabberChooseInstance(bool bIsLink)
{
	if (CMPlugin::g_arInstances.getCount() == 0)
		return nullptr;

	if (CMPlugin::g_arInstances.getCount() == 1) {
		if (CMPlugin::g_arInstances[0].m_iStatus != ID_STATUS_OFFLINE && CMPlugin::g_arInstances[0].m_iStatus != ID_STATUS_CONNECTING)
			return &CMPlugin::g_arInstances[0];
		
		return nullptr;
	}

	if (bIsLink)
		for (auto &it : CMPlugin::g_arInstances)
			if (it->m_bProcessXMPPLinks)
				return it;

	int nItems = 0, lastItemId = 0;
	for (auto &ppro : CMPlugin::g_arInstances) {
		if (ppro->m_iStatus != ID_STATUS_OFFLINE && ppro->m_iStatus != ID_STATUS_CONNECTING) {
			++nItems;
			lastItemId = CMPlugin::g_arInstances.indexOf(&ppro) + 1;
			Menu_ModifyItem(ppro->m_hChooseMenuItem, nullptr, Skin_GetProtoIcon(ppro->m_szModuleName, ppro->m_iStatus));
		}
		else Menu_ShowItem(ppro->m_hChooseMenuItem, false);
	}

	if (nItems > 1) {
		POINT pt;
		GetCursorPos(&pt);

		HMENU hMenu = CreatePopupMenu();
		Menu_Build(hMenu, hChooserMenu);
		int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, g_clistApi.hwndContactList, nullptr);
		DestroyMenu(hMenu);

		if (res) {
			CJabberProto* pro = nullptr;
			Menu_ProcessCommandById(res, (LPARAM)&pro);
			return pro;
		}

		return nullptr;
	}

	return lastItemId ? &CMPlugin::g_arInstances[lastItemId - 1] : nullptr;
}
