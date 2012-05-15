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

int g_nStatus = ID_STATUS_OFFLINE;
UINT_PTR timerId = 0;

void SetContactStatus(HANDLE hContact,int nNewStatus)
{
	if(DBGetContactSettingWord(hContact,MODULE,"Status",ID_STATUS_OFFLINE) != nNewStatus)
		DBWriteContactSettingWord(hContact,MODULE,"Status",nNewStatus);
}

static void __cdecl WorkingThread(void* param)
{
	int nStatus = (int)param;
//	UpdateAll(FALSE, FALSE);
	HANDLE hContact= (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) 
	{
		if(IsMyContact(hContact)) 
		{
			SetContactStatus(hContact, nStatus);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

int NewsAggrInit(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact= (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) 
	{
		if(IsMyContact(hContact)) 
		{
			SetContactStatus(hContact, ID_STATUS_OFFLINE);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	NetlibInit();
	InitIcons();
	InitMenu();

	// timer for the first update
	timerId = SetTimer(NULL, 0, 5000, timerProc2);  // first update is 5 sec after load

	return 0;
}

int NewsAggrPreShutdown(WPARAM wParam,LPARAM lParam)
{
	if (hAddFeedDlg)
	{
		SendMessage(hAddFeedDlg, WM_CLOSE, 0, 0);
	}
	if (hChangeFeedDlg)
	{
		SendMessage(hChangeFeedDlg, WM_CLOSE, 0, 0);
	}
	mir_forkthread(WorkingThread, (void*)ID_STATUS_OFFLINE);
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

INT_PTR NewsAggrSetStatus(WPARAM wp,LPARAM /*lp*/)
{
	int nStatus = wp;
	if((ID_STATUS_ONLINE == nStatus) || (ID_STATUS_OFFLINE == nStatus))
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
	HANDLE hContact= (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact != NULL) 
	{
		if(IsMyContact(hContact)) 
		{
			UpdateListAdd(hContact);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, NULL);


	return 0;
}

INT_PTR AddFeed(WPARAM wParam,LPARAM lParam)
{
	hAddFeedDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ADDFEED), NULL, DlgProcAddFeedOpts);
	ShowWindow(hAddFeedDlg, SW_SHOW);
	return 0;
}

INT_PTR ChangeFeed(WPARAM wParam,LPARAM lParam)
{
	hChangeFeedDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), NULL, DlgProcChangeFeedMenu, (LPARAM)wParam);
	ShowWindow(hChangeFeedDlg, SW_SHOW);
	return 0;
}

INT_PTR ImportFeeds(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR ExportFeeds(WPARAM wParam,LPARAM lParam)
{
	return 0;
}

INT_PTR CheckFeed(WPARAM wParam,LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if(IsMyContact(hContact))
		UpdateListAdd(hContact);
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, NULL);
	return 0;
}

INT_PTR NewsAggrGetAvatarInfo(WPARAM wParam,LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*) lParam;

	if(!IsMyContact(pai->hContact))
		return GAIR_NOAVATAR;

	// if GAIF_FORCE is set, we are updating the feed
	// otherwise, cached avatar is used
	if (wParam & GAIF_FORCE)
		UpdateListAdd(pai->hContact);
		//CheckCurrentFeed(pai->hContact);
	if (!ThreadRunning)
		mir_forkthread(UpdateThreadProc, NULL);

	DBVARIANT dbv = {0};
	if(DBGetContactSettingTString(pai->hContact,MODULE,"ImageURL",&dbv))
	{
		return GAIR_NOAVATAR;
	}
	DBFreeVariant(&dbv);
	return GAIR_WAITFOR;
}