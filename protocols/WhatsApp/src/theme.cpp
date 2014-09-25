	#include "common.h"

extern OBJLIST<WhatsAppProto> g_Instances;

static IconItem icons[] =
{
	{ LPGEN("WhatsApp icon"),             "whatsApp",                 IDI_WHATSAPP },
	{ LPGEN("Add to group"),              "addContactToGroup",        IDI_ADD_USER_TO_GROUP},
	{ LPGEN("Create chat group"),         "createGroup",              IDI_ADD_GROUP },
	{ LPGEN("Remove from chat group"),    "removeContactFromGroup",   IDI_REMOVE_USER_FROM_GROUP },
	{ LPGEN("Leave and delete group"),    "leaveAndDeleteGroup",      IDI_LEAVE_GROUP },
	{ LPGEN("Leave group"),               "leaveGroup",               IDI_LEAVE_GROUP },
	{ LPGEN("Change group subject"),      "changeGroupSubject",       IDI_CHANGE_GROUP_SUBJECT }
};

void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/WhatsApp", icons, SIZEOF(icons), "WhatsApp");
}

HANDLE GetIconHandle(const char* name)
{
	for (size_t i = 0; i < SIZEOF(icons); i++)
		if (strcmp(icons[i].szName, name) == 0)
			return icons[i].hIcolib;

	return 0;
}

char* GetIconDescription(const char* name)
{
	for (size_t i = 0; i < SIZEOF(icons); i++)
		if (strcmp(icons[i].szName, name) == 0)
			return icons[i].szDescr;

	return "";
}

// Contact List menu stuff
HGENMENU g_hContactMenuItems[CMITEMS_COUNT];

// Helper functions
static WhatsAppProto* GetInstanceByHContact(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (!proto)
		return 0;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!strcmp(proto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return 0;
}

template<INT_PTR(__cdecl WhatsAppProto::*Fcn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	WhatsAppProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam, lParam) : 0;
}

template<INT_PTR(__cdecl WhatsAppProto::*Fcn)(WPARAM, LPARAM, LPARAM)>
INT_PTR GlobalServiceParam(WPARAM wParam, LPARAM lParam, LPARAM lParam2)
{
	WhatsAppProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam, lParam, lParam2) : 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	for (size_t i = 0; i < SIZEOF(g_hContactMenuItems); i++)
		Menu_ShowItem(g_hContactMenuItems[i], false);

	WhatsAppProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void WhatsAppProto::InitContactMenus()
{
	::HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CLISTMENUITEM mi = { sizeof(mi) };

	mi.position = -2000006100;
	mi.icolibItem = GetIconHandle("leaveGroup");
	mi.pszName = GetIconDescription("leaveGroup");
	mi.pszService = "WhatsAppProto/LeaveGroup";
	CreateServiceFunction(mi.pszService, GlobalService<&WhatsAppProto::OnLeaveGroup>);
	g_hContactMenuItems[CMI_LEAVE_GROUP] = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006100;
	mi.icolibItem = GetIconHandle("leaveAndDeleteGroup");
	mi.pszName = GetIconDescription("leaveAndDeleteGroup");
	g_hContactMenuItems[CMI_REMOVE_GROUP] = Menu_AddContactMenuItem(&mi);

	mi.position = -2000006099;
	mi.icolibItem = GetIconHandle("changeGroupSubject");
	mi.pszName = GetIconDescription("changeGroupSubject");
	mi.pszService = "WhatsAppProto/ChangeGroupSubject";
	CreateServiceFunction(mi.pszService, GlobalService<&WhatsAppProto::OnChangeGroupSubject>);
	g_hContactMenuItems[CMI_CHANGE_GROUP_SUBJECT] = Menu_AddContactMenuItem(&mi);
}

int WhatsAppProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = MCONTACT(wParam);
	if (hContact)
		debugLog(pcli->pfnGetContactDisplayName(hContact, 0));
	else
		debugLogA("No contact found");

	if (g_hContactMenuItems[CMI_ADD_CONTACT_TO_GROUP] != NULL)
		CallService("CList/RemoveContactMenuItem", (WPARAM)g_hContactMenuItems[CMI_ADD_CONTACT_TO_GROUP], (LPARAM)0);

	if (g_hContactMenuItems[CMI_REMOVE_CONTACT_FROM_GROUP] != NULL)
		CallService("CList/RemoveContactMenuItem", (WPARAM)g_hContactMenuItems[CMI_REMOVE_CONTACT_FROM_GROUP], (LPARAM)0);

	int chatType = getByte(hContact, "SimpleChatRoom", 0);

	CLISTMENUITEM mi = { sizeof(mi) };

	if (chatType == 0) {
		mi.flags = CMIF_CHILDPOPUP;
		mi.position = -2000006102;
		mi.icolibItem = GetIconHandle("addContactToGroup");
		mi.pszName = GetIconDescription("addContactToGroup");
		mi.pszService = NULL;
		g_hContactMenuItems[CMI_ADD_CONTACT_TO_GROUP] = Menu_AddContactMenuItem(&mi);

		if (!isOnline()) {
			Menu_ShowItem(g_hContactMenuItems[CMI_ADD_CONTACT_TO_GROUP], false);
			return 0;
		}

		mi.hParentMenu = (HGENMENU)g_hContactMenuItems[CMI_ADD_CONTACT_TO_GROUP];
		mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;

		int iGrpCount = 0;
		string fullSvcName;
		string svcName = m_szModuleName;
		svcName += "/AddContactToGroup_";
		DBVARIANT dbv;

		for (map<MCONTACT, map<MCONTACT, bool>>::iterator it = this->isMemberByGroupContact.begin();
			  it != this->isMemberByGroupContact.end(); ++it) {
			map<MCONTACT, bool>::iterator memberIt = it->second.find(hContact);
			// Only, if current contact is not already member of this group
			if ((memberIt == it->second.end() || memberIt->second == false) && !getString(it->first, "ID", &dbv)) {
				fullSvcName = svcName + dbv.pszVal;
				mi.pszService = (char*)fullSvcName.c_str();
				mi.ptszName = pcli->pfnGetContactDisplayName(it->first, 0);
				CreateServiceFunctionParam(mi.pszService, GlobalServiceParam<&WhatsAppProto::OnAddContactToGroup>, (LPARAM)it->first);
				Menu_AddContactMenuItem(&mi);
				db_free(&dbv);
				iGrpCount++;
			}
		}
		if (!iGrpCount)
			Menu_ShowItem(g_hContactMenuItems[CMI_ADD_CONTACT_TO_GROUP], false);
	}
	else if (chatType == 1) {
		mi.flags = CMIM_FLAGS;
		if (!isOnline() || getByte(hContact, "IsGroupMember", 0) == 0)
			mi.flags |= CMIF_GRAYED;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hContactMenuItems[CMI_LEAVE_GROUP], (LPARAM)&mi);
	}
	else if (chatType == 2) {
		// owning chat/group
		mi.flags = CMIF_CHILDPOPUP;
		mi.position = -2000006102;
		mi.icolibItem = GetIconHandle("removeContactFromGroup");
		mi.pszName = GetIconDescription("removeContactFromGroup");
		mi.pszService = NULL;
		g_hContactMenuItems[CMI_REMOVE_CONTACT_FROM_GROUP] = Menu_AddContactMenuItem(&mi);

		bool bShow = false;
		if (isOnline() && getByte(hContact, "IsGroupMember", 0) == 1) {
			map<MCONTACT, map<MCONTACT, bool>>::iterator groupsIt = this->isMemberByGroupContact.find(hContact);
			if (groupsIt == this->isMemberByGroupContact.end())
				debugLogA("Group exists only on contact list");
			else {
				mi.hParentMenu = (HGENMENU)g_hContactMenuItems[CMI_REMOVE_CONTACT_FROM_GROUP];
				mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR;

				string fullSvcName;
				string svcName = m_szModuleName;
				svcName += "/RemoveContactFromGroup_";
				DBVARIANT dbv;

				for (map<MCONTACT, bool>::iterator it = groupsIt->second.begin(); it != groupsIt->second.end(); ++it) {
					if (!getString(it->first, "ID", &dbv)) {
						fullSvcName = svcName + dbv.pszVal;
						mi.pszService = (char*)fullSvcName.c_str();
						mi.ptszName = pcli->pfnGetContactDisplayName(it->first, 0);
						CreateServiceFunctionParam(mi.pszService,
															GlobalServiceParam<&WhatsAppProto::OnRemoveContactFromGroup>, (LPARAM)it->first);
						Menu_AddContactMenuItem(&mi);
						db_free(&dbv);
						bShow = true;
					}
				}
			}
		}
		else Menu_ShowItem(g_hContactMenuItems[CMI_REMOVE_CONTACT_FROM_GROUP], false);

		Menu_ShowItem(g_hContactMenuItems[CMI_REMOVE_GROUP], bShow);
		Menu_ShowItem(g_hContactMenuItems[CMI_CHANGE_GROUP_SUBJECT], bShow);
	}

	return 0;
}

int WhatsAppProto::OnBuildStatusMenu(WPARAM wParam, LPARAM lParam)
{
	char text[200];
	strcpy(text, m_szModuleName);
	char *tDest = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL) {
		mi.popupPosition = 500085000;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | (this->isOnline() ? 0 : CMIF_GRAYED);
		mi.icolibItem = GetIconHandle("whatsApp");
		mi.ptszName = m_tszUserName;
		hRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else {
		if (m_hMenuRoot)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.flags = CMIF_CHILDPOPUP | (this->isOnline() ? 0 : CMIF_GRAYED);
	mi.position = 201001;

	CreateProtoService("/CreateGroup", &WhatsAppProto::OnCreateGroup);
	strcpy(tDest, "/CreateGroup");
	mi.hParentMenu = hRoot;
	mi.pszName = LPGEN("Create group");
	mi.icolibItem = GetIconHandle("createGroup");
	m_hMenuCreateGroup = Menu_AddProtoMenuItem(&mi);

	return 0;
}

void WhatsAppProto::ToggleStatusMenuItems(BOOL bEnable)
{
	CLISTMENUITEM clmi = { sizeof(clmi) };
	clmi.flags = CMIM_FLAGS | ((bEnable) ? 0 : CMIF_GRAYED);

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)m_hMenuRoot, (LPARAM)&clmi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)m_hMenuCreateGroup, (LPARAM)&clmi);
}
