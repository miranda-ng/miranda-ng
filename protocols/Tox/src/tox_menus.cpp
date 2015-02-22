#include "common.h"

int CToxProto::OnInitStatusMenu()
{
	char text[MAX_PATH];
	mir_strcpy(text, m_szModuleName);
	char *tDest = text + strlen(text);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszService = text;

	HGENMENU hStatusMunuRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (!hStatusMunuRoot)
	{
		mi.ptszName = m_tszUserName;
		mi.position = -1999901006;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		//mi.icolibItem = CToxProto::GetSkinIconHandle("main");
		hStatusMunuRoot = /*m_hMenuRoot = */Menu_AddProtoMenuItem(&mi);
	}
	else
	{
		//if (m_hMenuRoot)
		//	CallService(MO_REMOVEMENUITEM, (WPARAM)m_hMenuRoot, 0);
		//m_hMenuRoot = NULL;
	}

	mi.hParentMenu = hStatusMunuRoot;
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;

	// Create chat room command
	mir_strcpy(tDest, "/CreateChatRoom");
	CreateProtoService(tDest, &CToxProto::OnCreateChatRoom);
	mi.ptszName = LPGENT("Create group chat");
	mi.position = 200000;// +SMI_CHAT_CREATE;
	//mi.icolibItem = CToxProto::GetSkinIconHandle("conference");
	Menu_AddProtoMenuItem(&mi);

	return 0;
}