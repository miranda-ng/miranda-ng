unit shlcom;

{$IFDEF FPC}
{$PACKRECORDS 4}
{$MODE Delphi}
{$ENDIF}

interface

uses

  Windows, m_api, shlipc, shlicons;

{$DEFINE COM_STRUCTS}
{$DEFINE SHLCOM}
{$INCLUDE shlc.inc}
{$UNDEF SHLCOM}
{$UNDEF COM_STRUCTS}
function DllGetClassObject(const CLSID: TCLSID; const IID: TIID; var Obj): HResult; stdcall;
function DllCanUnloadNow: HResult; stdcall;

procedure InvokeThreadServer;

procedure CheckRegisterServer;

procedure CheckUnregisterServer;

function RemoveCOMRegistryEntries: HResult;

function ExtractIcon(hInst: THandle; pszExe: PChar; nIndex: Integer): HICON; stdcall;
  external 'shell32.dll' name 'ExtractIconA';

implementation

var
  dllpublic: record
    FactoryCount: Integer;
    ObjectCount: Integer;
  end;

  VistaOrLater:Boolean;

{$DEFINE COMAPI}
{$INCLUDE shlc.inc}
{$UNDEF COMAPI}

const

  IPC_PACKET_SIZE = $1000 * 32;
  // IPC_PACKET_NAME = 'm.mi.miranda.ipc'; // prior to 1.0.6.6
  // IPC_PACKET_NAME = 'mi.miranda.IPCServer'; // prior to 2.0.0.9
  IPC_PACKET_NAME = 'm.mi.miranda.ipc.server';

const

  { Flags returned by IContextMenu*:QueryContextMenu() }

  CMF_NORMAL = $00000000;
  CMF_DEFAULTONLY = $00000001;
  CMF_VERBSONLY = $00000002;
  CMF_EXPLORE = $00000004;
  CMF_NOVERBS = $00000008;
  CMF_CANRENAME = $00000010;
  CMF_NODEFAULT = $00000020;
  CMF_INCLUDESTATIC = $00000040;
  CMF_RESERVED = $FFFF0000; { view specific }

  { IContextMenu*:GetCommandString() uType flags }

  GCS_VERBA = $00000000; // canonical verb
  GCS_HELPTEXTA = $00000001; // help text (for status bar)
  GCS_VALIDATEA = $00000002; // validate command exists
  GCS_VERBW = $00000004; // canonical verb (unicode)
  GC_HELPTEXTW = $00000005; // help text (unicode version)
  GCS_VALIDATEW = $00000006; // validate command exists (unicode)
  GCS_UNICODE = $00000004; // for bit testing - Unicode string
  GCS_VERB = GCS_VERBA; //
  GCS_HELPTEXT = GCS_HELPTEXTA;
  GCS_VALIDATE = GCS_VALIDATEA;

type

  { this structure is returned by InvokeCommand() }

  PCMInvokeCommandInfo = ^TCMInvokeCommandInfo;

  TCMInvokeCommandInfo = packed record
    cbSize: DWORD;
    fMask: DWORD;
    hwnd: hwnd;
    lpVerb: PChar; { maybe index, type cast as Integer }
    lpParams: PChar;
    lpDir: PChar;
    nShow: Integer;
    dwHotkey: DWORD;
    HICON: THandle;
  end;

  { completely stolen from modules.c: 'NameHashFunction' modified slightly }

function StrHash(const szStr: PChar): DWORD;// cdecl;
begin
  result:=mir_hash(szStr,strlen(szStr));
{
asm
  // esi content has to be preserved with basm
  push esi
  xor  edx,edx
  xor  eax,eax
  mov  esi,szStr
  mov  al,[esi]
  xor  cl,cl
@@lph_top:	    // only 4 of 9 instructions in here don't use AL, so optimal pipe use is impossible
  xor  edx,eax
  inc  esi
  xor  eax,eax
  and  cl,31
  mov  al,[esi]
  add  cl,5
  test al,al
  rol  eax,cl		    // rol is u-pipe only, but pairable
  // rol doesn't touch z-flag
  jnz  @@lph_top      // 5 clock tick loop. not bad.
  xor  eax,edx
  pop  esi
}
end;

function CreateProcessUID(const pid: Cardinal): string;
var
  pidrep: string[16];
begin
  str(pid, pidrep);
  Result := Concat('mim.shlext.', pidrep, '$');
end;

function CreateUID: string;
var
  pidrep, tidrep: string[16];
begin
  str(GetCurrentProcessId(), pidrep);
  str(GetCurrentThreadId(), tidrep);
  Result := Concat('mim.shlext.caller', pidrep, '$', tidrep);
end;

// FPC doesn't support array[0..n] of Char extended syntax with Str()

function wsprintf(lpOut, lpFmt: PChar; ArgInt: Integer): Integer; cdecl;
  external 'user32.dll' name 'wsprintfA';

procedure str(i: Integer; S: PChar);
begin
  i := wsprintf(S, '%d', i);
  if i > 2 then
    PChar(S)[i] := #0;
end;

{ IShlCom }

type

  PLResult = ^LResult;

  // bare minimum interface of IDataObject, since GetData() is only required.

  PVTable_IDataObject = ^TVTable_IDataObject;

  TVTable_IDataObject = record
    { IUnknown }
    QueryInterface: Pointer;
    AddRef: function(Self: Pointer): Cardinal; stdcall;
    Release: function(Self: Pointer): Cardinal; stdcall;
    { IDataObject }
    GetData: function(Self:Pointer; var formatetcIn:TFormatEtc; var medium:TStgMedium): HResult; stdcall;
    GetDataHere: Pointer;
    QueryGetData: Pointer;
    GetCanonicalFormatEtc: Pointer;
    SetData: Pointer;
    EnumFormatEtc: Pointer;
    DAdvise: Pointer;
    DUnadvise: Pointer;
    EnumDAdvise: Pointer;
  end;

  PDataObject_Interface = ^TDataObject_Interface;

  TDataObject_Interface = record
    ptrVTable: PVTable_IDataObject;
  end;

  { TShlComRec inherits from different interfaces with different function tables
    all "compiler magic" is lost in this case, but it's pretty easy to return
    a different function table for each interface, IContextMenu is returned
    as IContextMenu'3' since it inherits from '2' and '1' }

  PVTable_IShellExtInit = ^TVTable_IShellExtInit;

  TVTable_IShellExtInit = record
    { IUnknown }
    QueryInterface: Pointer;
    AddRef: Pointer;
    Release: Pointer;
    { IShellExtInit }
    Initialise: Pointer;
  end;

  PShlComRec = ^TShlComRec;
  PShellExtInit_Interface = ^TShellExtInit_Interface;

  TShellExtInit_Interface = record
    { pointer to function table }
    ptrVTable: PVTable_IShellExtInit;
    { instance data }
    ptrInstance: PShlComRec;
    { function table itself }
    vTable: TVTable_IShellExtInit;
  end;

  PVTable_IContextMenu3 = ^TVTable_IContextMenu3;

  TVTable_IContextMenu3 = record
    { IUnknown }
    QueryInterface: Pointer;
    AddRef: Pointer;
    Release: Pointer;
    { IContextMenu }
    QueryContextMenu: Pointer;
    InvokeCommand: Pointer;
    GetCommandString: Pointer;
    { IContextMenu2 }
    HandleMenuMsg: Pointer;
    { IContextMenu3 }
    HandleMenuMsg2: Pointer;
  end;

  PContextMenu3_Interface = ^TContextMenu3_Interface;

  TContextMenu3_Interface = record
    ptrVTable: PVTable_IContextMenu3;
    ptrInstance: PShlComRec;
    vTable: TVTable_IContextMenu3;
  end;

  PCommon_Interface = ^TCommon_Interface;

  TCommon_Interface = record
    ptrVTable: Pointer;
    ptrInstance: PShlComRec;
  end;

  TShlComRec = record
    ShellExtInit_Interface: TShellExtInit_Interface;
    ContextMenu3_Interface: TContextMenu3_Interface;
    { fields }
    RefCount: LongInt;
    // this is owned by the shell after items are added 'n' is used to
    // grab menu information directly via id rather than array indexin'
    hRootMenu: THandle;
    idCmdFirst: Integer;
    // most of the memory allocated is on this heap object so HeapDestroy()
    // can do most of the cleanup, extremely lazy I know.
    hDllHeap: THandle;
    // This is a submenu that recently used contacts are inserted into
    // the contact is inserted twice, once in its normal list (or group) and here
    // Note: These variables are global data, but refered to locally by each instance
    // Do not rely on these variables outside the process enumeration.
    hRecentMenu: THandle;
    RecentCount: Cardinal; // number of added items
    // array of all the protocol icons, for every running instance!
    ProtoIcons: ^TSlotProtoIconsArray;
    ProtoIconsCount: Cardinal;
    // maybe null, taken from IShellExtInit_Initalise() and AddRef()'d
    // only used if a Miranda instance is actually running and a user
    // is selected
    pDataObject: PDataObject_Interface;
    // DC is used for font metrics and saves on creating and destroying lots of DC handles
    // during WM_MEASUREITEM
    hMemDC: HDC;
  end;

  { this is passed to the enumeration callback so it can process PID's with
    main windows by the class name MIRANDANAME loaded with the plugin
    and use the IPC stuff between enumerations -- }

  PEnumData = ^TEnumData;

  TEnumData = record
    Self: PShlComRec;
    // autodetected, don't hard code since shells that don't support it
    // won't send WM_MEASUREITETM/WM_DRAWITEM at all.
    bOwnerDrawSupported: LongBool;
    // as per user setting (maybe of multiple Mirandas)
    bShouldOwnerDraw: LongBool;
    idCmdFirst: Integer;
    ipch: PHeaderIPC;
    // OpenEvent()'d handle to give each IPC server an object to set signalled
    hWaitFor: THandle;
    pid: DWORD; // sub-unique value used to make work object name
  end;

procedure FreeGroupTreeAndEmptyGroups(hParentMenu: THandle; pp, p: PGroupNode);
var
  q: PGroupNode;
begin
  while p <> nil do
  begin
    q := p^.Right;
    if p^.Left <> nil then
    begin
      FreeGroupTreeAndEmptyGroups(p^.Left^.hMenu, p, p^.Left);
    end; // if
    if p^.dwItems = 0 then
    begin
      if pp <> nil then
      begin
        DeleteMenu(pp^.hMenu, p^.hMenuGroupID, MF_BYCOMMAND)
      end
      else
      begin
        DeleteMenu(hParentMenu, p^.hMenuGroupID, MF_BYCOMMAND);
      end; // if
    end
    else
    begin
      // make sure this node's parent know's it exists
      if pp <> nil then
        inc(pp^.dwItems);
    end;
    Dispose(p);
    p := q;
  end;
end;

procedure DecideMenuItemInfo(pct: PSlotIPC; pg: PGroupNode; var mii: TMenuItemInfo; lParam: PEnumData);
var
  psd: PMenuDrawInfo;
  hDllHeap: THandle;
  c: Cardinal;
  pp: ^TSlotProtoIconsArray;
begin
  mii.wID := lParam^.idCmdFirst;
  inc(lParam^.idCmdFirst);
  // get the heap object
  hDllHeap := lParam^.Self^.hDllHeap;
  psd := HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
  if pct <> nil then
  begin
    psd^.cch := pct^.cbStrSection - 1; // no null;
    psd^.szText := HeapAlloc(hDllHeap, 0, pct^.cbStrSection);
    lstrcpya(psd^.szText, PChar(uint_ptr(pct) + sizeof(TSlotIPC)));
    psd^.hContact := pct^.hContact;
    psd^.fTypes := [dtContact];
    // find the protocol icon array to use and which status
    c := lParam^.Self^.ProtoIconsCount;
    pp := lParam^.Self^.ProtoIcons;
    psd^.hStatusIcon := 0;
    while c > 0 do
    begin
      dec(c);
      if (pp[c].hProto = pct^.hProto) and (pp[c].pid = lParam^.pid) then
      begin
        psd^.hStatusIcon := pp[c].hIcons[pct^.Status - ID_STATUS_OFFLINE];
        psd^.hStatusBitmap := pp[c].hBitmaps[pct^.Status - ID_STATUS_OFFLINE];
        break;
      end;
    end; // while
    psd^.pid := lParam^.pid;
  end
  else if pg <> nil then
  begin
    // store the given ID
    pg^.hMenuGroupID := mii.wID;
    // steal the pointer from the group node it should be on the heap
    psd^.cch := pg^.cchGroup;
    psd^.szText := pg^.szGroup;
    psd^.fTypes := [dtGroup];
  end; // if
  psd^.wID := mii.wID;
  psd^.szProfile := nil;
  // store
  mii.dwItemData := uint_ptr(psd);

  if ((lParam^.bOwnerDrawSupported) and (lParam^.bShouldOwnerDraw)) then
  begin
    mii.fType := MFT_OWNERDRAW;
    Pointer(mii.dwTypeData) := psd;
  end
  else
  begin
    // normal menu
    mii.fType := MFT_STRING;
    if pct <> nil then
    begin
      uint_ptr(mii.dwTypeData) := uint_ptr(pct) + sizeof(TSlotIPC);
    end
    else
    begin
      mii.dwTypeData := pg^.szGroup;
    end;
    { For Vista + let the system draw the theme and icons, pct = contact associated data }
    if VistaOrLater and (pct <> nil) and (psd <> nil) then
    begin
      mii.fMask := MIIM_BITMAP or MIIM_FTYPE or MIIM_ID or MIIM_DATA or MIIM_STRING;
      // BuildSkinIcons() built an array of bitmaps which we can use here
      mii.hBmpItem := psd^.hStatusBitmap;
    end;
  end; // if
end;

// must be called after DecideMenuItemInfo()
procedure BuildMRU(pct: PSlotIPC; var mii: TMenuItemInfo; lParam: PEnumData);
begin
  if pct^.MRU > 0 then
  begin
    inc(lParam^.Self^.RecentCount);
    // lParam^.Self == pointer to object data
    InsertMenuitem(lParam^.Self^.hRecentMenu, $FFFFFFFF, True, mii);
  end;
end;

procedure BuildContactTree(group: PGroupNode; lParam: PEnumData);
label
  grouploop;
var
  pct: PSlotIPC;
  pg, px: PGroupNode;
  str: TStrTokRec;
  sz: PChar;
  Hash: Cardinal;
  Depth: Cardinal;
  mii: TMenuItemInfo;
begin
  // set up the menu item
  mii.cbSize := sizeof(TMenuItemInfo);
  mii.fMask := MIIM_ID or MIIM_TYPE or MIIM_DATA;
  // set up the scanner
  str.szSet := ['\'];
  str.bSetTerminator := False;
  // go thru all the contacts
  pct := lParam^.ipch^.ContactsBegin;
  while (pct <> nil) and (pct^.cbSize = sizeof(TSlotIPC)) and (pct^.fType = REQUEST_CONTACTS) do
  begin
    if pct^.hGroup <> 0 then
    begin
      // at the end of the slot header is the contact's display name
      // and after a double NULL char there is the group string, which has the full path of the group
      // this must be tokenised at '\' and we must walk the in memory group tree til we find our group
      // this is faster than the old version since we only ever walk one or at most two levels of the tree
      // per tokenised section, and it doesn't matter if two levels use the same group name (which is valid)
      // as the tokens processed is equatable to depth of the tree
      str.szStr := PChar(uint_ptr(pct) + sizeof(TSlotIPC) + uint_ptr(pct^.cbStrSection) + 1);
      sz := StrTok(str);
      // restore the root
      pg := group;
      Depth := 0;
      while sz <> nil do
      begin
        Hash := StrHash(sz);
        // find this node within
        while pg <> nil do
        begin
          // does this node have the right hash and the right depth?
          if (Hash = pg^.Hash) and (Depth = pg^.Depth) then
            break;
          // each node may have a left pointer going to a sub tree
          // the path syntax doesn't know if a group is a group at the same level
          // or a nested one, which means the search node can be anywhere
          px := pg^.Left;
          if px <> nil then
          begin
            // keep searching this level
            while px <> nil do
            begin
              if (Hash = px^.Hash) and (Depth = px^.Depth) then
              begin
                // found the node we're looking for at the next level to pg, px is now pq for next time
                pg := px;
                goto grouploop;
              end; // if
              px := px^.Right;
            end; // if
          end; // if
          pg := pg^.Right;
        end; // while
      grouploop:
        inc(Depth);
        // process next token
        sz := StrTok(str);
      end; // while
      // tokenisation finished, if pg <> nil then the group is found
      if pg <> nil then
      begin
        DecideMenuItemInfo(pct, nil, mii, lParam);
        BuildMRU(pct, mii, lParam);
        InsertMenuitem(pg^.hMenu, $FFFFFFFF, True, mii);
        inc(pg^.dwItems);
      end;
    end; // if
    pct := pct^.Next;
  end; // while
end;

procedure BuildMenuGroupTree(p: PGroupNode; lParam: PEnumData; hLastMenu: hMenu);
var
  mii: TMenuItemInfo;
begin
  mii.cbSize := sizeof(TMenuItemInfo);
  mii.fMask := MIIM_ID or MIIM_DATA or MIIM_TYPE or MIIM_SUBMENU;
  // go thru each group and create a menu for it adding submenus too.
  while p <> nil do
  begin
    mii.hSubMenu := CreatePopupMenu();
    if p^.Left <> nil then
      BuildMenuGroupTree(p^.Left, lParam, mii.hSubMenu);
    p^.hMenu := mii.hSubMenu;
    DecideMenuItemInfo(nil, p, mii, lParam);
    InsertMenuitem(hLastMenu, $FFFFFFFF, True, mii);
    p := p^.Right;
  end; // while
end;

{ this callback is triggered by the menu code and IPC is already taking place,
  just the transfer type+data needs to be setup }
function ClearMRUIPC(pipch: PHeaderIPC; // IPC header info, already mapped
  hWorkThreadEvent: THandle; // event object being waited on on miranda thread
  hAckEvent: THandle; // ack event object that has been created
  psd: PMenuDrawInfo // command/draw info
  ): Integer; stdcall;
begin
  Result := S_OK;
  ipcPrepareRequests(IPC_PACKET_SIZE, pipch, REQUEST_CLEARMRU);
  ipcSendRequest(hWorkThreadEvent, hAckEvent, pipch, 100);
end;

procedure RemoveCheckmarkSpace(hMenu: hMenu);
const
  MIM_STYLE = $00000010;
  MNS_CHECKORBMP = $4000000;
type
  TMENUINFO = record
    cbSize: DWORD;
    fMask: DWORD;
    dwStyle: DWORD;
    cyMax: LongInt;
    hbrBack: THandle;
    dwContextHelpID: DWORD;
    dwMenuData: Pointer;
  end;
var
  SetMenuInfo: function(hMenu: hMenu; var mi: TMENUINFO): Boolean; stdcall;
  mi: TMENUINFO;
begin
  if not VistaOrLater then
    Exit;
  SetMenuInfo := GetProcAddress(GetModuleHandle('user32'), 'SetMenuInfo');
  if @SetMenuInfo = nil then
    Exit;
  mi.cbSize := sizeof(mi);
  mi.fMask := MIM_STYLE;
  mi.dwStyle := MNS_CHECKORBMP;
  SetMenuInfo(hMenu, mi);
end;

procedure BuildMenus(lParam: PEnumData);
{$DEFINE SHL_IDC}
{$DEFINE SHL_KEYS}
{$INCLUDE shlc.inc}
{$UNDEF SHL_KEYS}
{$UNDEF SHL_IDC}
var
  hBaseMenu: hMenu;
  hGroupMenu: hMenu;
  pg: PSlotIPC;
  mii: TMenuItemInfo;
  j: TGroupNodeList;
  p, q: PGroupNode;
  Depth, Hash: Cardinal;
  Token: PChar;
  tk: TStrTokRec;
  hDllHeap: THandle;
  psd: PMenuDrawInfo;
  c: Cardinal;
  pp: ^TSlotProtoIconsArray;
begin
  ZeroMemory(@mii, sizeof(mii));
  hDllHeap := lParam^.Self^.hDllHeap;
  hBaseMenu := lParam^.Self^.hRootMenu;
  // build an in memory tree of the groups
  pg := lParam^.ipch^.GroupsBegin;
  tk.szSet := ['\'];
  tk.bSetTerminator := False;
  j.First := nil;
  j.Last := nil;
  while pg <> nil do
  begin
    if (pg^.cbSize <> sizeof(TSlotIPC)) or (pg^.fType <> REQUEST_GROUPS) then
      break;
    Depth := 0;
    p := j.First; // start at root again
    // get the group
    uint_ptr(tk.szStr) := (uint_ptr(pg) + sizeof(TSlotIPC));
    // find each word between \ and create sub groups if needed.
    Token := StrTok(tk);
    while Token <> nil do
    begin
      Hash := StrHash(Token);
      // if the (sub)group doesn't exist, create it.
      q := FindGroupNode(p, Hash, Depth);
      if q = nil then
      begin
        q := AllocGroupNode(@j, p, Depth);
        q^.Depth := Depth;
        // this is the hash of this group node, but it can be anywhere
        // i.e. Foo\Foo this is because each node has a different depth
        // trouble is contacts don't come with depths!
        q^.Hash := Hash;
        // don't assume that pg^.hGroup's hash is valid for this token
        // since it maybe Miranda\Blah\Blah and we have created the first node
        // which maybe Miranda, thus giving the wrong hash
        // since "Miranda" can be a group of it's own and a full path
        q^.cchGroup := lstrlena(Token);
        q^.szGroup := HeapAlloc(hDllHeap, 0, q^.cchGroup + 1);
        lstrcpya(q^.szGroup, Token);
        q^.dwItems := 0;
      end;
      p := q;
      inc(Depth);
      Token := StrTok(tk);
    end; // while
    pg := pg^.Next;
  end; // while
  // build the menus inserting into hGroupMenu which will be a submenu of
  // the instance menu item. e.g. Miranda -> [Groups ->] contacts
  hGroupMenu := CreatePopupMenu();

  // allocate MRU menu, this will be associated with the higher up menu
  // so doesn't need to be freed (unless theres no MRUs items attached)
  // This menu is per process but the handle is stored globally (like a stack)
  lParam^.Self^.hRecentMenu := CreatePopupMenu();
  lParam^.Self^.RecentCount := 0;
  // create group menus only if they exist!
  if lParam^.ipch^.GroupsBegin <> nil then
  begin
    BuildMenuGroupTree(j.First, lParam, hGroupMenu);
    // add contacts that have a group somewhere
    BuildContactTree(j.First, lParam);
  end;
  //
  mii.cbSize := sizeof(TMenuItemInfo);
  mii.fMask := MIIM_ID or MIIM_TYPE or MIIM_DATA;
  // add all the contacts that have no group (which maybe all of them)
  pg := lParam^.ipch^.ContactsBegin;
  while pg <> nil do
  begin
    if (pg^.cbSize <> sizeof(TSlotIPC)) or (pg^.fType <> REQUEST_CONTACTS) then
      break;
    if pg^.hGroup = 0 then
    begin
      DecideMenuItemInfo(pg, nil, mii, lParam);
      BuildMRU(pg, mii, lParam);
      InsertMenuitem(hGroupMenu, $FFFFFFFF, True, mii);
    end; // if
    pg := pg^.Next;
  end; // while

  // insert MRU menu as a submenu of the contact menu only if
  // the MRU list has been created, the menu popup will be deleted by itself
  if lParam^.Self^.RecentCount > 0 then
  begin

    // insert seperator and 'clear list' menu
    mii.fType := MFT_SEPARATOR;
    mii.fMask := MIIM_TYPE;
    InsertMenuitem(lParam^.Self^.hRecentMenu, $FFFFFFFF, True, mii);

    // insert 'clear MRU' item and setup callback
    mii.fMask := MIIM_TYPE or MIIM_ID or MIIM_DATA;
    mii.wID := lParam^.idCmdFirst;
    inc(lParam^.idCmdFirst);
    mii.fType := MFT_STRING;
    mii.dwTypeData := lParam^.ipch^.ClearEntries; // "Clear entries"
    // allocate menu substructure
    psd := HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
    psd^.fTypes := [dtCommand];
    psd^.MenuCommandCallback := @ClearMRUIPC;
    psd^.wID := mii.wID;
    // this is needed because there is a clear list command per each process.
    psd^.pid := lParam^.pid;
    Pointer(mii.dwItemData) := psd;
    InsertMenuitem(lParam^.Self^.hRecentMenu, $FFFFFFFF, True, mii);

    // insert MRU submenu into group menu (with) ownerdraw support as needed
    psd := HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
    psd^.szProfile := 'MRU';
    psd^.fTypes := [dtGroup];
    // the IPC string pointer wont be around forever, must make a copy
    psd^.cch := strlen(lParam^.ipch^.MRUMenuName);
    psd^.szText := HeapAlloc(hDllHeap, 0, psd^.cch + 1);
    lstrcpyn(psd^.szText, lParam^.ipch^.MRUMenuName, sizeof(lParam^.ipch^.MRUMenuName) - 1);

    pointer(mii.dwItemData) := psd;
    if (lParam^.bOwnerDrawSupported) and (lParam^.bShouldOwnerDraw) then
    begin
      mii.fType := MFT_OWNERDRAW;
      Pointer(mii.dwTypeData) := psd;
    end
    else
    begin
      mii.dwTypeData := lParam^.ipch^.MRUMenuName; // 'Recent';
    end;
    mii.wID := lParam^.idCmdFirst;
    inc(lParam^.idCmdFirst);
    mii.fMask := MIIM_TYPE or MIIM_SUBMENU or MIIM_DATA or MIIM_ID;
    mii.hSubMenu := lParam^.Self^.hRecentMenu;
    InsertMenuitem(hGroupMenu, 0, True, mii);
  end
  else
  begin
    // no items were attached to the MRU, delete the MRU menu
    DestroyMenu(lParam^.Self^.hRecentMenu);
    lParam^.Self^.hRecentMenu := 0;
  end;

  // allocate display info/memory for "Miranda" string

  mii.cbSize := sizeof(TMenuItemInfo);
  mii.fMask := MIIM_ID or MIIM_DATA or MIIM_TYPE or MIIM_SUBMENU;
  if VistaOrLater then
  begin
    mii.fMask := MIIM_ID or MIIM_DATA or MIIM_FTYPE or MIIM_SUBMENU or MIIM_STRING or
      MIIM_BITMAP;
  end;
  mii.hSubMenu := hGroupMenu;

  // by default, the menu will have space for icons and checkmarks (on Vista+) and we don't need this
  RemoveCheckmarkSpace(hGroupMenu);

  psd := HeapAlloc(hDllHeap, 0, sizeof(TMenuDrawInfo));
  psd^.cch := strlen(lParam^.ipch^.MirandaName);
  psd^.szText := HeapAlloc(hDllHeap, 0, psd^.cch + 1);
  lstrcpyn(psd^.szText, lParam^.ipch^.MirandaName, sizeof(lParam^.ipch^.MirandaName) - 1);
  // there may not be a profile name
  pg := lParam^.ipch^.DataPtr;
  psd^.szProfile := nil;
  if ((pg <> nil) and (pg^.Status = STATUS_PROFILENAME)) then
  begin
    psd^.szProfile := HeapAlloc(hDllHeap, 0, pg^.cbStrSection);
    lstrcpya(psd^.szProfile, PChar(uint_ptr(pg) + sizeof(TSlotIPC)));
  end; // if
  // owner draw menus need ID's
  mii.wID := lParam^.idCmdFirst;
  inc(lParam^.idCmdFirst);
  psd^.fTypes := [dtEntry];
  psd^.wID := mii.wID;
  psd^.hContact := 0;
  // get Miranda's icon or bitmap
  c := lParam^.Self^.ProtoIconsCount;
  pp := lParam^.Self^.ProtoIcons;
  while c > 0 do
  begin
    dec(c);
    if (pp[c].pid = lParam^.pid) and (pp[c].hProto = 0) then
    begin
      // either of these can be 0
      psd^.hStatusIcon := pp[c].hIcons[0];
      mii.hBmpItem := pp[c].hBitmaps[0];
      break;
    end; // if
  end; // while
  pointer(mii.dwItemData) := psd;
  if ((lParam^.bOwnerDrawSupported) and (lParam^.bShouldOwnerDraw)) then
  begin
    mii.fType := MFT_OWNERDRAW;
    Pointer(mii.dwTypeData) := psd;
  end
  else
  begin
    mii.fType := MFT_STRING;
    mii.dwTypeData := lParam^.ipch^.MirandaName;
    mii.cch := sizeof(lParam^.ipch^.MirandaName) - 1;
  end;
  // add it all
  InsertMenuitem(hBaseMenu, 0, True, mii);
  // free the group tree
  FreeGroupTreeAndEmptyGroups(hGroupMenu, nil, j.First);
end;

procedure BuildSkinIcons(lParam: PEnumData);
var
  pct: PSlotIPC;
  p, d: PSlotProtoIcons;
  Self: PShlComRec;
  j: Cardinal;
  imageFactory: PImageFactory_Interface;
begin
  pct := lParam^.ipch^.NewIconsBegin;
  Self := lParam^.Self;
  while (pct <> nil) do
  begin
    if (pct^.cbSize <> sizeof(TSlotIPC)) or (pct^.fType <> REQUEST_NEWICONS) then
      break;
    uint_ptr(p) := uint_ptr(pct) + sizeof(TSlotIPC);
    ReAllocMem(Self^.ProtoIcons, (Self^.ProtoIconsCount + 1) * sizeof(TSlotProtoIcons));
    d := @Self^.ProtoIcons[Self^.ProtoIconsCount];
    CopyMemory(d, p, sizeof(TSlotProtoIcons));

    {
      If using Vista (or later), clone all the icons into bitmaps and keep these around,
      if using anything older, just use the default code, the bitmaps (and or icons) will be freed
      with the shell object.
    }

    imageFactory := nil;

    for j := 0 to 9 do
    begin
      if imageFactory = nil then
        imageFactory := ARGB_GetWorker();
      if VistaOrLater then
      begin
        d^.hBitmaps[j] := ARGB_BitmapFromIcon(imageFactory, Self^.hMemDC, p^.hIcons[j]);
        d^.hIcons[j] := 0;
      end
      else
      begin
        d^.hBitmaps[j] := 0;
        d^.hIcons[j] := CopyIcon(p^.hIcons[j]);
      end;
    end;

    if imageFactory <> nil then
    begin
      imageFactory^.ptrVTable^.Release(imageFactory);
      imageFactory := nil;
    end;

    inc(Self^.ProtoIconsCount);
    pct := pct^.Next;
  end;
end;

function ProcessRequest(hwnd: hwnd; lParam: PEnumData): BOOL; stdcall;
var
  pid: Integer;
  hMirandaWorkEvent: THandle;
  replyBits: Integer;
  szBuf: array [0 .. MAX_PATH] of Char;
begin
  Result := True;
  pid := 0;
  GetWindowThreadProcessId(hwnd, @pid);
  If pid <> 0 then
  begin
    // old system would get a window's pid and the module handle that created it
    // and try to OpenEvent() a event object name to it (prefixed with a string)
    // this was fine for most Oses (not the best way) but now actually compares
    // the class string (a bit slower) but should get rid of those bugs finally.
    hMirandaWorkEvent := OpenEvent(EVENT_ALL_ACCESS, False, PChar(CreateProcessUID(pid)));
    if (hMirandaWorkEvent <> 0) then
    begin
      GetClassName(hwnd, szBuf, sizeof(szBuf));
      if lstrcmp(szBuf, MirandaName) <> 0 then
      begin
        // opened but not valid.
        CloseHandle(hMirandaWorkEvent);
        Exit;
      end; // if
    end; // if
    { If the event object exists, then a shlext.dll running in the instance must of created it. }
    If hMirandaWorkEvent <> 0 then
    begin
      { prep the request }
      ipcPrepareRequests(IPC_PACKET_SIZE, lParam^.ipch, REQUEST_ICONS or REQUEST_GROUPS or
        REQUEST_CONTACTS or REQUEST_NEWICONS);
      // slots will be in the order of icon data, groups then contacts, the first
      // slot will contain the profile name
      replyBits := ipcSendRequest(hMirandaWorkEvent, lParam^.hWaitFor, lParam^.ipch, 1000);
      { replyBits will be REPLY_FAIL if the wait timed out, or it'll be the request
        bits as sent or a series of *_NOTIMPL bits where the request bit were, if there are no
        contacts to speak of, then don't bother showing this instance of Miranda }
      if (replyBits <> REPLY_FAIL) and (lParam^.ipch^.ContactsBegin <> nil) then
      begin
        // load the address again, the server side will always overwrite it
        lParam^.ipch^.pClientBaseAddress := lParam^.ipch;
        // fixup all the pointers to be relative to the memory map
        // the base pointer of the client side version of the mapped file
        ipcFixupAddresses(False, lParam^.ipch);
        // store the PID used to create the work event object
        // that got replied to -- this is needed since each contact
        // on the final menu maybe on a different instance and another OpenEvent() will be needed.
        lParam^.pid := pid;
        // check out the user options from the server
        lParam^.bShouldOwnerDraw := (lParam^.ipch^.dwFlags and HIPC_NOICONS) = 0;
        // process the icons
        BuildSkinIcons(lParam);
        // process other replies
        BuildMenus(lParam);
      end;
      { close the work object }
      CloseHandle(hMirandaWorkEvent);
    end; // if
  end; // if
end;

function TShlComRec_QueryInterface(Self: PCommon_Interface; const IID: TIID; var Obj): HResult; stdcall;
begin
  Pointer(Obj) := nil;
  { IShellExtInit is given when the TShlRec is created }
  if IsEqualIID(IID, IID_IContextMenu) or IsEqualIID(IID, IID_IContextMenu2) or
    IsEqualIID(IID, IID_IContextMenu3) then
  begin
    with Self^.ptrInstance^ do
    begin
      Pointer(Obj) := @ContextMenu3_Interface;
      inc(RefCount);
    end; { with }
    Result := S_OK;
  end
  else
  begin
    // under XP, it may ask for IShellExtInit again, this fixes the -double- click to see menus issue
    // which was really just the object not being created
    if IsEqualIID(IID, IID_IShellExtInit) then
    begin
      with Self^.ptrInstance^ do
      begin
        Pointer(Obj) := @ShellExtInit_Interface;
        inc(RefCount);
      end; // if
      Result := S_OK;
    end
    else
    begin
      Result := CLASS_E_CLASSNOTAVAILABLE;
    end; // if
  end; // if
end;

function TShlComRec_AddRef(Self: PCommon_Interface): LongInt; stdcall;
begin
  with Self^.ptrInstance^ do
  begin
    inc(RefCount);
    Result := RefCount;
  end; { with }
end;

function TShlComRec_Release(Self: PCommon_Interface): LongInt; stdcall;
var
  j, c: Cardinal;
begin
  with Self^.ptrInstance^ do
  begin
    dec(RefCount);
    Result := RefCount;
    If RefCount = 0 then
    begin
      // time to go byebye.
      with Self^.ptrInstance^ do
      begin
        // Note MRU menu is associated with a window (indirectly) so windows will free it.
        // free icons!
        if ProtoIcons <> nil then
        begin
          c := ProtoIconsCount;
          while c > 0 do
          begin
            dec(c);
            for j := 0 to 9 do
            begin
              with ProtoIcons[c] do
              begin
                if hIcons[j] <> 0 then
                  DestroyIcon(hIcons[j]);
                if hBitmaps[j] <> 0 then
                  DeleteObject(hBitmaps[j]);
              end;
            end;
          end;
          FreeMem(ProtoIcons);
          ProtoIcons := nil;
        end; // if
        // free IDataObject reference if pointer exists
        if pDataObject <> nil then
        begin
          pDataObject^.ptrVTable^.Release(pDataObject);
        end; // if
        pDataObject := nil;
        // free the heap and any memory allocated on it
        HeapDestroy(hDllHeap);
        // destroy the DC
        if hMemDC <> 0 then
          DeleteDC(hMemDC);
      end; // with
      // free the instance (class record) created
      Dispose(Self^.ptrInstance);
      dec(dllpublic.ObjectCount);
    end; { if }
  end; { with }
end;

function TShlComRec_Initialise(Self: PContextMenu3_Interface; pidLFolder: Pointer;
  DObj: PDataObject_Interface; hKeyProdID: HKEY): HResult; stdcall;
begin
  // DObj is a pointer to an instance of IDataObject which is a pointer itself
  // it contains a pointer to a function table containing the function pointer
  // address of GetData() - the instance data has to be passed explicitly since
  // all compiler magic has gone.
  with Self^.ptrInstance^ do
  begin
    if DObj <> nil then
    begin
      Result := S_OK;
      // if an instance already exists, free it.
      if pDataObject <> nil then
        pDataObject^.ptrVTable^.Release(pDataObject);
      // store the new one and AddRef() it
      pDataObject := DObj;
      pDataObject^.ptrVTable^.AddRef(pDataObject);
    end
    else
    begin
      Result := E_INVALIDARG;
    end; // if
  end; // if
end;

function MAKE_HRESULT(Severity, Facility, Code: Integer): HResult;
{$IFDEF FPC}
inline;
{$ENDIF}
begin
  Result := (Severity shl 31) or (Facility shl 16) or Code;
end;

function TShlComRec_QueryContextMenu(Self: PContextMenu3_Interface; Menu: hMenu;
  indexMenu, idCmdFirst, idCmdLast, uFlags: UINT): HResult; stdcall;
type
  TDllVersionInfo = record
    cbSize: DWORD;
    dwMajorVersion: DWORD;
    dwMinorVersion: DWORD;
    dwBuildNumber: DWORD;
    dwPlatformID: DWORD;
  end;

  TDllGetVersionProc = function(var dv: TDllVersionInfo): HResult; stdcall;
var
  hShellInst: THandle;
  bMF_OWNERDRAW: Boolean;
  DllGetVersionProc: TDllGetVersionProc;
  dvi: TDllVersionInfo;
  ed: TEnumData;
  hMap: THandle;
  pipch: PHeaderIPC;
begin
  Result := 0;
  if ((LOWORD(uFlags) and CMF_VERBSONLY) <> CMF_VERBSONLY) and
    ((LOWORD(uFlags) and CMF_DEFAULTONLY) <> CMF_DEFAULTONLY) then
  begin
    bMF_OWNERDRAW := False;
    // get the shell version
    hShellInst := LoadLibrary('shell32.dll');
    if hShellInst <> 0 then
    begin
      DllGetVersionProc := GetProcAddress(hShellInst, 'DllGetVersion');
      if @DllGetVersionProc <> nil then
      begin
        dvi.cbSize := sizeof(TDllVersionInfo);
        if DllGetVersionProc(dvi) >= 0 then
        begin
          // it's at least 4.00
          bMF_OWNERDRAW := (dvi.dwMajorVersion > 4) or (dvi.dwMinorVersion >= 71);
        end; // if
      end; // if
      FreeLibrary(hShellInst);
    end; // if

    // if we're using Vista (or later), then the ownerdraw code will be disabled, because the system draws the icons.
    if VistaOrLater then
      bMF_OWNERDRAW := False;

    hMap := CreateFileMapping(INVALID_HANDLE_VALUE, nil, PAGE_READWRITE, 0, IPC_PACKET_SIZE,
      IPC_PACKET_NAME);
    If (hMap <> 0) and (GetLastError <> ERROR_ALREADY_EXISTS) then
    begin
      { map the memory to this address space }
      pipch := MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
      If pipch <> nil then
      begin
        { let the callback have instance vars }
        ed.Self := Self^.ptrInstance;
        // not used 'ere
        ed.Self^.hRootMenu := Menu;
        // store the first ID to offset with index for InvokeCommand()
        Self^.ptrInstance^.idCmdFirst := idCmdFirst;
        // store the starting index to offset
        Result := idCmdFirst;
        ed.bOwnerDrawSupported := bMF_OWNERDRAW;
        ed.bShouldOwnerDraw := True;
        ed.idCmdFirst := idCmdFirst;
        ed.ipch := pipch;
        { allocate a wait object so the ST can signal us, it can't be anon
          since it has to used by OpenEvent() }
        lstrcpya(@pipch^.SignalEventName, PChar(CreateUID()));
        { create the wait wait-for-wait object }
        ed.hWaitFor := CreateEvent(nil, False, False, pipch^.SignalEventName);
        If ed.hWaitFor <> 0 then
        begin
          { enumerate all the top level windows to find all loaded MIRANDANAME
            classes -- }
          EnumWindows(@ProcessRequest, lParam(@ed));
          { close the wait-for-reply object }
          CloseHandle(ed.hWaitFor);
        end;
        { unmap the memory from this address space }
        UnmapViewOfFile(pipch);
      end; { if }
      { close the mapping }
      CloseHandle(hMap);
      // use the MSDN recommended way, thou there ain't much difference
      Result := MAKE_HRESULT(0, 0, (ed.idCmdFirst - Result) + 1);
    end
    else
    begin
      // the mapping file already exists, which is not good!
    end;
  end
  else
  begin
    // same as giving a SEVERITY_SUCCESS, FACILITY_NULL, since that
    // just clears the higher bits, which is done anyway
    Result := MAKE_HRESULT(0, 0, 1);
  end; // if
end;

function TShlComRec_GetCommandString(Self: PContextMenu3_Interface; idCmd, uType: UINT;
  pwReserved: PUINT; pszName: PChar; cchMax: UINT): HResult; stdcall;
begin
  Result := E_NOTIMPL;
end;

function ipcGetFiles(pipch: PHeaderIPC; pDataObject: PDataObject_Interface; const hContact: THandle): Integer;
type
  TDragQueryFile = function(hDrop: THandle; fileIndex: Integer; FileName: PChar;
    cbSize: Integer): Integer; stdcall;
var
  fet: TFormatEtc;
  stgm: TStgMedium;
  pct: PSlotIPC;
  iFile: Cardinal;
  iFileMax: Cardinal;
  hShell: THandle;
  DragQueryFile: TDragQueryFile;
  cbSize: Integer;
  hDrop: THandle;
begin
  Result := E_INVALIDARG;
  hShell := LoadLibrary('shell32.dll');
  if hShell <> 0 then
  begin
    DragQueryFile := GetProcAddress(hShell, 'DragQueryFileA');
    if @DragQueryFile <> nil then
    begin
      fet.cfFormat := CF_HDROP;
      fet.ptd := nil;
      fet.dwAspect := DVASPECT_CONTENT;
      fet.lindex := -1;
      fet.tymed := TYMED_HGLOBAL;
      Result := pDataObject^.ptrVTable^.GetData(pDataObject, fet, stgm);
      if Result = S_OK then
      begin
        // FIX, actually lock the global object and get a pointer
        Pointer(hDrop) := GlobalLock(stgm.hGlobal);
        if hDrop <> 0 then
        begin
          // get the maximum number of files
          iFileMax := DragQueryFile(stgm.hGlobal, $FFFFFFFF, nil, 0);
          iFile := 0;
          while iFile < iFileMax do
          begin
            // get the size of the file path
            cbSize := DragQueryFile(stgm.hGlobal, iFile, nil, 0);
            // get the buffer
            pct := ipcAlloc(pipch, cbSize + 1); // including null term
            // allocated?
            if pct = nil then
              break;
            // store the hContact
            pct^.hContact := hContact;
            // copy it to the buffer
            DragQueryFile(stgm.hGlobal, iFile, PChar(uint_ptr(pct) + sizeof(TSlotIPC)), pct^.cbStrSection);
            // next file
            inc(iFile);
          end; // while
          // store the number of files
          pipch^.Slots := iFile;
          GlobalUnlock(stgm.hGlobal);
        end; // if hDrop check
        // release the mediumn the lock may of failed
        ReleaseStgMedium(stgm);
      end; // if
    end; // if
    // free the dll
    FreeLibrary(hShell);
  end; // if
end;

function RequestTransfer(Self: PShlComRec; idxCmd: Integer): Integer;
var
  hMap: THandle;
  pipch: PHeaderIPC;
  mii: TMenuItemInfo;
  hTransfer: THandle;
  psd: PMenuDrawInfo;
  hReply: THandle;
  replyBits: Integer;
begin
  Result := E_INVALIDARG;
  // get the contact information
  mii.cbSize := sizeof(TMenuItemInfo);
  mii.fMask := MIIM_ID or MIIM_DATA;
  if GetMenuItemInfo(Self^.hRootMenu, Self^.idCmdFirst + idxCmd, False, mii) then
  begin
    // get the pointer
    uint_ptr(psd) := mii.dwItemData;
    // the ID stored in the item pointer and the ID for the menu must match
    if (psd = nil) or (psd^.wID <> mii.wID) then
    begin
      // MessageBox(0,'ptr assocated with menu is NULL','',MB_OK);
      Exit;
    end; // if
  end
  else
  begin
    // MessageBox(0,'GetMenuItemInfo failed?','',MB_OK);
    // couldn't get the info, can't start the transfer
    Result := E_INVALIDARG;
    Exit;
  end; // if
  // is there an IDataObject instance?
  if Self^.pDataObject <> nil then
  begin
    // OpenEvent() the work object to see if the instance is still around
    hTransfer := OpenEvent(EVENT_ALL_ACCESS, False, PChar(CreateProcessUID(psd^.pid)));
    if hTransfer <> 0 then
    begin
      // map the ipc file again
      hMap := CreateFileMapping(INVALID_HANDLE_VALUE,nil,PAGE_READWRITE,0,IPC_PACKET_SIZE,IPC_PACKET_NAME);
      if (hMap <> 0) and (GetLastError <> ERROR_ALREADY_EXISTS) then
      begin
        // map it to process
        pipch := MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if pipch <> nil then
        begin
          // create the name of the object to be signalled by the ST
          lstrcpya(pipch^.SignalEventName, PChar(CreateUID()));
          // create it
          hReply := CreateEvent(nil, False, False, pipch^.SignalEventName);
          if hReply <> 0 then
          begin
            if dtCommand in psd^.fTypes then
            begin
              if Assigned(psd^.MenuCommandCallback) then
                Result := psd^.MenuCommandCallback(pipch, hTransfer, hReply, psd);
            end
            else
            begin

              // prepare the buffer
              ipcPrepareRequests(IPC_PACKET_SIZE, pipch, REQUEST_XFRFILES);
              // get all the files into the packet
              if ipcGetFiles(pipch, Self^.pDataObject, psd^.hContact) = S_OK then
              begin
                // need to wait for the ST to open the mapping object
                // since if we close it before it's opened it the data it
                // has will be undefined
                replyBits := ipcSendRequest(hTransfer, hReply, pipch, 200);
                if replyBits <> REPLY_FAIL then
                begin
                  // they got the files!
                  Result := S_OK;
                end; // if
              end;

            end;
            // close the work object name
            CloseHandle(hReply);
          end; // if
          // unmap it from this process
          UnmapViewOfFile(pipch);
        end; // if
        // close the map
        CloseHandle(hMap);
      end; // if
      // close the handle to the ST object name
      CloseHandle(hTransfer);
    end; // if
  end // if;
end;

function TShlComRec_InvokeCommand(Self: PContextMenu3_Interface;
  var lpici: TCMInvokeCommandInfo): HResult; stdcall;
begin
  Result := RequestTransfer(Self^.ptrInstance, LOWORD(uint_ptr(lpici.lpVerb)));
end;

function TShlComRec_HandleMenuMsgs(Self: PContextMenu3_Interface; uMsg: UINT; wParam: wParam;
  lParam: lParam; pResult: PLResult): HResult;
const
  WM_DRAWITEM = $002B;
  WM_MEASUREITEM = $002C;
var
  dwi: PDrawItemStruct;
  msi: PMeasureItemStruct;
  psd: PMenuDrawInfo;
  ncm: TNonClientMetrics;
  hOldFont: THandle;
  hFont: THandle;
  tS: TSize;
  dx: Integer;
  hBr: HBRUSH;
  icorc: TRect;
  hMemDC: HDC;
begin
  pResult^ := Integer(True);
  if (uMsg = WM_DRAWITEM) and (wParam = 0) then
  begin
    // either a main sub menu, a group menu or a contact
    dwi := PDrawItemStruct(lParam);
    uint_ptr(psd) := dwi^.itemData;
    // don't fill
    SetBkMode(dwi^.HDC, TRANSPARENT);
    // where to draw the icon?
    icorc.Left := 0;
    // center it
    with dwi^ do
      icorc.Top := rcItem.Top + ((rcItem.Bottom - rcItem.Top) div 2) - (16 div 2);
    icorc.Right := icorc.Left + 16;
    icorc.Bottom := icorc.Top + 16;
    // draw for groups
    if (dtGroup in psd^.fTypes) or (dtEntry in psd^.fTypes) then
    begin
      hBr := GetSysColorBrush(COLOR_MENU);
      FillRect(dwi^.HDC, dwi^.rcItem, hBr);
      DeleteObject(hBr);
      //
      if (ODS_SELECTED and dwi^.itemState = ODS_SELECTED) then
      begin
        // only do this for entry menu types otherwise a black mask
        // is drawn under groups
        hBr := GetSysColorBrush(COLOR_HIGHLIGHT);
        FillRect(dwi^.HDC, dwi^.rcItem, hBr);
        DeleteObject(hBr);
        SetTextColor(dwi^.HDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
      end; // if
      // draw icon
      with dwi^, icorc do
      begin
        if (ODS_SELECTED and dwi^.itemState) = ODS_SELECTED then
        begin
          hBr := GetSysColorBrush(COLOR_HIGHLIGHT);
        end
        else
        begin
          hBr := GetSysColorBrush(COLOR_MENU);
        end; // if
        DrawIconEx(HDC, Left + 1, Top, psd^.hStatusIcon, 16, 16, // width, height
          0, // step
          hBr, // brush
          DI_NORMAL);
        DeleteObject(hBr);
      end; // with
      // draw the text
      with dwi^ do
      begin
        inc(rcItem.Left, ((rcItem.Bottom - rcItem.Top) - 2));
        DrawText(HDC, psd^.szText, psd^.cch, rcItem, DT_NOCLIP or DT_NOPREFIX or
          DT_SINGLELINE or DT_VCENTER);
        // draw the name of the database text if it's there
        if psd^.szProfile <> nil then
        begin
          GetTextExtentPoint32(dwi^.HDC, psd^.szText, psd^.cch, tS);
          inc(rcItem.Left, tS.cx + 8);
          SetTextColor(HDC, GetSysColor(COLOR_GRAYTEXT));
          DrawText(HDC, psd^.szProfile, lstrlena(psd^.szProfile), rcItem,
            DT_NOCLIP or DT_NOPREFIX or DT_SINGLELINE or DT_VCENTER);
        end; // if
      end; // with
    end
    else
    begin
      // it's a contact!
      hBr := GetSysColorBrush(COLOR_MENU);
      FillRect(dwi^.HDC, dwi^.rcItem, hBr);
      DeleteObject(hBr);
      if ODS_SELECTED and dwi^.itemState = ODS_SELECTED then
      begin
        hBr := GetSysColorBrush(COLOR_HIGHLIGHT);
        FillRect(dwi^.HDC, dwi^.rcItem, hBr);
        DeleteObject(hBr);
        SetTextColor(dwi^.HDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
      end;
      // draw icon
      with dwi^, icorc do
      begin
        if (ODS_SELECTED and dwi^.itemState) = ODS_SELECTED then
        begin
          hBr := GetSysColorBrush(COLOR_HIGHLIGHT);
        end
        else
        begin
          hBr := GetSysColorBrush(COLOR_MENU);
        end; // if
        DrawIconEx(HDC, Left + 2, Top, psd^.hStatusIcon, 16, 16, // width, height
          0, // step
          hBr, // brush
          DI_NORMAL);
        DeleteObject(hBr);
      end; // with
      // draw the text
      with dwi^ do
      begin
        inc(rcItem.Left, (rcItem.Bottom - rcItem.Top) + 1);
        DrawText(HDC, psd^.szText, psd^.cch, rcItem, DT_NOCLIP or DT_NOPREFIX or
          DT_SINGLELINE or DT_VCENTER);
      end; // with
    end; // if
  end
  else if (uMsg = WM_MEASUREITEM) then
  begin
    // don't check if it's really a menu
    msi := PMeasureItemStruct(lParam);
    uint_ptr(psd) := msi^.itemData;
    ncm.cbSize := sizeof(TNonClientMetrics);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, @ncm, 0);
    // create the font used in menus, this font should be cached somewhere really
{$IFDEF FPC}
    hFont := CreateFontIndirect(@ncm.lfMenuFont);
{$ELSE}
    hFont := CreateFontIndirect(ncm.lfMenuFont);
{$ENDIF}
    hMemDC := Self^.ptrInstance^.hMemDC;
    // select in the font
    hOldFont := SelectObject(hMemDC, hFont);
    // default to an icon
    dx := 16;
    // get the size 'n' account for the icon
    GetTextExtentPoint32(hMemDC, psd^.szText, psd^.cch, tS);
    inc(dx, tS.cx);
    // main menu item?
    if psd^.szProfile <> nil then
    begin
      GetTextExtentPoint32(hMemDC, psd^.szProfile, lstrlena(psd^.szProfile), tS);
      inc(dx, tS.cx);
    end;
    // store it
    msi^.itemWidth := dx + Integer(ncm.iMenuWidth);
    msi^.itemHeight := Integer(ncm.iMenuHeight) + 2;
    if tS.cy > msi^.itemHeight then
      inc(msi^.itemHeight, tS.cy - msi^.itemHeight);
    // clean up
    SelectObject(hMemDC, hOldFont);
    DeleteObject(hFont);
  end;
  Result := S_OK;
end;

function TShlComRec_HandleMenuMsg(Self: PContextMenu3_Interface; uMsg: UINT; wParam: wParam;
  lParam: lParam): HResult; stdcall;
var
  Dummy: HResult;
begin
  Result := TShlComRec_HandleMenuMsgs(Self, uMsg, wParam, lParam, @Dummy);
end;

function TShlComRec_HandleMenuMsg2(Self: PContextMenu3_Interface; uMsg: UINT; wParam: wParam;
  lParam: lParam; PLResult: Pointer { ^LResult } ): HResult; stdcall;
var
  Dummy: HResult;
begin
  // this will be null if a return value isn't needed.
  if PLResult = nil then
    PLResult := @Dummy;
  Result := TShlComRec_HandleMenuMsgs(Self, uMsg, wParam, lParam, PLResult);
end;

function TShlComRec_Create: PShlComRec;
var
  DC: HDC;
begin
  New(Result);
  { build all the function tables for interfaces }
  with Result^.ShellExtInit_Interface do
  begin
    { this is only owned by us... }
    ptrVTable := @vTable;
    { IUnknown }
    vTable.QueryInterface := @TShlComRec_QueryInterface;
    vTable.AddRef := @TShlComRec_AddRef;
    vTable.Release := @TShlComRec_Release;
    { IShellExtInit }
    vTable.Initialise := @TShlComRec_Initialise;
    { instance of a TShlComRec }
    ptrInstance := Result;
  end;
  with Result^.ContextMenu3_Interface do
  begin
    ptrVTable := @vTable;
    { IUnknown }
    vTable.QueryInterface := @TShlComRec_QueryInterface;
    vTable.AddRef := @TShlComRec_AddRef;
    vTable.Release := @TShlComRec_Release;
    { IContextMenu }
    vTable.QueryContextMenu := @TShlComRec_QueryContextMenu;
    vTable.InvokeCommand := @TShlComRec_InvokeCommand;
    vTable.GetCommandString := @TShlComRec_GetCommandString;
    { IContextMenu2 }
    vTable.HandleMenuMsg := @TShlComRec_HandleMenuMsg;
    { IContextMenu3 }
    vTable.HandleMenuMsg2 := @TShlComRec_HandleMenuMsg2;
    { instance data }
    ptrInstance := Result;
  end;
  { initalise variables }
  Result^.RefCount := 1;
  Result^.hDllHeap := HeapCreate(0, 0, 0);
  Result^.hRootMenu := 0;
  Result^.hRecentMenu := 0;
  Result^.RecentCount := 0;
  Result^.idCmdFirst := 0;
  Result^.pDataObject := nil;
  Result^.ProtoIcons := nil;
  Result^.ProtoIconsCount := 0;
  // create an inmemory DC
  DC := GetDC(0);
  Result^.hMemDC := CreateCompatibleDC(DC);
  ReleaseDC(0, DC);
  { keep count on the number of objects }
  inc(dllpublic.ObjectCount);
end;

{ IClassFactory }

type

  PVTable_IClassFactory = ^TVTable_IClassFactory;

  TVTable_IClassFactory = record
    { IUnknown }
    QueryInterface: Pointer;
    AddRef: Pointer;
    Release: Pointer;
    { IClassFactory }
    CreateInstance: Pointer;
    LockServer: Pointer;
  end;

  PClassFactoryRec = ^TClassFactoryRec;

  TClassFactoryRec = record
    ptrVTable: PVTable_IClassFactory;
    vTable: TVTable_IClassFactory;
    { fields }
    RefCount: LongInt;
  end;

function TClassFactoryRec_QueryInterface(Self: PClassFactoryRec; const IID: TIID; var Obj): HResult; stdcall;
begin
  Pointer(Obj) := nil;
  Result := E_NOTIMPL;
end;

function TClassFactoryRec_AddRef(Self: PClassFactoryRec): LongInt; stdcall;
begin
  inc(Self^.RefCount);
  Result := Self^.RefCount;
end;

function TClassFactoryRec_Release(Self: PClassFactoryRec): LongInt; stdcall;
begin
  dec(Self^.RefCount);
  Result := Self^.RefCount;
  if Result = 0 then
  begin
    Dispose(Self);
    dec(dllpublic.FactoryCount);
  end; { if }
end;

function TClassFactoryRec_CreateInstance(Self: PClassFactoryRec; unkOuter: Pointer;
  const IID: TIID; var Obj): HResult; stdcall;
var
  ShlComRec: PShlComRec;
begin
  Pointer(Obj) := nil;
  Result := CLASS_E_NOAGGREGATION;
  if unkOuter = nil then
  begin
    { Before Vista, the system queried for a IShell interface then queried for a context menu, Vista now
      queries for a context menu (or a shell menu) then QI()'s the other interface }
    if IsEqualIID(IID, IID_IContextMenu) then
    begin
      Result := S_OK;
      ShlComRec := TShlComRec_Create;
      Pointer(Obj) := @ShlComRec^.ContextMenu3_Interface;
    end;
    if IsEqualIID(IID, IID_IShellExtInit) then
    begin
      Result := S_OK;
      ShlComRec := TShlComRec_Create;
      Pointer(Obj) := @ShlComRec^.ShellExtInit_Interface;
    end; // if
  end; // if
end;

function TClassFactoryRec_LockServer(Self: PClassFactoryRec; fLock: BOOL): HResult; stdcall;
begin
  Result := E_NOTIMPL;
end;

function TClassFactoryRec_Create: PClassFactoryRec;
begin
  New(Result);
  Result^.ptrVTable := @Result^.vTable;
  { IUnknown }
  Result^.vTable.QueryInterface := @TClassFactoryRec_QueryInterface;
  Result^.vTable.AddRef := @TClassFactoryRec_AddRef;
  Result^.vTable.Release := @TClassFactoryRec_Release;
  { IClassFactory }
  Result^.vTable.CreateInstance := @TClassFactoryRec_CreateInstance;
  Result^.vTable.LockServer := @TClassFactoryRec_LockServer;
  { inital the variables }
  Result^.RefCount := 1;
  { count the number of factories }
  inc(dllpublic.FactoryCount);
end;

//
// IPC part
//

type
  PFileList = ^TFileList;
  TFileList = array [0 .. 0] of PChar;
  PAddArgList = ^TAddArgList;

  TAddArgList = record
    szFile: PChar; // file being processed
    cch: Cardinal; // it's length (with space for NULL char)
    count: Cardinal; // number we have so far
    files: PFileList;
    hContact: THandle;
    hEvent: THandle;
  end;

function AddToList(var args: TAddArgList): LongBool;
var
  attr: Cardinal;
  p: Pointer;
  hFind: THandle;
  fd: TWIN32FINDDATA;
  szBuf: array [0 .. MAX_PATH] of Char;
  szThis: PChar;
  cchThis: Cardinal;
begin
  Result := False;
  attr := GetFileAttributes(args.szFile);
  if (attr <> $FFFFFFFF) and ((attr and FILE_ATTRIBUTE_HIDDEN) = 0) then
  begin
    if args.count mod 10 = 5 then
    begin
      if CallService(MS_SYSTEM_TERMINATED, 0, 0) <> 0 then
      begin
        Result := True;
        Exit;
      end; // if
    end;
    if attr and FILE_ATTRIBUTE_DIRECTORY <> 0 then
    begin
      // add the directory
      lstrcpya(szBuf, args.szFile);
      ReAllocMem(args.files, (args.count + 1) * sizeof(PChar));
      GetMem(p, strlen(szBuf) + 1);
      lstrcpya(p, szBuf);
      args.files^[args.count] := p;
      inc(args.count);
      // tack on ending search token
      lstrcata(szBuf, '\*');
      hFind := FindFirstFile(szBuf, fd);
      while True do
      begin
        if fd.cFileName[0] <> '.' then
        begin
          lstrcpya(szBuf, args.szFile);
          lstrcata(szBuf, '\');
          lstrcata(szBuf, fd.cFileName);
          // keep a copy of the current thing being processed
          szThis := args.szFile;
          args.szFile := szBuf;
          cchThis := args.cch;
          args.cch := strlen(szBuf) + 1;
          // recurse
          Result := AddToList(args);
          // restore
          args.szFile := szThis;
          args.cch := cchThis;
          if Result then
            break;
        end; // if
        if not FindNextFile(hFind, fd) then
          break;
      end; // while
      FindClose(hFind);
    end
    else
    begin
      // add the file
      ReAllocMem(args.files, (args.count + 1) * sizeof(PChar));
      GetMem(p, args.cch);
      lstrcpya(p, args.szFile);
      args.files^[args.count] := p;
      inc(args.count);
    end; // if
  end;
end;

procedure MainThreadIssueTransfer(p: PAddArgList); stdcall;
{$DEFINE SHL_IDC}
{$DEFINE SHL_KEYS}
{$INCLUDE shlc.inc}
{$UNDEF SHL_KEYS}
{$UNDEF SHL_IDC}
begin
  DBWriteContactSettingByte(p^.hContact, SHLExt_Name, SHLExt_MRU, 1);
  CallService(MS_FILE_SENDSPECIFICFILES, p^.hContact, lParam(p^.files));
  SetEvent(p^.hEvent);
end;

procedure IssueTransferThread(pipch: PHeaderIPC); cdecl;
var
  szBuf: array [0 .. MAX_PATH] of Char;
  pct: PSlotIPC;
  args: TAddArgList;
  bQuit: LongBool;
  j, c: Cardinal;
  p: Pointer;
  hMainThread: THandle;
begin
  hMainThread := THandle(pipch^.Param);
  GetCurrentDirectory(sizeof(szBuf), szBuf);
  args.count := 0;
  args.files := nil;
  pct := pipch^.DataPtr;
  bQuit := False;
  while pct <> nil do
  begin
    if (pct^.cbSize <> sizeof(TSlotIPC)) then
      break;
    args.szFile := PChar(uint_ptr(pct) + sizeof(TSlotIPC));
    args.hContact := pct^.hContact;
    args.cch := pct^.cbStrSection + 1;
    bQuit := AddToList(args);
    if bQuit then
      break;
    pct := pct^.Next;
  end; // while
  if args.files <> nil then
  begin
    ReAllocMem(args.files, (args.count + 1) * sizeof(PChar));
    args.files^[args.count] := nil;
    inc(args.count);
    if (not bQuit) then
    begin
      args.hEvent := CreateEvent(nil, True, False, nil);
      QueueUserAPC(@MainThreadIssueTransfer, hMainThread, uint_ptr(@args));
      while True do
      begin
        if WaitForSingleObjectEx(args.hEvent, INFINITE, True) <> WAIT_IO_COMPLETION then
          break;
      end;
      CloseHandle(args.hEvent);
    end; // if
    c := args.count - 1;
    for j := 0 to c do
    begin
      p := args.files^[j];
      if p <> nil then
        FreeMem(p);
    end;
    FreeMem(args.files);
  end;
  SetCurrentDirectory(szBuf);
  FreeMem(pipch);
  CloseHandle(hMainThread);
end;

type

  PSlotInfo = ^TSlotInfo;

  TSlotInfo = record
    hContact: THandle;
    hProto: Cardinal;
    dwStatus: Integer; // will be aligned anyway
  end;

  TSlotArray = array [0 .. $FFFFFF] of TSlotInfo;
  PSlotArray = ^TSlotArray;

function SortContact(var Item1, Item2: TSlotInfo): Integer; stdcall;
begin
  Result := CallService(MS_CLIST_CONTACTSCOMPARE, Item1.hContact, Item2.hContact);
end;

// from FP FCL

procedure QuickSort(FList: PSlotArray; L, R: LongInt);
var
  i, j: LongInt;
  p, q: TSlotInfo;
begin
  repeat
    i := L;
    j := R;
    p := FList^[(L + R) div 2];
    repeat
      while SortContact(p, FList^[i]) > 0 do
        inc(i);
      while SortContact(p, FList^[j]) < 0 do
        dec(j);
      if i <= j then
      begin
        q := FList^[i];
        FList^[i] := FList^[j];
        FList^[j] := q;
        inc(i);
        dec(j);
      end; // if
    until i > j;
    if L < j then
      QuickSort(FList, L, j);
    L := i;
  until i >= R;
end;

{$DEFINE SHL_KEYS}
{$INCLUDE shlc.inc}
{$UNDEF SHL_KEYS}

procedure ipcGetSkinIcons(ipch: PHeaderIPC);
var
  protoCount: Integer;
  pp: ^PPROTOCOLDESCRIPTOR;
  spi: TSlotProtoIcons;
  j: Cardinal;
  pct: PSlotIPC;
  szTmp: array [0 .. 63] of Char;
  dwCaps: Cardinal;
begin
  if (CallService(MS_PROTO_ENUMACCOUNTS, wParam(@protoCount), lParam(@pp)) = 0) and
    (protoCount <> 0) then
  begin
    spi.pid := GetCurrentProcessId();
    while protoCount > 0 do
    begin
      lstrcpya(szTmp, pp^.szName);
      lstrcata(szTmp, PS_GETCAPS);
      dwCaps := CallService(szTmp, PFLAGNUM_1, 0);
      if (dwCaps and PF1_FILESEND) <> 0 then
      begin
        pct := ipcAlloc(ipch, sizeof(TSlotProtoIcons));
        if pct <> nil then
        begin
          // capture all the icons!
          spi.hProto := StrHash(pp^.szName);
          for j := 0 to 9 do
          begin
            spi.hIcons[j] := LoadSkinnedProtoIcon(pp^.szName, ID_STATUS_OFFLINE + j);
          end; // for
          pct^.fType := REQUEST_NEWICONS;
          CopyMemory(Pointer(uint_ptr(pct) + sizeof(TSlotIPC)), @spi, sizeof(TSlotProtoIcons));
          if ipch^.NewIconsBegin = nil then
            ipch^.NewIconsBegin := pct;
        end; // if
      end; // if
      inc(pp);
      dec(protoCount);
    end; // while
  end; // if
  // add Miranda icon
  pct := ipcAlloc(ipch, sizeof(TSlotProtoIcons));
  if pct <> nil then
  begin
    ZeroMemory(@spi.hIcons, sizeof(spi.hIcons));
    spi.hProto := 0; // no protocol
    spi.hIcons[0] := LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
    pct^.fType := REQUEST_NEWICONS;
    CopyMemory(Pointer(uint_ptr(pct) + sizeof(TSlotIPC)), @spi, sizeof(TSlotProtoIcons));
    if ipch^.NewIconsBegin = nil then
      ipch^.NewIconsBegin := pct;
  end; // if
end;

function ipcGetSortedContacts(ipch: PHeaderIPC; pSlot: pint; bGroupMode: Boolean): Boolean;
var
  dwContacts: Cardinal;
  pContacts: PSlotArray;
  hContact: THandle;
  i: Integer;
  dwOnline: Cardinal;
  szProto: PChar;
  dwStatus: Integer;
  pct: PSlotIPC;
  szContact: PChar;
  dbv: TDBVariant;
  bHideOffline: Boolean;
  szTmp: array [0 .. 63] of Char;
  dwCaps: Cardinal;
  szSlot: PChar;
  n, rc, cch: Cardinal;
begin
  Result := False;
  // hide offliners?
  bHideOffline := DBGetContactSettingByte(0, 'CList', 'HideOffline', 0) = 1;
  // do they wanna hide the offline people anyway?
  if DBGetContactSettingByte(0, SHLExt_Name, SHLExt_ShowNoOffline, 0) = 1 then
  begin
    // hide offline people
    bHideOffline := True;
  end;
  // get the number of contacts
  dwContacts := CallService(MS_DB_CONTACT_GETCOUNT, 0, 0);
  if dwContacts = 0 then
    Exit;
  // get the contacts in the array to be sorted by status, trim out anyone
  // who doesn't wanna be seen.
  GetMem(pContacts, (dwContacts + 2) * sizeof(TSlotInfo));
  i := 0;
  dwOnline := 0;
  hContact := db_find_first();
  while (hContact <> 0) do
  begin
    if i >= dwContacts then
      break;
    (* do they have a running protocol? *)
    uint_ptr(szProto) := CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);
    if szProto <> nil then
    begin
      (* does it support file sends? *)
      lstrcpya(szTmp, szProto);
      lstrcata(szTmp, PS_GETCAPS);
      dwCaps := CallService(szTmp, PFLAGNUM_1, 0);
      if (dwCaps and PF1_FILESEND) = 0 then
      begin
        hContact := db_find_next(hContact);
        continue;
      end;
      dwStatus := DBGetContactSettingWord(hContact, szProto, 'Status', ID_STATUS_OFFLINE);
      if dwStatus <> ID_STATUS_OFFLINE then
        inc(dwOnline)
      else if bHideOffline then
      begin
        hContact := db_find_next(hContact);
        continue;
      end; // if
      // is HIT on?
      if BST_UNCHECKED = DBGetContactSettingByte(0, SHLExt_Name, SHLExt_UseHITContacts,
        BST_UNCHECKED) then
      begin
        // don't show people who are "Hidden" "NotOnList" or Ignored
        if (DBGetContactSettingByte(hContact, 'CList', 'Hidden', 0) = 1) or
          (DBGetContactSettingByte(hContact, 'CList', 'NotOnList', 0) = 1) or
          (CallService(MS_IGNORE_ISIGNORED, hContact, IGNOREEVENT_MESSAGE or
          IGNOREEVENT_URL or IGNOREEVENT_FILE) <> 0) then
        begin
          hContact := db_find_next(hContact);
          continue;
        end; // if
      end; // if
      // is HIT2 off?
      if BST_UNCHECKED = DBGetContactSettingByte(0, SHLExt_Name, SHLExt_UseHIT2Contacts,
        BST_UNCHECKED) then
      begin
        if DBGetContactSettingWord(hContact, szProto, 'ApparentMode', 0) = ID_STATUS_OFFLINE
        then
        begin
          hContact := db_find_next(hContact);
          continue;
        end; // if
      end; // if
      // store
      pContacts^[i].hContact := hContact;
      pContacts^[i].dwStatus := dwStatus;
      pContacts^[i].hProto := StrHash(szProto);
      inc(i);
    end
    else
    begin
      // contact has no protocol!
    end; // if
    hContact := db_find_next(hContact);
  end; // while
  // if no one is online and the CList isn't showing offliners, quit
  if (dwOnline = 0) and (bHideOffline) then
  begin
    FreeMem(pContacts);
    Exit;
  end; // if
  dwContacts := i;
  i := 0;
  // sort the array
  QuickSort(pContacts, 0, dwContacts - 1);
  // create an IPC slot for each contact and store display name, etc
  while i < dwContacts do
  begin
    uint_ptr(szContact) := CallService(MS_CLIST_GETCONTACTDISPLAYNAME,pContacts^[i].hContact, 0);
    if (szContact <> nil) then
    begin
      n := 0;
      rc := 1;
      if bGroupMode then
      begin
        rc := DBGetContactSetting(pContacts^[i].hContact, 'CList', 'Group', @dbv);
        if rc = 0 then
        begin
          n := lstrlena(dbv.szVal.a) + 1;
        end;
      end; // if
      cch := lstrlena(szContact) + 1;
      pct := ipcAlloc(ipch, cch + 1 + n);
      if pct = nil then
      begin
        DBFreeVariant(@dbv);
        break;
      end;
      // lie about the actual size of the TSlotIPC
      pct^.cbStrSection := cch;
      szSlot := PChar(uint_ptr(pct) + sizeof(TSlotIPC));
      lstrcpya(szSlot, szContact);
      pct^.fType := REQUEST_CONTACTS;
      pct^.hContact := pContacts^[i].hContact;
      pct^.Status := pContacts^[i].dwStatus;
      pct^.hProto := pContacts^[i].hProto;
      pct^.MRU := DBGetContactSettingByte(pct^.hContact, SHLExt_Name, SHLExt_MRU, 0);
      if ipch^.ContactsBegin = nil then
        ipch^.ContactsBegin := pct;
      inc(szSlot, cch + 1);
      if rc = 0 then
      begin
        pct^.hGroup := StrHash(dbv.szVal.a);
        lstrcpya(szSlot, dbv.szVal.a);
        DBFreeVariant(@dbv);
      end
      else
      begin
        pct^.hGroup := 0;
        szSlot^ := #0;
      end;
      inc(pSlot^);
    end; // if
    inc(i);
  end; // while
  FreeMem(pContacts);
  //
  Result := True;
end;

// worker thread to clear MRU, called by the IPC bridge
procedure ClearMRUThread(notused: Pointer); cdecl;
{$DEFINE SHL_IDC}
{$DEFINE SHL_KEYS}
{$INCLUDE shlc.inc}
{$UNDEF SHL_KEYS}
{$UNDEF SHL_IDC}
var
  hContact: THandle;
begin
  begin
    hContact := db_find_first();
    while hContact <> 0 do
    begin
      if DBGetContactSettingByte(hContact, SHLExt_Name, SHLExt_MRU, 0) > 0 then
      begin
        DBWriteContactSettingByte(hContact, SHLExt_Name, SHLExt_MRU, 0);
      end;
      hContact := db_find_next(hContact);
    end;
  end;
end;

// this function is called from an APC into the main thread
procedure ipcService(dwParam: DWORD); stdcall;
label
  Reply;
var
  hMap: THandle;
  pMMT: PHeaderIPC;
  hSignal: THandle;
  pct: PSlotIPC;
  szBuf: PChar;
  iSlot: Integer;
  szGroupStr: array [0 .. 31] of Char;
  dbv: TDBVariant;
  bits: pint;
  bGroupMode: Boolean;
  cloned: PHeaderIPC;
  szMiranda: PChar;
begin
  { try to open the file mapping object the caller must make sure no other
    running instance is using this file }
  hMap := OpenFileMapping(FILE_MAP_ALL_ACCESS, False, IPC_PACKET_NAME);
  If hMap <> 0 then
  begin
    { map the file to this process }
    pMMT := MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    { if it fails the caller should of had some timeout in wait }
    if (pMMT <> nil) and (pMMT^.cbSize = sizeof(THeaderIPC)) and
      (pMMT^.dwVersion = PLUGIN_MAKE_VERSION(2, 0, 1, 2)) then
    begin
      // toggle the right bits
      bits := @pMMT^.fRequests;
      // jump right to a worker thread for file processing?
      if (bits^ and REQUEST_XFRFILES) = REQUEST_XFRFILES then
      begin
        GetMem(cloned, IPC_PACKET_SIZE);
        // translate from client space to cloned heap memory
        pMMT^.pServerBaseAddress := pMMT^.pClientBaseAddress;
        pMMT^.pClientBaseAddress := cloned;
        CopyMemory(cloned, pMMT, IPC_PACKET_SIZE);
        ipcFixupAddresses(True, cloned);
        DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
          @cloned^.Param, THREAD_SET_CONTEXT, False, 0);
        mir_forkThread(@IssueTransferThread, cloned);
        goto Reply;
      end;
      // the request was to clear the MRU entries, we have no return data
      if (bits^ and REQUEST_CLEARMRU) = REQUEST_CLEARMRU then
      begin
        mir_forkThread(@ClearMRUThread, nil);
        goto Reply;
      end;
      // the IPC header may have pointers that need to be translated
      // in either case the supplied data area pointers has to be
      // translated to this address space.
      // the server base address is always removed to get an offset
      // to which the client base is added, this is what ipcFixupAddresses() does
      pMMT^.pServerBaseAddress := pMMT^.pClientBaseAddress;
      pMMT^.pClientBaseAddress := pMMT;
      // translate to the server space map
      ipcFixupAddresses(True, pMMT);
      // store the address map offset so the caller can retranslate
      pMMT^.pServerBaseAddress := pMMT;
      // return some options to the client
      if DBGetContactSettingByte(0, SHLExt_Name, SHLExt_ShowNoIcons, 0) <> 0 then
      begin
        pMMT^.dwFlags := HIPC_NOICONS;
      end;
      // see if we have a custom string for 'Miranda'
      szMiranda := Translate('Miranda');
      lstrcpyn(pMMT^.MirandaName, szMiranda, sizeof(pMMT^.MirandaName) - 1);

      // for the MRU menu
      szBuf := Translate('Recently');
      lstrcpyn(pMMT^.MRUMenuName, szBuf, sizeof(pMMT^.MRUMenuName) - 1);

      // and a custom string for "clear entries"
      szBuf := Translate('Clear entries');
      lstrcpyn(pMMT^.ClearEntries, szBuf, sizeof(pMMT^.ClearEntries) - 1);

      // if the group mode is on, check if they want the CList setting
      bGroupMode := BST_CHECKED = DBGetContactSettingByte(0, SHLExt_Name, SHLExt_UseGroups,
        BST_UNCHECKED);
      if bGroupMode and (BST_CHECKED = DBGetContactSettingByte(0, SHLExt_Name,
        SHLExt_UseCListSetting, BST_UNCHECKED)) then
      begin
        bGroupMode := 1 = DBGetContactSettingByte(0, 'CList', 'UseGroups', 0);
      end;
      iSlot := 0;
      // return profile if set
      if BST_UNCHECKED = DBGetContactSettingByte(0, SHLExt_Name, SHLExt_ShowNoProfile,
        BST_UNCHECKED) then
      begin
        pct := ipcAlloc(pMMT, 50);
        if pct <> nil then
        begin
          // will actually return with .dat if there's space for it, not what the docs say
          pct^.Status := STATUS_PROFILENAME;
          CallService(MS_DB_GETPROFILENAME, 49, uint_ptr(pct) + sizeof(TSlotIPC));
        end; // if
      end; // if
      if (bits^ and REQUEST_NEWICONS) = REQUEST_NEWICONS then
      begin
        ipcGetSkinIcons(pMMT);
      end;
      if (bits^ and REQUEST_GROUPS = REQUEST_GROUPS) then
      begin
        // return contact's grouping if it's present
        while bGroupMode do
        begin
          str(iSlot, szGroupStr);
          if DBGetContactSetting(0, 'CListGroups', szGroupStr, @dbv) <> 0 then
            break;
          pct := ipcAlloc(pMMT, lstrlena(dbv.szVal.a + 1) + 1);
          // first byte has flags, need null term
          if pct <> nil then
          begin
            if pMMT^.GroupsBegin = nil then
              pMMT^.GroupsBegin := pct;
            pct^.fType := REQUEST_GROUPS;
            pct^.hContact := 0;
            uint_ptr(szBuf) := uint_ptr(pct) + sizeof(TSlotIPC); // get the end of the slot
            lstrcpya(szBuf, dbv.szVal.a + 1);
            pct^.hGroup := 0;
            DBFreeVariant(@dbv); // free the string
          end
          else
          begin
            // outta space
            DBFreeVariant(@dbv);
            break;
          end; // if
          inc(iSlot);
        end; { while }
        // if there was no space left, it'll end on null
        if pct = nil then
          bits^ := (bits^ or GROUPS_NOTIMPL) and not REQUEST_GROUPS;
      end; { if: group request }
      // SHOULD check slot space.
      if (bits^ and REQUEST_CONTACTS = REQUEST_CONTACTS) then
      begin
        if not ipcGetSortedContacts(pMMT, @iSlot, bGroupMode) then
        begin
          // fail if there were no contacts AT ALL
          bits^ := (bits^ or CONTACTS_NOTIMPL) and not REQUEST_CONTACTS;
        end; // if
      end; // if:contact request
      // store the number of slots allocated
      pMMT^.Slots := iSlot;
    Reply:
      { get the handle the caller wants to be signalled on }
      hSignal := OpenEvent(EVENT_ALL_ACCESS, False, pMMT^.SignalEventName);
      { did it open? }
      If hSignal <> 0 then
      begin
        { signal and close }
        SetEvent(hSignal);
        CloseHandle(hSignal);
      end;
      { unmap the shared memory from this process }
      UnmapViewOfFile(pMMT);
    end;
    { close the map file }
    CloseHandle(hMap);
  end; { if }
  //
end;

procedure ThreadServer(hMainThread: Pointer); cdecl;
var
  hEvent: THandle;
  retVal: Cardinal;
begin
  hEvent := CreateEvent(nil, False, False, PChar(CreateProcessUID(GetCurrentProcessId())));
  while True do
  begin
    retVal := WaitForSingleObjectEx(hEvent, INFINITE, True);
    if retVal = WAIT_OBJECT_0 then
    begin
      QueueUserAPC(@ipcService, THandle(hMainThread), 0);
    end; // if
    if CallService(MS_SYSTEM_TERMINATED, 0, 0) = 1 then
      break;
  end; // while
  CloseHandle(hEvent);
  CloseHandle(THandle(hMainThread));
end;

procedure InvokeThreadServer;
var
  hMainThread: THandle;
begin
  hMainThread := 0;
  DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), @hMainThread,
    THREAD_SET_CONTEXT, False, 0);
  if hMainThread <> 0 then
    mir_forkThread(@ThreadServer, Pointer(hMainThread));
end;

{ exported functions }

function DllGetClassObject(const CLSID: TCLSID; const IID: TIID; var Obj): HResult; stdcall;
begin
  Pointer(Obj) := nil;
  Result := CLASS_E_CLASSNOTAVAILABLE;
  if (IsEqualCLSID(CLSID, CLSID_ISHLCOM)) and (IsEqualIID(IID, IID_IClassFactory)) and
    (FindWindow(MirandaName, nil) <> 0) then
  begin
    Pointer(Obj) := TClassFactoryRec_Create;
    Result := S_OK;
  end; // if
end;

function DllCanUnloadNow: HResult;
begin
  if ((dllpublic.FactoryCount = 0) and (dllpublic.ObjectCount = 0)) then
  begin
    Result := S_OK;
  end
  else
  begin
    Result := S_FALSE;
  end; // if
end;

{ helper functions }

type

  PSHELLEXECUTEINFO = ^TSHELLEXECUTEINFO;

  TSHELLEXECUTEINFO = record
    cbSize: DWORD;
    fMask: LongInt;
    hwnd: THandle;
    lpVerb: PChar;
    lpFile: PChar;
    lpParameters: PChar;
    lpDirectory: PChar;
    nShow: Integer;
    hInstApp: THandle;
    lpIDLIst: Pointer;
    lpClass: PChar;
    HKEY: THandle;
    dwHotkey: DWORD;
    HICON: THandle; // is union
    hProcess: THandle;
  end;

function ShellExecuteEx(var se: TSHELLEXECUTEINFO): Boolean; stdcall;
  external 'shell32.dll' name 'ShellExecuteExA';

function wsprintfs(lpOut, lpFmt: PChar; args: PChar): Integer; cdecl;
  external 'user32.dll' name 'wsprintfA';

function RemoveCOMRegistryEntries: HResult;
var
  hRootKey: HKEY;
begin
  if RegOpenKeyEx(HKEY_CLASSES_ROOT, 'miranda.shlext', 0, KEY_READ, hRootKey) = ERROR_SUCCESS
  then
  begin
    (* need to delete the subkey before the parent key is deleted under NT/2000/XP *)
    RegDeleteKey(hRootKey, 'CLSID');
    (* close the key *)
    RegCloseKey(hRootKey);
    (* delete it *)
    if RegDeleteKey(HKEY_CLASSES_ROOT, 'miranda.shlext') <> ERROR_SUCCESS then
    begin
      MessageBox(0,
        'Unable to delete registry key for "shlext COM", this key may already be deleted or you may need admin rights.',
        'Problem', MB_ICONERROR);
    end; // if
  end; // if
  if RegOpenKeyEx(HKEY_CLASSES_ROOT, '\*\shellex\ContextMenuHandlers', 0, KEY_ALL_ACCESS,
    hRootKey) = ERROR_SUCCESS then
  begin
    if RegDeleteKey(hRootKey, 'miranda.shlext') <> ERROR_SUCCESS then
    begin
      MessageBox(0,
        'Unable to delete registry key for "File context menu handlers", this key may already be deleted or you may need admin rights.',
        'Problem', MB_ICONERROR);
    end; // if
    RegCloseKey(hRootKey);
  end; // if
  if RegOpenKeyEx(HKEY_CLASSES_ROOT, 'Directory\shellex\ContextMenuHandlers', 0, KEY_ALL_ACCESS,
    hRootKey) = ERROR_SUCCESS then
  begin
    if RegDeleteKey(hRootKey, 'miranda.shlext') <> ERROR_SUCCESS then
    begin
      MessageBox(0,
        'Unable to delete registry key for "Directory context menu handlers", this key may already be deleted or you may need admin rights.',
        'Problem', MB_ICONERROR);
    end; // if
    RegCloseKey(hRootKey);
  end; // if
  if ERROR_SUCCESS = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
    'Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved', 0, KEY_ALL_ACCESS,
    hRootKey) then
  begin
    if RegDeleteValue(hRootKey, '{72013A26-A94C-11d6-8540-A5E62932711D}') <> ERROR_SUCCESS then
    begin
      MessageBox(0,
        'Unable to delete registry entry for "Approved context menu handlers", this key may already be deleted or you may need admin rights.',
        'Problem', MB_ICONERROR);
    end; // if
    RegCloseKey(hRootKey);
  end; // if
  Result := S_OK;
end;

{ called by the options code to remove COM entries, and before that, get permission, if required.
}

procedure CheckUnregisterServer;
var
  sei: TSHELLEXECUTEINFO;
  szBuf: array [0 .. MAX_PATH * 2] of Char;
  szFileName: array [0 .. MAX_PATH] of Char;
begin
  if not VistaOrLater then
  begin
    RemoveCOMRegistryEntries();
    Exit;
  end;
  // launches regsvr to remove the dll under admin.
  GetModuleFileName(System.hInstance, szFileName, sizeof(szFileName));
  wsprintfs(szBuf, '/s /u "%s"', szFileName);
  ZeroMemory(@sei, sizeof(sei));
  sei.cbSize := sizeof(sei);
  sei.lpVerb := 'runas';
  sei.lpFile := 'regsvr32';
  sei.lpParameters := szBuf;
  ShellExecuteEx(sei);
  Sleep(1000);
  RemoveCOMRegistryEntries();
end;

{ Wow, I can't believe there isn't a direct API for this - 'runas' will invoke the UAC and ask
  for permission before installing the shell extension.  note the filepath arg has to be quoted }
procedure CheckRegisterServer;
var
  hRegKey: HKEY;
  sei: TSHELLEXECUTEINFO;
  szBuf: array [0 .. MAX_PATH * 2] of Char;
  szFileName: array [0 .. MAX_PATH] of Char;
begin
  if ERROR_SUCCESS = RegOpenKeyEx(HKEY_CLASSES_ROOT, 'miranda.shlext', 0, KEY_READ, hRegKey)
  then
  begin
    RegCloseKey(hRegKey);
  end
  else
  begin
    if VistaOrLater then
    begin
      MessageBox(0,
        'Shell context menus requires your permission to register with Windows Explorer (one time only).',
        'Miranda IM - Shell context menus (shlext.dll)', MB_OK or MB_ICONINFORMATION);
      // /s = silent
      GetModuleFileName(System.hInstance, szFileName, sizeof(szFileName));
      wsprintfs(szBuf, '/s "%s"', szFileName);
      ZeroMemory(@sei, sizeof(sei));
      sei.cbSize := sizeof(sei);
      sei.lpVerb := 'runas';
      sei.lpFile := 'regsvr32';
      sei.lpParameters := szBuf;
      ShellExecuteEx(sei);
    end;
  end;
end;

initialization

begin
  FillChar(dllpublic, sizeof(dllpublic), 0);
  IsMultiThread := True;
  VistaOrLater := GetProcAddress(GetModuleHandle('kernel32'), 'GetProductInfo') <> nil;
end;

end.
