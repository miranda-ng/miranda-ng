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

HANDLE hSendEvent;
HANDLE hBuildMenuEvent ;
HANDLE g_hHookContactDblClick, g_hHookPrebuildMenu;
CRITICAL_SECTION cs;

int Chat_SmileyOptionsChanged(WPARAM wParam,LPARAM lParam)
{
	SM_BroadcastMessage(NULL, GC_REDRAWLOG, 0, 1, FALSE);
	return 0;
}

int Chat_PreShutdown(WPARAM wParam,LPARAM lParam)
{
	SM_BroadcastMessage(NULL, GC_CLOSEWINDOW, 0, 1, FALSE);

	SM_RemoveAll();
	MM_RemoveAll();
	return 0;
}

int Chat_FontsChanged(WPARAM wParam,LPARAM lParam)
{
	LoadLogFonts();
	LoadMsgLogBitmaps();
	MM_FontsChanged();
	MM_FixColors();
	SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
	return 0;
}

int Chat_IconsChanged(WPARAM wParam,LPARAM lParam)
{
	FreeMsgLogBitmaps();
	LoadMsgLogBitmaps();
	MM_IconsChanged();
	SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, FALSE);
	return 0;
}

static INT_PTR Service_GetCount(WPARAM wParam,LPARAM lParam)
{
	if (!lParam)
		return -1;

	mir_cslock lock(cs);
	return SM_GetCount((char*)lParam);
}

static INT_PTR Service_GetInfo(WPARAM wParam,LPARAM lParam)
{
	GC_INFO *gci = (GC_INFO *) lParam;
	if (!gci || !gci->pszModule)
		return 1;

	SESSION_INFO *si = NULL;

	mir_cslock lock(cs);
	if (gci->Flags & BYINDEX)
		si = SM_FindSessionByIndex( gci->pszModule, gci->iItem );
	else
		si = SM_FindSession( gci->pszID, gci->pszModule );

	if (si == NULL)
		return 1;

	if (gci->Flags & DATA)     gci->dwItemData = si->dwItemData;
	if (gci->Flags & HCONTACT) gci->hContact = si->windowData.hContact;
	if (gci->Flags & TYPE)     gci->iType = si->iType;
	if (gci->Flags & COUNT)    gci->iCount = si->nUsersInNicklist;
	if (gci->Flags & USERS)    gci->pszUsers = SM_GetUsers(si);
	if (gci->Flags & ID)       gci->pszID = si->ptszID;
	if (gci->Flags & NAME)     gci->pszName = si->ptszName;
	return 0;
}

void LoadModuleIcons(MODULEINFO *mi)
{
	HIMAGELIST hList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);

	int overlayIcon = ImageList_AddIcon(hList, GetCachedIcon("chat_overlay"));
	ImageList_SetOverlayImage(hList, overlayIcon, 1);
	
	mi->hOnlineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_ONLINE);
	mi->hOnlineIcon = LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_ONLINE);
	int index = ImageList_AddIcon(hList, mi->hOnlineIcon); 
	mi->hOnlineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	mi->hOfflineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_OFFLINE);
	mi->hOfflineIcon = LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_OFFLINE);
	index = ImageList_AddIcon(hList, mi->hOfflineIcon); 
	mi->hOfflineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	ImageList_Destroy(hList);
}

static INT_PTR Service_Register(WPARAM wParam, LPARAM lParam)
{
	GCREGISTER *gcr = (GCREGISTER *)lParam;
	if (gcr == NULL)
		return GC_REGISTER_ERROR;

	if (gcr->cbSize != sizeof(GCREGISTER))
		return GC_REGISTER_WRONGVER;

	mir_cslock lock(cs);
	MODULEINFO *mi = MM_AddModule(gcr->pszModule);
	if (mi == NULL)
		return GC_REGISTER_ERROR;

	mi->ptszModDispName = mir_tstrdup(gcr->ptszDispName);
	mi->bBold = gcr->dwFlags & GC_BOLD;
	mi->bUnderline = gcr->dwFlags & GC_UNDERLINE ;
	mi->bItalics = gcr->dwFlags & GC_ITALICS ;
	mi->bColor = gcr->dwFlags & GC_COLOR ;
	mi->bBkgColor = gcr->dwFlags & GC_BKGCOLOR ;
	mi->bFontSize = gcr->dwFlags & GC_FONTSIZE;
	mi->bAckMsg = gcr->dwFlags & GC_ACKMSG ;
	mi->bChanMgr = gcr->dwFlags & GC_CHANMGR ;
	mi->bSingleFormat = gcr->dwFlags & GC_SINGLEFORMAT;
	mi->iMaxText= gcr->iMaxText;
	mi->nColorCount = gcr->nColors;
	if ( gcr->nColors > 0) {
		mi->crColors = (COLORREF *)mir_alloc(sizeof(COLORREF) * gcr->nColors);
		memcpy(mi->crColors, gcr->pColors, sizeof(COLORREF) * gcr->nColors);
	}

	CheckColorsInModule((char*)gcr->pszModule);
	CList_SetAllOffline(TRUE, gcr->pszModule);
	return 0;
}

static INT_PTR Service_NewChat(WPARAM wParam, LPARAM lParam)
{
	GCSESSION *gcw = (GCSESSION*)lParam;
	if (gcw == NULL)
		return GC_NEWSESSION_ERROR;

	if (gcw->cbSize != sizeof(GCSESSION))
		return GC_NEWSESSION_WRONGVER;

	mir_cslock lock(cs);
	MODULEINFO *mi = MM_FindModule(gcw->pszModule);
	if (mi == NULL)
		return GC_NEWSESSION_ERROR;

	SESSION_INFO *si = SM_AddSession(gcw->ptszID, gcw->pszModule);
	if (mi->hOfflineIcon == NULL)
		LoadModuleIcons(mi);

	// create a new session and set the defaults
	if (si != NULL) {
		TCHAR szTemp[256];

		si->dwItemData = gcw->dwItemData;
		if ( gcw->iType != GCW_SERVER )
			si->wStatus = ID_STATUS_ONLINE;
		si->iType = gcw->iType;
		si->dwFlags = gcw->dwFlags;
		si->ptszName = mir_tstrdup(gcw->ptszName);
		si->ptszStatusbarText = mir_tstrdup(gcw->ptszStatusbarText);
		si->iSplitterX = g_Settings.iSplitterX;
		si->iSplitterY = g_Settings.iSplitterY;
		si->iLogFilterFlags = (int)db_get_dw(NULL, "Chat", "FilterFlags", 0x03E0);
		si->bFilterEnabled = db_get_b(NULL, "Chat", "FilterEnabled", 0);
		si->bNicklistEnabled = db_get_b(NULL, "Chat", "ShowNicklist", 1);

		if (mi->bColor) {
			si->iFG = 4;
			si->bFGSet = TRUE;
		}
		if (mi->bBkgColor) {
			si->iBG = 2;
			si->bBGSet = TRUE;
		}
		if (si->iType == GCW_SERVER)
			mir_sntprintf(szTemp, SIZEOF(szTemp), LPGENT("Server: %s"), si->ptszName);
		else
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s"), si->ptszName);
		si->windowData.hContact = CList_AddRoom(gcw->pszModule, gcw->ptszID, szTemp, si->iType);
		si->windowData.codePage = db_get_w(si->windowData.hContact, si->pszModule, "CodePage", (WORD)CP_ACP);
		si->pszHeader = Log_CreateRtfHeader(mi, si);
		db_set_s(si->windowData.hContact, si->pszModule, "Topic", "");
		db_unset(si->windowData.hContact, "CList", "StatusMsg");
		if (si->ptszStatusbarText)
			db_set_ts(si->windowData.hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
		else
			db_set_s(si->windowData.hContact, si->pszModule, "StatusBar", "");
	}
	else {
		SESSION_INFO *si2 = SM_FindSession(gcw->ptszID, gcw->pszModule);
		if (si2) {
			UM_RemoveAll(&si2->pUsers);
			TM_RemoveAll(&si2->pStatuses);

			si2->iStatusCount = 0;
			si2->nUsersInNicklist = 0;

			if (si2->hWnd )
				RedrawWindow(GetDlgItem(si2->hWnd, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
		}
	}

	return 0;
}

static INT_PTR DoControl(GCEVENT *gce, WPARAM wp)
{
	SESSION_INFO *si;

	if ( gce->pDest->iType == GC_EVENT_CONTROL ) {
		switch (wp) {
		case WINDOW_HIDDEN:
			if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
				si->bInitDone = TRUE;
				SetActiveSession(si->ptszID, si->pszModule);
				if (si->hWnd)
					ShowRoom(si, wp, FALSE);
			}
			return 0;

		case WINDOW_MINIMIZE:
		case WINDOW_MAXIMIZE:
		case WINDOW_VISIBLE:
		case SESSION_INITDONE:
			if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
				si->bInitDone = TRUE;
				if (wp != SESSION_INITDONE || db_get_b(NULL, "Chat", "PopupOnJoin", 0) == 0)
					ShowRoom(si, wp, TRUE);
				return 0;
			}
			break;

		case SESSION_OFFLINE:
			SM_SetOffline(gce->pDest->ptszID, gce->pDest->pszModule);
			// fall through

		case SESSION_ONLINE:
			SM_SetStatus( gce->pDest->ptszID, gce->pDest->pszModule, wp==SESSION_ONLINE?ID_STATUS_ONLINE:ID_STATUS_OFFLINE);
			break;

		case WINDOW_CLEARLOG:
			if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
				LM_RemoveAll(&si->pLog, &si->pLogEnd);
				si->iEventCount = 0;
				si->LastTime = 0;
			}
			break;

		case SESSION_TERMINATE:
			return SM_RemoveSession(gce->pDest->ptszID, gce->pDest->pszModule, (gce->dwFlags & GCEF_REMOVECONTACT) != 0);
		}
		SM_SendMessage(gce->pDest->ptszID, gce->pDest->pszModule, GC_EVENT_CONTROL + WM_USER + 500, wp, 0);
	}
	else if (gce->pDest->iType == GC_EVENT_CHUID && gce->ptszText)
	{
		SM_ChangeUID( gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszNick, gce->ptszText);
	}
	else if (gce->pDest->iType == GC_EVENT_CHANGESESSIONAME && gce->ptszText)
	{
		if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
			replaceStrT(si->ptszName, gce->ptszText);
			if (si->hWnd)
				SendMessage(si->hWnd, DM_UPDATETITLEBAR, 0, 0);
		}
	}
	else if (gce->pDest->iType == GC_EVENT_SETITEMDATA) {
		if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule))
			si->dwItemData = gce->dwItemData;
	}
	else if (gce->pDest->iType ==GC_EVENT_GETITEMDATA) {
		if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
			gce->dwItemData = si->dwItemData;
			return si->dwItemData;
		}
		return 0;
	}
	else if (gce->pDest->iType ==GC_EVENT_SETSBTEXT)
	{
		if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
			replaceStrT(si->ptszStatusbarText, gce->ptszText);
			if (si->ptszStatusbarText)
				db_set_ts(si->windowData.hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
			else
				db_set_s(si->windowData.hContact, si->pszModule, "StatusBar", "");
			if (si->hWnd)
				SendMessage(si->hWnd, DM_UPDATESTATUSBAR, 0, 0);
		}
	}
	else if (gce->pDest->iType == GC_EVENT_ACK)
	{
		SM_SendMessage(gce->pDest->ptszID, gce->pDest->pszModule, GC_ACKMESSAGE, 0, 0);
	}
	else if (gce->pDest->iType == GC_EVENT_SENDMESSAGE && gce->ptszText)
	{
		SM_SendUserMessage( gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszText);
	}
	else if (gce->pDest->iType == GC_EVENT_SETSTATUSEX)
	{
		SM_SetStatusEx( gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszText, gce->dwItemData);
	}
	else return 1;

	return 0;
}

static void AddUser(GCEVENT *gce)
{
	SESSION_INFO *si = SM_FindSession( gce->pDest->ptszID, gce->pDest->pszModule);
	if (si == NULL) return;

	WORD status = TM_StringToWord(si->pStatuses, gce->ptszStatus);
	USERINFO *ui = SM_AddUser(si, gce->ptszUID, gce->ptszNick, status);
	if (ui == NULL) return;

	ui->pszNick = mir_tstrdup(gce->ptszNick);
	if (gce->bIsMe)
		si->pMe = ui;
	ui->Status = status;
	ui->Status |= si->pStatuses->Status;
	if (si->hWnd)
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
}

void ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground)
{
	HWND hParent = NULL;
	if (!si)
		return;

	//Do we need to create a window?
	if (si->hWnd == NULL)
	{
	    hParent = GetParentWindow(si->windowData.hContact, TRUE);
	    si->hWnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), hParent, RoomWndProc, (LPARAM)si);
	}
	SendMessage(si->hWnd, DM_UPDATETABCONTROL, -1, (LPARAM)si);
	SendMessage(GetParent(si->hWnd), CM_ACTIVATECHILD, 0, (LPARAM)si->hWnd);
	SendMessage(GetParent(si->hWnd), CM_POPUPWINDOW, 0, (LPARAM)si->hWnd);
	SendMessage(si->hWnd, WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(si->hWnd, IDC_CHAT_MESSAGE));
}

static INT_PTR Service_AddEvent(WPARAM wParam, LPARAM lParam)
{
	GCEVENT *gce = (GCEVENT*)lParam;
	SESSION_INFO *si;
	BOOL bIsHighlighted = FALSE;
	BOOL bRemoveFlag = FALSE;

	if (gce == NULL)
		return GC_EVENT_ERROR;

	GCDEST *gcd = gce->pDest;
	if (gcd == NULL)
		return GC_EVENT_ERROR;

	if (gce->cbSize != sizeof(GCEVENT))
		return GC_EVENT_WRONGVER;

	if (!IsEventSupported(gcd->iType))
		return GC_EVENT_ERROR;

	mir_cslock lck(cs);

	// Do different things according to type of event
	switch(gcd->iType) {
	case GC_EVENT_ADDGROUP:
		{
			STATUSINFO *si = SM_AddStatus(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszStatus);
			if (si && gce->dwItemData)
				si->hIcon = CopyIcon((HICON)gce->dwItemData);
		}
		return 0;

	case GC_EVENT_CHUID:
	case GC_EVENT_CHANGESESSIONAME:
	case GC_EVENT_SETITEMDATA:
	case GC_EVENT_GETITEMDATA:
	case GC_EVENT_CONTROL:
	case GC_EVENT_SETSBTEXT:
	case GC_EVENT_ACK:
	case GC_EVENT_SENDMESSAGE :
	case GC_EVENT_SETSTATUSEX :
		return DoControl(gce, wParam);

	case GC_EVENT_SETCONTACTSTATUS:
		return SM_SetContactStatus(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, (WORD)gce->dwItemData);

	case GC_EVENT_TOPIC:
		if (si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule)) {
			if (gce->ptszText) {
				replaceStrT(si->ptszTopic, gce->ptszText);
				db_set_ts(si->windowData.hContact, si->pszModule , "Topic", RemoveFormatting(si->ptszTopic));
				if (db_get_b(NULL, "Chat", "TopicOnClist", 0))
					db_set_ts(si->windowData.hContact, "CList" , "StatusMsg", RemoveFormatting(si->ptszTopic));
			}
		}
		break;

	case GC_EVENT_ADDSTATUS:
		SM_GiveStatus( gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, gce->ptszStatus);
		break;

	case GC_EVENT_REMOVESTATUS:
		SM_TakeStatus( gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, gce->ptszStatus);
		break;

	case GC_EVENT_MESSAGE:
	case GC_EVENT_ACTION:
		if (!gce->bIsMe && gce->pDest->ptszID && gce->ptszText) {
			if (si = SM_FindSession( gce->pDest->ptszID, gce->pDest->pszModule))
				if ( IsHighlighted(si, gce->ptszText))
					bIsHighlighted = TRUE;
		}
		break;

	case GC_EVENT_NICK:
		SM_ChangeNick( gce->pDest->ptszID, gce->pDest->pszModule, gce);
		break;

	case GC_EVENT_JOIN:
		AddUser(gce);
		break;

	case GC_EVENT_PART:
	case GC_EVENT_QUIT:
	case GC_EVENT_KICK:
		bRemoveFlag = TRUE;
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
		SESSION_INFO *si = GetActiveSession();
		if (si && !lstrcmpA(si->pszModule, gcd->pszModule)) {
			pWnd = si->ptszID;
			pMod = si->pszModule;
		}
		else return 0;
	}
	else {
		// Send the event to all windows with a user pszUID. Used for broadcasting QUIT etc
		SM_AddEventToAllMatchingUID(gce);
		if (!bRemoveFlag)
			return 0;
	}

	// add to log
	if (pWnd) {
		si = SM_FindSession(pWnd, pMod);

		// fix for IRC's old stuyle mode notifications. Should not affect any other protocol
		if ((gce->pDest->iType == GC_EVENT_ADDSTATUS || gce->pDest->iType == GC_EVENT_REMOVESTATUS) && !(gce->dwFlags & GCEF_ADDTOLOG))
			return 0;

		if (gce && gce->pDest->iType == GC_EVENT_JOIN && gce->time == 0)
			return 0;

		if (si && (si->bInitDone || gce->pDest->iType == GC_EVENT_TOPIC || (gce->pDest->iType == GC_EVENT_JOIN && gce->bIsMe))) {
			if (SM_AddEvent(pWnd, pMod, gce, bIsHighlighted) && si->hWnd)
				SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
			else if (si->hWnd)
				SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);

			if (!(gce->dwFlags & GCEF_NOTNOTIFY))
				DoSoundsFlashPopupTrayStuff(si, gce, bIsHighlighted, 0);
			if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings.LoggingEnabled)
				LogToFile(si, gce);
		}

		if (!bRemoveFlag)
			return 0;
	}

	if (bRemoveFlag)
		return SM_RemoveUser( gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID ) == 0;

	return GC_EVENT_ERROR;
}

static INT_PTR Service_GetAddEventPtr(WPARAM wParam, LPARAM lParam)
{
	GCPTRS *gp = (GCPTRS*)lParam;

	mir_cslock lck(cs);
	gp->pfnAddEvent = Service_AddEvent;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service creation

void HookEvents(void)
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CList_PrebuildContactMenu);
}

void CreateServiceFunctions(void)
{
	CreateServiceFunction(MS_GC_REGISTER,       Service_Register);
	CreateServiceFunction(MS_GC_NEWSESSION,     Service_NewChat);
	CreateServiceFunction(MS_GC_EVENT,          Service_AddEvent);
	CreateServiceFunction(MS_GC_GETEVENTPTR,    Service_GetAddEventPtr);
	CreateServiceFunction(MS_GC_GETINFO,        Service_GetInfo);
	CreateServiceFunction(MS_GC_GETSESSIONCOUNT, Service_GetCount);

	CreateServiceFunction("GChat/DblClickEvent",    CList_EventDoubleclickedSvc);
	CreateServiceFunction("GChat/PrebuildMenuEvent", CList_PrebuildContactMenuSvc);
	CreateServiceFunction("GChat/JoinChat",         CList_JoinChat);
	CreateServiceFunction("GChat/LeaveChat",        CList_LeaveChat);
}

void CreateHookableEvents(void)
{
	hSendEvent = CreateHookableEvent(ME_GC_EVENT);
	hBuildMenuEvent = CreateHookableEvent(ME_GC_BUILDMENU);
}

void DestroyHookableEvents(void)
{
	DestroyHookableEvent(hSendEvent);
	DestroyHookableEvent(hBuildMenuEvent);
}
