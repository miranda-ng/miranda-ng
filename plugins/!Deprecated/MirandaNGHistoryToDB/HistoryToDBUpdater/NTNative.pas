unit NTNative;
 
interface

uses Classes, SysUtils, Windows;

type
  Pointer32 = ULONG;
  THANDLE32 = ULONG;

  _UNICODE_STRING = record
    Length: Word;
    MaximumLength: Word;
    Buffer: LPWSTR;
  end;
  UNICODE_STRING = _UNICODE_STRING;

  //http://msdn.microsoft.com/en-us/library/windows/desktop/ms684280%28v=vs.85%29.aspx
  PROCESS_BASIC_INFORMATION = record
    Reserved1 : Pointer;
    PebBaseAddress: Pointer;
    Reserved2: array [0..1] of Pointer;
    UniqueProcessId: ULONG_PTR;
    Reserved3: Pointer;
  end;


  //http://undocumented.ntinternals.net/UserMode/Structures/RTL_DRIVE_LETTER_CURDIR.html
  _RTL_DRIVE_LETTER_CURDIR = record
    Flags: Word;
    Length: Word;
    TimeStamp: ULONG;
    DosPath: UNICODE_STRING;
  end;
  RTL_DRIVE_LETTER_CURDIR = _RTL_DRIVE_LETTER_CURDIR;

   _CURDIR = record
    DosPath: UNICODE_STRING;
    Handle: THANDLE;
  end;
  CURDIR = _CURDIR;

 //http://undocumented.ntinternals.net/UserMode/Structures/RTL_USER_PROCESS_PARAMETERS.html
  _RTL_USER_PROCESS_PARAMETERS = record
    MaximumLength: ULONG;
    Length: ULONG;
    Flags: ULONG;
    DebugFlags: ULONG;
    ConsoleHandle: THANDLE;
    ConsoleFlags: ULONG;
    StandardInput: THANDLE;
    StandardOutput: THANDLE;
    StandardError: THANDLE;
    CurrentDirectory: CURDIR;
    DllPath: UNICODE_STRING;
    ImagePathName: UNICODE_STRING;
    CommandLine: UNICODE_STRING;
    Environment: Pointer;
    StartingX: ULONG;
    StartingY: ULONG;
    CountX: ULONG;
    CountY: ULONG;
    CountCharsX: ULONG;
    CountCharsY: ULONG;
    FillAttribute: ULONG;
    WindowFlags: ULONG;
    ShowWindowFlags: ULONG;
    WindowTitle: UNICODE_STRING;
    DesktopInfo: UNICODE_STRING;
    ShellInfo: UNICODE_STRING;
    RuntimeData: UNICODE_STRING;
    CurrentDirectories: array[0..31] of RTL_DRIVE_LETTER_CURDIR;
  end;
  RTL_USER_PROCESS_PARAMETERS = _RTL_USER_PROCESS_PARAMETERS;
  PRTL_USER_PROCESS_PARAMETERS = ^RTL_USER_PROCESS_PARAMETERS;

  _PEB = record
    Reserved1     : array [0..1] of Byte;
    BeingDebugged : Byte;
    Reserved2     : Byte;
    Reserved3     : array [0..1] of Pointer;
    Ldr           : Pointer;
    ProcessParameters : PRTL_USER_PROCESS_PARAMETERS;
    Reserved4     : array [0..102] of Byte;
    Reserved5     : array [0..51] of Pointer;
    PostProcessInitRoutine : Pointer;
    Reserved6     : array [0..127] of byte;
    Reserved7     : Pointer;
    SessionId     : ULONG;
  end;
  PEB=_PEB;

{$IFDEF CPUX64}
  _UNICODE_STRING32 = record
    Length: Word;
    MaximumLength: Word;
    Buffer: Pointer32;
  end;
  UNICODE_STRING32 = _UNICODE_STRING32;

  _RTL_DRIVE_LETTER_CURDIR32 = record
    Flags: Word;
    Length: Word;
    TimeStamp: ULONG;
    DosPath: UNICODE_STRING32;
  end;
  RTL_DRIVE_LETTER_CURDIR32 = _RTL_DRIVE_LETTER_CURDIR32;

   _CURDIR32 = record
    DosPath: UNICODE_STRING32;
    Handle: THANDLE32;
  end;
  CURDIR32 = _CURDIR32;

  _RTL_USER_PROCESS_PARAMETERS32 = record
    MaximumLength: ULONG;
    Length: ULONG;
    Flags: ULONG;
    DebugFlags: ULONG;
    ConsoleHandle: THANDLE32;
    ConsoleFlags: ULONG;
    StandardInput: THANDLE32;
    StandardOutput: THANDLE32;
    StandardError: THANDLE32;
    CurrentDirectory: CURDIR32;
    DllPath: UNICODE_STRING32;
    ImagePathName: UNICODE_STRING32;
    CommandLine: UNICODE_STRING32;
    Environment: Pointer32;
    StartingX: ULONG;
    StartingY: ULONG;
    CountX: ULONG;
    CountY: ULONG;
    CountCharsX: ULONG;
    CountCharsY: ULONG;
    FillAttribute: ULONG;
    WindowFlags: ULONG;
    ShowWindowFlags: ULONG;
    WindowTitle: UNICODE_STRING32;
    DesktopInfo: UNICODE_STRING32;
    ShellInfo: UNICODE_STRING32;
    RuntimeData: UNICODE_STRING32;
    CurrentDirectories: array[0..31] of RTL_DRIVE_LETTER_CURDIR32;
  end;
  RTL_USER_PROCESS_PARAMETERS32 = _RTL_USER_PROCESS_PARAMETERS32;
  PRTL_USER_PROCESS_PARAMETERS32 = ^RTL_USER_PROCESS_PARAMETERS32;

  _PEB32 = record
    Reserved1     : array [0..1] of Byte;
    BeingDebugged : Byte;
    Reserved2     : Byte;
    Reserved3     : array [0..1] of Pointer32;
    Ldr           : Pointer32;
    ProcessParameters : Pointer32;//PRTL_USER_PROCESS_PARAMETERS;
    Reserved4     : array [0..102] of Byte;
    Reserved5     : array [0..51] of Pointer32;
    PostProcessInitRoutine : Pointer32;
    Reserved6     : array [0..127] of byte;
    Reserved7     : Pointer32;
    SessionId     : ULONG;
  end;
   PEB32=_PEB32;
{$ENDIF}

type
  TIsWow64Process = function(Handle:THandle; var IsWow64 : BOOL) : BOOL; stdcall;
var
  _IsWow64Process  : TIsWow64Process;

function NtQueryInformationProcess(ProcessHandle : THandle; ProcessInformationClass : DWORD; ProcessInformation : Pointer; ProcessInformationLength : ULONG; ReturnLength : PULONG ): LongInt; stdcall; external 'ntdll.dll';
function NtQueryVirtualMemory(ProcessHandle : THandle; BaseAddress : Pointer;  MemoryInformationClass : DWORD;  MemoryInformation : Pointer;  MemoryInformationLength : ULONG; ReturnLength : PULONG ): LongInt; stdcall; external 'ntdll.dll';
function ProcessIsX64(hProcess: DWORD): Boolean;
procedure Init_IsWow64Process;

implementation

procedure Init_IsWow64Process;
var
  hKernel32      : Integer;
begin
  hKernel32 := LoadLibrary(kernel32);
  if (hKernel32 = 0) then RaiseLastOSError;
  try
    _IsWow64Process := GetProcAddress(hkernel32, 'IsWow64Process');
  finally
    FreeLibrary(hKernel32);
  end;
end;

function ProcessIsX64(hProcess: DWORD): Boolean;
var
  IsWow64: BOOL;
begin
  Result := False;
  {$IFNDEF CPUX64}
    Exit;
  {$ENDIF}
  if not Assigned(_IsWow64Process) then
   Init_IsWow64Process;

  if Assigned(_IsWow64Process) then
  begin
    if (_IsWow64Process(hProcess, IsWow64)) then
      Result := not IsWow64
    else
      RaiseLastOSError;
  end;
end;

end.
