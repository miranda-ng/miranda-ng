#include "stdafx.h"

static HGENMENU hmiHistory, hmiEmpty;

INT_PTR svcEmptyHistory(WPARAM hContact, LPARAM)
{
	if (IDYES != MessageBoxW(nullptr, TranslateT("Are you sure to remove all events from history?"), _T(MODULETITLE), MB_YESNOCANCEL | MB_ICONQUESTION))
		return 1;

	DB::ECPTR pCursor(DB::Events(hContact));
	while (pCursor.FetchNext())
		pCursor.DeleteEvent();
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	bool bShow = (db_event_first(hContact) != 0);
	Menu_ShowItem(hmiEmpty, bShow);
	Menu_ShowItem(hmiHistory, bShow);
	return 0;
}

void InitMenus()
{
	CMenuItem mi(&g_plugin);

	// Contact menu items
	SET_UID(mi, 0xc20d7a69, 0x7607, 0x4aad, 0xa7, 0x42, 0x10, 0x86, 0xfb, 0x32, 0x49, 0x21);
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	mi.name.a = LPGEN("User history");
	mi.position = 1000090000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	hmiHistory = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, svcShowNewstory);

	SET_UID(mi, 0x0d4306aa, 0xe31e, 0x46ee, 0x89, 0x88, 0x3a, 0x2e, 0x05, 0xa6, 0xf3, 0xbc);
	mi.pszService = "Newstory/EmptyHistory";
	mi.name.a = LPGEN("Empty history");
	mi.position = 1000090001;
	mi.hIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	hmiEmpty = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, svcEmptyHistory);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
}
