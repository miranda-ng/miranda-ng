/*

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#include "commonheaders.h"

static LIST<TMsgQueue> msgQueue(5, NumericKeySortT);
static CRITICAL_SECTION csMsgQueue;
static UINT_PTR timerId;

void MessageFailureProcess(TMsgQueue *item, const char* err);

static VOID CALLBACK MsgTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	LIST<TMsgQueue> arTimedOut(1);
	{
		mir_cslock lck(csMsgQueue);
		for (int i = msgQueue.getCount() - 1; i >= 0; i--) {
			TMsgQueue *item = msgQueue[i];
			if (dwTime - item->ts > g_dat.msgTimeout) {
				arTimedOut.insert(item);
				msgQueue.remove(i);
			}
		}
	}

	for (int i = 0; i < arTimedOut.getCount(); ++i)
		MessageFailureProcess(arTimedOut[i], LPGEN("The message send timed out."));
}

void msgQueue_add(MCONTACT hContact, int id, const TCHAR* szMsg, HANDLE hDbEvent)
{
	TMsgQueue *item = (TMsgQueue*)mir_alloc(sizeof(TMsgQueue));
	item->hContact = hContact;
	item->id = id;
	item->szMsg = mir_tstrdup(szMsg);
	item->hDbEvent = hDbEvent;
	item->ts = GetTickCount();

	mir_cslock lck(csMsgQueue);
	if (!msgQueue.getCount() && !timerId)
		timerId = SetTimer(NULL, 0, 5000, MsgTimer);
	msgQueue.insert(item);
}

void msgQueue_processack(MCONTACT hContact, int id, BOOL success, const char* szErr)
{
	MCONTACT hMeta = db_mc_getMeta(hContact);

	mir_cslockfull lck(csMsgQueue);
	for (int i = 0; i < msgQueue.getCount(); i++) {
		TMsgQueue *item = msgQueue[i];
		if ((item->hContact == hContact || item->hContact == hMeta) && item->id == id) {
			msgQueue.remove(i); i--;

			if (!msgQueue.getCount() && timerId) {
				KillTimer(NULL, timerId);
				timerId = 0;
			}
			lck.unlock();

			if (success) {
				mir_free(item->szMsg);
				mir_free(item);
			}
			else MessageFailureProcess(item, szErr);
			break;
		}
	}
}

void msgQueue_init(void)
{
	InitializeCriticalSection(&csMsgQueue);
}

void msgQueue_destroy(void)
{
	for (int i = 0; i < msgQueue.getCount(); i++) {
		TMsgQueue *item = msgQueue[i];
		mir_free(item->szMsg);
		mir_free(item);
	}
	msgQueue.destroy();

	DeleteCriticalSection(&csMsgQueue);
}
