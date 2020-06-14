#include "stdafx.h"

static HGENMENU hmiHistory, hmiEmpty;

static INT_PTR svcEmptyHistory(WPARAM hContact, LPARAM)
{
	extern wchar_t wszDelete[];
	if (IDYES != MessageBoxW(nullptr, TranslateW(wszDelete), _T(MODULETITLE), MB_YESNOCANCEL | MB_ICONQUESTION))
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

	SET_UID(mi, 0xc20d7a69, 0x7607, 0x4aad, 0xa7, 0x42, 0x10, 0x86, 0xfb, 0x32, 0x49, 0x21);
	mi.pszService = "Newstory/EmptyHistory";
	mi.name.a = LPGEN("Empty history");
	mi.position = 1000090001;
	mi.hIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
	hmiEmpty = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, svcEmptyHistory);

	// Main menu items
	SET_UID(mi, 0xc20d7a69, 0x7607, 0x4aad, 0xa7, 0x42, 0x10, 0x86, 0xfb, 0x32, 0x49, 0x22);
	mi.pszService = "Newstory/System";
	mi.name.a = LPGEN("System history");
	mi.position = 1000090000;
	mi.hIcon = g_plugin.getIcon(ICO_NEWSTORY);
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, svcShowSystemNewstory);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
}
