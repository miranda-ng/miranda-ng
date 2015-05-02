/*
Copyright ©2006 Ricardo Pescuma Domenecci

Modified  ©2008-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

static int QueueSortItems(const CQueueItem *i1, const CQueueItem *i2)
{
	int rc = i1->check_time - i2->check_time;
	if (!rc)
		rc = i1->hContact != i2->hContact;

	return rc;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Constructor & destructor

CContactQueue::CContactQueue(int initialSize) :
	_queue(initialSize, QueueSortItems)
{
	_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_status = RUNNING;

	mir_forkthread((pThreadFunc)CContactQueue::ThreadProc, this);
}

CContactQueue::~CContactQueue()
{
	if (_status == RUNNING)
		_status = STOPPING;

	SetEvent(_hEvent);

	for (int count = 0; _status != STOPPED && ++count < 50;)
		Sleep(10);

	for (int i = 0; i < _queue.getCount(); i++)
		mir_free(_queue[i]);

	CloseHandle(_hEvent);
}

void CContactQueue::Remove(int idx)
{
	CQueueItem *qi = _queue[idx];
	_queue.remove(idx);
	mir_free(qi);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function removes all queue items.

void CContactQueue::RemoveAll()
{
	mir_cslock lck(_cs);

	for (int i = _queue.getCount() - 1; i >= 0; --i)
		mir_free(_queue[i]);
	_queue.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function removes all queue items for the hContact.

void CContactQueue::RemoveAll(MCONTACT hContact)
{
	mir_cslock lck(_cs);

	for (int i = _queue.getCount() - 1; i >= 0; --i) {
		CQueueItem *qi = _queue[i];
		if (qi->hContact == hContact) {
			_queue.remove(i);
			mir_free(qi);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// This function removes all queue items for the hContact considering the correct parameter.

void CContactQueue::RemoveAllConsiderParam(MCONTACT hContact, PVOID param)
{
	mir_cslock lck(_cs);

	for (int i = _queue.getCount() - 1; i >= 0; --i) {
		CQueueItem *qi = _queue[i];
		if (qi->hContact == hContact && qi->param == param) {
			_queue.remove(i);
			mir_free(qi);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// This method adds the desired new item.

BOOL CContactQueue::Add(int waitTime, MCONTACT hContact, PVOID param)
{
	mir_cslock lck(_cs);
	return InternalAdd(waitTime, hContact, param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This method adds the desired new item only, if the queue does not yet contain
// an item for the contact.

BOOL CContactQueue::AddIfDontHave(int waitTime, MCONTACT hContact, PVOID param)
{
	mir_cslock lck(_cs);

	for (int i = _queue.getCount() - 1; i >= 0; --i)
		if (_queue[i]->hContact == hContact)
			return FALSE;

	return InternalAdd(waitTime, hContact, param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This method removes all existing queue items for the contact and adds a new queue item
// for the given contact. This method might be used to move an existing entry,
// whose check_time has changed.

BOOL CContactQueue::AddUnique(int waitTime, MCONTACT hContact, PVOID param)
{
	mir_cslock lck(_cs);

	RemoveAll(hContact);
	return InternalAdd(waitTime, hContact, param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This method removes all existing queue items for the contact with the same parameter as @e param
// and adds a new queue item for the given contact. This method might be used to move an existing
// entry, whose check_time has changed.

BOOL CContactQueue::AddUniqueConsiderParam(int waitTime, MCONTACT hContact, PVOID param)
{
	mir_cslock lck(_cs);

	RemoveAllConsiderParam(hContact, param);
	return InternalAdd(waitTime, hContact, param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This member function really adds an item into the time sorted queue list.

BOOL CContactQueue::InternalAdd(int waitTime, MCONTACT hContact, PVOID param)
{
	CQueueItem *qi = (CQueueItem *) mir_alloc(sizeof(CQueueItem));
	qi->hContact = hContact;
	qi->check_time = GetTickCount() + waitTime;
	qi->param = param;

	int rc = _queue.insert(qi);
	if (!rc)
		mir_free(qi);

	SetEvent(_hEvent);
	return rc;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This is the real thread callback function. As long as _status
// is set to RUNNING it looks for items in the queue to perform
// the _pfnCallback function on them. If the queue is empty or the
// next upcoming item is located in the future, the thread is suspended
// in the meanwhile.

void CContactQueue::Thread()
{
	while (_status == RUNNING) {
		ResetEvent(_hEvent);

		mir_cslockfull lck(_cs);

		if (_queue.getCount() <= 0) {
			// can be used by a derivant
			OnEmpty();
			lck.unlock();

			Suspend(INFINITE);
			continue;
		}

		// Take a look at first queue item
		CQueueItem *qi = _queue[0];

		int dt = qi->check_time - GetTickCount();
		if (dt > 0) {
			lck.unlock();
			Suspend(dt);
			continue;
		}

		// Will request this queue item
		_queue.remove(0);
		lck.unlock();

		Callback(qi->hContact, qi->param);
		mir_free(qi);
	}
	_status = STOPPED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This method suspends the worker thread for the given ammount of time.

void CContactQueue::Suspend(int time) const
{
	if (_status == RUNNING)
		WaitForSingleObject(_hEvent, time);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This method resumes the worker thread and immitiatly goes on with the next entry.

void CContactQueue::ContinueWithNext()
{
	if (_status != RUNNING)
		return;

	mir_cslock lck(_cs);

	int c = _queue.getCount();
	if (c > 0) {
		int dt = _queue[0]->check_time - GetTickCount() - 3000;
		if (dt > 0)
			for (int i = 0; i < c; i++)
				_queue[i]->check_time -= dt;
	}
	SetEvent(_hEvent);
}
