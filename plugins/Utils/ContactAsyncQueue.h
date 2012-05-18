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


#ifndef __CONTACTASYNCQUEUE_H__
# define __CONTACTASYNCQUEUE_H__

#ifndef MIRANDA_VER
#define MIRANDA_VER 0x0A00
#endif

#include <windows.h>
#include <newpluginapi.h>
#include <m_system_cpp.h>


typedef void (*pfContactAsyncQueueCallback) (HANDLE hContact, void *param);


struct QueueItem
{
	DWORD check_time;
	HANDLE hContact;
	void *param;
};


class ContactAsyncQueue
{
public:

	ContactAsyncQueue(pfContactAsyncQueueCallback fContactAsyncQueueCallback, int initialSize = 10);
	~ContactAsyncQueue();

	void Finish();

	inline int Size() const						{ return queue.getCount(); }
	inline int Remove(int idx)					{ mir_free(queue[idx]); return queue.remove(idx); }
	inline QueueItem* Get(int idx) const		{ return queue[idx]; }

	
	void RemoveAll(HANDLE hContact);
	void RemoveAllConsiderParam(HANDLE hContact, void *param);
	void Add(int waitTime, HANDLE hContact, void *param = NULL);
	void AddIfDontHave(int waitTime, HANDLE hContact, void *param = NULL);
	void AddAndRemovePrevious(int waitTime, HANDLE hContact, void *param = NULL);
	void AddAndRemovePreviousConsiderParam(int waitTime, HANDLE hContact, void *param = NULL);

	void Lock();
	void Release();


	void Thread();

private:

	LIST<QueueItem> queue;

	CRITICAL_SECTION cs;
	pfContactAsyncQueueCallback callback;
	HANDLE hEvent;
	int finished;


	void InternalAdd(int waitTime, HANDLE hContact, void *param);
	void wait(int time);
};











#endif // __CONTACTASYNCQUEUE_H__
