/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "clc.h"

extern HANDLE hGroupChangeEvent;

static int GetContactStatus(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return ID_STATUS_OFFLINE;
	return db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
}

void fnLoadContactTree(void)
{
	int hideOffline = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		int status = GetContactStatus(hContact);
		if ((!hideOffline || status != ID_STATUS_OFFLINE) && !db_get_b(hContact, "CList", "Hidden", 0))
			cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(GetContactProto(hContact), status, hContact));
	}
	Clist_EndRebuild();
}

INT_PTR ContactChangeGroup(WPARAM wParam, LPARAM lParam)
{
	CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), NULL, NULL };

	if (lParam == NULL)
		db_unset(wParam, "CList", "Group");
	else {
		grpChg.pszNewName = Clist_GroupGetName(lParam, NULL);
		db_set_ws(wParam, "CList", "Group", grpChg.pszNewName);
	}

	NotifyEventHooks(hGroupChangeEvent, wParam, (LPARAM)&grpChg);
	return 0;
}

int fnSetHideOffline(int iValue)
{
	if (iValue == -1) // invert the current value
		iValue = !db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);

	switch (iValue) {
	case 0:
	case 1:
		db_set_b(NULL, "CList", "HideOffline", iValue);
		break;

	default:
		return -1;
	}
	cli.pfnLoadContactTree();
	return iValue;
}
