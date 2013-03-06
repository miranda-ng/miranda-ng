{
    This file is part of the Free Pascal run time library.
    Copyright (c) 2006 by Florian Klaempfl
    member of the Free Pascal development team.

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}
{$mode objfpc}
{$H+}
{$inline on}
unit mcomobj;

  interface

{ $define DEBUG_COM}

  uses
    Windows,Sysutils,classes;

  const
    CLSCTX_INPROC_SERVER        = $0001;     // server dll (runs in same process as caller)
    CLSCTX_INPROC_HANDLER       = $0002;     // handler dll (runs in same process as caller)
    CLSCTX_LOCAL_SERVER         = $0004;     // server exe (runs on same machine; diff proc)
    CLSCTX_INPROC_SERVER16      = $0008;     // 16-bit server dll (runs in same process as caller)
    CLSCTX_REMOTE_SERVER        = $0010;     // remote server exe (runs on different machine)
    CLSCTX_INPROC_HANDLER16     = $0020;     // 16-bit handler dll (runs in same process as caller)
    CLSCTX_INPROC_SERVERX86     = $0040;     // Wx86 server dll (runs in same process as caller)
    CLSCTX_INPROC_HANDLERX86    = $0080;     // Wx86 handler dll (runs in same process as caller)
    CLSCTX_ESERVER_HANDLER      = $0100;     // handler dll (runs in the server process)
    CLSCTX_RESERVED =$0200;                  // reserved
    CLSCTX_NO_CODE_DOWNLOAD     = $0400;     // disallow code download from the Directory Service (if any) or the internet   -rahulth
    CLSCTX_NO_WX86_TRANSLATION  = $0800;
    CLSCTX_NO_CUSTOM_MARSHAL    = $1000;
    CLSCTX_ENABLE_CODE_DOWNLOAD = $2000;     // allow code download from the Directory Service (if any) or the internet
    CLSCTX_NO_FAILURE_LOG       = $04000;    // do not log messages about activation failure (should one occur) to Event Log
    CLSCTX_DISABLE_AAA          = $08000;    // Disable EOAC_DISABLE_AAA capability for this activation only
    CLSCTX_ENABLE_AAA           = $10000;    // Enable EOAC_DISABLE_AAA capability for this activation only
    CLSCTX_FROM_DEFAULT_CONTEXT = $20000;    // Begin this activation from the default context of the current apartment
    CLSCTX_INPROC               = (CLSCTX_INPROC_SERVER OR CLSCTX_INPROC_HANDLER);
    CLSCTX_ALL                  = (CLSCTX_INPROC_SERVER OR CLSCTX_INPROC_HANDLER OR CLSCTX_LOCAL_SERVER {$ifdef Remote} OR CLSCTX_REMOTE_SERVER {$endif});
    CLSCTX_SERVER               = (CLSCTX_INPROC_SERVER OR CLSCTX_LOCAL_SERVER {$ifdef Remote} OR CLSCTX_REMOTE_SERVER {$endif});

    MEMCTX_TASK                 = 1;          // task (private) memory
    MEMCTX_SHARED               = 2;          // shared memory (between processes)
    MEMCTX_MACSYSTEM            = 3;          // on the mac, the system heap
    // these are mostly for internal use...
    MEMCTX_UNKNOWN              = -1;         // unknown context (when asked about it)
    MEMCTX_SAME                 = -2;         // same context (as some other pointer)

  type
    TOleChar = WideChar;
    POleStr = PWideChar;
    PPOleStr = ^POleStr;

    OleChar = WChar;
    LPOLESTR = ^OLECHAR;
    POLECHAR = LPOLESTR;
    PLPOLESTR = ^LPOLESTR;
    TClassInstancing = (ciInternal, ciSingleInstance, ciMultiInstance);
    TThreadingModel = (tmSingle, tmApartment, tmFree, tmBoth, tmNeutral);

    type
      EOleError = class(Exception);

      EOleSysError = class(EOleError)
      private
        FErrorCode: HRESULT;
      public
        constructor Create(const Msg: string; aErrorCode: HRESULT;aHelpContext: Integer);
        property ErrorCode: HRESULT read FErrorCode write FErrorCode;
      end;


  function CreateComObject(const ClassID: TGUID) : IUnknown;
  function CreateOleObject(const ClassName : string) : IDispatch;
  function GetActiveOleObject(const ClassName: string) : IDispatch;

  procedure OleCheck(Value : HResult);inline;
  procedure OleError(Code: HResult);

  function ProgIDToClassID(const id : string) : TGUID;
  function ClassIDToProgID(const classID: TGUID): string;

  type
    TCoInitializeExProc = function (pvReserved: Pointer;
    coInit: DWORD): HResult; stdcall;
    TCoAddRefServerProcessProc = function : ULONG; stdcall;
    TCoReleaseServerProcessProc = function : ULONG; stdcall;
    TCoResumeClassObjectsProc = function : HResult; stdcall;
    TCoSuspendClassObjectsProc = function : HResult; stdcall;

    _COSERVERINFO = Record
                        dwReserved1 : DWord;
                        pwszName    : LPWSTR;
                        pAuthInfo   : Pointer;
                        dwReserved2 : DWord;
                    end;
    TCOSERVERINFO = _COSERVERINFO;
    PCOSERVERINFO = ^TCOSERVERINFO;

    IMalloc   = Interface(IUnknown)
      ['{00000002-0000-0000-C000-000000000046}']
      Function  Alloc(cb :size_t):Pointer; Stdcall;
      Function  Realloc (pv :pointer;cb:size_t):Pointer;stdcall;
      Procedure Free({[in]} pv: pointer); Stdcall;
      Function  GetSize(pv:pointer):size_t;stdcall;
      Function  DidAlloc(pv:pointer):Longint;stdcall;
      procedure HeapMinimize; stdcall;
    End;

  const
    CoInitializeEx : TCoInitializeExProc = nil;
    CoAddRefServerProcess : TCoAddRefServerProcessProc = nil;
    CoReleaseServerProcess : TCoReleaseServerProcessProc = nil;
    CoResumeClassObjects : TCoResumeClassObjectsProc = nil;
    CoSuspendClassObjects : TCoSuspendClassObjectsProc = nil;
    CoInitFlags : Longint = -1;

  function CLSIDFromProgID(_para1:POLESTR; _para2:LPCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CLSIDFromProgID';
  function CLSIDFromProgID(_para1:POLESTR; out _para2:TCLSID):HRESULT;stdcall; external  'ole32.dll' name 'CLSIDFromProgID';
  function CoInitialize(_para1:PVOID):HRESULT;stdcall; external  'ole32.dll' name 'CoInitialize';
  function CoCreateInstance(const _para1:TCLSID; _para2:IUnknown; _para3:DWORD;const _para4:TIID;out _para5):HRESULT;stdcall; external 'ole32.dll' name 'CoCreateInstance';
  function GetActiveObject(const clsid: TCLSID; pvReserved: Pointer; out unk: IUnknown) : HResult; stdcall; external 'oleaut32.dll' name 'GetActiveObject';
  function ProgIDFromCLSID(para:PCLSID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'ProgIDFromCLSID';
  function ProgIDFromCLSID(const _para1:TCLSID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'ProgIDFromCLSID';
  procedure CoTaskMemFree(_para1:PVOID);stdcall; external  'ole32.dll' name 'CoTaskMemFree';
  function CoGetMalloc(_para1:DWORD; out _para2:IMalloc):HRESULT;stdcall; external  'ole32.dll' name 'CoGetMalloc';
  procedure CoUninitialize;stdcall; external  'ole32.dll' name 'CoUninitialize';
  function CoCreateGuid(out _para1:TGUID):HRESULT;stdcall;external 'ole32.dll' name 'CoCreateGuid';
  function StringFromCLSID(const _para1:TCLSID; out _para2:POLESTR):HRESULT;stdcall; external  'ole32.dll' name 'StringFromCLSID';

implementation

    uses
      ComConst, Ole2, RtlConsts;

    constructor EOleSysError.Create(const Msg: string; aErrorCode: HRESULT; aHelpContext: Integer);
      var
        m : string;
      begin
        if Msg='' then
          m:=SysErrorMessage(aErrorCode)
        else
          m:=Msg;
        inherited CreateHelp(m,HelpContext);
        FErrorCode:=aErrorCode;
      end;

    {$define FPC_COMOBJ_HAS_CREATE_CLASS_ID}
    function CreateClassID : ansistring;
      var
         ClassID : TCLSID;
         p : PWideChar;
      begin
         CoCreateGuid(ClassID);
         StringFromCLSID(ClassID,p);
         result:=p;
         CoTaskMemFree(p);
      end;

   function CreateComObject(const ClassID : TGUID) : IUnknown;
     begin
       OleCheck(CoCreateInstance(ClassID,nil,CLSCTX_INPROC_SERVER or CLSCTX_LOCAL_SERVER,IUnknown,result));
     end;

   function CreateOleObject(const ClassName : string) : IDispatch;
     var
       id : TCLSID;
     begin
        id:=ProgIDToClassID(ClassName);
        OleCheck(CoCreateInstance(id,nil,CLSCTX_INPROC_SERVER or CLSCTX_LOCAL_SERVER,IDispatch,result));
     end;

   function GetActiveOleObject(const ClassName : string) : IDispatch;
{$ifndef wince}
     var
     	 intf : IUnknown;
       id : TCLSID;
     begin
       id:=ProgIDToClassID(ClassName);
       OleCheck(GetActiveObject(id,nil,intf));
       OleCheck(intf.QueryInterface(IDispatch,Result));
     end;
{$else}
     begin
       Result:=nil;
     end;
{$endif wince}

   procedure OleError(Code: HResult);
     begin
       raise EOleSysError.Create('',Code,0);
     end;

   procedure OleCheck(Value : HResult);inline;
     begin
       if not(Succeeded(Value)) then
         OleError(Value);
     end;

   function ProgIDToClassID(const id : string) : TGUID;
     begin
       OleCheck(CLSIDFromProgID(PWideChar(WideString(id)),result));
     end;

   function ClassIDToProgID(const classID: TGUID): string;
     var
       progid : LPOLESTR;
     begin
       OleCheck(ProgIDFromCLSID(@classID,progid));
       result:=progid;
       CoTaskMemFree(progid);
     end;

const
  Initialized : boolean = false;
var
  Ole32Dll : HModule;

initialization
  Ole32Dll:=GetModuleHandle('ole32.dll');
  if Ole32Dll<>0 then
    begin
      Pointer(CoInitializeEx):=GetProcAddress(Ole32Dll,'CoInitializeEx');
      Pointer(CoAddRefServerProcess):=GetProcAddress(Ole32Dll,'CoAddRefServerProcess');
      Pointer(CoReleaseServerProcess):=GetProcAddress(Ole32Dll,'CoReleaseServerProcess');
      Pointer(CoResumeClassObjects):=GetProcAddress(Ole32Dll,'CoResumeClassObjects');
      Pointer(CoSuspendClassObjects):=GetProcAddress(Ole32Dll,'CoSuspendClassObjects');
    end;

  if not(IsLibrary) then
{$ifndef wince}
    if (CoInitFlags=-1) or not(assigned(mcomobj.CoInitializeEx)) then
      Initialized:=Succeeded(CoInitialize(nil))
    else
{$endif wince}
      Initialized:=Succeeded(mcomobj.CoInitializeEx(nil, CoInitFlags));

finalization
  if Initialized then
    CoUninitialize;
end.
