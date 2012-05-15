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

    { modules.h is never defined -- no check needed }

    TMIRANDAHOOK = function(wParam: WPARAM; lParam: LPARAM): int; cdecl;
    TMIRANDASERVICE = function(wParam: WPARAM; lParam: LPARAM): int; cdecl;

    //see modules.h tor what all this stuff is

    TCreateHookableEvent = function(const char: PChar): THandle; cdecl;
    TDestroyHookableEvent = function(Handle: THandle): int; cdecl;
    TNotifyEventHooks = function(Handle: THandle; wParam: WPARAM; lParam: LPARAM): int; cdecl;
    THookEvent  = function(const char: PChar; MIRANDAHOOK: TMIRANDAHOOK): THandle; cdecl;
    THookEventMessage = function(const char: PChar; Wnd: THandle; wMsg: Integer): THandle; cdecl;
    TUnhookEvent = function(Handle: THandle): int; cdecl;
    TCreateServiceFunction = function(const char: PChar; MIRANDASERVICE: TMIRANDASERVICE): THandle; cdecl;
    TCreateTransientServiceFunction = function(const char: PChar; MIRANDASERVICE: TMIRANDASERVICE): THandle; cdecl;
    TDestroyServiceFunction = function(Handle: THandle): int; cdecl;
    TCallService = function(const char: PChar; wParam: WPARAM; lParam: LPARAM): int; cdecl;
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

{$ENDIF}
