{$IFNDEF M_FUSE}
{$DEFINE M_FUSE}

const
  FUSE_INIT   = 0; // core started, Param=**FUSE_LINK
  FUSE_DEINIT = 1; // core stopped
  FUSE_DEFMOD = 3; // LoadDefaultModules() return code, Param=*int
  FUSE_DEATH  = 4; // DestroyingModularEngine() just got called

type
  PFUSE_LINK = ^TFUSE_LINK;
  TFUSE_LINK = record
    cbSize : longint;
    CreateHookableEvent  : function (para1:PAnsiChar):THANDLE;cdecl;
    DestroyHookableEvent : function (para1:THANDLE):longint;cdecl;
    NotifyEventHooks : function (para1:THANDLE; para2:WPARAM; para3:LPARAM):longint;cdecl;
    HookEvent        : function (para1:PAnsiChar; para2:TMIRANDAHOOK):THANDLE;cdecl;
    HookEventMessage : function (para1:PAnsiChar; para2:HWND; para3:dword):THANDLE;cdecl;
    UnhookEvent      : function (para1:THANDLE):longint;cdecl;
    CreateServiceFunction : function (para1:PAnsiChar; para2:TMIRANDASERVICE):THANDLE;cdecl;
    CreateTransientServiceFunction : function (para1:PAnsiChar; para2:TMIRANDASERVICE):THANDLE;cdecl;
    DestroyServiceFunction : function (para1:THANDLE):longint;cdecl;
    CallService     : function (para1:PAnsiChar; para2:WPARAM; para3:LPARAM):longint;cdecl;
    ServiceExists   : function (para1:PAnsiChar):longint;cdecl; {v0.1.0.1+ }
    CallServiceSync : function (para1:PAnsiChar; para2:WPARAM; para3:LPARAM):longint;cdecl; {v0.1.2.2+ }
  end;

{$ENDIF}
