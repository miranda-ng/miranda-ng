/*
 * $Id: pthread.c,v 1.4 2003/12/19 12:53:36 gena01 Exp $
 *
 * Skype Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * Code borrowed for Skype plugin. Fixed to compile on Mingw by G.Feldman
 * Original Copyright (c) 2003 Robert Rainwater
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */

#include "skype.h"

/* Gena01 - added some defined to fix compilation with mingw gcc */
/* __try/__finally taken from abiword patch found on the web */
#if 0
 #include <crtdbg.h>
#else
#define __try
#define __except(x) if (0) /* don't execute handler */
#define __finally

#define _try __try
#define _except __except
#define _finally __finally
#endif

#include <excpt.h> 

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
    __try {
        callercode(arg);
    }
    __finally {
        Thread_Pop();
    }
}

unsigned long pthread_create(pThreadFunc parFunc, void *arg)
{
    unsigned long rc;
    struct pthread_arg fa;
    fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    fa.threadcode = parFunc;
    fa.arg = arg;
    rc = _beginthread((pThreadFunc) pthread_r, 0, &fa);
    if ((unsigned long) -1L != rc) {
        WaitForSingleObject(fa.hEvent, INFINITE);
    }
    CloseHandle(fa.hEvent);
    return rc;
}

