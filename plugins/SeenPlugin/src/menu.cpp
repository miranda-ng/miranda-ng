/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

HANDLE   hLSUserDet = NULL;
HGENMENU hmenuitem = NULL;

void InitHistoryDialog(void);

/////////////////////////////////////////////////////////////////////////////////////////
// Handles the messages sent by clicking the contact's menu item

INT_PTR MenuitemClicked(WPARAM hContact, LPARAM)
{
	ShowHistory(hContact, 0);
	return 0;
}

int BuildContactMenu(WPARAM hContact, LPARAM)
{
	char *szProto = GetContactProto(hContact);
	if (!IsWatchedProtocol(szProto) || db_get_b(hContact, szProto, "ChatRoom", false) || !db_get_b(NULL, S_MOD, "MenuItem", 1)) {
		Menu_ShowItem(hmenuitem, false);
		return 0;
	}

	LPCTSTR ptszName;
	ptrW tszStamp(db_get_wsa(NULL, S_MOD, "MenuStamp"));
	if (tszStamp != NULL)
		ptszName = ParseString(tszStamp , hContact);
	else
		ptszName = ParseString(DEFAULT_MENUSTAMP, hContact);

	int flags = 0;
	HICON hIcon = NULL;
	if (!mir_wstrcmp(ptszName, TranslateT("<unknown>"))) {
		if (IsWatchedProtocol(szProto))
			flags |= CMIF_GRAYED;
		else
			flags |= CMIF_HIDDEN;
	}
	else if (db_get_b(NULL, S_MOD, "ShowIcon", 1)) {
		int isetting = db_get_w(hContact, S_MOD, "StatusTriger", -1);
		hIcon = Skin_LoadProtoIcon(szProto, isetting | 0x8000);
	}
	Menu_ModifyItem(hmenuitem, ptszName, hIcon, flags);
	return 0;
}

void InitMenuitem()
{
	CreateServiceFunction("LastSeenUserDetails", MenuitemClicked);

	CMenuItem mi;
	SET_UID(mi, 0x7d9b9518, 0x85fa, 0x40f1, 0xb5, 0x3e, 0x3e, 0x3c, 0x68, 0xae, 0xda, 0x90);
	mi.position = -0x7FFFFFFF;
	mi.name.a = LPGEN("Last seen");
	mi.pszService = "LastSeenUserDetails";
	hmenuitem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildContactMenu);

	InitHistoryDialog();
}
