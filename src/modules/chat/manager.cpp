/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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
		if ((!pszID && pTemp->iType != GCW_SERVER || !mir_tstrcmpi(pTemp->ptszID, pszID)) && !mir_strcmpi(pTemp->pszModule, pszModule)) {
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

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next)
		if (!mir_tstrcmpi(si->ptszID, pszID) && !mir_strcmpi(si->pszModule, pszModule))
			return si;

	return NULL;
}

static BOOL SM_SetOffline(const TCHAR *pszID, const char *pszModule)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;
		
		ci.UM_RemoveAll(&si->pUsers);
		si->nUsersInNicklist = 0;
		if (si->iType != GCW_SERVER)
			si->bInitDone = FALSE;
		if (ci.OnOfflineSession)
			ci.OnOfflineSession(si);
		if (pszID)
			return TRUE;
	}
	return TRUE;
}

static BOOL SM_SetStatusEx(const TCHAR *pszID, const char *pszModule, const TCHAR* pszText, int flags)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		ci.UM_SetStatusEx(si->pUsers, pszText, flags);
		if (si->hWnd)
			RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
		if (pszID)
			return TRUE;
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

	for (SESSION_INFO *p = ci.wndList; p != NULL; p = p->next) {
		if (!p->bInitDone || mir_strcmpi(p->pszModule, gce->pDest->pszModule))
			continue;

		if (!ci.UM_FindUser(p->pUsers, gce->ptszUID))
			continue;

		if (ci.OnEventBroadcast)
			ci.OnEventBroadcast(p, gce);

		if (!(gce->dwFlags & GCEF_NOTNOTIFY))
			ci.DoSoundsFlashPopupTrayStuff(p, gce, FALSE, bManyFix);

		bManyFix++;
		if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
			ci.LogToFile(p, gce);
	}

	return 0;
}

static BOOL SM_AddEvent(const TCHAR *pszID, const char *pszModule, GCEVENT *gce, BOOL bIsHighlighted)
{
	SESSION_INFO *p = SM_FindSession(pszID, pszModule);
	if (p == NULL)
		return TRUE;

	LOGINFO *li = ci.LM_AddEvent(&p->pLog, &p->pLogEnd);
	p->iEventCount += 1;

	li->iType = gce->pDest->iType;
	li->ptszNick = mir_tstrdup(gce->ptszNick);
	li->ptszText = mir_tstrdup(gce->ptszText);
	li->ptszStatus = mir_tstrdup(gce->ptszStatus);
	li->ptszUserInfo = mir_tstrdup(gce->ptszUserInfo);

	li->bIsMe = gce->bIsMe;
	li->time = gce->time;
	li->bIsHighlighted = bIsHighlighted;

	if (g_Settings->iEventLimit > 0 && p->iEventCount > g_Settings->iEventLimit + 20) {
		ci.LM_TrimLog(&p->pLog, &p->pLogEnd, p->iEventCount - g_Settings->iEventLimit);
		p->bTrimmed = true;
		p->iEventCount = g_Settings->iEventLimit;
		return FALSE;
	}
	return TRUE;
}

static USERINFO* SM_AddUser(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszNick, WORD wStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	USERINFO *p = ci.UM_AddUser(si->pStatuses, &si->pUsers, pszUID, pszNick, wStatus);
	si->nUsersInNicklist++;
	if (ci.OnAddUser)
		ci.OnAddUser(si, p);
	return p;
}

static BOOL SM_MoveUser(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID)
{
	if (!pszUID)
		return FALSE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	ci.UM_SortUser(&si->pUsers, pszUID);
	return TRUE;
}

static BOOL SM_RemoveUser(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID)
{
	if (!pszModule || !pszUID)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		USERINFO *ui = ci.UM_FindUser(si->pUsers, pszUID);
		if (ui) {
			si->nUsersInNicklist--;
			if (ci.OnRemoveUser)
				ci.OnRemoveUser(si, ui);

			ci.UM_RemoveUser(&si->pUsers, pszUID);

			if (si->hWnd)
				SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);

			if (pszID)
				return TRUE;
		}
	}

	return 0;
}

static USERINFO* SM_GetUserFromIndex(const TCHAR *pszID, const char *pszModule, int index)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	return (si == NULL) ? NULL : ci.UM_FindUserFromIndex(si->pUsers, index);
}

STATUSINFO* SM_AddStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	STATUSINFO *ti = ci.TM_AddStatus(&si->pStatuses, pszStatus, &si->iStatusCount);
	if (ti)
		si->iStatusCount++;
	if (ci.OnAddStatus)
		ci.OnAddStatus(si, ti);
	return ti;
}

static BOOL SM_GiveStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;
	
	USERINFO *ui = ci.UM_GiveStatus(si->pUsers, pszUID, ci.TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
	return TRUE;
}

static BOOL SM_SetContactStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, WORD wStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	USERINFO *ui = ci.UM_SetContactStatus(si->pUsers, pszUID, wStatus);
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
	return TRUE;
}

static BOOL SM_TakeStatus(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR *pszStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	USERINFO *ui = ci.UM_TakeStatus(si->pUsers, pszUID, ci.TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
	return TRUE;
}

static LRESULT SM_SendMessage(const TCHAR *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (pszModule == NULL)
		return 0;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		if (si->hWnd) {
			LRESULT i = SendMessage(si->hWnd, msg, wParam, lParam);
			if (pszID)
				return i;
		}
		if (pszID)
			return 0;
	}
	return 0;
}

static BOOL SM_PostMessage(const TCHAR *pszID, const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return FALSE;

	if (si->hWnd)
		return PostMessage(si->hWnd, msg, wParam, lParam);
	return FALSE;
}

static BOOL SM_BroadcastMessage(const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync)
{
	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if (pszModule && _strcmpi(si->pszModule, pszModule))
			continue;

		if (si->hWnd) {
			if (bAsync)
				PostMessage(si->hWnd, msg, wParam, lParam);
			else
				SendMessage(si->hWnd, msg, wParam, lParam);
		}
	}
	return TRUE;
}

static BOOL SM_SetStatus(const TCHAR *pszID, const char *pszModule, int wStatus)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		si->wStatus = wStatus;
		if (si->hContact) {
			if (si->iType != GCW_SERVER && wStatus != ID_STATUS_OFFLINE)
				db_unset(si->hContact, "CList", "Hidden");

			db_set_w(si->hContact, si->pszModule, "Status", (WORD)wStatus);
		}

		if (ci.OnSetStatus)
			ci.OnSetStatus(si, wStatus);

		if (pszID)
			return TRUE;
	}
	return TRUE;
}

static BOOL SM_SendUserMessage(const TCHAR *pszID, const char *pszModule, const TCHAR* pszText)
{
	if (!pszModule || !pszText)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		if (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS)
			DoEventHook(si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, pszText, 0);
		if (pszID)
			return TRUE;
	}
	return TRUE;
}

static BOOL SM_ChangeUID(const TCHAR *pszID, const char *pszModule, const TCHAR *pszUID, const TCHAR* pszNewUID)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((pszID && _tcsicmp(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		USERINFO* ui = ci.UM_FindUser(si->pUsers, pszUID);
		if (ui)
			replaceStrT(ui->pszUID, pszNewUID);
		if (pszID)
			return TRUE;
	}
	return TRUE;
}

static BOOL SM_ChangeNick(const TCHAR *pszID, const char *pszModule, GCEVENT *gce)
{
	if (!pszModule)
		return FALSE;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if ((!pszID || !mir_tstrcmpi(si->ptszID, pszID)) && !mir_strcmpi(si->pszModule, pszModule)) {
			USERINFO *ui = ci.UM_FindUser(si->pUsers, gce->ptszUID);
			if (ui) {
				replaceStrT(ui->pszNick, gce->ptszText);
				SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
				if (si->hWnd)
					SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
				if (ci.OnChangeNick)
					ci.OnChangeNick(si);
			}

			if (pszID)
				return TRUE;
		}
	}
	return TRUE;
}

static BOOL SM_SetTabbedWindowHwnd(SESSION_INFO *si, HWND hwnd)
{
	for (SESSION_INFO *p = ci.wndList; p != NULL; p = p->next) {
		if (si && si == p)
			p->hWnd = hwnd;
		else
			p->hWnd = NULL;
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
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return;

	COMMANDINFO *node = (COMMANDINFO *)mir_alloc(sizeof(COMMANDINFO));
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
		COMMANDINFO *pCurComm = si->lpCommands;
		while (pCurComm->next != NULL) { pCurComm = pCurComm->next; }
		COMMANDINFO *pLast = pCurComm->last;
		mir_free(pCurComm->lpCommand);
		mir_free(pCurComm);
		pLast->next = NULL;
		// done
		si->wCommandsNum--;
	}
}

static char* SM_GetPrevCommand(const TCHAR *pszID, const char *pszModule) // get previous command. returns NULL if previous command does not exist. current command remains as it was.
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	COMMANDINFO *pPrevCmd = NULL;
	if (si->lpCurrentCommand != NULL) {
		if (si->lpCurrentCommand->next != NULL) // not NULL
			pPrevCmd = si->lpCurrentCommand->next; // next command (newest at beginning)
		else
			pPrevCmd = si->lpCurrentCommand;
	}
	else pPrevCmd = si->lpCommands;

	si->lpCurrentCommand = pPrevCmd; // make it the new command
	return (pPrevCmd) ? pPrevCmd->lpCommand : NULL;
}

static char* SM_GetNextCommand(const TCHAR *pszID, const char *pszModule) // get next command. returns NULL if next command does not exist. current command becomes NULL (a prev command after this one will get you the last command)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == NULL)
		return NULL;

	COMMANDINFO *pNextCmd = NULL;
	if (si->lpCurrentCommand != NULL)
		pNextCmd = si->lpCurrentCommand->last; // last command (newest at beginning)

	si->lpCurrentCommand = pNextCmd; // make it the new command
	return (pNextCmd) ? pNextCmd->lpCommand : NULL;
}

static int SM_GetCount(const char *pszModule)
{
	int count = 0;

	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next)
		if (!mir_strcmpi(pszModule, si->pszModule))
			count++;

	return count;
}

static SESSION_INFO* SM_FindSessionByIndex(const char *pszModule, int iItem)
{
	int count = 0;
	for (SESSION_INFO *si = ci.wndList; si != NULL; si = si->next) {
		if (!mir_strcmpi(pszModule, si->pszModule)) {
			if (iItem == count)
				return si;

			count++;
		}
	}
	return NULL;

}

static char* SM_GetUsers(SESSION_INFO *si)
{
	if (si == NULL)
		return NULL;

	USERINFO *utemp = NULL;
	for (SESSION_INFO *p = ci.wndList; p != NULL; p = p->next) {
		if (si == p) {
			if ((utemp = p->pUsers) == NULL)
				return NULL;

			break;
		}
	}

	char* p = NULL;
	size_t alloced = 0;
	do {
		size_t pLen = mir_strlen(p), nameLen = mir_tstrlen(utemp->pszUID);
		if (pLen + nameLen + 2 > alloced)
			p = (char*)mir_realloc(p, alloced += 4096);

		WideCharToMultiByte(CP_ACP, 0, utemp->pszUID, -1, p + pLen, (int)nameLen + 1, 0, 0);
		mir_strcpy(p + pLen + nameLen, " ");
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

static void MM_IconsChanged()
{
	LoadChatIcons();

	for (MODULEINFO *mi = m_ModList; mi != NULL; mi = mi->next) {
		Safe_DestroyIcon(mi->hOnlineIcon);
		Safe_DestroyIcon(mi->hOfflineIcon);
		Safe_DestroyIcon(mi->hOnlineTalkIcon);
		Safe_DestroyIcon(mi->hOfflineTalkIcon);

		if (ci.OnCreateModule) // recreate icons
			ci.OnCreateModule(mi);
	}
}

static void MM_FontsChanged()
{
	for (MODULEINFO *mi = m_ModList; mi != NULL; mi = mi->next)
		mi->pszHeader = ci.Log_CreateRtfHeader(mi);
}

static MODULEINFO* MM_FindModule(const char *pszModule)
{
	if (!pszModule)
		return NULL;

	for (MODULEINFO *mi = m_ModList; mi != NULL; mi = mi->next)
		if (mir_strcmpi(mi->pszModule, pszModule) == 0)
			return mi;

	return NULL;
}

// stupid thing..
static void MM_FixColors()
{
	for (MODULEINFO *mi = m_ModList; mi != NULL; mi = mi->next)
		CheckColorsInModule(mi->pszModule);
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

static STATUSINFO* TM_AddStatus(STATUSINFO **ppStatusList, const TCHAR *pszStatus, int *iCount)
{
	if (!ppStatusList || !pszStatus)
		return NULL;

	if (!ci.TM_FindStatus(*ppStatusList, pszStatus)) {
		STATUSINFO *node = (STATUSINFO*)mir_alloc(sizeof(STATUSINFO));
		memset(node, 0, sizeof(STATUSINFO));
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

static STATUSINFO* TM_FindStatus(STATUSINFO *pStatusList, const TCHAR *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return NULL;

	for (STATUSINFO *pTemp = pStatusList; pTemp != NULL; pTemp = pTemp->next)
		if (mir_tstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp;

	return 0;
}

static WORD TM_StringToWord(STATUSINFO *pStatusList, const TCHAR *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return 0;

	for (STATUSINFO *pTemp = pStatusList; pTemp != NULL; pTemp = pTemp->next) {
		if (mir_tstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp->Status;

		if (pTemp->next == NULL)
			return pStatusList->Status;
	}
	return 0;
}

static TCHAR* TM_WordToString(STATUSINFO *pStatusList, WORD Status)
{
	if (!pStatusList)
		return NULL;

	for (STATUSINFO *pTemp = pStatusList; pTemp != NULL; pTemp = pTemp->next) {
		if (pTemp->Status & Status) {
			Status -= pTemp->Status;
			if (Status == 0)
				return pTemp->pszGroup;
		}
	}
	return 0;
}

static BOOL TM_RemoveAll(STATUSINFO **ppStatusList)
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

static int UM_CompareItem(USERINFO *u1, const TCHAR *pszNick, WORD wStatus)
{
	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (int i = 0; i < 8; i++) {
		if ((dw1 & 1) && !(dw2 & 1))
			return -1;
		if ((dw2 & 1) && !(dw1 & 1))
			return 1;
		if ((dw1 & 1) && (dw2 & 1))
			return mir_tstrcmpi(u1->pszNick, pszNick);

		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	return mir_tstrcmpi(u1->pszNick, pszNick);
}

static USERINFO* UM_SortUser(USERINFO **ppUserList, const TCHAR *pszUID)
{
	USERINFO *ui = *ppUserList, *pLast = NULL;
	if (!ui || !pszUID)
		return NULL;

	while (ui && mir_tstrcmpi(ui->pszUID, pszUID)) {
		pLast = ui;
		ui = ui->next;
	}

	if (ui == NULL)
		return NULL;
	
	USERINFO *node = ui;
	if (pLast)
		pLast->next = ui->next;
	else
		*ppUserList = ui->next;
	ui = *ppUserList;

	pLast = NULL;

	while (ui && ci.UM_CompareItem(ui, node->pszNick, node->Status) <= 0) {
		pLast = ui;
		ui = ui->next;
	}

	if (*ppUserList == NULL) { // list is empty
		*ppUserList = node;
		node->next = NULL;
	}
	else {
		if (pLast) {
			node->next = ui;
			pLast->next = node;
		}
		else {
			node->next = *ppUserList;
			*ppUserList = node;
		}
	}

	return node;
}

USERINFO* UM_AddUser(STATUSINFO *pStatusList, USERINFO **ppUserList, const TCHAR *pszUID, const TCHAR *pszNick, WORD wStatus)
{
	if (pStatusList == NULL || ppUserList == NULL || pszNick == NULL)
		return NULL;

	USERINFO *ui = *ppUserList, *pLast = NULL;
	while (ui && ci.UM_CompareItem(ui, pszNick, wStatus) <= 0) {
		pLast = ui;
		ui = ui->next;
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
			node->next = ui;
			pLast->next = node;
		}
		else {
			node->next = *ppUserList;
			*ppUserList = node;
		}
	}

	return node;
}

static USERINFO* UM_FindUser(USERINFO *pUserList, const TCHAR *pszUID)
{
	if (!pUserList || !pszUID)
		return NULL;

	for (USERINFO *ui = pUserList; ui != NULL; ui = ui->next)
		if (!mir_tstrcmpi(ui->pszUID, pszUID))
			return ui;

	return NULL;
}

static USERINFO* UM_FindUserFromIndex(USERINFO *pUserList, int index)
{
	if (!pUserList)
		return NULL;

	int i = 0;
	for (USERINFO *ui = pUserList; ui != NULL; ui = ui->next) {
		if (i == index)
			return ui;
		i++;
	}
	return NULL;
}

static USERINFO* UM_GiveStatus(USERINFO *pUserList, const TCHAR *pszUID, WORD status)
{
	USERINFO *ui = UM_FindUser(pUserList, pszUID);
	if (ui == NULL)
		return NULL;

	ui->Status |= status;
	return ui;
}

static USERINFO* UM_SetContactStatus(USERINFO *pUserList, const TCHAR *pszUID, WORD status)
{
	USERINFO *ui = UM_FindUser(pUserList, pszUID);
	if (ui == NULL)
		return NULL;
	
	ui->ContactStatus = status;
	return ui;
}

static BOOL UM_SetStatusEx(USERINFO *pUserList, const TCHAR* pszText, int flags)
{
	int bOnlyMe = (flags & GC_SSE_ONLYLISTED) != 0, bSetStatus = (flags & GC_SSE_ONLINE) != 0;
	char cDelimiter = (flags & GC_SSE_TABDELIMITED) ? '\t' : ' ';

	for (USERINFO *ui = pUserList; ui != NULL; ui = ui->next) {
		if (!bOnlyMe)
			ui->iStatusEx = 0;

		if (pszText != NULL) {
			TCHAR *s = (TCHAR *)_tcsstr(pszText, ui->pszUID);
			if (s) {
				ui->iStatusEx = 0;
				if (s == pszText || s[-1] == cDelimiter) {
					size_t len = mir_tstrlen(ui->pszUID);
					if (s[len] == cDelimiter || s[len] == '\0')
						ui->iStatusEx = (!bOnlyMe || bSetStatus) ? 1 : 0;
				}
			}
		}
	}
	return TRUE;
}

static USERINFO* UM_TakeStatus(USERINFO *pUserList, const TCHAR *pszUID, WORD status)
{
	USERINFO *ui = UM_FindUser(pUserList, pszUID);
	if (ui == NULL)
		return NULL;

	ui->Status &= ~status;
	return ui;
}

static TCHAR* UM_FindUserAutoComplete(USERINFO *pUserList, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	if (!pUserList || !pszOriginal || !pszCurrent)
		return NULL;

	TCHAR *pszName = NULL;
	for (USERINFO *ui = pUserList; ui != NULL; ui = ui->next)
		if (ui->pszNick && my_strstri(ui->pszNick, pszOriginal) == ui->pszNick)
			if (mir_tstrcmpi(ui->pszNick, pszCurrent) > 0 && (!pszName || mir_tstrcmpi(ui->pszNick, pszName) < 0))
				pszName = ui->pszNick;

	return pszName;
}

static BOOL UM_RemoveUser(USERINFO **ppUserList, const TCHAR *pszUID)
{
	if (!ppUserList || !pszUID)
		return FALSE;

	USERINFO *ui = *ppUserList, *pLast = NULL;
	while (ui != NULL) {
		if (!mir_tstrcmpi(ui->pszUID, pszUID)) {
			if (pLast == NULL)
				*ppUserList = ui->next;
			else
				pLast->next = ui->next;
			mir_free(ui->pszNick);
			mir_free(ui->pszUID);
			mir_free(ui);
			return TRUE;
		}
		pLast = ui;
		ui = ui->next;
	}
	return FALSE;
}

static BOOL UM_RemoveAll(USERINFO **ppUserList)
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

static LOGINFO* LM_AddEvent(LOGINFO **ppLogListStart, LOGINFO** ppLogListEnd)
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

static BOOL LM_TrimLog(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd, int iCount)
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

static BOOL LM_RemoveAll(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd)
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
	if (lParam == NULL)
		return (INT_PTR)&ci;

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
	ci.DoRtfToTags = DoRtfToTags;

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
	ci.GetColorIndex = GetColorIndex;
	ci.Log_SetStyle = Log_SetStyle;

	ci.IsHighlighted = IsHighlighted;
	ci.RemoveFormatting = RemoveFormatting;
	ci.ReloadSettings = LoadGlobalSettings;
	ci.ColorChooser = ColorChooser;

	ci.pLogIconBmpBits = pLogIconBmpBits;
	ci.logIconBmpSize = logIconBmpSize;

	RegisterFonts();
	OptionsInit();
	return (INT_PTR)&ci;
}
