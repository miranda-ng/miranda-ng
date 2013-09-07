/*
astyle --force-indent=tab=4 --brackets=linux --indent-switches
		--pad=oper --one-line=keep-blocks  --unpad=paren

Chat module plugin for Miranda NG

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

Group chat module: contact list support functions. Add chat rooms as
contacts and handle default action for such contacts.

This code was taken in almost unmodified from from the chat.dll
plugin, originally written by Jörgen Persson

*/

/*
 * handles interaction of the group chat implementation with the contact list plugin
 */

#include "..\commonheaders.h"

char *szChatIconString = "chaticon";

HANDLE CList_AddRoom(const char* pszModule, const TCHAR* pszRoom, const TCHAR* pszDisplayName, int iType)
{
	TCHAR pszGroup[50]; *pszGroup = '\0';

	ptrT groupName( db_get_tsa(NULL, "Chat", "AddToGroup"));
	if (groupName)
		_tcsncpy_s(pszGroup, SIZEOF(pszGroup), groupName, _TRUNCATE);
	else
		_tcscpy(pszGroup, _T("Chat rooms"));

	if (pszGroup[0])
		CList_CreateGroup(pszGroup);

	HANDLE hContact = CList_FindRoom(pszModule, pszRoom);
	if (hContact)
		return hContact;

	// here we create a new one since no one is to be found
	if ((hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0)) == NULL)
		return NULL;

	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)pszModule);
	if (pszGroup && lstrlen(pszGroup) > 0)
		CallService(MS_CLIST_CONTACTCHANGEGROUP, (WPARAM)hContact, (LPARAM)g_Settings.hGroup);
	else
		db_unset(hContact, "CList", "Group");

	db_set_ts(hContact, pszModule, "Nick", pszDisplayName);
	db_set_ts(hContact, pszModule, "ChatRoomID", pszRoom);
	db_set_b(hContact, pszModule, "ChatRoom", (BYTE)iType);
	db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
	return hContact;
}

BOOL CList_SetOffline(HANDLE hContact, BOOL bHide)
{
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		if (szProto == NULL)
			return FALSE;

		db_set_w(hContact, szProto, "ApparentMode", 0);
		db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		return TRUE;
	}
	return FALSE;
}

BOOL CList_SetAllOffline(BOOL bHide, const char *pszModule)
{
	for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *szProto = GetContactProto(hContact);
		if (MM_FindModule(szProto)) {
			if (!pszModule || (pszModule && !strcmp(pszModule, szProto))) {
				int i = db_get_b(hContact, szProto, "ChatRoom", 0);
				if (i != 0) {
					db_set_w(hContact, szProto, "ApparentMode", (LPARAM)(WORD) 0);
					db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
				}
			}
		}
	}
	return TRUE;
}

int CList_RoomDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (!hContact)
		return 0;

	char *szProto = GetContactProto(hContact);
	if ( !MM_FindModule(szProto) || !db_get_b(hContact, szProto, "ChatRoom", 0))
		return 0;

	ptrT roomid( db_get_tsa(hContact, szProto, "ChatRoomID"));
	if (roomid == NULL)
		return 0;

	SESSION_INFO *si = SM_FindSession(roomid, szProto);
	if (si) {
		// is the "toggle visibility option set, so we need to close the window?
		if (si->hWnd != NULL &&
				M.GetByte("Chat", "ToggleVisibility", 0) == 1 &&
				!CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0) &&
				IsWindowVisible(si->hWnd) && !IsIconic(si->pContainer->hwnd))
		{
			PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
			return 1;
		}

		ShowRoom(si, WINDOW_VISIBLE, TRUE);
		if (lParam) {
			SendMessage(si->hWnd, DM_ACTIVATEME, 0, 0);
			if (si->dat)
				SetForegroundWindow(si->dat->pContainer->hwnd);
		}
	}
	return 1;
}

INT_PTR CList_EventDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	return CList_RoomDoubleclicked((WPARAM)((CLISTEVENT*)lParam)->hContact, 0);
}

INT_PTR CList_JoinChat(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		if (szProto) {
			if ( db_get_w(hContact, szProto, "Status", 0) == ID_STATUS_OFFLINE)
				CallProtoService(szProto, PS_JOINCHAT, wParam, lParam);
			else
				CList_RoomDoubleclicked(wParam, 0);
		}
	}

	return 0;
}

INT_PTR CList_LeaveChat(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		if (szProto)
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
		if ( db_get_b(hContact, szProto, "ChatRoom", 0 )) {
			// still hide it for offline protos
			if (CallProtoService( szProto, PS_GETSTATUS, 0, 0 ) != ID_STATUS_OFFLINE) {
				CLISTMENUITEM clmi = { sizeof(clmi) };
				clmi.flags = CMIM_NAME;
				if (db_get_w( hContact, szProto, "Status", 0 ) == ID_STATUS_OFFLINE)
					clmi.pszName = (char*)LPGEN("Join chat");
				else
					clmi.pszName = (char*)LPGEN("Open chat window");
				Menu_ModifyItem(hJoinMenuItem, &clmi);
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
	if (!group)
		return;

	g_Settings.hGroup = Clist_GroupExists(group);
	if (g_Settings.hGroup == 0) {
		g_Settings.hGroup = Clist_CreateGroup(0, group);

		if (g_Settings.hGroup) {
			CallService(MS_CLUI_GROUPADDED, (WPARAM)g_Settings.hGroup, 0);
			CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM)g_Settings.hGroup, 1);
		}
	}
}

BOOL CList_AddEvent(HANDLE hContact, HICON Icon, HANDLE event, int type, const TCHAR* fmt, ...)
{
	if (!fmt || lstrlen(fmt) < 1 || lstrlen(fmt) > 2000)
		return FALSE;

	TCHAR* szBuf = (TCHAR*)alloca(4096 * sizeof(TCHAR));

	va_list marker;
	va_start(marker, fmt);
	mir_vsntprintf(szBuf, 4096, fmt, marker);
	va_end(marker);

	CLISTEVENT cle;
	cle.cbSize = sizeof(cle);
	cle.hContact = (HANDLE)hContact;
	cle.hDbEvent = (HANDLE)event;
	cle.flags = type + CLEF_TCHAR;
	cle.hIcon = Icon;
	cle.pszService = "GChat/DblClickEvent" ;
	cle.ptszTooltip = TranslateTS(szBuf);
	if (type) {
		if (!CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0))
			CallService(MS_CLIST_ADDEVENT, (WPARAM)hContact, (LPARAM)&cle);
	}
	else {
		if (CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM)"chaticon");
		CallService(MS_CLIST_ADDEVENT, (WPARAM)hContact, (LPARAM)&cle);
	}
	return TRUE;
}

HANDLE CList_FindRoom(const char* pszModule, const TCHAR* pszRoom)
{
	for (HANDLE hContact = db_find_first(pszModule); hContact; hContact = db_find_next(hContact, pszModule)) {
		if ( !db_get_b(hContact, pszModule, "ChatRoom", 0))
			continue;

		ptrT roomid( db_get_tsa(hContact, pszModule, "ChatRoomID"));
		if (roomid != NULL && !lstrcmpi(roomid, pszRoom))
			return hContact;
	}
	return 0;
}
