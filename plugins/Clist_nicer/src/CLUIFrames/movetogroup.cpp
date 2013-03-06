#include <commonheaders.h>

HANDLE hPriorityItem = 0, hFloatingItem = 0;

static int OnContactMenuBuild(WPARAM wParam,LPARAM lParam)
{
	BYTE bSetting = cfg::getByte("CList", "flt_enabled", 0);
	if (bSetting && !hFloatingItem) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position=200000;
		mi.pszPopupName=(char *)-1;
		mi.pszService="CList/SetContactFloating";
		mi.pszName=LPGEN("&Floating Contact");
		if (SendMessage(pcli->hwndContactTree, CLM_QUERYFLOATINGCONTACT, wParam, 0))
			mi.flags = CMIF_CHECKED;

		hFloatingItem = Menu_AddContactMenuItem(&mi);
	}
	else if ( !bSetting && hFloatingItem) {
		CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hFloatingItem, 0);
		hFloatingItem = 0;
	}
	else {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_FLAGS;
		if (SendMessage(pcli->hwndContactTree, CLM_QUERYFLOATINGCONTACT, wParam, 0))
			mi.flags |= CMIF_CHECKED;

		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hFloatingItem, (LPARAM)&mi);
	}
	return 0;
}

int MTG_OnmodulesLoad(WPARAM wParam,LPARAM lParam)
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU,OnContactMenuBuild);
	return 0;
}
