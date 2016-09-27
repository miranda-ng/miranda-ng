/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
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

HGENMENU hJoinMenuItem, hLeaveMenuItem;
mir_cs csChat;

static HANDLE
   hServiceRegister = NULL,
   hServiceNewChat = NULL,
   hServiceAddEvent = NULL,
   hServiceGetAddEventPtr = NULL,
   hServiceGetInfo = NULL,
   hServiceGetCount = NULL,
   hEventPrebuildMenu = NULL,
   hEventDoubleclicked = NULL,
   hEventJoinChat = NULL,
   hEventLeaveChat = NULL,
   hHookEvent = NULL;

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
	g_Settings->bLogIndentEnabled = (db_get_b(NULL, CHAT_MODULE, "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

	chatApi.MM_FontsChanged();
	chatApi.MM_FixColors();
	chatApi.SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogBitmaps();
	LoadMsgLogBitmaps();

	chatApi.MM_IconsChanged();
	chatApi.SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, FALSE);
	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	if (g_Settings != NULL) {
		chatApi.SM_BroadcastMessage(NULL, GC_CLOSEWINDOW, 0, 1, FALSE);

		SM_RemoveAll();
		chatApi.MM_RemoveAll();

		DeleteObject(chatApi.hListBkgBrush);
		DeleteObject(chatApi.hListSelectedBkgBrush);
	}
	return 0;
}

static int SmileyOptionsChanged(WPARAM, LPARAM)
{
	chatApi.SM_BroadcastMessage(NULL, GC_REDRAWLOG, 0, 1, FALSE);
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
	if (si == NULL)
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
	if (gcr == NULL)
		return GC_ERROR;

	mir_cslock lck(csChat);
	MODULEINFO *mi = chatApi.MM_AddModule(gcr->pszModule);
	if (mi == NULL)
		return GC_ERROR;

	mi->ptszModDispName = mir_wstrdup(gcr->ptszDispName);
	mi->bBold = (gcr->dwFlags & GC_BOLD) != 0;
	mi->bUnderline = (gcr->dwFlags & GC_UNDERLINE) != 0;
	mi->bItalics = (gcr->dwFlags & GC_ITALICS) != 0;
	mi->bColor = (gcr->dwFlags & GC_COLOR) != 0;
	mi->bBkgColor = (gcr->dwFlags & GC_BKGCOLOR) != 0;
	mi->bAckMsg = (gcr->dwFlags & GC_ACKMSG) != 0;
	mi->bChanMgr = (gcr->dwFlags & GC_CHANMGR) != 0;
	mi->bSingleFormat = (gcr->dwFlags & GC_SINGLEFORMAT) != 0;
	mi->bFontSize = (gcr->dwFlags & GC_FONTSIZE) != 0;
	mi->iMaxText = gcr->iMaxText;
	mi->nColorCount = gcr->nColors;
	if (gcr->nColors > 0) {
		mi->crColors = (COLORREF *)mir_alloc(sizeof(COLORREF)* gcr->nColors);
		memcpy(mi->crColors, gcr->pColors, sizeof(COLORREF)* gcr->nColors);
	}

	mi->pszHeader = chatApi.Log_CreateRtfHeader(mi);

	CheckColorsInModule((char*)gcr->pszModule);
	chatApi.SetAllOffline(TRUE, gcr->pszModule);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// starts new chat session

EXTERN_C MIR_APP_DLL(int) Chat_NewSession(
	int            iType,      // Use one of the GCW_* flags above to set the type of session
	const char    *pszModule,  // The name of the protocol owning the session (the same as pszModule when you register)
	const wchar_t *ptszID,     // The unique identifier for the session.
	const wchar_t *ptszName,   // The name of the session as it will be displayed to the user
	void          *pItemData)  // Set user defined data for this session. Retrieve it by using the Chat_GetUserInfo() call
{
	mir_cslockfull lck(csChat);
	MODULEINFO *mi = chatApi.MM_FindModule(pszModule);
	if (mi == NULL)
		return GC_ERROR;

	// try to restart a session first
	SESSION_INFO *si = chatApi.SM_FindSession(ptszID, pszModule);
	if (si != NULL) {
		chatApi.UM_RemoveAll(&si->pUsers);
		chatApi.TM_RemoveAll(&si->pStatuses);
		lck.unlock();

		si->iStatusCount = 0;
		si->nUsersInNicklist = 0;
		si->pMe = NULL;

		if (chatApi.OnReplaceSession)
			chatApi.OnReplaceSession(si);
		return 0;
	}

	// create a new session
	si = (SESSION_INFO*)mir_calloc(g_cbSession);
	si->ptszID = mir_wstrdup(ptszID);
	si->pszModule = mir_strdup(pszModule);

	if (chatApi.wndList == NULL) // list is empty
		chatApi.wndList = si;
	else {
		si->next = chatApi.wndList;
		chatApi.wndList = si;
	}
	lck.unlock();

	// set the defaults
	si->pItemData = pItemData;
	if (iType != GCW_SERVER)
		si->wStatus = ID_STATUS_ONLINE;
	si->iType = iType;
	si->ptszName = mir_wstrdup(ptszName);
	si->iSplitterX = g_Settings->iSplitterX;
	si->iSplitterY = g_Settings->iSplitterY;
	si->iLogFilterFlags = db_get_dw(NULL, CHAT_MODULE, "FilterFlags", 0x03E0);
	si->bFilterEnabled = db_get_b(NULL, CHAT_MODULE, "FilterEnabled", 0);
	si->bNicklistEnabled = db_get_b(NULL, CHAT_MODULE, "ShowNicklist", 1);

	if (mi->bColor) {
		si->iFG = 4;
		si->bFGSet = TRUE;
	}
	if (mi->bBkgColor) {
		si->iBG = 2;
		si->bBGSet = TRUE;
	}

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
	return 0;
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
	switch (p->command) {
	case WINDOW_HIDDEN:
		if (SESSION_INFO *si = chatApi.SM_FindSession(p->wszId, p->szModule)) {
			SetInitDone(si);
			chatApi.SetActiveSession(si->ptszID, si->pszModule);
			if (si->hWnd)
				chatApi.ShowRoom(si, p->command, FALSE);
		}
		return 0;

	case WINDOW_MINIMIZE:
	case WINDOW_MAXIMIZE:
	case WINDOW_VISIBLE:
	case SESSION_INITDONE:
		if (SESSION_INFO *si = chatApi.SM_FindSession(p->wszId, p->szModule)) {
			SetInitDone(si);
			if (p->command != SESSION_INITDONE || db_get_b(NULL, CHAT_MODULE, "PopupOnJoin", 0) == 0)
				chatApi.ShowRoom(si, p->command, TRUE);
			return 0;
		}
		break;

	case SESSION_OFFLINE:
		SM_SetOffline(p->wszId, p->szModule);
		// fall through

	case SESSION_ONLINE:
		SM_SetStatus(p->wszId, p->szModule, p->command == SESSION_ONLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
		break;

	case SESSION_TERMINATE:
		return SM_RemoveSession(p->wszId, p->szModule, false);

	case WINDOW_CLEARLOG:
		if (SESSION_INFO *si = chatApi.SM_FindSession(p->wszId, p->szModule)) {
			chatApi.LM_RemoveAll(&si->pLog, &si->pLogEnd);
			if (chatApi.OnClearLog)
				chatApi.OnClearLog(si);
			si->iEventCount = 0;
			si->LastTime = 0;
		}
		break;

	default:
		return GC_EVENT_ERROR;
	}

	SM_SendMessage(p->wszId, p->szModule, GC_CONTROL_MSG, p->command, 0);
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

struct DoFlashParam
{
	SESSION_INFO *si;
	GCEVENT *gce;
	int i1, i2;
};

static INT_PTR __stdcall stubFlash(void *param)
{
	DoFlashParam *p = (DoFlashParam*)param;
	return chatApi.DoSoundsFlashPopupTrayStuff(p->si, p->gce, p->i1, p->i2);
}

static void AddUser(GCEVENT *gce)
{
	SESSION_INFO *si = chatApi.SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
	if (si == NULL)
		return;

	WORD status = chatApi.TM_StringToWord(si->pStatuses, gce->ptszStatus);

	USERINFO *ui = chatApi.UM_AddUser(si->pStatuses, &si->pUsers, gce->ptszUID, gce->ptszNick, status);
	if (ui == NULL)
		return;

	si->nUsersInNicklist++;
	if (chatApi.OnAddUser)
		chatApi.OnAddUser(si, ui);

	ui->pszNick = mir_wstrdup(gce->ptszNick);
	if (gce->bIsMe)
		si->pMe = ui;
	ui->Status = status;
	ui->Status |= si->pStatuses->Status;

	if (chatApi.OnNewUser)
		chatApi.OnNewUser(si, ui);
}

static BOOL AddEventToAllMatchingUID(GCEVENT *gce)
{
	int bManyFix = 0;

	for (SESSION_INFO *p = chatApi.wndList; p != NULL; p = p->next) {
		if (!p->bInitDone || mir_strcmpi(p->pszModule, gce->pDest->pszModule))
			continue;

		if (!chatApi.UM_FindUser(p->pUsers, gce->ptszUID))
			continue;

		if (chatApi.OnEventBroadcast)
			chatApi.OnEventBroadcast(p, gce);

		if (!(gce->dwFlags & GCEF_NOTNOTIFY)) {
			DoFlashParam param = { p, gce, FALSE, bManyFix };
			CallFunctionSync(stubFlash, &param);
		}

		bManyFix++;
		if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
			chatApi.LogToFile(p, gce);
	}

	return 0;
}

EXTERN_C MIR_APP_DLL(int) Chat_Event(GCEVENT *gce)
{
	if (gce == NULL)
		return GC_EVENT_ERROR;

	GCDEST *gcd = gce->pDest;
	if (gcd == NULL)
		return GC_EVENT_ERROR;

	if (!IsEventSupported(gcd->iType))
		return GC_EVENT_ERROR;

	if (NotifyEventHooks(hHookEvent, 0, LPARAM(gce)))
		return 1;

	BOOL bIsHighlighted = FALSE;
	BOOL bRemoveFlag = FALSE;

	// Do different things according to type of event
	switch (gcd->iType) {
	case GC_EVENT_SETCONTACTSTATUS:
		return SM_SetContactStatus(gcd->ptszID, gcd->pszModule, gce->ptszUID, (WORD)gce->dwItemData);

	case GC_EVENT_TOPIC:
		if (SESSION_INFO *si = chatApi.SM_FindSession(gcd->ptszID, gcd->pszModule)) {
			if (gce->ptszText) {
				replaceStrW(si->ptszTopic, RemoveFormatting(gce->ptszText));
				db_set_ws(si->hContact, si->pszModule, "Topic", si->ptszTopic);
				if (chatApi.OnSetTopic)
					chatApi.OnSetTopic(si);
				if (db_get_b(NULL, CHAT_MODULE, "TopicOnClist", 0))
					db_set_ws(si->hContact, "CList", "StatusMsg", si->ptszTopic);
			}
		}
		break;

	case GC_EVENT_ADDSTATUS:
		SM_GiveStatus(gcd->ptszID, gcd->pszModule, gce->ptszUID, gce->ptszStatus);
		bIsHighlighted = chatApi.IsHighlighted(NULL, gce);
		break;

	case GC_EVENT_REMOVESTATUS:
		SM_TakeStatus(gcd->ptszID, gcd->pszModule, gce->ptszUID, gce->ptszStatus);
		bIsHighlighted = chatApi.IsHighlighted(NULL, gce);
		break;

	case GC_EVENT_MESSAGE:
	case GC_EVENT_ACTION:
		if (!gce->bIsMe && gcd->ptszID && gce->ptszText) {
			SESSION_INFO *si = chatApi.SM_FindSession(gcd->ptszID, gcd->pszModule);
			bIsHighlighted = chatApi.IsHighlighted(si, gce);
		}
		break;

	case GC_EVENT_NICK:
		SM_ChangeNick(gcd->ptszID, gcd->pszModule, gce);
		bIsHighlighted = chatApi.IsHighlighted(NULL, gce);
		break;

	case GC_EVENT_JOIN:
		AddUser(gce);
		bIsHighlighted = chatApi.IsHighlighted(NULL, gce);
		break;

	case GC_EVENT_PART:
	case GC_EVENT_QUIT:
	case GC_EVENT_KICK:
		bRemoveFlag = TRUE;
		bIsHighlighted = chatApi.IsHighlighted(NULL, gce);
		break;
	}

	// Decide which window (log) should have the event
	LPCTSTR pWnd = NULL;
	LPCSTR pMod = NULL;
	if (gcd->ptszID) {
		pWnd = gcd->ptszID;
		pMod = gcd->pszModule;
	}
	else if (gcd->iType == GC_EVENT_NOTICE || gcd->iType == GC_EVENT_INFORMATION) {
		SESSION_INFO *si = chatApi.GetActiveSession();
		if (si && !mir_strcmp(si->pszModule, gcd->pszModule)) {
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
		SESSION_INFO *si = chatApi.SM_FindSession(pWnd, pMod);

		// fix for IRC's old style mode notifications. Should not affect any other protocol
		if ((gcd->iType == GC_EVENT_ADDSTATUS || gcd->iType == GC_EVENT_REMOVESTATUS) && !(gce->dwFlags & GCEF_ADDTOLOG))
			return 0;

		if (gcd->iType == GC_EVENT_JOIN && gce->time == 0)
			return 0;

		if (si && (si->bInitDone || gcd->iType == GC_EVENT_TOPIC || (gcd->iType == GC_EVENT_JOIN && gce->bIsMe))) {
			int isOk = chatApi.SM_AddEvent(pWnd, pMod, gce, bIsHighlighted);
			if (chatApi.OnAddLog)
				chatApi.OnAddLog(si, isOk);
			if (!(gce->dwFlags & GCEF_NOTNOTIFY)) {
				DoFlashParam param = { si, gce, bIsHighlighted, 0 };
				CallFunctionSync(stubFlash, &param);
			}
			if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
				chatApi.LogToFile(si, gce);
		}

		if (!bRemoveFlag)
			return 0;
	}

	if (bRemoveFlag)
		return SM_RemoveUser(gcd->ptszID, gcd->pszModule, gce->ptszUID) == 0;

	return GC_EVENT_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat control functions

MIR_APP_DLL(int) Chat_AddGroup(const char *szModule, const wchar_t *wszId, const wchar_t *wszText, HICON hIcon)
{
	if (wszText == NULL)
		return GC_EVENT_ERROR;

	mir_cslock lck(csChat);
	SESSION_INFO *si = SM_FindSession(wszId, szModule);
	if (si == NULL)
		return NULL;

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
	if (wszNewName == NULL)
		return GC_EVENT_ERROR;

	mir_cslock lck(csChat);
	if (SESSION_INFO *si = chatApi.SM_FindSession(wszId, szModule)) {
		replaceStrW(si->ptszName, wszNewName);
		if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATETITLE, 0, 0);
		if (chatApi.OnRenameSession)
			chatApi.OnRenameSession(si);
	}
	return 0;
}

MIR_APP_DLL(int) Chat_ChangeUserId(const char *szModule, const wchar_t *wszId, const wchar_t *wszOldId, const wchar_t *wszNewId)
{
	if (szModule == NULL || wszNewId == NULL)
		return GC_EVENT_ERROR;
	
	mir_cslock lck(csChat);
	for (SESSION_INFO *si = chatApi.wndList; si != NULL; si = si->next) {
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
	return NULL;
}

MIR_APP_DLL(int) Chat_SendUserMessage(const char *szModule, const wchar_t *wszId, const wchar_t *wszText)
{
	if (wszText == NULL || szModule == NULL)
		return GC_EVENT_ERROR;
	
	mir_cslock lck(csChat);
	for (SESSION_INFO *si = chatApi.wndList; si != NULL; si = si->next) {
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		if (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS)
			DoEventHook(si->ptszID, si->pszModule, GC_USER_MESSAGE, NULL, wszText, 0);
		if (wszId)
			break;
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetStatusbarText(const char *szModule, const wchar_t *wszId, const wchar_t *wszText)
{
	mir_cslock lck(csChat);
	if (SESSION_INFO *si = chatApi.SM_FindSession(wszId, szModule)) {
		replaceStrW(si->ptszStatusbarText, wszText);
		if (si->ptszStatusbarText)
			db_set_ws(si->hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
		else
			db_set_s(si->hContact, si->pszModule, "StatusBar", "");

		if (chatApi.OnSetStatusBar)
			chatApi.OnSetStatusBar(si);
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetStatusEx(const char *szModule, const wchar_t *wszId, int flags, const wchar_t *wszText)
{
	if (!szModule)
		return GC_EVENT_ERROR;

	mir_cslock lck(csChat);
	for (SESSION_INFO *si = chatApi.wndList; si != NULL; si = si->next) {
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		chatApi.UM_SetStatusEx(si->pUsers, wszText, flags);
		if (si->hWnd)
			RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
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

	chatApi.SetAllOffline(TRUE, NULL);
	return 0;
}

static bool bInited = false;

int LoadChatModule(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);

	chatApi.hSendEvent = CreateHookableEvent(ME_GC_EVENT);
	chatApi.hBuildMenuEvent = CreateHookableEvent(ME_GC_BUILDMENU);
	hHookEvent = CreateHookableEvent(ME_GC_HOOK_EVENT);

	HookEvent(ME_FONT_RELOAD, FontsChanged);
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);

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

	DestroyHookableEvent(chatApi.hSendEvent);
	DestroyHookableEvent(chatApi.hBuildMenuEvent);
	DestroyHookableEvent(hHookEvent);
}
