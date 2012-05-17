/* 
Copyright (C) 2006-2009 Ricardo Pescuma Domenecci

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

#include "ContactAsyncQueue.h"
#include <process.h>


// Itens with higher time at end
static int QueueSortItems(const QueueItem *oldItem, const QueueItem *newItem)
{
	if (oldItem->check_time == newItem->check_time)
		return -1;

	return oldItem->check_time - newItem->check_time;
}

// Itens with higher time at end
static void ContactAsyncQueueThread(void *obj)
{
	((ContactAsyncQueue *)obj)->Thread();
}

ContactAsyncQueue::ContactAsyncQueue(pfContactAsyncQueueCallback fContactAsyncQueueCallback, int initialSize)
	: queue(30, QueueSortItems)
{
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	finished = 0;
	callback = fContactAsyncQueueCallback;

	InitializeCriticalSection(&cs);

	_beginthread(ContactAsyncQueueThread, 0, this);
	//mir_forkthread(ContactAsyncQueueThread, this);
}

ContactAsyncQueue::~ContactAsyncQueue()
{
	Finish();

	int count = 0;
	while(finished != 2 && ++count < 50)
		Sleep(30);

	for (int i = 0; i < queue.getCount(); i++)
		if (queue[i] != NULL)
			mir_free(queue[i]);

	DeleteCriticalSection(&cs);
}

void ContactAsyncQueue::Finish()
{
	if (finished == 0)
		finished = 1;
	SetEvent(hEvent);
}

void ContactAsyncQueue::Lock()
{
	EnterCriticalSection(&cs);
}

void ContactAsyncQueue::Release()
{
	LeaveCriticalSection(&cs);
}

void ContactAsyncQueue::RemoveAll(HANDLE hContact)
{
	Lock();

	for (int i = queue.getCount() - 1; i >= 0; --i)
	{
		QueueItem *item = queue[i];
		
		if (item->hContact == hContact)
		{
			queue.remove(i);
			mir_free(item);
		}
	}

	Release();
}

void ContactAsyncQueue::RemoveAllConsiderParam(HANDLE hContact, void *param)
{
	Lock();

	for (int i = queue.getCount() - 1; i >= 0; --i)
	{
		QueueItem *item = queue[i];
		
		if (item->hContact == hContact && item->param == param)
		{
			queue.remove(i);
			mir_free(item);
		}
	}

	Release();
}

void ContactAsyncQueue::Add(int waitTime, HANDLE hContact, void *param)
{
	Lock();

	InternalAdd(waitTime, hContact, param);
	
	Release();
}

void ContactAsyncQueue::AddIfDontHave(int waitTime, HANDLE hContact, void *param)
{
	Lock();

	int i;
	for (i = queue.getCount() - 1; i >= 0; --i)
		if (queue[i]->hContact == hContact)
			break;

	if (i < 0)
		InternalAdd(waitTime, hContact, param);

	Release();
}

void ContactAsyncQueue::AddAndRemovePrevious(int waitTime, HANDLE hContact, void *param)
{
	Lock();

	RemoveAll(hContact);
	InternalAdd(waitTime, hContact, param);

	Release();
}

void ContactAsyncQueue::AddAndRemovePreviousConsiderParam(int waitTime, HANDLE hContact, void *param)
{
	Lock();

	RemoveAllConsiderParam(hContact, param);
	InternalAdd(waitTime, hContact, param);

	Release();
}

void ContactAsyncQueue::InternalAdd(int waitTime, HANDLE hContact, void *param)
{
	QueueItem *item = (QueueItem *) mir_alloc(sizeof(QueueItem));
	item->hContact = hContact;
	item->check_time = GetTickCount() + waitTime;
	item->param = param;

	queue.insert(item);

	SetEvent(hEvent);
}

void ContactAsyncQueue::Thread()
{
	while (!finished)
	{
		ResetEvent(hEvent);

		if (finished)
			break;

		Lock();

		if (queue.getCount() <= 0)
		{
			// No items, so supend thread
			Release();

			wait(/*INFINITE*/ 2 * 60 * 1000);
		}
		else
		{
			// Take a look at first item
			QueueItem *qi = queue[0];

			int dt = qi->check_time - GetTickCount();
			if (dt > 0) 
			{
				// Not time to request yet, wait...
				Release();

				wait(dt);
			}
			else
			{
				// Will request this item
				queue.remove(0);

				Release();

				callback(qi->hContact, qi->param);

				mir_free(qi);
			}
		}
	}

	finished = 2;
}

void ContactAsyncQueue::wait(int time)
{
	if (!finished)
		WaitForSingleObject(hEvent, time);
}






