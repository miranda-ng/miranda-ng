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

CMOption<bool> Chat::bShowNicklist(CHAT_MODULE, "ShowNicklist", true);
CMOption<bool> Chat::bFilterEnabled(CHAT_MODULE, "FilterEnabled", false);
CMOption<bool> Chat::bTopicOnClist(CHAT_MODULE, "TopicOnClist", false);
CMOption<bool> Chat::bPopupOnJoin(CHAT_MODULE, "PopupOnJoin", false);
CMOption<bool> Chat::bDoubleClick4Privat(CHAT_MODULE, "DoubleClick4Privat", false);
CMOption<bool> Chat::bShowContactStatus(CHAT_MODULE, "ShowContactStatus", true);
CMOption<bool> Chat::bContactStatusFirst(CHAT_MODULE, "ContactStatusFirst", false);

CMOption<bool> Chat::bFlashWindow(CHAT_MODULE, "FlashWindow", false);
CMOption<bool> Chat::bFlashWindowHighlight(CHAT_MODULE, "FlashWindowHighlight", false);

CMOption<bool> Chat::bShowTime(CHAT_MODULE, "ShowTimeStamp", true);
CMOption<bool> Chat::bStripFormat(CHAT_MODULE, "StripFormatting", false);
CMOption<bool> Chat::bLogLimitNames(CHAT_MODULE, "LogLimitNames", true);
CMOption<bool> Chat::bLogIndentEnabled(CHAT_MODULE, "LogIndentEnabled", true);
CMOption<bool> Chat::bShowTimeIfChanged(CHAT_MODULE, "ShowTimeStampIfChanged", false);
CMOption<bool> Chat::bTimeStampEventColour(CHAT_MODULE, "TimeStampEventColour", false);


INT_PTR SvcGetChatManager(WPARAM, LPARAM);

#include "chat.h"
#include "resource.h"

mir_cs csChat;
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

void SrmmModulesLoaded();

/////////////////////////////////////////////////////////////////////////////////////////
// Post-load event hooks

void LoadChatIcons(void)
{
	g_hChatIcons[ICON_ACTION] = LoadIconEx("log_action", FALSE);
	g_hChatIcons[ICON_ADDSTATUS] = LoadIconEx("log_addstatus", FALSE);
	g_hChatIcons[ICON_HIGHLIGHT] = LoadIconEx("log_highlight", FALSE);
	g_hChatIcons[ICON_INFO] = LoadIconEx("log_info", FALSE);
	g_hChatIcons[ICON_JOIN] = LoadIconEx("log_join", FALSE);
	g_hChatIcons[ICON_KICK] = LoadIconEx("log_kick", FALSE);
	g_hChatIcons[ICON_MESSAGE] = LoadIconEx("log_message_in", FALSE);
	g_hChatIcons[ICON_MESSAGEOUT] = LoadIconEx("log_message_out", FALSE);
	g_hChatIcons[ICON_NICK] = LoadIconEx("log_nick", FALSE);
	g_hChatIcons[ICON_NOTICE] = LoadIconEx("log_notice", FALSE);
	g_hChatIcons[ICON_PART] = LoadIconEx("log_part", FALSE);
	g_hChatIcons[ICON_QUIT] = LoadIconEx("log_quit", FALSE);
	g_hChatIcons[ICON_REMSTATUS] = LoadIconEx("log_removestatus", FALSE);
	g_hChatIcons[ICON_TOPIC] = LoadIconEx("log_topic", FALSE);
	
	g_chatApi.hStatusIcons[0] = g_plugin.getIcon(IDI_STATUS0);
	g_chatApi.hStatusIcons[1] = g_plugin.getIcon(IDI_STATUS1);
	g_chatApi.hStatusIcons[2] = g_plugin.getIcon(IDI_STATUS2);
	g_chatApi.hStatusIcons[3] = g_plugin.getIcon(IDI_STATUS3);
	g_chatApi.hStatusIcons[4] = g_plugin.getIcon(IDI_STATUS4);
	g_chatApi.hStatusIcons[5] = g_plugin.getIcon(IDI_STATUS5);

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

	g_chatApi.MM_FontsChanged();
	Chat_UpdateOptions();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	FreeMsgLogBitmaps();
	LoadMsgLogBitmaps();

	g_chatApi.MM_IconsChanged();
	Chat_UpdateOptions();
	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	if (g_Settings != nullptr) {
		SM_RemoveAll();
		g_chatApi.MM_RemoveAll();

		DeleteObject(g_chatApi.hListBkgBrush);
		DeleteObject(g_chatApi.hListSelectedBkgBrush);
	}
	return 0;
}

static int SmileyOptionsChanged(WPARAM, LPARAM)
{
	for (auto &si : g_arSessions)
		if (si->pDlg)
			si->pDlg->RedrawLog();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieveing chat info

EXTERN_C MIR_APP_DLL(int) Chat_GetInfo(GC_INFO *gci)
{
	if (!gci || !gci->pszModule)
		return 1;

	SESSION_INFO *si;
	if (gci->Flags & GCF_BYINDEX)
		si = SM_FindSessionByIndex(gci->pszModule, gci->iItem);
	else
		si = SM_FindSession(gci->pszID, gci->pszModule);
	if (si == nullptr)
		return 1;

	if (gci->Flags & GCF_DATA)     gci->pItemData = si->pItemData;
	if (gci->Flags & GCF_HCONTACT) gci->hContact = si->hContact;
	if (gci->Flags & GCF_TYPE)     gci->iType = si->iType;
	if (gci->Flags & GCF_COUNT)    gci->iCount = si->getUserList().getCount();
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
	MODULEINFO *mi = MM_AddModule(gcr->pszModule);
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
	mi->pszHeader = g_chatApi.Log_CreateRtfHeader();

	g_chatApi.SetAllOffline(TRUE, gcr->pszModule);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// starts new chat session

EXTERN_C MIR_APP_DLL(SESSION_INFO*) Chat_NewSession(
	int            iType,      // Use one of the GCW_* flags above to set the type of session
	const char    *pszModule,  // The name of the protocol owning the session (the same as pszModule when you register)
	const wchar_t *ptszID,     // The unique identifier for the session.
	const wchar_t *ptszName,   // The name of the session as it will be displayed to the user
	void          *pItemData)  // Set user defined data for this session. Retrieve it by using the Chat_GetUserInfo() call
{
	mir_cslockfull lck(csChat);
	MODULEINFO *mi = MM_FindModule(pszModule);
	if (mi == nullptr)
		return nullptr;

	// try to restart a session first
	SESSION_INFO *si = SM_FindSession(ptszID, pszModule);
	if (si != nullptr) {
		UM_RemoveAll(si);
		g_chatApi.TM_RemoveAll(&si->pStatuses);
		lck.unlock();

		si->iStatusCount = 0;
		si->pMe = nullptr;

		if (g_chatApi.OnReplaceSession)
			g_chatApi.OnReplaceSession(si);
		return si;
	}

	// create a new session
	si = g_chatApi.SM_CreateSession();
	si->ptszID = mir_wstrdup(ptszID);
	si->pszModule = mir_strdup(pszModule);
	si->pMI = mi;

	g_chatApi.arSessions.insert(si);
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

	if (g_chatApi.OnCreateSession)
		g_chatApi.OnCreateSession(si, mi);
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
		if (p->iIconIndex < STATUSICONCOUNT)
			p->iIconIndex = si->iStatusCount - p->iIconIndex - 1;
}

static INT_PTR __stdcall stubRoomControl(void *param)
{
	ChatConrolParam *p = (ChatConrolParam*)param;

	mir_cslock lck(csChat);
	SESSION_INFO *si = nullptr;
	if (p->szModule)
		si = SM_FindSession(p->wszId, p->szModule);

	switch (p->command) {
	case WINDOW_HIDDEN:
		if (si == nullptr)
			return GC_EVENT_ERROR;

		SetInitDone(si);
		g_chatApi.SetActiveSession(si);
		break;

	case WINDOW_VISIBLE:
	case SESSION_INITDONE:
		if (si == nullptr)
			return GC_EVENT_ERROR;

		SetInitDone(si);
		if (p->command != SESSION_INITDONE || !Chat::bPopupOnJoin)
			g_chatApi.ShowRoom(si);
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

		g_chatApi.LM_RemoveAll(&si->pLog, &si->pLogEnd);
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
	SESSION_INFO *si = SM_FindSession(gce->pszID.w, gce->pszModule);
	if (si == nullptr)
		return;

	uint16_t status = TM_StringToWord(si->pStatuses, gce->pszStatus.w);

	USERINFO *ui = g_chatApi.UM_AddUser(si, gce->pszUID.w, gce->pszNick.w, status);
	if (ui == nullptr)
		return;

	if (g_chatApi.OnAddUser)
		g_chatApi.OnAddUser(si, ui);

	if (gce->bIsMe)
		si->pMe = ui;
	ui->Status = status;
	ui->Status |= si->pStatuses->iStatus;

	if (si->pDlg)
		si->pDlg->UpdateNickList();

	if (g_chatApi.OnNewUser)
		g_chatApi.OnNewUser(si, ui);
}

static BOOL AddEventToAllMatchingUID(GCEVENT *gce)
{
	int bManyFix = 0;

	for (auto &si : g_arSessions) {
		if (!si->bInitDone || mir_strcmpi(si->pszModule, gce->pszModule))
			continue;

		if (!g_chatApi.UM_FindUser(si, gce->pszUID.w))
			continue;

		if (g_chatApi.OnEventBroadcast)
			g_chatApi.OnEventBroadcast(si, gce);

		if (si->pDlg && si->bInitDone) {
			if (SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE))
				si->pDlg->AddLog();
			else
				RedrawLog2(si);
		}

		if (!(gce->dwFlags & GCEF_NOTNOTIFY))
			g_chatApi.DoSoundsFlashPopupTrayStuff(si, gce, FALSE, bManyFix);

		bManyFix++;
		if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
			LogToFile(si, gce);
	}

	return 0;
}

static INT_PTR CALLBACK sttEventStub(void *_param)
{
	ptrW wszId, wszUid, wszNick, wszText, wszStatus, wszUserInfo;

	GCEVENT gce = *(GCEVENT*)_param;
	if (gce.dwFlags & GCEF_UTF8) {
		gce.pszID.w = (wszId = mir_utf8decodeW(gce.pszID.a));
		gce.pszUID.w = (wszUid = mir_utf8decodeW(gce.pszUID.a));
		gce.pszNick.w = (wszNick = mir_utf8decodeW(gce.pszNick.a));
		gce.pszText.w = (wszText = mir_utf8decodeW(gce.pszText.a));
		gce.pszStatus.w = (wszStatus = mir_utf8decodeW(gce.pszStatus.a));
		gce.pszUserInfo.w = (wszUserInfo = mir_utf8decodeW(gce.pszUserInfo.a));
		gce.dwFlags &= ~GCEF_UTF8;
	}

	if (NotifyEventHooks(hHookEvent, 0, LPARAM(&gce)))
		return 1;

	bool bIsHighlighted = false, bRemoveFlag = false;

	// Do different things according to type of event
	switch (gce.iType) {
	case GC_EVENT_SETCONTACTSTATUS:
		return SM_SetContactStatus(gce.pszID.w, gce.pszModule, gce.pszUID.w, (uint16_t)gce.dwItemData);

	case GC_EVENT_TOPIC:
		if (SESSION_INFO *si = SM_FindSession(gce.pszID.w, gce.pszModule)) {
			wchar_t *pwszNew = RemoveFormatting(gce.pszText.w);
			if (!mir_wstrcmp(si->ptszTopic, pwszNew)) // nothing changed? exiting
				return 0;

			replaceStrW(si->ptszTopic, pwszNew);
			if (pwszNew != nullptr)
				db_set_ws(si->hContact, si->pszModule, "Topic", si->ptszTopic);
			else
				db_unset(si->hContact, si->pszModule, "Topic");

			if (g_chatApi.OnSetTopic)
				g_chatApi.OnSetTopic(si);

			if (Chat::bTopicOnClist) {
				if (pwszNew != nullptr)
					db_set_ws(si->hContact, "CList", "StatusMsg", si->ptszTopic);
				else
					db_unset(si->hContact, "CList", "StatusMsg");
			}
		}
		break;

	case GC_EVENT_ADDSTATUS:
		SM_GiveStatus(gce.pszID.w, gce.pszModule, gce.pszUID.w, gce.pszStatus.w);
		bIsHighlighted = g_chatApi.IsHighlighted(nullptr, &gce);
		break;

	case GC_EVENT_REMOVESTATUS:
		SM_TakeStatus(gce.pszID.w, gce.pszModule, gce.pszUID.w, gce.pszStatus.w);
		bIsHighlighted = g_chatApi.IsHighlighted(nullptr, &gce);
		break;

	case GC_EVENT_MESSAGE:
	case GC_EVENT_ACTION:
		if (!gce.bIsMe && gce.pszID.w && gce.pszText.w) {
			SESSION_INFO *si = SM_FindSession(gce.pszID.w, gce.pszModule);
			bIsHighlighted = g_chatApi.IsHighlighted(si, &gce);
		}
		break;

	case GC_EVENT_NICK:
		SM_ChangeNick(gce.pszID.w, gce.pszModule, &gce);
		bIsHighlighted = g_chatApi.IsHighlighted(nullptr, &gce);
		break;

	case GC_EVENT_TYPING:
		return SM_UserTyping(&gce);

	case GC_EVENT_JOIN:
		AddUser(&gce);
		bIsHighlighted = g_chatApi.IsHighlighted(nullptr, &gce);
		break;

	case GC_EVENT_PART:
	case GC_EVENT_QUIT:
	case GC_EVENT_KICK:
		bRemoveFlag = TRUE;
		bIsHighlighted = g_chatApi.IsHighlighted(nullptr, &gce);
		break;
	}

	// Decide which window (log) should have the event
	LPCTSTR pWnd = nullptr;
	LPCSTR pMod = nullptr;
	if (gce.pszID.w) {
		pWnd = gce.pszID.w;
		pMod = gce.pszModule;
	}
	else if (gce.iType == GC_EVENT_NOTICE || gce.iType == GC_EVENT_INFORMATION) {
		SESSION_INFO *si = g_chatApi.GetActiveSession();
		if (si && !mir_strcmp(si->pszModule, gce.pszModule)) {
			pWnd = si->ptszID;
			pMod = si->pszModule;
		}
		else return 0;
	}
	else {
		// Send the event to all windows with a user pszUID. Used for broadcasting QUIT etc
		AddEventToAllMatchingUID(&gce);
		if (!bRemoveFlag)
			return 0;
	}

	// add to log
	if (pWnd) {
		if (gce.dwFlags & GCEF_SILENT)
			return 0;

		SESSION_INFO *si = SM_FindSession(pWnd, pMod);

		// fix for IRC's old style mode notifications. Should not affect any other protocol
		if ((gce.iType == GC_EVENT_ADDSTATUS || gce.iType == GC_EVENT_REMOVESTATUS) && !(gce.dwFlags & GCEF_ADDTOLOG))
			return 0;

		if (gce.iType == GC_EVENT_JOIN && gce.time == 0)
			return 0;

		if (si && (si->bInitDone || gce.iType == GC_EVENT_TOPIC || (gce.iType == GC_EVENT_JOIN && gce.bIsMe))) {
			if (gce.pszNick.w == nullptr && gce.pszUID.w != nullptr) {
				USERINFO *ui = g_chatApi.UM_FindUser(si, gce.pszUID.w);
				if (ui != nullptr)
					gce.pszNick.w = ui->pszNick;
			}

			int isOk = SM_AddEvent(pWnd, pMod, &gce, bIsHighlighted);
			if (si->pDlg) {
				if (isOk)
					si->pDlg->AddLog();
				else
					RedrawLog2(si);
			}

			if (!(gce.dwFlags & GCEF_NOTNOTIFY))
				g_chatApi.DoSoundsFlashPopupTrayStuff(si, &gce, bIsHighlighted, 0);

			if ((gce.dwFlags & GCEF_ADDTOLOG) && g_Settings->bLoggingEnabled)
				LogToFile(si, &gce);
		}

		if (!bRemoveFlag)
			return 0;
	}

	if (bRemoveFlag)
		return SM_RemoveUser(gce.pszID.w, gce.pszModule, gce.pszUID.w) == 0;

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

MIR_APP_DLL(int) Chat_AddGroup(SESSION_INFO *si, const wchar_t *wszText)
{
	if (wszText == nullptr)
		return GC_EVENT_ERROR;
	if (si == nullptr)
		return 0;

	mir_cslock lck(csChat);
	STATUSINFO *ti = TM_AddStatus(&si->pStatuses, wszText, &si->iStatusCount);
	if (ti)
		si->iStatusCount++;

	if (g_chatApi.OnAddStatus)
		g_chatApi.OnAddStatus(si, ti);
	return 0;
}

MIR_APP_DLL(int) Chat_ChangeSessionName(const char *szModule, const wchar_t *wszId, const wchar_t *wszNewName)
{
	if (wszNewName == nullptr)
		return GC_EVENT_ERROR;

	SESSION_INFO *si = SM_FindSession(wszId, szModule);
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
	for (auto &si : g_arSessions) {
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		USERINFO *ui = g_chatApi.UM_FindUser(si, wszOldId);
		if (ui) {
			replaceStrW(ui->pszUID, wszNewId);
			UM_SortKeys(si);
		}
		if (wszId)
			break;
	}
	return 0;
}

MIR_APP_DLL(void*) Chat_GetUserInfo(const char *szModule, const wchar_t *wszId)
{
	if (SESSION_INFO *si = SM_FindSession(wszId, szModule))
		return si->pItemData;
	return nullptr;
}

MIR_APP_DLL(int) Chat_SendUserMessage(const char *szModule, const wchar_t *wszId, const wchar_t *wszText)
{
	if (wszText == nullptr || szModule == nullptr)
		return GC_EVENT_ERROR;

	if (wszId != nullptr) {
		SESSION_INFO *si = SM_FindSession(wszId, szModule);
		if (si)
			if (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS)
				Chat_DoEventHook(si, GC_USER_MESSAGE, nullptr, wszText, 0);
		return 0;
	}

	mir_cslock lck(csChat);
	for (auto &si : g_arSessions) {
		if (mir_strcmpi(si->pszModule, szModule))
			continue;

		if (si->iType == GCW_CHATROOM || si->iType == GCW_PRIVMESS)
			Chat_DoEventHook(si, GC_USER_MESSAGE, nullptr, wszText, 0);
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetStatusbarText(const char *szModule, const wchar_t *wszId, const wchar_t *wszText)
{
	SESSION_INFO *si = SM_FindSession(wszId, szModule);
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
	for (auto &si : g_arSessions) {
		if ((wszId && mir_wstrcmpi(si->ptszID, wszId)) || mir_strcmpi(si->pszModule, szModule))
			continue;

		UM_SetStatusEx(si, wszText, flags);
		if (si->pDlg)
			RedrawWindow(GetDlgItem(si->pDlg->GetHwnd(), IDC_LIST), nullptr, nullptr, RDW_INVALIDATE);
		if (wszId)
			break;
	}
	return 0;
}

MIR_APP_DLL(int) Chat_SetUserInfo(const char *szModule, const wchar_t *wszId, void *pItemData)
{
	if (SESSION_INFO *si = g_chatApi.SM_FindSession(wszId, szModule)) {
		si->pItemData = pItemData;
		return 0;
	}
	return GC_EVENT_ERROR;
}

EXTERN_C MIR_APP_DLL(void) Chat_UpdateOptions()
{
	for (auto &si : g_arSessions)
		if (si->pDlg)
			si->pDlg->UpdateOptions();
}

EXTERN_C MIR_APP_DLL(void) Chat_Mute(SESSION_INFO *si, int mode)
{
	db_set_b(si->hContact, "SRMM", "MuteMode", mode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// module initialization

static HGENMENU hJoinMenuItem, hLeaveMenuItem, hMuteRootMenuItem, hMute0MenuItem, hMute1MenuItem, hMute2MenuItem;

static INT_PTR JoinChat(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto) {
			if (db_get_w(hContact, szProto, "Status", 0) == ID_STATUS_OFFLINE)
				CallProtoService(szProto, PS_JOINCHAT, hContact, lParam);
			else
				RoomDoubleclicked(hContact, 0);
		}
	}

	return 0;
}

static INT_PTR LeaveChat(WPARAM hContact, LPARAM)
{
	if (hContact) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto)
			CallProtoService(szProto, PS_LEAVECHAT, hContact, 0);
	}
	return 0;
}

static int OnContactDeleted(WPARAM hContact, LPARAM)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto && Contact::IsGroupChat(hContact, szProto))
		if (Contact::GetStatus(hContact) != ID_STATUS_OFFLINE)
			CallProtoService(szProto, PS_LEAVECHAT, hContact, 0);

	return 0;
}	

static INT_PTR MuteChat(WPARAM hContact, LPARAM param)
{
	db_set_b(hContact, "SRMM", "MuteMode", param);
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		return 0;

	int iMuteMode = db_get_b(hContact, "SRMM", "MuteMode", CHATMODE_NORMAL);
	bool bEnabledJoin = false, bEnabledLeave = false, bIsChat = false;
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto) {
		// display this menu item only for chats
		if (Contact::IsGroupChat(hContact, szProto)) {
			bIsChat = true;
			// still hide it for offline protos
			if (Proto_GetStatus(szProto) != ID_STATUS_OFFLINE) {
				if (db_get_w(hContact, szProto, "Status", 0) == ID_STATUS_OFFLINE) {
					if (ProtoServiceExists(szProto, PS_JOINCHAT)) {
						bEnabledJoin = true;
						Menu_ModifyItem(hJoinMenuItem, LPGENW("&Join chat"));
					}
				}
				else {
					bEnabledJoin = true;
					Menu_ModifyItem(hJoinMenuItem, LPGENW("&Open/close chat window"));
				}
			}
			bEnabledLeave = ProtoServiceExists(szProto, PS_LEAVECHAT) != 0;
		}
	}

	Menu_ShowItem(hJoinMenuItem, bEnabledJoin);
	Menu_ShowItem(hLeaveMenuItem, bEnabledLeave);
	Menu_ShowItem(hMuteRootMenuItem, bIsChat);
	if (bIsChat) {
		Menu_ModifyItem(hMute0MenuItem, 0, INVALID_HANDLE_VALUE, (iMuteMode == CHATMODE_NORMAL) ? CMIF_CHECKED : 0);
		Menu_ModifyItem(hMute1MenuItem, 0, INVALID_HANDLE_VALUE, (iMuteMode == CHATMODE_MUTE) ? CMIF_CHECKED : 0);
		Menu_ModifyItem(hMute2MenuItem, 0, INVALID_HANDLE_VALUE, (iMuteMode == CHATMODE_UNMUTE) ? CMIF_CHECKED : 0);
	}
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	SrmmModulesLoaded();
	LoadChatIcons();

	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, SmileyOptionsChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x2bb76d5, 0x740d, 0x4fd2, 0x8f, 0xee, 0x7c, 0xa4, 0x5a, 0x74, 0x65, 0xa6);
	mi.position = -2000090002;
	mi.flags = CMIF_DEFAULT;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	mi.name.a = LPGEN("&Join chat");
	mi.pszService = "GChat/JoinChat";
	hJoinMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, JoinChat);

	SET_UID(mi, 0x72b7440b, 0xd2db, 0x4e22, 0xa6, 0xb1, 0x2, 0xd0, 0x96, 0xee, 0xad, 0x88);
	mi.position = -2000090001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_LEAVE);
	mi.flags = CMIF_NOTOFFLINE;
	mi.name.a = LPGEN("&Leave chat");
	mi.pszService = "GChat/LeaveChat";
	hLeaveMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, LeaveChat);

	SET_UID(mi, 0x1c16b08e, 0x923, 0x4321, 0x9e, 0x67, 0x3d, 0x69, 0x87, 0x37, 0xd9, 0x4a);
	mi.position = -2000090000;
	mi.hIcolibItem = 0;
	mi.flags = 0;
	mi.name.a = LPGEN("&Mute chat");
	mi.pszService = 0;
	hMuteRootMenuItem = Menu_AddContactMenuItem(&mi);

	UNSET_UID(mi);
	mi.root = hMuteRootMenuItem;
	mi.flags = CMIF_SYSTEM;
	mi.pszService = "GChat/MuteChat";
	CreateServiceFunction(mi.pszService, MuteChat);

	mi.position = 1;
	mi.hIcolibItem = 0;
	mi.name.a = LPGEN("Default");
	hMute0MenuItem = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hMute0MenuItem, MCI_OPT_EXECPARAM, INT_PTR(CHATMODE_NORMAL));

	mi.position = 2;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OFF);
	mi.name.a = LPGEN("Always");
	hMute1MenuItem = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hMute1MenuItem, MCI_OPT_EXECPARAM, INT_PTR(CHATMODE_MUTE));

	mi.position = 3;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ON);
	mi.name.a = LPGEN("Never");
	hMute2MenuItem = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hMute2MenuItem, MCI_OPT_EXECPARAM, INT_PTR(CHATMODE_UNMUTE));

	g_chatApi.SetAllOffline(TRUE, nullptr);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Status 1 (10x10)"), "status0", IDI_STATUS0, 10 },
	{ LPGEN("Status 2 (10x10)"), "status1", IDI_STATUS1, 10 },
	{ LPGEN("Status 3 (10x10)"), "status2", IDI_STATUS2, 10 },
	{ LPGEN("Status 4 (10x10)"), "status3", IDI_STATUS3, 10 },
	{ LPGEN("Status 5 (10x10)"), "status4", IDI_STATUS4, 10 },
	{ LPGEN("Status 6 (10x10)"), "status5", IDI_STATUS5, 10 }
};

static bool bInited = false;

int LoadChatModule(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_DB_CONTACT_DELETED, OnContactDeleted);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_FONT_RELOAD, FontsChanged);

	g_hWindowList = WindowList_Create();
	hHookEvent = CreateHookableEvent(ME_GC_HOOK_EVENT);
	hevSendEvent = CreateHookableEvent(ME_GC_EVENT);
	hevBuildMenuEvent = CreateHookableEvent(ME_GC_BUILDMENU);
	
	g_chatApi.hevPreCreate = CreateHookableEvent(ME_MSG_PRECREATEEVENT);
	g_chatApi.hevWinPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);

	g_plugin.registerIcon(LPGEN("Messaging") "/" LPGEN("Group chats"), iconList, "chat");

	g_plugin.addSound("ChatHighlight", LPGENW("Group chats"), LPGENW("Message is highlighted"));
	g_plugin.addSound("ChatAction", LPGENW("Group chats"), LPGENW("User has performed an action"));
	g_plugin.addSound("ChatJoin", LPGENW("Group chats"), LPGENW("User has joined"));
	g_plugin.addSound("ChatPart", LPGENW("Group chats"), LPGENW("User has left"));
	g_plugin.addSound("ChatKick", LPGENW("Group chats"), LPGENW("User has kicked some other user"));
	g_plugin.addSound("ChatMode", LPGENW("Group chats"), LPGENW("User's status was changed"));
	g_plugin.addSound("ChatNick", LPGENW("Group chats"), LPGENW("User has changed name"));
	g_plugin.addSound("ChatNotice", LPGENW("Group chats"), LPGENW("User has sent a notice"));
	g_plugin.addSound("ChatQuit", LPGENW("Group chats"), LPGENW("User has disconnected"));
	g_plugin.addSound("ChatTopic", LPGENW("Group chats"), LPGENW("The topic has been changed"));

	bInited = true;
	return 0;
}

void UnloadChatModule(void)
{
	if (!bInited)
		return;

	mir_free(g_chatApi.szActiveWndID);
	mir_free(g_chatApi.szActiveWndModule);

	FreeMsgLogBitmaps();
	OptionsUnInit();

	WindowList_Destroy(g_hWindowList);

	DestroyHookableEvent(hevSendEvent);
	DestroyHookableEvent(hevBuildMenuEvent);
	DestroyHookableEvent(hHookEvent);

	DestroyHookableEvent(g_chatApi.hevPreCreate);
	DestroyHookableEvent(g_chatApi.hevWinPopup);
}
