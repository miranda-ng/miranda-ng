	#include "common.h"

extern OBJLIST<WhatsAppProto> g_Instances;

static IconItem icons[] =
{
	{ LPGEN("WhatsApp icon"),             "whatsApp",                 IDI_WHATSAPP },
	{ LPGEN("Create chat group"),         "createGroup",              IDI_ADD_GROUP }
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

int WhatsAppProto::OnBuildStatusMenu(WPARAM wParam, LPARAM lParam)
{
	char text[200];
	strcpy(text, m_szModuleName);
	char *tDest = text + mir_strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL) {
		mi.popupPosition = 500085000;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | (isOnline() ? 0 : CMIF_GRAYED);
		mi.icolibItem = GetIconHandle("whatsApp");
		mi.ptszName = m_tszUserName;
		hRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else {
		if (m_hMenuRoot)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.flags = CMIF_CHILDPOPUP | (isOnline() ? 0 : CMIF_GRAYED);
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
