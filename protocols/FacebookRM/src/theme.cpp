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

	CMenuItem mi;

	SET_UID(mi, 0x4f006492, 0x9fe5, 0x4d10, 0x88, 0xce, 0x47, 0x53, 0xba, 0x27, 0xe9, 0xc9);
	mi.position = -2000006000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	mi.name.a = LPGEN("Visit profile");
	mi.pszService = "FacebookProto/VisitProfile";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitProfile>);
	g_hContactMenuVisitProfile = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x1e9d0534, 0xc319, 0x42a2, 0xbe, 0xd5, 0x1e, 0xae, 0xe1, 0x54, 0xd, 0x89);
	mi.position = -2000006001;
	mi.hIcolibItem = GetIconHandle("friendship");
	mi.name.a = LPGEN("Visit friendship details");
	mi.pszService = "FacebookProto/VisitFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitFriendship>);
	g_hContactMenuVisitFriendship = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xd3bfd7d6, 0x43c3, 0x4b05, 0x81, 0x40, 0xc8, 0xbe, 0x81, 0xd9, 0x95, 0xff);
	mi.position = -2000006002;
	mi.hIcolibItem = GetIconHandle("conversation");
	mi.name.a = LPGEN("Visit conversation");
	mi.pszService = "FacebookProto/VisitConversation";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitConversation>);
	g_hContactMenuVisitConversation = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xc631b2ea, 0xa133, 0x4cc9, 0x81, 0x1e, 0xad, 0x8f, 0x36, 0x5c, 0x74, 0xbf);
	mi.position = -2000006003;
	mi.hIcolibItem = GetIconHandle("mind");
	mi.name.a = LPGEN("Share status...");
	mi.pszService = "FacebookProto/Mind";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::OnMind>);
	g_hContactMenuPostStatus = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x10ce2dbf, 0x8acf, 0x4f51, 0x89, 0x76, 0xd9, 0x67, 0xef, 0x69, 0x1d, 0x9d);
	mi.position = -2000006004;
	mi.hIcolibItem = GetIconHandle("poke");
	mi.name.a = LPGEN("Poke");
	mi.pszService = "FacebookProto/Poke";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::Poke>);
	g_hContactMenuPoke = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x619efdcb, 0x99c0, 0x44a8, 0xbf, 0x28, 0xc3, 0xe0, 0x2f, 0xb3, 0x7e, 0x77);
	mi.position = -2000006010;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.name.a = LPGEN("Cancel friendship");
	mi.pszService = "FacebookProto/CancelFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::CancelFriendship>);
	g_hContactMenuAuthRevoke = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, x6d6b49b9, 0x71b8, 0x4a57, 0xab, 0x80, 0xc3, 0xb2, 0xbe, 0x2b, 0x9b, 0xf5);
	mi.position = -2000006011;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.name.a = LPGEN("Cancel friendship request");
	mi.pszService = "FacebookProto/CancelFriendshipRequest";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::OnCancelFriendshipRequest>);
	g_hContactMenuAuthCancel = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x36375a1f, 0xc142, 0x4d6e, 0xa6, 0x57, 0xe4, 0x76, 0x5d, 0xbc, 0x59, 0x8e);
	mi.position = -2000006012;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	mi.name.a = LPGEN("Request friendship");
	mi.pszService = "FacebookProto/RequestFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::RequestFriendship>);
	g_hContactMenuAuthAsk = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x4c90452a, 0x869a, 0x4a81, 0xaf, 0xa8, 0x28, 0x34, 0xaf, 0x2b, 0x6b, 0x30);
	mi.position = -2000006013;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_GRANT);
	mi.name.a = LPGEN("Approve friendship");
	mi.pszService = "FacebookProto/ApproveFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::ApproveFriendship>);
	g_hContactMenuAuthGrant = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x29d0a371, 0xb8a7, 0x4fb2, 0x91, 0x10, 0x13, 0x6f, 0x8c, 0x5f, 0xb5, 0x7);
	mi.position = -2000006014;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
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
	CMenuItem mi;
	mi.position = 201001;
	mi.root = Menu_GetProtocolRoot(this);

	mi.pszService = "/Mind";
	CreateProtoService(mi.pszService, &FacebookProto::OnMind);
	mi.name.a = LPGEN("Share status...");
	mi.hIcolibItem = GetIconHandle("mind");
	m_hStatusMind = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/VisitProfile";
	CreateProtoService(mi.pszService, &FacebookProto::VisitProfile);
	mi.name.a = LPGEN("Visit profile");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/VisitNotifications";
	CreateProtoService(mi.pszService, &FacebookProto::VisitNotifications);
	mi.name.a = LPGEN("Visit notifications");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	// Services...
	mi.name.a = LPGEN("Services...");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HELP);
	mi.root = m_hMenuServicesRoot = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/RefreshBuddyList";
	CreateProtoService(mi.pszService, &FacebookProto::RefreshBuddyList);
	mi.name.a = LPGEN("Refresh Buddy List");
	mi.hIcolibItem = GetIconHandle("friendship");
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CheckFriendRequests";
	CreateProtoService(mi.pszService, &FacebookProto::CheckFriendRequests);
	mi.name.a = LPGEN("Check Friends Requests");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CheckNewsfeeds";
	CreateProtoService(mi.pszService, &FacebookProto::CheckNewsfeeds);
	mi.name.a = LPGEN("Check Newsfeeds");
	mi.hIcolibItem = GetIconHandle("newsfeed");
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	ToggleStatusMenuItems(this->isOnline());
	return 0;
}

void FacebookProto::ToggleStatusMenuItems(bool bEnable)
{
	Menu_EnableItem(m_hMainMenuItem, bEnable);
	Menu_EnableItem(m_hStatusMind, bEnable);
	Menu_EnableItem(m_hMenuServicesRoot, bEnable);
}
