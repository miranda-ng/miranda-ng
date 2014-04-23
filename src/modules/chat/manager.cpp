/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-14 Miranda NG project,
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

#include "..\..\core\commonheaders.h"

#include "chat.h"

#define WINDOWS_COMMANDS_MAX 30

CHAT_MANAGER ci;

MODULEINFO *m_ModList = 0;

static void SetActiveSessionEx(SESSION_INFO *si)
{
	if (si) {
		replaceStrT(ci.szActiveWndID, si->ptszID);
		replaceStr(ci.szActiveWndModule, si->pszModule);
	}
}

static void SetActiveSession(const TCHAR *pszID, const char *pszModule)
{
	SESSION_INFO *si = ci.SM_FindSession(pszID, pszModule);
	if (si)
		SetActiveSessionEx(si);
}

static SESSION_INFO* GetActiveSession(void)
{
	SESSION_INFO *si = ci.SM_FindSession(ci.szActiveWndID, ci.szActiveWndModule);
	if (si)
		return si;

	return ci.wndList;
}

//---------------------------------------------------
//		Session Manager functions
//
//		Keeps track of all sessions and its windows
//---------------------------------------------------

static SESSION_INFO* SM_AddSession(const TCHAR *pszID, const char *pszModule)
{
	if (!pszID || !pszModule)
		return NULL;

	if (ci.SM_FindSession(pszID, pszModule))
		return NULL;

	SESSION_INFO *node = (SESSION_INFO*)mir_calloc(g_cbSession);
	node->ptszID = mir_tstrdup(pszID);
	node->pszModule = mir_strdup(pszModule);

	if (ci.wndList == NULL) { // list is empty
		ci.wndList = node;
		node->next = NULL;
	}
	else {
		node->next = ci.wndList;
		ci.wndList = node;
	}
	return node;
}

static void SM_FreeSession(SESSION_INFO *si)
{
	// contact may have been deleted here already, since function may be called after deleting
	// contact so the handle may be invalid, therefore db_get_b shall return 0
	if (si->hContact && db_get_b(si->hContact, si->pszModule, "ChatRoom", 0) != 0) {
		ci.SetOffline(si->hContact, (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS) ? TRUE : FALSE);
		db_set_s(si->hContact, si->pszModule, "Topic", "");
		db_set_s(si->hContact, si->pszModule, "StatusBar", "");
		db_unset(si->hContact, "CList", "StatusMsg");
	}

	ci.UM_RemoveAll(&si->pUsers);
	ci.TM_RemoveAll(&si->pStatuses);
	ci.LM_RemoveAll(&si->pLog, &si->pLogEnd);

	si->iStatusCount = 0;
	si->nUsersInNicklist = 0;

	mir_free(si->pszModule);
	mir_free(si->ptszID);
	mir_free(si->ptszName);
	mir_free(si->ptszStatusbarText);
	mir_free(si->ptszTopic);

	while (si->lpCommands != NULL) {
		COMMANDINFO *pNext = si->lpCommands->next;
		mir_free(si->lpCommands->lpCommand);
		mir_free(si->lpCommands);
		si->lpCommands = pNext;
	}

	mir_free(si);
}

static int SM_RemoveSession(const TCHAR *pszID, const char *pszModule, BOOL removeContact)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList, *pLast = NULL;
	while (pTemp != NULL) {
		// match
		if ((!pszID && pTemp->iType != GCW_SERVER || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			DWORD dw = pTemp->dwItemData;

			if (ci.OnRemoveSession)
				ci.OnRemoveSession(pTemp);
			DoEventHook(pTemp->ptszID, pTemp->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)pTemp->dwItemData);

			if (pLast == NULL)
				ci.wndList = pTemp->next;
			else
				pLast->next = pTemp->next;

			// contact may have been deleted here already, since function may be called after deleting
			// contact so the handle may be invalid, therefore db_get_b shall return 0
			if (pTemp->hContact && removeContact)
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)pTemp->hContact, 0);

			SM_FreeSession(pTemp);

			if (pszID)
				return (int)dw;
			if (pLast)
				pTemp = pLast->next;
			else
				pTemp = ci.wndList;
		}
		else {
			pLast = pTemp;
			pTemp = pTemp->next;
		}
	}
	return FALSE;
}

static SESSION_INFO* SM_FindSession(const TCHAR *pszID, const char *pszModule)
{
	if (!pszID || !pszModule)
		return NULL;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule))
			return pTemp;

		pTemp = pTemp->next;
	}
	return NULL;
}

static BOOL SM_SetOffline(const TCHAR *pszID, const char *pszModule)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			ci.UM_RemoveAll(&pTemp->pUsers);
			pTemp->nUsersInNicklist = 0;
			if (pTemp->iType != GCW_SERVER)
				pTemp->bInitDone = FALSE;
			if (ci.OnOfflineSession)
				ci.OnOfflineSession(pTemp);
			if (pszID)
				return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_SetStatusEx(const TCHAR *pszID, const char *pszModule, const TCHAR* pszText, int flags)
{
	SESSION_INFO *pTemp = ci.wndList;

	if (!pszModule)
		return FALSE;

	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			ci.UM_SetStatusEx(pTemp->pUsers, pszText, flags);
			if (pTemp->hWnd)
				RedrawWindow(GetDlgItem(pTemp->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
			if (pszID)
				return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static HICON SM_GetStatusIcon(SESSION_INFO *si, USERINFO * ui)
{
	if (!ui || !si)
		return NULL;

	STATUSINFO *ti = ci.TM_FindStatus(si->pStatuses, ci.TM_WordToString(si->pStatuses, ui->Status));
	if (ti != NULL) {
		if ((UINT_PTR)ti->hIcon >= STATUSICONCOUNT)
			return ti->hIcon;

		return ci.hIcons[ICON_STATUS0 + (int)ti->hIcon];
	}
	return ci.hIcons[ICON_STATUS0];
}

static BOOL SM_AddEventToAllMatchingUID(GCEVENT *gce)
{
	int bManyFix = 0;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpiA(pTemp->pszModule, gce->pDest->pszModule)) {
			if (ci.UM_FindUser(pTemp->pUsers, gce->ptszUID)) {
				if (pTemp->bInitDone) {
					if (ci.OnEventBroadcast)
						ci.OnEventBroadcast(pTemp, gce);

					if (!(gce->dwFlags & GCEF_NOTNOTIFY))
						ci.DoSoundsFlashPopupTrayStuff(pTemp, gce, FALSE, bManyFix);

					bManyFix++;
					if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
						ci.LogToFile(pTemp, gce);
				}
			}
		}

		pTemp = pTemp->next;
	}

	return 0;
}

static BOOL SM_AddEvent(const TCHAR *pszID, const char *pszModule, GCEVENT *gce, BOOL bIsHighlighted)
{
	if (!pszID || !pszModule)
		return TRUE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			LOGINFO *li = ci.LM_AddEvent(&pTemp->pLog, &pTemp->pLogEnd);
			pTemp->iEventCount += 1;

			li->iType = gce->pDest->iType;
			li->ptszNick = mir_tstrdup(gce->ptszNick);
			li->ptszText = mir_tstrdup(gce->ptszText);
			li->ptszStatus = mir_tstrdup(gce->ptszStatus);
			li->ptszUserInfo = mir_tstrdup(gce->ptszUserInfo);

			li->bIsMe = gce->bIsMe;
			li->time = gce->time;
			li->bIsHighlighted = bIsHighlighted;

			if (g_Settings->iEventLimit > 0 && pTemp->iEventCount > g_Settings->iEventLimit + 20) {
				ci.LM_TrimLog(&pTemp->pLog, &pTemp->pLogEnd, pTemp->iEventCount - g_Settings->iEventLimit);
				pTemp->bTrimmed = true;
				pTemp->iEventCount = g_Settings->iEventLimit;
				return FALSE;
			}
			return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static USERINFO* SM_AddUser(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszNick, WORD wStatus)
{
	if (!pszID || !pszModule)
		return NULL;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			USERINFO *p = ci.UM_AddUser(pTemp->pStatuses, &pTemp->pUsers, pszUID, pszNick, wStatus);
			pTemp->nUsersInNicklist++;
			if (ci.OnAddUser)
				ci.OnAddUser(pTemp, p);
			return p;
		}
		pTemp = pTemp->next;
	}

	return 0;
}

static BOOL SM_MoveUser(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID)
{
	if (!pszID || !pszModule || !pszUID)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			ci.UM_SortUser(&pTemp->pUsers, pszUID);
			return TRUE;
		}
		pTemp = pTemp->next;
	}

	return FALSE;
}

static BOOL SM_RemoveUser(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID)
{
	if (!pszModule || !pszUID)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			DWORD dw;
			USERINFO *ui = ci.UM_FindUser(pTemp->pUsers, pszUID);
			if (ui) {
				pTemp->nUsersInNicklist--;
				if (ci.OnRemoveUser)
					ci.OnRemoveUser(pTemp, ui);

				dw = ci.UM_RemoveUser(&pTemp->pUsers, pszUID);

				if (pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, 0, 0);

				if (pszID)
					return TRUE;
			}
		}

		pTemp = pTemp->next;
	}

	return 0;
}

static USERINFO* SM_GetUserFromIndex(const TCHAR *pszID, const char *pszModule, int index)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule))
			return ci.UM_FindUserFromIndex(pTemp->pUsers, index);
		pTemp = pTemp->next;
	}

	return NULL;
}

STATUSINFO* SM_AddStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszStatus)
{
	if (!pszID || !pszModule)
		return NULL;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			STATUSINFO *ti = ci.TM_AddStatus(&pTemp->pStatuses, pszStatus, &pTemp->iStatusCount);
			if (ti)
				pTemp->iStatusCount++;
			if (ci.OnAddStatus)
				ci.OnAddStatus(pTemp, ti);
			return ti;
		}
		pTemp = pTemp->next;
	}

	return 0;
}

static BOOL SM_GiveStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszStatus)
{
	if (!pszID || !pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			USERINFO *ui = ci.UM_GiveStatus(pTemp->pUsers, pszUID, ci.TM_StringToWord(pTemp->pStatuses, pszStatus));
			if (ui) {
				SM_MoveUser(pTemp->ptszID, pTemp->pszModule, ui->pszUID);
				if (pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, 0, 0);
			}
			return TRUE;
		}
		pTemp = pTemp->next;
	}

	return FALSE;
}

static BOOL SM_SetContactStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, WORD wStatus)
{
	if (!pszID || !pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			USERINFO *ui = ci.UM_SetContactStatus(pTemp->pUsers, pszUID, wStatus);
			if (ui) {
				SM_MoveUser(pTemp->ptszID, pTemp->pszModule, ui->pszUID);
				if (pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, 0, 0);
			}
			return TRUE;
		}
		pTemp = pTemp->next;
	}

	return FALSE;
}

static BOOL SM_TakeStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszStatus)
{
	if (!pszID || !pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			USERINFO* ui = ci.UM_TakeStatus(pTemp->pUsers, pszUID, ci.TM_StringToWord(pTemp->pStatuses, pszStatus));
			if (ui) {
				SM_MoveUser(pTemp->ptszID, pTemp->pszModule, ui->pszUID);
				if (pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, 0, 0);
			}
			return TRUE;
		}
		pTemp = pTemp->next;
	}

	return FALSE;
}

static LRESULT SM_SendMessage(const TCHAR *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *pTemp = ci.wndList;

	while (pTemp && pszModule) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			if (pTemp->hWnd) {
				LRESULT i = SendMessage(pTemp->hWnd, msg, wParam, lParam);
				if (pszID)
					return i;
			}
			if (pszID)
				return 0;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

static BOOL SM_PostMessage(const TCHAR *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!pszID || !pszModule)
		return 0;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->ptszID, pszID) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			if (pTemp->hWnd)
				return PostMessage(pTemp->hWnd, msg, wParam, lParam);

			return FALSE;
		}
		pTemp = pTemp->next;
	}
	return FALSE;
}

static BOOL SM_BroadcastMessage(const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync)
{
	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (!pszModule || !lstrcmpiA(pTemp->pszModule, pszModule)) {
			if (pTemp->hWnd) {
				if (bAsync)
					PostMessage(pTemp->hWnd, msg, wParam, lParam);
				else
					SendMessage(pTemp->hWnd, msg, wParam, lParam);
			}
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_SetStatus(const TCHAR *pszID, const char *pszModule, int wStatus)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			pTemp->wStatus = wStatus;
			if (pTemp->hContact) {
				if (pTemp->iType != GCW_SERVER && wStatus != ID_STATUS_OFFLINE)
					db_unset(pTemp->hContact, "CList", "Hidden");

				db_set_w(pTemp->hContact, pTemp->pszModule, "Status", (WORD)wStatus);
			}

			if (ci.OnSetStatus)
				ci.OnSetStatus(pTemp, wStatus);

			if (pszID)
				return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_SendUserMessage(const TCHAR *pszID, const char *pszModule, const TCHAR* pszText)
{
	if (!pszModule || !pszText)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			if (pTemp->iType == GCW_CHATROOM || pTemp->iType == GCW_PRIVMESS)
				DoEventHook(pTemp->ptszID, pTemp->pszModule, GC_USER_MESSAGE, NULL, pszText, 0);
			if (pszID)
				return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_ChangeUID(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR* pszNewUID)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			USERINFO* ui = ci.UM_FindUser(pTemp->pUsers, pszUID);
			if (ui)
				replaceStrT(ui->pszUID, pszNewUID);

			if (pszID)
				return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_ChangeNick(const TCHAR *pszID, const char *pszModule, GCEVENT *gce)
{
	if (!pszModule)
		return FALSE;

	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if ((!pszID || !lstrcmpi(pTemp->ptszID, pszID)) && !lstrcmpiA(pTemp->pszModule, pszModule)) {
			USERINFO* ui = ci.UM_FindUser(pTemp->pUsers, gce->ptszUID);
			if (ui) {
				replaceStrT(ui->pszNick, gce->ptszText);
				SM_MoveUser(pTemp->ptszID, pTemp->pszModule, ui->pszUID);
				if (pTemp->hWnd)
					SendMessage(pTemp->hWnd, GC_UPDATENICKLIST, 0, 0);
				if (ci.OnChangeNick)
					ci.OnChangeNick(pTemp);
			}

			if (pszID)
				return TRUE;
		}
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_SetTabbedWindowHwnd(SESSION_INFO *si, HWND hwnd)
{
	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (si && si == pTemp)
			pTemp->hWnd = hwnd;
		else
			pTemp->hWnd = NULL;
		pTemp = pTemp->next;
	}
	return TRUE;
}

static BOOL SM_RemoveAll(void)
{
	while (ci.wndList) {
		SESSION_INFO *pLast = ci.wndList->next;

		if (ci.wndList->hWnd)
			SendMessage(ci.wndList->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
		DoEventHook(ci.wndList->ptszID, ci.wndList->pszModule, GC_SESSION_TERMINATE, NULL, NULL, (DWORD)ci.wndList->dwItemData);

		SM_FreeSession(ci.wndList);
		ci.wndList = pLast;
	}
	ci.wndList = NULL;
	return TRUE;
}

static void SM_AddCommand(const TCHAR *pszID, const char *pszModule, const char* lpNewCommand)
{
	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (lstrcmpi(pTemp->ptszID, pszID) == 0 && lstrcmpiA(pTemp->pszModule, pszModule) == 0) { // match
			COMMANDINFO *node = (COMMANDINFO *)mir_alloc(sizeof(COMMANDINFO));
			node->lpCommand = mir_strdup(lpNewCommand);
			node->last = NULL; // always added at beginning!

			// new commands are added at start
			if (pTemp->lpCommands == NULL) {
				node->next = NULL;
				pTemp->lpCommands = node;
			}
			else {
				node->next = pTemp->lpCommands;
				pTemp->lpCommands->last = node; // hmm, weird
				pTemp->lpCommands = node;
			}
			pTemp->lpCurrentCommand = NULL; // current command
			pTemp->wCommandsNum++;

			if (pTemp->wCommandsNum > WINDOWS_COMMANDS_MAX) {
				COMMANDINFO *pCurComm = pTemp->lpCommands;
				while (pCurComm->next != NULL) { pCurComm = pCurComm->next; }
				COMMANDINFO *pLast = pCurComm->last;
				mir_free(pCurComm->lpCommand);
				mir_free(pCurComm);
				pLast->next = NULL;
				// done
				pTemp->wCommandsNum--;
			}
		}
		pTemp = pTemp->next;
	}
}

static char* SM_GetPrevCommand(const TCHAR *pszID, const char *pszModule) // get previous command. returns NULL if previous command does not exist. current command remains as it was.
{
	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (lstrcmpi(pTemp->ptszID, pszID) == 0 && lstrcmpiA(pTemp->pszModule, pszModule) == 0) { // match
			COMMANDINFO *pPrevCmd = NULL;
			if (pTemp->lpCurrentCommand != NULL) {
				if (pTemp->lpCurrentCommand->next != NULL) // not NULL
					pPrevCmd = pTemp->lpCurrentCommand->next; // next command (newest at beginning)
				else
					pPrevCmd = pTemp->lpCurrentCommand;
			}
			else pPrevCmd = pTemp->lpCommands;

			pTemp->lpCurrentCommand = pPrevCmd; // make it the new command
			return(((pPrevCmd) ? (pPrevCmd->lpCommand) : (NULL)));
		}
		pTemp = pTemp->next;
	}
	return(NULL);
}

static char* SM_GetNextCommand(const TCHAR *pszID, const char *pszModule) // get next command. returns NULL if next command does not exist. current command becomes NULL (a prev command after this one will get you the last command)
{
	SESSION_INFO *pTemp = ci.wndList;
	while (pTemp != NULL) {
		if (lstrcmpi(pTemp->ptszID, pszID) == 0 && lstrcmpiA(pTemp->pszModule, pszModule) == 0) { // match
			COMMANDINFO *pNextCmd = NULL;
			if (pTemp->lpCurrentCommand != NULL)
				pNextCmd = pTemp->lpCurrentCommand->last; // last command (newest at beginning)

			pTemp->lpCurrentCommand = pNextCmd; // make it the new command
			return(((pNextCmd) ? (pNextCmd->lpCommand) : (NULL)));
		}
		pTemp = pTemp->next;
	}
	return(NULL);
}

static int SM_GetCount(const char *pszModule)
{
	SESSION_INFO *pTemp = ci.wndList;
	int count = 0;

	while (pTemp != NULL) {
		if (!lstrcmpiA(pszModule, pTemp->pszModule))
			count++;

		pTemp = pTemp->next;
	}
	return count;
}

static SESSION_INFO* SM_FindSessionByIndex(const char *pszModule, int iItem)
{
	SESSION_INFO *pTemp = ci.wndList;
	int count = 0;
	while (pTemp != NULL) {
		if (!lstrcmpiA(pszModule, pTemp->pszModule)) {
			if (iItem == count)
				return pTemp;
			else
				count++;
		}

		pTemp = pTemp->next;
	}
	return NULL;

}

static char* SM_GetUsers(SESSION_INFO *si)
{
	SESSION_INFO *pTemp = ci.wndList;
	USERINFO *utemp = NULL;
	char* p = NULL;
	int alloced = 0;

	if (si == NULL)
		return NULL;

	while (pTemp != NULL) {
		if (si == pTemp) {
			if ((utemp = pTemp->pUsers) == NULL)
				return NULL;

			break;
		}
		pTemp = pTemp->next;
	}

	do {
		int pLen = lstrlenA(p), nameLen = lstrlen(utemp->pszUID);
		if (pLen + nameLen + 2 > alloced)
			p = (char*)mir_realloc(p, alloced += 4096);

		WideCharToMultiByte(CP_ACP, 0, utemp->pszUID, -1, p + pLen, nameLen + 1, 0, 0);
		lstrcpyA(p + pLen + nameLen, " ");
		utemp = utemp->next;
	}
		while (utemp != NULL);
	return p;
}

static void SM_InvalidateLogDirectories()
{
	for (SESSION_INFO *si = ci.wndList; si; si = si->next)
		si->pszLogFileName[0] = si->pszLogFileName[1] = 0;
}






//---------------------------------------------------
//		Module Manager functions
//
//		Necessary to keep track of all modules
//		that has registered with the plugin
//---------------------------------------------------

static MODULEINFO* MM_AddModule(const char *pszModule)
{
	if (pszModule == NULL)
		return NULL;

	if (ci.MM_FindModule(pszModule))
		return NULL;

	MODULEINFO *node = (MODULEINFO*)mir_calloc(g_cbModuleInfo);
	replaceStr(node->pszModule, pszModule);
	if (ci.OnCreateModule)
		ci.OnCreateModule(node);

	if (m_ModList == NULL) { // list is empty
		m_ModList = node;
		node->next = NULL;
	}
	else {
		node->next = m_ModList;
		m_ModList = node;
	}
	return node;
}

static void MM_IconsChanged(void)
{
	LoadChatIcons();

	MODULEINFO *pTemp = m_ModList;
	while (pTemp != NULL) {
		Safe_DestroyIcon(pTemp->hOnlineIcon);
		Safe_DestroyIcon(pTemp->hOfflineIcon);
		Safe_DestroyIcon(pTemp->hOnlineTalkIcon);
		Safe_DestroyIcon(pTemp->hOfflineTalkIcon);

		if (ci.OnCreateModule) // recreate icons
			ci.OnCreateModule(pTemp);

		pTemp = pTemp->next;
	}
}

static void MM_FontsChanged(void)
{
	MODULEINFO *pTemp = m_ModList;
	while (pTemp != NULL) {
		pTemp->pszHeader = ci.Log_CreateRtfHeader(pTemp);
		pTemp = pTemp->next;
	}
	return;
}

static MODULEINFO* MM_FindModule(const char *pszModule)
{
	if (!pszModule)
		return NULL;

	MODULEINFO *pTemp = m_ModList;
	while (pTemp != NULL) {
		if (lstrcmpiA(pTemp->pszModule, pszModule) == 0)
			return pTemp;

		pTemp = pTemp->next;
	}
	return 0;
}

// stupid thing..
static void MM_FixColors()
{
	MODULEINFO *pTemp = m_ModList;
	while (pTemp != NULL) {
		CheckColorsInModule(pTemp->pszModule);
		pTemp = pTemp->next;
	}
	return;
}

static BOOL MM_RemoveAll(void)
{
	while (m_ModList != NULL) {
		MODULEINFO *pLast = m_ModList->next;
		mir_free(m_ModList->pszModule);
		mir_free(m_ModList->ptszModDispName);
		mir_free(m_ModList->pszHeader);
		mir_free(m_ModList->crColors);

		Safe_DestroyIcon(m_ModList->hOnlineIcon);
		Safe_DestroyIcon(m_ModList->hOfflineIcon);
		Safe_DestroyIcon(m_ModList->hOnlineTalkIcon);
		Safe_DestroyIcon(m_ModList->hOfflineTalkIcon);

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

static STATUSINFO* TM_AddStatus(STATUSINFO** ppStatusList, const TCHAR *pszStatus, int* iCount)
{
	if (!ppStatusList || !pszStatus)
		return NULL;

	if (!ci.TM_FindStatus(*ppStatusList, pszStatus)) {
		STATUSINFO *node = (STATUSINFO*)mir_alloc(sizeof(STATUSINFO));
		ZeroMemory(node, sizeof(STATUSINFO));
		replaceStrT(node->pszGroup, pszStatus);
		node->hIcon = (HICON)(*iCount);
		while ((int)node->hIcon > STATUSICONCOUNT - 1)
			node->hIcon--;

		if (*ppStatusList == NULL) { // list is empty
			node->Status = 1;
			*ppStatusList = node;
			node->next = NULL;
		}
		else {
			node->Status = ppStatusList[0]->Status * 2;
			node->next = *ppStatusList;
			*ppStatusList = node;
		}
		return node;

	}
	return FALSE;
}

static STATUSINFO* TM_FindStatus(STATUSINFO* pStatusList, const TCHAR *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return NULL;

	STATUSINFO *pTemp = pStatusList;
	while (pTemp != NULL) {
		if (lstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp;

		pTemp = pTemp->next;
	}
	return 0;
}

static WORD TM_StringToWord(STATUSINFO* pStatusList, const TCHAR *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return 0;

	STATUSINFO *pTemp = pStatusList;
	while (pTemp != NULL) {
		if (lstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp->Status;

		if (pTemp->next == NULL)
			return pStatusList->Status;

		pTemp = pTemp->next;
	}
	return 0;
}

static TCHAR* TM_WordToString(STATUSINFO* pStatusList, WORD Status)
{
	if (!pStatusList)
		return NULL;

	STATUSINFO *pTemp = pStatusList;
	while (pTemp != NULL) {
		if (pTemp->Status&Status) {
			Status -= pTemp->Status;
			if (Status == 0)
				return pTemp->pszGroup;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

static BOOL TM_RemoveAll(STATUSINFO** ppStatusList)
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

static int UM_CompareItem(USERINFO * u1, const TCHAR *pszNick, WORD wStatus)
{
	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (int i = 0; i < 8; i++) {
		if ((dw1 & 1) && !(dw2 & 1))
			return -1;
		if ((dw2 & 1) && !(dw1 & 1))
			return 1;
		if ((dw1 & 1) && (dw2 & 1))
			return lstrcmp(u1->pszNick, pszNick);

		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	return lstrcmp(u1->pszNick, pszNick);
}

static USERINFO* UM_SortUser(USERINFO** ppUserList, const TCHAR *pszUID)
{
	USERINFO *pTemp = *ppUserList, *pLast = NULL;
	USERINFO *node = NULL;

	if (!pTemp || !pszUID)
		return NULL;

	while (pTemp && lstrcmpi(pTemp->pszUID, pszUID)) {
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	if (pTemp) {
		node = pTemp;
		if (pLast)
			pLast->next = pTemp->next;
		else
			*ppUserList = pTemp->next;
		pTemp = *ppUserList;

		pLast = NULL;

		while (pTemp && ci.UM_CompareItem(pTemp, node->pszNick, node->Status) <= 0) {
			pLast = pTemp;
			pTemp = pTemp->next;
		}

		if (*ppUserList == NULL) { // list is empty
			*ppUserList = node;
			node->next = NULL;
		}
		else {
			if (pLast) {
				node->next = pTemp;
				pLast->next = node;
			}
			else {
				node->next = *ppUserList;
				*ppUserList = node;
			}
		}

		return node;
	}
	return NULL;
}

USERINFO* UM_AddUser(STATUSINFO* pStatusList, USERINFO** ppUserList, const TCHAR *pszUID, const TCHAR *pszNick, WORD wStatus)
{
	if (!pStatusList || !ppUserList)
		return NULL;

	USERINFO *pTemp = *ppUserList, *pLast = NULL;
	while (pTemp && ci.UM_CompareItem(pTemp, pszNick, wStatus) <= 0) {
		pLast = pTemp;
		pTemp = pTemp->next;
	}

	//	if (!UM_FindUser(*ppUserList, pszUI, wStatus)
	USERINFO *node = (USERINFO*)mir_calloc(sizeof(USERINFO));
	replaceStrT(node->pszUID, pszUID);

	if (*ppUserList == NULL) { // list is empty
		*ppUserList = node;
		node->next = NULL;
	}
	else {
		if (pLast) {
			node->next = pTemp;
			pLast->next = node;
		}
		else {
			node->next = *ppUserList;
			*ppUserList = node;
		}
	}

	return node;
}

static USERINFO* UM_FindUser(USERINFO* pUserList, const TCHAR *pszUID)
{
	if (!pUserList || !pszUID)
		return NULL;

	USERINFO *pTemp = pUserList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->pszUID, pszUID))
			return pTemp;

		pTemp = pTemp->next;
	}
	return 0;
}

static USERINFO* UM_FindUserFromIndex(USERINFO* pUserList, int index)
{
	if (!pUserList)
		return NULL;

	int i = 0;
	USERINFO *pTemp = pUserList;
	while (pTemp != NULL) {
		if (i == index)
			return pTemp;

		pTemp = pTemp->next;
		i++;
	}
	return NULL;
}

static USERINFO* UM_GiveStatus(USERINFO* pUserList, const TCHAR *pszUID, WORD status)
{
	if (!pUserList || !pszUID)
		return NULL;

	USERINFO *pTemp = pUserList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
			pTemp->Status |= status;
			return pTemp;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

static USERINFO* UM_SetContactStatus(USERINFO* pUserList, const TCHAR *pszUID, WORD status)
{
	if (!pUserList || !pszUID)
		return NULL;

	USERINFO *pTemp = pUserList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
			pTemp->ContactStatus = status;
			return pTemp;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

static BOOL UM_SetStatusEx(USERINFO* pUserList, const TCHAR* pszText, int flags)
{
	USERINFO *pTemp = pUserList;
	int bOnlyMe = (flags & GC_SSE_ONLYLISTED) != 0, bSetStatus = (flags & GC_SSE_ONLINE) != 0;
	char cDelimiter = (flags & GC_SSE_TABDELIMITED) ? '\t' : ' ';

	while (pTemp != NULL) {
		if (!bOnlyMe)
			pTemp->iStatusEx = 0;

		if (pszText != NULL) {
			TCHAR* s = (TCHAR *)_tcsstr(pszText, pTemp->pszUID);
			if (s) {
				pTemp->iStatusEx = 0;
				if (s == pszText || s[-1] == cDelimiter) {
					int len = lstrlen(pTemp->pszUID);
					if (s[len] == cDelimiter || s[len] == '\0')
						pTemp->iStatusEx = (!bOnlyMe || bSetStatus) ? 1 : 0;
				}
			}
		}

		pTemp = pTemp->next;
	}
	return TRUE;
}

static USERINFO* UM_TakeStatus(USERINFO* pUserList, const TCHAR *pszUID, WORD status)
{
	if (!pUserList || !pszUID)
		return NULL;

	USERINFO *pTemp = pUserList;
	while (pTemp != NULL) {
		if (!lstrcmpi(pTemp->pszUID, pszUID)) {
			pTemp->Status &= ~status;
			return pTemp;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

static TCHAR* UM_FindUserAutoComplete(USERINFO* pUserList, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	if (!pUserList || !pszOriginal || !pszCurrent)
		return NULL;

	TCHAR* pszName = NULL;
	USERINFO *pTemp = pUserList;
	while (pTemp != NULL) {
		if (pTemp->pszNick && my_strstri(pTemp->pszNick, pszOriginal) == pTemp->pszNick)
			if (lstrcmpi(pTemp->pszNick, pszCurrent) > 0 && (!pszName || lstrcmpi(pTemp->pszNick, pszName) < 0))
				pszName = pTemp->pszNick;

		pTemp = pTemp->next;
	}
	return pszName;
}

static BOOL UM_RemoveUser(USERINFO** ppUserList, const TCHAR *pszUID)
{
	if (!ppUserList || !pszUID)
		return FALSE;

	USERINFO *pTemp = *ppUserList, *pLast = NULL;
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

static BOOL UM_RemoveAll(USERINFO** ppUserList)
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

static LOGINFO* LM_AddEvent(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
{
	if (!ppLogListStart || !ppLogListEnd)
		return NULL;

	LOGINFO *node = (LOGINFO*)mir_calloc(sizeof(LOGINFO));
	if (*ppLogListStart == NULL) { // list is empty
		*ppLogListStart = node;
		*ppLogListEnd = node;
		node->next = NULL;
		node->prev = NULL;
	}
	else {
		ppLogListStart[0]->prev = node;
		node->next = *ppLogListStart;
		*ppLogListStart = node;
		ppLogListStart[0]->prev = NULL;
	}

	return node;
}

static BOOL LM_TrimLog(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd, int iCount)
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

static BOOL LM_RemoveAll(LOGINFO** ppLogListStart, LOGINFO** ppLogListEnd)
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

INT_PTR SvcGetChatManager(WPARAM wParam, LPARAM lParam)
{
	// wipe out old junk
	memset(PBYTE(&ci) + offsetof(CHAT_MANAGER, OnCreateModule), 0, sizeof(CHAT_MANAGER)-offsetof(CHAT_MANAGER, OnCreateModule));

	CHAT_MANAGER_INITDATA *pInit = (CHAT_MANAGER_INITDATA*)lParam;
	if (g_cbSession) { // reallocate old sessions
		mir_cslock lck(cs);
		SESSION_INFO *pPrev = NULL;
		for (SESSION_INFO *p = ci.wndList; p; p = p->next) {
			SESSION_INFO *p1 = (SESSION_INFO*)mir_realloc(p, pInit->cbSession);
			memset(PBYTE(p1) + sizeof(GCSessionInfoBase), 0, pInit->cbSession - sizeof(GCSessionInfoBase));
			if (p1 != p) { // realloc could change a pointer, reinsert a structure
				if (ci.wndList == p)
					ci.wndList = p1;
				if (pPrev != NULL)
					pPrev->next = p1;
				p = p1;
			}
			pPrev = p;
		}
	}
	if (g_cbModuleInfo) { // reallocate old modules
		mir_cslock lck(cs);
		MODULEINFO *pPrev = NULL;
		for (MODULEINFO *p = m_ModList; p; p = p->next) {
			MODULEINFO *p1 = (MODULEINFO*)mir_realloc(p, pInit->cbModuleInfo);
			memset(PBYTE(p1) + sizeof(GCModuleInfoBase), 0, pInit->cbModuleInfo - sizeof(GCModuleInfoBase));
			if (p1 != p) { // realloc could change a pointer, reinsert a structure
				if (m_ModList == p)
					m_ModList = p1;
				if (pPrev != NULL)
					pPrev->next = p1;
				p = p1;
			}
			pPrev = p;
		}
	}
	g_Settings = pInit->pSettings;
	g_szFontGroup = pInit->szFontGroup;
	g_cbSession = pInit->cbSession;
	g_cbModuleInfo = pInit->cbModuleInfo;
	g_iFontMode = pInit->iFontMode;
	g_iChatLang = (int)wParam;

	ci.SetActiveSession = SetActiveSession;
	ci.SetActiveSessionEx = SetActiveSessionEx;
	ci.GetActiveSession = GetActiveSession;
	ci.SM_AddSession = SM_AddSession;
	ci.SM_RemoveSession = SM_RemoveSession;
	ci.SM_FindSession = SM_FindSession;
	ci.SM_AddUser = SM_AddUser;
	ci.SM_ChangeUID = SM_ChangeUID;
	ci.SM_ChangeNick = SM_ChangeNick;
	ci.SM_RemoveUser = SM_RemoveUser;
	ci.SM_SetOffline = SM_SetOffline;
	ci.SM_SetTabbedWindowHwnd = SM_SetTabbedWindowHwnd;
	ci.SM_GetStatusIcon = SM_GetStatusIcon;
	ci.SM_SetStatus = SM_SetStatus;
	ci.SM_SetStatusEx = SM_SetStatusEx;
	ci.SM_SendUserMessage = SM_SendUserMessage;
	ci.SM_AddStatus = SM_AddStatus;
	ci.SM_AddEventToAllMatchingUID = SM_AddEventToAllMatchingUID;
	ci.SM_AddEvent = SM_AddEvent;
	ci.SM_SendMessage = SM_SendMessage;
	ci.SM_PostMessage = SM_PostMessage;
	ci.SM_BroadcastMessage = SM_BroadcastMessage;
	ci.SM_RemoveAll = SM_RemoveAll;
	ci.SM_GiveStatus = SM_GiveStatus;
	ci.SM_SetContactStatus = SM_SetContactStatus;
	ci.SM_TakeStatus = SM_TakeStatus;
	ci.SM_MoveUser = SM_MoveUser;
	ci.SM_AddCommand = SM_AddCommand;
	ci.SM_GetPrevCommand = SM_GetPrevCommand;
	ci.SM_GetNextCommand = SM_GetNextCommand;
	ci.SM_GetCount = SM_GetCount;
	ci.SM_FindSessionByIndex = SM_FindSessionByIndex;
	ci.SM_GetUsers = SM_GetUsers;
	ci.SM_GetUserFromIndex = SM_GetUserFromIndex;
	ci.SM_InvalidateLogDirectories = SM_InvalidateLogDirectories;

	ci.MM_AddModule = MM_AddModule;
	ci.MM_FindModule = MM_FindModule;
	ci.MM_FixColors = MM_FixColors;
	ci.MM_FontsChanged = MM_FontsChanged;
	ci.MM_IconsChanged = MM_IconsChanged;
	ci.MM_RemoveAll = MM_RemoveAll;

	ci.TM_AddStatus = TM_AddStatus;
	ci.TM_FindStatus = TM_FindStatus;
	ci.TM_StringToWord = TM_StringToWord;
	ci.TM_WordToString = TM_WordToString;
	ci.TM_RemoveAll = TM_RemoveAll;

	ci.UM_SetStatusEx = UM_SetStatusEx;
	ci.UM_AddUser = UM_AddUser;
	ci.UM_SortUser = UM_SortUser;
	ci.UM_FindUser = UM_FindUser;
	ci.UM_FindUserFromIndex = UM_FindUserFromIndex;
	ci.UM_GiveStatus = UM_GiveStatus;
	ci.UM_SetContactStatus = UM_SetContactStatus;
	ci.UM_TakeStatus = UM_TakeStatus;
	ci.UM_FindUserAutoComplete = UM_FindUserAutoComplete;
	ci.UM_RemoveUser = UM_RemoveUser;
	ci.UM_RemoveAll = UM_RemoveAll;
	ci.UM_CompareItem = UM_CompareItem;

	ci.LM_AddEvent = LM_AddEvent;
	ci.LM_TrimLog = LM_TrimLog;
	ci.LM_RemoveAll = LM_RemoveAll;

	ci.AddRoom = AddRoom;
	ci.SetOffline = SetOffline;
	ci.SetAllOffline = SetAllOffline;
	ci.AddEvent = AddEvent;
	ci.FindRoom = FindRoom;

	ci.Log_CreateRTF = Log_CreateRTF;
	ci.Log_CreateRtfHeader = Log_CreateRtfHeader;
	ci.LoadMsgDlgFont = LoadMsgDlgFont;
	ci.MakeTimeStamp = MakeTimeStamp;

	ci.DoEventHook = DoEventHook;
	ci.DoEventHookAsync = DoEventHookAsync;

	ci.DoSoundsFlashPopupTrayStuff = DoSoundsFlashPopupTrayStuff;
	ci.DoTrayIcon = DoTrayIcon;
	ci.DoPopup = DoPopup;
	ci.ShowPopup = ShowPopup;
	ci.LogToFile = LogToFile;
	ci.GetChatLogsFilename = GetChatLogsFilename;
	ci.Log_SetStyle = Log_SetStyle;

	ci.IsHighlighted = IsHighlighted;
	ci.RemoveFormatting = RemoveFormatting;
	ci.ReloadSettings = LoadGlobalSettings;

	ci.pLogIconBmpBits = pLogIconBmpBits;
	ci.logIconBmpSize = logIconBmpSize;

	RegisterFonts();
	OptionsInit();
	return (INT_PTR)&ci;
}
