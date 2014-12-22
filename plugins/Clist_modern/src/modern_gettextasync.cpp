/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

/*
*	Author Artem Shpynov aka FYR
*   Copyright 2000-2008 Artem Shpynov
*/

//////////////////////////////////////////////////////////////////////////
// Module to async parsing of texts

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_gettextasync.h"
#include "newpluginapi.h"
#include "hdr/modern_sync.h"

int CLUI_SyncSetPDNCE(WPARAM wParam, LPARAM lParam);
int CLUI_SyncGetShortData(WPARAM wParam, LPARAM lParam);

typedef struct _GetTextAsyncItem {
	MCONTACT hContact;
	ClcData *dat;
	struct _GetTextAsyncItem *Next;
} GTACHAINITEM;

static GTACHAINITEM *gtaFirstItem = NULL;
static GTACHAINITEM *gtaLastItem = NULL;
static mir_cs gtaCS;
static HANDLE hgtaWakeupEvent = NULL;

static BOOL gtaGetItem(GTACHAINITEM *mpChain)
{
	if (!mpChain)
		return FALSE;

	mir_cslock lck(gtaCS);
	if (!gtaFirstItem)
		return FALSE;

	GTACHAINITEM *ch = gtaFirstItem;
	*mpChain = *ch;
	gtaFirstItem = (GTACHAINITEM*)ch->Next;
	if (!gtaFirstItem)
		gtaLastItem = NULL;
	free(ch);
	return TRUE;
}

static void gtaThreadProc(void*)
{
	Netlib_Logf(NULL, "GTA thread start");

	SHORTDATA data = { 0 };

	while (!MirandaExiting()) {
		Sync(CLUI_SyncGetShortData, (WPARAM)pcli->hwndContactTree, (LPARAM)&data);
		while (true) {
			if (MirandaExiting())
				goto LBL_Exit;

			SleepEx(0, TRUE); //1000 contacts per second

			GTACHAINITEM mpChain = { 0 };
			struct SHORTDATA dat2 = { 0 };
			if (!gtaGetItem(&mpChain))
				break;

			SHORTDATA *dat;
			if (mpChain.dat == NULL || (!IsBadReadPtr(mpChain.dat, sizeof(mpChain.dat)) && mpChain.dat->hWnd == data.hWnd))
				dat = &data;
			else {
				Sync(CLUI_SyncGetShortData, (WPARAM)mpChain.dat->hWnd, (LPARAM)&dat2);
				dat = &dat2;
			}
			if (MirandaExiting())
				goto LBL_Exit;

			ClcCacheEntry cacheEntry;
			memset(&cacheEntry, 0, sizeof(cacheEntry));
			cacheEntry.hContact = mpChain.hContact;
			if (!Sync(CLUI_SyncGetPDNCE, (WPARAM)0, (LPARAM)&cacheEntry)) {
				Cache_GetSecondLineText(dat, &cacheEntry);
				Cache_GetThirdLineText(dat, &cacheEntry);
				Sync(CLUI_SyncSetPDNCE, (WPARAM)CCI_LINES, (LPARAM)&cacheEntry);
				CListSettings_FreeCacheItemData(&cacheEntry);
			}

			KillTimer(dat->hWnd, TIMERID_INVALIDATE_FULL);
			CLUI_SafeSetTimer(dat->hWnd, TIMERID_INVALIDATE_FULL, 500, NULL);
		}

		WaitForSingleObjectEx(hgtaWakeupEvent, INFINITE, TRUE);
		ResetEvent(hgtaWakeupEvent);
	}

LBL_Exit:
	CloseHandle(hgtaWakeupEvent);
	hgtaWakeupEvent = NULL;
	g_hGetTextAsyncThread = NULL;
	Netlib_Logf(NULL, "GTA thread end");
}

BOOL gtaWakeThread()
{
	if (hgtaWakeupEvent && g_hGetTextAsyncThread) {
		SetEvent(hgtaWakeupEvent);
		return TRUE;
	}

	return FALSE;
}

int gtaAddRequest(ClcData *dat, MCONTACT hContact)
{
	if (MirandaExiting()) return 0;

	mir_cslock lck(gtaCS);

	GTACHAINITEM *mpChain = (GTACHAINITEM*)malloc(sizeof(GTACHAINITEM));
	mpChain->hContact = hContact;
	mpChain->dat = dat;
	mpChain->Next = NULL;
	if (gtaLastItem) {
		gtaLastItem->Next = (GTACHAINITEM*)mpChain;
		gtaLastItem = mpChain;
	}
	else {
		gtaFirstItem = mpChain;
		gtaLastItem = mpChain;
		SetEvent(hgtaWakeupEvent);
	}

	return FALSE;
}

void gtaRenewText(MCONTACT hContact)
{
	gtaAddRequest(NULL, hContact);
}

void gtaShutdown()
{
	SetEvent(hgtaWakeupEvent);
}

void InitCacheAsync()
{
	hgtaWakeupEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hGetTextAsyncThread = mir_forkthread(gtaThreadProc, 0);
}
