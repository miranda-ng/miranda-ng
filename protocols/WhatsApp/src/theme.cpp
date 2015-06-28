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
	HGENMENU hRoot = Menu_GetProtocolRoot(m_szModuleName);
	if (hRoot == NULL) {
		TMO_MenuItem mi = { 0 };
		mi.position = 500085000;
		mi.flags = CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | (isOnline() ? 0 : CMIF_GRAYED);
		mi.hIcolibItem = GetIconHandle("whatsApp");
		mi.name.t = m_tszUserName;
		hRoot = m_hMenuRoot = Menu_AddProtoMenuItem(&mi);
	}
	else {
		if (m_hMenuRoot) {
			Menu_RemoveItem(m_hMenuRoot);
			m_hMenuRoot = NULL;
		}
	}

	TMO_MenuItem mi = { 0 };
	mi.flags = (isOnline() ? 0 : CMIF_GRAYED);
	mi.position = 201001;

	mi.pszService = "/CreateGroup";
	CreateProtoService(mi.pszService, &WhatsAppProto::OnCreateGroup);
	mi.root = hRoot;
	mi.name.a = LPGEN("Create group");
	mi.hIcolibItem = GetIconHandle("createGroup");
	m_hMenuCreateGroup = Menu_AddProtoMenuItem(&mi, m_szModuleName);
	return 0;
}

void WhatsAppProto::ToggleStatusMenuItems(bool bEnable)
{
	Menu_EnableItem(m_hMenuRoot, bEnable);
	Menu_EnableItem(m_hMenuCreateGroup, bEnable);
}
