	#include "common.h"

extern OBJLIST<WhatsAppProto> g_Instances;

static IconItem icons[] =
{
	{ LPGEN("WhatsApp icon"),             "whatsApp",                 IDI_WHATSAPP },
	{ LPGEN("Create chat group"),         "createGroup",              IDI_ADD_GROUP }
};

void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/WhatsApp", icons, _countof(icons), "WhatsApp");
}

HANDLE GetIconHandle(const char* name)
{
	for (size_t i = 0; i < _countof(icons); i++)
		if (mir_strcmp(icons[i].szName, name) == 0)
			return icons[i].hIcolib;

	return 0;
}

char* GetIconDescription(const char* name)
{
	for (size_t i = 0; i < _countof(icons); i++)
		if (mir_strcmp(icons[i].szName, name) == 0)
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
		if (!mir_strcmp(proto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return 0;
}

int WhatsAppProto::OnBuildStatusMenu(WPARAM wParam, LPARAM lParam)
{
	char text[200];
	mir_strcpy(text, m_szModuleName);
	char *tDest = text + mir_strlen(text);

	CLISTMENUITEM mi = { 0 };
	mi.pszService = text;

	HGENMENU hRoot = Menu_GetProtocolRoot(m_szModuleName);
	if (hRoot == NULL) {
		mi.popupPosition = 500085000;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTHANDLE | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | (isOnline() ? 0 : CMIF_GRAYED);
		mi.icolibItem = GetIconHandle("whatsApp");
		mi.ptszName = m_tszUserName;
		hRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else {
		if (m_hMenuRoot)
			CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		m_hMenuRoot = NULL;
	}

	mi.flags = CMIF_ROOTHANDLE | (isOnline() ? 0 : CMIF_GRAYED);
	mi.position = 201001;

	CreateProtoService("/CreateGroup", &WhatsAppProto::OnCreateGroup);
	mir_strcpy(tDest, "/CreateGroup");
	mi.hParentMenu = hRoot;
	mi.pszName = LPGEN("Create group");
	mi.icolibItem = GetIconHandle("createGroup");
	m_hMenuCreateGroup = Menu_AddProtoMenuItem(&mi);

	return 0;
}

void WhatsAppProto::ToggleStatusMenuItems(BOOL bEnable)
{
	int flags = (bEnable) ? 0 : CMIF_GRAYED;
	Menu_ModifyItem(m_hMenuRoot, NULL, INVALID_HANDLE_VALUE, flags);
	Menu_ModifyItem(m_hMenuCreateGroup, NULL, INVALID_HANDLE_VALUE, flags);
}
