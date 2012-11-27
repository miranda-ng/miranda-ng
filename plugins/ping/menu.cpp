#include "common.h"
#include "menu.h"

HANDLE hMenuDisable, hMenuGraph, hMenuEdit;
HANDLE hEventMenuBuild;

/*
int MenuBuild(WPARAM wParam, LPARAM lParam) {
	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);

	menu.flags = CMIM_NAME | CMIM_ICON;
	bool disable = DBGetContactSettingWord((HANDLE)wParam, PLUG, "Status", ID_STATUS_OFFLINE) != options.off_status;
	if(disable) {
		menu.hIcon = hIconDisabled;
		menu.pszName = Translate("Disable");
	} else {
		menu.hIcon = hIconEnabled;
		menu.pszName = Translate("Enable");
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDisable, (LPARAM)&menu);

	// hide graph menu item if window displayed
	//menu.flags = CMIM_FLAGS | (DBGetContactSettingDword((HANDLE)wParam, PLUG, "WindowHandle", 0) == 0 ? 0 : CMIF_HIDDEN);
	//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuGraph, (LPARAM)&menu);

	return 0;
}
*/

void InitMenus() {

	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);

	// main menu
	menu.flags = CMIF_TCHAR;
	menu.popupPosition = 500099900;
	menu.ptszPopupName = LPGENT("PING");
	menu.cbSize = sizeof( menu );
	menu.position = 2000060000;
	menu.hIcon = hIconResponding;
	menu.ptszName = LPGENT("Enable All Pings");
	menu.pszService = PLUG "/EnableAll";
	Menu_AddMainMenuItem(&menu);

	menu.popupPosition = 500299901;
	menu.cbSize = sizeof( menu );
	menu.position = 2000060001;
	menu.popupPosition = 0;
	menu.hIcon = hIconDisabled;
	menu.ptszName = LPGENT("Disable All Pings");
	menu.pszService = PLUG "/DisableAll";
	Menu_AddMainMenuItem(&menu);

   /*
	// list items
	menu.flags = 0;
	menu.popupPosition = 0;
	menu.pszPopupName = 0;
	menu.cbSize = sizeof( menu );
	menu.position =-300100;
	//menu.popupPosition = 0;
	menu.hIcon = hIconDisabled;
	menu.pszName = Translate( "Disable" );
	menu.pszService = PLUG "/ToggleEnabled";
	menu.pszContactOwner = PLUG;
	hMenuDisable = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 0;
	menu.pszPopupName = 0;
	menu.cbSize = sizeof( menu );
	menu.position =-300090;
	//menu.popupPosition = 0;
	menu.hIcon = hIconResponding
	menu.pszName = Translate( "Graph" );
	menu.pszService = PLUG "/ShowGraph";
	menu.pszContactOwner = PLUG;
	hMenuGraph = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 0;
	menu.pszPopupName = 0;
	menu.cbSize = sizeof( menu );
	menu.position =-300080;
	//menu.popupPosition = 0;
	menu.hIcon = hIconResponding;
	menu.pszName = Translate( "Edit..." );
	menu.pszService = PLUG "/Edit";
	menu.pszContactOwner = PLUG;
	hMenuGraph = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );

	hEventMenuBuild = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MenuBuild);
	*/
}
