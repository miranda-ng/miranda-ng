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
// Module to Request Away Messages

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_awaymsg.h"
#include "newpluginapi.h"
#include "hdr/modern_sync.h"

#define AMASKPERIOD 3000
#define amlock EnterCriticalSection(&amLockChain)
#define amunlock LeaveCriticalSection(&amLockChain)

typedef struct _tag_amChainItem {
	HANDLE hContact;
	_tag_amChainItem *Next;
} AMCHAINITEM;

static AMCHAINITEM *	amFirstChainItem = NULL;
static AMCHAINITEM *	amLastChainItem  = NULL;
static CRITICAL_SECTION	amLockChain;
static HANDLE			hamProcessEvent	 = NULL;
static DWORD			amRequestTick	 = 0;


static int		amAddHandleToChain(HANDLE hContact);
static HANDLE	amGetCurrentChain();
static int		amThreadProc(HWND hwnd);

/*
*  Add contact handle to requests queue
*/
static int amAddHandleToChain(HANDLE hContact)
{
	AMCHAINITEM * workChain;
	amlock;
	{
		//check that handle is present
		AMCHAINITEM * wChain;
		wChain=amFirstChainItem;
		if (wChain)
			do {
				if (wChain->hContact==hContact)
				{
					amunlock;
					return 0;
				}
			} while(wChain=(AMCHAINITEM *)wChain->Next);
	}
	if (!amFirstChainItem)  
	{
		amFirstChainItem=(AMCHAINITEM*)malloc(sizeof(AMCHAINITEM));
		workChain=amFirstChainItem;
	}
	else 
	{
		amLastChainItem->Next=(AMCHAINITEM*)malloc(sizeof(AMCHAINITEM));
		workChain=(AMCHAINITEM *)amLastChainItem->Next;
	}
	amLastChainItem=workChain;
	workChain->Next=NULL;
	workChain->hContact=hContact;
	amunlock;
	SetEvent(hamProcessEvent);
	return 1;
}


/*
*	Gets handle from queue for request
*/
static HANDLE amGetCurrentChain()
{
	AMCHAINITEM * workChain;
	HANDLE res=NULL;
	amlock;
	if (amFirstChainItem)
	{
		res=amFirstChainItem->hContact;
		workChain=amFirstChainItem->Next;
		free(amFirstChainItem);
		amFirstChainItem=(AMCHAINITEM *)workChain;
	}
	amunlock;
	return res;
}

/*
*	Tread sub to ask protocol to retrieve away message
*/
static int amThreadProc(HWND hwnd)
{
	DWORD time;
	HANDLE hContact;
	HANDLE ACK=0;
	displayNameCacheEntry dnce;
	memset( &dnce, 0, sizeof(dnce));

	while (!MirandaExiting())
	{
		hContact=amGetCurrentChain(); 
		while (hContact)
		{ 
			time=GetTickCount();
			if ((time-amRequestTick)<AMASKPERIOD)
			{
				SleepEx(AMASKPERIOD-(time-amRequestTick)+10,TRUE);
				if (MirandaExiting())
				{
					g_dwAwayMsgThreadID=0;
					return 0; 
				}
			}
			CListSettings_FreeCacheItemData(&dnce);
			dnce.m_cache_hContact=(HANDLE)hContact;
			Sync(CLUI_SyncGetPDNCE, (WPARAM) 0,(LPARAM)&dnce);            
			if (dnce.ApparentMode!=ID_STATUS_OFFLINE) //don't ask if contact is always invisible (should be done with protocol)
				ACK=(HANDLE)CallContactService(hContact,PSS_GETAWAYMSG,0,0);		
			if (!ACK)
			{
				ACKDATA ack;
				ack.hContact=hContact;
				ack.type=ACKTYPE_AWAYMSG;
				ack.result=ACKRESULT_FAILED;
				if (dnce.m_cache_cszProto)
					ack.szModule=dnce.m_cache_cszProto;
				else
					ack.szModule=NULL;
				ClcDoProtoAck(hContact, &ack);
			}
			CListSettings_FreeCacheItemData(&dnce);
			amRequestTick=time;
			hContact=amGetCurrentChain();
			if (hContact) 
			{
				DWORD i=0;
				do 
				{
					i++;
					SleepEx(50,TRUE);
				} while (i<AMASKPERIOD/50&&!MirandaExiting());
			}
			else break;
			if (MirandaExiting()) 
			{	
				g_dwAwayMsgThreadID=0;
				return 0;			
			}
		}
		WaitForSingleObjectEx(hamProcessEvent, INFINITE, TRUE);
		ResetEvent(hamProcessEvent);
		if (MirandaExiting()) 
		{
			g_dwAwayMsgThreadID=0;
			return 0;
		}
	}
	g_dwAwayMsgThreadID=0;
	return 1;
}

BOOL amWakeThread()
{
	if (hamProcessEvent && g_dwAwayMsgThreadID)
	{
		SetEvent(hamProcessEvent);

		return TRUE;
	}

	return FALSE;
}

/*
*	Sub to be called outside on status changing to retrieve away message
*/
void amRequestAwayMsg(HANDLE hContact)
{
	char *szProto;
	if (!g_CluiData.bInternalAwayMsgDiscovery || !hContact) 
		return;
	//Do not re-ask for chat rooms   
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (szProto == NULL || ModernGetSettingByte(hContact, szProto, "ChatRoom", 0) != 0) 	
		return;
	amAddHandleToChain(hContact);        
}

void InitAwayMsgModule()
{
	InitializeCriticalSection(&amLockChain);
	hamProcessEvent=CreateEvent(NULL,FALSE,FALSE,NULL);   
	g_dwAwayMsgThreadID=(DWORD)mir_forkthread((pThreadFunc)amThreadProc,0);
}

void UninitAwayMsgModule()
{
	SetEvent(hamProcessEvent);
	CloseHandle(hamProcessEvent);
	amlock;
	while (amGetCurrentChain());
	amunlock;
	DeleteCriticalSection(&amLockChain);
}
