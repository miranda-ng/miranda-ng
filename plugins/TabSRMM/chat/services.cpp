/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2009 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * This code is based on and still contains large parts of the the
 * original chat module for Miranda IM, written and copyrighted
 * by Joergen Persson in 2005.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: services.cpp 13046 2010-10-28 10:02:50Z silvercircle $
 *
 * This implements the services that form the group chat API
 *
 */

#include "../src/commonheaders.h"

// defs
extern HICON		hIcons[30];

HANDLE				hSendEvent;
HANDLE				hBuildMenuEvent;
HANDLE				hJoinMenuItem, hLeaveMenuItem;
HANDLE				g_hHookPrebuildMenu;
CRITICAL_SECTION	cs;

static HANDLE		hServiceRegister = NULL,
					hServiceNewChat = NULL,
					hServiceAddEvent = NULL,
					hServiceGetAddEventPtr = NULL,
					hServiceGetInfo = NULL,
					hServiceGetCount = NULL,
					hEventPrebuildMenu = NULL,
					hEventDoubleclicked = NULL,
					hEventJoinChat = NULL,
					hEventLeaveChat = NULL;

#ifdef _WIN64

#define SIZEOF_STRUCT_GCREGISTER_V1 40
#define SIZEOF_STRUCT_GCWINDOW_V1	48
#define SIZEOF_STRUCT_GCEVENT_V1	76
#define SIZEOF_STRUCT_GCEVENT_V2	80

#else

#define SIZEOF_STRUCT_GCREGISTER_V1 28
#define SIZEOF_STRUCT_GCWINDOW_V1	32
#define SIZEOF_STRUCT_GCEVENT_V1	44
#define SIZEOF_STRUCT_GCEVENT_V2	48

#endif

int Chat_ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	if (!PluginConfig.m_chat_enabled)
		return 0;

	char * mods[3] = {"Chat", CHAT_FONTMODULE};
	CallService("DBEditorpp/RegisterModule", (WPARAM)mods, (LPARAM)2);

	LoadIcons();

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = -2000090001;
	mi.flags = CMIF_DEFAULT | CMIF_ICONFROMICOLIB;
	mi.icolibItem = LoadSkinnedIconHandle( SKINICON_CHAT_JOIN );
	mi.pszName = LPGEN("&Join");
	mi.pszService = "GChat/JoinChat";
	hJoinMenuItem = ( HANDLE )CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

	mi.position = -2000090000;
	mi.flags = CMIF_NOTOFFLINE | CMIF_ICONFROMICOLIB;
	mi.icolibItem = LoadSkinnedIconHandle( SKINICON_CHAT_LEAVE );
	mi.pszName = LPGEN("&Leave");
	mi.pszService = "GChat/LeaveChat";
	hLeaveMenuItem = ( HANDLE )CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);

	CList_SetAllOffline(TRUE, NULL);

	g_Settings.MathMod = ServiceExists(MATH_RTF_REPLACE_FORMULAE) && M->GetByte("Chat", "MathModSupport", 0);
	return 0;
}

int Chat_PreShutdown()
{
	SM_RemoveAll();
	MM_RemoveAll();
	return 0;
}

int Chat_IconsChanged(WPARAM wParam, LPARAM lParam)
{
	FreeMsgLogBitmaps();

	LoadLogIcons();
	LoadMsgLogBitmaps();
	//MM_IconsChanged();
	SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, FALSE);
	return 0;
}

INT_PTR Service_GetCount(WPARAM wParam, LPARAM lParam)
{
	int i;

	if (!lParam)
		return -1;

	EnterCriticalSection(&cs);

	i = SM_GetCount((char *)lParam);

	LeaveCriticalSection(&cs);
	return i;
}

INT_PTR Service_GetInfo(WPARAM wParam, LPARAM lParam)
{
	GC_INFO * gci = (GC_INFO *) lParam;
	SESSION_INFO* si = NULL;

	if (!gci || !gci->pszModule)
		return 1;

	EnterCriticalSection(&cs);

	if (gci->Flags&BYINDEX)
		si = SM_FindSessionByIndex(gci->pszModule, gci->iItem);
	else
		si = SM_FindSession(gci->pszID, gci->pszModule);

	if (si) {
		if (gci->Flags & DATA)     gci->dwItemData = si->dwItemData;
		if (gci->Flags & HCONTACT) gci->hContact = si->hContact;
		if (gci->Flags & TYPE)     gci->iType = si->iType;
		if (gci->Flags & COUNT)    gci->iCount = si->nUsersInNicklist;
		if (gci->Flags & USERS)    gci->pszUsers = SM_GetUsers(si);

		if (si->dwFlags & GC_UNICODE) {
			if (gci->Flags & ID)    gci->pszID = si->ptszID;
			if (gci->Flags & NAME)  gci->pszName = si->ptszName;
			}
		else {
			if (gci->Flags & ID)    gci->pszID = (TCHAR*)si->pszID;
			if (gci->Flags & NAME)  gci->pszName = (TCHAR*)si->pszName;
		}
		LeaveCriticalSection(&cs);
		return 0;
	}

	LeaveCriticalSection(&cs);
	return 1;
}

INT_PTR Service_Register(WPARAM wParam, LPARAM lParam)
{

	GCREGISTER *gcr = (GCREGISTER *)lParam;
	MODULEINFO * mi = NULL;
	if (gcr == NULL)
		return GC_REGISTER_ERROR;

	if (gcr->cbSize != SIZEOF_STRUCT_GCREGISTER_V1)
		return GC_REGISTER_WRONGVER;

	EnterCriticalSection(&cs);

	mi = MM_AddModule(gcr->pszModule);
	if (mi) {
		mi->ptszModDispName = a2tf( gcr->ptszModuleDispName, gcr->dwFlags, 0);
		mi->bBold = gcr->dwFlags & GC_BOLD;
		mi->bUnderline = gcr->dwFlags & GC_UNDERLINE ;
		mi->bItalics = gcr->dwFlags & GC_ITALICS ;
		mi->bColor = gcr->dwFlags & GC_COLOR ;
		mi->bBkgColor = gcr->dwFlags & GC_BKGCOLOR ;
		mi->bAckMsg = gcr->dwFlags & GC_ACKMSG ;
		mi->bChanMgr = gcr->dwFlags & GC_CHANMGR ;
		mi->iMaxText = gcr->iMaxText;
		mi->nColorCount = gcr->nColors;
		if (gcr->nColors > 0) {
			mi->crColors = (COLORREF *)mir_alloc(sizeof(COLORREF) * gcr->nColors);
			memcpy(mi->crColors, gcr->pColors, sizeof(COLORREF) * gcr->nColors);
		}
		mi->pszHeader = 0;

		CheckColorsInModule((char*)gcr->pszModule);
		CList_SetAllOffline(TRUE, gcr->pszModule);

		LeaveCriticalSection(&cs);
		return 0;
	}

	LeaveCriticalSection(&cs);
	return GC_REGISTER_ERROR;
}

INT_PTR Service_NewChat(WPARAM wParam, LPARAM lParam)
{
	MODULEINFO* mi;
	GCSESSION *gcw = (GCSESSION *)lParam;
	if (gcw == NULL)
		return GC_NEWSESSION_ERROR;

	if (gcw->cbSize != SIZEOF_STRUCT_GCWINDOW_V1)
		return GC_NEWSESSION_WRONGVER;

	EnterCriticalSection(&cs);

	if ((mi = MM_FindModule(gcw->pszModule)) != NULL) {
		TCHAR* ptszID = a2tf(gcw->ptszID, gcw->dwFlags, 0);
		SESSION_INFO* si = SM_AddSession(ptszID, gcw->pszModule);

		// create a new session and set the defaults
		if (si != NULL) {
			TCHAR szTemp[256];

			si->dwItemData = gcw->dwItemData;
			if (gcw->iType != GCW_SERVER)
				si->wStatus = ID_STATUS_ONLINE;
			si->iType = gcw->iType;
			si->dwFlags = gcw->dwFlags;
			si->ptszName = a2tf(gcw->ptszName, gcw->dwFlags, 0);
			si->ptszStatusbarText = a2tf(gcw->ptszStatusbarText, gcw->dwFlags, 0);
			si->iSplitterX = g_Settings.iSplitterX;
			si->bFilterEnabled = M->GetByte(si->hContact, "Chat", "FilterEnabled", M->GetByte("Chat", "FilterEnabled", 0));
			si->bNicklistEnabled = M->GetByte("Chat", "ShowNicklist", 1);
			if (!(gcw->dwFlags & GC_UNICODE)) {
				si->pszID = mir_strdup(gcw->pszID);
				si->pszName = mir_strdup(gcw->pszName);
			}

			if (mi->bColor) {
				si->iFG = 4;
				si->bFGSet = TRUE;
			}
			if (mi->bBkgColor) {
				si->iBG = 2;
				si->bBGSet = TRUE;
			}
			if (si->iType == GCW_SERVER)
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("Server: %s"), si->ptszName);
			else
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%s"), si->ptszName);
			si->hContact = CList_AddRoom(gcw->pszModule, ptszID, szTemp, si->iType);
			DBWriteContactSettingString(si->hContact, si->pszModule , "Topic", "");
			DBDeleteContactSetting(si->hContact, "CList", "StatusMsg");
			if (si->ptszStatusbarText)
				M->WriteTString(si->hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
			else
				DBWriteContactSettingString(si->hContact, si->pszModule, "StatusBar", "");
			if (si->hContact)
				Chat_SetFilters(si);
		} else {
			SESSION_INFO* si2 = SM_FindSession(ptszID, gcw->pszModule);
			if (si2) {

				UM_RemoveAll(&si2->pUsers);
				TM_RemoveAll(&si2->pStatuses);

				si2->iStatusCount = 0;
				si2->nUsersInNicklist = 0;

				if (si2->hContact)
					Chat_SetFilters(si2);
				if (si2->hWnd)
					RedrawWindow(GetDlgItem(si2->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
			}
		}

		LeaveCriticalSection(&cs);
		mir_free(ptszID);
		return 0;
	}

	LeaveCriticalSection(&cs);
	return GC_NEWSESSION_ERROR;
}

static int DoControl(GCEVENT * gce, WPARAM wp)
{
	if (gce->pDest->iType == GC_EVENT_CONTROL) {
		switch (wp) {
		case WINDOW_HIDDEN:
			{
				SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
				if (si) {
					si->bInitDone = TRUE;
					SetActiveSession(si->ptszID, si->pszModule);
					if (si->hWnd)
						ShowRoom(si, wp, FALSE);
				}
			}
			return 0;

			case WINDOW_MINIMIZE:
			case WINDOW_MAXIMIZE:
			case WINDOW_VISIBLE:
		case SESSION_INITDONE:
			{
				SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
				if (si) {
					si->bInitDone = TRUE;
					if (wp != SESSION_INITDONE || M->GetByte("Chat", "PopupOnJoin", 0) == 0)
						ShowRoom(si, wp, TRUE);
					return 0;
			}	}
			break;

			case SESSION_OFFLINE:
				SM_SetOffline(gce->pDest->ptszID, gce->pDest->pszModule);
				// fall through

			case SESSION_ONLINE:
				SM_SetStatus(gce->pDest->ptszID, gce->pDest->pszModule, wp == SESSION_ONLINE ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
				break;

		case WINDOW_CLEARLOG:
		{
				SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
				if (si) {
					LM_RemoveAll(&si->pLog, &si->pLogEnd);
					si->iEventCount = 0;
					si->LastTime = 0;
				}
				break;
			}
			case SESSION_TERMINATE:
				return SM_RemoveSession(gce->pDest->ptszID, gce->pDest->pszModule);
		}
		SM_SendMessage(gce->pDest->ptszID, gce->pDest->pszModule, GC_EVENT_CONTROL + WM_USER + 500, wp, 0);
	}

	else if (gce->pDest->iType == GC_EVENT_CHUID && gce->pszText)
	{
		SM_ChangeUID(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszNick, gce->ptszText);
	}

	else if (gce->pDest->iType == GC_EVENT_CHANGESESSIONAME && gce->pszText)
	{
		SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
		if (si) {
			replaceStr(&si->ptszName, gce->ptszText);
			if (si->hWnd)
				SendMessage(si->hWnd, GC_UPDATETITLE, 0, 0);
		}
	}

	else if (gce->pDest->iType == GC_EVENT_SETITEMDATA) {
		SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
		if (si)
			si->dwItemData = gce->dwItemData;
	}

	else if (gce->pDest->iType == GC_EVENT_GETITEMDATA) {
		SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
		if (si) {
			gce->dwItemData = si->dwItemData;
			return si->dwItemData;
		}
		return 0;
	}
	else if (gce->pDest->iType == GC_EVENT_SETSBTEXT)
	{
		SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
		if (si) {
			replaceStr(&si->ptszStatusbarText, gce->ptszText);
			if (si->ptszStatusbarText)
				M->WriteTString(si->hContact, si->pszModule, "StatusBar", si->ptszStatusbarText);
			else
				DBWriteContactSettingString(si->hContact, si->pszModule, "StatusBar", "");
			if (si->hWnd) {
				SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
			}
		}
	}
	else if (gce->pDest->iType == GC_EVENT_ACK)
	{
		SM_SendMessage(gce->pDest->ptszID, gce->pDest->pszModule, GC_ACKMESSAGE, 0, 0);
	}
	else if (gce->pDest->iType == GC_EVENT_SENDMESSAGE && gce->pszText)
	{
		SM_SendUserMessage(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszText);
	}
	else if (gce->pDest->iType == GC_EVENT_SETSTATUSEX)
	{
		SM_SetStatusEx(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszText, gce->dwItemData);
	}
	else return 1;

	return 0;
}

static void AddUser(GCEVENT * gce)
{
	SESSION_INFO* si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
	if (si) {
		WORD status = TM_StringToWord(si->pStatuses, gce->ptszStatus);
		USERINFO * ui = SM_AddUser(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, gce->ptszNick, status);
		if (ui) {
			ui->pszNick = mir_tstrdup(gce->ptszNick);

			if (gce->bIsMe)
				si->pMe = ui;

			ui->Status = status;
			ui->Status |= si->pStatuses->Status;

			if (si->hWnd) {
				SendMessage(si->hWnd, GC_UPDATENICKLIST, (WPARAM)0, (LPARAM)0);
				if(si->dat)
					GetMyNick(si->dat);
			}
		}
	}
}

HWND CreateNewRoom(TContainerData *pContainer, SESSION_INFO *si, BOOL bActivateTab, BOOL bPopupContainer, BOOL bWantPopup)
{
	TCHAR *contactName = NULL, newcontactname[128];
	char *szProto, *szStatus;
	WORD wStatus;
	int	newItem;
	HWND hwndNew = 0;
	struct TNewWindowData newData = {
		0
	};
	HANDLE hContact = si->hContact;
	HWND hwndTab;

	if (M->FindWindow(hContact) != 0)
		return 0;
	if (hContact != 0 && M->GetByte("limittabs", 0) &&  !_tcsncmp(pContainer->szName, _T("default"), 6)) {
		if ((pContainer = FindMatchingContainer(_T("default"), hContact)) == NULL) {
			TCHAR szName[CONTAINER_NAMELEN + 1];

			_sntprintf(szName, CONTAINER_NAMELEN, _T("default"));
			pContainer = CreateContainer(szName, CNT_CREATEFLAG_CLONED, hContact);
		}
	}
	newData.hContact = hContact;
	newData.isWchar = 0;
	newData.szInitialText = NULL;
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) newData.hContact, 0);

	ZeroMemory((void *)&newData.item, sizeof(newData.item));

	contactName = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) newData.hContact, GCDNF_TCHAR);

	/*
	 * cut nickname if larger than x chars...
	 */

	if (contactName && lstrlen(contactName) > 0) {
		if (M->GetByte("cuttitle", 0))
			CutContactName(contactName, newcontactname, safe_sizeof(newcontactname));
		else {
			lstrcpyn(newcontactname, contactName, safe_sizeof(newcontactname));
			newcontactname[127] = 0;
		}
	} else
		lstrcpyn(newcontactname, _T("_U_"), sizeof(newcontactname) / sizeof(TCHAR));

	wStatus = szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord((HANDLE) newData.hContact, szProto, "Status", ID_STATUS_OFFLINE);
	szStatus = (char *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, szProto == NULL ? ID_STATUS_OFFLINE : DBGetContactSettingWord((HANDLE)newData.hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);

	newData.item.pszText = newcontactname;

	newData.item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
	newData.item.iImage = 0;

	hwndTab = GetDlgItem(pContainer->hwnd, 1159);

	// hide the active tab
	if (pContainer->hwndActive && bActivateTab)
		ShowWindow(pContainer->hwndActive, SW_HIDE);

	{
		int iTabIndex_wanted = M->GetDword(hContact, "tabindex", pContainer->iChilds * 100);
		int iCount = TabCtrl_GetItemCount(hwndTab);
		TCITEM item = {0};
		HWND hwnd;
		struct TWindowData *dat;
		int relPos;
		int i;

		pContainer->iTabIndex = iCount;
		if (iCount > 0) {
			for (i = iCount - 1; i >= 0; i--) {
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(hwndTab, i, &item);
				hwnd = (HWND)item.lParam;
				dat = (struct TWindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (dat) {
					relPos = M->GetDword(dat->hContact, "tabindex", i * 100);
					if (iTabIndex_wanted <= relPos)
						pContainer->iTabIndex = i;
				}
			}
		}
	}

	newItem = TabCtrl_InsertItem(hwndTab, pContainer->iTabIndex, &newData.item);
	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	if (bActivateTab)
		TabCtrl_SetCurSel(hwndTab, newItem);
	newData.iTabID = newItem;
	newData.iTabImage = newData.item.iImage;
	newData.pContainer = pContainer;
	newData.iActivate = (int) bActivateTab;
	pContainer->iChilds++;
	newData.bWantPopup = bWantPopup;
	newData.hdbEvent = (HANDLE)si;
	hwndNew = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), GetDlgItem(pContainer->hwnd, 1159), RoomWndProc, (LPARAM) & newData);
	if(pContainer->dwFlags & CNT_SIDEBAR) {
		TWindowData *dat = (TWindowData *)GetWindowLongPtr(hwndNew, GWLP_USERDATA);
		if(dat)
			pContainer->SideBar->addSession(dat, pContainer->iTabIndex);
	}
	SendMessage(pContainer->hwnd, WM_SIZE, 0, 0);
	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->hwnd)) {
		if (bPopupContainer) {
			SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetFocus(pContainer->hwndActive);
		} else {
			if (pContainer->dwFlags & CNT_NOFLASH)
				SendMessage(pContainer->hwnd, DM_SETICON, 0, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			else
				FlashContainer(pContainer, 1, 0);
		}
	}
	if (bActivateTab) {
		if (PluginConfig.m_HideOnClose && !IsWindowVisible(pContainer->hwnd)){
			WINDOWPLACEMENT wp={0};
			wp.length = sizeof(wp);
			GetWindowPlacement(pContainer->hwnd, &wp);

			BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);			// make sure all tabs will re-check layout on activation
			if(wp.showCmd == SW_SHOWMAXIMIZED)
				ShowWindow(pContainer->hwnd, SW_SHOWMAXIMIZED);
			else {
				if(bPopupContainer)
					ShowWindow(pContainer->hwnd, SW_SHOWNORMAL);
				else
					ShowWindow(pContainer->hwnd, SW_SHOWMINNOACTIVE);
			}
			SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
			SetFocus(hwndNew);
		}
		else {
			SetFocus(hwndNew);
			RedrawWindow(pContainer->hwnd, NULL, NULL, RDW_INVALIDATE);
			UpdateWindow(pContainer->hwnd);
			if (GetForegroundWindow() != pContainer->hwnd && bPopupContainer == TRUE)
				SetForegroundWindow(pContainer->hwnd);
		}
	}
	if(PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled && !M->GetByte("forceAeroPeek", 0))
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, CWarning::CWF_UNTRANSLATED|MB_ICONWARNING|MB_OK);
	return hwndNew;		// return handle of the new dialog
}

void ShowRoom(SESSION_INFO* si, WPARAM wp, BOOL bSetForeground)
{
	if (!si)
		return;

	if (si->hWnd == NULL) {
		TCHAR szName[CONTAINER_NAMELEN + 2];
		struct TContainerData *pContainer = si->pContainer;

		szName[0] = 0;
		if (pContainer == NULL) {
			GetContainerNameForContact(si->hContact, szName, CONTAINER_NAMELEN);
			if (!g_Settings.OpenInDefault && !_tcscmp(szName, _T("default")))
				_tcsncpy(szName, _T("Chat Rooms"), CONTAINER_NAMELEN);
			szName[CONTAINER_NAMELEN] = 0;
			pContainer = FindContainerByName(szName);
		}
		if (pContainer == NULL)
			pContainer = CreateContainer(szName, FALSE, si->hContact);
		si->hWnd = CreateNewRoom(pContainer, si, TRUE, TRUE, FALSE);
	} else
		ActivateExistingTab(si->pContainer, si->hWnd);
}

INT_PTR Service_AddEvent(WPARAM wParam, LPARAM lParam)
{
	GCEVENT *gce = (GCEVENT*)lParam;
	GCDEST *gcd = NULL;
	GCEVENT save_gce;
	GCDEST  save_gcd;
	TCHAR* pWnd = NULL;
	char* pMod = NULL;
	BOOL bIsHighlighted = FALSE;
	BOOL bRemoveFlag = FALSE;
	int iRetVal = GC_EVENT_ERROR;
	SESSION_INFO *si = NULL;
	BOOL fFreeText = FALSE;

	if (CMimAPI::m_shutDown)
		return 0;

	if (gce == NULL)
		return GC_EVENT_ERROR;

	gcd = gce->pDest;
	if (gcd == NULL)
		return GC_EVENT_ERROR;

	if (gce->cbSize != SIZEOF_STRUCT_GCEVENT_V1 && gce->cbSize != SIZEOF_STRUCT_GCEVENT_V2)
		return GC_EVENT_WRONGVER;

	if (!IsEventSupported(gcd->iType))
		return GC_EVENT_ERROR;

	if (!(gce->dwFlags & GC_UNICODE)) {
		save_gce = *gce;
		save_gcd = *gce->pDest;
		gce->pDest->ptszID = a2tf(gce->pDest->ptszID, gce->dwFlags, 0);
		gce->ptszUID       = a2tf(gce->ptszUID,       gce->dwFlags, 0);
		gce->ptszNick      = a2tf(gce->ptszNick,      gce->dwFlags, 0);
		gce->ptszStatus    = a2tf(gce->ptszStatus,    gce->dwFlags, 0);
		if (gcd->iType != GC_EVENT_MESSAGE && gcd->iType != GC_EVENT_ACTION) {
			gce->ptszText   = a2tf(gce->ptszText,      gce->dwFlags, 0);
			fFreeText = TRUE;
		}
		gce->ptszUserInfo  = a2tf(gce->ptszUserInfo,  gce->dwFlags, 0);
	}

	EnterCriticalSection(&cs);

	// Do different things according to type of event
	switch (gcd->iType) {
	case GC_EVENT_ADDGROUP:
		{
			STATUSINFO* si = SM_AddStatus(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszStatus);
			if (si && gce->dwItemData)
				si->hIcon = CopyIcon((HICON)gce->dwItemData);
		}
		iRetVal = 0;
		goto LBL_Exit;

		case GC_EVENT_CHUID:
		case GC_EVENT_CHANGESESSIONAME:
		case GC_EVENT_SETITEMDATA:
		case GC_EVENT_GETITEMDATA:
		case GC_EVENT_CONTROL:
		case GC_EVENT_SETSBTEXT:
		case GC_EVENT_ACK:
		case GC_EVENT_SENDMESSAGE :
		case GC_EVENT_SETSTATUSEX :
			iRetVal = DoControl(gce, wParam);
			goto LBL_Exit;

		case GC_EVENT_SETCONTACTSTATUS:
			iRetVal = SM_SetContactStatus(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, (WORD)gce->dwItemData);
			goto LBL_Exit;

		case GC_EVENT_TOPIC: {
			si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
			if (si) {
				if (gce->pszText) {
					replaceStr(&si->ptszTopic, RemoveFormatting(gce->ptszText));
					M->WriteTString(si->hContact, si->pszModule , "Topic", /*RemoveFormatting*/(si->ptszTopic));
					if (M->GetByte("Chat", "TopicOnClist", 1))
						M->WriteTString(si->hContact, "CList" , "StatusMsg", /*RemoveFormatting*/(si->ptszTopic));
					if(si->hWnd)
						SendMessage(si->hWnd, DM_INVALIDATEPANEL, 0, 0);
				}
			}
			break;
		}
		case GC_EVENT_ADDSTATUS:
			SM_GiveStatus(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, gce->ptszStatus);
			if (!gce->bIsMe) {
				bIsHighlighted = g_Settings.Highlight->match(gce, 0, CMUCHighlight::MATCH_NICKNAME);
			}
			break;

		case GC_EVENT_REMOVESTATUS:
			SM_TakeStatus(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID, gce->ptszStatus);
			if (!gce->bIsMe) {
				bIsHighlighted = g_Settings.Highlight->match(gce, 0, CMUCHighlight::MATCH_NICKNAME);
			}
			break;

		case GC_EVENT_MESSAGE:
		case GC_EVENT_ACTION: {
			si = SM_FindSession(gce->pDest->ptszID, gce->pDest->pszModule);
			if (!(gce->dwFlags & GC_UNICODE)) {
				fFreeText = TRUE;
				if (si)
					gce->ptszText = a2tf(gce->ptszText, gce->dwFlags, M->GetDword(si->hContact, "ANSIcodepage", 0));
				else
					gce->ptszText = a2tf(gce->ptszText, gce->dwFlags, 0);
			}
			if (!gce->bIsMe && gce->pDest->pszID && gce->pszText) {
				if (si)
					bIsHighlighted = si->Highlight->match(gce, si, CMUCHighlight::MATCH_TEXT | CMUCHighlight::MATCH_NICKNAME);
			}
		}
		break;

		case GC_EVENT_NICK:
			SM_ChangeNick(gce->pDest->ptszID, gce->pDest->pszModule, gce);
			if (!gce->bIsMe) {
				bIsHighlighted = g_Settings.Highlight->match(gce, 0, CMUCHighlight::MATCH_NICKNAME);
			}

			break;

		case GC_EVENT_JOIN:
			AddUser(gce);
			if (!gce->bIsMe) {
				bIsHighlighted = g_Settings.Highlight->match(gce, 0, CMUCHighlight::MATCH_NICKNAME);
			}
			break;

		case GC_EVENT_PART:
		case GC_EVENT_QUIT:
		case GC_EVENT_KICK:
			bRemoveFlag = TRUE;
			if (!gce->bIsMe) {
				bIsHighlighted = g_Settings.Highlight->match(gce, 0, CMUCHighlight::MATCH_NICKNAME);
			}
			break;
	}

	// Decide which window (log) should have the event
	if (gcd->pszID) {
		pWnd = gcd->ptszID;
		pMod = gcd->pszModule;
	}
	else if ( gcd->iType == GC_EVENT_NOTICE || gcd->iType == GC_EVENT_INFORMATION ) {
		SESSION_INFO* si = GetActiveSession();
		if (si && !lstrcmpA(si->pszModule, gcd->pszModule)) {
			pWnd = si->ptszID;
			pMod = si->pszModule;
		}
		else {
			iRetVal = 0;
			goto LBL_Exit;
		}
	}
	else {
		// Send the event to all windows with a user pszUID. Used for broadcasting QUIT etc
		SM_AddEventToAllMatchingUID(gce, bIsHighlighted);
		if (!bRemoveFlag) {
			iRetVal = 0;
			goto LBL_Exit;
	}	}

	// add to log
	if (pWnd) {
		if (si == NULL)
			si = SM_FindSession(pWnd, pMod);

		// fix for IRC's old stuyle mode notifications. Should not affect any other protocol
		if ((gce->pDest->iType == GC_EVENT_ADDSTATUS || gce->pDest->iType == GC_EVENT_REMOVESTATUS) && !(gce->dwFlags & GCEF_ADDTOLOG)) {
			iRetVal = 0;
			goto LBL_Exit;
		}

		if (gce && gce->pDest->iType == GC_EVENT_JOIN && gce->time == 0) {
			iRetVal = 0;
			goto LBL_Exit;
		}

		if (si && (si->bInitDone || gce->pDest->iType == GC_EVENT_TOPIC || (gce->pDest->iType == GC_EVENT_JOIN && gce->bIsMe))) {
			if (SM_AddEvent(pWnd, pMod, gce, bIsHighlighted) && si->hWnd) {
				SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
			}
			else if (si->hWnd) {
				SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
			}
			DoSoundsFlashPopupTrayStuff(si, gce, bIsHighlighted, 0);
			if ((gce->dwFlags & GCEF_ADDTOLOG) && g_Settings.LoggingEnabled)
				LogToFile(si, gce);
		}

		if (!bRemoveFlag) {
			iRetVal = 0;
			goto LBL_Exit;
	}	}

	if (bRemoveFlag)
		iRetVal = (SM_RemoveUser(gce->pDest->ptszID, gce->pDest->pszModule, gce->ptszUID) == 0) ? 1 : 0;

LBL_Exit:
	LeaveCriticalSection(&cs);

	if (!(gce->dwFlags & GC_UNICODE)) {
		if (fFreeText)
			mir_free((void*)gce->ptszText);
		mir_free((void*)gce->ptszNick);
		mir_free((void*)gce->ptszUID);
		mir_free((void*)gce->ptszStatus);
		mir_free((void*)gce->ptszUserInfo);
		mir_free((void*)gce->pDest->ptszID);
		*gce = save_gce;
		*gce->pDest = save_gcd;
	}
	return iRetVal;
}

static INT_PTR Service_GetAddEventPtr(WPARAM wParam, LPARAM lParam)
{
	GCPTRS * gp = (GCPTRS *) lParam;

	EnterCriticalSection(&cs);

	gp->pfnAddEvent = Service_AddEvent;
	LeaveCriticalSection(&cs);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service creation

void HookEvents(void)
{
	InitializeCriticalSection(&cs);
	g_hHookPrebuildMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, CList_PrebuildContactMenu); // MIRANDAHOOK should return INT_PTR too
}

void UnhookEvents(void)
{
	UnhookEvent(g_hHookPrebuildMenu);
	DeleteCriticalSection(&cs);
}

int CreateServiceFunctions(void)
{
	PluginConfig.m_chat_enabled = false;

	if(ServiceExists(MS_GC_REGISTER)) {
		LRESULT result = CWarning::show(CWarning::WARN_CHAT_ENABLED, CWarning::CWF_NOALLOWHIDE | CWarning::CWF_UNTRANSLATED | MB_YESNOCANCEL | MB_ICONQUESTION);
			if(result == IDYES)
				M->WriteByte("PluginDisable", "chat.dll", 1);
		return(0);
	}
	PluginConfig.m_chat_enabled = true;

	hServiceRegister       = CreateServiceFunction(MS_GC_REGISTER,        Service_Register);
	hServiceNewChat        = CreateServiceFunction(MS_GC_NEWSESSION,      Service_NewChat);
	hServiceAddEvent       = CreateServiceFunction(MS_GC_EVENT,           Service_AddEvent);
	hServiceGetAddEventPtr = CreateServiceFunction(MS_GC_GETEVENTPTR,     Service_GetAddEventPtr);
	hServiceGetInfo        = CreateServiceFunction(MS_GC_GETINFO,         Service_GetInfo);
	hServiceGetCount       = CreateServiceFunction(MS_GC_GETSESSIONCOUNT, Service_GetCount);

	hEventDoubleclicked    = CreateServiceFunction("GChat/DblClickEvent",     CList_EventDoubleclicked);
	hEventPrebuildMenu     = CreateServiceFunction("GChat/PrebuildMenuEvent", CList_PrebuildContactMenuSvc);
	hEventJoinChat         = CreateServiceFunction("GChat/JoinChat",          CList_JoinChat);
	hEventLeaveChat        = CreateServiceFunction("GChat/LeaveChat",         CList_LeaveChat);
	return(1);
}

void DestroyServiceFunctions(void)
{
	DestroyServiceFunction(hServiceRegister);
	DestroyServiceFunction(hServiceNewChat);
	DestroyServiceFunction(hServiceAddEvent);
	DestroyServiceFunction(hServiceGetAddEventPtr);
	DestroyServiceFunction(hServiceGetInfo);
	DestroyServiceFunction(hServiceGetCount);
	DestroyServiceFunction(hEventDoubleclicked);
	DestroyServiceFunction(hEventPrebuildMenu);
	DestroyServiceFunction(hEventJoinChat);
	DestroyServiceFunction(hEventLeaveChat);
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

