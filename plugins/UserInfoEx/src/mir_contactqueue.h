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


#ifndef __CONTACTASYNCQUEUE_H__
#define __CONTACTASYNCQUEUE_H__

/**
 *
 *
 **/
struct CQueueItem
{
	DWORD	check_time;
	MCONTACT hContact;
	PVOID	param;
};

/////////////////////////////////////////////////////////////////////////////////////////

class CContactQueue
{
public:

	enum EQueueStatus
	{
		RUNNING		= 0,
		STOPPING	= 1,
		STOPPED		= 2
	};

	CContactQueue(int initialSize = 10);
	~CContactQueue();

	__forceinline int Size() const {
		return _queue.getCount();
	}
	
	CQueueItem*	Get(int idx) const {
		return _queue[idx];
	}
	
	void RemoveAll();
	
	// This function removes all queue items for the hContact.
	void RemoveAll(MCONTACT hContact);

	// This function removes all queue items for the hContact considering the correct parameter.
	void RemoveAllConsiderParam(MCONTACT hContact, PVOID param);

	/**
	 * This method adds the desired new item.
	 *
	 * @param		waitTime		- the time to wait until the callback is desired to run
	 * @param		hContact		- the contact to perform the action for
	 * @param		param			- a caller defined parameter passed to the callback function
	 *
	 * @retval		TRUE			- The item is added to the queue successfully.
	 * @retval		FALSE			- The item is not added to the queue.
	 **/
	BOOL Add(int waitTime, MCONTACT hContact, PVOID param = NULL);

	/**
	 * This method adds the desired new item only, if the queue does not yet contain
	 * an item for the contact.
	 *
	 * @param		waitTime		- the time to wait until the callback is desired to run
	 * @param		hContact		- the contact to perform the action for
	 * @param		param			- a caller defined parameter passed to the callback function
	 *
	 * @retval		TRUE			- The item is added to the queue successfully.
	 * @retval		FALSE			- The item is not added to the queue.
	 **/
	BOOL AddIfDontHave(int waitTime, MCONTACT hContact, PVOID param = NULL);

	/**
	 * This method removes all existing queue items for the contact and adds a new queue item
	 * for the given contact. This method might be used to move an existing entry, 
	 * whose check_time has changed.
	 *
	 * @param		waitTime		- the time to wait until the callback is desired to run
	 * @param		hContact		- the contact to perform the action for
	 * @param		param			- a caller defined parameter passed to the callback function
	 *
	 * @return		nothing
	 **/
	BOOL AddUnique(int waitTime, MCONTACT hContact, PVOID param = NULL);

	/**
	 * This method removes all existing queue items for the contact with the same parameter as @e param
	 * and adds a new queue item for the given contact. This method might be used to move an existing
	 * entry, whose check_time has changed.
	 *
	 * @param		waitTime		- the time to wait until the callback is desired to run
	 * @param		hContact		- the contact to perform the action for
	 * @param		param			- a caller defined parameter passed to the callback function
	 *
	 * @return	nothing
	 **/
	BOOL AddUniqueConsiderParam	(int waitTime, MCONTACT hContact, PVOID param = NULL);

	// This method resumes the worker thread and immitiatly goes on with the next entry.
	void ContinueWithNext();

protected:
	virtual void OnEmpty() {};
	virtual void Callback(MCONTACT hContact, PVOID param) = 0;

	/**
	 * This is the real thread callback function. As long as _status
	 * is set to RUNNING it looks for items in the queue to perform 
	 * the _pfnCallback function on them. If the queue is empty or the
	 * next upcoming item is located in the future, the thread is suspended
	 * in the meanwhile.
	 *
	 * @param		none
	 *
	 * @return	nothing
	 **/
	void Thread();
	
	// This is a static method to redirect the thread's callback function
	// to the desired class object.
	static void ThreadProc(CContactQueue* obj)
	{
		obj->Thread();
	}

	// This method suspends the worker thread for the given ammount of time.
	void Suspend(int time) const;

private:
	LIST<CQueueItem> _queue;
	mir_cs _cs;
	HANDLE _hEvent;
	EQueueStatus _status;

	/**
	 * This member function really adds an item into the time sorted queue list.
	 *
	 * @param		waitTime	- the time to wait until the callback is desired to run
	 * @param		hContact	- the contact to perform the action for
	 * @param		param		- a caller defined parameter passed to the callback function
	 *
	 * @retval		TRUE		- The item is added to the queue successfully.
	 * @retval		FALSE		- The item is not added to the queue.
	 **/
	BOOL InternalAdd(int waitTime, MCONTACT hContact, PVOID param);

	void Remove(int idx);
};

#endif // __CONTACTASYNCQUEUE_H__