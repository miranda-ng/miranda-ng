/*
Copyright 2000-2010 Miranda IM project, 
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

int tcmdlist_append(SortedList *list, TCHAR *data)
{
	TCmdList *new_list;
	
	if (!data)
		return list->realCount - 1;

	if (list->realCount >= 20)
	{
		TCmdList* n = (TCmdList*)list->items[0];
		mir_free(n->szCmd);
		mir_free(n);
		li.List_Remove(list, 0);
	}

	new_list = mir_alloc(sizeof(TCmdList));
	new_list->szCmd = mir_tstrdup(data);

	li.List_InsertPtr(list, new_list);

	return list->realCount - 1;
}

void tcmdlist_free(SortedList *list) 
{
	int i;
	TCmdList** n = (TCmdList**)list->items;

	for (i = 0; i < list->realCount; ++i) 
	{
		mir_free(n[i]->szCmd);
		mir_free(n[i]);
	}
	li.List_Destroy(list);
	mir_free(list);
}

static SortedList msgQueue = { NULL, 0, 0, 5, NULL };
static CRITICAL_SECTION csMsgQueue;
static UINT_PTR timerId;

void MessageFailureProcess(TMsgQueue *item, const char* err);

static VOID CALLBACK MsgTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	int i, ntl = 0;
	TMsgQueue **tmlst = NULL;
	
	EnterCriticalSection(&csMsgQueue);

	for (i = 0; i < msgQueue.realCount; ++i) 
	{
		TMsgQueue *item = (TMsgQueue*)msgQueue.items[i];
		if (dwTime - item->ts > g_dat->msgTimeout)
		{
			if (!ntl)
				tmlst = (TMsgQueue**)alloca((msgQueue.realCount - i) * sizeof(TMsgQueue*));
			tmlst[ntl++] = item;

			li.List_Remove(&msgQueue, i--);
		}
	}
	LeaveCriticalSection(&csMsgQueue);

	for (i = 0; i < ntl; ++i) 
		MessageFailureProcess(tmlst[i], LPGEN("The message send timed out."));
}

void msgQueue_add(HANDLE hContact, HANDLE id, const TCHAR* szMsg, HANDLE hDbEvent)
{
	TMsgQueue *item;
	
	item = mir_alloc(sizeof(TMsgQueue));
	item->hContact = hContact;
	item->id = id;
	item->szMsg = mir_tstrdup(szMsg);
	item->hDbEvent = hDbEvent;
	item->ts = GetTickCount();

	EnterCriticalSection(&csMsgQueue);
	if (!msgQueue.realCount && !timerId)
		timerId = SetTimer(NULL, 0, 5000, MsgTimer);
	li.List_InsertPtr(&msgQueue, item);
	LeaveCriticalSection(&csMsgQueue);

}

void msgQueue_processack(HANDLE hContact, HANDLE id, BOOL success, const char* szErr)
{
	int i;
	TMsgQueue* item = NULL;;
	
	EnterCriticalSection(&csMsgQueue);

	for (i = 0; i < msgQueue.realCount; ++i) 
	{
		item = (TMsgQueue*)msgQueue.items[i];
		if (item->hContact == hContact && item->id == id)
		{
			li.List_Remove(&msgQueue, i);

			if (!msgQueue.realCount && timerId)
			{
				KillTimer(NULL, timerId);
				timerId = 0;
			}
			break;
		}
		item = NULL;
	}
	LeaveCriticalSection(&csMsgQueue);
	
	if (item)
	{
		if (success)
		{
			mir_free(item->szMsg);
			mir_free(item);
		}
		else
			MessageFailureProcess(item, szErr);
	}
}

void msgQueue_init(void)
{
	InitializeCriticalSection(&csMsgQueue);
}

void msgQueue_destroy(void)
{
	int i;
	
	EnterCriticalSection(&csMsgQueue);

	for (i = 0; i < msgQueue.realCount; ++i) 
	{
		TMsgQueue* item = (TMsgQueue*)msgQueue.items[i];
		mir_free(item->szMsg);
		mir_free(item);
	}
	li.List_Destroy(&msgQueue);

	LeaveCriticalSection(&csMsgQueue);

	DeleteCriticalSection(&csMsgQueue);
}
