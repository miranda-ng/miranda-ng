/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-25 Miranda NG team,
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

static int CompareUser(const USERINFO *u1, const USERINFO *u2)
{
	return mir_wstrcmp(u1->pszUID, u2->pszUID);
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
// SESSION_INFO class members

static int CompareEvents(const LOGINFO *p1, const LOGINFO *p2)
{
	if (p1->time != p2->time)
		return (p1->time < p2->time) ? -1 : 1;

	return (int)p1->hEvent - (int)p2->hEvent;
}

SESSION_INFO::SESSION_INFO() :
	arUsers(10, CompareUser),
	arEvents(10, CompareEvents),
	arStatuses(1)
{
	iLastEvent = MAXINT/2;
}

SESSION_INFO::~SESSION_INFO()
{}

const char* SESSION_INFO::SESSION_INFO::getSoundName(int iEventType) const
{
	// if sounds are filtered out for this event type, do nothing;
	if (!(Chat::iSoundFlags & iEventType))
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

void SESSION_INFO::markRead(bool bForce)
{
	if ((wState & STATE_TALK) || bForce) {
		wState &= ~STATE_TALK;
		db_unset(hContact, pszModule, "ApparentMode");
	}

	if ((wState & GC_EVENT_HIGHLIGHT) || bForce) {
		wState &= ~GC_EVENT_HIGHLIGHT;

		if (Clist_GetEvent(hContact, 0))
			Clist_RemoveEvent(hContact, GC_FAKE_EVENT);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//	Session Manager functions
//	Keeps track of all sessions and its windows

void SM_FreeSession(SESSION_INFO *si)
{
	si->markRead(true);
	db_unset(si->hContact, si->pszModule, "ApparentMode");

	if (si->pDlg)
		si->pDlg->CloseTab();

	Chat_DoEventHook(si, GC_SESSION_TERMINATE, nullptr, nullptr, (INT_PTR)si->pItemData);

	// contact may have been deleted here already, since function may be called after deleting
	// contact so the handle may be invalid, therefore db_get_b shall return 0
	if (si->hContact && Contact::IsGroupChat(si->hContact, si->pszModule)) {
		SetOffline(si->hContact);
		db_set_s(si->hContact, si->pszModule, "Topic", "");
		db_set_s(si->hContact, si->pszModule, "StatusBar", "");
		db_unset(si->hContact, "CList", "StatusMsg");
	}

	UM_RemoveAll(si);

	si->arStatuses.destroy();
	si->arEvents.destroy();
	si->iStatusCount = 0;

	mir_free(si->pszModule);
	mir_free(si->ptszID);
	mir_free(si->ptszName);
	mir_free(si->ptszStatusbarText);
	mir_free(si->ptszTopic);
	delete si;
}

MIR_APP_DLL(SESSION_INFO*) Chat_Find(MCONTACT hContact, const char *pszModule)
{
	if (pszModule == nullptr)
		pszModule = Proto_GetBaseAccountName(hContact);

	ptrW contactId(Contact::GetInfo(CNF_UNIQUEID, hContact, pszModule));
	return Chat_Find(contactId, pszModule);
}

MIR_APP_DLL(SESSION_INFO*) Chat_Find(const wchar_t *pszID, const char *pszModule)
{
	if (!pszID || !pszModule)
		return nullptr;

	SESSION_INFO *tmp = (SESSION_INFO*)alloca(sizeof(SESSION_INFO));
	tmp->pszModule = (char*)pszModule;
	tmp->ptszID = (wchar_t*)pszID;

	mir_cslock lck(csChat);
	return g_arSessions.find(tmp);
}

/////////////////////////////////////////////////////////////////////////////////////////
// User manager functions
// Necessary to keep track of the users in a window nicklist

static USERINFO* UM_FindUser(SESSION_INFO *si, const wchar_t *pszUID)
{
	if (!si || !pszUID)
		return nullptr;

	mir_cslock lck(si->csLock);
	USERINFO tmp;
	tmp.pszUID = (wchar_t *)pszUID;
	return si->getUserList().find(&tmp);
}

USERINFO* UM_AddUser(SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszNick, uint16_t wStatus)
{
	if (!si || !pszUID || !pszNick)
		return nullptr;

	mir_cslock lck(si->csLock);
	auto *pUser = UM_FindUser(si, pszUID);
	if (pUser == nullptr) {
		pUser = new USERINFO();
		replaceStrW(pUser->pszUID, pszUID);
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

static USERINFO* UM_GiveStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t status)
{
	USERINFO *ui = UM_FindUser(si, pszUID);
	if (ui == nullptr)
		return nullptr;

	ui->Status |= status;
	return ui;
}

static USERINFO* UM_SetStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t status)
{
	USERINFO *ui = UM_FindUser(si, pszUID);
	if (ui == nullptr)
		return nullptr;

	if (ui->Status == status)
		return nullptr;

	ui->Status = status;
	return ui;
}

static USERINFO* UM_SetContactStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t status)
{
	if (auto *ui = UM_FindUser(si, pszUID)) {
		ui->ContactStatus = status;
		return ui;
	}

	return nullptr;
}

BOOL UM_SetStatusEx(SESSION_INFO *si, const wchar_t *pszText, int flags)
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

static wchar_t* UM_FindUserAutoComplete(SESSION_INFO *si, const wchar_t *pszOriginal, const wchar_t *pszCurrent)
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

BOOL UM_RemoveAll(SESSION_INFO *si)
{
	if (!si)
		return FALSE;

	if (!si->pParent) {
		for (auto &ui : si->arUsers) {
			mir_free(ui->pszUID);
			mir_free(ui->pszNick);
		}
		si->arUsers.destroy();
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat session manager

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

	STATUSINFO *ti = TM_FindStatus(si, TM_WordToString(si, ui->Status));
	if (ti != nullptr)
		return g_chatApi.hStatusIcons[ti->iIconIndex];
	
	return g_chatApi.hStatusIcons[0];
}

LOGINFO* SM_AddEvent(SESSION_INFO *si, GCEVENT *gce, bool bIsHighlighted)
{
	if (si == nullptr)
		return nullptr;

	// we don't add filtered events into array, let's simply skip them
	int iActiveFlags = (si->bFilterEnabled) ? si->iLogFilterFlags : Chat::iFilterFlags;
	if (!(gce->iType & iActiveFlags))
		return nullptr;

	LOGINFO *li = new LOGINFO(gce);
	li->bIsHighlighted = bIsHighlighted;
	if (si->pMI->bDatabase) {
		li->hEvent = gce->hEvent;
		if (gce->hEvent) {
			if (si->arEvents.find(li)) {
				delete li;
				return nullptr;
			}
		}
	}
	else li->hEvent = si->iLastEvent++;

	if (g_Settings->iEventLimit > 0 && si->arEvents.getCount() > g_Settings->iEventLimit + 20)
		for (int i = si->arEvents.getCount() - g_Settings->iEventLimit; i >= 0; i--)
			si->arEvents.remove(0);

	si->arEvents.insert(li);

	return li;
}

BOOL SM_RemoveUser(SESSION_INFO *si, const wchar_t *pszUID)
{
	if (!si || !pszUID)
		return FALSE;

	mir_cslock lck(si->csLock);
	USERINFO *ui = UM_FindUser(si, pszUID);
	if (!ui)
		return FALSE;

	if (g_chatApi.OnRemoveUser)
		g_chatApi.OnRemoveUser(si, ui);

	if (si->pMe == ui)
		si->pMe = nullptr;
	
	auto &arUsers = si->getUserList();
	for (auto &it : arUsers) {
		if (!mir_wstrcmpi(it->pszUID, pszUID)) {
			mir_free(it->pszNick);
			mir_free(it->pszUID);
			arUsers.removeItem(&it);
			break;
		}
	}
	
	if (si->pDlg)
		si->pDlg->UpdateNickList();

	return TRUE;
}

BOOL SM_GiveStatus(SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszStatus)
{
	if (si == nullptr)
		return FALSE;
	
	USERINFO *ui = UM_GiveStatus(si, pszUID, TM_StringToWord(si, pszStatus));
	if (ui && si->pDlg)
		si->pDlg->UpdateNickList();
	return TRUE;
}

BOOL SM_AssignStatus(SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszStatus)
{
	if (si != nullptr)
		if (USERINFO *ui = UM_SetStatus(si, pszUID, TM_StringToWord(si, pszStatus))) {
			if (si->pDlg)
				si->pDlg->UpdateNickList();
			return TRUE;
		}

	return FALSE;
}

BOOL SM_SetContactStatus(SESSION_INFO *si, const wchar_t *pszUID, uint16_t wStatus)
{
	if (si == nullptr)
		return FALSE;

	USERINFO *ui = UM_SetContactStatus(si, pszUID, wStatus);
	if (ui && si->pDlg)
		si->pDlg->UpdateNickList();
	return TRUE;
}

BOOL SM_TakeStatus(SESSION_INFO *si, const wchar_t *pszUID, const wchar_t *pszStatus)
{
	if (si == nullptr)
		return FALSE;

	USERINFO *ui = g_chatApi.UM_TakeStatus(si, pszUID, TM_StringToWord(si, pszStatus));
	if (ui && si->pDlg)
		si->pDlg->UpdateNickList();
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
	SESSION_INFO *si = gce->si;
	if (si == nullptr || si->pDlg == nullptr)
		return FALSE;

	USERINFO* ui = UM_FindUser(si, gce->pszUID.w);
	if (ui == nullptr)
		return FALSE;

	si->pDlg->setTyping(10, ui);
	return TRUE;
}

BOOL SM_ChangeNick(SESSION_INFO *si, GCEVENT *gce)
{
	if (!si)
		return FALSE;

	USERINFO *ui = UM_FindUser(si, gce->pszUID.w);
	if (ui) {
		replaceStrW(ui->pszNick, gce->pszText.w);
		if (si->pDlg)
			si->pDlg->UpdateNickList();
		if (g_chatApi.OnChangeNick)
			g_chatApi.OnChangeNick(si);
	}

	return TRUE;
}

void SM_RemoveAll(void)
{
	for (auto &it : g_arSessions.rev_iter()) {
		SM_FreeSession(it);
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

SESSION_INFO* SM_FindSessionByContact(MCONTACT hContact)
{
	mir_cslock lck(csChat);
	for (auto &si : g_arSessions)
		if (si->hContact == hContact)
			return si;

	return nullptr;
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

__inline OBJLIST<STATUSINFO>& getStatuses(SESSION_INFO *si)
{
	return (si->pParent) ? si->pParent->arStatuses : si->arStatuses;
}

STATUSINFO* TM_AddStatus(SESSION_INFO *si, const wchar_t *pszStatus)
{
	if (!si || !pszStatus)
		return nullptr;

	if (TM_FindStatus(si, pszStatus))
		return nullptr;

	auto *node = new STATUSINFO();
	node->pszGroup = mir_wstrdup(pszStatus);
	node->iIconIndex = si->iStatusCount;
	while (node->iIconIndex > STATUSICONCOUNT - 1)
		node->iIconIndex--;

	auto &pList = getStatuses(si);
	if (pList.getCount() == 0) // list is empty
		node->iStatus = 1;
	else
		node->iStatus = pList[pList.getCount()-1].iStatus * 2;
	
	pList.insert(node);
	si->bIsDirty = true;
	si->iStatusCount++;
	return node;
}

STATUSINFO* TM_FindStatus(SESSION_INFO *si, const wchar_t *pszStatus)
{
	if (!si || !pszStatus)
		return nullptr;

	for (auto &it: getStatuses(si))
		if (mir_wstrcmpi(it->pszGroup, pszStatus) == 0)
			return it;

	return nullptr;
}

uint16_t TM_StringToWord(SESSION_INFO *si, const wchar_t *pszStatus)
{
	if (!si || !pszStatus)
		return 0;

	auto &pList = getStatuses(si);
	for (auto &it : pList.rev_iter())
		if (mir_wstrcmpi(it->pszGroup, pszStatus) == 0)
			return it->iStatus;

	if (pList.getCount())
		return pList[pList.getCount()-1].iStatus;

	return 0;
}

wchar_t* TM_WordToString(SESSION_INFO *si, uint16_t Status)
{
	if (!si)
		return nullptr;

	for (auto &it : getStatuses(si).rev_iter())
		if (it->iStatus & Status) {
			Status -= it->iStatus;
			if (Status == 0)
				return it->pszGroup;
		}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int compareKeysStub(const void *p1, const void *p2)
{
	return CompareUser(*(USERINFO**)p1, *(USERINFO**)p2);
}

void UM_SortKeys(SESSION_INFO *si)
{
	qsort(si->arUsers.getArray(), si->arUsers.getCount(), sizeof(void*), compareKeysStub);
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
	g_chatApi.SM_GetStatusIcon = ::SM_GetStatusIcon;
	g_chatApi.SM_GetCount = ::SM_GetCount;
	g_chatApi.SM_FindSessionByIndex = ::SM_FindSessionByIndex;
	g_chatApi.SM_InvalidateLogDirectories = ::SM_InvalidateLogDirectories;

	g_chatApi.MM_CreateModule = ::MM_CreateModule;
	g_chatApi.MM_IconsChanged = ::MM_IconsChanged;
	g_chatApi.MM_RemoveAll = ::MM_RemoveAll;

	g_chatApi.UM_AddUser = ::UM_AddUser;
	g_chatApi.UM_CompareItem = ::UM_CompareItem;
	g_chatApi.UM_FindUser = ::UM_FindUser;
	g_chatApi.UM_GiveStatus = ::UM_GiveStatus;
	g_chatApi.UM_RemoveAll = ::UM_RemoveAll;
	g_chatApi.UM_SetContactStatus = ::UM_SetContactStatus;
	g_chatApi.UM_TakeStatus = ::UM_TakeStatus;
	g_chatApi.UM_FindUserAutoComplete = ::UM_FindUserAutoComplete;

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
	g_chatApi.ReloadSettings = ::LoadGlobalSettings;
}

/////////////////////////////////////////////////////////////////////////////////////////

void 	CheckChatCompatibility()
{
	if (db_get_b(0, "Compatibility", "GroupChats") < 1) {
		for (auto &cc : Contacts()) {
			if (auto *pa = Proto_GetContactAccount(cc)) {
				if (!db_get_b(cc, pa->szModuleName, "ChatRoom"))
					continue;

				ptrW wszId(db_get_wsa(cc, pa->szModuleName, "ChatRoomID"));
				Netlib_Logf(0, "Found contact %d with chat %S", cc, wszId.get());
				if (wszId == nullptr)
					continue;

				if (MBaseProto *pd = g_arProtos.find((MBaseProto *)&pa->szProtoName)) {
					if (pd->iUniqueIdType == DBVT_DWORD)
						db_set_dw(cc, pa->szModuleName, pd->szUniqueId, _wtoi(wszId));
					else
						db_set_ws(cc, pa->szModuleName, pd->szUniqueId, wszId);

					db_unset(cc, pa->szModuleName, "ChatRoomID");
				}
				else Netlib_Logf(0, "Protocol %s not found", pa->szModuleName);
			}
		}
		db_set_b(0, "Compatibility", "GroupChats", 1);
	}
}

MIR_APP_DLL(CHAT_MANAGER*) Chat_CustomizeApi(const CHAT_MANAGER_INITDATA *pInit)
{
	if (pInit == nullptr)
		return &g_chatApi;

	// wipe out old junk
	memset((uint8_t*)&g_chatApi + offsetof(CHAT_MANAGER, OnCreateModule), 0, sizeof(CHAT_MANAGER) - offsetof(CHAT_MANAGER, OnCreateModule));
	
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
