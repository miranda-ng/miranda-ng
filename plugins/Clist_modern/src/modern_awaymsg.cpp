/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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
// Module to Request Away Messages

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_awaymsg.h"
#include "hdr/modern_sync.h"

#define AMASKPERIOD 3000

static mir_cs amCS;
static LIST<void> amItems(10, PtrKeySortT);

static HANDLE  hamProcessEvent = NULL;
static DWORD   amRequestTick = 0;

/*
*  Add contact handle to requests queue
*/
static int amAddHandleToChain(MCONTACT hContact)
{
	mir_cslockfull lck(amCS);
	if (amItems.find((HANDLE)hContact) != NULL)
		return 0;

	amItems.insert((HANDLE)hContact);
	lck.unlock();
	SetEvent(hamProcessEvent);
	return 1;
}

/*
*	Gets handle from queue for request
*/
static MCONTACT amGetCurrentChain()
{
	mir_cslock lck(amCS);
	if (amItems.getCount() == 0)
		return NULL;

	MCONTACT res = (MCONTACT)amItems[0];
	amItems.remove(0);
	return res;
}

/*
*	Tread sub to ask protocol to retrieve away message
*/
static void amThreadProc(void *)
{
	Netlib_Logf(NULL, "amThreadProc thread start");

	ClcCacheEntry dnce;
	memset(&dnce, 0, sizeof(dnce));

	while (!MirandaExiting()) {
		MCONTACT hContact = amGetCurrentChain();
		while (hContact) {
			DWORD time = GetTickCount();
			if ((time - amRequestTick) < AMASKPERIOD) {
				SleepEx(AMASKPERIOD - (time - amRequestTick) + 10, TRUE);
				if (MirandaExiting())
					goto LBL_Exit;
			}
			CListSettings_FreeCacheItemData(&dnce);
			dnce.hContact = hContact;
			Sync(CLUI_SyncGetPDNCE, (WPARAM)0, (LPARAM)&dnce);

			HANDLE ACK = 0;
			if (dnce.ApparentMode != ID_STATUS_OFFLINE) //don't ask if contact is always invisible (should be done with protocol)
				ACK = (HANDLE)CallContactService(hContact, PSS_GETAWAYMSG, 0, 0);
			if (!ACK) {
				ACKDATA ack;
				ack.hContact = hContact;
				ack.type = ACKTYPE_AWAYMSG;
				ack.result = ACKRESULT_FAILED;
				if (dnce.m_cache_cszProto)
					ack.szModule = dnce.m_cache_cszProto;
				else
					ack.szModule = NULL;
				ClcDoProtoAck(hContact, &ack);
			}
			CListSettings_FreeCacheItemData(&dnce);
			amRequestTick = time;
			hContact = amGetCurrentChain();
			if (hContact) {
				DWORD i = 0;
				do {
					i++;
					SleepEx(50, TRUE);
				} while (i < AMASKPERIOD / 50 && !MirandaExiting());
			}
			else break;
			if (MirandaExiting())
				goto LBL_Exit;
		}
		WaitForSingleObjectEx(hamProcessEvent, INFINITE, TRUE);
		ResetEvent(hamProcessEvent);
		if (MirandaExiting())
			break;
	}

LBL_Exit:
	g_hAwayMsgThread = NULL;
	Netlib_Logf(NULL, "amThreadProc thread end");
}

BOOL amWakeThread()
{
	if (hamProcessEvent && g_hAwayMsgThread) {
		SetEvent(hamProcessEvent);
		return TRUE;
	}

	return FALSE;
}

/*
*	Sub to be called outside on status changing to retrieve away message
*/
void amRequestAwayMsg(MCONTACT hContact)
{
	if (!g_CluiData.bInternalAwayMsgDiscovery || !hContact)
		return;

	//Do not re-ask for chat rooms
	char *szProto = GetContactProto(hContact);
	if (szProto != NULL && !db_get_b(hContact, szProto, "ChatRoom", 0))
		amAddHandleToChain(hContact);
}

void InitAwayMsgModule()
{
	hamProcessEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hAwayMsgThread = mir_forkthread(amThreadProc, 0);
}

void UninitAwayMsgModule()
{
	SetEvent(hamProcessEvent);

	while (g_hAwayMsgThread)
		SleepEx(50, TRUE);

	CloseHandle(hamProcessEvent);
}
