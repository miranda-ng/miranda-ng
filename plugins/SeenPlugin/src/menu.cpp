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

HANDLE   hLSUserDet = nullptr;
HGENMENU hmenuitem = nullptr;

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
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (!IsWatchedProtocol(szProto) || Contact::IsGroupChat(hContact, szProto) || !g_plugin.getByte("MenuItem", 1)) {
		Menu_ShowItem(hmenuitem, false);
		return 0;
	}

	ptrW tszStamp(g_plugin.getWStringA("MenuStamp"));
	CMStringW wszName(ParseString(tszStamp ? tszStamp : DEFAULT_MENUSTAMP, hContact));

	int flags = 0;
	HANDLE hIcon = nullptr;
	if (wszName == TranslateT("<unknown>")) {
		if (IsWatchedProtocol(szProto))
			flags |= CMIF_GRAYED;
		else
			flags |= CMIF_HIDDEN;
	}
	else if (g_plugin.getByte("ShowIcon", 1)) {
		int isetting = g_plugin.getWord(hContact, "StatusTriger", -1);
		hIcon = Skin_GetProtoIcon(szProto, isetting | 0x8000);
	}
	Menu_ModifyItem(hmenuitem, wszName, hIcon, flags);
	return 0;
}

void InitMenuitem()
{
	CreateServiceFunction("LastSeenUserDetails", MenuitemClicked);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x7d9b9518, 0x85fa, 0x40f1, 0xb5, 0x3e, 0x3e, 0x3c, 0x68, 0xae, 0xda, 0x90);
	mi.position = -0x7FFFFFFF;
	mi.name.a = LPGEN("Last seen");
	mi.pszService = "LastSeenUserDetails";
	hmenuitem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildContactMenu);

	InitHistoryDialog();
}
