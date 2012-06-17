/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#define gtalock EnterCriticalSection(&gtaCS)
#define gtaunlock LeaveCriticalSection( &gtaCS )

typedef struct _GetTextAsyncItem {
	HANDLE hContact;
	struct ClcData *dat;
	struct _GetTextAsyncItem *Next;
} GTACHAINITEM;

static GTACHAINITEM * gtaFirstItem=NULL;
static GTACHAINITEM * gtaLastItem=NULL;
static CRITICAL_SECTION gtaCS;
static HANDLE hgtaWakeupEvent=NULL;


static BOOL gtaGetItem(GTACHAINITEM * mpChain)
{
	gtalock;
	if (!gtaFirstItem)
	{
		gtaunlock;
		return FALSE;
	}
	else if (mpChain)
	{
		GTACHAINITEM * ch;
		ch=gtaFirstItem;
		*mpChain=*ch;
		gtaFirstItem=(GTACHAINITEM *)ch->Next;
		if (!gtaFirstItem) gtaLastItem=NULL;
		free(ch);
		gtaunlock;
		return TRUE;
	}
	gtaunlock;
	return FALSE;
}

static int gtaThreadProc(void * lpParam)
{
	BOOL exit=FALSE;
	HWND hwnd=pcli->hwndContactList;
	struct SHORTDATA data={0};
	struct SHORTDATA * dat;

	while (!MirandaExiting())
	{
		Sync(CLUI_SyncGetShortData,(WPARAM)pcli->hwndContactTree,(LPARAM)&data);       
		do
		{
			if (!MirandaExiting()) 
				SleepEx(0,TRUE); //1000 contacts per second
			if (MirandaExiting()) 
			{
				g_dwGetTextAsyncThreadID=0;
				return 0;
			}
			else
			{
				GTACHAINITEM mpChain={0};
				struct SHORTDATA dat2={0};
				if (!gtaGetItem(&mpChain)) break;
				if (mpChain.dat==NULL || (!IsBadReadPtr(mpChain.dat,sizeof(mpChain.dat)) && mpChain.dat->hWnd==data.hWnd))	dat=&data;
				else
				{        
					Sync(CLUI_SyncGetShortData,(WPARAM)mpChain.dat->hWnd,(LPARAM)&dat2);       
					dat=&dat2;
				}
				if (!MirandaExiting())
				{
					displayNameCacheEntry cacheEntry;
					memset( &cacheEntry, 0, sizeof(cacheEntry));
					cacheEntry.m_cache_hContact=mpChain.hContact;
					if (!Sync(CLUI_SyncGetPDNCE, (WPARAM) 0,(LPARAM)&cacheEntry))
					{
						if (!MirandaExiting()) 
							Cache_GetSecondLineText(dat, &cacheEntry);
						if (!MirandaExiting()) 
							Cache_GetThirdLineText(dat, &cacheEntry);
						if (!MirandaExiting()) 
							Sync(CLUI_SyncSetPDNCE, (WPARAM) CCI_LINES,(LPARAM)&cacheEntry);  
						CListSettings_FreeCacheItemData(&cacheEntry);
					}
				}
				else
				{	
					g_dwGetTextAsyncThreadID=0;
					return 0;
				}
				KillTimer(dat->hWnd,TIMERID_INVALIDATE_FULL);
				CLUI_SafeSetTimer(dat->hWnd,TIMERID_INVALIDATE_FULL,500,NULL);
			}
		}
		while (!exit);

		WaitForSingleObjectEx(hgtaWakeupEvent, INFINITE, FALSE );
		ResetEvent(hgtaWakeupEvent);
	}
	g_dwGetTextAsyncThreadID=0;
	return 1;
}

BOOL gtaWakeThread()
{
	if (hgtaWakeupEvent && g_dwGetTextAsyncThreadID)
	{
		SetEvent(hgtaWakeupEvent);

		return TRUE;
	}

	return FALSE;
}

int gtaAddRequest(struct ClcData *dat,struct ClcContact *contact,HANDLE hContact)
{
	if (MirandaExiting()) return 0;
	gtalock;    
	{
		GTACHAINITEM * mpChain=(GTACHAINITEM *)malloc(sizeof(GTACHAINITEM));
		mpChain->hContact=hContact;
		mpChain->dat=dat;
		mpChain->Next=NULL;
		if (gtaLastItem) 
		{
			gtaLastItem->Next=(GTACHAINITEM *)mpChain;
			gtaLastItem=mpChain;
		}
		else 
		{
			gtaFirstItem=mpChain;
			gtaLastItem=mpChain;
			SetEvent(hgtaWakeupEvent);
		}
	}
	gtaunlock;
	return FALSE;
}
void gtaRenewText(HANDLE hContact)
{
	gtaAddRequest(NULL,NULL, hContact);
}
int gtaOnModulesUnload(WPARAM wParam,LPARAM lParam)
{
	SetEvent(hgtaWakeupEvent);
	return 0;
}
void InitCacheAsync()
{
	InitializeCriticalSection(&gtaCS);
	hgtaWakeupEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	g_dwGetTextAsyncThreadID=(DWORD)mir_forkthread((pThreadFunc)gtaThreadProc,0);
	ModernHookEvent(ME_SYSTEM_PRESHUTDOWN,  gtaOnModulesUnload);
}

void UninitCacheAsync()
{
	GTACHAINITEM mpChain;
	SetEvent(hgtaWakeupEvent);
	CloseHandle(hgtaWakeupEvent);
	gtalock;
	while(gtaGetItem(&mpChain));
	gtaunlock;
	DeleteCriticalSection(&gtaCS);
}
