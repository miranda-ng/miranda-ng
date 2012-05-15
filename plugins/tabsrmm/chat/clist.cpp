/*
astyle --force-indent=tab=4 --brackets=linux --indent-switches
		--pad=oper --one-line=keep-blocks  --unpad=paren

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

Group chat module: contact list support functions. Add chat rooms as
contacts and handle default action for such contacts.

This code was taken in almost unmodified from from the chat.dll
plugin, originally written by Jörgen Persson

$Id: clist.c 10402 2009-07-24 00:35:21Z silvercircle $

*/

/*
 * handles interaction of the group chat implementation with the contact list plugin
 */

#include "../src/commonheaders.h"

char *szChatIconString = "chaticon";

extern HANDLE hJoinMenuItem, hLeaveMenuItem;

static HANDLE Clist_GroupExists(TCHAR *tszGroup)
{
	unsigned int i = 0;
	TCHAR*		 _t = 0;
	char		 str[10];
	INT_PTR		 result = 0;
	DBVARIANT	 dbv = {0};
	int			 match;

	do {
		_itoa(i, str, 10);
		result = M->GetTString(0, "CListGroups", str, &dbv);
		if(!result) {
			match = (!_tcscmp(tszGroup, &dbv.ptszVal[1]) && (lstrlen(tszGroup) == lstrlen(&dbv.ptszVal[1])));
			DBFreeVariant(&dbv);
			if(match)
				return((HANDLE)(i + 1));
		}
		i++;
	}
	while(result == 0);
	return(0);
}

HANDLE CList_AddRoom(const char* pszModule, const TCHAR* pszRoom, const TCHAR* pszDisplayName, int iType)
{
	HANDLE hContact = CList_FindRoom(pszModule, pszRoom);
	DBVARIANT dbv;
	TCHAR pszGroup[50];

	*pszGroup = '\0';
	if (!M->GetTString(NULL, "Chat", "AddToGroup", &dbv)) {
		if (lstrlen(dbv.ptszVal) > 0)
			lstrcpyn(pszGroup, dbv.ptszVal, 50);
		DBFreeVariant(&dbv);
	} else
		lstrcpyn(pszGroup, _T("Chat rooms"), 50);

	if (pszGroup[0])
		CList_CreateGroup(pszGroup);

	if (hContact)
		return hContact;

	// here we create a new one since no one is to be found

	if ((hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0)) == NULL)
		return NULL;

	CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) pszModule);
	if (pszGroup && lstrlen(pszGroup) > 0)
		CallService(MS_CLIST_CONTACTCHANGEGROUP, (WPARAM)hContact, (LPARAM)g_Settings.hGroup);
	else
		DBDeleteContactSetting(hContact, "CList", "Group");

	M->WriteTString(hContact, pszModule, "Nick", pszDisplayName);
	M->WriteTString(hContact, pszModule, "ChatRoomID", pszRoom);
	M->WriteByte(hContact, pszModule, "ChatRoom", (BYTE)iType);
	DBWriteContactSettingWord(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
	return hContact;
}

BOOL CList_SetOffline(HANDLE hContact, BOOL bHide)
{
	if (hContact) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		int i = M->GetByte(hContact, szProto, "ChatRoom", 0);
		DBWriteContactSettingWord(hContact, szProto, "ApparentMode", (LPARAM) 0);
		DBWriteContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		return TRUE;
	}
	return FALSE;
}

BOOL CList_SetAllOffline(BOOL bHide, const char *pszModule)
{
	HANDLE hContact;
	char* szProto;

	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (MM_FindModule(szProto)) {
			if (!pszModule || (pszModule && !strcmp(pszModule, szProto))) {
				int i = M->GetByte(hContact, szProto, "ChatRoom", 0);
				if (i != 0) {
					DBWriteContactSettingWord(hContact, szProto, "ApparentMode", (LPARAM)(WORD) 0);
					DBWriteContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE);
				}
			}
		}

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	return TRUE;
}

int CList_RoomDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *szProto;
	BOOL bRedrawFlag = FALSE;
	bool fCreate = false;

	HANDLE hContact = (HANDLE)wParam;
	if (!hContact)
		return 0;

	szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (MM_FindModule(szProto)) {
		if (M->GetByte(hContact, szProto, "ChatRoom", 0) == 0)
			return 0;

		if (!M->GetTString(hContact, szProto, "ChatRoomID", &dbv)) {
			SESSION_INFO* si = SM_FindSession(dbv.ptszVal, szProto);
			if (si) {
				// is the "toggle visibility option set, so we need to close the window?
				if (si->hWnd != NULL
						&& M->GetByte("Chat", "ToggleVisibility", 0) == 1
						&& !CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, 0)
						&& IsWindowVisible(si->hWnd)
						&& !IsIconic(si->pContainer->hwnd)) {
					PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
					DBFreeVariant(&dbv);
					return 1;
				}
				else
					fCreate = true;

				ShowRoom(si, WINDOW_VISIBLE, TRUE);
				if(lParam && fCreate) {
					SendMessage(si->hWnd, DM_ACTIVATEME, 0, 0);
					if(si->dat)
						SetForegroundWindow(si->dat->pContainer->hwnd);
				}
			}
			DBFreeVariant(&dbv);
			return 1;
		}
	}

	return 0;
}

INT_PTR CList_EventDoubleclicked(WPARAM wParam, LPARAM lParam)
{
	return CList_RoomDoubleclicked((WPARAM)((CLISTEVENT*)lParam)->hContact, (LPARAM) 0);
}

INT_PTR CList_JoinChat(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( hContact ) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if ( szProto ) {
			if ( DBGetContactSettingWord( hContact, szProto, "Status", 0 ) == ID_STATUS_OFFLINE )
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
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if ( szProto )
			CallProtoService( szProto, PS_LEAVECHAT, wParam, lParam );
	}
	return 0;
}

int CList_PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( hContact ) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

		CLISTMENUITEM clmi = {0};
		clmi.cbSize = sizeof( CLISTMENUITEM );
		clmi.flags = CMIM_FLAGS | CMIF_DEFAULT | CMIF_HIDDEN;

		if ( szProto ) {
			// display this menu item only for chats
			if ( M->GetByte(hContact, szProto, "ChatRoom", 0 )) {
				// still hide it for offline protos
				if ( CallProtoService( szProto, PS_GETSTATUS, 0, 0 ) != ID_STATUS_OFFLINE ) {
					clmi.flags &= ~CMIF_HIDDEN;
					clmi.flags |= CMIM_NAME;

					if ( DBGetContactSettingWord( hContact, szProto, "Status", 0 ) == ID_STATUS_OFFLINE )
						clmi.pszName = ( char* )LPGEN("Join chat");
					else
						clmi.pszName = ( char* )LPGEN("Open chat window");
		}	}	}
		CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hJoinMenuItem, ( LPARAM )&clmi );

		clmi.flags &= ~(CMIM_NAME | CMIF_DEFAULT);
		clmi.flags |= CMIF_NOTOFFLINE;
		CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hLeaveMenuItem, ( LPARAM )&clmi );
	}
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

	if(g_Settings.hGroup == 0) {
		g_Settings.hGroup = (HANDLE)CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)group);

		if(g_Settings.hGroup) {
			CallService(MS_CLUI_GROUPADDED, (WPARAM)g_Settings.hGroup, 0);
			CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM)g_Settings.hGroup, 1);
		}
	}
}

BOOL CList_AddEvent(HANDLE hContact, HICON Icon, HANDLE event, int type, const TCHAR* fmt, ...)
{
	CLISTEVENT cle;
	va_list marker;
	TCHAR* szBuf = (TCHAR*)alloca(4096 * sizeof(TCHAR));

	if (!fmt || lstrlen(fmt) < 1 || lstrlen(fmt) > 2000)
		return FALSE;

	va_start(marker, fmt);
	_vstprintf(szBuf, fmt, marker);
	va_end(marker);

	cle.cbSize = sizeof(cle);
	cle.hContact = (HANDLE)hContact;
	cle.hDbEvent = (HANDLE)event;
	cle.flags = type + CLEF_TCHAR;
	cle.hIcon = Icon;
	cle.pszService = "GChat/DblClickEvent" ;
	cle.ptszTooltip = TranslateTS(szBuf);
	if (type) {
		if (!CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, (LPARAM)0))
			CallService(MS_CLIST_ADDEVENT, (WPARAM) hContact, (LPARAM) &cle);
	} else {
		if (CallService(MS_CLIST_GETEVENT, (WPARAM)hContact, (LPARAM)0))
			CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM)"chaticon");
		CallService(MS_CLIST_ADDEVENT, (WPARAM) hContact, (LPARAM) &cle);
	}
	return TRUE;
}

HANDLE CList_FindRoom(const char* pszModule, const TCHAR* pszRoom)
{
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto && !lstrcmpiA(szProto, pszModule)) {
			if (M->GetByte(hContact, szProto, "ChatRoom", 0) != 0) {
				DBVARIANT dbv;
				if (!M->GetTString(hContact, szProto, "ChatRoomID", &dbv)) {
					if (!lstrcmpi(dbv.ptszVal, pszRoom)) {
						DBFreeVariant(&dbv);
						return hContact;
					}
					DBFreeVariant(&dbv);
				}
			}
		}

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	return 0;
}

