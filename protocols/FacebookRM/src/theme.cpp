/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

// Contact menu items
HGENMENU g_hContactMenuVisitProfile;
HGENMENU g_hContactMenuVisitFriendship;
HGENMENU g_hContactMenuAuthCancel;
HGENMENU g_hContactMenuAuthDeny;
HGENMENU g_hContactMenuPoke;
HGENMENU g_hContactMenuPostStatus;
HGENMENU g_hContactMenuVisitConversation;
HGENMENU g_hContactMenuLoadHistory;

static IconItem icons[] =
{
	{ LPGEN("Facebook icon"),      "facebook",     IDI_FACEBOOK     },
	{ LPGEN("Mind"),               "mind",         IDI_MIND         },
	{ LPGEN("Poke"),               "poke",         IDI_POKE         },
	{ LPGEN("Notification"),       "notification", IDI_NOTIFICATION },
	{ LPGEN("Newsfeed"),           "newsfeed",     IDI_NEWSFEED     },
	{ LPGEN("Memories"),           "memories",     IDI_MEMORIES     },
	{ LPGEN("Friendship details"), "friendship",   IDI_FRIENDS      },
	{ LPGEN("Conversation"),       "conversation", IDI_CONVERSATION },
	{ LPGEN("Message read"),       "read",         IDI_READ         },
	{ LPGEN("Angry"),              "angry",        IDI_ANGRY        },
	{ LPGEN("Haha"),               "haha",         IDI_HAHA         },
	{ LPGEN("Like"),               "like",         IDI_LIKE         },
	{ LPGEN("Love"),               "love",         IDI_LOVE         },
	{ LPGEN("Sad"),                "sad",          IDI_SAD          },
	{ LPGEN("Wow"),                "wow",          IDI_WOW          },
};

void InitIcons(void)
{
	g_plugin.registerIcon("Protocols/Facebook", icons, "Facebook");
}

// Helper functions
template<INT_PTR(__cdecl FacebookProto::*Fcn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	FacebookProto *proto = CMPlugin::getInstance(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam, lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	// Hide our all contact menu items first
	Menu_ShowItem(g_hContactMenuVisitProfile, false);
	Menu_ShowItem(g_hContactMenuVisitFriendship, false);
	Menu_ShowItem(g_hContactMenuAuthCancel, false);
	Menu_ShowItem(g_hContactMenuAuthDeny, false);
	Menu_ShowItem(g_hContactMenuPoke, false);
	Menu_ShowItem(g_hContactMenuPostStatus, false);
	Menu_ShowItem(g_hContactMenuVisitConversation, false);
	Menu_ShowItem(g_hContactMenuLoadHistory, false);

	// Process them in correct account
	FacebookProto *proto = CMPlugin::getInstance(MCONTACT(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void InitContactMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x4f006492, 0x9fe5, 0x4d10, 0x88, 0xce, 0x47, 0x53, 0xba, 0x27, 0xe9, 0xc9);
	mi.position = -2000006000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	mi.name.a = LPGEN("Visit profile");
	mi.pszService = "FacebookProto/VisitProfile";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitProfile>);
	g_hContactMenuVisitProfile = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x1e9d0534, 0xc319, 0x42a2, 0xbe, 0xd5, 0x1e, 0xae, 0xe1, 0x54, 0xd, 0x89);
	mi.position = -2000006001;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_FRIENDS);
	mi.name.a = LPGEN("Visit friendship details");
	mi.pszService = "FacebookProto/VisitFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitFriendship>);
	g_hContactMenuVisitFriendship = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xd3bfd7d6, 0x43c3, 0x4b05, 0x81, 0x40, 0xc8, 0xbe, 0x81, 0xd9, 0x95, 0xff);
	mi.position = -2000006002;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_CONVERSATION);
	mi.name.a = LPGEN("Visit conversation");
	mi.pszService = "FacebookProto/VisitConversation";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitConversation>);
	g_hContactMenuVisitConversation = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xc631b2ea, 0xa133, 0x4cc9, 0x81, 0x1e, 0xad, 0x8f, 0x36, 0x5c, 0x74, 0xbf);
	mi.position = -2000006003;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MIND);
	mi.name.a = LPGEN("Share status...");
	mi.pszService = "FacebookProto/Mind";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::OnMind>);
	g_hContactMenuPostStatus = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x10ce2dbf, 0x8acf, 0x4f51, 0x89, 0x76, 0xd9, 0x67, 0xef, 0x69, 0x1d, 0x9d);
	mi.position = -2000006004;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_POKE);
	mi.name.a = LPGEN("Poke");
	mi.pszService = "FacebookProto/Poke";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::Poke>);
	g_hContactMenuPoke = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x58e75db0, 0xb9e0, 0x4aa8, 0xbb, 0x42, 0x8d, 0x7d, 0xd1, 0xf6, 0x8e, 0x99);
	mi.position = -2000006005;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_CONVERSATION); // TODO: Use better icon
	mi.name.a = LPGEN("Load history");
	mi.pszService = "FacebookProto/LoadHistory";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::LoadHistory>);
	g_hContactMenuLoadHistory = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x6d6b49b9, 0x71b8, 0x4a57, 0xab, 0x80, 0xc3, 0xb2, 0xbe, 0x2b, 0x9b, 0xf5);
	mi.position = -2000006011;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REVOKE);
	mi.name.a = LPGEN("Cancel friendship request");
	mi.pszService = "FacebookProto/CancelFriendshipRequest";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::OnCancelFriendshipRequest>);
	g_hContactMenuAuthCancel = Menu_AddContactMenuItem(&mi);
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

	BYTE type = getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0);
	bool bIsChatroom = isChatRoom(hContact);
	bool bIsSpecialChatroom = IsSpecialChatRoom(hContact);
	bool bIsPage = (type == CONTACT_PAGE);

	Menu_ShowItem(g_hContactMenuVisitProfile, !bIsChatroom);
	Menu_ShowItem(g_hContactMenuVisitFriendship, !bIsChatroom && !bIsPage);
	Menu_ShowItem(g_hContactMenuVisitConversation, !bIsSpecialChatroom);
	Menu_ShowItem(g_hContactMenuPostStatus, !bIsChatroom);
	Menu_ShowItem(g_hContactMenuLoadHistory, !bIsChatroom);

	if (!isOffline() && !bIsChatroom && !bIsPage) {
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

		Menu_ShowItem(m_hmiReqAuth, ctrlPressed || type == CONTACT_NONE || !type);
		Menu_ShowItem(m_hmiGrantAuth, ctrlPressed || type == CONTACT_APPROVE);
		Menu_ShowItem(m_hmiRevokeAuth, ctrlPressed || type == CONTACT_FRIEND);

		Menu_ShowItem(g_hContactMenuAuthCancel, ctrlPressed || type == CONTACT_REQUEST);
		Menu_ShowItem(g_hContactMenuAuthDeny, ctrlPressed || type == CONTACT_APPROVE);
		Menu_ShowItem(g_hContactMenuPoke, true);
	}

	return 0;
}

void FacebookProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.position = 201001;
	mi.root = Menu_GetProtocolRoot(this);

	mi.pszService = "/Mind";
	CreateProtoService(mi.pszService, &FacebookProto::OnMind);
	mi.name.a = LPGEN("Share status...");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MIND);
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

	mi.pszService = "/CheckFriendRequests";
	CreateProtoService(mi.pszService, &FacebookProto::CheckFriendRequests);
	mi.name.a = LPGEN("Check friendship requests");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_REQUEST);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CheckNewsfeeds";
	CreateProtoService(mi.pszService, &FacebookProto::CheckNewsfeeds);
	mi.name.a = LPGEN("Check newsfeeds");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_NEWSFEED);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CheckMemories";
	CreateProtoService(mi.pszService, &FacebookProto::CheckMemories);
	mi.name.a = LPGEN("Check memories");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MEMORIES);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = "/CheckNotifications";
	CreateProtoService(mi.pszService, &FacebookProto::CheckNotifications);
	mi.name.a = LPGEN("Check notifications");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_NOTIFICATION);
	Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

int FacebookProto::OnBuildStatusMenu(WPARAM, LPARAM)
{
	ToggleStatusMenuItems(this->isOnline());
	return 0;
}

void FacebookProto::ToggleStatusMenuItems(bool bEnable)
{
	Menu_EnableItem(m_hmiMainMenu, bEnable);
	Menu_EnableItem(m_hStatusMind, bEnable);
	Menu_EnableItem(m_hMenuServicesRoot, bEnable);
}
