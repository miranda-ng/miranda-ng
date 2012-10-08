unit PsAPI;

interface

{$WEAKPACKAGEUNIT}

uses Windows;

{$HPPEMIT '#include <psapi.h>'}

type
  PPointer = ^Pointer;

  TEnumProcesses = function (lpidProcess: LPDWORD; cb: DWORD; var cbNeeded: DWORD): BOOL stdcall;
  TEnumProcessModules = function (hProcess: THandle; lphModule: LPDWORD; cb: DWORD;
    var lpcbNeeded: DWORD): BOOL stdcall;
  TGetModuleBaseNameA = function (hProcess: THandle; hModule: HMODULE;
    lpBaseName: PAnsiChar; nSize: DWORD): DWORD stdcall;
  TGetModuleBaseNameW = function (hProcess: THandle; hModule: HMODULE;
    lpBaseName: PWideChar; nSize: DWORD): DWORD stdcall;
  TGetModuleBaseName = TGetModuleBaseNameA;
  TGetModuleFileNameExA = function (hProcess: THandle; hModule: HMODULE;
    lpFilename: PAnsiChar; nSize: DWORD): DWORD stdcall;
  TGetModuleFileNameExW = function (hProcess: THandle; hModule: HMODULE;
    lpFilename: PWideChar; nSize: DWORD): DWORD stdcall;
  TGetModuleFileNameEx = TGetModuleFileNameExA;

  {$EXTERNALSYM _MODULEINFO}
  _MODULEINFO = packed record
    lpBaseOfDll: Pointer;
    SizeOfImage: DWORD;
    EntryPoint: Pointer;
  end;
  {$EXTERNALSYM MODULEINFO}
  MODULEINFO = _MODULEINFO;
  {$EXTERNALSYM LPMODULEINFO}
  LPMODULEINFO = ^_MODULEINFO;
  TModuleInfo = _MODULEINFO;
  PModuleInfo = LPMODULEINFO;

  TGetModuleInformation = function (hProcess: THandle; hModule: HMODULE;
    lpmodinfo: LPMODULEINFO; cb: DWORD): BOOL stdcall;
  TEmptyWorkingSet = function (hProcess: THandle): BOOL stdcall;
  TQueryWorkingSet = function (hProcess: THandle; pv: Pointer; cb: DWORD): BOOL stdcall;
  TInitializeProcessForWsWatch = function (hProcess: THandle): BOOL stdcall;

  {$EXTERNALSYM _PSAPI_WS_WATCH_INFORMATION}
  _PSAPI_WS_WATCH_INFORMATION = packed record
    FaultingPc: Pointer;
    FaultingVa: Pointer;
  end;
  {$EXTERNALSYM PSAPI_WS_WATCH_INFORMATION}
  PSAPI_WS_WATCH_INFORMATION = _PSAPI_WS_WATCH_INFORMATION;
  {$EXTERNALSYM PPSAPI_WS_WATCH_INFORMATION}
  PPSAPI_WS_WATCH_INFORMATION = ^_PSAPI_WS_WATCH_INFORMATION;
  TPSAPIWsWatchInformation = _PSAPI_WS_WATCH_INFORMATION;
  PPSAPIWsWatchInformation = PPSAPI_WS_WATCH_INFORMATION;

  TGetWsChanges = function (hProcess: THandle; lpWatchInfo: PPSAPI_WS_WATCH_INFORMATION;
    cb: DWORD): BOOL stdcall;

  TGetMappedFileNameA = function (hProcess: THandle; lpv: Pointer;
    lpFilename: PAnsiChar; nSize: DWORD): DWORD stdcall;
  TGetMappedFileNameW = function (hProcess: THandle; lpv: Pointer;
    lpFilename: PWideChar; nSize: DWORD): DWORD stdcall;
  TGetMappedFileName = TGetMappedFileNameA;
  TGetDeviceDriverBaseNameA = function (ImageBase: Pointer; lpBaseName: PAnsiChar;
    nSize: DWORD): DWORD stdcall;
  TGetDeviceDriverBaseNameW = function (ImageBase: Pointer; lpBaseName: PWideChar;
    nSize: DWORD): DWORD stdcall;
  TGetDeviceDriverBaseName = TGetDeviceDriverBaseNameA;
  TGetDeviceDriverFileNameA = function (ImageBase: Pointer; lpFileName: PAnsiChar;
    nSize: DWORD): DWORD stdcall;
  TGetDeviceDriverFileNameW = function (ImageBase: Pointer; lpFileName: PWideChar;
    nSize: DWORD): DWORD stdcall;
  TGetDeviceDriverFileName = TGetDeviceDriverFileNameA;

  TEnumDeviceDrivers = function (lpImageBase: PPointer; cb: DWORD;
    var lpcbNeeded: DWORD): BOOL stdcall;

  {$EXTERNALSYM _PROCESS_MEMORY_COUNTERS}
  _PROCESS_MEMORY_COUNTERS = packed record
    cb: DWORD;
    PageFaultCount: DWORD;
    PeakWorkingSetSize: DWORD;
    WorkingSetSize: DWORD;
    QuotaPeakPagedPoolUsage: DWORD;
    QuotaPagedPoolUsage: DWORD;
    QuotaPeakNonPagedPoolUsage: DWORD;
    QuotaNonPagedPoolUsage: DWORD;
    PagefileUsage: DWORD;
    PeakPagefileUsage: DWORD;
  end;
  {$EXTERNALSYM PROCESS_MEMORY_COUNTERS}
  PROCESS_MEMORY_COUNTERS = _PROCESS_MEMORY_COUNTERS;
  {$EXTERNALSYM PPROCESS_MEMORY_COUNTERS}
  PPROCESS_MEMORY_COUNTERS = ^_PROCESS_MEMORY_COUNTERS;
  TProcessMemoryCounters = _PROCESS_MEMORY_COUNTERS;
  PProcessMemoryCounters = ^_PROCESS_MEMORY_COUNTERS;

  TGetProcessMemoryInfo = function (Process: THandle;
    ppsmemCounters: PPROCESS_MEMORY_COUNTERS; cb: DWORD): BOOL stdcall;

{$EXTERNALSYM EnumProcesses}
function EnumProcesses(lpidProcess: LPDWORD; cb: DWORD; var cbNeeded: DWORD): BOOL;
{$EXTERNALSYM EnumProcessModules}
function EnumProcessModules(hProcess: THandle; lphModule: LPDWORD; cb: DWORD;
  var lpcbNeeded: DWORD): BOOL;
{$EXTERNALSYM GetModuleBaseName}
function GetModuleBaseName(hProcess: THandle; hModule: HMODULE;
  lpBaseName: PChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetModuleBaseNameA}
function GetModuleBaseNameA(hProcess: THandle; hModule: HMODULE;
  lpBaseName: PAnsiChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetModuleBaseNameW}
function GetModuleBaseNameW(hProcess: THandle; hModule: HMODULE;
  lpBaseName: PWideChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetModuleFileNameEx}
function GetModuleFileNameEx(hProcess: THandle; hModule: HMODULE;
  lpFilename: PChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetModuleFileNameExA}
function GetModuleFileNameExA(hProcess: THandle; hModule: HMODULE;
  lpFilename: PAnsiChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetModuleFileNameExW}
function GetModuleFileNameExW(hProcess: THandle; hModule: HMODULE;
  lpFilename: PWideChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetModuleInformation}
function GetModuleInformation(hProcess: THandle; hModule: HMODULE;
  lpmodinfo: LPMODULEINFO; cb: DWORD): BOOL;
{$EXTERNALSYM EmptyWorkingSet}
function EmptyWorkingSet(hProcess: THandle): BOOL;
{$EXTERNALSYM QueryWorkingSet}
function QueryWorkingSet(hProcess: THandle; pv: Pointer; cb: DWORD): BOOL;
{$EXTERNALSYM InitializeProcessForWsWatch}
function InitializeProcessForWsWatch(hProcess: THandle): BOOL;
{$EXTERNALSYM GetMappedFileName}
function GetMappedFileName(hProcess: THandle; lpv: Pointer;
  lpFilename: PChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetMappedFileNameA}
function GetMappedFileNameA(hProcess: THandle; lpv: Pointer;
  lpFilename: PAnsiChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetMappedFileNameW}
function GetMappedFileNameW(hProcess: THandle; lpv: Pointer;
  lpFilename: PWideChar; nSize: DWORD): DWORD;
{$EXTERNALSYM GetDeviceDriverBaseName}
function GetDeviceDriverBaseName(ImageBase: Pointer; lpBaseName: PChar;
  nSize: DWORD): DWORD;
{$EXTERNALSYM GetDeviceDriverBaseNameA}
function GetDeviceDriverBaseNameA(ImageBase: Pointer; lpBaseName: PAnsiChar;
  nSize: DWORD): DWORD;
{$EXTERNALSYM GetDeviceDriverBaseNameW}
function GetDeviceDriverBaseNameW(ImageBase: Pointer; lpBaseName: PWideChar;
  nSize: DWORD): DWORD;
{$EXTERNALSYM GetDeviceDriverFileName}
function GetDeviceDriverFileName(ImageBase: Pointer; lpFileName: PChar;
  nSize: DWORD): DWORD;
{$EXTERNALSYM GetDeviceDriverFileNameA}
function GetDeviceDriverFileNameA(ImageBase: Pointer; lpFileName: PAnsiChar;
  nSize: DWORD): DWORD;
{$EXTERNALSYM GetDeviceDriverFileNameW}
function GetDeviceDriverFileNameW(ImageBase: Pointer; lpFileName: PWideChar;
  nSize: DWORD): DWORD;
{$EXTERNALSYM EnumDeviceDrivers}
function EnumDeviceDrivers(lpImageBase: PPointer; cb: DWORD;
  var lpcbNeeded: DWORD): BOOL;
{$EXTERNALSYM GetProcessMemoryInfo}
function GetProcessMemoryInfo(Process: THandle;
  ppsmemCounters: PPROCESS_MEMORY_COUNTERS; cb: DWORD): BOOL;

implementation

var
  hPSAPI: THandle;
  _EnumProcesses: TEnumProcesses;
  _EnumProcessModules: TEnumProcessModules;
  {procedure}_GetModuleBaseName: TGetModuleBaseNameA;
  {procedure}_GetModuleFileNameEx: TGetModuleFileNameExA;
  {procedure}_GetModuleBaseNameA: TGetModuleBaseNameA;
  {procedure}_GetModuleFileNameExA: TGetModuleFileNameExA;
  {procedure}_GetModuleBaseNameW: TGetModuleBaseNameW;
  {procedure}_GetModuleFileNameExW: TGetModuleFileNameExW;
  _GetModuleInformation: TGetModuleInformation;
  _EmptyWorkingSet: TEmptyWorkingSet;
  _QueryWorkingSet: TQueryWorkingSet;
  _InitializeProcessForWsWatch: TInitializeProcessForWsWatch;
  {procedure}_GetMappedFileName: TGetMappedFileNameA;
  {procedure}_GetDeviceDriverBaseName: TGetDeviceDriverBaseNameA;
  {procedure}_GetDeviceDriverFileName: TGetDeviceDriverFileNameA;
  {procedure}_GetMappedFileNameA: TGetMappedFileNameA;
  {procedure}_GetDeviceDriverBaseNameA: TGetDeviceDriverBaseNameA;
  {procedure}_GetDeviceDriverFileNameA: TGetDeviceDriverFileNameA;
  {procedure}_GetMappedFileNameW: TGetMappedFileNameW;
  {procedure}_GetDeviceDriverBaseNameW: TGetDeviceDriverBaseNameW;
  {procedure}_GetDeviceDriverFileNameW: TGetDeviceDriverFileNameW;
  _EnumDeviceDrivers: TEnumDeviceDrivers;
  _GetProcessMemoryInfo: TGetProcessMemoryInfo;

function CheckPSAPILoaded: Boolean;
begin
  if hPSAPI = 0 then
  begin
{$IFDEF MSWINDOWS}
    hPSAPI := LoadLibrary('PSAPI.dll');
    if hPSAPI < 32 then
    begin
      hPSAPI := 0;
      Result := False;
      Exit;
    end;
{$ENDIF}
    @_EnumProcesses := GetProcAddress(hPSAPI, 'EnumProcesses');
    @_EnumProcessModules := GetProcAddress(hPSAPI, 'EnumProcessModules');
    {procedure}@_GetModuleBaseName := GetProcAddress(hPSAPI, 'GetModuleBaseNameA');
    {procedure}@_GetModuleFileNameEx := GetProcAddress(hPSAPI, 'GetModuleFileNameExA');
    {procedure}@_GetModuleBaseNameA := GetProcAddress(hPSAPI, 'GetModuleBaseNameA');
    {procedure}@_GetModuleFileNameExA := GetProcAddress(hPSAPI, 'GetModuleFileNameExA');
    {procedure}@_GetModuleBaseNameW := GetProcAddress(hPSAPI, 'GetModuleBaseNameW');
    {procedure}@_GetModuleFileNameExW := GetProcAddress(hPSAPI, 'GetModuleFileNameExW');
    @_GetModuleInformation := GetProcAddress(hPSAPI, 'GetModuleInformation');
    @_EmptyWorkingSet := GetProcAddress(hPSAPI, 'EmptyWorkingSet');
    @_QueryWorkingSet := GetProcAddress(hPSAPI, 'QueryWorkingSet');
    @_InitializeProcessForWsWatch := GetProcAddress(hPSAPI, 'InitializeProcessForWsWatch');
    {procedure}@_GetMappedFileName := GetProcAddress(hPSAPI, 'GetMappedFileNameA');
    {procedure}@_GetDeviceDriverBaseName := GetProcAddress(hPSAPI, 'GetDeviceDriverBaseNameA');
    {procedure}@_GetDeviceDriverFileName := GetProcAddress(hPSAPI, 'GetDeviceDriverFileNameA');
    {procedure}@_GetMappedFileNameA := GetProcAddress(hPSAPI, 'GetMappedFileNameA');
    {procedure}@_GetDeviceDriverBaseNameA := GetProcAddress(hPSAPI, 'GetDeviceDriverBaseNameA');
    {procedure}@_GetDeviceDriverFileNameA := GetProcAddress(hPSAPI, 'GetDeviceDriverFileNameA');
    {procedure}@_GetMappedFileNameW := GetProcAddress(hPSAPI, 'GetMappedFileNameW');
    {procedure}@_GetDeviceDriverBaseNameW := GetProcAddress(hPSAPI, 'GetDeviceDriverBaseNameW');
    {procedure}@_GetDeviceDriverFileNameW := GetProcAddress(hPSAPI, 'GetDeviceDriverFileNameW');
    @_EnumDeviceDrivers := GetProcAddress(hPSAPI, 'EnumDeviceDrivers');
    @_GetProcessMemoryInfo := GetProcAddress(hPSAPI, 'GetProcessMemoryInfo');
  end;
  Result := True;
end;

function EnumProcesses(lpidProcess: LPDWORD; cb: DWORD; var cbNeeded: DWORD): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _EnumProcesses(lpidProcess, cb, cbNeeded)
  else Result := False;
end;

function EnumProcessModules(hProcess: THandle; lphModule: LPDWORD; cb: DWORD;
  var lpcbNeeded: DWORD): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _EnumProcessModules(hProcess, lphModule, cb, lpcbNeeded)
  else Result := False;
end;

function GetModuleBaseName(hProcess: THandle; hModule: HMODULE;
  lpBaseName: PChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleBaseName(hProcess, hModule, lpBaseName, nSize)
  else Result := 0;
end;

function GetModuleBaseNameA(hProcess: THandle; hModule: HMODULE;
  lpBaseName: PAnsiChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleBaseNameA(hProcess, hModule, lpBaseName, nSize)
  else Result := 0;
end;

function GetModuleBaseNameW(hProcess: THandle; hModule: HMODULE;
  lpBaseName: PWideChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleBaseNameW(hProcess, hModule, lpBaseName, nSize)
  else Result := 0;
end;

function GetModuleFileNameEx(hProcess: THandle; hModule: HMODULE;
  lpFilename: PChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleFileNameEx(hProcess, hModule, lpFileName, nSize)
  else Result := 0;
end;

function GetModuleFileNameExA(hProcess: THandle; hModule: HMODULE;
  lpFilename: PAnsiChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleFileNameExA(hProcess, hModule, lpFileName, nSize)
  else Result := 0;
end;

function GetModuleFileNameExW(hProcess: THandle; hModule: HMODULE;
  lpFilename: PWideChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleFileNameExW(hProcess, hModule, lpFileName, nSize)
  else Result := 0;
end;

function GetModuleInformation(hProcess: THandle; hModule: HMODULE;
  lpmodinfo: LPMODULEINFO; cb: DWORD): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _GetModuleInformation(hProcess, hModule, lpmodinfo, cb)
  else Result := False;
end;

function EmptyWorkingSet(hProcess: THandle): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _EmptyWorkingSet(hProcess)
  else Result := False;
end;

function QueryWorkingSet(hProcess: THandle; pv: Pointer; cb: DWORD): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _QueryWorkingSet(hProcess, pv, cb)
  else Result := False;
end;

function InitializeProcessForWsWatch(hProcess: THandle): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _InitializeProcessForWsWatch(hProcess)
  else Result := False;
end;

function GetMappedFileName(hProcess: THandle; lpv: Pointer;
  lpFilename: PChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetMappedFileName(hProcess, lpv, lpFileName, nSize)
  else Result := 0;
end;

function GetMappedFileNameA(hProcess: THandle; lpv: Pointer;
  lpFilename: PAnsiChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetMappedFileNameA(hProcess, lpv, lpFileName, nSize)
  else Result := 0;
end;

function GetMappedFileNameW(hProcess: THandle; lpv: Pointer;
  lpFilename: PWideChar; nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetMappedFileNameW(hProcess, lpv, lpFileName, nSize)
  else Result := 0;
end;

function GetDeviceDriverBaseName(ImageBase: Pointer; lpBaseName: PChar;
  nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetDeviceDriverBasename(ImageBase, lpBaseName, nSize)
  else Result := 0;
end;

function GetDeviceDriverBaseNameA(ImageBase: Pointer; lpBaseName: PAnsiChar;
  nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetDeviceDriverBasenameA(ImageBase, lpBaseName, nSize)
  else Result := 0;
end;

function GetDeviceDriverBaseNameW(ImageBase: Pointer; lpBaseName: PWideChar;
  nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetDeviceDriverBasenameW(ImageBase, lpBaseName, nSize)
  else Result := 0;
end;

function GetDeviceDriverFileName(ImageBase: Pointer; lpFileName: PChar;
  nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetDeviceDriverFileName(ImageBase, lpFileName, nSize)
  else Result := 0;
end;

function GetDeviceDriverFileNameA(ImageBase: Pointer; lpFileName: PAnsiChar;
  nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetDeviceDriverFileNameA(ImageBase, lpFileName, nSize)
  else Result := 0;
end;

function GetDeviceDriverFileNameW(ImageBase: Pointer; lpFileName: PWideChar;
  nSize: DWORD): DWORD;
begin
  if CheckPSAPILoaded then
    Result := _GetDeviceDriverFileNameW(ImageBase, lpFileName, nSize)
  else Result := 0;
end;

function EnumDeviceDrivers(lpImageBase: PPointer; cb: DWORD;
  var lpcbNeeded: DWORD): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _EnumDeviceDrivers(lpImageBase, cb, lpcbNeeded)
  else Result := False;
end;

function GetProcessMemoryInfo(Process: THandle;
  ppsmemCounters: PPROCESS_MEMORY_COUNTERS; cb: DWORD): BOOL;
begin
  if CheckPSAPILoaded then
    Result := _GetProcessMemoryInfo(Process, ppsmemCounters, cb)
  else Result := False;
end;

end.
