/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

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
#include "Utils.h"

struct FORK_ARG {
	HANDLE hEvent;
	void (__cdecl *threadcode)(void*);
	void *arg;
};

static void __cdecl forkthread_r(struct FORK_ARG *fa)
{	
	void (*callercode)(void*) = fa->threadcode;
	void *arg = fa->arg;
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
	SetEvent(fa->hEvent);
	callercode(arg);
	CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	return;
}

unsigned long Utils::forkThread(void (__cdecl *threadcode)(void*), unsigned long stacksize,	void *arg) {

	unsigned long rc;
	struct FORK_ARG fa;
	
	fa.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	fa.threadcode = threadcode;
	fa.arg = arg;
	rc = _beginthread((void (__cdecl *)(void*))forkthread_r, stacksize, &fa);
	if ((unsigned long) -1L != rc) {
		WaitForSingleObject(fa.hEvent, INFINITE);
	}
	CloseHandle(fa.hEvent);
	return rc;
}

void Utils::copyString(char **a, const char *b) {
	char *oldA;
	char *newA;
	oldA = *a;
	if (b!=NULL) {
		newA = new char[strlen(b)+1];
		strcpy(newA, b);
	} else {
		newA = NULL;
	}
	*a = newA;
	if (oldA!=NULL) {
		delete oldA;
	}

}



MM_INTERFACE Utils::mmi;

void Utils::mucc_mir_free(void* ptr)
{
	mir_free(ptr);
}

char* Utils::mucc_mir_t2a(LPCTSTR src)
{
	return mir_t2a(src);
}

LPTSTR Utils::mucc_mir_a2t(const char* src)
{
	return mir_a2t(src);
}



void Utils::log(const char *fmt, ...)
{
#ifdef ENABLE_LOGGING
	char *str;
	va_list vararg;
	int strsize;
	char *text;
	char *p, *q;
	int extra;

	va_start(vararg, fmt);
	str = (char *) malloc(strsize=2048);
	while (_vsnprintf(str, strsize, fmt, vararg) == -1)
		str = (char *) realloc(str, strsize+=2048);
	va_end(vararg);

	extra = 0;
	for (p=str; *p!='\0'; p++)
		if (*p=='\n' || *p=='\r')
			extra++;
	text = (char *) malloc(strlen(muccModuleName)+2+strlen(str)+2+extra);
	wsprintf(text, "[%s]", muccModuleName);
	for (p=str,q=text+strlen(text); *p!='\0'; p++,q++) {
		if (*p == '\r') {
			*q = '\\';
			*(q+1) = 'r';
			q++;
		}
		else if (*p == '\n') {
			*q = '\\';
			*(q+1) = 'n';
			q++;
		}
		else
			*q = *p;
	}
	*q = '\n';
	*(q+1) = '\0';
//	CallService(MS_NETLIB_LOG, (WPARAM) NULL, (LPARAM) text);
	//OutputDebugString(text);
	free(text);
	free(str);
#endif
}

