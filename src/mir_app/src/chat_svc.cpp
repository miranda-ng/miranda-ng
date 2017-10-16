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

INT_PTR SvcGetChatManager(WPARAM, LPARAM);

#include "chat.h"
#include "resource.h"

mir_cs csChat;
HMENU g_hMenu = nullptr;
HGENMENU hJoinMenuItem, hLeaveMenuItem;
MWindowList g_hWindowList;
HANDLE hevSendEvent, hevBuildMenuEvent;

static HANDLE
   hServiceRegister = nullptr,
   hServiceNewChat = nullptr,
   hServiceAddEvent = nullptr,
   hServiceGetAddEventPtr = nullptr,
   hServiceGetInfo = nullptr,
   hServiceGetCount = nullptr,
   hEventPrebuildMenu = nullptr,
   hEventDoubleclicked = nullptr,
   hEventJoinChat = nullptr,
   hEventLeaveChat = nullptr,
   hHookEvent = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// Post-load event hooks

void LoadChatIcons(void)
{
	chatApi.hIcons[ICON_ACTION] = LoadIconEx("log_action", FALSE);
	chatApi.hIcons[ICON_ADDSTATUS] = LoadIconEx("log_addstatus", FALSE);
	chatApi.hIcons[ICON_HIGHLIGHT] = LoadIconEx("log_highlight", FALSE);
	chatApi.hIcons[ICON_INFO] = LoadIconEx("log_info", FALSE);
	chatApi.hIcons[ICON_JOIN] = LoadIconEx("log_join", FALSE);
	chatApi.hIcons[ICON_KICK] = LoadIconEx("log_kick", FALSE);
	chatApi.hIcons[ICON_MESSAGE] = LoadIconEx("log_message_in", FALSE);
	chatApi.hIcons[ICON_MESSAGEOUT] = LoadIconEx("log_message_out", FALSE);
	chatApi.hIcons[ICON_NICK] = LoadIconEx("log_nick", FALSE);
	chatApi.hIcons[ICON_NOTICE] = LoadIconEx("log_notice", FALSE);
	chatApi.hIcons[ICON_PART] = LoadIconEx("log_part", FALSE);
	chatApi.hIcons[ICON_QUIT] = LoadIconEx("log_quit", FALSE);
	chatApi.hIcons[ICON_REMSTATUS] = LoadIconEx("log_removestatus", FALSE);
	chatApi.hIcons[ICON_TOPIC] = LoadIconEx("log_topic", FALSE);
	chatApi.hIcons[ICON_STATUS0] = LoadIconEx("status0", FALSE);
	chatApi.hIcons[ICON_STATUS1] = LoadIconEx("status1", FALSE);
	chatApi.hIcons[ICON_STATUS2] = LoadIconEx("status2", FALSE);
	chatApi.hIcons[ICON_STATUS3] = LoadIconEx("status3", FALSE);
	chatApi.hIcons[ICON_STATUS4] = LoadIconEx("status4", FALSE);
	chatApi.hIcons[ICON_STATUS5] = LoadIconEx("status5", FALSE);

	FreeMsgLogBitmaps();
	LoadMsgLogBitmaps();
}

static int FontsChanged(WPARAM, LPARAM)
{
	LoadGlobalSettings();
	LoadLogFonts();

	FreeMsgLogBitmaps();
	LoadMsgLogBitmaps();

	SetIndentSize();
	g_Settings->bLogIndentEnabled = (db_get_b(0, CHAT_MODULE, "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

	chatApi.MM_FontsChanged();
	Chat_UpdateOptions();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogBitmaps();
	LoadMsgLogBitmaps();

	chatApi.MM_IconsChanged();
	Chat_UpdateOptions();
	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	if (g_Settings != nullptr) {
		SM_RemoveAll();
		chatApi.MM_RemoveAll();

		DeleteObject(chatApi.hListBkgBrush);
		DeleteObject(chatApi.hListSelectedBkgBrush);
	}
	return 0;
}

static int SmileyOptionsChanged(WPARAM, LPARAM)
{
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if (si->pDlg)
			si->pDlg->RedrawLog();
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieveing chat info

EXTERN_C MIR_APP_DLL(int) Chat_GetInfo(GC_INFO *gci)
{
	if (!gci || !gci->pszModule)
		return 1;

	mir_cslock lck(csChat);

	SESSION_INFO *si;
	if (gci->Flags & GCF_BYINDEX)
		si = chatApi.SM_FindSessionByIndex(gci->pszModule, gci->iItem);
	else
		si = chatApi.SM_FindSession(gci->pszID, gci->pszModule);
	if (si == nullptr)
		return 1;

	if (gci->Flags & GCF_DATA)     gci->pItemData = si->pItemData;
	if (gci->Flags & GCF_HCONTACT) gci->hContact = si->hContact;
	if (gci->Flags & GCF_TYPE)     gci->iType = si->iType;
	if (gci->Flags & GCF_COUNT)    gci->iCount = si->nUsersInNicklist;
	if (gci->Flags & GCF_USERS)    gci->pszUsers = SM_GetUsers(si);
	if (gci->Flags & GCF_ID)       gci->pszID = si->ptszID;
	if (gci->Flags & GCF_NAME)     gci->pszName = si->ptszName;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// registers protocol as chat provider

MIR_APP_DLL(int) Chat_Register(const GCREGISTER *gcr)
{
	if (gcr == nullptr)
		return GC_ERROR;

	mir_cslock lck(csChat);
	MODULEINFO *mi = chatApi.MM_AddModule(gcr->pszModule);
	if (mi == nullptr)
		return GC_ERROR;

	mi->ptszModDispName = mir_wstrdup(gcr->ptszDispName);
	mi->bBold = (gcr->dwFlags & GC_BOLD) != 0;
	mi->bUnderline = (gcr->dwFlags & GC_UNDERLINE) != 0;
	mi->bItalics = (gcr->dwFlags & GC_ITALICS) != 0;
	mi->bColor = (gcr->dwFlags & GC_COLOR) != 0;
	mi->bBkgColor = (gcr->dwFlags & GC_BKGCOLOR) != 0;
	mi->bAckMsg = (gcr->dwFlags & GC_ACKMSG) != 0;
	mi->bChanMgr = (gcr->dwFlags & GC_CHANMGR) != 0;
	mi->iMaxText = gcr->iMaxText;
	mi->pszHeader = chatApi.Log_CreateRtfHeader();

	chatApi.SetAllOffline(TRUE, gcr->pszModule);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// starts new chat session

EXTERN_C MIR_APP_DLL(GCSessionInfoBase*) Chat_NewSession(
	int            iType,      // Use one of the GCW_* flags above to set the type of session
	const char    *pszModule,  // The name of the protocol owning the session (the same as pszModule when you register)
	const wchar_t *ptszID,     // The unique identifier for the session.
	const wchar_t *ptszName,   // The name of the session as it will be displayed to the user
	void          *pItemData)  // Set user defined data for this session. Retrieve it by using the Chat_GetUserInfo() call
{
	mir_cslockfull lck(csChat);
	MODULEINFO *mi = chatApi.MM_FindModule(pszModule);
	if (mi == nullptr)
		return nullptr;

	// try to restart a session first
	SESSION_INFO *si = chatApi.SM_FindSession(ptszID, pszModule);
	if (si != nullptr) {
		chatApi.UM_RemoveAll(&si->pUsers);
		chatApi.TM_RemoveAll(&si->pStatuses);
		lck.unlock();

		si->iStatusCount = 0;
		si->nUsersInNicklist = 0;
		si->pMe = nullptr;

		if (chatApi.OnReplaceSession)
			chatApi.OnReplaceSession(si);
		return si;
	}

	// create a new session
	si = (SESSION_INFO*)mir_calloc(g_cbSession);
	si->ptszID = mir_wstrdup(ptszID);
	si->pszModule = mir_strdup(pszModule);

	chatApi.arSessions.insert(si);
	lck.unlock();

	// set the defaults
	si->pItemData = pItemData;
	if (iType != GCW_SERVER)
		si->wStatus = ID_STATUS_ONLINE;
	si->iType = iType;
	si->ptszName = mir_wstrdup(ptszName);
	si->currentHovered = -1;

	wchar_t szTemp[256];
	if (si->iType == GCW_SERVER)
		mir_snwprintf(szTemp, L"Server: %s", si->ptszName);
	else
		wcsncpy_s(szTemp, si->ptszName, _TRUNCATE);
	si->hContact = AddRoom(pszModule, ptszID, szTemp, si->iType);
	db_set_s(si->hContact, si->pszModule, "Topic", "");
	db_unset(si->hContact, "CList", "StatusMsg");
	if (si->ptszStatusbarText)
		db_set_ws(si->hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
	else
		db_set_s(si->hContact, si->pszModule, "StatusBar", "");

	if (chatApi.OnCreateSession)
		chatApi.OnCreateSession(si, mi);
	return si;
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat control

struct ChatConrolParam
{
	const char *szModule;
	const wchar_t *wszId;
	int command;
};

static void SetInitDone(SESSION_INFO *si)
{
	if (si->bInitDone)
		return;

	si->bInitDone = true;
	for (STATUSINFO *p = si->pStatuses; p; p = p->next)
		if ((UINT_PTR)p->hIcon < STATUSICONCOUNT)
			p->hIcon = HICON(si->iStatusCount - (INT_PTR)p->hIcon - 1);
}

static INT_PTR __stdcall stubRoomControl(void *param)
{
	ChatConrolParam *p = (ChatConrolParam*)param;

	mir_cslock lck(csChat);
	SESSION_INFO *si = nullptr;
	if (p->szModule)
		si = chatApi.SM_FindSession(p->wszId, p->szModule);

	switch (p->command) {
	case WINDOW_HIDDEN:
		if (si == nullptr)
			return GC_EVENT_ERROR;

		SetInitDone(si);
		chatApi.SetActiveSession(si);
		break;

	case WINDOW_VISIBLE:
	case SESSION_INITDONE:
		if (si == nullptr)
			return GC_EVENT_ERROR;

		SetInitDone(si);
		if (p->command != SESSION_INITDONE || db_get_b(0, CHAT_MODULE, "PopupOnJoin", 0) == 0)
			chatApi.ShowRoom(si);
		break;

	case SESSION_OFFLINE:
		if (si == nullptr && p->wszId != nullptr)
			return GC_EVENT_ERROR;

		SM_SetOffline(p->szModule, si);
		SM_SetStatus(p->szModule, si, ID_STATUS_OFFLINE);
		if (si && si->pDlg) {
			si->pDlg->UpdateStatusBar();
			si->pDlg->UpdateNickList();
		}
		break;

	case SESSION_ONLINE:
		if (si == nullptr && p->wszId != nullptr)
			return GC_EVENT_ERROR;

		SM_SetStatus(p->szModule, si, ID_STATUS_ONLINE);
		if (si && si->pDlg)
			si->pDlg->UpdateStatusBar();
		break;

	case WINDOW_CLEARLOG:
		if (si == nullptr)
			return GC_EVENT_ERROR;

		chatApi.LM_RemoveAll(&si->pLog, &si->pLogEnd);
		si->iEventCount = 0;
		si->LastTime = 0;
		if (si->pDlg)
			si->pDlg->ClearLog();
		break;

	default:
		return GC_EVENT_ERROR;
	}

	return 0;
}

MIR_APP_DLL(int) Chat_Control(const char *szModule, const wchar_t *wszId, int iCommand)
{
	ChatConrolParam param = { szModule, wszId, iCommand };
	return CallFunctionSync(stubRoomControl, &param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat termination

struct ChatTerminateParam
{
	const char *szModule;
	const wchar_t *wszId;
	bool bRemoveContact;
};

static INT_PTR __stdcall stubRoomTerminate(void *param)
{
	ChatTerminateParam *p = (ChatTerminateParam*)param;
	return SM_RemoveSession(p->wszId, p->szModule, p->bRemoveContact);
}

MIR_APP_DLL(int) Chat_Terminate(const char *szModule, const wchar_t *wszId, bool bRemoveContact)
{
	ChatTerminateParam param = { szModule, wszId, bRemoveContact };
	return CallFunctionSync(stubRoomTerminate, &param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// handles chat event

static void AddUser(GCEVENT *gce)
{
	SESSION_INFO *si = chatApi.SM_FindSession(gce->ptszID, gce->pszModule);
	if (si == nullptr)
		return;

	WORD status = chatApi.TM_StringToWord(si->pStatuses, gce->ptszStatus);

	USERINFO *ui = chatApi.UM_AddUser(si->pStatuses, &si->pUsers, gce->ptszUID, gce->ptszNick, status);
	if (ui == nullptr)
		return;

	si->nUsersInNicklist++;
	if (chatApi.OnAddUser)
		chatApi.OnAddUser(si, ui);

	ui->pszNick = mir_wstrdup(gce->ptszNick);
	if (gce->bIsMe)
		si->pMe = ui;
	ui->Status = status;
	ui->Status |= si->pStatuses->Status;

	if (si->pDlg)
		si->pDlg->UpdateNickList();

	if (chatApi.OnNewUser)
		chatApi.OnNewUser(si, ui);
}

static BOOL AddEventToAllMatchingUID(GCEVENT *gce)
{
	int bManyFix = 0;

	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if (!si->bInitDone || mir_strcmpi(si->pszModule, gce->pszModule))
			continue;

		if (!chatApi.UM_FindUser(si->pUsers, gce->ptszUID))
			continue;

		if (chatApi.OnEventBroadcast)
			chatApi.OnEventBroadcast(si, gce);

		if (si->pDlg && si->bInitDone) {
			if (SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE))
				si->pDlg->AddLog();
			else
				si->pDlg->RedrawLog2();
		}

		if (!(gce->dwFlags & GCEF_NOTNOTIFY))
			chatApi.DoSoundsFlashPopupTrayStuff(si, gce, FALSE, bManyFix);

		bManyFix++;
		if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
			chatApi.LogToFile(si, gce);
	}

	return 0;
}

static INT_PTR CALLBACK sttEventStub(void *_param)
{
	GCEVENT *gce = (GCEVENT*)_param;
	if (NotifyEventHooks(hHookEvent, 0, LPARAM(gce)))
		return 1;

	bool bIsHighlighted = false, bRemoveFlag = false;

	// Do different things according to type of event
	switch (gce->iType) {
	case GC_EVENT_SETCONTACTSTATUS:
		return SM_SetContactStatus(gce->ptszID, gce->pszModule, gce->ptszUID, (WORD)gce->dwItemData);

	case GC_EVENT_TOPIC:
		if (SESSION_INFO *si = chatApi.SM_FindSession(gce->ptszID, gce->pszModule)) {
			wchar_t *pwszNew = RemoveFormatting(gce->ptszText);
			if (!mir_wstrcmp(si->ptszTopic, pwszNew)) // nothing changed? exiting
				return 0;

			replaceStrW(si->ptszTopic, pwszNew);
			if (pwszNew != nullptr)
				db_set_ws(si->hContact, si->pszModule, "Topic", si->ptszTopic);
			else
				db_unset(si->hContact, si->pszModule, "Topic");

			if (chatApi.OnSetTopic)
				chatApi.OnSetTopic(si);

			if (db_get_b(0, CHAT_MODULE, "TopicOnClist", 0)) {
				if (pwszNew != nullptr)
					db_set_ws(si->hContact, "CList", "StatusMsg", si->ptszTopic);
				else
					db_unset(si->hContact, "CList", "StatusMsg");
			}
		}
		break;

	case GC_EVENT_ADDSTATUS:
		SM_GiveStatus(gce->ptszID, gce->pszModule, gce->ptszUID, gce->ptszStatus);
		bIsHighlighted = chatApi.IsHighlighted(nullptr, gce);
		break;

	case GC_EVENT_REMOVESTATUS:
		SM_TakeStatus(gce->ptszID, gce->pszModule, gce->ptszUID, gce->ptszStatus);
		bIsHighlighted = chatApi.IsHighlighted(nullptr, gce);
		break;

	case GC_EVENT_MESSAGE:
	case GC_EVENT_ACTION:
		if (!gce->bIsMe && gce->ptszID && gce->ptszText) {
			SESSION_INFO *si = chatApi.SM_FindSession(gce->ptszID, gce->pszModule);
			bIsHighlighted = chatApi.IsHighlighted(si, gce);
		}
		break;

	case GC_EVENT_NICK:
		SM_ChangeNick(gce->ptszID, gce->pszModule, gce);
		bIsHighlighted = chatApi.IsHighlighted(nullptr, gce);
		break;

	case GC_EVENT_JOIN:
		AddUser(gce);
		bIsHighlighted = chatApi.IsHighlighted(nullptr, gce);
		break;

	case GC_EVENT_PART:
	case GC_EVENT_QUIT:
	case GC_EVENT_KICK:
		bRemoveFlag = TRUE;
		bIsHighlighted = chatApi.IsHighlighted(nullptr, gce);
		break;
	}

	// Decide which window (log) should have the event
	LPCTSTR pWnd = nullptr;
	LPCSTR pMod = nullptr;
	if (gce->ptszID) {
		pWnd = gce->ptszID;
		pMod = gce->pszModule;
	}
	else if (gce->iType == GC_EVENT_NOTICE || gce->iType == GC_EVENT_INFORMATION) {
		SESSION_INFO *si = chatApi.GetActiveSession();
		if (si && !mir_strcmp(si->pszModule, gce->pszModule)) {
			pWnd = si->ptszID;
			pMod = si->pszModule;
		}
		else return 0;
	}
	else {
		// Send the event to all windows with a user pszUID. Used for broadcasting QUIT etc
		AddEventToAllMatchingUID(gce);
		if (!bRemoveFlag)
			return 0;
	}

	// add to log
	if (pWnd) {
		if (gce->dwFlags & GCEF_SILENT)
			return 0;

		SESSION_INFO *si = chatApi.SM_FindSession(pWnd, pMod);

		// fix for IRC's old style mode notifications. Should not affect any other protocol
		if ((gce->iType == GC_EVENT_ADDSTATUS || gce->iType == GC_EVENT_REMOVESTATUS) && !(gce->dwFlags & GCEF_ADDTOLOG))
			return 0;

		if (gce->iType == GC_EVENT_JOIN && gce->time == 0)
			return 0;

		if (si && (si->bInitDone || gce->iType == GC_EVENT_TOPIC || (gce->iType == GC_EVENT_JOIN && gce->bIsMe))) {
			if (gce->ptszNick == nullptr && gce->ptszUID != nullptr) {
				USERINFO *ui = chatApi.UM_FindUser(si->pUsers, gce->ptszUID);
				if (ui != nullptr)
					gce->ptszNick = ui->pszNick;
			}

			int isOk = SM_AddEvent(pWnd, pMod, gce, bIsHighlighted);
			if (si->pDlg) {
				if (isOk)
					si->pDlg->AddLog();
				else
					si->pDlg->RedrawLog2();
			}

			if (!(gce->dwFlags & GCEF_NOTNOTIFY))
				chatApi.DoSoundsFlashPopupTrayStuff(si, gce, bIsHighlighted, 0);

			if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
				chatApi.LogToFile(si, gce);
		}

		if (!bRemoveFlag)
			return 0;
	}

	if (bRemoveFlag)
		return SM_RemoveUser(gce->ptszID, gce->pszModule, gce->ptszUID) == 0;

	return GC_EVENT_ERROR;
}

EXTERN_C MIR_APP_DLL(int) Chat_Event(GCEVENT *gce)
{
	if (gce == nullptr)
		return GC_EVENT_ERROR;

	if (!IsEventSupported(gce->iType))
		return GC_EVENT_ERROR;

	return CallFunctionSync(sttEventStub, gce);
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat control functions

MIR_APP_DLL(int) Chat_AddGroup(const char *szModule, const wchar_t *wszId, const wchar_t *wszText, HICON hIcon)
{
	if (wszText == nullptr)
		return GC_EVENT_ERROR;

	mir_cslock lck(csChat);
	SESSION_INFO *si = SM_FindSession(wszId, szModule);
	if (si == nullptr)
		return 0;

	STATUSINFO *ti = chatApi.TM_AddStatus(&si->pStatuses, wszText, &si->iStatusCount);
	if (ti) {
		si->iStatusCount++;
		if (hIcon)
			ti->hIcon = CopyIcon(hIcon);
	}

	if (chatApi.OnAddStatus)
		chatApi.OnAddStatus(si, ti);
	return 0;
}

MIR_APP_DLL(int) Chat_ChangeSessionName(const char *szModule, const wchar_t *wszId, const wchar_t *wszNewName)
{
	if (wszNewName == nullptr)
		return GC_EVENT_ERROR;

	SESSION_INFO *si;
	{
		mir_cslock lck(csChat);
		si = chatApi.SM_FindSession(wszId, szModule);
	}
	if (si != nullptr) {
		// nothing really changed? exiting
		if (!mir_wstrcmp(si->ptszName, wszNewName))
			return 0;

		replaceStrW(si->ptszName, wszNewName);
		db_set_ws(si->hContact, szModule, "Nick", wszNewName);
		if (si->pDlg)
			si->pDlg->UpdateTitle();
	}
	return 0;
}

MIR_APP_DLL(int) Chat_ChangeUserId(const char *szModule, const wchar_t *wszId, const wchar_t *wszOldId, const wchar_t *wszNewId)
{
	if (szModule == nullptr || wszNewId == nullptr)
		return GC_EVENT_ERROR;
	
	mir_cslock lck(csChat);
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		USERINFO *ui = chatApi.UM_FindUser(si->pUsers, wszOldId);
		if (ui)
			replaceStrW(ui->pszUID, wszNewId);
		if (wszId)
			break;
	}
	return 0;
}

MIR_APP_DLL(void*) Chat_GetUserInfo(const char *szModule, const wchar_t *wszId)
{
	mir_cslock lck(csChat);
	if (SESSION_INFO *si = chatApi.SM_FindSession(wszId, szModule))
		return si->pItemData;
	return nullptr;
}

MIR_APP_DLL(int) Chat_SendUserMessage(const char *szModule, const wchar_t *wszId, const wchar_t *wszText)
{
	if (wszText == nullptr || szModule == nullptr)
		return GC_EVENT_ERROR;
	
	mir_cslock lck(csChat);
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		if (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS)
			Chat_DoEventHook(si, GC_USER_MESSAGE, nullptr, wszText, 0);
		if (wszId)
			break;
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetStatusbarText(const char *szModule, const wchar_t *wszId, const wchar_t *wszText)
{
	SESSION_INFO *si;
	{
		mir_cslock lck(csChat);
		si = chatApi.SM_FindSession(wszId, szModule);
	}
	if (si != nullptr) {
		replaceStrW(si->ptszStatusbarText, wszText);
		if (si->ptszStatusbarText)
			db_set_ws(si->hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
		else
			db_set_s(si->hContact, si->pszModule, "StatusBar", "");

		if (si->pDlg)
			si->pDlg->UpdateStatusBar();
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetStatusEx(const char *szModule, const wchar_t *wszId, int flags, const wchar_t *wszText)
{
	if (!szModule)
		return GC_EVENT_ERROR;

	mir_cslock lck(csChat);
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		chatApi.UM_SetStatusEx(si->pUsers, wszText, flags);
		if (si->pDlg)
			RedrawWindow(GetDlgItem(si->pDlg->GetHwnd(), IDC_LIST), nullptr, nullptr, RDW_INVALIDATE);
		if (wszId)
			break;
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetUserInfo(const char *szModule, const wchar_t *wszId, void *pItemData)
{
	mir_cslock lck(csChat);
	if (SESSION_INFO *si = chatApi.SM_FindSession(wszId, szModule)) {
		si->pItemData = pItemData;
		return 0;
	}
	return GC_EVENT_ERROR;
}

EXTERN_C MIR_APP_DLL(void) Chat_UpdateOptions()
{
	for (int i = 0; i < g_arSessions.getCount(); i++) {
		SESSION_INFO *si = g_arSessions[i];
		if (si->pDlg)
			si->pDlg->UpdateOptions();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// module initialization

static int ModulesLoaded(WPARAM, LPARAM)
{
	LoadChatIcons();

	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, SmileyOptionsChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CMenuItem mi;
	SET_UID(mi, 0x2bb76d5, 0x740d, 0x4fd2, 0x8f, 0xee, 0x7c, 0xa4, 0x5a, 0x74, 0x65, 0xa6);
	mi.position = -2000090001;
	mi.flags = CMIF_DEFAULT;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	mi.name.a = LPGEN("&Join chat");
	mi.pszService = "GChat/JoinChat";
	hJoinMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JoinChat);

	SET_UID(mi, 0x72b7440b, 0xd2db, 0x4e22, 0xa6, 0xb1, 0x2, 0xd0, 0x96, 0xee, 0xad, 0x88);
	mi.position = -2000090000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_LEAVE);
	mi.flags = CMIF_NOTOFFLINE;
	mi.name.a = LPGEN("&Leave chat");
	mi.pszService = "GChat/LeaveChat";
	hLeaveMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, LeaveChat);

	chatApi.SetAllOffline(TRUE, nullptr);
	return 0;
}

static bool bInited = false;

int LoadChatModule(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);

	g_hWindowList = WindowList_Create();
	hHookEvent = CreateHookableEvent(ME_GC_HOOK_EVENT);
	hevSendEvent = CreateHookableEvent(ME_GC_EVENT);
	hevBuildMenuEvent = CreateHookableEvent(ME_GC_BUILDMENU);
	
	chatApi.hevPreCreate = CreateHookableEvent(ME_MSG_PRECREATEEVENT);
	chatApi.hevWinPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);

	HookEvent(ME_FONT_RELOAD, FontsChanged);
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_SRMM));
	TranslateMenu(g_hMenu);

	bInited = true;
	return 0;
}

void UnloadChatModule(void)
{
	if (!bInited)
		return;

	mir_free(chatApi.szActiveWndID);
	mir_free(chatApi.szActiveWndModule);

	FreeMsgLogBitmaps();
	OptionsUnInit();

	WindowList_Destroy(g_hWindowList);

	DestroyHookableEvent(hevSendEvent);
	DestroyHookableEvent(hevBuildMenuEvent);
	DestroyHookableEvent(hHookEvent);

	DestroyHookableEvent(chatApi.hevPreCreate);
	DestroyHookableEvent(chatApi.hevWinPopup);

	DestroyMenu(g_hMenu);
}
