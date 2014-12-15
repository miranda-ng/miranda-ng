unit syswin;
{$include compilers.inc}

interface

uses windows;

type
  tFFWFilterProc = function(fname:pWideChar):boolean;

const
  ThreadTimeout = 50;
const
  gffdMultiThread = 1;
  gffdOld         = 2;

function GetWorkOfflineStatus:integer;

function SendString(wnd:HWND;astr:PWideChar):integer; overload;
function SendString(wnd:HWND;astr:PAnsiChar):integer; overload;
procedure ProcessMessages;
function GetFocusedChild(wnd:HWND):HWND;
function GetAssoc(key:PAnsiChar):PAnsiChar;
function GetFileFromWnd(wnd:HWND;Filter:tFFWFilterProc;
         flags:dword=gffdMultiThread+gffdOld;TimeOut:cardinal=ThreadTimeout):pWideChar;

function WaitFocusedWndChild(wnd:HWND):HWND;

function ExecuteWaitW(AppPath:pWideChar; CmdLine:pWideChar=nil; DfltDirectory:PWideChar=nil;
         Show:dword=SW_SHOWNORMAL; TimeOut:dword=0; ProcID:PDWORD=nil):dword;
function ExecuteWait(AppPath:PAnsiChar; CmdLine:PAnsiChar=nil; DfltDirectory:PAnsiChar=nil;
         Show:dword=SW_SHOWNORMAL; TimeOut:dword=0; ProcID:PDWORD=nil):dword;

function GetEXEbyWnd(w:HWND; var dst:pWideChar):pWideChar; overload;
function GetEXEbyWnd(w:HWND; var dst:PAnsiChar):PAnsiChar; overload;
function IsExeRunning(exename:PWideChar):boolean; {hwnd}

implementation

uses
//  {$IFNDEF FPC}shellapi,{$ENDIF}
{$IFDEF COMPILER_16_UP}
  WinAPI.PsApi,
{$ELSE}
  psapi,
{$ENDIF}
  common,messages;

{$IFNDEF FPC} // shellapi import
function FindExecutableA(FileName, Directory: PAnsiChar; Result: PAnsiChar): HINST; stdcall;
         external 'shell32.dll' name 'FindExecutableA';
function FindExecutableW(FileName, Directory: PWideChar; Result: PWideChar): HINST; stdcall;
         external 'shell32.dll' name 'FindExecutableW';
{$ENDIF}

{$IFDEF COMPILER_16_UP}
type  pqword = ^int64;
{$ENDIF}

function ExecuteWaitW(AppPath:pWideChar; CmdLine:pWideChar=nil; DfltDirectory:PWideChar=nil;
         Show:dword=SW_SHOWNORMAL; TimeOut:dword=0; ProcID:PDWORD=nil):dword;
var
  flags: dword;
  {$IFDEF FPC}
  Startup: StartupInfo;
  {$ELSE}
  Startup: StartupInfoW;
  {$ENDIF}
  ProcInf: TProcessInformation;
  App: array [0..1023] of widechar;
  p:PWideChar;
  ext1,ext2:array [0..7] of widechar;
begin
  Result := cardinal(-1);
  if FindExecutableW(AppPath,DfltDirectory,App)<=32 then
    exit;
  if lstrcmpiw(GetExt(AppPath,ext1,7),GetExt(App,ext2,7))<>0 then
    CmdLine:=AppPath;
  flags := CREATE_NEW_CONSOLE;
  if Show = SW_HIDE then
    flags := flags or CREATE_NO_WINDOW;
  FillChar(Startup, SizeOf(Startup),0);
  with Startup do
  begin
    cb         :=SizeOf(Startup);
    wShowWindow:=Show;
    dwFlags    :=STARTF_USESHOWWINDOW;
  end;
  if ProcID <> nil then
    ProcID^ := 0;
  p:=StrEndW(App);
  if (CmdLine<>nil) and (CmdLine^<>#0) then
  begin
    p^:=' ';
    inc(p);
    StrCopyW(p,CmdLine);
  end;
  if CreateProcessW(nil,App,nil,nil,FALSE,flags,nil,DfltDirectory,Startup,ProcInf) then
  begin
    if TimeOut<>0 then
    begin
      if WaitForSingleObject(ProcInf.hProcess,TimeOut)=WAIT_OBJECT_0 then
      begin
        GetExitCodeProcess(ProcInf.hProcess,result);
        CloseHandle(ProcInf.hProcess);
      end
      else
      begin
        result:=1;
        if ProcID<>nil then
          ProcID^:=ProcInf.hProcess;
      end;
    end
    else
    begin
      GetExitCodeProcess(ProcInf.hProcess,result);
      CloseHandle(ProcInf.hProcess);
    end;
    CloseHandle(ProcInf.hThread);
  end;
end;

function ExecuteWait(AppPath:PAnsiChar; CmdLine:PAnsiChar=nil; DfltDirectory:PAnsiChar=nil;
         Show:dword=SW_SHOWNORMAL; TimeOut:dword=0; ProcID:PDWORD=nil):dword;
var
  flags: dword;
  {$IFDEF FPC}
  Startup: StartupInfo;
  {$ELSE}
  Startup: StartupInfoA;
  {$ENDIF}
  ProcInf: TProcessInformation;
  App: array [0..1023] of AnsiChar;
  p:PAnsiChar;
  ext1,ext2:array [0..7] of AnsiChar;
begin
  Result := cardinal(-1);
  if FindExecutableA(AppPath,DfltDirectory,App)<=32 then
    exit;
  if lstrcmpia(GetExt(AppPath,ext1,7),GetExt(App,ext2,7))<>0 then
    CmdLine:=AppPath;
  flags := CREATE_NEW_CONSOLE;
  if Show = SW_HIDE then
    flags := flags or CREATE_NO_WINDOW;
  FillChar(Startup, SizeOf(Startup),0);
  with Startup do
  begin
    cb         :=SizeOf(Startup);
    wShowWindow:=Show;
    dwFlags    :=STARTF_USESHOWWINDOW;
  end;
  if ProcID <> nil then
    ProcID^ := 0;
  p:=StrEnd(App);
  if (CmdLine<>nil) and (CmdLine^<>#0) then
  begin
    p^:=' ';
    inc(p);
    StrCopy(p,CmdLine);
  end;
  if CreateProcessA(nil,App,nil,nil,FALSE,flags,nil,DfltDirectory,Startup,ProcInf) then
  begin
    if TimeOut<>0 then
    begin
      if WaitForSingleObject(ProcInf.hProcess,TimeOut)=WAIT_OBJECT_0 then
      begin
        GetExitCodeProcess(ProcInf.hProcess,result);
        CloseHandle(ProcInf.hProcess);
      end
      else
      begin
        result:=1;
        if ProcID<>nil then
          ProcID^:=ProcInf.hProcess;
      end;
    end
    else
    begin
      GetExitCodeProcess(ProcInf.hProcess,result);
      CloseHandle(ProcInf.hProcess);
    end;
    CloseHandle(ProcInf.hThread);
  end;
end;

//----- Information functions -----

function GetWorkOfflineStatus:integer;
var
  lKey:HKEY;
  len,typ:dword;
begin
  result:=0;
  if RegOpenKeyEx(HKEY_CURRENT_USER,
    'Software\Microsoft\Windows\CurrentVersion\Internet Settings',0,
     KEY_READ,lKey)=ERROR_SUCCESS then
  begin
    len:=4;
    typ:=REG_DWORD;
    if RegQueryValueEx(lKey,'GlobalUserOffline',NIL,@typ,@result,@len)=ERROR_SUCCESS then
      ;
    RegCloseKey(lKey);
  end;
end;

function GetAssoc(key:PAnsiChar):PAnsiChar;
var
  lKey:HKEY;
  tmpbuf:array [0..511] of AnsiChar;
  len:integer;
begin
  result:=nil;
  if RegOpenKeyExA(HKEY_CLASSES_ROOT,key,0,
     KEY_READ,lKey)=ERROR_SUCCESS then
  begin
    len:=511;
    if (RegQueryValueExA(lKey,NIL,NIL,NIL,@tmpbuf,@len)=ERROR_SUCCESS) then
    begin
      StrDup(result,tmpbuf);
//      only path
//      while result[len]<>'\' do dec(len);
//      StrCopy(result,result+2,len-3);
    end;
    RegCloseKey(lKey);
  end;
end;

function GetFocusedChild(wnd:HWND):HWND;
var
  dwTargetOwner:dword;
  dwThreadID:dword;
  res:boolean;
begin
  dwTargetOwner:=GetWindowThreadProcessId(wnd,nil);
  dwThreadID:=GetCurrentThreadId();
  res:=false;
  if (dwTargetOwner<>dwThreadID) then
    res:=AttachThreadInput(dwThreadID,dwTargetOwner,TRUE);
  result:=GetFocus;
  if res then
    AttachThreadInput(dwThreadID,dwTargetOwner,FALSE);
end;

function WaitFocusedWndChild(wnd:HWND):HWND;
var
  T1,T2:integer;
  w:HWND;
begin
  Sleep(50);
  T1:=GetTickCount;
  repeat
    w:=GetTopWindow(wnd);
    if w=0 then w:=wnd;
    w:=GetFocusedChild(w);
    if w<>0 then
    begin
      wnd:=w;
      break;
    end;
    T2:=GetTickCount;
    if Abs(T1-T2)>100 then break;
  until false;
  Result:=wnd;
end;

function SendString(wnd:HWND;astr:PWideChar):integer;
var
  s,s0:PWideChar;
  style:integer;
begin
  result:=0;
  if (astr=nil) or (astr^=#0) then exit;
  if wnd=0 then
  begin
    wnd:=WaitFocusedWndChild(GetForegroundWindow);
    if wnd=0 then Exit;
  end;
  style:=GetWindowLongW(wnd,GWL_STYLE);
  if (style and (WS_DISABLED or ES_READONLY))=0 then
  begin
    StrDupW(s,astr); //??
    s0:=s;
    while s^<>#0 do
    begin
      if s^<>#10 then
        PostMessageW(wnd,WM_CHAR,ord(s^),1);
      Inc(s);
    end;
    mFreeMem(s0); //??
    result:=1;
  end;
end;

function SendString(wnd:HWND;astr:PAnsiChar):integer;
var
  s,s0:PAnsiChar;
  style:integer;
begin
  result:=0;
  if (astr=nil) or (astr^=#0) then exit;
  if wnd=0 then
  begin
    wnd:=WaitFocusedWndChild(GetForegroundWindow);
    if wnd=0 then Exit;
  end;
  style:=GetWindowLongA(wnd,GWL_STYLE);
  if (style and (WS_DISABLED or ES_READONLY))=0 then
  begin
    StrDup(s,astr); //??
    s0:=s;
    while s^<>#0 do
    begin
      if s^<>#10 then
        PostMessageA(wnd,WM_CHAR,ord(s^),1);
      Inc(s);
    end;
    mFreeMem(s0); //??
    result:=1;
  end;
end;

procedure ProcessMessages;
var
  Unicode: Boolean;
  MsgExists: Boolean;
  Msg:tMsg;
begin
  repeat
    if PeekMessageA(Msg,0,0,0,PM_NOREMOVE) then
    begin
      Unicode:=(Msg.hwnd<>0) and IsWindowUnicode(Msg.hwnd);
      if Unicode then
        MsgExists:=PeekMessageW(Msg,0,0,0,PM_REMOVE)
      else
        MsgExists:=PeekMessageA(Msg,0,0,0,PM_REMOVE);
      if not MsgExists then break;

      if Msg.Message<>WM_QUIT then
      begin
        TranslateMessage({$IFDEF FPC}@{$ENDIF}Msg);
        if Unicode then
          DispatchMessageW({$IFDEF FPC}@{$ENDIF}Msg)
        else
          DispatchMessageA({$IFDEF FPC}@{$ENDIF}Msg);
      end;
    end
    else
      break;
  until false;
end;

//----- work with EXE -----

function GetEXEbyWnd(w:HWND; var dst:pWideChar):pWideChar;
var
  hProcess:THANDLE;
  ProcID:dword;
  ModuleName: array [0..300] of WideChar;
begin
  dst:=nil;
  GetWindowThreadProcessId(w,@ProcID);
  if ProcID<>0 then
  begin
    hProcess:=OpenProcess(PROCESS_QUERY_INFORMATION or PROCESS_VM_READ,False,ProcID);
    if hProcess<>0 then
    begin
      ModuleName[0]:=#0;
      GetModuleFilenameExW(hProcess,0,ModuleName,SizeOf(ModuleName));
      StrDupW(dst,ModuleName);
      CloseHandle(hProcess);
    end;
  end;
  result:=dst;
end;

function GetEXEbyWnd(w:HWND; var dst:PAnsiChar):PAnsiChar;
var
  hProcess:THANDLE;
  ProcID:dword;
  ModuleName: array [0..300] of AnsiChar;
begin
  dst:=nil;
  GetWindowThreadProcessId(w,@ProcID);
  if ProcID<>0 then
  begin
    hProcess:=OpenProcess(PROCESS_QUERY_INFORMATION or PROCESS_VM_READ,False,ProcID);
    if hProcess<>0 then
    begin
      ModuleName[0]:=#0;
      GetModuleFilenameExA(hProcess,0,ModuleName,SizeOf(ModuleName));
      StrDup(dst,ModuleName);
      CloseHandle(hProcess);
    end;
  end;
  result:=dst;
end;

function IsExeRunning(exename:PWideChar):boolean;{hwnd}
const
  nCount = 4096;
var
  Processes:array [0..nCount-1] of dword;
  nProcess:dword;
  hProcess:THANDLE;
  ModuleName: array [0..300] of WideChar;
  i:integer;
begin
  result:=false;
  EnumProcesses(pointer(@Processes),nCount*SizeOf(dword),nProcess);
  nProcess:=(nProcess div 4)-1;
  for i:=2 to nProcess do //skip Idle & System
  begin
    hProcess:=OpenProcess(PROCESS_QUERY_INFORMATION or PROCESS_VM_READ,
      False,Processes[i]);
    if hProcess<>0 then
    begin
      GetModuleFilenameExW(hProcess,0,ModuleName,SizeOf(ModuleName));
      result:=lstrcmpiw(ExtractW(ModuleName,true),exename)=0;
      CloseHandle(hProcess);
      if result then exit;
    end;
  end;
end;

//----- work with handles -----
function GetProcessHandleCount(hProcess:THANDLE;var pdwHandleCount:dword):bool; stdcall; external 'kernel32.dll';

function NtQueryObject(ObjectHandle:THANDLE;ObjectInformationClass:integer;
         ObjectInformation:pointer;Length:ulong;var ResultLength:longint):cardinal; stdcall; external 'ntdll.dll';

const
  ObjectNameInformation = 1; // +4 bytes
  ObjectTypeInformation = 2; // +$60 bytes
const
  STATUS_INFO_LENGTH_MISMATCH = $C0000004;

function TranslatePath(fn:PWideChar):PWideChar;
const
  LANPrefix:PWideChar = '\Device\LanmanRedirector\';
var
  szTemp:array [0..511] of WideChar;
  szName:array [0..511] of WideChar;
  p:PWideChar;
  uNameLen:word;
  szTempFile:array [0..511] of WideChar;
begin
  if StrPosW(fn,LANPrefix)=fn then
  begin
    uNameLen:=StrLenW(LANPrefix);
    mGetMem(result,(StrLenW(fn)-uNameLen+3)*SizeOf(WideChar));
    result[0]:='\';
    result[1]:='\';
    StrCopyW(result+2,fn+uNameLen);
    exit;
  end;
  if GetLogicalDriveStringsW(255,@szTemp)<>0 then
  begin
    p:=szTemp;
    repeat
      p[2]:=#0;
      if QueryDosDeviceW(p,szName,255)<>0 then
      begin
        uNameLen:=StrLenW(szName)+1;
        if uNameLen<255 then
        begin
          StrCopyW(szTempFile,fn,uNameLen-1);
          if lstrcmpiw(szTempFile,szName)=0 then
          begin
            mGetMem(result,(StrLenW(fn+uNameLen)+4)*SizeOf(WideChar));
            result[0]:=WideChar(ORD(p[0]));
            result[1]:=':';
            result[2]:='\';
            StrCopyW(result+3,fn+uNameLen);
            exit;
          end;
        end;
      end;
      inc(p,4);
    until p^=#0;
  end;
  StrDupW(result,fn);
end;

const
  maxhandles = 15;
var
  har,hold:array [0..maxhandles-1] of PWideChar;
  harcnt:integer;
const
  oldcnt:integer=0;

procedure ArSwitch(idx:integer);
var
  h:pWideChar;
begin
//clear old
  while oldcnt>0 do
  begin
    dec(oldcnt);
    FreeMem(hold[oldcnt]);
  end;
//copy new to old
  move(har,hold,SizeOf(har));
  oldcnt:=harcnt;
// move active to begin
  if idx<>0 then
  begin
    h        :=hold[idx];
    hold[idx]:=hold[0];
    hold[0]  :=h;
  end;
end;

function CheckHandles(ReturnNew:bool):integer;
var
  i,j:integer;
  flg:boolean;
begin
  result:=0;
  if oldcnt=0 then //first time
  begin
    ArSwitch(0);
    exit;
  end;
  i:=0;
  if ReturnNew then
  begin
    while i<harcnt do
    begin
      flg:=false;
      j:=0;
      while j<oldcnt do
      begin
        if StrCmpW(har[i],hold[j])=0 then
        begin
          flg:=true; //old=new
          break;
        end;
        inc(j);
      end;
      if not flg then  // new!!
      begin
        ArSwitch(i);
        exit;
      end;
      inc(i);
    end;
  end
  else
  begin
    while i<oldcnt do
    begin
      j:=0;
      while j<harcnt do
      begin
        if StrCmpW(hold[i],har[j])=0 then
        begin
          ArSwitch(j);
          exit;
        end;
        inc(j);
      end;
      inc(i);
    end;
  end;
  ArSwitch(0);
  result:=-1;
end;

const
  MaxHandle = $2000;

type
  ptrec = ^trec;
  trec = record
    handle:THANDLE;
    fname:pWideChar;
  end;

type
  pint_ptr  = ^int_ptr;

function GetName(param:pointer):integer; //stdcall;
const
  BufSize = $800;
  // depends of record align
  offset=SizeOf(pointer) div 2; // 4 for win64, 2 for win32
var
  tmpbuf:array [0..BufSize-1] of WideChar;
var
  dummy:longint;
  size:integer;
  pc:pWideChar;
begin
  result:=0;

  if NtQueryObject(ptrec(param)^.handle,ObjectNameInformation,
     @tmpbuf,BufSize*SizeOf(WideChar),dummy)=0 then
  begin
    // UNICODE_STRING: 2b - length, 2b - maxlen, (align), next - pWideChar
    size:=pword(@tmpbuf)^; // length in bytes
    if size>=0 then
    begin
      GetMem(ptrec(param)^.fname,size+SizeOf(WideChar)); // length in bytes

      pc:=pWideChar(pint_ptr(@tmpbuf[offset])^);
      move(pc^,ptrec(param)^.fname^,size); // can be without zero
      pword(pAnsiChar(ptrec(param)^.fname)+size)^:=0;
    end
    else
      ptrec(param)^.fname:=nil;
  end;
end;

function TestHandle(Handle:THANDLE;MultiThread:bool;TimeOut:cardinal):pWideChar;
var
  hThread:THANDLE;
  rec:trec;
//  dummy:longint;
  res:{$IFDEF COMPILER_16_UP}Longword{$ELSE}uint_ptr{$ENDIF};
begin
  result:=nil;
{
  // check what it - file
  if (NtQueryObject(Handle,ObjectTypeInformation,
     @tmpbuf,BufSize*SizeOf(WideChar),dummy)<>0) or
     (StrCmpW(tmpbuf+$30,'File')<>0) then
    Exit;
}
  // check what it disk file
//!!! need to check again
  if GetFileType(Handle)<>FILE_TYPE_DISK then exit;

  rec.handle:=Handle;
  rec.fname:=nil;

  if not MultiThread then
  begin
    GetName(@rec);
    result:=rec.fname;
  end
  else
  begin
    hThread:=BeginThread(nil,0,@GetName,@rec,0,res);
    if WaitForSingleObject(hThread,TimeOut)=WAIT_TIMEOUT then
    begin
      TerminateThread(hThread,0);
    end
    else
      result:=rec.fname;
    CloseHandle(hThread);
  end;
end;

function GetFileFromWnd(wnd:HWND;Filter:tFFWFilterProc;
         flags:dword=gffdMultiThread+gffdOld;TimeOut:cardinal=ThreadTimeout):pWideChar;
var
  hProcess,h:THANDLE;
  pid:THANDLE;
  i:THANDLE;
  c:THANDLE;
  handles:dword;
  pc:pWideChar;
begin
  result:=nil;
  GetWindowThreadProcessId(wnd,@c);
  pid:=OpenProcess(//PROCESS_VM_READ or
    PROCESS_DUP_HANDLE or PROCESS_QUERY_INFORMATION {or PROCESS_QUERY_LIMITED_INFORMATION},
    true,c);
  if pid=0 then exit;
  harcnt:=0;
  if GetProcessHandleCount(pid,handles) then
  begin
    handles:=handles*4; // count no matter, check "every 4th" handle
//    Handles:=Handles*SizeOf(THANDLE);
    hProcess:=GetCurrentProcess;
    i:=SIZEOF(THANDLE); // skip first

    while true do
    begin
      if DuplicateHandle(pid,i,hProcess,@h,GENERIC_READ,false,0) then
      begin
        pc:=TestHandle(h,(flags and gffdMultiThread)<>0,TimeOut);
        if pc<>nil then
        begin
  //        if GetFileType(h)=FILE_TYPE_DISK then
          begin
            if (@Filter=nil) or Filter(pc) and (harcnt<maxhandles) then
            begin
              har[harcnt]:=pc;
              inc(harcnt);
            end
            else
              FreeMem(pc);
          end;
        end;
        CloseHandle(h);
      end
      else
      begin
//        inc(handles,SizeOf(THANDLE)); //????skip empty number and non-duplicates
        inc(handles,4); //????skip empty number and non-duplicates
        if handles>MaxHandle then break; //file not found
      end;
      inc(i,4);
//!!      inc(i,SizeOf(THANDLE));
      if i>handles then
        break;
    end;
  end;

  CloseHandle(pid);
  if harcnt>0 then
  begin
    CheckHandles((flags and gffdOld)=0);
    result:=TranslatePath(hold[0]);
  end
end;

procedure ClearHandles;
begin
  while oldcnt>0 do
  begin
    dec(oldcnt);
    FreeMem(hold[oldcnt]);
  end;
end;

initialization
finalization
  ClearHandles;
end.
