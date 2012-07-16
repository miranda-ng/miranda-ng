#include "headers.h"

// thread stuff

struct FORK_ARG {
	HANDLE hEvent;
	void (__cdecl *threadcode)(void*);
	unsigned (__stdcall *threadcodeex)(void*);
	void *arg;
};

void __cdecl forkthread_r(void *param) 
{    
   struct FORK_ARG *fa=(struct FORK_ARG*)param; 
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

unsigned long forkthread (   void (__cdecl *threadcode)(void*),unsigned long stacksize,void *arg) 
{ 
   unsigned long rc; 
   struct FORK_ARG fa; 

   fa.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL); 
   fa.threadcode=threadcode; 
   fa.arg=arg; 

   rc=_beginthread(forkthread_r,stacksize,&fa); 

   if ((unsigned long)-1L != rc) { 
      WaitForSingleObject(fa.hEvent,INFINITE); 
   } 
   CloseHandle(fa.hEvent); 

   return rc; 
}
