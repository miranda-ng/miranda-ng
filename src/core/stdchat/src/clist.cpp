/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "chat.h"

HANDLE CList_AddRoom(const char* pszModule, const TCHAR* pszRoom, const TCHAR* pszDisplayName, int iType)
{
	HANDLE hContact = CList_FindRoom(pszModule, pszRoom);
	DBVARIANT dbv;
	TCHAR pszGroup[50];

	*pszGroup = '\0';
	if ( !db_get_ts( NULL, "Chat", "AddToGroup", &dbv )) {
		if ( lstrlen( dbv.ptszVal ) > 0 )
			lstrcpyn( pszGroup, dbv.ptszVal, 50);
		db_free(&dbv);
	}
	else lstrcpyn( pszGroup, _T("Chat rooms"), 50);

	if ( pszGroup[0] )
		CList_CreateGroup( pszGroup );

	if ( hContact ) { //contact exist, make sure it is in the right group
		DBVARIANT dbv;
		DBVARIANT dbv2;
		char str[50];
		int i;

		if ( pszGroup[0] ) {
			for (i = 0;; i++) {
				_itoa( i, str, 10 );
				if ( db_get_ts( NULL, "CListGroups", str, &dbv )) {
					db_set_ts(hContact, "CList", "Group", pszGroup);
					goto END_GROUPLOOP;
				}

				if ( !db_get_ts( hContact, "CList", "Group", &dbv2 )) {
					if ( dbv.ptszVal[0] != '\0' && dbv2.ptszVal[0] != '\0' && !lstrcmpi( dbv.ptszVal + 1, dbv2.ptszVal )) {
						db_free( &dbv );
						db_free( &dbv2 );
						goto END_GROUPLOOP;
					}
					db_free(&dbv2);
				}
				db_free(&dbv);
		}	}

END_GROUPLOOP:
		db_set_w( hContact, pszModule, "Status", ID_STATUS_OFFLINE );
		db_set_ts(hContact, pszModule, "Nick", pszDisplayName );
		return hContact;
	}

	// here we create a new one since no one is to be found
	if (( hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0)) == NULL )
		return NULL;

	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) pszModule );
	if ( pszGroup && lstrlen( pszGroup ) > 0 )
		db_set_ts(hContact, "CList", "Group", pszGroup );
	else
		db_unset( hContact, "CList", "Group" );
	db_set_ts( hContact, pszModule, "Nick", pszDisplayName );
	db_set_ts( hContact, pszModule, "ChatRoomID", pszRoom );
	db_set_b( hContact, pszModule, "ChatRoom", (BYTE)iType );
	db_set_w( hContact, pszModule, "Status", ID_STATUS_OFFLINE );
	return hContact;
}

BOOL CList_SetOffline(HANDLE hContact, BOOL bHide)
{
	if ( hContact ) {
		char* szProto = GetContactProto(hContact);
		int i = db_get_b(hContact, szProto, "ChatRoom", 0);
		db_set_w(hContact, szProto,"ApparentMode",(LPARAM) 0);
		db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		return TRUE;
	}

	return FALSE;
}

BOOL CList_SetAllOffline(BOOL bHide, const char *pszModule)
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *szProto = GetContactProto(hContact);
		if ( MM_FindModule( szProto )) {
			if ( !pszModule || ( pszModule && !strcmp( pszModule, szProto ))) {
				int i = db_get_b(hContact, szProto, "ChatRoom", 0);
				if ( i != 0 ) {
					db_set_w(hContact, szProto,"ApparentMode",(LPARAM)(WORD) 0);
					db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
	}	}	}	}

	return TRUE;
}

int CList_RoomDoubleclicked( WPARAM wParam, LPARAM lParam )
{
	DBVARIANT dbv;
	char *szProto;
	BOOL bRedrawFlag = FALSE;

	HANDLE hContact = (HANDLE)wParam;
	if ( !hContact )
		return 0;

	szProto = GetContactProto(hContact);
	if ( MM_FindModule(szProto)) {
		if ( db_get_b( hContact, szProto, "ChatRoom", 0 ) == 0 )
			return 0;

		if ( !db_get_ts( hContact, szProto, "ChatRoomID", &dbv )) {
			SESSION_INFO* si = SM_FindSession( dbv.ptszVal, szProto );
			if ( si ) {
				// is the "toggle visibility option set, so we need to close the window?
				if (si->hWnd != NULL
					&& db_get_b(NULL, "Chat", "ToggleVisibility", 0)==1
					&& !CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0)
					&& IsWindowVisible(si->hWnd)
					&& !IsIconic(si->hWnd))
				{
					if (g_Settings.TabsEnable)
						SendMessage(si->hWnd, GC_REMOVETAB, 1, (LPARAM) si );
					else
						PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
					db_free(&dbv);
					return 1;
				}
				ShowRoom(si, WINDOW_VISIBLE, TRUE);
			}
			db_free(&dbv);
			return 1;
	}	}

	return 0;
}

INT_PTR CList_EventDoubleclicked(WPARAM wParam,LPARAM lParam)
{
	return CList_RoomDoubleclicked((WPARAM) ((CLISTEVENT*)lParam)->hContact,(LPARAM) 0);
}

INT_PTR CList_JoinChat(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( hContact ) {
		char* szProto = GetContactProto(hContact);
		if ( szProto ) {
			if ( db_get_w( hContact, szProto, "Status", 0 ) == ID_STATUS_OFFLINE )
				CallProtoService( szProto, PS_JOINCHAT, wParam, lParam );
			else
				CList_RoomDoubleclicked( wParam, 0 );
	}	}

	return 0;
}

INT_PTR CList_LeaveChat(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( hContact ) {
		char* szProto = GetContactProto(hContact);
		if ( szProto )
			CallProtoService( szProto, PS_LEAVECHAT, wParam, lParam );
	}
	return 0;
}

int CList_PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact == NULL)
		return 0;

	bool bEnabled = false;
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		// display this menu item only for chats
		if ( db_get_b(hContact, szProto, "ChatRoom", 0)) {
			// still hide it for offline protos
			if ( CallProtoService(szProto, PS_GETSTATUS, 0, 0) != ID_STATUS_OFFLINE ) {
				CLISTMENUITEM mi = { sizeof(mi) };
				mi.flags = CMIM_NAME;
				if ( db_get_w( hContact, szProto, "Status", 0 ) == ID_STATUS_OFFLINE )
					mi.pszName = ( char* )LPGEN("Join chat");
				else
					mi.pszName = ( char* )LPGEN("Open chat window");
				Menu_ModifyItem(hJoinMenuItem, &mi);
				bEnabled = true;
	}	}	}

	Menu_ShowItem(hJoinMenuItem, bEnabled);
	Menu_ShowItem(hLeaveMenuItem, bEnabled);
	return 0;
}

INT_PTR CList_PrebuildContactMenuSvc(WPARAM wParam, LPARAM lParam)
{
	return CList_PrebuildContactMenu(wParam, lParam);
}


void CList_CreateGroup(TCHAR* group)
{
	int i;
	char str[50];
	TCHAR name[256];
	DBVARIANT dbv;

	if ( !group )
		return;

	for (i = 0;; i++)
	{
		_itoa(i, str, 10);
		if ( db_get_ts( NULL, "CListGroups", str, &dbv ))
			break;

		if ( dbv.pszVal[0] != '\0' && !lstrcmpi(dbv.ptszVal + 1, group)) {
			db_free(&dbv);
			return;
		}

		db_free(&dbv);
	}

	name[0] = 1 | GROUPF_EXPANDED;
	_tcsncpy(name + 1, group, SIZEOF(name) - 1);
	name[ lstrlen(group) + 1] = '\0';
	db_set_ts(NULL, "CListGroups", str, name);
	CallService(MS_CLUI_GROUPADDED, i + 1, 0);
}

BOOL CList_AddEvent(HANDLE hContact, HICON hIcon, HANDLE hEvent, int type, TCHAR* fmt, ... )
{
	CLISTEVENT cle = {0};
	va_list marker;
	TCHAR szBuf[4096];

	if (!fmt || !fmt[0] || _tcslen(fmt) > 2000)
		return FALSE;

	va_start(marker, fmt);
	mir_vsntprintf(szBuf, SIZEOF(szBuf), fmt, marker);
	va_end(marker);

	cle.cbSize = sizeof(cle);
	cle.hContact = hContact;
	cle.hDbEvent = hEvent;
	cle.flags = type | CLEF_TCHAR;
	cle.hIcon = hIcon;
	cle.pszService = "GChat/DblClickEvent" ;
	cle.ptszTooltip = TranslateTS(szBuf);
	if (type) {
		if (!CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0))
			CallService(MS_CLIST_ADDEVENT, (WPARAM) hContact, (LPARAM) &cle);
	}
	else {
		if (CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM)hEvent);
		CallService(MS_CLIST_ADDEVENT, (WPARAM)hContact, (LPARAM)&cle);
	}
	return TRUE;
}

HANDLE CList_FindRoom (const char* pszModule, const TCHAR* pszRoom)
{
	for (HANDLE hContact = db_find_first(pszModule); hContact; hContact = db_find_next(hContact, pszModule)) {
		if ( !db_get_b(hContact, pszModule, "ChatRoom", 0))
			continue;

		DBVARIANT dbv;
		if ( !db_get_ts( hContact, pszModule, "ChatRoomID", &dbv )) {
			if ( !lstrcmpi(dbv.ptszVal, pszRoom)) {
				db_free(&dbv);
				return hContact;
			}
			db_free(&dbv);
		}
	}

	return 0;
}

int WCCmp(TCHAR* wild, TCHAR* string)
{
	TCHAR *cp, *mp;
	if ( wild == NULL || !wild[0] || string == NULL || !string[0])
		return 0;

	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?'))
			return 0;

		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild)
				return 1;

			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		}
		else {
			wild = mp;
			string = cp++;
	}	}

	while (*wild == '*')
		wild++;

	return !*wild;
}
