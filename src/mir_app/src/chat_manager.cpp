/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

#include "stdafx.h"

#include "chat.h"

#define WINDOWS_COMMANDS_MAX 30

static int compareSessions(const SESSION_INFO *p1, const SESSION_INFO *p2)
{
	int res = mir_strcmpi(p1->pszModule, p2->pszModule);
	return (res != 0) ? res : mir_wstrcmpi(p1->ptszID, p2->ptszID);
}

LIST<SESSION_INFO> g_arSessions(10, compareSessions);

/////////////////////////////////////////////////////////////////////////////////////////

static int compareModules(const MODULEINFO *p1, const MODULEINFO *p2)
{
	return mir_strcmp(p1->pszModule, p2->pszModule);
}

LIST<MODULEINFO> g_arModules(5, compareModules);

/////////////////////////////////////////////////////////////////////////////////////////

CHAT_MANAGER::CHAT_MANAGER() :
	arSessions(g_arSessions)
{}

CHAT_MANAGER chatApi;

/////////////////////////////////////////////////////////////////////////////////////////

static void SetActiveSession(SESSION_INFO *si)
{
	if (si) {
		replaceStrW(chatApi.szActiveWndID, si->ptszID);
		replaceStr(chatApi.szActiveWndModule, si->pszModule);
	}
}

static SESSION_INFO* GetActiveSession(void)
{
	SESSION_INFO *si = SM_FindSession(chatApi.szActiveWndID, chatApi.szActiveWndModule);
	if (si)
		return si;

	return g_arSessions[0];
}

/////////////////////////////////////////////////////////////////////////////////////////
//	Session Manager functions
//	Keeps track of all sessions and its windows

static void SM_FreeSession(SESSION_INFO *si, bool bRemoveContact = false)
{
	if (cli.pfnGetEvent(si->hContact, 0))
		cli.pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);
	si->wState &= ~STATE_TALK;
	db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);

	if (si->pDlg)
		si->pDlg->CloseTab();

	Chat_DoEventHook(si, GC_SESSION_TERMINATE, nullptr, nullptr, (INT_PTR)si->pItemData);

	if (si->hContact && bRemoveContact)
		db_delete_contact(si->hContact);

	// contact may have been deleted here already, since function may be called after deleting
	// contact so the handle may be invalid, therefore db_get_b shall return 0
	if (si->hContact && db_get_b(si->hContact, si->pszModule, "ChatRoom", 0) != 0) {
		chatApi.SetOffline(si->hContact, (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS) ? TRUE : FALSE);
		db_set_s(si->hContact, si->pszModule, "Topic", "");
		db_set_s(si->hContact, si->pszModule, "StatusBar", "");
		db_unset(si->hContact, "CList", "StatusMsg");
	}

	chatApi.UM_RemoveAll(&si->pUsers);
	chatApi.TM_RemoveAll(&si->pStatuses);
	chatApi.LM_RemoveAll(&si->pLog, &si->pLogEnd);

	si->iStatusCount = 0;
	si->nUsersInNicklist = 0;

	mir_free(si->pszModule);
	mir_free(si->ptszID);
	mir_free(si->ptszName);
	mir_free(si->ptszStatusbarText);
	mir_free(si->ptszTopic);

	while (si->lpCommands != nullptr) {
		COMMANDINFO *pNext = si->lpCommands->next;
		mir_free(si->lpCommands->lpCommand);
		mir_free(si->lpCommands);
		si->lpCommands = pNext;
	}

	mir_free(si);
}

int SM_RemoveSession(const wchar_t *pszID, const char *pszModule, bool removeContact)
{
	if (pszModule == nullptr)
		return FALSE;

	if (pszID != nullptr) {
		SESSION_INFO *si = SM_FindSession(pszID, pszModule);
		if (si == nullptr)
			return FALSE;

		g_arSessions.remove(si);
		SM_FreeSession(si, removeContact);
		return TRUE;
	}

	for (int i = g_arSessions.getCount() - 1; i >= 0; i--) {
		SESSION_INFO *si = g_arSessions[i];
		if (si->iType != GCW_SERVER && !mir_strcmpi(si->pszModule, pszModule)) {
			SM_FreeSession(si, removeContact);
			g_arSessions.remove(i);
		}
	}
	return TRUE;
}

SESSION_INFO* SM_FindSession(const wchar_t *pszID, const char *pszModule)
{
	if (!pszID || !pszModule)
		return nullptr;

	SESSION_INFO tmp;
	tmp.pszModule = (char*)pszModule;
	tmp.ptszID = (wchar_t*)pszID;

	mir_cslock lck(csChat);
	return g_arSessions.find(&tmp);
}

BOOL SM_SetOffline(const char *pszModule, SESSION_INFO *si)
{
	if (si == nullptr) {
		if (pszModule == nullptr)
			return FALSE;

		for (int i = 0; i < g_arSessions.getCount(); i++) {
			si = g_arSessions[i];
			if (!_strcmpi(si->pszModule, pszModule))
				SM_SetOffline(pszModule, si);
		}
		return TRUE;
	}

	chatApi.UM_RemoveAll(&si->pUsers);
	si->pMe = nullptr;
	si->nUsersInNicklist = 0;
	if (si->iType != GCW_SERVER)
		si->bInitDone = false;
	if (chatApi.OnOfflineSession)
		chatApi.OnOfflineSession(si);
	return TRUE;
}

static HICON SM_GetStatusIcon(SESSION_INFO *si, USERINFO *ui)
{
	if (!ui || !si)
		return nullptr;

	STATUSINFO *ti = chatApi.TM_FindStatus(si->pStatuses, chatApi.TM_WordToString(si->pStatuses, ui->Status));
	if (ti != nullptr) {
		if ((UINT_PTR)ti->hIcon >= STATUSICONCOUNT)
			return ti->hIcon;

		return chatApi.hIcons[ICON_STATUS0 + (INT_PTR)ti->hIcon];
	}
	return chatApi.hIcons[ICON_STATUS0];
}

BOOL SM_AddEvent(const wchar_t *pszID, const char *pszModule, GCEVENT *gce, bool bIsHighlighted)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return TRUE;

	LOGINFO *li = chatApi.LM_AddEvent(&si->pLog, &si->pLogEnd);
	si->iEventCount++;

	li->iType = gce->iType;
	li->ptszNick = mir_wstrdup(gce->ptszNick);
	li->ptszText = mir_wstrdup(gce->ptszText);
	li->ptszStatus = mir_wstrdup(gce->ptszStatus);
	li->ptszUserInfo = mir_wstrdup(gce->ptszUserInfo);

	li->bIsMe = gce->bIsMe;
	li->time = gce->time;
	li->bIsHighlighted = bIsHighlighted;

	if (g_Settings->iEventLimit > 0 && si->iEventCount > g_Settings->iEventLimit + 20) {
		chatApi.LM_TrimLog(&si->pLog, &si->pLogEnd, si->iEventCount - g_Settings->iEventLimit);
		si->bTrimmed = true;
		si->iEventCount = g_Settings->iEventLimit;
		return FALSE;
	}
	return TRUE;
}

BOOL SM_MoveUser(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID)
{
	if (!pszUID)
		return FALSE;

	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return FALSE;

	chatApi.UM_SortUser(&si->pUsers, pszUID);
	return TRUE;
}

BOOL SM_RemoveUser(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID)
{
	if (!pszModule || !pszUID)
		return FALSE;

	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if ((pszID && mir_wstrcmpi(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		USERINFO *ui = chatApi.UM_FindUser(si->pUsers, pszUID);
		if (ui) {
			si->nUsersInNicklist--;
			if (chatApi.OnRemoveUser)
				chatApi.OnRemoveUser(si, ui);

			if (si->pMe == ui)
				si->pMe = nullptr;
			chatApi.UM_RemoveUser(&si->pUsers, pszUID);

			if (si->pDlg)
				si->pDlg->UpdateNickList();

			if (pszID)
				return TRUE;
		}
	}

	return FALSE;
}

static USERINFO* SM_GetUserFromIndex(const wchar_t *pszID, const char *pszModule, int index)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	return (si == nullptr) ? nullptr : chatApi.UM_FindUserFromIndex(si->pUsers, index);
}

BOOL SM_GiveStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return FALSE;
	
	USERINFO *ui = chatApi.UM_GiveStatus(si->pUsers, pszUID, chatApi.TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->pDlg)
			si->pDlg->UpdateNickList();
	}
	return TRUE;
}

BOOL SM_SetContactStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, WORD wStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return FALSE;

	USERINFO *ui = chatApi.UM_SetContactStatus(si->pUsers, pszUID, wStatus);
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->pDlg)
			si->pDlg->UpdateNickList();
	}
	return TRUE;
}

BOOL SM_TakeStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return FALSE;

	USERINFO *ui = chatApi.UM_TakeStatus(si->pUsers, pszUID, chatApi.TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
		if (si->pDlg)
			si->pDlg->UpdateNickList();
	}
	return TRUE;
}

static BOOL SM_BroadcastMessage(const char *pszModule, UINT msg, WPARAM wParam, LPARAM lParam, BOOL bAsync)
{
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if (pszModule && _strcmpi(si->pszModule, pszModule))
			continue;

		if (si->pDlg) {
			if (bAsync)
				PostMessage(si->pDlg->GetHwnd(), msg, wParam, lParam);
			else
				SendMessage(si->pDlg->GetHwnd(), msg, wParam, lParam);
		}
	}
	return TRUE;
}

BOOL SM_SetStatus(const char *pszModule, SESSION_INFO *si, int wStatus)
{
	if (si == nullptr) {
		if (pszModule == nullptr)
			return FALSE;

		for (int i = 0; i < g_arSessions.getCount(); i++) {
			si = g_arSessions[i];
			if (!_strcmpi(si->pszModule, pszModule))
				SM_SetStatus(pszModule, si, wStatus);
		}
		return TRUE;
	}

	si->wStatus = wStatus;
	if (si->hContact) {
		if (si->iType != GCW_SERVER && wStatus != ID_STATUS_OFFLINE)
			db_unset(si->hContact, "CList", "Hidden");

		db_set_w(si->hContact, si->pszModule, "Status", (WORD)wStatus);
	}

	if (chatApi.OnSetStatus)
		chatApi.OnSetStatus(si, wStatus);

	return TRUE;
}

BOOL SM_ChangeNick(const wchar_t *pszID, const char *pszModule, GCEVENT *gce)
{
	if (!pszModule)
		return FALSE;

	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if ((!pszID || !mir_wstrcmpi(si->ptszID, pszID)) && !mir_strcmpi(si->pszModule, pszModule)) {
			USERINFO *ui = chatApi.UM_FindUser(si->pUsers, gce->ptszUID);
			if (ui) {
				replaceStrW(ui->pszNick, gce->ptszText);
				SM_MoveUser(si->ptszID, si->pszModule, ui->pszUID);
				if (si->pDlg)
					si->pDlg->UpdateNickList();
				if (chatApi.OnChangeNick)
					chatApi.OnChangeNick(si);
			}

			if (pszID)
				return TRUE;
		}
	}
	return TRUE;
}

void SM_RemoveAll(void)
{
	while (g_arSessions.getCount()) {
		SM_FreeSession(g_arSessions[0], false);
		g_arSessions.remove(0);
	}
}

static void SM_AddCommand(const wchar_t *pszID, const char *pszModule, const char* lpNewCommand)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return;

	COMMANDINFO *node = (COMMANDINFO *)mir_alloc(sizeof(COMMANDINFO));
	node->lpCommand = mir_strdup(lpNewCommand);
	node->last = nullptr; // always added at beginning!

	// new commands are added at start
	if (si->lpCommands == nullptr) {
		node->next = nullptr;
		si->lpCommands = node;
	}
	else {
		node->next = si->lpCommands;
		si->lpCommands->last = node; // hmm, weird
		si->lpCommands = node;
	}
	si->lpCurrentCommand = nullptr; // current command
	si->wCommandsNum++;

	if (si->wCommandsNum > WINDOWS_COMMANDS_MAX) {
		COMMANDINFO *pCurComm = si->lpCommands;
		while (pCurComm->next != nullptr)
			pCurComm = pCurComm->next;
		
		COMMANDINFO *pLast = pCurComm->last;
		mir_free(pCurComm->lpCommand);
		mir_free(pCurComm);
		pLast->next = nullptr;
		// done
		si->wCommandsNum--;
	}
}

static char* SM_GetPrevCommand(const wchar_t *pszID, const char *pszModule) // get previous command. returns nullptr if previous command does not exist. current command remains as it was.
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return nullptr;

	COMMANDINFO *pPrevCmd = nullptr;
	if (si->lpCurrentCommand != nullptr) {
		if (si->lpCurrentCommand->next != nullptr) // not nullptr
			pPrevCmd = si->lpCurrentCommand->next; // next command (newest at beginning)
		else
			pPrevCmd = si->lpCurrentCommand;
	}
	else pPrevCmd = si->lpCommands;

	si->lpCurrentCommand = pPrevCmd; // make it the new command
	return (pPrevCmd) ? pPrevCmd->lpCommand : nullptr;
}

static char* SM_GetNextCommand(const wchar_t *pszID, const char *pszModule) // get next command. returns nullptr if next command does not exist. current command becomes nullptr (a prev command after this one will get you the last command)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return nullptr;

	COMMANDINFO *pNextCmd = nullptr;
	if (si->lpCurrentCommand != nullptr)
		pNextCmd = si->lpCurrentCommand->last; // last command (newest at beginning)

	si->lpCurrentCommand = pNextCmd; // make it the new command
	return (pNextCmd) ? pNextCmd->lpCommand : nullptr;
}

static int SM_GetCount(const char *pszModule)
{
	int count = 0;

	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if (!mir_strcmpi(pszModule, si->pszModule))
			count++;
	}

	return count;
}

static SESSION_INFO* SM_FindSessionByIndex(const char *pszModule, int iItem)
{
	int count = 0;
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if (!mir_strcmpi(pszModule, si->pszModule)) {
			if (iItem == count)
				return si;

			count++;
		}
	}
	return nullptr;

}

char* SM_GetUsers(SESSION_INFO *si)
{
	if (si == nullptr)
		return nullptr;

	USERINFO *utemp = nullptr;
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *p = g_arSessions[i];
		if (si == p) {
			if ((utemp = p->pUsers) == nullptr)
				return nullptr;

			break;
		}
	}
	if (utemp == nullptr)
		return nullptr;

	char *p = nullptr;
	size_t alloced = 0;
	do {
		size_t pLen = mir_strlen(p), nameLen = mir_wstrlen(utemp->pszUID);
		if (pLen + nameLen + 2 > alloced)
			p = (char*)mir_realloc(p, alloced += 4096);

		WideCharToMultiByte(CP_ACP, 0, utemp->pszUID, -1, p + pLen, (int)nameLen + 1, 0, 0);
		mir_strcpy(p + pLen + nameLen, " ");
		utemp = utemp->next;
	}
		while (utemp != nullptr);
	return p;
}

static void SM_InvalidateLogDirectories()
{
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		si->pszLogFileName[0] = si->pszLogFileName[1] = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//	Module Manager functions
//	Necessary to keep track of all modules that has registered with the plugin

static MODULEINFO* MM_AddModule(const char *pszModule)
{
	if (pszModule == nullptr)
		return nullptr;

	if (chatApi.MM_FindModule(pszModule))
		return nullptr;

	MODULEINFO *node = (MODULEINFO*)mir_calloc(g_cbModuleInfo);
	replaceStr(node->pszModule, pszModule);
	if (chatApi.OnCreateModule)
		chatApi.OnCreateModule(node);

	g_arModules.insert(node);
	return node;
}

static void MM_IconsChanged()
{
	LoadChatIcons();

	for (int i = 0; i < g_arModules.getCount(); i++) {
		MODULEINFO *mi = g_arModules[i];
		if (chatApi.OnCreateModule) // recreate icons
			chatApi.OnCreateModule(mi);
	}
}

static void MM_FontsChanged()
{
	for (int i = 0; i < g_arModules.getCount(); i++) {
		MODULEINFO *mi = g_arModules[i];
		mi->pszHeader = chatApi.Log_CreateRtfHeader();
	}
}

static MODULEINFO* MM_FindModule(const char *pszModule)
{
	if (!pszModule)
		return nullptr;

	return g_arModules.find((MODULEINFO*)&pszModule);
}

static BOOL MM_RemoveAll(void)
{
	for (int i = 0; i < g_arModules.getCount(); i++) {
		MODULEINFO *mi = g_arModules[i];
		if (chatApi.OnDestroyModule)
			chatApi.OnDestroyModule(mi);

		mir_free(mi->pszModule);
		mir_free(mi->ptszModDispName);
		mir_free(mi->pszHeader);
		mir_free(mi);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Status manager functions
// Necessary to keep track of what user statuses per window nicklist that is available

static STATUSINFO* TM_AddStatus(STATUSINFO **ppStatusList, const wchar_t *pszStatus, int *iCount)
{
	if (!ppStatusList || !pszStatus)
		return nullptr;

	if (!chatApi.TM_FindStatus(*ppStatusList, pszStatus)) {
		STATUSINFO *node = (STATUSINFO*)mir_alloc(sizeof(STATUSINFO));
		memset(node, 0, sizeof(STATUSINFO));
		replaceStrW(node->pszGroup, pszStatus);
		node->hIcon = (HICON)(*iCount);
		while ((INT_PTR)node->hIcon > STATUSICONCOUNT - 1)
			node->hIcon--;

		if (*ppStatusList == nullptr) { // list is empty
			node->Status = 1;
			*ppStatusList = node;
			node->next = nullptr;
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

static STATUSINFO* TM_FindStatus(STATUSINFO *pStatusList, const wchar_t *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return nullptr;

	for (STATUSINFO *pTemp = pStatusList; pTemp != nullptr; pTemp = pTemp->next)
		if (mir_wstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp;

	return 0;
}

static WORD TM_StringToWord(STATUSINFO *pStatusList, const wchar_t *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return 0;

	for (STATUSINFO *pTemp = pStatusList; pTemp != nullptr; pTemp = pTemp->next) {
		if (mir_wstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp->Status;

		if (pTemp->next == nullptr)
			return pStatusList->Status;
	}
	return 0;
}

static wchar_t* TM_WordToString(STATUSINFO *pStatusList, WORD Status)
{
	if (!pStatusList)
		return nullptr;

	for (STATUSINFO *pTemp = pStatusList; pTemp != nullptr; pTemp = pTemp->next) {
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

	while (*ppStatusList != nullptr) {
		STATUSINFO *pLast = ppStatusList[0]->next;
		mir_free(ppStatusList[0]->pszGroup);
		if ((INT_PTR)ppStatusList[0]->hIcon > 10)
			DestroyIcon(ppStatusList[0]->hIcon);
		mir_free(*ppStatusList);
		*ppStatusList = pLast;
	}
	*ppStatusList = nullptr;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// User manager functions
// Necessary to keep track of the users in a window nicklist

static int UM_CompareItem(USERINFO *u1, const wchar_t *pszNick, WORD wStatus)
{
	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (int i = 0; i < 8; i++) {
		if ((dw1 & 1) && !(dw2 & 1))
			return -1;
		if ((dw2 & 1) && !(dw1 & 1))
			return 1;
		if ((dw1 & 1) && (dw2 & 1))
			return mir_wstrcmpi(u1->pszNick, pszNick);

		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	return mir_wstrcmpi(u1->pszNick, pszNick);
}

static USERINFO* UM_SortUser(USERINFO **ppUserList, const wchar_t *pszUID)
{
	USERINFO *ui = *ppUserList, *pLast = nullptr;
	if (!ui || !pszUID)
		return nullptr;

	while (ui && mir_wstrcmpi(ui->pszUID, pszUID)) {
		pLast = ui;
		ui = ui->next;
	}

	if (ui == nullptr)
		return nullptr;
	
	USERINFO *node = ui;
	if (pLast)
		pLast->next = ui->next;
	else
		*ppUserList = ui->next;
	ui = *ppUserList;

	pLast = nullptr;

	while (ui && chatApi.UM_CompareItem(ui, node->pszNick, node->Status) <= 0) {
		pLast = ui;
		ui = ui->next;
	}

	if (*ppUserList == nullptr) { // list is empty
		*ppUserList = node;
		node->next = nullptr;
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

USERINFO* UM_AddUser(STATUSINFO *pStatusList, USERINFO **ppUserList, const wchar_t *pszUID, const wchar_t *pszNick, WORD wStatus)
{
	if (pStatusList == nullptr || ppUserList == nullptr || pszNick == nullptr)
		return nullptr;

	USERINFO *ui = *ppUserList, *pLast = nullptr;
	while (ui && chatApi.UM_CompareItem(ui, pszNick, wStatus) <= 0) {
		pLast = ui;
		ui = ui->next;
	}

	//	if (!UM_FindUser(*ppUserList, pszUI, wStatus)
	USERINFO *node = (USERINFO*)mir_calloc(sizeof(USERINFO));
	replaceStrW(node->pszUID, pszUID);

	if (*ppUserList == nullptr) { // list is empty
		*ppUserList = node;
		node->next = nullptr;
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

static USERINFO* UM_FindUser(USERINFO *pUserList, const wchar_t *pszUID)
{
	if (!pUserList || !pszUID)
		return nullptr;

	for (USERINFO *ui = pUserList; ui != nullptr; ui = ui->next)
		if (!mir_wstrcmpi(ui->pszUID, pszUID))
			return ui;

	return nullptr;
}

static USERINFO* UM_FindUserFromIndex(USERINFO *pUserList, int index)
{
	if (!pUserList)
		return nullptr;

	int i = 0;
	for (USERINFO *ui = pUserList; ui != nullptr; ui = ui->next) {
		if (i == index)
			return ui;
		i++;
	}
	return nullptr;
}

static USERINFO* UM_GiveStatus(USERINFO *pUserList, const wchar_t *pszUID, WORD status)
{
	USERINFO *ui = UM_FindUser(pUserList, pszUID);
	if (ui == nullptr)
		return nullptr;

	ui->Status |= status;
	return ui;
}

static USERINFO* UM_SetContactStatus(USERINFO *pUserList, const wchar_t *pszUID, WORD status)
{
	USERINFO *ui = UM_FindUser(pUserList, pszUID);
	if (ui == nullptr)
		return nullptr;
	
	ui->ContactStatus = status;
	return ui;
}

static BOOL UM_SetStatusEx(USERINFO *pUserList, const wchar_t* pszText, int flags)
{
	int bOnlyMe = (flags & GC_SSE_ONLYLISTED) != 0, bSetStatus = (flags & GC_SSE_ONLINE) != 0;
	char cDelimiter = (flags & GC_SSE_TABDELIMITED) ? '\t' : ' ';

	for (USERINFO *ui = pUserList; ui != nullptr; ui = ui->next) {
		if (!bOnlyMe)
			ui->iStatusEx = 0;

		if (pszText != nullptr) {
			wchar_t *s = (wchar_t *)wcsstr(pszText, ui->pszUID);
			if (s) {
				ui->iStatusEx = 0;
				if (s == pszText || s[-1] == cDelimiter) {
					size_t len = mir_wstrlen(ui->pszUID);
					if (s[len] == cDelimiter || s[len] == '\0')
						ui->iStatusEx = (!bOnlyMe || bSetStatus) ? 1 : 0;
				}
			}
		}
	}
	return TRUE;
}

static USERINFO* UM_TakeStatus(USERINFO *pUserList, const wchar_t *pszUID, WORD status)
{
	USERINFO *ui = UM_FindUser(pUserList, pszUID);
	if (ui == nullptr)
		return nullptr;

	ui->Status &= ~status;
	return ui;
}

static wchar_t* UM_FindUserAutoComplete(USERINFO *pUserList, const wchar_t* pszOriginal, const wchar_t* pszCurrent)
{
	if (!pUserList || !pszOriginal || !pszCurrent)
		return nullptr;

	wchar_t *pszName = nullptr;
	for (USERINFO *ui = pUserList; ui != nullptr; ui = ui->next)
		if (ui->pszNick && my_strstri(ui->pszNick, pszOriginal) == ui->pszNick)
			if (mir_wstrcmpi(ui->pszNick, pszCurrent) > 0 && (!pszName || mir_wstrcmpi(ui->pszNick, pszName) < 0))
				pszName = ui->pszNick;

	return pszName;
}

static BOOL UM_RemoveUser(USERINFO **ppUserList, const wchar_t *pszUID)
{
	if (!ppUserList || !pszUID)
		return FALSE;

	USERINFO *ui = *ppUserList, *pLast = nullptr;
	while (ui != nullptr) {
		if (!mir_wstrcmpi(ui->pszUID, pszUID)) {
			if (pLast == nullptr)
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

	while (*ppUserList != nullptr) {
		USERINFO *pLast = ppUserList[0]->next;
		mir_free(ppUserList[0]->pszUID);
		mir_free(ppUserList[0]->pszNick);
		mir_free(*ppUserList);
		*ppUserList = pLast;
	}
	*ppUserList = nullptr;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Log manager functions
//	Necessary to keep track of events in a window log

static LOGINFO* LM_AddEvent(LOGINFO **ppLogListStart, LOGINFO** ppLogListEnd)
{
	if (!ppLogListStart || !ppLogListEnd)
		return nullptr;

	LOGINFO *node = (LOGINFO*)mir_calloc(sizeof(LOGINFO));
	if (*ppLogListStart == nullptr) { // list is empty
		*ppLogListStart = node;
		*ppLogListEnd = node;
		node->next = nullptr;
		node->prev = nullptr;
	}
	else {
		ppLogListStart[0]->prev = node;
		node->next = *ppLogListStart;
		*ppLogListStart = node;
		ppLogListStart[0]->prev = nullptr;
	}

	return node;
}

static BOOL LM_TrimLog(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd, int iCount)
{
	LOGINFO *pTemp = *ppLogListEnd;
	while (pTemp != nullptr && iCount > 0) {
		*ppLogListEnd = pTemp->prev;
		if (*ppLogListEnd == nullptr)
			*ppLogListStart = nullptr;

		mir_free(pTemp->ptszNick);
		mir_free(pTemp->ptszUserInfo);
		mir_free(pTemp->ptszText);
		mir_free(pTemp->ptszStatus);
		mir_free(pTemp);
		pTemp = *ppLogListEnd;
		iCount--;
	}
	ppLogListEnd[0]->next = nullptr;

	return TRUE;
}

static BOOL LM_RemoveAll(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd)
{
	while (*ppLogListStart != nullptr) {
		LOGINFO *pLast = ppLogListStart[0]->next;
		mir_free(ppLogListStart[0]->ptszText);
		mir_free(ppLogListStart[0]->ptszNick);
		mir_free(ppLogListStart[0]->ptszStatus);
		mir_free(ppLogListStart[0]->ptszUserInfo);
		mir_free(*ppLogListStart);
		*ppLogListStart = pLast;
	}
	*ppLogListStart = nullptr;
	*ppLogListEnd = nullptr;
	return TRUE;
}

MIR_APP_DLL(CHAT_MANAGER*) Chat_GetInterface(CHAT_MANAGER_INITDATA *pInit, int _hLangpack)
{
	if (pInit == nullptr)
		return &chatApi;

	// wipe out old junk
	memset(PBYTE(&chatApi) + offsetof(CHAT_MANAGER, OnCreateModule), 0, sizeof(CHAT_MANAGER) - offsetof(CHAT_MANAGER, OnCreateModule));

	if (g_cbSession) { // reallocate old sessions
		mir_cslock lck(csChat);

		for (int i = 0; i < g_arSessions.getCount(); i++) {
			SESSION_INFO *p = g_arSessions[i];
			SESSION_INFO *p1 = (SESSION_INFO*)mir_realloc(p, pInit->cbSession);
			memset(PBYTE(p1) + sizeof(GCSessionInfoBase), 0, pInit->cbSession - sizeof(GCSessionInfoBase));
			if (p1 != p) { // realloc could change a pointer, reinsert a structure
				g_arSessions.remove(i);
				g_arSessions.insert(p1);
			}
		}
	}
	if (g_cbModuleInfo) { // reallocate old modules
		mir_cslock lck(csChat);

		for (int i = 0; i < g_arModules.getCount(); i++) {
			MODULEINFO *mi = g_arModules[i];
			MODULEINFO *p1 = (MODULEINFO*)mir_realloc(mi, pInit->cbModuleInfo);
			memset(PBYTE(p1) + sizeof(GCModuleInfoBase), 0, pInit->cbModuleInfo - sizeof(GCModuleInfoBase));
			if (p1 != mi) { // realloc could change a pointer, reinsert a structure
				g_arModules.remove(i);
				g_arModules.insert(p1, i);
			}
		}
	}

	g_Settings = pInit->pSettings;
	g_szFontGroup = pInit->szFontGroup;
	g_cbSession = pInit->cbSession;
	g_cbModuleInfo = pInit->cbModuleInfo;
	g_iFontMode = pInit->iFontMode;
	g_iChatLang = _hLangpack;

	chatApi.SetActiveSession = SetActiveSession;
	chatApi.GetActiveSession = GetActiveSession;
	chatApi.SM_FindSession = SM_FindSession;
	chatApi.SM_GetStatusIcon = SM_GetStatusIcon;
	chatApi.SM_BroadcastMessage = SM_BroadcastMessage;
	chatApi.SM_AddCommand = SM_AddCommand;
	chatApi.SM_GetPrevCommand = SM_GetPrevCommand;
	chatApi.SM_GetNextCommand = SM_GetNextCommand;
	chatApi.SM_GetCount = SM_GetCount;
	chatApi.SM_FindSessionByIndex = SM_FindSessionByIndex;
	chatApi.SM_GetUserFromIndex = SM_GetUserFromIndex;
	chatApi.SM_InvalidateLogDirectories = SM_InvalidateLogDirectories;

	chatApi.MM_AddModule = MM_AddModule;
	chatApi.MM_FindModule = MM_FindModule;
	chatApi.MM_FontsChanged = MM_FontsChanged;
	chatApi.MM_IconsChanged = MM_IconsChanged;
	chatApi.MM_RemoveAll = MM_RemoveAll;

	chatApi.TM_AddStatus = TM_AddStatus;
	chatApi.TM_FindStatus = TM_FindStatus;
	chatApi.TM_StringToWord = TM_StringToWord;
	chatApi.TM_WordToString = TM_WordToString;
	chatApi.TM_RemoveAll = TM_RemoveAll;

	chatApi.UM_SetStatusEx = UM_SetStatusEx;
	chatApi.UM_AddUser = UM_AddUser;
	chatApi.UM_SortUser = UM_SortUser;
	chatApi.UM_FindUser = UM_FindUser;
	chatApi.UM_FindUserFromIndex = UM_FindUserFromIndex;
	chatApi.UM_GiveStatus = UM_GiveStatus;
	chatApi.UM_SetContactStatus = UM_SetContactStatus;
	chatApi.UM_TakeStatus = UM_TakeStatus;
	chatApi.UM_FindUserAutoComplete = UM_FindUserAutoComplete;
	chatApi.UM_RemoveUser = UM_RemoveUser;
	chatApi.UM_RemoveAll = UM_RemoveAll;
	chatApi.UM_CompareItem = UM_CompareItem;

	chatApi.LM_AddEvent = LM_AddEvent;
	chatApi.LM_TrimLog = LM_TrimLog;
	chatApi.LM_RemoveAll = LM_RemoveAll;

	chatApi.SetOffline = SetOffline;
	chatApi.SetAllOffline = SetAllOffline;
	chatApi.AddEvent = AddEvent;
	chatApi.FindRoom = FindRoom;
	chatApi.DoRtfToTags = DoRtfToTags;

	chatApi.Log_CreateRTF = Log_CreateRTF;
	chatApi.Log_CreateRtfHeader = Log_CreateRtfHeader;
	chatApi.LoadMsgDlgFont = LoadMsgDlgFont;
	chatApi.MakeTimeStamp = MakeTimeStamp;

	chatApi.DoSoundsFlashPopupTrayStuff = DoSoundsFlashPopupTrayStuff;
	chatApi.DoTrayIcon = DoTrayIcon;
	chatApi.DoPopup = DoPopup;
	chatApi.ShowPopup = ShowPopup;
	chatApi.LogToFile = LogToFile;
	chatApi.GetChatLogsFilename = GetChatLogsFilename;
	chatApi.Log_SetStyle = Log_SetStyle;

	chatApi.IsHighlighted = IsHighlighted;
	chatApi.RemoveFormatting = RemoveFormatting;
	chatApi.ReloadSettings = LoadGlobalSettings;

	chatApi.pLogIconBmpBits = pLogIconBmpBits;

	RegisterFonts();
	OptionsInit();
	return &chatApi;
}
