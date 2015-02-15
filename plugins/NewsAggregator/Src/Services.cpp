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

void SetContactStatus(MCONTACT hContact, int nNewStatus)
{
	if(db_get_w(hContact, MODULE, "Status", ID_STATUS_ONLINE) != nNewStatus)
		db_set_w(hContact, MODULE, "Status", nNewStatus);
}

int OnFoldersChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathT(hNewsAggregatorFolder, tszRoot, MAX_PATH, _T(""));
	return 0;
}

int NewsAggrInit(WPARAM, LPARAM)
{
	if (hNewsAggregatorFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), LPGEN("News Aggregator"), MIRANDA_USERDATAT _T("\\Avatars\\")_T(DEFAULT_AVATARS_FOLDER)))
		FoldersGetCustomPathT(hNewsAggregatorFolder, tszRoot, MAX_PATH, _T(""));
	else
		mir_tstrncpy(tszRoot, VARST( _T("%miranda_userdata%\\Avatars\\"_T(DEFAULT_AVATARS_FOLDER))), SIZEOF(tszRoot));

	for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
		if (!db_get_b(NULL, MODULE, "StartupRetrieve", 1))
			db_set_dw(hContact, MODULE, "LastCheck", time(NULL));
		SetContactStatus(hContact, ID_STATUS_ONLINE);
	}

	NetlibInit();
	InitMenu();

	HookEvent(ME_TTB_MODULELOADED, OnToolbarLoaded);
	HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);

	// timer for the first update
	timerId = SetTimer(NULL, 0, 10000, timerProc2); // first update is 10 sec after load

	return 0;
}

int NewsAggrPreShutdown(WPARAM, LPARAM)
{
	if (hAddFeedDlg)
		SendMessage(hAddFeedDlg, WM_CLOSE, 0, 0);

	WindowList_Broadcast(hChangeFeedDlgList, WM_CLOSE, 0, 0);

	KillTimer(NULL, timerId);
	NetlibUnInit();

	CallService(MS_HOTKEY_UNREGISTER, 0, (LPARAM)"NewsAggregator/CheckAllFeeds");
	return 0;
}

INT_PTR NewsAggrGetName(WPARAM wParam, LPARAM lParam)
{
	if(lParam) {
		mir_strncpy((char *)lParam, MODULE, wParam);
		return 0;
	}

	return 1;
}

INT_PTR NewsAggrGetCaps(WPARAM wp, LPARAM)
{
	switch(wp) {
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

INT_PTR NewsAggrSetStatus(WPARAM wp, LPARAM)
{
	int nStatus = wp;
	if ((ID_STATUS_ONLINE == nStatus) || (ID_STATUS_OFFLINE == nStatus)) {
		int nOldStatus = g_nStatus;
		if(nStatus != g_nStatus) {
			g_nStatus = nStatus;

			for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE))
				SetContactStatus(hContact, nStatus);

			ProtoBroadcastAck(MODULE, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)nOldStatus, g_nStatus);
		}
	}

	return 0;
}

INT_PTR NewsAggrGetStatus(WPARAM, LPARAM)
{
	return g_nStatus;
}

INT_PTR NewsAggrLoadIcon(WPARAM wParam, LPARAM)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx("main", FALSE)) : 0;
}

static void __cdecl AckThreadProc(void *param)
{
	Sleep(100);
	ProtoBroadcastAck(MODULE, (MCONTACT)param, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

INT_PTR NewsAggrGetInfo(WPARAM, LPARAM lParam)
{
	CCSDATA *ccs = (CCSDATA *)lParam;
	mir_forkthread(AckThreadProc, (void*)ccs->hContact);
	return 0;
}

INT_PTR CheckAllFeeds(WPARAM, LPARAM lParam)
{
	for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
		if (lParam && db_get_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME))
			UpdateListAdd(hContact);
		else if (!lParam)
			UpdateListAdd(hContact);
	}
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, 0);

	return 0;
}

INT_PTR AddFeed(WPARAM, LPARAM)
{
	if (hAddFeedDlg == 0)
		hAddFeedDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADDFEED), NULL, DlgProcAddFeedOpts);
	ShowWindow(hAddFeedDlg, SW_SHOW);
	return 0;
}

INT_PTR ChangeFeed(WPARAM hContact, LPARAM)
{
	HWND hChangeFeedDlg = WindowList_Find(hChangeFeedDlgList, hContact);
	if (!hChangeFeedDlg) {
		hChangeFeedDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), NULL, DlgProcChangeFeedMenu, hContact);
		ShowWindow(hChangeFeedDlg, SW_SHOW);
	} else {
		SetForegroundWindow(hChangeFeedDlg);
		SetFocus(hChangeFeedDlg);
	}
	return 0;
}

INT_PTR ImportFeeds(WPARAM, LPARAM)
{
	CreateDialog(hInst, MAKEINTRESOURCE(IDD_FEEDIMPORT), NULL, DlgProcImportOpts);
	return 0;
}

INT_PTR ExportFeeds(WPARAM, LPARAM)
{
	CreateDialog(hInst, MAKEINTRESOURCE(IDD_FEEDEXPORT), NULL, DlgProcExportOpts);
	return 0;
}

INT_PTR CheckFeed(WPARAM hContact, LPARAM)
{
	if(IsMyContact(hContact))
		UpdateListAdd(hContact);
	if ( !ThreadRunning)
		mir_forkthread(UpdateThreadProc, FALSE);
	return 0;
}

INT_PTR NewsAggrGetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT *pai = (PROTO_AVATAR_INFORMATIONT *)lParam;
	if (!IsMyContact(pai->hContact))
		return GAIR_NOAVATAR;

	// if GAIF_FORCE is set, we are updating the feed
	// otherwise, cached avatar is used
	if ((wParam & GAIF_FORCE) && db_get_dw(pai->hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME))
		UpdateListAdd(pai->hContact);
	if (db_get_b(NULL, MODULE, "AutoUpdate", 1) != 0 && !ThreadRunning)
		mir_forkthread(UpdateThreadProc, (void *)TRUE);

	TCHAR *ptszImageURL = db_get_tsa(pai->hContact, MODULE, "ImageURL");
	if(ptszImageURL == NULL)
		return GAIR_NOAVATAR;

	mir_free(ptszImageURL);
	return GAIR_WAITFOR;
}

INT_PTR NewsAggrRecvMessage(WPARAM, LPARAM lParam)
{
	CallService(MS_PROTO_RECVMSG, 0, lParam);
	return 0;
}

void UpdateMenu(bool State)
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

	mi.flags = CMIM_ICON | CMIM_NAME | CMIF_TCHAR;
	Menu_ModifyItem(hService2[0], &mi);
	CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTBButton, State ? TTBST_PUSHED : 0);
	db_set_b(NULL, MODULE, "AutoUpdate", !State);
}

// update the newsaggregator auto-update menu item when click on it
INT_PTR EnableDisable(WPARAM, LPARAM)
{
	UpdateMenu(db_get_b(NULL, MODULE, "AutoUpdate", 1) != 0);
	NewsAggrSetStatus(db_get_b(NULL, MODULE, "AutoUpdate", 1) ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE, 0);
	return 0;
}

int OnToolbarLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.name = LPGEN("Enable/disable auto update");
	ttb.pszService = MS_NEWSAGGREGATOR_ENABLED;
	ttb.pszTooltipUp = LPGEN("Auto Update Enabled");
	ttb.pszTooltipDn = LPGEN("Auto Update Disabled");
	ttb.hIconHandleUp = GetIconHandle("enabled");
	ttb.hIconHandleDn = GetIconHandle("disabled");
	ttb.dwFlags = (db_get_b(NULL, MODULE, "AutoUpdate", 1) ? 0 : TTBBF_PUSHED) | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
	hTBButton = TopToolbar_AddButton(&ttb);
	return 0;
}
