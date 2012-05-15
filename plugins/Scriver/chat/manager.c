/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#include "../commonheaders.h"
#include "chat.h"

extern TCHAR* pszActiveWndID ;
extern char*  pszActiveWndModule ;
extern struct MM_INTERFACE		mmi ;

extern struct GlobalMessageData *g_dat;

void LoadModuleIcons(MODULEINFO * mi);

#define WINDOWS_COMMANDS_MAX 30
#define	STATUSICONCOUNT 6

SESSION_INFO* m_WndList = 0;
TABLIST * g_TabList = 0;
MODULEINFO *m_ModList = 0;

void SetActiveSession(const TCHAR* pszID, const char* pszModule)
{
	SESSION_INFO* si = SM_FindSession(pszID, pszModule);
	if ( si )
		SetActiveSessionEx(si);
}

void SetActiveSessionEx(SESSION_INFO* si)
{
	if ( si ) {
		replaceStr( &pszActiveWndID, si->ptszID );
		replaceStrA( &pszActiveWndModule, si->pszModule );
}	}

SESSION_INFO* GetActiveSession( void )
{
	SESSION_INFO* si = SM_FindSession( pszActiveWndID, pszActiveWndModule );
	if ( si )
		return si;

	return m_WndList;
}

//---------------------------------------------------
//		Session Manager functions
//
//		Keeps track of all sessions and its windows
//---------------------------------------------------

SESSION_INFO* SM_AddSession( const TCHAR* pszID, const char* pszModule)
{
	if ( !pszID || !pszModule )
		return NULL;

	if ( !SM_FindSession(pszID, pszModule)) {
		SESSION_INFO*node = (SESSION_INFO*) mir_alloc(sizeof(SESSION_INFO));
		ZeroMemory(node, sizeof(SESSION_INFO));
		node->ptszID = mir_tstrdup( pszID );
		node->pszModule = mir_strdup( pszModule );
		node->windowData.flags = CWDF_RTF_INPUT;

		if (m_WndList == NULL) { // list is empty
			m_WndList = node;
			node->next = NULL;
		}
		else {
			node->next = m_WndList;
			m_WndList = node;
		}
		return node;
	}
	return NULL;
}

int SM_RemoveSession( const TCHAR* pszID, const char* pszModule)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if (((!pszID && pTemp->iType != GCW_SERVER) || !lstrcmpi(pTemp->ptszID,pszID)) && !lstrcmpiA(pTemp->pszModule,pszModule)) // match
		{
			DWORD dw = pTemp->dwItemData;

			if (pTemp->hWnd )
				SendMessage(pTemp->hWnd, GC_EVENT_CONTROL+WM_USER+500, SESSION_TERMINATE, 0);

			DoEventHook(pTemp->ptszID, pTemp->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)pTemp->dwItemData);

			if (pLast == NULL)
				m_WndList = pTemp->next;
			else
				pLast->next = pTemp->next;

			UM_RemoveAll(&pTemp->pUsers);
			TM_RemoveAll(&pTemp->pStatuses);
			LM_RemoveAll(&pTemp->pLog, &pTemp->pLogEnd);
			pTemp->iStatusCount = 0;
			pTemp->nUsersInNicklist = 0;

			if (pTemp->windowData.hContact)
			{
				CList_SetOffline(pTemp->windowData.hContact, pTemp->iType == GCW_CHATROOM?TRUE:FALSE);
/*				if (pTemp->iType != GCW_SERVER)
					DBWriteContactSettingByte(pTemp->windowData.hContact, "CList", "Hidden", 1);*/
			}
			DBWriteContactSettingString(pTemp->windowData.hContact, pTemp->pszModule , "Topic", "");
			DBWriteContactSettingString(pTemp->windowData.hContact, pTemp->pszModule, "StatusBar", "");
			DBDeleteContactSetting(pTemp->windowData.hContact, "CList", "StatusMsg");

			mir_free( pTemp->pszModule );
			mir_free( pTemp->ptszID );
			mir_free( pTemp->ptszName );
			mir_free( pTemp->ptszStatusbarText );
			mir_free( pTemp->ptszTopic );
			mir_free( pTemp->pszHeader );
			#if defined( _UNICODE )
				mir_free( pTemp->pszID );
				mir_free( pTemp->pszName );
			#endif

			// delete commands
			tcmdlist_free(pTemp->windowData.cmdList);
			mir_free(pTemp);
			if (pszID)
				return (int)dw;
			if (pLast)
				pTemp = pLast->next;
			else
				pTemp = m_WndList;
		}
		else
		{
			pLast = pTemp;
			pTemp = pTemp->next;
		}
	}
	return FALSE;
}

SESSION_INFO* SM_FindSession(const TCHAR* pszID, const char* pszModule)
{
	SESSION_INFO *pTemp = m_WndList;

	if ( !pszID || !pszModule )
		return NULL;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA(pTemp->pszModule,pszModule))
			return pTemp;

		pTemp = pTemp->next;
	}
	return NULL;
}

HWND SM_FindWindowByContact(HANDLE hContact)
{
	SESSION_INFO *pTemp = m_WndList;

	while ( pTemp != NULL ) {
		if ( pTemp->windowData.hContact == hContact)
			return pTemp->hWnd;

		pTemp = pTemp->next;
	}
	return NULL;
}

BOOL SM_SetOffline(const TCHAR* pszID, const char* pszModule)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule)
		return FALSE;

	while (pTemp != NULL)
	{
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA(pTemp->pszModule,pszModule))
		{
			UM_RemoveAll(&pTemp->pUsers);
			pTemp->nUsersInNicklist = 0;
			if (pTemp->iType != GCW_SERVER)
				pTemp->bInitDone = FALSE;

			if (pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

BOOL SM_SetStatusEx( const TCHAR* pszID, const char* pszModule, const TCHAR* pszText, int flags )
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule)
		return FALSE;

	while ( pTemp != NULL ) {
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA(pTemp->pszModule,pszModule)) {
			UM_SetStatusEx(pTemp->pUsers, pszText, flags);
			if (pTemp->hWnd)
				RedrawWindow(GetDlgItem(pTemp->hWnd, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
			if (pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

char SM_GetStatusIndicator(SESSION_INFO* si, USERINFO * ui)
{
	STATUSINFO * ti;
	if (!ui || !si)
		return '\0';

	ti = TM_FindStatus(si->pStatuses, TM_WordToString(si->pStatuses, ui->Status));
	if (ti)
	{
		if ((INT_PTR)ti->hIcon < STATUSICONCOUNT)
		{
			INT_PTR id = si->iStatusCount - (INT_PTR)ti->hIcon - 1;
			if (id == 0)
				return '\0';
			if (id == 1)
				return '+';
			if (id == 2)
				return '%';
			if (id == 3)
				return '@';
			if (id == 4)
				return '!';
			if (id == 5)
				return '*';
		}
		else
			return '\0';
	}
	return '\0';
}

HICON SM_GetStatusIcon(SESSION_INFO* si, USERINFO * ui)
{
	STATUSINFO * ti;
	if (!ui || !si)
		return NULL;

	ti = TM_FindStatus(si->pStatuses, TM_WordToString(si->pStatuses, ui->Status));
	if (ti)
	{
		if ((INT_PTR)ti->hIcon < STATUSICONCOUNT)
		{
			INT_PTR id = si->iStatusCount - (INT_PTR)ti->hIcon - 1;
			char name[128];
			sprintf(name, "chat_status%d", id);
			return GetCachedIcon(name);
		}
		else
			return ti->hIcon;
	}
	return GetCachedIcon("chat_status0");
}

BOOL SM_AddEventToAllMatchingUID(GCEVENT * gce)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;
	int bManyFix = 0;

	while (pTemp != NULL) {
		if ( !lstrcmpiA( pTemp->pszModule, gce->pDest->pszModule )) {
			if ( UM_FindUser( pTemp->pUsers, gce->ptszUID )) {
				if ( pTemp->bInitDone ) {
					if ( SM_AddEvent(pTemp->ptszID, pTemp->pszModule, gce, FALSE ) && pTemp->hWnd && pTemp->bInitDone) {
						SendMessage(pTemp->hWnd, GC_ADDLOG, 0, 0);
					}
					else if (pTemp->hWnd && pTemp->bInitDone) {
						SendMessage(pTemp->hWnd, GC_REDRAWLOG2, 0, 0);
					}
					DoSoundsFlashPopupTrayStuff(pTemp, gce, FALSE, bManyFix);
					bManyFix ++;
					if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings.LoggingEnabled)
						LogToFile(pTemp, gce);
		}	}	}

		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;
}

BOOL SM_AddEvent(const TCHAR* pszID, const char* pszModule, GCEVENT * gce, BOOL bIsHighlighted)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszID || !pszModule)
		return TRUE;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA(pTemp->pszModule,pszModule)) {
			LOGINFO * li = LM_AddEvent(&pTemp->pLog, &pTemp->pLogEnd);
			pTemp->iEventCount += 1;

			li->iType = gce->pDest->iType;
			li->ptszNick = mir_tstrdup( gce->ptszNick );
			li->ptszText = mir_tstrdup( gce->ptszText );
			li->ptszStatus = mir_tstrdup( gce->ptszStatus );
			li->ptszUserInfo = mir_tstrdup( gce->ptszUserInfo );

			li->bIsMe = gce->bIsMe;
			li->time = gce->time;
			li->bIsHighlighted = bIsHighlighted;

			if (g_Settings.iEventLimit > 0 && pTemp->iEventCount > g_Settings.iEventLimit + 20) {
				LM_TrimLog(&pTemp->pLog, &pTemp->pLogEnd, pTemp->iEventCount - g_Settings.iEventLimit);
				pTemp->iEventCount = g_Settings.iEventLimit;
				return FALSE;
			}
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

USERINFO * SM_AddUser( SESSION_INFO * si, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus)
{
	USERINFO * p = UM_AddUser( si->pStatuses, &si->pUsers, pszUID, pszNick, wStatus);
	si->nUsersInNicklist++;
	return p;
}

BOOL SM_MoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID)
{
	SESSION_INFO *pTemp = m_WndList;

	if (!pszID || !pszModule || !pszUID)
		return FALSE;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			UM_SortUser( &pTemp->pUsers, pszUID );
			return TRUE;
		}
		pTemp = pTemp->next;
	}

	return FALSE;
}

BOOL SM_RemoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule || !pszUID)
		return FALSE;

	while ( pTemp != NULL ) {
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			DWORD dw;
			USERINFO * ui = UM_FindUser(pTemp->pUsers, pszUID);
			if ( ui ) {
				pTemp->nUsersInNicklist--;

				dw = UM_RemoveUser(&pTemp->pUsers, pszUID);

				if (pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);

				if (pszID)
					return TRUE;
		}	}

		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;
}

USERINFO * SM_GetUserFromIndex(const TCHAR* pszID, const char* pszModule, int index)
{
	SESSION_INFO *pTemp = m_WndList;

	if (!pszModule)
		return FALSE;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule ))
			return UM_FindUserFromIndex( pTemp->pUsers, index );
		pTemp = pTemp->next;
	}

	return NULL;
}


STATUSINFO * SM_AddStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszID || !pszModule )
		return NULL;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			STATUSINFO* ti = TM_AddStatus( &pTemp->pStatuses, pszStatus, &pTemp->iStatusCount );
			if ( ti )
				pTemp->iStatusCount++;
			return ti;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return 0;
}

BOOL SM_GiveStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if ( !pszID || !pszModule )
		return FALSE;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			USERINFO * ui = UM_GiveStatus(pTemp->pUsers, pszUID, TM_StringToWord(pTemp->pStatuses, pszStatus));
			if (ui) {
				SM_MoveUser( pTemp->ptszID, pTemp->pszModule, ui->pszUID );
				if ( pTemp->hWnd )
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
			}
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return FALSE;
}

BOOL SM_SetContactStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, WORD wStatus)
{
	SESSION_INFO* pTemp = m_WndList, *pLast = NULL;

	if ( !pszID || !pszModule )
		return FALSE;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			USERINFO * ui = UM_SetContactStatus(pTemp->pUsers, pszUID, wStatus);
			if (ui) {
				SM_MoveUser( pTemp->ptszID, pTemp->pszModule, ui->pszUID );
				if ( pTemp->hWnd )
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
			}
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return FALSE;
}

BOOL SM_TakeStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszID || !pszModule )
		return FALSE;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			USERINFO* ui = UM_TakeStatus(pTemp->pUsers, pszUID, TM_StringToWord(pTemp->pStatuses, pszStatus));
			if ( ui ) {
				SM_MoveUser(pTemp->ptszID, pTemp->pszModule, ui->pszUID);
				if ( pTemp->hWnd )
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
			}
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	return FALSE;
}
LRESULT SM_SendMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	while ( pTemp && pszModule ) {
		if (( !pszID ||!lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			if ( pTemp->hWnd ) {
				LRESULT i = SendMessage(pTemp->hWnd, msg, wParam, lParam);
				if ( pszID )
					return i;
			}
			if ( pszID )
				return 0;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

BOOL SM_PostMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszID || !pszModule)
		return 0;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->ptszID, pszID ) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			if ( pTemp->hWnd )
				return PostMessage(pTemp->hWnd, msg, wParam, lParam);

			return FALSE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return FALSE;
}

BOOL SM_BroadcastMessage(const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	while (pTemp != NULL)
	{
		if (!pszModule || !lstrcmpiA(pTemp->pszModule, pszModule))
		{
			if (pTemp->hWnd)
			{
				if (bAsync)
					PostMessage(pTemp->hWnd, msg, wParam, lParam);
				else
					SendMessage(pTemp->hWnd, msg, wParam, lParam);
			}

		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

BOOL SM_SetStatus(const TCHAR* pszID, const char* pszModule, int wStatus)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule)
		return FALSE;

	while ( pTemp != NULL ) {
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			pTemp->wStatus = wStatus;

			if ( pTemp->windowData.hContact ) {
				if ( pTemp->iType != GCW_SERVER && wStatus != ID_STATUS_OFFLINE )
						DBDeleteContactSetting(pTemp->windowData.hContact, "CList", "Hidden");

				DBWriteContactSettingWord(pTemp->windowData.hContact, pTemp->pszModule, "Status", (WORD)wStatus);
			}

			PostMessage(pTemp->hWnd, GC_FIXTABICONS, 0, 0);

			if (pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

BOOL SM_SendUserMessage(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if ( !pszModule || !pszText )
		return FALSE;

	while ( pTemp != NULL ) {
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			if ( pTemp->iType == GCW_CHATROOM )
				DoEventHook( pTemp->ptszID, pTemp->pszModule, GC_USER_MESSAGE, NULL, pszText, (LPARAM)NULL);
			if (pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}


BOOL SM_ChangeUID(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszNewUID)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule)
		return FALSE;

	while ( pTemp != NULL ) {
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			USERINFO* ui = UM_FindUser( pTemp->pUsers, pszUID );
			if ( ui )
				replaceStr( &ui->pszUID, pszNewUID );

			if ( pszID )
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}


BOOL SM_ChangeNick(const TCHAR* pszID, const char* pszModule, GCEVENT * gce)
{
	SESSION_INFO *pTemp = m_WndList, *pLast = NULL;

	if (!pszModule)
		return FALSE;

	while ( pTemp != NULL ) {
		if (( !pszID || !lstrcmpi( pTemp->ptszID, pszID )) && !lstrcmpiA( pTemp->pszModule, pszModule )) {
			USERINFO* ui = UM_FindUser(pTemp->pUsers, gce->ptszUID );
			if ( ui ) {
				replaceStr( &ui->pszNick, gce->ptszText);
				SM_MoveUser( pTemp->ptszID, pTemp->pszModule, ui->pszUID );
				if ( pTemp->hWnd )
					SendMessage( pTemp->hWnd, GC_UPDATENICKLIST, 0, 0 );
			}

			if (pszID)
				return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}
BOOL SM_RemoveAll (void)
{
	while (m_WndList)
	{
		SESSION_INFO*pLast = m_WndList->next;

		if (m_WndList->hWnd)
			SendMessage(m_WndList->hWnd, GC_EVENT_CONTROL+WM_USER+500, SESSION_TERMINATE, 0);
		DoEventHook(m_WndList->ptszID, m_WndList->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)m_WndList->dwItemData);
		if (m_WndList->windowData.hContact)
			CList_SetOffline(m_WndList->windowData.hContact, m_WndList->iType == GCW_CHATROOM?TRUE:FALSE);
		DBWriteContactSettingString(m_WndList->windowData.hContact, m_WndList->pszModule , "Topic", "");
		DBDeleteContactSetting(m_WndList->windowData.hContact, "CList", "StatusMsg");
		DBWriteContactSettingString(m_WndList->windowData.hContact, m_WndList->pszModule, "StatusBar", "");

		UM_RemoveAll(&m_WndList->pUsers);
		TM_RemoveAll(&m_WndList->pStatuses);
		LM_RemoveAll(&m_WndList->pLog, &m_WndList->pLogEnd);
		m_WndList->iStatusCount = 0;
		m_WndList->nUsersInNicklist = 0;

		mir_free( m_WndList->pszModule );
		mir_free( m_WndList->ptszID );
		mir_free( m_WndList->ptszName );
		mir_free( m_WndList->ptszStatusbarText );
		mir_free( m_WndList->ptszTopic );
		mir_free( m_WndList->pszHeader);
		tcmdlist_free(m_WndList->windowData.cmdList);
		mir_free(m_WndList);
		m_WndList = pLast;
	}
	m_WndList = NULL;
	return TRUE;
}

int SM_GetCount(const char* pszModule)
{
	SESSION_INFO* pTemp = m_WndList;
	int count = 0;

	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pszModule, pTemp->pszModule))
			count++;

		pTemp = pTemp->next;
	}
	return count;
}

SESSION_INFO* SM_FindSessionByIndex(const char* pszModule, int iItem)
{
	SESSION_INFO* pTemp = m_WndList;
	int count = 0;
	while (pTemp != NULL)
	{
		if (!lstrcmpiA(pszModule, pTemp->pszModule))
		{
			if (iItem ==count)
				return pTemp;
			else
				count++;
		}

		pTemp = pTemp->next;
	}
	return NULL;

}

char* SM_GetUsers(SESSION_INFO* si)
{
	SESSION_INFO* pTemp = m_WndList;
	USERINFO* utemp = NULL;
	char* p = NULL;
	int alloced = 0;

	if ( si == NULL )
		return NULL;

	while (pTemp != NULL) {
		if ( si == pTemp ) {
			if (( utemp = pTemp->pUsers ) == NULL )
				return NULL;

			break;
		}
		pTemp = pTemp->next;
	}

	do {
		int pLen = lstrlenA(p), nameLen = lstrlen(utemp->pszUID);
		if ( pLen + nameLen + 2 > alloced )
			p = mir_realloc( p, alloced += 4096 );

		#if !defined( _UNICODE )
			lstrcpy( p + pLen, utemp->pszUID );
		#else
			WideCharToMultiByte( CP_ACP, 0, utemp->pszUID, -1, p + pLen, nameLen+1, 0, 0 );
		#endif
		lstrcpyA( p + pLen + nameLen, " " );
		utemp = utemp->next;
	}
		while ( utemp != NULL );
	return p;
}


SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
        SESSION_INFO* pResult = NULL;
        if (prevSession == NULL && my_strstri( currSession->ptszName, pszOriginal) == currSession->ptszName) {
            pResult = currSession;
        } else {
            TCHAR* pszName = NULL;
            SESSION_INFO* pTemp = m_WndList;
			if (currSession == prevSession) {
				pszCurrent = pszOriginal;
			}
            while (pTemp != NULL) {
                    if (pTemp != currSession && !lstrcmpiA(pszModule, pTemp->pszModule)) {
                            if ( my_strstri( pTemp->ptszName, pszOriginal) == pTemp->ptszName ) {
                                    if ( prevSession != pTemp && lstrcmpi( pTemp->ptszName, pszCurrent ) > 0 && ( !pszName || lstrcmpi( pTemp->ptszName, pszName ) < 0) ) {
                                            pResult = pTemp;
                                            pszName = pTemp->ptszName;
                                    }
                            }
                    }
                    pTemp = pTemp->next;
            }
        }
        return pResult;

}


//---------------------------------------------------
//		Module Manager functions
//
//		Necessary to keep track of all modules
//		that has registered with the plugin
//---------------------------------------------------

MODULEINFO* MM_AddModule(const char* pszModule)
{
	if (!pszModule)
		return NULL;
	if (!MM_FindModule(pszModule))
	{
		MODULEINFO *node = (MODULEINFO*) mir_calloc(sizeof(MODULEINFO));

		node->pszModule = mir_strdup(pszModule);

		if (m_ModList == NULL) // list is empty
		{
			m_ModList = node;
		}
		else
		{
			node->next = m_ModList;
			m_ModList = node;
		}
		return node;
	}
	return FALSE;
}

void MM_IconsChanged(void)
{
	MODULEINFO *pTemp = m_ModList;

	while (pTemp != NULL)
	{
		if (pTemp->hOnlineTalkIcon)
			DestroyIcon(pTemp->hOnlineTalkIcon);
		if (pTemp->hOfflineTalkIcon)
			DestroyIcon(pTemp->hOfflineTalkIcon);

		LoadModuleIcons(pTemp);
		pTemp = pTemp->next;
	}
	return;
}
void MM_FontsChanged(void)
{
	SESSION_INFO* pTemp = m_WndList;
	SetIndentSize();
	while (pTemp != NULL)
	{
		pTemp->pszHeader = Log_CreateRtfHeader(MM_FindModule(pTemp->pszModule), pTemp);
		pTemp = pTemp->next;
	}
	return;
}
MODULEINFO* MM_FindModule(const char* pszModule)
{
	MODULEINFO *pTemp = m_ModList, *pLast = NULL;

	if (!pszModule)
		return NULL;

	while (pTemp != NULL) {
		if (lstrcmpiA(pTemp->pszModule,pszModule) == 0)
			return pTemp;

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

// stupid thing..
void MM_FixColors()
{
	MODULEINFO *pTemp = m_ModList;

	while (pTemp != NULL)
	{
		CheckColorsInModule(pTemp->pszModule);
		pTemp = pTemp->next;
	}
	return;
}

BOOL MM_RemoveAll (void)
{
	while (m_ModList != NULL)
	{
		MODULEINFO *pLast = m_ModList->next;
		mir_free(m_ModList->pszModule);
		mir_free(m_ModList->ptszModDispName);
		mir_free(m_ModList->crColors);

		if (m_ModList->hOnlineTalkIcon)
			DestroyIcon(m_ModList->hOnlineTalkIcon);
		if (m_ModList->hOfflineTalkIcon)
			DestroyIcon(m_ModList->hOfflineTalkIcon);

		mir_free(m_ModList);
		m_ModList = pLast;
	}
	m_ModList = NULL;
	return TRUE;
}

//---------------------------------------------------
//		Status manager functions
//
//		Necessary to keep track of what user statuses
//		per window nicklist that is available
//---------------------------------------------------

STATUSINFO * TM_AddStatus(STATUSINFO** ppStatusList, const TCHAR* pszStatus, int* iCount)
{
	if (!ppStatusList || !pszStatus)
		return NULL;

	if ( !TM_FindStatus(*ppStatusList, pszStatus)) {
		STATUSINFO *node = (STATUSINFO*) mir_alloc(sizeof(STATUSINFO));
		ZeroMemory(node, sizeof(STATUSINFO));
		replaceStr( &node->pszGroup, pszStatus );
		node->hIcon = (HICON)(*iCount);
		while ((INT_PTR)node->hIcon > STATUSICONCOUNT - 1)
			node->hIcon--;

		if (*ppStatusList == NULL) // list is empty
		{
			node->Status = 1;
			*ppStatusList = node;
			node->next = NULL;
		}
		else
		{
			node->Status = ppStatusList[0]->Status*2;
			node->next = *ppStatusList;
			*ppStatusList = node;
		}
		return node;

	}
	return FALSE;
}

STATUSINFO * TM_FindStatus(STATUSINFO* pStatusList, const TCHAR* pszStatus)
{
	STATUSINFO *pTemp = pStatusList, *pLast = NULL;

	if (!pStatusList || !pszStatus)
		return NULL;

	while ( pTemp != NULL ) {
		if ( lstrcmpi(pTemp->pszGroup, pszStatus) == 0 )
			return pTemp;

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

WORD TM_StringToWord(STATUSINFO* pStatusList, const TCHAR* pszStatus)
{
	STATUSINFO *pTemp = pStatusList, *pLast = NULL;

	if (!pStatusList || !pszStatus)
		return 0;

	while (pTemp != NULL) {
		if ( lstrcmpi( pTemp->pszGroup, pszStatus ) == 0 )
			return pTemp->Status;

		if ( pTemp->next == NULL )
			return pStatusList->Status;

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

TCHAR* TM_WordToString(STATUSINFO* pStatusList, WORD Status)
{
	STATUSINFO *pTemp = pStatusList, *pLast = NULL;

	if (!pStatusList)
		return NULL;

	while (pTemp != NULL) {
		if (pTemp->Status&Status) {
			Status -= pTemp->Status;
			if (Status == 0)
				return pTemp->pszGroup;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

BOOL TM_RemoveAll (STATUSINFO** ppStatusList)
{

	if (!ppStatusList)
		return FALSE;

	while (*ppStatusList != NULL)
	{
		STATUSINFO *pLast = ppStatusList[0]->next;
		mir_free(ppStatusList[0]->pszGroup);
		if ((INT_PTR)ppStatusList[0]->hIcon > 10)
			DestroyIcon(ppStatusList[0]->hIcon);
		mir_free(*ppStatusList);
		*ppStatusList = pLast;
	}
	*ppStatusList = NULL;
	return TRUE;
}

//---------------------------------------------------
//		User manager functions
//
//		Necessary to keep track of the users
//		in a window nicklist
//---------------------------------------------------


static int UM_CompareItem(USERINFO * u1, const TCHAR* pszNick, WORD wStatus)
{
	int i;

	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (i=0; i<8; i++ )
	{
		if (( dw1 & 1 ) && !( dw2 & 1 ))
			return -1;
		if (( dw2 & 1 ) && !( dw1 & 1 ))
			return 1;
		if (( dw1 & 1 ) &&  ( dw2 & 1 ))
			return lstrcmp( u1->pszNick, pszNick );

		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	return lstrcmp( u1->pszNick, pszNick );

}

USERINFO * UM_SortUser(USERINFO** ppUserList, const TCHAR* pszUID)
{
	USERINFO * pTemp = *ppUserList, *pLast = NULL;
	USERINFO * node = NULL;

	if (!pTemp || !pszUID)
		return NULL;

	while(pTemp && lstrcmpi( pTemp->pszUID, pszUID)) {
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	if ( pTemp ) {
		node = pTemp;
		if (pLast)
			pLast->next = pTemp->next;
		else
			*ppUserList = pTemp->next;
		pTemp = *ppUserList;

		pLast = NULL;

		while ( pTemp && UM_CompareItem(pTemp, node->pszNick, node->Status ) <= 0) {
			pLast = pTemp;
			pTemp = pTemp->next;
		}

		if (*ppUserList == NULL) { // list is empty
			*ppUserList = node;
			node->next = NULL;
		}
		else {
			if ( pLast ) {
				node->next = pTemp;
				pLast->next = node;
			}
			else {
				node->next = *ppUserList;
				*ppUserList = node;
		}	}

		return node;
	}
	return NULL;
}

USERINFO* UM_AddUser(STATUSINFO* pStatusList, USERINFO** ppUserList, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus)
{
	USERINFO * pTemp = *ppUserList, *pLast = NULL;

	if (!pStatusList || !ppUserList || !ppUserList)
		return NULL;

	while(pTemp && UM_CompareItem(pTemp, pszNick, wStatus) <= 0)
	{
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	//	if (!UM_FindUser(*ppUserList, pszUI, wStatus)
	{
		USERINFO *node = (USERINFO*) mir_alloc(sizeof(USERINFO));
		ZeroMemory(node, sizeof(USERINFO));
		replaceStr( &node->pszUID, pszUID );

		if (*ppUserList == NULL) { // list is empty
			*ppUserList = node;
			node->next = NULL;
		}
		else {
			if ( pLast ) {
				node->next = pTemp;
				pLast->next = node;
			}
			else {
				node->next = *ppUserList;
				*ppUserList = node;
		}	}

		return node;
	}
	return NULL;
}

USERINFO* UM_FindUser(USERINFO* pUserList, const TCHAR* pszUID)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if (!pUserList || !pszUID)
		return NULL;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->pszUID, pszUID ))
			return pTemp;

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

USERINFO* UM_FindUserFromIndex(USERINFO* pUserList, int index)
{
	int i = 0;
	USERINFO *pTemp = pUserList;

	if (!pUserList)
		return NULL;

	while (pTemp != NULL)
	{
		if (i == index)
		{
			return pTemp;
		}
		pTemp = pTemp->next;
		i++;
	}
	return NULL;
}

USERINFO* UM_GiveStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if (!pUserList || !pszUID)
		return NULL;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->pszUID, pszUID )) {
			pTemp->Status |= status;
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

USERINFO* UM_SetContactStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if (!pUserList || !pszUID)
		return NULL;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->pszUID, pszUID )) {
			pTemp->ContactStatus = status;
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

BOOL UM_SetStatusEx(USERINFO* pUserList, const TCHAR* pszText, int flags )
{
	USERINFO *pTemp = pUserList, *pLast = NULL;
	int bOnlyMe = ( flags & GC_SSE_ONLYLISTED ) != 0, bSetStatus = ( flags & GC_SSE_ONLINE ) != 0;
	char cDelimiter = ( flags & GC_SSE_TABDELIMITED ) ? '\t' : ' ';

	while (pTemp != NULL)
	{
		if ( !bOnlyMe )
			pTemp->iStatusEx = 0;

		if ( pszText != NULL ) {
			TCHAR* s = _tcsstr( pszText, pTemp->pszUID );
			if ( s ) {
				pTemp->iStatusEx = 0;
				if ( s == pszText || s[-1] == cDelimiter ) {
					int len = lstrlen( pTemp->pszUID );
					if ( s[len] == cDelimiter || s[len] == '\0' )
						pTemp->iStatusEx = ( !bOnlyMe || bSetStatus ) ? 1 : 0;
		}	}	}

		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return TRUE;
}

USERINFO* UM_TakeStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status)
{
	USERINFO *pTemp = pUserList, *pLast = NULL;

	if (!pUserList || !pszUID)
		return NULL;

	while ( pTemp != NULL ) {
		if ( !lstrcmpi( pTemp->pszUID, pszUID )) {
			pTemp->Status &= ~status;
			return pTemp;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return 0;
}

TCHAR* UM_FindUserAutoComplete(USERINFO* pUserList, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	TCHAR* pszName = NULL;
	USERINFO *pTemp = pUserList;

	if (!pUserList || !pszOriginal || !pszCurrent)
		return NULL;

	while ( pTemp != NULL ) {
		if ( my_strstri( pTemp->pszNick, pszOriginal) == pTemp->pszNick )
			if ( lstrcmpi( pTemp->pszNick, pszCurrent ) > 0 && ( !pszName || lstrcmpi( pTemp->pszNick, pszName ) < 0) )
				pszName = pTemp->pszNick;

		pTemp = pTemp->next;
	}
	return pszName;
}

BOOL UM_RemoveUser(USERINFO** ppUserList, const TCHAR* pszUID)
{
	USERINFO *pTemp = *ppUserList, *pLast = NULL;

	if (!ppUserList || !pszUID)
		return FALSE;

	while (pTemp != NULL) {
		if (!lstrcmpi( pTemp->pszUID, pszUID )) {
			if (pLast == NULL)
				*ppUserList = pTemp->next;
			else
				pLast->next = pTemp->next;
			mir_free(pTemp->pszNick);
			mir_free(pTemp->pszUID);
			mir_free(pTemp);
			return TRUE;
		}
		pLast = pTemp;
		pTemp = pTemp->next;
	}
	return FALSE;
}

BOOL UM_RemoveAll (USERINFO** ppUserList)
{
	if (!ppUserList)
		return FALSE;

	while (*ppUserList != NULL)
	{
		USERINFO *pLast = ppUserList[0]->next;
		mir_free( ppUserList[0]->pszUID );
		mir_free( ppUserList[0]->pszNick );
		mir_free( *ppUserList );
		*ppUserList = pLast;
	}
	*ppUserList = NULL;
	return TRUE;
}

//---------------------------------------------------
//		Log manager functions
//
//		Necessary to keep track of events
//		in a window log
//---------------------------------------------------

LOGINFO * LM_AddEvent(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
{

	LOGINFO *node = NULL;

	if (!ppLogListStart || !ppLogListEnd)
		return NULL;

	node = (LOGINFO*) mir_alloc(sizeof(LOGINFO));
	ZeroMemory(node, sizeof(LOGINFO));


	if (*ppLogListStart == NULL) // list is empty
	{
		*ppLogListStart = node;
		*ppLogListEnd = node;
		node->next = NULL;
		node->prev = NULL;
	}
	else
	{
		ppLogListStart[0]->prev = node;
		node->next = *ppLogListStart;
		*ppLogListStart = node;
		ppLogListStart[0]->prev=NULL;
	}

	return node;
}

BOOL LM_TrimLog(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd, int iCount)
{
	LOGINFO *pTemp = *ppLogListEnd;
	while (pTemp != NULL && iCount > 0) {
		*ppLogListEnd = pTemp->prev;
		if (*ppLogListEnd == NULL)
			*ppLogListStart = NULL;

		mir_free(pTemp->ptszNick);
		mir_free(pTemp->ptszUserInfo);
		mir_free(pTemp->ptszText);
		mir_free(pTemp->ptszStatus);
		mir_free(pTemp);
		pTemp = *ppLogListEnd;
		iCount--;
	}
	ppLogListEnd[0]->next = NULL;

	return TRUE;
}

BOOL LM_RemoveAll (LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
{
	while ( *ppLogListStart != NULL ) {
		LOGINFO *pLast = ppLogListStart[0]->next;
		mir_free( ppLogListStart[0]->ptszText );
		mir_free( ppLogListStart[0]->ptszNick );
		mir_free( ppLogListStart[0]->ptszStatus );
		mir_free( ppLogListStart[0]->ptszUserInfo );
		mir_free( *ppLogListStart );
		*ppLogListStart = pLast;
	}
	*ppLogListStart = NULL;
	*ppLogListEnd = NULL;
	return TRUE;
}
