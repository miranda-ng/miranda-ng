{$IFDEF M_API_UNIT}

function ProtoServiceExists(const proto,service: PAnsiChar): int;

function CreateVersionString(version:dword;buf:PAnsiChar):PAnsiChar;
function CreateVersionStringPlugin(pluginInfo:PPluginInfo;buf:PAnsiChar):PAnsiChar;
function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): int;
function PLUGIN_CMP_VERSION(verA: LongInt; verB: LongInt): int;

function mir_forkthread(aFunc:pThreadFunc; arg:pointer):THANDLE; {inline}
function mir_forkthreadex(aFunc:pThreadFuncEx; arg:pointer; stackSize:int; pThreadID:puint_ptr):THANDLE; {inline}
function mir_forkthreadowner(aFunc:pThreadFuncOwner; owner:pointer; arg:pointer; pThreadID:puint_ptr):THANDLE; {inline}

function Netlib_CloseHandle(Handle: THandle): int;
function Netlib_GetBase64DecodedBufferSize(const cchEncoded: int): int;
function Netlib_GetBase64EncodedBufferSize(const cbDecoded: int): int;
function Netlib_Send(hConn: THandle; const buf: PAnsiChar; len: int; flags: int): int_ptr;
function Netlib_Recv(hConn: THandle; const buf: PAnsiChar; len: int; flags: int): int_ptr;
procedure Netlib_Log(hNetLib: THandle; const sz: PAnsiChar);

function WindowList_Add(hList: THandle; hWnd: HWND; hContact: THandle): int_ptr;
function WindowList_Remove(hList: THandle; hWnd: THandle): int_ptr;
function WindowList_Find(hList: THandle; hContact: THandle): int_ptr;
function WindowList_Broadcast(hList: THandle; message: int; wParam: WPARAM; lParam: LPARAM): int_ptr;
function Utils_SaveWindowPosition(hWnd: THandle; hContact: THandle; const szModule, szNamePrefix: PAnsiChar): int_ptr;
function Utils_RestoreWindowPosition(hWnd: THandle; hContact: THandle; Flags: int; const szModule, szNamePrefix: PAnsiChar): int_ptr;

function mir_hashstr(const key:pAnsiChar):uint; {inline;}
function mir_hashstrW(const key:pWideChar):uint; {inline;}
function lrtrim(str:pAnsiChar):pAnsiChar; {inline}
function lrtrimp(str:pAnsiChar):pAnsiChar; {inline}
function mir_utf8decodeA(const src:pAnsiChar):pAnsiChar; {inline}

function Translate(sz: PAnsiChar): PAnsiChar;
function Translatew(sz: PWideChar): PWideChar;
function TranslateString(const sz: AnsiString): AnsiString;
function TranslateWideString(const sz:WideString):WideString;
procedure TranslateDialogDefault(hwndDlg: THandle);
procedure TranslateMenu(hMenu: HMENU);
function Langpack_PCharToTChar(astr:PAnsiChar):int_ptr;
function Langpack_Register:int_ptr;

function ProtoBroadcastAck(const szModule: PAnsiChar; hContact: THandle; type_: int; result_: int; hProcess: THandle; lParam: LPARAM): int_ptr;
function CreateProtoServiceFunction(const szModule, szService: PAnsiChar; serviceProc: TMIRANDASERVICE): int_ptr;

function LoadSkinnedIcon(id: int): THandle;
function LoadSkinnedProtoIcon(const szProto: PAnsiChar; status: int): THandle;
function SkinAddNewSound(const name, description, defaultFile: PAnsiChar): int_ptr;
function SkinPlaySound (const name: PAnsiChar): int_ptr;

function Menu_AddContextFrameMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddMainMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddContactMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddStatusMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddProtoMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddSubGroupMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddGroupMenuItem(mi:PCLISTMENUITEM):HGENMENU;
function Menu_AddTrayMenuItem(mi:PCLISTMENUITEM):HGENMENU;

function UserInfo_AddPage(wParam:WPARAM; odp:POPTIONSDIALOGPAGE):int_ptr;
function Options_AddPage(wParam:WPARAM; odp:POPTIONSDIALOGPAGE):int_ptr;
function Hotkey_Register(hk:PHOTKEYDESC):int_ptr;
function Skin_AddIcon(si:PSKINICONDESC):THANDLE;

procedure FontRegister(pFontID:pFontID);
procedure FontRegisterW(pFontID:pFontIDW);

procedure ColourRegister(pColorID:pColourID);
procedure ColourRegisterW(pColorID:pColourIDW);

procedure EffectRegister(pEffectID:pEffectID);
procedure EffectRegisterW(pEffectID:pEffectIDW);

{$ELSE}

function ProtoServiceExists(const proto,service: PAnsiChar): int;
var
  buf:array [0..127] of AnsiChar;
begin
  if (proto=nil) or (service=nil) then
  begin
    result:=0;
    exit;
  end;
  lStrCpyA(@buf,proto);
  lStrCatA(@buf,service);
  result:=ServiceExists(@buf);
end;


function CreateVersionString(version:dword;buf:PAnsiChar):PAnsiChar;
var
  vers:array [0..3] of integer;
begin
  vers[0]:=(version shr 24) and $FF;
  vers[1]:=(version shr 16) and $FF;
  vers[2]:=(version shr  8) and $FF;
  vers[3]:=version and $FF;
  wvsprintfa(buf,'%d.%d.%d.%d',@vers);
  result:=buf;
end;

function CreateVersionStringPlugin(pluginInfo:PPluginInfo;buf:PAnsiChar):PAnsiChar;
begin
  result:=CreateVersionString(pluginInfo^.version,buf);
end;

function PLUGIN_MAKE_VERSION(a,b,c,d: Cardinal): int;
begin
  Result := (a shl 24) or (b shl 16) or (c shl 8) or d;
end;

function PLUGIN_CMP_VERSION(verA: LongInt; verB: LongInt): int;
begin
  Result := 0;
  { could be used to compare for severity of age for positive values, if a<b
  results are minus values, 0 for equal, positive if a is newer }
  Inc(Result, (verA and $FF      ) - (verB and $FF));
  Inc(Result, (verA and $FF00    ) - (verB and $FF00));
  Inc(Result, (verA and $FF0000  ) - (verB and $FF0000));
  Inc(Result, (verA and $FF000000) - (verB and $FF000000));
end;


function mir_forkthread(aFunc:pThreadFunc; arg:pointer):THANDLE; {inline}
begin
	result:=forkthread(aFunc, 0, arg);
end;
function mir_forkthreadex(aFunc:pThreadFuncEx; arg:pointer; stackSize:int; pThreadID:puint_ptr):THANDLE; {inline}
begin
	result:=forkthreadex(nil, stackSize, aFunc, nil, arg, pThreadID);
end;
function mir_forkthreadowner(aFunc:pThreadFuncOwner; owner:pointer; arg:pointer; pThreadID:puint_ptr):THANDLE; {inline}
begin
	result:=forkthreadex(nil, 0, pThreadFuncEx(aFunc), owner, arg, pThreadID);
end;


function Netlib_CloseHandle(Handle: THandle): int;
begin
  Result := CallService(MS_NETLIB_CLOSEHANDLE, Handle, 0);
end;

function Netlib_GetBase64DecodedBufferSize(const cchEncoded: int): int;
begin
  Result := (cchEncoded shr 2) * 3;
end;

function Netlib_GetBase64EncodedBufferSize(const cbDecoded: int): int;
begin
  Result := (cbDecoded * 4+11) div 12*4+1;
end;

function Netlib_Send(hConn: THandle; const buf: PAnsiChar; len: int; flags: int): int_ptr;
var
  nlb: TNETLIBBUFFER;
begin
  nlb.buf   := buf;
  nlb.len   := len;
  nlb.flags := flags;
  Result := CallService(MS_NETLIB_SEND, wParam(hConn), lParam(@nlb));
end;

function Netlib_Recv(hConn: THandle; const buf: PAnsiChar; len: int; flags: int): int_ptr;
var
  nlb: TNETLIBBUFFER;
begin
  nlb.buf   := buf;
  nlb.len   := len;
  nlb.flags := flags;
  Result := CallService(MS_NETLIB_RECV, wParam(hConn), lParam(@nlb));
end;

procedure Netlib_Log(hNetLib: THandle; const sz: PAnsiChar);
begin
  CallService(MS_NETLIB_LOG, hNetLib, lParam(sz));
end;


function WindowList_Add(hList: THandle; hWnd: hWnd; hContact: THandle): int_ptr;
var
  wle: TWINDOWLISTENTRY;
begin
  wle.hList    := hList;
  wle.hWnd     := hWnd;
  wle.hContact := hContact;
  Result := CallService(MS_UTILS_ADDTOWINDOWLIST, 0, lParam(@wle));
end;

function WindowList_Remove(hList: THandle; hWnd: THandle): int_ptr;
begin
  Result := CallService(MS_UTILS_REMOVEFROMWINDOWLIST, hList, hWnd);
end;

function WindowList_Find(hList: THandle; hContact: THandle): int_ptr;
begin
  Result := CallService(MS_UTILS_FINDWINDOWINLIST, hList, hContact);
end;

function WindowList_Broadcast(hList: THandle; message: int; wParam: WPARAM; lParam: LPARAM): int_ptr;
var
  msg: TMSG;
begin
  msg.message := message;
  msg.wParam  := wParam;
  msg.lParam  := lParam;
  Result := CallService(MS_UTILS_BROADCASTTOWINDOWLIST, hList, tLparam(@Msg));
end;

function Utils_SaveWindowPosition(hWnd: THandle; hContact: THandle; const szModule, szNamePrefix: PAnsiChar): int_ptr;
var
  swp: TSAVEWINDOWPOS;
begin
  swp.hWnd     := hWnd;
  swp.hContact := hContact;
  swp.szModule := szModule;
  swp.szNamePrefix := szNamePrefix;
  Result := CallService(MS_UTILS_SAVEWINDOWPOSITION, 0, lParam(@swp));
end;

function Utils_RestoreWindowPosition(hWnd: THandle; hContact: THandle; Flags: int; const szModule, szNamePrefix: PAnsiChar): int_ptr;
var
  swp: TSAVEWINDOWPOS;
begin
  swp.hWnd         := hWnd;
  swp.hContact     := hContact;
  swp.szModule     := szModule;
  swp.szNamePrefix := szNamePrefix;
  Result := CallService(MS_UTILS_RESTOREWINDOWPOSITION, Flags, lParam(@swp));
end;


function mir_hashstr(const key:pAnsiChar):uint; {inline;}
var
  len:int;
begin
  if key=NIL then
    result:=0
  else
  begin
    len:=lstrlena(key);
    result:=mir_hash(key, len);
  end;
end;

function mir_hashstrW(const key:pWideChar):uint; {inline;}
var
  len:int;
begin
  if key=NIL then
    result:=0
  else
  begin
    len:=lstrlenw(key);
    result:=mir_hash(key, len*SizeOF(WideChar));
  end;
end;

function lrtrim(str:pAnsiChar):pAnsiChar; {inline}
begin
  result:=ltrim(rtrim(str));
end;
function lrtrimp(str:pAnsiChar):pAnsiChar; {inline}
begin
  result:=ltrimp(rtrim(str));
end;

function mir_utf8decodeA(const src:pAnsiChar):pAnsiChar; {inline}
type
 ppWideChar = ^pWidechar;
var
  tmp:pAnsiChar;
begin
  tmp:=mir_strdup(src);
  mir_utf8decode(tmp,ppWideChar(nil)^);
  result:=tmp;
end;


function TranslateW(sz: PWideChar): PWideChar;
begin
  result:=TranslateW_LP(sz, hLangpack);
end;

function Translate(sz: PAnsiChar): PAnsiChar;
begin
  result:=TranslateA_LP(sz, hLangpack);
end;

function TranslateString(const sz:AnsiString):AnsiString;
begin
  Result:=AnsiString(PAnsiChar(
    CallService(MS_LANGPACK_TRANSLATESTRING,hLangpack,lparam(@sz))));
end;

function TranslateWideString(const sz:WideString):WideString;
begin
  Result:=WideString(PWideChar(
    CallService(MS_LANGPACK_TRANSLATESTRING, LANG_UNICODE, lParam(sz))));
end;

procedure TranslateDialogDefault(hwndDlg: THandle);
begin
  TranslateDialog_LP(hwndDlg,hLangpack);
end;

procedure TranslateMenu(hMenu: HMENU);
begin
  TranslateMenu_LP(hMenu,hLangpack);
end;

function Langpack_PCharToTChar(astr:PAnsiChar):int_ptr;
begin
  result:=CallService(MS_LANGPACK_PCHARTOTCHAR, hLangpack, LPARAM(astr));
end;

function Langpack_Register:int_ptr;
begin
  result:=CallService(MS_LANGPACK_REGISTER,WPARAM(@hLangpack),LPARAM(@PluginInfo));
end;


function ProtoBroadcastAck(const szModule: PAnsiChar; hContact: THandle; type_: int; result_: int; hProcess: THandle; lParam: LPARAM): int_ptr;
var
  ack: TACKDATA;
begin
  ack.cbSize   := sizeof(TACKDATA);
  ack.szModule := szModule;
  ack.hContact := hContact;
  ack._type    := type_;
  ack._result  := result_;
  ack.hProcess := hProcess;
  ack.lParam   := lParam;
  Result := CallService(MS_PROTO_BROADCASTACK, 0, tlParam(@ack));
end;

function CreateProtoServiceFunction(const szModule, szService: PAnsiChar; serviceProc: TMIRANDASERVICE): int_ptr;
var
  szStr: array[0..MAXMODULELABELLENGTH*2] of AnsiChar;
begin
  if (szModule=nil) or (szService=nil) then
  begin
    result:=0;
    exit;
  end;
  lstrcpya(szStr, szModule);
  lstrcata(szStr, szService);
  Result := CreateServiceFunction(szStr, @serviceProc);
end;


function LoadSkinnedIcon(id: int): THandle;
begin
  Result := CallService(MS_SKIN_LOADICON, id, 0);
end;

function LoadSkinnedProtoIcon(const szProto: PAnsiChar; status: int): THandle;
begin
  Result := CallService(MS_SKIN_LOADPROTOICON, wParam(szProto), status);
end;

function SkinAddNewSound(const name, description, defaultFile: PAnsiChar): int_ptr;
var
  ssd: TSKINSOUNDDESCEX;
begin
  FillChar(ssd,SizeOf(ssd),0);
  ssd.cbSize         := sizeof(ssd);
  ssd.pszName        := name;
  ssd.pszDescription.a := description;
  ssd.pszDefaultFile.a := defaultFile;
  Result := CallService(MS_SKIN_ADDNEWSOUND, hLangpack, lParam(@ssd));
end;

function SkinPlaySound (const name: PAnsiChar): int_ptr;
begin
  Result := CallService(MS_SKIN_PLAYSOUND, 0, lParam(name));
end;


function Menu_AddContextFrameMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDCONTEXTFRAMEMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddMainMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDMAINMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddContactMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDCONTACTMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddStatusMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDSTATUSMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddProtoMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDPROTOMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddSubGroupMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDSUBGROUPMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddGroupMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDGROUPMENUITEM, 0,LPARAM(mi));
end;

function Menu_AddTrayMenuItem(mi:PCLISTMENUITEM):HGENMENU;
begin
  mi^.hLangpack:=hLangpack;
  result:=CallService(MS_CLIST_ADDTRAYMENUITEM, 0,LPARAM(mi));
end;


function UserInfo_AddPage(wParam:WPARAM; odp:POPTIONSDIALOGPAGE):int_ptr;
begin
  odp^.hLangpack:=hLangpack;
	result:=CallService(MS_USERINFO_ADDPAGE, wParam, LPARAM(odp));
end;

function Options_AddPage(wParam:WPARAM; odp:POPTIONSDIALOGPAGE):int_ptr;
begin
  odp^.hLangpack:=hLangpack;
	result:=CallService(MS_OPT_ADDPAGE, wParam, LPARAM(odp));
end;

function Hotkey_Register(hk:PHOTKEYDESC):int_ptr;
begin
	result:=CallService(MS_HOTKEY_REGISTER, hLangpack, LPARAM(hk));
end;

function Skin_AddIcon(si:PSKINICONDESC):THANDLE;
begin
  result:=CallService(MS_SKIN2_ADDICON, hLangpack, LPARAM(si));
end;

procedure FontRegister(pFontID:pFontID);
begin
	CallService('Font/Register', WPARAM(pFontID), hLangpack);
end;
procedure FontRegisterW(pFontID:pFontIDW);
begin
	CallService('Font/RegisterW', WPARAM(pFontID), hLangpack);
end;

procedure ColourRegister(pColorID:pColourID);
begin
	CallService('Colour/Register', WPARAM(pColorID), hLangpack);
end;
procedure ColourRegisterW(pColorID:pColourIDW);
begin
	CallService('Colour/RegisterW', WPARAM(pColorID), hLangpack);
end;

procedure EffectRegister(pEffectID:pEffectID);
begin
	CallService('Effect/Register', WPARAM(pEffectID), hLangpack);
end;
procedure EffectRegisterW(pEffectID:pEffectIDW);
begin
	CallService('Effect/RegisterW', WPARAM(pEffectID), hLangpack);
end;

{$ENDIF}
