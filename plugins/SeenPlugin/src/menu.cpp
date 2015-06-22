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

#include "seen.h"

HANDLE   hLSUserDet = NULL;
HGENMENU hmenuitem = NULL;

void InitHistoryDialog(void);

/////////////////////////////////////////////////////////////////////////////////////////
// Handles the messages sent by clicking the contact's menu item

INT_PTR MenuitemClicked(WPARAM wparam, LPARAM)
{
	ShowHistory((MCONTACT)wparam, 0);
	return 0;
}

int BuildContactMenu(WPARAM wparam, LPARAM)
{
	int id = -1, isetting;
	MCONTACT hContact = (MCONTACT)wparam;
	char *szProto = GetContactProto(hContact);

	if (!IsWatchedProtocol(szProto) || db_get_b(hContact, szProto, "ChatRoom", false) || !db_get_b(NULL, S_MOD, "MenuItem", 1)) {
		Menu_ShowItem(hmenuitem, false);
		return 0;
	}

	LPCTSTR ptszName;
	ptrT tszStamp(db_get_tsa(NULL, S_MOD, "MenuStamp"));
	if (tszStamp != NULL)
		ptszName = ParseString(tszStamp , (MCONTACT)wparam, 0);
	else
		ptszName = ParseString(DEFAULT_MENUSTAMP, (MCONTACT)wparam, 0);

	int flags = 0;
	HICON hIcon = NULL;
	if (!mir_tstrcmp(ptszName, TranslateT("<unknown>"))) {
		if (IsWatchedProtocol(szProto))
			flags |= CMIF_GRAYED;
		else
			flags |= CMIF_HIDDEN;
	}
	else if (db_get_b(NULL, S_MOD, "ShowIcon", 1)) {
		isetting = db_get_w(hContact, S_MOD, "StatusTriger", -1);
		hIcon = Skin_LoadProtoIcon(szProto, isetting | 0x8000);
	}
	Menu_ModifyItem(hmenuitem, ptszName, hIcon, flags);
	return 0;
}

void InitMenuitem()
{
	CreateServiceFunction("LastSeenUserDetails", MenuitemClicked);

	CLISTMENUITEM cmi = { 0 };
	cmi.position = -0x7FFFFFFF;
	cmi.flags = CMIF_TCHAR;
	cmi.ptszName = LPGENT("<none>");
	cmi.pszService = "LastSeenUserDetails";
	hmenuitem = Menu_AddContactMenuItem(&cmi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, BuildContactMenu);

	InitHistoryDialog();
}
