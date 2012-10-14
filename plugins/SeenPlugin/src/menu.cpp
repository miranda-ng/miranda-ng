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

File name      : $URL: http://svn.berlios.de/svnroot/repos/mgoodies/trunk/lastseen-mod/menu.c $
Revision       : $Rev: 1570 $
Last change on : $Date: 2007-12-30 01:30:07 +0300 (Вс, 30 дек 2007) $
Last change by : $Author: y_b $
*/
#include "seen.h"

HANDLE hmenuitem=NULL, hLSUserDet = NULL, hBuildMenu = NULL;

void ShowHistory(HANDLE hContact, BYTE isAlert);
void InitHistoryDialog(void);

/*
Handles the messages sent by clicking the contact's menu item
*/
INT_PTR MenuitemClicked(WPARAM wparam,LPARAM lparam)
{
	ShowHistory((HANDLE)wparam, 0);
	return 0;
}

int BuildContactMenu(WPARAM wparam,LPARAM lparam)
{
	int id = -1, isetting;
	HANDLE hContact = (HANDLE)wparam;
	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	CLISTMENUITEM cmi = { sizeof(cmi) };
	if (!IsWatchedProtocol(szProto) || !db_get_b(NULL,S_MOD,"MenuItem",1))
		cmi.flags = CMIM_FLAGS | CMIF_HIDDEN | CMIF_TCHAR;
	else {
		cmi.flags = CMIM_NAME | CMIM_FLAGS | CMIM_ICON | CMIF_TCHAR;
		cmi.hIcon = NULL;

		DBVARIANT dbv;
		if ( !DBGetContactSettingTString(NULL, S_MOD, "MenuStamp", &dbv)) {
			cmi.ptszName = ParseString(dbv.ptszVal, (HANDLE)wparam, 0);
			db_free(&dbv);
		}
		else cmi.ptszName = ParseString( _T(DEFAULT_MENUSTAMP), (HANDLE)wparam, 0);
		
		if ( !_tcscmp(cmi.ptszName, TranslateT("<unknown>"))) {	
			if ( IsWatchedProtocol(szProto))
				cmi.flags |= CMIF_GRAYED;
			else
				cmi.flags |= CMIF_HIDDEN;	
		}
		else if ( db_get_b(NULL, S_MOD, "ShowIcon",1)) {
			isetting = db_get_w(hContact, S_MOD, "StatusTriger", -1);
			cmi.hIcon = LoadSkinnedProtoIcon(szProto, isetting|0x8000);
		}
	}

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hmenuitem, (LPARAM)&cmi);
	return 0;
}

void InitMenuitem()
{
	hLSUserDet = CreateServiceFunction("LastSeenUserDetails", MenuitemClicked);

	CLISTMENUITEM cmi = { sizeof(cmi) };
	cmi.position = -0x7FFFFFFF;
	cmi.flags = CMIF_TCHAR;
	cmi.ptszName = LPGENT("<none>");
	cmi.pszService = "LastSeenUserDetails";
	hmenuitem = Menu_AddContactMenuItem(&cmi);
	
	hBuildMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU,BuildContactMenu);

	InitHistoryDialog();
}

void UninitMenuitem()
{
	DestroyServiceFunction(hLSUserDet);
	UnhookEvent(hBuildMenu);
}
