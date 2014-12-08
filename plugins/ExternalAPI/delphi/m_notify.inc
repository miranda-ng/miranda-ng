{$IFNDEF M_NOTIFY}
{$DEFINE M_NOTIFY}

{** Miranda Notify Dispatcher ************************************************
Notify Dispatcher provides common interface to different notification plugins
like osd, popup, ticker etc.
******************************************************************************}

const
{ Options UI event and service. The same as for miranda options }
  ME_NOTIFY_OPT_INITIALISE:PAnsiChar = 'Notify/Opt/Initialise';
  MS_NOTIFY_OPT_ADDPAGE:PAnsiChar    = 'Notify/Opt/AddPage';

type
  PMNOTIFYACTIONINFO = ^TMNOTIFYACTIONINFO;
  TMNOTIFYACTIONINFO = record
    icon   :HICON;
    name   :array [0..MAXMODULELABELLENGTH-1] of AnsiChar;
    service:array [0..MAXMODULELABELLENGTH-1] of AnsiChar;
    cookie :dword;
  end;

// Just like miranda pluginLink... This should work faster then services,
// we need some reactivity in notifications.
type
  PMNOTIFYLINK = ^TMNOTIFYLINK;
  TMNOTIFYLINK = record
    // Create a new notification type
    Register:function(name:PAnsiChar;icon:HICON):THANDLE;cdecl;

    // Create a new notification object
    Create:function(atype:THANDLE):THANDLE;cdecl;

    // Check is handle is a valid notification object
    IsValid:function(notify:THANDLE):integer;cdecl;

    // Set/get information about object, or type defaults
    _Set:function(notifyORtype:THANDLE;name:PAnsiChar;val:TDBVARIANT):integer;cdecl;
    _Get:function(notifyORtype:THANDLE;name:PAnsiChar;val:PDBVARIANT):integer;cdecl;

    // Set/get actions
    AddAction :function(notifyORtype:THANDLE;icon:HICON;name:PAnsiChar;service:PAnsiChar;cookie:dword):integer;cdecl;
    GetActions:function(notifyORtype:THANDLE;actions:PMNOTIFYACTIONINFO):integer;cdecl;

    // Increment/decrement refer count of notification object. Unreferred objects are destroyed
    AddRef :function(notify:THANDLE):integer;cdecl;
    Release:function(notify:THANDLE):integer;cdecl;

    // Notify user
    Show  :procedure(notify:THANDLE);cdecl;
    Update:procedure(notify:THANDLE);cdecl;
    Remove:procedure(notify:THANDLE);cdecl;
  end;

const
// Get the MNOTIFYLINK struct
// result = (LRESULT)(MNOTIFYLINK* )notifyLink
  MS_NOTIFY_GETLINK:PAnsiChar = 'Notify/GetLink';

// Hook this to process corresponding actions
  ME_NOTIFY_SHOW  :PAnsiChar = 'Notify/Show';
  ME_NOTIFY_UPDATE:PAnsiChar = 'Notify/Update';
  ME_NOTIFY_REMOVE:PAnsiChar = 'Notify/Remove';

var
  notifyLink:PMNOTIFYLINK;
(*
function MNotifyRegister(name:PAnsiChar;icon:HICON):THANDLE;
begin
  if notifyLink<>nil then
    result:=notifyLink^.Register(name,icon)
  else
    result:=0;
end;
function MNotifyCreate(atype:THANDLE):THANDLE;
begin
  if notifyLink<>nil then
    result:=notifyLink^.Create(atype)
  else
    result:=0;
end;
function MNotifyIsValid(notify:THANDLE):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.IsValid(notify)
  else
    result:=0;
end;
function MNotifySet(notifyORtype:THANDLE,name:PAnsiChar;val:TDBVARIANT):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.Set(notifyORtype,name,val)
  else
    result:=0;
end;
function MNotifyGet(notifyORtype:THANDLE,name:PAnsiChar;val:PDBVARIANT):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.Get(notifyORtype,name,val)
  else
    result:=0;
end;
function MNotifyAddAction(notifyORtype:THANDLE;icon:HICON;name:PAnsiChar;service:PAnsiChar=nil;cookie:dword=0):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.AddAction(notifyORtype,icon,name)
  else
    result:=0;
end;
function MNotifyGetActions(notifyORtype:THANDLE;actions:PMNOTIFYACTIONINFO):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.GetActions(notifyORtype,actions)
  else
    result:=0;
end;
function MNotifyAddRef(notify:THANDLE):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.AddRef(notify)
  else
    result:=0;
end;
function MNotifyRelease(notify:THANDLE):int;
begin
  if notifyLink<>nil then
    result:=notifyLink^.Release(notify)
  else
    result:=0;
end;
procedure MNotifyShow(notify:THANDLE);
begin
  if notifyLink<>nil then
    notifyLink^.Show(notify)
end;
procedure MNotifyUpdate(notify:THANDLE);
begin
  if notifyLink<>nil then
    notifyLink^.Update(notify)
end;
procedure MNotifyRemove(notify:THANDLE);
begin
  if notifyLink<>nil then
    notifyLink^.Remove(notify)
end;

procedure MNotifyGetLink;
begin
  if PluginLink^.ServiceExists(MS_NOTIFY_GETLINK)<>0 then
    notifyLink:=PMNOTIFYLINK(CallService(MS_NOTIFY_GETLINK,0,0))
  else
    notifyLink:=nil;
end;

// get helpers
function MNotifyGetByte(notifyORtype:THANDLE;name:PAnsiChar;defValue:byte):byte;
var
  dbv:TDBVARIANT;
begin
  MNotifyGet(notifyORtype,name,dbv);
  if dbv._type<>DBVT_BYTE then
    result:=defValue
  else
    result:=dbv.bVal;
end;
function MNotifyGetWord(notifyORtype:THANDLE;name:PAnsiChar;defValue:word):word;
var
  dbv:TDBVARIANT;
begin
  MNotifyGet(notifyORtype,name,dbv);
  if dbv._type<>DBVT_WORD then
    result:=defValue
  else
   result:=dbv.wVal;
end;
function MNotifyGetDWord(notifyORtype:THANDLE;name:PAnsiChar;defValue:dword):dword;
var
  dbv:TDBVARIANT;
begin
  MNotifyGet(notifyORtype,name,dbv);
  if dbv._type<>DBVT_DWORD then
    result:=defValue
  else
    result:=dbv.dVal;
end;
function MNotifyGetString(notifyORtype:THANDLE;name:PAnsiChar;defValue:PAnsiChar):PAnsiChar;
var
  dbv:TDBVARIANT;
begin
  MNotifyGet(notifyORtype,name,dbv);
  if dbv._type<>DBVT_ASCIIZ then
    result:=defValue
  else
    result:=dbv.szVal.a;
end;
function MNotifyGetWString(notifyORtype:THANDLE;name:PAnsiChar;defValue:PWideChar):PWideChar;
var
  dbv:TDBVARIANT;
begin
  MNotifyGet(notifyORtype,name,dbv);
  if dbv._type<>DBVT_WCHAR then
    result:=defValue
  else
    result:=dbv.szVal.w;
end;

// set helpers
procedure MNotifySetByte(notifyORtype:THANDLE;name:PAnsiChar;value:byte);
var
  dbv:TDBVARIANT;
begin
  dbv._type:=DBVT_BYTE;
  dbv.bVal :=value;
  MNotifySet(notifyORtype,name,dbv);
end;
procedure MNotifySetWord(notifyORtype:THANDLE;name:PAnsiChar;value:word);
var
  dbv:TDBVARIANT;
begin
  dbv._type:=DBVT_WORD;
  dbv.wVal :=value;
  MNotifySet(notifyORtype,name,dbv);
end;
procedure MNotifySetDWord(notifyORtype:THANDLE;name:PAnsiChar;value:dword);
var
  dbv:TDBVARIANT;
begin
  dbv._type:=DBVT_DWORD;
  dbv.dVal :=value;
  MNotifySet(notifyORtype,name,dbv);
end;
procedure MNotifySetString(notifyORtype:THANDLE;name:PAnsiChar;value:PAnsiChar);
var
  dbv:TDBVARIANT;
begin
  dbv._type  :=DBVT_ASCIIZ;
  dbv.szVal.a:=value;
  MNotifySet(notifyORtype,name,dbv);
end;
procedure MNotifySetWString(notifyORtype:THANDLE;name:PAnsiChar;value:PWideChar);
var
  dbv:TDBVARIANT;
begin
  dbv._type  :=DBVT_WCHAR;
  dbv.szVal.w:=value;
  MNotifySet(notifyORtype,name,dbv);
end;
*)

const
// Common options for Get/Set actions
  NFOPT_TYPENAME  = 'General/TypeName';
  NFOPT_ICON      = 'General/Icon';
  NFOPT_CONTACT   = 'General/Contact';
  NFOPT_EVENT     = 'General/Event';
  NFOPT_TEXT      = 'General/Text';
  NFOPT_TEXTW     = 'General/TextW';
  NFOPT_TITLE     = 'General/Title';
  NFOPT_TITLEW    = 'General/TitleW';
  NFOPT_BACKCOLOR = 'General/BackColor';
  NFOPT_TEXTCOLOR = 'General/TextColor';
  NFOPT_TIMEOUT   = 'General/Timeout';
//  NFOPT_ONDESTROY = 'General/OnDestroy';

{$ENDIF}
