/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

static USERINFO* UM_FindUser(SESSION_INFO *si, const wchar_t *pszUID);

static int CompareKeys(const USERINFO *u1, const USERINFO *u2)
{
	return mir_wstrcmp(u1->pszUID, u2->pszUID);
}

static int CompareUser(const USERINFO *u1, const USERINFO *u2)
{
	return g_chatApi.UM_CompareItem(u1, u2);
}

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

static LIST<MODULEINFO> g_arModules(5, compareModules);

/////////////////////////////////////////////////////////////////////////////////////////

static void SetActiveSession(SESSION_INFO *si)
{
	if (si) {
		replaceStrW(g_chatApi.szActiveWndID, si->ptszID);
		replaceStr(g_chatApi.szActiveWndModule, si->pszModule);
	}
}

static SESSION_INFO* GetActiveSession(void)
{
	SESSION_INFO *si = SM_FindSession(g_chatApi.szActiveWndID, g_chatApi.szActiveWndModule);
	if (si)
		return si;

	return g_arSessions[0];
}

/////////////////////////////////////////////////////////////////////////////////////////
// Log manager functions
//	Necessary to keep track of events in a window log

static LOGINFO* LM_AddEvent(LOGINFO **ppLogListStart, LOGINFO **ppLogListEnd)
{
	if (!ppLogListStart || !ppLogListEnd)
		return nullptr;

	LOGINFO *node = (LOGINFO *)mir_calloc(sizeof(LOGINFO));
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

/////////////////////////////////////////////////////////////////////////////////////////
//	Session Manager functions
//	Keeps track of all sessions and its windows

GCSessionInfoBase::GCSessionInfoBase() :
	arKeys(10, CompareKeys),
	arUsers(10, CompareUser)
{}

GCSessionInfoBase::~GCSessionInfoBase()
{}

const char *GCSessionInfoBase::getSoundName(int iEventType) const
{
	// if sounds are filtered out for this event type, do nothing;
	if (!(db_get_dw(0, CHAT_MODULE, "SoundFlags", GC_EVENT_HIGHLIGHT) & iEventType))
		return nullptr;

	if (iEventType & GC_EVENT_HIGHLIGHT)
		return "ChatHighlight";

	switch (iEventType) {
	case GC_EVENT_JOIN:           return "ChatJoin";
	case GC_EVENT_PART:           return "ChatPart";
	case GC_EVENT_QUIT:           return "ChatQuit";
	case GC_EVENT_ADDSTATUS:
	case GC_EVENT_REMOVESTATUS:   return "ChatMode";
	case GC_EVENT_KICK:           return "ChatKick";
	case GC_EVENT_ACTION:         return "ChatAction";
	case GC_EVENT_NICK:           return "ChatNick";
	case GC_EVENT_NOTICE:         return "ChatNotice";
	case GC_EVENT_TOPIC:          return "ChatTopic";
	case GC_EVENT_MESSAGE:
		bool bInactive = pDlg == nullptr || !pDlg->IsActive();
		return (bInactive) ? "RecvMsgInactive" : "RecvMsgActive";
	}

	return nullptr;
}

static SESSION_INFO* SM_CreateSession(void)
{
	return new SESSION_INFO();
}

static void SM_FreeSession(SESSION_INFO *si, bool bRemoveContact = false)
{
	if (g_clistApi.pfnGetEvent(si->hContact, 0))
		g_clistApi.pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);
	si->wState &= ~STATE_TALK;
	db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);

	if (si->pDlg)
		si->pDlg->CloseTab();

	Chat_DoEventHook(si, GC_SESSION_TERMINATE, nullptr, nullptr, (INT_PTR)si->pItemData);

	if (si->hContact && bRemoveContact)
		db_delete_contact(si->hContact);

	// contact may have been deleted here already, since function may be called after deleting
	// contact so the handle may be invalid, therefore db_get_b shall return 0
	if (si->hContact && Contact::IsGroupChat(si->hContact, si->pszModule)) {
		g_chatApi.SetOffline(si->hContact, (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS) ? TRUE : FALSE);
		db_set_s(si->hContact, si->pszModule, "Topic", "");
		db_set_s(si->hContact, si->pszModule, "StatusBar", "");
		db_unset(si->hContact, "CList", "StatusMsg");
	}

	UM_RemoveAll(si);
	g_chatApi.TM_RemoveAll(&si->pStatuses);
	g_chatApi.LM_RemoveAll(&si->pLog, &si->pLogEnd);

	si->iStatusCount = 0;

	mir_free(si->pszModule);
	mir_free(si->ptszID);
	mir_free(si->ptszName);
	mir_free(si->ptszStatusbarText);
	mir_free(si->ptszTopic);
	delete si;
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

	auto T = g_arSessions.rev_iter();
	for (auto &si : T) {
		if (si->iType != GCW_SERVER && !mir_strcmpi(si->pszModule, pszModule)) {
			SM_FreeSession(si, removeContact);
			g_arSessions.removeItem(&si);
		}
	}
	return TRUE;
}

SESSION_INFO* SM_FindSession(const wchar_t *pszID, const char *pszModule)
{
	if (!pszID || !pszModule)
		return nullptr;

	SESSION_INFO *tmp = (SESSION_INFO*)alloca(sizeof(SESSION_INFO));
	tmp->pszModule = (char*)pszModule;
	tmp->ptszID = (wchar_t*)pszID;

	mir_cslock lck(csChat);
	return g_arSessions.find(tmp);
}

BOOL SM_SetOffline(const char *pszModule, SESSION_INFO *si)
{
	if (si == nullptr) {
		if (pszModule == nullptr)
			return FALSE;

		for (auto &p : g_arSessions) {
			if (!_strcmpi(p->pszModule, pszModule))
				SM_SetOffline(pszModule, p);
		}
		return TRUE;
	}

	UM_RemoveAll(si);
	si->pMe = nullptr;

	if (si->iType != GCW_SERVER)
		si->bInitDone = false;
	if (g_chatApi.OnOfflineSession)
		g_chatApi.OnOfflineSession(si);
	return TRUE;
}

static HICON SM_GetStatusIcon(SESSION_INFO *si, USERINFO *ui)
{
	if (!ui || !si)
		return nullptr;

	STATUSINFO *ti = g_chatApi.TM_FindStatus(si->pStatuses, g_chatApi.TM_WordToString(si->pStatuses, ui->Status));
	if (ti != nullptr)
		return g_chatApi.hStatusIcons[ti->iIconIndex];
	
	return g_chatApi.hStatusIcons[0];
}

BOOL SM_AddEvent(const wchar_t *pszID, const char *pszModule, GCEVENT *gce, bool bIsHighlighted)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return TRUE;

	LOGINFO *li = LM_AddEvent(&si->pLog, &si->pLogEnd);
	si->iEventCount++;

	li->iType = gce->iType;
	li->ptszNick = mir_wstrdup(gce->pszNick.w);
	li->ptszText = mir_wstrdup(gce->pszText.w);
	li->ptszStatus = mir_wstrdup(gce->pszStatus.w);
	li->ptszUserInfo = mir_wstrdup(gce->pszUserInfo.w);

	li->bIsMe = gce->bIsMe;
	li->time = gce->time;
	li->bIsHighlighted = bIsHighlighted;

	if (g_Settings->iEventLimit > 0 && si->iEventCount > g_Settings->iEventLimit + 20) {
		LM_TrimLog(&si->pLog, &si->pLogEnd, si->iEventCount - g_Settings->iEventLimit);
		si->bTrimmed = true;
		si->iEventCount = g_Settings->iEventLimit;
		return FALSE;
	}
	return TRUE;
}

BOOL SM_RemoveUser(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID)
{
	if (!pszModule || !pszUID)
		return FALSE;

	for (auto &si : g_arSessions) {
		if ((pszID && mir_wstrcmpi(si->ptszID, pszID)) || mir_strcmpi(si->pszModule, pszModule))
			continue;

		USERINFO *ui = UM_FindUser(si, pszUID);
		if (ui) {
			if (g_chatApi.OnRemoveUser)
				g_chatApi.OnRemoveUser(si, ui);

			if (si->pMe == ui)
				si->pMe = nullptr;
			g_chatApi.UM_RemoveUser(si, pszUID);

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
	return (si == nullptr) ? nullptr : g_chatApi.UM_FindUserFromIndex(si, index);
}

BOOL SM_GiveStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, const wchar_t *pszStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return FALSE;
	
	USERINFO *ui = g_chatApi.UM_GiveStatus(si, pszUID, TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		UM_SortUser(si);
		if (si->pDlg)
			si->pDlg->UpdateNickList();
	}
	return TRUE;
}

BOOL SM_SetContactStatus(const wchar_t *pszID, const char *pszModule, const wchar_t *pszUID, uint16_t wStatus)
{
	SESSION_INFO *si = SM_FindSession(pszID, pszModule);
	if (si == nullptr)
		return FALSE;

	USERINFO *ui = g_chatApi.UM_SetContactStatus(si, pszUID, wStatus);
	if (ui) {
		UM_SortUser(si);
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

	USERINFO *ui = g_chatApi.UM_TakeStatus(si, pszUID, TM_StringToWord(si->pStatuses, pszStatus));
	if (ui) {
		UM_SortUser(si);
		if (si->pDlg)
			si->pDlg->UpdateNickList();
	}
	return TRUE;
}

BOOL SM_SetStatus(const char *pszModule, SESSION_INFO *si, int wStatus)
{
	if (si == nullptr) {
		if (pszModule == nullptr)
			return FALSE;

		for (auto &p : g_arSessions) {
			if (!_strcmpi(p->pszModule, pszModule))
				SM_SetStatus(pszModule, p, wStatus);
		}
		return TRUE;
	}

	si->wStatus = wStatus;
	if (si->hContact)
		db_set_w(si->hContact, si->pszModule, "Status", (uint16_t)wStatus);

	if (g_chatApi.OnSetStatus)
		g_chatApi.OnSetStatus(si, wStatus);

	return TRUE;
}

BOOL SM_UserTyping(GCEVENT *gce)
{
	SESSION_INFO *si = SM_FindSession(gce->pszID.w, gce->pszModule);
	if (si == nullptr || si->pDlg == nullptr)
		return FALSE;

	USERINFO* ui = UM_FindUser(si, gce->pszUID.w);
	if (ui == nullptr)
		return FALSE;

	si->pDlg->setTyping(10, ui);
	return TRUE;
}

BOOL SM_ChangeNick(const wchar_t *pszID, const char *pszModule, GCEVENT *gce)
{
	if (!pszModule)
		return FALSE;

	for (auto &si : g_arSessions) {
		if ((!pszID || !mir_wstrcmpi(si->ptszID, pszID)) && !mir_strcmpi(si->pszModule, pszModule)) {
			USERINFO *ui = UM_FindUser(si, gce->pszUID.w);
			if (ui) {
				replaceStrW(ui->pszNick, gce->pszText.w);
				UM_SortUser(si);
				if (si->pDlg)
					si->pDlg->UpdateNickList();
				if (g_chatApi.OnChangeNick)
					g_chatApi.OnChangeNick(si);
			}

			if (pszID)
				return TRUE;
		}
	}
	return TRUE;
}

void SM_RemoveAll(void)
{
	for (auto &it : g_arSessions.rev_iter()) {
		SM_FreeSession(it, false);
		g_arSessions.removeItem(&it);
	}
}

static int SM_GetCount(const char *pszModule)
{
	int count = 0;

	for (auto &si : g_arSessions)
		if (!mir_strcmpi(pszModule, si->pszModule))
			count++;

	return count;
}

SESSION_INFO* SM_FindSessionByIndex(const char *pszModule, int iItem)
{
	int count = 0;

	mir_cslock lck(csChat);
	for (auto &si : g_arSessions) {
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

	CMStringA res;
	
	for (auto &it : si->getUserList())
		res.AppendFormat("%S ", it->pszNick);

	return res.Detach();
}

static void SM_InvalidateLogDirectories()
{
	for (auto &si : g_arSessions)
		si->pszLogFileName[0] = si->pszLogFileName[1] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	Module Manager functions
//	Necessary to keep track of all modules that has registered with the plugin

GCModuleInfoBase::GCModuleInfoBase()
{}

GCModuleInfoBase::~GCModuleInfoBase()
{
	if (g_chatApi.OnDestroyModule)
		g_chatApi.OnDestroyModule((MODULEINFO*)this);

	mir_free(pszModule);
	mir_free(ptszModDispName);
	mir_free(pszHeader);
}

MODULEINFO* MM_AddModule(const char *pszModule)
{
	if (pszModule == nullptr)
		return nullptr;

	if (MM_FindModule(pszModule))
		return nullptr;

	MODULEINFO *node = g_chatApi.MM_CreateModule();
	replaceStr(node->pszModule, pszModule);
	if (g_chatApi.OnCreateModule)
		g_chatApi.OnCreateModule(node);

	g_arModules.insert(node);
	return node;
}

static MODULEINFO* MM_CreateModule()
{
	return new MODULEINFO();
}

static void MM_IconsChanged()
{
	LoadChatIcons();

	for (auto &mi : g_arModules)
		if (g_chatApi.OnCreateModule) // recreate icons
			g_chatApi.OnCreateModule(mi);
}

static void MM_FontsChanged()
{
	for (auto &mi : g_arModules)
		mi->pszHeader = g_chatApi.Log_CreateRtfHeader();
}

MODULEINFO* MM_FindModule(const char *pszModule)
{
	if (!pszModule)
		return nullptr;

	MODULEINFO *tmp = (MODULEINFO*)_alloca(sizeof(MODULEINFO));
	tmp->pszModule = (char*)pszModule;
	return g_arModules.find(tmp);
}

static BOOL MM_RemoveAll(void)
{
	for (auto &it : g_arModules)
		delete it;
	g_arModules.destroy();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Status manager functions
// Necessary to keep track of what user statuses per window nicklist that is available

STATUSINFO* TM_AddStatus(STATUSINFO **ppStatusList, const wchar_t *pszStatus, int *iCount)
{
	if (!ppStatusList || !pszStatus)
		return nullptr;

	if (!g_chatApi.TM_FindStatus(*ppStatusList, pszStatus)) {
		STATUSINFO *node = (STATUSINFO*)mir_calloc(sizeof(STATUSINFO));
		replaceStrW(node->pszGroup, pszStatus);
		node->iIconIndex = *iCount;
		while (node->iIconIndex > STATUSICONCOUNT - 1)
			node->iIconIndex--;

		if (*ppStatusList == nullptr) { // list is empty
			node->iStatus = 1;
			*ppStatusList = node;
			node->next = nullptr;
		}
		else {
			node->iStatus = ppStatusList[0]->iStatus * 2;
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

	return nullptr;
}

uint16_t TM_StringToWord(STATUSINFO *pStatusList, const wchar_t *pszStatus)
{
	if (!pStatusList || !pszStatus)
		return 0;

	for (STATUSINFO *pTemp = pStatusList; pTemp != nullptr; pTemp = pTemp->next) {
		if (mir_wstrcmpi(pTemp->pszGroup, pszStatus) == 0)
			return pTemp->iStatus;

		if (pTemp->next == nullptr)
			return pStatusList->iStatus;
	}
	return 0;
}

static wchar_t* TM_WordToString(STATUSINFO *pStatusList, uint16_t Status)
{
	if (!pStatusList)
		return nullptr;

	for (STATUSINFO *pTemp = pStatusList; pTemp != nullptr; pTemp = pTemp->next) {
		if (pTemp->iStatus & Status) {
			Status -= pTemp->iStatus;
			if (Status == 0)
				return pTemp->pszGroup;
		}
	}
	return nullptr;
}

static BOOL TM_RemoveAll(STATUSINFO **ppStatusList)
{
	if (!ppStatusList)
		return FALSE;

	while (*ppStatusList != nullptr) {
		STATUSINFO *pLast = ppStatusList[0]->next;
		mir_free(ppStatusList[0]->pszGroup);
		mir_free(*ppStatusList);
		*ppStatusList = pLast;
	}
	*ppStatusList = nullptr;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// User manager functions
// Necessary to keep track of the users in a window nicklist

static USERINFO* UM_FindUser(SESSION_INFO *si, const wchar_t *pszUID)
{
	if (!si || !pszUID)
		return nullptr;

	USERINFO tmp;
	tmp.pszUID = (wchar_t*)pszUID;
	return si->getKeyList().find(&tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int compareKeysStub(const void *p1, const void *p2)
{
	return CompareKeys(*(USERINFO**)p1, *(USERINFO**)p2);
}

void UM_SortKeys(SESSION_INFO *si)
{
	qsort(si->arKeys.getArray(), si->arKeys.getCount(), sizeof(void*), compareKeysStub);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int compareStub(const void *p1, const void *p2)
{
	return CompareUser(*(USERINFO**)p1, *(USERINFO**)p2);
}

void UM_SortUser(SESSION_INFO *si)
{
	qsort(si->arUsers.getArray(), si->arUsers.getCount(), sizeof(void*), compareStub);
}

/////////////////////////////////////////////////////////////////////////////////////////

USERINFO* UM_AddUser(SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszNick, uint16_t wStatus)
{
	if (pszNick == nullptr)
		return nullptr;

	auto *pUser = UM_FindUser(si, pszUID);
	if (pUser == nullptr) {
		pUser = new USERINFO();
		replaceStrW(pUser->pszUID, pszUID);
		si->getKeyList().insert(pUser);
		si->getUserList().insert(pUser);
	}
	
	replaceStrW(pUser->pszNick, pszNick);
	pUser->Status = wStatus;
	return pUser;
}

static int UM_CompareItem(const USERINFO *u1, const USERINFO *u2)
{
	uint16_t dw1 = u1->Status;
	uint16_t dw2 = u2->Status;

	for (int i = 0; i < 8; i++) {
		if ((dw1 & 1) && !(dw2 & 1))
			return -1;
		if ((dw2 & 1) && !(dw1 & 1))
			return 1;
		if ((dw1 & 1) && (dw2 & 1))
			break;

		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}
	return mir_wstrcmpi(u1->pszNick, u2->pszNick);
}

static USERINFO* UM_FindUserFromIndex(SESSION_INFO *si, int index)
{
	if (!si)
		return nullptr;

	int i = 0;
	for (auto &ui : si->getUserList()) {
		if (i == index)
			return ui;
		i++;
	}
	return nullptr;
}

static USERINFO* UM_GiveStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t status)
{
	USERINFO *ui = UM_FindUser(si, pszUID);
	if (ui == nullptr)
		return nullptr;

	ui->Status |= status;
	return ui;
}

static USERINFO* UM_SetContactStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t status)
{
	USERINFO *ui = UM_FindUser(si, pszUID);
	if (ui == nullptr)
		return nullptr;
	
	ui->ContactStatus = status;
	return ui;
}

BOOL UM_SetStatusEx(SESSION_INFO *si, const wchar_t* pszText, int flags)
{
	int bOnlyMe = (flags & GC_SSE_ONLYLISTED) != 0, bSetStatus = (flags & GC_SSE_ONLINE) != 0;
	char cDelimiter = (flags & GC_SSE_TABDELIMITED) ? '\t' : ' ';

	if (bOnlyMe) {
		USERINFO *ui = UM_FindUser(si, pszText);
		if (ui == nullptr)
			return FALSE;

		ui->iStatusEx = (bSetStatus) ? 1 : 0;
		return TRUE;
	}

	for (auto &ui : si->getUserList()) {
		ui->iStatusEx = 0;

		if (pszText != nullptr) {
			wchar_t *s = (wchar_t *)wcsstr(pszText, ui->pszUID);
			if (s) {
				ui->iStatusEx = 0;
				if (s == pszText || s[-1] == cDelimiter) {
					size_t len = mir_wstrlen(ui->pszUID);
					if (s[len] == cDelimiter || s[len] == '\0')
						ui->iStatusEx = bSetStatus ? 1 : 0;
				}
			}
		}
	}
	return TRUE;
}

static USERINFO* UM_TakeStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t status)
{
	USERINFO *ui = UM_FindUser(si, pszUID);
	if (ui == nullptr)
		return nullptr;

	ui->Status &= ~status;
	return ui;
}

static wchar_t* UM_FindUserAutoComplete(SESSION_INFO *si, const wchar_t* pszOriginal, const wchar_t* pszCurrent)
{
	if (!si || !pszOriginal || !pszCurrent)
		return nullptr;

	wchar_t *pszName = nullptr;
	for (auto &ui : si->getUserList())
		if (ui->pszNick && mir_wstrstri(ui->pszNick, pszOriginal) == ui->pszNick)
			if (mir_wstrcmpi(ui->pszNick, pszCurrent) > 0 && (!pszName || mir_wstrcmpi(ui->pszNick, pszName) < 0))
				pszName = ui->pszNick;

	return pszName;
}

static BOOL UM_RemoveUser(SESSION_INFO *si, const wchar_t *pszUID)
{
	auto *pUser = UM_FindUser(si, pszUID);
	if (pUser == nullptr)
		return FALSE;

	auto &arKeys = si->getKeyList();
	if (arKeys.remove(pUser) == -1)
		DebugBreak();

	auto &arUsers = si->getUserList();
	for (auto &ui : arUsers) {
		if (!mir_wstrcmpi(ui->pszUID, pszUID)) {
			mir_free(ui->pszNick);
			mir_free(ui->pszUID);
			arUsers.removeItem(&ui);
			break;
		}
	}
	return TRUE;
}

BOOL UM_RemoveAll(SESSION_INFO *si)
{
	if (!si)
		return FALSE;

	if (!si->pParent) {
		for (auto &ui : si->arUsers) {
			mir_free(ui->pszUID);
			mir_free(ui->pszNick);
		}
		si->arKeys.destroy();
		si->arUsers.destroy();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void CreateNick(const SESSION_INFO *, const LOGINFO *lin, CMStringW &wszNick)
{
	if (lin->ptszNick) {
		wszNick = lin->ptszNick;
		if (g_Settings->bLogLimitNames && mir_wstrlen(lin->ptszNick) > 20) {
			wszNick.Truncate(20);
			wszNick.Append(L"...");
		}

		if (lin->ptszUserInfo && lin->iType != GC_EVENT_TOPIC)
			wszNick.AppendFormat(L" (%s)", lin->ptszUserInfo);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void ResetApi()
{
	g_chatApi.SetActiveSession = ::SetActiveSession;
	g_chatApi.GetActiveSession = ::GetActiveSession;
	g_chatApi.SM_CreateSession = ::SM_CreateSession;
	g_chatApi.SM_FindSession = ::SM_FindSession;
	g_chatApi.SM_GetStatusIcon = ::SM_GetStatusIcon;
	g_chatApi.SM_GetCount = ::SM_GetCount;
	g_chatApi.SM_FindSessionByIndex = ::SM_FindSessionByIndex;
	g_chatApi.SM_GetUserFromIndex = ::SM_GetUserFromIndex;
	g_chatApi.SM_InvalidateLogDirectories = ::SM_InvalidateLogDirectories;

	g_chatApi.MM_CreateModule = ::MM_CreateModule;
	g_chatApi.MM_FontsChanged = ::MM_FontsChanged;
	g_chatApi.MM_IconsChanged = ::MM_IconsChanged;
	g_chatApi.MM_RemoveAll = ::MM_RemoveAll;

	g_chatApi.TM_FindStatus = ::TM_FindStatus;
	g_chatApi.TM_WordToString = ::TM_WordToString;
	g_chatApi.TM_RemoveAll = ::TM_RemoveAll;

	g_chatApi.UM_AddUser = ::UM_AddUser;
	g_chatApi.UM_CompareItem = ::UM_CompareItem;
	g_chatApi.UM_FindUser = ::UM_FindUser;
	g_chatApi.UM_FindUserFromIndex = ::UM_FindUserFromIndex;
	g_chatApi.UM_GiveStatus = ::UM_GiveStatus;
	g_chatApi.UM_SetContactStatus = ::UM_SetContactStatus;
	g_chatApi.UM_TakeStatus = ::UM_TakeStatus;
	g_chatApi.UM_FindUserAutoComplete = ::UM_FindUserAutoComplete;
	g_chatApi.UM_RemoveUser = ::UM_RemoveUser;

	g_chatApi.LM_RemoveAll = ::LM_RemoveAll;

	g_chatApi.SetOffline = ::SetOffline;
	g_chatApi.SetAllOffline = ::SetAllOffline;
	g_chatApi.FindRoom = ::FindRoom;
	g_chatApi.DoRtfToTags = ::DoRtfToTags;

	g_chatApi.Log_CreateRTF = ::Log_CreateRTF;
	g_chatApi.Log_CreateRtfHeader = ::Log_CreateRtfHeader;
	g_chatApi.LoadMsgDlgFont = ::LoadMsgDlgFont;
	g_chatApi.MakeTimeStamp = ::MakeTimeStamp;

	g_chatApi.DoSoundsFlashPopupTrayStuff = ::DoSoundsFlashPopupTrayStuff;
	g_chatApi.DoTrayIcon = ::DoTrayIcon;
	g_chatApi.DoPopup = ::DoPopup;
	g_chatApi.ShowPopup = ::ShowPopup;
	g_chatApi.GetChatLogsFilename = ::GetChatLogsFilename;
	g_chatApi.Log_SetStyle = ::Log_SetStyle;

	g_chatApi.CreateNick = ::CreateNick;
	g_chatApi.IsHighlighted = ::IsHighlighted;
	g_chatApi.RemoveFormatting = ::RemoveFormatting;
	g_chatApi.ReloadSettings = ::LoadGlobalSettings;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(CHAT_MANAGER*) Chat_CustomizeApi(const CHAT_MANAGER_INITDATA *pInit)
{
	if (pInit == nullptr)
		return &g_chatApi;

	// wipe out old junk
	memset((uint8_t*)&g_chatApi + offsetof(CHAT_MANAGER, OnCreateModule), 0, sizeof(CHAT_MANAGER) - offsetof(CHAT_MANAGER, OnCreateModule));

	if (g_cbSession) { // reallocate old sessions
		mir_cslock lck(csChat);

		LIST<SESSION_INFO> tmp(g_arSessions);
		g_arSessions.destroy();

		for (auto &p : tmp) {
			SESSION_INFO *p1 = (SESSION_INFO*)realloc(p, pInit->cbSession);
			memset((uint8_t*)p1 + sizeof(SESSION_INFO), 0, pInit->cbSession - sizeof(SESSION_INFO));
			g_arSessions.insert(p1);
		}
	}
	
	if (g_cbModuleInfo != pInit->cbModuleInfo) { // reallocate old modules
		bool bReallocated = false;
		mir_cslock lck(csChat);

		LIST<MODULEINFO> tmp(g_arModules);
		g_arModules.destroy();

		for (auto &mi : tmp) {
			MODULEINFO *p1 = (MODULEINFO*)realloc(mi, pInit->cbModuleInfo);
			memset((uint8_t*)p1 + sizeof(GCModuleInfoBase), 0, pInit->cbModuleInfo - sizeof(GCModuleInfoBase));
			g_arModules.insert(p1);
			if (p1 != mi) // realloc could change a pointer
				bReallocated = true;
		}
	
		if (bReallocated)
			for (auto &si : g_arSessions)
				si->pMI = MM_FindModule(si->pszModule);
	}

	g_Settings = pInit->pSettings;
	g_szFontGroup = pInit->szFontGroup;
	g_cbSession = pInit->cbSession;
	g_cbModuleInfo = pInit->cbModuleInfo;
	g_iFontMode = pInit->iFontMode;
	g_pChatPlugin = pInit->pPlugin;

	ResetApi();

	LoadChatIcons();
	RegisterFonts();
	OptionsInit();
	return &g_chatApi;
}

/////////////////////////////////////////////////////////////////////////////////////////

void Chat_RemoveContact(MCONTACT hContact)
{
	for (auto &si : g_arSessions) {
		if (si->hContact == hContact) {
			SM_FreeSession(si);
			g_arSessions.removeItem(&si);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

CHAT_MANAGER::CHAT_MANAGER() :
	arSessions(g_arSessions),
	bRightClickFilter(CHAT_MODULE, "RightClickFilter", false)
{
	pLogIconBmpBits = ::pLogIconBmpBits;

	ResetApi();
}

MIR_APP_EXPORT CHAT_MANAGER g_chatApi;
