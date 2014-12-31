/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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
//Modules Core - Richard
#ifndef MODULES_H_
#define MODULES_H_

#pragma warning(disable:4201)

/* MAXMODULELABELLENGTH
The maximum allowed length of a 'name' parameter. Very likely to change with
restructuring modules.c for performance.
*/
#define MAXMODULELABELLENGTH 64

typedef int (*MIRANDAHOOK)(WPARAM, LPARAM);
typedef int (*MIRANDAHOOKPARAM)(WPARAM, LPARAM, LPARAM);
typedef int (*MIRANDAHOOKOBJ)(void*, WPARAM, LPARAM);
typedef int (*MIRANDAHOOKOBJPARAM)(void*, WPARAM, LPARAM, LPARAM);

typedef INT_PTR (*MIRANDASERVICE)(WPARAM, LPARAM);
typedef INT_PTR (*MIRANDASERVICEPARAM)(WPARAM, LPARAM, LPARAM);
typedef INT_PTR (*MIRANDASERVICEOBJ)(void*, LPARAM, LPARAM);
typedef INT_PTR (*MIRANDASERVICEOBJPARAM)(void*, WPARAM, LPARAM, LPARAM);

typedef struct
{
	HINSTANCE hOwner;
	int type;
	union {
		struct {
			union {
				MIRANDAHOOK pfnHook;
				MIRANDAHOOKPARAM pfnHookParam;
				MIRANDAHOOKOBJ pfnHookObj;
				MIRANDAHOOKOBJPARAM pfnHookObjParam;
			};
			void* object;
			LPARAM lParam;
		};
		struct {
			HWND hwnd;
			UINT message;
		};
	};
}
	THookSubscriber;

typedef struct
{
	char name[ MAXMODULELABELLENGTH ];
	int  id;
	int  subscriberCount;
	THookSubscriber* subscriber;
	MIRANDAHOOK pfnHook;
	CRITICAL_SECTION csHook;
}
	THook;

/**************************hook functions****************************/
/* CreateHookableEvent
Adds an named event to the list and returns a handle referring to it, or NULL
on failure. Will be automatically destroyed on exit, or can be removed from the
list earlier using DestroyHookableEvent()
Will fail if the given name has already been used
*/
HANDLE CreateHookableEvent(const char *name);

/* DestroyHookableEvent
Removes the event hEvent from the list of events. All modules hooked to it are
automatically unhooked. NotifyEventHooks() will fail if called with this hEvent
again. hEvent must have been returned by CreateHookableEvent()
Returns 0 on success, or nonzero if hEvent is invalid
*/
int DestroyHookableEvent(HANDLE hEvent);

/* NotifyEventHooks
Calls every module in turn that has hooked hEvent, using the parameters wParam
and lParam. hEvent must have been returned by CreateHookableEvent()
Returns 0 on success
  -1 if hEvent is invalid
  If one of the hooks returned nonzero to indicate abort, returns that abort
      value immediately, without calling the rest of the hooks in the chain

Notes on calling NotifyEventHooks() from a thread other than that which owns
the main Miranda window:
It works. The call is routed to the main thread and all hook subcribers are
called in the context of the main thread. The thread which called
NotifyHookEvents() is paused until all the processing is complete at which
point it returns with the correct return value.
This procedure requires more than one wait object so naturally there are
possibilities for deadlocks, but not many.
Calling NotifyEventHooks() from other than the main thread will be
considerably slower than from the main thread, but will consume only slightly
more actual CPU time, the rest will mostly be spent waiting for the main thread
to return to the message loop so it can be interrupted neatly.
*/
int NotifyEventHooks(HANDLE hEvent, WPARAM wParam, LPARAM lParam);

/* CallHookSubscribers
Works precisely like NotifyEventHooks, but without switching to the first thread
It guarantees that the execution time for these events is always tiny
*/

int CallHookSubscribers(HANDLE hEvent, WPARAM wParam, LPARAM lParam);

/*
	hEvent : a HANDLE which has been returned by CreateHookableEvent()
	pfnHook: a function pointer (MIRANDAHOOK) which is called when there are no hooks.
	Affect:  This core service allows hooks to have a 'default' hook which is called
		when no one has hooked the given event, this allows hook creators to add default
		processing which is ONLY CALLED when no one else has HookEvent()'d
	Notes:	 The return value from pfnHook() is returned to NotifyEventHooks()
	Returns: 0 on success, non zero on failure
	Version: 0.3.4+ (2004/09/15)
*/
int SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pfnHook);

/* HookEvent
Adds a new hook to the chain 'name', to be called when the hook owner calls
NotifyEventHooks(). Returns NULL if name is not a valid event or a handle
referring to the hook otherwise. Note that debug builds will warn with a
MessageBoxA if a hook is attempted on an unknown event. All hooks will be
automatically destroyed when their parent event is destroyed or the programme
ends, but can be unhooked earlier using UnhookEvent(). hookProc() is defined as
  int HookProc(WPARAM wParam, LPARAM lParam)
where you can substitute your own name for HookProc. wParam and lParam are
defined by the creator of the event when NotifyEventHooks() is called.
The return value is 0 to continue processing the other hooks, or nonzero
to stop immediately. This abort value is returned to the caller of
NotifyEventHooks() and should not be -1 since that is a special return code
for NotifyEventHooks() (see above)
*/
HANDLE HookEvent(const char *name, MIRANDAHOOK hookProc);
HANDLE HookEventParam(const char *name, MIRANDAHOOKPARAM hookProc, LPARAM lParam);
HANDLE HookEventObj(const char *name, MIRANDAHOOKOBJ hookProc, void* object);
HANDLE HookEventObjParam(const char *name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam);

/* HookEventMessage
Works as for HookEvent(), except that when the notifier is called a message is
sent to a window, rather than a function being called.
Note that SendMessage() is a decidedly slow function so please limit use of
this function to events that are not called frequently, or to hooks that are
only installed briefly
The window procedure is called with the message 'message' and the wParam and
lParam given to NotifyEventHooks(). The return value of SendMessage() is used
in the same way as the return value in HookEvent().
*/
HANDLE HookEventMessage(const char *name, HWND hwnd, UINT message);

/* UnhookEvent
Removes a hook from its event chain. It will no longer receive any events.
hHook must have been returned by HookEvent() or HookEventMessage().
Returns 0 on success or nonzero if hHook is invalid.
*/
int UnhookEvent(HANDLE hHook);


/*************************service functions**************************/
/* CreateServiceFunction
Adds a new service function called 'name' to the global list and returns a
handle referring to it. Service function handles are destroyed automatically
on exit, but can be removed from the list earlier using
DestroyServiceFunction()
Returns NULL if name has already been used. serviceProc is defined by the
caller as
  int ServiceProc(WPARAM wParam, LPARAM lParam)
where the creator publishes the meanings of wParam, lParam and the return value
Service functions must not return CALLSERVICE_NOTFOUND since that would confuse
callers of CallService().
*/
HANDLE CreateServiceFunction(const char *name, MIRANDASERVICE serviceProc);

/* CreateServiceFunctionParam
Same as CreateServiceFunction - adds new parameter, to pass to service handler function.
serviceProc is defined by the caller as
  int ServiceProc(WPARAM wParam, LPARAM lParam, LPARAM fnParam)
where fnParam does not need to be publicly known. Gives the ability to handle multiple services
with the same function.

added during 0.7+ (2007/04/24)
*/
HANDLE CreateServiceFunctionParam(const char *name, MIRANDASERVICEPARAM serviceProc, LPARAM lParam);

/* CreateServiceFunctionObj
   CreateServiceFunctionObjParam
Same as CreateServiceFunction - adds new parameter, an object, to pass to service handler function.
serviceProc is defined by the caller as
  int ServiceProc(void* object, WPARAM wParam, LPARAM lParam[, LPARAM fnParam])
where fnParam does not need to be publicly known. Gives the ability to handle multiple services
with the same function.

added during 0.7+ (2007/04/24)
*/

HANDLE CreateServiceFunctionObj(const char *name, MIRANDASERVICEOBJ serviceProc, void* object);
HANDLE CreateServiceFunctionObjParam(const char *name, MIRANDASERVICEOBJPARAM serviceProc, void* object, LPARAM lParam);

/* DestroyServiceFunction
Removes the function associated with hService from the global service function
list. Modules calling CallService() will fail if they try to call this
service's name. hService must have been returned by CreateServiceFunction().
Returns 0 on success or non-zero if hService is invalid.
*/
int DestroyServiceFunction(HANDLE hService);

/* CallService
Finds and calls the service function 'name' using the parameters wParam and
lParam.
Returns CALLSERVICE_NOTFOUND if no service function called 'name' has been
created, or the value the service function returned otherwise.
*/
#ifdef _WIN64
    #define CALLSERVICE_NOTFOUND      ((INT_PTR)0x8000000000000000)
#else
    #define CALLSERVICE_NOTFOUND      ((int)0x80000000)
#endif
INT_PTR CallService(const char *name, WPARAM wParam, LPARAM lParam);

/* ServiceExists
Finds if a service with the given name exists
Returns nonzero if the service was found, and zero if it was not
*/
int ServiceExists(const char *name);

/* CallServiceSync
Calls a given service executed within the context of the main thread
only useful to multi threaded modules calling thread unsafe services!
*/
INT_PTR CallServiceSync(const char *name, WPARAM wParam, LPARAM lParam);

/* CallFunctionAsync
Calls a given function pointer, it doesn't go thru the core at all, it is
just a wrapper around QueueUserAPC() and other workarounds to make APC
work even if there are non APC message loops, this allows plugins who
need this feature to get it without recoding it themselves.

The function 'func' will always be called from the main thread in idle time even
if it is invokved from a worker thread, 'arg' must not be on the stack.

Returns nonzero on success, zero on failure

added during 0.3.4+ (2004/08/14)
*/
int CallFunctionAsync(void (__stdcall *func)(void *), void *arg);

#endif  // MODULES_H_
