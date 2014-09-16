/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-14  Miranda NG project

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

#include "jabber.h"
#include "jabber_list.h"
#include "jabber_caps.h"
#include "jabber_privacy.h"
#include "jabber_disco.h"

#define MENUITEM_LASTSEEN	1
#define MENUITEM_SERVER		2
#define MENUITEM_RESOURCES	10

static HANDLE hDialogsList = NULL;
static HANDLE hChooserMenu, hStatusMenuInit;
static int iChooserMenuPos = 30000;

static HGENMENU g_hMenuRequestAuth;
static HGENMENU g_hMenuGrantAuth;
static HGENMENU g_hMenuRevokeAuth;
static HGENMENU g_hMenuConvert;
static HGENMENU g_hMenuRosterAdd;
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
static HGENMENU g_hMenuDirectPresence[SIZEOF(PresenceModeArray) + 1];

static INT_PTR JabberMenuChooseService(WPARAM wParam, LPARAM lParam)
{
	if (lParam)
		*(void**)lParam = (void*)wParam;
	return 0;
}

static CJabberProto* JabberGetInstanceByHContact(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return NULL;

	for (int i=0; i < g_Instances.getCount(); i++)
		if (!strcmp(szProto, g_Instances[i]->m_szModuleName))
			return g_Instances[i];

	return NULL;
}

static INT_PTR JabberMenuHandleRequestAuth(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuHandleRequestAuth(wParam, lParam) : 0;
}

static INT_PTR JabberMenuHandleGrantAuth(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuHandleGrantAuth(wParam, lParam) : 0;
}

static INT_PTR JabberMenuRevokeAuth(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuRevokeAuth(wParam, lParam) : 0;
}

static INT_PTR JabberMenuConvertChatContact(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuConvertChatContact(wParam, lParam) : 0;
}

static INT_PTR JabberMenuRosterAdd(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuRosterAdd(wParam, lParam) : 0;
}

static INT_PTR JabberMenuBookmarkAdd(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuBookmarkAdd(wParam, lParam) : 0;
}

static INT_PTR JabberMenuTransportLogin(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuTransportLogin(wParam, lParam) : 0;
}

static INT_PTR JabberMenuTransportResolve(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuTransportResolve(wParam, lParam) : 0;
}

static INT_PTR JabberContactMenuRunCommands(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->ContactMenuRunCommands(wParam, lParam) : 0;
}

static INT_PTR JabberMenuSendNote(WPARAM wParam, LPARAM lParam)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuSendNote(wParam, lParam) : 0;
}

static INT_PTR JabberMenuHandleResource(WPARAM wParam, LPARAM lParam, LPARAM lRes)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuHandleResource(wParam, lParam, lRes) : 0;
}

static INT_PTR JabberMenuHandleDirectPresence(WPARAM wParam, LPARAM lParam, LPARAM lRes)
{
	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnMenuHandleDirectPresence(wParam, lParam, lRes) : 0;
}

static int JabberPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	Menu_ShowItem(g_hMenuRequestAuth, FALSE);
	Menu_ShowItem(g_hMenuGrantAuth, FALSE);
	Menu_ShowItem(g_hMenuRevokeAuth, FALSE);
	Menu_ShowItem(g_hMenuCommands, FALSE);
	Menu_ShowItem(g_hMenuSendNote, FALSE);
	Menu_ShowItem(g_hMenuConvert, FALSE);
	Menu_ShowItem(g_hMenuRosterAdd, FALSE);
	Menu_ShowItem(g_hMenuLogin, FALSE);
	Menu_ShowItem(g_hMenuRefresh, FALSE);
	Menu_ShowItem(g_hMenuAddBookmark, FALSE);
	Menu_ShowItem(g_hMenuResourcesRoot, FALSE);
	Menu_ShowItem(g_hMenuDirectPresence[0], FALSE);

	CJabberProto *ppro = JabberGetInstanceByHContact(wParam);
	return(ppro) ? ppro->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void g_MenuInit(void)
{
	hStatusMenuInit = CreateHookableEvent(ME_JABBER_MENUINIT);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, JabberPrebuildContactMenu);

	CreateServiceFunction("Jabber/MenuChoose", JabberMenuChooseService);

	hChooserMenu = MO_CreateMenuObject("JabberAccountChooser", LPGEN("Jabber account chooser"), 0, "Jabber/MenuChoose");

	TMO_MenuItem tmi = { sizeof(tmi) };
	tmi.pszName = "Cancel";
	tmi.position = 9999999;
	tmi.hIcolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE);
	CallService(MO_ADDNEWMENUITEM, (WPARAM)hChooserMenu, (LPARAM)&tmi);

	//////////////////////////////////////////////////////////////////////////////////////
	// Contact menu initialization

	CLISTMENUITEM mi = { sizeof(mi) };

	// "Request authorization"
	mi.pszName = LPGEN("Request authorization");
	mi.position = -2000001000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	mi.pszService = "Jabber/ReqAuth";
	g_hMenuRequestAuth = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuHandleRequestAuth);

	// "Grant authorization"
	mi.pszService = "Jabber/GrantAuth";
	mi.pszName = LPGEN("Grant authorization");
	mi.position = -2000001001;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	g_hMenuGrantAuth = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuHandleGrantAuth);

	// Revoke auth
	mi.pszService = "Jabber/RevokeAuth";
	mi.pszName = LPGEN("Revoke authorization");
	mi.position = -2000001002;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	g_hMenuRevokeAuth = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuRevokeAuth);

	// "Convert Chat/Contact"
	mi.pszService = "Jabber/ConvertChatContact";
	mi.pszName = LPGEN("Convert");
	mi.position = -1999901004;
	mi.icolibItem = g_GetIconHandle(IDI_USER2ROOM);
	g_hMenuConvert = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuConvertChatContact);

	// "Add to roster"
	mi.pszService = "Jabber/AddToRoster";
	mi.pszName = LPGEN("Add to roster");
	mi.position = -1999901005;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_ADD);
	g_hMenuRosterAdd = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuRosterAdd);

	// "Add to Bookmarks"
	mi.pszService = "Jabber/AddToBookmarks";
	mi.pszName = LPGEN("Add to Bookmarks");
	mi.position = -1999901006;
	mi.icolibItem = g_GetIconHandle(IDI_BOOKMARKS);
	g_hMenuAddBookmark = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuBookmarkAdd);

	// Login/logout
	mi.pszService = "Jabber/TransportLogin";
	mi.pszName = LPGEN("Login/logout");
	mi.position = -1999901007;
	mi.icolibItem = g_GetIconHandle(IDI_LOGIN);
	g_hMenuLogin = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuTransportLogin);

	// Retrieve nicks
	mi.pszService = "Jabber/TransportGetNicks";
	mi.pszName = LPGEN("Resolve nicks");
	mi.position = -1999901008;
	mi.icolibItem = g_GetIconHandle(IDI_REFRESH);
	g_hMenuRefresh = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuTransportResolve);

	// Run Commands
	mi.pszService = "Jabber/RunCommands";
	mi.pszName = LPGEN("Commands");
	mi.position = -1999901009;
	mi.icolibItem = g_GetIconHandle(IDI_COMMAND);
	g_hMenuCommands = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberContactMenuRunCommands);

	// Send Note
	mi.pszService = "Jabber/SendNote";
	mi.pszName = LPGEN("Send Note");
	mi.position = -1999901010;
	mi.icolibItem = g_GetIconHandle(IDI_SEND_NOTE);
	g_hMenuSendNote = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JabberMenuSendNote);

	// Direct Presence
	mi.pszService = "Jabber/DirectPresenceDummySvc";
	mi.pszName = LPGEN("Send Presence");
	mi.position = -1999901011;
	mi.pszPopupName = (char *)-1;
	mi.icolibItem = g_GetIconHandle(IDI_NOTES);
	g_hMenuDirectPresence[0] = Menu_AddContactMenuItem(&mi);

	mi.flags |= CMIF_ROOTHANDLE | CMIF_TCHAR;
	for (int i=0; i < SIZEOF(PresenceModeArray); i++) {
		char buf[] = "Jabber/DirectPresenceX";
		buf[SIZEOF(buf)-2] = '0' + i;
		mi.pszService = buf;
		mi.ptszName = pcli->pfnGetStatusModeDescription(PresenceModeArray[i].mode, 0);
		mi.position = -1999901000;
		mi.hParentMenu = g_hMenuDirectPresence[0];
		mi.icolibItem = LoadSkinnedIcon(PresenceModeArray[i].icon);
		g_hMenuDirectPresence[i+1] = Menu_AddContactMenuItem(&mi);
		CreateServiceFunctionParam(mi.pszService, JabberMenuHandleDirectPresence, PresenceModeArray[i].mode);
	}

	mi.flags &= ~(CMIF_ROOTHANDLE | CMIF_TCHAR);

	// Resource selector
	mi.pszService = "Jabber/ResourceSelectorDummySvc";
	mi.pszName = LPGEN("Jabber Resource");
	mi.position = -1999901011;
	mi.hParentMenu = HGENMENU_ROOT;
	mi.icolibItem = g_GetIconHandle(IDI_JABBER);
	g_hMenuResourcesRoot = Menu_AddContactMenuItem(&mi);

	mi.pszService = "Jabber/UseResource_last";
	mi.pszName = LPGEN("Last Active");
	mi.position = -1999901000;
	mi.hParentMenu = g_hMenuResourcesRoot;
	mi.icolibItem = g_GetIconHandle(IDI_JABBER);
	mi.flags |= CMIF_ROOTHANDLE;
	g_hMenuResourcesActive = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionParam(mi.pszService, JabberMenuHandleResource, MENUITEM_LASTSEEN);

	mi.pszService = "Jabber/UseResource_server";
	mi.pszName = LPGEN("Server's Choice");
	mi.position = -1999901000;
	mi.icolibItem = g_GetIconHandle(IDI_NODE_SERVER);
	g_hMenuResourcesServer = Menu_AddContactMenuItem(&mi);
	CreateServiceFunctionParam(mi.pszService, JabberMenuHandleResource, MENUITEM_SERVER);
}

void g_MenuUninit(void)
{
	DestroyHookableEvent(hStatusMenuInit);

	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuRequestAuth, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuGrantAuth, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuRevokeAuth, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuConvert, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuRosterAdd, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuLogin, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuRefresh, 0);
	CallService(MO_REMOVEMENUITEM, (WPARAM)g_hMenuAddBookmark, 0);

	WindowList_Destroy(hDialogsList);
}

/////////////////////////////////////////////////////////////////////////////////////////
// contact menu services

int CJabberProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		return 0;

	bool bIsChatRoom  = isChatRoom(hContact);
	bool bIsTransport = getBool(hContact, "IsTransport", false);

	if ((bIsChatRoom == GCW_CHATROOM) || bIsChatRoom == 0) {
		if ( ptrT( getTStringA(hContact, bIsChatRoom?(char*)"ChatRoomID":(char*)"jid")) != NULL) {
			Menu_ShowItem(g_hMenuConvert, TRUE);

			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.pszName = bIsChatRoom ? (char *)LPGEN("&Convert to Contact") : (char *)LPGEN("&Convert to Chat Room");
			clmi.flags = CMIM_NAME | CMIM_FLAGS;
			Menu_ModifyItem(g_hMenuConvert, &clmi);
		}
	}

	if (!m_bJabberOnline)
		return 0;

	Menu_ShowItem(g_hMenuDirectPresence[0], TRUE);
	for (int i=0; i < SIZEOF(PresenceModeArray); i++) {
		CLISTMENUITEM clmi = { sizeof(clmi) };
		clmi.flags = CMIM_ICON | CMIM_FLAGS;
		clmi.hIcon = (HICON)LoadSkinnedProtoIcon(m_szModuleName, PresenceModeArray[i].mode);
		Menu_ModifyItem(g_hMenuDirectPresence[i+1], &clmi);
	}

	if (bIsChatRoom) {
		ptrT roomid( getTStringA(hContact, "ChatRoomID"));
		if (roomid != NULL) {
			Menu_ShowItem(g_hMenuRosterAdd, FALSE);

			if (ListGetItemPtr(LIST_BOOKMARK, roomid) == NULL)
				if (m_ThreadInfo && m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE)
					Menu_ShowItem(g_hMenuAddBookmark, TRUE);
		}
	}

	if (bIsChatRoom == GCW_CHATROOM)
		return 0;

	if (bIsTransport) {
		Menu_ShowItem(g_hMenuLogin, TRUE);
		Menu_ShowItem(g_hMenuRefresh, TRUE);
	}

	ptrT jid( getTStringA(hContact, "jid"));
	if (jid == NULL)
		return 0;

	JabberCapsBits jcb = GetTotalJidCapabilites(jid);
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == NULL)
		return 0;

	BOOL bCtrlPressed = (GetKeyState(VK_CONTROL)&0x8000) != 0;
	Menu_ShowItem(g_hMenuRequestAuth, item->subscription == SUB_FROM || item->subscription == SUB_NONE || bCtrlPressed);
	Menu_ShowItem(g_hMenuGrantAuth, bCtrlPressed);
	Menu_ShowItem(g_hMenuRevokeAuth, item->subscription == SUB_FROM || item->subscription == SUB_BOTH || bCtrlPressed);
	Menu_ShowItem(g_hMenuCommands, ((jcb & JABBER_CAPS_COMMANDS) != 0) || bCtrlPressed);
	Menu_ShowItem(g_hMenuSendNote, TRUE);

	if (item->arResources.getCount() == 0)
		return 0;

	Menu_ShowItem(g_hMenuResourcesRoot, TRUE);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON | CMIM_FLAGS;
	mi.icolibItem = m_hProtoIcon;
	Menu_ModifyItem(g_hMenuResourcesRoot, &mi);

	mi.flags = CMIM_ICON | CMIM_FLAGS | ((item->resourceMode == RSMODE_LASTSEEN) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(g_hMenuResourcesActive, &mi);

	mi.flags = CMIM_FLAGS | ((item->resourceMode == RSMODE_SERVER) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(g_hMenuResourcesServer, &mi);

	int nMenuResourceItemsNew = m_nMenuResourceItems;
	if (m_nMenuResourceItems < item->arResources.getCount()) {
		m_phMenuResourceItems = (HGENMENU*)mir_realloc(m_phMenuResourceItems, item->arResources.getCount() * sizeof(HGENMENU));
		nMenuResourceItemsNew = item->arResources.getCount();
	}

	char text[ 256 ];
	strcpy(text, m_szModuleName);
	size_t nModuleNameLength = strlen(text);
	char* tDest = text + nModuleNameLength;

	mi.flags = CMIF_CHILDPOPUP;
	mi.position = 0;
	mi.icolibItem = NULL;
	mi.pszService = text;
	mi.pszContactOwner = m_szModuleName;

	CMString szTmp;
	for (int i=0; i < nMenuResourceItemsNew; i++) {
		mir_snprintf(tDest, SIZEOF(text) - nModuleNameLength, "/UseResource_%d", i);
		if (i >= m_nMenuResourceItems) {
			CreateProtoServiceParam(tDest, &CJabberProto::OnMenuHandleResource, MENUITEM_RESOURCES+i);
			mi.pszName = "";
			mi.position = i;
			mi.hParentMenu = g_hMenuResourcesRoot;
			m_phMenuResourceItems[i] = Menu_AddContactMenuItem(&mi);
		}
		if (i < item->arResources.getCount()) {
			pResourceStatus r(item->arResources[i]);
			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.flags = CMIM_NAME | CMIM_FLAGS | CMIF_CHILDPOPUP | CMIF_TCHAR;
			if (item->resourceMode == RSMODE_MANUAL && item->m_pManualResource == r)
				clmi.flags |= CMIF_CHECKED;
			if (ServiceExists(MS_FP_GETCLIENTICONT)) {
				clmi.flags |= CMIM_ICON;
				FormatMirVer(r, szTmp);
				clmi.hIcon = Finger_GetClientIcon(szTmp, 0);
			}
			szTmp.Format(_T("%s [%s, %d]"), r->m_tszResourceName, pcli->pfnGetStatusModeDescription(r->m_iStatus, 0), r->m_iPriority);
			clmi.ptszName = szTmp.GetBuffer();
			Menu_ModifyItem(m_phMenuResourceItems[i], &clmi);
			DestroyIcon(clmi.hIcon);
		}
		else Menu_ShowItem(m_phMenuResourceItems[i], FALSE);
	}

	m_nMenuResourceItems = nMenuResourceItemsNew;
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuConvertChatContact(WPARAM hContact, LPARAM)
{
	BYTE bIsChatRoom = isChatRoom(hContact);
	const char *szSetting = (bIsChatRoom) ? "ChatRoomID" : "jid";

	ptrT jid(getTStringA(hContact, szSetting));
	if (jid != NULL) {
		delSetting(hContact, szSetting);
		setTString(hContact, szSetting, jid);
		setByte(hContact, "ChatRoom", !bIsChatRoom);
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuRosterAdd(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0; // we do not add ourself to the roster. (buggy situation - should not happen)

	ptrT roomID(getTStringA(hContact, "ChatRoomID"));
	if (roomID == NULL)
		return 0;

	if (ListGetItemPtr(LIST_ROSTER, roomID) == NULL) {
		ptrT group(db_get_tsa(hContact, "CList", "Group"));
		ptrT nick(getTStringA(hContact, "Nick"));

		AddContactToRoster(roomID, nick, group);
		if (m_options.AddRoster2Bookmarks == TRUE) {
			JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_BOOKMARK, roomID);
			if (item == NULL) {
				item = new JABBER_LIST_ITEM();
				item->jid = mir_tstrdup(roomID);
				item->name = mir_tstrdup(nick);
				item->nick = getTStringA(hContact, "MyNick");
				AddEditBookmark(item);
				delete item;
			}
		}
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleRequestAuth(WPARAM hContact, LPARAM)
{
	if (hContact != NULL && m_bJabberOnline) {
		ptrT jid(getTStringA(hContact, "jid"));
		if (jid != NULL)
			m_ThreadInfo->send(XmlNode(_T("presence")) << XATTR(_T("to"), jid) << XATTR(_T("type"), _T("subscribe")));
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuHandleGrantAuth(WPARAM hContact, LPARAM)
{
	if (hContact != NULL && m_bJabberOnline) {
		ptrT jid(getTStringA(hContact, "jid"));
		if (jid != NULL)
			m_ThreadInfo->send(XmlNode(_T("presence")) << XATTR(_T("to"), jid) << XATTR(_T("type"), _T("subscribed")));
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuRevokeAuth(WPARAM hContact, LPARAM)
{
	if (hContact != NULL && m_bJabberOnline) {
		ptrT jid(getTStringA(hContact, "jid"));
		if (jid != NULL)
			m_ThreadInfo->send(XmlNode(_T("presence")) << XATTR(_T("to"), jid) << XATTR(_T("type"), _T("unsubscribed")));
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuTransportLogin(WPARAM hContact, LPARAM)
{
	if (!getByte(hContact, "IsTransport", 0))
		return 0;

	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, ptrT(getTStringA(hContact, "jid")));
	if (item != NULL) {
		XmlNode p(_T("presence")); xmlAddAttr(p, _T("to"), item->jid);
		if (item->getTemp()->m_iStatus == ID_STATUS_ONLINE)
			xmlAddAttr(p, _T("type"), _T("unavailable"));
		m_ThreadInfo->send(p);
	}
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuTransportResolve(WPARAM hContact, LPARAM)
{
	if (!getByte(hContact, "IsTransport", 0))
		return 0;

	ptrT jid(getTStringA(hContact, "jid"));
	if (jid != NULL)
		ResolveTransportNicks(jid);
	return 0;
}

INT_PTR __cdecl CJabberProto::OnMenuBookmarkAdd(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0; // we do not add ourself to the roster. (buggy situation - should not happen)

	ptrT roomID(getTStringA(hContact, "ChatRoomID"));
	if (roomID == NULL)
		return 0;

	if (ListGetItemPtr(LIST_BOOKMARK, roomID) == NULL) {
		JABBER_LIST_ITEM *item = new JABBER_LIST_ITEM();
		item->jid = mir_tstrdup(roomID);
		item->name = pcli->pfnGetContactDisplayName(hContact, 0);
		item->type = _T("conference");
		item->nick = getTStringA(hContact, "MyNick");
		AddEditBookmark(item);
		delete item;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// status menu

void CJabberProto::MenuInit()
{
	char text[200];
	strcpy(text, m_szModuleName);
	char* tDest = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hJabberRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hJabberRoot == NULL) {
		mi.ptszName = m_tszUserName;
		mi.position = -1999901006;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = m_hProtoIcon;
		hJabberRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else {
		if (m_hMenuRoot)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	// "Bookmarks..."
	CreateProtoService("/Bookmarks", &CJabberProto::OnMenuHandleBookmarks);
	strcpy(tDest, "/Bookmarks");
	mi.flags = CMIF_CHILDPOPUP;
	mi.hParentMenu = hJabberRoot;
	mi.pszName = LPGEN("Bookmarks");
	mi.position = 200001;
	mi.icolibItem = GetIconHandle(IDI_BOOKMARKS);
	m_hMenuBookmarks = Menu_AddProtoMenuItem(&mi);

	// "Options..."
	CreateProtoService("/Options", &CJabberProto::OnMenuOptions);
	strcpy(tDest, "/Options");
	mi.pszName = LPGEN("Options...");
	mi.position = 200002;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS);
	Menu_AddProtoMenuItem(&mi);

	// "Services..."
	mi.pszName = LPGEN("Services...");
	strcpy(tDest, "/Services");
	mi.position = 200003;
	mi.icolibItem = GetIconHandle(IDI_SERVICE_DISCOVERY);
	HGENMENU hMenuServicesRoot = Menu_AddProtoMenuItem(&mi);

	// "Service Discovery..."
	CreateProtoService("/ServiceDiscovery", &CJabberProto::OnMenuHandleServiceDiscovery);
	strcpy(tDest, "/ServiceDiscovery");
	mi.flags = CMIF_ROOTHANDLE;
	mi.pszName = LPGEN("Service Discovery");
	mi.position = 2000050001;
	mi.icolibItem = GetIconHandle(IDI_SERVICE_DISCOVERY);
	mi.hParentMenu = hMenuServicesRoot;
	m_hMenuServiceDiscovery = Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/SD/MyTransports", &CJabberProto::OnMenuHandleServiceDiscoveryMyTransports);
	strcpy(tDest, "/SD/MyTransports");
	mi.pszName = LPGEN("Registered Transports");
	mi.position = 2000050003;
	mi.icolibItem = GetIconHandle(IDI_TRANSPORTL);
	m_hMenuSDMyTransports = Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/SD/Transports", &CJabberProto::OnMenuHandleServiceDiscoveryTransports);
	strcpy(tDest, "/SD/Transports");
	mi.pszName = LPGEN("Local Server Transports");
	mi.position = 2000050004;
	mi.icolibItem = GetIconHandle(IDI_TRANSPORT);
	m_hMenuSDTransports = Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/SD/Conferences", &CJabberProto::OnMenuHandleServiceDiscoveryConferences);
	strcpy(tDest, "/SD/Conferences");
	mi.pszName = LPGEN("Browse chatrooms");
	mi.position = 2000050005;
	mi.icolibItem = GetIconHandle(IDI_GROUP);
	m_hMenuSDConferences = Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/Groupchat", &CJabberProto::OnMenuHandleJoinGroupchat);
	strcpy(tDest, "/Groupchat");
	mi.pszName = LPGEN("Create/Join group chat");
	mi.position = 2000050006;
	mi.icolibItem = GetIconHandle(IDI_GROUP);
	m_hMenuGroupchat = Menu_AddProtoMenuItem(&mi);

	// "Change Password..."
	CreateProtoService("/ChangePassword", &CJabberProto::OnMenuHandleChangePassword);
	strcpy(tDest, "/ChangePassword");
	mi.pszName = LPGEN("Change Password");
	mi.position = 2000050007;
	mi.icolibItem = GetIconHandle(IDI_KEYS);
	m_hMenuChangePassword = Menu_AddProtoMenuItem(&mi);

	// "Roster editor"
	CreateProtoService("/RosterEditor", &CJabberProto::OnMenuHandleRosterControl);
	strcpy(tDest, "/RosterEditor");
	mi.pszName = LPGEN("Roster editor");
	mi.position = 2000050009;
	mi.icolibItem = GetIconHandle(IDI_AGENTS);
	m_hMenuRosterControl = Menu_AddProtoMenuItem(&mi);

	// "XML Console"
	CreateProtoService("/XMLConsole", &CJabberProto::OnMenuHandleConsole);
	strcpy(tDest, "/XMLConsole");
	mi.pszName = LPGEN("XML Console");
	mi.position = 2000050010;
	mi.icolibItem = GetIconHandle(IDI_CONSOLE);
	Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/Notes", &CJabberProto::OnMenuHandleNotes);
	strcpy(tDest, "/Notes");
	mi.pszName = LPGEN("Notes");
	mi.position = 2000050011;
	mi.icolibItem = GetIconHandle(IDI_NOTES);
	m_hMenuNotes = Menu_AddProtoMenuItem(&mi);

	BuildPrivacyMenu();
	if (m_menuItemsStatus)
		BuildPrivacyListsMenu(false);

	//////////////////////////////////////////////////////////////////////////////////////
	// build priority menu

	m_priorityMenuVal = 0;
	m_priorityMenuValSet = false;

	mi.position = 200006;
	mi.pszContactOwner = m_szModuleName;
	mi.hParentMenu = hJabberRoot;
	mi.pszName = LPGEN("Resource priority");
	mi.flags = CMIF_ROOTPOPUP | CMIF_HIDDEN;
	m_hMenuPriorityRoot = Menu_AddProtoMenuItem(&mi);

	TCHAR szName[128];
	char srvFce[MAX_PATH + 64], *svcName = srvFce + strlen(m_szModuleName);
	mi.pszService = srvFce;
	mi.ptszName = szName;
	mi.position = 2000040000;
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
	mi.hParentMenu = m_hMenuPriorityRoot;

	mir_snprintf(srvFce, SIZEOF(srvFce), "%s/menuSetPriority/0", m_szModuleName);
	bool needServices = !ServiceExists(srvFce);
	if (needServices)
		CreateProtoServiceParam(svcName, &CJabberProto::OnMenuSetPriority, 0);

	int steps[] = { 10, 5, 1, 0, -1, -5, -10 };
	for (int i = 0; i < SIZEOF(steps); i++) {
		if (!steps[i]) {
			mi.position += 100000;
			continue;
		}

		mir_snprintf(srvFce, SIZEOF(srvFce), "%s/menuSetPriority/%d", m_szModuleName, steps[i]);
		if (steps[i] > 0) {
			mir_sntprintf(szName, SIZEOF(szName), TranslateT("Increase priority by %d"), steps[i]);
			mi.icolibItem = GetIconHandle(IDI_ARROW_UP);
		}
		else {
			mir_sntprintf(szName, SIZEOF(szName), TranslateT("Decrease priority by %d"), -steps[i]);
			mi.icolibItem = GetIconHandle(IDI_ARROW_DOWN);
		}

		if (needServices)
			CreateProtoServiceParam(svcName, &CJabberProto::OnMenuSetPriority, (LPARAM)steps[i]);

		mi.position++;
		Menu_AddProtoMenuItem(&mi);
	}

	UpdatePriorityMenu((short)getWord("Priority", 0));

	//////////////////////////////////////////////////////////////////////////////////////
	// finalize status menu

	m_pepServices.RebuildMenu();
	CheckMenuItems();

	NotifyFastHook(hStatusMenuInit, (WPARAM)hJabberRoot, (LPARAM)(IJabberInterface*)this);
}

//////////////////////////////////////////////////////////////////////////
// priority popup in status menu

INT_PTR CJabberProto::OnMenuSetPriority(WPARAM, LPARAM, LPARAM dwDelta)
{
	int iDelta = (int)dwDelta;
	int priority = getWord("Priority", 0) + iDelta;
	if (priority > 127)
		priority = 127;
	else if (priority < -128)
		priority = -128;
	setWord("Priority", priority);
	SendPresence(m_iStatus, true);
	return 0;
}

void CJabberProto::UpdatePriorityMenu(short priority)
{
	if (!m_hMenuPriorityRoot || m_priorityMenuValSet && (priority == m_priorityMenuVal))
		return;

	TCHAR szName[128];
	mir_sntprintf(szName, SIZEOF(szName), TranslateT("Resource priority [%d]"), (int)priority);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_TCHAR | CMIM_NAME | CMIF_KEEPUNTRANSLATED;
	mi.ptszName = szName;
	Menu_ModifyItem(m_hMenuPriorityRoot, &mi);

	m_priorityMenuVal = priority;
	m_priorityMenuValSet = true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CJabberProto::GlobalMenuInit()
{
	//////////////////////////////////////////////////////////////////////////////////////
	// Account chooser menu

	TMO_MenuItem tmi = { sizeof(tmi) };
	tmi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
	tmi.ownerdata = this;
	tmi.position = iChooserMenuPos++;
	tmi.ptszName = m_tszUserName;
	m_hChooseMenuItem = (HGENMENU)CallService(MO_ADDNEWMENUITEM, (WPARAM)hChooserMenu, (LPARAM)&tmi);

	//////////////////////////////////////////////////////////////////////////////////////
	// Hotkeys

	char text[200];
	strcpy(text, m_szModuleName);
	char* tDest = text + strlen(text);

	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszName = text;
	hkd.pszService = text;
	hkd.ptszSection = m_tszUserName;
	hkd.dwFlags = HKD_TCHAR;

	strcpy(tDest, "/Groupchat");
	hkd.ptszDescription = LPGENT("Join conference");
	Hotkey_Register(&hkd);

	strcpy(tDest, "/Bookmarks");
	hkd.ptszDescription = LPGENT("Open bookmarks");
	Hotkey_Register(&hkd);

	strcpy(tDest, "/PrivacyLists");
	hkd.ptszDescription = LPGENT("Privacy lists");
	Hotkey_Register(&hkd);

	strcpy(tDest, "/ServiceDiscovery");
	hkd.ptszDescription = LPGENT("Service discovery");
	Hotkey_Register(&hkd);
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
	if (g_Instances.getCount() == 0)
		return 0;

	TTBButton ttb = { sizeof(ttb) };
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;

	CreateServiceFunction("JABBER/*/Groupchat", g_ToolbarHandleJoinGroupchat);
	ttb.pszService = "JABBER/*/Groupchat";
	ttb.pszTooltipUp = ttb.name = LPGEN("Join conference");
	ttb.hIconHandleUp = g_GetIconHandle(IDI_GROUP);
	TopToolbar_AddButton(&ttb);

	CreateServiceFunction("JABBER/*/Bookmarks", g_ToolbarHandleBookmarks);
	ttb.pszService = "JABBER/*/Bookmarks";
	ttb.pszTooltipUp = ttb.name = LPGEN("Open bookmarks");
	ttb.hIconHandleUp = g_GetIconHandle(IDI_BOOKMARKS);
	TopToolbar_AddButton(&ttb);

	CreateServiceFunction("JABBER/*/ServiceDiscovery", g_ToolbarHandleServiceDiscovery);
	ttb.pszService = "JABBER/*/ServiceDiscovery";
	ttb.pszTooltipUp = ttb.name = LPGEN("Service discovery");
	ttb.hIconHandleUp = g_GetIconHandle(IDI_SERVICE_DISCOVERY);
	TopToolbar_AddButton(&ttb);
	return 0;
}

void CJabberProto::GlobalMenuUninit()
{
	if (m_phMenuResourceItems) {
		for (int i=0; i < m_nMenuResourceItems; i++)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_phMenuResourceItems[i], 0);
		mir_free(m_phMenuResourceItems);
		m_phMenuResourceItems = NULL;
	}
	m_nMenuResourceItems = 0;

	if (m_hMenuRoot)
		CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
	m_hMenuRoot = NULL;
}

void CJabberProto::EnableMenuItems(BOOL bEnable)
{
	m_menuItemsStatus = bEnable;
	CheckMenuItems();
}

void CJabberProto::CheckMenuItems()
{
	CLISTMENUITEM clmi = { sizeof(clmi) };
	clmi.flags = CMIM_FLAGS;
	if (!m_menuItemsStatus)
		clmi.flags |= CMIF_GRAYED;

	Menu_ModifyItem(m_hMenuChangePassword, &clmi);
	Menu_ModifyItem(m_hMenuGroupchat, &clmi);

	Menu_ModifyItem(m_hMenuPrivacyLists, &clmi);
	Menu_ModifyItem(m_hMenuRosterControl, &clmi);
	Menu_ModifyItem(m_hMenuServiceDiscovery, &clmi);
	Menu_ModifyItem(m_hMenuSDMyTransports, &clmi);
	Menu_ModifyItem(m_hMenuSDTransports, &clmi);
	Menu_ModifyItem(m_hMenuSDConferences, &clmi);

	bool bEnabled = m_ThreadInfo && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVATE_STORAGE);
	Menu_ShowItem(m_hMenuNotes, bEnabled);
	Menu_ShowItem(m_hMenuBookmarks, bEnabled);

	Menu_ShowItem(m_hPrivacyMenuRoot, m_ThreadInfo && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_PRIVACY_LISTS));

	Menu_ShowItem(m_hMenuPriorityRoot, m_menuItemsStatus != 0);

	for (int i=0; i < m_pepServices.getCount(); i++)
		Menu_ShowItem(m_pepServices[i].GetMenu(), m_bPepSupported);

	JabberUpdateDialogs(m_menuItemsStatus);
}

//////////////////////////////////////////////////////////////////////////
// resource menu

void CJabberProto::MenuUpdateSrmmIcon(JABBER_LIST_ITEM *item)
{
	if (item->list != LIST_ROSTER)
		return;

	MCONTACT hContact = HContactFromJID(item->jid);
	if (!hContact)
		return;

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = m_szModuleName;
	sid.flags = item->arResources.getCount() ? 0 : MBF_DISABLED;
	Srmm_ModifyIcon(hContact, &sid);
}

int CJabberProto::OnProcessSrmmEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *event = (MessageWindowEventData *)lParam;

	if (event->uType == MSG_WINDOW_EVT_OPEN) {
		if (!hDialogsList)
			hDialogsList = WindowList_Create();
		WindowList_Add(hDialogsList, event->hwndWindow, event->hContact);

		ptrT jid(getTStringA(event->hContact, "jid"));
		if (jid != NULL) {
			JABBER_LIST_ITEM *pItem = ListGetItemPtr(LIST_ROSTER, jid);
			if (pItem && (m_ThreadInfo->jabberServerCaps & JABBER_CAPS_ARCHIVE_AUTO) && m_options.EnableMsgArchive)
				RetrieveMessageArchive(event->hContact, pItem);
		}
	}
	else if (event->uType == MSG_WINDOW_EVT_CLOSING) {
		if (hDialogsList)
			WindowList_Remove(hDialogsList, event->hwndWindow);

		DBVARIANT dbv;
		BOOL bSupportTyping = FALSE;
		if (!db_get(event->hContact, "SRMsg", "SupportTyping", &dbv)) {
			bSupportTyping = dbv.bVal == 1;
			db_free(&dbv);
		}
		else if (!db_get(NULL, "SRMsg", "DefaultTyping", &dbv)) {
			bSupportTyping = dbv.bVal == 1;
			db_free(&dbv);
		}
		if (!bSupportTyping || !m_bJabberOnline)
			return 0;

		TCHAR jid[JABBER_MAX_JID_LEN];
		if (GetClientJID(event->hContact, jid, SIZEOF(jid))) {
			pResourceStatus r(ResourceInfoFromJID(jid));
			if (r && r->m_bMessageSessionActive) {
				r->m_bMessageSessionActive = FALSE;

				if (GetResourceCapabilites(jid, TRUE) & JABBER_CAPS_CHATSTATES)
					m_ThreadInfo->send(
					XmlNode(_T("message")) << XATTR(_T("to"), jid) << XATTR(_T("type"), _T("chat")) << XATTRID(SerialNext())
					<< XCHILDNS(_T("gone"), JABBER_FEAT_CHATSTATES));
			}
		}
	}

	return 0;
}

int CJabberProto::OnProcessSrmmIconClick(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (lstrcmpA(sicd->szModule, m_szModuleName))
		return 0;

	if (!hContact)
		return 0;

	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, ptrT(getTStringA(hContact, "jid")));
	if (LI == NULL)
		return 0;

	HMENU hMenu = CreatePopupMenu();
	TCHAR buf[256];

	mir_sntprintf(buf, SIZEOF(buf), TranslateT("Last active (%s)"),
		LI->m_pLastSeenResource ? LI->m_pLastSeenResource->m_tszResourceName : TranslateT("No activity yet, use server's choice"));
	AppendMenu(hMenu, MF_STRING, MENUITEM_LASTSEEN, buf);

	AppendMenu(hMenu, MF_STRING, MENUITEM_SERVER, TranslateT("Highest priority (server's choice)"));

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	for (int i = 0; i < LI->arResources.getCount(); i++)
		AppendMenu(hMenu, MF_STRING, MENUITEM_RESOURCES + i, LI->arResources[i]->m_tszResourceName);

	if (LI->resourceMode == RSMODE_LASTSEEN)
		CheckMenuItem(hMenu, MENUITEM_LASTSEEN, MF_BYCOMMAND | MF_CHECKED);
	else if (LI->resourceMode == RSMODE_SERVER)
		CheckMenuItem(hMenu, MENUITEM_SERVER, MF_BYCOMMAND | MF_CHECKED);
	else if (LI->m_pManualResource)
		CheckMenuItem(hMenu, MENUITEM_RESOURCES + LI->arResources.indexOf(LI->m_pManualResource), MF_BYCOMMAND | MF_CHECKED);

	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, WindowList_Find(hDialogsList, hContact), NULL);

	if (res == MENUITEM_LASTSEEN) {
		LI->m_pManualResource = NULL;
		LI->resourceMode = RSMODE_LASTSEEN;
	}
	else if (res == MENUITEM_SERVER) {
		LI->m_pManualResource = NULL;
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

	ptrT tszJid(getTStringA(hContact, "jid"));
	if (tszJid == NULL)
		return 0;

	JABBER_LIST_ITEM *LI = ListGetItemPtr(LIST_ROSTER, tszJid);
	if (LI == NULL)
		return 0;

	if (res == MENUITEM_LASTSEEN) {
		LI->m_pManualResource = NULL;
		LI->resourceMode = RSMODE_LASTSEEN;
	}
	else if (res == MENUITEM_SERVER) {
		LI->m_pManualResource = NULL;
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

INT_PTR __cdecl CJabberProto::OnMenuHandleDirectPresence(WPARAM hContact, LPARAM lParam, LPARAM res)
{
	if (!m_bJabberOnline || !hContact)
		return 0;

	TCHAR *jid, text[1024];
	ptrT tszJid(getTStringA(hContact, "jid"));
	if (tszJid == NULL) {
		ptrT roomid(getTStringA(hContact, "ChatRoomID"));
		if (roomid == NULL)
			return 0;

		JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_CHATROOM, roomid);
		if (item == NULL)
			return 0;

		mir_sntprintf(text, SIZEOF(text), _T("%s/%s"), item->jid, item->nick);
		jid = text;
	}
	else jid = tszJid;

	CMString szValue;
	if (EnterString(szValue, TranslateT("Status Message"), ESF_MULTILINE))
		SendPresenceTo(res, jid, NULL, szValue);
	return 0;
}

////////////////////////////////////////////////////////////////////////
// Choose protocol instance

CJabberProto* JabberChooseInstance(bool bIsLink)
{
	if (g_Instances.getCount() == 0)
		return NULL;

	if (g_Instances.getCount() == 1) {
		if (g_Instances[0]->m_iStatus != ID_STATUS_OFFLINE && g_Instances[0]->m_iStatus != ID_STATUS_CONNECTING)
			return g_Instances[0];
		return NULL;
	}

	if (bIsLink)
		for (int i = 0; i < g_Instances.getCount(); i++)
			if (g_Instances[i]->m_options.ProcessXMPPLinks)
				return g_Instances[i];

	CLISTMENUITEM clmi = { sizeof(clmi) };

	int nItems = 0, lastItemId = 0;
	for (int i = 0; i < g_Instances.getCount(); i++) {
		clmi.flags = CMIM_FLAGS;

		CJabberProto *ppro = g_Instances[i];
		if (ppro->m_iStatus != ID_STATUS_OFFLINE && ppro->m_iStatus != ID_STATUS_CONNECTING) {
			++nItems;
			lastItemId = i + 1;
			clmi.flags |= CMIM_ICON;
			clmi.hIcon = LoadSkinnedProtoIcon(ppro->m_szModuleName, ppro->m_iStatus);
		}
		else clmi.flags |= CMIF_HIDDEN;

		Menu_ModifyItem(ppro->m_hChooseMenuItem, &clmi);
	}

	if (nItems > 1) {
		ListParam param = { 0 };
		param.MenuObjectHandle = hChooserMenu;
		HMENU hMenu = CreatePopupMenu();
		CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);

		POINT pt;
		GetCursorPos(&pt);

		int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, pcli->hwndContactList, NULL);
		DestroyMenu(hMenu);

		if (res) {
			CJabberProto* pro = NULL;
			CallService(MO_PROCESSCOMMANDBYMENUIDENT, res, (LPARAM)&pro);
			return pro;
		}

		return NULL;
	}

	return lastItemId ? g_Instances[lastItemId - 1] : NULL;
}
