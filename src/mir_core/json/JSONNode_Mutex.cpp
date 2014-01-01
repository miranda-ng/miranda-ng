/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "..\commonheaders.h"
#include <m_json.h>

#ifdef JSON_MUTEX_CALLBACKS

json_mutex_callback_t json_lock_callback = 0;
json_mutex_callback_t json_unlock_callback = 0;
void * global_mutex = 0;
void * manager_mutex = 0;

struct AutoLock {
	AutoLock(void){
		json_lock_callback(manager_mutex);
	}
	~AutoLock(void){
		json_unlock_callback(manager_mutex);
	}
};

#include <map>
#ifdef JSON_MUTEX_MANAGE
	json_mutex_callback_t json_destroy = 0;
	std::map<void *, unsigned int> mutex_manager;

	//make sure that the global mutex is taken care of too
	struct auto_global {
		auto_global(void){}
		~auto_global(void){
			if (global_mutex){
				JSON_ASSERT_SAFE(json_destroy, JSON_TEXT("No json_destroy in mutex managed mode"), return;);
				json_destroy(global_mutex);
			}
		}
	};
	auto_global cleanupGlobal;
#endif

void JSONNode::register_mutex_callbacks(json_mutex_callback_t lock, json_mutex_callback_t unlock, void * manager_lock){
	json_lock_callback = lock;
	json_unlock_callback = unlock;
	manager_mutex = manager_lock;
}

void JSONNode::set_global_mutex(void * mutex){
	global_mutex = mutex;
}

void JSONNode::set_mutex(void * mutex){
	makeUniqueInternal();
	internal -> _set_mutex(mutex);
}

std::map<int, std::map<void *, unsigned int> > threadlocks;

void * JSONNode::getThisLock(JSONNode * pthis){
	if (pthis -> internal -> mylock){
		return pthis -> internal -> mylock;
	}
	JSON_ASSERT_SAFE(global_mutex, JSON_TEXT("No global_mutex"), return 0;);
	return global_mutex;
}

void JSONNode::lock(int thread){
	JSON_ASSERT_SAFE(json_lock_callback, JSON_TEXT("No locking callback"), return;);

	AutoLock lockControl;

	//first, figure out what needs to be locked
	void * thislock = getThisLock(this);
	#ifdef JSON_SAFE
		if (!thislock) return;
	#endif

	//make sure that the same thread isn't locking it more than once (possible due to complex ref counting)
	std::map<int, std::map<void *, unsigned int> >::iterator it = threadlocks.find(thread);
	if (it == threadlocks.end()) {
		std::map<void *, unsigned int> newthread;
		newthread[thislock] = 1;
		threadlocks.insert(std::pair<int, std::map<void *, unsigned int> >(thread, newthread));
	} else { //this thread already has some things locked, check if the current mutex is
		std::map<void *, unsigned int> & newthread = it -> second;
		std::map<void *, unsigned int>::iterator locker = newthread.find(thislock);
		if (locker == newthread.end()) { //current mutex is not locked, set it to locked
			newthread.insert(std::pair<void *, unsigned int>(thislock, 1));
		} else { //it's already locked, don't relock it
			++(locker -> second);
			return; //don't try to relock, it will deadlock the program
		}
	}

	//if I need to, lock it
	json_lock_callback(thislock);
}

void JSONNode::unlock(int thread){
	JSON_ASSERT_SAFE(json_unlock_callback, JSON_TEXT("No unlocking callback"), return;);

	AutoLock lockControl;

	//first, figure out what needs to be locked
	void * thislock = getThisLock(this);
	#ifdef JSON_SAFE
		if (!thislock) return;
	#endif

	//get it out of the map
	std::map<int, std::map<void *, unsigned int> >::iterator it = threadlocks.find(thread);
	JSON_ASSERT_SAFE(it != threadlocks.end(), JSON_TEXT("thread unlocking something it didn't lock"), return;);

	//get the mutex out of the thread
	std::map<void *, unsigned int> & newthread = it -> second;
	std::map<void *, unsigned int>::iterator locker = newthread.find(thislock);
	JSON_ASSERT_SAFE(locker != newthread.end(), JSON_TEXT("thread unlocking mutex it didn't lock"), return;);

	//unlock it
	if (--(locker -> second)) return; //other nodes is this same thread still have a lock on it

	//if I need to, unlock it
	newthread.erase(locker);
	json_unlock_callback(thislock);
}

#ifdef JSON_MUTEX_MANAGE
	void JSONNode::register_mutex_destructor(json_mutex_callback_t destroy){
		json_destroy = destroy;
	}
#endif

void internalJSONNode::_set_mutex(void * mutex, bool unset){
	if (unset) _unset_mutex(); //for reference counting
	mylock = mutex;
	if (mutex){
		#ifdef JSON_MUTEX_MANAGE
			std::map<void *, unsigned int>::iterator it = mutex_manager.find(mutex);
			if (it == mutex_manager.end()) {
				mutex_manager.insert(std::pair<void *, unsigned int>(mutex, 1));
			} else {
				++it -> second;
			}
		#endif
		json_foreach(Children, myrunner){
			(*myrunner) -> set_mutex(mutex);
		}
	}
}

void internalJSONNode::_unset_mutex(void){
	#ifdef JSON_MUTEX_MANAGE
		if (mylock){
			std::map<void *, unsigned int>::iterator it = mutex_manager.find(mylock);
			JSON_ASSERT_SAFE(it != mutex_manager.end(), JSON_TEXT("Mutex not managed"), return;);
			--it -> second;
			if (it -> second == 0){
				JSON_ASSERT_SAFE(json_destroy, JSON_TEXT("You didn't register a destructor for mutexes"), return;);
				mutex_manager.erase(it);
			}
		}
	#endif
}

#ifdef JSON_DEBUG
	#ifndef JSON_LIBRARY
		JSONNode internalJSONNode::DumpMutex(void) const {
			JSONNode mut(JSON_NODE);
			mut.set_name(JSON_TEXT("mylock"));
			#ifdef JSON_MUTEX_MANAGE
				if (mylock){
				mut.push_back(JSON_NEW(JSONNode(JSON_TEXT("this"), (long)mylock)));
				std::map<void *, unsigned int>::iterator it = mutex_manager.find(mylock);
				if (it == mutex_manager.end()) {
					mut.push_back(JSON_NEW(JSONNode(JSON_TEXT("references"), JSON_TEXT("error"))));
				} else {
					mut.push_back(JSON_NEW(JSONNode(JSON_TEXT("references"), it -> second)));
				}
				} else {
				mut = (long)mylock;
				}
			#else
				mut = (long)mylock;
			#endif
			return mut;
		}
	#endif
#endif

#else
	#ifdef JSON_MUTEX_MANAGE
		#error You can not have JSON_MUTEX_MANAGE on without JSON_MUTEX_CALLBACKS
	#endif
#endif
