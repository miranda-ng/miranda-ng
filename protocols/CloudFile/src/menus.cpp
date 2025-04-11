#include "stdafx.h"

static HGENMENU hContactMenu;

void InitializeMenus()
{
	if (hContactMenu)
		Menu_RemoveItem(hContactMenu);

	// no money, no honey
	if (g_arServices.getCount() == 0)
		return;

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x93d4495b, 0x259b, 0x4fba, 0xbc, 0x14, 0xf9, 0x46, 0x2c, 0xda, 0xfc, 0x6d);
	mi.position = -2000019999;

	CMStringA szService;
	if (g_arServices.getCount() == 1) {
		auto *S = g_arServices[0];

		szService.Format("%s%s", S->m_szModuleName, PS_UPLOAD);
		CMStringW wszTitle(FORMAT, L"%s %s", TranslateT("Upload to"), S->m_tszUserName);
		mi.flags |= CMIF_UNICODE;
		mi.pszService = szService;
		mi.name.w = wszTitle.GetBuffer();
		mi.hIcolibItem = g_plugin.getIconHandle(S->GetIconId());
		hContactMenu = Menu_AddContactMenuItem(&mi);
	}
	else {
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_UPLOAD);
		mi.name.a = LPGEN("Upload to...");
		hContactMenu = Menu_AddContactMenuItem(&mi);

		int i = 1000;
		for (auto &S : g_arServices) {
			szService.Format("%s%s", S->m_szModuleName, PS_UPLOAD);

			CMenuItem mi2(S->GetId());
			mi2.root = hContactMenu;
			mi2.flags = CMIF_SYSTEM | CMIF_UNICODE;
			mi2.name.w = S->m_tszUserName;
			mi2.position = i++;
			mi2.pszService = szService;
			mi2.hIcolibItem = g_plugin.getIconHandle(S->GetIconId());
			Menu_AddContactMenuItem(&mi2);
		}
	}
}

int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hContactMenu, CanSendToContact(hContact));
	return 0;
}
