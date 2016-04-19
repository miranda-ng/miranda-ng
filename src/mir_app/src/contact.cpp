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
	CallService(MS_CLUI_LISTBEGINREBUILD, 0, 0);
	for (int i = 1;; i++) {
		if (Clist_GroupGetName(i, NULL) == NULL)
			break;
		CallService(MS_CLUI_GROUPADDED, i, 0);
	}

	int hideOffline = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		int status = GetContactStatus(hContact);
		if ((!hideOffline || status != ID_STATUS_OFFLINE) && !db_get_b(hContact, "CList", "Hidden", 0))
			cli.pfnChangeContactIcon(hContact, cli.pfnIconFromStatusMode(GetContactProto(hContact), status, hContact));
	}
	CallService(MS_CLUI_LISTENDREBUILD, 0, 0);
}

INT_PTR ContactChangeGroup(WPARAM wParam, LPARAM lParam)
{
	CLISTGROUPCHANGE grpChg = { sizeof(CLISTGROUPCHANGE), NULL, NULL };

	if (lParam == NULL)
		db_unset(wParam, "CList", "Group");
	else {
		grpChg.pszNewName = Clist_GroupGetName(lParam, NULL);
		db_set_ts(wParam, "CList", "Group", grpChg.pszNewName);
	}

	NotifyEventHooks(hGroupChangeEvent, wParam, (LPARAM)&grpChg);
	return 0;
}

int fnSetHideOffline(WPARAM wParam, LPARAM)
{
	switch ((int)wParam) {
	case 0:
		db_set_b(NULL, "CList", "HideOffline", 0);
		break;
	case 1:
		db_set_b(NULL, "CList", "HideOffline", 1);
		break;
	case -1:
		db_set_b(NULL, "CList", "HideOffline", !db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT));
		break;
	}
	cli.pfnLoadContactTree();
	return 0;
}
