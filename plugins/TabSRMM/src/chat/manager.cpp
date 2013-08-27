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
*/

#include "..\commonheaders.h"

#define	WINDOWS_COMMANDS_MAX 30
#define	STATUSICONCOUNT 6

static SESSION_INFO *s_WndList = NULL;
static MODULEINFO *s_ModList = NULL;

void SetActiveSession(const TCHAR* pszID, const char* pszModule)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si)
		SetActiveSessionEx(si);
}

void SetActiveSessionEx(SESSION_INFO *si)
{
	if (si) {
		replaceStrT(pszActiveWndID, si->ptszID);
		replaceStr(pszActiveWndModule, si->pszModule);
	}
}

SESSION_INFO* GetActiveSession(void)
{
	SESSION_INFO *si = SM_FindSession(pszActiveWndID, pszActiveWndModule);
	if (si)
		return si;

	return s_WndList;
}

//---------------------------------------------------
//		Session Manager functions
//
//		Keeps track of all sessions and its windows
//---------------------------------------------------

SESSION_INFO* SM_AddSession(const TCHAR* pszID, const char* pszModule)
{
	if (!pszID || !pszModule)
		return NULL;

	if (SM_FindSession(pszID, pszModule))
		return NULL;

	SESSION_INFO *node = (SESSION_INFO*)mir_calloc(sizeof(SESSION_INFO));
	node->ptszID = mir_tstrdup(pszID);
	node->pszModule = mir_strdup(pszModule);

	MODULEINFO *mi = MM_FindModule(pszModule);
	if (mi) {
		mi->idleTimeStamp = time(0);
		SM_BroadcastMessage(pszModule, GC_UPDATESTATUSBAR, 0, 1, TRUE);
	}

	if (s_WndList == NULL) { // list is empty
		s_WndList = node;
		node->next = NULL;
	}
	else {
		node->next = s_WndList;
		s_WndList = node;
	}
	node->Highlight = g_Settings.Highlight;
	return node;
}

int SM_RemoveSession(const TCHAR* pszID, const char* pszModule, bool removeContact)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO* pTemp = s_WndList, *pLast = NULL;
	while (pTemp != NULL) {
		if ((!pszID && pTemp->iType != GCW_SERVER || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) { // match
			DWORD dw = pTemp->dwItemData;

			if (pTemp->hWnd)
				SendMessage(pTemp->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);

			DoEventHook(pTemp->ptszID, pTemp->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)pTemp->dwItemData);

			if (pLast == NULL)
				s_WndList = pTemp->next;
			else
				pLast->next = pTemp->next;

			UM_RemoveAll(&pTemp->pUsers);
			TM_RemoveAll(&pTemp->pStatuses);
			LM_RemoveAll(&pTemp->pLog, &pTemp->pLogEnd);
			pTemp->iStatusCount = 0;
			pTemp->nUsersInNicklist = 0;

			if (pTemp->hContact)
				CList_SetOffline(pTemp->hContact, pTemp->iType == GCW_CHATROOM ? TRUE : FALSE);

			db_set_s(pTemp->hContact, pTemp->pszModule , "Topic", "");
			db_set_s(pTemp->hContact, pTemp->pszModule, "StatusBar", "");
			db_unset(pTemp->hContact, "CList", "StatusMsg");

			if (removeContact)
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)pTemp->hContact, 0);

			mir_free(pTemp->pszModule);
			mir_free(pTemp->ptszID);
			mir_free(pTemp->ptszName);
			mir_free(pTemp->ptszStatusbarText);
			mir_free(pTemp->ptszTopic);
			mir_free(pTemp->pszID);
			mir_free(pTemp->pszName);

			// delete commands
			COMMAND_INFO *pCurComm = pTemp->lpCommands;
			while (pCurComm != NULL) {
				COMMAND_INFO *pNext = pCurComm->next;
				mir_free(pCurComm->lpCommand);
				mir_free(pCurComm);
				pCurComm = pNext;
			}

			mir_free(pTemp);
			if (pszID)
				return (int)dw;
			if (pLast)
				pTemp = pLast->next;
			else
				pTemp = s_WndList;
		}
		else {
			pLast = pTemp;
			pTemp = pTemp->next;
		}
	}
	return FALSE;
}

void SM_RemoveContainer(TContainerData *pContainer)
{
	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		if (si->pContainer == pContainer)
			si->pContainer = NULL;
}

SESSION_INFO* SM_FindSession(const TCHAR* pszID, const char* pszModule)
{
	if (!pszID || !pszModule)
		return NULL;

	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		if (!lstrcmpi(si->ptszID, pszID) && !lstrcmpiA(si->pszModule, pszModule))
			return si;

	return NULL;
}

BOOL SM_SetOffline(const TCHAR* pszID, const char* pszModule)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			UM_RemoveAll(&si->pUsers);
			si->nUsersInNicklist = 0;
			if (si->iType != GCW_SERVER)
				si->bInitDone = FALSE;

			if (pszID)
				return TRUE;
		}
	}

	return TRUE;
}

BOOL SM_SetStatusEx(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText, int flags)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			UM_SetStatusEx(si->pUsers, pszText, flags);
			if (si->hWnd)
				RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
			if (pszID)
				return TRUE;
		}
	}

	return TRUE;
}

HICON SM_GetStatusIcon(SESSION_INFO *si, USERINFO* ui, char *szIndicator)
{
	if (!ui || !si)
		return NULL;

	*szIndicator = 0;

	STATUSINFO *ti = TM_FindStatus(si->pStatuses, TM_WordToString(si->pStatuses, ui->Status));
	if (ti == NULL)
		return hIcons[ICON_STATUS0];

	if ((INT_PTR)ti->hIcon >= STATUSICONCOUNT)
		return ti->hIcon;

	int id = si->iStatusCount - (int)ti->hIcon - 1;
	if (id == 0) {
		*szIndicator = 0;
		return hIcons[ICON_STATUS0];
	}
	if (id == 1) {
		*szIndicator = '+';
		return hIcons[ICON_STATUS1];
	}
	if (id == 2) {
		*szIndicator = '%';
		return hIcons[ICON_STATUS2];
	}
	if (id == 3) {
		*szIndicator = '@';
		return hIcons[ICON_STATUS3];
	}
	if (id == 4) {
		*szIndicator = '!';
		return hIcons[ICON_STATUS4];
	}
	if (id == 5) {
		*szIndicator = '*';
		return hIcons[ICON_STATUS5];
	}
		
	return hIcons[ICON_STATUS0];
}

BOOL SM_AddEventToAllMatchingUID(GCEVENT *gce, BOOL bIsHighLight)
{
	int bManyFix = 0;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if (!lstrcmpiA(si->pszModule, gce->pDest->pszModule)) {
			if (UM_FindUser(si->pUsers, gce->ptszUID)) {
				if (si->bInitDone) {
					if (SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE) && si->hWnd && si->bInitDone)
						SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
					else if (si->hWnd && si->bInitDone)
						SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);

					if (!(gce->dwFlags & GCEF_NOTNOTIFY))
						DoSoundsFlashPopupTrayStuff(si, gce, bIsHighLight, bManyFix);
					bManyFix ++;
					if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings.bLoggingEnabled)
						LogToFile(si, gce);
				}
			}
		}
	}

	return 0;
}

BOOL SM_AddEvent(const TCHAR* pszID, const char* pszModule, GCEVENT *gce, BOOL bIsHighlighted)
{
	if (!pszID || !pszModule)
		return TRUE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return TRUE;

	if (!lstrcmpi(si->ptszID, pszID) && !lstrcmpiA(si->pszModule, pszModule)) {
		LOGINFO *li = LM_AddEvent(&si->pLog, &si->pLogEnd);
		si->iEventCount += 1;

		li->iType = gce->pDest->iType;
		li->ptszNick = mir_tstrdup(gce->ptszNick);
		li->ptszText = mir_tstrdup(gce->ptszText);
		li->ptszStatus = mir_tstrdup(gce->ptszStatus);
		li->ptszUserInfo = mir_tstrdup(gce->ptszUserInfo);

		li->bIsMe = gce->bIsMe != 0;
		li->bIsHighlighted = bIsHighlighted != 0;
		li->time = gce->time;

		if (g_Settings.iEventLimit > 0 && si->iEventCount > g_Settings.iEventLimit + g_Settings.iEventLimitThreshold) {
			LM_TrimLog(&si->pLog, &si->pLogEnd, si->iEventCount - g_Settings.iEventLimit);
			si->wasTrimmed = TRUE;
			si->iEventCount = g_Settings.iEventLimit;
		}
	}

	return TRUE;
}

USERINFO* SM_AddUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus)
{
	if (!pszID || !pszModule)
		return NULL;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si) {
		USERINFO *p = UM_AddUser(si->pStatuses, &si->pUsers, pszUID, pszNick, wStatus);
		si->nUsersInNicklist++;
		return p;
	}

	return 0;
}

BOOL SM_MoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID)
{
	if (!pszID || !pszModule || !pszUID)
		return FALSE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	UM_SortUser(&si->pUsers, pszUID);
	return TRUE;
}

BOOL SM_RemoveUser(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID)
{
	if (!pszModule || !pszUID)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			USERINFO *ui = UM_FindUser(si->pUsers, pszUID);
			if (ui) {
				si->nUsersInNicklist--;

				UM_RemoveUser(&si->pUsers, pszUID);

				if (si->hWnd)
					SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);

				if (pszID)
					return TRUE;
			}
		}
	}

	return 0;
}

USERINFO* SM_GetUserFromIndex(const TCHAR* pszID, const char* pszModule, int index)
{
	if (!pszModule)
		return NULL;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	return UM_FindUserFromIndex(si->pUsers, index);
}

STATUSINFO* SM_AddStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszStatus)
{
	if (!pszID || !pszModule)
		return NULL;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	STATUSINFO *ti = TM_AddStatus(&si->pStatuses, pszStatus, &si->iStatusCount);
	if (ti)
		si->iStatusCount++;
	return ti;
}

BOOL SM_GiveStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus)
{
	if (!pszID || !pszModule)
		return FALSE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;
		
	USERINFO *ui = UM_GiveStatus(si->pUsers, pszUID, TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
	return TRUE;
}

BOOL SM_SetContactStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, WORD wStatus)
{
	if (!pszID || !pszModule)
		return FALSE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	USERINFO *ui = UM_SetContactStatus(si->pUsers, pszUID, wStatus);
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
	return TRUE;
}

BOOL SM_TakeStatus(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszStatus)
{
	if (!pszID || !pszModule)
		return FALSE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	USERINFO* ui = UM_TakeStatus(si->pUsers, pszUID, TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
	return TRUE;
}

LRESULT SM_SendMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for (SESSION_INFO *si = s_WndList; si && pszModule; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			if (si->hWnd) {
				LRESULT i = SendMessage(si->hWnd, msg, wParam, lParam);
				if (pszID)
					return i;
			}
			if (pszID)
				return 0;
		}
	}

	return 0;
}

BOOL SM_PostMessage(const TCHAR* pszID, const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!pszID || !pszModule)
		return 0;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si && si->hWnd)
		return PostMessage(si->hWnd, msg, wParam, lParam);

	return FALSE;
}

BOOL SM_BroadcastMessage(const char* pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync)
{
	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if (!pszModule || !lstrcmpiA(si->pszModule, pszModule)) {
			if (si->hWnd) {
				if (bAsync)
					PostMessage(si->hWnd, msg, wParam, lParam);
				else
					SendMessage(si->hWnd, msg, wParam, lParam);
			}
		}
	}

	return TRUE;
}

BOOL SM_ReconfigureFilters()
{
	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		Chat_SetFilters(si);

	return TRUE;
}

BOOL SM_InvalidateLogDirectories()
{
	mir_cslock lck(cs);

	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		si->pszLogFileName[0] = si->pszLogFileName[1] = 0;

	return TRUE;
}

BOOL SM_SetStatus(const TCHAR* pszID, const char* pszModule, int wStatus)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			si->wStatus = wStatus;

			if (si->hContact) {
				if (si->iType != GCW_SERVER && wStatus != ID_STATUS_OFFLINE)
					db_unset(si->hContact, "CList", "Hidden");

				db_set_w(si->hContact, si->pszModule, "Status", (WORD)wStatus);
			}

			if (pszID)
				return TRUE;
		}
	}

	return TRUE;
}

BOOL SM_SendUserMessage(const TCHAR* pszID, const char* pszModule, const TCHAR* pszText)
{
	if (!pszModule || !pszText)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			if (si->iType == GCW_CHATROOM)
				DoEventHook(si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, pszText, 0);
			if (pszID)
				return TRUE;
		}
	}

	return TRUE;
}

BOOL SM_ChangeUID(const TCHAR* pszID, const char* pszModule, const TCHAR* pszUID, const TCHAR* pszNewUID)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			USERINFO* ui = UM_FindUser(si->pUsers, pszUID);
			if (ui)
				replaceStrT(ui->pszUID, pszNewUID);

			if (pszID)
				return TRUE;
		}
	}

	return TRUE;
}

BOOL SM_ChangeNick(const TCHAR* pszID, const char* pszModule, GCEVENT *gce)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if ((!pszID || !lstrcmpi(si->ptszID, pszID)) && !lstrcmpiA(si->pszModule, pszModule)) {
			USERINFO* ui = UM_FindUser(si->pUsers, gce->ptszUID);
			if (ui) {
				replaceStrT(ui->pszNick, gce->ptszText);
				SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
				if (si->hWnd) {
					SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
					if (si->dat)
						GetMyNick(si->dat);
					SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
				}
			}
			if (pszID)
				return TRUE;
		}
	}

	return TRUE;
}

BOOL SM_RemoveAll(void)
{
	while (s_WndList) {
		SESSION_INFO *pLast = s_WndList->next;

		if (s_WndList->hWnd)
			SendMessage(s_WndList->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
		DoEventHook(s_WndList->ptszID, s_WndList->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)s_WndList->dwItemData);
		if (s_WndList->hContact)
			CList_SetOffline(s_WndList->hContact, s_WndList->iType == GCW_CHATROOM ? TRUE : FALSE);
		db_set_s(s_WndList->hContact, s_WndList->pszModule , "Topic", "");
		db_unset(s_WndList->hContact, "CList", "StatusMsg");
		db_set_s(s_WndList->hContact, s_WndList->pszModule, "StatusBar", "");

		UM_RemoveAll(&s_WndList->pUsers);
		TM_RemoveAll(&s_WndList->pStatuses);
		LM_RemoveAll(&s_WndList->pLog, &s_WndList->pLogEnd);
		s_WndList->iStatusCount = 0;
		s_WndList->nUsersInNicklist = 0;

		mir_free(s_WndList->pszModule);
		mir_free(s_WndList->ptszID);
		mir_free(s_WndList->ptszName);
		mir_free(s_WndList->ptszStatusbarText);
		mir_free(s_WndList->ptszTopic);

		while (s_WndList->lpCommands != NULL) {
			COMMAND_INFO *pNext = s_WndList->lpCommands->next;
			mir_free(s_WndList->lpCommands->lpCommand);
			mir_free(s_WndList->lpCommands);
			s_WndList->lpCommands = pNext;
		}

		mir_free(s_WndList);
		s_WndList = pLast;
	}
	s_WndList = NULL;
	return TRUE;
}

void SM_AddCommand(const TCHAR* pszID, const char* pszModule, const char* lpNewCommand)
{
	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if (lstrcmpi(si->ptszID, pszID) == 0 && lstrcmpiA(si->pszModule, pszModule) == 0) {      // match
			COMMAND_INFO *node = (COMMAND_INFO *)mir_alloc(sizeof(COMMAND_INFO));
			node->lpCommand = mir_strdup(lpNewCommand);
			node->last = NULL; // always added at beginning!
			// new commands are added at start
			if (si->lpCommands == NULL) {
				node->next = NULL;
				si->lpCommands = node;
			}
			else {
				node->next = si->lpCommands;
				si->lpCommands->last = node; // hmm, weird
				si->lpCommands = node;
			}
			si->lpCurrentCommand = NULL; // current command
			si->wCommandsNum++;

			if (si->wCommandsNum > WINDOWS_COMMANDS_MAX) {
				COMMAND_INFO *pCurComm = si->lpCommands;
				COMMAND_INFO *pLast;
				while (pCurComm->next != NULL) {
					pCurComm = pCurComm->next;
				}
				pLast = pCurComm->last;
				mir_free(pCurComm->lpCommand);
				mir_free(pCurComm);
				pLast->next = NULL;
				// done
				si->wCommandsNum--;
			}
		}
	}
}

char* SM_GetPrevCommand(const TCHAR* pszID, const char* pszModule) // get previous command. returns NULL if previous command does not exist. current command remains as it was.
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;
	
	COMMAND_INFO *pPrevCmd = NULL;
	if (si->lpCurrentCommand != NULL) {
		if (si->lpCurrentCommand->next != NULL) // not NULL
			pPrevCmd = si->lpCurrentCommand->next; // next command (newest at beginning)
		else
			pPrevCmd = si->lpCurrentCommand;
	}
	else pPrevCmd = si->lpCommands;

	si->lpCurrentCommand = pPrevCmd; // make it the new command
	return pPrevCmd ? pPrevCmd->lpCommand : NULL;
}

char* SM_GetNextCommand(const TCHAR* pszID, const char* pszModule) // get next command. returns NULL if next command does not exist. current command becomes NULL (a prev command after this one will get you the last command)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	COMMAND_INFO *pNextCmd = NULL;
	if (si->lpCurrentCommand != NULL)
		pNextCmd = si->lpCurrentCommand->last; // last command (newest at beginning)
	
	si->lpCurrentCommand = pNextCmd; // make it the new command
	return pNextCmd ? pNextCmd->lpCommand : NULL;
}

int SM_GetCount(const char* pszModule)
{
	int count = 0;

	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		if (!lstrcmpiA(pszModule, si->pszModule))
			count++;

	return count;
}

SESSION_INFO* SM_FindSessionByHWND(HWND hWnd)
{
	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		if (si->hWnd == hWnd)
			return si;

	return NULL;
}

SESSION_INFO* SM_FindSessionByHCONTACT(HANDLE h)
{
	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		if (si->hContact == h)
			return si;

	return NULL;
}

SESSION_INFO* SM_FindSessionByIndex(const char* pszModule, int iItem)
{
	int count = 0;
	for (SESSION_INFO *si = s_WndList; si; si = si->next) {
		if (!lstrcmpiA(pszModule, si->pszModule)) {
			if (iItem == count)
				return si;

			count++;
		}
	}

	return NULL;
}

SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	if (prevSession == NULL && my_strstri(currSession->ptszName, pszOriginal) == currSession->ptszName)
		return currSession;

	TCHAR* pszName = NULL;
	if (currSession == prevSession)
		pszCurrent = pszOriginal;

	SESSION_INFO *pResult = NULL;
	for (SESSION_INFO *si = s_WndList; si; si = si->next)
		if (si != currSession && !lstrcmpiA(pszModule, si->pszModule))
			if (my_strstri(si->ptszName, pszOriginal) == si->ptszName)
				if (prevSession != si && lstrcmpi(si->ptszName, pszCurrent) > 0 && (!pszName || lstrcmpi(si->ptszName, pszName) < 0)) {
					pResult = si;
					pszName = si->ptszName;
				}

	return pResult;
}

char* SM_GetUsers(SESSION_INFO *si)
{
	if (si == NULL)
		return NULL;

	for (SESSION_INFO *psi = s_WndList; psi; psi = psi->next)
		if (psi == si)
			goto LBL_Found;
	return NULL;

LBL_Found:
	USERINFO *utemp = si->pUsers;
	if (utemp == NULL)
		return NULL;

	char* p = NULL;
	int alloced = 0;

	while (utemp != NULL) {
		int pLen = lstrlenA(p), nameLen = lstrlen(utemp->pszUID);
		if (pLen + nameLen + 2 > alloced)
			p = (char *)mir_realloc(p, alloced += 4096);
		WideCharToMultiByte(CP_ACP, 0, utemp->pszUID, -1, p + pLen, nameLen + 1, 0, 0);
		lstrcpyA(p + pLen + nameLen, " ");
		utemp = utemp->next;
	}
		
	return p;
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
	if (!MM_FindModule(pszModule)) {
		MODULEINFO *node = (MODULEINFO*) mir_alloc(sizeof(MODULEINFO));
		ZeroMemory(node, sizeof(MODULEINFO));

		node->pszModule = (char*)mir_alloc(lstrlenA(pszModule) + 1);
		lstrcpyA(node->pszModule, pszModule);
		node->idleTimeStamp = time(0);
		if (s_ModList == NULL) { // list is empty
			s_ModList = node;
			node->next = NULL;
		} else {
			node->next = s_ModList;
			s_ModList = node;
		}
		return node;
	}
	return FALSE;
}

void MM_FontsChanged(void)
{
	MODULEINFO *pTemp = s_ModList;
	while (pTemp != NULL) {
		pTemp->pszHeader = Log_CreateRtfHeader(pTemp);
		pTemp = pTemp->next;
	}
	return;
}

MODULEINFO* MM_FindModule(const char* pszModule)
{
	MODULEINFO *pTemp = s_ModList;

	if (!pszModule)
		return NULL;

	while (pTemp != NULL) {
		if (lstrcmpiA(pTemp->pszModule, pszModule) == 0)
			return pTemp;

		pTemp = pTemp->next;
	}
	return 0;
}

// stupid thing..
void MM_FixColors()
{
	MODULEINFO *pTemp = s_ModList;

	while (pTemp != NULL) {
		CheckColorsInModule(pTemp->pszModule);
		pTemp = pTemp->next;
	}
	return;
}

BOOL MM_RemoveAll(void)
{
	while (s_ModList != NULL) {
		MODULEINFO *pLast = s_ModList->next;
		mir_free(s_ModList->pszModule);
		mir_free(s_ModList->ptszModDispName);
		if (s_ModList->pszHeader)
			mir_free(s_ModList->pszHeader);
		mir_free(s_ModList->crColors);

		mir_free(s_ModList);
		s_ModList = pLast;
	}
	s_ModList = NULL;
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

	if (!TM_FindStatus(*ppStatusList, pszStatus)) {
		STATUSINFO *node = (STATUSINFO*)mir_calloc(sizeof(STATUSINFO));
		node->pszGroup = mir_tstrdup(pszStatus);
		node->hIcon = (HICON)(*iCount);
		while ((int)node->hIcon > STATUSICONCOUNT - 1)
			node->hIcon--;

		if (*ppStatusList == NULL) { // list is empty
			node->Status = 1;
			*ppStatusList = node;
			node->next = NULL;
		} else {
			node->Status = ppStatusList[0]->Status * 2;
			node->next = *ppStatusList;
			*ppStatusList = node;
		}
		return node;

	}
	return FALSE;
}

STATUSINFO * TM_FindStatus(STATUSINFO* pStatusList, const TCHAR* pszStatus)
{
	if (!pStatusList || !pszStatus)
		return NULL;

	for (STATUSINFO *si = pStatusList; si != NULL; si = si->next)
		if (lstrcmpi(si->pszGroup, pszStatus) == 0)
			return si;

	return 0;
}

WORD TM_StringToWord(STATUSINFO* pStatusList, const TCHAR* pszStatus)
{
	if (!pStatusList || !pszStatus)
		return 0;

	for (STATUSINFO *si = pStatusList; si != NULL; si = si->next) {
		if (lstrcmpi(si->pszGroup, pszStatus) == 0)
			return si->Status;

		if (si->next == NULL)
			return pStatusList->Status;
	}
	return 0;
}

TCHAR* TM_WordToString(STATUSINFO* pStatusList, WORD Status)
{
	if (!pStatusList)
		return NULL;

	for (STATUSINFO *si = pStatusList; si != NULL; si = si->next)
		if (si->Status&Status) {
			Status -= si->Status;
			if (Status == 0)
				return si->pszGroup;
		}

	return 0;
}

BOOL TM_RemoveAll(STATUSINFO** ppStatusList)
{
	if (!ppStatusList)
		return FALSE;

	while (*ppStatusList != NULL) {
		STATUSINFO *pLast = ppStatusList[0]->next;
		mir_free(ppStatusList[0]->pszGroup);
		if ((int)ppStatusList[0]->hIcon > 10)
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

//MAD: alternative sorting by Nullbie
static int sttCompareNicknames(const TCHAR *s1, const TCHAR *s2)
{
	if (!s1 && !s2) return 0;
	if (!s1 && s2) return +1;
	if (s1 && !s2) return -1;

	// skip rubbish
	while (*s1 && !_istalpha(*s1)) ++s1;
	while (*s2 && !_istalpha(*s2)) ++s2;

	// are there ~0veRy^kEwL_n1kz?
	if (!*s1 && !*s2) return 0;
	if (!*s1 && *s2) return +1;
	if (*s1 && !*s2) return -1;

	// compare tails
	return lstrcmpi(s1, s2);
}
//

static int UM_CompareItem(USERINFO * u1, const TCHAR* pszNick, WORD wStatus)
{
	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (int i=0; i < 8; i++) {
		if ((dw1 & 1) && !(dw2 & 1))
			return -1;

		if ((dw2 & 1) && !(dw1 & 1))
			return 1;

		if ((dw1 & 1) && (dw2 & 1)) {
			if (g_Settings.bAlternativeSorting)
				return sttCompareNicknames(u1->pszNick, pszNick);
			return lstrcmp(u1->pszNick, pszNick);
		}
		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	if (g_Settings.bAlternativeSorting)
		return sttCompareNicknames(u1->pszNick, pszNick);
	return lstrcmp(u1->pszNick, pszNick);
}

USERINFO* UM_SortUser(USERINFO** ppUserList, const TCHAR* pszUID)
{
	USERINFO *pTemp = *ppUserList, *pLast = NULL;
	if (!pTemp || !pszUID)
		return NULL;

	while (pTemp && lstrcmpi(pTemp->pszUID, pszUID)) {
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	if (pTemp == NULL)
		return NULL;

	USERINFO *node = pTemp;
	if (pLast)
		pLast->next = pTemp->next;
	else
		*ppUserList = pTemp->next;
	pTemp = *ppUserList;

	pLast = NULL;

	while (pTemp && UM_CompareItem(pTemp, node->pszNick, node->Status) <= 0) {
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	if (*ppUserList == NULL) { // list is empty
		*ppUserList = node;
		node->next = NULL;
	}
	else if (pLast) {
		node->next = pTemp;
		pLast->next = node;
	}
	else {
		node->next = *ppUserList;
		*ppUserList = node;
	}

	return node;
}

USERINFO* UM_AddUser(STATUSINFO* pStatusList, USERINFO** ppUserList, const TCHAR* pszUID, const TCHAR* pszNick, WORD wStatus)
{
	if (!pStatusList || !ppUserList)
		return NULL;

	USERINFO *pTemp = *ppUserList, *pLast = NULL;
	while (pTemp && UM_CompareItem(pTemp, pszNick, wStatus) <= 0) {
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	USERINFO *node = (USERINFO*)mir_calloc(sizeof(USERINFO));
	node->pszUID = mir_tstrdup(pszUID);

	if (*ppUserList == NULL) { // list is empty
		*ppUserList = node;
		node->next = NULL;
	}
	else if (pLast) {
		node->next = pTemp;
		pLast->next = node;
	}
	else {
		node->next = *ppUserList;
		*ppUserList = node;
	}

	return node;
}

USERINFO* UM_FindUser(USERINFO* pUserList, const TCHAR* pszUID)
{
	if (!pUserList || !pszUID)
		return NULL;

	for (USERINFO *pTemp = pUserList; pTemp != NULL; pTemp = pTemp->next)
		if (!lstrcmpi(pTemp->pszUID, pszUID))
			return pTemp;

	return 0;
}

USERINFO* UM_FindUserFromIndex(USERINFO* pUserList, int index)
{
	if (!pUserList)
		return NULL;

	int i=0;
	for (USERINFO *pTemp = pUserList; pTemp != NULL; pTemp = pTemp->next, i++)
		if (i == index)
			return pTemp;

	return NULL;
}

USERINFO* UM_GiveStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status)
{
	if (!pUserList || !pszUID)
		return NULL;

	for (USERINFO *pTemp = pUserList; pTemp != NULL; pTemp = pTemp->next)
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
			pTemp->Status |= status;
			return pTemp;
		}

	return 0;
}

USERINFO* UM_SetContactStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status)
{
	if (!pUserList || !pszUID)
		return NULL;

	for (USERINFO *pTemp = pUserList; pTemp != NULL; pTemp = pTemp->next)
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
			pTemp->ContactStatus = status;
			return pTemp;
		}

	return 0;
}

BOOL UM_SetStatusEx(USERINFO* pUserList, const TCHAR* pszText, int flags)
{
	bool bOnlyMe = (flags & GC_SSE_ONLYLISTED) != 0, bAwaySetStatus = (flags & GC_SSE_ONLINE) != 0, bOfflineSetStatus = (flags & GC_SSE_OFFLINE) != 0;
	char cDelimiter = (flags & GC_SSE_TABDELIMITED) ? '\t' : ' ';

	for (USERINFO *p = pUserList; p != NULL; p = p->next) {
		if (!bOnlyMe)
			p->iStatusEx = CHAT_STATUS_NORMAL;

		if (pszText == NULL)
			continue;
			
		TCHAR* s = (TCHAR*)_tcsstr(pszText, p->pszUID);
		if (s == NULL)
			continue;

		p->iStatusEx = CHAT_STATUS_NORMAL;
		if (s == pszText || s[-1] == cDelimiter) {
			int len = lstrlen(p->pszUID);
			if (s[len] == cDelimiter || s[len] == '\0') {
				if (!bOnlyMe || bAwaySetStatus)
					p->iStatusEx = CHAT_STATUS_AWAY;
				else if (bOfflineSetStatus)
					p->iStatusEx = CHAT_STATUS_OFFLINE;
			}
		}
	}
	return TRUE;
}

USERINFO* UM_TakeStatus(USERINFO* pUserList, const TCHAR* pszUID, WORD status)
{
	if (!pUserList || !pszUID)
		return NULL;

	for (USERINFO *pTemp = pUserList; pTemp != NULL; pTemp = pTemp->next)
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
			pTemp->Status &= ~status;
			return pTemp;
		}

	return 0;
}

TCHAR* UM_FindUserAutoComplete(USERINFO* pUserList, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	if (!pUserList || !pszOriginal || !pszCurrent)
		return NULL;

	TCHAR *pszName = NULL;
	for (USERINFO *pTemp = pUserList; pTemp != NULL; pTemp = pTemp->next)
		if (my_strstri(pTemp->pszNick, pszOriginal) == pTemp->pszNick)
			if (lstrcmpi(pTemp->pszNick, pszCurrent) > 0 && (!pszName || lstrcmpi(pTemp->pszNick, pszName) < 0))
				pszName = pTemp->pszNick;

	return pszName;
}

BOOL UM_RemoveUser(USERINFO** ppUserList, const TCHAR* pszUID)
{
	USERINFO *pTemp = *ppUserList, *pLast = NULL;

	if (!ppUserList || !pszUID)
		return FALSE;

	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
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

BOOL UM_RemoveAll(USERINFO** ppUserList)
{
	if (!ppUserList)
		return FALSE;

	while (*ppUserList != NULL) {
		USERINFO *pLast = ppUserList[0]->next;
		mir_free(ppUserList[0]->pszUID);
		mir_free(ppUserList[0]->pszNick);
		mir_free(*ppUserList);
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
	if (!ppLogListStart || !ppLogListEnd)
		return NULL;

	LOGINFO *node = (LOGINFO*)mir_calloc(sizeof(LOGINFO));

	if (*ppLogListStart == NULL) { // list is empty
		*ppLogListStart = node;
		*ppLogListEnd = node;
		node->next = NULL;
		node->prev = NULL;
	} else {
		ppLogListStart[0]->prev = node;
		node->next = *ppLogListStart;
		*ppLogListStart = node;
		ppLogListStart[0]->prev = NULL;
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

BOOL LM_RemoveAll(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
{
	while (*ppLogListStart != NULL) {
		LOGINFO *pLast = ppLogListStart[0]->next;
		mir_free(ppLogListStart[0]->ptszText);
		mir_free(ppLogListStart[0]->ptszNick);
		mir_free(ppLogListStart[0]->ptszStatus);
		mir_free(ppLogListStart[0]->ptszUserInfo);
		mir_free(*ppLogListStart);
		*ppLogListStart = pLast;
	}
	*ppLogListStart = NULL;
	*ppLogListEnd = NULL;
	return TRUE;
}
