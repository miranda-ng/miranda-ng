(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

*)
{$IFNDEF NEWPLUGINAPI}
{$DEFINE NEWPLUGINAPI}

const

    MAXMODULELABELLENGTH = 64;

type

    PPLUGININFO = ^TPLUGININFO;
    TPLUGININFO = record
        cbSize: int;
        shortName: PChar;
        version: DWORD;
        description: PChar;
        author: PChar;
        authorEmail: PChar;
        copyright: PChar;
        homepage: PChar;
        isTransient: Byte;              // leave zero for now
        { one of the DEFMOD_* consts in m_plugin or zero, if non zero, this will
        suppress loading of the specified builtin module }
        replacesDefaultModule: int;
    end;

    PPLUGININFOEX = ^TPLUGININFOEX;
    TPLUGININFOEX = record
        cbSize: int;
        shortName: PChar;
        version: DWORD;
        description: PChar;
        author: PChar;
        authorEmail: PChar;
        copyright: PChar;
        homepage: PChar;
        isTransient: Byte;              // leave zero for now
        { one of the DEFMOD_* consts in m_plugin or zero, if non zero, this will
        suppress loading of the specified builtin module }
        replacesDefaultModule: int;
        uuid: TGUID;
    end;

    { modules.h is never defined -- no check needed }

    TMIRANDAHOOK = function(wParam: WPARAM; lParam: LPARAM): int; cdecl;
    TMIRANDASERVICE = function(wParam: WPARAM; lParam: LPARAM): int; cdecl;

	{**************************hook functions****************************}
	(* TCreateHookableEvent
	Adds an named event to the list and returns a handle referring to it, or NIL
	on failure. Will be automatically destroyed on exit, or can be removed from the
	list earlier using TDestroyHookableEvent()
	Will fail if the given name has already been used
	*)
    TCreateHookableEvent = function(const char: PChar): THandle; cdecl;

	(* TDestroyHookableEvent
	Removes the event Handle from the list of events. All modules hooked to it are
	automatically unhooked. TNotifyEventHooks() will fail if called with this hEvent
	again. Handle must have been returned by CreateHookableEvent()
	Returns 0 on success, or nonzero if Handle is invalid
	*)
    TDestroyHookableEvent = function(Handle: THandle): int; cdecl;

	(* TNotifyEventHooks
	Calls every module in turn that has hooked Handle, using the parameters wParam
	and lParam. Handle must have been returned by TCreateHookableEvent()
	Returns 0 on success,  -1 if Handle is invalid
	If one of the hooks returned nonzero to indicate abort, returns that abort
	value immediately, without calling the rest of the hooks in the chain
	Notes on calling TNotifyEventHooks() from a thread other than that which owns
	the main Miranda window:
	It works. The call is routed to the main thread and all hook subcribers are
	called in the context of the main thread. The thread which called
	TNotifyHookEvents() is paused until all the processing is complete at which
	point it returns with the correct return value.
	This procedure requires more than one wait object so naturally there are
	possibilities for deadlocks, but not many.
	Calling TNotifyEventHooks() from other than the main thread will be
	considerably slower than from the main thread, but will consume only slightly
	more actual CPU time, the rest will mostly be spent waiting for the main thread
	to return to the message loop so it can be interrupted neatly.
	*)
    TNotifyEventHooks = function(Handle: THandle; wParam: WPARAM; lParam: LPARAM): int; cdecl;
	(* THookEvent
	Adds a new hook to the chain 'char', to be called when the hook owner calls
	TNotifyEventHooks(). Returns NIL if name is not a valid event or a handle
	referring to the hook otherwise. Note that debug builds will warn with a
	MessageBoxA if a hook is attempted on an unknown event. All hooks will be
	automatically destroyed when their parent event is destroyed or the programme
	ends, but can be unhooked earlier using TUnhookEvent(). MIRANDAHOOK is defined as
	  function(wParam: WPARAM; lParam: LPARAM): int; cdecl;
	where you can substitute your own name for function. wParam and lParam are
	defined by the creator of the event when TNotifyEventHooks() is called.
	The return value is 0 to continue processing the other hooks, or nonzero
	to stop immediately. This abort value is returned to the caller of
	TNotifyEventHooks() and should not be -1 since that is a special return code
	for TNotifyEventHooks() (see above)
	*)
    THookEvent  = function(const char: PChar; MIRANDAHOOK: TMIRANDAHOOK): THandle; cdecl;

	(* THookEventMessage
	Works as for THookEvent(), except that when the notifier is called a message is
	sent to a window, rather than a function being called.
	Note that SendMessage() is a decidedly slow function so please limit use of
	this function to events that are not called frequently, or to hooks that are
	only installed briefly
	The window procedure is called with the message 'message' and the wParam and
	lParam given to TNotifyEventHooks(). The return value of SendMessage() is used
	in the same way as the return value in HookEvent().
	*)
    THookEventMessage = function(const char: PChar; Wnd: THandle; wMsg: Integer): THandle; cdecl;

	(* TUnhookEvent
	Removes a hook from its event chain. It will no longer receive any events.
	Handle must have been returned by THookEvent() or THookEventMessage().
	Returns 0 on success or nonzero if hHook is invalid.
	*)
    TUnhookEvent = function(Handle: THandle): int; cdecl;
	
	{*************************service functions**************************}
	(* TCreateServiceFunction
	Adds a new service function called 'char' to the global list and returns a
	handle referring to it. Service function handles are destroyed automatically
	on exit, but can be removed from the list earlier using
	TDestroyServiceFunction()
	Returns NIL if name has already been used. MIRANDASERVICE is defined by the
	caller as
	  function(wParam: WPARAM; lParam: LPARAM): int; cdecl;
	where the creator publishes the meanings of wParam, lParam and the return value
	Service functions must not return CALLSERVICE_NOTFOUND since that would confuse
	callers of TCallService().
	*)
    TCreateServiceFunction = function(const char: PChar; MIRANDASERVICE: TMIRANDASERVICE): THandle; cdecl;

	(* DestroyServiceFunction
	Removes the function associated with MIRANDASERVICE from the global service function
	list. Modules calling TCallService() will fail if they try to call this
	service's name. MIRANDASERVICE must have been returned by TCreateServiceFunction().
	Returns 0 on success or non-zero if MIRANDASERVICE is invalid.
	*)
    TCreateTransientServiceFunction = function(const char: PChar; MIRANDASERVICE: TMIRANDASERVICE): THandle; cdecl;

    TDestroyServiceFunction = function(Handle: THandle): int; cdecl;

	(* TCallService
	Finds and calls the service function 'char' using the parameters wParam and
	lParam.
	Returns CALLSERVICE_NOTFOUND if no service function called 'char' has been
	created, or the value the service function returned otherwise.
	*)
    TCallService = function(const char: PChar; wParam: WPARAM; lParam: LPARAM): int; cdecl;

	(* TServiceExists
	Finds if a service with the given name exists
	Returns nonzero if the service was found, and zero if it was not
	*)
    TServiceExists = function(const char: PChar): int; cdecl;

    PPLUGINLINK = ^TPLUGINLINK;
    TPLUGINLINK = record
        CreateHookableEvent: TCreateHookableEvent;
        DestroyHookableEvent: TDestroyHookableEvent;
        NotifyEventHooks: TNotifyEventHooks;
        HookEvent: THookEvent;
        HookEventMessage: THookEventMessage;
        UnhookEvent: TUnhookEvent;
        CreateServiceFunction: TCreateServiceFunction;
        CreateTransientServiceFunction: TCreateTransientServiceFunction;
        DestroyServiceFunction: TDestroyServiceFunction;
        CallService: TCallService;
        ServiceExists: TServiceExists;      // v0.1.0.1+
    end;

    { any module must export the below functions to be valid plugin
    the export names MUST be 'MirandaPluginInfo' 'Load' 'Unload' }

    TMirandaPluginInfo      = function(mirandaVersion: DWORD): PPLUGININFO; cdecl;
    TLoad                   = function(link: PPLUGINLINK): int; cdecl;
    TUnload                 = function: int; cdecl;

const
    CALLSERVICE_NOTFOUND        = $80000000;
    MIID_LAST: TGUID = '{00000000-0000-0000-0000-000000000000}';

{$ENDIF}
