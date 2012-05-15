/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project,
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

struct FORK_ARG {
	HANDLE hEvent;
	void (__cdecl *threadcode)(void*);
	unsigned (__stdcall *threadcodeex)(void*);
	void *arg;
};

void __cdecl forkthread_r(struct FORK_ARG *fa)
{
	void (*callercode)(void*)=fa->threadcode;
	void *arg=fa->arg;
	CallService(MS_SYSTEM_THREAD_PUSH,0,0);
	SetEvent(fa->hEvent);
	__try {
		callercode(arg);
	} __finally {
		CallService(MS_SYSTEM_THREAD_POP,0,0);
	}
	return;
}

unsigned long forkthread (
	void (__cdecl *threadcode)(void*),
	unsigned long stacksize,
	void *arg
)
{
	unsigned long rc;
	struct FORK_ARG fa;
    if (g_bSTATE!=STATE_NORMAL) return 0; //not start the new thead if exiting
	fa.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	fa.threadcode=threadcode;
	fa.arg=arg;
	rc=_beginthread(forkthread_r,stacksize,&fa);
	if ((unsigned long)-1L != rc) {
		WaitForSingleObject(fa.hEvent,1000);
	} //if
	CloseHandle(fa.hEvent);
	return rc;
}

unsigned long __stdcall forkthreadex_r(struct FORK_ARG *fa)
{
	unsigned (__stdcall * threadcode) (void *)=fa->threadcodeex;
	void *arg=fa->arg;
	unsigned long rc=0;

	CallService(MS_SYSTEM_THREAD_PUSH,0,0);
	SetEvent(fa->hEvent);
	__try {
		rc=threadcode(arg);
	} __finally {
		CallService(MS_SYSTEM_THREAD_POP,0,0);
	}
	return rc;
}

unsigned long forkthreadex(
	void *sec,
	unsigned stacksize,
	unsigned (__stdcall *threadcode)(void*),
	void *arg,
	unsigned cf,
	unsigned *thraddr
)
{
	unsigned long rc;
	struct FORK_ARG fa;
	fa.threadcodeex=threadcode;
	fa.arg=arg;
	fa.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	rc=_beginthreadex(sec,stacksize,forkthreadex_r,&fa,0,thraddr);
	if (rc) {
		WaitForSingleObject(fa.hEvent,INFINITE);
	}
	CloseHandle(fa.hEvent);
	return rc;
}