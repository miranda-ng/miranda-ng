#include <commonheaders.h>

HANDLE hOnCntMenuBuild;
HANDLE hPriorityItem = 0, hFloatingItem = 0;

static int OnContactMenuBuild(WPARAM wParam,LPARAM lParam)
{
	CLISTMENUITEM mi;
	BYTE bSetting;

	// Menu Item - Floating Contact
	bSetting = cfg::getByte("CList", "flt_enabled", 0);
	if (bSetting && !hFloatingItem)
	{
		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=200000;
		mi.pszPopupName=(char *)-1;
		mi.pszService="CList/SetContactFloating";
		mi.pszName=LPGEN("&Floating Contact");
		if (pcli)
		{
			if (SendMessage(pcli->hwndContactTree, CLM_QUERYFLOATINGCONTACT, wParam, 0))
				mi.flags=CMIF_CHECKED;
		}
		hFloatingItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);
	}
	else if (!bSetting && hFloatingItem)
	{
		CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hFloatingItem, 0);
		hFloatingItem = 0;
	}
	else
	{
		memset(&mi,0,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.flags=CMIM_FLAGS;
		if (pcli)
		{
			if (SendMessage(pcli->hwndContactTree, CLM_QUERYFLOATINGCONTACT, wParam, 0))
				mi.flags |= CMIF_CHECKED;
		}
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hFloatingItem, (LPARAM)&mi);
	}
	return 0;
}

int MTG_OnmodulesLoad(WPARAM wParam,LPARAM lParam)
{
	hOnCntMenuBuild=HookEvent(ME_CLIST_PREBUILDCONTACTMENU,OnContactMenuBuild);
	return 0;
}

int UnloadMoveToGroup(void)
{
	if (hOnCntMenuBuild)
		UnhookEvent(hOnCntMenuBuild);

	return 0;
}
