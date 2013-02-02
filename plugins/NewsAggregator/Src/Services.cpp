/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

int g_nStatus = ID_STATUS_ONLINE;
UINT_PTR timerId = 0;
HANDLE hTBButton = NULL, hNewsAggregatorFolder = NULL;

void SetContactStatus(HANDLE hContact, int nNewStatus)
{
	if(DBGetContactSettingWord(hContact, MODULE, "Status", ID_STATUS_ONLINE) != nNewStatus)
		DBWriteContactSettingWord(hContact, MODULE, "Status", nNewStatus);
}

static void __cdecl WorkingThread(void* param)
{
	int nStatus = (int)param;
	HANDLE hContact= db_find_first();
	while (hContact != NULL)
	{
		if(IsMyContact(hContact))
		{
			SetContactStatus(hContact, nStatus);
		}
		hContact = db_find_next(hContact);
	}
}

int OnFoldersChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathT(hNewsAggregatorFolder, tszRoot, MAX_PATH, _T(""));
	return 0;
}

int NewsAggrInit(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
	{
		hNewsAggregatorFolder = FoldersRegisterCustomPathT("News Aggregator", "Avatars", MIRANDA_USERDATAT _T("\\Avatars\\")_T(DEFAULT_AVATARS_FOLDER));
		FoldersGetCustomPathT(hNewsAggregatorFolder, tszRoot, MAX_PATH, _T(""));
	}
	else
	{
		TCHAR* tszFolder = Utils_ReplaceVarsT(_T("%miranda_userdata%\\"_T(DEFAULT_AVATARS_FOLDER)));
		lstrcpyn(tszRoot, tszFolder, SIZEOF(tszRoot));
		mir_free(tszFolder);
	}

	HANDLE hContact = db_find_first();
	while (hContact != NULL)
	{
		if(IsMyContact(hContact))
		{
			if (!db_get_b(NULL, MODULE, "StartupRetrieve", 1))
				db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
			SetContactStatus(hContact, ID_STATUS_ONLINE);
		}
		hContact = db_find_next(hContact);
	}
	NetlibInit();
	InitIcons();
	InitMenu();

	HookEvent(ME_TTB_MODULELOADED, OnToolbarLoaded);
	HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);

	// timer for the first update
	timerId = SetTimer(NULL, 0, 10000, timerProc2); // first update is 10 sec after load

	return 0;
}

int NewsAggrPreShutdown(WPARAM wParam,LPARAM lParam)
{
	if (hAddFeedDlg)
	{
		SendMessage(hAddFeedDlg, WM_CLOSE, 0, 0);
	}
	WindowList_Broadcast(hChangeFeedDlgList, WM_CLOSE, 0, 0);

	KillTimer(NULL, timerId);
	NetlibUnInit();

	return 0;
}

INT_PTR NewsAggrGetName(WPARAM wParam, LPARAM lParam)
{
	if(lParam)
	{
		lstrcpynA((char*)lParam, MODULE, wParam);
		return 0;
	}
	else
	{
		return 1;
	}
}

INT_PTR NewsAggrGetCaps(WPARAM wp,LPARAM lp)
{
	switch(wp)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_PEER2PEER;
	case PFLAGNUM_3:
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_AVATARS;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR) "News Feed";
	case PFLAG_UNIQUEIDSETTING:
		return (INT_PTR) "URL";
	default:
		return 0;
	}
}

INT_PTR NewsAggrSetStatus(WPARAM wp, LPARAM /*lp*/)
{
	int nStatus = wp;
	if ((ID_STATUS_ONLINE == nStatus) || (ID_STATUS_OFFLINE == nStatus))
	{
		int nOldStatus = g_nStatus;
		if(nStatus != g_nStatus)
		{
			g_nStatus = nStatus;
			mir_forkthread(WorkingThread, (void*)g_nStatus);
			ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)nOldStatus, g_nStatus);
		}

	}

	return 0;
}

INT_PTR NewsAggrGetStatus(WPARAM/* wp*/,LPARAM/* lp*/)
{
	return g_nStatus;
}

INT_PTR NewsAggrLoadIcon(WPARAM wParam,LPARAM lParam)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx("main", FALSE)) : 0;
}

static void __cdecl AckThreadProc(HANDLE param)
{
	Sleep(100);
	ProtoBroadcastAck(MODULE, param, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
}

INT_PTR NewsAggrGetInfo(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *) lParam;
	mir_forkthread(AckThreadProc, ccs->hContact);
	return 0;
}

INT_PTR CheckAllFeeds(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = db_find_first();
	while (hContact != NULL)
	{
		if (IsMyContact(hContact) && lParam && DBGetContactSettingDword(hContact, MODULE, "UpdateTime", 60))
			UpdateListAdd(hContact);
		else if (IsMyContact(hContact) && !lParam)
			UpdateListAdd(hContact);
		hContact = db_find_next(hContact);
	}
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, (LPVOID)FALSE);

	return 0;
}

INT_PTR AddFeed(WPARAM wParam,LPARAM lParam)
{
	hAddFeedDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADDFEED), NULL, DlgProcAddFeedOpts);
	ShowWindow(hAddFeedDlg, SW_SHOW);
	return 0;
}

INT_PTR ChangeFeed(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	HWND hChangeFeedDlg = WindowList_Find(hChangeFeedDlgList,hContact);
	if (!hChangeFeedDlg)
	{
		hChangeFeedDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), NULL, DlgProcChangeFeedMenu, (LPARAM)hContact);
		ShowWindow(hChangeFeedDlg, SW_SHOW);
	}
	else
	{
		SetForegroundWindow(hChangeFeedDlg);
		SetFocus(hChangeFeedDlg);
	}
	return 0;
}

INT_PTR ImportFeeds(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR ExportFeeds(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR CheckFeed(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if(IsMyContact(hContact))
		UpdateListAdd(hContact);
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, (LPVOID)FALSE);
	return 0;
}

INT_PTR NewsAggrGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT* pai = (PROTO_AVATAR_INFORMATIONT*) lParam;

	if (!IsMyContact(pai->hContact))
		return GAIR_NOAVATAR;

	// if GAIF_FORCE is set, we are updating the feed
	// otherwise, cached avatar is used
	if (wParam & GAIF_FORCE && DBGetContactSettingDword(pai->hContact, MODULE, "UpdateTime", 60))
		UpdateListAdd(pai->hContact);
	if (db_get_b(NULL, MODULE, "AutoUpdate", 1) != 0 && !ThreadRunning)
		mir_forkthread(UpdateThreadProc, (LPVOID)TRUE);

	DBVARIANT dbv = {0};
	if(DBGetContactSettingTString(pai->hContact, MODULE, "ImageURL", &dbv))
	{
		return GAIR_NOAVATAR;
	}
	DBFreeVariant(&dbv);
	return GAIR_WAITFOR;
}

INT_PTR NewsAggrRecvMessage(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_PROTO_RECVMSG, 0, lParam);
	return 0;
}

void UpdateMenu(BOOL State)
{
	CLISTMENUITEM mi = { sizeof(mi) };

	if (!State) { // to enable auto-update
		mi.ptszName = LPGENT("Auto Update Enabled");
		mi.icolibItem = GetIconHandle("enabled");
	}
	else { // to disable auto-update
		mi.ptszName = LPGENT("Auto Update Disabled");
		mi.icolibItem = GetIconHandle("disabled");
	}

	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_ICONFROMICOLIB | CMIF_TCHAR;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hService2[0], (LPARAM)&mi);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTBButton, State ? TTBST_PUSHED : TTBST_RELEASED);
	db_set_b(NULL, MODULE, "AutoUpdate", !State);
}

// update the newsaggregator auto-update menu item when click on it
INT_PTR EnableDisable(WPARAM wParam, LPARAM lParam)
{
	UpdateMenu(db_get_b(NULL, MODULE, "AutoUpdate", 1));
	return 0;
}

int OnToolbarLoaded(WPARAM wParam, LPARAM lParam)
{
	TTBButton tbb = {0};
	tbb.cbSize = sizeof(TTBButton);
	tbb.name = LPGEN("Enable/disable auto update");
	tbb.pszService = MS_NEWSAGGREGATOR_ENABLED;
	tbb.pszTooltipUp = LPGEN("Auto Update Enabled");
	tbb.pszTooltipDn = LPGEN("Auto Update Disabled");
	tbb.hIconHandleUp = GetIconHandle("enabled");
	tbb.hIconHandleDn = GetIconHandle("disabled");
	tbb.dwFlags = (db_get_b(NULL, MODULE, "AutoUpdate", 1) ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
	hTBButton = TopToolbar_AddButton(&tbb);
	return 0;
}