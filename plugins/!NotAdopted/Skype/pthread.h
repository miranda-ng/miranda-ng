/*
 * $Id: pthread.h,v 1.3 2003/10/05 04:03:05 gena01 Exp $
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
#ifndef PTHREAD_H
#define PTHREAD_H

unsigned long pthread_create(void (*threadcode) (void *), void *arg);
typedef CRITICAL_SECTION pthread_mutex_t;
#define pthread_mutex_init(pmutex)           InitializeCriticalSection(pmutex)
#define pthread_mutex_destroy(pmutex)        DeleteCriticalSection(pmutex)
#define pthread_mutex_lock(pmutex)           EnterCriticalSection(pmutex)
#define pthread_mutex_unlock(pmutex)         LeaveCriticalSection(pmutex)

#endif
