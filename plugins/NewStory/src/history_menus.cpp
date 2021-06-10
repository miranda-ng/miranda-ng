#include "stdafx.h"

static HGENMENU hmiHistory;

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hmiHistory, db_event_first(hContact) != 0);
	return 0;
}

void InitMenus()
{
	// Contact menu items
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xc20d7a69, 0x7607, 0x4aad, 0xa7, 0x42, 0x10, 0x86, 0xfb, 0x32, 0x49, 0x21);
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	mi.name.a = LPGEN("User history");
	mi.position = 1000090000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	hmiHistory = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, svcShowNewstory);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
}
