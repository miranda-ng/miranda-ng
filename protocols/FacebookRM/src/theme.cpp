/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-15 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

extern OBJLIST<FacebookProto> g_Instances;

// Contact menu items
HGENMENU g_hContactMenuVisitProfile;
HGENMENU g_hContactMenuVisitFriendship;
HGENMENU g_hContactMenuAuthRevoke;
HGENMENU g_hContactMenuAuthAsk;
HGENMENU g_hContactMenuAuthGrant;
HGENMENU g_hContactMenuAuthCancel;
HGENMENU g_hContactMenuAuthDeny;
HGENMENU g_hContactMenuPoke;
HGENMENU g_hContactMenuPostStatus;
HGENMENU g_hContactMenuVisitConversation;

static IconItem icons[] =
{
	{ LPGEN("Facebook icon"), "facebook", IDI_FACEBOOK },
	{ LPGEN("Mind"), "mind", IDI_MIND },
	{ LPGEN("Poke"), "poke", IDI_POKE },
	{ LPGEN("Notification"), "notification", IDI_NOTIFICATION },
	{ LPGEN("Newsfeed"), "newsfeed", IDI_NEWSFEED },
	{ LPGEN("Friendship details"), "friendship", IDI_FRIENDS },
	{ LPGEN("Conversation"), "conversation", IDI_CONVERSATION },
	{ LPGEN("Message read"), "read", IDI_READ },
	{ LPGEN("Captcha form icon"), "key", IDI_KEYS }
};

void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/Facebook", icons, _countof(icons), "Facebook");
}

HANDLE GetIconHandle(const char* name)
{
	for (size_t i = 0; i < _countof(icons); i++)
		if (mir_strcmp(icons[i].szName, name) == 0)
			return icons[i].hIcolib;

	return 0;
}

// Helper functions
static FacebookProto * GetInstanceByHContact(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (!proto)
		return 0;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!mir_strcmp(proto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return 0;
}

template<INT_PTR(__cdecl FacebookProto::*Fcn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	FacebookProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam, lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	// Hide our all contact menu items first
	Menu_ShowItem(g_hContactMenuVisitProfile, false);
	Menu_ShowItem(g_hContactMenuVisitFriendship, false);
	Menu_ShowItem(g_hContactMenuAuthRevoke, false);
	Menu_ShowItem(g_hContactMenuAuthAsk, false);
	Menu_ShowItem(g_hContactMenuAuthGrant, false);
	Menu_ShowItem(g_hContactMenuAuthCancel, false);
	Menu_ShowItem(g_hContactMenuAuthDeny, false);
	Menu_ShowItem(g_hContactMenuPoke, false);
	Menu_ShowItem(g_hContactMenuPostStatus, false);
	Menu_ShowItem(g_hContactMenuVisitConversation, false);

	// Process them in correct account
	FacebookProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void InitContactMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CLISTMENUITEM mi = { 0 };
	mi.position = -2000006000;
	mi.icolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	mi.name.a = LPGEN("Visit profile");
	mi.pszService = "FacebookProto/VisitProfile";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitProfile>);
	g_hContactMenuVisitProfile = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006001;
	mi.icolibItem = GetIconHandle("friendship");
	mi.name.a = LPGEN("Visit friendship details");
	mi.pszService = "FacebookProto/VisitFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitFriendship>);
	g_hContactMenuVisitFriendship = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006002;
	mi.icolibItem = GetIconHandle("conversation");
	mi.name.a = LPGEN("Visit conversation");
	mi.pszService = "FacebookProto/VisitConversation";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitConversation>);
	g_hContactMenuVisitConversation = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006003;
	mi.icolibItem = GetIconHandle("mind");
	mi.name.a = LPGEN("Share status...");
	mi.pszService = "FacebookProto/Mind";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::OnMind>);
	g_hContactMenuPostStatus = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006004;
	mi.icolibItem = GetIconHandle("poke");
	mi.name.a = LPGEN("Poke");
	mi.pszService = "FacebookProto/Poke";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::Poke>);
	g_hContactMenuPoke = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006010;
	mi.icolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.name.a = LPGEN("Cancel friendship");
	mi.pszService = "FacebookProto/CancelFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::CancelFriendship>);
	g_hContactMenuAuthRevoke = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006011;
	mi.icolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.name.a = LPGEN("Cancel friendship request");
	mi.pszService = "FacebookProto/CancelFriendshipRequest";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::OnCancelFriendshipRequest>);
	g_hContactMenuAuthCancel = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006012;
	mi.icolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	mi.name.a = LPGEN("Request friendship");
	mi.pszService = "FacebookProto/RequestFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::RequestFriendship>);
	g_hContactMenuAuthAsk = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006013;
	mi.icolibItem = Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	mi.name.a = LPGEN("Approve friendship");
	mi.pszService = "FacebookProto/ApproveFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::ApproveFriendship>);
	g_hContactMenuAuthGrant = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006014;
	mi.icolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.name.a = LPGEN("Deny friendship request");
	mi.pszService = "FacebookProto/DenyFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::DenyFriendship>);
	g_hContactMenuAuthDeny = Menu_AddContactMenuItem(&mi);
}

int FacebookProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = MCONTACT(wParam);
	bool bIsChatroom = isChatRoom(hContact);

	Menu_ShowItem(g_hContactMenuVisitProfile, !bIsChatroom);
	Menu_ShowItem(g_hContactMenuVisitFriendship, !bIsChatroom);
	Menu_ShowItem(g_hContactMenuVisitConversation, true);
	Menu_ShowItem(g_hContactMenuPostStatus, !bIsChatroom);

	if (!isOffline() && !bIsChatroom)
	{
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		BYTE type = getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0);

		Menu_ShowItem(g_hContactMenuAuthAsk, ctrlPressed || type == CONTACT_NONE || !type);
		Menu_ShowItem(g_hContactMenuAuthGrant, ctrlPressed || type == CONTACT_APPROVE);
		Menu_ShowItem(g_hContactMenuAuthDeny, ctrlPressed || type == CONTACT_APPROVE);
		Menu_ShowItem(g_hContactMenuAuthRevoke, ctrlPressed || type == CONTACT_FRIEND);
		Menu_ShowItem(g_hContactMenuAuthCancel, ctrlPressed || type == CONTACT_REQUEST);

		Menu_ShowItem(g_hContactMenuPoke, true);
	}

	return 0;
}

int FacebookProto::OnBuildStatusMenu(WPARAM, LPARAM)
{
	char text[200];
	mir_strncpy(text, m_szModuleName, 100);
	char *tDest = text + mir_strlen(text);

	CLISTMENUITEM mi = { 0 };
	mi.pszService = text;

	HGENMENU hRoot = Menu_GetProtocolRoot(m_szModuleName);
	if (hRoot == NULL) {
		CLISTMENUITEM miRoot = { 0 };
		miRoot.position = 500085000;
		miRoot.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | (this->isOnline() ? 0 : CMIF_GRAYED);
		miRoot.icolibItem = GetIconHandle("facebook");
		miRoot.name.t = m_tszUserName;
		hRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&miRoot);
	}
	else {
		if (m_hMenuRoot) {
			Menu_RemoveItem(m_hMenuRoot);
			m_hMenuRoot = NULL;
		}
	}

	mi.flags = (this->isOnline() ? 0 : CMIF_GRAYED);
	mi.position = 201001;
	mi.hParentMenu = hRoot;

	//CreateProtoService(m_szModuleName,"/Mind",&FacebookProto::OnMind,this);
	mir_strcpy(tDest, "/Mind");
	mi.name.a = LPGEN("Share status...");
	mi.icolibItem = GetIconHandle("mind");
	m_hStatusMind = Menu_AddProtoMenuItem(&mi);

	//CreateProtoService("/VisitProfile",&FacebookProto::VisitProfile);
	mir_strcpy(tDest, "/VisitProfile");
	mi.name.a = LPGEN("Visit profile");
	mi.icolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	// TODO RM: remember and properly free in destructor?
	/*m_hStatusMind = */Menu_AddProtoMenuItem(&mi);

	//CreateProtoService("/VisitNotifications", &FacebookProto::VisitNotifications);
	mir_strcpy(tDest, "/VisitNotifications");
	mi.name.a = LPGEN("Visit notifications");
	mi.icolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	Menu_AddProtoMenuItem(&mi);

	// Services...
	mi.hParentMenu = m_hMenuServicesRoot = Menu_CreateRoot(MO_PROTO, LPGENT("Services..."), mi.position, Skin_GetIconHandle(SKINICON_OTHER_HELP));

	CreateProtoService("/RefreshBuddyList", &FacebookProto::RefreshBuddyList);
	mir_strcpy(tDest, "/RefreshBuddyList");
	mi.name.a = LPGEN("Refresh Buddy List");
	mi.icolibItem = GetIconHandle("friendship");
	Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/CheckFriendRequests", &FacebookProto::CheckFriendRequests);
	mir_strcpy(tDest, "/CheckFriendRequests");
	mi.name.a = LPGEN("Check Friends Requests");
	mi.icolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/CheckNewsfeeds", &FacebookProto::CheckNewsfeeds);
	mir_strcpy(tDest, "/CheckNewsfeeds");
	mi.name.a = LPGEN("Check Newsfeeds");
	mi.icolibItem = GetIconHandle("newsfeed");
	Menu_AddProtoMenuItem(&mi);
	return 0;
}

void FacebookProto::ToggleStatusMenuItems(bool bEnable)
{
	Menu_EnableItem(m_hMenuRoot, bEnable);
	Menu_EnableItem(m_hStatusMind, bEnable);
	Menu_EnableItem(m_hMenuServicesRoot, bEnable);
}
