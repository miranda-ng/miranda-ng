/*
AOL Instant Messenger Plugin for Miranda IM

Copyright © 2003 Robert Rainwater

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

#include <process.h>
#include "miranda.h"

struct pthread_arg
{
    HANDLE hEvent;
    void (*threadcode) (void *);
    void *arg;
};

void pthread_r(struct pthread_arg *fa)
{
    void (*callercode) (void *) = fa->threadcode;
    void *arg = fa->arg;
    Thread_Push(0, 0);
    SetEvent(fa->hEvent);
/*    __try { */
        callercode(arg);
/*    } */
/*    __finally { */
        Thread_Pop();
/*    } */
}

unsigned long pthread_create(void (*threadcode) (void *), void *arg)
{
    unsigned long rc;
    struct pthread_arg fa;
    fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    fa.threadcode = threadcode;
    fa.arg = arg;
    rc = _beginthread((void *) (void *) pthread_r, 0, &fa);
    if ((unsigned long) -1L != rc) {
        WaitForSingleObject(fa.hEvent, INFINITE);
    }
    CloseHandle(fa.hEvent);
    return rc;
}
