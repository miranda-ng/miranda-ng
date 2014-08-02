/*
AOL Instant Messenger Plugin for Miranda IM

Copyright (c) 2003-2006 Robert Rainwater

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

#include "pthread.h"
#include <process.h>
#include <newpluginapi.h>
#include <m_system.h>

/****************************************/
/* Portable pthread code for Miranda IM */

/* create thread */
int pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(__stdcall * thread_start) (void *), void *param)
{
	tid->hThread = (HANDLE)mir_forkthreadex((pThreadFuncEx)*(void**)&thread_start, param, (unsigned *)&tid->dwThreadId);
	return 0;
}

/* detach a thread */
int pthread_detach(pthread_t *tid)
{
	CloseHandle(tid->hThread);
	return 0;
}

/* wait for thread termination */
int pthread_join(pthread_t *tid, void **value_ptr)
{
	if (tid->dwThreadId == GetCurrentThreadId())
		return 35 /*EDEADLK*/;

	WaitForSingleObject(tid->hThread, INFINITE);
	return 0;
}

/* get calling thread's ID */
pthread_t *pthread_self(void)
{
	static int poolId = 0;
	static pthread_t tidPool[32];
	/* mark & round pool to 32 items */
	pthread_t *tid = &tidPool[poolId ++];
	poolId %= 32;

	tid->hThread = GetCurrentThread();
	tid->dwThreadId = GetCurrentThreadId();
	return tid;
}

/* cancel execution of a thread */
int pthread_cancel(pthread_t *thread)
{
	return TerminateThread(thread->hThread, 0) ? 0 : 3 /*ESRCH*/;
}

/* terminate thread */
void pthread_exit(void *value_ptr)
{
//	_endthreadex((unsigned)value_ptr);
}
