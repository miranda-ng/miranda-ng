/*

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

#include "stdafx.h"

static LIST<TMsgQueue> msgQueue(5, NumericKeySortT);
static mir_cs csMsgQueue;
static UINT_PTR timerId;

void MessageFailureProcess(TMsgQueue *item, const wchar_t *err);

static VOID CALLBACK MsgTimer(HWND, UINT, UINT_PTR, DWORD dwTime)
{
	LIST<TMsgQueue> arTimedOut(1);
	{
		mir_cslock lck(csMsgQueue);

		for (auto &it : msgQueue.rev_iter())
			if (dwTime - it->ts > g_dat.msgTimeout) {
				arTimedOut.insert(it);
				msgQueue.removeItem(&it);
			}
	}

	for (auto &it : arTimedOut)
		MessageFailureProcess(it, TranslateT("The message send timed out."));
}

void msgQueue_add(MCONTACT hContact, int id, char *szMsg, int flags)
{
	TMsgQueue *item = (TMsgQueue*)mir_alloc(sizeof(TMsgQueue));
	item->hContact = hContact;
	item->id = id;
	item->szMsg = szMsg;
	item->flags = flags;
	item->ts = GetTickCount();

	mir_cslock lck(csMsgQueue);
	if (!msgQueue.getCount() && !timerId)
		timerId = SetTimer(nullptr, 0, 5000, MsgTimer);
	msgQueue.insert(item);
}

TMsgQueue* msgQueue_find(MCONTACT hContact, int id)
{
	MCONTACT hMeta = db_mc_getMeta(hContact);

	mir_cslockfull lck(csMsgQueue);
	for (auto &it : msgQueue) {
		if ((it->hContact == hContact || it->hContact == hMeta) && it->id == id) {
			if (msgQueue.getCount() == 1 && timerId) {
				KillTimer(nullptr, timerId);
				timerId = 0;
			}

			return msgQueue.removeItem(&it);
		}
	}
	return nullptr;
}

void msgQueue_processack(MCONTACT hContact, int id, BOOL success, LPARAM lParam)
{
	TMsgQueue *p = msgQueue_find(hContact, id);
	if (p == nullptr)
		return;

	if (!success) {
		MessageFailureProcess(p, (wchar_t*)lParam);
		return;
	}

	DBEVENTINFO dbei = {};
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT | DBEF_UTF | (p->flags & PREF_RTL ? DBEF_RTL : 0);
	dbei.szModule = Proto_GetBaseAccountName(hContact);
	dbei.timestamp = time(0);
	dbei.cbBlob = (uint32_t)(mir_strlen(p->szMsg) + 1);
	dbei.pBlob = (uint8_t*)p->szMsg;
	dbei.szId = (char *)lParam;

	MessageWindowEvent evt = { id, hContact, &dbei };
	NotifyEventHooks(g_chatApi.hevPreCreate, 0, (LPARAM)&evt);

	p->szMsg = (char*)dbei.pBlob;

	db_event_add(hContact, &dbei);

	mir_free(p->szMsg);
	mir_free(p);
}

void msgQueue_destroy(void)
{
	for (auto &it : msgQueue) {
		mir_free(it->szMsg);
		mir_free(it);
	}
	msgQueue.destroy();
}
