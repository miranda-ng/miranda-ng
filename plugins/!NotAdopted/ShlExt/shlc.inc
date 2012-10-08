{$IFDEF SHL_IDC}

const
  IDD_SHLOPTS = 101;
  IDC_USEGROUPS = 1014;
  IDC_CLISTGROUPS = 1015;
  // Show "HIT"
  IDC_SHOWFULL = 1016;
  IDC_NOPROF = 1020;
  IDC_SHOWINVISIBLES = 1021;
  IDC_HIDEOFFLINE = 1022;
  // only in the options dialog
  IDC_STATUS = 1023;
  IDC_CAPMENUS = 1025;
  IDC_CAPSTATUS = 1026;
  IDC_CAPSHLSTATUS = 1027;
  IDC_REMOVE = 1028;
  IDC_USEOWNERDRAW = 1029;
{$ENDIF}
{$IFDEF SHL_KEYS}

const
  SHLExt_Name: PChar = 'shlext15';
  SHLExt_MRU: PChar = 'MRU';
  SHLExt_UseGroups: PChar = 'UseGroups';
  SHLExt_UseCListSetting: PChar = 'UseCLGroups';
  SHLExt_UseHITContacts: PChar = 'UseHITContacts';
  // HIT2 contacts will get your messages but don't know your state
  SHLExt_UseHIT2Contacts: PChar = 'UseHIT2Contacts';
  SHLExt_ShowNoProfile: PChar = 'ShowNoProfile';
  SHLExt_ShowNoIcons: PChar = 'ShowNoIcons';
  SHLExt_ShowNoOffline: PChar = 'ShowNoOffline';
{$ENDIF}
{$IFDEF SHLCOM}

const

  S_OK = 0;
  S_FALSE = 1;

  E_UNEXPECTED = $8000FFFF;
  E_NOTIMPL = $80004001;
  E_INVALIDARG = $80070057;

  CLASS_E_NOAGGREGATION = $80040110;
  CLASS_E_CLASSNOTAVAILABLE = $80040111;

  CLSCTX_INPROC_SERVER = $1;

  { for FORMATETC }

  TYMED_HGLOBAL = 1;
  DVASPECT_CONTENT = 1;

type

  PGUID = ^TGUID;

  TGUID = record
    D1: Longword;
    D2: Word;
    D3: Word;
    D4: array [0 .. 7] of Byte;
  end;

  TIID = TGUID;
  TCLSID = TGUID;

  TStgMedium = record
    tymed: Longint;
    case Integer of
      0: (hBitmap: hBitmap; unkForRelease: Pointer { IUnknown } );
      1: (hMetaFilePict: THandle);
      2: (hEnhMetaFile: THandle);
      3: (hGlobal: hGlobal);
      4: (lpszFileName: Pointer { POleStr } );
      5: (stm: Pointer { IUnknown } );
      6: (stg: Pointer { IStorage } );
  end;

  PFormatEtc = ^TFormatEtc;

  TFormatEtc = record
    cfFormat: Word; { TClipFormat; }
    ptd: Pointer; { PDVTargetDevice; }
    dwAspect: Longint;
    lindex: Longint;
    tymed: Longint;
  end;

{$ENDIF}
{$IFDEF COM_STRUCTS}

const

  IID_IUnknown: TGUID = (D1: $00000000; D2: $0000; D3: $0000;
    D4: ($C0, $00, $00, $00, $00, $00, $00, $46));

  IID_IClassFactory: TGUID = (D1: $00000001; D2: $0000; D3: $0000;
    D4: ($C0, $00, $00, $00, $00, $00, $00, $46));

  IID_IShellExtInit: TGUID = (D1: $000214E8; D2: $0000; D3: $0000;
    D4: ($C0, $00, $00, $00, $00, $00, $00, $46));

  IID_IContextMenu: TGUID = (D1: $000214E4; D2: $0000; D3: $0000;
    D4: ($C0, $00, $00, $00, $00, $00, $00, $46));

  IID_IContextMenu2: TGUID = (D1: $000214F4; D2: $0000; D3: $0000;
    D4: ($C0, $00, $00, $00, $00, $00, $00, $46));

  IID_IContextMenu3: TGUID = (D1: $BCFCE0A0; D2: $EC17; D3: $11D0;
    D4: ($8D, $10, $00, $A0, $C9, $0F, $27, $19));

  IID_WICImagingFactory: TGUID = (D1: $EC5EC8A9; D2: $C395; D3: $4314;
    D4: ($9C, $77, $54, $D7, $A9, $35, $FF, $70));


  // Vista+ only

  CLSID_WICImagingFactory: TGUID = (D1: $CACAF262; D2: $9370; D3: $4615;
    D4: ($A1, $3B, $9F, $55, $39, $DA, $4C, $0A));

  // anything before 0.0.1.5 was : {A321A032-7976-11d6-A310-ED893982BF28}
  // changed to a new GUID to avoid older plugins
  // {72013A26-A94C-11d6-8540-A5E62932711D}
  // the IPC header now checks the plugin version given anyway.

  CLSID_ISHLCOM: TGUID = (D1: $72013A26; D2: $A94C; D3: $11D6;
    D4: ($85, $40, $A5, $E6, $29, $32, $71, $1D););
{$ENDIF}
{$IFDEF COMAPI}
function CoCreateInstance(const rclsid: TCLSID; pUnkOuter: Pointer; dwClsContext: DWORD;
  const riid: TIID; var ppv): HResult; stdcall; external 'ole32.dll' name 'CoCreateInstance';
procedure ReleaseStgMedium(var medium: TStgMedium); stdcall;
  external 'ole32.dll' name 'ReleaseStgMedium';
function IsEqualGUID(const guid1, guid2: TGUID): Boolean; stdcall;
  external 'ole32.dll' name 'IsEqualGUID';
function IsEqualIID(const iid1, iid2: TIID): Boolean; stdcall;
  external 'ole32.dll' name 'IsEqualGUID';
function IsEqualCLSID(const clsid1, clsid2: TCLSID): Boolean; stdcall;
  external 'ole32.dll' name 'IsEqualGUID';
function QueueUserAPC(pfnAPC: Pointer; hThread: THandle; dwData: DWORD): BOOL; stdcall;
  external 'kernel32' name 'QueueUserAPC';
{$ENDIF}
