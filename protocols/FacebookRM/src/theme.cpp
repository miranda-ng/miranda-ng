/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

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

#include "common.h"

extern OBJLIST<FacebookProto> g_Instances;

static IconItem icons[] =
{
	{ LPGEN("Facebook icon"),             "facebook",      IDI_FACEBOOK },
	{ LPGEN("Mind"),                      "mind",          IDI_MIND },
	{ LPGEN("Poke"),                      "poke",          IDI_POKE },
	{ LPGEN("Notification"),              "notification",  IDI_NOTIFICATION },
	{ LPGEN("Newsfeed"),                  "newsfeed",      IDI_NEWSFEED },
	{ LPGEN("Friendship details"),        "friendship",    IDI_FRIENDS },
	{ LPGEN("Conversation"),              "conversation",  IDI_CONVERSATION },
	{ LPGEN("Message read"),              "read",          IDI_READ },
	{ LPGEN("Captcha form icon"),         "key",           IDI_KEYS }
};

// TODO: uninit
void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/Facebook", icons, SIZEOF(icons), "Facebook");
}

HANDLE GetIconHandle(const char* name)
{
	for(size_t i=0; i<SIZEOF(icons); i++)
		if(strcmp(icons[i].szName, name) == 0)
			return icons[i].hIcolib;

	return 0;
}

// Contact List menu stuff
HGENMENU g_hContactMenuItems[CMITEMS_COUNT];

// Helper functions
static FacebookProto * GetInstanceByHContact(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if(!proto)
		return 0;

	for(int i=0; i<g_Instances.getCount(); i++)
		if(!strcmp(proto,g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return 0;
}

template<INT_PTR (__cdecl FacebookProto::*Fcn)(WPARAM,LPARAM)>
INT_PTR GlobalService(WPARAM wParam,LPARAM lParam)
{
	FacebookProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam,lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	for (size_t i=0; i<SIZEOF(g_hContactMenuItems); i++)
		Menu_ShowItem(g_hContactMenuItems[i], false);

	FacebookProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam,lParam) : 0;
}

void InitContactMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,PrebuildContactMenu);

	CLISTMENUITEM mi = {sizeof(mi)};
	mi.position=-2000006000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_URL);
	mi.pszName = LPGEN("Visit profile");
	mi.pszService = "FacebookProto/VisitProfile";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::VisitProfile>);
	g_hContactMenuItems[CMI_VISIT_PROFILE] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006001;
	mi.icolibItem = GetIconHandle("friendship");
	mi.pszName = LPGEN("Visit friendship details");
	mi.pszService = "FacebookProto/VisitFriendship";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::VisitFriendship>);
	g_hContactMenuItems[CMI_VISIT_FRIENDSHIP] = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006002;
	mi.icolibItem = GetIconHandle("conversation");
	mi.pszName = LPGEN("Visit conversation");
	mi.pszService = "FacebookProto/VisitConversation";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::VisitConversation>);
	g_hContactMenuItems[CMI_VISIT_CONVERSATION] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006003;
	mi.icolibItem = GetIconHandle("mind");
	mi.pszName = LPGEN("Share status...");
	mi.pszService = "FacebookProto/Mind";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::OnMind>);
	g_hContactMenuItems[CMI_POST_STATUS] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006004;
	mi.icolibItem = GetIconHandle("poke");
	mi.pszName = LPGEN("Poke");
	mi.pszService = "FacebookProto/Poke";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::Poke>);
	g_hContactMenuItems[CMI_POKE] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006010;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	mi.pszName = LPGEN("Cancel friendship");
	mi.pszService = "FacebookProto/CancelFriendship";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::CancelFriendship>);
	g_hContactMenuItems[CMI_AUTH_REVOKE] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006011;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	mi.pszName = LPGEN("Cancel friendship request");
	mi.pszService = "FacebookProto/CancelFriendshipRequest";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::OnCancelFriendshipRequest>);
	g_hContactMenuItems[CMI_AUTH_CANCEL] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006012;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	mi.pszName = LPGEN("Request friendship");
	mi.pszService = "FacebookProto/RequestFriendship";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::RequestFriendship>);
	g_hContactMenuItems[CMI_AUTH_ASK] = Menu_AddContactMenuItem(&mi);

	mi.position=-2000006013;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_GRANT);
	mi.pszName = LPGEN("Approve friendship");
	mi.pszService = "FacebookProto/ApproveFriendship";
	CreateServiceFunction(mi.pszService,GlobalService<&FacebookProto::ApproveFriendship>);
	g_hContactMenuItems[CMI_AUTH_GRANT] = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006014;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REVOKE);
	mi.pszName = LPGEN("Deny friendship request");
	mi.pszService = "FacebookProto/DenyFriendship";
	CreateServiceFunction(mi.pszService, GlobalService<&FacebookProto::DenyFriendship>);
	g_hContactMenuItems[CMI_AUTH_DENY] = Menu_AddContactMenuItem(&mi);
}

void UninitContactMenus()
{
	for(size_t i=0; i<SIZEOF(g_hContactMenuItems); i++)
		CallService(MS_CLIST_REMOVECONTACTMENUITEM,(WPARAM)g_hContactMenuItems[i],0);
}

int FacebookProto::OnPrebuildContactMenu(WPARAM wParam,LPARAM lParam)
{	
	MCONTACT hContact = MCONTACT(wParam);
	bool bIsChatroom = isChatRoom(hContact);

	Menu_ShowItem(g_hContactMenuItems[CMI_VISIT_PROFILE], !bIsChatroom);
	Menu_ShowItem(g_hContactMenuItems[CMI_VISIT_FRIENDSHIP], !bIsChatroom);
	Menu_ShowItem(g_hContactMenuItems[CMI_VISIT_CONVERSATION], true);
	Menu_ShowItem(g_hContactMenuItems[CMI_POST_STATUS], !bIsChatroom);

	if (!isOffline() && !bIsChatroom) 
	{
		bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		BYTE type = getByte(hContact, FACEBOOK_KEY_CONTACT_TYPE, 0);

		Menu_ShowItem(g_hContactMenuItems[CMI_AUTH_ASK], ctrlPressed || type == CONTACT_NONE || !type);
		Menu_ShowItem(g_hContactMenuItems[CMI_AUTH_GRANT], ctrlPressed || type == CONTACT_APPROVE);
		Menu_ShowItem(g_hContactMenuItems[CMI_AUTH_DENY], ctrlPressed || type == CONTACT_APPROVE);
		Menu_ShowItem(g_hContactMenuItems[CMI_AUTH_REVOKE], ctrlPressed || type == CONTACT_FRIEND);
		Menu_ShowItem(g_hContactMenuItems[CMI_AUTH_CANCEL], ctrlPressed || type == CONTACT_REQUEST);		

		Menu_ShowItem(g_hContactMenuItems[CMI_POKE], true);
	}

	return 0;
}

int FacebookProto::OnBuildStatusMenu(WPARAM wParam,LPARAM lParam)
{
	char text[200];
	strcpy(text,m_szModuleName);
	char *tDest = text+strlen(text);

	CLISTMENUITEM mi = {sizeof(mi)};
	mi.pszService = text;

	HGENMENU hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL) {
		CLISTMENUITEM miRoot = { sizeof(miRoot) };
		miRoot.popupPosition = 500085000;
		miRoot.hParentMenu = HGENMENU_ROOT;
		miRoot.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | (this->isOnline() ? 0 : CMIF_GRAYED);
		miRoot.icolibItem = GetIconHandle("facebook");
		miRoot.ptszName = m_tszUserName;
		hRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&miRoot);
	}
	else {
		if (m_hMenuRoot)
			CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.flags = CMIF_CHILDPOPUP | (this->isOnline() ? 0 : CMIF_GRAYED);
	mi.position = 201001;
	mi.hParentMenu = hRoot;

	//CreateProtoService(m_szModuleName,"/Mind",&FacebookProto::OnMind,this);
	strcpy(tDest,"/Mind");
	mi.pszName = LPGEN("Share status...");
	mi.icolibItem = GetIconHandle("mind");
	m_hStatusMind = Menu_AddProtoMenuItem(&mi);

	//CreateProtoService("/VisitProfile",&FacebookProto::VisitProfile);
	strcpy(tDest,"/VisitProfile");
	mi.flags = CMIF_CHILDPOPUP;
	mi.pszName = LPGEN("Visit profile");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_URL);
	// TODO RM: remember and properly free in destructor?
	/*m_hStatusMind = */Menu_AddProtoMenuItem(&mi);

	//CreateProtoService("/VisitNotifications", &FacebookProto::VisitNotifications);
	strcpy(tDest, "/VisitNotifications");
	mi.pszName = LPGEN("Visit notifications");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_URL);
	Menu_AddProtoMenuItem(&mi);

	// Services...
	mi.pszName = LPGEN("Services...");
	strcpy(tDest, "/Services");
	mi.flags = CMIF_CHILDPOPUP | (this->isOnline() ? 0 : CMIF_GRAYED);
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_HELP);
	m_hMenuServicesRoot = Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/RefreshBuddyList",&FacebookProto::RefreshBuddyList);
	strcpy(tDest,"/RefreshBuddyList");
	mi.flags = CMIF_ROOTHANDLE;
	mi.pszName = LPGEN("Refresh Buddy List");
	mi.pszPopupName = LPGEN("Services");
	mi.icolibItem = GetIconHandle("friendship");
	mi.hParentMenu = m_hMenuServicesRoot;
	Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/CheckFriendRequests",&FacebookProto::CheckFriendRequests);
	strcpy(tDest,"/CheckFriendRequests");
	mi.flags = CMIF_ROOTHANDLE;
	mi.pszName = LPGEN("Check Friends Requests");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_AUTH_REQUEST);
	mi.hParentMenu = m_hMenuServicesRoot;
	Menu_AddProtoMenuItem(&mi);

	CreateProtoService("/CheckNewsfeeds",&FacebookProto::CheckNewsfeeds);
	strcpy(tDest,"/CheckNewsfeeds");
	mi.flags = CMIF_ROOTHANDLE;
	mi.pszName = LPGEN("Check Newsfeeds");
	mi.pszPopupName = LPGEN("Services");
	mi.icolibItem = GetIconHandle("newsfeed");
	mi.hParentMenu = m_hMenuServicesRoot;
	Menu_AddProtoMenuItem(&mi);

	return 0;
}

void FacebookProto::ToggleStatusMenuItems(BOOL bEnable)
{
	CLISTMENUITEM clmi = {sizeof(clmi)};
	clmi.flags = CMIM_FLAGS | ((bEnable) ? 0 : CMIF_GRAYED);

	Menu_ModifyItem(m_hMenuRoot, &clmi);
	Menu_ModifyItem(m_hStatusMind, &clmi);
	Menu_ModifyItem(m_hMenuServicesRoot, &clmi);
}
